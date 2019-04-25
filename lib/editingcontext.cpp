#include "editingcontext.h"

EditingContext::EditingContext() :
    mBrush(new BrushManager),
    mStrokeMode(StrokeMode::Spaced),
    mStrokeProportionalSpacing(0.25),
    mStrokeAbsoluteSpacing(0),
    mStrokeCount(1),
    mStrokeTime(1),

    mBrushScalingMode(ScalingMode::None),
    mBrushScalingMin(0.0),
    mBrushScalingMax(1.0),
    mBrushRotationMode(RotationMode::None),
    mBrushRotationMin(0.0),
    mBrushRotationMax(1.0),
    mBrushHardnessMode(ScalingMode::None),
    mBrushHardnessMin(0.0),
    mBrushHardnessMax(1.0),
    mBrushOpacityMode(ScalingMode::None),
    mBrushOpacityMin(0.0),
    mBrushOpacityMax(1.0),

    mDynamicsProportionalSpacingMode(ScalingMode::None),
    mDynamicsProportionalSpacingMin(0.0),
    mDynamicsProportionalSpacingMax(1.0),
    mDynamicsAbsoluteSpacingMode(ScalingMode::None),
    mDynamicsAbsoluteSpacingMin(0.0),
    mDynamicsAbsoluteSpacingMax(1.0),

    mBlendMode(BlendMode::Blend),
    mForegroundColour(0, 0, 0, 255),
    mBackgroundColour(255, 255, 255, 255)
{
}

EditingContext::EditingContext(const EditingContext &other)
{
    mBrush = new BrushManager(*other.mBrush);
    mStrokeMode = other.mStrokeMode;
    mStrokeProportionalSpacing = other.mStrokeProportionalSpacing;
    mStrokeAbsoluteSpacing = other.mStrokeAbsoluteSpacing;
    mStrokeCount = other.mStrokeCount;
    mStrokeTime = other.mStrokeTime;

    mBrushScalingMode = other.mBrushScalingMode;
    mBrushScalingMin = other.mBrushScalingMin;
    mBrushScalingMax = other.mBrushScalingMax;
    mBrushRotationMode = other.mBrushRotationMode;
    mBrushRotationMin = other.mBrushRotationMin;
    mBrushRotationMax = other.mBrushRotationMax;
    mBrushHardnessMode = other.mBrushHardnessMode;
    mBrushHardnessMin = other.mBrushHardnessMin;
    mBrushHardnessMax = other.mBrushHardnessMax;
    mBrushOpacityMode = other.mBrushOpacityMode;
    mBrushOpacityMin = other.mBrushOpacityMin;
    mBrushOpacityMax = other.mBrushOpacityMax;

    mDynamicsProportionalSpacingMode = other.mDynamicsProportionalSpacingMode;
    mDynamicsProportionalSpacingMin = other.mDynamicsProportionalSpacingMin;
    mDynamicsProportionalSpacingMax = other.mDynamicsProportionalSpacingMax;
    mDynamicsAbsoluteSpacingMode = other.mDynamicsAbsoluteSpacingMode;
    mDynamicsAbsoluteSpacingMin = other.mDynamicsAbsoluteSpacingMin;
    mDynamicsAbsoluteSpacingMax = other.mDynamicsAbsoluteSpacingMax;

    mBlendMode = other.mBlendMode;
    mForegroundColour = other.mForegroundColour;
    mBackgroundColour = other.mBackgroundColour;
}

EditingContext::~EditingContext()
{
    delete mBrush;
}

