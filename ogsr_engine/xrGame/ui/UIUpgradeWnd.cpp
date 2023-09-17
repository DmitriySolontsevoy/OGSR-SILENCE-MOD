#include "stdafx.h"

#include "UIUpgradeWnd.h"

#include "xrUIXmlParser.h"
#include "UIXmlInit.h"

#include "../Entity.h"
#include "../HUDManager.h"
#include "../WeaponAmmo.h"
#include "../Actor.h"
#include "../UIGameSP.h"
#include "UIInventoryUtilities.h"
#include "../inventoryowner.h"
#include "../eatable_item.h"
#include "../inventory.h"
#include "../level.h"
#include "../string_table.h"
#include "../character_info.h"
#include "UIMultiTextStatic.h"
#include "UI3tButton.h"
#include "UIItemInfo.h"

#include "UICharacterInfo.h"
#include "UIDragDropListEx.h"
#include "UICellItem.h"
#include "UICellItemFactory.h"
#include "UIPropertiesBox.h"
#include "UIListBoxItem.h"

#include "../game_object_space.h"
#include "../script_callback_ex.h"
#include "../script_game_object.h"
#include "../xr_3da/xr_input.h"

#include <string>
#include <vector>
#include <sstream>

#define UPGRADE_XML "ui_repair.xml"
#define TRADE_CHARACTER_XML "trade_character.xml"

struct CUIUpgradeInternal
{
    CUIStatic UIStaticMain;

    CUIDragDropListEx UIOurBagList;
    CUIDragDropListEx UIRepairBox;

    // кнопки
    CUI3tButton UIRepairButton;
    CUI3tButton UIToTalkButton;

    // апгрейды
    CUI3tButton UIUpgrade1;
    CUI3tButton UIUpgrade2;
    CUI3tButton UIUpgrade3;
    CUI3tButton UIUpgrade4;
    CUI3tButton UIUpgrade5;
    CUI3tButton UIUpgrade6;
    CUI3tButton UIUpgrade7;
    CUI3tButton UIUpgrade8;
    CUI3tButton UIUpgrade9;
    CUI3tButton UIUpgrade10;

    // информация о персонажах
    CUIStatic UIOurIcon;
    CUIStatic UIOthersIcon;
    CUICharacterInfo UICharacterInfoLeft;
    CUICharacterInfo UICharacterInfoRight;
};

CUIUpgradeWnd::CUIUpgradeWnd() : m_bDealControlsVisible(false), bStarted(false), needsAnUpdate(true), upgrader(CUpgrade())
{
    m_uidata = xr_new<CUIUpgradeInternal>();

    Init();
    Hide();
    SetCurrentItem(NULL);
}

CUIUpgradeWnd::~CUIUpgradeWnd()
{
    m_uidata->UIOurBagList.ClearAll(true);
    m_uidata->UIRepairBox.ClearAll(true);
    xr_delete(m_uidata);
}

void CUIUpgradeWnd::SwitchToTalk() { GetMessageTarget()->SendMessage(this, UPGRADE_WND_CLOSED); }

void CUIUpgradeWnd::StartUpgrade() { bStarted = true; }

void CUIUpgradeWnd::StopUpgrade() { bStarted = false; }

void CUIUpgradeWnd::InitUpgrade(CInventoryOwner* pOur, CInventoryOwner* pOthers)
{
    VERIFY(pOur);
    VERIFY(pOthers);

    m_pInvOwner = pOur;
    m_pOthersInvOwner = pOthers;

    needsAnUpdate = true;

    m_pInv = &m_pInvOwner->inventory();

    m_uidata->UICharacterInfoLeft.InitCharacter(m_pInvOwner->object_id());
    m_uidata->UICharacterInfoRight.InitCharacter(m_pOthersInvOwner->object_id());

    m_pInv = &m_pInvOwner->inventory();

    EnableAll();

    UpdateLists();
}

