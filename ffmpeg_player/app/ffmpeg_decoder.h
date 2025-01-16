#ifndef FFMPEG_DECODER_H
#define FFMPEG_DECODER_H

#include <QList>
#include <memory>
#include <deque>
#include <thread>
#include <condition_variable>
#include <mutex>

extern "C"{
#include <libavutil/avutil.h>       //avutil:工具库（大部分库都需要这个库的支持）
}

struct AVFormatContext;
struct AVCodecContext;
struct AVRational;
struct AVPacket;
struct AVFrame;
struct AVCodec;
struct SwsContext;
struct AVBufferRef;
struct AVCodecParserContext;

#include "abstract_video_decoder.h"
#include "ffmpeg_decoder_inner.h"

#define VIDEO_PACK_QUE_MINSIZE   20
#define AUDIO_PACK_QUE_MINSIZE   10
#define VIDEO_PACK_QUE_MAXSIZE    2000
#define AUDIO_PACK_QUE_MAXSIZE   4000

#define VIDEO_FRAME_QUE_MINSIZE   20
#define AUDIO_FRAME_QUE_MINSIZE   40
#define VIDEO_FRAME_QUE_MAXSIZE    1000
#define AUDIO_FRAME_QUE_MAXSIZE   2000

#define ERROR_LEN 1024  // 异常信息数组长度

#define DEBUG_SHOW_VIDEO_PACK_INFO    0
#define DEBUG_SHOW_AUDIO_PACK_INFO      0

#define DEBUG_SHOW_VIDEO_FRAME_INFO   0
#define DEBUG_SHOW_AUDIO_FRAME_INFO   0


#include "ffmpeg_utils.h"

class FFmpegDecoder: public AbstractMediaDecoder
{
public:
    FFmpegDecoder();
    ~FFmpegDecoder();

    std::string openFile(const std::string fileName);         // 打开视频文件，如果无法打开文件返回错误信息
    void closeFile();
    std::shared_ptr<VideoInfo> getVideoInfo();                // 获得视频文件信息
    std::shared_ptr<AudioInfo> getAudioInfo();
    bool seek(double target_sec);                   // 位置指针跳转到视频指定位置, 通常并不是精确的毫秒位置，而是相近的某一帧。 注意该函数只对视频定位，而不管音频，所以后续需要音视频同步处理
    const std::shared_ptr<AbstractVideoFrame> firstVideoInQue();
    const std::shared_ptr<AbstractAudioFrame> firstAudioInQue();
    void popVideoQue();
    void popAudioQue();
    // bool isEndOfFile();                            // 指示是否到了文件结束
    bool isEndOfDecode();                           // 指示是否解码结束
    const std::string getErrInfo();                 // 如果没有解码错误，返回为空，否则不为空

private:    // functions
    void closeAll();
    void demux_thread_func();                       // 分解音视频的线程函数
    void decode_thread_func();
    void close_thread();
    void pause_thread();

private:    // variables
    // VideoInfo videoInfo_;
    // AudioInfo audioInfo_;
    std::shared_ptr<AVFormatContextWrapper> formatCtxPtr_;
    std::shared_ptr<FFmpegDecoderInner> videoDecoder_;
    std::shared_ptr<FFmpegDecoderInner> audioDecoder_;
    bool endOfFile_=false;
    bool endOfVideoDecode_ = false;
    bool endOfAudioDecode_ = false;


    std::unique_ptr<std::thread> demuxThread_ = nullptr;
    std::unique_ptr<std::thread> decodeThread_ = nullptr;
    std::atomic<bool> threadExitFlag_=false;        // 线程退出标志位
    std::atomic_bool threadPause_ = false;
    std::atomic_bool demuxThrdPuased_ = false;
    std::atomic_bool decodeThrdPaused_ = false;

    std::condition_variable cv_;                    // 线程唤醒条件变量
    std::mutex cvMutex_;                           // 条件变量的互斥操作
    bool needRead_=false;                           // 指示是否需要读取媒体文件

    std::deque<std::shared_ptr<AVPacketWrapper>> videoPackQue_;             // 未解码数据包队列
    std::deque<std::shared_ptr<AVPacketWrapper>> audioPackQue_;             //
    std::mutex packRwMutex_;                                                // 条件变量的互斥操作

    std::deque<std::shared_ptr<AbstractVideoFrame>> videoFrameQue_;
    std::deque<std::shared_ptr<AbstractAudioFrame>> audioFrameQue_;
    std::mutex frameRwMutex_;

    double sync_err_;

    char errBuf_[ERROR_LEN];
};


inline double TB2Second(const long &ts, const double &time_base)
{
    return ts*time_base;
}

inline long Second2TB(const double &t, const double &time_base)
{
    return t/time_base;
}


#endif // FFMPEG_DECODER_H
