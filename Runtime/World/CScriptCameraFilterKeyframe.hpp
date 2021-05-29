#pragma once

#include <string_view>

#include "Runtime/GCNTypes.hpp"
#include "Runtime/Camera/CCameraFilter.hpp"
#include "Runtime/World/CEntity.hpp"

#include <zeus/CColor.hpp>

namespace metaforce {
class CScriptCameraFilterKeyframe : public CEntity {
  EFilterType x34_type;
  EFilterShape x38_shape;
  u32 x3c_filterIdx;
  u32 x40_;
  zeus::CColor x44_color;
  float x48_timeIn;
  float x4c_timeOut;
  CAssetId x50_txtr;

public:
  DEFINE_ENTITY
  CScriptCameraFilterKeyframe(TUniqueId uid, std::string_view name, const CEntityInfo& info, EFilterType type,
                              EFilterShape shape, u32 filterIdx, u32 unk, const zeus::CColor& color, float timeIn,
                              float timeOut, CAssetId txtr, bool active);

  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId objId, CStateManager& stateMgr) override;
  void Accept(IVisitor& visitor) override;
};
} // namespace metaforce
