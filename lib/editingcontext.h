#ifndef EDITINGCONTEXT_H
#define EDITINGCONTEXT_H

#include <QObject>
#include <QVector>
#include <QColor>
#include <QTransform>

#include "brush.h"
#include "stroke.h"

class ImageCanvas;

struct SLATE_EXPORT EditingContext {
    Q_GADGET

public:
    enum class StrokeMode {
        Spaced,
        Counted,
        Timed,
    };
    Q_ENUM(StrokeMode)

    enum class ScalingMode {
        None,
        StylusPressure,
        StrokeTaper,
    };
    Q_ENUM(ScalingMode)

    enum class RotationMode {
        None,
        StylusRotation,
        StylusTilt,
        StrokeAngle,
    };
    Q_ENUM(RotationMode)

    enum class BlendMode {
        Blend,
        Replace,
        ReplaceColour,
        ReplaceAlpha,
        Add,
        Subtract,
        Multiply,
        Erase,
    };
    Q_ENUM(BlendMode)

    EditingContext();
    EditingContext(const EditingContext &other);

    bool operator==(const EditingContext &other) const;
    bool operator!=(const EditingContext &other) const;
    EditingContext &operator=(const EditingContext &other);

    Brush brush;

    StrokeMode strokeMode;
    qreal strokeProportionalSpacing;
    qreal strokeAbsoluteSpacing;
    int strokeCount;
    qreal strokeTime;

    ScalingMode brushScalingMode;
    qreal brushScalingMin;
    qreal brushScalingMax;
    RotationMode brushRotationMode;
    qreal brushRotationMin;
    qreal brushRotationMax;

    ScalingMode brushHardnessMode;
    qreal brushHardnessMin;
    qreal brushHardnessMax;
    ScalingMode brushOpacityMode;
    qreal brushOpacityMin;
    qreal brushOpacityMax;

    ScalingMode dynamicsProportionalSpacingMode;
    qreal dynamicsProportionalSpacingMin;
    qreal dynamicsProportionalSpacingMax;
    ScalingMode dynamicsAbsoluteSpacingMode;
    qreal dynamicsAbsoluteSpacingMin;
    qreal dynamicsAbsoluteSpacingMax;

    BlendMode blendMode;
    QColor foregroundColour;
    QColor backgroundColour;
};

class SLATE_EXPORT EditingContextManager : public QObject
{
    Q_OBJECT

    // Brush
    Q_PROPERTY(BrushManager *brushManager READ brushManager NOTIFY brushManagerChanged)

    // Stroke
    Q_PROPERTY(EditingContext::StrokeMode strokeMode READ strokeMode WRITE setStrokeMode NOTIFY strokeModeChanged)
    Q_PROPERTY(qreal strokeProportionalSpacing READ strokeProportionalSpacing WRITE setStrokeProportionalSpacing NOTIFY strokeProportionalSpacingChanged)
    Q_PROPERTY(qreal strokeAbsoluteSpacing READ strokeAbsoluteSpacing WRITE setStrokeAbsoluteSpacing NOTIFY strokeAbsoluteSpacingChanged)
    Q_PROPERTY(int strokeCount READ strokeCount WRITE setStrokeCount NOTIFY strokeCountChanged)
    Q_PROPERTY(qreal strokeTime READ strokeTime WRITE setStrokeTime NOTIFY strokeTimeChanged)

    // Dynamics
    Q_PROPERTY(EditingContext::ScalingMode brushScalingMode READ brushScalingMode WRITE setBrushScalingMode NOTIFY brushScalingModeChanged)
    Q_PROPERTY(qreal brushScalingMin READ brushScalingMin WRITE setBrushScalingMin NOTIFY brushScalingMinChanged)
    Q_PROPERTY(qreal brushScalingMax READ brushScalingMax WRITE setBrushScalingMax NOTIFY brushScalingMaxChanged)
    Q_PROPERTY(EditingContext::RotationMode brushRotationMode READ brushRotationMode WRITE setBrushRotationMode NOTIFY brushRotationModeChanged)
    Q_PROPERTY(qreal brushRotationMin READ brushRotationMin WRITE setBrushRotationMin NOTIFY brushRotationMinChanged)
    Q_PROPERTY(qreal brushRotationMax READ brushRotationMax WRITE setBrushRotationMax NOTIFY brushRotationMaxChanged)

