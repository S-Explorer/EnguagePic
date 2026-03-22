#include "MarkerTable.h"
#include "DataModel.h"
#include <QMessageBox>
#include <QtGui/QColor>
#include <QDebug>
#include <QtMath>

QVector<QString> header_data{"name","x","y","remove"};

/* * * * * * * * * * * * * * *
 *       MarkerTable         *
 * * * * * * * * * * * * * * */

MarkerTable::MarkerTable(DataManager* manager, QObject* parent)
    : QAbstractTableModel(parent), dataManager(manager), currentAxis(nullptr), currentCurve(nullptr) {
    
    if (dataManager) {
        connect(dataManager, &DataManager::currentAxisChanged, 
                this, &MarkerTable::onCurrentAxisChanged);
        connect(dataManager, &DataManager::currentCurveChanged, 
                this, &MarkerTable::onCurrentCurveChanged);
    }
}

void MarkerTable::setCurrentCurve(CurveData* curve) {
    // 断开旧曲线的信号
    if (currentCurve && currentCurve != curve) {
        disconnect(currentCurve, &CurveData::dataChanged, 
                  this, &MarkerTable::onCurveDataChanged);
    }
    
    currentCurve = curve;
    
    // 连接新曲线的信号
    if (currentCurve) {
        connect(currentCurve, &CurveData::dataChanged, 
               this, &MarkerTable::onCurveDataChanged);
    }
    
    // 总是刷新模型
    beginResetModel();
    endResetModel();
}

void MarkerTable::setCurrentAxis(AxisData* axis) {
    // 断开旧坐标轴的信号
    if (currentAxis && currentAxis != axis) {
        disconnect(currentAxis, &AxisData::axePointsChanged,
                  this, &MarkerTable::onCurveDataChanged);
    }
    
    currentAxis = axis;
    
    // 连接新坐标轴的信号
    if (currentAxis) {
        connect(currentAxis, &AxisData::axePointsChanged,
               this, &MarkerTable::onCurveDataChanged);
    }
    
    // 总是刷新模型
    beginResetModel();
    endResetModel();
}

void MarkerTable::onCurrentCurveChanged(CurveData* curve) {
    // 当曲线改变时，确保坐标轴也是正确的
    // 注意：DataManager 会在 setCurrentCurve 时保持坐标轴不变
    
    // 先确保坐标轴正确
    if (dataManager && dataManager->currentAxis != currentAxis) {
        if (currentAxis) {
            disconnect(currentAxis, &AxisData::axePointsChanged,
                      this, &MarkerTable::onCurveDataChanged);
        }
        
        currentAxis = dataManager->currentAxis;
        
        if (currentAxis) {
            connect(currentAxis, &AxisData::axePointsChanged,
                   this, &MarkerTable::onCurveDataChanged);
        }
    }
    
    // 然后更新曲线
    if (currentCurve != curve) {
        if (currentCurve) {
            disconnect(currentCurve, &CurveData::dataChanged,
                      this, &MarkerTable::onCurveDataChanged);
        }
        
        currentCurve = curve;
        
        if (currentCurve) {
            connect(currentCurve, &CurveData::dataChanged,
                   this, &MarkerTable::onCurveDataChanged);
        }
    }
    
    // 统一重置模型
    beginResetModel();
    endResetModel();
}

void MarkerTable::onCurrentAxisChanged(AxisData* axis) {
    // 当坐标轴改变时，同时更新坐标轴和曲线
    // 注意：DataManager 会在 setCurrentAxis 中自动设置当前曲线
    // 所以我们需要同时更新两者，但只重置一次模型
    
    // 先更新坐标轴（不触发模型重置）
    if (currentAxis != axis) {
        if (currentAxis) {
            disconnect(currentAxis, &AxisData::axePointsChanged,
                      this, &MarkerTable::onCurveDataChanged);
        }
        
        currentAxis = axis;
        
        if (currentAxis) {
            connect(currentAxis, &AxisData::axePointsChanged,
                   this, &MarkerTable::onCurveDataChanged);
        }
    }
    
    // 然后更新曲线（不触发模型重置）
    if (dataManager && dataManager->currentCurve != currentCurve) {
        if (currentCurve) {
            disconnect(currentCurve, &CurveData::dataChanged,
                      this, &MarkerTable::onCurveDataChanged);
        }
        
        currentCurve = dataManager->currentCurve;
        
        if (currentCurve) {
            connect(currentCurve, &CurveData::dataChanged,
                   this, &MarkerTable::onCurveDataChanged);
        }
    }
    
    // 最后统一重置模型
    beginResetModel();
    endResetModel();
}

