#pragma once

#include "DNACommon.hpp"
#include "DeafBabe.hpp"
#include "zeus/CAABox.hpp"
#include "CMDL.hpp"
#include <set>

namespace DataSpec {
namespace DNAPATH {
template <atUint32 PathVer>
struct PATH;
}


struct AROTBuilder {
  using ColMesh = hecl::blender::ColMesh;

  struct BitmapPool {
    std::vector<std::set<int>> m_pool;
    size_t addIndices(const std::set<int>& indices);
  } bmpPool;

  struct Node {
    std::vector<Node> childNodes;
    std::set<int> childIndices;
    size_t poolIdx = 0;
    uint16_t flags = 0;
    uint16_t compSubdivs = 0;

    size_t nodeOff = 0;
    size_t nodeSz = 4;

    void addChild(int level, int minChildren, const std::vector<zeus::CAABox>& triBoxes, const zeus::CAABox& curAABB,
                  BspNodeType& typeOut);
    void mergeSets(int a, int b);
    bool compareSets(int a, int b) const;
    void nodeCount(size_t& sz, size_t& idxRefs, BitmapPool& bmpPool, size_t& curOff);
    void writeIndirectionTable(athena::io::MemoryWriter& w);
    void writeNodes(athena::io::MemoryWriter& w, int nodeIdx);
    void advanceIndex(int& nodeIdx);

    void colSize(size_t& totalSz);
    void writeColNodes(uint8_t*& ptr, const zeus::CAABox& curAABB);

    void pathCountNodesAndLookups(size_t& nodeCount, size_t& lookupCount);
    template <uint32_t PathVer>
    void pathWrite(DNAPATH::PATH<PathVer>& path, const zeus::CAABox& curAABB);
  } rootNode;

  void build(std::vector<std::vector<uint8_t>>& secs, const zeus::CAABox& fullAabb,
             const std::vector<zeus::CAABox>& meshAabbs, const std::vector<DNACMDL::Mesh>& meshes);
  std::pair<std::unique_ptr<uint8_t[]>, uint32_t> buildCol(const ColMesh& mesh, BspNodeType& rootOut);
  template <uint32_t PathVer>
  void buildPath(DNAPATH::PATH<PathVer>& path);
};

} // namespace DataSpec
