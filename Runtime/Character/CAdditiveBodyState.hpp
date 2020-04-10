#pragma once

#include <array>

#include "Runtime/RetroTypes.hpp"
#include "Runtime/Character/CBodyStateCmdMgr.hpp"
#include "Runtime/Character/CharacterCommon.hpp"

namespace urde {
class CActor;
class CBodyController;
class CStateManager;

class CAdditiveBodyState {
public:
  virtual ~CAdditiveBodyState() = default;
  virtual bool ApplyHeadTracking() const { return true; }
  virtual bool CanShoot() const { return true; }
  virtual void Start(CBodyController& bc, CStateManager& mgr) = 0;
  virtual pas::EAnimationState UpdateBody(float dt, CBodyController& bc, CStateManager& mgr) = 0;
  virtual void Shutdown(CBodyController& bc) = 0;
};

class CABSAim : public CAdditiveBodyState {
  bool x4_needsIdle = false;
  std::array<s32, 4> x8_anims{};
  std::array<float, 4> x18_angles{};
  float x28_hWeight = 0.f;
  float x2c_hWeightVel = 0.f;
  float x30_vWeight = 0.f;
  float x34_vWeightVel = 0.f;
  pas::EAnimationState GetBodyStateTransition(float dt, CBodyController& bc) const;

public:
  void Start(CBodyController& bc, CStateManager& mgr) override;
  pas::EAnimationState UpdateBody(float dt, CBodyController& bc, CStateManager& mgr) override;
  void Shutdown(CBodyController& bc) override;
};

class CABSFlinch : public CAdditiveBodyState {
  float x4_weight = 1.f;
  u32 x8_anim = 0;
  pas::EAnimationState GetBodyStateTransition(float dt, CBodyController& bc) const;

public:
  void Start(CBodyController& bc, CStateManager& mgr) override;
  pas::EAnimationState UpdateBody(float dt, CBodyController& bc, CStateManager& mgr) override;
  void Shutdown(CBodyController& bc) override {}
};

class CABSIdle : public CAdditiveBodyState {
  pas::EAnimationState GetBodyStateTransition(float dt, CBodyController& bc) const;

public:
  void Start(CBodyController& bc, CStateManager& mgr) override {}
  pas::EAnimationState UpdateBody(float dt, CBodyController& bc, CStateManager& mgr) override;
  void Shutdown(CBodyController& bc) override {}
};

class CABSReaction : public CAdditiveBodyState {
  float x4_weight = 1.f;
  s32 x8_anim = -1;
  pas::EAdditiveReactionType xc_type = pas::EAdditiveReactionType::Invalid;
  bool x10_active = false;
  pas::EAnimationState GetBodyStateTransition(float dt, CBodyController& bc) const;
  void StopAnimation(CBodyController& bc);

public:
  void Start(CBodyController& bc, CStateManager& mgr) override;
  pas::EAnimationState UpdateBody(float dt, CBodyController& bc, CStateManager& mgr) override;
  void Shutdown(CBodyController& bc) override { StopAnimation(bc); }
};

} // namespace urde
