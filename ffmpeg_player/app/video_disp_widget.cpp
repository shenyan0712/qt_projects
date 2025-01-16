#include <QPainter>
#include <QMutex>

#include "video_disp_widget.h"


#define ATTRIB_VERTEX_LOC 3
#define ATTRIB_TEXTURE_LOC 4

QMutex loc_mutex;

std::string vsrc = R"(
#version 330 core

in vec2 vertexIn;         // 因为是平面，只需要vec2即可
in vec2 textureIn;
out vec2 TextureCoord;    // 纹理坐标
void main()
{
    gl_Position =  vec4(vertexIn.x, vertexIn.y, 0.0, 1.0);
    TextureCoord = textureIn;
}
)";

std::string fsrc = R"(
#version 330 core

in vec2 TextureCoord;
uniform sampler2D textureY;
uniform sampler2D textureU;
uniform sampler2D textureV;

void main(void){
    vec3 yuv;
    vec3 rgb;

    yuv.x = texture2D(textureY, TextureCoord).r;
    yuv.y = texture2D(textureU, TextureCoord).r-0.5;
    yuv.z = texture2D(textureV, TextureCoord).r-0.5;

    // rgb.r = yuv.r+1.13983*yuv.b;
    // rgb.g = yuv.r - 0.39465*yuv.g - 0.5806*yuv.b;
    // rgb.b = yuv.r + 2.0321*yuv.g;

    rgb = mat3( 1.0,        1.0,        1.0,
                0.0,        -0.3455,   1.779,
                1.4075,    -0.7168,   0.0)*yuv;

    gl_FragColor = vec4(rgb, 1.0);
}
)";



VideoDispWidget::VideoDispWidget(QWidget* parent): QOpenGLWidget(parent)
{
    // img.load("d:/test.jpg");
    frame_ = nullptr;
}

VideoDispWidget::~VideoDispWidget()
{
    // delete[] reinterpret_cast<unsigned char*>(openGLStruct->mBufYuv);
}

// void VideoDispWidget::InitDrawBuffer(unsigned bsize)
// {
//     openGLStruct->mFrameSize = bsize;
//     openGLStruct->mBufYuv = new unsigned char[bsize];
// }

void VideoDispWidget::updateVideoFrame(std::shared_ptr<AbstractVideoFrame> frame)
{
    loc_mutex.lock();
    frame_ = frame;
    video_w_ = frame->width();
    video_h_ = frame->height();
    line_size_ = frame->line_size();

    loc_mutex.unlock();
    update();
}

/*
 * 1. 为了保持图像的宽高比例，对顶点进行额外的处理:
 *          QOpenGLWidget视口的尺寸在OpenGL下的坐标为：使用数学上的坐标系，视口中心为0点，最左上角为(-1, 1), 最右下角为(1, -1)
 *          因此满屏显示时，左右是从-1到1，上下是从[1, -1]。
 *          为了保持图像的宽高比例并在视口上居中显示，则顶点的位置就需要根据视口的像素尺寸以及图像的像素尺寸进行调整
 * 2. 视频解码后的视频数据，为了对齐右边可能由padding像素，为此需要对纹理进行裁剪，只取其中左边需要的部分。
 * 输出参数：
 *      points[16]: [0-7]为矩形四个顶点的坐标，[8-15]为对应的纹理坐标
*/
void VideoDispWidget::processRatio(GLfloat points[16])
{
    auto view_w = this->width();
    auto view_h = this->height();

    float imgAspectRatio = (float)video_w_ / (float)video_h_;
    float viewAspectRatio = (float)view_w / (float)view_h;

    float xScale = 1.0f;
    float yScale = 1.0f;
    if (imgAspectRatio > viewAspectRatio) {
        //图像的宽高比比view的更大，所有图像需要在高度上缩小，以适应窗口
        yScale = viewAspectRatio / imgAspectRatio;
    } else {
        xScale = imgAspectRatio / viewAspectRatio;
    }

    /*
     * 顶点坐标数据，在paintGL中使用TRIANGLE_STRIP（带状三角形）的方式绘制。
     * GL_TRIANGLE_STRIP会尝试共享前三个顶点构造第一个三角形，然后根据后面的顶点构造接下来的三角形
     * openGL的坐标系与数学上惯用坐标系一致，而与屏幕坐标系不同。
     * 因此，绘制出左下三角形和右上三角形。
    */
    static const GLfloat points_temp[] = {
        -1.0f, 1.0f,   // 左下角
        1.0f, 1.0f,    // 右下角
        1.0f, -1.0f,    // 左上角
        -1.0f, -1.0f,     // 右上角

        0.0f,  0.0f,    // 左下角顶点对应纹理坐标
        1.0f,  0.0f,
        1.0f,  1.0f,
        0.0f,  1.0f,
    };

    //
    for(int i=0; i<4; i++) {
        points[i*2] = points_temp[i*2]*xScale;
        points[i*2+1] = points_temp[i*2+1]*yScale;
    }

    //纹理裁剪
    float ratio = video_w_/(float)line_size_;
    for(int i=0; i<4; i++) {
        points[8+2*i] = points_temp[8+2*i]*ratio;
        points[8+2*i+1] = points_temp[8+2*i+1];
    }
}



