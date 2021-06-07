#pragma once

#include <memory>
#include <string>

#include "Runtime/GCNTypes.hpp"
#include "Runtime/Character/CAnimSysContext.hpp"
#include "Runtime/Character/CAnimTreeSingleChild.hpp"
#include "Runtime/Character/CSequenceHelper.hpp"

namespace metaforce {

class CAnimTreeLoopIn : public CAnimTreeSingleChild {
  std::shared_ptr<CAnimTreeNode> x18_nextAnim;
  bool x1c_didLoopIn = false;
  CAnimSysContext x20_animCtx;
  CSequenceFundamentals x30_fundamentals;
  CCharAnimTime x88_curTime;

public:
  static std::string CreatePrimitiveName(const std::weak_ptr<CAnimTreeNode>& a, const std::weak_ptr<CAnimTreeNode>& b,
                                         const std::weak_ptr<CAnimTreeNode>& c);
  CAnimTreeLoopIn(const std::weak_ptr<CAnimTreeNode>& a, const std::weak_ptr<CAnimTreeNode>& b,
                  const std::weak_ptr<CAnimTreeNode>& c, const CAnimSysContext& animCtx, std::string_view name);
  CAnimTreeLoopIn(const std::weak_ptr<CAnimTreeNode>& a, const std::weak_ptr<CAnimTreeNode>& b, bool didLoopIn,
                  CAnimSysContext animCtx, std::string_view name, CSequenceFundamentals fundamentals,
                  const CCharAnimTime& time);
  CAnimTreeEffectiveContribution VGetContributionOfHighestInfluence() const override;
  bool VSupportsReverseView() const { return false; }
  std::optional<std::unique_ptr<IAnimReader>> VSimplified() override;
  std::shared_ptr<IAnimReader> VGetBestUnblendedChild() const override;
  std::unique_ptr<IAnimReader> VClone() const override;
  size_t VGetBoolPOIList(const CCharAnimTime& time, CBoolPOINode* listOut, size_t capacity, size_t iterator,
                         u32) const override;
  size_t VGetInt32POIList(const CCharAnimTime& time, CInt32POINode* listOut, size_t capacity, size_t iterator,
                          u32) const override;
  size_t VGetParticlePOIList(const CCharAnimTime& time, CParticlePOINode* listOut, size_t capacity, size_t iterator,
                             u32) const override;
  size_t VGetSoundPOIList(const CCharAnimTime& time, CSoundPOINode* listOut, size_t capacity, size_t iterator,
                          u32) const override;
  CSteadyStateAnimInfo VGetSteadyStateAnimInfo() const override;
  CCharAnimTime VGetTimeRemaining() const override;
  SAdvancementResults VAdvanceView(const CCharAnimTime& dt) override;
};

} // namespace metaforce
