#include "DataSpec/DNACommon/PAK.hpp"

#include "DataSpec/DNAMP1/DNAMP1.hpp"
#include "DataSpec/DNAMP2/DNAMP2.hpp"
#include "DataSpec/DNAMP3/DNAMP3.hpp"

namespace DataSpec {

template <class PAKBRIDGE>
void UniqueResult::checkEntry(const PAKBRIDGE& pakBridge, const typename PAKBRIDGE::PAKType::Entry& entry) {
  UniqueResult::Type resultType = UniqueResult::Type::NotFound;
  bool foundOneLayer = false;
  const hecl::SystemString* levelName = nullptr;
  typename PAKBRIDGE::PAKType::IDType useLevelId;
  typename PAKBRIDGE::PAKType::IDType useAreaId;
  unsigned layerIdx = 0;
  for (const auto& [levelId, level] : pakBridge.m_levelDeps) {
    if (entry.id == levelId || level.resources.find(entry.id) != level.resources.end()) {
      levelName = &level.name;
      resultType = UniqueResult::Type::Level;
      break;
    }

    for (const auto& [areaId, area] : level.areas) {
      unsigned l = 0;
      for (const auto& layer : area.layers) {
        if (layer.resources.find(entry.id) != layer.resources.end()) {
          if (foundOneLayer) {
            if (useAreaId == areaId) {
              resultType = UniqueResult::Type::Area;
            } else if (useLevelId == levelId) {
              resultType = UniqueResult::Type::Level;
              break;
            } else {
              m_type = UniqueResult::Type::Pak;
              return;
            }
            continue;
          } else
            resultType = UniqueResult::Type::Layer;
          levelName = &level.name;
          useLevelId = levelId;
          useAreaId = areaId;
          layerIdx = l;
          foundOneLayer = true;
        }
        ++l;
      }
      if (area.resources.find(entry.id) != area.resources.end()) {
        if (foundOneLayer) {
          if (useAreaId == areaId) {
            resultType = UniqueResult::Type::Area;
          } else if (useLevelId == levelId) {
            resultType = UniqueResult::Type::Level;
            break;
          } else {
            m_type = UniqueResult::Type::Pak;
            return;
          }
          continue;
        } else
          resultType = UniqueResult::Type::Area;
        levelName = &level.name;
        useLevelId = levelId;
        useAreaId = areaId;
        foundOneLayer = true;
      }
    }
  }
  m_type = resultType;
  m_levelName = levelName;
  if (resultType == UniqueResult::Type::Layer || resultType == UniqueResult::Type::Area) {
    const typename PAKBRIDGE::Level::Area& area = pakBridge.m_levelDeps.at(useLevelId).areas.at(useAreaId);
    m_areaName = &area.name;
    if (resultType == UniqueResult::Type::Layer) {
      const typename PAKBRIDGE::Level::Area::Layer& layer = area.layers[layerIdx];
      m_layerName = &layer.name;
    }
  }
}

template void UniqueResult::checkEntry(const DNAMP1::PAKBridge& pakBridge,
                                       const DNAMP1::PAKBridge::PAKType::Entry& entry);
template void UniqueResult::checkEntry(const DNAMP2::PAKBridge& pakBridge,
                                       const DNAMP2::PAKBridge::PAKType::Entry& entry);
template void UniqueResult::checkEntry(const DNAMP3::PAKBridge& pakBridge,
                                       const DNAMP3::PAKBridge::PAKType::Entry& entry);

hecl::ProjectPath UniqueResult::uniquePath(const hecl::ProjectPath& pakPath) const {
  if (m_type == Type::Pak)
    return pakPath;

  hecl::ProjectPath levelDir;
  if (m_levelName)
    levelDir.assign(pakPath, *m_levelName);
  else
    levelDir = pakPath;

  if (m_type == Type::Area) {
    hecl::ProjectPath areaDir(levelDir, *m_areaName);
    return areaDir;
  } else if (m_type == Type::Layer) {
    hecl::ProjectPath areaDir(levelDir, *m_areaName);
    hecl::ProjectPath layerDir(areaDir, *m_layerName);
    return layerDir;
  }

  return levelDir;
}

template <class BRIDGETYPE>
void PAKRouter<BRIDGETYPE>::build(std::vector<BRIDGETYPE>& bridges, std::function<void(float)> progress) {
  m_bridges = &bridges;
  m_bridgePaths.clear();

  m_uniqueEntries.clear();
  m_sharedEntries.clear();
  m_charAssoc.m_cmdlRigs.clear();
  size_t count = 0;
  float bridgesSz = bridges.size();

  /* Route entries unique/shared per-pak */
  size_t bridgeIdx = 0;
  for (BRIDGETYPE& bridge : bridges) {
    const auto& name = bridge.getName();
    hecl::SystemStringConv sysName(name);

    hecl::SystemStringView::const_iterator extit = sysName.sys_str().end() - 4;
    hecl::SystemString baseName(sysName.sys_str().begin(), extit);

    m_bridgePaths.emplace_back(
        std::make_pair(hecl::ProjectPath(m_gameWorking, baseName), hecl::ProjectPath(m_gameCooked, baseName)));

    /* Index this PAK */
    bridge.build();

    /* Add to global entry lookup */
    const typename BRIDGETYPE::PAKType& pak = bridge.getPAK();
    for (const auto& entry : pak.m_entries) {
      if (!pak.m_noShare) {
        auto sSearch = m_sharedEntries.find(entry.first);
        if (sSearch != m_sharedEntries.end())
          continue;
        auto uSearch = m_uniqueEntries.find(entry.first);
        if (uSearch != m_uniqueEntries.end()) {
          m_uniqueEntries.erase(uSearch);
          m_sharedEntries[entry.first] = std::make_pair(bridgeIdx, &entry.second);
        } else
          m_uniqueEntries[entry.first] = std::make_pair(bridgeIdx, &entry.second);
      } else
        m_uniqueEntries[entry.first] = std::make_pair(bridgeIdx, &entry.second);
    }

    /* Add RigPairs to global map */
    bridge.addCMDLRigPairs(*this, m_charAssoc);

    progress(++count / bridgesSz);
    ++bridgeIdx;
  }

  /* Add named resources to catalog YAML files */
  for (BRIDGETYPE& bridge : bridges) {
    athena::io::YAMLDocWriter catalogWriter;

    enterPAKBridge(bridge);

    /* Add MAPA transforms to global map */
    bridge.addMAPATransforms(*this, m_mapaTransforms, m_overrideEntries);

    const typename BRIDGETYPE::PAKType& pak = bridge.getPAK();
    for (const auto& namedEntry : pak.m_nameEntries) {
      if (namedEntry.name == "holo_cinf")
        continue; /* Problematic corner case */
      if (auto rec = catalogWriter.enterSubRecord(namedEntry.name)) {
        hecl::ProjectPath working = getWorking(namedEntry.id);
        if (working.getAuxInfoUTF8().size()) {
          if (auto v = catalogWriter.enterSubVector()) {
            catalogWriter.writeString(working.getRelativePathUTF8());
            catalogWriter.writeString(working.getAuxInfoUTF8());
          }
        } else
          catalogWriter.writeString(working.getRelativePathUTF8());
      }
    }

    /* Write catalog */
    intptr_t curBridgeIdx = reinterpret_cast<intptr_t>(m_curBridgeIdx);
    const hecl::ProjectPath& pakPath = m_bridgePaths[curBridgeIdx].first;
    pakPath.makeDirChain(true);
    athena::io::FileWriter writer(hecl::ProjectPath(pakPath, "!catalog.yaml").getAbsolutePath());
    catalogWriter.finish(&writer);
  }
}

template <class BRIDGETYPE>
void PAKRouter<BRIDGETYPE>::enterPAKBridge(const BRIDGETYPE& pakBridge) {
  g_PakRouter = this;
  auto pit = m_bridgePaths.begin();
  size_t bridgeIdx = 0;
  for (const BRIDGETYPE& bridge : *m_bridges) {
    if (&bridge == &pakBridge) {
      m_pak = &pakBridge.getPAK();
      m_node = &pakBridge.getNode();
      m_curBridgeIdx = reinterpret_cast<void*>(bridgeIdx);
      return;
    }
    ++pit;
    ++bridgeIdx;
  }
  LogDNACommon.report(logvisor::Fatal, FMT_STRING("PAKBridge provided to PAKRouter::enterPAKBridge() was not part of build()"));
}

template <class BRIDGETYPE>
hecl::ProjectPath PAKRouter<BRIDGETYPE>::getCharacterWorking(const EntryType* entry) const {
  auto characterSearch = m_charAssoc.m_cskrToCharacter.find(entry->id);
  if (characterSearch != m_charAssoc.m_cskrToCharacter.cend()) {
    hecl::ProjectPath characterPath = getWorking(characterSearch->second.first);
    if (entry->type == FOURCC('EVNT')) {
      hecl::SystemStringConv wideStr(characterSearch->second.second);
      return characterPath.getWithExtension((hecl::SystemString(_SYS_STR(".")) + wideStr.c_str()).c_str(), true);
    }
    return characterPath.ensureAuxInfo(characterSearch->second.second);
  }
  return {};
}

template <class BRIDGETYPE>
hecl::ProjectPath PAKRouter<BRIDGETYPE>::getWorking(const EntryType* entry,
                                                    const ResExtractor<BRIDGETYPE>& extractor) const {
  if (!entry)
    return hecl::ProjectPath();

  auto overrideSearch = m_overrideEntries.find(entry->id);
  if (overrideSearch != m_overrideEntries.end())
    return overrideSearch->second;

  const PAKType* pak = m_pak;
  intptr_t curBridgeIdx = reinterpret_cast<intptr_t>(m_curBridgeIdx);
  if (pak && pak->m_noShare) {
    const EntryType* singleSearch = pak->lookupEntry(entry->id);
    if (singleSearch) {
      const hecl::ProjectPath& pakPath = m_bridgePaths[curBridgeIdx].first;
      hecl::SystemString entName = hecl::UTF8StringToSysString(getBestEntryName(*entry));
      hecl::SystemString auxInfo;
      if (extractor.fileExts[0] && !extractor.fileExts[1])
        entName += extractor.fileExts[0];
      else if (extractor.fileExts[0])
        entName += _SYS_STR(".*");
      else if (hecl::ProjectPath chWork = getCharacterWorking(entry))
        return chWork;
      return hecl::ProjectPath(pakPath, entName).ensureAuxInfo(auxInfo);
    }
  }

  auto uniqueSearch = m_uniqueEntries.find(entry->id);
  if (uniqueSearch != m_uniqueEntries.end()) {
    const BRIDGETYPE& bridge = m_bridges->at(uniqueSearch->second.first);
    const hecl::ProjectPath& pakPath = m_bridgePaths[uniqueSearch->second.first].first;
    hecl::SystemString entName = hecl::UTF8StringToSysString(getBestEntryName(*entry));
    hecl::SystemString auxInfo;
    if (extractor.fileExts[0] && !extractor.fileExts[1])
      entName += extractor.fileExts[0];
    else if (extractor.fileExts[0])
      entName += _SYS_STR(".*");
    else if (hecl::ProjectPath chWork = getCharacterWorking(entry))
      return chWork;
    if (bridge.getPAK().m_noShare) {
      return hecl::ProjectPath(pakPath, entName).ensureAuxInfo(auxInfo);
    } else {
      hecl::ProjectPath uniquePath = entry->unique.uniquePath(pakPath);
      return hecl::ProjectPath(uniquePath, entName).ensureAuxInfo(auxInfo);
    }
  }

  auto sharedSearch = m_sharedEntries.find(entry->id);
  if (sharedSearch != m_sharedEntries.end()) {
    hecl::SystemString entBase = hecl::UTF8StringToSysString(getBestEntryName(*entry));
    hecl::SystemString auxInfo;
    hecl::SystemString entName = entBase;
    if (extractor.fileExts[0] && !extractor.fileExts[1])
      entName += extractor.fileExts[0];
    else if (extractor.fileExts[0])
      entName += _SYS_STR(".*");
    else if (hecl::ProjectPath chWork = getCharacterWorking(entry))
      return chWork;
    hecl::ProjectPath sharedPath(m_sharedWorking, entName);
    return sharedPath.ensureAuxInfo(auxInfo);
  }

  LogDNACommon.report(logvisor::Fatal, FMT_STRING("Unable to find entry {}"), entry->id);
  return hecl::ProjectPath();
}

template <class BRIDGETYPE>
hecl::ProjectPath PAKRouter<BRIDGETYPE>::getWorking(const EntryType* entry) const {
  if (!entry)
    return hecl::ProjectPath();
  return getWorking(entry, BRIDGETYPE::LookupExtractor(*m_node, *m_pak, *entry));
}

template <class BRIDGETYPE>
hecl::ProjectPath PAKRouter<BRIDGETYPE>::getWorking(const IDType& id, bool silenceWarnings) const {
  return getWorking(lookupEntry(id, nullptr, silenceWarnings, false));
}

template <class BRIDGETYPE>
hecl::ProjectPath PAKRouter<BRIDGETYPE>::getCooked(const EntryType* entry) const {
  if (!entry)
    return hecl::ProjectPath();

  auto overrideSearch = m_overrideEntries.find(entry->id);
  if (overrideSearch != m_overrideEntries.end()) {
    return overrideSearch->second.getCookedPath(*m_dataSpec.overrideDataSpec(
        overrideSearch->second, m_dataSpec.getDataSpecEntry()));
  }

  const PAKType* pak = m_pak;
  intptr_t curBridgeIdx = reinterpret_cast<intptr_t>(m_curBridgeIdx);
  if (pak && pak->m_noShare) {
    const EntryType* singleSearch = pak->lookupEntry(entry->id);
    if (singleSearch) {
      const hecl::ProjectPath& pakPath = m_bridgePaths[curBridgeIdx].second;
      return hecl::ProjectPath(pakPath, getBestEntryName(*entry));
    }
  }
  auto uniqueSearch = m_uniqueEntries.find(entry->id);
  if (uniqueSearch != m_uniqueEntries.end()) {
    const BRIDGETYPE& bridge = m_bridges->at(uniqueSearch->second.first);
    const hecl::ProjectPath& pakPath = m_bridgePaths[uniqueSearch->second.first].second;
    if (bridge.getPAK().m_noShare) {
      return hecl::ProjectPath(pakPath, getBestEntryName(*entry));
    } else {
      hecl::ProjectPath uniquePath = entry->unique.uniquePath(pakPath);
      return hecl::ProjectPath(uniquePath, getBestEntryName(*entry));
    }
  }
  auto sharedSearch = m_sharedEntries.find(entry->id);
  if (sharedSearch != m_sharedEntries.end()) {
    return hecl::ProjectPath(m_sharedCooked, getBestEntryName(*entry));
  }
  LogDNACommon.report(logvisor::Fatal, FMT_STRING("Unable to find entry {}"), entry->id);
  return hecl::ProjectPath();
}

template <class BRIDGETYPE>
hecl::ProjectPath PAKRouter<BRIDGETYPE>::getCooked(const IDType& id, bool silenceWarnings) const {
  return getCooked(lookupEntry(id, nullptr, silenceWarnings, false));
}

template <class BRIDGETYPE>
hecl::SystemString PAKRouter<BRIDGETYPE>::getResourceRelativePath(const EntryType& a, const IDType& b) const {
  const nod::Node* node = m_node;
  const PAKType* pak = m_pak;
  if (!pak)
    LogDNACommon.report(logvisor::Fatal,
                        FMT_STRING("PAKRouter::enterPAKBridge() must be called before PAKRouter::getResourceRelativePath()"));
  const typename BRIDGETYPE::PAKType::Entry* be = lookupEntry(b);
  if (!be)
    return hecl::SystemString();
  hecl::ProjectPath aPath = getWorking(&a, BRIDGETYPE::LookupExtractor(*node, *pak, a));
  hecl::SystemString ret;
  for (size_t i = 0; i < aPath.levelCount(); ++i)
    ret += _SYS_STR("../");
  hecl::ProjectPath bPath = getWorking(be, BRIDGETYPE::LookupExtractor(*node, *pak, *be));
  ret += bPath.getRelativePath();
  return ret;
}

template <class BRIDGETYPE>
std::string PAKRouter<BRIDGETYPE>::getBestEntryName(const EntryType& entry, bool stdOverride) const {
  std::string name;
  for (const BRIDGETYPE& bridge : *m_bridges) {
    const typename BRIDGETYPE::PAKType& pak = bridge.getPAK();
    const typename BRIDGETYPE::PAKType::Entry* e = pak.lookupEntry(entry.id);
    if (!e)
      continue;

    if (stdOverride && !pak.m_noShare) {
      if (entry.type == FOURCC('MLVL'))
        return fmt::format(FMT_STRING("!world_{}"), entry.id);
      else if (entry.type == FOURCC('MREA'))
        return fmt::format(FMT_STRING("!area_{}"), entry.id);
      else if (entry.type == FOURCC('MAPA'))
        return fmt::format(FMT_STRING("!map_{}"), entry.id);
      else if (entry.type == FOURCC('PATH'))
        return fmt::format(FMT_STRING("!path_{}"), entry.id);
      else if (entry.type == FOURCC('MAPW'))
        return fmt::format(FMT_STRING("!mapw_{}"), entry.id);
      else if (entry.type == FOURCC('SAVW'))
        return fmt::format(FMT_STRING("!savw_{}"), entry.id);
    }

    std::string catalogueName;
    name = pak.bestEntryName(bridge.getNode(), entry, catalogueName);
    if (!catalogueName.empty())
      return name;
  }
  return name;
}

template <class BRIDGETYPE>
std::string PAKRouter<BRIDGETYPE>::getBestEntryName(const IDType& entry, bool stdOverride) const {
  std::string name;
  for (const BRIDGETYPE& bridge : *m_bridges) {
    const typename BRIDGETYPE::PAKType& pak = bridge.getPAK();
    const typename BRIDGETYPE::PAKType::Entry* e = pak.lookupEntry(entry);
    if (!e)
      continue;

    if (stdOverride && !pak.m_noShare) {
      if (e->type == FOURCC('MLVL'))
        return fmt::format(FMT_STRING("!world_{}"), e->id);
      else if (e->type == FOURCC('MREA'))
        return fmt::format(FMT_STRING("!area_{}"), e->id);
      else if (e->type == FOURCC('MAPA'))
        return fmt::format(FMT_STRING("!map_{}"), e->id);
      else if (e->type == FOURCC('PATH'))
        return fmt::format(FMT_STRING("!path_{}"), e->id);
      else if (e->type == FOURCC('MAPW'))
        return fmt::format(FMT_STRING("!mapw_{}"), e->id);
      else if (e->type == FOURCC('SAVW'))
        return fmt::format(FMT_STRING("!savw_{}"), e->id);
    }

    std::string catalogueName;
    name = pak.bestEntryName(bridge.getNode(), *e, catalogueName);
    if (!catalogueName.empty())
      return name;
  }
  return name;
}

template <class BRIDGETYPE>
bool PAKRouter<BRIDGETYPE>::extractResources(const BRIDGETYPE& pakBridge, bool force, hecl::blender::Token& btok,
                                             std::function<void(const hecl::SystemChar*, float)> progress) {
  enterPAKBridge(pakBridge);
  size_t count = 0;
  size_t sz = m_pak->m_entries.size();
  float fsz = sz;
  for (unsigned w = 0; count < sz; ++w) {
    for (const auto& item : m_pak->m_firstEntries) {
      const auto* entryPtr = m_pak->lookupEntry(item);
      ResExtractor<BRIDGETYPE> extractor = BRIDGETYPE::LookupExtractor(*m_node, *m_pak, *entryPtr);
      if (extractor.weight != w)
        continue;

      std::string bestName = getBestEntryName(*entryPtr, false);
      hecl::SystemStringConv bestNameView(bestName);
      float thisFac = ++count / fsz;
      progress(bestNameView.c_str(), thisFac);

      const nod::Node* node = m_node;

      hecl::ProjectPath working = getWorking(entryPtr, extractor);
      working.makeDirChain(false);
      hecl::ResourceLock resLk(working);
      if (!resLk)
        continue;

      /* Extract to unmodified directory */
      hecl::ProjectPath cooked = working.getCookedPath(m_dataSpec.getUnmodifiedSpec());
      if (force || cooked.isNone()) {
        cooked.makeDirChain(false);
        PAKEntryReadStream s = entryPtr->beginReadStream(*node);
        const auto fout = hecl::FopenUnique(cooked.getAbsolutePath().data(), _SYS_STR("wb"));
        std::fwrite(s.data(), 1, s.length(), fout.get());
      }

      if (extractor.func_a) /* Doesn't need PAKRouter access */
      {
        if (force || !extractor.IsFullyExtracted(working)) {
          PAKEntryReadStream s = entryPtr->beginReadStream(*node);
          extractor.func_a(s, working);
        }
      } else if (extractor.func_b) /* Needs PAKRouter access */
      {
        if (force || !extractor.IsFullyExtracted(working)) {
          PAKEntryReadStream s = entryPtr->beginReadStream(*node);
          extractor.func_b(m_dataSpec, s, working, *this, *entryPtr, force, btok,
                           [&progress, thisFac](const hecl::SystemChar* update) { progress(update, thisFac); });
        }
      }
    }
  }

  return true;
}

template <class BRIDGETYPE>
const typename BRIDGETYPE::PAKType::Entry* PAKRouter<BRIDGETYPE>::lookupEntry(const IDType& entry,
                                                                              const nod::Node** nodeOut,
                                                                              bool silenceWarnings,
                                                                              bool currentPAK) const {
  if (!entry.isValid())
    return nullptr;

  if (!m_bridges)
    LogDNACommon.report(logvisor::Fatal, FMT_STRING("PAKRouter::build() must be called before PAKRouter::lookupEntry()"));

  const PAKType* pak = m_pak;
  const nod::Node* node = m_node;
  if (pak) {
    const EntryType* ent = pak->lookupEntry(entry);
    if (ent) {
      if (nodeOut)
        *nodeOut = node;
      return ent;
    }
  }

  if (currentPAK) {
#ifndef NDEBUG
    if (!silenceWarnings)
      LogDNACommon.report(logvisor::Warning, FMT_STRING("unable to find PAK entry {} in current PAK"), entry);
#endif
    return nullptr;
  }

  for (const BRIDGETYPE& bridge : *m_bridges) {
    const PAKType& pak = bridge.getPAK();
    const EntryType* ent = pak.lookupEntry(entry);
    if (ent) {
      if (nodeOut)
        *nodeOut = &bridge.getNode();
      return ent;
    }
  }

#ifndef NDEBUG
  if (!silenceWarnings)
    LogDNACommon.report(logvisor::Warning, FMT_STRING("unable to find PAK entry {}"), entry);
#endif
  if (nodeOut)
    *nodeOut = nullptr;
  return nullptr;
}

template <class BRIDGETYPE>
const typename CharacterAssociations<typename PAKRouter<BRIDGETYPE>::IDType>::RigPair*
PAKRouter<BRIDGETYPE>::lookupCMDLRigPair(const IDType& id) const {
  auto search = m_charAssoc.m_cmdlRigs.find(id);
  if (search == m_charAssoc.m_cmdlRigs.end())
    return nullptr;
  return &search->second;
}

template <class BRIDGETYPE>
const typename CharacterAssociations<typename PAKRouter<BRIDGETYPE>::IDType>::MultimapIteratorPair
PAKRouter<BRIDGETYPE>::lookupCharacterAttachmentRigs(const IDType& id) const {
  return m_charAssoc.m_characterToAttachmentRigs.equal_range(id);
}

template <class BRIDGETYPE>
const zeus::CMatrix4f* PAKRouter<BRIDGETYPE>::lookupMAPATransform(const IDType& id) const {
  auto search = m_mapaTransforms.find(id);
  if (search == m_mapaTransforms.end())
    return nullptr;
  return &search->second;
}

template <class BRIDGETYPE>
hecl::ProjectPath PAKRouter<BRIDGETYPE>::getAreaLayerWorking(const IDType& areaId, int layerIdx) const {
  if (!m_bridges)
    LogDNACommon.report(logvisor::Fatal, FMT_STRING("PAKRouter::build() must be called before PAKRouter::getAreaLayerWorking()"));
  auto bridgePathIt = m_bridgePaths.cbegin();
  for (const BRIDGETYPE& bridge : *m_bridges) {
    for (const auto& level : bridge.m_levelDeps)
      for (const auto& area : level.second.areas)
        if (area.first == areaId) {
          hecl::ProjectPath levelPath(bridgePathIt->first, level.second.name);
          hecl::ProjectPath areaPath(levelPath, area.second.name);
          if (layerIdx < 0)
            return areaPath;
          return hecl::ProjectPath(areaPath, area.second.layers.at(layerIdx).name);
        }
    ++bridgePathIt;
  }
  return hecl::ProjectPath();
}

template <class BRIDGETYPE>
hecl::ProjectPath PAKRouter<BRIDGETYPE>::getAreaLayerWorking(const IDType& areaId, int layerIdx,
                                                             bool& activeOut) const {
  activeOut = false;
  if (!m_bridges)
    LogDNACommon.report(logvisor::Fatal, FMT_STRING("PAKRouter::build() must be called before PAKRouter::getAreaLayerWorking()"));
  auto bridgePathIt = m_bridgePaths.cbegin();
  for (const BRIDGETYPE& bridge : *m_bridges) {
    for (const auto& level : bridge.m_levelDeps)
      for (const auto& area : level.second.areas)
        if (area.first == areaId) {
          hecl::ProjectPath levelPath(bridgePathIt->first, level.second.name);
          hecl::ProjectPath areaPath(levelPath, area.second.name);
          if (layerIdx < 0)
            return areaPath;
          const typename Level<IDType>::Area::Layer& layer = area.second.layers.at(layerIdx);
          activeOut = layer.active;
          return hecl::ProjectPath(areaPath, layer.name);
        }
    ++bridgePathIt;
  }
  return hecl::ProjectPath();
}

template <class BRIDGETYPE>
hecl::ProjectPath PAKRouter<BRIDGETYPE>::getAreaLayerCooked(const IDType& areaId, int layerIdx) const {
  if (!m_bridges)
    LogDNACommon.report(logvisor::Fatal, FMT_STRING("PAKRouter::build() must be called before PAKRouter::getAreaLayerCooked()"));
  auto bridgePathIt = m_bridgePaths.cbegin();
  for (const BRIDGETYPE& bridge : *m_bridges) {
    for (const auto& level : bridge.m_levelDeps)
      for (const auto& area : level.second.areas)
        if (area.first == areaId) {
          hecl::ProjectPath levelPath(bridgePathIt->second, level.second.name);
          hecl::ProjectPath areaPath(levelPath, area.second.name);
          if (layerIdx < 0)
            return areaPath;
          return hecl::ProjectPath(areaPath, area.second.layers.at(layerIdx).name);
        }
    ++bridgePathIt;
  }
  return hecl::ProjectPath();
}

template <class BRIDGETYPE>
hecl::ProjectPath PAKRouter<BRIDGETYPE>::getAreaLayerCooked(const IDType& areaId, int layerIdx, bool& activeOut) const {
  activeOut = false;
  if (!m_bridges)
    LogDNACommon.report(logvisor::Fatal, FMT_STRING("PAKRouter::build() must be called before PAKRouter::getAreaLayerCooked()"));
  auto bridgePathIt = m_bridgePaths.cbegin();
  for (const BRIDGETYPE& bridge : *m_bridges) {
    for (const auto& level : bridge.m_levelDeps)
      for (const auto& area : level.second.areas)
        if (area.first == areaId) {
          hecl::ProjectPath levelPath(bridgePathIt->second, level.second.name);
          hecl::ProjectPath areaPath(levelPath, area.second.name);
          if (layerIdx < 0)
            return areaPath;
          const typename Level<IDType>::Area::Layer& layer = area.second.layers.at(layerIdx);
          activeOut = layer.active;
          return hecl::ProjectPath(areaPath, layer.name);
        }
    ++bridgePathIt;
  }
  return hecl::ProjectPath();
}

template <class BRIDGETYPE>
void PAKRouter<BRIDGETYPE>::enumerateResources(const std::function<bool(const EntryType*)>& func) {
  if (!m_bridges)
    LogDNACommon.report(logvisor::Fatal, FMT_STRING("PAKRouter::build() must be called before PAKRouter::enumerateResources()"));
  for (const auto& entryPair : m_uniqueEntries)
    if (!func(entryPair.second.second))
      return;
  for (const auto& entryPair : m_sharedEntries)
    if (!func(entryPair.second.second))
      return;
}

template <class BRIDGETYPE>
bool PAKRouter<BRIDGETYPE>::mreaHasDupeResources(const IDType& id) const {
  const PAKType* pak = m_pak;
  if (!pak)
    LogDNACommon.report(logvisor::Fatal,
                        FMT_STRING("PAKRouter::enterPAKBridge() must be called before PAKRouter::mreaHasDupeResources()"));
  return pak->mreaHasDupeResources(id);
}

template class PAKRouter<DNAMP1::PAKBridge>;
template class PAKRouter<DNAMP2::PAKBridge>;
template class PAKRouter<DNAMP3::PAKBridge>;

} // namespace DataSpec
