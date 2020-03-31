#pragma once

#include <memory>

#include "Runtime/IOStreams.hpp"
#include "Runtime/Character/IMetaAnim.hpp"

namespace urde {

class CMetaAnimPhaseBlend : public IMetaAnim {
  std::shared_ptr<IMetaAnim> x4_animA;
  std::shared_ptr<IMetaAnim> x8_animB;
  float xc_blend;
  bool x10_;

public:
  explicit CMetaAnimPhaseBlend(CInputStream& in);
  EMetaAnimType GetType() const override { return EMetaAnimType::PhaseBlend; }

  void GetUniquePrimitives(std::set<CPrimitive>& primsOut) const override;
  std::shared_ptr<CAnimTreeNode> VGetAnimationTree(const CAnimSysContext& animSys,
                                                   const CMetaAnimTreeBuildOrders& orders) const override;
};

} // namespace urde
