#ifndef FFMPEG_FRAME_H
#define FFMPEG_FRAME_H

// #include <string>
#include "abstract_video_decoder.h"
#include "ffmpeg_utils.h"
// #include <memory>

struct AVFrame;
struct AVPacket;

/*
 * 用于保存视频帧, 统一使用YUV420p格式
 * 提供的信息：
 *      1，数据buffer的指针，
 *      2，Y分量偏移，U分量偏移，V分量偏移
 *      3，图像的尺寸
 */
class VideoFrame: public AbstractVideoFrame
{
public:
    VideoFrame(std::shared_ptr<AVFrameWrapper> frame, double time_base,int frame_num);
    ~VideoFrame();      // 在里面释放AVFrame
    double startTime();
    int frame_num();
    uint8_t* y_buf();
    uint8_t* u_buf();
    uint8_t* v_buf();
    int width();
    int height();
    /*
     * 某些解码器解码的图像为了字节对齐，每行的实际数据宽度可能大于width。
     * 例如width=854, 而line_size=896，实际的数据宽度为64个像素对齐
     * 所以在显示处理时需要将linesize考虑进来, 例如OpenGL中忽略掉多余的填补像素
     */
    int line_size();
    std::shared_ptr<QImage> getQImage();

private:
    std::shared_ptr<AVFrameWrapper> frame_;
    double time_base_;
    int frame_num_;
};

/*
 * 用于保存视频帧, 统一使用S16LE格式
 * 提供的信息：
 *      1，数据buffer的指针，
 *      2，Y分量偏移，U分量偏移，V分量偏移
 *      3，图像的尺寸
 */
class AudioFrame: public AbstractAudioFrame
{
public:
    AudioFrame(std::shared_ptr<AVFrameWrapper> frame, double time_base);
    ~AudioFrame();
    double startTime();              // 该帧的起始时间
    uint8_t* data();            // 通道数量
    int sampleSize();           // 采样值数量
    int sampleBytes();          // 采样值字节数
private:
    std::shared_ptr<AVFrameWrapper> frame_;
    double time_base_;
};


/*
 * 未解码的视频包
 */
class VideoPack
{
    VideoPack(AVPacket* pack)
    {

    }
    ~VideoPack() {

    }

private:
    AVPacket *pack;
};

class AudioPack
{


};




#endif // FFMPEG_FRAME_H
