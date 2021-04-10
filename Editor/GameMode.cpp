#include "GameMode.hpp"

namespace metaforce {
void GameMode::think() { ViewerSpace::think(); }

void GameMode::View::draw(boo::IGraphicsCommandQueue* gfxQ) {
  if (m_gMode.m_main)
    m_gMode.m_main->Draw();
}

} // namespace metaforce
