#include "Runtime/GuiSys/CSaveableState.hpp"

#include "Runtime/GuiSys/CRasterFont.hpp"

namespace metaforce {

bool CSaveableState::IsFinishedLoading() const {
  if (!x48_font || !x48_font.IsLoaded())
    return false;
  return x48_font->IsFinishedLoading();
}

} // namespace metaforce
