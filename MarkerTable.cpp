#include "MarkerTable.h"
#include "ImagePreviewer.h"

#include <QMessageBox>
#include <QtGui/QColor>

#include <QTransform>
#include <QPolygonF>
#include <QThread>

#include <QDebug>
#include <cmath>

QVector<QString> header_data{"name","x","y","remove"};

/* * * * * * * * * * * * * * *
 *       Calculate           *
 * * * * * * * * * * * * * * */

void CalLinMode(QVector<QPointF>& axe_p, QVector<QPointF>& axe_real,
                QVector<QPointF>& cur_p, QVector<QPointF>& cur_real,
                qreal height){

    auto p1 = QPointF(axe_p.at(0).x(), height - axe_p.at(0).y());
    auto p2 = QPointF(axe_p.at(1).x(), height - axe_p.at(1).y());
    auto p3 = QPointF(axe_p.at(2).x(), height - axe_p.at(2).y());

    auto d1 = axe_real.at(0);
    auto d2 = axe_real.at(1);
    auto d3 = axe_real.at(2);

    auto solve = [](double x1, double y1, double x2, double y2, double x3, double y3,
                    double r1, double r2, double r3) -> QVector3D {
        double det = x1*(y2 - y3) - y1*(x2 - x3) + (x2*y3 - x3*y2);
        if (qAbs(det) < 1e-9) return QVector3D(0, 0, 0); // 共线异常

        double a = ((y2 - y3)*r1 + (y3 - y1)*r2 + (y1 - y2)*r3) / det;
        double b = ((x3 - x2)*r1 + (x1 - x3)*r2 + (x2 - x1)*r3) / det;
        double c = ((x2*y3 - x3*y2)*r1 + (x3*y1 - x1*y3)*r2 + (x1*y2 - x2*y1)*r3) / det;
        return QVector3D(a, b, c);
    };

    QVector3D resX = solve(p1.x(), p1.y(), p2.x(), p2.y(), p3.x(), p3.y(), d1.x(), d2.x(), d3.x());
    QVector3D resY = solve(p1.x(), p1.y(), p2.x(), p2.y(), p3.x(), p3.y(), d1.y(), d2.y(), d3.y());

    // m11=a, m12=d, m13=0
    // m21=b, m22=e, m23=0
    // m31=c, m32=f, m33=1
    QTransform transform =  QTransform(resX.x(), resY.x(), 0,
                                       resX.y(), resY.y(), 0,
                                       resX.z(), resY.z(), 1);

    for (qsizetype idx = 0; idx < cur_p.size(); idx++){
        cur_real[idx] = transform.map(QPointF(cur_p.at(idx).x(), height - cur_p.at(idx).y()));
    }
}

// 计算单个轴的映射值
// pixel: 像素坐标
// p1, p2: 两个参考点的像素坐标和实际值
// is_log: 是否对数轴
// 返回值: 实际值
double MapAxisValue(double pixel, double p1_pixel, double p2_pixel, 
                    double v1, double v2, bool is_log) {
    // 检查除零
    if (qAbs(p2_pixel - p1_pixel) < 1e-9) {
        return v1;
    }
    
    // 像素位置的比例
    double ratio = (pixel - p1_pixel) / (p2_pixel - p1_pixel);
    
    if (is_log) {
        // 对数轴：在log空间中线性插值
        double log_v1 = log10(v1);
        double log_v2 = log10(v2);
        double log_result = log_v1 + ratio * (log_v2 - log_v1);
        return pow(10.0, log_result);
    } else {
        // 线性轴：直接线性插值
        return v1 + ratio * (v2 - v1);
    }
}