void VideoDispWidget::initializeGL()
{
    isInitialized_ = true;

    initializeOpenGLFunctions();
    glEnable(GL_DEPTH_TEST);

    // 初始化顶点shader对象
    vshader_ = new QOpenGLShader(QOpenGLShader::Vertex, this);
    // if(!vshader_->compileSourceFile(":/opengl/vertex.vsh"))
    if(!vshader_->compileSourceCode(vsrc.c_str() ) )
    {
        throw OpenGlException();
    }

    // 初始化Fragment Shader，用于将yuv转为rgb
    fshader_ = new QOpenGLShader(QOpenGLShader::Fragment, this);
    // if(!fshader_->compileSourceFile(":/opengl/fragment.fsh"))
    if(!fshader_->compileSourceCode(fsrc.c_str() ) )
    {
        throw OpenGlException();
    }

    // 创建Shader程序容器
    program_ = new QOpenGLShaderProgram(this);
    // 添加fragment shader到程序容器
    program_->addShader(fshader_);
    // 添加vertex shader到程序容器
    program_->addShader(vshader_);

    // 绑定vertexIn属性到指定的位置ATTRIB_VERTEX, 该属性在vertex shader的代码中有对应的声明
    program_->bindAttributeLocation("vertexIn", ATTRIB_VERTEX_LOC);
    // 绑定textureIn属性到指定的位置ATTRIB_TEXTURE, 该属性在vertex shader的代码中有对应的声明
    program_->bindAttributeLocation("textureIn", ATTRIB_TEXTURE_LOC);

    //链接所有添加的shader程序
    if(!program_->link()) {
        throw OpenGlException();
    }

    GLuint ids[3];
    glGenTextures(3,ids);
    texture_id_Y_ = ids[0];
    texture_id_U_ = ids[1];
    texture_id_V_ = ids[2];

    vbo_.create();      // 创建顶点和纹理坐标数据的buffer

    // 设置背景颜色
    glClearColor(0.3, 0.3, 0.3, 0.0);
}

void VideoDispWidget::resizeGL( int width, int height)
{
    qDebug()<<"OpenGL resize:"<<width<<" x "<<height;
    if(height == 0)// prevents being divided by zero
        height = 1;// set the height to 1

    // Set the viewport
    glViewport(0, 0, width, height);
    update();
}

// void VideoDispWidget::paintGL(){
//     static int i= 0;
//     QPainter p(this);
//     i++;
//     if(i>20) i=0;
//     QRectF target( i, i, 500, 500);
//     p.drawImage( target, img);

// }

void VideoDispWidget::paintGL()
{
    if(frame_ == nullptr) {
        QOpenGLWidget::paintGL();
        return;
    }
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);

    //绑定程序到当前OpenGL上下文环境, 下面的相关操作都针对该program
    if(!program_->bind()) {
        throw OpenGlException();
    }

    GLfloat points_[16];
    processRatio(points_);

    vbo_.bind(); // 绑定该buffer到当前OpenGL上下文环境,下面的相关操作都关联该buffer
    vbo_.allocate(points_,sizeof(points_));
    program_->enableAttributeArray(ATTRIB_VERTEX_LOC);
    program_->enableAttributeArray(ATTRIB_TEXTURE_LOC);
    /*
     * 设置vertex shader代码中的vertexIn和textureIn属性关联的vbo中的地址。
     * vbo_中数据的前四个为顶点坐标， 后四个为纹理坐标
    */
    program_->setAttributeBuffer(ATTRIB_VERTEX_LOC,GL_FLOAT, 0, 2, 2*sizeof(GLfloat));
    program_->setAttributeBuffer(ATTRIB_TEXTURE_LOC,GL_FLOAT, 2 * 4 * sizeof(GLfloat),2,2*sizeof(GLfloat));

    loc_mutex.lock();

    // 载入Y纹理数据
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D,texture_id_Y_);
    glTexImage2D(GL_TEXTURE_2D,0,GL_RED, line_size_, video_h_,0,GL_RED,GL_UNSIGNED_BYTE, frame_->y_buf());
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // 载入U纹理数据
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D,texture_id_U_);
    glTexImage2D(GL_TEXTURE_2D,0,GL_RED, line_size_ >> 1,video_h_ >> 1,0,GL_RED,GL_UNSIGNED_BYTE, frame_->u_buf());
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // 载入V纹理数据
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D,texture_id_V_);
    glTexImage2D(GL_TEXTURE_2D,0,GL_RED, line_size_ >> 1,video_h_ >> 1,0,GL_RED,GL_UNSIGNED_BYTE, frame_->v_buf());
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    frame_ = nullptr;
    loc_mutex.unlock();

    program_->setUniformValue("textureY",0);
    program_->setUniformValue("textureU",1);
    program_->setUniformValue("textureV",2);
    glDrawArrays(GL_QUADS,0,4);
    program_->disableAttributeArray("vertexIn");
    program_->disableAttributeArray("textureIn");
    program_->release();

}

