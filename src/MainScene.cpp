#include "MainScene.h"
#include "ImagePreviewer.h"
#include "MarkerTable.h"
#include "CustomDelegate.h"
#include "ScreenCapture.h"
#include "Style.h"
#include "AxisCurveList.h"
#include "DataModel.h"

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
#include <QIcon>
#include <QSplitter>

MainScene::MainScene(QWidget* parent)
    : QWidget(parent), dataManager(nullptr) {
    setGeometry(600, 300, 1200, 600);
    setMinimumWidth(1200);
    setMinimumHeight(600);
    
    // 设置窗口图标（Linux 需要）
    #ifdef Q_OS_LINUX
        setWindowIcon(QIcon(":/enguage.ico"));
    #endif
    
    // 创建数据管理器
    dataManager = new DataManager(this);
    
    setupUI();
    connectSignals();
    
    // 默认创建一个坐标轴
    if (dataManager->axisCount() == 0) {
        dataManager->addAxis();
    }
}

MainScene::~MainScene() {
    delete m_screenCapture;
}

void MainScene::setupUI() {
    QVBoxLayout* main_layout = new QVBoxLayout;
    main_layout->setContentsMargins(10, 10, 10, 10);
    main_layout->setSpacing(10);
    
    // 顶部按钮布局
    QHBoxLayout* btn_Layout = new QHBoxLayout;
    
    btn_pic          = new QPushButton("打开图片");
    btn_screen       = new QPushButton("截屏");
    btn_axe          = new QPushButton("轴点");
    btn_point        = new QPushButton("线点");
    
    // 设置按钮为可勾选状态，实现互斥
    btn_axe->setCheckable(true);
    btn_point->setCheckable(true);
    btn_axe->setChecked(true);  // 默认选中轴点模式
    
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
    setStyleSheet(AppStyle::getGlobalStyle());
    btn_pic->setStyleSheet(AppStyle::getPrimaryButtonStyle());
    btn_screen->setStyleSheet(AppStyle::getPrimaryButtonStyle());
    btn_cal_data->setStyleSheet(AppStyle::getPrimaryButtonStyle());
    btn_save->setStyleSheet(AppStyle::getPrimaryButtonStyle());
    btn_axe->setStyleSheet(AppStyle::getSecondaryButtonStyle());
    btn_point->setStyleSheet(AppStyle::getSecondaryButtonStyle());
    btn_clear_marker->setStyleSheet(AppStyle::getDangerButtonStyle());
    btn_clear_axe->setStyleSheet(AppStyle::getDangerButtonStyle());
    btn_clear_point->setStyleSheet(AppStyle::getDangerButtonStyle());
    cbx_x_type->setStyleSheet(AppStyle::getComboBoxStyle());
    cbx_y_type->setStyleSheet(AppStyle::getComboBoxStyle());
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
    btn_Layout->addStretch();
    
    main_layout->addLayout(btn_Layout);
    
    // 中间内容区域（使用 QSplitter 实现可调整大小）
    QSplitter* splitter = new QSplitter(Qt::Horizontal);
    
    // 左侧面板
    leftPanel = new LeftPanel(this);
    leftPanel->setDataManager(dataManager);
    splitter->addWidget(leftPanel);
    
    // 图片预览区
    pic_viewer = new ImagePreviewer;
    pic_viewer->setDataManager(dataManager);
    splitter->addWidget(pic_viewer);
    splitter->setStretchFactor(1, 1);  // 图片区域占据更多空间
    
    // 数据表格
    data_viewer = new QTableView;
    m_table_model = new MarkerTable(dataManager, this);
    data_viewer->setModel(m_table_model);
    
    btn_delegate = new ButtonDelegate(3, data_viewer);
    EditDelegate* x_delegate = new EditDelegate(data_viewer);
    EditDelegate* y_delegate = new EditDelegate(data_viewer);
    
    data_viewer->setMaximumWidth(450);
    data_viewer->setItemDelegateForColumn(3, btn_delegate);
    data_viewer->setItemDelegateForColumn(1, x_delegate);
    data_viewer->setItemDelegateForColumn(2, y_delegate);
    data_viewer->verticalHeader()->hide();
    data_viewer->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    data_viewer->setEditTriggers(QAbstractItemView::DoubleClicked);
    data_viewer->setMouseTracking(true);
    data_viewer->setStyleSheet(AppStyle::getTableViewStyle());
    
    splitter->addWidget(data_viewer);
    
    // 设置分割器的初始大小
    splitter->setSizes({200, 600, 350});
    
    main_layout->addWidget(splitter);
    setLayout(main_layout);
    
    // 创建截图工具
    m_screenCapture = new ScreenCapture(this);
}

