////////////////////////////////////////////////////////////////////////////
//	Module 		: eatable_item.cpp
//	Created 	: 24.03.2003
//  Modified 	: 29.01.2004
//	Author		: Yuri Dobronravin
//	Description : Eatable item
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "eatable_item.h"
#include "physic_item.h"
#include "Level.h"
#include "entity_alive.h"
#include "EntityCondition.h"
#include "InventoryOwner.h"

CEatableItem::CEatableItem()
{
    m_fHealthInfluence = 0;
    m_fPowerInfluence = 0;
    m_fSatietyInfluence = 0;
    m_fRadiationInfluence = 0;
    m_fPsyHealthInfluence = 0;

    m_iPortionsNum = -1;

    m_physic_item = 0;
}

CEatableItem::~CEatableItem() {}

DLL_Pure* CEatableItem::_construct()
{
    m_physic_item = smart_cast<CPhysicItem*>(this);
    return (inherited::_construct());
}

void CEatableItem::Load(LPCSTR section)
{
    inherited::Load(section);

    m_fHealthInfluence = READ_IF_EXISTS(pSettings, r_float, section, "eat_health", 0.0f);
    m_fPsyHealthInfluence = READ_IF_EXISTS(pSettings, r_float, section, "eat_psy_health", 0.0f);
    m_fPowerInfluence = READ_IF_EXISTS(pSettings, r_float, section, "eat_power", 0.0f);
    m_fSatietyInfluence = READ_IF_EXISTS(pSettings, r_float, section, "eat_satiety", 0.0f);
    m_fRadiationInfluence = READ_IF_EXISTS(pSettings, r_float, section, "eat_radiation", 0.0f);
    m_fWoundsHealPerc = READ_IF_EXISTS(pSettings, r_float, section, "wounds_heal_perc", 0.0f);

    // Бустер-параметры

    m_fHealthBoostTime = READ_IF_EXISTS(pSettings, r_float, section, "eat_boost_health_time", 0.0f);
    m_fHealthBoostValue = READ_IF_EXISTS(pSettings, r_float, section, "eat_boost_health_value", 0.0f);

    m_fPsyHealthBoostTime = READ_IF_EXISTS(pSettings, r_float, section, "eat_boost_psy_health_time", 0.0f);
    m_fPsyHealthBoostValue = READ_IF_EXISTS(pSettings, r_float, section, "eat_boost_psy_health_value", 0.0f);

    m_fPowerBoostTime = READ_IF_EXISTS(pSettings, r_float, section, "eat_boost_power_time", 0.0f);
    m_fPowerBoostValue = READ_IF_EXISTS(pSettings, r_float, section, "eat_boost_power_value", 0.0f);

    m_fRadiationBoostTime = READ_IF_EXISTS(pSettings, r_float, section, "eat_boost_radiation_time", 0.0f);
    m_fRadiationBoostValue = READ_IF_EXISTS(pSettings, r_float, section, "eat_boost_radiation_value", 0.0f);

    m_fBleedingBoostTime = READ_IF_EXISTS(pSettings, r_float, section, "eat_boost_bleeding_time", 0.0f);
    m_fBleedingBoostValue = READ_IF_EXISTS(pSettings, r_float, section, "eat_boost_bleeding_value", 0.0f);

    m_fMaxWeightBoostTime = READ_IF_EXISTS(pSettings, r_float, section, "eat_boost_max_weight_time", 0.0f);
    m_fMaxWeightBoostValue = READ_IF_EXISTS(pSettings, r_float, section, "eat_boost_max_weight_value", 0.0f);

    m_fRadiationImmunityBoostTime = READ_IF_EXISTS(pSettings, r_float, section, "eat_boost_radiation_immunity_time", 0.0f);
    m_fRadiationImmunityBoostValue = READ_IF_EXISTS(pSettings, r_float, section, "eat_boost_radiation_immunity_value", 0.0f);

    m_fChemicalImmunityBoostTime = READ_IF_EXISTS(pSettings, r_float, section, "eat_boost_chemical_immunity_time", 0.0f);
    m_fChemicalImmunityBoostValue = READ_IF_EXISTS(pSettings, r_float, section, "eat_boost_chemical_immunity_value", 0.0f);

    m_fThermalImmunityBoostTime = READ_IF_EXISTS(pSettings, r_float, section, "eat_boost_thermal_immunity_time", 0.0f);
    m_fThermalImmunityBoostValue = READ_IF_EXISTS(pSettings, r_float, section, "eat_boost_thermal_immunity_value", 0.0f);

    m_fElectricImmunityBoostTime = READ_IF_EXISTS(pSettings, r_float, section, "eat_boost_electric_immunity_time", 0.0f);
    m_fElectricImmunityBoostValue = READ_IF_EXISTS(pSettings, r_float, section, "eat_boost_electric_immunity_value", 0.0f);

    m_fPsychicImmunityBoostTime = READ_IF_EXISTS(pSettings, r_float, section, "eat_boost_psychic_immunity_time", 0.0f);
    m_fPsychicImmunityBoostValue = READ_IF_EXISTS(pSettings, r_float, section, "eat_boost_psychic_immunity_value", 0.0f);

    clamp(m_fWoundsHealPerc, 0.f, 1.f);

    m_fPsyHealthInfluence = READ_IF_EXISTS(pSettings, r_float, section, "eat_psy_health", 0.0f);
    m_fThirstInfluence = READ_IF_EXISTS(pSettings, r_float, section, "eat_thirst", 0.0f);

    m_iStartPortionsNum = pSettings->r_s32(section, "eat_portions_num");
    m_fMaxPowerUpInfluence = READ_IF_EXISTS(pSettings, r_float, section, "eat_max_power", 0.0f);
    VERIFY(m_iPortionsNum < 10000);
}

