#pragma once

#include <optional>

#include "Runtime/RetroTypes.hpp"
#include "Runtime/Weapon/CEnergyProjectile.hpp"

namespace urde {

class CTargetableProjectile : public CEnergyProjectile {
  TLockedToken<CWeaponDescription> x3d8_weaponDesc;
  CDamageInfo x3e0_damage;

public:
  CTargetableProjectile(const TToken<CWeaponDescription>& desc, EWeaponType type, const zeus::CTransform& xf,
                        EMaterialTypes materials, const CDamageInfo& damage, const CDamageInfo& damage2, TUniqueId uid,
                        TAreaId aid, TUniqueId owner, const TLockedToken<CWeaponDescription>& weapDesc,
                        TUniqueId homingTarget, EProjectileAttrib attribs,
                        const std::optional<TLockedToken<CGenDescription>>& visorParticle, u16 visorSfx,
                        bool sendCollideMsg);

  void Accept(IVisitor&) override;
  zeus::CVector3f GetAimPosition(const CStateManager&, float) const override;
  bool Explode(const zeus::CVector3f& pos, const zeus::CVector3f& normal, EWeaponCollisionResponseTypes type,
               CStateManager& mgr, const CDamageVulnerability& dVuln, TUniqueId hitActor) override;
  void ResolveCollisionWithActor(const CRayCastResult& res, CActor& act, CStateManager& mgr) override;
};

} // namespace urde
