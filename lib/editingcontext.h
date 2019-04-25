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
    virtual ~EditingContext();

    bool operator==(const EditingContext &other) const;
    bool operator!=(const EditingContext &other) const;
    EditingContext &operator=(const EditingContext &other);

    BrushManager *mBrush;

    StrokeMode mStrokeMode;
    qreal mStrokeProportionalSpacing;
    qreal mStrokeAbsoluteSpacing;
    int mStrokeCount;
    qreal mStrokeTime;

    ScalingMode mBrushScalingMode;
    qreal mBrushScalingMin;
    qreal mBrushScalingMax;
    RotationMode mBrushRotationMode;
    qreal mBrushRotationMin;
    qreal mBrushRotationMax;

    ScalingMode mBrushHardnessMode;
    qreal mBrushHardnessMin;
    qreal mBrushHardnessMax;
    ScalingMode mBrushOpacityMode;
    qreal mBrushOpacityMin;
    qreal mBrushOpacityMax;

    ScalingMode mDynamicsProportionalSpacingMode;
    qreal mDynamicsProportionalSpacingMin;
    qreal mDynamicsProportionalSpacingMax;
    ScalingMode mDynamicsAbsoluteSpacingMode;
    qreal mDynamicsAbsoluteSpacingMin;
    qreal mDynamicsAbsoluteSpacingMax;

    BlendMode mBlendMode;
    QColor mForegroundColour;
    QColor mBackgroundColour;
};

class SLATE_EXPORT EditingContextManager : public QObject
{
    Q_OBJECT

    // Brush
    Q_PROPERTY(BrushManager *brush READ brush WRITE setBrush NOTIFY brushChanged)

    // Stroke
    Q_PROPERTY(StrokeMode strokeMode READ strokeMode WRITE setStrokeMode NOTIFY strokeModeChanged)
    Q_PROPERTY(qreal strokeProportionalSpacing READ strokeProportionalSpacing WRITE setStrokeProportionalSpacing NOTIFY strokeProportionalSpacingChanged)
    Q_PROPERTY(qreal strokeAbsoluteSpacing READ strokeAbsoluteSpacing WRITE setStrokeAbsoluteSpacing NOTIFY strokeAbsoluteSpacingChanged)
    Q_PROPERTY(int strokeCount READ strokeCount WRITE setStrokeCount NOTIFY strokeCountChanged)
    Q_PROPERTY(qreal strokeTime READ strokeTime WRITE setStrokeTime NOTIFY strokeTimeChanged)

    // Dynamics
    Q_PROPERTY(ScalingMode brushScalingMode READ brushScalingMode WRITE setBrushScalingMode NOTIFY brushScalingModeChanged)
    Q_PROPERTY(qreal brushScalingMin READ brushScalingMin WRITE setBrushScalingMin NOTIFY brushScalingMinChanged)
    Q_PROPERTY(qreal brushScalingMax READ brushScalingMax WRITE setBrushScalingMax NOTIFY brushScalingMaxChanged)
    Q_PROPERTY(RotationMode brushRotationMode READ brushRotationMode WRITE setBrushRotationMode NOTIFY brushRotationModeChanged)
    Q_PROPERTY(qreal brushRotationMin READ brushRotationMin WRITE setBrushRotationMin NOTIFY brushRotationMinChanged)
    Q_PROPERTY(qreal brushRotationMax READ brushRotationMax WRITE setBrushRotationMax NOTIFY brushRotationMaxChanged)

    Q_PROPERTY(ScalingMode brushHardnessMode READ brushHardnessMode WRITE setBrushHardnessMode NOTIFY brushHardnessModeChanged)
    Q_PROPERTY(qreal brushHardnessMin READ brushHardnessMin WRITE setBrushHardnessMin NOTIFY brushHardnessMinChanged)
    Q_PROPERTY(qreal brushHardnessMax READ brushHardnessMax WRITE setBrushHardnessMax NOTIFY brushHardnessMaxChanged)
    Q_PROPERTY(ScalingMode brushOpacityMode READ brushOpacityMode WRITE setBrushOpacityMode NOTIFY brushOpacityModeChanged)
    Q_PROPERTY(qreal brushOpacityMin READ brushOpacityMin WRITE setBrushOpacityMin NOTIFY brushOpacityMinChanged)
    Q_PROPERTY(qreal brushOpacityMax READ brushOpacityMax WRITE setBrushOpacityMax NOTIFY brushOpacityMaxChanged)

