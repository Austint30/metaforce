#pragma once

#include <string_view>

#include "Runtime/World/CEntity.hpp"

#include <zeus/CVector3f.hpp>

namespace metaforce {
class CScriptRipple : public CEntity {
  float x34_magnitude;
  zeus::CVector3f x38_center;

public:
  DEFINE_ENTITY
  CScriptRipple(TUniqueId, std::string_view, const CEntityInfo&, const zeus::CVector3f&, bool, float);

  void Accept(IVisitor&) override;
  void Think(float, CStateManager&) override {}
  void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&) override;
};
} // namespace metaforce
