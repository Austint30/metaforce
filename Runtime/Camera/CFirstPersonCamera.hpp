#pragma once

#include "Runtime/RetroTypes.hpp"
#include "Runtime/Camera/CGameCamera.hpp"

#include <zeus/CTransform.hpp>
#include <zeus/CVector3f.hpp>

namespace metaforce {

class CFirstPersonCamera : public CGameCamera {
  float x188_orbitCameraSpeed;
  bool x18c_lockCamera = false;
  zeus::CTransform x190_gunFollowXf;
  float x1c0_pitch = 0.f;
  TUniqueId x1c4_pitchId = kInvalidUniqueId;
  bool x1c6_24_deferBallTransitionProcessing : 1 = false;
  zeus::CVector3f x1c8_closeInVec;
  float x1d4_closeInTimer = 0.f;
  std::vector<zeus::CTransform> m_xrViews;
  void _fovListener(hecl::CVar* cv);

public:
  DEFINE_ENTITY
  CFirstPersonCamera(TUniqueId, const zeus::CTransform& xf, TUniqueId, float orbitCameraSpeed, float fov,
                     float nearplane, float farplane, float aspect);

  void Accept(IVisitor& visitor) override;
  void PreThink(float dt, CStateManager& mgr) override;
  void Think(float dt, CStateManager& mgr) override;
  void ProcessInput(const CFinalInput&, CStateManager& mgr) override;
  void Reset(const zeus::CTransform&, CStateManager& mgr) override;

  void SkipCinematic();
  const zeus::CTransform& GetGunFollowTransform() const { return x190_gunFollowXf; }
  void UpdateTransform(CStateManager& mgr, float dt);
  void UpdateElevation(CStateManager& mgr);
  void CalculateGunFollowOrientationAndTransform(zeus::CTransform&, zeus::CQuaternion&, float, zeus::CVector3f&) const;
  void SetScriptPitchId(TUniqueId uid) { x1c4_pitchId = uid; }
  void SetLockCamera(bool v) { x18c_lockCamera = v; }
  void DeferBallTransitionProcessing() { x1c6_24_deferBallTransitionProcessing = true; }

  void ProcessVRInput(const CVRInput& input, CStateManager& mgr) override {
    m_xrViews = input.m_eyeTransforms;
  }
  zeus::CTransform GetTransformVR() const override {
    float camPitch = zeus::CQuaternion(x34_transform.getRotation().buildMatrix3f()).pitch();
    zeus::CTransform xLockedTransform = x34_transform * zeus::CTransform::RotateX(-camPitch);
    if (m_xrViews.size() != 2){
      return xLockedTransform;
    }
    return xLockedTransform * m_xrViews[0];
  }
};
} // namespace metaforce
