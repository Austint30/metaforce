#pragma once

#include <functional>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "Runtime/GCNTypes.hpp"
#include "Runtime/IOStreams.hpp"
#include "Runtime/rstl.hpp"

#include <hecl/hecl.hpp>
#include <zeus/CMatrix3f.hpp>
#include <zeus/CMatrix4f.hpp>
#include <zeus/CTransform.hpp>
#include <zeus/CVector2f.hpp>
#include <zeus/CVector3f.hpp>

#undef min
#undef max

using namespace std::literals;

namespace urde {

using FourCC = hecl::FourCC;

class CAssetId {
  u64 id = UINT64_MAX;

public:
  constexpr CAssetId() noexcept = default;
  constexpr CAssetId(u64 v) noexcept { Assign(v); }
  explicit CAssetId(CInputStream& in);
  [[nodiscard]] constexpr bool IsValid() const noexcept { return id != UINT64_MAX; }
  [[nodiscard]] constexpr u64 Value() const noexcept { return id; }
  constexpr void Assign(u64 v) noexcept { id = (v == UINT32_MAX ? UINT64_MAX : (v == 0 ? UINT64_MAX : v)); }
  constexpr void Reset() noexcept { id = UINT64_MAX; }
  void PutTo(COutputStream& out);
  [[nodiscard]] constexpr bool operator==(CAssetId other) const noexcept { return id == other.id; }
  [[nodiscard]] constexpr bool operator!=(CAssetId other) const noexcept { return !operator==(other); }
  [[nodiscard]] constexpr bool operator<(CAssetId other) const noexcept { return id < other.id; }
};

//#define kInvalidAssetId CAssetId()

struct SObjectTag {
  FourCC type;
  CAssetId id;

  constexpr explicit operator bool() const noexcept { return id.IsValid(); }
  [[nodiscard]] constexpr bool operator==(const SObjectTag& other) const noexcept { return id == other.id; }
  [[nodiscard]] constexpr bool operator!=(const SObjectTag& other) const noexcept { return !operator==(other); }
  [[nodiscard]] constexpr bool operator<(const SObjectTag& other) const noexcept { return id < other.id; }
  constexpr SObjectTag() noexcept = default;
  constexpr SObjectTag(FourCC tp, CAssetId rid) noexcept : type(tp), id(rid) {}
  explicit SObjectTag(CInputStream& in) {
    in.readBytesToBuf(&type, 4);
    id = CAssetId(in);
  }
  void readMLVL(CInputStream& in) {
    id = CAssetId(in);
    in.readBytesToBuf(&type, 4);
  }
};

struct TEditorId {
  u32 id = u32(-1);

  constexpr TEditorId() noexcept = default;
  constexpr TEditorId(u32 idin) noexcept : id(idin) {}
  [[nodiscard]] constexpr u8 LayerNum() const noexcept { return u8((id >> 26) & 0x3f); }
  [[nodiscard]] constexpr u16 AreaNum() const noexcept { return u16((id >> 16) & 0x3ff); }
  [[nodiscard]] constexpr u16 Id() const noexcept { return u16(id & 0xffff); }
  [[nodiscard]] constexpr bool operator<(TEditorId other) const noexcept { return (id & 0x3ffffff) < (other.id & 0x3ffffff); }
  [[nodiscard]] constexpr bool operator==(TEditorId other) const noexcept {
    return (id & 0x3ffffff) == (other.id & 0x3ffffff);
  }
  [[nodiscard]] constexpr bool operator!=(TEditorId other) const noexcept { return !operator==(other); }
};

#define kInvalidEditorId TEditorId()

struct TUniqueId {
  u16 id = u16(-1);

  constexpr TUniqueId() noexcept = default;
  constexpr TUniqueId(u16 value, u16 version) noexcept : id(value | (version << 10)) {}
  [[nodiscard]] constexpr u16 Version() const noexcept { return u16((id >> 10) & 0x3f); }
  [[nodiscard]] constexpr u16 Value() const noexcept { return u16(id & 0x3ff); }
  [[nodiscard]] constexpr bool operator<(TUniqueId other) const noexcept { return id < other.id; }
  [[nodiscard]] constexpr bool operator==(TUniqueId other) const noexcept { return id == other.id; }
  [[nodiscard]] constexpr bool operator!=(TUniqueId other) const noexcept { return !operator==(other); }
};

#define kInvalidUniqueId TUniqueId()

using TAreaId = s32;

#define kInvalidAreaId TAreaId(-1)

#if 0
template <class T, size_t N>
class TRoundRobin
{
    rstl::reserved_vector<T, N> vals;

public:
    TRoundRobin(const T& val) : vals(N, val) {}