// x_type: 0=linear, 1=log
// y_type: 0=linear, 1=log
// 从3个参考点中找出X/Y方向的极值点来确定映射
void CalMixedMode(QVector<QPointF>& axe_p, QVector<QPointF>& axe_real,
                  QVector<QPointF>& cur_p, QVector<QPointF>& cur_real,
                  qreal height, int x_type, int y_type){
    
    if (axe_p.size() < 2) return;
    
    cur_real.resize(cur_p.size());
    
    // 找出X方向的极值点（最小和最大X）
    int x_min_idx = 0, x_max_idx = 0;
    for (int i = 1; i < axe_p.size(); i++) {
        if (axe_p[i].x() < axe_p[x_min_idx].x()) x_min_idx = i;
        if (axe_p[i].x() > axe_p[x_max_idx].x()) x_max_idx = i;
    }
    
    // 找出Y方向的极值点（最小和最大Y，注意Y坐标翻转）
    int y_min_idx = 0, y_max_idx = 0;
    for (int i = 1; i < axe_p.size(); i++) {
        double y_curr = height - axe_p[i].y();
        double y_min = height - axe_p[y_min_idx].y();
        double y_max = height - axe_p[y_max_idx].y();
        if (y_curr < y_min) y_min_idx = i;
        if (y_curr > y_max) y_max_idx = i;
    }
    
    // X轴映射参数（使用X极值点）
    double px_min = axe_p[x_min_idx].x();
    double px_max = axe_p[x_max_idx].x();
    double vx_min = axe_real[x_min_idx].x();
    double vx_max = axe_real[x_max_idx].x();
    
    // Y轴映射参数（使用Y极值点）
    double py_min = height - axe_p[y_min_idx].y();
    double py_max = height - axe_p[y_max_idx].y();
    double vy_min = axe_real[y_min_idx].y();
    double vy_max = axe_real[y_max_idx].y();
    
    for (qsizetype idx = 0; idx < cur_p.size(); idx++){
        double px = cur_p[idx].x();
        double py = height - cur_p[idx].y();
        
        double x_value = MapAxisValue(px, px_min, px_max, vx_min, vx_max, x_type == 1);
        double y_value = MapAxisValue(py, py_min, py_max, vy_min, vy_max, y_type == 1);
        
        cur_real[idx] = QPointF(x_value, y_value);
    }
}

void CalAixMode(QVector<QPointF>& axe_p, QVector<QPointF>& axe_real,
                QVector<QPointF>& cur_p, QVector<QPointF>& cur_real,
                qreal height){
    // 轴向模式（预留）
    CalLinMode(axe_p, axe_real, cur_p, cur_real, height);
}


/* * * * * * * * * * * * * * *
 *       MarkerTable         *
 * * * * * * * * * * * * * * */

MarkerTable::MarkerTable(ImagePreviewer* viwer)
    : m_viwer(viwer){
    // prepare for store value
    axe_real.resize(3);

    connect(m_viwer, &ImagePreviewer::AddRow, this, &MarkerTable::InsertData);
    connect(m_viwer, &ImagePreviewer::ClearData, this, &MarkerTable::ClearData);
    connect(this, &MarkerTable::RowDeleted, m_viwer, &ImagePreviewer::DelMarkerData);
}

int MarkerTable::rowCount(const QModelIndex & /* parent */) const  {
    return 3 + cur_points.size();
}

int MarkerTable::columnCount(const QModelIndex & /* parent */) const  {
    /* name x y btn */
    return 4;
}

QVariant MarkerTable::data(const QModelIndex &index, int role) const {
    // show data
    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        switch (index.column()) {
        case 0:
            if (index.row() < 3) {
                return QString("axe");
            }else{
                return QString("curve");
            }
            break;
        case 1:
            // x position
            if (index.row() < 3) {
                if (index.row() < axe_points.size()) {
                    return axe_real.at(index.row()).x();
                }else {
                    return QString("-");
                }
            }else{
                if (index.row() < cur_points.size() + 3) {
                    return cur_real.at(index.row() - 3).x();
                }else {
                    return QString("-");
                }
            }
            break;
        case 2:
            // y position
            if (index.row() < 3) {
                if (index.row() < axe_points.size()) {
                    return axe_real.at(index.row()).y();
                }else {
                    return QString("-");
                }
            }else{
                if (index.row() < cur_points.size() + 3) {
                    return cur_real.at(index.row() - 3).y();
                }else {
                    return QString("-");
                }
            }
            break;
        case 3:
            return QString("remove");
            break;
        default:
            break;
        }
    }
    // color
    if (role == Qt::BackgroundRole) {
        if (index.row() < 3) {
            return QColor(Qt::gray);
        }else{
            return QColor(Qt::magenta);
        }
    }
    // center
    if (role == Qt::TextAlignmentRole) {
        return Qt::AlignCenter;
    }
    // edit

    return QVariant();
}

QVariant MarkerTable::headerData(int section, Qt::Orientation orientation, int role) const {

    if (role != Qt::DisplayRole){
        return QVariant();
    }

    if (orientation == Qt::Horizontal){
        return header_data.at(section);
    }

    return QVariant();
}

