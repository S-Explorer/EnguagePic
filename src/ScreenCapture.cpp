#include "ScreenCapture.h"

#include <QPainter>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QApplication>
#include <QScreen>
#include <QDebug>

ScreenCapture::ScreenCapture(QWidget* parent)
    : QWidget(nullptr, Qt::Window | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint) {
    
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_ShowWithoutActivating, false);
    setCursor(Qt::CrossCursor);
}

ScreenCapture::~ScreenCapture() {
}

bool ScreenCapture::startCapture() {
    // 获取当前鼠标所在的屏幕（多显示器支持）
    QPoint cursorPos = QCursor::pos();
    QScreen* screen = QApplication::screenAt(cursorPos);
    if (!screen) {
        screen = QApplication::primaryScreen();
    }
    if (!screen) {
        emit cancelled();
        return false;
    }
    
    // 截取全屏
    m_fullScreenPixmap = screen->grabWindow(0);
    
    // 重置状态
    m_isDragging = false;
    m_hasSelection = false;
    m_captureRect = QRect();
    m_capturedPixmap = QPixmap();
    
    // 设置窗口覆盖整个屏幕（包括任务栏区域）
    QRect screenGeometry = screen->geometry();
    setGeometry(screenGeometry);
    
    // 确保窗口在最前面，覆盖所有其他窗口
    setWindowState(Qt::WindowActive);
    
    // 显示窗口
    show();
    raise();
    activateWindow();
    
    // 强制重绘
    repaint();
    
    // 使用 QEventLoop 实现模态等待
    m_eventLoop = new QEventLoop(this);
    m_eventLoop->exec();
    delete m_eventLoop;
    m_eventLoop = nullptr;
    
    return !m_capturedPixmap.isNull();
}

void ScreenCapture::showEvent(QShowEvent* event) {
    QWidget::showEvent(event);
}

void ScreenCapture::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event)
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // 绘制全屏截图作为背景
    painter.drawPixmap(0, 0, m_fullScreenPixmap);
    
    if (m_hasSelection && m_captureRect.isValid()) {
        // 绘制灰色遮罩层（非选中区域）
        QRegion maskRegion(rect());
        maskRegion = maskRegion.subtracted(m_captureRect);
        
        painter.setClipRegion(maskRegion);
        painter.fillRect(rect(), QColor(0, 0, 0, 128));  // 半透明灰色
        painter.setClipping(false);
        
        // 绘制选中区域的边框
        QPen pen(QColor(0, 150, 255), 2);
        painter.setPen(pen);
        painter.drawRect(m_captureRect.adjusted(1, 1, -1, -1));
        
        // 绘制尺寸提示
        QString sizeText = QString("%1 x %2").arg(m_captureRect.width()).arg(m_captureRect.height());
        painter.setPen(Qt::white);
        painter.drawText(m_captureRect.topLeft() + QPoint(5, -5), sizeText);
        
        // 绘制按钮
        drawButtons(painter);
    } else {
        // 整个屏幕都是遮罩层
        painter.fillRect(rect(), QColor(0, 0, 0, 128));
        
        // 绘制提示文字
        painter.setPen(Qt::white);
        QFont font = painter.font();
        font.setPointSize(14);
        painter.setFont(font);
        QString hint = "拖拽鼠标选择截图区域，按 ESC 取消";
        painter.drawText(rect(), Qt::AlignCenter, hint);
    }
}

