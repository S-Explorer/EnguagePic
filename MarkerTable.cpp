#include "MarkerTable.h"
#include "ImagePreviewer.h"

#include <QMessageBox>
#include <QtGui/QColor>

#include <QTransform>
#include <QPolygonF>
#include <QThread>

#include <QDebug>

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

void CalLogMode(QVector<QPointF>& axe_p, QVector<QPointF>& axe_real,
                QVector<QPointF>& cur_p, QVector<QPointF>& cur_real){

}

void CalAixMode(QVector<QPointF>& axe_p, QVector<QPointF>& axe_real,
                QVector<QPointF>& cur_p, QVector<QPointF>& cur_real){

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

void MarkerTable::CalRelData(int type){

    if (axe_points.size() < 3) {
        QMessageBox::warning(m_viwer, "warning", "not enough axe point");
        return;
    }

    if (cur_points.size() < 1) {
        QMessageBox::warning(m_viwer, "warning", "there no data point of current curve");
        return;
    }

    CalLinMode(axe_points, axe_real, cur_points, cur_real, m_viwer->GetImgHeight());
}
