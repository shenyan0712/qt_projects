#ifndef LABEL_ABSTRACT_ITEM_H
#define LABEL_ABSTRACT_ITEM_H

// #include <QGraphicsItem>
#include <QList>
#include <QString>
#include <QAbstractGraphicsShapeItem>
#include <QGraphicsRectItem>
#include <QMouseEvent>
#include <Qt>
#include <QCursor>


#define ANCHOR_W    8

class LabelAbstractItem : public QAbstractGraphicsShapeItem
// class LabelAbstractItem:public QGraphicsRectItem
{
public:
    enum LabelMsg{
        FINISHED,       // 创建完成
        // ENTER_EDIT,     // 进入
    };
    enum LabelState{
        LABEL_STATE_INITIALIZED,    // 刚初始化
        LABEL_STATE_DISPLAY,        // 仅仅显示，创建完成后会转入这个状态
        LABEL_STATE_CREATING,       // 正在创建
        LABEL_STATE_EDITING,        // 正在编辑
        LABEL_STATE_SELECTED,       // 仅仅被选中，但是无法编辑
    };

    LabelAbstractItem();

    virtual void MousePressForCreation(Qt::MouseButton button,  QPointF scenePos) =0;
    virtual void MouseMoveForCreateion(Qt::MouseButton button,  QPointF scenePos) =0;
    virtual void MouseReleaseForCreation(Qt::MouseButton button,  QPointF scenePos) =0;
    virtual QList<QString> ContextMenuForCreation(){ return QList<QString>(); };      // 查询创建时右击鼠标是否有上下文菜单
    virtual void ContextMenuConform(QString menuItemStr) { };                         // 上下文菜单反馈
    // virtual bool pointCheck(QPointF scenePt)=0;                                       // 检测给定场景点是否在该item的有效范围内
    void setEditable(bool editable) { editable_ = editable; }
    void setAngle(float angle);


    QRectF rect() { return rect_; }
    LabelState state() { return state_; }
    float angle() { return curRotAngle_; }
    void configCursor(QCursor* resizeCursor, QCursor* rotateCursor) {
        resizeCursor_ = resizeCursor;
        rotateCursor_ = rotateCursor;
    }
    QCursor* rotateCursor() { return rotateCursor_; }
    QCursor* resizeCursor() { return resizeCursor_; }

protected:
    void setRect(const QRectF &rect) { rect_ = rect; }
    void setState(LabelState state) { state_ = state;  }

private:
    QRectF boundingRect() const override { return this->rect_;  }       // 返回其在自身坐标系下的约束区域
    QPainterPath shape()const override {
        QPainterPath path;
        path.addRect(this->rect_);
        return path;
    }

    // virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    QVariant itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value) override;


    // void mousePressEvent(QGraphicsSceneMouseEvent *event) override final;
    // void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    // void mouseReleaseEvent(QGraphicsSceneMouseEvent *event)override;
    // void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;

private:
    QRectF rect_;
    LabelState state_;
    bool editable_;
    float curRotAngle_;

    QCursor *resizeCursor_;
    QCursor *rotateCursor_;
};





#endif // LABEL_ABSTRACT_ITEM_H
