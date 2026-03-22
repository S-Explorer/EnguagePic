#include "CustomDelegate.h"

#include <QLineEdit>
#include <QDebug>

#define BTN_BOUNDRY_SZIE 0

/* * * * * * * * * * * * * * *
 *       ButtonDeleGate      *
 * * * * * * * * * * * * * * */

ButtonDelegate::ButtonDelegate(int btn_col, QObject* parent)
    : QStyledItemDelegate(parent), m_btn_col(btn_col) {}

void ButtonDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option,
        const QModelIndex& index) const {
    // btn column - 为所有行显示删除按钮（包括轴点和线点）
    if (index.column() == m_btn_col){
        m_btn_rect = option.rect.adjusted(BTN_BOUNDRY_SZIE, BTN_BOUNDRY_SZIE,
                -BTN_BOUNDRY_SZIE, -BTN_BOUNDRY_SZIE);
        // btn style
        QStyleOptionButton btn_option;
        btn_option.rect = m_btn_rect;
        btn_option.text = "delete";
        btn_option.state = QStyle::State_Enabled;

        if (option.state & QStyle::State_MouseOver){
            btn_option.state |= QStyle::State_MouseOver;
        }

        if (index == m_press_index) {
            btn_option.state |= QStyle::State_Sunken;
        } else {
            btn_option.state |= QStyle::State_Raised;
        }

        QApplication::style()->drawControl(QStyle::CE_PushButton, &btn_option,
                painter);
    }else {
        QStyledItemDelegate::paint(painter, option, index);
    }
}

bool ButtonDelegate::editorEvent(QEvent* e, QAbstractItemModel* model,
        const QStyleOptionViewItem& option, const QModelIndex& index){

    QMouseEvent* mosue_e = static_cast<QMouseEvent*>(e);

    if (mosue_e->type() == QEvent::MouseButtonPress) {
        if (option.rect.contains(mosue_e->pos())) {
            m_press_index = index;
            const_cast<QWidget*>(option.widget)->update(option.rect);
            return true;
        }
    }
    else if (mosue_e->type() == QEvent::MouseButtonRelease) {
        if (m_press_index == index) {
            m_press_index = QModelIndex();
            const_cast<QWidget*>(option.widget)->update(option.rect);

            if (option.rect.contains(mosue_e->pos())) {
                emit Clicked(index.row());
            }
            return true;
        }
    }
    else if (mosue_e->type() == QEvent::MouseMove) {
        if (m_press_index == index && !option.rect.contains(mosue_e->pos())) {
            m_press_index = QModelIndex();
            const_cast<QWidget*>(option.widget)->update(option.rect);
        }
    }

    // if (index.column() == m_btn_col && index.row() < 3) {
    //     if (e->type() == QEvent::MouseButtonRelease){
    //         qDebug() << "Button Delegate Preses : " << index.row() ;

    //         if (option.rect.contains(mosue_e->pos())) {
    //             emit Clicked(index.row());
    //             qDebug() << "\tButton emit sig end!";
    //             return true;
    //         }
    //     }
    // }

    return QStyledItemDelegate::editorEvent(e, model, option, index);
}

QSize ButtonDelegate::sizeHint(const QStyleOptionViewItem& option,
        const QModelIndex& index) const {
    if (index.column() == m_btn_col ){
        return QSize(60, 30);
    }

    return QStyledItemDelegate::sizeHint(option, index);
}

/* * * * * * * * * * * * * * *
 *       EditDelegate        *
 * * * * * * * * * * * * * * */

EditDelegate::EditDelegate(QObject *parent)
    : QStyledItemDelegate(parent){}

QWidget *EditDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                                    const QModelIndex &index) const{
    QLineEdit* new_editor = new QLineEdit(parent);
    new_editor->setStyleSheet("QLineEdit { background-color: white; color: black; }");
    return new_editor;
}

void EditDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const{
    QString text = index.model()->data(index, Qt::DisplayRole).toString();
    QLineEdit *lineEdit = qobject_cast<QLineEdit*>(editor);
    lineEdit->setText(text);
}

void EditDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option,
                                        const QModelIndex &index) const{
    editor->setGeometry(option.rect);
}

void EditDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                         const QModelIndex &index) const{
    QStyledItemDelegate::paint(painter, option, index);
}
