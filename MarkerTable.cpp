#include "MarkerTable.h"
#include "ImagePreviewer.h"

#include <QMessageBox>
#include <QtGui/QColor>

#include <QDebug>


QVector<QString> header_data{"name","x","y","remove"};

/* * * * * * * * * * * * * * *
 *       MarkerTable         *
 * * * * * * * * * * * * * * */

MarkerTable::MarkerTable(ImagePreviewer* viwer)
    : m_viwer(viwer){
    connect(m_viwer, &ImagePreviewer::AddRow, this, &MarkerTable::InsertData);
    connect(m_viwer, &ImagePreviewer::ClearData, this, &MarkerTable::ClearData);
    connect(this, &MarkerTable::RowDeleted, m_viwer, &ImagePreviewer::DelMarkerData);
}

int MarkerTable::rowCount(const QModelIndex & /* parent */) const  {
    return 3 + cur_points.size();
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
                if (index.row() < axe_points.size()) {
                    return axe_points.at(index.row()).x();
                }else {
                    return QString("-");
                }
            }else{
                if (index.row() < cur_points.size() + 3) {
                    return cur_points.at(index.row() - 3).x();
                }else {
                    return QString("-");
                }
            }
            break;
        case 2:
            // y position
            if (index.row() < 3) {
                if (index.row() < axe_points.size()) {
                    return axe_points.at(index.row()).y();
                }else {
                    return QString("-");
                }
            }else{
                if (index.row() < cur_points.size() + 3) {
                    return cur_points.at(index.row() - 3).y();
                }else {
                    return QString("-");
                }
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
        if (index.row() < 3) {
            return QColor(Qt::gray);
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

void MarkerTable::InsertData(bool is_axe, qreal x, qreal y){
    
    beginResetModel();
    if (is_axe){
        axe_points.emplace_back(QPointF(x, y));
    }else {
        cur_points.emplace_back(QPointF(x, y));
    }
    endResetModel();
}

void MarkerTable::ClearData(){
    beginResetModel();
    axe_points.clear();
    cur_points.clear();
    endResetModel();
}

void MarkerTable::DeleteRow(int row){
    qDebug() << "Table delete row : " << row + 1 << " , axe size : " << axe_points.size();  
    // protect
    if (row > axe_points.size() - 1) return;

    QMessageBox::StandardButton reply = QMessageBox::question( m_viwer,
            "delete data",
            "please make sure that you want delete this line",
            QMessageBox::Yes | QMessageBox::No);

    if (row >= 0 && reply == QMessageBox::Yes){
        beginResetModel();
        if (row < 3){
            axe_points.removeAt(row);
        }else{
            cur_points.removeAt(row - 3);
        }
        endResetModel();
        emit RowDeleted(row);
    }
}
