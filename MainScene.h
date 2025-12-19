#ifndef MAIN_SCENE_H
#define MAIN_SCENE_H

#include <QWidget>

// predeclare
class ImagePreviewer;
class QPushButton;
class QTableView;

class MainScene : public QWidget{
    Q_OBJECT
public:
    MainScene(QWidget* parent = nullptr);
    ~MainScene();

private:
    void OpenTargetPic();
    void ExportData();

    void SetAxeMode();
    void SetPointMode();

    QPushButton* btn_pic;
    QPushButton* btn_screen;
    QPushButton* btn_axe;
    QPushButton* btn_point;
    QPushButton* btn_clear_marker;
    QPushButton* btn_clear_axe;
    QPushButton* btn_clear_point;

    QTableView* data_viewer;

    ImagePreviewer* pic_viewer;
};

#endif // !MAIN_SCENE_H

