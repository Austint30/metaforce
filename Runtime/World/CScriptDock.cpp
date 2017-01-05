#include "CScriptDock.hpp"
#include "CActorParameters.hpp"
#include "Character/CModelData.hpp"
#include "Collision/CMaterialList.hpp"
#include "CWorld.hpp"
#include "CStateManager.hpp"
#include "CScriptDoor.hpp"
#include "CPlayer.hpp"

namespace urde
{
CMaterialList MakeDockMaterialList()
{
    CMaterialList list;
    list.Add(EMaterialTypes::Trigger);
    list.Add(EMaterialTypes::Immovable);
    list.Add(EMaterialTypes::AIBlock);
    return list;
}

CScriptDock::CScriptDock(TUniqueId uid, const std::string& name, const CEntityInfo& info,
                         const zeus::CVector3f position, const zeus::CVector3f& extents, s32 dock, TAreaId area,
                         bool active, s32 dockReferenceCount, bool loadConnected)
: CPhysicsActor(uid, active, name, info, zeus::CTransform(zeus::CMatrix3f::skIdentityMatrix3f, position),
                CModelData::CModelDataNull(), MakeDockMaterialList(), zeus::CAABox(-extents * 0.5f, extents * 0.5f),
                SMoverData(1.f), CActorParameters::None(), 0.3f, 0.1f)
, x258_dockReferenceCount(dockReferenceCount)
, x25c_dock(dock)
, x260_area(area)
, x268_25_loadConnected(loadConnected)
{
}

void CScriptDock::Think(float dt, CStateManager& mgr)
{
    if (!GetActive())
    {
        UpdateAreaActivateFlags(mgr);
        x268_24_dockReferenced = false;
    }

    if (x268_26_areaPostConstructed != mgr.WorldNC()->GetArea(x260_area)->IsPostConstructed())
    {
        if (mgr.WorldNC()->GetArea(x260_area)->IsPostConstructed())
            CEntity::SendScriptMsgs(EScriptObjectState::MaxReached, mgr, EScriptObjectMessage::None);
        else
            CEntity::SendScriptMsgs(EScriptObjectState::Zero, mgr, EScriptObjectMessage::None);
    }

    if (mgr.GetNextAreaId() != x260_area)
        x264_dockState = EDockState::Three;
    else if (x264_dockState == EDockState::Three)
        x264_dockState = EDockState::Idle;
    else if (x264_dockState == EDockState::PlayerTouched)
        x264_dockState = EDockState::EnterNextArea;
    else if (x264_dockState == EDockState::EnterNextArea)
    {
        CPlayer& player = mgr.GetPlayer();
        if (HasPointCrossedDock(mgr, player.GetTransform().origin))
        {
            IGameArea::Dock* dock = mgr.WorldNC()->GetArea(mgr.GetNextAreaId())->DockNC(x25c_dock);
            TAreaId aid = dock->GetConnectedAreaId(dock->GetReferenceCount());
            if (aid != kInvalidAreaId && mgr.WorldNC()->GetArea(aid)->GetActive())
            {
                mgr.SetCurrentAreaId(aid);
                s32 otherDock = dock->GetOtherDockNumber(dock->GetReferenceCount());

                CObjectList& objs = mgr.WorldNC()->GetArea(aid)->GetAreaObjects();
                for (CEntity* ent : objs)
                {
                    CScriptDock* dock = static_cast<CScriptDock*>(ent);
                    if (dock && dock->GetDockId() == otherDock)
                        dock->SetLoadConnected(mgr, true);
                }
            }
        }

        x264_dockState = EDockState::Idle;
    }
}

void CScriptDock::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr)
{
    switch (msg)
    {
    case EScriptObjectMessage::InternalMessage11:
    {
        CGameArea* area = mgr.WorldNC()->GetArea(x260_area);
        if (area->GetDockCount() <= x25c_dock)
            return;
        IGameArea::Dock* dock = area->DockNC(x25c_dock);
        if (!dock->IsReferenced())
            dock->SetReferenceCount(x258_dockReferenceCount);
    }
    break;
    case EScriptObjectMessage::InternalMessage12:
        CleanUp();
        break;
    case EScriptObjectMessage::InternalMessage13:
        AreaLoaded(mgr);
        break;
    case EScriptObjectMessage::InternalMessage14:
    {
        UpdateAreaActivateFlags(mgr);
        CMaterialList exclude = GetMaterialFilter().GetExcludeList();
        CMaterialList include = GetMaterialFilter().GetIncludeList();
        include.Add(EMaterialTypes::AIBlock);
        SetMaterialFilter({include, exclude, CMaterialFilter::EFilterType::Three});
    }
    break;
    case EScriptObjectMessage::SetToZero:
    {
        if (mgr.GetNextAreaId() != x260_area)
            return;

        SetLoadConnected(mgr, false);

        CGameArea* area = mgr.WorldNC()->GetArea(x260_area);
        IGameArea::Dock* dock = area->DockNC(x25c_dock);

        TAreaId aid = dock->GetConnectedAreaId(dock->GetReferenceCount());

        CPlatformAndDoorList& lst = mgr.GetPlatformAndDoorObjectList();
        for (CEntity* ent : lst)
        {
            CScriptDoor* door = static_cast<CScriptDoor*>(ent);
            if (door && !door->IsConnectedToArea(mgr, aid))
                door->ForceClosed(mgr);
        }
    }
    break;
    case EScriptObjectMessage::SetToMax:
        if (mgr.GetNextAreaId() != x260_area)
            return;

        SetLoadConnected(mgr, true);
        break;
    case EScriptObjectMessage::Increment:
        SetLoadConnected(mgr, true);
    case EScriptObjectMessage::Decrement:
    {
        TAreaId aid = x260_area;
        if (mgr.GetNextAreaId() == x260_area)
        {
            IGameArea::Dock* dock = mgr.WorldNC()->GetArea(x260_area)->DockNC(x25c_dock);
            aid = dock->GetConnectedAreaId(dock->GetReferenceCount());
        }
        else if (aid == 0 || (mgr.GetWorld()->GetNumAreas() <= aid || !mgr.WorldNC()->GetArea(aid)->GetActive()))
            return;
#if 0
        /* Propogate through area chain */
        sub800C40DC((msg == EScriptObjectMessage::Increment), mgr.GetWorld()->GetAreaAlways(aid), mgr.WorldNC());
#endif
    }
    break;
    default:
        CPhysicsActor::AcceptScriptMsg(msg, uid, mgr);
        break;
    }
}

