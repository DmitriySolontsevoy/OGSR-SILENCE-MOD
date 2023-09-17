#include "stdafx.h"
#include "StalkerHelmet.h"

CStalkerHelmet::CStalkerHelmet() {}

CStalkerHelmet::~CStalkerHelmet() {}

using namespace luabind;

#pragma optimize("s", on)
void CStalkerHelmet::script_register(lua_State* L) { module(L)[class_<CStalkerHelmet, CGameObject>("CStalkerHelmet").def(constructor<>())]; }
