#pragma once

#include <vector>

#include "Runtime/CToken.hpp"

#include <zeus/CVector2f.hpp>

namespace metaforce {
class CTexture;

class CFontImageDef {
public:
  float x0_fps;
  std::vector<TLockedToken<CTexture>> x4_texs;
  zeus::CVector2f x14_cropFactor;

  CFontImageDef(const std::vector<TToken<CTexture>>& texs, float fps, const zeus::CVector2f& cropFactor);
  CFontImageDef(const TToken<CTexture>& tex, const zeus::CVector2f& cropFactor);
  bool IsLoaded() const;
  s32 CalculateBaseline() const;
  s32 CalculateHeight() const;
};

} // namespace metaforce
