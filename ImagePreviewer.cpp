#include "ImagePreviewer.h"


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


