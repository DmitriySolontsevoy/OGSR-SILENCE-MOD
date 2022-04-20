#pragma once

#include "inventory_item.h"

class CPhysicItem;
class CEntityAlive;

class CEatableItem : public CInventoryItem {
	friend class CEatableItemScript;
private:
	typedef CInventoryItem	inherited;

private:
	CPhysicItem		*m_physic_item;

public:
							CEatableItem				();
	virtual					~CEatableItem				();
	virtual	DLL_Pure*		_construct					();
	virtual CEatableItem	*cast_eatable_item			()	{return this;}

	virtual void			Load						(LPCSTR section);
	virtual bool			Useful						() const;

	virtual BOOL			net_Spawn					(CSE_Abstract* DC);

	virtual void			OnH_B_Independent			(bool just_before_destroy);
	virtual	void			UseBy						(CEntityAlive* npc);
			bool			Empty						()	const				{return m_iPortionsNum==0;};
	virtual	void			ZeroAllEffects				();
			void			SetRadiation				(float rad);
protected:	
	//влияние при поедании вещи на параметры игрока
	float					m_fHealthInfluence;
	float					m_fPowerInfluence;
	float					m_fSatietyInfluence;
	float					m_fRadiationInfluence;
	float					m_fMaxPowerUpInfluence{};
	float					m_fPsyHealthInfluence;
	float					m_fThirstInfluence{};
	//заживление ран на кол-во процентов
	float					m_fWoundsHealPerc{};

    //бустер-параметры
    float                   m_fHealthBoostTime;
    float                   m_fHealthBoostValue;

    float                   m_fPsyHealthBoostTime;
    float                   m_fPsyHealthBoostValue;

    float                   m_fPowerBoostTime;
    float                   m_fPowerBoostValue;

    float                   m_fRadiationBoostTime;
    float                   m_fRadiationBoostValue;

    float                   m_fBleedingBoostTime;
    float                   m_fBleedingBoostValue;

    float                   m_fMaxWeightBoostTime;
    float                   m_fMaxWeightBoostValue;

    float                   m_fRadiationImmunityBoostTime;
    float                   m_fRadiationImmunityBoostValue;

    float                   m_fChemicalImmunityBoostTime;
    float                   m_fChemicalImmunityBoostValue;

    float                   m_fThermalImmunityBoostTime;
    float                   m_fThermalImmunityBoostValue;

    float                   m_fElectricImmunityBoostTime;
    float                   m_fElectricImmunityBoostValue;

    float                   m_fPsychicImmunityBoostTime;
    float                   m_fPsychicImmunityBoostValue;

	//количество порций еды, 
	//-1 - порция одна и больше не бывает (чтоб не выводить надпись в меню)
	int						m_iPortionsNum;
	int						m_iStartPortionsNum{};
};