bool EditingContext::operator==(const EditingContext &other) const
{
    return *mBrush == *other.mBrush &&
        mStrokeMode == other.mStrokeMode &&
        qFuzzyCompare(mStrokeProportionalSpacing, other.mStrokeProportionalSpacing) &&
        qFuzzyCompare(mStrokeAbsoluteSpacing, other.mStrokeAbsoluteSpacing) &&
        mStrokeCount == other.mStrokeCount &&
        qFuzzyCompare(mStrokeTime, other.mStrokeTime) &&

        mBrushScalingMode == other.mBrushScalingMode &&
        qFuzzyCompare(mBrushScalingMin, other.mBrushScalingMin) &&
        qFuzzyCompare(mBrushScalingMax, other.mBrushScalingMax) &&
        mBrushRotationMode == other.mBrushRotationMode &&
        qFuzzyCompare(mBrushRotationMin, other.mBrushRotationMin) &&
        qFuzzyCompare(mBrushRotationMax, other.mBrushRotationMax) &&
        mBrushHardnessMode == other.mBrushHardnessMode &&
        qFuzzyCompare(mBrushHardnessMin, other.mBrushHardnessMin) &&
        qFuzzyCompare(mBrushHardnessMax, other.mBrushHardnessMax) &&
        mBrushOpacityMode == other.mBrushOpacityMode &&
        qFuzzyCompare(mBrushOpacityMin, other.mBrushOpacityMin) &&
        qFuzzyCompare(mBrushOpacityMax, other.mBrushOpacityMax) &&

        mDynamicsProportionalSpacingMode == other.mDynamicsProportionalSpacingMode &&
        qFuzzyCompare(mDynamicsProportionalSpacingMin, other.mDynamicsProportionalSpacingMin) &&
        qFuzzyCompare(mDynamicsProportionalSpacingMax, other.mDynamicsProportionalSpacingMax) &&
        mDynamicsAbsoluteSpacingMode == other.mDynamicsAbsoluteSpacingMode &&
        qFuzzyCompare(mDynamicsAbsoluteSpacingMin, other.mDynamicsAbsoluteSpacingMin) &&
        qFuzzyCompare(mDynamicsAbsoluteSpacingMax, other.mDynamicsAbsoluteSpacingMax) &&

        mBlendMode == other.mBlendMode &&
        mForegroundColour == other.mForegroundColour &&
        mBackgroundColour == other.mBackgroundColour;
}

bool EditingContext::operator!=(const EditingContext &other) const
{
    return !(*this == other);
}

EditingContext &EditingContext::operator=(const EditingContext &other)
{
    mBrush = new BrushManager(*other.mBrush);
    mStrokeMode = other.mStrokeMode;
    mStrokeProportionalSpacing = other.mStrokeProportionalSpacing;
    mStrokeAbsoluteSpacing = other.mStrokeAbsoluteSpacing;
    mStrokeCount = other.mStrokeCount;
    mStrokeTime = other.mStrokeTime;

    mBrushScalingMode = other.mBrushScalingMode;
    mBrushScalingMin = other.mBrushScalingMin;
    mBrushScalingMax = other.mBrushScalingMax;
    mBrushRotationMode = other.mBrushRotationMode;
    mBrushRotationMin = other.mBrushRotationMin;
    mBrushRotationMax = other.mBrushRotationMax;
    mBrushHardnessMode = other.mBrushHardnessMode;
    mBrushHardnessMin = other.mBrushHardnessMin;
    mBrushHardnessMax = other.mBrushHardnessMax;
    mBrushOpacityMode = other.mBrushOpacityMode;
    mBrushOpacityMin = other.mBrushOpacityMin;
    mBrushOpacityMax = other.mBrushOpacityMax;

    mDynamicsProportionalSpacingMode = other.mDynamicsProportionalSpacingMode;
    mDynamicsProportionalSpacingMin = other.mDynamicsProportionalSpacingMin;
    mDynamicsProportionalSpacingMax = other.mDynamicsProportionalSpacingMax;
    mDynamicsAbsoluteSpacingMode = other.mDynamicsAbsoluteSpacingMode;
    mDynamicsAbsoluteSpacingMin = other.mDynamicsAbsoluteSpacingMin;
    mDynamicsAbsoluteSpacingMax = other.mDynamicsAbsoluteSpacingMax;

    mBlendMode = other.mBlendMode;
    mForegroundColour = other.mForegroundColour;
    mBackgroundColour = other.mBackgroundColour;
    return *this;
}

