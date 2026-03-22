#ifndef MAIN_SCENE_H
#define MAIN_SCENE_H

#include <QWidget>

// predeclare
class ImagePreviewer;
class ScreenCapture;
class LeftPanel;
class MarkerTable;
class DataManager;
class QPushButton;
class QTableView;
class QComboBox;
class AxisData;
class CurveData;

class MainScene : public QWidget{
    Q_OBJECT
public:
    MainScene(QWidget* parent = nullptr);
    ~MainScene();

private slots:
    void OpenTargetPic();
    void ExportData();
    void CaptureScreen();

    void SetAxeMode();
    void SetPointMode();
    void restoreWindowState();
    
    // 左侧面板操作
    void onAddAxis();
    void onAddCurve();
    void onDeleteItem();
    void onAxisSelected(AxisData* axis);
    void onCurveSelected(CurveData* curve);
    
    // 数据变化处理
    void onAxisPointAdded(AxisData* axis, const QPointF& pos);
    void onCurvePointAdded(CurveData* curve, const QPointF& pos);

private:
    void setupUI();
    void connectSignals();
    
    // 按钮
    QPushButton* btn_pic;
    QPushButton* btn_screen;
    QPushButton* btn_axe;
    QPushButton* btn_point;
    QPushButton* btn_clear_marker;
    QPushButton* btn_clear_axe;
    QPushButton* btn_clear_point;
    QPushButton* btn_cal_data;
    QPushButton* btn_save;

    QComboBox* cbx_x_type;
    QComboBox* cbx_y_type;

    QTableView* data_viewer;

    ImagePreviewer* pic_viewer;
    ScreenCapture* m_screenCapture;
    LeftPanel* leftPanel;
    
    // 数据管理
    DataManager* dataManager;
    MarkerTable* m_table_model;
    
    // 委托
    class ButtonDelegate* btn_delegate;
    
    // 截图前保存的窗口状态
    struct WindowState {
        Qt::WindowStates windowState;
        QRect geometry;
        bool isMaximized;
        bool isMinimized;
    };
    WindowState m_savedWindowState;
    bool m_isCapturing = false;
};

#endif // !MAIN_SCENE_H
