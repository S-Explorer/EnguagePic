#include "Style.h"

QString AppStyle::getGlobalStyle() {
    return R"(
        /* 全局字体 */
        QWidget {
            font-family: "Microsoft YaHei", "Segoe UI", sans-serif;
            font-size: 13px;
            color: #212121;
        }
        
        /* 主窗口背景 */
        QMainWindow, QWidget {
            background-color: #F5F5F5;
        }
        
        /* 分组框样式 */
        QGroupBox {
            background-color: #FFFFFF;
            border: 1px solid #E0E0E0;
            border-radius: 6px;
            margin-top: 12px;
            padding-top: 12px;
            padding: 12px;
        }
        
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 10px;
            padding: 0 8px;
            color: #009688;
            font-weight: bold;
        }
    )";
}

QString AppStyle::getPrimaryButtonStyle() {
    return R"(
        QPushButton {
            background-color: #009688;
            color: white;
            border: none;
            border-radius: 3px;
            padding: 4px 12px;
            font-weight: 500;
            min-width: 60px;
            min-height: 24px;
            font-size: 12px;
        }
        
        QPushButton:hover {
            background-color: #00BFA5;
        }
        
        QPushButton:pressed {
            background-color: #00796B;
        }
        
        QPushButton:disabled {
            background-color: #BDBDBD;
            color: #757575;
        }
    )";
}

QString AppStyle::getSecondaryButtonStyle() {
    return R"(
        QPushButton {
            background-color: #FFFFFF;
            color: #009688;
            border: 1px solid #009688;
            border-radius: 3px;
            padding: 4px 12px;
            font-weight: 500;
            min-width: 60px;
            min-height: 24px;
            font-size: 12px;
        }

        QPushButton:hover {
            background-color: #E0F2F1;
        }

        QPushButton:pressed {
            background-color: #B2DFDB;
        }

        QPushButton:checked {
            background-color: #009688;
            color: white;
            border: 1px solid #009688;
        }

        QPushButton:checked:hover {
            background-color: #00BFA5;
        }

        QPushButton:disabled {
            background-color: #F5F5F5;
            color: #BDBDBD;
            border-color: #E0E0E0;
        }
    )";
}

QString AppStyle::getDangerButtonStyle() {
    return R"(
        QPushButton {
            background-color: #F44336;
            color: white;
            border: none;
            border-radius: 3px;
            padding: 4px 12px;
            font-weight: 500;
            min-width: 60px;
            min-height: 24px;
            font-size: 12px;
        }
        
        QPushButton:hover {
            background-color: #EF5350;
        }
        
        QPushButton:pressed {
            background-color: #D32F2F;
        }
    )";
}

QString AppStyle::getLineEditStyle() {
    return R"(
        QLineEdit {
            background-color: #FFFFFF;
            border: 1px solid #E0E0E0;
            border-radius: 3px;
            padding: 3px 6px;
            font-size: 12px;
            selection-background-color: #009688;
        }
        
        QLineEdit:focus {
            border: 2px solid #009688;
        }
        
        QLineEdit:disabled {
            background-color: #F5F5F5;
            color: #9E9E9E;
        }
    )";
}

QString AppStyle::getTableViewStyle() {
    return R"(
        QTableView {
            background-color: #FFFFFF;
            border: 1px solid #E0E0E0;
            border-radius: 6px;
            gridline-color: #EEEEEE;
            selection-background-color: #B2DFDB;
            selection-color: #212121;
            alternate-background-color: #FAFAFA;
        }
        
        QTableView::item {
            padding: 4px;
            border: none;
        }
        
        QTableView::item:selected {
            background-color: #009688;
            color: white;
        }
        
        QTableView::item:hover {
            background-color: #E0F2F1;
        }
        
        /* 表头样式 */
        QHeaderView::section {
            background-color: #009688;
            color: white;
            padding: 5px;
            border: none;
            font-weight: bold;
            font-size: 12px;
        }
        
        QHeaderView::section:hover {
            background-color: #00BFA5;
        }
        
        /* 滚动条样式 */
        QScrollBar:vertical {
            background-color: #F5F5F5;
            width: 12px;
            border-radius: 6px;
        }
        
        QScrollBar::handle:vertical {
            background-color: #BDBDBD;
            border-radius: 6px;
            min-height: 30px;
        }
        
        QScrollBar::handle:vertical:hover {
            background-color: #9E9E9E;
        }
        
        QScrollBar::add-line:vertical,
        QScrollBar::sub-line:vertical {
            height: 0px;
        }
        
        QScrollBar:horizontal {
            background-color: #F5F5F5;
            height: 12px;
            border-radius: 6px;
        }
        
        QScrollBar::handle:horizontal {
            background-color: #BDBDBD;
            border-radius: 6px;
            min-width: 30px;
        }
        
        QScrollBar::handle:horizontal:hover {
            background-color: #9E9E9E;
        }
        
        QScrollBar::add-line:horizontal,
        QScrollBar::sub-line:horizontal {
            width: 0px;
        }
    )";
}

QString AppStyle::getComboBoxStyle() {
    return R"(
        QComboBox {
            background-color: #FFFFFF;
            border: 1px solid #E0E0E0;
            border-radius: 3px;
            padding: 3px 6px;
            min-width: 60px;
            font-size: 12px;
        }
        
        QComboBox:hover {
            border-color: #009688;
        }
        
        QComboBox:focus {
            border: 2px solid #009688;
        }
        
        QComboBox::drop-down {
            subcontrol-origin: padding;
            subcontrol-position: top right;
            width: 24px;
            border-left: 1px solid #E0E0E0;
        }
        
        QComboBox::down-arrow {
            image: none;
            border-left: 5px solid transparent;
            border-right: 5px solid transparent;
            border-top: 6px solid #009688;
            width: 0px;
            height: 0px;
        }
        
        QComboBox QAbstractItemView {
            background-color: #FFFFFF;
            border: 1px solid #E0E0E0;
            border-radius: 4px;
            selection-background-color: #E0F2F1;
            selection-color: #009688;
        }
    )";
}
