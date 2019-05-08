#include "strokerenderer.h"

#include <QPainter>
#include <QThread>
#include <QGuiApplication>
#include <QFile>

#include "utils.h"

StrokeRenderer::StrokeRenderer() :
    QOpenGLExtraFunctions(),
    mSurface(),
    mContext(),
    mImage(nullptr),
    mBrushTexture(nullptr),
    mStroke(),
    mEditingContext(),
    mBrushProgram(),
    mStrokeFramebufferCopyProgram(),
    strokeUniformBuffer()
{
    // Create offscreen render context
    QSurfaceFormat format;
    if (QOpenGLContext::openGLModuleType() == QOpenGLContext::LibGL) {
        // Desktop OpenGL
        format.setRenderableType(QSurfaceFormat::OpenGL);
        format.setVersion(3, 3);
        format.setProfile(QSurfaceFormat::CoreProfile);
    }
    else {
        // OpenGL ES
        format.setRenderableType(QSurfaceFormat::OpenGLES);
        format.setVersion(3, 0);
    }
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
        bool programLinkingSuccess;
        Utils::ContextGrabber grabber(&mSurface, &mContext);
        QOpenGLFunctions::initializeOpenGLFunctions();

        // Create shader programs
        mBrushProgram.addCacheableShaderFromSourceCode(QOpenGLShader::Vertex, shaderVersionString() + Utils::fileToString(":/shaders/stroke.vsh"));
        mBrushProgram.addCacheableShaderFromSourceCode(QOpenGLShader::Fragment, shaderVersionString() + Utils::fileToString(":/shaders/brush.fsh"));
        programLinkingSuccess = mBrushProgram.link();
        Q_ASSERT_X(programLinkingSuccess, "StrokeRenderer: mBrushProgram", mBrushProgram.log().toLatin1());
        mStrokeFramebufferCopyProgram.addCacheableShaderFromSourceCode(QOpenGLShader::Vertex, shaderVersionString() + Utils::fileToString(":/shaders/quad.vsh"));
        mStrokeFramebufferCopyProgram.addCacheableShaderFromSourceCode(QOpenGLShader::Fragment, shaderVersionString() + Utils::fileToString(":/shaders/strokebuffer.fsh"));
        programLinkingSuccess = mStrokeFramebufferCopyProgram.link();
        Q_ASSERT_X(programLinkingSuccess, "StrokeRenderer: mStrokeFramebufferCopyProgram", mStrokeFramebufferCopyProgram.log().toLatin1());

        // Create stroke uniform buffer
        glGenBuffers(1, &strokeUniformBuffer);
        glBindBuffer(GL_UNIFORM_BUFFER, strokeUniformBuffer);
        glBufferData(GL_UNIFORM_BUFFER, STROKE_SIZE_MAX * sizeof(StrokePoint), nullptr, GL_STREAM_DRAW);

        const GLuint strokeUniformBlockIndex = glGetUniformBlockIndex(mBrushProgram.programId(), "Stroke");
        glUniformBlockBinding(mBrushProgram.programId(), strokeUniformBlockIndex, strokeUniformBlockBinding);
//        glBindBufferBase(GL_UNIFORM_BUFFER, strokeUniformBlockBinding, strokeUniformBuffer);

        // Create framebuffers
        GLuint framebuffers[2];
        glGenFramebuffers(2, framebuffers);
        strokeFramebuffer = framebuffers[0];
        imageFramebuffer = framebuffers[1];
        // Create textures
        GLuint textures[3];
        glGenTextures(3, textures);
        strokeTexture = textures[0];
        imageTexture = textures[1];
        depthStencilTexture = textures[2];
    }
}