void MarkerTable::onCurveDataChanged() {
    // 通知视图数据已更新
    emit QAbstractTableModel::dataChanged(index(0, 0), index(rowCount(QModelIndex()) - 1, columnCount(QModelIndex()) - 1));
}

int MarkerTable::getTotalRowCount() const {
    int count = 0;
    
    // 坐标轴点（最多3个）
    if (currentAxis) {
        count += currentAxis->axePointCount();
    }
    
    // 曲线点
    if (currentCurve) {
        count += currentCurve->pointCount();
    }
    
    return count;
}

int MarkerTable::rowCount(const QModelIndex & /* parent */) const  {
    return getTotalRowCount();
}

int MarkerTable::columnCount(const QModelIndex & /* parent */) const  {
    /* name x y btn */
    return 4;
}

QVariant MarkerTable::data(const QModelIndex &index, int role) const {
    if (!currentAxis) return QVariant();
    
    // show data
    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        switch (index.column()) {
        case 0: {
            // name
            if (isAxeRow(index.row())) {
                return QString("axe");
            } else {
                return QString("curve");
            }
        }
        case 1: {
            // x position
            if (isAxeRow(index.row())) {
                int axeIdx = getAxePointIndex(index.row());
                if (axeIdx < currentAxis->axeRealPoints.size()) {
                    return currentAxis->axeRealPoints[axeIdx].x();
                }
            } else {
                if (currentCurve) {
                    int curveIdx = getCurvePointIndex(index.row());
                    if (curveIdx < currentCurve->realPoints.size()) {
                        return currentCurve->realPoints[curveIdx].x();
                    }
                }
            }
            return QString("-");
        }
        case 2: {
            // y position
            if (isAxeRow(index.row())) {
                int axeIdx = getAxePointIndex(index.row());
                if (axeIdx < currentAxis->axeRealPoints.size()) {
                    return currentAxis->axeRealPoints[axeIdx].y();
                }
            } else {
                if (currentCurve) {
                    int curveIdx = getCurvePointIndex(index.row());
                    if (curveIdx < currentCurve->realPoints.size()) {
                        return currentCurve->realPoints[curveIdx].y();
                    }
                }
            }
            return QString("-");
        }
        case 3:
            return QString("remove");
        default:
            break;
        }
    }
    
    // color
    if (role == Qt::BackgroundRole) {
        if (isAxeRow(index.row())) {
            return QColor(Qt::gray);
        } else {
            return QColor(Qt::magenta);
        }
    }
    
    // center
    if (role == Qt::TextAlignmentRole) {
        return Qt::AlignCenter;
    }
    
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
    if (!currentAxis) return false;
    
    // editor
    if (role == Qt::EditRole) {
        if (index.column() != 1 && index.column() != 2) return false;

        if (isAxeRow(index.row())) {
            // axe data
            int axeIdx = getAxePointIndex(index.row());
            if (axeIdx >= currentAxis->axeRealPoints.size()) return false;
            
            if (index.column() == 1) {
                currentAxis->axeRealPoints[axeIdx].setX(value.toDouble());
            } else {
                currentAxis->axeRealPoints[axeIdx].setY(value.toDouble());
            }
        } else {
            // curve data
            if (!currentCurve) return false;
            
            int curveIdx = getCurvePointIndex(index.row());
            if (curveIdx >= currentCurve->realPoints.size()) return false;
            
            if (index.column() == 1) {
                currentCurve->realPoints[curveIdx].setX(value.toDouble());
            } else {
                currentCurve->realPoints[curveIdx].setY(value.toDouble());
            }
        }
        
        emit QAbstractTableModel::dataChanged(index, index);
        return true;
    }

    return QAbstractTableModel::setData(index, value, role);
}

