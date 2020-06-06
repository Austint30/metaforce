#pragma once

#include <optional>
#include <vector>

#include "Runtime/CToken.hpp"
#include "Runtime/RetroTypes.hpp"

#include <zeus/CColor.hpp>

namespace urde {
class CActor;
class CStateManager;
class CTexture;

class CVisorFlare {
public:
  enum class EBlendMode {

  };
  class CFlareDef {
    TToken<CTexture> x0_tex;
    float x8_f1;
    float xc_f2;
    zeus::CColor x10_color;

  public:
    CFlareDef()=default;
    CFlareDef(const CFlareDef&)=default;
    CFlareDef(const TToken<CTexture>& tex, float f1, float f2, const zeus::CColor& color)
    : x0_tex(tex), x8_f1(f1), xc_f2(f2), x10_color(color) {
      x0_tex.Lock();
    }

    TToken<CTexture> GetTexture() const { return x0_tex; }
    zeus::CColor GetColor() const { return x10_color; }
    float GetScale() const;
    float GetPosition() const;
  };

private:
  EBlendMode x0_blendMode;
  std::vector<CFlareDef> x4_flareDefs;
  bool x14_b1;
  float x18_f1;
  float x1c_f2;
  float x20_f3;
  float x24_ = 0.f;
  float x28_ = 0.f;
  u32 x2c_w1;
  u32 x30_w2;

public:
  CVisorFlare(EBlendMode blendMode, bool, float, float, float, u32, u32, std::vector<CFlareDef> flares);
  void Update(float dt, const zeus::CVector3f& pos, const CActor* act, CStateManager& mgr);
  void Render(const zeus::CVector3f& pos, const CStateManager& mgr) const;
  static std::optional<CFlareDef> LoadFlareDef(CInputStream& in);
};

} // namespace urde
