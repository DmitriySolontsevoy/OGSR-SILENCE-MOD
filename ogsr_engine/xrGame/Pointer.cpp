///////////////////////////////////////////////////////////////
// Pointer.cpp
// Pointer - апгрейд оружия
///////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "pointer.h"
// #include "PhysicsShell.h"

CPointer::CPointer() {}

CPointer::~CPointer() {}

BOOL CPointer::net_Spawn(CSE_Abstract* DC) { return (inherited::net_Spawn(DC)); }

void CPointer::Load(LPCSTR section) { inherited::Load(section); }

void CPointer::net_Destroy() { inherited::net_Destroy(); }

void CPointer::UpdateCL() { inherited::UpdateCL(); }

void CPointer::OnH_A_Chield() { inherited::OnH_A_Chield(); }

void CPointer::OnH_B_Independent(bool just_before_destroy) { inherited::OnH_B_Independent(just_before_destroy); }

void CPointer::renderable_Render() { inherited::renderable_Render(); }