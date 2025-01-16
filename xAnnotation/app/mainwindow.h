#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>

#include "label_view.h"


QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void createRect(bool checked);
    void createEllipse(bool checked);
    void rotateItem(bool checked);
    void deleteItem(bool checked);

    void mousePosDispUpdate(QPointF pos);

private:
    Ui::MainWindow *ui;
    LabelView* view;
    QLabel *statusLabel;

};
#endif // MAINWINDOW_H
