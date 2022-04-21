///////////////////////////////////////////////////////////////
// StalkerOutfit.cpp
// StalkerOutfit - защитный костюм сталкера
///////////////////////////////////////////////////////////////


#pragma once

#include "customhelmet.h"
#include "script_export_space.h"

class CStalkerHelmet : public CCustomHelmet {
private:
    typedef	CCustomHelmet inherited;
public:
	CStalkerHelmet(void);
	virtual ~CStalkerHelmet(void);

	DECLARE_SCRIPT_REGISTER_FUNCTION
};
add_to_type_list(CStalkerHelmet)
#undef script_type_list
#define script_type_list save_type_list(CStalkerHelmet)
