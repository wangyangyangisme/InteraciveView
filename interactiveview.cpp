#include "interactiveview.h"
#include <QWheelEvent>
#include <QKeyEvent>
#include <QScrollBar>
#include <QDebug>

#define VIEW_CENTER viewport()->rect().center()
#define VIEW_WIDTH  viewport()->rect().width()
#define VIEW_HEIGHT viewport()->rect().height()

InteractiveView::InteractiveView(QWidget *parent)
    : QGraphicsView(parent),
      m_translateButton(Qt::LeftButton),
      m_scale(1.0),
      m_zoomDelta(0.1),
      m_translateSpeed(1.0),
      m_bMouseTranslate(false)
{

    // 去掉滚动条
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    setCursor(Qt::PointingHandCursor);
    setRenderHint(QPainter::Antialiasing);

    setSceneRect(INT_MIN/2, INT_MIN/2, INT_MAX, INT_MAX);
    centerOn(0, 0);

}

// 平移速度
void InteractiveView::setTranslateSpeed(qreal speed)
{
    // 建议速度范围
    Q_ASSERT_X(speed >= 0.0 && speed <= 2.0,
               "InteractiveView::setTranslateSpeed", "Speed should be in range [0.0, 2.0].");
    m_translateSpeed = speed;
}

qreal InteractiveView::translateSpeed() const
{
    return m_translateSpeed;
}

// 缩放的增量
void InteractiveView::setZoomDelta(qreal delta)
{
    // 建议增量范围
    Q_ASSERT_X(delta >= 0.0 && delta <= 1.0,
               "InteractiveView::setZoomDelta", "Delta should be in range [0.0, 1.0].");
    m_zoomDelta = delta;
}

qreal InteractiveView::zoomDelta() const
{
    return m_zoomDelta;
}

// 上/下/左/右键向各个方向移动、加/减键进行缩放、空格/回车键旋转
void InteractiveView::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Up:
        translateF(QPointF(0, -2));  // 上移
        break;
    case Qt::Key_Down:
        translateF(QPointF(0, 2));  // 下移
        break;
    case Qt::Key_Left:
        translateF(QPointF(-2, 0));  // 左移
        break;
    case Qt::Key_Right:
        translateF(QPointF(2, 0));  // 右移
        break;
    case Qt::Key_Plus:  // 放大
        zoomIn();
        break;
    case Qt::Key_Minus:  // 缩小
        zoomOut();
        break;
    case Qt::Key_Space:  // 逆时针旋转
        rotate(-5);
        break;
    case Qt::Key_Enter:  // 顺时针旋转
    case Qt::Key_Return:
        rotate(5);
        break;
    default:
        QGraphicsView::keyPressEvent(event);
    }
}

// 平移
void InteractiveView::mouseMoveEvent(QMouseEvent *event)
{
    if (m_bMouseTranslate){
        setCursor (Qt::ClosedHandCursor);
        QPointF mouseDelta = mapToScene(event->pos()) - mapToScene(m_lastMousePos);
        translateF(mouseDelta);
        qDebug() << "mouseMove";
    }
    m_lastMousePos = event->pos();
    QGraphicsView::mouseMoveEvent(event);
}

void InteractiveView::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == m_translateButton) {
        //  QPointF point = mapToScene(event->pos());
        //  if (scene()->itemAt(point, transform()) == NULL)  {  // 当光标底下没有 item 时，才能移动
        m_bMouseTranslate = true;
        m_lastMousePos = event->pos();
        setCursor (Qt::ArrowCursor);
         qDebug() << "mousePress";
        //   }
    }
    QGraphicsView::mousePressEvent(event);
}

void InteractiveView::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == m_translateButton){
        m_bMouseTranslate = false;
        setCursor (Qt::ArrowCursor);
        qDebug() << "mouseRelease";
    }
    QGraphicsView::mouseReleaseEvent(event);
}

// 放大/缩小
void InteractiveView::wheelEvent(QWheelEvent *event)
{
    setCursor(Qt::BusyCursor);
    QPoint scrollAmount = event->angleDelta();
    scrollAmount.y() > 0 ? zoomIn() : zoomOut();
    setCursor(Qt::ArrowCursor);
    qDebug() << "mouseWheel";
}

void InteractiveView::mouseDoubleClickEvent(QMouseEvent *event)
{
  //  centerOn (0,0);
}

// 放大
void InteractiveView::zoomIn()
{
    zoom(1 + m_zoomDelta);
}

// 缩小
void InteractiveView::zoomOut()
{
    zoom(1 - m_zoomDelta);
}

// 缩放 - scaleFactor：缩放的比例因子
void InteractiveView::zoom(float scaleFactor)
{
    m_scale *= scaleFactor;
    // 防止过小或过大
//    qreal factor = transform().scale(scaleFactor, scaleFactor).mapRect(QRectF(0, 0, 1, 1)).width();
//    if (factor < 0.07 || factor > 100)
//        return;
    QPointF oldPos = mapToScene (m_lastMousePos);
    scale(scaleFactor, scaleFactor);
    QPointF newPos = mapToScene (m_lastMousePos);
    translateF (newPos - oldPos);

}

// 平移
void InteractiveView::translateF(QPointF delta)
{
    // 根据当前 zoom 缩放平移数
    delta *= m_scale;
    delta *= m_translateSpeed;

    //  view 根据鼠标下的点作为锚点来定位 scene
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    QPoint newCenter(VIEW_WIDTH / 2 - delta.x(),  VIEW_HEIGHT / 2 - delta.y());
    centerOn(mapToScene(newCenter));

    //scene 在 view 的中心点作为锚点
    setTransformationAnchor(QGraphicsView::AnchorViewCenter);
}


