#include "editingcontext.h"

EditingContext::EditingContext() :
    brush(),
    strokeMode(StrokeMode::Spaced),
    strokeProportionalSpacing(0.25),
    strokeAbsoluteSpacing(0),
    strokeCount(1),
    strokeTime(1),

    brushScalingMode(ScalingMode::None),
    brushScalingMin(0.0),
    brushScalingMax(1.0),
    brushRotationMode(RotationMode::None),
    brushRotationMin(0.0),
    brushRotationMax(1.0),
    brushHardnessMode(ScalingMode::None),
    brushHardnessMin(0.0),
    brushHardnessMax(1.0),
    brushOpacityMode(ScalingMode::None),
    brushOpacityMin(0.0),
    brushOpacityMax(1.0),

    dynamicsProportionalSpacingMode(ScalingMode::None),
    dynamicsProportionalSpacingMin(0.0),
    dynamicsProportionalSpacingMax(1.0),
    dynamicsAbsoluteSpacingMode(ScalingMode::None),
    dynamicsAbsoluteSpacingMin(0.0),
    dynamicsAbsoluteSpacingMax(1.0),

    blendMode(BlendMode::Blend),
    foregroundColour(0, 0, 0, 255),
    backgroundColour(255, 255, 255, 255)
{
}

EditingContext::EditingContext(const EditingContext &other)
{
    brush = other.brush;
    strokeMode = other.strokeMode;
    strokeProportionalSpacing = other.strokeProportionalSpacing;
    strokeAbsoluteSpacing = other.strokeAbsoluteSpacing;
    strokeCount = other.strokeCount;
    strokeTime = other.strokeTime;

    brushScalingMode = other.brushScalingMode;
    brushScalingMin = other.brushScalingMin;
    brushScalingMax = other.brushScalingMax;
    brushRotationMode = other.brushRotationMode;
    brushRotationMin = other.brushRotationMin;
    brushRotationMax = other.brushRotationMax;
    brushHardnessMode = other.brushHardnessMode;
    brushHardnessMin = other.brushHardnessMin;
    brushHardnessMax = other.brushHardnessMax;
    brushOpacityMode = other.brushOpacityMode;
    brushOpacityMin = other.brushOpacityMin;
    brushOpacityMax = other.brushOpacityMax;

    dynamicsProportionalSpacingMode = other.dynamicsProportionalSpacingMode;
    dynamicsProportionalSpacingMin = other.dynamicsProportionalSpacingMin;
    dynamicsProportionalSpacingMax = other.dynamicsProportionalSpacingMax;
    dynamicsAbsoluteSpacingMode = other.dynamicsAbsoluteSpacingMode;
    dynamicsAbsoluteSpacingMin = other.dynamicsAbsoluteSpacingMin;
    dynamicsAbsoluteSpacingMax = other.dynamicsAbsoluteSpacingMax;

    blendMode = other.blendMode;
    foregroundColour = other.foregroundColour;
    backgroundColour = other.backgroundColour;
}

bool EditingContext::operator==(const EditingContext &other) const
{
    return brush == other.brush &&
        strokeMode == other.strokeMode &&
        qFuzzyCompare(strokeProportionalSpacing, other.strokeProportionalSpacing) &&
        qFuzzyCompare(strokeAbsoluteSpacing, other.strokeAbsoluteSpacing) &&
        strokeCount == other.strokeCount &&
        qFuzzyCompare(strokeTime, other.strokeTime) &&

        brushScalingMode == other.brushScalingMode &&
        qFuzzyCompare(brushScalingMin, other.brushScalingMin) &&
        qFuzzyCompare(brushScalingMax, other.brushScalingMax) &&
        brushRotationMode == other.brushRotationMode &&
        qFuzzyCompare(brushRotationMin, other.brushRotationMin) &&
        qFuzzyCompare(brushRotationMax, other.brushRotationMax) &&
        brushHardnessMode == other.brushHardnessMode &&
        qFuzzyCompare(brushHardnessMin, other.brushHardnessMin) &&
        qFuzzyCompare(brushHardnessMax, other.brushHardnessMax) &&
        brushOpacityMode == other.brushOpacityMode &&
        qFuzzyCompare(brushOpacityMin, other.brushOpacityMin) &&
        qFuzzyCompare(brushOpacityMax, other.brushOpacityMax) &&

        dynamicsProportionalSpacingMode == other.dynamicsProportionalSpacingMode &&
        qFuzzyCompare(dynamicsProportionalSpacingMin, other.dynamicsProportionalSpacingMin) &&
        qFuzzyCompare(dynamicsProportionalSpacingMax, other.dynamicsProportionalSpacingMax) &&
        dynamicsAbsoluteSpacingMode == other.dynamicsAbsoluteSpacingMode &&
        qFuzzyCompare(dynamicsAbsoluteSpacingMin, other.dynamicsAbsoluteSpacingMin) &&
        qFuzzyCompare(dynamicsAbsoluteSpacingMax, other.dynamicsAbsoluteSpacingMax) &&

        blendMode == other.blendMode &&
        foregroundColour == other.foregroundColour &&
        backgroundColour == other.backgroundColour;
}

