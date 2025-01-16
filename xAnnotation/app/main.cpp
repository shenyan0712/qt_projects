#include "mainwindow.h"

#include <QApplication>

extern "C"
{
#include "libavutil/ffversion.h"
#include <libavcodec/avcodec.h>     //avcodec:编解码(最重要的库)
#include <libavformat/avformat.h>   //avformat:封装格式处理
#include <libswscale/swscale.h>     //swscale:视频像素数据格式转换
#include <libavdevice/avdevice.h>   //avdevice:各种设备的输入输出
#include <libavutil/avutil.h>       //avutil:工具库（大部分库都需要这个库的支持）
}


int main(int argc, char *argv[])
{
    // // 测试avcodec版本
    // qDebug() << "ffmpeg version: "<<FFMPEG_VERSION
    //          <<", avcodec version: "<<avcodec_version();
    // return 0;

    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();

}
