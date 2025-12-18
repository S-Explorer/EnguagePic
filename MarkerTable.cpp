#include "MarkerTable.h"
#include "ImagePreviewer.h"

#include <QtGui/QColor>

QVector<QString> header_data{"name","x","y","remove"};

MarkerTable::MarkerTable(ImagePreviewer* viwer)
    : m_viwer(viwer){}

int MarkerTable::rowCount(const QModelIndex & /* parent */) const  {
    int marker_size = m_viwer->CurMarkerSize();
    return marker_size;
}

int MarkerTable::columnCount(const QModelIndex & /* parent */) const  {
    /* name x y btn */
    return 4;
}

QVariant MarkerTable::data(const QModelIndex &index, int role) const {
    // show data
    if (role == Qt::DisplayRole) {
        switch (index.column()) {
        case 0:
            if (index.row() < 3) {
                return QString("axe");
            }else{
                return QString("curve");
            }
            break;
        case 1:
            // x position
            if (index.row() < 3) {
                return m_viwer->axe_marker.at(index.row())->pixelPos.x();
            }else{
                return m_viwer->cur_marker.at(index.row())->pixelPos.x();
            }
            break;
        case 2:
            // y position
            if (index.row() < 3) {
                return m_viwer->axe_marker.at(index.row())->pixelPos.y();
            }else{
                return m_viwer->cur_marker.at(index.row())->pixelPos.y();
            }
            break;
        case 3:
            return QString("remove");
            break;
        default:
            break;
        }
    }
    // color
    if (role == Qt::BackgroundRole) {
        if (index.row() == 0) {
            return QColor(Qt::gray);
        }else if (index.row() < 3) {
            return QColor(Qt::yellow);
        }else{
            return QColor(Qt::magenta);
        }
    }
    // center
    if (role == Qt::TextAlignmentRole) {
        return Qt::AlignCenter;
    }

    return QVariant();
}

QVariant MarkerTable::headerData(int section, Qt::Orientation orientation, int role) const {

    if (role != Qt::DisplayRole){
        return QVariant();
    }

    if (orientation == Qt::Horizontal){
        return header_data.at(section);
    }

    return QVariant();
}
