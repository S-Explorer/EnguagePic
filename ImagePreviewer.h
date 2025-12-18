#ifndef IMAGE_PREVIEWER_H
#define IMAGE_PREVIEWER_H
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QWheelEvent>
#include <QFileDialog>

enum class DRAW_MODE{
    EMPTY,
    AXE,
    POINT,
};

class MarkerPoint {
public:
    MarkerPoint(Qt::GlobalColor c = Qt::red);
    QGraphicsEllipseItem* circle = nullptr;
    QGraphicsLineItem* lineH = nullptr;
    QGraphicsLineItem* lineV = nullptr;
    Qt::GlobalColor colorP = Qt::red;
    // record
    QPointF pixelPos;

    void addToScene(QGraphicsScene* scene, QPointF pos);
    void clear(QGraphicsScene* scene);
};

class ImagePreviewer : public QGraphicsView {
    Q_OBJECT

public:
    friend class MarkerTable;

    ImagePreviewer(QWidget *parent = nullptr);

    void LoadImage(const QString &pic_path);
    void SetCurMode(DRAW_MODE mode);
    void ResetMarker();
    int CurMarkerSize();

protected:
    void wheelEvent(QWheelEvent* e) override;

    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:

    bool m_is_midbtn_press = false;
    bool m_has_pic = false;

    QGraphicsScene *m_scene;
    QGraphicsPixmapItem *m_item = nullptr;
    QPoint m_last_m_pos;
    DRAW_MODE cur_mode = DRAW_MODE::AXE;

    QVector<MarkerPoint*> axe_marker;
    QVector<MarkerPoint*> cur_marker;
};

#endif
