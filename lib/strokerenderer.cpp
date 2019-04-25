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
    mStroke(),
    mStrokeVertexBuffer(nullptr),
    mEditingContext(),
    mProgram(),
    mUnitQuadVertexBuffer(QOpenGLBuffer::VertexBuffer)
{
    // Create offscreen render context
    QSurfaceFormat format;
    format.setRenderableType(QSurfaceFormat::OpenGLES);
    format.setVersion(2, 0);
#ifdef QT_DEBUG
    format.setOption(QSurfaceFormat::DebugContext);
#endif
    mSurface.setFormat(format);
    Q_ASSERT(QThread::currentThread() == QGuiApplication::instance()->thread());
    mSurface.create();
    const bool createContextSuccess = mContext.create();
    Q_ASSERT(createContextSuccess);

    // Initialise
    {
        Utils::ContextGrabber grabber(&mSurface, &mContext);
        initializeOpenGLFunctions();

        glClearDepthf(1.0);

        // Create shader program
        mProgram.addCacheableShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/brush.vsh");
        mProgram.addCacheableShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/brush.fsh");
        const bool programLinkingSuccess = mProgram.link();
        Q_ASSERT_X(programLinkingSuccess, "StrokeRenderer", mProgram.log().toLatin1());

        // Create vertex buffer
        const bool createVertexBufferSuccess = mUnitQuadVertexBuffer.create();
        Q_ASSERT(createVertexBufferSuccess);
        mUnitQuadVertexBuffer.bind();
        mUnitQuadVertexBuffer.allocate(clipQuad.data(), sizeof(clipQuad) * sizeof(QVector2D));
    }
}

StrokeRenderer::~StrokeRenderer()
{
    Utils::ContextGrabber grabber(&mSurface, &mContext);

    delete mFramebuffer;
    delete mBrushTexture;
    delete mStrokeVertexBuffer;
    mUnitQuadVertexBuffer.destroy();
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
    mFramebuffer = new QOpenGLFramebufferObject(mImage->size(), QOpenGLFramebufferObject::Depth);
    Q_ASSERT(mFramebuffer);
}

const Stroke &StrokeRenderer::stroke() const
{
    return mStroke;
}

void StrokeRenderer::setStroke(const Stroke &stroke)
{
    if (stroke == mStroke)
        return;

    mStroke = stroke;
//    delete mStrokeVertexBuffer;
//    mStrokeVertexBuffer = nullptr;
//    if (!mStroke.isEmpty()) {
//        mStrokeVertexBuffer = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    //    }
}

EditingContextManager &StrokeRenderer::editingContext()
{
    return mEditingContext;
}

const EditingContextManager &StrokeRenderer::editingContext() const
{
    return mEditingContext;
}

void StrokeRenderer::setEditingContext(const EditingContextManager &editingContext)
{
    if (editingContext == mEditingContext)
        return;

    mEditingContext = editingContext;
    delete mBrushTexture;
    mBrushTexture = nullptr;
    if (mEditingContext.brush()->type() == Brush::ImageType && !mEditingContext.brush()->image().isNull()) {
        mBrushTexture = new QOpenGLTexture(mEditingContext.brush()->image(), QOpenGLTexture::DontGenerateMipMaps);
        mBrushTexture->setMinMagFilters(QOpenGLTexture::Nearest, QOpenGLTexture::Nearest);
    }
}

void StrokeRenderer::upload(const QRect &rect)
{
    const QRect &uploadRect = rect.isValid() ? rect : mImage->rect();
    // Convert linear alpha to premultiplied alpha for correct blending
    const QImage uploadImage = mImage->copy(uploadRect).rgbSwapped().convertToFormat(QImage::Format_ARGB32_Premultiplied);

    glBindTexture(GL_TEXTURE_2D, mFramebuffer->texture());
    glTexSubImage2D(GL_TEXTURE_2D, 0, uploadRect.x(), uploadRect.y(), uploadRect.width(), uploadRect.height(), GL_RGBA, GL_UNSIGNED_BYTE,  uploadImage.constBits());
    glBindTexture(GL_TEXTURE_2D, 0);
}

void StrokeRenderer::download(const QRect &rect)
{
    const QRect &downloadRect = rect.isValid() ? rect : mImage->rect();
    QImage downloadImage(downloadRect.size(), QImage::Format_ARGB32_Premultiplied);

    mFramebuffer->bind();
    glReadPixels(downloadRect.x(), downloadRect.y(), downloadRect.width(), downloadRect.height(), GL_RGBA, GL_UNSIGNED_BYTE, downloadImage.bits());
    mFramebuffer->release();

    QPainter painter(mImage);
    painter.setCompositionMode(QPainter::CompositionMode_Source);
    // Convert premultiplied alpha back to linear alpha
    painter.drawImage(downloadRect.topLeft(), downloadImage.convertToFormat(QImage::Format_ARGB32).rgbSwapped());
}

