#include "DataModel.h"
#include <QDebug>
#include <QtMath>

/* * * * * * * * * * * * * * *
 *       CurveData           *
 * * * * * * * * * * * * * * */

CurveData::CurveData(const QString& name, QObject* parent)
    : QObject(parent), id(QUuid::createUuid().toString()), name(name) {
    if (this->name.isEmpty()) {
        // 从父对象获取计数器来命名
        if (parent) {
            int count = 0;
            for (auto* child : parent->findChildren<CurveData*>()) {
                if (child != this) count++;
            }
            this->name = QString("曲线%1").arg(count + 1);
        } else {
            this->name = "曲线1";
        }
    }
}

void CurveData::addPoint(const QPointF& pixel, const QPointF& real) {
    pixelPoints.append(pixel);
    realPoints.append(real);
    emit pointAdded(pixelPoints.size() - 1);
    emit dataChanged();
}

void CurveData::removePoint(int index) {
    if (index >= 0 && index < pixelPoints.size()) {
        pixelPoints.removeAt(index);
        realPoints.removeAt(index);
        emit pointRemoved(index);
        emit dataChanged();
    }
}

void CurveData::clear() {
    pixelPoints.clear();
    realPoints.clear();
    emit dataChanged();
}

void CurveData::setVisible(bool visible) {
    if (m_isVisible != visible) {
        m_isVisible = visible;
        emit visibilityChanged(visible);
        emit dataChanged();
    }
}

/* * * * * * * * * * * * * * *
 *       AxisData            *
 * * * * * * * * * * * * * * */

AxisData::AxisData(const QString& name, QObject* parent)
    : QObject(parent), id(QUuid::createUuid().toString()), name(name) {
    if (this->name.isEmpty()) {
        // 从父对象获取计数器来命名
        if (parent) {
            int count = 0;
            for (auto* child : parent->findChildren<AxisData*>()) {
                if (child != this) count++;
            }
            this->name = QString("坐标轴%1").arg(count + 1);
        } else {
            this->name = "坐标轴1";
        }
    }
    
    // 初始化参考点数组大小为3
    axePixelPoints.reserve(3);
    axeRealPoints.reserve(3);
}

AxisData::~AxisData() {
    // 删除所有曲线
    for (auto* curve : curves) {
        delete curve;
    }
    curves.clear();
}

CurveData* AxisData::addCurve(const QString& name) {
    CurveData* curve = new CurveData(name, this);
    curves.append(curve);
    emit curveAdded(curve);
    emit dataChanged();
    return curve;
}

void AxisData::removeCurve(CurveData* curve) {
    int index = curves.indexOf(curve);
    if (index >= 0) {
        removeCurve(index);
    }
}

void AxisData::removeCurve(int index) {
    if (index >= 0 && index < curves.size()) {
        CurveData* curve = curves.takeAt(index);
        emit curveRemoved(curve);
        delete curve;
        emit dataChanged();
    }
}

CurveData* AxisData::getCurve(int index) {
    if (index >= 0 && index < curves.size()) {
        return curves[index];
    }
    return nullptr;
}

void AxisData::addAxePoint(const QPointF& pixel, const QPointF& real) {
    if (axePixelPoints.size() < 3) {
        axePixelPoints.append(pixel);
        axeRealPoints.append(real);
        emit axePointsChanged();
        emit dataChanged();
    }
}

void AxisData::removeAxePoint(int index) {
    if (index >= 0 && index < axePixelPoints.size()) {
        axePixelPoints.removeAt(index);
        axeRealPoints.removeAt(index);
        emit axePointsChanged();
        emit dataChanged();
    }
}

