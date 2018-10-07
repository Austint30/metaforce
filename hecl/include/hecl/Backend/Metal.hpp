#pragma once

#include "ProgrammableCommon.hpp"

namespace hecl::Backend
{

struct Metal : ProgrammableCommon
{
    void reset(const IR& ir, Diagnostics& diag);
    std::string makeVert(unsigned col, unsigned uv, unsigned w,
                         unsigned skinSlots, size_t extTexCount,
                         const TextureInfo* extTexs, ReflectionType reflectionType) const;
    std::string makeFrag(size_t blockCount, const char** blockNames, bool alphaTest,
                         ReflectionType reflectionType,
                         const Function& lighting=Function()) const;
    std::string makeFrag(size_t blockCount, const char** blockNames, bool alphaTest,
                         ReflectionType reflectionType, const Function& lighting,
                         const Function& post, size_t extTexCount,
                         const TextureInfo* extTexs) const;

private:
    std::string GenerateVertInStruct(unsigned col, unsigned uv, unsigned w) const;
    std::string GenerateVertToFragStruct(size_t extTexCount, bool reflectionCoords) const;
    std::string GenerateVertUniformStruct(unsigned skinSlots) const;
    std::string GenerateFragOutStruct() const;
    std::string GenerateAlphaTest() const;
    std::string GenerateReflectionExpr(ReflectionType type) const;

    std::string EmitVec3(const atVec4f& vec) const
    {
        return hecl::Format("float3(%g,%g,%g)", vec.vec[0], vec.vec[1], vec.vec[2]);
    }

    std::string EmitVec3(const std::string& a, const std::string& b, const std::string& c) const
    {
        return hecl::Format("float3(%s,%s,%s)", a.c_str(), b.c_str(), c.c_str());
    }

    std::string EmitTexGenSource2(TexGenSrc src, int uvIdx) const;
    std::string EmitTexGenSource4(TexGenSrc src, int uvIdx) const;
};

}