void MainScene::connectSignals() {
    // 截图工具 - 截图完成后恢复窗口
    connect(m_screenCapture, &ScreenCapture::captured, this, [this](const QPixmap& pixmap) {
        // 恢复窗口到之前保存的状态
        restoreWindowState();
        
        QString tempPath = QDir::temp().filePath(QString("capture_%1.png")
            .arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss")));
        if (pixmap.save(tempPath)) {
            pic_viewer->LoadImage(tempPath);
        }
    });
    
    // 截图取消后也恢复窗口
    connect(m_screenCapture, &ScreenCapture::cancelled, this, [this]() {
        restoreWindowState();
    });

    // 按钮连接
    connect(btn_pic, &QPushButton::clicked, this, &MainScene::OpenTargetPic);
    connect(btn_screen, &QPushButton::clicked, this, &MainScene::CaptureScreen);
    connect(btn_axe, &QPushButton::clicked, this, &MainScene::SetAxeMode);
    connect(btn_point, &QPushButton::clicked, this, &MainScene::SetPointMode);
    
    // 清除按钮
    connect(btn_clear_marker, &QPushButton::clicked, this, [this]() {
        if (dataManager->currentCurve) {
            dataManager->currentCurve->clear();
            pic_viewer->refreshDisplay();
        }
    });
    
    connect(btn_clear_axe, &QPushButton::clicked, this, [this]() {
        if (dataManager->currentAxis) {
            dataManager->currentAxis->clearAxePoints();
            pic_viewer->refreshDisplay();
        }
    });
    
    connect(btn_clear_point, &QPushButton::clicked, this, [this]() {
        // 清除所有数据
        dataManager->clearAll();
        pic_viewer->refreshDisplay();
        
        // 重新创建一个默认坐标轴
        if (dataManager->axisCount() == 0) {
            dataManager->addAxis();
        }
    });
    
    // 计算和导出
    connect(btn_cal_data, &QPushButton::clicked, [this](){
        int x_type = cbx_x_type->currentIndex();
        int y_type = cbx_y_type->currentIndex();
        m_table_model->CalRelData(x_type, y_type);
    });
    
    connect(btn_save, &QPushButton::clicked, this, &MainScene::ExportData);
    
    // 左侧面板信号
    connect(leftPanel, &LeftPanel::addAxisRequested, this, &MainScene::onAddAxis);
    connect(leftPanel, &LeftPanel::addCurveRequested, this, &MainScene::onAddCurve);
    connect(leftPanel, &LeftPanel::deleteRequested, this, &MainScene::onDeleteItem);
    connect(leftPanel, &LeftPanel::axisSelected, this, &MainScene::onAxisSelected);
    connect(leftPanel, &LeftPanel::curveSelected, this, &MainScene::onCurveSelected);
    connect(leftPanel, &LeftPanel::curveVisibilityChanged, this, [this](CurveData* curve, bool visible) {
        Q_UNUSED(curve)
        Q_UNUSED(visible)
        // 立即刷新图片显示
        pic_viewer->refreshDisplay();
    });
    
    // 图片预览器信号
    connect(pic_viewer, &ImagePreviewer::axisPointAdded, this, &MainScene::onAxisPointAdded);
    connect(pic_viewer, &ImagePreviewer::curvePointAdded, this, &MainScene::onCurvePointAdded);
    
    // 表格删除按钮
    connect(btn_delegate, &ButtonDelegate::Clicked, 
            m_table_model, &MarkerTable::DeleteRow);
    
    // 表格行删除后刷新图片显示
    connect(m_table_model, &MarkerTable::RowDeleted, this, [this](int row) {
        Q_UNUSED(row)
        // 删除后刷新图片预览区
        pic_viewer->refreshDisplay();
    });
}

void MainScene::CaptureScreen() {
    // 保存当前窗口完整状态
    m_savedWindowState.windowState = windowState();
    m_savedWindowState.geometry = geometry();
    m_savedWindowState.isMaximized = isMaximized();
    m_savedWindowState.isMinimized = isMinimized();
    m_isCapturing = true;
    
    // 最小化窗口以便截图
    showMinimized();
    
    // 延迟启动截图工具，确保窗口已最小化
    QTimer::singleShot(500, this, [this]() {
        // 启动截图，不在这里恢复窗口，而是在截图完成信号中恢复
        bool success = m_screenCapture->startCapture();
        
        // 如果截图启动失败，立即恢复窗口
        if (!success) {
            restoreWindowState();
        }
    });
}

void MainScene::restoreWindowState() {
    if (!m_isCapturing) return;
    m_isCapturing = false;
    
    // 先恢复几何位置和大小
    setGeometry(m_savedWindowState.geometry);
    
    // 然后恢复窗口状态（最大化/最小化/正常）
    if (m_savedWindowState.isMaximized) {
        showMaximized();
    } else if (m_savedWindowState.isMinimized) {
        showMinimized();
    } else {
        showNormal();
    }
    
    // 确保窗口在最前面并激活
    raise();
    activateWindow();
}

void MainScene::OpenTargetPic(){
    QString pic_path = QFileDialog::getOpenFileName(this,
            "open data pic",
            QString(),
            "image file(*.png *.jpg)");

    if (!pic_path.isEmpty()) {
        pic_viewer->LoadImage(pic_path);
    }
}

void MainScene::ExportData() {
    // 只导出当前坐标轴的数据
    if (!dataManager->currentAxis) {
        QMessageBox::warning(this, "Warning", "请先选择一个坐标轴");
        return;
    }
    
    QString data_path = QFileDialog::getSaveFileName(this,
                            "save data",
                            QString(),
                            "csv file(*.csv)");
    
    if (data_path.isEmpty()) {
        return;
    }
    
    if (!data_path.endsWith(".csv", Qt::CaseInsensitive)) {
        data_path += ".csv";
    }
    
    QFile file(data_path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Error", "Failed to open file for writing.");
        return;
    }
    
    QTextStream stream(&file);
    
    // 写入表头
    stream << "axis_name,curve_name,x,y\n";
    
    // 只遍历当前坐标轴
    AxisData* axis = dataManager->currentAxis;
    
    for (int j = 0; j < axis->curveCount(); j++) {
        CurveData* curve = axis->getCurve(j);
        if (!curve) continue;
        
        // 只导出被勾选（可见）的曲线
        if (!curve->isVisible()) continue;
        
        for (int k = 0; k < curve->pointCount(); k++) {
            stream << axis->getName() << ","
                 << curve->getName() << ","
                 << curve->realPoints[k].x() << ","
                 << curve->realPoints[k].y() << "\n";
        }
    }
    
    file.close();
    
    QMessageBox::information(this, "Success", 
        QString("Data exported to:\n%1").arg(data_path));
}

void MainScene::SetAxeMode(){
    // 设置轴点模式
    btn_axe->setChecked(true);
    btn_point->setChecked(false);
    pic_viewer->SetCurMode(DRAW_MODE::AXE);
}

void MainScene::SetPointMode(){
    // 设置线点模式
    btn_axe->setChecked(false);
    btn_point->setChecked(true);
    pic_viewer->SetCurMode(DRAW_MODE::POINT);
}

void MainScene::onAddAxis() {
    AxisData* axis = dataManager->addAxis();
    if (axis) {
        dataManager->setCurrentAxis(axis);
        // 自动为新坐标轴添加一条曲线
        axis->addCurve();
    }
}

void MainScene::onAddCurve() {
    if (!dataManager->currentAxis) {
        QMessageBox::warning(this, "Warning", "请先选择一个坐标轴");
        return;
    }
    
    CurveData* curve = dataManager->currentAxis->addCurve();
    if (curve) {
        dataManager->setCurrentCurve(curve);
    }
}

void MainScene::onDeleteItem() {
    // 获取左侧面板树中选中的项
    AxisCurveTreeWidget* tree = leftPanel->getTreeWidget();
    
    if (tree->isCurveSelected()) {
        // 删除曲线
        CurveData* curve = tree->getSelectedCurve();
        if (curve && dataManager->currentAxis) {
            dataManager->currentAxis->removeCurve(curve);
        }
    } else if (tree->isAxisSelected()) {
        // 删除坐标轴
        AxisData* axis = tree->getSelectedAxis();
        if (axis) {
            // 如果只有一个坐标轴，不允许删除
            if (dataManager->axisCount() <= 1) {
                QMessageBox::warning(this, "Warning", "至少保留一个坐标轴");
                return;
            }
            dataManager->removeAxis(axis);
        }
    }
    
    pic_viewer->refreshDisplay();
}

void MainScene::onAxisSelected(AxisData* axis) {
    // 坐标轴选中时，更新当前坐标轴
    if (axis) {
        dataManager->setCurrentAxis(axis);
        
        // 更新坐标轴类型下拉框
        cbx_x_type->setCurrentIndex(axis->xType);
        cbx_y_type->setCurrentIndex(axis->yType);
    }
}

void MainScene::onCurveSelected(CurveData* curve) {
    // 曲线选中时，更新当前曲线
    if (curve) {
        dataManager->setCurrentCurve(curve);
    }
}

void MainScene::onAxisPointAdded(AxisData* axis, const QPointF& pos) {
    // 坐标轴点已添加，刷新表格
    Q_UNUSED(pos)
    if (axis) {
        m_table_model->setCurrentAxis(axis);
    }
}

void MainScene::onCurvePointAdded(CurveData* curve, const QPointF& pos) {
    // 曲线点已添加，刷新表格
    Q_UNUSED(pos)
    if (curve) {
        m_table_model->setCurrentCurve(curve);
    }
}