StrokeRenderer::~StrokeRenderer()
{
    Utils::ContextGrabber grabber(&mSurface, &mContext);

    const GLuint framebuffers[2] = {strokeFramebuffer, imageFramebuffer};
    glDeleteFramebuffers(2, framebuffers);
    const GLuint textures[3] = {strokeTexture, imageTexture, depthStencilTexture};
    glDeleteTextures(3, textures);

    delete mBrushTexture;
    glDeleteBuffers(1, &strokeUniformBuffer);
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

    // Recreate stroke texture
    glBindTexture(GL_TEXTURE_2D, strokeTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, image->width(), image->height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    // Recreate image texture
    glBindTexture(GL_TEXTURE_2D, imageTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, image->width(), image->height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    // Recreate depth/stencil texture used by both framebuffers
    glBindTexture(GL_TEXTURE_2D, depthStencilTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, image->width(), image->height(), 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, nullptr);

    // Bind textures to stroke framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, strokeFramebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, strokeTexture, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, depthStencilTexture, 0);
    Q_ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
    // Bind textures to image framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, imageFramebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, imageTexture, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, depthStencilTexture, 0);
    Q_ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
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

    // Copy stroke to uniform buffer
    glBindBuffer(GL_UNIFORM_BUFFER, strokeUniformBuffer);
    glBufferData(GL_UNIFORM_BUFFER, mStroke.size() * sizeof(StrokePoint), mStroke.constData(), GL_STREAM_DRAW);
//    glBufferSubData(GL_UNIFORM_BUFFER, 0, mStroke.size() * sizeof(StrokePoint), mStroke.constData());
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
    if (mEditingContext.brush.type == Brush::Type::Image && !mEditingContext.brush.image.isNull()) {
        mBrushTexture = new QOpenGLTexture(mEditingContext.brush.image, QOpenGLTexture::DontGenerateMipMaps);
        mBrushTexture->setMinMagFilters(QOpenGLTexture::Nearest, QOpenGLTexture::Nearest);
    }
}

void StrokeRenderer::prerender(const QRect &rect)
{
    // Upload image to GPU
    const QRect &uploadRect = rect.isValid() ? rect : mImage->rect();
    // Convert linear alpha to premultiplied alpha for correct blending
    const QImage uploadImage = mImage->copy(uploadRect).convertToFormat(QImage::Format_ARGB32_Premultiplied);
    glBindTexture(GL_TEXTURE_2D, imageTexture);
    glTexSubImage2D(GL_TEXTURE_2D, 0, uploadRect.x(), uploadRect.y(), uploadRect.width(), uploadRect.height(), GL_BGRA, GL_UNSIGNED_BYTE,  uploadImage.constBits());
    glBindTexture(GL_TEXTURE_2D, 0);

    // Prepare for rendering stroke(s)
    glBindFramebuffer(GL_FRAMEBUFFER, strokeFramebuffer);
    glViewport(0, 0, mImage->width(), mImage->height());
    glDisable(GL_SCISSOR_TEST);

    glDisable(GL_BLEND);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glClearDepthf(1.0);

    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_ALWAYS, 1, 0xff);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    glClearStencil(0);
    glStencilMask(0xff);

    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void StrokeRenderer::postrender(const QRect &rect)
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Copy stroke framebuffer to image framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, imageFramebuffer);
    glViewport(0, 0, mImage->width(), mImage->height());
    if (rect.isValid()) {
        glEnable(GL_SCISSOR_TEST);
        glScissor(rect.x(), rect.y(), rect.width(), rect.height());
    }
    else {
        glDisable(GL_SCISSOR_TEST);
    }
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_EQUAL, 1, 0xff);

    glEnable(GL_BLEND);
    glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
    switch (mEditingContext.blendMode) {
        case EditingContext::BlendMode::Blend: {
//                glBlendFuncSeparate(GL_ONE, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
            glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA); // No need for premul in shader?
        } break;
        case EditingContext::BlendMode::Replace: {
//                glBlendFuncSeparate(GL_ONE, GL_ZERO, GL_ONE, GL_ZERO);
            glBlendFuncSeparate(GL_SRC_ALPHA, GL_ZERO, GL_ONE, GL_ZERO);
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
    glBindTexture(GL_TEXTURE_2D, strokeTexture);

    QTransform textureTransform;
    textureTransform.scale(0.5, 0.5);
    textureTransform.translate(1.0, 1.0);

    mStrokeFramebufferCopyProgram.setUniformValue(mStrokeFramebufferCopyProgram.uniformLocation("vertexMatrix"), QMatrix4x4());
    mStrokeFramebufferCopyProgram.setUniformValue(mStrokeFramebufferCopyProgram.uniformLocation("textureMatrix"), QMatrix4x4(textureTransform));
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    glBindTexture(GL_TEXTURE_2D, 0);
    mStrokeFramebufferCopyProgram.release();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Download image from GPU
    const QRect &downloadRect = rect.isValid() ? rect : mImage->rect();
    QImage downloadImage(downloadRect.size(), QImage::Format_ARGB32_Premultiplied);

    glBindFramebuffer(GL_FRAMEBUFFER, imageFramebuffer);
    glReadPixels(downloadRect.x(), downloadRect.y(), downloadRect.width(), downloadRect.height(), GL_BGRA, GL_UNSIGNED_BYTE, downloadImage.bits());
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    QPainter painter(mImage);
    painter.setCompositionMode(QPainter::CompositionMode_Source);
    // Convert premultiplied alpha back to linear alpha
    painter.drawImage(downloadRect.topLeft(), downloadImage.convertToFormat(QImage::Format_ARGB32));
}