void CUIUpgradeWnd::Init()
{
    CUIXml uiXml;
    bool xml_result = uiXml.Init(CONFIG_PATH, UI_PATH, UPGRADE_XML);
    R_ASSERT3(xml_result, "xml file not found", UPGRADE_XML);
    CUIXmlInit xml_init;

    xml_init.InitWindow(uiXml, "main", 0, this);

    // статические элементы интерфейса
    AttachChild(&m_uidata->UIStaticMain);
    xml_init.InitStatic(uiXml, "main_window", 0, &m_uidata->UIStaticMain);

    // иконки с изображение нас и партнера по торговле
    AttachChild(&m_uidata->UIOurIcon);
    xml_init.InitStatic(uiXml, "static_icon", 0, &m_uidata->UIOurIcon);
    AttachChild(&m_uidata->UIOthersIcon);
    xml_init.InitStatic(uiXml, "static_icon", 1, &m_uidata->UIOthersIcon);
    m_uidata->UIOurIcon.AttachChild(&m_uidata->UICharacterInfoLeft);
    m_uidata->UICharacterInfoLeft.Init(0, 0, m_uidata->UIOurIcon.GetWidth(), m_uidata->UIOurIcon.GetHeight(), TRADE_CHARACTER_XML);
    m_uidata->UIOthersIcon.AttachChild(&m_uidata->UICharacterInfoRight);
    m_uidata->UICharacterInfoRight.Init(0, 0, m_uidata->UIOthersIcon.GetWidth(), m_uidata->UIOthersIcon.GetHeight(), TRADE_CHARACTER_XML);

    // Списки Drag&Drop
    AttachChild(&m_uidata->UIOurBagList);
    xml_init.InitDragDropListEx(uiXml, "dragdrop_list", 0, &m_uidata->UIOurBagList);

    AttachChild(&m_uidata->UIRepairBox);
    xml_init.InitDragDropListEx(uiXml, "dragdrop_repair", 0, &m_uidata->UIRepairBox);

    AttachChild(&m_uidata->UIRepairButton);
    xml_init.Init3tButton(uiXml, "repair", 0, &m_uidata->UIRepairButton);

    AttachChild(&m_uidata->UIToTalkButton);
    xml_init.Init3tButton(uiXml, "button", 0, &m_uidata->UIToTalkButton);

    AttachChild(&m_uidata->UIUpgrade1);
    xml_init.Init3tButton(uiXml, "upgrade_window_1", 0, &m_uidata->UIUpgrade1);

    AttachChild(&m_uidata->UIUpgrade2);
    xml_init.Init3tButton(uiXml, "upgrade_window_2", 0, &m_uidata->UIUpgrade2);

    AttachChild(&m_uidata->UIUpgrade3);
    xml_init.Init3tButton(uiXml, "upgrade_window_3", 0, &m_uidata->UIUpgrade3);

    AttachChild(&m_uidata->UIUpgrade4);
    xml_init.Init3tButton(uiXml, "upgrade_window_4", 0, &m_uidata->UIUpgrade4);

    AttachChild(&m_uidata->UIUpgrade5);
    xml_init.Init3tButton(uiXml, "upgrade_window_5", 0, &m_uidata->UIUpgrade5);

    AttachChild(&m_uidata->UIUpgrade6);
    xml_init.Init3tButton(uiXml, "upgrade_window_6", 0, &m_uidata->UIUpgrade6);

    AttachChild(&m_uidata->UIUpgrade7);
    xml_init.Init3tButton(uiXml, "upgrade_window_7", 0, &m_uidata->UIUpgrade7);

    AttachChild(&m_uidata->UIUpgrade8);
    xml_init.Init3tButton(uiXml, "upgrade_window_8", 0, &m_uidata->UIUpgrade8);

    AttachChild(&m_uidata->UIUpgrade9);
    xml_init.Init3tButton(uiXml, "upgrade_window_9", 0, &m_uidata->UIUpgrade9);

    AttachChild(&m_uidata->UIUpgrade10);
    xml_init.Init3tButton(uiXml, "upgrade_window_10", 0, &m_uidata->UIUpgrade10);

    BindDragDropListEvents(&m_uidata->UIOurBagList);
    BindDragDropListEvents(&m_uidata->UIRepairBox);
}

void CUIUpgradeWnd::BindDragDropListEvents(CUIDragDropListEx* lst)
{
    lst->m_f_item_drop = fastdelegate::MakeDelegate(this, &CUIUpgradeWnd::OnItemDrop);
    lst->m_f_item_start_drag = fastdelegate::MakeDelegate(this, &CUIUpgradeWnd::OnItemStartDrag);
    lst->m_f_item_db_click = fastdelegate::MakeDelegate(this, &CUIUpgradeWnd::OnItemDbClick);
    lst->m_f_item_selected = fastdelegate::MakeDelegate(this, &CUIUpgradeWnd::OnItemSelected);
    lst->m_f_item_rbutton_click = fastdelegate::MakeDelegate(this, &CUIUpgradeWnd::OnItemRButtonClick);
}

