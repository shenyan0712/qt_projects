#ifndef LABEL_VIEW_H
#define LABEL_VIEW_H

#include <QGraphicsView>
#include <QGraphicsPixmapItem>
#include <QMouseEvent>

// #include <labelitem.h>
// #include <labeleditor_lib.h>

class LabelAbstractItem;

#include "label_abstract_item.h"

class LabelView : public QGraphicsView
{
    Q_OBJECT
signals:
    void sig_mousePosDispUpdate(QPointF pos);
public:
    LabelView(QWidget *parant=nullptr);
    ~LabelView();
    void setImage(QGraphicsPixmapItem *imgItem);    // 设置图像，由窗口类调用
    void rotateItem();                      // 旋转item
    void deleteItem();
    void createItem(LabelAbstractItem *newItem);    // 创建新的item, 由窗口类调用
    void cancelCreate();            // 结束当前的绘制，可当按下ESC键盘时由窗口类调用
    void deSelectItems();

public slots:
    void fitIn();           // 设置缩放比例，让view刚好可以显示所有的内容
    // void zoomIn();
    // void zoomOut();
//     void zoomInBy(int level);
//     void zoomOutBy(int level);

private:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

private:
    bool pan_;                              // 是否通过手掌移动view
    int panStartX_;
    int panStartY_;

    QPointF startPos_;                      // 记录鼠标按下的点，scene坐标系
    QGraphicsPixmapItem *imgItem_;
    LabelAbstractItem *newItem_;            // 当该项不为空时，指向正处于创建状态的Item

    QCursor rotateCursor_;
    QCursor resizeCursor_;

};

#endif // LABEL_VIEW_H
