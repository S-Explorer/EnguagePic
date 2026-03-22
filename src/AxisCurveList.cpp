#include "AxisCurveList.h"
#include "DataModel.h"
#include "Style.h"
#include <QMenu>
#include <QMessageBox>
#include <QHeaderView>

/* * * * * * * * * * * * * * *
 *    AxisCurveTreeWidget    *
 * * * * * * * * * * * * * * */

AxisCurveTreeWidget::AxisCurveTreeWidget(QWidget* parent)
    : QTreeWidget(parent), isRefreshing(false) {

    setHeaderLabel("坐标轴 / 曲线");
    setContextMenuPolicy(Qt::CustomContextMenu);

    connect(this, &QTreeWidget::itemClicked, this, &AxisCurveTreeWidget::onItemClicked);
    connect(this, &QTreeWidget::itemChanged, this, &AxisCurveTreeWidget::onItemChanged);
    connect(this, &QTreeWidget::customContextMenuRequested, this, &AxisCurveTreeWidget::onContextMenu);
}

void AxisCurveTreeWidget::setDataManager(DataManager* manager) {
    dataManager = manager;
    
    if (dataManager) {
        connect(dataManager, &DataManager::axisAdded, this, &AxisCurveTreeWidget::refresh);
        connect(dataManager, &DataManager::axisRemoved, this, &AxisCurveTreeWidget::refresh);
        connect(dataManager, &DataManager::currentAxisChanged, this, &AxisCurveTreeWidget::refresh);
        connect(dataManager, &DataManager::currentCurveChanged, this, &AxisCurveTreeWidget::refresh);
        connect(dataManager, &DataManager::dataCleared, this, &AxisCurveTreeWidget::refresh);
    }
    
    refresh();
}

void AxisCurveTreeWidget::refresh() {
    isRefreshing = true;
    clear();

    if (!dataManager) {
        isRefreshing = false;
        return;
    }

    for (int i = 0; i < dataManager->axisCount(); i++) {
        AxisData* axis = dataManager->getAxis(i);
        if (!axis) continue;

        // 创建坐标轴项
        QTreeWidgetItem* axisItem = new QTreeWidgetItem(this);
        axisItem->setText(0, axis->getName());
        axisItem->setData(0, AxisRole, QVariant::fromValue(axis));

        // 设置图标
        axisItem->setIcon(0, QIcon("::/axis_icon.png"));

        // 如果是当前选中的坐标轴，高亮显示
        if (axis == dataManager->currentAxis) {
            axisItem->setSelected(true);
            axisItem->setExpanded(true);
        }

        // 添加曲线子项
        for (int j = 0; j < axis->curveCount(); j++) {
            CurveData* curve = axis->getCurve(j);
            if (!curve) continue;

            QTreeWidgetItem* curveItem = new QTreeWidgetItem(axisItem);
            curveItem->setText(0, curve->getName());
            curveItem->setData(0, CurveRole, QVariant::fromValue(curve));
            curveItem->setData(0, AxisRole, QVariant::fromValue(axis));

            // 添加复选框用于显示/隐藏曲线
            curveItem->setFlags(curveItem->flags() | Qt::ItemIsUserCheckable);
            curveItem->setCheckState(0, curve->isVisible() ? Qt::Checked : Qt::Unchecked);

            // 设置图标
            curveItem->setIcon(0, QIcon("::/curve_icon.png"));

            // 如果是当前选中的曲线，高亮显示并设置为当前项
            if (curve == dataManager->currentCurve) {
                curveItem->setSelected(true);
                setCurrentItem(curveItem);
            }
        }
    }

    // 如果没有选中项，但存在坐标轴，默认选中第一个坐标轴
    if (selectedItems().isEmpty() && dataManager->axisCount() > 0) {
        QTreeWidgetItem* firstItem = topLevelItem(0);
        if (firstItem) {
            firstItem->setSelected(true);
            setCurrentItem(firstItem);
        }
    }

    isRefreshing = false;
}

bool AxisCurveTreeWidget::isAxisSelected() const {
    QTreeWidgetItem* item = currentItem();
    if (!item) return false;
    return item->data(0, AxisRole).isValid() && !item->data(0, CurveRole).isValid();
}

bool AxisCurveTreeWidget::isCurveSelected() const {
    QTreeWidgetItem* item = currentItem();
    if (!item) return false;
    return item->data(0, CurveRole).isValid();
}

AxisData* AxisCurveTreeWidget::getSelectedAxis() const {
    QTreeWidgetItem* item = currentItem();
    if (!item) return nullptr;
    
    QVariant data = item->data(0, AxisRole);
    if (data.isValid()) {
        return data.value<AxisData*>();
    }
    return nullptr;
}

CurveData* AxisCurveTreeWidget::getSelectedCurve() const {
    QTreeWidgetItem* item = currentItem();
    if (!item) return nullptr;
    
    QVariant data = item->data(0, CurveRole);
    if (data.isValid()) {
        return data.value<CurveData*>();
    }
    return nullptr;
}

void AxisCurveTreeWidget::selectAxis(AxisData* axis) {
    for (int i = 0; i < topLevelItemCount(); i++) {
        QTreeWidgetItem* item = topLevelItem(i);
        if (item->data(0, AxisRole).value<AxisData*>() == axis) {
            setCurrentItem(item);
            item->setExpanded(true);
            break;
        }
    }
}

