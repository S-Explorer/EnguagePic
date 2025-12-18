#include "MainScene.h"
#include "ImagePreviewer.h"
#include "MarkerTable.h"

#include <QFileDialog>
#include <QPushButton>
#include <QTableView>
#include <QBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>

MainScene::MainScene(QWidget* parent)
    :QWidget(parent){
    setGeometry(600, 300, 800, 500);

    QVBoxLayout* main_layout = new QVBoxLayout;
    QHBoxLayout* btn_Layout = new QHBoxLayout;
    QHBoxLayout* sec_layout = new QHBoxLayout;

    main_layout->addLayout(btn_Layout);
    main_layout->addLayout(sec_layout);

    pic_viewer = new ImagePreviewer;
    data_viewer = new QTableView;
    MarkerTable* m_table_model = new MarkerTable(pic_viewer);
    data_viewer->setMaximumWidth(400);
    data_viewer->setModel(m_table_model);
    data_viewer->verticalHeader()->hide();

    sec_layout->addWidget(pic_viewer);
    sec_layout->addWidget(data_viewer);

    btn_pic = new QPushButton("open pic");
    btn_axe = new QPushButton("axe");
    btn_point = new QPushButton("P");
    btn_clear_marker = new QPushButton("clear marker");
    btn_Layout->addWidget(btn_pic);
    btn_Layout->addWidget(btn_axe);
    btn_Layout->addWidget(btn_point);
    btn_Layout->addWidget(btn_clear_marker);

    setLayout(main_layout);

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

void MainScene::ExportData()
{
    QString data_path = QFileDialog::getSaveFileName(this,
                            "save data",
                            QString(),
                            "csv file(*.csv)");
}


