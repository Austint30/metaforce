#pragma once

#include "hecl/CVarManager.hpp"
#include "hecl/CVarCommons.hpp"
#include "boo/audiodev/IAudioVoiceEngine.hpp"
#include "amuse/BooBackend.hpp"
#include "ProjectManager.hpp"
#include "Space.hpp"
#include "specter/IViewManager.hpp"
#include "specter/FontCache.hpp"
#include "specter/ViewResources.hpp"

#include "Runtime/CGameHintInfo.hpp"
#include "Runtime/Particle/CElementGen.hpp"
#include "Runtime/Graphics/CLineRenderer.hpp"
#include "Runtime/Graphics/CMoviePlayer.hpp"
#include "Runtime/Graphics/CModel.hpp"
#include "Runtime/Particle/CGenDescription.hpp"
#include "Runtime/Character/CAssetFactory.hpp"
#include "Runtime/Graphics/Shaders/CColoredQuadFilter.hpp"
#include "Runtime/Graphics/Shaders/CXRayBlurFilter.hpp"
#include "Runtime/Graphics/Shaders/CCameraBlurFilter.hpp"
#include "Runtime/Audio/CStaticAudioPlayer.hpp"

namespace hecl {
class PipelineConverterBase;
}

namespace urde {
class SplashScreen;

class ViewManager final : public specter::IViewManager {
  friend class ProjectManager;
  friend class Space;
  friend class RootSpace;
  friend class SplitSpace;

  std::shared_ptr<boo::IWindow> m_mainWindow;
  hecl::Runtime::FileStoreManager& m_fileStoreManager;
  hecl::CVarManager& m_cvarManager;
  ProjectManager m_projManager;
  specter::FontCache m_fontCache;
  specter::DefaultThemeData m_themeData;
  specter::ViewResources m_viewResources;
  locale::ELocale m_locale = locale::ELocale::en_US;
  boo::IGraphicsDataFactory* m_mainBooFactory = nullptr;
  boo::IGraphicsCommandQueue* m_mainCommandQueue = nullptr;
  std::unique_ptr<hecl::PipelineConverterBase> m_pipelineConv;
  boo::ObjToken<boo::ITextureR> m_renderTex;
  const boo::SystemChar* m_mainPlatformName;

  std::unique_ptr<specter::RootView> m_rootView;
  std::unique_ptr<SplashScreen> m_splash;
  std::unique_ptr<RootSpace> m_rootSpace;
  specter::View* m_rootSpaceView = nullptr;
  bool m_skipWait = false;

  class TestGameView : public specter::View {
    ViewManager& m_vm;
    std::unique_ptr<specter::MultiLineTextView> m_debugText;
    hecl::CVarCommons m_cvarCommons;

  public:
    TestGameView(ViewManager& vm, specter::ViewResources& res, specter::View& parent, hecl::CVarManager& cvarMgr)
    : View(res, parent), m_vm(vm), m_cvarCommons(cvarMgr) {}
    void resized(const boo::SWindowRect& root, const boo::SWindowRect& sub) override;
    void draw(boo::IGraphicsCommandQueue* gfxQ) override;
    void think() override;

    void mouseDown(const boo::SWindowCoord& coord, boo::EMouseButton button, boo::EModifierKey mkey) override {
      if (MP1::CMain* m = m_vm.m_projManager.gameMain())
        if (MP1::CGameArchitectureSupport* as = m->GetArchSupport())
          as->mouseDown(coord, button, mkey);
    }

    void mouseUp(const boo::SWindowCoord& coord, boo::EMouseButton button, boo::EModifierKey mkey) override {
      if (MP1::CMain* m = m_vm.m_projManager.gameMain())
        if (MP1::CGameArchitectureSupport* as = m->GetArchSupport())
          as->mouseUp(coord, button, mkey);
    }

    void mouseMove(const boo::SWindowCoord& coord) override {
      if (MP1::CMain* m = m_vm.m_projManager.gameMain())
        if (MP1::CGameArchitectureSupport* as = m->GetArchSupport())
          as->mouseMove(coord);
    }

    void scroll(const boo::SWindowCoord& coord, const boo::SScrollDelta& sd) override {
      if (MP1::CMain* m = m_vm.m_projManager.gameMain())
        if (MP1::CGameArchitectureSupport* as = m->GetArchSupport())
          as->scroll(coord, sd);
    }

