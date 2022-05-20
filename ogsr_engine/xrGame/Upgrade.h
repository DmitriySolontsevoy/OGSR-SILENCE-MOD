#pragma once
#include <inventory_space.h>

class CUpgrade
{
public:
	CUpgrade();
	~CUpgrade() {};

	bool IsUpgradable(PIItem item);
	PIItem RepairItem(PIItem item);
	PIItem UpgradeItem(PIItem item, std::string upgrade);
	char* ComposeNewSectionName(LPCSTR sectionBase, std::string upgradeName);
	std::vector<std::string> LoadUpgradesForSect(shared_str section);
private:
	std::map<shared_str, std::vector<std::string>> availableUpgrades;
};