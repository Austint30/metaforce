#include "Runtime/CGameOptionsTouchBar.hpp"

namespace urde {

CGameOptionsTouchBar::EAction CGameOptionsTouchBar::PopAction() { return EAction::None; }

void CGameOptionsTouchBar::GetSelection(int& left, int& right, int& value) {
  left = -1;
  right = -1;
  value = -1;
}

void CGameOptionsTouchBar::SetSelection([[maybe_unused]] int left, [[maybe_unused]] int right, [[maybe_unused]] int value) {
}

#ifndef __APPLE__
std::unique_ptr<CGameOptionsTouchBar> NewGameOptionsTouchBar() { return std::make_unique<CGameOptionsTouchBar>(); }
#endif

} // namespace urde
