#include "mainwindow.h"

#include <QApplication>

extern "C"
{
// #include "libavutil/ffversion.h"
#include <libavcodec/avcodec.h>     //avcodec:编解码(最重要的库)
#include <libavformat/avformat.h>   //avformat:封装格式处理
#include <libswscale/swscale.h>     //swscale:视频像素数据格式转换
#include <libavdevice/avdevice.h>   //avdevice:各种设备的输入输出
#include <libavutil/avutil.h>       //avutil:工具库（大部分库都需要这个库的支持）
}

#include "ffmpeg_decoder.h"
#include "pcm_play_device.h"
#include "video_disp_widget.h"


void play_pcm();
void play_pcm2();
void ffmpeg_test();
void ffmpeg_write_to_imgefile();
int print_frame_time();
void VideoDispWidget_test(QMainWindow&);

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    // 主窗口
    MainWindow w;
    w.show();



    // // 测试avcodec版本
    // qDebug() << "ffmpeg version: "<<FFMPEG_VERSION
    //          <<", avcodec version: "<<avcodec_version();
    // return 0;

    // // 打印各视频帧的时间
    // print_frame_time();
    // return 0;

    // ffmpeg_write_to_imgefile();
    // return 0;

    // // VideoDispWidget的显示测试，通过解码器解码一帧YUV420P格式的数据帧，然后放到该控件上显示
    // QMainWindow win;
    // VideoDispWidget_test(win);

    // ffmpeg_test();

    app.exec();
}

#include <QAudioFormat>
#include <QAudioSink>
#include <QFile>
#include <QThread>

void VideoDispWidget_test(QMainWindow& win)
{
    win.setWindowTitle("test");
    win.setFixedSize(640, 480);
    auto *videoDispWidget= new VideoDispWidget;
    win.setCentralWidget(videoDispWidget);
    win.show();

    std::string retStr;
    FFmpegDecoder decoder;
    retStr = decoder.openFile("D:\\test3.mp4");
    if(!retStr.empty()) {
        qDebug()<<"open file err: "<<retStr;
    }
    decoder.seek(5.0);
    auto videoFrame = decoder.firstVideoInQue();
    if(videoFrame)
        videoDispWidget->updateVideoFrame(videoFrame);
}

void ffmpeg_write_to_imgefile()
{
    std::string retStr;
    FFmpegDecoder decoder;
    retStr = decoder.openFile("D:\\test3.mp4");
    if(!retStr.empty()) {
        qDebug()<<"open file err: "<<retStr;
    }
    decoder.seek(1.0);
    auto videoFrame = decoder.firstVideoInQue();
    auto img = videoFrame->getQImage();
    img->save("d:\\test.jpg");
}


void FFmpegDecoder_test()
{
    // FFmpegDecoder解码测试
    std::string retStr;
    FFmpegDecoder decoder;
    retStr = decoder.openFile("D:\\test1_av1.mp4");
    if(!retStr.empty()) {
        qDebug()<<"open file err: "<<retStr;
    }
    // return 0;
    std::shared_ptr<AbstractVideoFrame> videoFrame;
    std::shared_ptr<AbstractAudioFrame> audioFrame;
    decoder.seek(1.0);
    // int vf=0;
    while(true)
    {
        if(1) {
            videoFrame = decoder.firstVideoInQue();
            if(!videoFrame) {
                if(decoder.isEndOfDecode()) {
                    qDebug()<<"end of file";
                    break;
                }
                // qDebug()<<"no video frame";
                std::this_thread::sleep_for(std::chrono::microseconds(20));
                continue;
            }
            decoder.popVideoQue();
            qDebug()<<"===== get video frame: "<<videoFrame->startTime()<<"s";
            if(videoFrame->startTime()>3.0) break;
            // std::this_thread::sleep_for(std::chrono::microseconds(10));
        }
        else {
            audioFrame = decoder.firstAudioInQue();
            if(!audioFrame) {
                if(decoder.isEndOfDecode())
                    break;
                std::this_thread::sleep_for(std::chrono::microseconds(10));
                continue;
            }
            decoder.popAudioQue();
            qDebug()<<"get audio frame: "<<audioFrame->startTime()<<"s";
        }
    }
}


void MediaPlayer_test()
{
    // MediaPlayer类测试
    MediaPlayer player(std::make_unique<FFmpegDecoder>(), std::make_unique<PcmPlayDevice>());
    auto retStr = player.openFile("D:\\test1_av1.mp4");
    if(!retStr.isEmpty()) {
        qDebug()<<retStr;
        return;
    }
    qDebug()<<"open media file ok.";
    player.seek(1.0);
    player.start();
}

