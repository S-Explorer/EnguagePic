#include "MainScene.h"
#include "ImagePreviewer.h"
#include <QFileDialog>
#include <QPushButton>

#include <QBoxLayout>
#include <QHBoxLayout>

MainScene::MainScene(QWidget* parent)
    :QWidget(parent){
    setGeometry(600, 300, 800, 500);
   
    pic_viewer = new ImagePreviewer(); 
    btn_pic = new QPushButton("open pic");

    QHBoxLayout* main_layout = new QHBoxLayout;
    QWidget* container_widget = new QWidget;
    container_widget->setFixedWidth(150);
    
    main_layout->addWidget(pic_viewer);
    main_layout->addWidget(container_widget);
    setLayout(main_layout);

    QVBoxLayout* util_layout = new QVBoxLayout;
    util_layout->addWidget(btn_pic);
    container_widget->setLayout(util_layout);

    connect(btn_pic, &QPushButton::clicked, this, &MainScene::OpenTargetPic);
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


