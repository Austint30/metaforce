//
// Created by austin on 1/30/22.
//

#include "CVRInput.hpp"
#include "extern/aurora/include/aurora/xr/xr.hpp"

namespace metaforce {

CVRInput::CVRInput() {
    // TODO: Get rid of this hard coded stuff.

    m_hmdTransform = m_hmdTransform * zeus::CTransform::RotateZ(zeus::degToRad(-1.f));
};

CVRInput::~CVRInput() {

}

std::vector<zeus::CTransform> CVRInput::getEyeTransforms() {
  std::vector<XrView> views = aurora::xr::g_OpenXRSessionManager->GetViews();

  std::vector<zeus::CTransform> eyeTrans;
  for (const XrView& view : views){
    zeus::CQuaternion quat = zeus::CQuaternion(
        view.pose.orientation.w,
        view.pose.orientation.x,
        -view.pose.orientation.z,
        view.pose.orientation.y
      );
    zeus::CVector3f pos = zeus::CVector3f(
        view.pose.position.x,
        -view.pose.position.z,
        view.pose.position.y
    );
    zeus::CTransform trans = zeus::CTransform{quat, pos};
    eyeTrans.push_back(trans);
  }
  m_eyeTransforms = eyeTrans;
//  Log.report(logvisor::Info, FMT_STRING("X={} Y={} Z={}"), eyeTrans[0].origin.x(), eyeTrans[0].origin.y(), eyeTrans[0].origin.z());
  return eyeTrans;
}

void CVRInput::Update() {
  if (aurora::xr::g_OpenXRSessionManager != nullptr){
    getEyeTransforms();
  }
}

} // namespace metaforce