void CUIUpgradeWnd::ColorizeItem(CUICellItem* itm, bool canTrade, bool highlighted)
{
    static const bool highlight_cop_enabled = !Core.Features.test(xrCore::Feature::colorize_untradable);

    if (!canTrade)
    {
        if (highlight_cop_enabled)
            itm->m_select_untradable = true;
        itm->SetColor(reinterpret_cast<CInventoryItem*>(itm->m_pData)->ClrUntradable);
    }
    else
    {
        if (highlight_cop_enabled)
            itm->m_select_untradable = false;
        if (highlighted)
            itm->SetColor(reinterpret_cast<CInventoryItem*>(itm->m_pData)->ClrEquipped);
    }
}

CUICellItem* CUIUpgradeWnd::CurrentItem() { return m_pCurrentCellItem; }

PIItem CUIUpgradeWnd::CurrentIItem() { return (m_pCurrentCellItem) ? (PIItem)m_pCurrentCellItem->m_pData : NULL; }

void CUIUpgradeWnd::SetCurrentItem(CUICellItem* itm)
{
    if (m_pCurrentCellItem == itm)
        return;
    m_pCurrentCellItem = itm;

    if (!m_pCurrentCellItem)
        return;

    m_pCurrentCellItem->m_select_armament = true;

    auto script_obj = CurrentIItem()->object().lua_game_object();
    g_actor->callback(GameObject::eCellItemSelect)(script_obj);
}

bool CUIUpgradeWnd::MoveItem(CUICellItem* itm)
{
    CUIDragDropListEx* old_owner = itm->OwnerList();

    if (old_owner == &m_uidata->UIRepairBox)
        ToOurBag(itm);
    else if (old_owner == &m_uidata->UIOurBagList)
        ToRepairBox(itm, false);

    return true;
}

bool CUIUpgradeWnd::OnItemDbClick(CUICellItem* itm)
{
    SetCurrentItem(itm);
    MoveItem(itm);

    return true;
}

bool CUIUpgradeWnd::OnItemStartDrag(CUICellItem* itm) { return false; }

bool CUIUpgradeWnd::OnItemSelected(CUICellItem* itm)
{
    SetCurrentItem(itm);
    itm->ColorizeItems({&m_uidata->UIOurBagList, &m_uidata->UIRepairBox});
    return false;
}

bool CUIUpgradeWnd::OnItemRButtonClick(CUICellItem* itm)
{
    SetCurrentItem(itm);
    return false;
}

bool CUIUpgradeWnd::OnItemDrop(CUICellItem* itm)
{
    CUIDragDropListEx* old_owner = itm->OwnerList();
    CUIDragDropListEx* new_owner = CUIDragDropListEx::m_drag_item->BackList();
    if (old_owner == new_owner || !old_owner || !new_owner)
        return false;

    MoveItem(itm);

    return true;
}

void CUIUpgradeWnd::UpdateLists()
{
    if (needsAnUpdate)
    {
        m_uidata->UIOurBagList.ClearAll(true);
        m_uidata->UIRepairBox.ClearAll(true);

        ruck_list.clear();
        m_pInv->AddAvailableItems(ruck_list, true);
        std::sort(ruck_list.begin(), ruck_list.end(), InventoryUtilities::GreaterRoomInRuck);
        FillList(ruck_list, m_uidata->UIOurBagList, true);
    }
    needsAnUpdate = false;
}

bool CUIUpgradeWnd::CanMoveToOther(PIItem pItem, bool our) { return true; }

void CUIUpgradeWnd::FillList(TIItemContainer& cont, CUIDragDropListEx& dragDropList, bool our)
{
    TIItemContainer::iterator it = cont.begin();
    TIItemContainer::iterator it_e = cont.end();

    for (; it != it_e; ++it)
    {
        CInventoryItem* item = *it;
        CUICellItem* itm = create_cell_item(item);
        if (item->m_highlight_equipped)
            itm->m_select_equipped = true;
        bool canTrade = CanMoveToOther(item, our);
        ColorizeItem(itm, canTrade, itm->m_select_equipped);
        dragDropList.SetItem(itm);
    }
}

