#pragma once

#include <optional>
#include <vector>

#include "Runtime/RetroTypes.hpp"
#include "Runtime/World/CActor.hpp"

#include <zeus/CAABox.hpp>

namespace metaforce {
class CScriptSpiderBallWaypoint : public CActor {
  enum class ECheckActiveWaypoint { Check, SkipCheck };
  u32 xe8_;
  std::vector<TUniqueId> xec_waypoints;
  std::optional<zeus::CAABox> xfc_aabox;

public:
  CScriptSpiderBallWaypoint(TUniqueId, std::string_view, const CEntityInfo&, const zeus::CTransform&, bool, u32);
  void Accept(IVisitor&) override;
  void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&) override;
  void Render(CStateManager& mgr) override { CActor::Render(mgr); }
  void AddToRenderer(const zeus::CFrustum&, CStateManager&) override {}
  std::optional<zeus::CAABox> GetTouchBounds() const override { return xfc_aabox; }
  void AccumulateBounds(const zeus::CVector3f& v);
  void BuildWaypointListAndBounds(CStateManager& mgr);
  void AddPreviousWaypoint(TUniqueId uid);
  TUniqueId PreviousWaypoint(const CStateManager& mgr, ECheckActiveWaypoint checkActive) const;
  TUniqueId NextWaypoint(const CStateManager& mgr, ECheckActiveWaypoint checkActive) const;
  void GetClosestPointAlongWaypoints(CStateManager& mgr, const zeus::CVector3f& ballPos, float maxPointToBallDist,
                                     const CScriptSpiderBallWaypoint*& closestWaypoint, zeus::CVector3f& closestPoint,
                                     zeus::CVector3f& deltaBetweenPoints, float deltaBetweenInterpDist,
                                     zeus::CVector3f& interpDeltaBetweenPoints) const;
  void ClearWaypoints() {
    xfc_aabox.reset();
    xec_waypoints.clear();
  }
};
} // namespace metaforce
