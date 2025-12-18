#ifndef MARKERTABLE_H
#define MARKERTABLE_H

#include <QAbstractTableModel>

class ImagePreviewer;

class MarkerTable : public QAbstractTableModel{
    Q_OBJECT
public:
    MarkerTable(ImagePreviewer* viwer);
    int rowCount(const QModelIndex & /* parent */) const override;

    int columnCount(const QModelIndex & /* parent */) const override;

    QVariant data(const QModelIndex &index, int role) const override;

    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

private:
    ImagePreviewer* m_viwer;
};

#endif // MARKERTABLE_H
