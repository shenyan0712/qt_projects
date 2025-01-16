#include "ffmpeg_decoder.h"

#include <QDebug>
#include <QTime>
#include <QStringList>

extern "C"
{
// #include "libavutil/ffversion.h"
#include <libavcodec/avcodec.h>     //avcodec:编解码(最重要的库)
#include <libavformat/avformat.h>   //avformat:封装格式处理
#include <libswscale/swscale.h>     //swscale:视频像素数据格式转换
#include <libswresample/swresample.h>   // 音频格式转换
#include <libavdevice/avdevice.h>   //avdevice:各种设备的输入输出
#include <libavutil/avutil.h>       //avutil:工具库（大部分库都需要这个库的支持）
#include <libavutil/imgutils.h>
#include <libavutil/avstring.h>
#include <libavutil/opt.h>
#include <libavfilter/avfilter.h>
#include <libavutil/channel_layout.h>

}

#include "ffmpeg_frame.h"
#include "ffmpeg_utils.h"



FFmpegDecoder::FFmpegDecoder()
{

}

FFmpegDecoder::~FFmpegDecoder()
{
    closeAll();
    qDebug()<<"FFmpegDecoder::~FFmpegDecoder";
}


// 打开视频文件，如果无法打开文件返回错误信息
std::string FFmpegDecoder::openFile(const std::string fileName)
{
    int ret;
    std::string retStr;
    // 如果之前打开了文件，可能demux线程还在运行，先关闭之
    close_thread();

    formatCtxPtr_ = std::make_shared<AVFormatContextWrapper>();
    // 打开文件
    if(not formatCtxPtr_->openFile(fileName)) {
        return "can't open file.";
    }
    // videoInfo_.totalTime = (*formatCtxPtr_)->duration /((double)AV_TIME_BASE);
    // qDebug() << QString("Total video time：%1 s，[%2]").arg(videoInfo_.totalTime)
    //                 .arg(QTime::fromMSecsSinceStartOfDay(int(videoInfo_.totalTime*1000)).toString("HH:mm:ss zzz"));


    // 如果有视频，创建视频解码类
    int streamIdx = av_find_best_stream(*formatCtxPtr_, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
    if(streamIdx < 0)    {
        av_strerror(streamIdx, errBuf_, ERROR_LEN);
    } else {
        videoDecoder_=std::make_shared<FFmpegDecoderInner>();
        ret = videoDecoder_->init(formatCtxPtr_, streamIdx);
        if(ret<0) {
            av_strerror(streamIdx, errBuf_, ERROR_LEN);
            videoDecoder_ = nullptr;    // 没有视频，但可能还有音频
        }
    }
    // 如果有音频，创建音频解码类
    streamIdx = av_find_best_stream(*formatCtxPtr_, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);
    if(streamIdx < 0)    {
        av_strerror(streamIdx, errBuf_, ERROR_LEN);
    } else {
        audioDecoder_ = std::make_shared<FFmpegDecoderInner>();
        ret = audioDecoder_->init(formatCtxPtr_, streamIdx);
        if(ret<0) {
            audioDecoder_ = nullptr;    // 没有视频，但可能还有音频
        }
    }
    //如果音视频都没有，则返回错误信息
    if(!audioDecoder_ && !videoDecoder_) {
        return errBuf_;
    }

    // 开始demux线程
    threadExitFlag_ = false;
    demuxThread_= std::make_unique<std::thread>(&FFmpegDecoder::demux_thread_func, this);
    decodeThread_= std::make_unique<std::thread>(&FFmpegDecoder::decode_thread_func, this);

    return "";
}


void FFmpegDecoder::closeFile()
{
    closeAll();
}

void FFmpegDecoder::closeAll()
{
    close_thread();   // 先关闭读写线程
    formatCtxPtr_->closeFile();
    audioDecoder_ = nullptr;
    videoDecoder_ = nullptr;
}

// 获得视频文件信息
std::shared_ptr<VideoInfo> FFmpegDecoder::getVideoInfo()
{
    if(videoDecoder_) {
        auto vidoInfo = std::static_pointer_cast<VideoInfo>(videoDecoder_->getMediaInfo());
        return vidoInfo;
    }
    return nullptr;
}

std::shared_ptr<AudioInfo> FFmpegDecoder::getAudioInfo()
{
    if(audioDecoder_) {
        auto audioInfo = std::static_pointer_cast<AudioInfo>(audioDecoder_->getMediaInfo());
        return audioInfo;
    }
    return nullptr;
}


bool FFmpegDecoder::seek(double target_sec)
{
    // 先将两个处理线程暂停
    pause_thread();

    qDebug()<<"thread paused.";

    //清除队列及相关标志
    videoFrameQue_.clear();
    videoPackQue_.clear();
    audioFrameQue_.clear();
    audioPackQue_.clear();

    endOfAudioDecode_ = false;
    endOfVideoDecode_ = false;
    if(videoDecoder_) videoDecoder_->flush_buffer();
    if(audioDecoder_) audioDecoder_->flush_buffer();

    // 转换为pts
    double T;
    long target_pts;
    if(videoDecoder_) {
        auto videoInfo = std::static_pointer_cast<VideoInfo>(videoDecoder_->getMediaInfo());
        T = 1/videoInfo->fps;
        target_pts = Second2TB(target_sec, videoInfo->timeBase);
    }
    else if(audioDecoder_) {
        auto audioInfo = std::static_pointer_cast<AudioInfo>(audioDecoder_->getMediaInfo());
        T = audioInfo->frameSize/audioInfo->sampleRate;
        target_pts = Second2TB(target_sec, audioInfo->timeBase);
    }
    else return false; //音视频都没有

    qDebug()<<"Seek frame: "<<target_sec<<" s, dts: "<<target_pts;
    // 寻找关键帧
    int ret = av_seek_frame(*formatCtxPtr_, videoDecoder_->streamIdx(), target_pts, AVSEEK_FLAG_BACKWARD);
    if(ret<0) {
        av_strerror(ret, errBuf_, ERROR_LEN);
        qDebug()<<"av_seek_frame, err: "<<errBuf_;
        return false;
    }
    // 恢复线程，从定位到的关键帧开始读取
    threadPause_ = false;

    bool videoFrameFound=false;
    bool audioFrameFound=false;
    if(!videoDecoder_) videoFrameFound = true; // 如果没有视频，则不对其进行定位
    if(!audioDecoder_) audioFrameFound = true;
    while(true) {
        // 检测音视频队列队首帧是否与定位的时间相符，不符合则丢弃
        // 先视频帧
        if(ret<0) break;
        if(!videoFrameQue_.empty() && !videoFrameFound) {
            auto& videoFrame =videoFrameQue_.front();
            double startTm = videoFrame->startTime();
            // qDebug()<<"seeking, video frame:"<<startTm;
            if(abs(startTm-target_sec)<=T/2
                || startTm> target_sec ) { //
                videoFrameFound = true;
            } else {
                std::lock_guard<std::mutex> lock(frameRwMutex_);
                videoFrameQue_.pop_front();
            }
        }
        if(!audioFrameQue_.empty() && !audioFrameFound ) {
            auto& audioFrame = audioFrameQue_.front();
            double startTm = audioFrame->startTime();
            // qDebug()<<"seeking, audio frame:"<<startTm;
            if(abs(startTm-target_sec)<=T/2
                || startTm>target_sec) {
                audioFrameFound = true;
            } else {
                std::lock_guard<std::mutex> lock(frameRwMutex_);
                audioFrameQue_.pop_front();
            }
        }
        // 如果找到对应的帧，或者解码已结束，则跳出
        if((videoFrameFound && audioFrameFound)
            || (isEndOfDecode() && audioFrameQue_.empty() && videoFrameQue_.empty() ))
            break;
    }
    return true;
}

const std::shared_ptr<AbstractVideoFrame> FFmpegDecoder::firstVideoInQue()
{
    if(videoFrameQue_.empty())  return nullptr;
    else return videoFrameQue_.front();
}

const std::shared_ptr<AbstractAudioFrame> FFmpegDecoder::firstAudioInQue()
{
    if(audioFrameQue_.empty())  return nullptr;
    else return audioFrameQue_.front();
}

void FFmpegDecoder::popVideoQue()
{
    std::lock_guard<std::mutex> lock(frameRwMutex_);
    videoFrameQue_.pop_front();
}

void FFmpegDecoder::popAudioQue()
{
    std::lock_guard<std::mutex> lock(frameRwMutex_);
    audioFrameQue_.pop_front();
}


// 指示是否解码结束
bool FFmpegDecoder::isEndOfDecode()
{
    bool endOfDecode = false;
    if(videoDecoder_) {
        return videoFrameQue_.empty() && endOfVideoDecode_;
    }
    else if(audioDecoder_) {
        return audioFrameQue_.empty() && endOfAudioDecode_;
    }
    return true;
}


// 如果没有解码错误，返回为空，否则不为空
const std::string FFmpegDecoder::getErrInfo()
{
    return "";
}



// =================================================================================
// =================================================================================

void FFmpegDecoder::close_thread()
{
    if(demuxThread_) {
        threadExitFlag_ = true;
        demuxThread_->join();
        decodeThread_->join();
    }
}

void FFmpegDecoder::pause_thread()
{
    threadPause_ = true;
    while(!demuxThrdPuased_ || !decodeThrdPaused_)
        std::this_thread::sleep_for(std::chrono::microseconds(20));
}

void FFmpegDecoder::demux_thread_func()
{
    int videoPackCnt=0;
    int audioPackCnt=0;
    // AVPacket *pack=av_packet_alloc();

    while(!threadExitFlag_) {
        if(threadPause_) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            demuxThrdPuased_ = true;
            continue;
        }
        demuxThrdPuased_ = false;

        // 如果音视频的packet队列中数量都满足最小数据量，则等待10ms
        bool videoNeedWait = false;
        bool audioNeedWait = false;
        if(videoDecoder_ ) videoNeedWait =videoPackQue_.size()>=VIDEO_PACK_QUE_MINSIZE? true: false;
        if(audioDecoder_ ) audioNeedWait =audioPackQue_.size()>=AUDIO_PACK_QUE_MINSIZE ? true: false;
        if((videoNeedWait && audioNeedWait) ) {
            // qDebug()<<"demux_thread_func waiting..."
            //          <<videoPackQue_.size()<<","<<audioPackQue_.size()
            //          <<videoFrameQue_.size()<<","<<audioFrameQue_.size() ;
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
        }
        //读取文件中的packet
        auto packetPtr=std::make_shared<AVPacketWrapper>();
        int ret = av_read_frame(*formatCtxPtr_, *packetPtr);
        if(ret<0) {
            //虽然读取错误, 但解码器中可能还有未解码的数据
            // 因此发送空白帧，以便继续让解码器解码
            {
                // AVERROR(EAGAIN), AVERROR_INVALIDDATA and other
                // qDebug()<<"***** push empty packet *****";
                std::lock_guard<std::mutex> lock(packRwMutex_);
                if(videoDecoder_){
                    if(videoPackQue_.size()>=VIDEO_PACK_QUE_MAXSIZE)
                        videoPackQue_.pop_back();
                    videoPackQue_.push_back(std::make_shared<AVPacketWrapper>());
                }
                if(audioDecoder_) {
                    if(audioPackQue_.size()>=AUDIO_PACK_QUE_MAXSIZE)
                        audioPackQue_.pop_back();
                    audioPackQue_.push_back(std::make_shared<AVPacketWrapper>());
                }
            }
            if(ret == AVERROR_EOF) {
                // qDebug()<<"demux_thread_func, end of file.";
                endOfFile_ = true;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        else {
            std::lock_guard<std::mutex> lock(packRwMutex_);
                // =========== 视频packet ===========
            if( (*packetPtr)->stream_index == videoDecoder_->streamIdx()) {
                videoPackCnt++;
                // qDebug()<<"get video packet, num "<<videoPackCnt;
                if(videoPackQue_.size()>=VIDEO_PACK_QUE_MAXSIZE)
                    videoPackQue_.pop_front();
                if(DEBUG_SHOW_VIDEO_PACK_INFO && false) {
                    auto videoInfo = std::static_pointer_cast<VideoInfo>(videoDecoder_->getMediaInfo());
                    qDebug()<<"push video packet: "<< TB2Second((*packetPtr)->pts, videoInfo->timeBase);
                }
                videoPackQue_.push_back(packetPtr);
            }
            else if( (*packetPtr)->stream_index == audioDecoder_->streamIdx()) {
                // ======== 音频packet ==========
                audioPackCnt++;
                // qDebug()<<"get audio packet, num "<<audioPackCnt;
                if(audioPackQue_.size()>=AUDIO_PACK_QUE_MAXSIZE)
                    audioPackQue_.pop_front();
                if(DEBUG_SHOW_AUDIO_PACK_INFO) {
                    auto audioInfo = std::static_pointer_cast<AudioInfo>(audioDecoder_->getMediaInfo());
                    qDebug()<<"push audio packet: "<< TB2Second((*packetPtr)->pts, audioInfo->timeBase);
                }
                audioPackQue_.push_back(packetPtr);
            }
        }
    }
    qDebug()<<"demux_thread_func exit.";
}

/*
 * 解码线程，只要音视频packet队列中有数据就进行解码，直到
 * 音视频frame队列中的解码帧都满足指定数量，则等待
 */
void FFmpegDecoder::decode_thread_func()
{
    while(!threadExitFlag_) {
        if(threadPause_) {
            decodeThrdPaused_ = true;
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
        }
        decodeThrdPaused_ = false;

        // 音视频frame队列中的解码帧都满足指定数量，或者音视频的pack队列都为空时，则等待
        bool videoNeedWait = true;
        bool audioNeedWait = true;
        if(videoDecoder_ ) videoNeedWait =(videoFrameQue_.size()>=VIDEO_FRAME_QUE_MINSIZE || videoPackQue_.empty())? true: false;
        if(audioDecoder_ ) audioNeedWait =(audioFrameQue_.size()>=AUDIO_FRAME_QUE_MINSIZE || audioPackQue_.empty())? true: false;

        if((videoNeedWait && audioNeedWait) ) {
            // qDebug()<<"decode_thread_func waiting...  "
            //          <<videoPackQue_.size()<<","<<audioPackQue_.size()
            //          <<videoFrameQue_.size()<<","<<audioFrameQue_.size() ;
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
        }
        // =======================================================================
        // 如果视频frame队列未满，则尝试解码一个
        if(videoDecoder_ && videoFrameQue_.size()<VIDEO_FRAME_QUE_MINSIZE) {
            // 先提取一个packet, 放入解码器
            std::shared_ptr<AVPacketWrapper> packPtr;
            { // 加锁操作区域
                std::lock_guard<std::mutex> lock(packRwMutex_);
                if(videoPackQue_.empty()) packPtr = nullptr;
                else {
                    packPtr = videoPackQue_.front();
                    videoPackQue_.pop_front();
                }
            }
            if(packPtr) {
                // if(DEBUG_SHOW_VIDEO_PACK_INFO) {
                auto videoInfo = std::static_pointer_cast<VideoInfo>(videoDecoder_->getMediaInfo());
                // qDebug()<<"pull video packet:"<<TB2Second((*packPtr)->pts, videoInfo->timeBase);
                // }
                videoDecoder_->push_packet(packPtr);
            }
            //取解码视频帧
            std::shared_ptr<AbstractFrame> frame;
            int ret;
            ret = videoDecoder_->getFrame(frame);
            // test, 模拟解码耗时
            // std::this_thread::sleep_for(std::chrono::milliseconds(5));
            if(ret<0) {    //解码结束
                if(ret==AVERROR_EOF) {
                    endOfVideoDecode_ = true;
                }
                else if(ret == AVERROR_INVALIDDATA) {
                    // qDebug()<<"##### AVERROR_INVALIDDATA #####";
                    // std::this_thread::sleep_for(std::chrono::milliseconds(10));
                    continue;
                }
                else if(ret == AVERROR(EAGAIN)) {
                    // qDebug()<<"##### AVERROR(EAGAIN) #####";
                    // std::this_thread::sleep_for(std::chrono::milliseconds(10));
                    continue;
                }
                else {
                    // other case, do nothing
                    qDebug()<<"##### get wrong frame, err code:"<<ret;
                }
            }
            else if(ret == 0) {  // 有视频帧，将其加入视频frame队列
                std::lock_guard<std::mutex> lock(frameRwMutex_); //写入加锁
                auto videoFrame = std::static_pointer_cast<AbstractVideoFrame>(frame);
                // qDebug()<<"push video frame to que, tm:"<<videoFrame->startTime();
                videoFrameQue_.push_back(videoFrame);

            } else {
                // do nothing.
            }
        }
        // =======================================================================
        // 如果音频frame队列未满，则尝试解码一个
        if(audioDecoder_ && audioFrameQue_.size()<AUDIO_FRAME_QUE_MINSIZE) {
            // 先提取一个packet, 放入解码器
            std::shared_ptr<AVPacketWrapper> packPtr;
            {// 加锁操作区域
                std::lock_guard<std::mutex> lock(packRwMutex_);
                if(audioPackQue_.empty()) packPtr = nullptr;
                else {
                    packPtr = audioPackQue_.front(); audioPackQue_.pop_front();
                }
            }
            if(packPtr) {
                audioDecoder_->push_packet(packPtr);
                //取解码视频帧
                std::shared_ptr<AbstractFrame> frame;
                int ret = audioDecoder_->getFrame(frame);
                // test, 模拟解码耗时
                // std::this_thread::sleep_for(std::chrono::milliseconds(2));
                if(ret<0) {
                    if(ret == AVERROR_EOF)  //解码结束
                        endOfAudioDecode_ = true;
                    // otehr case, do nothing.
                }
                else if(ret == 0) {  // 有视频帧，将其加入视频frame队列
                    std::lock_guard<std::mutex> lock(frameRwMutex_); //写入加锁
                    audioFrameQue_.push_back(std::static_pointer_cast<AbstractAudioFrame>(frame));
                } else {
                    // do nothing.
                }
            }
        }

    }
    qDebug()<<"decode_thread_func exit.";
}
