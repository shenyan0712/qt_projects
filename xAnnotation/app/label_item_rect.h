#ifndef LABEL_ITEM_RECT_H
#define LABEL_ITEM_RECT_H

#include "label_abstract_item.h"


enum class ResizeAnchor{
    Left,
    Right,
    Top,
    Bottom,
    None
};

enum class RotateAnchor{
    LeftTop,        // 左上
    RightTop,
    LeftBot,
    RightBot,
    None
};


class LabelItemRect : public LabelAbstractItem
{
public:
    LabelItemRect();
    void MousePressForCreation(Qt::MouseButton button,  QPointF scenePos) final;
    void MouseMoveForCreateion(Qt::MouseButton button,  QPointF scenePos) final;
    void MouseReleaseForCreation(Qt::MouseButton button,  QPointF scenePos) final;
    // void setAngle(float angle) final;

private:

    void mousePressEvent(QGraphicsSceneMouseEvent *event) override final;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override final;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event)override final;
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event)override final;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event)override final;
    void hoverMoveEvent(QGraphicsSceneHoverEvent *event)override final;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override final;

private:
    QRectF calcRect(const QPointF &firstPt, const QPointF &secondPt);
    void computeAnchors();            // 根据当前rect绘制anchors
private:
    int ptCnt_;                       //
    QPointF ptsForCreation_[2];       // 记录创建时的点

    QMap<ResizeAnchor, QRectF> resizeAnchorRects_;
    QMap<RotateAnchor, QRectF> rotateAnchorRects_;
    ResizeAnchor resizeAnchor_;       // 指示当前操作的是那一个resize锚点
    RotateAnchor rotateAnchor_;       // 指示当前操作的是那一个rotate锚点

    QPointF dragStartPt_;             // in scene's coordinate frame
    QRectF oldRect_;                  // in scene's coordinate frame
    float oldRotAngle_;               // in scene's coordinate frame
};

#endif // LABEL_ITEM_RECT_H
