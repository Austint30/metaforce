#pragma once

#include <memory>
#include <vector>

#include "Runtime/CToken.hpp"
#include "Runtime/RetroTypes.hpp"
#include "Runtime/Character/CCharAnimTime.hpp"
#include "Runtime/Character/CSegId.hpp"

#include <zeus/CQuaternion.hpp>
#include <zeus/CVector3f.hpp>

namespace urde {
class CAnimPOIData;
class CBoolPOINode;
class CInt32POINode;
class CParticlePOINode;
class CSegId;
class CSegIdList;
class CSegStatementSet;
class CSoundPOINode;
class IObjectStore;

class RotationAndOffsetStorage {
  friend class CAnimSource;
  std::unique_ptr<float[]> x0_storage;
  u32 x8_frameCount;
  u32 xc_rotPerFrame;
  u32 x10_transPerFrame;

  std::unique_ptr<float[]> GetRotationsAndOffsets(const std::vector<zeus::CQuaternion>& rots,
                                                  const std::vector<zeus::CVector3f>& offs, u32 frameCount);
  static void CopyRotationsAndOffsets(const std::vector<zeus::CQuaternion>& rots,
                                      const std::vector<zeus::CVector3f>& offs, u32 frameCount, float*);
  static u32 DataSizeInBytes(u32 rotPerFrame, u32 transPerFrame, u32 frameCount);

public:
  struct CRotationAndOffsetVectors {
    std::vector<zeus::CQuaternion> x0_rotations;
    std::vector<zeus::CVector3f> x10_offsets;
    explicit CRotationAndOffsetVectors(CInputStream& in);
  };
  u32 GetFrameSizeInBytes() const;
  RotationAndOffsetStorage(const CRotationAndOffsetVectors& vectors, u32 frameCount);
};

class CAnimSource {
  friend class CAnimSourceInfo;
  CCharAnimTime x0_duration;
  CCharAnimTime x8_interval;
  u32 x10_frameCount;
  CSegId x1c_rootBone;
  std::vector<u8> x20_rotationChannels;
  std::vector<u8> x30_translationChannels;
  RotationAndOffsetStorage x40_data;
  CAssetId x54_evntId;
  TCachedToken<CAnimPOIData> x58_evntData;
  float x60_averageVelocity;

  void CalcAverageVelocity();

public:
  explicit CAnimSource(CInputStream& in, IObjectStore& store);

  void GetSegStatementSet(const CSegIdList& list, CSegStatementSet& set, const CCharAnimTime& time) const;

  const std::vector<CSoundPOINode>& GetSoundPOIStream() const;
  const std::vector<CParticlePOINode>& GetParticlePOIStream() const;
  const std::vector<CInt32POINode>& GetInt32POIStream() const;
  const std::vector<CBoolPOINode>& GetBoolPOIStream() const;
  const TCachedToken<CAnimPOIData>& GetPOIData() const { return x58_evntData; }
  float GetAverageVelocity() const { return x60_averageVelocity; }
  zeus::CQuaternion GetRotation(const CSegId& seg, const CCharAnimTime& time) const;
  zeus::CVector3f GetOffset(const CSegId& seg, const CCharAnimTime& time) const;
  bool HasOffset(const CSegId& seg) const;
  const CCharAnimTime& GetDuration() const { return x0_duration; }
  const CSegId& GetRootBoneId() const { return x1c_rootBone; }
};

} // namespace urde