void CUIUpgradeWnd::DisableAll() { m_uidata->UIStaticMain.Enable(false); }

void CUIUpgradeWnd::EnableAll() { m_uidata->UIStaticMain.Enable(true); }

void move_item_(CUICellItem* itm, CUIDragDropListEx* from, CUIDragDropListEx* to)
{
    CUICellItem* _itm = from->RemoveItem(itm, false);
    to->SetItem(_itm);
}

bool CUIUpgradeWnd::ToOurBag(CUICellItem* itm)
{
    upgradeBySlot.clear();

    m_uidata->UIUpgrade1.InitTexture("ui_no_upgrade");
    m_uidata->UIUpgrade2.InitTexture("ui_no_upgrade");
    m_uidata->UIUpgrade3.InitTexture("ui_no_upgrade");
    m_uidata->UIUpgrade4.InitTexture("ui_no_upgrade");
    m_uidata->UIUpgrade5.InitTexture("ui_no_upgrade");
    m_uidata->UIUpgrade6.InitTexture("ui_no_upgrade");
    m_uidata->UIUpgrade7.InitTexture("ui_no_upgrade");
    m_uidata->UIUpgrade8.InitTexture("ui_no_upgrade");
    m_uidata->UIUpgrade9.InitTexture("ui_no_upgrade");
    m_uidata->UIUpgrade10.InitTexture("ui_no_upgrade");

    move_item_(itm, &m_uidata->UIRepairBox, &m_uidata->UIOurBagList);
    needsAnUpdate = true;

    PIItem inventoryItem = (PIItem)itm->m_pData;

    if (!m_pInv->InSlot(inventoryItem) && m_pInv->CanPutInSlot(inventoryItem))
    {
        m_pInv->Slot(inventoryItem);
    }

    return true;
}