    Q_PROPERTY(ScalingMode dynamicsProportionalSpacingMode READ dynamicsProportionalSpacingMode WRITE setDynamicsProportionalSpacingMode NOTIFY dynamicsProportionalSpacingModeChanged)
    Q_PROPERTY(qreal dynamicsProportionalSpacingMin READ dynamicsProportionalSpacingMin WRITE setDynamicsProportionalSpacingMin NOTIFY dynamicsProportionalSpacingMinChanged)
    Q_PROPERTY(qreal dynamicsProportionalSpacingMax READ dynamicsProportionalSpacingMax WRITE setDynamicsProportionalSpacingMax NOTIFY dynamicsProportionalSpacingMaxChanged)
    Q_PROPERTY(ScalingMode dynamicsAbsoluteSpacingMode READ dynamicsAbsoluteSpacingMode WRITE setDynamicsAbsoluteSpacingMode NOTIFY dynamicsAbsoluteSpacingModeChanged)
    Q_PROPERTY(qreal dynamicsAbsoluteSpacingMin READ dynamicsAbsoluteSpacingMin WRITE setDynamicsAbsoluteSpacingMin NOTIFY dynamicsAbsoluteSpacingMinChanged)
    Q_PROPERTY(qreal dynamicsAbsoluteSpacingMax READ dynamicsAbsoluteSpacingMax WRITE setDynamicsAbsoluteSpacingMax NOTIFY dynamicsAbsoluteSpacingMaxChanged)

    // Colour
    Q_PROPERTY(BlendMode blendMode READ blendMode WRITE setBlendMode NOTIFY blendModeChanged)
    Q_PROPERTY(QColor foregroundColour READ foregroundColour WRITE setForegroundColour NOTIFY foregroundColourChanged)
    Q_PROPERTY(QColor backgroundColour READ backgroundColour WRITE setBackgroundColour NOTIFY backgroundColourChanged)

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

    EditingContextManager();
    EditingContextManager(const EditingContextManager &other);
    virtual ~EditingContextManager();

    bool operator==(const EditingContextManager &other) const;
    bool operator!=(const EditingContextManager &other) const;
    EditingContextManager &operator=(const EditingContextManager &other);

    BrushManager *brush() const;

    StrokeMode strokeMode() const;
    qreal strokeProportionalSpacing() const;
    qreal strokeAbsoluteSpacing() const;
    int strokeCount() const;
    qreal strokeTime() const;

    ScalingMode brushScalingMode() const;
    qreal brushScalingMin() const;
    qreal brushScalingMax() const;
    RotationMode brushRotationMode() const;
    qreal brushRotationMin() const;
    qreal brushRotationMax() const;

    ScalingMode brushHardnessMode() const;
    qreal brushHardnessMin() const;
    qreal brushHardnessMax() const;
    ScalingMode brushOpacityMode() const;
    qreal brushOpacityMin() const;
    qreal brushOpacityMax() const;

    ScalingMode dynamicsProportionalSpacingMode() const;
    qreal dynamicsProportionalSpacingMin() const;
    qreal dynamicsProportionalSpacingMax() const;
    ScalingMode dynamicsAbsoluteSpacingMode() const;
    qreal dynamicsAbsoluteSpacingMin() const;
    qreal dynamicsAbsoluteSpacingMax() const;

    BlendMode blendMode() const;
    QColor foregroundColour() const;
    QColor backgroundColour() const;

public slots:
    void setBrush(BrushManager *const brush);

    void setStrokeMode(StrokeMode strokeMode);
    void setStrokeProportionalSpacing(qreal strokeProportionalSpacing);
    void setStrokeAbsoluteSpacing(qreal strokeAbsoluteSpacing);
    void setStrokeCount(int strokeCount);
    void setStrokeTime(qreal strokeTime);

    void setBrushScalingMode(ScalingMode brushScalingMode);
    void setBrushScalingMin(qreal brushScalingMin);
    void setBrushScalingMax(qreal brushScalingMax);
    void setBrushRotationMode(RotationMode brushRotationMode);
    void setBrushRotationMin(qreal brushRotationMin);
    void setBrushRotationMax(qreal brushRotationMax);