void AxisData::clearAxePoints() {
    axePixelPoints.clear();
    axeRealPoints.clear();
    emit axePointsChanged();
    emit dataChanged();
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

void AxisData::calculateCurvePoints(CurveData* curve) {
    if (!curve || axePixelPoints.size() < 2 || curve->pixelPoints.isEmpty()) {
        return;
    }
    
    curve->realPoints.resize(curve->pixelPoints.size());
    
    // 找出X方向的极值点
    int x_min_idx = 0, x_max_idx = 0;
    for (int i = 1; i < axePixelPoints.size(); i++) {
        if (axePixelPoints[i].x() < axePixelPoints[x_min_idx].x()) x_min_idx = i;
        if (axePixelPoints[i].x() > axePixelPoints[x_max_idx].x()) x_max_idx = i;
    }
    
    // 找出Y方向的极值点
    int y_min_idx = 0, y_max_idx = 0;
    for (int i = 1; i < axePixelPoints.size(); i++) {
        if (axePixelPoints[i].y() > axePixelPoints[y_min_idx].y()) y_min_idx = i;  // Y轴向下，所以大的y值实际上是小的
        if (axePixelPoints[i].y() < axePixelPoints[y_max_idx].y()) y_max_idx = i;
    }
    
    // X轴映射参数
    double px_min = axePixelPoints[x_min_idx].x();
    double px_max = axePixelPoints[x_max_idx].x();
    double vx_min = axeRealPoints[x_min_idx].x();
    double vx_max = axeRealPoints[x_max_idx].x();
    
    // Y轴映射参数（注意Y坐标翻转）
    double py_min = axePixelPoints[y_min_idx].y();
    double py_max = axePixelPoints[y_max_idx].y();
    double vy_min = axeRealPoints[y_min_idx].y();
    double vy_max = axeRealPoints[y_max_idx].y();
    
    for (int i = 0; i < curve->pixelPoints.size(); i++) {
        double px = curve->pixelPoints[i].x();
        double py = curve->pixelPoints[i].y();
        
        double x_value = MapAxisValue(px, px_min, px_max, vx_min, vx_max, xType == 1);
        double y_value = MapAxisValue(py, py_min, py_max, vy_min, vy_max, yType == 1);
        
        curve->realPoints[i] = QPointF(x_value, y_value);
    }
    
    emit curve->dataChanged();
}

void AxisData::calculateAllCurves() {
    for (auto* curve : curves) {
        calculateCurvePoints(curve);
    }
}

/* * * * * * * * * * * * * * *
 *       DataManager         *
 * * * * * * * * * * * * * * */

DataManager::DataManager(QObject* parent) : QObject(parent) {
}

DataManager::~DataManager() {
    clearAll();
}

AxisData* DataManager::addAxis(const QString& name) {
    AxisData* axis = new AxisData(name, this);
    axes.append(axis);
    
    // 如果是第一个坐标轴，设为当前选中
    if (axes.size() == 1) {
        setCurrentAxis(axis);
    }
    
    emit axisAdded(axis);
    return axis;
}

void DataManager::removeAxis(AxisData* axis) {
    int index = axes.indexOf(axis);
    if (index >= 0) {
        removeAxis(index);
    }
}

void DataManager::removeAxis(int index) {
    if (index >= 0 && index < axes.size()) {
        AxisData* axis = axes.takeAt(index);
        
        // 如果删除的是当前选中的坐标轴，需要切换
        if (currentAxis == axis) {
            if (axes.isEmpty()) {
                setCurrentAxis(nullptr);
            } else {
                setCurrentAxis(axes[qMin(index, axes.size() - 1)]);
            }
        }
        
        emit axisRemoved(axis);
        delete axis;
    }
}

AxisData* DataManager::getAxis(int index) {
    if (index >= 0 && index < axes.size()) {
        return axes[index];
    }
    return nullptr;
}

void DataManager::setCurrentAxis(AxisData* axis) {
    if (currentAxis != axis) {
        currentAxis = axis;
        
        // 设置当前曲线为该坐标轴的第一条曲线（如果有）
        if (axis && axis->curveCount() > 0) {
            setCurrentCurve(axis->getCurve(0));
        } else {
            setCurrentCurve(nullptr);
        }
        
        emit currentAxisChanged(axis);
    }
}

void DataManager::setCurrentCurve(CurveData* curve) {
    if (currentCurve != curve) {
        currentCurve = curve;
        emit currentCurveChanged(curve);
    }
}

void DataManager::clearAll() {
    for (auto* axis : axes) {
        delete axis;
    }
    axes.clear();
    currentAxis = nullptr;
    currentCurve = nullptr;
    emit dataCleared();
}
