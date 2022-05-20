#pragma once
#include "UIWindow.h"
#include "../inventory_space.h"
#include <Upgrade.h>
#include "UIStatic.h"

class CInventoryOwner;
class CEatableItem;
class CTrade;
class CUIPropertiesBox;

struct CUIUpgradeInternal;

class CUIDragDropListEx;
class CUICellItem;

class CUIUpgradeWnd : public CUIWindow
{
private:
	typedef CUIWindow inherited;

	std::map<int, std::string> upgradeBySlot;
public:
	CUIUpgradeWnd();
	virtual				~CUIUpgradeWnd();

	virtual void		Init();

	virtual void		SendMessage(CUIWindow* pWnd, s16 msg, void* pData);

	void				InitUpgrade(CInventoryOwner* pOur, CInventoryOwner* pOthers);

	virtual void 		Update();
	virtual void 		Show();
	virtual void 		Hide();

	void 				DisableAll();
	void 				EnableAll();
	virtual bool		OnKeyboard(int dik, EUIMessages keyboard_action);
	virtual bool		OnMouse(float x, float y, EUIMessages mouse_action);

	void 				SwitchToTalk();

	virtual void 		StartUpgrade();
	virtual void 		StopUpgrade();
protected:
	CUIUpgradeInternal* m_uidata;

	CUpgrade			upgrader;

	bool				bStarted;
	bool				needsAnUpdate;

	bool 				ToOurBag(CUICellItem* itm);
	bool 				ToRepairBox(CUICellItem* itm, bool upgraded);

	void				ColorizeItem(CUICellItem* itm, bool canTrade, bool highlighted);
	bool				MoveItem(CUICellItem* itm);

	enum EListType { eNone, e1st, e2nd, eBoth };

	void				UpdateLists();

	void				FillList(TIItemContainer&, CUIDragDropListEx&, bool);

	bool				m_bDealControlsVisible;

	bool				CanMoveToOther(PIItem, bool);

	//указатели игрока и того с кем торгуем
	CInventory* m_pInv;
	CInventory* m_pOthersInv;
	CInventoryOwner* m_pInvOwner;
	CInventoryOwner* m_pOthersInvOwner;

public:
	CUICellItem* m_pCurrentCellItem;
protected:
	TIItemContainer		ruck_list;

	void				SetCurrentItem(CUICellItem* itm);
	CUICellItem*		CurrentItem();
	PIItem				CurrentIItem();

	bool				OnItemDrop(CUICellItem* itm);
	bool				OnItemStartDrag(CUICellItem* itm);
	bool				OnItemDbClick(CUICellItem* itm);
	bool				OnItemSelected(CUICellItem* itm);
	bool				OnItemRButtonClick(CUICellItem* itm);

	void				BindDragDropListEvents(CUIDragDropListEx* lst);
};