    void setBrushHardnessMode(ScalingMode brushHardnessMode);
    void setBrushHardnessMin(qreal brushHardnessMin);
    void setBrushHardnessMax(qreal brushHardnessMax);
    void setBrushOpacityMode(ScalingMode brushOpacityMode);
    void setBrushOpacityMin(qreal brushOpacityMin);
    void setBrushOpacityMax(qreal brushOpacityMax);

    void setDynamicsProportionalSpacingMode(ScalingMode dynamicsProportionalSpacingMode);
    void setDynamicsProportionalSpacingMin(qreal dynamicsProportionalSpacingMin);
    void setDynamicsProportionalSpacingMax(qreal dynamicsProportionalSpacingMax);
    void setDynamicsAbsoluteSpacingMode(ScalingMode dynamicsAbsoluteSpacingMode);
    void setDynamicsAbsoluteSpacingMin(qreal dynamicsAbsoluteSpacingMin);
    void setDynamicsAbsoluteSpacingMax(qreal dynamicsAbsoluteSpacingMax);

    void setBlendMode(BlendMode blendMode);
    void setForegroundColour(QColor foregroundColour);
    void setBackgroundColour(QColor backgroundColour);

signals:
    void brushChanged(BrushManager *const brush);

    void strokeModeChanged(StrokeMode strokeMode);
    void strokeProportionalSpacingChanged(qreal strokeProportionalSpacing);
    void strokeAbsoluteSpacingChanged(qreal strokeAbsoluteSpacing);
    void strokeCountChanged(int strokeCount);
    void strokeTimeChanged(qreal strokeTime);

    void brushScalingModeChanged(ScalingMode brushScalingMode);
    void brushScalingMinChanged(qreal brushScalingMin);
    void brushScalingMaxChanged(qreal brushScalingMax);
    void brushRotationModeChanged(RotationMode brushRotationMode);
    void brushRotationMinChanged(qreal brushRotationMin);
    void brushRotationMaxChanged(qreal brushRotationMax);

    void brushHardnessModeChanged(ScalingMode brushHardnessMode);
    void brushHardnessMinChanged(qreal brushHardnessMin);
    void brushHardnessMaxChanged(qreal brushHardnessMax);
    void brushOpacityModeChanged(ScalingMode brushOpacityMode);
    void brushOpacityMinChanged(qreal brushOpacityMin);
    void brushOpacityMaxChanged(qreal brushOpacityMax);

    void dynamicsProportionalSpacingModeChanged(ScalingMode dynamicsProportionalSpacingMode);
    void dynamicsProportionalSpacingMinChanged(qreal dynamicsProportionalSpacingMin);
    void dynamicsProportionalSpacingMaxChanged(qreal dynamicsProportionalSpacingMax);
    void dynamicsAbsoluteSpacingModeChanged(ScalingMode dynamicsAbsoluteSpacingMode);
    void dynamicsAbsoluteSpacingMinChanged(qreal dynamicsAbsoluteSpacingMin);
    void dynamicsAbsoluteSpacingMaxChanged(qreal dynamicsAbsoluteSpacingMax);

    void blendModeChanged(BlendMode blendMode);
    void foregroundColourChanged(QColor foregroundColour);
    void backgroundColourChanged(QColor backgroundColour);

protected:
    EditingContext mEditingContext;

    BrushManager *mBrush;

    StrokeMode mStrokeMode;
    qreal mStrokeProportionalSpacing;
    qreal mStrokeAbsoluteSpacing;
    int mStrokeCount;
    qreal mStrokeTime;

    ScalingMode mBrushScalingMode;
    qreal mBrushScalingMin;
    qreal mBrushScalingMax;
    RotationMode mBrushRotationMode;
    qreal mBrushRotationMin;
    qreal mBrushRotationMax;

    ScalingMode mBrushHardnessMode;
    qreal mBrushHardnessMin;
    qreal mBrushHardnessMax;
    ScalingMode mBrushOpacityMode;
    qreal mBrushOpacityMin;
    qreal mBrushOpacityMax;

    ScalingMode mDynamicsProportionalSpacingMode;
    qreal mDynamicsProportionalSpacingMin;
    qreal mDynamicsProportionalSpacingMax;
    ScalingMode mDynamicsAbsoluteSpacingMode;
    qreal mDynamicsAbsoluteSpacingMin;
    qreal mDynamicsAbsoluteSpacingMax;

    BlendMode mBlendMode;
    QColor mForegroundColour;
    QColor mBackgroundColour;
};

#endif // EDITINGCONTEXT_H
