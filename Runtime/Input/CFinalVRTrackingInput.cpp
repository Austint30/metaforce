//
// Created by austin on 1/30/22.
//

#include "CFinalVRTrackingInput.hpp"

namespace metaforce {


CFinalVRTrackingInput::CFinalVRTrackingInput() {
    // TODO: Get rid of this hard coded stuff.

    m_hmdTransform = m_hmdTransform * zeus::CTransform::RotateZ(zeus::degToRad(-1.f));
};

CFinalVRTrackingInput::~CFinalVRTrackingInput() {

}

} // namespace metaforce