bool MarkerTable::setData(const QModelIndex &index, const QVariant &value, int role){
    // editor
    if (role == Qt::EditRole) {
        if (index.column() != 1 && index.column() != 2 ) return false;;

        if (index.row() < 3) {
            qDebug() << "edit data in axe points";
            // axe data
            if (index.row() > axe_points.size() - 1) return false;
            // ok
            if (index.column() == 1) axe_real[index.row()].setX(value.toDouble());
            else axe_real[index.row()].setY(value.toDouble());
        }else {
            qDebug() << "edit data in curve points";
            // cur data
            if (index.row() > cur_points.size() + 2) return false;
            // ok
            if (index.column() == 1) cur_real[index.row() - 3].setX(value.toDouble());
            else cur_real[index.row() - 3].setY(value.toDouble());
        }
        return true;
    }

    return QAbstractTableModel::setData(index, value, role);
}

void MarkerTable::InsertData(bool is_axe, qreal x, qreal y){
    
    beginResetModel();
    if (is_axe){
        axe_points.emplace_back(QPointF(x, y));
    }else {
        cur_points.emplace_back(QPointF(x, y));
        cur_real.emplace_back(QPointF(x, y));
    }
    endResetModel();
}

void MarkerTable::ClearData(){
    beginResetModel();
    axe_points.clear();
    cur_points.clear();
    cur_real.clear();
    endResetModel();
}

void MarkerTable::DeleteRow(int row){
    qDebug() << "Table delete row : " << row + 1 << " , axe size : " << axe_points.size();  
    // protect
    if (row > axe_points.size() - 1) return;

    QMessageBox::StandardButton reply = QMessageBox::question( m_viwer,
            "delete data",
            "please make sure that you want delete this line",
            QMessageBox::Yes | QMessageBox::No);

    if (row >= 0 && reply == QMessageBox::Yes){
        beginResetModel();
        if (row < 3){
            axe_points.removeAt(row);
        }else{
            cur_points.removeAt(row - 3);
            cur_real.removeAt(row - 3);
        }
        endResetModel();
        emit RowDeleted(row);
    }
}

Qt::ItemFlags MarkerTable::flags(const QModelIndex &index) const
{
    if (!index.isValid()) return Qt::NoItemFlags;

    Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;

    if (index.column() != 1 && index.column() != 2) return flags;

    flags |= Qt::ItemIsEditable;

    return flags;
}

void MarkerTable::CalRelData(int x_type, int y_type){

    if (axe_points.size() < 2) {
        QMessageBox::warning(m_viwer, "warning", "not enough axe point (need at least 2)");
        return;
    }

    if (cur_points.size() < 1) {
        QMessageBox::warning(m_viwer, "warning", "there no data point of current curve");
        return;
    }

    // 检查对数轴的参考值是否为正
    if (x_type == 1) {  // X轴是对数轴
        for (int i = 0; i < axe_real.size() && i < 3; i++) {
            if (axe_real[i].x() <= 0) {
                QMessageBox::warning(m_viwer, "warning", 
                    QString("X轴为对数轴，要求所有X参考值必须大于0\n"
                            "点%1的X值为: %2").arg(i+1).arg(axe_real[i].x()));
                return;
            }
        }
    }
    if (y_type == 1) {  // Y轴是对数轴
        for (int i = 0; i < axe_real.size() && i < 3; i++) {
            if (axe_real[i].y() <= 0) {
                QMessageBox::warning(m_viwer, "warning", 
                    QString("Y轴为对数轴，要求所有Y参考值必须大于0\n"
                            "点%1的Y值为: %2").arg(i+1).arg(axe_real[i].y()));
                return;
            }
        }
    }

    // x_type: 0=linear, 1=log
    // y_type: 0=linear, 1=log
    if (x_type == 0 && y_type == 0) {
        // 双线性
        CalLinMode(axe_points, axe_real, cur_points, cur_real, m_viwer->GetImgHeight());
    } else {
        // 包含对数的混合模式
        CalMixedMode(axe_points, axe_real, cur_points, cur_real, 
                     m_viwer->GetImgHeight(), x_type, y_type);
    }
    
    // 通知视图数据已更新
    emit dataChanged(index(0, 1), index(rowCount(QModelIndex()) - 1, 2));
}
