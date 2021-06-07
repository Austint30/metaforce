#include "Runtime/World/CScriptRipple.hpp"

#include "Runtime/CStateManager.hpp"
#include "Runtime/World/CScriptWater.hpp"

#include "TCastTo.hpp" // Generated file, do not modify include path

namespace metaforce {

CScriptRipple::CScriptRipple(TUniqueId uid, std::string_view name, const CEntityInfo& info, const zeus::CVector3f& vec,
                             bool active, float f1)
: CEntity(uid, info, active, name), x34_magnitude(f1 >= 0.f ? f1 : 0.5f), x38_center(vec) {}

void CScriptRipple::Accept(IVisitor& visitor) { visitor.Visit(this); }

void CScriptRipple::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  if (msg != EScriptObjectMessage::Play) {
    CEntity::AcceptScriptMsg(msg, uid, mgr);
    return;
  }

  if (!GetActive()) {
    return;
  }

  for (const SConnection& conn : x20_conns) {
    if (conn.x0_state != EScriptObjectState::Active || conn.x4_msg != EScriptObjectMessage::Next) {
      continue;
    }

    const auto search = mgr.GetIdListForScript(conn.x8_objId);
    for (auto it = search.first; it != search.second; ++it) {
      if (const TCastToPtr<CScriptWater> water = mgr.ObjectById(it->second)) {
        water->GetFluidPlane().AddRipple(x34_magnitude, GetUniqueId(), x38_center, *water, mgr);
      }
    }
  }
}
} // namespace metaforce
