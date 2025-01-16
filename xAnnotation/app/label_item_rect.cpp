#include <QPainter>
#include<QAction>
#include<QMenu>
#include <QGraphicsSceneMouseEvent>

#include "label_item_rect.h"

LabelItemRect::LabelItemRect() {
    ptCnt_ = 0;
    // curRotAngle_ = 0;
}


// 在view上点击了一个点
void LabelItemRect::MousePressForCreation(Qt::MouseButton button,  QPointF scenePos)
{
    if(button == Qt::LeftButton) {
        qDebug()<<"RectLabelItem::MousePressForCreation for creation";
        // 记录起点
        ptsForCreation_[ptCnt_++] = scenePos;
    }
}

void LabelItemRect::MouseMoveForCreateion(Qt::MouseButton button,  QPointF scenePos)
{
    if(state() == LABEL_STATE_CREATING) {
        // qDebug()<<"RectLabelItem::MouseMoveForCreateion";
        // 更新
        auto newRect = calcRect(ptsForCreation_[0], scenePos);
        prepareGeometryChange();
        setRect(QRectF(0,0, newRect.width(), newRect.height()) );
        setPos(newRect.topLeft());
    }
}

void LabelItemRect::MouseReleaseForCreation(Qt::MouseButton button,  QPointF scenePos)
{
    qDebug()<<"RectLabelItem::MouseReleaseForCreation";
    if(button == Qt::LeftButton) {
        ptsForCreation_[1] = scenePos;
        // 计算最终的rect
        auto newRect = calcRect(ptsForCreation_[0], ptsForCreation_[1]);
        prepareGeometryChange();
        setRect(QRectF(0,0, newRect.width(), newRect.height()) );
        setPos(newRect.topLeft());
        setState(LABEL_STATE_DISPLAY);
        qDebug()<<"RectLabelItem::MousePressForCreation, RectLabelItem create ok.("
                 <<ptsForCreation_[0].x()<<", "<<ptsForCreation_[0].y()<<")->("
                 <<ptsForCreation_[1].x()<<", "<<ptsForCreation_[1].y()<<")";
    }
}

// void LabelItemRect::setAngle(float angle)
// {
//     curRotAngle_ = angle;
//     this->setTransformOriginPoint(this->rect().center());
//     this->setRotation(angle);
// }


QRectF LabelItemRect::calcRect(const QPointF &firstPt, const QPointF &secondPt)
{
    float left_x,right_x, top_y, bot_y;
    //检查，获得左上角和右下角的坐标
    if( firstPt.x() < secondPt.x()) {
        left_x = firstPt.x();
        right_x = secondPt.x();
    } else {
        left_x = secondPt.x();
        right_x = firstPt.x();
    }
    if(firstPt.y() < secondPt.y()) {
        top_y = firstPt.y();
        bot_y = secondPt.y();
    } else {
        top_y = secondPt.y();
        bot_y = firstPt.y();
    }
    return QRectF(left_x, top_y, right_x-left_x, bot_y-top_y);
}


// 根据当前rect绘制anchors
void LabelItemRect::computeAnchors()
{
    auto rect = this->rect();
    auto x1 = rect.left();
    auto y1 = rect.top();
    auto x2 = rect.right();
    auto y2 = rect.bottom();
    auto mx = (x1+x2)/2;
    auto my = (y1+y2)/2;
    resizeAnchorRects_.insert(ResizeAnchor::Top, QRect(mx-ANCHOR_W, y1, ANCHOR_W*2, ANCHOR_W));             // 上
    resizeAnchorRects_.insert(ResizeAnchor::Bottom, QRect(mx-ANCHOR_W,y2-ANCHOR_W, ANCHOR_W*2, ANCHOR_W));   // 下
    resizeAnchorRects_.insert(ResizeAnchor::Left, QRect(x1,my-ANCHOR_W, ANCHOR_W, ANCHOR_W*2));             // 左
    resizeAnchorRects_.insert(ResizeAnchor::Right, QRect(x2-ANCHOR_W,my-ANCHOR_W, ANCHOR_W, ANCHOR_W*2));    // 右

    rotateAnchorRects_.insert(RotateAnchor::LeftTop, QRect(x1, y1, ANCHOR_W, ANCHOR_W));
    rotateAnchorRects_.insert(RotateAnchor::LeftBot, QRect(x1, y2-ANCHOR_W, ANCHOR_W, ANCHOR_W));
    rotateAnchorRects_.insert(RotateAnchor::RightTop, QRect(x2-ANCHOR_W, y1, ANCHOR_W, ANCHOR_W));
    rotateAnchorRects_.insert(RotateAnchor::RightBot, QRect(x2-ANCHOR_W, y2-ANCHOR_W, ANCHOR_W, ANCHOR_W));
}


