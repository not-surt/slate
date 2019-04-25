#ifndef STROKERENDERER_H
#define STROKERENDERER_H

#include "brush.h"
#include "stroke.h"
#include "editingcontext.h"

#include <QOpenGLFunctions>
#include <QOffscreenSurface>
#include <QOpenGLTexture>
#include <QOpenGLFramebufferObject>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>

class StrokeRenderer : public QOpenGLFunctions {
public:
    StrokeRenderer();
    ~StrokeRenderer();

    QOffscreenSurface &surface();
    QOpenGLContext &context();

    QImage *image() const;
    void setImage(QImage *const image);

    const Stroke &stroke() const;
    void setStroke(const Stroke &brush);

    EditingContextManager &editingContext();
    const EditingContextManager &editingContext() const;
    void setEditingContext(const EditingContextManager &editingContext);

    void upload(const QRect &rect = QRect());
    void download(const QRect &rect = QRect());

    void renderBrush(const QTransform &transform);
    void render(const QTransform &transform, const QRect &clip = QRect());

    QRect brushBounds(const BrushManager &brush) const;
    QRect strokeBounds(const Stroke &stroke, const BrushManager &brush) const;

private:    
    static QVector<QVector2D> clipQuad;

    QOffscreenSurface mSurface;
    QOpenGLContext mContext;

    QOpenGLFramebufferObject *mFramebuffer;
    QImage *mImage;
    QOpenGLTexture *mBrushTexture;
    Stroke mStroke;
    QOpenGLBuffer *mStrokeVertexBuffer;
    EditingContextManager mEditingContext;

    QOpenGLShaderProgram mProgram;
    QOpenGLBuffer mUnitQuadVertexBuffer;
};

#endif // STROKERENDERER_H
