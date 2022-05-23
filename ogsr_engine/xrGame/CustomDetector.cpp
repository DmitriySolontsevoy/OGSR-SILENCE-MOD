#include "stdafx.h"
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
#include "actorcondition.h"
#include "clsid_game.h"
#include "game_base_space.h"

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
        SwitchState(eShowing);
        TurnDetectorInternal(true);
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
    m_sndWorking = false;

    HUD_SOUND::DestroySound(sndShow);
    HUD_SOUND::DestroySound(sndHide);

    m_artefacts.destroy();
    TurnDetectorInternal(false);

    ZONE_TYPE_MAP_IT it;
    for (it = m_ZoneTypeMap.begin(); m_ZoneTypeMap.end() != it; ++it)
        HUD_SOUND::DestroySound(it->second.detect_snds);

    m_ZoneInfoMap.clear();

    xr_delete(m_ui);
}

BOOL CCustomDetector::net_Spawn(CSE_Abstract* DC)
{
    m_pCurrentActor = NULL;
    m_pCurrentInvOwner = NULL;

    TurnDetectorInternal(false);
    return inherited::net_Spawn(DC);
}

void CCustomDetector::Load(LPCSTR section)
{
    m_animation_slot = 7;
    inherited::Load(section);

    m_fAfDetectRadius = READ_IF_EXISTS(pSettings, r_float, section, "af_radius", 30.0f);
    m_fAfVisRadius = READ_IF_EXISTS(pSettings, r_float, section, "af_vis_radius", 2.0f);
    m_fDecayRate = READ_IF_EXISTS(pSettings, r_float, section, "decay_rate", 0.f);
    m_artefacts.load(section, "af");

    HUD_SOUND::LoadSound(section, "snd_draw", sndShow, SOUND_TYPE_ITEM_TAKING);
    HUD_SOUND::LoadSound(section, "snd_holster", sndHide, SOUND_TYPE_ITEM_HIDING);

    m_fRadius = pSettings->r_float(section, "radius");

    if (pSettings->line_exist(section, "night_vision_particle"))
        m_nightvision_particle = pSettings->r_string(section, "night_vision_particle");

    u32 i = 1;
    string256 temp;

    do
    {
        sprintf_s(temp, "snd_zone_class_%d", i);
        if (pSettings->line_exist(section, temp))
        {
            LPCSTR z_Class = pSettings->r_string(section, temp);
            CLASS_ID zone_cls = TEXT2CLSID(pSettings->r_string(z_Class, "class"));

            m_ZoneTypeMap.insert(std::make_pair(zone_cls, ZONE_TYPE_SHOC()));
            ZONE_TYPE_SHOC& zone_type = m_ZoneTypeMap[zone_cls];
            sprintf_s(temp, "zone_min_freq_%d", i);
            zone_type.min_freq = pSettings->r_float(section, temp);
            sprintf_s(temp, "zone_max_freq_%d", i);
            zone_type.max_freq = pSettings->r_float(section, temp);
            R_ASSERT(zone_type.min_freq < zone_type.max_freq);
            sprintf_s(temp, "zone_sound_%d_", i);

            HUD_SOUND::LoadSound(section, temp, zone_type.detect_snds, SOUND_TYPE_ITEM);

            sprintf_s(temp, "zone_map_location_%d", i);

            if (pSettings->line_exist(section, temp))
                zone_type.zone_map_location = pSettings->r_string(section, temp);

            sprintf_s(temp, "zone_radius_%d", i);
            zone_type.m_fRadius = READ_IF_EXISTS(pSettings, r_float, section, temp, m_fRadius);

            ++i;
        }
        else break;
    } while (true);

    m_ef_detector_type = pSettings->r_u32(section, "ef_detector_type");
    m_detect_actor_radiation = READ_IF_EXISTS(pSettings, r_bool, section, "detect_actor_radiation", false);
}

