#include <QFileDialog>
#include <QMessageBox>

#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include "ffmpeg_decoder.h"
#include "./video_disp_widget.h"
#include "media_player.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    // auto videoDispWidget = new VideoDispWidget();
    // this->setCentralWidget(videoDispWidget);

    player_ = new MediaPlayer(std::make_unique<FFmpegDecoder>(), std::make_unique<PcmPlayDevice>(this), this);

    connect(ui->actionOpenFile, &QAction::triggered, this, &MainWindow::openFile);
    connect(ui->actionPlay, &QAction::triggered, this,  [this](bool triggered){
        qDebug()<<"slider pos:"<<ui->playSlider->value()<<", max:"<<ui->playSlider->maximum();
        if(ui->playSlider->value()== ui->playSlider->maximum()) {
            // 重新播放
            player_->seek(0.0);
        }
        player_->start();
    });
    connect(ui->actionPause, &QAction::triggered, this, [this](bool triggered){
        player_->pause();
    });
    connect(ui->actionStop, &QAction::triggered, this, [this](bool triggered){
        player_->stop();
        this->blockSignals(true);
        ui->playSlider->blockSignals(true);
        ui->playSlider->setValue(0);
        ui->playSlider->blockSignals(false);
        this->blockSignals(false);
        ui->videoDispWidget->updateVideoFrame(player_->get_preview());
    });
    connect(player_, &MediaPlayer::endOfFile, this, [this](){
        QMessageBox::information(this, "提示", "播放完毕");
    });

    connect(player_, &MediaPlayer::updateVideoFrame, ui->videoDispWidget, &VideoDispWidget::updateVideoFrame);
    connect(player_, &MediaPlayer::updateVideoFrame, this, &MainWindow::updateVideoFrame);

    connect(ui->playSlider, &QSlider::sliderReleased, this, &MainWindow::playSliderMouseRelease);
    connect(ui->playSlider, &QSlider::sliderPressed, this, [this]() {
        isPlayingBeforeSliderPress_ = player_->isPlaying();
        qDebug()<<"slider press, is playing:"<<isPlayingBeforeSliderPress_;
        player_->pause();
    });
    // 该事件先于sliderPressed，会造成处理上的混乱，因此不使用。
    // connect(ui->playSlider, &QSlider::valueChanged, this, &MainWindow::playSliderPosChanged);
    ui->playSlider->setFocusPolicy(Qt::FocusPolicy::NoFocus);       // 不接受按键，统一由父类处理

    // //test
    // VideoPlayer videoPlayer(std::make_unique<FFmpegDecoder>());
    // videoPlayer.openFile("D:\\test.mp4");
    // videoPlayer.seek(170);
    // videoPlayer.start();

    // for test
    // player_->openFile("D:\\test.mp4");

}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::openFile()
{
    QString mediaFilePath = QFileDialog::getOpenFileName(nullptr, QObject::tr("Open Media File"),
                                                         QDir::homePath(),
                                                         QObject::tr("Media Files (*.mp4 *.mkv)"));

    if (mediaFilePath.isEmpty()) return;
    auto retStr = player_->openFile(mediaFilePath);
    if( retStr.isEmpty()) {
        // 读取成功，显示相关信息
        auto fileInfo = player_->getVideoInfo();
        ui->playSlider->setMaximum(fileInfo->totalFrames-1);
        // 显示预览
        auto frame = player_->get_preview();
        if(frame) ui->videoDispWidget->updateVideoFrame(frame);
    } else {
        QMessageBox::critical(this, "错误", retStr);
    }
}



void MainWindow::playSliderMouseRelease()
{
    // return;
    long frameNum =  ui->playSlider->value();
    player_->seek(frameNum);

    if(isPlayingBeforeSliderPress_) {
        player_->start();
    } else {
        // 显示预览帧
        auto frame = player_->get_preview();
        if(frame)
            ui->videoDispWidget->updateVideoFrame(frame);
    }
}

void MainWindow::updateVideoFrame(std::shared_ptr<AbstractVideoFrame> frame)
{
    // qDebug()<<"MainWindow::updateFrame";
    // qDebug()<<"frame num:"<<frame->frame_num();
    this->blockSignals(true);
    ui->playSlider->blockSignals(true);
    ui->playSlider->setValue(frame->frame_num());
    ui->playSlider->blockSignals(false);
    this->blockSignals(false);
}