bool EditingContext::operator!=(const EditingContext &other) const
{
    return !(*this == other);
}

EditingContext &EditingContext::operator=(const EditingContext &other)
{
    brush = other.brush;
    strokeMode = other.strokeMode;
    strokeProportionalSpacing = other.strokeProportionalSpacing;
    strokeAbsoluteSpacing = other.strokeAbsoluteSpacing;
    strokeCount = other.strokeCount;
    strokeTime = other.strokeTime;

    brushScalingMode = other.brushScalingMode;
    brushScalingMin = other.brushScalingMin;
    brushScalingMax = other.brushScalingMax;
    brushRotationMode = other.brushRotationMode;
    brushRotationMin = other.brushRotationMin;
    brushRotationMax = other.brushRotationMax;
    brushHardnessMode = other.brushHardnessMode;
    brushHardnessMin = other.brushHardnessMin;
    brushHardnessMax = other.brushHardnessMax;
    brushOpacityMode = other.brushOpacityMode;
    brushOpacityMin = other.brushOpacityMin;
    brushOpacityMax = other.brushOpacityMax;

    dynamicsProportionalSpacingMode = other.dynamicsProportionalSpacingMode;
    dynamicsProportionalSpacingMin = other.dynamicsProportionalSpacingMin;
    dynamicsProportionalSpacingMax = other.dynamicsProportionalSpacingMax;
    dynamicsAbsoluteSpacingMode = other.dynamicsAbsoluteSpacingMode;
    dynamicsAbsoluteSpacingMin = other.dynamicsAbsoluteSpacingMin;
    dynamicsAbsoluteSpacingMax = other.dynamicsAbsoluteSpacingMax;

    blendMode = other.blendMode;
    foregroundColour = other.foregroundColour;
    backgroundColour = other.backgroundColour;
    return *this;
}

EditingContextManager::EditingContextManager(EditingContext *const editingContext) :
    QObject(),
    mEditingContext(editingContext),
    mBrushManager(&editingContext->brush)
{
}

EditingContextManager::EditingContextManager(const EditingContextManager &other) :
    QObject()
{
    mEditingContext = other.mEditingContext;
    mBrushManager = BrushManager(other.mBrushManager);
}

EditingContextManager::~EditingContextManager()
{
}

bool EditingContextManager::operator==(const EditingContextManager &other) const
{
    return mEditingContext == other.mEditingContext;
}

bool EditingContextManager::operator!=(const EditingContextManager &other) const
{
    return !(*this == other);
}

EditingContextManager &EditingContextManager::operator=(const EditingContextManager &other)
{
    mEditingContext = other.mEditingContext;
    mBrushManager = other.mBrushManager;
    return *this;
}

EditingContext *EditingContextManager::editingContext()
{
    return mEditingContext;
}

BrushManager *EditingContextManager::brushManager()
{
    return &mBrushManager;
}

const Brush &EditingContextManager::brush() const
{
    return mEditingContext->brush;
}

EditingContext::StrokeMode EditingContextManager::strokeMode() const
{
    return mEditingContext->strokeMode;
}

qreal EditingContextManager::strokeProportionalSpacing() const
{
    return mEditingContext->strokeProportionalSpacing;
}

qreal EditingContextManager::strokeAbsoluteSpacing() const
{
    return mEditingContext->strokeAbsoluteSpacing;
}

int EditingContextManager::strokeCount() const
{
    return mEditingContext->strokeCount;
}

