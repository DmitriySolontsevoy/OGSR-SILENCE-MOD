#include "StdAfx.h"
#include "CustomDetector.h"
#include "ui/ArtefactDetectorUI.h"
#include "HUDManager.h"
#include "Inventory.h"
#include "Level.h"
#include "map_manager.h"
#include "ActorEffector.h"
#include "Actor.h"
#include "player_hud.h"
#include "Weapon.h"

ITEM_INFO::~ITEM_INFO()
{
    if (pParticle)
        CParticlesObject::Destroy(pParticle);
}

bool CCustomDetector::Activate(bool now)
{
    inherited::Activate(now);
    ToggleDetector(false);
    return true;
}

void CCustomDetector::Deactivate(bool now)
{
    inherited::Deactivate(now);
    ToggleDetector(false);
}

void CCustomDetector::HideDetector(bool bFastMode)
{
    if (GetState() == eIdle)
        ToggleDetector(bFastMode);
}

void CCustomDetector::ShowDetector(bool bFastMode)
{
    if (GetState() == eHidden)
        ToggleDetector(bFastMode);
}

void CCustomDetector::ToggleDetector(bool bFastMode)
{
    m_bNeedActivation = false;
    m_bFastAnimMode = bFastMode;

    if (GetState() == eHidden)
    {
        CActor* pActor = smart_cast<CActor*>(H_Parent());
        u16 slot_to_activate = NO_ACTIVE_SLOT;

        if (slot_to_activate != NO_ACTIVE_SLOT)
        {
            pActor->inventory().Activate(slot_to_activate);
            m_bNeedActivation = true;
        }
        else
        {
            SwitchState(eShowing);
            TurnDetectorInternal(true);
        }
    }
    else if (GetState() == eIdle)
        SwitchState(eHiding);
}

void CCustomDetector::OnStateSwitch(u32 S, u32 oldState)
{
    inherited::OnStateSwitch(S, oldState);

    switch (S)
    {
    case eShowing:
    {
        g_player_hud->attach_item(this);
        HUD_SOUND::PlaySound(sndShow, Fvector{}, this, !!GetHUDmode(), false, false);
        PlayHUDMotion({ m_bFastAnimMode ? "anim_show_fast" : "anim_show" }, false, GetState());
        SetPending(TRUE);
    }
    break;
    case eHiding:
    {
        if (oldState != eHiding)
        {
            HUD_SOUND::PlaySound(sndHide, Fvector{}, this, !!GetHUDmode(), false, false);
            PlayHUDMotion({ m_bFastAnimMode ? "anim_hide_fast" : "anim_hide" }, false, GetState());
            SetPending(TRUE);
        }
    }
    break;
    case eIdle:
    {
        PlayAnimIdle();
        SetPending(FALSE);
    }
    break;
    }
}

void CCustomDetector::OnAnimationEnd(u32 state)
{
    inherited::OnAnimationEnd(state);
    switch (state)
    {
    case eShowing:
    {
        SwitchState(eIdle);
        if (m_fDecayRate > 0.f)
            this->SetCondition(-m_fDecayRate);
    }
    break;
    case eHiding:
    {
        SwitchState(eHidden);
        TurnDetectorInternal(false);
        g_player_hud->detach_item(this);
    }
    break;
    }
}

void CCustomDetector::UpdateXForm() { CInventoryItem::UpdateXForm(); }

void CCustomDetector::OnActiveItem() {}

void CCustomDetector::OnHiddenItem() {}

CCustomDetector::~CCustomDetector()
{
    HUD_SOUND::DestroySound(sndShow);
    HUD_SOUND::DestroySound(sndHide);

    m_artefacts.destroy();
    TurnDetectorInternal(false);
    xr_delete(m_ui);
}

BOOL CCustomDetector::net_Spawn(CSE_Abstract* DC)
{
    TurnDetectorInternal(false);
    return inherited::net_Spawn(DC);
}

