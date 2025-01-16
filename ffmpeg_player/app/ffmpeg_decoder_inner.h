#ifndef FFMPEG_DECODER_INNER_H
#define FFMPEG_DECODER_INNER_H

#include <string>
#include <deque>
#include <mutex>

extern "C"{
#include <libavutil/avutil.h>       //avutil:工具库（大部分库都需要这个库的支持）
#include <libavformat/avformat.h>   //avformat:封装格式处理
}

#include "abstract_video_decoder.h"
#include "ffmpeg_utils.h"

#define VIDEO_PACKET_QUE_MAXSIZE    1024
#define AUDIO_PACKET_QUE_MAXSIZE   2048

#define VIDEO_ea


class FFmpegDecoderInner
{
public:
    FFmpegDecoderInner();
    ~FFmpegDecoderInner();
    int init(std::shared_ptr<AVFormatContextWrapper> fmtCtx, int streamIdx);
    void push_packet(std::shared_ptr<AVPacketWrapper>);
    int streamIdx() { return streamIdx_; }
    int getFrame(std::shared_ptr<AbstractFrame>&);
    std::shared_ptr<MediaInfo> getMediaInfo() { return mediaInfo_; }
    void flush_buffer() { avcodec_flush_buffers(decodecCtx_);  }
private:
    int initHWDecoder(const AVCodec *codec);
    void release();
    std::shared_ptr<AVFrameWrapper> videoFormatConvert(AVFrame* srcFrame);
    std::shared_ptr<AVFrameWrapper> audioFormatConvert(AVFrame* srcFrame);

private:
    std::shared_ptr<AVFormatContextWrapper> fmtCtx_;
    int streamIdx_=-1;
    AVMediaType type_;
    std::shared_ptr<MediaInfo> mediaInfo_;

    AVCodecContext*  decodecCtx_ = nullptr;

    QList<int> HWDeviceTypes_;                          // 保存当前环境支持的硬件解码器
    bool useHWDecoder_ = false;
    AVBufferRef* hw_device_ctx = nullptr;               // 对数据缓冲区的引用
};

#endif // FFMPEG_DECODER_INNER_H

