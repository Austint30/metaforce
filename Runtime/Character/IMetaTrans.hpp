#pragma once

#include <memory>

#include "Runtime/RetroTypes.hpp"

namespace metaforce {
class CAnimTreeNode;
struct CAnimSysContext;

enum class EMetaTransType { MetaAnim, Trans, PhaseTrans, Snap };

class IMetaTrans {
public:
  virtual ~IMetaTrans() = default;
  virtual std::shared_ptr<CAnimTreeNode> VGetTransitionTree(const std::weak_ptr<CAnimTreeNode>& a,
                                                            const std::weak_ptr<CAnimTreeNode>& b,
                                                            const CAnimSysContext& animSys) const = 0;
  virtual EMetaTransType GetType() const = 0;

  std::shared_ptr<CAnimTreeNode> GetTransitionTree(const std::weak_ptr<CAnimTreeNode>& a,
                                                   const std::weak_ptr<CAnimTreeNode>& b,
                                                   const CAnimSysContext& animSys) {
    return VGetTransitionTree(a, b, animSys);
  }
};

} // namespace metaforce
