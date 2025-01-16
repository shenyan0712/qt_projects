#include "pcm_play_device.h"

#include <QDebug>
#include <QFile>
#include <stdio.h>




PcmPlayDevice::PcmPlayDevice(QObject* parent):QObject(parent)
{
}

PcmPlayDevice::~PcmPlayDevice()
{
}

void PcmPlayDevice::playFile(std::string filePath)
{

    QFile audioFile("d:\\test.pcm");
    if (!audioFile.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open audio file";
        return ;
    }

    QAudioFormat format;
    format.setSampleRate(44100);
    format.setChannelCount(2);
    format.setSampleFormat(QAudioFormat::Int16);

    auto audioSink_=new QAudioSink(format, this);
    auto ioDevice_ = audioSink_->start();


    QByteArray audioData = audioFile.readAll();
    while (!audioData.isEmpty()) {
        qint64 bytesFree = audioSink_->bytesFree();
        if (bytesFree > 0) {
            QByteArray chunk = audioData.left(bytesFree);
            ioDevice_->write(chunk);
            audioData.remove(0, chunk.size());
        }
    }

    audioSink_->stop();
    audioFile.close();
}


std::string PcmPlayDevice::start(int sampleRate, int channels, QAudioFormat::SampleFormat fmt)
{
    if(audioSink_) {
        audioSink_->stop();
        delete audioSink_;
    }

    sampleRate_ = sampleRate;
    channels_ = channels;
    switch(fmt) {
    case QAudioFormat::SampleFormat::UInt8:
        bytesForOneSample_ = 1; break;
    case QAudioFormat::SampleFormat::Int16:
        bytesForOneSample_ = 2; break;
    case QAudioFormat::SampleFormat::Int32:
    case QAudioFormat::SampleFormat::Float:
        bytesForOneSample_ = 4; break;
    default:
        bytesForOneSample_ = 2;
    }

    QAudioFormat format;
    format.setSampleRate(sampleRate);
    format.setChannelCount(channels);
    format.setSampleFormat(fmt);

    audioSink_=new QAudioSink(format, this);
    qDebug()<<"QAudioSink default buffer size:"<< audioSink_->bufferSize();
    // audioSink_->setBufferSize(4096*100);
    ioDevice_ = audioSink_->start();
    audioSink_->setVolume(1);

    return "";
}

void PcmPlayDevice::reset()
{
    if(audioSink_) {
        audioSink_->stop();
        audioSink_->reset();
        ioDevice_ = audioSink_->start();
        ioDevice_->reset();
    }
}

float PcmPlayDevice::remainTime()
{
    if(!audioSink_) return 0;
    double tm = audioSink_->bytesFree()/bytesForOneSample_/sampleRate_;
    return tm;
}

void PcmPlayDevice::writeRaw(uint8_t* buf, int len)
{
    if(audioSink_==nullptr || ioDevice_==nullptr) return;
    //转为QByteArray
    QByteArray byteArray(reinterpret_cast<char*>(buf), len);
    ioDevice_->write(byteArray);
}

void PcmPlayDevice::writeRaw(QByteArray &bytes)
{
    if(audioSink_==nullptr || ioDevice_==nullptr) return;
    ioDevice_->write(bytes);
}

// /*
//  * 接收PCM数据，放入音频设备的缓存进行播放
//  */
// void PcmPlayDevice::updateAudioFrame(std::shared_ptr<AbstractAudioFrame> frame)
// {
//     // qDebug()<<"PcmPlayDevice: recv audio data";
//     //
//     if(audioSink_->bytesFree()> frame->size())
//         writeRaw(frame->data(), frame->size());
// }
