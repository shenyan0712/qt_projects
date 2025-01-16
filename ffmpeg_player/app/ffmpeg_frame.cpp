#include "ffmpeg_decoder.h"
#include "ffmpeg_frame.h"

#include <QDebug>

extern "C"
{
// #include "libavutil/ffversion.h"
#include <libavcodec/avcodec.h>     //avcodec:编解码(最重要的库)
#include <libavformat/avformat.h>   //avformat:封装格式处理
#include <libswscale/swscale.h>     //swscale:视频像素数据格式转换
#include <libavdevice/avdevice.h>   //avdevice:各种设备的输入输出
#include <libavutil/avutil.h>       //avutil:工具库（大部分库都需要这个库的支持）
#include <libavutil/imgutils.h>
}


/*
 * AVFrame中的视频帧格式必须为YUV420p
 *
 */

VideoFrame::VideoFrame(std::shared_ptr<AVFrameWrapper> frame, double time_base, int frame_num)
{
    // frame_ = av_frame_alloc();
    // av_frame_ref(frame_, frame);
    frame_ = frame;
    time_base_ = time_base;
    frame_num_ = frame_num;
}

VideoFrame::~VideoFrame()
{
}

double VideoFrame::startTime()
{
    return TB2Second( (*frame_)->pts, time_base_);
}

int VideoFrame::frame_num()        // 当前帧的编号
{
    return frame_num_;
}


uint8_t* VideoFrame::y_buf()
{
    return (*frame_)->data[0];
}

uint8_t* VideoFrame::u_buf()
{
    return (*frame_)->data[1];

}

uint8_t* VideoFrame::v_buf()
{
    return (*frame_)->data[2];
}

int VideoFrame::width()
{
    return (*frame_)->width;
}

int VideoFrame::height()
{
    return (*frame_)->height;
}

int VideoFrame::line_size()
{
    return (*frame_)->linesize[0];
}

std::shared_ptr<QImage> VideoFrame::getQImage()
{

    // 将视频帧转为RGB888
    SwsContext *img_convert_ctx;
    img_convert_ctx = sws_getContext(
        (*frame_)->linesize[0], (*frame_)->height,
        AV_PIX_FMT_YUV420P,
        (*frame_)->linesize[0], (*frame_)->height, AV_PIX_FMT_RGB24,
        SWS_BICUBIC, NULL, NULL, NULL);

    // rgb帧动态分配
    AVFrame *rgb_frame = av_frame_alloc();
    if (!rgb_frame) {
        fprintf(stderr, "Could not allocate destination frame\n");
        return nullptr;
    }
    rgb_frame->width = (*frame_)->width;
    rgb_frame->height = (*frame_)->height;
    rgb_frame->format = AV_PIX_FMT_RGB24;
    av_frame_get_buffer(rgb_frame, 0);

    // YUV420P转换为RGB
    sws_scale(img_convert_ctx, (*frame_)->data, (const int*)(*frame_)->linesize,
              0, (*frame_)->height, rgb_frame->data, rgb_frame->linesize);

    // RGB存入QImage
    auto img =  std::make_shared<QImage>((*frame_)->width, (*frame_)->height, QImage::Format_RGB888);
    // 检查是否有填充字节
    if (rgb_frame->linesize[0] == rgb_frame->width * 3) {
        // 如果没有填充字节，直接拷贝整个数据
        memcpy(img->bits(), rgb_frame->data[0], rgb_frame->linesize[0] * rgb_frame->height);
    } else {
        // 如果有填充字节，逐行拷贝数据
        for (int y = 0; y < rgb_frame->height; y++) {
            memcpy(img->scanLine(y), rgb_frame->data[0] + y * rgb_frame->linesize[0], rgb_frame->width * 3);
        }
    }

    // 释放资源
    av_frame_free(&rgb_frame);
    sws_freeContext(img_convert_ctx);
    return img;
}



// std::shared_ptr<QImage> VideoFrame::getQImage()
// {
//     SwsContext *img_convert_ctx;
//     img_convert_ctx = sws_getContext(
//         frame_->width, frame_->height,
//         AV_PIX_FMT_YUV420P,
//         frame_->width, frame_->height, AV_PIX_FMT_RGB24,
//         SWS_BICUBIC, NULL, NULL, NULL);

//     auto img =  std::make_shared<QImage>(frame_->width, frame_->height, QImage::Format_RGB888);
//     uint8_t* dst[] = { img->bits() };
//     int dstStride[4];
//     // AV_PIX_FMT_RGB24对应QImage::Format_RGB888
//     av_image_fill_linesizes(dstStride, AV_PIX_FMT_RGB24, frame_->width);
//     // 转换
//     sws_scale(img_convert_ctx, frame_->data, (const int*)frame_->linesize,
//               0, frame_->height, dst, dstStride);

//     // 释放SwsContext
//     sws_freeContext(img_convert_ctx);
//     return img;
// }


AudioFrame::AudioFrame(std::shared_ptr<AVFrameWrapper> frame, double time_base)
{
    frame_ = frame;
    time_base_ = time_base;
}

AudioFrame::~AudioFrame()
{
}

//该帧的起始时间
double AudioFrame::startTime()
{
    return TB2Second((*frame_)->pts, time_base_);
}

// 通道数量
uint8_t* AudioFrame::data()
{
    return (*frame_)->data[0];
}

 // 数据长度
int AudioFrame::sampleSize()
{
    return (*frame_)->nb_samples;
}

 // 采样值字节数
int AudioFrame::sampleBytes()
{
    return (*frame_)->nb_samples*(*frame_)->ch_layout.nb_channels*sizeof(int16_t);

}
