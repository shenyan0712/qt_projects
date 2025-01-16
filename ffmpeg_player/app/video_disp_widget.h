#ifndef VIDEO_DISP_WIDGET_H
#define VIDEO_DISP_WIDGET_H

#include <QException>
#include <QOpenGLWidget>
#include <QOpenGLShaderProgram>
#include <QOpenGLFunctions>
#include <QOpenGLTexture>
#include <QOpenGLBuffer>

#include "abstract_video_decoder.h"

class VideoDispWidget:public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT
public:
    VideoDispWidget(QWidget* parent = nullptr);
    ~VideoDispWidget();

    virtual void initializeGL();
    virtual void paintGL();
    virtual void resizeGL( int width, int heigh);

public slots:
    void updateVideoFrame(std::shared_ptr<AbstractVideoFrame> frame);     // 通知显示控件

private:
    void processRatio(GLfloat points[16]);        // 为了保持图像的宽高比例，需要对顶点进行额外的处理

private:
    bool isInitialized_;
    std::shared_ptr<AbstractVideoFrame> frame_;
    // QImage img;

    // opengl相关
    QOpenGLBuffer vbo_;         // 顶点和纹理坐标的数据buffer
    GLuint texture_id_Y_;       // 生成的纹理对象的标识符
    GLuint texture_id_U_;
    GLuint texture_id_V_;
    int video_w_;
    int video_h_;
    int line_size_;
    QOpenGLShader *vshader_ = nullptr;
    QOpenGLShader *fshader_ = nullptr;
    QOpenGLShaderProgram *program_ = nullptr;

    //
    // std::shared_ptr<std::vector<uchar>> Y_buf=nullptr;
    // std::shared_ptr<std::vector<uchar>> U_buf=nullptr;
    // std::shared_ptr<std::vector<uchar>> V_buf=nullptr;

};


class OpenGlException: public QException
{
public:
    void raise() const { throw *this; }
    OpenGlException *clone() const { return new OpenGlException(*this); }
};

#endif // VIDEO_DISP_WIDGET_H
