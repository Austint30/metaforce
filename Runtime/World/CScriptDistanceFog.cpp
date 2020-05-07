#include "Runtime/World/CScriptDistanceFog.hpp"

#include "Runtime/CStateManager.hpp"
#include "Runtime/World/CWorld.hpp"

#include "TCastTo.hpp" // Generated file, do not modify include path

namespace urde {
CScriptDistanceFog::CScriptDistanceFog(TUniqueId uid, std::string_view name, const CEntityInfo& info, ERglFogMode mode,
                                       const zeus::CColor& color, const zeus::CVector2f& range, float colorDelta,
                                       const zeus::CVector2f& rangeDelta, bool expl, bool active, float thermalTarget,
                                       float thermalSpeed, float xrayTarget, float xraySpeed)
: CEntity(uid, info, active, name)
, x34_mode(mode)
, x38_color(color)
, x3c_range(range)
, x44_colorDelta(colorDelta)
, x48_rangeDelta(rangeDelta)
, x50_thermalTarget(thermalTarget)
, x54_thermalSpeed(thermalSpeed)
, x58_xrayTarget(xrayTarget)
, x5c_xraySpeed(xraySpeed)
, x60_explicit(expl)

{
  if (zeus::close_enough(rangeDelta, zeus::skZero2f) && zeus::close_enough(colorDelta, 0.f)) {
    x61_nonZero = false;
  } else {
    x61_nonZero = true;
  }
}

void CScriptDistanceFog::Accept(IVisitor& visitor) { visitor.Visit(this); }

void CScriptDistanceFog::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId objId, CStateManager& stateMgr) {
  CEntity::AcceptScriptMsg(msg, objId, stateMgr);

  if (x4_areaId == kInvalidAreaId || !GetActive()) {
    return;
  }

  if (msg == EScriptObjectMessage::InitializedInArea) {
    if (!x60_explicit) {
      return;
    }

    CGameArea::CAreaFog* fog = stateMgr.GetWorld()->GetArea(x4_areaId)->GetAreaFog();
    if (x34_mode == ERglFogMode::None) {
      fog->DisableFog();
    } else {
      fog->SetFogExplicit(x34_mode, x38_color, x3c_range);
    }
  } else if (msg == EScriptObjectMessage::Action) {
    if (x61_nonZero) {
      CGameArea::CAreaFog* fog = stateMgr.GetWorld()->GetArea(x4_areaId)->GetAreaFog();
      if (x34_mode == ERglFogMode::None) {
        fog->RollFogOut(x48_rangeDelta.x(), x44_colorDelta, x38_color);
      } else {
        fog->FadeFog(x34_mode, x38_color, x3c_range, x44_colorDelta, x48_rangeDelta);
      }
    }

    CWorld* world = stateMgr.GetWorld();
    CGameArea* area = world->GetArea(x4_areaId);
    if (!zeus::close_enough(x54_thermalSpeed, 0.f)) {
      area->SetThermalSpeedAndTarget(x54_thermalSpeed, x50_thermalTarget);
    }

    if (!zeus::close_enough(x5c_xraySpeed, 0.f)) {
      area->SetXRaySpeedAndTarget(x5c_xraySpeed, x58_xrayTarget);
    }
  }
}
} // namespace urde
