#pragma once

#include "Runtime/GCNTypes.hpp"
#include "Runtime/Collision/CCollisionPrimitive.hpp"

#include <zeus/CAABox.hpp>

namespace metaforce {
namespace Collide {
bool AABox_AABox(const CInternalCollisionStructure&, CCollisionInfoList&);
bool AABox_AABox_Bool(const CInternalCollisionStructure&);
} // namespace Collide

class CCollidableAABox : public CCollisionPrimitive {
  static inline u32 sTableIndex = UINT32_MAX;

  zeus::CAABox x10_aabox;

public:
  CCollidableAABox();
  CCollidableAABox(const zeus::CAABox&, const CMaterialList&);

  zeus::CAABox Transform(const zeus::CTransform&) const;
  u32 GetTableIndex() const override;
  zeus::CAABox CalculateAABox(const zeus::CTransform&) const override;
  zeus::CAABox CalculateLocalAABox() const override;
  FourCC GetPrimType() const override;
  CRayCastResult CastRayInternal(const CInternalRayCastStructure&) const override;
  const zeus::CAABox& GetBox() const { return x10_aabox; }
  zeus::CAABox& Box() { return x10_aabox; }
  void SetBox(const zeus::CAABox& box) { x10_aabox = box; }

  static const CCollisionPrimitive::Type& GetType();
  static void SetStaticTableIndex(u32 index);
  static bool CollideMovingAABox(const CInternalCollisionStructure&, const zeus::CVector3f&, double&, CCollisionInfo&);
  static bool CollideMovingSphere(const CInternalCollisionStructure&, const zeus::CVector3f&, double&, CCollisionInfo&);
};
} // namespace metaforce
