#pragma once

#include "Runtime/RetroTypes.hpp"

namespace metaforce {

class CHealthInfo {
  float x0_health;
  float x4_knockbackResistance;

public:
  explicit CHealthInfo(float hp) : x0_health(hp), x4_knockbackResistance(0.f) {}

  CHealthInfo(float hp, float resist) : x0_health(hp), x4_knockbackResistance(resist) {}

  explicit CHealthInfo(CInputStream& in);
  void SetHP(float hp) { x0_health = hp; }
  float GetHP() const { return x0_health; }
  float GetKnockbackResistance() const { return x4_knockbackResistance; }
  void SetKnockbackResistance(float r) { x4_knockbackResistance = r; }
};

} // namespace metaforce
