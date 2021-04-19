#pragma once

#include <optional>

#include "Runtime/Collision/CCollidableAABox.hpp"
#include "Runtime/World/CActor.hpp"

#include <zeus/CAxisAngle.hpp>
#include <zeus/CQuaternion.hpp>
#include <zeus/CTransform.hpp>
#include <zeus/CVector3f.hpp>

namespace metaforce {
class CCollisionInfoList;
struct SMoverData;

struct SMoverData {
  zeus::CVector3f x0_velocity;
  zeus::CAxisAngle xc_angularVelocity;
  zeus::CVector3f x18_momentum;
  zeus::CAxisAngle x24_;
  float x30_mass;

  explicit SMoverData(float mass) : x30_mass(mass) {}
};

struct CMotionState {
  zeus::CVector3f x0_translation;
  zeus::CNUQuaternion xc_orientation;
  zeus::CVector3f x1c_velocity;
  zeus::CAxisAngle x28_angularMomentum;
  CMotionState(const zeus::CVector3f& origin, const zeus::CNUQuaternion& orientation, const zeus::CVector3f& velocity,
               const zeus::CAxisAngle& angle)
  : x0_translation(origin), xc_orientation(orientation), x1c_velocity(velocity), x28_angularMomentum(angle) {}
  CMotionState(const zeus::CVector3f& origin, const zeus::CNUQuaternion& orientation)
  : x0_translation(origin), xc_orientation(orientation) {}
};

class CPhysicsState {
  zeus::CVector3f x0_translation;
  zeus::CQuaternion xc_orientation;
  zeus::CVector3f x1c_constantForce;
  zeus::CAxisAngle x28_angularMomentum;
  zeus::CVector3f x34_momentum;
  zeus::CVector3f x40_force;
  zeus::CVector3f x4c_impulse;
  zeus::CAxisAngle x58_torque;
  zeus::CAxisAngle x64_angularImpulse;

public:
  CPhysicsState(const zeus::CVector3f& translation, const zeus::CQuaternion& orient,
                const zeus::CVector3f& constantForce, const zeus::CAxisAngle& angularMomentum,
                const zeus::CVector3f& momentum, const zeus::CVector3f& force, const zeus::CVector3f& impulse,
                const zeus::CAxisAngle& torque, const zeus::CAxisAngle& angularImpulse)
  : x0_translation(translation)
  , xc_orientation(orient)
  , x1c_constantForce(constantForce)
  , x28_angularMomentum(angularMomentum)
  , x34_momentum(momentum)
  , x40_force(force)
  , x4c_impulse(impulse)
  , x58_torque(torque)
  , x64_angularImpulse(angularImpulse) {}

  void SetTranslation(const zeus::CVector3f& tr) { x0_translation = tr; }
  void SetOrientation(const zeus::CQuaternion& orient) { xc_orientation = orient; }
  const zeus::CQuaternion& GetOrientation() const { return xc_orientation; }
  const zeus::CVector3f& GetTranslation() const { return x0_translation; }
  const zeus::CVector3f& GetConstantForceWR() const { return x1c_constantForce; }
  const zeus::CAxisAngle& GetAngularMomentumWR() const { return x28_angularMomentum; }
  const zeus::CVector3f& GetMomentumWR() const { return x34_momentum; }
  const zeus::CVector3f& GetForceWR() const { return x40_force; }
  const zeus::CVector3f& GetImpulseWR() const { return x4c_impulse; }
  const zeus::CAxisAngle& GetTorque() const { return x58_torque; }
  const zeus::CAxisAngle& GetAngularImpulseWR() const { return x64_angularImpulse; }
};

class CPhysicsActor : public CActor {
  friend class CGroundMovement;

protected:
  float xe8_mass;
  float xec_massRecip;
  float xf0_inertiaTensor = 0.f;
  float xf4_inertiaTensorRecip = 0.f;
  bool xf8_24_movable : 1 = true;
  bool xf8_25_angularEnabled : 1 = false;
  bool xf9_standardCollider = false;
  zeus::CVector3f xfc_constantForce;
  zeus::CAxisAngle x108_angularMomentum;
  zeus::CMatrix3f x114_;
  zeus::CVector3f x138_velocity;
  zeus::CAxisAngle x144_angularVelocity;
  zeus::CVector3f x150_momentum;
  zeus::CVector3f x15c_force;
  zeus::CVector3f x168_impulse;
  zeus::CAxisAngle x174_torque;
  zeus::CAxisAngle x180_angularImpulse;
  zeus::CVector3f x18c_moveImpulse;
  zeus::CAxisAngle x198_moveAngularImpulse;
  zeus::CAABox x1a4_baseBoundingBox;
  CCollidableAABox x1c0_collisionPrimitive;
  zeus::CVector3f x1e8_primitiveOffset;
  CMotionState x1f4_lastNonCollidingState;
  std::optional<zeus::CVector3f> x228_lastFloorPlaneNormal;
  float x238_maximumCollisionVelocity = 1000000.0f;
  float x23c_stepUpHeight;
  float x240_stepDownHeight;
  float x244_restitutionCoefModifier = 0.f;
  float x248_collisionAccuracyModifier = 1.f;
  u32 x24c_numTicksStuck = 0;
  u32 x250_numTicksPartialUpdate = 0;

public:
  CPhysicsActor(TUniqueId uid, bool active, std::string_view name, const CEntityInfo& info, const zeus::CTransform& xf,
                CModelData&& mData, const CMaterialList& matList, const zeus::CAABox& box, const SMoverData& moverData,
                const CActorParameters& actorParms, float stepUp, float stepDown);

