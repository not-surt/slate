#include "strokerenderer.h"

#include <QPainter>
#include <QThread>
#include <QGuiApplication>

#include "utils.h"

QVector<QVector2D> StrokeRenderer::clipQuad{
    {-1.0, 1.0},
    {-1.0, -1.0},
    {1.0, -1.0},
    {1.0, 1.0}
};

StrokeRenderer::StrokeRenderer() :
    QOpenGLFunctions(),
    mSurface(),
    mContext(),
    mFramebuffer(nullptr),
    mImage(nullptr),
    mBrushTexture(nullptr),
    mBrushTextureSize(),
    mProgram(),
    mVertexBuffer(QOpenGLBuffer::VertexBuffer)
{
    // Create offscreen render context
    QSurfaceFormat format;
    format.setRenderableType(QSurfaceFormat::OpenGLES);
    format.setVersion(2, 0);
    format.setOption(QSurfaceFormat::DebugContext);
    mSurface.setFormat(format);
    Q_ASSERT(QThread::currentThread() == QGuiApplication::instance()->thread());
    mSurface.create();
    const bool createContextSuccess = mContext.create();
    Q_ASSERT(createContextSuccess);

    // Initialise
    Utils::ContextGrabber grabber(&mSurface, &mContext);
    initializeOpenGLFunctions();

    // Create shader program
    mProgram.addCacheableShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/brush.vsh");
    mProgram.addCacheableShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/brush.fsh");
    const bool programLinkingSuccess = mProgram.link();
    Q_ASSERT_X(programLinkingSuccess, "StrokeRenderer", mProgram.log().toLatin1());

    // Create vertex buffer
    const bool createVertexBufferSuccess = mVertexBuffer.create();
    Q_ASSERT(createVertexBufferSuccess);
    mVertexBuffer.bind();
    mVertexBuffer.allocate(clipQuad.data(), sizeof(clipQuad) * sizeof(QVector2D));
}

StrokeRenderer::~StrokeRenderer()
{
    Utils::ContextGrabber grabber(&mSurface, &mContext);

    delete mFramebuffer;
    delete mBrushTexture;
    mVertexBuffer.destroy();
    mSurface.destroy();
}

QOffscreenSurface &StrokeRenderer::surface()
{
    return mSurface;
}

QOpenGLContext &StrokeRenderer::context()
{
    return mContext;
}

QImage *StrokeRenderer::image() const
{
    return mImage;
}

void StrokeRenderer::setImage(QImage *const image)
{
    mImage = image;

    delete mFramebuffer;
    Q_ASSERT(QOpenGLContext::currentContext() == &mContext);
    mFramebuffer = new QOpenGLFramebufferObject(mImage->size());
    Q_ASSERT(mFramebuffer);
}

void StrokeRenderer::updateBrushTexture(const QImage &image)
{
    delete mBrushTexture;
    mBrushTexture = nullptr;
    if (!image.isNull()) {
        mBrushTexture = new QOpenGLTexture(image, QOpenGLTexture::DontGenerateMipMaps);
        mBrushTextureSize = image.size();
    }
}

void StrokeRenderer::upload(const QRect &rect)
{
    glActiveTexture(GL_TEXTURE0 + 0);
    glBindTexture(GL_TEXTURE_2D, mFramebuffer->texture());
    if (rect.isValid()) {
        glTexSubImage2D(GL_TEXTURE_2D, 0, rect.x(), rect.y(), rect.width(), rect.height(), GL_RGBA, GL_UNSIGNED_BYTE,  mImage->rgbSwapped().copy(rect).constBits());
    }
    else {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, mImage->width(), mImage->height(), 0, GL_RGBA, GL_UNSIGNED_BYTE,  mImage->rgbSwapped().constBits());
    }
    glBindTexture(GL_TEXTURE_2D, 0);
}

