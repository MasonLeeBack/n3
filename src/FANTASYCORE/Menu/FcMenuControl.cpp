#include "stdafx.h"

#include "FcMenuControl.h"
#include "BSuiSystem.h"

#include "BsUiClassIDManager.h"
#include "BsUiFontAttrManager.h"

#include "FcInterfaceManager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG



CFcMenuControl::CFcMenuControl(_FC_MENU_TYPE nType)
: CFcMenuForm(nType)
{
	m_pLayer = new CFcMenuControlLayer(this);
	AddUiLayer(m_pLayer, SN_Control, -1, -1);
}


void CFcMenuControl::RenderProcess()
{
	switch(GetStatus())
	{
	case _MS_OPEN:
		{
			m_pLayer->SetColor(D3DXCOLOR(1, 1, 1, 0.f));

			SetStatus(_MS_OPENING);
			break;
		}
	case _MS_OPENING:
		{
			int nTick = g_BsUiSystem.GetMenuTick() - m_nFormTick;
			RenderMoveLayer(nTick);

			if(nTick >= _TICK_END_OPENING_CLOSING){
				SetStatus(_MS_NORMAL);
			}
			break;
		}
	case _MS_NORMAL:	break;
	case _MS_CLOSING:
		{
			int nTick = g_BsUiSystem.GetMenuTick() - m_nFormTick;
			RenderMoveLayer(_TICK_END_OPENING_CLOSING - nTick);

			if(nTick >= _TICK_END_OPENING_CLOSING){
				SetStatus(_MS_CLOSE);
			}
			break;
		}
	case _MS_CLOSE:	break;
	}
}

void CFcMenuControl::RenderMoveLayer(int nTick)
{
	if(nTick <= _TICK_FADE_IN_BASE)
	{
		float fAlpha = (float)nTick / _TICK_FADE_IN_BASE;
		m_pLayer->SetColor(D3DXCOLOR(1, 1, 1, fAlpha));
	}
}

//-----------------------------------------------------------------------------------------------------
DWORD CFcMenuControlLayer::ProcMessage(xwMessageToken* pMsgToken)
{
	BsAssert(pMsgToken);

	BsUiCLASS hClass = BsUi::BsUiGetClass(pMsgToken->hWnd);

	switch(pMsgToken->message)
	{
	case XWMSG_KEYDOWN:
		{
			if(pMsgToken->lParam == MENU_INPUT_A)
			{
				;
			}
			else if(pMsgToken->lParam == MENU_INPUT_B)
			{
				m_pMenu->SetStatus(_MS_CLOSE);
				g_MenuHandle->PostMessage(fcMSG_PAUSE_START);
			}
			break;
		}
	}

	return BsUiWindow::ProcMessage(pMsgToken);
}
