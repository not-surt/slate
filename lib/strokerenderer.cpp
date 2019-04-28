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
//    QOpenGLExtraFunctions(),
    mSurface(),
    mContext(),
    mStrokeFramebuffer(nullptr),
    mImageFramebuffer(nullptr),
    mImage(nullptr),
    mBrushTexture(nullptr),
    mStroke(),
    mStrokeVertexBuffer(nullptr),
    mEditingContext(),
    mBrushProgram(),
    mStrokeFramebufferCopyProgram(),
    mClipQuadVertexBuffer(QOpenGLBuffer::VertexBuffer),
    glDrawArraysInstanced(nullptr),
    glVertexAttribDivisor(nullptr)
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

    // Used extensions
    // GL_EXT_frag_depth
    // GL_ANGLE_instanced_arrays
    // GL_OES_vertex_array_object

//    qDebug() << mContext.isOpenGLES() << mContext.format().majorVersion() << mContext.format().minorVersion() << mContext.hasExtension("GL_ANGLE_instanced_arrays");
//    qDebug() << mContext.format();
    qDebug() << mContext.extensions();
    if (mContext.isOpenGLES() && mContext.format().majorVersion() == 2 && mContext.hasExtension("GL_ANGLE_instanced_arrays")) {
        glDrawArraysInstanced = reinterpret_cast<ProcGlDrawArraysInstanced *>(mContext.getProcAddress("glDrawArraysInstancedANGLE"));
        glVertexAttribDivisor = reinterpret_cast<ProcGlVertexAttribDivisor *>(mContext.getProcAddress("glVertexAttribDivisorANGLE"));
    }
    else if (!mContext.isOpenGLES() && mContext.format().majorVersion() == 2 && mContext.hasExtension("GL_ARB_instanced_arrays")) {
        glDrawArraysInstanced = reinterpret_cast<ProcGlDrawArraysInstanced *>(mContext.getProcAddress("glDrawArraysInstancedARB"));
        glVertexAttribDivisor = reinterpret_cast<ProcGlVertexAttribDivisor *>(mContext.getProcAddress("glVertexAttribDivisorARB"));
    }
    else if ((!mContext.isOpenGLES() && mContext.format().majorVersion() == 3 && mContext.format().minorVersion() >= 3) ||
             (!mContext.isOpenGLES() && mContext.format().majorVersion() > 3) ||
             (mContext.isOpenGLES() && mContext.format().majorVersion() >= 3)) {
        glDrawArraysInstanced = reinterpret_cast<ProcGlDrawArraysInstanced *>(mContext.getProcAddress("glDrawArraysInstanced"));
        glVertexAttribDivisor = reinterpret_cast<ProcGlVertexAttribDivisor *>(mContext.getProcAddress("glVertexAttribDivisor"));
    }
    else {
        qFatal("Requires OpenGL 3.3+, GLES 3+, OpenGL 2 with GL_ARB_instanced_arrays extension or GLES 2 with GL_ANGLE_instanced_arrays extension");
    }

    // Initialise
    {
        bool programLinkingSuccess;
        Utils::ContextGrabber grabber(&mSurface, &mContext);
        QOpenGLFunctions::initializeOpenGLFunctions();

        glClearDepthf(1.0);

        // Create shader programs
        mBrushProgram.addCacheableShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/brush.vsh");
        mBrushProgram.addCacheableShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/brush.fsh");
        programLinkingSuccess = mBrushProgram.link();
        Q_ASSERT_X(programLinkingSuccess, "StrokeRenderer: mBrushProgram", mBrushProgram.log().toLatin1());
        mStrokeFramebufferCopyProgram.addCacheableShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/brush.vsh");
        mStrokeFramebufferCopyProgram.addCacheableShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/strokebuffer.fsh");
        programLinkingSuccess = mStrokeFramebufferCopyProgram.link();
        Q_ASSERT_X(programLinkingSuccess, "StrokeRenderer: mStrokeFramebufferCopyProgram", mStrokeFramebufferCopyProgram.log().toLatin1());

        // Create vertex buffer
        const bool createVertexBufferSuccess = mClipQuadVertexBuffer.create();
        Q_ASSERT(createVertexBufferSuccess);
        mClipQuadVertexBuffer.bind();
        mClipQuadVertexBuffer.allocate(clipQuad.data(), sizeof(clipQuad) * sizeof(QVector2D));
        mClipQuadVertexBuffer.release();
    }
}

