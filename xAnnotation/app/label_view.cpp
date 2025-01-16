
#include <QScrollBar>
#include <QGraphicsItem>
#include <QMenu>
#include <QAction>

#include "label_view.h"
#include "label_abstract_item.h"

LabelView::LabelView(QWidget *parant): QGraphicsView(parant)
{
    this->setMouseTracking(true);
    imgItem_ = nullptr;
    newItem_ = nullptr;
    pan_ = false;
    // setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    setDragMode(QGraphicsView::RubberBandDrag);     // 默认是框选模式
    setInteractive(true);

    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setResizeAnchor(QGraphicsView::AnchorUnderMouse);
    setBackgroundBrush(QBrush(Qt::green));

    rotateCursor_ = QCursor(QPixmap(":/icon/rotate.png"));
    resizeCursor_ = QCursor(QPixmap(":/icon/resize.png"));

}

LabelView::~LabelView()
{

}

void LabelView::setImage(QGraphicsPixmapItem *imgItem)
{
    imgItem_ = imgItem;
    this->scene()->addItem(imgItem_);

    imgItem_->setPos(100,100);
    imgItem_->setZValue(-1);
    imgItem_->setFlag(QGraphicsItem::ItemIsSelectable, false);
    this->scene()->setSceneRect(0,0, imgItem_->pixmap().width()+200, imgItem_->pixmap().height()+200);
}

void LabelView::createItem(LabelAbstractItem *newItem)
{
    newItem_ = newItem;
    newItem_->configCursor(&resizeCursor_, &rotateCursor_);
    //添加到scene中,以便显示
    this->scene()->addItem(newItem);
    // newItem->setPos(100,100);
}

// 旋转item
void LabelView::rotateItem()
{
    auto items = scene()->selectedItems();
    foreach (auto item, items) {
        LabelAbstractItem* labelItem = dynamic_cast<LabelAbstractItem*>(item);
        if(labelItem!=nullptr) {
            auto angle = labelItem->angle();
            labelItem->setAngle(angle+10);
        }
    }

}

void LabelView::deleteItem()
{
    auto items = scene()->selectedItems();
    while(!items.empty()) {
        auto item = items.front();
        items.pop_front();
        scene()->removeItem(item);
        delete item;
    }
}


// 结束当前的绘制，可当按下ESC键盘时调用
void LabelView::cancelCreate()
{
    if(newItem_!=nullptr) {
        this->scene()->removeItem(newItem_);
        delete newItem_;
        newItem_ = nullptr;
    }

}

void LabelView::deSelectItems()
{
    auto selectedItems = this->scene()->selectedItems();
    for(auto item: selectedItems)
        item->setSelected(false);
}

// 设置缩放比例，让view刚好可以显示所有的内容
void LabelView::fitIn()
{

}

void LabelView::mousePressEvent(QMouseEvent *event)
{
    startPos_ = mapToScene(event->pos());
    if(event->button() == Qt::LeftButton && newItem_ !=nullptr) {
        newItem_->MousePressForCreation(event->button(), mapToScene(event->pos()));
        return;
    }
    if(event->button() == Qt::MiddleButton) {
        pan_ = true;
        panStartX_ = event->position().x();
        panStartY_ = event->position().y();
        setCursor(Qt::ClosedHandCursor);
        event->accept();
        return;
    }
    QGraphicsView::mousePressEvent(event);
}

void LabelView::mouseMoveEvent(QMouseEvent *event)
{
    // qDebug()<<"LabelingView::mouseMoveEvent";
    auto scenePos = mapToScene(event->pos());
    emit sig_mousePosDispUpdate(scenePos);

    if(newItem_ !=nullptr) {    //创建状态处理
        newItem_->MouseMoveForCreateion(event->button(), scenePos);
        // 当处于创建状态时，在状态栏显示鼠标坐标
        QGraphicsView::mouseMoveEvent(event);      // 调用基类处理函数，其中会处理item的鼠标callback
        newItem_->update();
        return;
    }
    if(pan_) {
        horizontalScrollBar()->setValue(horizontalScrollBar()->value() - (event->position().x() - panStartX_));
        verticalScrollBar()->setValue(verticalScrollBar()->value() - (event->position().y() - panStartY_));
        panStartX_ = event->position().x();
        panStartY_ = event->position().y();
        event->accept();
        return;
    }
    if(event->buttons().testFlag(Qt::LeftButton) && dragMode() == QGraphicsView::RubberBandDrag) {
        if(scenePos.x()> startPos_.x()) // 从左到右的框选
            this->setRubberBandSelectionMode(Qt::ItemSelectionMode::ContainsItemShape);
        else    // 从右到左的框选
            this->setRubberBandSelectionMode(Qt::ItemSelectionMode::IntersectsItemShape);

        QGraphicsView::mouseMoveEvent(event);   // 需调用父类函数才有选择框处理
        // 当选择框超出可视范围，调用scrollbar进行滚动
        auto x = event->pos().x();
        auto y = event->pos().y();
        if(this->horizontalScrollBar()!=nullptr) {  //处理X方向
            auto bar = this->horizontalScrollBar();
            if(x<0) bar->setValue(bar->value() -1);
            if(x>rect().width()) bar->setValue(bar->value()+1);
        }
        if(this->verticalScrollBar()!=nullptr) { //处理Y方向
            auto bar = this->verticalScrollBar();
            if(y<0) bar->setValue(bar->value() -1);
            if(y>rect().height()) bar->setValue(bar->value()+1);
        }
    }
    else {
        // qDebug()<<"default QGraphicsView::mouseMoveEvent";
        QGraphicsView::mouseMoveEvent(event);
    }
}

void LabelView::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button() == Qt::MiddleButton) {
        pan_ = false;
        setCursor(Qt::ArrowCursor);
        event->accept();
        return;
    }
    // qDebug()<<"LabelingView::mouseReleaseEvent";
    QGraphicsView::mouseReleaseEvent(event);
    if(event->button() == Qt::LeftButton) {
        if(newItem_ !=nullptr) {
            newItem_->MouseReleaseForCreation(event->button(), mapToScene(event->pos()));
            // ==== 创建时左键释放，检查是否创建完成
            if(newItem_->state() == LabelAbstractItem::LABEL_STATE_DISPLAY) {
                // 完成
                newItem_ ->update();
                newItem_ = nullptr;
                qDebug()<<"LabelView::mouseReleaseEvent, Item creation is done.";
            }
        }
        return;
    }
    else if(event->button() == Qt::RightButton) {
        if(newItem_ !=nullptr) {
            // ==== 创建时右键释放, 检查是否有上下文菜单
            QList<QString> menuItems = newItem_->ContextMenuForCreation();
            if(menuItems.size()!= 0) {
                // 有上下文，遵照上下文菜单处理
                QMenu menu(this);
                // 添加动作到菜单
                for(QString &itemStr: menuItems)
                    menu.addAction(itemStr);
                // 在鼠标点击位置显示菜单
                menu.exec(event->globalPosition().toPoint());
            } else {
                //无上下文菜单，中止创建
                this->scene()->removeItem(newItem_);
                delete newItem_;
                newItem_ = nullptr;
                qDebug()<<"LabelView::mouseReleaseEvent, Item creation canceled.";
            }
            return;
        }
    }
}


void LabelView::wheelEvent(QWheelEvent *event)
{
    if(event->angleDelta().y()>0) {
        this->scale(1.2, 1.2);
    } else {
        this->scale(0.8,0.8);
    }
}
