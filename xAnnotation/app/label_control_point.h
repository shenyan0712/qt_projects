#ifndef LABEL_CONTROL_POINT_H
#define LABEL_CONTROL_POINT_H

#include <QGraphicsRectItem>

class AbstractLabelItem;

class LabelCtrlPoint : public QGraphicsRectItem
{
public:
    LabelCtrlPoint(AbstractLabelItem* labelItem);
private:
    AbstractLabelItem* labelItem_;
};

#endif // LABEL_CONTROL_POINT_H