void CCustomDetector::shedule_Update(u32 dt)
{
    inherited::shedule_Update(dt);

    if (H_Parent() && H_Parent() == Level().CurrentViewEntity() && m_sndWorking)
    {
        Position().set(H_Parent()->Position());

        Fvector	P;
        P.set(H_Parent()->Position());
        feel_touch_update(P, m_fRadius);
        UpdateNightVisionMode();
    }

    if (IsWorking())
    {
        Position().set(H_Parent()->Position());

        Fvector P;
        P.set(H_Parent()->Position());

        if (GetCondition() <= 0.01f)
            return;

        m_artefacts.feel_touch_update(P, m_fAfDetectRadius);
    }
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

    if (m_sndWorking && H_Parent())
    {
        ZONE_INFO_MAP_IT it;
        for (it = m_ZoneInfoMap.begin(); m_ZoneInfoMap.end() != it; ++it)
        {
            CCustomZone* pZone = it->first;
            ZONE_INFO_SHOC& zone_info = it->second;

            if (m_ZoneTypeMap.find(pZone->CLS_ID) == m_ZoneTypeMap.end() ||
                !pZone->VisibleByDetector())
                continue;

            ZONE_TYPE_SHOC& zone_type = m_ZoneTypeMap[pZone->CLS_ID];

            CSpaceRestrictor* pSR = smart_cast<CSpaceRestrictor*>(pZone);
            float dist_to_zone = pSR->distance_to(H_Parent()->Position());
            if (dist_to_zone > zone_type.m_fRadius)
                continue;
            if (dist_to_zone < 0) dist_to_zone = 0;

            float fRelPow = 1.f - dist_to_zone / zone_type.m_fRadius;
            clamp(fRelPow, 0.f, 1.f);

            zone_info.cur_freq = zone_type.min_freq +
                (zone_type.max_freq - zone_type.min_freq) * fRelPow * fRelPow * fRelPow * fRelPow;

            float current_snd_time = 1000.f * 1.f / zone_info.cur_freq;

            if ((float)zone_info.snd_time > current_snd_time)
            {
                zone_info.snd_time = 0;
                HUD_SOUND::PlaySound(zone_type.detect_snds, Fvector().set(0, 0, 0), this, true, false);
            }
            else
                zone_info.snd_time += Device.dwTimeDelta;
        }

        update_actor_radiation();
    }

    if (H_Parent() != Level().CurrentEntity())
        return;

    UpdateVisibility();
    if (!IsWorking())
        return;
        
    UpdateWork();
}

void CCustomDetector::OnH_A_Chield()
{
    inherited::OnH_A_Chield();
}

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

void CCustomDetector::feel_touch_new(CObject* O)
{
    CCustomZone* pZone = smart_cast<CCustomZone*>(O);
    if (pZone && pZone->IsEnabled())
    {
        m_ZoneInfoMap[pZone].snd_time = 0;

        AddRemoveMapSpot(pZone, true);
    }
}

void CCustomDetector::feel_touch_delete(CObject* O)
{
    CCustomZone* pZone = smart_cast<CCustomZone*>(O);
    if (pZone)
    {
        m_ZoneInfoMap.erase(pZone);
        AddRemoveMapSpot(pZone, false);
    }
}

BOOL CCustomDetector::feel_touch_contact(CObject* O)
{
    return (NULL != smart_cast<CCustomZone*>(O));
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
    TurnOff();

    m_pCurrentActor = NULL;
    m_pCurrentInvOwner = NULL;
    m_sndWorking = false;

    StopAllSounds();

    m_ZoneInfoMap.clear();
    Feel::Touch::feel_touch.clear();
}

void CCustomDetector::OnMoveToSlot() 
{
    m_pCurrentActor = smart_cast<CActor*>(H_Parent());
    m_pCurrentInvOwner = smart_cast<CInventoryOwner*>(H_Parent());
    m_sndWorking = true;
    inherited::OnMoveToSlot();
    TurnOn();
}

void CCustomDetector::TurnOn()
{
    UpdateMapLocations();
    UpdateNightVisionMode();
}

