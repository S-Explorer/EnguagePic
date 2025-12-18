#ifndef IMAGE_PREVIEWER_H
#define IMAGE_PREVIEWER_H
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QWheelEvent>
#include <QFileDialog>

class ImagePreviewer : public QGraphicsView {
    Q_OBJECT

public:
    ImagePreviewer(QWidget *parent = nullptr);

    void LoadImage(const QString &pic_path);

protected:
    void wheelEvent(QWheelEvent* e) override;

    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    QGraphicsScene *m_scene;
    QGraphicsPixmapItem *m_item = nullptr;
    QPoint m_last_m_pos;
    bool m_is_midbtn_press = false;
};

#endif
