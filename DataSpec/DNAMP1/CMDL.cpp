#include "CMDL.hpp"
#include "hecl/Blender/Connection.hpp"

namespace DataSpec::DNAMP1 {

bool CMDL::Extract(const SpecBase& dataSpec, PAKEntryReadStream& rs, const hecl::ProjectPath& outPath,
                   PAKRouter<PAKBridge>& pakRouter, const PAK::Entry& entry, bool force, hecl::blender::Token& btok,
                   std::function<void(const char*)> fileChanged) {
  /* Check for RigPair */
  CINF cinf;
  CSKR cskr;
  using RigPair = std::pair<std::pair<UniqueID32, CSKR*>, std::pair<UniqueID32, CINF*>>;
  RigPair loadRp = {};
  if (const typename CharacterAssociations<UniqueID32>::RigPair* rp = pakRouter.lookupCMDLRigPair(entry.id)) {
    pakRouter.lookupAndReadDNA(rp->cskr, cskr);
    pakRouter.lookupAndReadDNA(rp->cinf, cinf);
    loadRp.first = {rp->cskr, &cskr};
    loadRp.second = {rp->cinf, &cinf};
  }

  /* Do extract */
  hecl::blender::Connection& conn = btok.getBlenderConnection();
  if (!conn.createBlend(outPath, hecl::blender::BlendType::Mesh))
    return false;
  DNACMDL::ReadCMDLToBlender<PAKRouter<PAKBridge>, MaterialSet, RigPair, DNACMDL::SurfaceHeader_1, 2>(
      conn, rs, pakRouter, entry, dataSpec, loadRp);
  conn.saveBlend();

#if 0
  /* Cook and re-extract test */
  hecl::ProjectPath tempOut = outPath.getWithExtension(".recook", true);
  hecl::blender::Connection::DataStream ds = conn.beginData();
  DNACMDL::Mesh mesh = ds.compileMesh(hecl::TopologyTriStrips, -1);
  ds.close();
  DNACMDL::WriteCMDL<MaterialSet, DNACMDL::SurfaceHeader_1_2, 2>(tempOut, outPath, mesh);

  athena::io::FileReader reader(tempOut.getAbsolutePath());
  hecl::ProjectPath tempBlend = outPath.getWithExtension(".recook.blend", true);
  if (!conn.createBlend(tempBlend, hecl::blender::Connection::TypeMesh))
      return false;
  DNACMDL::ReadCMDLToBlender<PAKRouter<PAKBridge>, MaterialSet, std::pair<CSKR*,CINF*>, DNACMDL::SurfaceHeader_1_2, 2>
          (conn, reader, pakRouter, entry, dataSpec, loadRp);
  return conn.saveBlend();
#elif 0
  /* HMDL cook test */
  hecl::ProjectPath tempOut = outPath.getWithExtension(".recook", true);
  hecl::blender::Connection::DataStream ds = conn.beginData();
  DNACMDL::Mesh mesh = ds.compileMesh(hecl::HMDLTopology::TriStrips, 16);
  ds.close();
  DNACMDL::WriteHMDLCMDL<HMDLMaterialSet, DNACMDL::SurfaceHeader_1, 2>(tempOut, outPath, mesh);
#endif

  return true;
}

bool CMDL::Cook(const hecl::ProjectPath& outPath, const hecl::ProjectPath& inPath, const DNACMDL::Mesh& mesh) {
  if (!mesh.skins.empty()) {
    DNACMDL::Mesh skinMesh = mesh.getContiguousSkinningVersion();
    if (!DNACMDL::WriteCMDL<MaterialSet, DNACMDL::SurfaceHeader_1, 2>(outPath, inPath, skinMesh))
      return false;

    /* Output skinning intermediate */
    auto vertCountIt = skinMesh.contiguousSkinVertCounts.cbegin();
    athena::io::FileWriter writer(outPath.getWithExtension(".skinint").getAbsolutePath());
    writer.writeUint32Big(skinMesh.boneNames.size());
    for (const std::string& boneName : skinMesh.boneNames)
      writer.writeString(boneName);

    writer.writeUint32Big(skinMesh.skins.size());
    for (const auto& skin : skinMesh.skins) {
      size_t numBinds = skinMesh.countSkinBinds(skin);
      writer.writeUint32Big(numBinds);
      for (size_t i = 0; i < numBinds; ++i) {
        writer.writeUint32Big(skin[i].vg_idx);
        writer.writeFloatBig(skin[i].weight);
      }
      writer.writeUint32Big(*vertCountIt++);
    }
    writer.writeUint32Big(skinMesh.pos.size());
    writer.writeUint32Big(skinMesh.norm.size());
  } else if (!DNACMDL::WriteCMDL<MaterialSet, DNACMDL::SurfaceHeader_1, 2>(outPath, inPath, mesh))
    return false;
  return true;
}

bool CMDL::HMDLCook(const hecl::ProjectPath& outPath, const hecl::ProjectPath& inPath, const DNACMDL::Mesh& mesh) {
  hecl::blender::PoolSkinIndex poolSkinIndex;
  if (mesh.skins.size()) {
    if (!DNACMDL::WriteHMDLCMDL<HMDLMaterialSet, DNACMDL::SurfaceHeader_2, 2>(outPath, inPath, mesh, poolSkinIndex))
      return false;

    /* Output skinning intermediate */
    athena::io::FileWriter writer(outPath.getWithExtension(".skinint").getAbsolutePath());
    writer.writeUint32Big(mesh.skinBanks.banks.size());
    for (const DNACMDL::Mesh::SkinBanks::Bank& sb : mesh.skinBanks.banks) {
      writer.writeUint32Big(sb.m_boneIdxs.size());
      for (uint32_t bind : sb.m_boneIdxs)
        writer.writeUint32Big(bind);
    }
    writer.writeUint32Big(mesh.boneNames.size());
    for (const std::string& boneName : mesh.boneNames)
      writer.writeString(boneName);

    /* CVirtualBone structure just like original (for CPU skinning) */
    writer.writeUint32Big(mesh.skins.size());
    for (auto& s : mesh.skins) {
      size_t numBinds = mesh.countSkinBinds(s);
      writer.writeUint32Big(numBinds);
      for (size_t i = 0; i < numBinds; ++i) {
        writer.writeUint32Big(s[i].vg_idx);
        writer.writeFloatBig(s[i].weight);
      }
    }

    /* Write indirection table mapping pool verts to CVirtualBones */
    writer.writeUint32Big(poolSkinIndex.m_poolSz);
    for (uint32_t i = 0; i < poolSkinIndex.m_poolSz; ++i)
      writer.writeUint32Big(poolSkinIndex.m_poolToSkinIndex[i]);
  } else if (!DNACMDL::WriteHMDLCMDL<HMDLMaterialSet, DNACMDL::SurfaceHeader_2, 2>(outPath, inPath, mesh,
                                                                                   poolSkinIndex))
    return false;
  return true;
}

} // namespace DataSpec::DNAMP1