EditingContextManager::EditingContextManager() :
    QObject(),

    mBrush(new BrushManager),
    mStrokeMode(StrokeMode::Spaced),
    mStrokeProportionalSpacing(0.25),
    mStrokeAbsoluteSpacing(0),
    mStrokeCount(1),
    mStrokeTime(1),

    mBrushScalingMode(ScalingMode::None),
    mBrushScalingMin(0.0),
    mBrushScalingMax(1.0),
    mBrushRotationMode(RotationMode::None),
    mBrushRotationMin(0.0),
    mBrushRotationMax(1.0),
    mBrushHardnessMode(ScalingMode::None),
    mBrushHardnessMin(0.0),
    mBrushHardnessMax(1.0),
    mBrushOpacityMode(ScalingMode::None),
    mBrushOpacityMin(0.0),
    mBrushOpacityMax(1.0),

    mDynamicsProportionalSpacingMode(ScalingMode::None),
    mDynamicsProportionalSpacingMin(0.0),
    mDynamicsProportionalSpacingMax(1.0),
    mDynamicsAbsoluteSpacingMode(ScalingMode::None),
    mDynamicsAbsoluteSpacingMin(0.0),
    mDynamicsAbsoluteSpacingMax(1.0),

    mBlendMode(BlendMode::Blend),
    mForegroundColour(0, 0, 0, 255),
    mBackgroundColour(255, 255, 255, 255)
{
}

EditingContextManager::EditingContextManager(const EditingContextManager &other) :
    QObject()
{
    mBrush = new BrushManager(*other.mBrush);
    mStrokeMode = other.mStrokeMode;
    mStrokeProportionalSpacing = other.mStrokeProportionalSpacing;
    mStrokeAbsoluteSpacing = other.mStrokeAbsoluteSpacing;
    mStrokeCount = other.mStrokeCount;
    mStrokeTime = other.mStrokeTime;

    mBrushScalingMode = other.mBrushScalingMode;
    mBrushScalingMin = other.mBrushScalingMin;
    mBrushScalingMax = other.mBrushScalingMax;
    mBrushRotationMode = other.mBrushRotationMode;
    mBrushRotationMin = other.mBrushRotationMin;
    mBrushRotationMax = other.mBrushRotationMax;
    mBrushHardnessMode = other.mBrushHardnessMode;
    mBrushHardnessMin = other.mBrushHardnessMin;
    mBrushHardnessMax = other.mBrushHardnessMax;
    mBrushOpacityMode = other.mBrushOpacityMode;
    mBrushOpacityMin = other.mBrushOpacityMin;
    mBrushOpacityMax = other.mBrushOpacityMax;

    mDynamicsProportionalSpacingMode = other.mDynamicsProportionalSpacingMode;
    mDynamicsProportionalSpacingMin = other.mDynamicsProportionalSpacingMin;
    mDynamicsProportionalSpacingMax = other.mDynamicsProportionalSpacingMax;
    mDynamicsAbsoluteSpacingMode = other.mDynamicsAbsoluteSpacingMode;
    mDynamicsAbsoluteSpacingMin = other.mDynamicsAbsoluteSpacingMin;
    mDynamicsAbsoluteSpacingMax = other.mDynamicsAbsoluteSpacingMax;

    mBlendMode = other.mBlendMode;
    mForegroundColour = other.mForegroundColour;
    mBackgroundColour = other.mBackgroundColour;
}

EditingContextManager::~EditingContextManager()
{
    delete mBrush;
}

