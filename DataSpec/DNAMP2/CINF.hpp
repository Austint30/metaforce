#pragma once

#include "DataSpec/DNACommon/DNACommon.hpp"
#include "DataSpec/DNACommon/RigInverter.hpp"
#include "DNAMP2.hpp"

namespace DataSpec::DNAMP2 {

struct CINF : BigDNA {
  AT_DECL_DNA
  Value<atUint32> boneCount;
  struct Bone : BigDNA {
    AT_DECL_DNA
    Value<atUint32> id;
    Value<atUint32> parentId;
    Value<atVec3f> origin;
    Value<atVec4f> q1;
    Value<atVec4f> q2;
    Value<atUint32> linkedCount;
    Vector<atUint32, AT_DNA_COUNT(linkedCount)> linked;
  };
  Vector<Bone, AT_DNA_COUNT(boneCount)> bones;

  Value<atUint32> boneIdCount;
  Vector<atUint32, AT_DNA_COUNT(boneIdCount)> boneIds;

  Value<atUint32> nameCount;
  struct Name : BigDNA {
    AT_DECL_DNA
    String<-1> name;
    Value<atUint32> boneId;
  };
  Vector<Name, AT_DNA_COUNT(nameCount)> names;

  atUint32 getInternalBoneIdxFromId(atUint32 id) const;
  atUint32 getBoneIdxFromId(atUint32 id) const;
  const std::string* getBoneNameFromId(atUint32 id) const;
  void sendVertexGroupsToBlender(hecl::blender::PyOutStream& os) const;
  void sendCINFToBlender(hecl::blender::PyOutStream& os, const UniqueID32& cinfId) const;
  static std::string GetCINFArmatureName(const UniqueID32& cinfId);

  CINF() = default;
  using Armature = hecl::blender::Armature;
  using BlenderBone = hecl::blender::Bone;

  int RecursiveAddArmatureBone(const Armature& armature, const BlenderBone* bone, int parent, int& curId,
                               std::unordered_map<std::string, atInt32>& idMap, std::map<std::string, int>& nameMap);

  CINF(const Armature& armature, std::unordered_map<std::string, atInt32>& idMap);

  static bool Extract(const SpecBase& dataSpec, PAKEntryReadStream& rs, const hecl::ProjectPath& outPath,
                      PAKRouter<PAKBridge>& pakRouter, const PAK::Entry& entry, bool force, hecl::blender::Token& btok,
                      std::function<void(const hecl::SystemChar*)> fileChanged);

  static bool Cook(const hecl::ProjectPath& outPath, const hecl::ProjectPath& inPath,
                   const hecl::blender::Armature& armature);
};

} // namespace DataSpec::DNAMP2
