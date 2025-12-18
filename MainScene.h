#ifndef MAIN_SCENE_H
#define MAIN_SCENE_H

#include <QWidget>

// predeclare
class ImagePreviewer;
class QPushButton;

class MainScene : public QWidget{
    Q_OBJECT
public:
    MainScene(QWidget* parent = nullptr);
    ~MainScene();

private:
    void OpenTargetPic();


    QPushButton* btn_pic;
    QPushButton* btn_clear_marker;
    ImagePreviewer* pic_viewer;
};

#endif // !MAIN_SCENE_H