bool EditingContextManager::operator==(const EditingContextManager &other) const
{
    return *mBrush == *other.mBrush &&
        mStrokeMode == other.mStrokeMode &&
        qFuzzyCompare(mStrokeProportionalSpacing, other.mStrokeProportionalSpacing) &&
        qFuzzyCompare(mStrokeAbsoluteSpacing, other.mStrokeAbsoluteSpacing) &&
        mStrokeCount == other.mStrokeCount &&
        qFuzzyCompare(mStrokeTime, other.mStrokeTime) &&

        mBrushScalingMode == other.mBrushScalingMode &&
        qFuzzyCompare(mBrushScalingMin, other.mBrushScalingMin) &&
        qFuzzyCompare(mBrushScalingMax, other.mBrushScalingMax) &&
        mBrushRotationMode == other.mBrushRotationMode &&
        qFuzzyCompare(mBrushRotationMin, other.mBrushRotationMin) &&
        qFuzzyCompare(mBrushRotationMax, other.mBrushRotationMax) &&
        mBrushHardnessMode == other.mBrushHardnessMode &&
        qFuzzyCompare(mBrushHardnessMin, other.mBrushHardnessMin) &&
        qFuzzyCompare(mBrushHardnessMax, other.mBrushHardnessMax) &&
        mBrushOpacityMode == other.mBrushOpacityMode &&
        qFuzzyCompare(mBrushOpacityMin, other.mBrushOpacityMin) &&
        qFuzzyCompare(mBrushOpacityMax, other.mBrushOpacityMax) &&

        mDynamicsProportionalSpacingMode == other.mDynamicsProportionalSpacingMode &&
        qFuzzyCompare(mDynamicsProportionalSpacingMin, other.mDynamicsProportionalSpacingMin) &&
        qFuzzyCompare(mDynamicsProportionalSpacingMax, other.mDynamicsProportionalSpacingMax) &&
        mDynamicsAbsoluteSpacingMode == other.mDynamicsAbsoluteSpacingMode &&
        qFuzzyCompare(mDynamicsAbsoluteSpacingMin, other.mDynamicsAbsoluteSpacingMin) &&
        qFuzzyCompare(mDynamicsAbsoluteSpacingMax, other.mDynamicsAbsoluteSpacingMax) &&

        mBlendMode == other.mBlendMode &&
        mForegroundColour == other.mForegroundColour &&
        mBackgroundColour == other.mBackgroundColour;
}

bool EditingContextManager::operator!=(const EditingContextManager &other) const
{
    return !(*this == other);
}

EditingContextManager &EditingContextManager::operator=(const EditingContextManager &other)
{
    mBrush = new BrushManager(*other.mBrush);
    mStrokeMode = other.mStrokeMode;
    mStrokeProportionalSpacing = other.mStrokeProportionalSpacing;
    mStrokeAbsoluteSpacing = other.mStrokeAbsoluteSpacing;
    mStrokeCount = other.mStrokeCount;
    mStrokeTime = other.mStrokeTime;

    mBrushScalingMode = other.mBrushScalingMode;
    mBrushScalingMin = other.mBrushScalingMin;
    mBrushScalingMax = other.mBrushScalingMax;
    mBrushRotationMode = other.mBrushRotationMode;
    mBrushRotationMin = other.mBrushRotationMin;
    mBrushRotationMax = other.mBrushRotationMax;
    mBrushHardnessMode = other.mBrushHardnessMode;
    mBrushHardnessMin = other.mBrushHardnessMin;
    mBrushHardnessMax = other.mBrushHardnessMax;
    mBrushOpacityMode = other.mBrushOpacityMode;
    mBrushOpacityMin = other.mBrushOpacityMin;
    mBrushOpacityMax = other.mBrushOpacityMax;

    mDynamicsProportionalSpacingMode = other.mDynamicsProportionalSpacingMode;
    mDynamicsProportionalSpacingMin = other.mDynamicsProportionalSpacingMin;
    mDynamicsProportionalSpacingMax = other.mDynamicsProportionalSpacingMax;
    mDynamicsAbsoluteSpacingMode = other.mDynamicsAbsoluteSpacingMode;
    mDynamicsAbsoluteSpacingMin = other.mDynamicsAbsoluteSpacingMin;
    mDynamicsAbsoluteSpacingMax = other.mDynamicsAbsoluteSpacingMax;

    mBlendMode = other.mBlendMode;
    mForegroundColour = other.mForegroundColour;
    mBackgroundColour = other.mBackgroundColour;
    return *this;
}