    Q_PROPERTY(EditingContext::ScalingMode brushHardnessMode READ brushHardnessMode WRITE setBrushHardnessMode NOTIFY brushHardnessModeChanged)
    Q_PROPERTY(qreal brushHardnessMin READ brushHardnessMin WRITE setBrushHardnessMin NOTIFY brushHardnessMinChanged)
    Q_PROPERTY(qreal brushHardnessMax READ brushHardnessMax WRITE setBrushHardnessMax NOTIFY brushHardnessMaxChanged)
    Q_PROPERTY(EditingContext::ScalingMode brushOpacityMode READ brushOpacityMode WRITE setBrushOpacityMode NOTIFY brushOpacityModeChanged)
    Q_PROPERTY(qreal brushOpacityMin READ brushOpacityMin WRITE setBrushOpacityMin NOTIFY brushOpacityMinChanged)
    Q_PROPERTY(qreal brushOpacityMax READ brushOpacityMax WRITE setBrushOpacityMax NOTIFY brushOpacityMaxChanged)

    Q_PROPERTY(EditingContext::ScalingMode dynamicsProportionalSpacingMode READ dynamicsProportionalSpacingMode WRITE setDynamicsProportionalSpacingMode NOTIFY dynamicsProportionalSpacingModeChanged)
    Q_PROPERTY(qreal dynamicsProportionalSpacingMin READ dynamicsProportionalSpacingMin WRITE setDynamicsProportionalSpacingMin NOTIFY dynamicsProportionalSpacingMinChanged)
    Q_PROPERTY(qreal dynamicsProportionalSpacingMax READ dynamicsProportionalSpacingMax WRITE setDynamicsProportionalSpacingMax NOTIFY dynamicsProportionalSpacingMaxChanged)
    Q_PROPERTY(EditingContext::ScalingMode dynamicsAbsoluteSpacingMode READ dynamicsAbsoluteSpacingMode WRITE setDynamicsAbsoluteSpacingMode NOTIFY dynamicsAbsoluteSpacingModeChanged)
    Q_PROPERTY(qreal dynamicsAbsoluteSpacingMin READ dynamicsAbsoluteSpacingMin WRITE setDynamicsAbsoluteSpacingMin NOTIFY dynamicsAbsoluteSpacingMinChanged)
    Q_PROPERTY(qreal dynamicsAbsoluteSpacingMax READ dynamicsAbsoluteSpacingMax WRITE setDynamicsAbsoluteSpacingMax NOTIFY dynamicsAbsoluteSpacingMaxChanged)

    // Colour
    Q_PROPERTY(EditingContext::BlendMode blendMode READ blendMode WRITE setBlendMode NOTIFY blendModeChanged)
    Q_PROPERTY(QColor foregroundColour READ foregroundColour WRITE setForegroundColour NOTIFY foregroundColourChanged)
    Q_PROPERTY(QColor backgroundColour READ backgroundColour WRITE setBackgroundColour NOTIFY backgroundColourChanged)

public:
    EditingContextManager(EditingContext *const editingContext = nullptr);
    EditingContextManager(const EditingContextManager &other);
    virtual ~EditingContextManager();

    bool operator==(const EditingContextManager &other) const;
    bool operator!=(const EditingContextManager &other) const;
    EditingContextManager &operator=(const EditingContextManager &other);

    EditingContext *editingContext();

    BrushManager *brushManager();

    const Brush &brush() const;

    EditingContext::StrokeMode strokeMode() const;
    qreal strokeProportionalSpacing() const;
    qreal strokeAbsoluteSpacing() const;
    int strokeCount() const;
    qreal strokeTime() const;

    EditingContext::ScalingMode brushScalingMode() const;
    qreal brushScalingMin() const;
    qreal brushScalingMax() const;
    EditingContext::RotationMode brushRotationMode() const;
    qreal brushRotationMin() const;
    qreal brushRotationMax() const;

    EditingContext::ScalingMode brushHardnessMode() const;
    qreal brushHardnessMin() const;
    qreal brushHardnessMax() const;
    EditingContext::ScalingMode brushOpacityMode() const;
    qreal brushOpacityMin() const;
    qreal brushOpacityMax() const;

    EditingContext::ScalingMode dynamicsProportionalSpacingMode() const;
    qreal dynamicsProportionalSpacingMin() const;
    qreal dynamicsProportionalSpacingMax() const;
    EditingContext::ScalingMode dynamicsAbsoluteSpacingMode() const;
    qreal dynamicsAbsoluteSpacingMin() const;
    qreal dynamicsAbsoluteSpacingMax() const;

