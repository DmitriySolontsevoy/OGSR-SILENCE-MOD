#include "stdafx.h"

#include "Upgrade.h"
#include <Weapon.h>
#include <WeaponMagazined.h>
#include <CustomOutfit.h>
#include <smart_cast.h>

#include <string>
#include <vector>
#include <sstream>

char* CUpgrade::ComposeNewSectionName(LPCSTR sectionBase, std::string upgradeName)
{
    char* buf = new char[100];

    std::stringstream test(sectionBase);
    std::string segment;
    std::vector<std::string> upgrades;

    std::string sectionMain;

    bool skipped = false;
    while (std::getline(test, segment, ':'))
    {
        if (!skipped)
        {
            sectionMain = segment;
            skipped = true;
            continue;
        }

        upgrades.push_back(segment);
    }

    upgrades.push_back(upgradeName);

    if (upgrades.size() < 2)
    {
        sprintf(buf, "%s:%s", sectionMain.c_str(), upgradeName.c_str());
        return buf;
    }

    std::sort(upgrades.begin(), upgrades.end());

    skipped = false;
    for (const auto& i : upgrades)
    {
        if (!skipped)
        {
            sprintf(buf, "%s:%s", sectionMain.c_str(), i.c_str());
            skipped = true;
            continue;
        }

        sprintf(buf, "%s:%s", buf, i.c_str());
    }

    return buf;
}

CUpgrade::CUpgrade()
{
    CInifile::Sect& section = pSettings->r_section("upgrade_lists");

    for (const auto& [key, value] : section.Data)
    {
        std::stringstream test(value._get()->value);
        std::string segment;
        std::vector<std::string> upgrades;

        while (std::getline(test, segment, ','))
        {
            upgrades.push_back(segment);
        }

        availableUpgrades[key] = upgrades;
    }
}

bool CUpgrade::IsUpgradable(PIItem item)
{
    CWeapon* pWeapon = smart_cast<CWeapon*>(item);
    CCustomOutfit* pOutfit = smart_cast<CCustomOutfit*>(item);

    if (!pWeapon && !pOutfit)
    {
        return false;
    }

    return true;
}

PIItem CUpgrade::RepairItem(PIItem item)
{
    CWeapon* pWeapon = smart_cast<CWeapon*>(item);
    CCustomOutfit* pOutfit = smart_cast<CCustomOutfit*>(item);

    if (pWeapon || pOutfit)
    {
        item->SetCondition(1.0f);
    }

    return item;
}

PIItem CUpgrade::UpgradeItem(PIItem item, std::string upgrade)
{
    CWeaponMagazined* pWeapon = smart_cast<CWeaponMagazined*>(item);
    int ammo = pWeapon->GetAmmoElapsed();

    char* newName = ComposeNewSectionName(pWeapon->m_section_name, upgrade.c_str());

    if (pWeapon)
    {
        pWeapon->Load(newName);
        pWeapon->cNameSect_set(newName);
        pWeapon->alife_object()->set_name(newName);
        pWeapon->alife_object()->visual()->set_visual(pSettings->r_string(newName, "visual"));
        pWeapon->SetAmmoElapsed(ammo);
    }

    return pWeapon;
}

std::vector<std::string> CUpgrade::LoadUpgradesForSect(shared_str section)
{
    return availableUpgrades[section];
}