#ifndef MEDIA_PLAYER_H
#define MEDIA_PLAYER_H

#include <QObject>
#include <QTimer>
#include <memory>
#include <deque>
#include <QElapsedTimer>

// #include "ffmpeg_video_frame.h"
#include "abstract_video_decoder.h"
// #include "ffmpeg_decoder.h"
#include "pcm_play_device.h"

/*
 * 该类用于对整个视频的播放进行控制
 * 1. 封装打开视频文件、播放、暂停、停止、随机定位等基本功能
 * 2. 按照时间点从解码类提取解码后的视频帧
 * 3. 将视频帧发送给显示Widget进行显示
 *
*/


#define TIMER_PERIOD_SEC    0.01       // 音视频处理周期，即QTimer的定时执行周期
#define AV_SYNC_ERR     0.02            // 音频可延迟于视频，但保持在该范围内

class MediaPlayer:public QObject
{
    Q_OBJECT
signals:
    void updateVideoFrame(std::shared_ptr<AbstractVideoFrame>);     // 通知显示控件
    void updateAudioFrame(std::shared_ptr<AbstractAudioFrame>);     // 通知声音组件播放
    void endOfFile();

public:
    MediaPlayer(std::unique_ptr<AbstractMediaDecoder> decoder,
                std::unique_ptr<PcmPlayDevice> pcmDev, QObject* parent=nullptr);
    ~MediaPlayer();
    QString openFile(QString filePath); // 打开文件
    std::shared_ptr<VideoInfo> getVideoInfo();
    std::shared_ptr<AudioInfo> getAudioInfo();
    QString start();            // 启动播放, 会定时地发送视频准备好的视频
    QString pause();
    QString stop();             // 停止播放
    bool isPlaying() { return isPlaying_; }
    QString seek(double target_src);    // 跳到指定的视频位置，如果正在播放则从跳转后的位置继续播放
    QString seek(long frame_num);
    std::shared_ptr<AbstractVideoFrame> get_preview();     // 获得预览帧

private:
    void printAudioData(std::shared_ptr<AbstractAudioFrame>);

private slots:
    void timerCallback(void);
    bool endOfFileCheck();

private:
    std::unique_ptr<AbstractMediaDecoder> decoder_;
    bool isPlaying_=false;
    QTimer *timer_;
    std::shared_ptr<AbstractVideoFrame> preview_frame_=nullptr;      // 当打开文件或定位到视频的某个位置时用于预览
    std::unique_ptr<PcmPlayDevice> pcmDev_;
    double endTimeInAudioBuf_=0;      // 再音频设备中声音的结束时间

    QElapsedTimer elapsedTimer_;
    bool firstRun_ = true;             // 用于第一次播放，或者重新播放
    int timer_delay_=-1;               // 用于丢弃定时器前面几个不稳定的callback

};

#endif // MEDIA_PLAYER_H
