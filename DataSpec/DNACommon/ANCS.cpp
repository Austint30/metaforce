#include "DataSpec/DNACommon/ANCS.hpp"

#include "DataSpec/DNACommon/CMDL.hpp"
#include "DataSpec/DNACommon/DNACommon.hpp"
#include "DataSpec/DNACommon/RigInverter.hpp"
#include "DataSpec/DNAMP1/DNAMP1.hpp"
#include "DataSpec/DNAMP1/ANCS.hpp"
#include "DataSpec/DNAMP2/DNAMP2.hpp"
#include "DataSpec/DNAMP2/ANCS.hpp"
#include "DataSpec/DNAMP3/DNAMP3.hpp"
#include "DataSpec/DNAMP3/CHAR.hpp"

#include <hecl/Blender/Connection.hpp>

namespace DataSpec::DNAANCS {

template <class PAKRouter, class ANCSDNA, class MaterialSet, class SurfaceHeader, atUint32 CMDLVersion>
bool ReadANCSToBlender(hecl::blender::Token& btok, const ANCSDNA& ancs, const hecl::ProjectPath& outPath,
                       PAKRouter& pakRouter, const typename PAKRouter::EntryType& entry, const SpecBase& dataspec,
                       std::function<void(const hecl::SystemChar*)> fileChanged, bool force) {
  auto& conn = btok.getBlenderConnection();
  /* Extract character CMDL/CSKR/CINF first */
  std::vector<CharacterResInfo<typename PAKRouter::IDType>> chResInfo;
  ancs.getCharacterResInfo(chResInfo);
  for (const auto& info : chResInfo) {
    const nod::Node* node;
    if (const typename PAKRouter::EntryType* cmdlE = pakRouter.lookupEntry(info.cmdl, &node, true, false)) {
      hecl::ProjectPath cmdlPath = pakRouter.getWorking(cmdlE);
      if (force || cmdlPath.isNone()) {
        cmdlPath.makeDirChain(false);
        if (!conn.createBlend(cmdlPath, hecl::blender::BlendType::Mesh))
          return false;

        std::string bestName = pakRouter.getBestEntryName(*cmdlE);
        hecl::SystemStringConv bestNameView(bestName);
        fileChanged(bestNameView.c_str());

        typename ANCSDNA::CSKRType cskr;
        pakRouter.lookupAndReadDNA(info.cskr, cskr);
        typename ANCSDNA::CINFType cinf;
        pakRouter.lookupAndReadDNA(info.cinf, cinf);
        using RigPair = std::pair<std::pair<typename PAKRouter::IDType, typename ANCSDNA::CSKRType*>,
                                  std::pair<typename PAKRouter::IDType, typename ANCSDNA::CINFType*>>;
        RigPair rigPair({info.cskr, &cskr}, {info.cinf, &cinf});

        PAKEntryReadStream rs = cmdlE->beginReadStream(*node);
        DNACMDL::ReadCMDLToBlender<PAKRouter, MaterialSet, RigPair, SurfaceHeader, CMDLVersion>(
            conn, rs, pakRouter, *cmdlE, dataspec, rigPair);

        conn.saveBlend();
      }
    }
    if (const typename PAKRouter::EntryType* cinfE = pakRouter.lookupEntry(info.cinf, &node, true, false)) {
      hecl::ProjectPath cinfPath = pakRouter.getWorking(cinfE);
      if (cinfPath.getPathType() == hecl::ProjectPath::Type::None) {
        PAKEntryReadStream rs = cinfE->beginReadStream(*node);
        ANCSDNA::CINFType::Extract(dataspec, rs, cinfPath, pakRouter, *cinfE, false, btok, fileChanged);
      }
    }
  }

  /* Extract attachment CMDL/CSKR/CINFs first */
  auto attRange = pakRouter.lookupCharacterAttachmentRigs(entry.id);
  for (auto it = attRange.first; it != attRange.second; ++it) {
    auto cinfid = it->second.first.cinf;
    auto cmdlid = it->second.first.cmdl;

    const nod::Node* node;
    if (const typename PAKRouter::EntryType* cmdlE = pakRouter.lookupEntry(cmdlid, &node, true, false)) {
      hecl::ProjectPath cmdlPath = pakRouter.getWorking(cmdlE);
      if (force || cmdlPath.isNone()) {
        cmdlPath.makeDirChain(false);
        if (!conn.createBlend(cmdlPath, hecl::blender::BlendType::Mesh)) {
          return false;
        }

        std::string bestName = pakRouter.getBestEntryName(*cmdlE);
        hecl::SystemStringConv bestNameView(bestName);
        fileChanged(bestNameView.c_str());

        const auto* rp = pakRouter.lookupCMDLRigPair(cmdlid);
        typename ANCSDNA::CSKRType cskr;
        pakRouter.lookupAndReadDNA(rp->cskr, cskr);
        typename ANCSDNA::CINFType cinf;
        pakRouter.lookupAndReadDNA(rp->cinf, cinf);
        using RigPair = std::pair<std::pair<typename PAKRouter::IDType, typename ANCSDNA::CSKRType*>,
                                  std::pair<typename PAKRouter::IDType, typename ANCSDNA::CINFType*>>;
        RigPair rigPair({rp->cskr, &cskr}, {rp->cinf, &cinf});

        PAKEntryReadStream rs = cmdlE->beginReadStream(*node);
        DNACMDL::ReadCMDLToBlender<PAKRouter, MaterialSet, RigPair, SurfaceHeader, CMDLVersion>(
            conn, rs, pakRouter, *cmdlE, dataspec, rigPair);

        conn.saveBlend();
      }
    }
    if (cinfid.isValid()) {
      if (const typename PAKRouter::EntryType* cinfE = pakRouter.lookupEntry(cinfid, &node, true, false)) {
        hecl::ProjectPath cinfPath = pakRouter.getWorking(cinfE);
        if (cinfPath.getPathType() == hecl::ProjectPath::Type::None) {
          PAKEntryReadStream rs = cinfE->beginReadStream(*node);
          ANCSDNA::CINFType::Extract(dataspec, rs, cinfPath, pakRouter, *cinfE, false, btok, fileChanged);
        }
      }
    }
  }

  std::string bestName = pakRouter.getBestEntryName(entry);
  hecl::SystemStringConv bestNameView(bestName);
  fileChanged(bestNameView.c_str());

  /* Establish ANCS blend */
  if (!conn.createBlend(outPath, hecl::blender::BlendType::Actor))
    return false;

  std::string firstName;
  typename ANCSDNA::CINFType firstCinf;
  {
    hecl::blender::PyOutStream os = conn.beginPythonOut(true);

    os.format(FMT_STRING(
        "import bpy\n"
        "from mathutils import Vector\n"
        "bpy.context.scene.name = '{}'\n"
        "bpy.context.scene.hecl_mesh_obj = bpy.context.scene.name\n"
        "\n"
        "# Clear Scene\n"
        "if len(bpy.data.collections):\n"
        "    bpy.data.collections.remove(bpy.data.collections[0])\n"
        "\n"
        "actor_data = bpy.context.scene.hecl_sact_data\n"
        "arm_obj = None\n"),
        pakRouter.getBestEntryName(entry));

    std::unordered_set<typename PAKRouter::IDType> cinfsDone;
    for (const auto& info : chResInfo) {
      /* Provide data to add-on */
      os.format(FMT_STRING(
          "actor_subtype = actor_data.subtypes.add()\n"
          "actor_subtype.name = '{}'\n\n"),
          info.name);

      /* Build CINF if needed */
      if (cinfsDone.find(info.cinf) == cinfsDone.end()) {
        if (const typename PAKRouter::EntryType* cinfE = pakRouter.lookupEntry(info.cinf, nullptr, true, false)) {
          hecl::ProjectPath cinfPath = pakRouter.getWorking(cinfE);
          os.linkArmature(cinfPath.getAbsolutePathUTF8(), fmt::format(FMT_STRING("CINF_{}"), info.cinf));
          os << "if obj.name not in bpy.context.scene.objects:\n"
                "    bpy.context.scene.collection.objects.link(obj)\n";
        }
        if (cinfsDone.empty()) {
          firstName = ANCSDNA::CINFType::GetCINFArmatureName(info.cinf);
          pakRouter.lookupAndReadDNA(info.cinf, firstCinf);
        }
        cinfsDone.insert(info.cinf);
      }
      os.format(FMT_STRING("arm_obj = bpy.data.objects['CINF_{}']\n"), info.cinf);
      os << "actor_subtype.linked_armature = arm_obj.name\n";

      /* Link CMDL */
      if (const typename PAKRouter::EntryType* cmdlE = pakRouter.lookupEntry(info.cmdl, nullptr, true, false)) {
        hecl::ProjectPath cmdlPath = pakRouter.getWorking(cmdlE);
        os.linkMesh(cmdlPath.getAbsolutePathUTF8(), pakRouter.getBestEntryName(*cmdlE));

        /* Attach CMDL to CINF */
        os << "if obj.name not in bpy.context.scene.objects:\n"
              "    bpy.context.scene.collection.objects.link(obj)\n"
              "obj.parent = arm_obj\n"
              "obj.parent_type = 'ARMATURE'\n"
              "actor_subtype.linked_mesh = obj.name\n\n";
      }

      /* Link overlays */
      for (const auto& overlay : info.overlays) {
        os << "overlay = actor_subtype.overlays.add()\n";
        os.format(FMT_STRING("overlay.name = '{}'\n"), overlay.first);

        /* Link CMDL */
        if (const typename PAKRouter::EntryType* cmdlE =
            pakRouter.lookupEntry(overlay.second.first, nullptr, true, false)) {
          hecl::ProjectPath cmdlPath = pakRouter.getWorking(cmdlE);
          os.linkMesh(cmdlPath.getAbsolutePathUTF8(), pakRouter.getBestEntryName(*cmdlE));

          /* Attach CMDL to CINF */
          os << "if obj.name not in bpy.context.scene.objects:\n"
                "    bpy.context.scene.collection.objects.link(obj)\n"
                "obj.parent = arm_obj\n"
                "obj.parent_type = 'ARMATURE'\n"
                "overlay.linked_mesh = obj.name\n\n";
        }
      }
    }

    /* Link attachments */
    for (auto it = attRange.first; it != attRange.second; ++it) {
      os << "attachment = actor_data.attachments.add()\n";
      os.format(FMT_STRING("attachment.name = '{}'\n"), it->second.second);

      auto cinfid = it->second.first.cinf;
      auto cmdlid = it->second.first.cmdl;

      if (cinfid.isValid()) {
        /* Build CINF if needed */
        if (cinfsDone.find(cinfid) == cinfsDone.end()) {
          if (const typename PAKRouter::EntryType* cinfE = pakRouter.lookupEntry(cinfid, nullptr, true, false)) {
            hecl::ProjectPath cinfPath = pakRouter.getWorking(cinfE);
            os.linkArmature(cinfPath.getAbsolutePathUTF8(), fmt::format(FMT_STRING("CINF_{}"), cinfid));
            os << "if obj.name not in bpy.context.scene.objects:\n"
                  "    bpy.context.scene.collection.objects.link(obj)\n";
          }
          if (cinfsDone.empty()) {
            firstName = ANCSDNA::CINFType::GetCINFArmatureName(cinfid);
            pakRouter.lookupAndReadDNA(cinfid, firstCinf);
          }
          cinfsDone.insert(cinfid);
        }
        os.format(FMT_STRING("arm_obj = bpy.data.objects['CINF_{}']\n"), cinfid);
        os << "attachment.linked_armature = arm_obj.name\n";
      }

      /* Link CMDL */
      if (const typename PAKRouter::EntryType* cmdlE = pakRouter.lookupEntry(cmdlid, nullptr, true, false)) {
        hecl::ProjectPath cmdlPath = pakRouter.getWorking(cmdlE);
        os.linkMesh(cmdlPath.getAbsolutePathUTF8(), pakRouter.getBestEntryName(*cmdlE));

        /* Attach CMDL to CINF */
        os << "if obj.name not in bpy.context.scene.objects:\n"
              "    bpy.context.scene.collection.objects.link(obj)\n"
              "obj.parent = arm_obj\n"
              "obj.parent_type = 'ARMATURE'\n"
              "attachment.linked_mesh = obj.name\n\n";
      }
    }
  }

  {
    hecl::blender::DataStream ds = conn.beginData();
    std::unordered_map<std::string, hecl::blender::Matrix3f> matrices = ds.getBoneMatrices(firstName);
    ds.close();
    DNAANIM::RigInverter<typename ANCSDNA::CINFType> inverter(firstCinf, matrices);

    hecl::blender::PyOutStream os = conn.beginPythonOut(true);
    os << "import bpy\n"
          "actor_data = bpy.context.scene.hecl_sact_data\n";

    /* Get animation primitives */
    std::map<atUint32, AnimationResInfo<typename PAKRouter::IDType>> animResInfo;
    ancs.getAnimationResInfo(&pakRouter, animResInfo);
    for (const auto& id : animResInfo) {
      typename ANCSDNA::ANIMType anim;
      if (pakRouter.lookupAndReadDNA(id.second.animId, anim, true)) {
        os.format(FMT_STRING(
            "act = bpy.data.actions.new('{}')\n"
            "act.use_fake_user = True\n"
            "act.anim_id = '{}'\n"),
            id.second.name, id.second.animId);
        anim.sendANIMToBlender(os, inverter, id.second.additive);
      }

      os.format(FMT_STRING(
          "actor_action = actor_data.actions.add()\n"
          "actor_action.name = '{}'\n"),
          id.second.name);

      /* Extract EVNT if present */
      anim.extractEVNT(id.second, outPath, pakRouter, force);
    }
  }
  conn.saveBlend();
  return true;
}

template bool
ReadANCSToBlender<PAKRouter<DNAMP1::PAKBridge>, DNAMP1::ANCS, DNAMP1::MaterialSet, DNACMDL::SurfaceHeader_1, 2>(
    hecl::blender::Token& btok, const DNAMP1::ANCS& ancs, const hecl::ProjectPath& outPath,
    PAKRouter<DNAMP1::PAKBridge>& pakRouter, const typename PAKRouter<DNAMP1::PAKBridge>::EntryType& entry,
    const SpecBase& dataspec, std::function<void(const hecl::SystemChar*)> fileChanged, bool force);
template bool
ReadANCSToBlender<PAKRouter<DNAMP2::PAKBridge>, DNAMP2::ANCS, DNAMP2::MaterialSet, DNACMDL::SurfaceHeader_2, 4>(
    hecl::blender::Token& btok, const DNAMP2::ANCS& ancs, const hecl::ProjectPath& outPath,
    PAKRouter<DNAMP2::PAKBridge>& pakRouter, const typename PAKRouter<DNAMP2::PAKBridge>::EntryType& entry,
    const SpecBase& dataspec, std::function<void(const hecl::SystemChar*)> fileChanged, bool force);
template bool
ReadANCSToBlender<PAKRouter<DNAMP3::PAKBridge>, DNAMP3::CHAR, DNAMP3::MaterialSet, DNACMDL::SurfaceHeader_3, 4>(
    hecl::blender::Token& btok, const DNAMP3::CHAR& ancs, const hecl::ProjectPath& outPath,
    PAKRouter<DNAMP3::PAKBridge>& pakRouter, const typename PAKRouter<DNAMP3::PAKBridge>::EntryType& entry,
    const SpecBase& dataspec, std::function<void(const hecl::SystemChar*)> fileChanged, bool force);

} // namespace DataSpec::DNAANCS
