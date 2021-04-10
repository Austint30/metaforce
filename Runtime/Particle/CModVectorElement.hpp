#pragma once

#include <memory>
#include "Runtime/Particle/IElement.hpp"

/* Documentation at: https://wiki.axiodl.com/w/Particle_Script#Mod_Vector_Elements */

namespace metaforce {

class CMVEImplosion : public CModVectorElement {
  std::unique_ptr<CVectorElement> x4_implPoint;
  std::unique_ptr<CRealElement> x8_magScale;
  std::unique_ptr<CRealElement> xc_maxMag;
  std::unique_ptr<CRealElement> x10_minMag;
  bool x14_enableMinMag;

public:
  CMVEImplosion(std::unique_ptr<CVectorElement>&& a, std::unique_ptr<CRealElement>&& b,
                std::unique_ptr<CRealElement>&& c, std::unique_ptr<CRealElement>&& d, bool e)
  : x4_implPoint(std::move(a))
  , x8_magScale(std::move(b))
  , xc_maxMag(std::move(c))
  , x10_minMag(std::move(d))
  , x14_enableMinMag(e) {}
  bool GetValue(int frame, zeus::CVector3f& pVel, zeus::CVector3f& pPos) const override;
};

class CMVEExponentialImplosion : public CModVectorElement {
  std::unique_ptr<CVectorElement> x4_implPoint;
  std::unique_ptr<CRealElement> x8_magScale;
  std::unique_ptr<CRealElement> xc_maxMag;
  std::unique_ptr<CRealElement> x10_minMag;
  bool x14_enableMinMag;

public:
  CMVEExponentialImplosion(std::unique_ptr<CVectorElement>&& a, std::unique_ptr<CRealElement>&& b,
                           std::unique_ptr<CRealElement>&& c, std::unique_ptr<CRealElement>&& d, bool e)
  : x4_implPoint(std::move(a))
  , x8_magScale(std::move(b))
  , xc_maxMag(std::move(c))
  , x10_minMag(std::move(d))
  , x14_enableMinMag(e) {}
  bool GetValue(int frame, zeus::CVector3f& pVel, zeus::CVector3f& pPos) const override;
};

class CMVELinearImplosion : public CModVectorElement {
  std::unique_ptr<CVectorElement> x4_implPoint;
  std::unique_ptr<CRealElement> x8_magScale;
  std::unique_ptr<CRealElement> xc_maxMag;
  std::unique_ptr<CRealElement> x10_minMag;
  bool x14_enableMinMag;

public:
  CMVELinearImplosion(std::unique_ptr<CVectorElement>&& a, std::unique_ptr<CRealElement>&& b,
                      std::unique_ptr<CRealElement>&& c, std::unique_ptr<CRealElement>&& d, bool e)
  : x4_implPoint(std::move(a))
  , x8_magScale(std::move(b))
  , xc_maxMag(std::move(c))
  , x10_minMag(std::move(d))
  , x14_enableMinMag(e) {}
  bool GetValue(int frame, zeus::CVector3f& pVel, zeus::CVector3f& pPos) const override;
};

class CMVETimeChain : public CModVectorElement {
  std::unique_ptr<CModVectorElement> x4_a;
  std::unique_ptr<CModVectorElement> x8_b;
  std::unique_ptr<CIntElement> xc_swFrame;

public:
  CMVETimeChain(std::unique_ptr<CModVectorElement>&& a, std::unique_ptr<CModVectorElement>&& b,
                std::unique_ptr<CIntElement>&& c)
  : x4_a(std::move(a)), x8_b(std::move(b)), xc_swFrame(std::move(c)) {}
  bool GetValue(int frame, zeus::CVector3f& pVel, zeus::CVector3f& pPos) const override;
};

class CMVEBounce : public CModVectorElement {
  std::unique_ptr<CVectorElement> x4_planePoint;
  std::unique_ptr<CVectorElement> x8_planeNormal;
  std::unique_ptr<CRealElement> xc_friction;
  std::unique_ptr<CRealElement> x10_restitution;
  bool x14_planePrecomputed = false;
  bool x15_dieOnPenetrate;
  zeus::CVector3f x18_planeValidatedNormal;
  float x24_planeD = 0.0f;

public:
  CMVEBounce(std::unique_ptr<CVectorElement>&& planePoint, std::unique_ptr<CVectorElement>&& planeNormal,
             std::unique_ptr<CRealElement>&& friction, std::unique_ptr<CRealElement>&& restitution, bool e);
  bool GetValue(int frame, zeus::CVector3f& pVel, zeus::CVector3f& pPos) const override;
};

class CMVEConstant : public CModVectorElement {
  std::unique_ptr<CRealElement> x4_x;
  std::unique_ptr<CRealElement> x8_y;
  std::unique_ptr<CRealElement> xc_z;

public:
  CMVEConstant(std::unique_ptr<CRealElement>&& a, std::unique_ptr<CRealElement>&& b, std::unique_ptr<CRealElement>&& c)
  : x4_x(std::move(a)), x8_y(std::move(b)), xc_z(std::move(c)) {}
  bool GetValue(int frame, zeus::CVector3f& pVel, zeus::CVector3f& pPos) const override;
};

class CMVEFastConstant : public CModVectorElement {
  zeus::CVector3f x4_val;

public:
  CMVEFastConstant(float a, float b, float c) : x4_val(a, b, c) {}
  bool GetValue(int frame, zeus::CVector3f& pVel, zeus::CVector3f& pPos) const override;
};

class CMVEGravity : public CModVectorElement {
  std::unique_ptr<CVectorElement> x4_a;

public:
  explicit CMVEGravity(std::unique_ptr<CVectorElement>&& a) : x4_a(std::move(a)) {}
  bool GetValue(int frame, zeus::CVector3f& pVel, zeus::CVector3f& pPos) const override;
};

class CMVEExplode : public CModVectorElement {
  std::unique_ptr<CRealElement> x4_a;
  std::unique_ptr<CRealElement> x8_b;

public:
  CMVEExplode(std::unique_ptr<CRealElement>&& a, std::unique_ptr<CRealElement>&& b)
  : x4_a(std::move(a)), x8_b(std::move(b)) {}
  bool GetValue(int frame, zeus::CVector3f& pVel, zeus::CVector3f& pPos) const override;
};

class CMVESetPosition : public CModVectorElement {
  std::unique_ptr<CVectorElement> x4_a;

public:
  explicit CMVESetPosition(std::unique_ptr<CVectorElement>&& a) : x4_a(std::move(a)) {}
  bool GetValue(int frame, zeus::CVector3f& pVel, zeus::CVector3f& pPos) const override;
};

class CMVEPulse : public CModVectorElement {
  std::unique_ptr<CIntElement> x4_aDuration;
  std::unique_ptr<CIntElement> x8_bDuration;
  std::unique_ptr<CModVectorElement> xc_aVal;
  std::unique_ptr<CModVectorElement> x10_bVal;

public:
  CMVEPulse(std::unique_ptr<CIntElement>&& a, std::unique_ptr<CIntElement>&& b, std::unique_ptr<CModVectorElement>&& c,
            std::unique_ptr<CModVectorElement>&& d)
  : x4_aDuration(std::move(a)), x8_bDuration(std::move(b)), xc_aVal(std::move(c)), x10_bVal(std::move(d)) {}
  bool GetValue(int frame, zeus::CVector3f& pVel, zeus::CVector3f& pPos) const override;
};

class CMVEWind : public CModVectorElement {
  std::unique_ptr<CVectorElement> x4_velocity;
  std::unique_ptr<CRealElement> x8_factor;

public:
  CMVEWind(std::unique_ptr<CVectorElement>&& a, std::unique_ptr<CRealElement>&& b)
  : x4_velocity(std::move(a)), x8_factor(std::move(b)) {}
  bool GetValue(int frame, zeus::CVector3f& pVel, zeus::CVector3f& pPos) const override;
};

class CMVESwirl : public CModVectorElement {
  std::unique_ptr<CVectorElement> x4_helixPoint;
  std::unique_ptr<CVectorElement> x8_curveBinormal;
  std::unique_ptr<CRealElement> xc_filterGain;
  std::unique_ptr<CRealElement> x10_tangentialVelocity;

public:
  CMVESwirl(std::unique_ptr<CVectorElement>&& a, std::unique_ptr<CVectorElement>&& b, std::unique_ptr<CRealElement>&& c,
            std::unique_ptr<CRealElement>&& d)
  : x4_helixPoint(std::move(a))
  , x8_curveBinormal(std::move(b))
  , xc_filterGain(std::move(c))
  , x10_tangentialVelocity(std::move(d)) {}
  bool GetValue(int frame, zeus::CVector3f& pVel, zeus::CVector3f& pPos) const override;
};

} // namespace metaforce