    void charKeyDown(unsigned long cc, boo::EModifierKey mkey, bool repeat) override {
      if (MP1::CMain* m = m_vm.m_projManager.gameMain())
        if (MP1::CGameArchitectureSupport* as = m->GetArchSupport())
          as->charKeyDown(cc, mkey, repeat);
    }

    void charKeyUp(unsigned long cc, boo::EModifierKey mkey) override {
      if (MP1::CMain* m = m_vm.m_projManager.gameMain())
        if (MP1::CGameArchitectureSupport* as = m->GetArchSupport())
          as->charKeyUp(cc, mkey);
    }

    void specialKeyDown(boo::ESpecialKey skey, boo::EModifierKey mkey, bool repeat) override {
      if (MP1::CMain* m = m_vm.m_projManager.gameMain())
        if (MP1::CGameArchitectureSupport* as = m->GetArchSupport())
          as->specialKeyDown(skey, mkey, repeat);

      if (skey == boo::ESpecialKey::F1)
        m_vm.m_skipWait = true;
    }

    void specialKeyUp(boo::ESpecialKey skey, boo::EModifierKey mkey) override {
      if (MP1::CMain* m = m_vm.m_projManager.gameMain())
        if (MP1::CGameArchitectureSupport* as = m->GetArchSupport())
          as->specialKeyUp(skey, mkey);

      if (skey == boo::ESpecialKey::F1)
        m_vm.m_skipWait = false;
    }
  };
  std::unique_ptr<TestGameView> m_testGameView;
  std::unique_ptr<boo::IAudioVoiceEngine> m_voiceEngine;
  std::optional<amuse::BooBackendVoiceAllocator> m_amuseAllocWrapper;

  hecl::SystemString m_recentProjectsPath;
  std::vector<hecl::SystemString> m_recentProjects;
  hecl::SystemString m_recentFilesPath;
  std::vector<hecl::SystemString> m_recentFiles;

  bool m_updatePf = false;
  float m_reqPf;

  specter::View* BuildSpaceViews();
  specter::RootView* SetupRootView();
  SplashScreen* SetupSplashView();
  void RootSpaceViewBuilt(specter::View* view);
  void ProjectChanged(hecl::Database::Project& proj);
  void SetupEditorView();
  void SetupEditorView(ConfigReader& r);
  void SaveEditorView(ConfigWriter& w);

  bool m_showSplash = false;
  void DismissSplash();

  unsigned m_editorFrames = 120;
  void FadeInEditors() { m_editorFrames = 0; }

  void InitMP1(MP1::CMain& main);

  Space* m_deferSplit = nullptr;
  specter::SplitView::Axis m_deferSplitAxis;
  int m_deferSplitThisSlot;
  boo::SWindowCoord m_deferSplitCoord;
  hecl::SystemString m_deferedProject;
  bool m_noShaderWarmup = false;

public:
  ViewManager(hecl::Runtime::FileStoreManager& fileMgr, hecl::CVarManager& cvarMgr);
  ~ViewManager();

  specter::RootView& rootView() const { return *m_rootView; }
  void requestPixelFactor(float pf) {
    m_reqPf = pf;
    m_updatePf = true;
  }

  ProjectManager& projectManager() { return m_projManager; }
  hecl::Database::Project* project() { return m_projManager.project(); }
  locale::ELocale getTranslatorLocale() const override { return m_locale; }

  void deferSpaceSplit(specter::ISpaceController* split, specter::SplitView::Axis axis, int thisSlot,
                       const boo::SWindowCoord& coord) override {
    m_deferSplit = static_cast<Space*>(split);
    m_deferSplitAxis = axis;
    m_deferSplitThisSlot = thisSlot;
    m_deferSplitCoord = coord;
  }

  const std::vector<hecl::SystemString>* recentProjects() const override { return &m_recentProjects; }
  void pushRecentProject(hecl::SystemStringView path) override;

  const std::vector<hecl::SystemString>* recentFiles() const override { return &m_recentFiles; }
  void pushRecentFile(hecl::SystemStringView path) override;

  void init(boo::IApplication* app);
  const boo::SystemChar* platformName() { return m_mainPlatformName; }
  bool proc();
  void stop();

  void deferOpenProject(const hecl::SystemString& path) { m_deferedProject = path; }
};

} // namespace urde
