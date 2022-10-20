#pragma once

#include "BsUiLayer.h"
#include "FcMenuForm.h"

class CFcMenuControlLayer;
class CFcMenuControl : public CFcMenuForm
{
public:
	CFcMenuControl(_FC_MENU_TYPE nType);
	virtual void	RenderProcess();

protected:
	void RenderMoveLayer(int nTick);

protected:
	CFcMenuControlLayer* m_pLayer;
};



class CFcMenuControlLayer : public BsUiLayer
{
public:
	CFcMenuControlLayer(CFcMenuControl* pMenu)	{ m_pMenu = pMenu; }
	virtual DWORD	ProcMessage(xwMessageToken* pMsgToken);

protected:
	CFcMenuControl* m_pMenu;
};