void AxisCurveTreeWidget::selectCurve(CurveData* curve) {
    for (int i = 0; i < topLevelItemCount(); i++) {
        QTreeWidgetItem* axisItem = topLevelItem(i);
        for (int j = 0; j < axisItem->childCount(); j++) {
            QTreeWidgetItem* curveItem = axisItem->child(j);
            if (curveItem->data(0, CurveRole).value<CurveData*>() == curve) {
                setCurrentItem(curveItem);
                return;
            }
        }
    }
}

void AxisCurveTreeWidget::onItemClicked(QTreeWidgetItem* item, int column) {
    if (!item) return;
    
    if (item->data(0, CurveRole).isValid()) {
        // 点击的是曲线
        CurveData* curve = item->data(0, CurveRole).value<CurveData*>();
        AxisData* axis = item->data(0, AxisRole).value<AxisData*>();
        
        if (axis && dataManager) {
            // 即使坐标轴已经是当前选中的，也要强制更新曲线
            if (dataManager->currentAxis == axis) {
                // 坐标轴已经是当前选中的，只更新曲线
                if (curve) {
                    dataManager->setCurrentCurve(curve);
                }
            } else {
                // 坐标轴不是当前选中的，先更新坐标轴（这会触发曲线更新）
                dataManager->setCurrentAxis(axis);
                // 然后选择特定的曲线
                if (curve) {
                    dataManager->setCurrentCurve(curve);
                }
            }
        }
        
        emit curveSelected(curve);
    } else if (item->data(0, AxisRole).isValid()) {
        // 点击的是坐标轴
        AxisData* axis = item->data(0, AxisRole).value<AxisData*>();
        
        if (axis && dataManager) {
            dataManager->setCurrentAxis(axis);
        }
        
        emit axisSelected(axis);
    }
}

void AxisCurveTreeWidget::onItemChanged(QTreeWidgetItem* item, int column) {
    // 避免在刷新时处理信号
    if (isRefreshing) return;
    
    // 只处理曲线项的复选框变化
    if (item->data(0, CurveRole).isValid()) {
        CurveData* curve = item->data(0, CurveRole).value<CurveData*>();
        if (curve) {
            bool visible = (item->checkState(0) == Qt::Checked);
            curve->setVisible(visible);
            // 立即发出信号通知外部刷新显示
            emit curveVisibilityChanged(curve, visible);
        }
    }
}

void AxisCurveTreeWidget::onContextMenu(const QPoint& pos) {
    QTreeWidgetItem* item = itemAt(pos);
    if (!item) return;
    
    QMenu menu(this);
    QAction* deleteAction = menu.addAction("删除");
    
    if (menu.exec(mapToGlobal(pos)) == deleteAction) {
        emit deleteRequested();
    }
}

/* * * * * * * * * * * * * * *
 *        LeftPanel          *
 * * * * * * * * * * * * * * */

LeftPanel::LeftPanel(QWidget* parent)
    : QWidget(parent) {
    setupUI();
}

void LeftPanel::setupUI() {
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(5, 5, 5, 5);
    layout->setSpacing(5);
    
    // 树形控件
    treeWidget = new AxisCurveTreeWidget(this);
    treeWidget->setMinimumWidth(150);
    treeWidget->setMaximumWidth(250);
    
    // 按钮
    btnAddAxis = new QPushButton("+ 坐标轴", this);
    btnAddCurve = new QPushButton("+ 曲线", this);
    btnDelete = new QPushButton("- 删除", this);
    
    // 应用样式
    btnAddAxis->setStyleSheet(AppStyle::getPrimaryButtonStyle());
    btnAddCurve->setStyleSheet(AppStyle::getSecondaryButtonStyle());
    btnDelete->setStyleSheet(AppStyle::getDangerButtonStyle());
    
    // 连接信号
    connect(btnAddAxis, &QPushButton::clicked, this, &LeftPanel::addAxisRequested);
    connect(btnAddCurve, &QPushButton::clicked, this, &LeftPanel::addCurveRequested);
    connect(btnDelete, &QPushButton::clicked, this, &LeftPanel::deleteRequested);
    
    connect(treeWidget, &AxisCurveTreeWidget::axisSelected, this, &LeftPanel::axisSelected);
    connect(treeWidget, &AxisCurveTreeWidget::curveSelected, this, &LeftPanel::curveSelected);
    connect(treeWidget, &AxisCurveTreeWidget::deleteRequested, this, &LeftPanel::deleteRequested);
    connect(treeWidget, &AxisCurveTreeWidget::curveVisibilityChanged, this, &LeftPanel::curveVisibilityChanged);
    
    // 添加到布局
    layout->addWidget(treeWidget);
    
    QHBoxLayout* btnLayout = new QHBoxLayout;
    btnLayout->addWidget(btnAddAxis);
    btnLayout->addWidget(btnAddCurve);
    layout->addLayout(btnLayout);
    
    layout->addWidget(btnDelete);
    
    setLayout(layout);
}

void LeftPanel::setDataManager(DataManager* manager) {
    dataManager = manager;
    treeWidget->setDataManager(manager);
}
