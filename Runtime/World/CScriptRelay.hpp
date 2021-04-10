#pragma once

#include <string_view>

#include "Runtime/RetroTypes.hpp"
#include "Runtime/World/CEntity.hpp"

namespace metaforce {
class CScriptRelay : public CEntity {
  TUniqueId x34_nextRelay = kInvalidUniqueId;
  u32 x38_sendCount = 0;

public:
  CScriptRelay(TUniqueId, std::string_view, const CEntityInfo&, bool);

  void Accept(IVisitor& visitor) override;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId objId, CStateManager& stateMgr) override;
  void Think(float, CStateManager& stateMgr) override;
  void UpdateObjectRef(CStateManager& stateMgr);
};
} // namespace metaforce
