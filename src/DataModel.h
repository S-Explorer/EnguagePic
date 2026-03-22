#ifndef DATA_MODEL_H
#define DATA_MODEL_H

#include <QObject>
#include <QVector>
#include <QPointF>
#include <QString>
#include <QUuid>

// 曲线数据
class CurveData : public QObject {
    Q_OBJECT
public:
    explicit CurveData(const QString& name = "", QObject* parent = nullptr);
    
    QString getId() const { return id; }
    QString getName() const { return name; }
    void setName(const QString& newName) { name = newName; }
    
    // 数据点
    QVector<QPointF> pixelPoints;  // 像素坐标
    QVector<QPointF> realPoints;   // 实际坐标
    
    void addPoint(const QPointF& pixel, const QPointF& real);
    void removePoint(int index);
    void clear();
    int pointCount() const { return pixelPoints.size(); }
    
    // 可见性控制
    bool isVisible() const { return m_isVisible; }
    void setVisible(bool visible);
    
signals:
    void dataChanged();
    void pointAdded(int index);
    void pointRemoved(int index);
    void visibilityChanged(bool visible);

public:
    bool m_isVisible = true;

private:
    QString id;
    QString name;
};

// 坐标轴数据
class AxisData : public QObject {
    Q_OBJECT
public:
    explicit AxisData(const QString& name = "", QObject* parent = nullptr);
    ~AxisData();
    
    QString getId() const { return id; }
    QString getName() const { return name; }
    void setName(const QString& newName) { name = newName; }
    
    // 坐标轴类型：0=线性, 1=对数
    int xType = 0;
    int yType = 0;
    
    // 坐标轴参考点（3个点）
    QVector<QPointF> axePixelPoints;  // 像素坐标
    QVector<QPointF> axeRealPoints;   // 实际坐标
    
    // 该坐标轴下的曲线
    QVector<CurveData*> curves;
    
    CurveData* addCurve(const QString& name = "");
    void removeCurve(CurveData* curve);
    void removeCurve(int index);
    CurveData* getCurve(int index);
    int curveCount() const { return curves.size(); }
    
    void addAxePoint(const QPointF& pixel, const QPointF& real);
    void removeAxePoint(int index);
    void clearAxePoints();
    int axePointCount() const { return axePixelPoints.size(); }
    
    // 计算曲线点的实际坐标
    void calculateCurvePoints(CurveData* curve);
    void calculateAllCurves();
    
signals:
    void dataChanged();
    void curveAdded(CurveData* curve);
    void curveRemoved(CurveData* curve);
    void axePointsChanged();
    
private:
    QString id;
    QString name;
};

// 数据管理器
class DataManager : public QObject {
    Q_OBJECT
public:
    explicit DataManager(QObject* parent = nullptr);
    ~DataManager();
    
    // 坐标轴管理
    QVector<AxisData*> axes;
    AxisData* currentAxis = nullptr;
    CurveData* currentCurve = nullptr;
    
    AxisData* addAxis(const QString& name = "");
    void removeAxis(AxisData* axis);
    void removeAxis(int index);
    AxisData* getAxis(int index);
    int axisCount() const { return axes.size(); }
    
    void setCurrentAxis(AxisData* axis);
    void setCurrentCurve(CurveData* curve);
    
    // 清空所有数据
    void clearAll();
    
signals:
    void axisAdded(AxisData* axis);
    void axisRemoved(AxisData* axis);
    void currentAxisChanged(AxisData* axis);
    void currentCurveChanged(CurveData* curve);
    void dataCleared();
    
private:
    int axisCounter = 0;
    int curveCounter = 0;
};

#endif // DATA_MODEL_H