    EditingContext::BlendMode blendMode() const;
    QColor foregroundColour() const;
    QColor backgroundColour() const;

public slots:
    void setEditingContext(EditingContext *const editingContext);

    void setBrush(const Brush &brush);

    void setStrokeMode(EditingContext::StrokeMode strokeMode);
    void setStrokeProportionalSpacing(qreal strokeProportionalSpacing);
    void setStrokeAbsoluteSpacing(qreal strokeAbsoluteSpacing);
    void setStrokeCount(int strokeCount);
    void setStrokeTime(qreal strokeTime);

    void setBrushScalingMode(EditingContext::ScalingMode brushScalingMode);
    void setBrushScalingMin(qreal brushScalingMin);
    void setBrushScalingMax(qreal brushScalingMax);
    void setBrushRotationMode(EditingContext::RotationMode brushRotationMode);
    void setBrushRotationMin(qreal brushRotationMin);
    void setBrushRotationMax(qreal brushRotationMax);

    void setBrushHardnessMode(EditingContext::ScalingMode brushHardnessMode);
    void setBrushHardnessMin(qreal brushHardnessMin);
    void setBrushHardnessMax(qreal brushHardnessMax);
    void setBrushOpacityMode(EditingContext::ScalingMode brushOpacityMode);
    void setBrushOpacityMin(qreal brushOpacityMin);
    void setBrushOpacityMax(qreal brushOpacityMax);

    void setDynamicsProportionalSpacingMode(EditingContext::ScalingMode dynamicsProportionalSpacingMode);
    void setDynamicsProportionalSpacingMin(qreal dynamicsProportionalSpacingMin);
    void setDynamicsProportionalSpacingMax(qreal dynamicsProportionalSpacingMax);
    void setDynamicsAbsoluteSpacingMode(EditingContext::ScalingMode dynamicsAbsoluteSpacingMode);
    void setDynamicsAbsoluteSpacingMin(qreal dynamicsAbsoluteSpacingMin);
    void setDynamicsAbsoluteSpacingMax(qreal dynamicsAbsoluteSpacingMax);

    void setBlendMode(EditingContext::BlendMode blendMode);
    void setForegroundColour(QColor foregroundColour);
    void setBackgroundColour(QColor backgroundColour);

signals:
    void editingContextChanged();

    void brushManagerChanged();

    void brushChanged();

    void strokeModeChanged(EditingContext::StrokeMode strokeMode);
    void strokeProportionalSpacingChanged(qreal strokeProportionalSpacing);
    void strokeAbsoluteSpacingChanged(qreal strokeAbsoluteSpacing);
    void strokeCountChanged(int strokeCount);
    void strokeTimeChanged(qreal strokeTime);

    void brushScalingModeChanged(EditingContext::ScalingMode brushScalingMode);
    void brushScalingMinChanged(qreal brushScalingMin);
    void brushScalingMaxChanged(qreal brushScalingMax);
    void brushRotationModeChanged(EditingContext::RotationMode brushRotationMode);
    void brushRotationMinChanged(qreal brushRotationMin);
    void brushRotationMaxChanged(qreal brushRotationMax);

    void brushHardnessModeChanged(EditingContext::ScalingMode brushHardnessMode);
    void brushHardnessMinChanged(qreal brushHardnessMin);
    void brushHardnessMaxChanged(qreal brushHardnessMax);
    void brushOpacityModeChanged(EditingContext::ScalingMode brushOpacityMode);
    void brushOpacityMinChanged(qreal brushOpacityMin);
    void brushOpacityMaxChanged(qreal brushOpacityMax);

    void dynamicsProportionalSpacingModeChanged(EditingContext::ScalingMode dynamicsProportionalSpacingMode);
    void dynamicsProportionalSpacingMinChanged(qreal dynamicsProportionalSpacingMin);
    void dynamicsProportionalSpacingMaxChanged(qreal dynamicsProportionalSpacingMax);
    void dynamicsAbsoluteSpacingModeChanged(EditingContext::ScalingMode dynamicsAbsoluteSpacingMode);
    void dynamicsAbsoluteSpacingMinChanged(qreal dynamicsAbsoluteSpacingMin);
    void dynamicsAbsoluteSpacingMaxChanged(qreal dynamicsAbsoluteSpacingMax);

    void blendModeChanged(EditingContext::BlendMode blendMode);
    void foregroundColourChanged(QColor foregroundColour);
    void backgroundColourChanged(QColor backgroundColour);

protected:
    EditingContext *mEditingContext;
    BrushManager mBrushManager;
};

#endif // EDITINGCONTEXT_H