qreal EditingContextManager::strokeTime() const
{
    return mEditingContext->strokeTime;
}

EditingContext::ScalingMode EditingContextManager::brushScalingMode() const
{
    return mEditingContext->brushScalingMode;
}

qreal EditingContextManager::brushScalingMin() const
{
    return mEditingContext->brushScalingMin;
}

qreal EditingContextManager::brushScalingMax() const
{
    return mEditingContext->brushScalingMax;
}

EditingContext::RotationMode EditingContextManager::brushRotationMode() const
{
    return mEditingContext->brushRotationMode;
}

qreal EditingContextManager::brushRotationMin() const
{
    return mEditingContext->brushRotationMin;
}

qreal EditingContextManager::brushRotationMax() const
{
    return mEditingContext->brushRotationMax;
}

EditingContext::ScalingMode EditingContextManager::brushHardnessMode() const
{
    return mEditingContext->brushHardnessMode;
}

qreal EditingContextManager::brushHardnessMin() const
{
    return mEditingContext->brushHardnessMin;
}

qreal EditingContextManager::brushHardnessMax() const
{
    return mEditingContext->brushHardnessMax;
}

EditingContext::ScalingMode EditingContextManager::brushOpacityMode() const
{
    return mEditingContext->brushOpacityMode;
}

qreal EditingContextManager::brushOpacityMin() const
{
    return mEditingContext->brushOpacityMin;
}

qreal EditingContextManager::brushOpacityMax() const
{
    return mEditingContext->brushOpacityMax;
}

EditingContext::ScalingMode EditingContextManager::dynamicsProportionalSpacingMode() const
{
    return mEditingContext->dynamicsProportionalSpacingMode;
}

qreal EditingContextManager::dynamicsProportionalSpacingMin() const
{
    return mEditingContext->dynamicsProportionalSpacingMin;
}

qreal EditingContextManager::dynamicsProportionalSpacingMax() const
{
    return mEditingContext->dynamicsProportionalSpacingMax;
}

EditingContext::ScalingMode EditingContextManager::dynamicsAbsoluteSpacingMode() const
{
    return mEditingContext->dynamicsAbsoluteSpacingMode;
}

qreal EditingContextManager::dynamicsAbsoluteSpacingMin() const
{
    return mEditingContext->dynamicsAbsoluteSpacingMin;
}

qreal EditingContextManager::dynamicsAbsoluteSpacingMax() const
{
    return mEditingContext->dynamicsAbsoluteSpacingMax;
}

EditingContext::BlendMode EditingContextManager::blendMode() const
{
    return mEditingContext->blendMode;
}

QColor EditingContextManager::foregroundColour() const
{
    return mEditingContext->foregroundColour;
}

QColor EditingContextManager::backgroundColour() const
{
    return mEditingContext->backgroundColour;
}

void EditingContextManager::setEditingContext(EditingContext *const editingContext)
{
    if (mEditingContext == editingContext)
        return;

    mEditingContext = editingContext;
    emit editingContextChanged();
}

void EditingContextManager::setBrush(const Brush &brush)
{
    if (mEditingContext->brush == brush)
        return;

    mEditingContext->brush = brush;
    emit brushChanged();
}

void EditingContextManager::setStrokeMode(EditingContext::StrokeMode strokeMode)
{
    if (mEditingContext->strokeMode == strokeMode)
        return;

    mEditingContext->strokeMode = strokeMode;
    emit strokeModeChanged(mEditingContext->strokeMode);
}

void EditingContextManager::setStrokeProportionalSpacing(qreal strokeProportionalSpacing)
{
    if (qFuzzyCompare(mEditingContext->strokeProportionalSpacing, strokeProportionalSpacing))
        return;

    mEditingContext->strokeProportionalSpacing = strokeProportionalSpacing;
    emit strokeProportionalSpacingChanged(mEditingContext->strokeProportionalSpacing);
}

void EditingContextManager::setStrokeAbsoluteSpacing(qreal strokeAbsoluteSpacing)
{
    if (qFuzzyCompare(mEditingContext->strokeAbsoluteSpacing, strokeAbsoluteSpacing))
        return;

    mEditingContext->strokeAbsoluteSpacing = strokeAbsoluteSpacing;
    emit strokeAbsoluteSpacingChanged(mEditingContext->strokeAbsoluteSpacing);
}

