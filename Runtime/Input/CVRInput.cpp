//
// Created by austin on 1/30/22.
//

#include "CVRInput.hpp"
#include <boo/boo.hpp>

namespace metaforce {

CVRInput::CVRInput() {
    // TODO: Get rid of this hard coded stuff.

    m_hmdTransform = m_hmdTransform * zeus::CTransform::RotateZ(zeus::degToRad(-1.f));
};

CVRInput::~CVRInput() {

}

std::vector<zeus::CTransform> CVRInput::getEyeTransforms() {
  std::vector<XrView> views = boo::g_OpenXRSessionManager->GetViews();

  std::vector<zeus::CTransform> eyeTrans;
  for (const XrView& view : views){
    zeus::CQuaternion quat = zeus::CQuaternion(
        view.pose.orientation.w,
        view.pose.orientation.x,
        view.pose.orientation.y,
        view.pose.orientation.z
      );
    zeus::CVector3f pos = zeus::CVector3f(
        view.pose.position.x,
        view.pose.position.y,
        view.pose.position.z
    );
    zeus::CTransform trans = zeus::CTransform{quat, pos};
    eyeTrans.push_back(trans);
  }
  m_eyeTransforms = eyeTrans;
//  Log.report(logvisor::Info, FMT_STRING("X={} Y={} Z={}"), eyeTrans[0].origin.x(), eyeTrans[0].origin.y(), eyeTrans[0].origin.z());
  return eyeTrans;
}

void CVRInput::Update() {
  if (boo::g_OpenXRSessionManager != nullptr){
    getEyeTransforms();
  }
}

} // namespace metaforce