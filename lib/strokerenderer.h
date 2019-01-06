#ifndef STROKERENDERER_H
#define STROKERENDERER_H

#include "brush.h"
#include "stroke.h"

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

    void updateBrushTexture(const QImage &image = QImage());

    void upload(const QRect &rect = QRect());
    void download(const QRect &rect = QRect());

    void renderBrush(const Brush &brush, const QColor &colour, const QTransform &transform, const QRect &rect = QRect());
    void render(const Stroke &stroke, const Brush &brush, const QColor &colour, const qreal scaleMin, const qreal scaleMax, const qreal opacityMin, const qreal opacityMax, const qreal hardnessMin, const qreal hardnessMax, const bool singleColour, const QTransform &transform, const QRect &rect = QRect());

private:
    static QVector<QVector2D> clipQuad;

    QOffscreenSurface mSurface;
    QOpenGLContext mContext;

    QOpenGLFramebufferObject *mFramebuffer;
    QImage *mImage;
    QOpenGLTexture *mBrushTexture;
    QSize mBrushTextureSize;

    QOpenGLShaderProgram mProgram;
    QOpenGLBuffer mVertexBuffer;
};

#endif // STROKERENDERER_H
