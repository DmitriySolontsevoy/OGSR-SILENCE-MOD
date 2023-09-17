///////////////////////////////////////////////////////////////
// Foregrip.cpp
// Foregrip - апгрейд оружия
///////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "foregrip.h"
// #include "PhysicsShell.h"

CForegrip::CForegrip() {}

CForegrip::~CForegrip() {}

BOOL CForegrip::net_Spawn(CSE_Abstract* DC) { return (inherited::net_Spawn(DC)); }

void CForegrip::Load(LPCSTR section) { inherited::Load(section); }

void CForegrip::net_Destroy() { inherited::net_Destroy(); }

void CForegrip::UpdateCL() { inherited::UpdateCL(); }

void CForegrip::OnH_A_Chield() { inherited::OnH_A_Chield(); }

void CForegrip::OnH_B_Independent(bool just_before_destroy) { inherited::OnH_B_Independent(just_before_destroy); }

void CForegrip::renderable_Render() { inherited::renderable_Render(); }