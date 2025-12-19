#include "CustomDelegate.h"

#include <QDebug>

#define BTN_BOUNDRY_SZIE 2

/* * * * * * * * * * * * * * *
 *       ButtonDeleGate      *
 * * * * * * * * * * * * * * */

ButtonDelegate::ButtonDelegate(int btn_col, QObject* parent)
    : QStyledItemDelegate(parent), m_btn_col(btn_col) {}

void ButtonDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option,
        const QModelIndex& index) const {
    // btn column
    if (index.column() == m_btn_col && index.row() < 3){
        m_btn_rect = option.rect.adjusted(BTN_BOUNDRY_SZIE, BTN_BOUNDRY_SZIE,
                -BTN_BOUNDRY_SZIE, -BTN_BOUNDRY_SZIE);
        // btn style
        QStyleOptionButton btn_option;
        btn_option.rect = m_btn_rect;
        btn_option.text = "delete";
        btn_option.state = QStyle::State_Enabled;
        
        QWidget* widget = qobject_cast<QWidget*>(parent());
        if (widget) {
            QPoint mouse_pos = widget->mapFromGlobal(QCursor::pos());
            if (m_btn_rect.contains(mouse_pos)){
                btn_option.state |= QStyle::State_MouseOver;
            }
        }

        QApplication::style()->drawControl(QStyle::CE_PushButton, &btn_option,
                painter);
    }else {
        QStyledItemDelegate::paint(painter, option, index);
    }
}

bool ButtonDelegate::editorEvent(QEvent* e, QAbstractItemModel* model,
        const QStyleOptionViewItem& option, const QModelIndex& index){
    if (index.column() == m_btn_col && index.row() < 3) {
        if (e->type() == QEvent::MouseButtonPress){
            qDebug() << "Button Delegate Preses : " << index.row() ;
            QMouseEvent* mosue_e = static_cast<QMouseEvent*>(e);

            if (m_btn_rect.contains(mosue_e->pos())) {
                emit Clicked(index.row());
                qDebug() << "\tButton emit sig end!";
                return true;
            }
        }
    }

    return QStyledItemDelegate::editorEvent(e, model, option, index);
}

QSize ButtonDelegate::sizeHint(const QStyleOptionViewItem& option,
        const QModelIndex& index) const {
    if (index.column() == m_btn_col ){
        return QSize(60, 30);
    }

    return QStyledItemDelegate::sizeHint(option, index);
}

