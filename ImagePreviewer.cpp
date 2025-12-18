#include "ImagePreviewer.h"
#include <QScrollBar>

ImagePreviewer::ImagePreviewer(QWidget *parent)
    : QGraphicsView(parent){
    m_scene = new QGraphicsScene(this);
    setScene(m_scene);

    // add interact
    setDragMode(QGraphicsView::ScrollHandDrag);
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setRenderHint(QPainter::SmoothPixmapTransform);
}

void ImagePreviewer::LoadImage(const QString &pic_path){
    m_scene->clear();
    QPixmap pixmap(pic_path);
    // open target pic
    if (!pixmap.isNull()) {
        m_item = m_scene->addPixmap(pixmap);
        m_scene->setSceneRect(pixmap.rect());
    }
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
    if (event->button() == Qt::MiddleButton) {
        m_last_m_pos = event->pos();
        this->setCursor(Qt::ClosedHandCursor);
        m_is_midbtn_press = true;
        event->accept();
    } else {
        QGraphicsView::mousePressEvent(event);
    }
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


