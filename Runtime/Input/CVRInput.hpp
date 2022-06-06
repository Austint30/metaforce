//
// Created by austin on 1/30/22.
//
#pragma once

#include <zeus/zeus.hpp>

namespace metaforce {

struct CVRInput {

public:
  zeus::CTransform m_hmdTransform;
  std::vector<zeus::CTransform> m_eyeTransforms;

  CVRInput();
  ~CVRInput();

  std::vector<zeus::CTransform> getEyeTransforms();

  void Update();
};

} // namespace metaforce