void StrokeRenderer::renderBrush(const QTransform &transform)
{
    QTransform brushTransform;
    brushTransform.scale(mEditingContext.brush()->size().width() / 2.0, mEditingContext.brush()->size().height() / 2.0);

    QTransform textureTransform;
    if (mEditingContext.brush()->type() == Brush::ImageType) {
        textureTransform.scale(0.5, 0.5);
        textureTransform.translate(1.0, 1.0);
    }

    mProgram.setUniformValue(mProgram.uniformLocation("matrix"), QMatrix4x4(brushTransform * transform));
    mProgram.setUniformValue(mProgram.uniformLocation("textureMatrix"), QMatrix4x4(textureTransform));

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

void StrokeRenderer::render(const QTransform &transform, const QRect &clip)
{
    mFramebuffer->bind();
    glViewport(0, 0, mFramebuffer->width(), mFramebuffer->height());
    if (clip.isValid()) {
        glEnable(GL_SCISSOR_TEST);
        glScissor(clip.x(), clip.y(), clip.width(), clip.height());
    }
    else {
        glDisable(GL_SCISSOR_TEST);
    }
    glClear(GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glEnable(GL_BLEND);
    glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
    switch (mEditingContext.blendMode()) {
        case EditingContextManager::BlendMode::Blend: {
            glBlendFuncSeparate(GL_ONE, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
//            glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA); // No need for premul in shader?
        } break;
        case EditingContextManager::BlendMode::Replace: {
            glBlendFuncSeparate(GL_ONE, GL_ZERO, GL_ONE, GL_ZERO);
        } break;
        case EditingContextManager::BlendMode::ReplaceColour: {
            glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ZERO, GL_ONE);
        } break;
        case EditingContextManager::BlendMode::ReplaceAlpha: {
            glBlendFuncSeparate(GL_ZERO, GL_ONE, GL_ONE, GL_SRC_ALPHA);
        } break;
        case EditingContextManager::BlendMode::Add: {
//            glBlendFuncSeparate(GL_ONE, GL_ONE, GL_ONE, GL_ONE); // Allow draw over transparent?
            glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE, GL_ZERO, GL_ONE);
        } break;
        case EditingContextManager::BlendMode::Subtract: {
            glBlendEquationSeparate(GL_FUNC_REVERSE_SUBTRACT, GL_FUNC_ADD);
            glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE, GL_ZERO, GL_ONE);
        } break;
        case EditingContextManager::BlendMode::Multiply: {
            glBlendFuncSeparate(GL_DST_COLOR, GL_ONE_MINUS_SRC_ALPHA, GL_ZERO, GL_ONE);
        } break;
        case EditingContextManager::BlendMode::Erase: {
            glBlendFuncSeparate(GL_ZERO, GL_ONE_MINUS_SRC_ALPHA, GL_ZERO, GL_ONE_MINUS_SRC_ALPHA);
        } break;
    }

    mProgram.bind();
    // Transform from image space to clip space
    QTransform strokeTransform;
    strokeTransform.translate(-1.0, -1.0);
    strokeTransform.scale(2.0 / mFramebuffer->width(), 2.0 / mFramebuffer->height());
    strokeTransform = transform * strokeTransform;

    if (mEditingContext.brush()->type() == Brush::ImageType && !mEditingContext.brush()->image().isNull()) {
        Q_ASSERT(mBrushTexture);
        const int textureUnit = 0;
        mProgram.setUniformValue(mProgram.uniformLocation("imageTypeTexture"), textureUnit);
        mBrushTexture->bind(textureUnit);
    }

    mProgram.setUniformValue(mProgram.uniformLocation("type"), mEditingContext.brush()->type());
    mProgram.setUniformValue(mProgram.uniformLocation("colour"), mEditingContext.foregroundColour());
    mProgram.setUniformValue(mProgram.uniformLocation("opacity"), GLfloat(mEditingContext.brush()->opacity()));
    mProgram.setUniformValue(mProgram.uniformLocation("hardness"), GLfloat(mEditingContext.brush()->hardness()));
    mProgram.setUniformValue(mProgram.uniformLocation("singleColour"), mEditingContext.brush()->singleColour());

    const int vertexLocation = mProgram.attributeLocation("vertex");
    mProgram.enableAttributeArray(vertexLocation);
    mUnitQuadVertexBuffer.bind();
    mProgram.setAttributeBuffer(vertexLocation, GL_FLOAT, 0, 2, sizeof(QVector2D));

    mStroke.apply([&](const StrokePoint &point) {
        renderBrush(QTransform().translate(point.pos.x(), point.pos.y()) * strokeTransform);
    }, *mEditingContext.brush(), true);

    mUnitQuadVertexBuffer.release();
    if (mEditingContext.brush()->type() == Brush::ImageType && !mEditingContext.brush()->image().isNull()) {
        mBrushTexture->release();
    }
    mProgram.release();
    mFramebuffer->release();
}

QRect StrokeRenderer::brushBounds(const BrushManager &brush) const
{
    return QRect();
}

QRect StrokeRenderer::strokeBounds(const Stroke &stroke, const BrushManager &brush) const
{
    return QRect();
}
