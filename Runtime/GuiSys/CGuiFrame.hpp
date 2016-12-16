#ifndef __URDE_CGUIFRAME_HPP__
#define __URDE_CGUIFRAME_HPP__

#include "CGuiWidget.hpp"
#include "CGuiWidgetIdDB.hpp"
#include "IObj.hpp"

namespace urde
{
class CGuiSys;
class CLight;
class CGuiCamera;
class CGuiHeadWidget;
class CFinalInput;
class CGuiLight;
class CVParamTransfer;
class CObjectReference;
class CSimplePool;

class CGuiFrame
{
private:
    ResId x0_id;
    u32 x4_ = 0;
    CGuiSys& x8_guiSys;
    CGuiHeadWidget* xc_headWidget = nullptr;
    std::unique_ptr<CGuiWidget> x10_rootWidget;
    CGuiCamera* x14_camera = nullptr;
    CGuiWidgetIdDB x18_idDB;
    std::vector<CGuiWidget*> x2c_widgets;
    std::vector<CGuiLight*> x3c_lights;
    int x4c_a;
    int x50_b;
    int x54_c;
    bool x58_24_loaded : 1;

public:
    CGuiFrame(ResId id, CGuiSys& sys, int a, int b, int c, CSimplePool* sp);

    CGuiSys& GetGuiSys() {return x8_guiSys;}

    CGuiLight* GetFrameLight(int idx) {return x3c_lights[idx];}
    CGuiWidget* FindWidget(const std::string& name) const;
    CGuiWidget* FindWidget(s16 id) const;
    void SetFrameCamera(CGuiCamera* camr) {x14_camera = camr;}
    void SetHeadWidget(CGuiHeadWidget* hwig) {xc_headWidget = hwig;}
    void SortDrawOrder();
    void EnableLights(u32 lights) const;
    void DisableLights() const;
    void RemoveLight(CGuiLight* light);
    void AddLight(CGuiLight* light);
    bool GetIsFinishedLoading() const;
    void Touch() const;

    void Update(float dt);
    void Draw(const CGuiWidgetDrawParms& parms) const;
    void Initialize();
    void LoadWidgetsInGame(CInputStream& in);
    void ProcessUserInput(const CFinalInput& input) const;

    CGuiWidgetIdDB& GetWidgetIdDB() {return x18_idDB;}

    static CGuiFrame* CreateFrame(ResId frmeId, CGuiSys& sys, CInputStream& in, CSimplePool* sp);
};

std::unique_ptr<IObj> RGuiFrameFactoryInGame(const SObjectTag& tag, CInputStream& in,
                                             const CVParamTransfer& vparms,
                                             CObjectReference* selfRef);

}

#endif // __URDE_CGUIFRAME_HPP__
