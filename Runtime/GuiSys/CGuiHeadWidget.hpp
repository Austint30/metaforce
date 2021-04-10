#pragma once

#include "Runtime/GuiSys/CGuiWidget.hpp"

namespace metaforce {

class CGuiHeadWidget : public CGuiWidget {
public:
  FourCC GetWidgetTypeID() const override { return FOURCC('HWIG'); }
  explicit CGuiHeadWidget(const CGuiWidgetParms& parms);
  static std::shared_ptr<CGuiWidget> Create(CGuiFrame* frame, CInputStream& in, CSimplePool* sp);

  std::shared_ptr<CGuiHeadWidget> shared_from_this() {
    return std::static_pointer_cast<CGuiHeadWidget>(CGuiObject::shared_from_this());
  }
};

} // namespace metaforce
