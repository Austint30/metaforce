#pragma once

#include <memory>
#include <optional>
#include <utility>
#include <vector>

#include "Runtime/CToken.hpp"
#include "Runtime/Character/CSkinRules.hpp"
#include "Runtime/Graphics/CModel.hpp"

#include <zeus/CVector3f.hpp>

namespace metaforce {
class CCharLayoutInfo;
class CModel;
class CPoseAsTransforms;
class CVertexMorphEffect;
class IObjectStore;

class CSkinnedModel {
  friend class CBooModel;
  std::unique_ptr<CBooModel> m_modelInst;
  TLockedToken<CModel> x4_model;
  TLockedToken<CSkinRules> x10_skinRules;
  TLockedToken<CCharLayoutInfo> x1c_layoutInfo;
  std::vector<std::pair<zeus::CVector3f, zeus::CVector3f>> m_vertWorkspace;
  bool m_modifiedVBO = false;

public:
  enum class EDataOwnership { Zero, One };
  CSkinnedModel(TLockedToken<CModel> model, TLockedToken<CSkinRules> skinRules,
                TLockedToken<CCharLayoutInfo> layoutInfo, int shaderIdx);
  CSkinnedModel(IObjectStore& store, CAssetId model, CAssetId skinRules, CAssetId layoutInfo, int shaderIdx);
  std::unique_ptr<CSkinnedModel> Clone(int shaderIdx = 0) const {
    return std::make_unique<CSkinnedModel>(x4_model, x10_skinRules, x1c_layoutInfo, shaderIdx);
  }

  const TLockedToken<CModel>& GetModel() const { return x4_model; }
  const std::unique_ptr<CBooModel>& GetModelInst() const { return m_modelInst; }
  const TLockedToken<CSkinRules>& GetSkinRules() const { return x10_skinRules; }
  void SetLayoutInfo(const TLockedToken<CCharLayoutInfo>& inf) { x1c_layoutInfo = inf; }
  const TLockedToken<CCharLayoutInfo>& GetLayoutInfo() const { return x1c_layoutInfo; }

  void Calculate(const CPoseAsTransforms& pose, const CModelFlags& drawFlags,
                 const std::optional<CVertexMorphEffect>& morphEffect, const float* morphMagnitudes);
  void Draw(const CModelFlags& drawFlags) const;

  using FPointGenerator = void (*)(void* item, const std::vector<std::pair<zeus::CVector3f, zeus::CVector3f>>& vn);
  static void SetPointGeneratorFunc(void* ctx, FPointGenerator func) {
    g_PointGenFunc = func;
    g_PointGenCtx = ctx;
  }
  static void ClearPointGeneratorFunc() { g_PointGenFunc = nullptr; }
  static FPointGenerator g_PointGenFunc;
  static void* g_PointGenCtx;
};

class CMorphableSkinnedModel : public CSkinnedModel {
  std::unique_ptr<float[]> x40_morphMagnitudes;

public:
  CMorphableSkinnedModel(IObjectStore& store, CAssetId model, CAssetId skinRules, CAssetId layoutInfo, int shaderIdx);
  const float* GetMorphMagnitudes() const { return x40_morphMagnitudes.get(); }
};

} // namespace metaforce
