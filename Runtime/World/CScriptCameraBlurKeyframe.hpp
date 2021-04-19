#pragma once

#include <string_view>

#include "Runtime/GCNTypes.hpp"
#include "Runtime/Camera/CCameraFilter.hpp"
#include "Runtime/World/CEntity.hpp"

namespace metaforce {
class CScriptCameraBlurKeyframe : public CEntity {
  EBlurType x34_type;
  float x38_amount;
  u32 x3c_;
  float x40_timeIn;
  float x44_timeOut;

public:
  CScriptCameraBlurKeyframe(TUniqueId uid, std::string_view name, const CEntityInfo& info, EBlurType type, float amount,
                            u32 unk, float timeIn, float timeOut, bool active);

  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId objId, CStateManager& stateMgr) override;
  void Accept(IVisitor& visitor) override;
};
} // namespace metaforce
