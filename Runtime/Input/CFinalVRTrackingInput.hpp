//
// Created by austin on 1/30/22.
//
#pragma once

#include <zeus/zeus.hpp>

namespace metaforce {

struct CFinalVRTrackingInput {
  zeus::CTransform m_hmdTransform;
  CFinalVRTrackingInput();
  ~CFinalVRTrackingInput();
};

} // namespace metaforce