BrushManager *EditingContextManager::brush() const
{
    return mBrush;
}

EditingContextManager::StrokeMode EditingContextManager::strokeMode() const
{
    return mStrokeMode;
}

qreal EditingContextManager::strokeProportionalSpacing() const
{
    return mStrokeProportionalSpacing;
}

qreal EditingContextManager::strokeAbsoluteSpacing() const
{
    return mStrokeAbsoluteSpacing;
}

int EditingContextManager::strokeCount() const
{
    return mStrokeCount;
}

qreal EditingContextManager::strokeTime() const
{
    return mStrokeTime;
}

EditingContextManager::ScalingMode EditingContextManager::brushScalingMode() const
{
    return mBrushScalingMode;
}

qreal EditingContextManager::brushScalingMin() const
{
    return mBrushScalingMin;
}

qreal EditingContextManager::brushScalingMax() const
{
    return mBrushScalingMax;
}

EditingContextManager::RotationMode EditingContextManager::brushRotationMode() const
{
    return mBrushRotationMode;
}

qreal EditingContextManager::brushRotationMin() const
{
    return mBrushRotationMin;
}

qreal EditingContextManager::brushRotationMax() const
{
    return mBrushRotationMax;
}

EditingContextManager::ScalingMode EditingContextManager::brushHardnessMode() const
{
    return mBrushHardnessMode;
}

qreal EditingContextManager::brushHardnessMin() const
{
    return mBrushHardnessMin;
}

qreal EditingContextManager::brushHardnessMax() const
{
    return mBrushHardnessMax;
}

EditingContextManager::ScalingMode EditingContextManager::brushOpacityMode() const
{
    return mBrushOpacityMode;
}

qreal EditingContextManager::brushOpacityMin() const
{
    return mBrushOpacityMin;
}

qreal EditingContextManager::brushOpacityMax() const
{
    return mBrushOpacityMax;
}

EditingContextManager::ScalingMode EditingContextManager::dynamicsProportionalSpacingMode() const
{
    return mDynamicsProportionalSpacingMode;
}

qreal EditingContextManager::dynamicsProportionalSpacingMin() const
{
    return mDynamicsProportionalSpacingMin;
}

qreal EditingContextManager::dynamicsProportionalSpacingMax() const
{
    return mDynamicsProportionalSpacingMax;
}

EditingContextManager::ScalingMode EditingContextManager::dynamicsAbsoluteSpacingMode() const
{
    return mDynamicsAbsoluteSpacingMode;
}

qreal EditingContextManager::dynamicsAbsoluteSpacingMin() const
{
    return mDynamicsAbsoluteSpacingMin;
}

qreal EditingContextManager::dynamicsAbsoluteSpacingMax() const
{
    return mDynamicsAbsoluteSpacingMax;
}

EditingContextManager::BlendMode EditingContextManager::blendMode() const
{
    return mBlendMode;
}

QColor EditingContextManager::foregroundColour() const
{
    return mForegroundColour;
}

QColor EditingContextManager::backgroundColour() const
{
    return mBackgroundColour;
}

void EditingContextManager::setBrush(BrushManager *const brush)
{
    if (mBrush == brush)
        return;

    mBrush = brush;
    emit brushChanged(mBrush);
}

void EditingContextManager::setStrokeMode(EditingContextManager::StrokeMode strokeMode)
{
    if (mStrokeMode == strokeMode)
        return;

    mStrokeMode = strokeMode;
    emit strokeModeChanged(mStrokeMode);
}

void EditingContextManager::setStrokeProportionalSpacing(qreal strokeProportionalSpacing)
{
    if (qFuzzyCompare(mStrokeProportionalSpacing, strokeProportionalSpacing))
        return;

    mStrokeProportionalSpacing = strokeProportionalSpacing;
    emit strokeProportionalSpacingChanged(mStrokeProportionalSpacing);
}

