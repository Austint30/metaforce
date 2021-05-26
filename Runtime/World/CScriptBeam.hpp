#pragma once

#include <string_view>

#include "Runtime/Weapon/CBeamInfo.hpp"
#include "Runtime/World/CActor.hpp"
#include "Runtime/World/CDamageInfo.hpp"

namespace metaforce {
class CWeaponDescription;
class CScriptBeam : public CActor {
  TCachedToken<CWeaponDescription> xe8_weaponDescription;
  CBeamInfo xf4_beamInfo;
  CDamageInfo x138_damageInfo;
  TUniqueId x154_projectileId;

public:
  DEFINE_ENTITY
  CScriptBeam(TUniqueId, std::string_view, const CEntityInfo&, const zeus::CTransform&, bool,
              const TToken<CWeaponDescription>&, const CBeamInfo&, const CDamageInfo&);

  void Accept(IVisitor& visitor) override;
  void Think(float, CStateManager&) override;
  void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&) override;
};
} // namespace metaforce
