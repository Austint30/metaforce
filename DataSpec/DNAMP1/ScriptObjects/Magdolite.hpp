#pragma once

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace DataSpec::DNAMP1 {
struct Magdolite : IScriptObject {
  AT_DECL_DNA_YAMLV
  String<-1> name;
  Value<atVec3f> location;
  Value<atVec3f> orientation;
  Value<atVec3f> scale;
  PatternedInfo patternedInfo;
  ActorParameters actorParameters;
  Value<float> unknown1;
  Value<float> unknown2;
  DamageInfo damageInfo1;
  DamageInfo damageInfo2;
  DamageVulnerability damageVulnerabilty1;
  DamageVulnerability damageVulnerabilty2;
  UniqueID32 cmdlHeadless;
  UniqueID32 cskrHeadless;
  Value<float> unknown3;
  Value<float> unknown4;
  Value<float> unknown5;
  Value<float> unknown6;
  struct MagdoliteParameters : BigDNA {
    AT_DECL_DNA
    Value<atUint32> propertyCount;
    Value<atUint32> unknown1;
    UniqueID32 particle;
    Value<atUint32> unknown2;
    Value<float> unknown3;
    Value<float> unknown4;
    Value<float> unknown5;
  } magdoliteParameters;
  Value<float> unknown7;
  Value<float> unknown8;
  Value<float> unknown9;

  void addCMDLRigPairs(PAKRouter<PAKBridge>& pakRouter, CharacterAssociations<UniqueID32>& charAssoc) const override {
    UniqueID32 cinf = patternedInfo.animationParameters.getCINF(pakRouter);
    actorParameters.addCMDLRigPairs(pakRouter, charAssoc, patternedInfo.animationParameters);

    if (cmdlHeadless.isValid() && cskrHeadless.isValid()) {
      charAssoc.m_cmdlRigs[cmdlHeadless] = {cskrHeadless, cinf};
      charAssoc.m_cskrToCharacter[cskrHeadless] =
          std::make_pair(patternedInfo.animationParameters.animationCharacterSet,
              fmt::format(FMT_STRING("ATTACH.HEADLESS_{}.CSKR"), cskrHeadless));
      charAssoc.addAttachmentRig(patternedInfo.animationParameters.animationCharacterSet, {}, cmdlHeadless, "HEADLESS");
    }
  }

  void nameIDs(PAKRouter<PAKBridge>& pakRouter) const override {
    if (cmdlHeadless.isValid()) {
      PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(cmdlHeadless);
      ent->name = name + "_emodel";
    }
    if (cskrHeadless.isValid()) {
      PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(cskrHeadless);
      ent->name = name + "_eskin";
    }
    if (magdoliteParameters.particle.isValid()) {
      PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(magdoliteParameters.particle);
      ent->name = name + "_part";
    }
    patternedInfo.nameIDs(pakRouter, name + "_patterned");
    actorParameters.nameIDs(pakRouter, name + "_actp");
  }

  void gatherDependencies(std::vector<hecl::ProjectPath>& pathsOut,
                          std::vector<hecl::ProjectPath>& lazyOut) const override {
    g_curSpec->flattenDependencies(cmdlHeadless, pathsOut);
    g_curSpec->flattenDependencies(cskrHeadless, pathsOut);
    g_curSpec->flattenDependencies(magdoliteParameters.particle, pathsOut);
    patternedInfo.depIDs(pathsOut);
    actorParameters.depIDs(pathsOut, lazyOut);
  }

  void gatherScans(std::vector<Scan>& scansOut) const override { actorParameters.scanIDs(scansOut); }
};
} // namespace DataSpec::DNAMP1