void ScreenCapture::drawButtons(QPainter& painter) {
    if (!m_captureRect.isValid()) return;
    
    // 按钮位置在选中区域下方
    int btnY = m_captureRect.bottom() + BUTTON_MARGIN;
    int btnX = m_captureRect.right() - BUTTON_WIDTH * 2 - BUTTON_MARGIN;
    
    // 如果下方空间不够，放在上方
    if (btnY + BUTTON_HEIGHT > height()) {
        btnY = m_captureRect.top() - BUTTON_HEIGHT - BUTTON_MARGIN;
    }
    
    // 确认按钮
    m_confirmBtnRect = QRect(btnX + BUTTON_WIDTH + BUTTON_MARGIN, btnY, BUTTON_WIDTH, BUTTON_HEIGHT);
    QColor confirmColor = m_isConfirmHovered ? QColor(0, 200, 100) : QColor(0, 150, 80);
    painter.fillRect(m_confirmBtnRect, confirmColor);
    painter.setPen(Qt::white);
    painter.drawText(m_confirmBtnRect, Qt::AlignCenter, "确认");
    painter.setPen(QColor(0, 100, 50));
    painter.drawRect(m_confirmBtnRect);
    
    // 取消按钮
    m_cancelBtnRect = QRect(btnX, btnY, BUTTON_WIDTH, BUTTON_HEIGHT);
    QColor cancelColor = m_isCancelHovered ? QColor(200, 80, 80) : QColor(150, 60, 60);
    painter.fillRect(m_cancelBtnRect, cancelColor);
    painter.setPen(Qt::white);
    painter.drawText(m_cancelBtnRect, Qt::AlignCenter, "取消");
    painter.setPen(QColor(100, 40, 40));
    painter.drawRect(m_cancelBtnRect);
}

void ScreenCapture::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        // 检查是否点击了按钮
        if (m_hasSelection && m_captureRect.isValid()) {
            if (m_confirmBtnRect.contains(event->pos())) {
                confirmCapture();
                return;
            }
            if (m_cancelBtnRect.contains(event->pos())) {
                cancelCapture();
                return;
            }
        }
        
        // 开始拖拽选择
        m_isDragging = true;
        m_startPos = event->pos();
        m_endPos = event->pos();
        m_hasSelection = false;
        update();
    }
}

void ScreenCapture::mouseMoveEvent(QMouseEvent* event) {
    QPoint pos = event->pos();
    
    if (m_isDragging) {
        m_endPos = pos;
        updateCaptureRect();
        m_hasSelection = true;
        update();
    } else if (m_hasSelection) {
        // 检查按钮悬停状态
        bool oldConfirmHover = m_isConfirmHovered;
        bool oldCancelHover = m_isCancelHovered;
        
        m_isConfirmHovered = m_confirmBtnRect.contains(pos);
        m_isCancelHovered = m_cancelBtnRect.contains(pos);
        
        if (oldConfirmHover != m_isConfirmHovered || oldCancelHover != m_isCancelHovered) {
            update();
        }
        
        // 如果鼠标在选中区域内，改变光标
        if (m_captureRect.contains(pos)) {
            setCursor(Qt::ArrowCursor);
        } else {
            setCursor(Qt::CrossCursor);
        }
    }
}

void ScreenCapture::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton && m_isDragging) {
        m_isDragging = false;
        m_endPos = event->pos();
        updateCaptureRect();
        
        // 如果区域太小，视为无效
        if (m_captureRect.width() < 10 || m_captureRect.height() < 10) {
            m_hasSelection = false;
        } else {
            m_hasSelection = true;
        }
        
        update();
    }
}

void ScreenCapture::updateCaptureRect() {
    int x1 = qMin(m_startPos.x(), m_endPos.x());
    int y1 = qMin(m_startPos.y(), m_endPos.y());
    int x2 = qMax(m_startPos.x(), m_endPos.x());
    int y2 = qMax(m_startPos.y(), m_endPos.y());
    
    m_captureRect = QRect(QPoint(x1, y1), QPoint(x2, y2));
}

void ScreenCapture::confirmCapture() {
    if (m_captureRect.isValid() && !m_captureRect.isEmpty()) {
        // 截取选中区域的图片
        m_capturedPixmap = m_fullScreenPixmap.copy(m_captureRect);
        emit captured(m_capturedPixmap);
    }
    if (m_eventLoop) {
        m_eventLoop->quit();
    }
    close();
}

void ScreenCapture::cancelCapture() {
    m_capturedPixmap = QPixmap();
    emit cancelled();
    if (m_eventLoop) {
        m_eventLoop->quit();
    }
    close();
}

void ScreenCapture::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Escape) {
        cancelCapture();
    } else if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        if (m_hasSelection) {
            confirmCapture();
        }
    }
}
