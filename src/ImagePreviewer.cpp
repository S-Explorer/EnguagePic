#include "ImagePreviewer.h"
#include "DataModel.h"
#include <QScrollBar>
#include <QDebug>
#include <QtMath>

// 颜色池，用于区分不同曲线
const Qt::GlobalColor ImagePreviewer::curveColors[] = {
    Qt::blue, Qt::green, Qt::cyan, Qt::magenta, 
    Qt::yellow, Qt::darkBlue, Qt::darkGreen, Qt::darkCyan,
    Qt::darkMagenta, Qt::darkYellow
};
const int ImagePreviewer::colorCount = 10;

/* * * * * * * * * * * * * * *
 *       MarkerPoint         *
 * * * * * * * * * * * * * * */
MarkerPoint::MarkerPoint(Qt::GlobalColor c, qreal radius, qreal lineWidth)
    : colorP(c), m_radius(radius), m_lineWidth(lineWidth){}

void MarkerPoint::addToScene(QGraphicsScene* scene, QPointF pos) {
    pixelPos = pos;
    qreal r = m_radius;
    QPen pen(colorP, m_lineWidth);

    // 十字圈：圆 + 横线 + 竖线
    circle = scene->addEllipse(pos.x() - r, pos.y() - r, r * 2, r * 2, pen);
    lineH = scene->addLine(pos.x() - r - r*0.6, pos.y(), pos.x() + r + r*0.6, pos.y(), pen);
    lineV = scene->addLine(pos.x(), pos.y() - r - r*0.6, pos.x(), pos.y() + r + r*0.6, pen);
}

void MarkerPoint::clear(QGraphicsScene* scene) {
    if (circle) scene->removeItem(circle);
    if (lineH) scene->removeItem(lineH);
    if (lineV) scene->removeItem(lineV);
    delete circle; delete lineH; delete lineV;
    circle = nullptr; lineH = nullptr; lineV = nullptr;
}


/* * * * * * * * * * * * * * *
 *       ImagePreviewer      *
 * * * * * * * * * * * * * * */

ImagePreviewer::ImagePreviewer(QWidget *parent)
    : QGraphicsView(parent), dataManager(nullptr), currentVisibleAxis(nullptr) {
    m_scene = new QGraphicsScene(this);
    m_scene->setItemIndexMethod(QGraphicsScene::NoIndex);

    setScene(m_scene);

    // add interact
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setRenderHint(QPainter::SmoothPixmapTransform);
}

ImagePreviewer::~ImagePreviewer() {
    clearAllMarkers();
}

void ImagePreviewer::setDataManager(DataManager* manager) {
    dataManager = manager;
    
    if (dataManager) {
        // 连接数据变化信号
        connect(dataManager, &DataManager::currentAxisChanged, this, [this](AxisData* axis) {
            currentVisibleAxis = axis;
            refreshDisplay();
        });
        
        connect(dataManager, &DataManager::currentCurveChanged, this, [this](CurveData* curve) {
            highlightCurve(curve);
        });
        
        connect(dataManager, &DataManager::axisAdded, this, &ImagePreviewer::refreshDisplay);
        connect(dataManager, &DataManager::axisRemoved, this, &ImagePreviewer::refreshDisplay);
        connect(dataManager, &DataManager::dataCleared, this, [this]() {
            clearAllMarkers();
        });
    }
}

void ImagePreviewer::LoadImage(const QString &pic_path){
    // 清空所有标记
    clearAllMarkers();
    
    // 清空场景
    m_scene->clear();
    m_item = nullptr;

    QPixmap pixmap(pic_path);

    img_height = pixmap.height();

    // 根据图片像素面积动态计算点大小和线宽
    // 使用图片对角线的百分比来计算
    qreal imageDiagonal = sqrt(pixmap.width() * pixmap.width() + pixmap.height() * pixmap.height());
    m_pointRadius = qMax(3.0, imageDiagonal / 200.0);  // 点半径：对角线的0.5%，最小3像素
    m_lineWidth = qMax(1.5, m_pointRadius / 2.5);      // 点线宽：点半径的40%
    m_connectionLineWidth = qMax(1.0, m_pointRadius / 4.0);  // 连接线宽：比圆点细

    // open target pic
    if (!pixmap.isNull()) {
        m_item = m_scene->addPixmap(pixmap);
        m_scene->setSceneRect(pixmap.rect());
        fitInView(m_scene->sceneRect(), Qt::KeepAspectRatio);
    }
    m_has_pic = true;
    
    // 刷新显示
    refreshDisplay();
}

void ImagePreviewer::SetCurMode(DRAW_MODE mode){
    cur_mode = mode;
    // 不再在这里设置光标，而是在 enterEvent/leaveEvent 中处理
}

