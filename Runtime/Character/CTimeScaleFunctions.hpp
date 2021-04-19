#pragma once

#include <memory>

#include "Runtime/RetroTypes.hpp"
#include "Runtime/Character/CCharAnimTime.hpp"

namespace metaforce {

enum class EVaryingAnimationTimeScaleType { Constant, Linear };

class IVaryingAnimationTimeScale {
public:
  virtual ~IVaryingAnimationTimeScale() = default;
  virtual EVaryingAnimationTimeScaleType GetType() const = 0;
  virtual float VTimeScaleIntegral(float lowerLimit, float upperLimit) const = 0;
  virtual float VFindUpperLimit(float lowerLimit, float root) const = 0;
  virtual std::unique_ptr<IVaryingAnimationTimeScale> VClone() const = 0;
  virtual std::unique_ptr<IVaryingAnimationTimeScale> VGetFunctionMirrored(float value) const = 0;
  std::unique_ptr<IVaryingAnimationTimeScale> Clone() const;
};

class CConstantAnimationTimeScale : public IVaryingAnimationTimeScale {
private:
  float x4_scale;

public:
  explicit CConstantAnimationTimeScale(float scale) : x4_scale(scale) {}

  EVaryingAnimationTimeScaleType GetType() const override { return EVaryingAnimationTimeScaleType::Constant; }
  float VTimeScaleIntegral(float lowerLimit, float upperLimit) const override;
  float VFindUpperLimit(float lowerLimit, float root) const override;
  std::unique_ptr<IVaryingAnimationTimeScale> VClone() const override;
  std::unique_ptr<IVaryingAnimationTimeScale> VGetFunctionMirrored(float value) const override;
};

class CLinearAnimationTimeScale : public IVaryingAnimationTimeScale {
  struct CFunctionDescription {
    float x4_slope;
    float x8_yIntercept;
    float xc_t1;
    float x10_t2;
    std::unique_ptr<IVaryingAnimationTimeScale> FunctionMirroredAround(float value) const;
  } x4_desc;
  static float FindUpperLimitFromRoot(const CFunctionDescription& desc, float lowerLimit, float root);
  static float TimeScaleIntegralWithSortedLimits(const CFunctionDescription& desc, float lowerLimit, float upperLimit);

public:
  explicit CLinearAnimationTimeScale(const CCharAnimTime& t1, float y1, const CCharAnimTime& t2, float y2);

  EVaryingAnimationTimeScaleType GetType() const override { return EVaryingAnimationTimeScaleType::Linear; }
  float VTimeScaleIntegral(float lowerLimit, float upperLimit) const override;
  float VFindUpperLimit(float lowerLimit, float root) const override;
  std::unique_ptr<IVaryingAnimationTimeScale> VClone() const override;
  std::unique_ptr<IVaryingAnimationTimeScale> VGetFunctionMirrored(float value) const override;
};

} // namespace metaforce
