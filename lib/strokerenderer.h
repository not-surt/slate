#ifndef STROKERENDERER_H
#define STROKERENDERER_H

#include "brush.h"
#include "stroke.h"
#include "editingcontext.h"

#include <QOpenGLFunctions>
//#include <QOpenGLExtraFunctions>
#include <QOffscreenSurface>
#include <QOpenGLTexture>
#include <QOpenGLFramebufferObject>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>

class StrokeRenderer : public QOpenGLFunctions/*QOpenGLExtraFunctions*/ {
public:
    StrokeRenderer();
    ~StrokeRenderer();

    QOffscreenSurface &surface();
    QOpenGLContext &context();

    QImage *image() const;
    void setImage(QImage *const image);

    const Stroke &stroke() const;
    void setStroke(const Stroke &brush);

    EditingContext &editingContext();
    const EditingContext &editingContext() const;
    void setEditingContext(const EditingContext &editingContext);

    void upload(const QRect &rect = QRect());
    void download(const QRect &rect = QRect());

    void renderBrush(const QTransform &transform);
    void render(const QTransform &transform, const QRect &clip = QRect());

    QRect brushBounds(const Brush &brush) const;
    QRect strokeBounds(const Stroke &stroke, const Brush &brush) const;

private:    
    static QVector<QVector2D> clipQuad;

    QOffscreenSurface mSurface;
    QOpenGLContext mContext;

    QOpenGLFramebufferObject *mStrokeFramebuffer;
    QOpenGLFramebufferObject *mImageFramebuffer;
    QImage *mImage;
    QOpenGLTexture *mBrushTexture;
    Stroke mStroke;
    QOpenGLBuffer *mStrokeVertexBuffer;
    EditingContext mEditingContext;

    QOpenGLShaderProgram mBrushProgram;
    QOpenGLShaderProgram mStrokeFramebufferCopyProgram;
    QOpenGLBuffer mClipQuadVertexBuffer;

    typedef void (ProcGlDrawArraysInstanced)(GLenum mode, GLint first, GLsizei count, GLsizei instancecount);
    ProcGlDrawArraysInstanced *glDrawArraysInstanced;
    typedef void (ProcGlVertexAttribDivisor)(GLuint index, GLuint divisor);
    ProcGlVertexAttribDivisor *glVertexAttribDivisor;
};

#endif // STROKERENDERER_H