    void PushBack(const T& val) { vals.push_back(val); }

    size_t Size() const { return vals.size(); }

    const T& GetLastValue() const { return vals.back(); }

    void Clear() { vals.clear(); }

    const T& GetValue(s32) const {}
};
#endif

template <class T>
[[nodiscard]] T GetAverage(const T* v, s32 count) noexcept {
  T r = v[0];
  for (s32 i = 1; i < count; ++i)
    r += v[i];

  return r / count;
}

template <class T, size_t N>
class TReservedAverage : rstl::reserved_vector<T, N> {
public:
  TReservedAverage() = default;

  TReservedAverage(const T& t) { rstl::reserved_vector<T, N>::resize(N, t); }

  void AddValue(const T& t) {
    if (this->size() < N) {
      this->insert(this->begin(), t);
    } else {
      this->pop_back();
      this->insert(this->begin(), t);
    }
  } 

  [[nodiscard]] std::optional<T> GetAverage() const {
    if (this->empty()) {
      return std::nullopt;
    }

    return {urde::GetAverage<T>(this->data(), this->size())};
  }

  [[nodiscard]] std::optional<T> GetEntry(int i) const {
    if (i >= this->size()) {
      return std::nullopt;
    }
    return this->operator[](i);
  }

  void Clear() { this->clear(); }

  [[nodiscard]] size_t Size() const { return this->size(); }
};

} // namespace urde

namespace std {
template <>
struct hash<urde::SObjectTag> {
  size_t operator()(const urde::SObjectTag& tag) const noexcept { return tag.id.Value(); }
};

template <>
struct hash<urde::CAssetId> {
  size_t operator()(const urde::CAssetId& id) const noexcept { return id.Value(); }
};
} // namespace std

FMT_CUSTOM_FORMATTER(urde::CAssetId, "{:08X}", obj.Value())
FMT_CUSTOM_FORMATTER(urde::TEditorId, "{:08X}", obj.id)
FMT_CUSTOM_FORMATTER(urde::TUniqueId, "{:04X}", obj.id)
FMT_CUSTOM_FORMATTER(urde::SObjectTag, "{} {}", obj.type, obj.id)

FMT_CUSTOM_FORMATTER(zeus::CVector3f, "({} {} {})", float(obj.x()), float(obj.y()), float(obj.z()))
FMT_CUSTOM_FORMATTER(zeus::CVector2f, "({} {})", float(obj.x()), float(obj.y()))
FMT_CUSTOM_FORMATTER(zeus::CMatrix3f, "\n({} {} {})"
                                      "\n({} {} {})"
                                      "\n({} {} {})",
                     float(obj[0][0]), float(obj[1][0]), float(obj[2][0]),
                     float(obj[0][1]), float(obj[1][1]), float(obj[2][1]),
                     float(obj[0][2]), float(obj[1][2]), float(obj[2][2]))
FMT_CUSTOM_FORMATTER(zeus::CMatrix4f, "\n({} {} {} {})"
                                      "\n({} {} {} {})"
                                      "\n({} {} {} {})"
                                      "\n({} {} {} {})",
                     float(obj[0][0]), float(obj[1][0]), float(obj[2][0]), float(obj[3][0]),
                     float(obj[0][1]), float(obj[1][1]), float(obj[2][1]), float(obj[3][1]),
                     float(obj[0][2]), float(obj[1][2]), float(obj[2][2]), float(obj[3][2]),
                     float(obj[0][3]), float(obj[1][3]), float(obj[2][3]), float(obj[3][3]))
FMT_CUSTOM_FORMATTER(zeus::CTransform, "\n({} {} {} {})"
                                       "\n({} {} {} {})"
                                       "\n({} {} {} {})",
                     float(obj.basis[0][0]), float(obj.basis[1][0]), float(obj.basis[2][0]), float(obj.origin[0]),
                     float(obj.basis[0][1]), float(obj.basis[1][1]), float(obj.basis[2][1]), float(obj.origin[1]),
                     float(obj.basis[0][2]), float(obj.basis[1][2]), float(obj.basis[2][2]), float(obj.origin[2]))

#if defined(__has_feature)
#if __has_feature(memory_sanitizer)
#define URDE_MSAN 1
#endif
#endif
