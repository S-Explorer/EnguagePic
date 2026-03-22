#ifndef IMAGE_PREVIEWER_H
#define IMAGE_PREVIEWER_H
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QWheelEvent>
#include <QFileDialog>
#include <QMap>

class DataManager;
class AxisData;
class CurveData;

enum class DRAW_MODE{
    EMPTY,
    AXE,
    POINT,
};

class MarkerPoint {
public:
    MarkerPoint(Qt::GlobalColor c = Qt::red, qreal radius = 5.0, qreal lineWidth = 2.0);
    QGraphicsEllipseItem* circle = nullptr;
    QGraphicsLineItem* lineH = nullptr;
    QGraphicsLineItem* lineV = nullptr;
    Qt::GlobalColor colorP = Qt::red;
    qreal m_radius = 5.0;
    qreal m_lineWidth = 2.0;
    // record
    QPointF pixelPos;

    void addToScene(QGraphicsScene* scene, QPointF pos);
    void clear(QGraphicsScene* scene);
};

class ImagePreviewer : public QGraphicsView {
    Q_OBJECT

public:
    ImagePreviewer(QWidget *parent = nullptr);
    ~ImagePreviewer();

    void LoadImage(const QString &pic_path);
    void SetCurMode(DRAW_MODE mode);
    qreal GetImgHeight() const;
    
    // 设置数据管理器
    void setDataManager(DataManager* manager);
    
    // 刷新显示（当数据变化时调用）
    void refreshDisplay();
    
    // 显示/隐藏特定坐标轴
    void showAxis(AxisData* axis);
    void hideAxis(AxisData* axis);
    void setVisibleAxis(AxisData* axis);
    
    // 高亮显示当前选中的曲线
    void highlightCurve(CurveData* curve);

signals:
    // 数据变化信号（通知外部更新模型）
    void axisPointAdded(AxisData* axis, const QPointF& pos);
    void curvePointAdded(CurveData* curve, const QPointF& pos);
    void pointDeleted(void* data, int index);  // data可以是AxisData*或CurveData*

protected:
    void wheelEvent(QWheelEvent* e) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;

private:
    void clearAllMarkers();
    void renderAxis(AxisData* axis);
    void renderCurve(CurveData* curve, AxisData* parentAxis, bool isHighlighted = false);
    void removeAxisMarkers(AxisData* axis);
    void removeCurveMarkers(CurveData* curve);
    
    bool m_is_midbtn_press = false;
    bool m_has_pic = false;

    qreal img_height = 0.0;

    QGraphicsScene *m_scene;
    QGraphicsPixmapItem *m_item = nullptr;
    QPoint m_last_m_pos;
    DRAW_MODE cur_mode = DRAW_MODE::AXE;
    
    DataManager* dataManager = nullptr;
    
    // 存储所有标记点，按坐标轴和曲线组织
    // key: AxisData*, value: 该坐标轴的3个参考点
    QMap<AxisData*, QVector<MarkerPoint*>> axisMarkers;
    // key: CurveData*, value: 该曲线的所有数据点
    QMap<CurveData*, QVector<MarkerPoint*>> curveMarkers;
    
    // 轴点连接线（透明度30%的红线）
    QMap<AxisData*, QVector<QGraphicsLineItem*>> axisConnectionLines;
    
    // 当前可见的坐标轴（用于添加点）
    AxisData* currentVisibleAxis = nullptr;
    
    // 动态计算的点大小和线宽
    qreal m_pointRadius = 5.0;
    qreal m_lineWidth = 2.0;
    qreal m_connectionLineWidth = 1.0;
    
    // 颜色池，用于不同曲线
    static const Qt::GlobalColor curveColors[];
    static const int colorCount;
};

#endif
