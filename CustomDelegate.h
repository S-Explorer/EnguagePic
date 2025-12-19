#ifndef CUSTOM_DELEGATE_H
#define CUSTOM_DELEGATE_H

#include <QStyledItemDelegate>
#include <QPushButton>
#include <QApplication>
#include <QMouseEvent>


class ButtonDelegate : public QStyledItemDelegate {
    Q_OBJECT
public:
    explicit ButtonDelegate(int btn_col, QObject* parent = nullptr);

    // virtual func
    void paint(QPainter* painter, const QStyleOptionViewItem& option, 
            const QModelIndex& index) const override;
    bool editorEvent(QEvent* e, QAbstractItemModel* model,
            const QStyleOptionViewItem& option,
            const QModelIndex& index) override;
    QSize sizeHint(const QStyleOptionViewItem& option,
            const QModelIndex& index) const override;

signals:
    void Clicked(int row);

private:
    int m_btn_col;
    mutable QRect m_btn_rect;
    mutable QModelIndex m_press_index;
};

class EditDelegate : public QStyledItemDelegate {
    Q_OBJECT
public:
    explicit EditDelegate(QObject* parent = nullptr);

    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option,
            const QModelIndex& index)const override;
    void setEditorData(QWidget* editor, const QModelIndex& index)const override;
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option,
            const QModelIndex &index) const override;
    void paint(QPainter *painter, const QStyleOptionViewItem &option,
            const QModelIndex &index) const override;

private:
    QMap<int, double> m_data;
};

#endif
