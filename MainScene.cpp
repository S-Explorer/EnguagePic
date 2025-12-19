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
    setGeometry(600, 300, 1000, 500);
    setMinimumWidth(1000);
    QVBoxLayout* main_layout = new QVBoxLayout;
    QHBoxLayout* btn_Layout = new QHBoxLayout;
    QHBoxLayout* sec_layout = new QHBoxLayout;

    main_layout->addLayout(btn_Layout);
    main_layout->addLayout(sec_layout);

    pic_viewer = new ImagePreviewer;
    data_viewer = new QTableView;
    MarkerTable* m_table_model = new MarkerTable(pic_viewer);
    ButtonDelegate* m_btn_del  = new ButtonDelegate(3, data_viewer);
    data_viewer->setMaximumWidth(450);
    data_viewer->setModel(m_table_model);
    data_viewer->setItemDelegateForColumn(3, m_btn_del);
    data_viewer->verticalHeader()->hide();
    data_viewer->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    sec_layout->addWidget(pic_viewer);
    sec_layout->addWidget(data_viewer);

    btn_pic          = new QPushButton("open pic");
    btn_screen       = new QPushButton("screen");
    btn_axe          = new QPushButton("axe");
    btn_point        = new QPushButton("P");
    btn_clear_marker = new QPushButton("clear marker");
    btn_clear_axe    = new QPushButton("clear axe");
    btn_clear_point  = new QPushButton("clear points");
    btn_Layout->addWidget(btn_pic);
    btn_Layout->addWidget(btn_screen);
    btn_Layout->addWidget(btn_axe);
    btn_Layout->addWidget(btn_point);
    btn_Layout->addWidget(btn_clear_marker);
    btn_Layout->addWidget(btn_clear_axe);
    btn_Layout->addWidget(btn_clear_point);
    setLayout(main_layout);

    connect(btn_pic, &QPushButton::clicked, this, &MainScene::OpenTargetPic);
    connect(btn_clear_marker, &QPushButton::clicked, pic_viewer, &ImagePreviewer::ResetMarker);
    connect(btn_axe, &QPushButton::clicked, this, &MainScene::SetAxeMode);
    connect(btn_point, &QPushButton::clicked, this, &MainScene::SetPointMode);
    connect(m_btn_del, &ButtonDelegate::Clicked, m_table_model, &MarkerTable::DeleteRow);
}

MainScene::~MainScene() {
    delete pic_viewer;
}

void MainScene::OpenTargetPic(){
    QString pic_path = QFileDialog::getOpenFileName( this,
            "open data pic",
            QString(),
            "image file(*.png *.jpg)");

    pic_viewer->LoadImage(pic_path);
}

void MainScene::ExportData()
{
    QString data_path = QFileDialog::getSaveFileName(this,
                            "save data",
                            QString(),
                            "csv file(*.csv)");
}

void MainScene::SetAxeMode(){
    pic_viewer->SetCurMode(DRAW_MODE::AXE);
}

void MainScene::SetPointMode(){
    pic_viewer->SetCurMode(DRAW_MODE::POINT);
}
