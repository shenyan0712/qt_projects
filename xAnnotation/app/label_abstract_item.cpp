
#include "label_abstract_item.h"


LabelAbstractItem::LabelAbstractItem() {
    setPos(QPointF(0,0));
    rect_.setRect(0,0,1,1);
    state_ = LABEL_STATE_CREATING;
    editable_ = true;
    this->setFlags(QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsMovable);
    setAcceptHoverEvents(true);

    curRotAngle_ = 0;
    rotateCursor_ = nullptr;

}

void LabelAbstractItem::setAngle(float angle)
{
    curRotAngle_ = angle;
    this->setTransformOriginPoint(this->rect().center());
    this->setRotation(angle);
}


QVariant LabelAbstractItem::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value)
{
    // qDebug()<<"itemChanged:"<<change;
    if(change == QGraphicsItem::ItemSelectedHasChanged) {
        if(this->isSelected()) {
            if(this->editable_)
                state_ = LABEL_STATE_EDITING;
            else state_ = LABEL_STATE_SELECTED;
        }
        else
            state_ = LABEL_STATE_DISPLAY;
    }

    return QAbstractGraphicsShapeItem::itemChange(change, value);
}