void StrokeRenderer::render(const QTransform &transform, const QRect &rect)
{
    // Apply clipping
    if (rect.isValid()) {
        glEnable(GL_SCISSOR_TEST);
        glScissor(rect.x(), rect.y(), rect.width(), rect.height());
    }
    else {
        glDisable(GL_SCISSOR_TEST);
    }

    mBrushProgram.bind();

    // Transform from image space to clip space
    QTransform imageToClipMatrix;
    imageToClipMatrix.translate(-1.0, -1.0);
    imageToClipMatrix.scale(2.0 / mImage->width(), 2.0 / mImage->height());

    if (mEditingContext.brush.type == Brush::Type::Image && !mEditingContext.brush.image.isNull()) {
        Q_ASSERT(mBrushTexture);
        const int textureUnit = 0;
        mBrushProgram.setUniformValue(mBrushProgram.uniformLocation("imageTypeTexture"), textureUnit);
        mBrushTexture->bind(textureUnit);
    }

    mBrushProgram.setUniformValue(mBrushProgram.uniformLocation("type"), int(mEditingContext.brush.type));
    mBrushProgram.setUniformValue(mBrushProgram.uniformLocation("colour"), mEditingContext.foregroundColour);
    mBrushProgram.setUniformValue(mBrushProgram.uniformLocation("opacity"), GLfloat(mEditingContext.brush.opacity));
    mBrushProgram.setUniformValue(mBrushProgram.uniformLocation("hardness"), GLfloat(mEditingContext.brush.hardness));
    mBrushProgram.setUniformValue(mBrushProgram.uniformLocation("singleColour"), mEditingContext.brush.singleColour);

    mBrushProgram.setUniformValue(mBrushProgram.uniformLocation("imageMatrix"), QMatrix4x4(transform * imageToClipMatrix));
    QTransform textureTransform;
    if (mEditingContext.brush.type == Brush::Type::Image) {
        textureTransform.scale(0.5, 0.5);
        textureTransform.translate(1.0, 1.0);
    }
    mBrushProgram.setUniformValue(mBrushProgram.uniformLocation("textureMatrix"), QMatrix4x4(textureTransform));
    QTransform brushTransform;
    brushTransform.scale(mEditingContext.brush.size.width() / 2.0, mEditingContext.brush.size.height() / 2.0);
    mBrushProgram.setUniformValue(mBrushProgram.uniformLocation("brushMatrix"), QMatrix4x4(brushTransform));

    auto generateSegmentDabs = [this](const StrokePoint &from, const StrokePoint &to, float &offset) -> Stroke {
        Stroke segmentDabs;
        const QVector2D posDelta = {to.pos.x() - from.pos.x(), to.pos.y() - from.pos.y()};
        const float steps = qMax(qMax(qAbs(posDelta.x()), qAbs(posDelta.y())), 1.0f);
        const float step = 1.0f / steps;
        float pos = offset * step;
        while (pos < 1.0f/* || qFuzzyCompare(pos, 1.0f)*/) {
            segmentDabs.append(StrokePoint::lerp(from, to, pos));
//                if (!stepOffsetOnly) {
//                }
            pos += step;
        }
    //    qDebug() << pos << (offset + qFloor(steps)) * step;
        offset = (pos - 1.0f) * steps;
        return segmentDabs;
    };

    if (mStroke.size() == 0) {
        // Render nothing
    }
    else if (mStroke.size() == 1) {
        // Draw single brush dab
        glBufferSubData(GL_UNIFORM_BUFFER, 0, GLsizeiptr(sizeof(StrokePoint)), &mStroke[0]);
        glBindBufferRange(GL_UNIFORM_BUFFER, strokeUniformBlockBinding, strokeUniformBuffer, 0, 1 * sizeof(StrokePoint));
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    }
    else {
        // Draw stroke
        float offset = 0.0f;
        int from = 0, to = 1;
        while (to < mStroke.size()) {
            const Stroke segmentDabs = generateSegmentDabs(mStroke[from], mStroke[to], offset);
            glBufferSubData(GL_UNIFORM_BUFFER, 0, GLsizeiptr(segmentDabs.size()) * GLsizeiptr(sizeof(StrokePoint)), segmentDabs.constData());
            for (int offset = 0; offset < segmentDabs.size(); offset += STROKE_SIZE_MAX) {
                glBindBufferRange(GL_UNIFORM_BUFFER, strokeUniformBlockBinding, strokeUniformBuffer, GLintptr(offset) * GLintptr(sizeof(StrokePoint)), STROKE_SIZE_MAX * sizeof(StrokePoint));
                glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, 4, GLsizei(qMin(GLuint(segmentDabs.size() - offset), STROKE_SIZE_MAX)));
            }
            ++from;
            ++to;
        }
    }

    if (mEditingContext.brush.type == Brush::Type::Image && !mEditingContext.brush.image.isNull()) {
        mBrushTexture->release();
    }
    mBrushProgram.release();
}

QRect StrokeRenderer::pointBounds(const StrokePoint &point) const
{
    return mEditingContext.brush.bounds(point.pos.toPointF(), Utils::lerp(mEditingContext.brushScalingMin, mEditingContext.brushScalingMax, point.pressure)).toAlignedRect();
}

QRect StrokeRenderer::segmentBounds(const StrokePoint &from, const StrokePoint &to) const
{
    return pointBounds(from).united(pointBounds(to));
}

QRect StrokeRenderer::strokeBounds(const Stroke &stroke) const
{
    QRect bounds;
    for (auto point : stroke) {
        bounds = bounds.united(pointBounds(point));
    }
    return bounds;
}
