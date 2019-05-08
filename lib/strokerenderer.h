#ifndef STROKERENDERER_H
#define STROKERENDERER_H

#include "brush.h"
#include "stroke.h"
#include "editingcontext.h"
#include "slate-global.h"

#include <QOpenGLExtraFunctions>
#include <QOffscreenSurface>
#include <QOpenGLTexture>
#include <QOpenGLFramebufferObject>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>

class SLATE_EXPORT StrokeRenderer : public QOpenGLExtraFunctions {
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

    void prerender(const QRect &rect = QRect());
    void postrender(const QRect &rect = QRect());

    void render(const QTransform &transform, const QRect &rect = QRect());

    QRect pointBounds(const StrokePoint &point) const;
    QRect segmentBounds(const StrokePoint &from, const StrokePoint &to) const;
    QRect strokeBounds(const Stroke &stroke) const;

private:    
    static QVector<QVector2D> clipQuad;

    QOffscreenSurface mSurface;
    QOpenGLContext mContext;

    QImage *mImage;
    QOpenGLTexture *mBrushTexture;
    Stroke mStroke;
    EditingContext mEditingContext;

    QOpenGLShaderProgram mBrushProgram;
    QOpenGLShaderProgram mStrokeFramebufferCopyProgram;

    GLuint strokeFramebuffer, imageFramebuffer;
    GLuint strokeTexture, imageTexture, depthStencilTexture;
    GLuint strokeUniformBuffer;

    QByteArray shaderVersionString() {
        if (mContext.isOpenGLES()) return QByteArrayLiteral(
                    "#version 300 es\n"
                    "precision highp float;\n"
                    "precision highp int;\n"
                    );
        else return QByteArrayLiteral(
                    "#version 330 core\n"
                    );
    }

    const GLuint STROKE_SIZE_MAX = 4096;
    const GLuint strokeUniformBlockBinding = 0;
};

#endif // STROKERENDERER_H
