#include "MainScene.h"
#include "ImagePreviewer.h"
#include <QFileDialog>
#include <QPushButton>

#include <QBoxLayout>
#include <QHBoxLayout>

MainScene::MainScene(QWidget* parent)
    :QWidget(parent){
    setGeometry(600, 300, 800, 500);


    QHBoxLayout* main_layout = new QHBoxLayout;
    QWidget* container_widget = new QWidget;
    pic_viewer = new ImagePreviewer;

    container_widget->setFixedWidth(150);
    main_layout->addWidget(pic_viewer);
    main_layout->addWidget(container_widget);
    setLayout(main_layout);

    QVBoxLayout* util_layout = new QVBoxLayout;

    btn_pic = new QPushButton("open pic");
    btn_clear_marker = new QPushButton("clear marker");
    util_layout->addWidget(btn_pic);
    util_layout->addWidget(btn_clear_marker);

    container_widget->setLayout(util_layout);

    connect(btn_pic, &QPushButton::clicked, this, &MainScene::OpenTargetPic);
    connect(btn_clear_marker, &QPushButton::clicked, pic_viewer, &ImagePreviewer::ResetMarker);
}

MainScene::~MainScene() {
    delete pic_viewer;
}

void MainScene::OpenTargetPic(){
    QString pic_path = QFileDialog::getOpenFileName( this,
            "open data pic",
            QString(),
            "image file(*.png, *.jpg)");

    pic_viewer->LoadImage(pic_path);
}


