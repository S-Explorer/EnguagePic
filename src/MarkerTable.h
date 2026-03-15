#ifndef MARKERTABLE_H
#define MARKERTABLE_H

#define LIN_MODE 0
#define LOG_MODE 1
#define AIX_MODE 2

#include <QAbstractTableModel>

class ImagePreviewer;

class MarkerTable : public QAbstractTableModel{
    Q_OBJECT
public:
    MarkerTable(ImagePreviewer* viwer);
    void DeleteRow(int row);
    void CalRelData(int x_type, int y_type);
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    int rowCount(const QModelIndex & /* parent */) const override;
    int columnCount(const QModelIndex & /* parent */) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

signals:
    void RowDeleted(int row);

private:
    void InsertData(bool is_axe, qreal x, qreal y);
    void ClearData();

    ImagePreviewer* m_viwer;
    QVector<QPointF> axe_points;
    QVector<QPointF> cur_points;

    QVector<QPointF> axe_real;
    QVector<QPointF> cur_real;
};

#endif // MARKERTABLE_H
