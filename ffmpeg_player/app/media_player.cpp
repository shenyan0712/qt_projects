
#include <QDebug>
#include <QDateTime>
#include <queue>

#include "media_player.h"


MediaPlayer::MediaPlayer(std::unique_ptr<AbstractMediaDecoder> decoder,std::unique_ptr<PcmPlayDevice> pcmDev,
                         QObject* parent):QObject(parent)
{

    decoder_ = std::move(decoder);
    pcmDev_ = std::move(pcmDev);

    isPlaying_ = false;
    timer_ = new QTimer(this);
    timer_->setTimerType(Qt::PreciseTimer);   // 精准定时设置
    connect(timer_, &QTimer::timeout,this, &MediaPlayer::timerCallback);

}

MediaPlayer::~MediaPlayer()
{
    if(pcmDev_) {
        pcmDev_.release();
    }
}

// 打开文件
QString MediaPlayer::openFile(QString filePath)
{
    std::string retStr = decoder_->openFile(filePath.toStdString());
    if(!retStr.empty()) return QString::fromStdString(retStr);

    // 启动音频设备
    auto audioInfo = decoder_->getAudioInfo();
    if(audioInfo==nullptr) {
        qDebug()<<"媒体文件无音频";
    }
    else {
        pcmDev_->start(audioInfo->sampleRate, audioInfo->channels, QAudioFormat::SampleFormat::Int16);
    }

    return QString::fromStdString(retStr);
}

std::shared_ptr<VideoInfo> MediaPlayer::getVideoInfo()
{
    return decoder_->getVideoInfo();
}

std::shared_ptr<AudioInfo> MediaPlayer::getAudioInfo()
{
    return decoder_->getAudioInfo();
}

// 启动播放, 会定时地发送视频准备好的视频。从当前位置播放
QString MediaPlayer::start()
{
    if(isPlaying_) return "";
    // 检测文件是否打开
    auto videoInfo = decoder_->getVideoInfo();
    if(videoInfo == nullptr) return "未打开文件";
    // 启动定时器
    // int timeForOneFrame= 1000/videoInfo->fps;
    timer_->setInterval(TIMER_PERIOD_SEC*1000);     // 统一5ms定时
    timer_->setTimerType(Qt::PreciseTimer);
    timer_->setSingleShot(false);
    timer_->start();
    isPlaying_ = true;
    firstRun_ = true;
    timer_delay_=-1;
    return "";
}

QString MediaPlayer::pause()
{
    timer_->stop();
    isPlaying_ = false;
    return "";
}

// 停止播放
QString MediaPlayer::stop()
{
    // if(!isPlaying_) return "";
    timer_->stop();
    // 回到视频开始处
    decoder_->seek(0);
    // 获得预览帧
    std::string retStr;
    // getFrames(retStr);
    isPlaying_ = false;
    return "";
}

// 跳到指定的视频位置，如果正在播放则从跳转后的位置继续播放
QString MediaPlayer::seek(double target_src)
{
    timer_->stop();
    isPlaying_ = false;

    if(decoder_->seek(target_src)) {
        std::string retStr;
        return QString::fromStdString(retStr);
    }
    else return "无法定位";


}

QString MediaPlayer::seek(long frame_num)
{
    if(decoder_->getVideoInfo()==nullptr) return "未打开文件";
    auto videoInfo = decoder_->getVideoInfo();
    double start_tm = frame_num*videoInfo->totalTime/videoInfo->totalFrames;
    return seek(start_tm);
}

std::shared_ptr<AbstractVideoFrame> MediaPlayer::get_preview()
{
    return decoder_->firstVideoInQue();
}

bool MediaPlayer::endOfFileCheck()
{
    if(decoder_->isEndOfDecode()) {
        timer_->stop();
        qDebug()<<"end of file.";
        emit endOfFile();
        return true;
    }
    return false;
}

