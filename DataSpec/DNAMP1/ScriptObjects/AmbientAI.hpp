#pragma once

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace DataSpec::DNAMP1
{
struct AmbientAI : IScriptObject
{
    AT_DECL_DNA_YAML
    AT_DECL_DNAV
    String<-1>          name;
    Value<atVec3f>      location;
    Value<atVec3f>      orientation;
    Value<atVec3f>      scale;
    Value<atVec3f>      collisionExtent;
    Value<atVec3f>      collisionOffset;
    Value<float>        mass;
    HealthInfo          healthInfo;
    DamageVulnerability damageVulnerability;
    AnimationParameters animationParameters;
    ActorParameters     actorParameters;
    Value<float>        alertRange;
    Value<float>        impactRange;
    Value<atInt32>      alertAnim;
    Value<atInt32>      impactAnim;
    Value<bool>         active;

    void addCMDLRigPairs(PAKRouter<PAKBridge>& pakRouter,
            std::unordered_map<UniqueID32, std::pair<UniqueID32, UniqueID32>>& addTo) const
    {
        actorParameters.addCMDLRigPairs(addTo, animationParameters.getCINF(pakRouter));
    }

    void nameIDs(PAKRouter<PAKBridge>& pakRouter) const
    {
        animationParameters.nameANCS(pakRouter, name + "_animp");
        actorParameters.nameIDs(pakRouter, name + "_actp");
    }

    void gatherDependencies(std::vector<hecl::ProjectPath>& pathsOut,
                            std::vector<hecl::ProjectPath>& lazyOut) const
    {
        animationParameters.depANCS(pathsOut);
        actorParameters.depIDs(pathsOut, lazyOut);
    }

    void gatherScans(std::vector<Scan>& scansOut) const
    {
        actorParameters.scanIDs(scansOut);
    }
};
}