/*
 * 在paint中分三种状态显示形状：
 * 1. 创建状态
 * 2. 显示状态
 */
void LabelItemRect::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    // painter->setRenderHints(QPainter::Antialiasing| QPainter::TextAntialiasing);
    if(state() == LABEL_STATE_CREATING) {
        // 创建状态
        if(ptCnt_ != 1) return;
        // 绘制从起始点到当前点的矩形
        painter->drawRect(rect());
    } else if(state() == LABEL_STATE_DISPLAY) {
        // 显示状态
        painter->setBrush(QBrush(QColor(128, 128, 0, 255/3)));
        painter->drawRect(rect());
    } else if(state() == LABEL_STATE_EDITING) {
        // 编辑状态, 绘制anchor
        painter->setBrush(QBrush(QColor(128, 0, 128, 255/3)));
        painter->drawRect(rect());
        computeAnchors();
        painter->drawRects(resizeAnchorRects_.values());
        painter->drawRects(rotateAnchorRects_.values());

    } else if(state() == LABEL_STATE_SELECTED) {
        painter->setBrush(QBrush(QColor(255, 0, 0, 255/3)));
        painter->drawRect(rect());
    }
}

void LabelItemRect::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if(state() == LABEL_STATE_CREATING) {   // 创建状态下不处理
        LabelAbstractItem::mousePressEvent(event);
        return;
    }
    // 记录按下时item的状态
    oldRect_ = rect();      //
    oldRotAngle_ = angle();

    // 检查鼠标press是否在resize锚点内，是就进行resize处理
    resizeAnchor_ = ResizeAnchor::None;
    foreach (auto anchorPos, resizeAnchorRects_.keys()) {
        auto anchorRect = resizeAnchorRects_[anchorPos];
        if(anchorRect.contains(event->pos() )) {
            resizeAnchor_ = anchorPos;
            // 注意Resize必须在Item自身的坐标系下操作
            dragStartPt_ = event->pos();        // in item self's coordinate frame
            return;
        }
    }
    // 检查鼠标press是否在resize锚点内，是就进行resize处理
    rotateAnchor_ = RotateAnchor::None;
    foreach (auto anchorPos, rotateAnchorRects_.keys()) {
        auto anchorRect = rotateAnchorRects_[anchorPos];
        if(anchorRect.contains(event->pos() )) {
            rotateAnchor_ = anchorPos;
            // 注意Rotate必须在scene坐标系下操作
            dragStartPt_ = event->scenePos();        // in scene's coordinate frame
            return;
        }
    }
    if(resizeAnchor_ == ResizeAnchor::None && rotateAnchor_ == RotateAnchor::None) {
        LabelAbstractItem::mousePressEvent(event);
    }
}

