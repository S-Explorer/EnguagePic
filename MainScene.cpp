#include "MainScene.h"
#include "ImagePreviewer.h"
#include "MarkerTable.h"
#include "CustomDelegate.h"
#include "ScreenCapture.h"
#include "Style.h"

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
#include <QDir>
#include <QDateTime>
#include <QTimer>

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

    btn_pic          = new QPushButton("打开图片");
    btn_screen       = new QPushButton("截屏");
    btn_axe          = new QPushButton("轴点");
    btn_point        = new QPushButton("线点");
    btn_clear_marker = new QPushButton("删除线点");
    btn_clear_axe    = new QPushButton("删除轴点");
    btn_clear_point  = new QPushButton("删除所有点");
    cbx_x_type       = new QComboBox();
    cbx_y_type       = new QComboBox();
    cbx_x_type->addItems({"线性","对数"});
    cbx_y_type->addItems({"线性","对数"});
    cbx_x_type->setToolTip("X轴类型");
    cbx_y_type->setToolTip("Y轴类型");
    btn_cal_data     = new QPushButton("计算坐标");
    btn_save         = new QPushButton("导出数据");
    
    // ========== 应用样式 ==========
    // 全局样式
    setStyleSheet(AppStyle::getGlobalStyle());
    
    // 主要按钮样式（功能按钮）
    btn_pic->setStyleSheet(AppStyle::getPrimaryButtonStyle());
    btn_screen->setStyleSheet(AppStyle::getPrimaryButtonStyle());
    btn_cal_data->setStyleSheet(AppStyle::getPrimaryButtonStyle());
    btn_save->setStyleSheet(AppStyle::getPrimaryButtonStyle());
    
    // 次要按钮样式（模式切换）
    btn_axe->setStyleSheet(AppStyle::getSecondaryButtonStyle());
    btn_point->setStyleSheet(AppStyle::getSecondaryButtonStyle());
    
    // 清除按钮使用危险样式（红色）
    btn_clear_marker->setStyleSheet(AppStyle::getDangerButtonStyle());
    btn_clear_axe->setStyleSheet(AppStyle::getDangerButtonStyle());
    btn_clear_point->setStyleSheet(AppStyle::getDangerButtonStyle());
    
    // 下拉框样式
    cbx_x_type->setStyleSheet(AppStyle::getComboBoxStyle());
    cbx_y_type->setStyleSheet(AppStyle::getComboBoxStyle());
    
    // 表格样式
    data_viewer->setStyleSheet(AppStyle::getTableViewStyle());
    // =============================
    
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

    // 创建截图工具
    m_screenCapture = new ScreenCapture(this);
    connect(m_screenCapture, &ScreenCapture::captured, this, [this](const QPixmap& pixmap) {
        // 生成临时文件名并保存
        QString tempPath = QDir::temp().filePath(QString("capture_%1.png")
            .arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss")));
        if (pixmap.save(tempPath)) {
            pic_viewer->LoadImage(tempPath);
        }
    });

    connect(btn_pic, &QPushButton::clicked, this, &MainScene::OpenTargetPic);
    connect(btn_screen, &QPushButton::clicked, this, &MainScene::CaptureScreen);
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
    delete m_screenCapture;
}

void MainScene::CaptureScreen() {
    // 先最小化主窗口，避免截到自己（保留任务栏图标）
    showMinimized();
    
    // 短暂延迟，确保窗口完全最小化
    QTimer::singleShot(500, this, [this]() {
        // 启动截图
        bool success = m_screenCapture->startCapture();
        
        // 截图完成后恢复主窗口
        showNormal();
        raise();
        activateWindow();
    });
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
