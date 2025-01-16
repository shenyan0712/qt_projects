#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "media_player.h"
#include "pcm_play_device.h"

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
    void openFile();
    void playSliderMouseRelease();
    void updateVideoFrame(std::shared_ptr<AbstractVideoFrame>);

private:
    Ui::MainWindow *ui;
    MediaPlayer * player_;
    bool isPlayingBeforeSliderPress_=false;
    // PcmPlayDevice *pcmPlayDevice_;      // 音频播放设备
};
#endif // MAINWINDOW_H
