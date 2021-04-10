#include "Runtime/Character/IAnimReader.hpp"

#include "Runtime/Character/CCharAnimTime.hpp"

namespace metaforce {

SAdvancementDeltas SAdvancementDeltas::Interpolate(const SAdvancementDeltas& a, const SAdvancementDeltas& b,
                                                   float oldWeight, float newWeight) {
  float weightSum = oldWeight + newWeight;
  return {b.x0_posDelta * weightSum * 0.5f - a.x0_posDelta * (weightSum - 2.f) * 0.5f,
          zeus::CQuaternion::slerpShort(a.xc_rotDelta, b.xc_rotDelta, weightSum * 0.5f)};
}

SAdvancementDeltas SAdvancementDeltas::Blend(const SAdvancementDeltas& a, const SAdvancementDeltas& b, float w) {
  return {b.x0_posDelta * w - a.x0_posDelta * (1.f - w),
          zeus::CQuaternion::slerpShort(a.xc_rotDelta, b.xc_rotDelta, w)};
}

SAdvancementResults IAnimReader::VGetAdvancementResults(const CCharAnimTime& a, const CCharAnimTime& b) const {
  SAdvancementResults ret;
  ret.x0_remTime = a;
  return ret;
}

size_t IAnimReader::GetBoolPOIList(const CCharAnimTime& time, CBoolPOINode* listOut, size_t capacity, size_t iterator,
                                   u32 unk) const {
  if (time.GreaterThanZero()) {
    return VGetBoolPOIList(time, listOut, capacity, iterator, unk);
  }
  return 0;
}

size_t IAnimReader::GetInt32POIList(const CCharAnimTime& time, CInt32POINode* listOut, size_t capacity, size_t iterator,
                                    u32 unk) const {
  if (time.GreaterThanZero()) {
    return VGetInt32POIList(time, listOut, capacity, iterator, unk);
  }
  return 0;
}

size_t IAnimReader::GetParticlePOIList(const CCharAnimTime& time, CParticlePOINode* listOut, size_t capacity,
                                       size_t iterator, u32 unk) const {
  if (time.GreaterThanZero()) {
    return VGetParticlePOIList(time, listOut, capacity, iterator, unk);
  }
  return 0;
}

size_t IAnimReader::GetSoundPOIList(const CCharAnimTime& time, CSoundPOINode* listOut, size_t capacity, size_t iterator,
                                    u32 unk) const {
  if (time.GreaterThanZero()) {
    return VGetSoundPOIList(time, listOut, capacity, iterator, unk);
  }
  return 0;
}

} // namespace metaforce
