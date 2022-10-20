#include "stdafx.h"

#include "FcMenuLogo.h"
#include "BSuiSystem.h"

#include "BsUiClassIDManager.h"
#include "BsUiFontAttrManager.h"

#include "FcInterfaceManager.h"
#include "FcGlobal.h"

#define LOGO_TIME			3 * 40


#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG


CFcMenuLogo::CFcMenuLogo(_FC_MENU_TYPE nType)
: CFcMenuForm(nType)
{	
	if(g_FCGameData.bPlayMovie == false)
	{
		CFcMenuLogoPublisherLayer* pLayer = new CFcMenuLogoPublisherLayer(this);
		AddUiLayer(pLayer, SN_LogoPublisher, -1, -1);
		pLayer->GetWindow(IM_MS)->SetColor(D3DXCOLOR(1, 1, 1, 0));
	}
	else
	{
		CFcMenuLogoDeveloperLayer* pLayer = new CFcMenuLogoDeveloperLayer(this);
		AddUiLayer(pLayer, SN_LogoDeveloper, -1, -1);
		pLayer->GetWindow(IM_LogoDev)->SetColor(D3DXCOLOR(1, 1, 1, 0));
	}

	m_ntime = g_BsUiSystem.GetMenuTick();
	m_bCloseLogoDeveloper = false;

	SetStatus(_MS_NORMAL);
}


void CFcMenuLogo::RenderProcess()
{
	int ntime = g_BsUiSystem.GetMenuTick();

	BsUiHANDLE hLayer1 = GetUiLayer(SN_LogoPublisher);
	BsUiHANDLE hLayer2 = GetUiLayer(SN_LogoDeveloper);

	int nTick = m_ntime + LOGO_TIME - ntime;
	
	float fAlpha = 0.f;
	if(nTick < 40){
		fAlpha = nTick * 0.025f;
	}
	else if(nTick > 80){
		fAlpha = 1.f - ((nTick - 80) * 0.025f);
	}
	else{
		fAlpha = 1.f;
	}

	D3DXCOLOR clr = D3DXCOLOR(1, 1, 1, fAlpha);

	if(hLayer1 != NULL)
	{	
		BsUi::BsUiGetWindow(hLayer1)->GetWindow(IM_MS)->SetColor(clr);

		if(m_ntime + LOGO_TIME < ntime)
		{
			RemoveUiLayer(hLayer1);

			CFcMenuLogoDeveloperLayer* pLayer = new CFcMenuLogoDeveloperLayer(this);
			AddUiLayer(pLayer, SN_LogoDeveloper, -1, -1);
			pLayer->Initialize();

			m_ntime = ntime;
		}
	}
	else if(hLayer2 != NULL)
	{
		BsUi::BsUiGetWindow(hLayer2)->GetWindow(IM_LogoDev)->SetColor(clr);

		if(m_ntime + LOGO_TIME < ntime)
		{
			RemoveUiLayer(hLayer2);
			g_MenuHandle->PostMessage(fcMSG_OPENING_START);
		}
	}

	if(m_bCloseLogoDeveloper == true){
		SetNextLogoDeveloper();
	}
}

void CFcMenuLogo::SetNextLogoDeveloper()
{
	BsUiHANDLE hLayer1 = GetUiLayer(SN_LogoPublisher);
	if(hLayer1 == NULL){
		return;
	}

	RemoveUiLayer(hLayer1);

	CFcMenuLogoDeveloperLayer* pLayer = new CFcMenuLogoDeveloperLayer(this);
	AddUiLayer(pLayer, SN_LogoDeveloper, -1, -1);
	pLayer->Initialize();

	m_ntime = g_BsUiSystem.GetMenuTick();
}

//-----------------------------------------------------------------------------------------------------
CFcMenuLogoPublisherLayer::CFcMenuLogoPublisherLayer(CFcMenuForm* pMenu)
{
	m_pMenu = pMenu;
}

DWORD CFcMenuLogoPublisherLayer::ProcMessage(xwMessageToken* pMsgToken)
{
	BsAssert(pMsgToken);

	switch(pMsgToken->message)
	{
	case XWMSG_KEYDOWN:
		{	
			switch (pMsgToken->lParam)
			{
			case MENU_INPUT_B:
				{	
					break;
				}
			case MENU_INPUT_START:
			case MENU_INPUT_A:
				{
					((CFcMenuLogo*)m_pMenu)->SetCloseLogoDeveloper(true);
					break;
				}
			}
			
			break;
		}
	case XWMSG_BN_CLICKED:
		{
			break;
		}
	}

	return BsUiWindow::ProcMessage(pMsgToken);
}

//-----------------------------------------------------------------------------------------------------
CFcMenuLogoDeveloperLayer::CFcMenuLogoDeveloperLayer(CFcMenuForm* pMenu)
{
	m_pMenu = pMenu;
}

void CFcMenuLogoDeveloperLayer::Initialize()
{
	GetWindow(IM_LogoDev)->SetColor(D3DXCOLOR(1, 1, 1, 0));
}

DWORD CFcMenuLogoDeveloperLayer::ProcMessage(xwMessageToken* pMsgToken)
{
	BsAssert(pMsgToken);

	switch(pMsgToken->message)
	{
	case XWMSG_KEYDOWN:
		{	
			switch (pMsgToken->lParam)
			{
			case MENU_INPUT_B:
				{	
					break;
				}
			case MENU_INPUT_START:
			case MENU_INPUT_A:
				{
					g_MenuHandle->PostMessage(fcMSG_OPENING_START);
					break;
				}
			}

			break;
		}
	case XWMSG_BN_CLICKED:
		{
			break;
		}
	}

	return BsUiWindow::ProcMessage(pMsgToken);
}