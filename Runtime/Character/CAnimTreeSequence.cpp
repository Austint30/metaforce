#include "Runtime/Character/CAnimTreeSequence.hpp"

#include "Runtime/Character/CAnimSysContext.hpp"
#include "Runtime/Character/CTreeUtils.hpp"
#include "Runtime/Character/IMetaAnim.hpp"

namespace urde {

CAnimTreeSequence::CAnimTreeSequence(std::vector<std::shared_ptr<IMetaAnim>> seq, CAnimSysContext animSys,
                                     std::string_view name)
: CAnimTreeSingleChild(seq[0]->GetAnimationTree(animSys, CMetaAnimTreeBuildOrders::NoSpecialOrders()), name)
, x18_animCtx(std::move(animSys))
, x28_sequence(std::move(seq))
, x3c_fundamentals(CSequenceHelper(x28_sequence, x18_animCtx).ComputeSequenceFundamentals())
, x94_curTime(0.f) {}

CAnimTreeSequence::CAnimTreeSequence(const std::shared_ptr<CAnimTreeNode>& curNode,
                                     std::vector<std::shared_ptr<IMetaAnim>> metaAnims,
                                     CAnimSysContext animSys, std::string_view name,
                                     CSequenceFundamentals fundamentals, const CCharAnimTime& time)
: CAnimTreeSingleChild(curNode, name)
, x18_animCtx(std::move(animSys))
, x28_sequence(std::move(metaAnims))
, x3c_fundamentals(std::move(fundamentals))
, x94_curTime(time) {}

CAnimTreeEffectiveContribution CAnimTreeSequence::VGetContributionOfHighestInfluence() const {
  return x14_child->GetContributionOfHighestInfluence();
}

std::shared_ptr<IAnimReader> CAnimTreeSequence::VGetBestUnblendedChild() const {
  std::shared_ptr<IAnimReader> ch = x14_child->GetBestUnblendedChild();
  if (!ch)
    return ch;
  return std::make_shared<CAnimTreeSequence>(
      std::static_pointer_cast<CAnimTreeNode>(std::shared_ptr<IAnimReader>(ch->Clone())), x28_sequence, x18_animCtx,
      x4_name, x3c_fundamentals, x94_curTime);
}

SAdvancementResults CAnimTreeSequence::VAdvanceView(const CCharAnimTime& dt) {
  CCharAnimTime totalDelta;
  zeus::CVector3f posDelta;
  zeus::CQuaternion rotDelta;

  std::shared_ptr<CAnimTreeNode> curChild = x14_child;
  if (x38_curIdx >= x28_sequence.size() && curChild->VGetTimeRemaining().EqualsZero()) {
    x3c_fundamentals = CSequenceHelper(x28_sequence, x18_animCtx).ComputeSequenceFundamentals();
    x38_curIdx = 0;
    x14_child = CTreeUtils::GetTransitionTree(
        curChild, x28_sequence[x38_curIdx]->GetAnimationTree(x18_animCtx, CMetaAnimTreeBuildOrders::NoSpecialOrders()),
        x18_animCtx);
    curChild = x14_child;
  }

  CCharAnimTime remTime = dt;
  // Note: EpsilonZero check added
  while (remTime.GreaterThanZero() && !remTime.EpsilonZero() && x38_curIdx < x28_sequence.size()) {
    CCharAnimTime chRem = curChild->VGetTimeRemaining();
    if (chRem.EqualsZero()) {
      ++x38_curIdx;
      if (x38_curIdx < x28_sequence.size()) {
        x14_child = CTreeUtils::GetTransitionTree(
            curChild,
            x28_sequence[x38_curIdx]->GetAnimationTree(x18_animCtx, CMetaAnimTreeBuildOrders::NoSpecialOrders()),
            x18_animCtx);
      }
    }
    curChild = x14_child;
    if (x38_curIdx < x28_sequence.size()) {
      SAdvancementResults res = curChild->VAdvanceView(remTime);
      if (auto simp = curChild->Simplified()) {
        curChild = CAnimTreeNode::Cast(std::move(*simp));
        x14_child = curChild;
      }
      CCharAnimTime prevRemTime = remTime;
      remTime = res.x0_remTime;
      totalDelta += prevRemTime - remTime;
      posDelta += res.x8_deltas.x0_posDelta;
      rotDelta = rotDelta * res.x8_deltas.xc_rotDelta;
    }
  }

  x94_curTime += totalDelta;
  return {dt - totalDelta, {posDelta, rotDelta}};
}

CCharAnimTime CAnimTreeSequence::VGetTimeRemaining() const {
  if (x38_curIdx == x28_sequence.size() - 1)
    return x14_child->VGetTimeRemaining();
  return x3c_fundamentals.GetSteadyStateAnimInfo().GetDuration() - x94_curTime;
}

CSteadyStateAnimInfo CAnimTreeSequence::VGetSteadyStateAnimInfo() const {
  return x3c_fundamentals.GetSteadyStateAnimInfo();
}

size_t CAnimTreeSequence::VGetBoolPOIList(const CCharAnimTime& time, CBoolPOINode* listOut, size_t capacity,
                                          size_t iterator, u32 unk) const {
  return _getPOIList(time, listOut, capacity, iterator, unk, x3c_fundamentals.GetBoolPointsOfInterest(), x94_curTime);
}

size_t CAnimTreeSequence::VGetInt32POIList(const CCharAnimTime& time, CInt32POINode* listOut, size_t capacity,
                                           size_t iterator, u32 unk) const {
  return _getPOIList(time, listOut, capacity, iterator, unk, x3c_fundamentals.GetInt32PointsOfInterest(), x94_curTime);
}

size_t CAnimTreeSequence::VGetParticlePOIList(const CCharAnimTime& time, CParticlePOINode* listOut, size_t capacity,
                                              size_t iterator, u32 unk) const {
  return _getPOIList(time, listOut, capacity, iterator, unk, x3c_fundamentals.GetParticlePointsOfInterest(),
                     x94_curTime);
}

size_t CAnimTreeSequence::VGetSoundPOIList(const CCharAnimTime& time, CSoundPOINode* listOut, size_t capacity,
                                           size_t iterator, u32 unk) const {
  return _getPOIList(time, listOut, capacity, iterator, unk, x3c_fundamentals.GetSoundPointsOfInterest(), x94_curTime);
}

std::unique_ptr<IAnimReader> CAnimTreeSequence::VClone() const {
  return std::make_unique<CAnimTreeSequence>(
      std::static_pointer_cast<CAnimTreeNode>(std::shared_ptr<IAnimReader>(x14_child->Clone())), x28_sequence,
      x18_animCtx, x4_name, x3c_fundamentals, x94_curTime);
}

} // namespace urde