void EditingContextManager::setStrokeAbsoluteSpacing(qreal strokeAbsoluteSpacing)
{
    if (qFuzzyCompare(mStrokeAbsoluteSpacing, strokeAbsoluteSpacing))
        return;

    mStrokeAbsoluteSpacing = strokeAbsoluteSpacing;
    emit strokeAbsoluteSpacingChanged(mStrokeAbsoluteSpacing);
}

void EditingContextManager::setStrokeCount(int strokeCount)
{
    if (mStrokeCount == strokeCount)
        return;

    mStrokeCount = strokeCount;
    emit strokeCountChanged(mStrokeCount);
}

void EditingContextManager::setStrokeTime(qreal strokeTime)
{
    if (qFuzzyCompare(mStrokeTime, strokeTime))
        return;

    mStrokeTime = strokeTime;
    emit strokeTimeChanged(mStrokeTime);
}

void EditingContextManager::setBrushScalingMode(EditingContextManager::ScalingMode brushScalingMode)
{
    if (mBrushScalingMode == brushScalingMode)
        return;

    mBrushScalingMode = brushScalingMode;
    emit brushScalingModeChanged(mBrushScalingMode);
}

void EditingContextManager::setBrushScalingMin(qreal brushScalingMin)
{
    if (qFuzzyCompare(mBrushScalingMin, brushScalingMin))
        return;

    mBrushScalingMin = brushScalingMin;
    emit brushScalingMinChanged(mBrushScalingMin);
}

void EditingContextManager::setBrushScalingMax(qreal brushScalingMax)
{
    if (qFuzzyCompare(mBrushScalingMax, brushScalingMax))
        return;

    mBrushScalingMax = brushScalingMax;
    emit brushScalingMaxChanged(mBrushScalingMax);
}

void EditingContextManager::setBrushRotationMode(EditingContextManager::RotationMode brushRotationMode)
{
    if (mBrushRotationMode == brushRotationMode)
        return;

    mBrushRotationMode = brushRotationMode;
    emit brushRotationModeChanged(mBrushRotationMode);
}

void EditingContextManager::setBrushRotationMin(qreal brushRotationMin)
{
    if (qFuzzyCompare(mBrushRotationMin, brushRotationMin))
        return;

    mBrushRotationMin = brushRotationMin;
    emit brushRotationMinChanged(mBrushRotationMin);
}

void EditingContextManager::setBrushRotationMax(qreal brushRotationMax)
{
    if (qFuzzyCompare(mBrushRotationMax, brushRotationMax))
        return;

    mBrushRotationMax = brushRotationMax;
    emit brushRotationMaxChanged(mBrushRotationMax);
}

void EditingContextManager::setBrushHardnessMode(EditingContextManager::ScalingMode brushHardnessMode)
{
    if (mBrushHardnessMode == brushHardnessMode)
        return;

    mBrushHardnessMode = brushHardnessMode;
    emit brushHardnessModeChanged(mBrushHardnessMode);
}

void EditingContextManager::setBrushHardnessMin(qreal brushHardnessMin)
{
    if (qFuzzyCompare(mBrushHardnessMin, brushHardnessMin))
        return;

    mBrushHardnessMin = brushHardnessMin;
    emit brushHardnessMinChanged(mBrushHardnessMin);
}

void EditingContextManager::setBrushHardnessMax(qreal brushHardnessMax)
{
    if (qFuzzyCompare(mBrushHardnessMax, brushHardnessMax))
        return;

    mBrushHardnessMax = brushHardnessMax;
    emit brushHardnessMaxChanged(mBrushHardnessMax);
}

void EditingContextManager::setBrushOpacityMode(EditingContextManager::ScalingMode brushOpacityMode)
{
    if (mBrushOpacityMode == brushOpacityMode)
        return;

    mBrushOpacityMode = brushOpacityMode;
    emit brushOpacityModeChanged(mBrushOpacityMode);
}

