#pragma once

#include <memory>
#include <optional>
#include <set>

#include "Runtime/RetroTypes.hpp"
#include "Runtime/Character/CCharAnimTime.hpp"

namespace metaforce {
class CAnimTreeNode;
class CPrimitive;
class IAnimReader;
struct CAnimSysContext;
struct CMetaAnimTreeBuildOrders;

enum class EMetaAnimType { Play, Blend, PhaseBlend, Random, Sequence };

class CPreAdvanceIndicator {
  bool x0_isTime;
  CCharAnimTime x4_time;
  const char* xc_string;
  /*
  u32 x10_;
  u32 x14_;
  u32 x18_fireTime;
  u32 x1c_damageDelay;
  u32 x20_;
  u32 x24_;
  u32 x28_;
  u32 x2c_;
  u32 x30_;
  u32 x34_;
  u32 x38_;
  u16 x3c_;
  */
public:
  explicit CPreAdvanceIndicator(const CCharAnimTime& time) : x0_isTime(true), x4_time(time) {}
  explicit CPreAdvanceIndicator(const char* string) : x0_isTime(false), xc_string(string) {}
  const char* GetString() const { return xc_string; }
  bool IsString() const { return !x0_isTime; }
  const CCharAnimTime& GetTime() const { return x4_time; }
  bool IsTime() const { return x0_isTime; }
};

struct CMetaAnimTreeBuildOrders {
  std::optional<CPreAdvanceIndicator> x0_recursiveAdvance;
  std::optional<CPreAdvanceIndicator> x44_singleAdvance;
  static CMetaAnimTreeBuildOrders NoSpecialOrders() { return {}; }
  static CMetaAnimTreeBuildOrders PreAdvanceForAll(const CPreAdvanceIndicator& ind) {
    CMetaAnimTreeBuildOrders ret;
    ret.x44_singleAdvance.emplace(ind);
    return ret;
  }
};

class IMetaAnim {
public:
  virtual ~IMetaAnim() = default;
  virtual std::shared_ptr<CAnimTreeNode> GetAnimationTree(const CAnimSysContext& animSys,
                                                          const CMetaAnimTreeBuildOrders& orders) const;
  virtual void GetUniquePrimitives(std::set<CPrimitive>& primsOut) const = 0;
  virtual EMetaAnimType GetType() const = 0;
  virtual std::shared_ptr<CAnimTreeNode> VGetAnimationTree(const CAnimSysContext& animSys,
                                                           const CMetaAnimTreeBuildOrders& orders) const = 0;

  static void AdvanceAnim(IAnimReader& anim, const CCharAnimTime& dt);
  static CCharAnimTime GetTime(const CPreAdvanceIndicator& ind, const IAnimReader& anim);
};

} // namespace metaforce