void play_pcm()
{
    QAudioFormat format;
    format.setSampleRate(44100);
    format.setChannelCount(2);
    format.setSampleFormat(QAudioFormat::Int16);

    QAudioSink audioSink(format);
    auto ioDevice = audioSink.start();

    QFile audioFile("d:\\test.pcm");
    if (!audioFile.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open audio file";
        return ;
    }

    QByteArray audioData = audioFile.readAll();
    while (!audioData.isEmpty()) {
        qint64 bytesFree = audioSink.bytesFree();
        if (bytesFree > 0) {
            QByteArray chunk = audioData.left(bytesFree);
            ioDevice->write(chunk);
            audioData.remove(0, chunk.size());
        }
    }

    audioSink.stop();
    audioFile.close();

    return;
}

void play_pcm2()
{
    AudioInfo audioInfo;
    audioInfo.channels = 2;
    audioInfo.sampleRate = 44100;
    // audioInfo.sampleSize

    PcmPlayDevice pcmDevice;
    pcmDevice.start(44100, 2, QAudioFormat::SampleFormat::Int16);

    //读取文件
    QFile audioFile("d:\\test.pcm");
    if (!audioFile.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open audio file";
        return ;
    }

    while(!audioFile.atEnd()) {
        auto bytes = audioFile.read(4096);
        pcmDevice.writeRaw(bytes);
        while(pcmDevice.remainTime()>0)
            QThread::msleep(10);
    }

}

void play_pcm3()
{
    // PcmPlayDevice类测试
    PcmPlayDevice *pcmPlayDev = new PcmPlayDevice();
    pcmPlayDev->playFile("d:\\test.pcm");
    play_pcm2();
    delete pcmPlayDev;
}






extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>     //avcodec:编解码(最重要的库)
}

#include "ffmpeg_utils.h"

void ffmpeg_test()
{

    // 打开输入文件
    AVFormatContextWrapper formatContextWrapper;
    if(formatContextWrapper.openFile("d:\\test.mp4")) {
        qDebug()<<"open file ok.";
    } else {
        qDebug()<<"open file err.";
    }


    // // 获取流信息
    // if (avformat_find_stream_info(formatContextWrapper.get(), nullptr) < 0) {
    //     qDebug() << "无法获取流信息";
    //     return;
    // }
    qDebug()<< "文件打开成功，流信息已获取。";

    formatContextWrapper.closeFile();
    qDebug()<<"done.";
}


int print_frame_time()
{
    const char* input_file = "d:\\test1_h264.mp4";

    // 打开输入文件
    AVFormatContext* format_ctx = nullptr;
    if (avformat_open_input(&format_ctx, input_file, nullptr, nullptr) != 0) {
        std::cerr << "Could not open input file: " << input_file << std::endl;
        return -1;
    }

    // 获取流信息
    if (avformat_find_stream_info(format_ctx, nullptr) < 0) {
        std::cerr << "Could not find stream information." << std::endl;
        avformat_close_input(&format_ctx);
        return -1;
    }

    // 查找视频流
    int video_stream_index = -1;
    for (unsigned int i = 0; i < format_ctx->nb_streams; i++) {
        if (format_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            video_stream_index = i;
            break;
        }
    }

    if (video_stream_index == -1) {
        std::cerr << "Could not find video stream." << std::endl;
        avformat_close_input(&format_ctx);
        return -1;
    }

    // 获取视频流的时间基
    AVStream* video_stream = format_ctx->streams[video_stream_index];
    AVRational time_base = video_stream->time_base;

    // 读取数据包
    AVPacket packet;
    while (av_read_frame(format_ctx, &packet) >= 0) {
        double pts=0;
        if (packet.stream_index == video_stream_index) {
            // 计算帧的显示时间（秒）
            pts = packet.pts * av_q2d(time_base);
            double dts = packet.dts * av_q2d(time_base);
            double duration = packet.duration * av_q2d(time_base);

            std::cout << "Frame PTS: " << pts << "s, DTS: " << dts << "s, Duration: " << duration << "s" << std::endl;
        }

        // 释放数据包
        av_packet_unref(&packet);
        if(packet.stream_index == video_stream_index && pts>1.8) break;
    }

    // 关闭输入文件
    avformat_close_input(&format_ctx);

    std::cout << "Frame time extraction completed." << std::endl;
    return 0;
}
