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
#include <QFile>
#include <QTextStream>
#include <QMessageBox>

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
    cbx_x_type       = new QComboBox();
    cbx_y_type       = new QComboBox();
    cbx_x_type->addItems({"linear","log"});
    cbx_y_type->addItems({"linear","log"});
    cbx_x_type->setToolTip("X轴类型");
    cbx_y_type->setToolTip("Y轴类型");
    btn_cal_data     = new QPushButton("calculate");
    btn_save         = new QPushButton("export data");
    btn_Layout->addWidget(btn_pic);
    btn_Layout->addWidget(btn_screen);
    btn_Layout->addWidget(btn_axe);
    btn_Layout->addWidget(btn_point);
    btn_Layout->addWidget(btn_clear_marker);
    btn_Layout->addWidget(btn_clear_axe);
    btn_Layout->addWidget(btn_clear_point);
    btn_Layout->addWidget(cbx_x_type);
    btn_Layout->addWidget(cbx_y_type);
    btn_Layout->addWidget(btn_cal_data);
    btn_Layout->addWidget(btn_save);
    setLayout(main_layout);

    connect(btn_pic, &QPushButton::clicked, this, &MainScene::OpenTargetPic);
    connect(btn_clear_marker, &QPushButton::clicked, pic_viewer, &ImagePreviewer::ResetMarker);
    connect(btn_axe, &QPushButton::clicked, this, &MainScene::SetAxeMode);
    connect(btn_point, &QPushButton::clicked, this, &MainScene::SetPointMode);
    connect(m_btn_del, &ButtonDelegate::Clicked, m_table_model, &MarkerTable::DeleteRow);
    connect(btn_cal_data, &QPushButton::clicked, [=](){
        int x_type = cbx_x_type->currentIndex();
        int y_type = cbx_y_type->currentIndex();
        m_table_model->CalRelData(x_type, y_type);
    });
    connect(btn_save, &QPushButton::clicked, this, &MainScene::ExportData);
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
    
    if (data_path.isEmpty()) {
        return;
    }
    
    // 确保文件后缀为 .csv
    if (!data_path.endsWith(".csv", Qt::CaseInsensitive)) {
        data_path += ".csv";
    }
    
    QFile file(data_path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Error", "Failed to open file for writing.");
        return;
    }
    
    QTextStream stream(&file);
    
    // 获取模型
    QAbstractItemModel* model = data_viewer->model();
    if (!model) {
        QMessageBox::warning(this, "Error", "No data model available.");
        file.close();
        return;
    }
    
    // 写入表头
    stream << "name,x,y\n";
    
    // 写入数据（前3列：name, x, y）
    int rowCount = model->rowCount();
    for (int row = 0; row < rowCount; ++row) {
        QStringList rowData;
        for (int col = 0; col < 3; ++col) {
            QModelIndex index = model->index(row, col);
            QVariant value = model->data(index, Qt::DisplayRole);
            rowData << value.toString();
        }
        stream << rowData.join(",") << "\n";
    }
    
    file.close();
    
    QMessageBox::information(this, "Success", 
        QString("Data exported to:\n%1").arg(data_path));
}

void MainScene::SetAxeMode(){
    pic_viewer->SetCurMode(DRAW_MODE::AXE);
}

void MainScene::SetPointMode(){
    pic_viewer->SetCurMode(DRAW_MODE::POINT);
}