bool CUIUpgradeWnd::ToRepairBox(CUICellItem* itm, bool upgraded)
{
    PIItem item = (PIItem)itm->m_pData;

    if (!upgrader.IsUpgradable(item))
        return false;

    if (m_uidata->UIRepairBox.ItemsCount() > 0)
    {
        upgradeBySlot.clear();

        m_uidata->UIUpgrade1.InitTexture("ui_no_upgrade");
        m_uidata->UIUpgrade2.InitTexture("ui_no_upgrade");
        m_uidata->UIUpgrade3.InitTexture("ui_no_upgrade");
        m_uidata->UIUpgrade4.InitTexture("ui_no_upgrade");
        m_uidata->UIUpgrade5.InitTexture("ui_no_upgrade");
        m_uidata->UIUpgrade6.InitTexture("ui_no_upgrade");
        m_uidata->UIUpgrade7.InitTexture("ui_no_upgrade");
        m_uidata->UIUpgrade8.InitTexture("ui_no_upgrade");
        m_uidata->UIUpgrade9.InitTexture("ui_no_upgrade");
        m_uidata->UIUpgrade10.InitTexture("ui_no_upgrade");

        if (!upgraded)
            move_item_(m_uidata->UIRepairBox.GetItemIdx(0), &m_uidata->UIRepairBox, &m_uidata->UIOurBagList);
    }

    if (!upgraded)
        move_item_(itm, &m_uidata->UIOurBagList, &m_uidata->UIRepairBox);

    PIItem inventoryItem = (PIItem)itm->m_pData;

    if (!m_pInv->InRuck(inventoryItem) && m_pInv->CanPutInRuck(inventoryItem))
    {
        m_pInv->Ruck(inventoryItem);
    }

    std::stringstream test(inventoryItem->m_section_name);
    std::string segment;
    std::vector<std::string> installedUpgrades;

    while (std::getline(test, segment, ':'))
    {
        installedUpgrades.push_back(segment);
    }

    std::vector<std::string> upgrades = upgrader.LoadUpgradesForSect(installedUpgrades[0].c_str());

    if (upgrades.size() == 0)
        return true;

    CUIXml uiXml;
    bool xml_result = uiXml.Init(CONFIG_PATH, UI_PATH, UPGRADE_XML);
    R_ASSERT3(xml_result, "xml file not found", UPGRADE_XML);
    CUIXmlInit xml_init;

    int counter = 0;
    for (const auto& i : upgrades)
    {
        if (counter == 10 || counter == upgrades.size())
            break;

        char* buf = new char[100];
        sprintf(buf, "ui_%s_upgrade", i.c_str());
        upgradeBySlot[counter] = i.c_str();

        switch (counter)
        {
        case 0:
            m_uidata->UIUpgrade1.InitTexture(buf);
            m_uidata->UIUpgrade1.SetStretchTexture(true);
            if (std::find(installedUpgrades.begin(), installedUpgrades.end(), i.c_str()) != installedUpgrades.end())
            {
                m_uidata->UIUpgrade1.Enable(false);
            }
            else
            {
                m_uidata->UIUpgrade1.Enable(true);
            }
            break;
        case 1:
            m_uidata->UIUpgrade2.InitTexture(buf);
            m_uidata->UIUpgrade2.SetStretchTexture(true);
            if (std::find(installedUpgrades.begin(), installedUpgrades.end(), i.c_str()) != installedUpgrades.end())
            {
                m_uidata->UIUpgrade2.Enable(false);
            }
            else
            {
                m_uidata->UIUpgrade2.Enable(true);
            }
            break;
        case 2:
            m_uidata->UIUpgrade3.InitTexture(buf);
            m_uidata->UIUpgrade3.SetStretchTexture(true);
            if (std::find(installedUpgrades.begin(), installedUpgrades.end(), i.c_str()) != installedUpgrades.end())
            {
                m_uidata->UIUpgrade3.Enable(false);
            }
            else
            {
                m_uidata->UIUpgrade3.Enable(true);
            }
            break;
        case 3:
            m_uidata->UIUpgrade4.InitTexture(buf);
            m_uidata->UIUpgrade4.SetStretchTexture(true);
            if (std::find(installedUpgrades.begin(), installedUpgrades.end(), i.c_str()) != installedUpgrades.end())
            {
                m_uidata->UIUpgrade4.Enable(false);
            }
            else
            {
                m_uidata->UIUpgrade4.Enable(true);
            }
            break;
        case 4:
            m_uidata->UIUpgrade5.InitTexture(buf);
            m_uidata->UIUpgrade5.SetStretchTexture(true);
            if (std::find(installedUpgrades.begin(), installedUpgrades.end(), i.c_str()) != installedUpgrades.end())
            {
                m_uidata->UIUpgrade5.Enable(false);
            }
            else
            {
                m_uidata->UIUpgrade5.Enable(true);
            }
            break;
        case 5:
            m_uidata->UIUpgrade6.InitTexture(buf);
            m_uidata->UIUpgrade6.SetStretchTexture(true);
            if (std::find(installedUpgrades.begin(), installedUpgrades.end(), i.c_str()) != installedUpgrades.end())
            {
                m_uidata->UIUpgrade6.Enable(false);
            }
            else
            {
                m_uidata->UIUpgrade6.Enable(true);
            }
            break;
        case 6:
            m_uidata->UIUpgrade7.InitTexture(buf);
            m_uidata->UIUpgrade7.SetStretchTexture(true);
            if (std::find(installedUpgrades.begin(), installedUpgrades.end(), i.c_str()) != installedUpgrades.end())
            {
                m_uidata->UIUpgrade7.Enable(false);
            }
            else
            {
                m_uidata->UIUpgrade7.Enable(true);
            }
            break;
        case 7:
            m_uidata->UIUpgrade8.InitTexture(buf);
            m_uidata->UIUpgrade8.SetStretchTexture(true);
            if (std::find(installedUpgrades.begin(), installedUpgrades.end(), i.c_str()) != installedUpgrades.end())
            {
                m_uidata->UIUpgrade8.Enable(false);
            }
            else
            {
                m_uidata->UIUpgrade8.Enable(true);
            }
            break;
        case 8:
            m_uidata->UIUpgrade9.InitTexture(buf);
            m_uidata->UIUpgrade9.SetStretchTexture(true);
            if (std::find(installedUpgrades.begin(), installedUpgrades.end(), i.c_str()) != installedUpgrades.end())
            {
                m_uidata->UIUpgrade9.Enable(false);
            }
            else
            {
                m_uidata->UIUpgrade9.Enable(true);
            }
            break;
        case 9:
            m_uidata->UIUpgrade10.InitTexture(buf);
            m_uidata->UIUpgrade10.SetStretchTexture(true);
            if (std::find(installedUpgrades.begin(), installedUpgrades.end(), i.c_str()) != installedUpgrades.end())
            {
                m_uidata->UIUpgrade10.Enable(false);
            }
            else
            {
                m_uidata->UIUpgrade10.Enable(true);
            }
            break;
        }

        counter++;
    }

    return true;
}