void CCustomDetector::Load(LPCSTR section)
{
    m_animation_slot = 7;
    inherited::Load(section);

    m_fAfDetectRadius = READ_IF_EXISTS(pSettings, r_float, section, "af_radius", 30.0f);
    m_fAfVisRadius = READ_IF_EXISTS(pSettings, r_float, section, "af_vis_radius", 2.0f);
    m_fDecayRate = READ_IF_EXISTS(pSettings, r_float, section, "decay_rate", 0.f); //Alundaio
    m_artefacts.load(section, "af");

    HUD_SOUND::LoadSound(section, "snd_draw", sndShow, SOUND_TYPE_ITEM_TAKING);
    HUD_SOUND::LoadSound(section, "snd_holster", sndHide, SOUND_TYPE_ITEM_HIDING);
}

void CCustomDetector::shedule_Update(u32 dt)
{
    inherited::shedule_Update(dt);

    if (!IsWorking())
        return;

    Position().set(H_Parent()->Position());

    Fvector P;
    P.set(H_Parent()->Position());

    if (GetCondition() <= 0.01f)
        return;

    m_artefacts.feel_touch_update(P, m_fAfDetectRadius);
}

bool CCustomDetector::IsWorking() const { return m_bWorking && H_Parent() && H_Parent() == Level().CurrentViewEntity(); }

void CCustomDetector::UpdateWork()
{
    UpdateAf();
    m_ui->update();
}

void CCustomDetector::UpdateVisibility()
{
    if (HudItemData())
    {
        bool bClimb = ((Actor()->MovingState() & mcClimb) != 0);
        if (bClimb)
        {
            HideDetector(true);
            m_bNeedActivation = true;
        }
    }
    else if (m_bNeedActivation)
    {
        bool bClimb = ((Actor()->MovingState() & mcClimb) != 0);
        if (!bClimb)
        {
            ShowDetector(true);
        }
    }
}

void CCustomDetector::UpdateCL()
{
    inherited::UpdateCL();

    if (H_Parent() != Level().CurrentEntity())
        return;

    UpdateVisibility();
    if (!IsWorking())
        return;
        
    UpdateWork();
}

void CCustomDetector::OnH_A_Chield() { inherited::OnH_A_Chield(); }

void CCustomDetector::OnH_B_Independent(bool just_before_destroy)
{
    inherited::OnH_B_Independent(just_before_destroy);

    m_artefacts.clear();

    if (GetState() != eHidden)
    {
        TurnDetectorInternal(false);
        SwitchState(eHidden);
    }
}

void CCustomDetector::OnMoveToRuck(EItemPlace prevPlace)
{
    inherited::OnMoveToRuck(prevPlace);
    if (prevPlace == eItemPlaceSlot)
    {
        SwitchState(eHidden);
        g_player_hud->detach_item(this);
    }
    TurnDetectorInternal(false);
    StopCurrentAnimWithoutCallback();
}

void CCustomDetector::OnMoveToSlot() { inherited::OnMoveToSlot(); }

void CCustomDetector::TurnDetectorInternal(bool b)
{
    m_bWorking = b;
    if (b && !m_ui)
        CreateUI();

    //UpdateNightVisionMode(b);
}

//void CCustomDetector::UpdateNightVisionMode(bool b_on) {}



BOOL CAfList::feel_touch_contact(CObject* O)
{
    auto pAf = smart_cast<CArtefact*>(O);
    if (!pAf)
        return false;

    bool res = (m_TypesMap.find(O->cNameSect()) != m_TypesMap.end()) || (m_TypesMap.find("class_all") != m_TypesMap.end());
    if (res)
        if (pAf->GetAfRank() > m_af_rank)
            res = false;

    return res;
}



BOOL CZoneList::feel_touch_contact(CObject* O)
{
    auto pZone = smart_cast<CCustomZone*>(O);
    if (!pZone)
        return false;

    bool res = (m_TypesMap.find(O->cNameSect()) != m_TypesMap.end()) || (m_TypesMap.find("class_all") != m_TypesMap.end());
    if (!pZone->IsEnabled())
        res = false;

    return res;
}

CZoneList::~CZoneList()
{
    clear();
    destroy();
}
