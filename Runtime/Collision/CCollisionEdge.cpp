#include "Runtime/Collision/CCollisionEdge.hpp"

namespace metaforce {
CCollisionEdge::CCollisionEdge(CInputStream& in) {
  x0_index1 = in.readUint16Big();
  x2_index2 = in.readUint16Big();
}
} // namespace metaforce