void CUIUpgradeWnd::Show()
{
    inherited::Show(true);
    inherited::Enable(true);

    SetCurrentItem(NULL);
    ResetAll();
}

void CUIUpgradeWnd::Hide()
{
    inherited::Show(false);
    inherited::Enable(false);
    if (bStarted)
        StopUpgrade();

    m_uidata->UIOurBagList.ClearAll(true);
    m_uidata->UIRepairBox.ClearAll(true);
}

extern void UpdateCameraDirection(CGameObject* pTo);

void CUIUpgradeWnd::Update()
{
    UpdateLists();

    inherited::Update();
    UpdateCameraDirection(smart_cast<CGameObject*>(m_pOthersInvOwner));
}

void CUIUpgradeWnd::SendMessage(CUIWindow* pWnd, s16 msg, void* pData)
{
    if (pWnd == &m_uidata->UIToTalkButton && msg == BUTTON_CLICKED)
    {
        SwitchToTalk();
    }
    else if (pWnd == &m_uidata->UIRepairButton && msg == BUTTON_CLICKED)
    {
        if (m_uidata->UIRepairBox.ItemsCount() > 0)
        {
            PIItem item = (PIItem)m_uidata->UIRepairBox.GetItemIdx(0)->m_pData;
            PIItem repairedItem = upgrader.RepairItem(item);

            ToRepairBox(create_cell_item(repairedItem), true);
            CUICellItem* cellItem = create_cell_item(repairedItem);
            m_uidata->UIRepairBox.ClearAll(true);
            m_uidata->UIRepairBox.SetItem(cellItem);
        }
    }
    else if (pWnd == &m_uidata->UIUpgrade1 && msg == BUTTON_CLICKED)
    {
        PIItem item = (PIItem)m_uidata->UIRepairBox.GetItemIdx(0)->m_pData;
        PIItem upgradedItem = upgrader.UpgradeItem(item, upgradeBySlot[0]);

        ToRepairBox(create_cell_item(upgradedItem), true);
        CUICellItem* cellItem = create_cell_item(upgradedItem);
        m_uidata->UIRepairBox.ClearAll(true);
        m_uidata->UIRepairBox.SetItem(cellItem);
    }
    else if (pWnd == &m_uidata->UIUpgrade2 && msg == BUTTON_CLICKED)
    {
        PIItem item = (PIItem)m_uidata->UIRepairBox.GetItemIdx(0)->m_pData;
        PIItem upgradedItem = upgrader.UpgradeItem(item, upgradeBySlot[1]);

        ToRepairBox(create_cell_item(upgradedItem), true);
        CUICellItem* cellItem = create_cell_item(upgradedItem);
        m_uidata->UIRepairBox.ClearAll(true);
        m_uidata->UIRepairBox.SetItem(cellItem);
    }
    else if (pWnd == &m_uidata->UIUpgrade3 && msg == BUTTON_CLICKED)
    {
        PIItem item = (PIItem)m_uidata->UIRepairBox.GetItemIdx(0)->m_pData;
        PIItem upgradedItem = upgrader.UpgradeItem(item, upgradeBySlot[2]);

        ToRepairBox(create_cell_item(upgradedItem), true);
        CUICellItem* cellItem = create_cell_item(upgradedItem);
        m_uidata->UIRepairBox.ClearAll(true);
        m_uidata->UIRepairBox.SetItem(cellItem);
    }
    else if (pWnd == &m_uidata->UIUpgrade4 && msg == BUTTON_CLICKED)
    {
        PIItem item = (PIItem)m_uidata->UIRepairBox.GetItemIdx(0)->m_pData;
        PIItem upgradedItem = upgrader.UpgradeItem(item, upgradeBySlot[3]);

        ToRepairBox(create_cell_item(upgradedItem), true);
        CUICellItem* cellItem = create_cell_item(upgradedItem);
        m_uidata->UIRepairBox.ClearAll(true);
        m_uidata->UIRepairBox.SetItem(cellItem);
    }
    else if (pWnd == &m_uidata->UIUpgrade5 && msg == BUTTON_CLICKED)
    {
        PIItem item = (PIItem)m_uidata->UIRepairBox.GetItemIdx(0)->m_pData;
        PIItem upgradedItem = upgrader.UpgradeItem(item, upgradeBySlot[4]);

        ToRepairBox(create_cell_item(upgradedItem), true);
        CUICellItem* cellItem = create_cell_item(upgradedItem);
        m_uidata->UIRepairBox.ClearAll(true);
        m_uidata->UIRepairBox.SetItem(cellItem);
    }
    else if (pWnd == &m_uidata->UIUpgrade6 && msg == BUTTON_CLICKED)
    {
        PIItem item = (PIItem)m_uidata->UIRepairBox.GetItemIdx(0)->m_pData;
        PIItem upgradedItem = upgrader.UpgradeItem(item, upgradeBySlot[5]);

        ToRepairBox(create_cell_item(upgradedItem), true);
        CUICellItem* cellItem = create_cell_item(upgradedItem);
        m_uidata->UIRepairBox.ClearAll(true);
        m_uidata->UIRepairBox.SetItem(cellItem);
    }
    else if (pWnd == &m_uidata->UIUpgrade7 && msg == BUTTON_CLICKED)
    {
        PIItem item = (PIItem)m_uidata->UIRepairBox.GetItemIdx(0)->m_pData;
        PIItem upgradedItem = upgrader.UpgradeItem(item, upgradeBySlot[6]);

        ToRepairBox(create_cell_item(upgradedItem), true);
        CUICellItem* cellItem = create_cell_item(upgradedItem);
        m_uidata->UIRepairBox.ClearAll(true);
        m_uidata->UIRepairBox.SetItem(cellItem);
    }
    else if (pWnd == &m_uidata->UIUpgrade8 && msg == BUTTON_CLICKED)
    {
        PIItem item = (PIItem)m_uidata->UIRepairBox.GetItemIdx(0)->m_pData;
        PIItem upgradedItem = upgrader.UpgradeItem(item, upgradeBySlot[7]);

        ToRepairBox(create_cell_item(upgradedItem), true);
        CUICellItem* cellItem = create_cell_item(upgradedItem);
        m_uidata->UIRepairBox.ClearAll(true);
        m_uidata->UIRepairBox.SetItem(cellItem);
    }
    else if (pWnd == &m_uidata->UIUpgrade9 && msg == BUTTON_CLICKED)
    {
        PIItem item = (PIItem)m_uidata->UIRepairBox.GetItemIdx(0)->m_pData;
        PIItem upgradedItem = upgrader.UpgradeItem(item, upgradeBySlot[8]);

        ToRepairBox(create_cell_item(upgradedItem), true);
        CUICellItem* cellItem = create_cell_item(upgradedItem);
        m_uidata->UIRepairBox.ClearAll(true);
        m_uidata->UIRepairBox.SetItem(cellItem);
    }
    else if (pWnd == &m_uidata->UIUpgrade10 && msg == BUTTON_CLICKED)
    {
        PIItem item = (PIItem)m_uidata->UIRepairBox.GetItemIdx(0)->m_pData;
        PIItem upgradedItem = upgrader.UpgradeItem(item, upgradeBySlot[9]);

        ToRepairBox(create_cell_item(upgradedItem), true);
        CUICellItem* cellItem = create_cell_item(upgradedItem);
        m_uidata->UIRepairBox.ClearAll(true);
        m_uidata->UIRepairBox.SetItem(cellItem);
    }

    CUIWindow::SendMessage(pWnd, msg, pData);
}

bool CUIUpgradeWnd::OnKeyboard(int dik, EUIMessages keyboard_action)
{
    if (dik == DIK_ESCAPE && keyboard_action == EUIMessages::WINDOW_KEY_PRESSED)
    {
        if (m_uidata->UIRepairBox.ItemsCount() > 0)
        {
            ToOurBag(m_uidata->UIRepairBox.GetItemIdx(0));
        }
        SwitchToTalk();
    }

    if (inherited::OnKeyboard(dik, keyboard_action))
        return true;

    return false;
}

bool CUIUpgradeWnd::OnMouse(float x, float y, EUIMessages mouse_action)
{
    CUIWindow::OnMouse(x, y, mouse_action);

    return true;
}