void EditingContextManager::setStrokeCount(int strokeCount)
{
    if (mEditingContext->strokeCount == strokeCount)
        return;

    mEditingContext->strokeCount = strokeCount;
    emit strokeCountChanged(mEditingContext->strokeCount);
}

void EditingContextManager::setStrokeTime(qreal strokeTime)
{
    if (qFuzzyCompare(mEditingContext->strokeTime, strokeTime))
        return;

    mEditingContext->strokeTime = strokeTime;
    emit strokeTimeChanged(mEditingContext->strokeTime);
}

void EditingContextManager::setBrushScalingMode(EditingContext::ScalingMode brushScalingMode)
{
    if (mEditingContext->brushScalingMode == brushScalingMode)
        return;

    mEditingContext->brushScalingMode = brushScalingMode;
    emit brushScalingModeChanged(mEditingContext->brushScalingMode);
}

void EditingContextManager::setBrushScalingMin(qreal brushScalingMin)
{
    if (qFuzzyCompare(mEditingContext->brushScalingMin, brushScalingMin))
        return;

    mEditingContext->brushScalingMin = brushScalingMin;
    emit brushScalingMinChanged(mEditingContext->brushScalingMin);
}

void EditingContextManager::setBrushScalingMax(qreal brushScalingMax)
{
    if (qFuzzyCompare(mEditingContext->brushScalingMax, brushScalingMax))
        return;

    mEditingContext->brushScalingMax = brushScalingMax;
    emit brushScalingMaxChanged(mEditingContext->brushScalingMax);
}

void EditingContextManager::setBrushRotationMode(EditingContext::RotationMode brushRotationMode)
{
    if (mEditingContext->brushRotationMode == brushRotationMode)
        return;

    mEditingContext->brushRotationMode = brushRotationMode;
    emit brushRotationModeChanged(mEditingContext->brushRotationMode);
}

void EditingContextManager::setBrushRotationMin(qreal brushRotationMin)
{
    if (qFuzzyCompare(mEditingContext->brushRotationMin, brushRotationMin))
        return;

    mEditingContext->brushRotationMin = brushRotationMin;
    emit brushRotationMinChanged(mEditingContext->brushRotationMin);
}

void EditingContextManager::setBrushRotationMax(qreal brushRotationMax)
{
    if (qFuzzyCompare(mEditingContext->brushRotationMax, brushRotationMax))
        return;

    mEditingContext->brushRotationMax = brushRotationMax;
    emit brushRotationMaxChanged(mEditingContext->brushRotationMax);
}

void EditingContextManager::setBrushHardnessMode(EditingContext::ScalingMode brushHardnessMode)
{
    if (mEditingContext->brushHardnessMode == brushHardnessMode)
        return;

    mEditingContext->brushHardnessMode = brushHardnessMode;
    emit brushHardnessModeChanged(mEditingContext->brushHardnessMode);
}

void EditingContextManager::setBrushHardnessMin(qreal brushHardnessMin)
{
    if (qFuzzyCompare(mEditingContext->brushHardnessMin, brushHardnessMin))
        return;

    mEditingContext->brushHardnessMin = brushHardnessMin;
    emit brushHardnessMinChanged(mEditingContext->brushHardnessMin);
}

void EditingContextManager::setBrushHardnessMax(qreal brushHardnessMax)
{
    if (qFuzzyCompare(mEditingContext->brushHardnessMax, brushHardnessMax))
        return;

    mEditingContext->brushHardnessMax = brushHardnessMax;
    emit brushHardnessMaxChanged(mEditingContext->brushHardnessMax);
}

void EditingContextManager::setBrushOpacityMode(EditingContext::ScalingMode brushOpacityMode)
{
    if (mEditingContext->brushOpacityMode == brushOpacityMode)
        return;

    mEditingContext->brushOpacityMode = brushOpacityMode;
    emit brushOpacityModeChanged(mEditingContext->brushOpacityMode);
}

void EditingContextManager::setBrushOpacityMin(qreal brushOpacityMin)
{
    if (qFuzzyCompare(mEditingContext->brushOpacityMin, brushOpacityMin))
        return;

    mEditingContext->brushOpacityMin = brushOpacityMin;
    emit brushOpacityMinChanged(mEditingContext->brushOpacityMin);
}