void LabelItemRect::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    // qDebug()<<"LabelRect::mouseMoveEvent";
    if(state() == LABEL_STATE_CREATING) {   // 创建状态下不处理
        LabelAbstractItem::mouseMoveEvent(event);
        return;
    }
    else if(state() == LABEL_STATE_EDITING) {
        if(resizeAnchor_ != ResizeAnchor::None ) {
            // 根据鼠标拖动情况，实时计算新的尺寸
            auto x = oldRect_.left();
            auto y = oldRect_.top();
            auto w = oldRect_.width();
            auto h = oldRect_.height();
            auto d = event->pos() - dragStartPt_;
            QRectF newRect;
            switch (resizeAnchor_) {
            case  ResizeAnchor::Top:
                newRect = QRectF(x, y+d.y(), w, h-d.y());
                break;
            case ResizeAnchor::Bottom:
                newRect = QRectF(x, y, w, h+d.y());
                break;
            case ResizeAnchor::Left:
                newRect = QRectF(x+d.x(), y, w-d.x(), h);
                break;
            case ResizeAnchor::Right:
                newRect = QRectF(x, y, w+d.x(), h);
                break;
            default:
                break;
            }
            if(newRect.width()<3*ANCHOR_W || newRect.height()< 3*ANCHOR_W)
                return;
            // 实时更新item的尺寸
            prepareGeometryChange();
            setRect(newRect);
            // LabelAbstractItem::mouseMoveEvent(event);
        }
        else if(rotateAnchor_ != RotateAnchor::None) {
            // 根据鼠标拖动情况，实时计算新的尺寸
            auto rect_center = this->pos()+rect().center();         //item's center pos in scene
            QVector2D v0 = QVector2D(dragStartPt_ - rect_center);
            QVector2D v1 = QVector2D(event->scenePos() - rect_center);
            auto theta = atan2(v1.y(), v1.x()) - atan2(v0.y(), v0.x());
            int angle = theta*180/M_PI;
            angle = angle % 360;
            if(angle< -180) angle +=360;
            else if(angle>180) angle -=360;
            // prepareGeometryChange();
            this->setAngle(oldRotAngle_ + angle);
        }
        else {
            LabelAbstractItem::mouseMoveEvent(event);
        }
    }
    else
        LabelAbstractItem::mouseMoveEvent(event);

}

void LabelItemRect::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    LabelAbstractItem::mouseReleaseEvent(event);
    if(state() == LABEL_STATE_CREATING) {
        // 创建状态下不处理
        return;
    }
    else if(state() == LABEL_STATE_EDITING) {
        // qDebug()<<"LabelRect::mouseReleaseEvent for editing";
        // 为了防止在有旋转角度的情况下，再对item改变尺寸，然后旋转会产生错误偏移。
        // 在改变Item的尺寸后，进行修正。
        auto theta = M_PI*angle()/180;
        auto u = QVector2D(rect().center() - oldRect_.center());
        auto vx = u.x()*cos(theta) - u.y()*sin(theta);
        auto vy = u.x()*sin(theta) + u.y()*cos(theta);
        prepareGeometryChange();
        setPos(pos()+ QPointF(vx-u.x(), vy-u.y()) ); //更改item在scene中的位置
        setAngle(angle());
    }
}

void LabelItemRect::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    event->accept();
    qDebug()<<"LabelRect::hoverEnterEvent";
}

void LabelItemRect::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    event->accept();
    qDebug()<<"LabelRect::hoverLeaveEvent";
}

void LabelItemRect::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    if(state()!=LABEL_STATE_EDITING) return;

    //检测是否在Resize锚点内, 是则改变光标
    resizeAnchor_ = ResizeAnchor::None;
    foreach (auto anchorPos, resizeAnchorRects_.keys()) {
        auto anchorRect = resizeAnchorRects_[anchorPos];
        if(anchorRect.contains(event->pos() )) {
            if(rotateCursor()!=nullptr)
                setCursor(*resizeCursor());
            return;
        }
    }
    //检测是否在Rotate锚点内, 是则改变光标
    rotateAnchor_ = RotateAnchor::None;
    foreach (auto anchorPos, rotateAnchorRects_.keys()) {
        auto anchorRect = rotateAnchorRects_[anchorPos];
        if(anchorRect.contains(event->pos() )) {
            if(rotateCursor()!=nullptr)
                setCursor(*rotateCursor());
            return;
        }
    }
    setCursor(Qt::ArrowCursor);
}
