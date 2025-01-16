
#include <QDebug>

extern "C"{
#include <libavutil/avutil.h>       //avutil:工具库（大部分库都需要这个库的支持）
#include <libswscale/swscale.h>     //swscale:视频像素数据格式转换
#include <libswresample/swresample.h>   // 音频格式转换
#include <libavutil/imgutils.h>
}

struct SwsContext;

#include "ffmpeg_frame.h"
#include "ffmpeg_decoder_inner.h"


/*********************************** FFmpeg获取GPU硬件解码帧格式的回调函数 *****************************************/
static enum AVPixelFormat g_pixelFormat;
/**
 * @brief      回调函数，获取GPU硬件解码帧的格式
 * @param s
 * @param fmt
 * @return
 */
AVPixelFormat get_hw_format(AVCodecContext* s, const enum AVPixelFormat* fmt)
{
    Q_UNUSED(s)
    const enum AVPixelFormat* p;

    for (p = fmt; *p != -1; p++)
    {
        if(*p == g_pixelFormat)
        {
            return *p;
        }
    }

    qDebug() << "无法获取硬件表面格式.";         // 当同时打开太多路视频时，如果超过了GPU的能力，可能会返回找不到解码帧格式
    return AV_PIX_FMT_NONE;
}



FFmpegDecoderInner::FFmpegDecoderInner() {}

FFmpegDecoderInner::~FFmpegDecoderInner()
{
    qDebug()<<"FFmpegDecoderInner::~FFmpegDecoderInner";
    if(decodecCtx_) {
        // 释放AVCodecContext
        avcodec_free_context(&decodecCtx_);
    }
}

// 如果出错返回ffmpeg错误代码
int FFmpegDecoderInner::init(std::shared_ptr<AVFormatContextWrapper> fmtCtx, int streamIdx)
{
    fmtCtx_ = fmtCtx;
    streamIdx_ = streamIdx;

    AVStream* stream = (*fmtCtx)->streams[streamIdx];  // 通过查询到的索引获取视频流
    // 通过解码器ID获取视频解码器（新版本返回值必须使用const）
    const AVCodec* decodec = avcodec_find_decoder(stream->codecpar->codec_id);
    // 获取视频图像分辨率（AVStream中的AVCodecContext在新版本中弃用，改为使用AVCodecParameters）
    type_ = decodec->type;
    if(type_ == AVMEDIA_TYPE_VIDEO) {
        qDebug()<<"Find video index: "<<streamIdx;
        auto videoInfo = std::make_shared<VideoInfo>();
        videoInfo->width = stream->codecpar->width;
        videoInfo->height =  stream->codecpar->height;
        videoInfo->timeBase = av_q2d(stream->time_base);
        videoInfo->fps = stream->avg_frame_rate.den==0?0:
                             ((float)stream->avg_frame_rate.num/stream->avg_frame_rate.den) ;
        if(stream->nb_frames>0)
            videoInfo->totalFrames = stream->nb_frames;
        else {
            videoInfo->totalFrames= stream->duration*videoInfo->timeBase*videoInfo->fps;
        }
        videoInfo->totalTime = (*fmtCtx)->duration /((double)AV_TIME_BASE);

        qDebug() << QString("[视频信息] 分辨率：[w:%1,h:%2] 帧率：%3  总帧数：%4 总时长:%5s TIME_BASE：%6  视频解码器：%7")
                        .arg(videoInfo->width).arg(videoInfo->height).arg(videoInfo->fps).
                    arg(videoInfo->totalFrames).arg(videoInfo->totalTime).arg(videoInfo->timeBase).arg(decodec->name);
        mediaInfo_ = videoInfo;
    }
    else if(type_ == AVMEDIA_TYPE_AUDIO) {
        AVCodecParameters* codecParameters = stream->codecpar;
        qDebug()<<"Find audio index: "<<streamIdx;
        auto audioInfo = std::make_shared<AudioInfo>();
        audioInfo->channels = 2;
        audioInfo->sampleRate = codecParameters->sample_rate;
        audioInfo->frameSize = stream->codecpar->frame_size;
        audioInfo->timeBase = av_q2d(stream->time_base);
        audioInfo->totalTime = (*fmtCtx)->duration /((double)AV_TIME_BASE);
        qDebug()<<QString("[音频信息] 采样率：%1Hz 通道数：%2 音频解码器：%3 TIME_BASE：%4 ").arg(audioInfo->sampleRate)
                        .arg(audioInfo->channels).arg(decodec->name).arg(audioInfo->timeBase);
        mediaInfo_ = audioInfo;
    }

    // 分配AVCodecContext并将其字段设置为默认值。
    decodecCtx_ = avcodec_alloc_context3(decodec);
    if(!decodecCtx_) {
        throw std::runtime_error("avcodec_alloc_context3 err!");
    }
    // 使用视频流的codecpar为解码器上下文赋值
    int ret = avcodec_parameters_to_context(decodecCtx_, stream->codecpar);
    if(ret < 0) return ret;

    if(decodec->type== AVMEDIA_TYPE_VIDEO) {     // 适用于video
        // decodecCtx_->flags2 |= AV_CODEC_FLAG2_FAST;    // 允许不符合规范的加速技巧。
        decodecCtx_->thread_count = 4;                 // 使用4线程解码, 使用8线程时会出现丢帧的情况
        decodecCtx_->thread_type = FF_THREAD_FRAME; //使用帧级多线程

        // 初始化硬件解码器（在avcodec_open2前调用）
        if(useHWDecoder_) {
            ret = initHWDecoder(decodec);
            if(ret<0) return ret;
        }
    }
    // 打开解码器，如果之前avcodec_alloc_context3传入了解码器，这里设置NULL就可以
    ret = avcodec_open2(decodecCtx_, nullptr, nullptr);
    if(ret < 0) return ret;
    return 0;
}


