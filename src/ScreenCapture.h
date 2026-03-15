#ifndef SCREEN_CAPTURE_H
#define SCREEN_CAPTURE_H

#include <QWidget>
#include <QScreen>
#include <QPixmap>
#include <QPoint>
#include <QEventLoop>

// 截图选择窗口
class ScreenCapture : public QWidget {
    Q_OBJECT
public:
    explicit ScreenCapture(QWidget* parent = nullptr);
    ~ScreenCapture();

    // 开始截图，返回截图是否成功
    bool startCapture();

    // 获取截图结果
    QPixmap capturedPixmap() const { return m_capturedPixmap; }

signals:
    void captured(const QPixmap& pixmap);
    void cancelled();

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void showEvent(QShowEvent* event) override;

private:
    void updateCaptureRect();
    void confirmCapture();
    void cancelCapture();
    void drawButtons(QPainter& painter);

    QPixmap m_fullScreenPixmap;  // 全屏截图
    QPixmap m_capturedPixmap;    // 最终截取的图片
    
    QPoint m_startPos;           // 鼠标按下位置
    QPoint m_endPos;             // 鼠标当前位置
    QRect m_captureRect;         // 截图区域
    
    bool m_isDragging = false;   // 是否正在拖拽
    bool m_hasSelection = false; // 是否有选中区域
    
    // 按钮区域
    QRect m_confirmBtnRect;
    QRect m_cancelBtnRect;
    bool m_isConfirmHovered = false;
    bool m_isCancelHovered = false;
    
    QEventLoop* m_eventLoop = nullptr;
    
    static constexpr int BUTTON_WIDTH = 80;
    static constexpr int BUTTON_HEIGHT = 32;
    static constexpr int BUTTON_MARGIN = 10;
};

#endif // SCREEN_CAPTURE_H