void MarkerTable::DeleteRow(int row){
    if (!currentAxis) return;
    
    QMessageBox::StandardButton reply = QMessageBox::question(
        nullptr,  // 使用nullptr，因为parent可能不是QWidget
        "delete data",
        "please make sure that you want delete this line",
        QMessageBox::Yes | QMessageBox::No);

    if (reply != QMessageBox::Yes) return;

    beginResetModel();
    
    if (isAxeRow(row)) {
        // 删除坐标轴点
        int axeIdx = getAxePointIndex(row);
        if (axeIdx < currentAxis->axePointCount()) {
            currentAxis->removeAxePoint(axeIdx);
        }
    } else {
        // 删除曲线点
        if (currentCurve) {
            int curveIdx = getCurvePointIndex(row);
            if (curveIdx < currentCurve->pointCount()) {
                currentCurve->removePoint(curveIdx);
            }
        }
    }
    
    endResetModel();
    emit RowDeleted(row);
}

Qt::ItemFlags MarkerTable::flags(const QModelIndex &index) const
{
    if (!index.isValid()) return Qt::NoItemFlags;

    Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;

    if (index.column() != 1 && index.column() != 2) return flags;

    flags |= Qt::ItemIsEditable;

    return flags;
}

// 计算单个轴的映射值
static double MapAxisValue(double pixel, double p1_pixel, double p2_pixel, 
                           double v1, double v2, bool is_log) {
    if (qAbs(p2_pixel - p1_pixel) < 1e-9) {
        return v1;
    }
    
    double ratio = (pixel - p1_pixel) / (p2_pixel - p1_pixel);
    
    if (is_log) {
        double log_v1 = log10(v1);
        double log_v2 = log10(v2);
        double log_result = log_v1 + ratio * (log_v2 - log_v1);
        return pow(10.0, log_result);
    } else {
        return v1 + ratio * (v2 - v1);
    }
}

void MarkerTable::CalRelData(int x_type, int y_type){
    if (!currentAxis) {
        QMessageBox::warning(nullptr, "warning", "no axis selected");
        return;
    }
    
    if (currentAxis->axePointCount() < 2) {
        QMessageBox::warning(nullptr, "warning", "not enough axe point (need at least 2)");
        return;
    }

    if (!currentCurve || currentCurve->pointCount() < 1) {
        QMessageBox::warning(nullptr, "warning", "there no data point of current curve");
        return;
    }

    // 检查对数轴的参考值是否为正
    if (x_type == 1) {  // X轴是对数轴
        for (int i = 0; i < currentAxis->axeRealPoints.size() && i < 3; i++) {
            if (currentAxis->axeRealPoints[i].x() <= 0) {
                QMessageBox::warning(nullptr, "warning", 
                    QString("X轴为对数轴，要求所有X参考值必须大于0\n"
                            "点%1的X值为: %2").arg(i+1).arg(currentAxis->axeRealPoints[i].x()));
                return;
            }
        }
    }
    if (y_type == 1) {  // Y轴是对数轴
        for (int i = 0; i < currentAxis->axeRealPoints.size() && i < 3; i++) {
            if (currentAxis->axeRealPoints[i].y() <= 0) {
                QMessageBox::warning(nullptr, "warning", 
                    QString("Y轴为对数轴，要求所有Y参考值必须大于0\n"
                            "点%1的Y值为: %2").arg(i+1).arg(currentAxis->axeRealPoints[i].y()));
                return;
            }
        }
    }

    // 更新坐标轴类型
    currentAxis->xType = x_type;
    currentAxis->yType = y_type;
    
    // 计算曲线点的实际坐标
    currentAxis->calculateCurvePoints(currentCurve);
    
    // 通知视图数据已更新
    emit QAbstractTableModel::dataChanged(index(0, 1), index(rowCount(QModelIndex()) - 1, 2));
}
