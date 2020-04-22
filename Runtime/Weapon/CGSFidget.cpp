#include "Runtime/Weapon/CGSFidget.hpp"

#include "Runtime/CStateManager.hpp"
#include "Runtime/Character/CPASAnimParmData.hpp"
#include "Runtime/Character/CAnimData.hpp"
#include "Runtime/Weapon/WeaponCommon.hpp"

namespace urde {
bool CGSFidget::Update(CAnimData& data, float dt, CStateManager& mgr) {
  return !data.IsAnimTimeRemaining(0.001f, "Whole Body");
}

s32 CGSFidget::SetAnim(CAnimData& data, s32 type, s32 gunId, s32 animSet, CStateManager& mgr) {
  const CPASDatabase& pas = data.GetCharacterInfo().GetPASDatabase();
  CPASAnimParmData parms(pas::EAnimationState::Getup, CPASAnimParm::FromEnum(type), CPASAnimParm::FromInt32(gunId),
                         CPASAnimParm::FromInt32(animSet));
  auto anim = pas.FindBestAnimation(parms, *mgr.GetActiveRandom(), -1);
  bool loop = pas.GetAnimState(pas::EAnimationState::Getup)->GetAnimParmData(anim.second, 3).GetBoolValue();
  x14_gunId = gunId;
  x18_animSet = animSet;
  if (anim.second != -1) {
    data.EnableLooping(loop);
    CAnimPlaybackParms aParms(anim.second, -1, 1.f, true);
    data.SetAnimation(aParms, false);
    UnLoadAnim();
  }
  return anim.second;
}

void CGSFidget::LoadAnimAsync(CAnimData& data, s32 type, s32 gunId, s32 animSet, CStateManager& mgr) {
  CPASAnimParmData parms(pas::EAnimationState::Getup, CPASAnimParm::FromEnum(type), CPASAnimParm::FromInt32(gunId),
                         CPASAnimParm::FromInt32(animSet));
  auto anim = data.GetCharacterInfo().GetPASDatabase().FindBestAnimation(parms, *mgr.GetActiveRandom(), -1);
  if (anim.second != -1)
    NWeaponTypes::get_token_vector(data, anim.second, x0_anims, true);
}

void CGSFidget::UnLoadAnim() { x0_anims.clear(); }

bool CGSFidget::IsAnimLoaded() const { return NWeaponTypes::are_tokens_ready(x0_anims); }
} // namespace urde
