#pragma once

#include <optional>
#include <string_view>

#include "Runtime/CToken.hpp"
#include "Runtime/GuiSys/CStringTable.hpp"
#include "Runtime/World/CEntity.hpp"
#include "Runtime/World/CHUDMemoParms.hpp"

namespace metaforce {

class CScriptHUDMemo : public CEntity {
public:
  enum class EDisplayType {
    StatusMessage,
    MessageBox,
  };
  CHUDMemoParms x34_parms;
  EDisplayType x3c_dispType;
  CAssetId x40_stringTableId;
  std::optional<TLockedToken<CStringTable>> x44_stringTable;

private:
public:
  DEFINE_ENTITY
  CScriptHUDMemo(TUniqueId, std::string_view, const CEntityInfo&, const CHUDMemoParms&, CScriptHUDMemo::EDisplayType,
                 CAssetId, bool);

  void Accept(IVisitor& visitor) override;
  void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&) override;
};
} // namespace metaforce