BOOL CEatableItem::net_Spawn(CSE_Abstract* DC)
{
    if (!inherited::net_Spawn(DC))
        return FALSE;

    m_iPortionsNum = m_iStartPortionsNum;

    return TRUE;
};

bool CEatableItem::Useful() const
{
    if (!inherited::Useful())
        return false;

    //проверить не все ли еще съедено
    if (Empty())
        return false;

    return true;
}

void CEatableItem::OnH_B_Independent(bool just_before_destroy)
{
    if (!Useful())
    {
        object().setVisible(FALSE);
        object().setEnabled(FALSE);
        if (m_physic_item)
            m_physic_item->m_ready_to_destroy = true;
    }
    inherited::OnH_B_Independent(just_before_destroy);
}

void CEatableItem::UseBy(CEntityAlive* entity_alive)
{
    CInventoryOwner* IO = smart_cast<CInventoryOwner*>(entity_alive);
    R_ASSERT(IO);
    R_ASSERT(m_pCurrentInventory == IO->m_inventory);
    R_ASSERT(object().H_Parent()->ID() == entity_alive->ID());

    entity_alive->conditions().ChangeHealth(m_fHealthInfluence);
    entity_alive->conditions().ChangePower(m_fPowerInfluence);
    entity_alive->conditions().ChangeSatiety(m_fSatietyInfluence);
    entity_alive->conditions().ChangeRadiation(m_fRadiationInfluence);
    entity_alive->conditions().ChangeBleeding(m_fWoundsHealPerc);
    entity_alive->conditions().ChangePsyHealth(m_fPsyHealthInfluence);
    entity_alive->conditions().ChangeThirst(m_fThirstInfluence);

    if (m_fHealthBoostTime)
        entity_alive->conditions().ApplyHealthBooster(m_fHealthBoostTime, m_fHealthBoostValue);
    if (m_fPsyHealthBoostTime)
        entity_alive->conditions().ApplyPsyHealthBooster(m_fPsyHealthBoostTime, m_fPsyHealthBoostValue);
    if (m_fPowerBoostTime)
        entity_alive->conditions().ApplyPowerBooster(m_fPowerBoostTime, m_fPowerBoostValue);
    if (m_fRadiationBoostTime)
        entity_alive->conditions().ApplyRadiationBooster(m_fRadiationBoostTime, m_fRadiationBoostValue);
    if (m_fBleedingBoostTime)
        entity_alive->conditions().ApplyBleedingBooster(m_fBleedingBoostTime, m_fBleedingBoostValue);
    if (m_fMaxWeightBoostTime)
        entity_alive->conditions().ApplyMaxWeightBooster(m_fMaxWeightBoostTime, m_fMaxWeightBoostValue);
    if (m_fRadiationImmunityBoostTime)
        entity_alive->conditions().ApplyRadiationImmunityBooster(m_fRadiationImmunityBoostTime, m_fRadiationImmunityBoostValue);
    if (m_fChemicalImmunityBoostTime)
        entity_alive->conditions().ApplyChemicalImmunityBooster(m_fChemicalImmunityBoostTime, m_fChemicalImmunityBoostTime);
    if (m_fThermalImmunityBoostTime)
        entity_alive->conditions().ApplyThermalImmunityBooster(m_fThermalImmunityBoostTime, m_fThermalImmunityBoostValue);
    if (m_fElectricImmunityBoostTime)
        entity_alive->conditions().ApplyElectricImmunityBooster(m_fElectricImmunityBoostTime, m_fElectricImmunityBoostValue);
    if (m_fPsychicImmunityBoostTime)
        entity_alive->conditions().ApplyPsychicImmunityBooster(m_fPsychicImmunityBoostTime, m_fPsychicImmunityBoostValue);

    entity_alive->conditions().SetMaxPower(entity_alive->conditions().GetMaxPower() + m_fMaxPowerUpInfluence);

    // уменьшить количество порций
    if (m_iPortionsNum > 0)
        --m_iPortionsNum;
    else
        m_iPortionsNum = 0;
}

void CEatableItem::ZeroAllEffects()
{
    m_fHealthInfluence = 0.f;
    m_fPowerInfluence = 0.f;
    m_fSatietyInfluence = 0.f;
    m_fRadiationInfluence = 0.f;
    m_fMaxPowerUpInfluence = 0.f;
    m_fPsyHealthInfluence = 0.f;
    m_fWoundsHealPerc = 0.f;
    m_fThirstInfluence = 0.f;
}

void CEatableItem::SetRadiation(float _rad) { m_fRadiationInfluence = _rad; }
