#ifndef ABSTRACT_VIDEO_DECODER_H
#define ABSTRACT_VIDEO_DECODER_H

#include <string>
#include <memory>
#include <QPixmap>

struct MediaInfo
{
};

struct VideoInfo:public MediaInfo
{
    float fps;
    double totalTime;       //总时间, second
    long totalFrames;
    int width;
    int height;
    double timeBase;
};

struct AudioInfo: public MediaInfo
{
    double sampleRate;     //采样率， Hz            //采用浮点防止整数除法出现的舍出错误
    int channels;       // 通道数
    int frameSize;      // 每一帧采样数据尺寸
    double totalTime;       //总时间, second
    double timeBase;
};

class AbstractFrame
{
public:
    enum FrameType{
        VIDEO_FRAME,
        AUDIO_FRAME
    };

    virtual FrameType type()=0;
};

/*
 * 视频帧统一提供YUV420p格式的数据，具体由不同解码库来实现
 */
class AbstractVideoFrame:public AbstractFrame
{
public:
    FrameType type() { return AbstractFrame::VIDEO_FRAME; }
    virtual double startTime()=0;        //该帧的起始时间
    virtual int frame_num()=0;        // 当前帧的编号
    virtual uint8_t* y_buf()=0;       // Y分量数据
    virtual uint8_t* u_buf()=0;       // U分量数据
    virtual uint8_t* v_buf()=0;       // V分量数据
    virtual int width()=0;
    virtual int height()=0;
    /*
     * 某些解码器解码的图像为了字节对齐，每行的实际数据宽度可能大于width。
     * 例如width=854, 而line_size=896，实际的数据宽度为64个像素对齐
     * 所以在显示处理时需要将linesize考虑进来, 例如OpenGL中忽略掉多余的填补像素
     * 这里line_size为Y分量的行宽，UV的行宽为Y的一半。
     */
    virtual int line_size()=0;
    virtual std::shared_ptr<QImage> getQImage() { return nullptr; }  // 将当前帧转为QPixmap格式
};

class AbstractAudioFrame:public AbstractFrame
{
public:
    FrameType type() { return AbstractFrame::AUDIO_FRAME; }
    virtual double startTime()=0;        //该帧的起始时间
    virtual uint8_t* data()=0;               // 通道数量
    virtual int sampleSize()=0;              // 数据长度
    virtual int sampleBytes()=0;          // 采样值字节数
};


/*
 * 音视频解码器抽象类，提供统一的接口供媒体播放器使用
 */
class AbstractMediaDecoder
{
public:
    virtual std::string openFile(const std::string fileName)=0;         // 打开视频文件，如果无法打开文件返回错误信息
    virtual void closeFile()=0;                                 // 关闭文件
    virtual std::shared_ptr<VideoInfo> getVideoInfo()=0;                // 获得视频文件信息
    virtual std::shared_ptr<AudioInfo> getAudioInfo()=0;
    virtual bool seek(double target_sec)=0;                   // 位置指针跳转到视频指定位置, 通常并不是精确的毫秒位置，而是相近的某一帧
    virtual const std::shared_ptr<AbstractVideoFrame> firstVideoInQue()=0;
    virtual const std::shared_ptr<AbstractAudioFrame> firstAudioInQue()=0;
    virtual void popVideoQue()=0;
    virtual void popAudioQue()=0;
    // virtual bool isEndOfFile()=0;                            // 指示是否到了视频结束
    virtual bool isEndOfDecode()=0;                             // 指示是否视频解码结束
    virtual const std::string getErrInfo()=0;                 // 如果没有解码错误，返回为空，否则不为空
};


#endif // ABSTRACT_VIDEO_DECODER_H
