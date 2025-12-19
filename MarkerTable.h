#ifndef MARKERTABLE_H
#define MARKERTABLE_H

#include <QStyledItemDelegate>
#include <QPushButton>
#include <QApplication>
#include <QMouseEvent>
#include <QAbstractTableModel>

class ImagePreviewer;

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
};


class MarkerTable : public QAbstractTableModel{
    Q_OBJECT
public:
    MarkerTable(ImagePreviewer* viwer);
    void DeleteRow(int row);
    int rowCount(const QModelIndex & /* parent */) const override;
    int columnCount(const QModelIndex & /* parent */) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

signals:
    void RowDeleted(int row);

private:
    void InsertData(bool is_axe, qreal x, qreal y);
    void ClearData();

    ImagePreviewer* m_viwer;
    QVector<QPointF> axe_points;
    QVector<QPointF> cur_points;
};

#endif // MARKERTABLE_H