void FFmpegDecoderInner::push_packet(std::shared_ptr<AVPacketWrapper> packPtr)
{
    avcodec_send_packet(decodecCtx_, *packPtr);
}


/*
 * 获得解码帧，由解码线程调用，然后将解码帧放入帧队列
 * 返回值：
 * 0 -> 正确获得数据帧
 * <0 -> ffmpeg错误代码
 */
int FFmpegDecoderInner::getFrame(std::shared_ptr<AbstractFrame>& outFrame)
{
    int ret;
    AVFrame* frame = av_frame_alloc();           //分配结构体
    ret = avcodec_receive_frame(decodecCtx_, frame); //从解码器获取解码帧
    if(ret<0) {
        av_frame_free(&frame);
        outFrame = nullptr;
        return ret;
    }
     // 有效帧，封装成std::shared_ptr<AbstractFrame>
    if(type_ == AVMEDIA_TYPE_VIDEO) {
        auto framePtr = videoFormatConvert(frame);
        if(framePtr==nullptr) outFrame = nullptr;
        else {
            auto videoInfo = std::static_pointer_cast<VideoInfo>(mediaInfo_);
            int T = 1/(videoInfo->fps*videoInfo->timeBase);
            int frame_num = round( (*framePtr)->pts/T);
            outFrame = std::make_shared<VideoFrame>(framePtr, videoInfo->timeBase, frame_num);
        }
    }
    else if(type_ == AVMEDIA_TYPE_AUDIO){
        auto framePtr = audioFormatConvert(frame);
        if(framePtr==nullptr) outFrame = nullptr;
        else {
            auto audioInfo = std::static_pointer_cast<AudioInfo>(mediaInfo_);
            outFrame = std::make_shared<AudioFrame>(framePtr, audioInfo->timeBase);
        }

    }

    // 完成处理后释放动态分配的frame
    av_frame_free(&frame);
    return 0;
}


int FFmpegDecoderInner::initHWDecoder(const AVCodec *codec)
{
    if(!codec) return -1;

    // 获取当前环境支持的硬件解码器
    AVHWDeviceType type = AV_HWDEVICE_TYPE_NONE;      // ffmpeg支持的硬件解码器
    QStringList strTypes;
    while ((type = av_hwdevice_iterate_types(type)) != AV_HWDEVICE_TYPE_NONE)       // 遍历支持的设备类型。
    {
        HWDeviceTypes_.append(type);
        const char* ctype = av_hwdevice_get_type_name(type);  // 获取AVHWDeviceType的字符串名称。
        if(ctype)
            strTypes.append(QString(ctype));
    }

    for(int i = 0; ; i++)
    {
        const AVCodecHWConfig* config = avcodec_get_hw_config(codec, i);    // 检索编解码器支持的硬件配置。
        if(!config) {
            qDebug() << "打开硬件解码器失败！";
            return -1;          // 没有找到支持的硬件配置
        }

        // 判断是否是设备类型
        if (config->methods & AV_CODEC_HW_CONFIG_METHOD_HW_DEVICE_CTX) {
            for(auto i : HWDeviceTypes_) {
                // 判断设备类型是否是支持的硬件解码器
                if(config->device_type == AVHWDeviceType(i)) {
                    g_pixelFormat = config->pix_fmt;
                    // 打开指定类型的设备，并为其创建AVHWDeviceContext。
                    int ret = av_hwdevice_ctx_create(&hw_device_ctx, config->device_type, nullptr, nullptr, 0);
                    if(ret<0) return ret;
                    qDebug() << "打开硬件解码器：" << av_hwdevice_get_type_name(config->device_type);
                    decodecCtx_->hw_device_ctx = av_buffer_ref(hw_device_ctx);  // 创建一个对AVBuffer的新引用。
                    decodecCtx_->get_format = get_hw_format;                    // 由一些解码器调用，以选择将用于输出帧的像素格式
                    return 0;
                }
            }
        }
    }
    return -1;
}

