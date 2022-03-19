#include "stream/shader.hpp"

#include "../gpu.hpp"
#include "common.hpp"
#include "gx.hpp"

static logvisor::Module Log("aurora::gfx::stream");

using aurora::gfx::gx::g_gxState;

struct SStreamState {
  GX::Primitive primitive;
  u16 vertexCount = 0;
  aurora::ByteBuffer vertexBuffer;
  std::vector<u16> indices;
#ifndef NDEBUG
  GX::Attr currentAttr{};
#endif

  explicit SStreamState(GX::Primitive primitive, u16 numVerts, u16 vertexSize) noexcept : primitive(primitive) {
    vertexBuffer.reserve_extra(size_t(numVerts) * vertexSize);
    if (numVerts > 3 && (primitive == GX::TRIANGLEFAN || primitive == GX::TRIANGLESTRIP)) {
      indices.reserve((u32(numVerts) - 3) * 3 + 3);
    } else if (numVerts > 4 && primitive == GX::QUADS) {
      indices.reserve(u32(numVerts) / 4 * 6);
    } else {
      indices.reserve(numVerts);
    }
  }
};
static std::optional<SStreamState> sStreamState;

void GXBegin(GX::Primitive primitive, GX::VtxFmt vtxFmt, u16 nVerts) noexcept {
#ifndef NDEBUG
  if (sStreamState) {
    Log.report(logvisor::Fatal, FMT_STRING("Stream began twice!"));
    unreachable();
  }
#endif
  uint16_t vertexSize = 0;
  for (GX::Attr attr{}; const auto type : g_gxState.vtxDesc) {
    if (type == GX::DIRECT) {
      if (attr == GX::VA_POS || attr == GX::VA_NRM) {
        vertexSize += 12;
      } else if (attr == GX::VA_CLR0 || attr == GX::VA_CLR1) {
        vertexSize += 16;
      } else if (attr >= GX::VA_TEX0 && attr <= GX::VA_TEX7) {
        vertexSize += 8;
      } else {
        Log.report(logvisor::Fatal, FMT_STRING("don't know how to handle attr {}"), attr);
        unreachable();
      }
    } else if (type != GX::NONE) {
      Log.report(logvisor::Fatal, FMT_STRING("invalid vtx type {} for attr {}"), type, attr);
      unreachable();
    }
    attr = GX::Attr(attr + 1);
  }
  if (vertexSize == 0) {
    Log.report(logvisor::Fatal, FMT_STRING("no vtx attributes enabled?"));
    unreachable();
  }
  sStreamState.emplace(primitive, nVerts, vertexSize);
}
static inline void check_attr_order(GX::Attr attr) noexcept {
#ifndef NDEBUG
  if (!sStreamState) {
    Log.report(logvisor::Fatal, FMT_STRING("Stream not started!"));
    unreachable();
  }
  if (sStreamState->currentAttr >= attr) {
    Log.report(logvisor::Fatal, FMT_STRING("bad attribute order: {}, last {}"), attr, sStreamState->currentAttr);
    unreachable();
  }
  sStreamState->currentAttr = attr;
#endif
}
void GXPosition3f32(const zeus::CVector3f& pos) noexcept {
#ifndef NDEBUG
  if (!sStreamState) {
    Log.report(logvisor::Fatal, FMT_STRING("Stream not started!"));
    unreachable();
  }
  sStreamState->currentAttr = GX::VA_POS;
#endif
  auto& state = *sStreamState;
  state.vertexBuffer.append(&pos, 12);
  if (state.primitive == GX::TRIANGLES || state.vertexCount < 3) {
    state.indices.push_back(state.vertexCount);
  } else if (state.primitive == GX::TRIANGLEFAN) {
    state.indices.push_back(0);
    state.indices.push_back(state.vertexCount - 1);
    state.indices.push_back(state.vertexCount);
  } else if (state.primitive == GX::TRIANGLESTRIP) {
    if ((state.vertexCount & 1) == 0) {
      state.indices.push_back(state.vertexCount - 2);
      state.indices.push_back(state.vertexCount - 1);
    } else {
      state.indices.push_back(state.vertexCount - 1);
      state.indices.push_back(state.vertexCount - 2);
    }
    state.indices.push_back(state.vertexCount);
  } else if (state.primitive == GX::QUADS) {
    if ((state.vertexCount & 3) == 3) {
      state.indices.push_back(state.vertexCount - 1);
      state.indices.push_back(state.vertexCount);
      state.indices.push_back(state.vertexCount - 3);
    } else {
      state.indices.push_back(state.vertexCount);
    }
  }
  ++state.vertexCount;
}
void GXNormal3f32(const zeus::CVector3f& nrm) noexcept {
  check_attr_order(GX::VA_NRM);
  sStreamState->vertexBuffer.append(&nrm, 12);
}
void GXColor4f32(const zeus::CColor& color) noexcept {
  check_attr_order(GX::VA_CLR0);
  sStreamState->vertexBuffer.append(&color, 16);
}
void GXTexCoord2f32(const zeus::CVector2f& uv) noexcept {
  check_attr_order(GX::VA_TEX0);
  sStreamState->vertexBuffer.append(&uv, 8);
}
void GXEnd() noexcept {
  const auto vertRange = aurora::gfx::push_verts(sStreamState->vertexBuffer.data(), sStreamState->vertexBuffer.size());
  const auto indexRange = aurora::gfx::push_indices(aurora::ArrayRef{sStreamState->indices});
  aurora::gfx::stream::PipelineConfig config{};
  const auto info = populate_pipeline_config(config, GX::TRIANGLES, {});
  const auto pipeline = aurora::gfx::pipeline_ref(config);
  aurora::gfx::push_draw_command(aurora::gfx::stream::DrawData{
      .pipeline = pipeline,
      .vertRange = vertRange,
      .uniformRange = build_uniform(info),
      .indexRange = indexRange,
      .indexCount = static_cast<uint32_t>(sStreamState->indices.size()),
      .bindGroups = info.bindGroups,
  });
  sStreamState.reset();
}
