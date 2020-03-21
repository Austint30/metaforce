#include "Runtime/GuiSys/CRasterFont.hpp"

#include "Runtime/CSimplePool.hpp"
#include "Runtime/Graphics/CTexture.hpp"
#include "Runtime/GuiSys/CDrawStringOptions.hpp"
#include "Runtime/GuiSys/CTextRenderBuffer.hpp"

namespace urde {
CRasterFont::CRasterFont(urde::CInputStream& in, urde::IObjectStore& store) {
  u32 magic = 0;
  in.readBytesToBuf(&magic, 4);
  if (magic != SBIG('FONT'))
    return;

  u32 version = in.readUint32Big();
  x4_monoWidth = in.readUint32Big();
  x8_monoHeight = in.readUint32Big();

  if (version >= 1)
    x8c_baseline = in.readUint32Big();
  else
    x8c_baseline = x8_monoHeight;

  if (version >= 2)
    x90_lineMargin = in.readUint32Big();

  bool tmp1 = in.readBool();
  bool tmp2 = in.readBool();

  u32 tmp3 = in.readUint32Big();
  u32 tmp4 = in.readUint32Big();
  std::string name = in.readString();
  u32 txtrId = (version == 5 ? in.readUint64Big() : in.readUint32Big());
  x30_fontInfo = CFontInfo(tmp1, tmp2, tmp3, tmp4, name.c_str());
  x80_texture = store.GetObj({FOURCC('TXTR'), txtrId});
  x2c_mode = CTexture::EFontType(in.readUint32Big());

  u32 glyphCount = in.readUint32Big();
  xc_glyphs.reserve(glyphCount);

  for (u32 i = 0; i < glyphCount; ++i) {
    char16_t chr = in.readUint16Big();
    float startU = in.readFloatBig();
    float startV = in.readFloatBig();
    float endU = in.readFloatBig();
    float endV = in.readFloatBig();
    s32 layer = 0;
    s32 a, b, c, cellWidth, cellHeight, baseline, kernStart;
    if (version < 4) {
      a = in.readInt32Big();
      b = in.readInt32Big();
      c = in.readInt32Big();
      cellWidth = in.readInt32Big();
      cellHeight = in.readInt32Big();
      baseline = in.readInt32Big();
      kernStart = in.readInt32Big();
    } else {
      layer = in.readByte();
      a = in.readByte();
      b = in.readByte();
      c = in.readByte();
      cellWidth = in.readByte();
      cellHeight = in.readByte();
      baseline = in.readByte();
      kernStart = in.readInt16Big();
    }
    xc_glyphs.emplace_back(
        chr, CGlyph(a, b, c, startU, startV, endU, endV, cellWidth, cellHeight, baseline, kernStart, layer));
  }

  std::sort(xc_glyphs.begin(), xc_glyphs.end(), [=](auto& a, auto& b) -> bool { return a.first < b.first; });

  u32 kernCount = in.readUint32Big();
  x1c_kerning.reserve(kernCount);

  for (u32 i = 0; i < kernCount; ++i) {
    char16_t first = in.readUint16Big();
    char16_t second = in.readUint16Big();
    s32 howMuch = in.readInt32Big();
    x1c_kerning.emplace_back(first, second, howMuch);
  }

  if (magic == SBIG('FONT') && version <= 4)
    x0_initialized = true;
}

void CRasterFont::SinglePassDrawString(const CDrawStringOptions& opts, int x, int y, int& xout, int& yout,
                                       CTextRenderBuffer* renderBuf, const char16_t* str, s32 length) const {
  if (!x0_initialized)
    return;

  const char16_t* chr = str;
  const CGlyph* prevGlyph = nullptr;
  while (*chr != u'\0') {
    const CGlyph* glyph = GetGlyph(*chr);
    if (glyph) {
      if (opts.x0_direction == ETextDirection::Horizontal) {
        x += glyph->GetLeftPadding();

        if (prevGlyph != 0)
          x += KernLookup(x1c_kerning, prevGlyph->GetKernStart(), *chr);
        int left = 0;
        int top = 0;

        if (renderBuf) {
          left += x;
          top += y - glyph->GetBaseline();
          renderBuf->AddCharacter(zeus::CVector2i(left, top), *chr, opts.x4_colors[2]);
        }
        x += glyph->GetRightPadding() + glyph->GetAdvance();
      }
    }
    prevGlyph = glyph;
    chr++;
    if (length == -1)
      continue;

    if ((chr - str) >= length)
      break;
  }

  xout = x;
  yout = y;
}

void CRasterFont::DrawSpace(const CDrawStringOptions& opts, int x, int y, int& xout, int& yout, int len) const {
  if (opts.x0_direction != ETextDirection::Horizontal)
    return;

  xout = x + len;
  yout = y;
}

void CRasterFont::DrawString(const CDrawStringOptions& opts, int x, int y, int& xout, int& yout,
                             CTextRenderBuffer* renderBuf, const char16_t* str, int len) const {
  if (!x0_initialized)
    return;

  if (renderBuf) {
    /* CGraphicsPalette pal = CGraphicsPalette::CGraphcisPalette(2, 4); */
    /* zeus::CColor color = zeus::CColor(0.f, 0.f, 0.f, 0.f) */
    /* tmp = color.ToRGB5A3(); */
    /* tmp2 = opts.x8_.ToRGB5A3(); */
    /* tmp3 = opts.xc_.ToRGB5A3(); */
    /* tmp4 = zeus::CColor(0.f, 0.f, 0.f, 0.f); */
    /* tmp5 = tmp4.ToRGBA5A3(); */
    /* pal.UnLock(); */
    /* renderBuf->AddPaletteChange(pal); */
    renderBuf->AddPaletteChange(opts.x4_colors[0], opts.x4_colors[1]);
  }

  SinglePassDrawString(opts, x, y, xout, yout, renderBuf, str, len);
}

void CRasterFont::GetSize(const CDrawStringOptions& opts, int& width, int& height, const char16_t* str, int len) const {
  width = 0;
  height = 0;

  const char16_t* chr = str;
  const CGlyph* prevGlyph = nullptr;
  int prevWidth = 0;
  while (*chr != u'\0') {
    const CGlyph* glyph = GetGlyph(*chr);

    if (glyph) {
      if (opts.x0_direction == ETextDirection::Horizontal) {
        int advance = 0;
        if (prevGlyph)
          advance = KernLookup(x1c_kerning, prevGlyph->GetKernStart(), *chr);

        int curWidth = prevWidth + (glyph->GetLeftPadding() + glyph->GetAdvance() + glyph->GetRightPadding() + advance);
        int curHeight = glyph->GetBaseline() - (x8_monoHeight + glyph->GetCellHeight());

        width = curWidth;
        prevWidth = curWidth;

        if (curHeight > height)
          height = curHeight;
      }
    }

    prevGlyph = glyph;
    chr++;
    if (len == -1)
      continue;

    if ((chr - str) >= len)
      break;
  }
}

bool CRasterFont::IsFinishedLoading() const {
  if (!x80_texture || !x80_texture.IsLoaded())
    return false;
  return true;
}

std::unique_ptr<IObj> FRasterFontFactory(const SObjectTag& tag, CInputStream& in, const CVParamTransfer& vparms,
                                         CObjectReference* selfRef) {
  CSimplePool* sp = vparms.GetOwnedObj<CSimplePool*>();
  return TToken<CRasterFont>::GetIObjObjectFor(std::make_unique<CRasterFont>(in, *sp));
}

} // namespace urde