void FFmpegDecoderInner::release()
{
    if(decodecCtx_)
        avcodec_free_context(&decodecCtx_);
}



// 将视频帧转换为YUV420P格式
std::shared_ptr<AVFrameWrapper> FFmpegDecoderInner::videoFormatConvert(AVFrame* srcFrame)
{
    int ret;
    auto frameYUV = std::make_shared<AVFrameWrapper>();
    if(srcFrame->format != AV_PIX_FMT_YUV420P ) {
        // 由于解码后的数据不一定都是yuv420p，因此需要将解码后的数据统一转换成YUV420P
        SwsContext* imgConvertCtx = sws_getContext(srcFrame->width, srcFrame->height,
                                                   (AVPixelFormat)srcFrame->format,
                                                   srcFrame->width, srcFrame->height,
                                                   AV_PIX_FMT_YUV420P, SWS_BILINEAR, NULL, NULL, NULL);

        // 推断buffer的对齐字节数


        // 自动分配存储所需的buffer
        (*frameYUV)->width = srcFrame->width;
        (*frameYUV)->height = srcFrame->height;
        (*frameYUV)->format = AV_PIX_FMT_YUV420P;
        (*frameYUV)->pkt_dts = srcFrame->pkt_dts;
        (*frameYUV)->pts = srcFrame->pts;
        ret = av_frame_get_buffer(*frameYUV, 256);

        if(ret<0) {
            char errBuf[1024];
            av_strerror(ret, errBuf, 1024);
            qDebug()<<errBuf;
            throw std::runtime_error("av_frame_get_buffer err!");
        }
        // 进行格式转换
        ret = sws_scale(imgConvertCtx,
                        (uint8_t const * const *) srcFrame->data,
                        srcFrame->linesize, 0, srcFrame->height, (*frameYUV)->data,
                        (*frameYUV)->linesize);
        sws_freeContext(imgConvertCtx);
        if(ret<0) {
            qDebug()<<"sws_scale err.";
            return nullptr;
        }
    }
    else {
        //格式不需要转换，则将srcFrame中的数据buffer直接转移到frameYUV
        av_frame_move_ref(*frameYUV, srcFrame);
    }
    return frameYUV;
}


// 将音频帧转换为s16格式
std::shared_ptr<AVFrameWrapper> FFmpegDecoderInner::audioFormatConvert(AVFrame* srcFrame)
{
    int ret;
    auto dstFrame = std::make_shared<AVFrameWrapper>();
    av_frame_copy_props(*dstFrame, srcFrame);
    if(srcFrame->format != AV_SAMPLE_FMT_S16) {
        // 将数据格式统一转换为AV_SAMPLE_FMT_S16
        // 输出音频参数
        enum AVSampleFormat out_sample_fmt = AV_SAMPLE_FMT_S16; // 输出格式：16 位整型 PCM
        int out_sample_rate = 44100;                            // 输出采样率：44.1kHz
        AVChannelLayout out_channel_layout = AV_CHANNEL_LAYOUT_STEREO;      // 输出声道布局：立体声
        // 分配帧的缓冲区
        (*dstFrame)->format = out_sample_fmt;
        (*dstFrame)->sample_rate = out_sample_rate;
        (*dstFrame)->ch_layout = out_channel_layout;
        (*dstFrame)->nb_samples = srcFrame->nb_samples;
        if (av_frame_get_buffer(*dstFrame, 0) < 0) {
            qDebug()<<"Failed to allocate frame buffer for audioFrame!";
            return nullptr;
        }
        // 创建重采样上下文
        SwrContext *swr_ctx = swr_alloc();
        ret = swr_alloc_set_opts2(&swr_ctx, &out_channel_layout, out_sample_fmt, out_sample_rate,
                                  &srcFrame->ch_layout, (AVSampleFormat)srcFrame->format,
                                  srcFrame->sample_rate, 0, NULL);
        if(ret<0) {
            swr_free(&swr_ctx);
            return nullptr;
        }
        ret = swr_init(swr_ctx);
        if (ret < 0) {
            swr_free(&swr_ctx);
            return nullptr;
        }
        // 执行重采样
        ret = swr_convert(
            swr_ctx,                      // 重采样上下文
            (*dstFrame)->data,              // 输出数据
            (*dstFrame)->nb_samples,        // 输出样本数
            (const uint8_t **)srcFrame->data, // 输入数据
            srcFrame->nb_samples          // 输入样本数
            );
        swr_free(&swr_ctx);
        if(ret<0) {
            return nullptr;
        }
    }
    else{
        //格式不需要转换，则将srcFrame中的数据buffer直接转移到dstFrame
        av_frame_move_ref(*dstFrame, srcFrame);
    }
    return dstFrame;
}