qreal ImagePreviewer::GetImgHeight() const{
    return img_height;
}

void ImagePreviewer::refreshDisplay() {
    if (!dataManager) return;

    // 清除所有标记但保留图片
    clearAllMarkers();

    // 只渲染当前选中的坐标轴系统的点
    AxisData* currentAxis = dataManager->currentAxis;
    if (currentAxis) {
        renderAxis(currentAxis);

        // 渲染该坐标轴下的所有可见曲线
        for (int j = 0; j < currentAxis->curveCount(); j++) {
            CurveData* curve = currentAxis->getCurve(j);
            if (curve && curve->isVisible()) {
                bool isHighlighted = (curve == dataManager->currentCurve);
                renderCurve(curve, currentAxis, isHighlighted);
            }
        }
    }
}

void ImagePreviewer::clearAllMarkers() {
    // 清除所有坐标轴标记
    for (auto it = axisMarkers.begin(); it != axisMarkers.end(); ++it) {
        for (auto* marker : it.value()) {
            marker->clear(m_scene);
            delete marker;
        }
    }
    axisMarkers.clear();

    // 清除所有轴点连接线
    for (auto it = axisConnectionLines.begin(); it != axisConnectionLines.end(); ++it) {
        for (auto* line : it.value()) {
            m_scene->removeItem(line);
            delete line;
        }
    }
    axisConnectionLines.clear();

    // 清除所有曲线标记
    for (auto it = curveMarkers.begin(); it != curveMarkers.end(); ++it) {
        for (auto* marker : it.value()) {
            marker->clear(m_scene);
            delete marker;
        }
    }
    curveMarkers.clear();
}

void ImagePreviewer::renderAxis(AxisData* axis) {
    if (!axis) return;

    QVector<MarkerPoint*>& markers = axisMarkers[axis];
    QVector<QGraphicsLineItem*>& connectionLines = axisConnectionLines[axis];

    // 清除旧的标记
    for (auto* marker : markers) {
        marker->clear(m_scene);
        delete marker;
    }
    markers.clear();

    // 清除旧的连接线
    for (auto* line : connectionLines) {
        m_scene->removeItem(line);
        delete line;
    }
    connectionLines.clear();

    // 创建新的标记（使用动态计算的半径和线宽）
    for (int i = 0; i < axis->axePixelPoints.size(); i++) {
        MarkerPoint* marker = new MarkerPoint(Qt::red, m_pointRadius, m_lineWidth);
        marker->addToScene(m_scene, axis->axePixelPoints[i]);
        markers.append(marker);
    }

    // 如果有3个点，用透明度30%的红线连接形成平行四边形
    if (axis->axePixelPoints.size() == 3) {
        // 改用包围盒算法计算
        QPen connectionPen(QColor(255, 0, 0, 77), m_connectionLineWidth);

        // 计算所有点的边界
        qreal minX = axis->axePixelPoints[0].x();
        qreal maxX = axis->axePixelPoints[0].x();
        qreal minY = axis->axePixelPoints[0].y();
        qreal maxY = axis->axePixelPoints[0].y();

        for (const auto& point : axis->axePixelPoints) {
            minX = qMin(minX, point.x());
            maxX = qMax(maxX, point.x());
            minY = qMin(minY, point.y());
            maxY = qMax(maxY, point.y());
        }

        // 创建矩形的四个顶点
        QPointF topLeft(minX, minY);
        QPointF topRight(maxX, minY);
        QPointF bottomRight(maxX, maxY);
        QPointF bottomLeft(minX, maxY);

        // 连接矩形
        auto addLine = [&](const QPointF& p1, const QPointF& p2) {
            QGraphicsLineItem* line = m_scene->addLine(p1.x(), p1.y(), p2.x(), p2.y(), connectionPen);
            connectionLines.append(line);
        };

        addLine(topLeft, topRight);
        addLine(topRight, bottomRight);
        addLine(bottomRight, bottomLeft);
        addLine(bottomLeft, topLeft);
    }
}

void ImagePreviewer::renderCurve(CurveData* curve, AxisData* parentAxis, bool isHighlighted) {
    if (!curve || !parentAxis) return;

    QVector<MarkerPoint*>& markers = curveMarkers[curve];

    // 清除旧的标记
    for (auto* marker : markers) {
        marker->clear(m_scene);
        delete marker;
    }
    markers.clear();

    // 确定曲线颜色
    int colorIndex = 0;
    for (int i = 0; i < parentAxis->curveCount(); i++) {
        if (parentAxis->getCurve(i) == curve) {
            colorIndex = i % colorCount;
            break;
        }
    }

    Qt::GlobalColor color = curveColors[colorIndex];

    // 创建新的标记（使用动态计算的半径和线宽）
    for (int i = 0; i < curve->pixelPoints.size(); i++) {
        MarkerPoint* marker = new MarkerPoint(color, m_pointRadius, m_lineWidth);
        marker->addToScene(m_scene, curve->pixelPoints[i]);
        markers.append(marker);
    }
}