  void Render(CStateManager& mgr) override;
  zeus::CVector3f GetOrbitPosition(const CStateManager& mgr) const override;
  zeus::CVector3f GetAimPosition(const CStateManager& mgr, float val) const override;
  virtual const CCollisionPrimitive* GetCollisionPrimitive() const;
  virtual zeus::CTransform GetPrimitiveTransform() const;
  virtual void CollidedWith(TUniqueId id, const CCollisionInfoList& list, CStateManager& mgr);
  virtual float GetStepUpHeight() const;
  virtual float GetStepDownHeight() const;
  virtual float GetWeight() const;

  float GetMass() const { return xe8_mass; }
  void SetPrimitiveOffset(const zeus::CVector2f& offset);
  const zeus::CVector3f& GetPrimitiveOffset() const { return x1e8_primitiveOffset; }
  void MoveCollisionPrimitive(const zeus::CVector3f& offset);
  void SetBoundingBox(const zeus::CAABox& box);
  zeus::CAABox GetMotionVolume(float dt) const;
  zeus::CVector3f CalculateNewVelocityWR_UsingImpulses() const;
  zeus::CAABox GetBoundingBox() const;
  const zeus::CAABox& GetBaseBoundingBox() const;
  void AddMotionState(const CMotionState& mst);
  CMotionState GetMotionState() const;
  const CMotionState& GetLastNonCollidingState() const { return x1f4_lastNonCollidingState; }
  void SetLastNonCollidingState(const CMotionState& mst) { x1f4_lastNonCollidingState = mst; }
  void SetMotionState(const CMotionState& mst);
  float GetMaximumCollisionVelocity() const { return x238_maximumCollisionVelocity; }
  void SetMaximumCollisionVelocity(float velocity) { x238_maximumCollisionVelocity = velocity; }
  void SetInertiaTensorScalar(float tensor);
  void SetMass(float mass);
  void SetAngularVelocityOR(const zeus::CAxisAngle& angVel);
  zeus::CAxisAngle GetAngularVelocityOR() const;
  const zeus::CAxisAngle& GetAngularVelocityWR() const { return x144_angularVelocity; }
  void SetAngularVelocityWR(const zeus::CAxisAngle& angVel);
  const zeus::CVector3f& GetForceOR() const { return x15c_force; }
  const zeus::CVector3f& GetImpulseOR() const { return x168_impulse; }
  const zeus::CVector3f& GetMoveImpulseOR() const { return x18c_moveImpulse; }
  void SetVelocityWR(const zeus::CVector3f& vel);
  void SetVelocityOR(const zeus::CVector3f& vel);
  void SetMomentumWR(const zeus::CVector3f& momentum) { x150_momentum = momentum; }
  const zeus::CVector3f& GetConstantForce() const { return xfc_constantForce; }
  void SetConstantForce(const zeus::CVector3f& force) { xfc_constantForce = force; }
  const zeus::CVector3f& GetAngularMomentum() const { return x108_angularMomentum; }
  void SetAngularMomentum(const zeus::CAxisAngle& momentum) { x108_angularMomentum = momentum; }
  const zeus::CVector3f& GetMomentum() const { return x150_momentum; }
  const zeus::CVector3f& GetVelocity() const { return x138_velocity; }
  const zeus::CAxisAngle& GetAngularImpulse() const { return x180_angularImpulse; }
  void SetAngularImpulse(const zeus::CAxisAngle& impulse) { x180_angularImpulse = impulse; }
  zeus::CVector3f GetTotalForcesWR() const;
  void RotateInOneFrameOR(const zeus::CQuaternion& q, float d);
  void MoveInOneFrameOR(const zeus::CVector3f& trans, float d);
  void RotateToOR(const zeus::CQuaternion& q, float d);
  void MoveToOR(const zeus::CVector3f& trans, float d);
  void MoveToInOneFrameWR(const zeus::CVector3f& v1, float d);
  void MoveToWR(const zeus::CVector3f& trans, float d);
  zeus::CAxisAngle GetRotateToORAngularMomentumWR(const zeus::CQuaternion& q, float d) const;
  zeus::CVector3f GetMoveToORImpulseWR(const zeus::CVector3f& trans, float d) const;
  void ClearImpulses();
  void ClearForcesAndTorques();
  void Stop();
  void ComputeDerivedQuantities();
  bool WillMove(const CStateManager& mgr) const;
  void SetPhysicsState(const CPhysicsState& state);
  CPhysicsState GetPhysicsState() const;
  bool IsMovable() const { return xf8_24_movable; }
  void SetMovable(bool movable) { xf8_24_movable = movable; }
  bool IsAngularEnabled() const { return xf8_25_angularEnabled; }
  void SetAngularEnabled(bool enabled) { xf8_25_angularEnabled = enabled; }
  float GetCollisionAccuracyModifier() const { return x248_collisionAccuracyModifier; }
  void SetCollisionAccuracyModifier(float modifier) { x248_collisionAccuracyModifier = modifier; }
  float GetCoefficientOfRestitutionModifier() const { return x244_restitutionCoefModifier; }
  void SetCoefficientOfRestitutionModifier(float modifier) { x244_restitutionCoefModifier = modifier; }
  bool IsUseStandardCollider() const { return xf9_standardCollider; }
  u32 GetNumTicksPartialUpdate() const { return x250_numTicksPartialUpdate; }
  void SetNumTicksPartialUpdate(u32 ticks) { x250_numTicksPartialUpdate = ticks; }
  u32 GetNumTicksStuck() const { return x24c_numTicksStuck; }
  void SetNumTicksStuck(u32 ticks) { x24c_numTicksStuck = ticks; }
  const std::optional<zeus::CVector3f>& GetLastFloorPlaneNormal() const {
    return x228_lastFloorPlaneNormal;
  }
  void SetLastFloorPlaneNormal(const std::optional<zeus::CVector3f>& normal) { x228_lastFloorPlaneNormal = normal; }

  CMotionState PredictMotion_Internal(float) const;
  CMotionState PredictMotion(float dt) const;
  CMotionState PredictLinearMotion(float dt) const;
  CMotionState PredictAngularMotion(float dt) const;
  void ApplyForceOR(const zeus::CVector3f& force, const zeus::CAxisAngle& angle);
  void ApplyForceWR(const zeus::CVector3f& force, const zeus::CAxisAngle& angle);
  void ApplyImpulseOR(const zeus::CVector3f& impulse, const zeus::CAxisAngle& angle);
  void ApplyImpulseWR(const zeus::CVector3f& impulse, const zeus::CAxisAngle& angle);
  void ApplyTorqueWR(const zeus::CVector3f& torque);

  void UseCollisionImpulses();
  static constexpr float GravityConstant() { return 9.81f * 2.5f; } /* 9.81 m/s ^ 2 is normal acceleration under earth gravity, Tallon 4 is 2.5 times that */
};
} // namespace metaforce