void EditingContextManager::setBrushOpacityMax(qreal brushOpacityMax)
{
    if (qFuzzyCompare(mEditingContext->brushOpacityMax, brushOpacityMax))
        return;

    mEditingContext->brushOpacityMax = brushOpacityMax;
    emit brushOpacityMaxChanged(mEditingContext->brushOpacityMax);
}

void EditingContextManager::setDynamicsProportionalSpacingMode(EditingContext::ScalingMode dynamicsProportionalSpacingMode)
{
    if (mEditingContext->dynamicsProportionalSpacingMode == dynamicsProportionalSpacingMode)
        return;

    mEditingContext->dynamicsProportionalSpacingMode = dynamicsProportionalSpacingMode;
    emit dynamicsProportionalSpacingModeChanged(mEditingContext->dynamicsProportionalSpacingMode);
}

void EditingContextManager::setDynamicsProportionalSpacingMin(qreal dynamicsProportionalSpacingMin)
{
    if (qFuzzyCompare(mEditingContext->dynamicsProportionalSpacingMin, dynamicsProportionalSpacingMin))
        return;

    mEditingContext->dynamicsProportionalSpacingMin = dynamicsProportionalSpacingMin;
    emit dynamicsProportionalSpacingMinChanged(mEditingContext->dynamicsProportionalSpacingMin);
}

void EditingContextManager::setDynamicsProportionalSpacingMax(qreal dynamicsProportionalSpacingMax)
{
    if (qFuzzyCompare(mEditingContext->dynamicsProportionalSpacingMax, dynamicsProportionalSpacingMax))
        return;

    mEditingContext->dynamicsProportionalSpacingMax = dynamicsProportionalSpacingMax;
    emit dynamicsProportionalSpacingMaxChanged(mEditingContext->dynamicsProportionalSpacingMax);
}

void EditingContextManager::setDynamicsAbsoluteSpacingMode(EditingContext::ScalingMode dynamicsAbsoluteSpacingMode)
{
    if (mEditingContext->dynamicsAbsoluteSpacingMode == dynamicsAbsoluteSpacingMode)
        return;

    mEditingContext->dynamicsAbsoluteSpacingMode = dynamicsAbsoluteSpacingMode;
    emit dynamicsAbsoluteSpacingModeChanged(mEditingContext->dynamicsAbsoluteSpacingMode);
}

void EditingContextManager::setDynamicsAbsoluteSpacingMin(qreal dynamicsAbsoluteSpacingMin)
{
    if (qFuzzyCompare(mEditingContext->dynamicsAbsoluteSpacingMin, dynamicsAbsoluteSpacingMin))
        return;

    mEditingContext->dynamicsAbsoluteSpacingMin = dynamicsAbsoluteSpacingMin;
    emit dynamicsAbsoluteSpacingMinChanged(mEditingContext->dynamicsAbsoluteSpacingMin);
}

void EditingContextManager::setDynamicsAbsoluteSpacingMax(qreal dynamicsAbsoluteSpacingMax)
{
    if (qFuzzyCompare(mEditingContext->dynamicsAbsoluteSpacingMax, dynamicsAbsoluteSpacingMax))
        return;

    mEditingContext->dynamicsAbsoluteSpacingMax = dynamicsAbsoluteSpacingMax;
    emit dynamicsAbsoluteSpacingMaxChanged(mEditingContext->dynamicsAbsoluteSpacingMax);
}

void EditingContextManager::setBlendMode(EditingContext::BlendMode blendMode)
{
    if (mEditingContext->blendMode == blendMode)
        return;

    mEditingContext->blendMode = blendMode;
    emit blendModeChanged(mEditingContext->blendMode);
}

void EditingContextManager::setForegroundColour(QColor foregroundColour)
{
    if (mEditingContext->foregroundColour == foregroundColour)
        return;

    mEditingContext->foregroundColour = foregroundColour;
    emit foregroundColourChanged(mEditingContext->foregroundColour);
}

void EditingContextManager::setBackgroundColour(QColor backgroundColour)
{
    if (mEditingContext->backgroundColour == backgroundColour)
        return;

    mEditingContext->backgroundColour = backgroundColour;
    emit backgroundColourChanged(mEditingContext->backgroundColour);
}
