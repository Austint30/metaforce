#pragma once

#include <vector>

#include "Runtime/RetroTypes.hpp"
#include "Runtime/Graphics/CPVSVisOctree.hpp"

namespace metaforce {

class CPVSAreaSet {
  u32 x0_numFeatures;
  u32 x4_numLights;
  u32 x8_num2ndLights;
  u32 xc_numActors;
  u32 x10_leafSize;
  u32 x14_lightIndexCount;
  std::vector<u32> x18_entityIndex;
  const u8* x1c_lightLeaves;
  CPVSVisOctree x20_octree;

  CPVSVisSet _GetLightSet(size_t lightIdx) const {
    CPVSVisSet ret;
    ret.SetFromMemory(x20_octree.GetNumObjects(), x20_octree.GetNumLights(), x1c_lightLeaves + x10_leafSize * lightIdx);
    return ret;
  }

public:
  explicit CPVSAreaSet(const u8* data, u32 len);
  u32 GetNumFeatures() const { return x0_numFeatures; }
  u32 GetNumActors() const { return xc_numActors; }
  u32 Get1stLightIndex(u32 lightIdx) const { return x0_numFeatures + x8_num2ndLights + lightIdx; }
  u32 Get2ndLightIndex(u32 lightIdx) const { return x0_numFeatures + lightIdx; }
  bool Has2ndLayerLights() const { return x8_num2ndLights != 0; }
  u32 GetEntityIdByIndex(size_t idx) const { return x18_entityIndex[idx]; }
  const CPVSVisOctree& GetVisOctree() const { return x20_octree; }
  CPVSVisSet Get1stLightSet(size_t lightIdx) const { return _GetLightSet(x8_num2ndLights + lightIdx); }
  CPVSVisSet Get2ndLightSet(size_t lightIdx) const { return _GetLightSet(lightIdx); }
};

} // namespace metaforce