void MediaPlayer::timerCallback(void)
{
    // 因为QTimer在开始的几个timeout中定时是不准确的，因此跳过前面几个timeout。
    timer_delay_++;
    if(timer_delay_<10) return;

    static double t_vs_=0;                   // 播放定时处理用，起始帧的时间
    // 启动高精定时器
    if(firstRun_) {
        elapsedTimer_.start();
        firstRun_=false;
        // 取视频队列中第一个视频帧的起始时间作为t_vs_
        auto videoFrame = decoder_->firstVideoInQue();
        if(videoFrame == nullptr) return;   // 没有视频帧则跳出
        t_vs_ = videoFrame->startTime();
    }
    double t_cur = elapsedTimer_.nsecsElapsed()/1000000000.0 + t_vs_;  // 当前时间
    qDebug()<<"=== cur tm: "<<t_cur<<" ===";

    // ===== 处理视频帧 =====
    int ret;
    std::shared_ptr<AbstractVideoFrame> videoFrame;
    while(true) {
        videoFrame = decoder_->firstVideoInQue();
        if(!videoFrame ) {
            // 队列中没有视频了跳出循环
            qDebug()<<"no video frame";
            break;
        }
        double startTm = videoFrame->startTime();
        // qDebug()<<"video frame: "<<startTm;
        if( startTm < (t_cur-AV_SYNC_ERR/2)) {
            qDebug()<<"discard video frame: "<<startTm;
            decoder_->popVideoQue();        //落后于当前的时间，直接丢弃
            continue;
        }
        else if((t_cur-AV_SYNC_ERR/2)<=startTm && startTm<=(t_cur+AV_SYNC_ERR/2) ) {
            qDebug()<<"disp video frame:"<<startTm;
            emit updateVideoFrame(videoFrame);
            decoder_->popVideoQue();
            break;
        }
        else {
            // videoFrame->startTime() > t_cur+AV_SYNC_ERR的情况，不做处理，等待下一次
            // qDebug()<<"waiting video frame: "<<videoFrame->startTime();
            break;
        }
    }
    // ===== 处理音频 =====
    std::queue<std::shared_ptr<AbstractAudioFrame>> audioQue;
    std::shared_ptr<AbstractAudioFrame> audioFrame;
    while(true) {
        audioFrame = decoder_->firstAudioInQue();
        // 队列中没有音频帧了，跳出循环
        if(!audioFrame) {
            break;
        }
        if(audioFrame->startTime()< (t_cur-AV_SYNC_ERR) ) {
            // qDebug()<<"discard audio frame: "<<audioFrame->startTime();
            decoder_->popAudioQue();        //落后于当前的时间，直接丢弃
            continue;
        }
        else if( (t_cur-AV_SYNC_ERR)<=audioFrame->startTime() && audioFrame->startTime()<= (t_cur+2*AV_SYNC_ERR) ) {
            // qDebug()<<"play audio frame:"<<audioFrame->startTime();
            audioQue.push(audioFrame);
            decoder_->popAudioQue();
        }
        else {
            // audioFrame->startTime() > t_cur+2*AV_SYNC_ERR的情况，不做处理，等待下一次
            break;
        }
    }
    // 播放音频
    // qDebug()<<"QTimer start play sound.";
    while(!audioQue.empty()) {
        audioFrame = audioQue.front(); audioQue.pop();
        // qDebug()<<"=== play audio frame:"<<audioFrame->startTime();
        pcmDev_->writeRaw(audioFrame->data(), audioFrame->sampleBytes());
    }
    if(decoder_->isEndOfDecode()) {
        timer_->stop();
        emit endOfFile();
        qDebug()<<"end of file.";
    }
    // qDebug()<<"QTimer done.";
    // if(t_cur>3.5) timer_->stop(); //test
}


void MediaPlayer::printAudioData(std::shared_ptr<AbstractAudioFrame> frame)
{
    int16_t* data = (int16_t*) frame->data();
    int sampleSize = frame->sampleSize();
    QStringList list;
    for(int i=0; i<sampleSize*2; i++) {
        list<<QString::number(data[i]);
    }
    qDebug()<<list.join(" ");
}
