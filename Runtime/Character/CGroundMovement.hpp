#pragma once

#include <optional>

#include "Runtime/RetroTypes.hpp"
#include "Runtime/rstl.hpp"
#include "Runtime/Collision/CCollisionInfo.hpp"

#include <zeus/CVector3f.hpp>

namespace metaforce {
class CAreaCollisionCache;
class CCollisionInfoList;
class CMaterialFilter;
class CPhysicsActor;
class CStateManager;

class CGroundMovement {
public:
  struct SMovementOptions {
    bool x0_setWaterLandingForce;
    float x4_waterLandingForceCoefficient;
    float x8_minimumWaterLandingForce;
    float xc_anyZThreshold;
    float x10_downwardZThreshold;
    float x14_waterLandingVelocityReduction;
    bool x18_dampForceAndMomentum;
    bool x19_alwaysClip;
    bool x1a_disableClipForFloorOnly;
    u32 x1c_maxCollisionCycles;
    float x20_minimumTranslationDelta;
    float x24_dampedNormalCoefficient;
    float x28_dampedDeltaCoefficient;
    float x2c_floorElasticForce;
    float x30_wallElasticConstant;
    float x34_wallElasticLinear;
    float x38_maxPositiveVerticalVelocity;
    std::optional<zeus::CVector3f> x3c_floorPlaneNormal;
  };

  struct SMoveObjectResult {
    std::optional<TUniqueId> x0_id;
    std::optional<CCollisionInfo> x8_collision;
    u32 x6c_processedCollisions;
    float x70_processedDt;
  };

  static void CheckFalling(CPhysicsActor& actor, CStateManager& mgr, float);
  static void MoveGroundCollider(CStateManager& mgr, CPhysicsActor& actor, float dt, const EntityList* nearList);
  static bool ResolveUpDown(CAreaCollisionCache& cache, CStateManager& mgr, CPhysicsActor& actor,
                            const CMaterialFilter& filter, EntityList& nearList, float, float, float&,
                            CCollisionInfoList& list);
  static bool MoveGroundColliderZ(CAreaCollisionCache& cache, CStateManager& mgr, CPhysicsActor& actor,
                                  const CMaterialFilter& filter, EntityList& nearList, float, float&,
                                  CCollisionInfoList& list, TUniqueId& idOut);
  static void MoveGroundColliderXY(CAreaCollisionCache& cache, CStateManager& mgr, CPhysicsActor& actor,
                                   const CMaterialFilter& filter, EntityList& nearList, float);
  static zeus::CVector3f CollisionDamping(const zeus::CVector3f& a, const zeus::CVector3f& b, const zeus::CVector3f& c,
                                          float d, float e);
  static void MoveGroundCollider_New(CStateManager& mgr, CPhysicsActor& actor, float, const EntityList* nearList);
  static bool RemoveNormalComponent(const zeus::CVector3f&, const zeus::CVector3f&, zeus::CVector3f&, float&);
  static bool RemoveNormalComponent(const zeus::CVector3f& a, zeus::CVector3f& b);
  static CMaterialList MoveObjectAnalytical(CStateManager& mgr, CPhysicsActor& actor, float, EntityList& nearList,
                                            CAreaCollisionCache& cache, const SMovementOptions& opts,
                                            SMoveObjectResult& result);
};

} // namespace metaforce
