#pragma once

#include "BsUiLayer.h"
#include "FcMenuForm.h"

class CFcMenuLogo : public CFcMenuForm
{
public:
	CFcMenuLogo(_FC_MENU_TYPE nType);

	void RenderProcess();

	void SetCloseLogoDeveloper(bool bSet)	{ m_bCloseLogoDeveloper = bSet; }

protected:
	void SetNextLogoDeveloper();

protected:
	int			m_ntime;
	bool		m_bCloseLogoDeveloper;
};


class CFcMenuLogoPublisherLayer : public BsUiLayer
{
public:
	CFcMenuLogoPublisherLayer(CFcMenuForm* pMenu);
	virtual DWORD	ProcMessage(xwMessageToken* pMsgToken);

protected:
	CFcMenuForm* m_pMenu;
};


class CFcMenuLogoDeveloperLayer : public BsUiLayer
{
public:
	CFcMenuLogoDeveloperLayer(CFcMenuForm* pMenu);
	void			Initialize();
	virtual DWORD	ProcMessage(xwMessageToken* pMsgToken);

protected:
	CFcMenuForm* m_pMenu;
};
