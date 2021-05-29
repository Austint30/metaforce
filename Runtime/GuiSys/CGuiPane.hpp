#pragma once

#include <memory>
#include <vector>

#include "Runtime/GuiSys/CGuiWidget.hpp"

#include <zeus/CVector2f.hpp>
#include <zeus/CVector3f.hpp>

namespace metaforce {

class CGuiPane : public CGuiWidget {
protected:
  zeus::CVector2f xb8_dim;

  struct TexShaderVert {
    zeus::CVector3f m_pos;
    zeus::CVector2f m_uv;
  };
  /* Originally a vert-buffer pointer for GX */
  std::vector<TexShaderVert> xc0_verts;
  // u32 x104_ = 4; /* vert count */

  zeus::CVector3f xc8_scaleCenter;

public:
  CGuiPane(const CGuiWidgetParms& parms, const zeus::CVector2f& dim, const zeus::CVector3f& scaleCenter);
  FourCC GetWidgetTypeID() const override { return FOURCC('PANE'); }

  virtual void ScaleDimensions(const zeus::CVector3f& scale);
  virtual void SetDimensions(const zeus::CVector2f& dim, bool initVBO);
  virtual zeus::CVector2f GetDimensions() const;
  virtual void InitializeBuffers();
  virtual void WriteData(COutputStream& out, bool flag) const;

  static std::shared_ptr<CGuiWidget> Create(CGuiFrame* frame, CInputStream& in, CSimplePool* sp);
};

} // namespace metaforce
