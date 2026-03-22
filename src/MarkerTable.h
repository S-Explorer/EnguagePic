#ifndef MARKERTABLE_H
#define MARKERTABLE_H

#define LIN_MODE 0
#define LOG_MODE 1
#define AIX_MODE 2

#include <QAbstractTableModel>

class DataManager;
class AxisData;
class CurveData;

class MarkerTable : public QAbstractTableModel{
    Q_OBJECT
public:
    MarkerTable(DataManager* manager, QObject* parent = nullptr);
    
    // 设置当前显示的曲线
    void setCurrentCurve(CurveData* curve);
    void setCurrentAxis(AxisData* axis);
    
    // 删除指定行
    void DeleteRow(int row);
    
    // 计算当前曲线的实际坐标
    void CalRelData(int x_type, int y_type);
    
    // 模型接口
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    int rowCount(const QModelIndex & /* parent */) const override;
    int columnCount(const QModelIndex & /* parent */) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

signals:
    void RowDeleted(int row);

private slots:
    void onCurrentCurveChanged(CurveData* curve);
    void onCurrentAxisChanged(AxisData* axis);
    void onCurveDataChanged();

private:
    DataManager* dataManager;
    AxisData* currentAxis = nullptr;
    CurveData* currentCurve = nullptr;
    
    // 获取总行数（3个坐标轴点 + 曲线点）
    int getTotalRowCount() const;
    
    // 判断某行是否为坐标轴点
    bool isAxeRow(int row) const { return row < 3; }
    
    // 获取坐标轴点索引
    int getAxePointIndex(int row) const { return row; }
    
    // 获取曲线点索引
    int getCurvePointIndex(int row) const { return row - 3; }
};

#endif // MARKERTABLE_H