void ImagePreviewer::removeAxisMarkers(AxisData* axis) {
    if (axisMarkers.contains(axis)) {
        for (auto* marker : axisMarkers[axis]) {
            marker->clear(m_scene);
            delete marker;
        }
        axisMarkers.remove(axis);
    }
}

void ImagePreviewer::removeCurveMarkers(CurveData* curve) {
    if (curveMarkers.contains(curve)) {
        for (auto* marker : curveMarkers[curve]) {
            marker->clear(m_scene);
            delete marker;
        }
        curveMarkers.remove(curve);
    }
}

void ImagePreviewer::showAxis(AxisData* axis) {
    if (axis) {
        renderAxis(axis);
        currentVisibleAxis = axis;
    }
}

void ImagePreviewer::hideAxis(AxisData* axis) {
    if (axis) {
        removeAxisMarkers(axis);
        
        // 同时隐藏该坐标轴下的所有曲线
        for (int i = 0; i < axis->curveCount(); i++) {
            CurveData* curve = axis->getCurve(i);
            if (curve) {
                removeCurveMarkers(curve);
            }
        }
    }
}

void ImagePreviewer::setVisibleAxis(AxisData* axis) {
    currentVisibleAxis = axis;
    refreshDisplay();
}

void ImagePreviewer::highlightCurve(CurveData* curve) {
    // 重新渲染以更新高亮状态
    refreshDisplay();
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
    if (cur_mode != DRAW_MODE::EMPTY && event->button() == Qt::LeftButton && m_has_pic) {
        // convert press point to scene pos
        QPointF scenePos = mapToScene(event->pos());
        
        // check press point is in scene?
        if (!m_scene->sceneRect().contains(scenePos)) {
            return;
        }
        
        if (!dataManager || !dataManager->currentAxis) {
            return;
        }
        
        AxisData* currentAxis = dataManager->currentAxis;
        
        if (cur_mode == DRAW_MODE::AXE){
            // 添加坐标轴点
            if (currentAxis->axePointCount() < 3) {
                currentAxis->addAxePoint(scenePos, scenePos);

                // 创建标记（使用动态计算的半径和线宽）
                MarkerPoint* marker = new MarkerPoint(Qt::red, m_pointRadius, m_lineWidth);
                marker->addToScene(m_scene, scenePos);
                axisMarkers[currentAxis].append(marker);

                // 如果现在有3个点了，添加连接线
                if (currentAxis->axePointCount() >= 3) {
                    renderAxis(currentAxis);
                }

                emit axisPointAdded(currentAxis, scenePos);
            }
        } else if (cur_mode == DRAW_MODE::POINT) {
            // 添加曲线点
            if (!dataManager->currentCurve) {
                // 如果没有当前曲线，自动创建一条
                if (currentAxis->curveCount() == 0) {
                    dataManager->setCurrentCurve(currentAxis->addCurve());
                } else {
                    dataManager->setCurrentCurve(currentAxis->getCurve(0));
                }
            }
            
            if (dataManager->currentCurve) {
                CurveData* curve = dataManager->currentCurve;
                curve->addPoint(scenePos, scenePos);
                
                // 确定曲线颜色
                int colorIndex = 0;
                for (int i = 0; i < currentAxis->curveCount(); i++) {
                    if (currentAxis->getCurve(i) == curve) {
                        colorIndex = i % colorCount;
                        break;
                    }
                }
                
                // 创建标记（使用动态计算的半径和线宽）
                MarkerPoint* marker = new MarkerPoint(curveColors[colorIndex], m_pointRadius, m_lineWidth);
                marker->addToScene(m_scene, scenePos);
                curveMarkers[curve].append(marker);
                
                emit curvePointAdded(curve, scenePos);
            }
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
        // 鼠标释放后恢复十字光标（如果仍在区域内）
        setCursor(Qt::CrossCursor);
        event->accept();
    } else {
        QGraphicsView::mouseReleaseEvent(event);
    }
}

void ImagePreviewer::enterEvent(QEnterEvent *event)
{
    Q_UNUSED(event)
    // 鼠标进入Graphics区域时显示十字光标
    setCursor(Qt::CrossCursor);
}

void ImagePreviewer::leaveEvent(QEvent *event)
{
    Q_UNUSED(event)
    // 鼠标离开Graphics区域时恢复默认光标
    unsetCursor();
}
