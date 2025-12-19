#include "MainScene.h"
#include "ImagePreviewer.h"
#include "MarkerTable.h"
#include "CustomDelegate.h"

#include <QFileDialog>
#include <QPushButton>
#include <QTableView>
#include <QBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QComboBox>

MainScene::MainScene(QWidget* parent)
    :QWidget(parent){
    setGeometry(600, 300, 1000, 500);
    setMinimumWidth(1000);
    QVBoxLayout* main_layout = new QVBoxLayout;
    QHBoxLayout* btn_Layout  = new QHBoxLayout;
    QHBoxLayout* sec_layout  = new QHBoxLayout;

    main_layout->addLayout(btn_Layout);
    main_layout->addLayout(sec_layout);

    pic_viewer = new ImagePreviewer;
    data_viewer = new QTableView;
    MarkerTable* m_table_model = new MarkerTable(pic_viewer);
    ButtonDelegate* m_btn_del  = new ButtonDelegate(3, data_viewer);
    EditDelegate* x_delegate   = new EditDelegate(data_viewer);
    EditDelegate* y_delegate   = new EditDelegate(data_viewer);
    data_viewer->setMaximumWidth(450);
    data_viewer->setModel(m_table_model);
    data_viewer->setItemDelegateForColumn(3, m_btn_del);
    data_viewer->setItemDelegateForColumn(1, x_delegate);
    data_viewer->setItemDelegateForColumn(2, y_delegate);
    data_viewer->verticalHeader()->hide();
    data_viewer->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    data_viewer->setEditTriggers(QAbstractItemView::DoubleClicked);
    data_viewer->setMouseTracking(true);

    sec_layout->addWidget(pic_viewer);
    sec_layout->addWidget(data_viewer);

    btn_pic          = new QPushButton("open pic");
    btn_screen       = new QPushButton("screen");
    btn_axe          = new QPushButton("axe");
    btn_point        = new QPushButton("P");
    btn_clear_marker = new QPushButton("clear marker");
    btn_clear_axe    = new QPushButton("clear axe");
    btn_clear_point  = new QPushButton("clear points");
    cbx_type         = new QComboBox();
    cbx_type->addItems({"linear","logarithmic","axial"});
    btn_cal_data     = new QPushButton("calculate");
    btn_save         = new QPushButton("export data");
    btn_Layout->addWidget(btn_pic);
    btn_Layout->addWidget(btn_screen);
    btn_Layout->addWidget(btn_axe);
    btn_Layout->addWidget(btn_point);
    btn_Layout->addWidget(btn_clear_marker);
    btn_Layout->addWidget(btn_clear_axe);
    btn_Layout->addWidget(btn_clear_point);
    btn_Layout->addWidget(cbx_type);
    btn_Layout->addWidget(btn_cal_data);
    btn_Layout->addWidget(btn_save);
    setLayout(main_layout);

    connect(btn_pic, &QPushButton::clicked, this, &MainScene::OpenTargetPic);
    connect(btn_clear_marker, &QPushButton::clicked, pic_viewer, &ImagePreviewer::ResetMarker);
    connect(btn_axe, &QPushButton::clicked, this, &MainScene::SetAxeMode);
    connect(btn_point, &QPushButton::clicked, this, &MainScene::SetPointMode);
    connect(m_btn_del, &ButtonDelegate::Clicked, m_table_model, &MarkerTable::DeleteRow);
    connect(btn_cal_data, &QPushButton::clicked, [=](){m_table_model->CalRelData(0);});
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
