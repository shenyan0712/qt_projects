#ifndef FFMPEG_UTILS_H
#define FFMPEG_UTILS_H

#include <memory>
#include <functional>
#include <iostream>

#include <QDebug>

extern "C"
{
#include <libavcodec/avcodec.h>     //avcodec:编解码(最重要的库)
#include <libavformat/avformat.h>   //avformat:封装格式处理
}

// ================================================================================
// ======================= AVOFormatContext的 C++封装 ==============================
class AVFormatContextWrapper {
public:
    AVFormatContextWrapper() : formatContext_(nullptr) {
    }
    ~AVFormatContextWrapper() {
        closeFile();
    }
    bool openFile(const std::string& filename) {
        if (avformat_open_input(&formatContext_, filename.c_str(), nullptr, nullptr) != 0) {
            formatContext_ = nullptr;
            return false; // Failed to open file
        }
        // 获取流信息
        if (avformat_find_stream_info(formatContext_, nullptr) < 0) {
            closeFile();
            return false; // Failed to retrieve stream info
        }
        return true;
    }
    void closeFile() {
        if (formatContext_) {
            avformat_close_input(&formatContext_);
            formatContext_ = nullptr;
        }
    }
    AVFormatContext* get() const { //get返回nullptr表示未打开文件
        return formatContext_;
    }
    // 重载 -> 运算符
    AVFormatContext* operator->() const {
        return formatContext_;
    }
    operator AVFormatContext*() {
        return formatContext_;
    }
private:
    AVFormatContext* formatContext_=nullptr;
};


// =========================================================================
// ======================= AVPacket的C++封装 ================================
class AVPacketWrapper {
public:
    AVPacketWrapper() {
        packet = av_packet_alloc();
        if (!packet) {
            throw std::runtime_error("Failed to allocate AVPacket");
        }
    }
    ~AVPacketWrapper() {
        if (packet) {
            av_packet_free(&packet);
        }
    }
    // 禁止拷贝构造函数和拷贝赋值操作符
    AVPacketWrapper(const AVPacketWrapper&) = delete;
    AVPacketWrapper& operator=(const AVPacketWrapper&) = delete;
    // 移动构造函数
    AVPacketWrapper(AVPacketWrapper&& other) noexcept : packet(other.packet) {
        other.packet = nullptr;
    }
    // 移动赋值操作符
    AVPacketWrapper& operator=(AVPacketWrapper&& other) noexcept {
        if (this != &other) {
            if (packet) {
                av_packet_free(&packet);
            }
            packet = other.packet;
            other.packet = nullptr;
        }
        return *this;
    }
    // 获取内部的 AVPacket 指针
    AVPacket* get() const {
        return packet;
    }
    // 解引用操作符重载
    AVPacket* operator->() const {
        if(!packet)
            throw std::runtime_error("AVPacket is null");
        return packet;
    }
    operator AVPacket* () {
        return packet;
    }

    // 释放 packet 中的数据
    void unref() {
        av_packet_unref(packet);
    }
private:
    AVPacket* packet=nullptr;
};


// =========================================================================
// ======================= AVPacket的C++封装 ================================
class AVFrameWrapper {
public:
    AVFrameWrapper() {
        frame = av_frame_alloc();
        if (!frame) {
            throw std::runtime_error("Failed to allocate AVPacket");
        }
    }
    ~AVFrameWrapper() {
        if (frame) {
            av_frame_free(&frame);
        }
    }
    // 禁止拷贝构造函数和拷贝赋值操作符
    AVFrameWrapper(const AVFrameWrapper&) = delete;
    AVFrameWrapper& operator=(const AVFrameWrapper&) = delete;
    // 移动构造函数
    AVFrameWrapper(AVFrameWrapper&& other) noexcept : frame(other.frame) {
        other.frame = nullptr;
    }
    // 移动赋值操作符
    AVFrameWrapper& operator=(AVFrameWrapper&& other) noexcept {
        if (this != &other) {
            if (frame) {
                av_frame_free(&frame);
            }
            frame = other.frame;
            other.frame = nullptr;
        }
        return *this;
    }
    // 获取内部的 AVPacket 指针
    AVFrame* get() const {
        return frame;
    }
    // 解引用操作符重载
    AVFrame* operator->() const {
        return frame;
    }
    operator AVFrame* () {
        return frame;
    }

    // 释放 frame 中的数据
    void unref() {
        av_frame_unref(frame);
    }
private:
    AVFrame* frame=nullptr;
};


#endif // FFMPEG_UTILS_H
