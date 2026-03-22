#ifndef AXIS_CURVE_LIST_H
#define AXIS_CURVE_LIST_H

#include <QWidget>
#include <QTreeWidget>
#include <QPushButton>
#include <QVBoxLayout>

class DataManager;
class AxisData;
class CurveData;

// 树形控件，显示坐标轴和曲线的层次结构
class AxisCurveTreeWidget : public QTreeWidget {
    Q_OBJECT
public:
    explicit AxisCurveTreeWidget(QWidget* parent = nullptr);
    
    void setDataManager(DataManager* manager);
    void refresh();
    
    // 获取选中的项类型
    bool isAxisSelected() const;
    bool isCurveSelected() const;
    AxisData* getSelectedAxis() const;
    CurveData* getSelectedCurve() const;
    
    // 选中特定项
    void selectAxis(AxisData* axis);
    void selectCurve(CurveData* curve);
    
signals:
    void axisSelected(AxisData* axis);
    void curveSelected(CurveData* curve);
    void deleteRequested();
    void curveVisibilityChanged(CurveData* curve, bool visible);
    
private slots:
    void onItemClicked(QTreeWidgetItem* item, int column);
    void onItemChanged(QTreeWidgetItem* item, int column);
    void onContextMenu(const QPoint& pos);
    
private:
    DataManager* dataManager = nullptr;
    
    // 自定义角色，用于存储指针
    static const int AxisRole = Qt::UserRole + 1;
    static const int CurveRole = Qt::UserRole + 2;
    
    // 记录是否正在刷新，避免信号循环
    bool isRefreshing = false;
};

// 左侧控制面板
class LeftPanel : public QWidget {
    Q_OBJECT
public:
    explicit LeftPanel(QWidget* parent = nullptr);
    
    void setDataManager(DataManager* manager);
    AxisCurveTreeWidget* getTreeWidget() const { return treeWidget; }
    
signals:
    void addAxisRequested();
    void addCurveRequested();
    void deleteRequested();
    void axisSelected(AxisData* axis);
    void curveSelected(CurveData* curve);
    void curveVisibilityChanged(CurveData* curve, bool visible);
    
private:
    void setupUI();
    
    AxisCurveTreeWidget* treeWidget;
    QPushButton* btnAddAxis;
    QPushButton* btnAddCurve;
    QPushButton* btnDelete;
    
    DataManager* dataManager = nullptr;
};

#endif // AXIS_CURVE_LIST_H
