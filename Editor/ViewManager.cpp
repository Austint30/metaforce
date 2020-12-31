#include "ViewManager.hpp"
#include "specter/Space.hpp"
#include "specter/Menu.hpp"
#include "SplashScreen.hpp"
#include "locale/locale.hpp"
#include "ResourceBrowser.hpp"
#include "icons/icons.hpp"
#include "badging/Badging.hpp"
#include "Runtime/Graphics/CModel.hpp"
#include "Runtime/Graphics/CGraphics.hpp"
#include "Runtime/Character/CSkinRules.hpp"
#include "Graphics/CMetroidModelInstance.hpp"
#include "World/CWorldTransManager.hpp"
#include "Graphics/Shaders/CColoredQuadFilter.hpp"
#include "Audio/CStreamAudioManager.hpp"
#include "Runtime/CStateManager.hpp"
#include "Runtime/World/CPlayer.hpp"
#include "hecl/Pipeline.hpp"
#include <cstdio>

using YAMLNode = athena::io::YAMLNode;

extern hecl::SystemString ExeDir;

namespace urde {

void ViewManager::InitMP1(MP1::CMain& main) {
  main.Init(m_fileStoreManager, &m_cvarManager, m_mainWindow.get(), m_voiceEngine.get(), *m_amuseAllocWrapper);
  if (!m_noShaderWarmup)
    main.WarmupShaders();

  m_testGameView.reset(new TestGameView(*this, m_viewResources, *m_rootView, m_cvarManager));

  m_rootView->accessContentViews().clear();
  m_rootView->accessContentViews().push_back(m_testGameView.get());
  m_rootView->updateSize();
}

void ViewManager::TestGameView::resized(const boo::SWindowRect& root, const boo::SWindowRect& sub) {
  specter::View::resized(root, sub);
  urde::CGraphics::SetViewportResolution({sub.size[0], sub.size[1]});
  if (m_debugText) {
    boo::SWindowRect newSub = sub;
    newSub.location[1] = 5 * m_vm.m_viewResources.pixelFactor();
    m_debugText->resized(root, newSub);
  }
}

void ViewManager::TestGameView::draw(boo::IGraphicsCommandQueue* gfxQ) {
  m_vm.m_projManager.mainDraw();
  if (m_debugText)
    m_debugText->draw(gfxQ);
}

void ViewManager::TestGameView::think() {
  if (!m_debugText) {
    m_debugText.reset(new specter::MultiLineTextView(m_vm.m_viewResources, *this, m_vm.m_viewResources.m_monoFont18));
    boo::SWindowRect sub = subRect();
    sub.location[1] = 5 * m_vm.m_viewResources.pixelFactor();
    m_debugText->resized(rootView().subRect(), sub);
  }

  if (m_debugText) {
    std::string overlayText;
    if (g_StateManager) {
      if (m_cvarCommons.m_debugOverlayShowFrameCounter->toBoolean())
        overlayText += fmt::format(FMT_STRING("Frame: {}\n"), g_StateManager->GetUpdateFrameIndex());

      if (m_cvarCommons.m_debugOverlayShowInGameTime->toBoolean()) {
        double igt = g_GameState->GetTotalPlayTime();
        u32 ms = u64(igt * 1000) % 1000;
        auto pt = std::div(igt, 3600);
        overlayText +=
            fmt::format(FMT_STRING("PlayTime: {:02d}:{:02d}:{:02d}.{:03d}\n"), pt.quot, pt.rem / 60, pt.rem % 60, ms);
        if (g_StateManager->GetCurrentArea() != nullptr) {
          if (m_currentRoom != g_StateManager->GetCurrentArea()) {
            m_currentRoom = static_cast<const void*>(g_StateManager->GetCurrentArea());
            m_lastRoomTime = igt - m_currentRoomStart;
            m_currentRoomStart = igt;
          }
          double currentRoomTime = igt - m_currentRoomStart;
          u32 curFrames = std::round(u32(currentRoomTime * 60));
          u32 lastFrames = std::round(u32(m_lastRoomTime * 60));
          overlayText += fmt::format(FMT_STRING("Room Time:{:8.3f}/{:6d}| Last Room:{:8.3f}/{:6d}\n"),
                                     currentRoomTime, curFrames,
                                     m_lastRoomTime, lastFrames);
        }
      }

      if (g_StateManager->Player() && m_cvarCommons.m_debugOverlayPlayerInfo->toBoolean()) {
        const CPlayer& pl = g_StateManager->GetPlayer();
        const zeus::CQuaternion plQ = zeus::CQuaternion(pl.GetTransform().getRotation().buildMatrix3f());
        const zeus::CTransform camXf = g_StateManager->GetCameraManager()->GetCurrentCameraTransform(*g_StateManager);
        const zeus::CQuaternion camQ = zeus::CQuaternion(camXf.getRotation().buildMatrix3f());
        overlayText += fmt::format(FMT_STRING("Player Position: x {}, y {}, z {}\n"
                                       "       Roll: {}, Pitch: {}, Yaw: {}\n"
                                       "       Momentum: x {}, y: {}, z: {}\n"
                                       "       Velocity: x {}, y: {}, z: {}\n"
                                       "Camera Position: x {}, y {}, z {}\n"
                                       "       Roll: {}, Pitch: {}, Yaw: {}\n"),
                                   pl.GetTranslation().x(), pl.GetTranslation().y(), pl.GetTranslation().z(),
                                   zeus::radToDeg(plQ.roll()), zeus::radToDeg(plQ.pitch()), zeus::radToDeg(plQ.yaw()),
                                   pl.GetMomentum().x(), pl.GetMomentum().y(), pl.GetMomentum().z(),
                                   pl.GetVelocity().x(), pl.GetVelocity().y(), pl.GetVelocity().z(), camXf.origin.x(),
                                   camXf.origin.y(), camXf.origin.z(), zeus::radToDeg(camQ.roll()),
                                   zeus::radToDeg(camQ.pitch()), zeus::radToDeg(camQ.yaw()));
      }
      if (m_cvarCommons.m_debugOverlayWorldInfo->toBoolean()) {
        TLockedToken<CStringTable> tbl =
            g_SimplePool->GetObj({FOURCC('STRG'), g_StateManager->GetWorld()->IGetStringTableAssetId()});
        const urde::TAreaId aId = g_GameState->CurrentWorldState().GetCurrentAreaId();
        overlayText += fmt::format(FMT_STRING("World: 0x{}{}, Area: {}\n"), g_GameState->CurrentWorldAssetId(),
                                   (tbl.IsLoaded() ? (" " + hecl::Char16ToUTF8(tbl->GetString(0))).c_str() : ""), aId);
      }

      const urde::TAreaId aId = g_GameState->CurrentWorldState().GetCurrentAreaId();
      if (m_cvarCommons.m_debugOverlayAreaInfo->toBoolean() && g_StateManager->GetWorld() &&
          g_StateManager->GetWorld()->DoesAreaExist(aId)) {
        const auto& layerStates = g_GameState->CurrentWorldState().GetLayerState();
        std::string layerBits;
        u32 totalActive = 0;
        for (u32 i = 0; i < layerStates->GetAreaLayerCount(aId); ++i) {
          if (layerStates->IsLayerActive(aId, i)) {
            ++totalActive;
            layerBits += "1";
          } else
            layerBits += "0";
        }
        overlayText += fmt::format(FMT_STRING("Area AssetId: 0x{}, Total Objects: {}\n"
                                       "Active Layer bits: {}\n"),
                                   g_StateManager->GetWorld()->GetArea(aId)->GetAreaAssetId(),
                                   g_StateManager->GetAllObjectList().size(), layerBits);
      }
    }

    if (m_cvarCommons.m_debugOverlayShowRandomStats->toBoolean()) {
      overlayText += fmt::format(FMT_STRING("CRandom16::Next calls: {}\n"), urde::CRandom16::GetNumNextCalls());
    }

    if (m_cvarCommons.m_debugOverlayShowResourceStats->toBoolean())
      overlayText += fmt::format(FMT_STRING("Resource Objects: {}\n"), g_SimplePool->GetLiveObjects());

    if (!overlayText.empty())
      m_debugText->typesetGlyphs(overlayText);
  }
}

specter::View* ViewManager::BuildSpaceViews() {
  m_rootSpaceView = m_rootSpace->buildSpaceView(m_viewResources);
  return m_rootSpaceView;
}

specter::RootView* ViewManager::SetupRootView() {
  m_rootView.reset(new specter::RootView(*this, m_viewResources, m_mainWindow.get()));
  m_rootView->setBackground(zeus::skBlack);
  return m_rootView.get();
}

SplashScreen* ViewManager::SetupSplashView() {
  m_splash.reset(new SplashScreen(*this, m_viewResources));
  if (!m_showSplash)
    m_splash->close(true);
  return m_splash.get();
}

void ViewManager::RootSpaceViewBuilt(specter::View* view) {
  std::vector<specter::View*>& cViews = m_rootView->accessContentViews();
  cViews.clear();
  cViews.push_back(view);
  cViews.push_back(m_splash.get());
  m_rootView->updateSize();
}

void ViewManager::ProjectChanged(hecl::Database::Project& proj) {
  CDvdFile::Shutdown();
  // FIXME trilogy hack
  hecl::ProjectPath projectPath(proj.getProjectWorkingPath(), _SYS_STR("out/files/MP1"));
  if (!projectPath.isDirectory()) {
    projectPath = hecl::ProjectPath(proj.getProjectWorkingPath(), _SYS_STR("out/files"));
  }
  CDvdFile::Initialize(projectPath);
}

void ViewManager::SetupEditorView() {
  m_rootSpace.reset(new RootSpace(*this));

  SplitSpace* split = new SplitSpace(*this, nullptr, specter::SplitView::Axis::Horizontal);
  m_rootSpace->setChild(std::unique_ptr<Space>(split));
  split->setChildSlot(0, std::make_unique<ResourceBrowser>(*this, split));
  split->setChildSlot(1, std::make_unique<ResourceBrowser>(*this, split));

  BuildSpaceViews();
}

void ViewManager::SetupEditorView(ConfigReader& r) {
  m_rootSpace.reset(Space::NewRootSpaceFromConfigStream(*this, r));
  BuildSpaceViews();
}

void ViewManager::SaveEditorView(ConfigWriter& w) {
  if (!m_rootSpace)
    return;
  m_rootSpace->saveState(w);
}

void ViewManager::DismissSplash() {
  if (!m_showSplash)
    return;
  m_showSplash = false;
  m_splash->close();
}

ViewManager::ViewManager(hecl::Runtime::FileStoreManager& fileMgr, hecl::CVarManager& cvarMgr)
: m_fileStoreManager(fileMgr)
, m_cvarManager(cvarMgr)
, m_projManager(*this)
, m_fontCache(fileMgr)
, m_locale(locale::SystemLocaleOrEnglish())
, m_recentProjectsPath(fmt::format(FMT_STRING(_SYS_STR("{}/recent_projects.txt")), fileMgr.getStoreRoot()))
, m_recentFilesPath(fmt::format(FMT_STRING(_SYS_STR("{}/recent_files.txt")), fileMgr.getStoreRoot())) {
  Space::SpaceMenuNode::InitializeStrings(*this);
  char path[2048];
  hecl::Sstat theStat;

  auto fp = hecl::FopenUnique(m_recentProjectsPath.c_str(), _SYS_STR("r"), hecl::FileLockType::Read);
  if (fp) {
    while (std::fgets(path, std::size(path), fp.get())) {
      std::string pathStr(path);
      pathStr.pop_back();
      hecl::SystemStringConv pathStrView(pathStr);
      if (!hecl::Stat(pathStrView.c_str(), &theStat) && S_ISDIR(theStat.st_mode)) {
        m_recentProjects.emplace_back(pathStrView.sys_str());
      }
    }
  }

  fp = hecl::FopenUnique(m_recentFilesPath.c_str(), _SYS_STR("r"), hecl::FileLockType::Read);
  if (fp) {
    while (std::fgets(path, std::size(path), fp.get())) {
      std::string pathStr(path);
      pathStr.pop_back();
      hecl::SystemStringConv pathStrView(pathStr);
      if (!hecl::Stat(pathStrView.c_str(), &theStat) && S_ISDIR(theStat.st_mode)) {
        m_recentFiles.emplace_back(pathStrView.sys_str());
      }
    }
  }
}

ViewManager::~ViewManager() {}

void ViewManager::pushRecentProject(hecl::SystemStringView path) {
  for (hecl::SystemString& testPath : m_recentProjects) {
    if (path == testPath)
      return;
  }
  m_recentProjects.emplace_back(path);

  const auto fp = hecl::FopenUnique(m_recentProjectsPath.c_str(), _SYS_STR("w"), hecl::FileLockType::Write);
  if (fp == nullptr) {
    return;
  }

  for (const hecl::SystemString& pPath : m_recentProjects) {
    fmt::print(fp.get(), FMT_STRING("{}\n"), hecl::SystemUTF8Conv(pPath));
  }
}

void ViewManager::pushRecentFile(hecl::SystemStringView path) {
  for (hecl::SystemString& testPath : m_recentFiles) {
    if (path == testPath)
      return;
  }
  m_recentFiles.emplace_back(path);

  const auto fp = hecl::FopenUnique(m_recentFilesPath.c_str(), _SYS_STR("w"), hecl::FileLockType::Write);
  if (fp == nullptr) {
    return;
  }

  for (const hecl::SystemString& pPath : m_recentFiles) {
    fmt::print(fp.get(), FMT_STRING("{}\n"), hecl::SystemUTF8Conv(pPath));
  }
}

void ViewManager::init(boo::IApplication* app) {
  m_mainWindow = app->newWindow(_SYS_STR("URDE"));
  m_mainWindow->showWindow();
  m_mainWindow->setWaitCursor(true);

  float pixelFactor = m_mainWindow->getVirtualPixelFactor();

  m_mainBooFactory = m_mainWindow->getMainContextDataFactory();
  m_pipelineConv = hecl::NewPipelineConverter(m_mainBooFactory);
  hecl::conv = m_pipelineConv.get();
  m_mainPlatformName = m_mainBooFactory->platformName();
  m_mainWindow->setTitle(fmt::format(FMT_STRING(_SYS_STR("URDE [{}]")), m_mainPlatformName));
  m_mainCommandQueue = m_mainWindow->getCommandQueue();
  m_viewResources.init(m_mainBooFactory, &m_fontCache, &m_themeData, pixelFactor);
  InitializeIcons(m_viewResources);
  InitializeBadging(m_viewResources);
  m_viewResources.prepFontCacheAsync(m_mainWindow.get());
  specter::RootView* root = SetupRootView();
  m_showSplash = true;
  root->accessContentViews().push_back(SetupSplashView());
  root->updateSize();
  m_renderTex = root->renderTex();
  m_mainWindow->setWaitCursor(false);
  m_voiceEngine = boo::NewAudioVoiceEngine();
  m_voiceEngine->setVolume(0.7f);
  m_amuseAllocWrapper.emplace(*m_voiceEngine);

  for (const auto& arg : app->getArgs()) {
    if (m_deferedProject.empty() && hecl::SearchForProject(arg))
      m_deferedProject = arg;
    if (arg == _SYS_STR("--no-shader-warmup"))
      m_noShaderWarmup = true;
    else if (arg == _SYS_STR("--no-sound"))
      m_voiceEngine->setVolume(0.f);
  }

  if (m_deferedProject.empty()) {
    /* Default behavior - search upwards for packaged project containing the program */
    if (hecl::ProjectRootPath projRoot = hecl::SearchForProject(ExeDir)) {
      hecl::SystemString rootPath(projRoot.getAbsolutePath());
      hecl::Sstat theStat;
      if (!hecl::Stat((rootPath + _SYS_STR("/out/files/Metroid1.upak")).c_str(), &theStat) && S_ISREG(theStat.st_mode))
        m_deferedProject = rootPath + _SYS_STR("/out");
    }
  }
}

bool ViewManager::proc() {
  if (!m_deferedProject.empty() && m_viewResources.fontCacheReady()) {
    m_projManager.openProject(m_deferedProject);
    m_deferedProject.clear();
  }

  boo::IGraphicsCommandQueue* gfxQ = m_mainWindow->getCommandQueue();
  if (m_rootView->isDestroyed())
    return false;

  if (m_updatePf) {
    m_viewResources.resetPixelFactor(m_reqPf);
    specter::RootView* root = SetupRootView();
    if (m_rootSpace)
      BuildSpaceViews();
    else {
      std::vector<specter::View*>& cViews = m_rootView->accessContentViews();
      cViews.push_back(SetupSplashView());
    }
    root->updateSize();
    m_updatePf = false;
  }

  m_rootView->dispatchEvents();
  m_rootView->internalThink();
  if (m_rootSpace)
    m_rootSpace->think();
  if (m_splash)
    m_splash->think();

  if (m_deferSplit) {
    SplitSpace* ss = static_cast<SplitSpace*>(m_deferSplit->spaceSplit(m_deferSplitAxis, m_deferSplitThisSlot));
    m_rootView->startSplitDrag(ss->splitView(), m_deferSplitCoord);
    m_deferSplit = nullptr;
  }

  ++m_editorFrames;
  if (m_rootSpaceView && m_editorFrames <= 30)
    m_rootSpaceView->setMultiplyColor(zeus::CColor::lerp({1, 1, 1, 0}, {1, 1, 1, 1}, m_editorFrames / 30.0));

  m_cvarManager.proc();
  m_projManager.mainUpdate();

  if (m_testGameView)
    m_testGameView->think();

  if (g_Renderer)
    g_Renderer->BeginScene();
  m_rootView->draw(gfxQ);
  if (g_Renderer)
    g_Renderer->EndScene();
  gfxQ->execute();
  if (g_ResFactory)
    g_ResFactory->AsyncIdle();
#ifndef URDE_MSAN
  m_voiceEngine->pumpAndMixVoices();
#endif
  if (!m_skipWait || !hecl::com_developer->toBoolean())
    m_mainWindow->waitForRetrace();
  CBooModel::ClearModelUniformCounters();
  CGraphics::TickRenderTimings();
  ++logvisor::FrameIndex;
  return true;
}

void ViewManager::stop() {
  m_mainWindow->getCommandQueue()->stopRenderer();
  m_projManager.shutdown();
  CDvdFile::Shutdown();
  DestroyIcons();
  DestroyBadging();
  m_viewResources.destroyResData();
  m_fontCache.destroyAtlases();
}

} // namespace urde