rstl::optional_object<zeus::CAABox> CScriptDock::GetTouchBounds() const
{
    if (x264_dockState == EDockState::Three)
        return {};

    return GetBoundingBox();
}

void CScriptDock::Touch(CActor& act, CStateManager&)
{
    if (x264_dockState == EDockState::Three)
        return;

    if (static_cast<CPlayer*>(&act) != nullptr)
        x264_dockState = EDockState::PlayerTouched;
}

zeus::CPlane CScriptDock::GetPlane(const CStateManager& mgr) const
{
    const IGameArea::Dock* dock = mgr.GetWorld()->GetAreaAlways(x260_area)->GetDock(x25c_dock);

    return zeus::CPlane(dock->GetPoint(0), dock->GetPoint(1), dock->GetPoint(2));
}

void CScriptDock::SetDockReference(CStateManager& mgr, s32 ref)
{
    mgr.WorldNC()->GetArea(x260_area)->DockNC(x25c_dock)->SetReferenceCount(ref);
    x268_24_dockReferenced = true;
}

s32 CScriptDock::GetDockReference(CStateManager& mgr) const
{
    return mgr.GetWorld()->GetAreaAlways(x260_area)->GetDock(x25c_dock)->GetReferenceCount();
}

TAreaId CScriptDock::GetCurrentConnectedAreaId(const CStateManager& mgr) const
{
    if (mgr.GetWorld()->GetNumAreas() < x260_area)
        return kInvalidAreaId;
    const CGameArea* area = mgr.GetWorld()->GetAreaAlways(x260_area);
    if (area->GetDockCount() < x25c_dock)
        return kInvalidAreaId;

    const IGameArea::Dock* dock = area->GetDock(x25c_dock);
    return dock->GetConnectedAreaId(dock->GetReferenceCount());
}

void CScriptDock::UpdateAreaActivateFlags(CStateManager& mgr)
{
    CWorld* world = mgr.WorldNC();
    if (x260_area >= world->GetNumAreas())
        return;

    CGameArea* area = world->GetArea(x260_area);

    if (x25c_dock >= area->GetDockCount())
        return;

    IGameArea::Dock* dock = area->DockNC(x25c_dock);

    for (s32 i = 0; i < dock->GetDockRefs().size(); ++i)
    {
        s32 dockRefCount = dock->GetReferenceCount();
        TAreaId aid = dock->GetConnectedAreaId(i);
        if (aid != kInvalidAreaId)
            world->GetArea(aid)->SetActive(i == dockRefCount);
    }
    mgr.SetCurrentAreaId(mgr.GetNextAreaId());
}

bool CScriptDock::HasPointCrossedDock(const CStateManager& mgr, const zeus::CVector3f& point) const
{
    const zeus::CPlane plane = GetPlane(mgr);

    return (plane.vec.dot(point) >= plane.d);
}

void CScriptDock::AreaLoaded(CStateManager& mgr) { SetLoadConnected(mgr, x268_25_loadConnected); }

void CScriptDock::SetLoadConnected(CStateManager& mgr, bool loadOther)
{
    IGameArea::Dock* dock = mgr.WorldNC()->GetArea(x260_area)->DockNC(x25c_dock);
    bool cur = dock->GetShouldLoadOther(dock->GetReferenceCount());
    if (cur == loadOther)
        return;

    dock->SetShouldLoadOther(dock->GetReferenceCount(), loadOther);
}
}