StrokeRenderer::~StrokeRenderer()
{
    Utils::ContextGrabber grabber(&mSurface, &mContext);

    delete mStrokeFramebuffer;
    delete mImageFramebuffer;
    delete mBrushTexture;
    delete mStrokeVertexBuffer;
    mClipQuadVertexBuffer.destroy();
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

    Q_ASSERT(QOpenGLContext::currentContext() == &mContext);

    delete mStrokeFramebuffer;
    mStrokeFramebuffer = new QOpenGLFramebufferObject(mImage->size(), QOpenGLFramebufferObject::Depth);
    Q_ASSERT(mStrokeFramebuffer);

    delete mImageFramebuffer;
    mImageFramebuffer = new QOpenGLFramebufferObject(mImage->size(), QOpenGLFramebufferObject::Depth);
    Q_ASSERT(mImageFramebuffer);
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

EditingContext &StrokeRenderer::editingContext()
{
    return mEditingContext;
}

const EditingContext &StrokeRenderer::editingContext() const
{
    return mEditingContext;
}

void StrokeRenderer::setEditingContext(const EditingContext &editingContext)
{
    if (editingContext == mEditingContext)
        return;

    mEditingContext = editingContext;
    delete mBrushTexture;
    mBrushTexture = nullptr;
    if (mEditingContext.brush.type == Brush::ImageType && !mEditingContext.brush.image.isNull()) {
        mBrushTexture = new QOpenGLTexture(mEditingContext.brush.image, QOpenGLTexture::DontGenerateMipMaps);
        mBrushTexture->setMinMagFilters(QOpenGLTexture::Nearest, QOpenGLTexture::Nearest);
    }
}

void StrokeRenderer::upload(const QRect &rect)
{
    const QRect &uploadRect = rect.isValid() ? rect : mImage->rect();
    // Convert linear alpha to premultiplied alpha for correct blending
    const QImage uploadImage = mImage->copy(uploadRect).rgbSwapped().convertToFormat(QImage::Format_ARGB32_Premultiplied);

    glBindTexture(GL_TEXTURE_2D, mImageFramebuffer->texture());
    glTexSubImage2D(GL_TEXTURE_2D, 0, uploadRect.x(), uploadRect.y(), uploadRect.width(), uploadRect.height(), GL_RGBA, GL_UNSIGNED_BYTE,  uploadImage.constBits());
    glBindTexture(GL_TEXTURE_2D, 0);
}

void StrokeRenderer::download(const QRect &rect)
{
    const QRect &downloadRect = rect.isValid() ? rect : mImage->rect();
    QImage downloadImage(downloadRect.size(), QImage::Format_ARGB32_Premultiplied);

    mImageFramebuffer->bind();
    glReadPixels(downloadRect.x(), downloadRect.y(), downloadRect.width(), downloadRect.height(), GL_RGBA, GL_UNSIGNED_BYTE, downloadImage.bits());
    mImageFramebuffer->release();

    QPainter painter(mImage);
    painter.setCompositionMode(QPainter::CompositionMode_Source);
    // Convert premultiplied alpha back to linear alpha
    painter.drawImage(downloadRect.topLeft(), downloadImage.convertToFormat(QImage::Format_ARGB32).rgbSwapped());
}

void StrokeRenderer::renderBrush(const QTransform &transform)
{
    QTransform brushTransform;
    brushTransform.scale(mEditingContext.brush.size.width() / 2.0, mEditingContext.brush.size.height() / 2.0);

    QTransform textureTransform;
    if (mEditingContext.brush.type == Brush::ImageType) {
        textureTransform.scale(0.5, 0.5);
        textureTransform.translate(1.0, 1.0);
    }

    mBrushProgram.setUniformValue(mBrushProgram.uniformLocation("vertexMatrix"), QMatrix4x4(brushTransform * transform));
    mBrushProgram.setUniformValue(mBrushProgram.uniformLocation("textureMatrix"), QMatrix4x4(textureTransform));

//    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, 4, 1);
}

void StrokeRenderer::render(const QTransform &transform, const QRect &clip)
{
    // Render brush dabs to stroke buffer
    {
        mStrokeFramebuffer->bind();
        glViewport(0, 0, mStrokeFramebuffer->width(), mStrokeFramebuffer->height());
        if (clip.isValid()) {
            glEnable(GL_SCISSOR_TEST);
            glScissor(clip.x(), clip.y(), clip.width(), clip.height());
        }
        else {
            glDisable(GL_SCISSOR_TEST);
        }
        glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        glDisable(GL_BLEND);

        mBrushProgram.bind();

        // Transform from image space to clip space
        QTransform vertexTransform;
        vertexTransform.translate(-1.0, -1.0);
        vertexTransform.scale(2.0 / mStrokeFramebuffer->width(), 2.0 / mStrokeFramebuffer->height());
        vertexTransform = transform * vertexTransform;

        if (mEditingContext.brush.type == Brush::ImageType && !mEditingContext.brush.image.isNull()) {
            Q_ASSERT(mBrushTexture);
            const int textureUnit = 0;
            mBrushProgram.setUniformValue(mBrushProgram.uniformLocation("imageTypeTexture"), textureUnit);
            mBrushTexture->bind(textureUnit);
        }

        mBrushProgram.setUniformValue(mBrushProgram.uniformLocation("type"), mEditingContext.brush.type);
        mBrushProgram.setUniformValue(mBrushProgram.uniformLocation("colour"), mEditingContext.foregroundColour);
        mBrushProgram.setUniformValue(mBrushProgram.uniformLocation("opacity"), GLfloat(mEditingContext.brush.opacity));
        mBrushProgram.setUniformValue(mBrushProgram.uniformLocation("hardness"), GLfloat(mEditingContext.brush.hardness));
        mBrushProgram.setUniformValue(mBrushProgram.uniformLocation("singleColour"), mEditingContext.brush.singleColour);

        const int vertexLocation = mBrushProgram.attributeLocation("vertex");
        mBrushProgram.enableAttributeArray(vertexLocation);
        mClipQuadVertexBuffer.bind();
        mBrushProgram.setAttributeBuffer(vertexLocation, GL_FLOAT, 0, 2, sizeof(QVector2D));

        glVertexAttribDivisor(vertexLocation, 0);

        mStroke.apply([&](const StrokePoint &point) {
            renderBrush(QTransform().translate(point.pos.x(), point.pos.y()) * vertexTransform);
        }, mEditingContext.brush, clip, true);

        mClipQuadVertexBuffer.release();
        if (mEditingContext.brush.type == Brush::ImageType && !mEditingContext.brush.image.isNull()) {
            mBrushTexture->release();
        }
        mBrushProgram.release();
        mStrokeFramebuffer->release();
    }

    // Copy stroke framebuffer to image framebuffer
    {
        mImageFramebuffer->bind();
        glViewport(0, 0, mImageFramebuffer->width(), mImageFramebuffer->height());
        if (clip.isValid()) {
            glEnable(GL_SCISSOR_TEST);
            glScissor(clip.x(), clip.y(), clip.width(), clip.height());
        }
        else {
            glDisable(GL_SCISSOR_TEST);
        }
        glDisable(GL_DEPTH_TEST);

        glEnable(GL_BLEND);
        glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
        switch (mEditingContext.blendMode) {
            case EditingContext::BlendMode::Blend: {
                glBlendFuncSeparate(GL_ONE, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    //            glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA); // No need for premul in shader?
            } break;
            case EditingContext::BlendMode::Replace: {
                glBlendFuncSeparate(GL_ONE, GL_ZERO, GL_ONE, GL_ZERO);
            } break;
            case EditingContext::BlendMode::ReplaceColour: {
                glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ZERO, GL_ONE);
            } break;
            case EditingContext::BlendMode::ReplaceAlpha: {
                glBlendFuncSeparate(GL_ZERO, GL_ONE, GL_ONE, GL_SRC_ALPHA);
            } break;
            case EditingContext::BlendMode::Add: {
    //            glBlendFuncSeparate(GL_ONE, GL_ONE, GL_ONE, GL_ONE); // Allow draw over transparent?
                glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE, GL_ZERO, GL_ONE);
            } break;
            case EditingContext::BlendMode::Subtract: {
                glBlendEquationSeparate(GL_FUNC_REVERSE_SUBTRACT, GL_FUNC_ADD);
                glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE, GL_ZERO, GL_ONE);
            } break;
            case EditingContext::BlendMode::Multiply: {
                glBlendFuncSeparate(GL_DST_COLOR, GL_ONE_MINUS_SRC_ALPHA, GL_ZERO, GL_ONE);
            } break;
            case EditingContext::BlendMode::Erase: {
                glBlendFuncSeparate(GL_ZERO, GL_ONE_MINUS_SRC_ALPHA, GL_ZERO, GL_ONE_MINUS_SRC_ALPHA);
            } break;
        }

        mStrokeFramebufferCopyProgram.bind();

        const int textureUnit = 0;
        mStrokeFramebufferCopyProgram.setUniformValue(mStrokeFramebufferCopyProgram.uniformLocation("texture"), textureUnit);
        glBindTexture(GL_TEXTURE_2D, mStrokeFramebuffer->texture());

        const int vertexLocation = mStrokeFramebufferCopyProgram.attributeLocation("vertex");
        mStrokeFramebufferCopyProgram.enableAttributeArray(vertexLocation);
        mClipQuadVertexBuffer.bind();
        mStrokeFramebufferCopyProgram.setAttributeBuffer(vertexLocation, GL_FLOAT, 0, 2, sizeof(QVector2D));

        QTransform textureTransform;
        textureTransform.scale(0.5, 0.5);
        textureTransform.translate(1.0, 1.0);

        mStrokeFramebufferCopyProgram.setUniformValue(mStrokeFramebufferCopyProgram.uniformLocation("vertexMatrix"), QMatrix4x4());
        mStrokeFramebufferCopyProgram.setUniformValue(mStrokeFramebufferCopyProgram.uniformLocation("textureMatrix"), QMatrix4x4(textureTransform));
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

        mClipQuadVertexBuffer.release();
        glBindTexture(GL_TEXTURE_2D, 0);
        mStrokeFramebufferCopyProgram.release();
        mImageFramebuffer->release();
    }
}

QRect StrokeRenderer::brushBounds(const Brush &brush) const
{
    return QRect();
}

QRect StrokeRenderer::strokeBounds(const Stroke &stroke, const Brush &brush) const
{
    return QRect();
}
