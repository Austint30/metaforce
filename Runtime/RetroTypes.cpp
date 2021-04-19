#include "Runtime/RetroTypes.hpp"

#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/IMain.hpp"

#include <logvisor/logvisor.hpp>

namespace metaforce {
logvisor::Module Log("metaforce::RetroTypes::CAssetId");

CAssetId::CAssetId(CInputStream& in) {
  if (g_Main) {
    if (g_Main->GetExpectedIdSize() == sizeof(u32))
      Assign(in.readUint32Big());
    else if (g_Main->GetExpectedIdSize() == sizeof(u64))
      Assign(in.readUint64Big());
    else
      Log.report(logvisor::Fatal, FMT_STRING("Unsupported id length {}"), g_Main->GetExpectedIdSize());
  } else
    Log.report(logvisor::Fatal, FMT_STRING("Input constructor called before runtime Main entered!"));
}

void CAssetId::PutTo(COutputStream& out) {
  if (g_Main) {
    if (g_Main->GetExpectedIdSize() == sizeof(u32))
      out.writeUint32Big(u32(id));
    else if (g_Main->GetExpectedIdSize() == sizeof(u64))
      out.writeUint64Big(id);
    else
      Log.report(logvisor::Fatal, FMT_STRING("Unsupported id length {}"), g_Main->GetExpectedIdSize());
  } else
    Log.report(logvisor::Fatal, FMT_STRING("PutTo called before runtime Main entered!"));
}

} // namespace metaforce