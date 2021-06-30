#pragma once

#include <vector>

#include "DataSpec/DNACommon/PAK.hpp"
#include "DataSpec/DNACommon/MAPU.hpp"
#include "DNAMP2.hpp"

namespace DataSpec::DNAMP2 {

struct MAPU : DNAMAPU::MAPU {
  static bool Extract(const SpecBase& dataSpec, PAKEntryReadStream& rs, const hecl::ProjectPath& outPath,
                      PAKRouter<PAKBridge>& pakRouter, const DNAMP2::PAK::Entry& entry, bool force,
                      hecl::blender::Token& btok, std::function<void(const char*)> fileChanged) {
    MAPU mapu;
    mapu.read(rs);
    hecl::blender::Connection& conn = btok.getBlenderConnection();
    return DNAMAPU::ReadMAPUToBlender(conn, mapu, outPath, pakRouter, entry, force);
  }
};
} // namespace DataSpec::DNAMP2
