#pragma once

#include "BsUiLayer.h"
#include "FcMenuForm.h"

class CFcMenuMainOpening : public CFcMenuForm
{
public:
	CFcMenuMainOpening(_FC_MENU_TYPE nType);
	void RenderProcess();

protected:
	int			m_ntime;
};
