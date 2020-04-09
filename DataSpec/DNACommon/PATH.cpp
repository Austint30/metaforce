#include "PATH.hpp"
#include "hecl/Blender/Connection.hpp"
#include "zeus/CAABox.hpp"
#include "DataSpec/DNACommon/AROTBuilder.hpp"

namespace DataSpec::DNAPATH {

#define DUMP_OCTREE 0

#if DUMP_OCTREE
/* octree dumper */
static void OutputOctreeNode(hecl::blender::PyOutStream& os, int idx, const zeus::CAABox& aabb) {
  const zeus::CVector3f pos = aabb.center();
  const zeus::CVector3f extent = aabb.extents();
  os.format(
      "obj = bpy.data.objects.new('Leaf_%d', None)\n"
      "bpy.context.scene.collection.objects.link(obj)\n"
      "obj.location = (%f,%f,%f)\n"
      "obj.scale = (%f,%f,%f)\n"
      "obj.empty_display_type = 'CUBE'\n"
      "obj.layers[1] = True\n"
      "obj.layers[0] = False\n",
      idx, pos.x(), pos.y(), pos.z(), extent.x(), extent.y(), extent.z());
}
#endif

template <class PAKBridge>
void PATH<PAKBridge>::sendToBlender(hecl::blender::Connection& conn, std::string_view entryName,
                                    const zeus::CMatrix4f* xf, const std::string& areaPath) {
  /* Open Py Stream and read sections */
  hecl::blender::PyOutStream os = conn.beginPythonOut(true);
  os << "import bpy\n"
        "import bmesh\n"
        "from mathutils import Vector, Matrix\n"
        "\n"
        "bpy.types.Material.retro_path_idx_mask = bpy.props.IntProperty(name='Retro: Path Index Mask')\n"
        "bpy.types.Material.retro_path_type_mask = bpy.props.IntProperty(name='Retro: Path Type Mask')\n"
        "\n"
        "material_dict = {}\n"
        "material_index = []\n"
        "def make_ground_material(idxMask):\n"
        "    mat = bpy.data.materials.new('Ground %X' % idxMask)\n"
        "    mat.diffuse_color = (0.8, 0.460, 0.194, 1.0)\n"
        "    return mat\n"
        "def make_flyer_material(idxMask):\n"
        "    mat = bpy.data.materials.new('Flyer %X' % idxMask)\n"
        "    mat.diffuse_color = (0.016, 0.8, 0.8, 1.0)\n"
        "    return mat\n"
        "def make_swimmer_material(idxMask):\n"
        "    mat = bpy.data.materials.new('Swimmer %X' % idxMask)\n"
        "    mat.diffuse_color = (0.074, 0.293, 0.8, 1.0)\n"
        "    return mat\n"
        "def select_material(meshIdxMask, meshTypeMask):\n"
        "    key = (meshIdxMask, meshTypeMask)\n"
        "    if key in material_index:\n"
        "        return material_index.index(key)\n"
        "    elif key in material_dict:\n"
        "        material_index.append(key)\n"
        "        return len(material_index)-1\n"
        "    else:\n"
        "        if meshTypeMask == 0x2:\n"
        "            mat = make_flyer_material(meshIdxMask)\n"
        "        elif meshTypeMask == 0x4:\n"
        "            mat = make_swimmer_material(meshIdxMask)\n"
        "        else:\n"
        "            mat = make_ground_material(meshIdxMask)\n"
        "        mat.retro_path_idx_mask = meshIdxMask\n"
        "        mat.retro_path_type_mask = meshTypeMask\n"
        "        material_dict[key] = mat\n"
        "        material_index.append(key)\n"
        "        return len(material_index)-1\n"
        "\n";
  os.format(fmt("bpy.context.scene.name = '{}'\n"), entryName);
  os << "# Clear Scene\n"
        "if len(bpy.data.collections):\n"
        "    bpy.data.collections.remove(bpy.data.collections[0])\n"
        "\n"
        "bm = bmesh.new()\n"
        "height_lay = bm.faces.layers.float.new('Height')\n";

  for (const Node& n : nodes) {
    zeus::simd_floats f(n.position.simd);
    os.format(fmt("bm.verts.new(({},{},{}))\n"), f[0], f[1], f[2]);
  }

  os << "bm.verts.ensure_lookup_table()\n";

  for (const Region& r : regions) {
    os << "tri_verts = []\n";
    for (atUint32 i = 0; i < r.nodeCount; ++i)
      os.format(fmt("tri_verts.append(bm.verts[{}])\n"), r.nodeStart + i);

    os.format(fmt("face = bm.faces.get(tri_verts)\n"
                  "if face is None:\n"
                  "    face = bm.faces.new(tri_verts)\n"
                  "    face.normal_flip()\n"
                  "face.material_index = select_material(0x{:04X}, 0x{:04X})\n"
                  "face.smooth = False\n"
                  "face[height_lay] = {}\n"
                  "\n"),
              r.meshIndexMask, r.meshTypeMask, r.height);

#if 0
        const zeus::CVector3f center = xf->multiplyOneOverW(r.centroid);
        zeus::CAABox aabb(xf->multiplyOneOverW(r.aabb[0]), xf->multiplyOneOverW(r.aabb[1]));
        os.format(fmt("aabb = bpy.data.objects.new('AABB', None)\n")
                  "aabb.location = (%f,%f,%f)\n"
                  "aabb.scale = (%f,%f,%f)\n"
                  "aabb.empty_display_type = 'CUBE'\n"
                  "bpy.context.scene.collection.objects.link(aabb)\n"
                  "centr = bpy.data.objects.new('Center', None)\n"
                  "centr.location = (%f,%f,%f)\n"
                  "bpy.context.scene.collection.objects.link(centr)\n",
                  aabb.min[0] + (aabb.max[0] - aabb.min[0]) / 2.f,
                  aabb.min[1] + (aabb.max[1] - aabb.min[1]) / 2.f,
                  aabb.min[2] + (aabb.max[2] - aabb.min[2]) / 2.f,
                  (aabb.max[0] - aabb.min[0]) / 2.f,
                  (aabb.max[1] - aabb.min[1]) / 2.f,
                  (aabb.max[2] - aabb.min[2]) / 2.f,
                  center.x(), center.y(), center.z());
#endif
  }

#if 0
  for (const Node& n : nodes) {
    zeus::simd_floats f(n.position.simd);
    zeus::simd_floats no(n.position.simd + n.normal.simd);
    os.format(fmt("v = bm.verts.new((%f,%f,%f))\n")
              "v2 = bm.verts.new((%f,%f,%f))\n"
              "bm.edges.new((v, v2))\n", f[0], f[1], f[2], no[0], no[1], no[2]);
  }
#endif

  os << "bmesh.ops.remove_doubles(bm, verts=bm.verts, dist=0.001)\n"
        "path_mesh = bpy.data.meshes.new('PATH')\n"
        "bm.to_mesh(path_mesh)\n"
        "path_mesh_obj = bpy.data.objects.new(path_mesh.name, path_mesh)\n"
        "\n"
        "for mat_name in material_index:\n"
        "    mat = material_dict[mat_name]\n"
        "    path_mesh.materials.append(mat)\n"
        "\n"
        "bpy.context.scene.collection.objects.link(path_mesh_obj)\n"
        "path_mesh_obj.display_type = 'SOLID'\n"
        "bpy.context.scene.hecl_path_obj = path_mesh_obj.name\n"
        "\n";

  if (xf) {
    const zeus::CMatrix4f& w = *xf;
    zeus::simd_floats xfMtxF[4];
    for (int i = 0; i < 4; ++i)
      w.m[i].mSimd.copy_to(xfMtxF[i]);
    os.format(fmt("mtx = Matrix((({},{},{},{}),({},{},{},{}),({},{},{},{}),(0.0,0.0,0.0,1.0)))\n"
                  "mtxd = mtx.decompose()\n"
                  "path_mesh_obj.rotation_mode = 'QUATERNION'\n"
                  "path_mesh_obj.location = mtxd[0]\n"
                  "path_mesh_obj.rotation_quaternion = mtxd[1]\n"
                  "path_mesh_obj.scale = mtxd[2]\n"),
              xfMtxF[0][0], xfMtxF[1][0], xfMtxF[2][0], xfMtxF[3][0], xfMtxF[0][1], xfMtxF[1][1], xfMtxF[2][1],
              xfMtxF[3][1], xfMtxF[0][2], xfMtxF[1][2], xfMtxF[2][2], xfMtxF[3][2]);
  }

#if DUMP_OCTREE
  {
    int idx = 0;
    for (const auto& n : octree) {
      if (n.isLeaf)
        OutputOctreeNode(os, idx, zeus::CAABox(n.aabb[0], n.aabb[1]));
      ++idx;
    }
  }
#endif

  os.linkBackground(fmt::format(fmt("//{}"), areaPath));
  os.centerView();
  os.close();
}

template <class PAKBridge>
bool PATH<PAKBridge>::Extract(const SpecBase& dataSpec, PAKEntryReadStream& rs, const hecl::ProjectPath& outPath,
                              PAKRouter<PAKBridge>& pakRouter, const typename PAKBridge::PAKType::Entry& entry,
                              bool force, hecl::blender::Token& btok,
                              std::function<void(const hecl::SystemChar*)> fileChanged) {
  PATH path;
  path.read(rs);
  hecl::blender::Connection& conn = btok.getBlenderConnection();
  if (!conn.createBlend(outPath, hecl::blender::BlendType::PathMesh))
    return false;

  std::string areaPath;
  for (const auto& ent : hecl::DirectoryEnumerator(outPath.getParentPath().getAbsolutePath())) {
    if (hecl::StringUtils::BeginsWith(ent.m_name, _SYS_STR("!area_"))) {
      areaPath = hecl::SystemUTF8Conv(ent.m_name).str();
      break;
    }
  }

  const zeus::CMatrix4f* xf = pakRouter.lookupMAPATransform(entry.id);
  path.sendToBlender(conn, pakRouter.getBestEntryName(entry, false), xf, areaPath);
  return conn.saveBlend();
}

template <class PAKBridge>
bool PATH<PAKBridge>::Cook(const hecl::ProjectPath& outPath, const hecl::ProjectPath& inPath, const PathMesh& mesh,
                           hecl::blender::Token& btok) {
  athena::io::MemoryReader r(mesh.data.data(), mesh.data.size());
  PATH path;
  path.read(r);
  if (!path.regions.empty()) {
    AROTBuilder octreeBuilder;
    octreeBuilder.buildPath(path);
  } else {
    path.octreeNodeCount = 1;
    path.octree.emplace_back();
    OctreeNode& n = path.octree.back();
    n.isLeaf = 1;
    n.aabb[0] = zeus::CVector3f{FLT_MAX, FLT_MAX, FLT_MAX};
    n.aabb[1] = zeus::CVector3f{-FLT_MAX, -FLT_MAX, -FLT_MAX};
    for (int i = 0; i < 8; ++i)
      n.children[i] = 0xffffffff;
  }

#if DUMP_OCTREE
  {
    hecl::blender::Connection& conn = btok.getBlenderConnection();
    if (!conn.createBlend(inPath.getWithExtension(_SYS_STR(".octree.blend"), true), hecl::blender::BlendType::PathMesh))
      return false;

    zeus::CMatrix4f xf;
    path.sendToBlender(conn, "PATH"sv, &xf);
    conn.saveBlend();
  }
#endif

  athena::io::FileWriter w(outPath.getAbsolutePath());
  path.write(w);
  int64_t rem = w.position() % 32;
  if (rem)
    for (int64_t i = 0; i < 32 - rem; ++i)
      w.writeUByte(0xff);
  return true;
}

template struct PATH<DataSpec::DNAMP1::PAKBridge>;
template struct PATH<DataSpec::DNAMP2::PAKBridge>;
template struct PATH<DataSpec::DNAMP3::PAKBridge>;

} // namespace DataSpec::DNAPATH