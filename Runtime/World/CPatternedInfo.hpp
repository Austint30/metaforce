#pragma once

#include "Runtime/RetroTypes.hpp"
#include "Runtime/World/CAnimationParameters.hpp"
#include "Runtime/World/CDamageInfo.hpp"
#include "Runtime/World/CDamageVulnerability.hpp"
#include "Runtime/World/CHealthInfo.hpp"

#include <zeus/CVector3f.hpp>

namespace urde {

class CPatternedInfo {
  friend class CPatterned;
  float x0_mass;
  float x4_speed;
  float x8_turnSpeed;
  float xc_detectionRange;
  float x10_detectionHeightRange;
  float x14_dectectionAngle;
  float x18_minAttackRange;
  float x1c_maxAttackRange;
  float x20_averageAttackTime;
  float x24_attackTimeVariation;
  float x28_leashRadius;
  float x2c_playerLeashRadius;
  float x30_playerLeashTime;
  CDamageInfo x34_contactDamageInfo;
  float x50_damageWaitTime;
  CHealthInfo x54_healthInfo;
  CDamageVulnerability x5c_damageVulnerability;
  float xc4_halfExtent;
  float xc8_height;
  zeus::CVector3f xcc_bodyOrigin;
  float xd8_stepUpHeight;
  float xdc_xDamage;
  float xe0_frozenXDamage;
  float xe4_xDamageDelay;
  u16 xe8_deathSfx;
  CAnimationParameters xec_animParams;
  bool xf8_active;
  CAssetId xfc_stateMachineId;
  float x100_intoFreezeDur;
  float x104_outofFreezeDur;
  float x108_freezeDur;

  u32 x10c_pathfindingIndex;

  zeus::CVector3f x110_particle1Scale;
  CAssetId x11c_particle1;
  CAssetId x120_electric;
  zeus::CVector3f x124_particle2Scale;
  CAssetId x130_particle2;

  u16 x134_iceShatterSfx = 0xffff;

public:
  CPatternedInfo(CInputStream& in, u32 pcount);
  static std::pair<bool, u32> HasCorrectParameterCount(CInputStream& in);

  float GetTurnSpeed() const { return x8_turnSpeed; }
  float GetDetectionHeightRange() const { return x10_detectionHeightRange; }
  const CHealthInfo& GetHealthInfo() const { return x54_healthInfo; }
  const CDamageVulnerability& GetDamageVulnerability() const { return x5c_damageVulnerability; }
  float GetHalfExtent() const { return xc4_halfExtent; }
  float GetHeight() const { return xc8_height; }
  zeus::CVector3f GetBodyOrigin() const { return xcc_bodyOrigin; }
  CAnimationParameters& GetAnimationParameters() { return xec_animParams; }
  const CAnimationParameters& GetAnimationParameters() const { return xec_animParams; }
  u32 GetPathfindingIndex() const { return x10c_pathfindingIndex; }
  bool GetActive() const { return xf8_active; }
  void SetActive(bool active) { xf8_active = active; }
};
} // namespace urde
