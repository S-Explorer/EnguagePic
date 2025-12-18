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

    QPushButton* btn_pic;
    QPushButton* btn_axe;
    QPushButton* btn_point;
    QPushButton* btn_clear_marker;

    QTableView* data_viewer;

    ImagePreviewer* pic_viewer;
};

#endif // !MAIN_SCENE_H

