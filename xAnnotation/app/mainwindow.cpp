
#include <QtQuick/QQuickView>
#include <QQmlEngine>
#include <QQmlContext>
#include <QScopedPointer>
#include <QGraphicsScene>
#include <QGraphicsView>

#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "label_item_rect.h"
#include "label_item_poly.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    statusLabel = new QLabel();
    ui->statusbar->addWidget(statusLabel);

    connect(ui->actionCreateRect, &QAction::triggered,this, &MainWindow::createRect);
    connect(ui->actionCreateEllipse, &QAction::triggered,this, &MainWindow::createEllipse);
    connect(ui->actionRotate, &QAction::triggered, this, &MainWindow::rotateItem);
    connect(ui->actionDelete, &QAction::triggered, this, &MainWindow::deleteItem);

    // 放大view
    connect(ui->actionZoomIn, &QAction::triggered, this, [this](bool checked) { this->view->scale(1.2, 1.2); });
    // 缩小view
    connect(ui->actionZoomOut, &QAction::triggered, this, [this](bool checked) { this->view->scale(0.8, 0.8); });
    connect(ui->actionScrollHandDrag, &QAction::triggered, this, [this](bool checked){
        if(view->dragMode() ==QGraphicsView::ScrollHandDrag ) this->view->setDragMode(QGraphicsView::RubberBandDrag);
        else this->view->setDragMode(QGraphicsView::ScrollHandDrag);
    });

    // const QIcon editIcon = QIcon(":/icon/rotate.png");
    // ui->actionCreateRect->setIcon(QIcon(img.scaled(32,32)));



    // 创建Scene
    QGraphicsScene *scene = new QGraphicsScene(this);
    // LabelingScene *scene = new LabelingScene(this);
    scene->setSceneRect(0,0, 640,640);          // 场景的尺寸
    scene->setBackgroundBrush(QBrush(Qt::gray));
    view = new LabelView(this);
    view->setScene(scene);
    connect(view, &LabelView::sig_mousePosDispUpdate, this, &MainWindow::mousePosDispUpdate);

    // image test
    QImage image("D:\\test.jpg");
    if(!image.isNull()) {
        QGraphicsPixmapItem *imgItem = new QGraphicsPixmapItem(QPixmap::fromImage(image));
        view->setImage(imgItem);
    }

    this->setCentralWidget(view);

    // MyItem *item = new MyItem();
    // item->setRect(0,0,100,100);
    // item->setPen(QPen(Qt::blue));
    // // item->setBrush(QBrush(Qt::blue));
    // scene->addItem(item);
    // item->setPos(100,100);
    // auto item = new RectLabelItem();
    // // item->setPos(100,100);
    // view->createItem(item);
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::createRect(bool checked)
{
    qDebug()<<"MainWindow::createRect";
    auto item = new LabelItemRect();
    // item->setPos(100,100);
    view->createItem(item);
}


void MainWindow::createEllipse(bool checked)
{
    qDebug()<<"MainWindow::createEllipse";
    auto item = new LabelItemRect();
    view->createItem(item);
}

void MainWindow::rotateItem(bool checked)
{
    view->rotateItem();
}

void MainWindow::deleteItem(bool checked)
{
    view->deleteItem();
}

void MainWindow::mousePosDispUpdate(QPointF pos)
{
    statusLabel->setText(QString::number(pos.x())+","+QString::number(pos.y()));
}