void EditingContextManager::setBrushOpacityMin(qreal brushOpacityMin)
{
    if (qFuzzyCompare(mBrushOpacityMin, brushOpacityMin))
        return;

    mBrushOpacityMin = brushOpacityMin;
    emit brushOpacityMinChanged(mBrushOpacityMin);
}

void EditingContextManager::setBrushOpacityMax(qreal brushOpacityMax)
{
    if (qFuzzyCompare(mBrushOpacityMax, brushOpacityMax))
        return;

    mBrushOpacityMax = brushOpacityMax;
    emit brushOpacityMaxChanged(mBrushOpacityMax);
}

void EditingContextManager::setDynamicsProportionalSpacingMode(EditingContextManager::ScalingMode dynamicsProportionalSpacingMode)
{
    if (mDynamicsProportionalSpacingMode == dynamicsProportionalSpacingMode)
        return;

    mDynamicsProportionalSpacingMode = dynamicsProportionalSpacingMode;
    emit dynamicsProportionalSpacingModeChanged(mDynamicsProportionalSpacingMode);
}

void EditingContextManager::setDynamicsProportionalSpacingMin(qreal dynamicsProportionalSpacingMin)
{
    if (qFuzzyCompare(mDynamicsProportionalSpacingMin, dynamicsProportionalSpacingMin))
        return;

    mDynamicsProportionalSpacingMin = dynamicsProportionalSpacingMin;
    emit dynamicsProportionalSpacingMinChanged(mDynamicsProportionalSpacingMin);
}

void EditingContextManager::setDynamicsProportionalSpacingMax(qreal dynamicsProportionalSpacingMax)
{
    if (qFuzzyCompare(mDynamicsProportionalSpacingMax, dynamicsProportionalSpacingMax))
        return;

    mDynamicsProportionalSpacingMax = dynamicsProportionalSpacingMax;
    emit dynamicsProportionalSpacingMaxChanged(mDynamicsProportionalSpacingMax);
}

void EditingContextManager::setDynamicsAbsoluteSpacingMode(EditingContextManager::ScalingMode dynamicsAbsoluteSpacingMode)
{
    if (mDynamicsAbsoluteSpacingMode == dynamicsAbsoluteSpacingMode)
        return;

    mDynamicsAbsoluteSpacingMode = dynamicsAbsoluteSpacingMode;
    emit dynamicsAbsoluteSpacingModeChanged(mDynamicsAbsoluteSpacingMode);
}

void EditingContextManager::setDynamicsAbsoluteSpacingMin(qreal dynamicsAbsoluteSpacingMin)
{
    if (qFuzzyCompare(mDynamicsAbsoluteSpacingMin, dynamicsAbsoluteSpacingMin))
        return;

    mDynamicsAbsoluteSpacingMin = dynamicsAbsoluteSpacingMin;
    emit dynamicsAbsoluteSpacingMinChanged(mDynamicsAbsoluteSpacingMin);
}

void EditingContextManager::setDynamicsAbsoluteSpacingMax(qreal dynamicsAbsoluteSpacingMax)
{
    if (qFuzzyCompare(mDynamicsAbsoluteSpacingMax, dynamicsAbsoluteSpacingMax))
        return;

    mDynamicsAbsoluteSpacingMax = dynamicsAbsoluteSpacingMax;
    emit dynamicsAbsoluteSpacingMaxChanged(mDynamicsAbsoluteSpacingMax);
}

void EditingContextManager::setBlendMode(EditingContextManager::BlendMode blendMode)
{
    if (mBlendMode == blendMode)
        return;

    mBlendMode = blendMode;
    emit blendModeChanged(mBlendMode);
}

void EditingContextManager::setForegroundColour(QColor foregroundColour)
{
    if (mForegroundColour == foregroundColour)
        return;

    mForegroundColour = foregroundColour;
    emit foregroundColourChanged(mForegroundColour);
}

void EditingContextManager::setBackgroundColour(QColor backgroundColour)
{
    if (mBackgroundColour == backgroundColour)
        return;

    mBackgroundColour = backgroundColour;
    emit backgroundColourChanged(mBackgroundColour);
}
