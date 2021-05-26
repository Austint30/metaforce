#pragma once

#include <string_view>

#include "Runtime/RetroTypes.hpp"
#include "Runtime/rstl.hpp"
#include "Runtime/World/CActor.hpp"

namespace metaforce {

class CScriptPlayerHint : public CActor {
  rstl::reserved_vector<TUniqueId, 8> xe8_objectList;
  bool xfc_deactivated = false;
  u32 x100_priority;
  u32 x104_overrideFlags;
  TUniqueId x108_mpId = kInvalidUniqueId;
  void AddToObjectList(TUniqueId uid);
  void RemoveFromObjectList(TUniqueId uid);

public:
  DEFINE_ENTITY
  CScriptPlayerHint(TUniqueId uid, std::string_view name, const CEntityInfo& info, const zeus::CTransform& xf,
                    bool active, u32 priority, u32 overrideFlags);
  void Accept(IVisitor& visit) override;
  void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&) override;
  u32 GetPriority() const { return x100_priority; }
  u32 GetOverrideFlags() const { return x104_overrideFlags; }
  TUniqueId GetActorId() const { return x108_mpId; }
  void ClearObjectList() { xe8_objectList.clear(); }
  void SetDeactivated() { xfc_deactivated = true; }
};

} // namespace metaforce
