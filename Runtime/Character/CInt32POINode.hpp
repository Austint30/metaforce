#pragma once

#include <string>

#include "Runtime/GCNTypes.hpp"
#include "Runtime/Character/CPOINode.hpp"

namespace metaforce {
class IAnimSourceInfo;

class CInt32POINode : public CPOINode {
  s32 x38_val;
  std::string x3c_locatorName;

public:
  CInt32POINode();
  CInt32POINode(std::string_view, EPOIType, const CCharAnimTime&, s32, bool, float, s32, s32, s32, std::string_view);
  explicit CInt32POINode(CInputStream& in);
  s32 GetValue() const { return x38_val; }
  std::string_view GetLocatorName() const { return x3c_locatorName; }

  static CInt32POINode CopyNodeMinusStartTime(const CInt32POINode& node, const CCharAnimTime& startTime);
};

} // namespace metaforce
