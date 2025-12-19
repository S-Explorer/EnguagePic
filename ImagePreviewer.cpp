#include "ImagePreviewer.h"
#include <QScrollBar>
#include <QDebug>

/* * * * * * * * * * * * * * *
 *       MarkerPoint         *
 * * * * * * * * * * * * * * */
MarkerPoint::MarkerPoint(Qt::GlobalColor c)
    : colorP(c){}

void MarkerPoint::addToScene(QGraphicsScene* scene, QPointF pos) {
    pixelPos = pos;
    qreal r = 5;
    QPen pen(colorP, 2);

    // 十字圈：圆 + 横线 + 竖线
    circle = scene->addEllipse(pos.x() - r, pos.y() - r, r * 2, r * 2, pen);
    lineH = scene->addLine(pos.x() - r - 3, pos.y(), pos.x() + r + 3, pos.y(), pen);
    lineV = scene->addLine(pos.x(), pos.y() - r - 3, pos.x(), pos.y() + r + 3, pen);
}

void MarkerPoint::clear(QGraphicsScene* scene) {
    if (circle) scene->removeItem(circle);
    if (lineH) scene->removeItem(lineH);
    if (lineV) scene->removeItem(lineV);
    delete circle; delete lineH; delete lineV;
    circle = nullptr;
}


/* * * * * * * * * * * * * * *
 *       ImagePreviewer      *
 * * * * * * * * * * * * * * */

ImagePreviewer::ImagePreviewer(QWidget *parent)
    : QGraphicsView(parent){
    m_scene = new QGraphicsScene(this);
    m_scene->setItemIndexMethod(QGraphicsScene::NoIndex);

    setScene(m_scene);

    // add interact
    // setDragMode(QGraphicsView::ScrollHandDrag);
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setRenderHint(QPainter::SmoothPixmapTransform);

    //debug test
    //cur_mode = DRAW_MODE::AXE;
    //setCursor(Qt::CrossCursor);
}

void ImagePreviewer::LoadImage(const QString &pic_path){
    // clear old marker
    // warninig first
    if (!axe_marker.empty() || !cur_marker.empty()){
        ResetMarker();
    }
    // warning second
    // * first remove all items *
    // * second clear scene     *
    m_scene->clear();
    m_item = nullptr;

    QPixmap pixmap(pic_path);
    // open target pic
    if (!pixmap.isNull()) {
        m_item = m_scene->addPixmap(pixmap);
        m_scene->setSceneRect(pixmap.rect());
        fitInView(m_scene->sceneRect(), Qt::KeepAspectRatio);
    }
    m_has_pic = true;
}

void ImagePreviewer::SetCurMode(DRAW_MODE mode){
    cur_mode = mode;
    if (cur_mode == DRAW_MODE::EMPTY){
        setCursor(Qt::ArrowCursor);
    }else{
        setCursor(Qt::CrossCursor);
    }
}

void ImagePreviewer::ResetMarker(){
    for ( auto& marker_set : { axe_marker, cur_marker }){
        for (auto marker : marker_set) {
            marker->clear(m_scene);
            delete marker;
        }
    }
    axe_marker.clear();
    cur_marker.clear();

    emit ClearData();
}

int ImagePreviewer::CurMarkerSize(){
    return axe_marker.size() + cur_marker.size();
}

void ImagePreviewer::wheelEvent(QWheelEvent* e){
    const double scale_factor = 1.20;
    if (e->angleDelta().y() > 0){
        scale(scale_factor, scale_factor);
    } else {
        scale( 1. / scale_factor, 1. / scale_factor);
    }
}

void ImagePreviewer::mousePressEvent(QMouseEvent *event)
{
    if (cur_mode != DRAW_MODE::EMPTY  && event->button() == Qt::LeftButton && m_has_pic) {
        QVector<MarkerPoint*>* tmp;
        Qt::GlobalColor tmp_c;

        if (cur_mode == DRAW_MODE::AXE){
            if (axe_marker.size() < 3) {
                tmp = &axe_marker;
                tmp_c = Qt::red;
            }else{
                return;
            }
        }else{
            tmp = &cur_marker;
            tmp_c = Qt::blue;
        }

        // convert press point to scene pos
        QPointF scenePos = mapToScene(event->pos());
        // check press point is in scene?
        if (m_scene->sceneRect().contains(scenePos)) {
            MarkerPoint* m = new MarkerPoint(tmp_c);
            m->addToScene(m_scene, scenePos);
            tmp->append(m);
            emit AddRow(cur_mode == DRAW_MODE::AXE, scenePos.x(), scenePos.y());
            qDebug() << "marker size : " << tmp->size() << ", pos : " << scenePos;
        }

        return;
    }

    if (event->button() == Qt::MiddleButton) {
        m_last_m_pos = event->pos();
        this->setCursor(Qt::ClosedHandCursor);
        m_is_midbtn_press = true;
        event->accept();

        return;
    }

    QGraphicsView::mousePressEvent(event);
}

void ImagePreviewer::mouseMoveEvent(QMouseEvent *event)
{
    if (m_is_midbtn_press) {
        QPoint delta = m_last_m_pos - event->pos();
        m_last_m_pos = event->pos();

        // move pic by bar
        horizontalScrollBar()->setValue(horizontalScrollBar()->value() + delta.x());
        verticalScrollBar()->setValue(verticalScrollBar()->value() + delta.y());

        event->accept();
    } else {
        QGraphicsView::mouseMoveEvent(event);
    }
}

void ImagePreviewer::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::MiddleButton) {
        m_is_midbtn_press = false;
        this->unsetCursor();
        event->accept();
    } else {
        QGraphicsView::mouseReleaseEvent(event);
    }
}

void ImagePreviewer::DelMarkerData(int row){
    // remove marker
    qDebug() << "ImagePreviewer Delete Marker : " << row + 1;
    if (row < 3){
        axe_marker.at(row)->clear(m_scene);
        axe_marker.removeAt(row);
    }else {
        axe_marker.at(row - 3)->clear(m_scene);
        cur_marker.removeAt(row - 3);
    }
}

