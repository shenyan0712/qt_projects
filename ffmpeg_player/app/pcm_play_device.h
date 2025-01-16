#ifndef PCM_PLAY_DEVICE_H
#define PCM_PLAY_DEVICE_H

#include <QObject>
#include <QAudioFormat>
#include <QAudioSink>
#include <QIODevice>


class PcmPlayDevice: public QObject
{
    Q_OBJECT
public:
    PcmPlayDevice(QObject* parent=nullptr);
    ~PcmPlayDevice();
    void playFile(std::string filePath);
    std::string start(int sampleRate, int channels, QAudioFormat::SampleFormat fmt);
    void reset();
    float remainTime();           // 返回buffer中剩余的播放时间
    void writeRaw(uint8_t* buf, int len);
    void writeRaw(QByteArray &bytes);
public slots:
    // void updateAudioFrame(std::shared_ptr<AbstractAudioFrame>);

private:
    QAudioSink* audioSink_=nullptr;
    QIODevice* ioDevice_=nullptr;

    double sampleRate_;
    int channels_;
    int bytesForOneSample_;
};

#endif // PCM_PLAY_DEVICE_H