void StrokeRenderer::download(const QRect &rect)
{
    mFramebuffer->bind();
    if (rect.isValid()) {
        QImage readImage(rect.size(), mImage->format());
        glReadPixels(rect.x(), rect.y(), rect.width(), rect.height(), GL_RGBA, GL_UNSIGNED_BYTE, readImage.bits());
        QPainter painter(mImage);
        painter.setCompositionMode(QPainter::CompositionMode_Source);
        painter.drawImage(rect.topLeft(), readImage.rgbSwapped());
    }
    else {
        glReadPixels(0, 0, mImage->width(), mImage->height(), GL_RGBA, GL_UNSIGNED_BYTE, mImage->bits());
        *mImage = mImage->rgbSwapped();
    }
    mFramebuffer->release();
}

void StrokeRenderer::renderBrush(const Brush &brush, const QColor &colour, const QTransform &transform, const QRect &rect)
{
    QTransform brushTransform;
    brushTransform.scale(brush.size.width() / 2.0, brush.size.height() / 2.0);

    mProgram.setUniformValue(mProgram.uniformLocation("matrix"), QMatrix4x4(brushTransform * transform));

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

void StrokeRenderer::render(const Stroke &stroke, const Brush &brush, const QColor &colour, const qreal scaleMin, const qreal scaleMax, const qreal opacityMin, const qreal opacityMax, const qreal hardnessMin, const qreal hardnessMax, const bool singleColour, const QTransform &transform, const QRect &rect)
{
    mFramebuffer->bind();
    glViewport(0, 0, mFramebuffer->width(), mFramebuffer->height());
    glEnable(GL_BLEND);
    glBlendEquationSeparate(GL_FUNC_ADD,GL_FUNC_ADD);
    glBlendFuncSeparate(GL_ONE,GL_ONE_MINUS_SRC_ALPHA,GL_ONE,GL_ONE_MINUS_SRC_ALPHA);
    if (rect.isValid()) {
        glEnable(GL_SCISSOR_TEST);
        glScissor(rect.x(), rect.y(), rect.width(), rect.height());
    }
    else {
        glDisable(GL_SCISSOR_TEST);
    }

    mProgram.bind();
    // Transform from image space to clip space
    QTransform framebufferTransform;
    framebufferTransform.translate(-1.0, -1.0);
    framebufferTransform.scale(2.0 / mFramebuffer->width(), 2.0 / mFramebuffer->height());
    qDebug() << framebufferTransform.map(QPointF(mFramebuffer->width(), 0.0));///////////////////////////

    if (brush.type == Brush::ImageType) {
        updateBrushTexture(brush.pixmap.toImage());
        Q_ASSERT(mBrushTexture);
        const int textureUnit = 0;
        mProgram.setUniformValue(mProgram.uniformLocation("imageTypeTexture"), textureUnit);
        mBrushTexture->bind(textureUnit);
        mProgram.setUniformValue(mProgram.uniformLocation("imageTypeTextureSize"), mBrushTextureSize);
    }

    mProgram.setUniformValue(mProgram.uniformLocation("type"), brush.type);
    mProgram.setUniformValue(mProgram.uniformLocation("colour"), colour);
    mProgram.setUniformValue(mProgram.uniformLocation("opacity"), GLfloat(opacityMax));
    mProgram.setUniformValue(mProgram.uniformLocation("hardness"), GLfloat(hardnessMax));
    mProgram.setUniformValue(mProgram.uniformLocation("singleColour"), singleColour);

    const int vertexLocation = mProgram.attributeLocation("vertex");
    mProgram.enableAttributeArray(vertexLocation);
    mVertexBuffer.bind();
    mProgram.setAttributeBuffer(vertexLocation, GL_FLOAT, 0, 2, sizeof(QVector2D));

    auto render = [&](const StrokePoint &point) {
        qDebug() << point.pos;////////////////
        renderBrush(brush, colour, QTransform().translate(point.pos.x(), point.pos.y()) * transform * framebufferTransform, rect);
    };

    stroke.draw(render, nullptr, brush, scaleMin, scaleMax, colour, QPainter::CompositionMode_Source, true);

    mVertexBuffer.release();
    if (brush.type == Brush::ImageType) {
        mBrushTexture->release();
    }
    mProgram.release();
    mFramebuffer->release();
}