void CCustomDetector::TurnOff()
{
    UpdateMapLocations();
    UpdateNightVisionMode();
}

void CCustomDetector::StopAllSounds()
{
    ZONE_TYPE_MAP_IT it;
    for (it = m_ZoneTypeMap.begin(); m_ZoneTypeMap.end() != it; ++it)
    {
        ZONE_TYPE_SHOC& zone_type = (*it).second;
        HUD_SOUND::StopSound(zone_type.detect_snds);
    }
}

void CCustomDetector::TurnDetectorInternal(bool b)
{
    m_bWorking = b;
    if (b && !m_ui)
        CreateUI();
}

void CCustomDetector::AddRemoveMapSpot(CCustomZone* pZone, bool bAdd)
{
    if (m_ZoneTypeMap.find(pZone->CLS_ID) == m_ZoneTypeMap.end()) return;
    if (bAdd && !pZone->VisibleByDetector()) return;

    ZONE_TYPE_SHOC& zone_type = m_ZoneTypeMap[pZone->CLS_ID];
    if (xr_strlen(zone_type.zone_map_location)) {
        if (bAdd)
            Level().MapManager().AddMapLocation(*zone_type.zone_map_location, pZone->ID());
        else
            Level().MapManager().RemoveMapLocation(*zone_type.zone_map_location, pZone->ID());
    }
}

void CCustomDetector::UpdateMapLocations()
{
    ZONE_INFO_MAP_IT it;
    for (it = m_ZoneInfoMap.begin(); it != m_ZoneInfoMap.end(); ++it)
        AddRemoveMapSpot(it->first, m_sndWorking);
}

void CCustomDetector::UpdateNightVisionMode()
{
    bool bNightVision = Actor()->Cameras().GetPPEffector(EEffectorPPType(effNightvision)) != NULL;

    bool bOn = bNightVision &&
        m_pCurrentActor &&
        m_pCurrentActor == Level().CurrentViewEntity() &&
        m_sndWorking &&
        m_nightvision_particle.size();

    ZONE_INFO_MAP_IT it;
    for (it = m_ZoneInfoMap.begin(); m_ZoneInfoMap.end() != it; ++it)
    {
        CCustomZone* pZone = it->first;
        ZONE_INFO_SHOC& zone_info = it->second;

        if (bOn) {
            Fvector zero_vector;
            zero_vector.set(0.f, 0.f, 0.f);

            if (!zone_info.pParticle)
                zone_info.pParticle = CParticlesObject::Create(*m_nightvision_particle, FALSE);

            zone_info.pParticle->UpdateParent(pZone->XFORM(), zero_vector);
            if (!zone_info.pParticle->IsPlaying())
                zone_info.pParticle->Play();
        }
        else {
            if (zone_info.pParticle) {
                zone_info.pParticle->Stop();
                CParticlesObject::Destroy(zone_info.pParticle);
            }
        }
    }
}

void CCustomDetector::update_actor_radiation() {
    if (!m_detect_actor_radiation) return;
    if (m_ZoneTypeMap.find(CLSID_Z_RADIO) == m_ZoneTypeMap.end()) return;
    ZONE_TYPE_SHOC& zone_type = m_ZoneTypeMap[CLSID_Z_RADIO];

    float fRelPow = m_pCurrentActor->conditions().GetRadiation();
    if (fis_zero(fRelPow)) {
        radiation_snd_time = 0;
        return;
    }

    float cur_freq = zone_type.min_freq
        + (zone_type.max_freq - zone_type.min_freq)
        * fRelPow * fRelPow * fRelPow * fRelPow;
    float current_snd_time = 1000.f * 1.f / cur_freq;

    if (radiation_snd_time > current_snd_time) {
        radiation_snd_time = 0;
        HUD_SOUND::PlaySound(zone_type.detect_snds, Fvector().set(0, 0, 0), this, true, false);
    }
    else
        radiation_snd_time += Device.dwTimeDelta;
}

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
