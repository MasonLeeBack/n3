#include "stdafx.h"

#include "FcMenuLibrary.h"
#include "BSuiSystem.h"

#include "BsUiClassIDManager.h"
#include "BsUiFontAttrManager.h"

#include "FcInterfaceManager.h"

#include "FcGlobal.h"
#include "FcSoundManager.h"
#include "FcSOXLoader.h"

#include "FantasyCore.h"
#include "FcLiveManager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG

CFcMenuLibrary::CFcMenuLibrary(_FC_MENU_TYPE nType, _FC_MENU_TYPE nPrevType)
: CFcMenuForm(nType)
{
	m_pBaseLayer = new BsUiLayer();
	AddUiLayer(m_pBaseLayer, SN_Library0, -1, -1);

	m_pLibLayer = new CFcMenuLibLayer(this);
	AddUiLayer(m_pLibLayer, SN_Library1, -1, -1);

	m_pPointLayer = new BsUiLayer();
	AddUiLayer(m_pPointLayer, SN_Library2, -1, -1);

	m_pNewLayer = new CFcMenuLibNewLayer(this);
	AddUiLayer(m_pNewLayer, SN_Library3, -1, -1);

	UpdateLibLayer(nPrevType);
	UpdateEtcLayer(nPrevType);
}

void CFcMenuLibrary::UpdateLibLayer(_FC_MENU_TYPE nPrevType)
{
	m_nPrevType = nPrevType;

	CFcMenuLibraryManager* pLibMgr = g_MenuHandle->GetLibManager();

	if(pLibMgr->IsNewItemInSection(LIB_CHAR) == true){	
		m_pLibLayer->GetWindow(TT_LibChar)->SetItemText("@(New_icon)");
	}

	if(pLibMgr->IsNewItemInSection(LIB_MISSION) == true){	
		m_pLibLayer->GetWindow(TT_LibMission)->SetItemText("@(New_icon)");
	}

	if(pLibMgr->IsNewItemInSection(LIB_ART) == true){	
		m_pLibLayer->GetWindow(TT_LibArtwork)->SetItemText("@(New_icon)");
	}

	switch(nPrevType)
	{
	case _FC_MENU_TITLE:
	case _FC_MENU_LIB_CHAR:		m_pLibLayer->SetFocusWindowClass(BT_LibChar); break;
	case _FC_MENU_LIB_MISSION:	m_pLibLayer->SetFocusWindowClass(BT_LibMission); break;
	case _FC_MENU_LIB_ART:		m_pLibLayer->SetFocusWindowClass(BT_LibArtwork); break;
	}
}


void CFcMenuLibrary::UpdateEtcLayer(_FC_MENU_TYPE nPrevType)
{
	char cPoint[256];
	sprintf(cPoint, "%d", g_FCGameData.tempFixedSave.nTotalPoint);
	m_pPointLayer->GetWindow(TT_Library_point_num)->SetItemText(cPoint);

	if(nPrevType == _FC_MENU_TITLE)
	{
		CFcMenuLibraryManager* pLibMgr = g_MenuHandle->GetLibManager();
		if(pLibMgr->IsFirstNewItem())
		{
			m_pNewLayer->SetWindowAttr(XWATTR_SHOWWINDOW, true);
			g_BsUiSystem.SetFocusWindow(m_pNewLayer);

			return;
		}
	}

	
	m_pNewLayer->SetWindowAttr(XWATTR_SHOWWINDOW, false);
}

void CFcMenuLibrary::RenderProcess()
{
	switch(GetStatus())
	{
	case _MS_OPEN:
		{
			m_pBaseLayer->SetColor(D3DXCOLOR(1, 1, 1, 0.f));
			m_pLibLayer->SetColor(D3DXCOLOR(1, 1, 1, 0.f));
			m_pPointLayer->SetColor(D3DXCOLOR(1, 1, 1, 0.f));
			m_pNewLayer->SetColor(D3DXCOLOR(1, 1, 1, 0.f));

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
	case _MS_NORMAL: break;
	case _MS_CLOSING:
		{
			int nTick = g_BsUiSystem.GetMenuTick() - m_nFormTick;
			RenderMoveLayer(_TICK_END_OPENING_CLOSING - nTick);

			if(nTick >= _TICK_END_OPENING_CLOSING){
				SetStatus(_MS_CLOSE);
			}
			break;
		}
	case _MS_CLOSE:
		{
			if(g_FC.GetLiveManager()->IsUserSignedIn() && g_FCGameData.m_DeviceID != XCONTENTDEVICE_ANY ){
				g_FCGameData.SaveFixedGameData();
			}
			break;
		}
	}
}


#define _START_POS_X_LIB1				-640
#define _END_POS_X_LIB1					0
#define _START_POS_X_LIB2				1280
#define _END_POS_X_LIB2					640
void CFcMenuLibrary::RenderMoveLayer(int nTick)
{
	//------------------------------------------------------
	//base
	if(nTick <= _TICK_FADE_IN_BASE)
	{
		float fAlpha = (float)nTick / _TICK_FADE_IN_BASE;
		m_pBaseLayer->SetColor(D3DXCOLOR(1, 1, 1, fAlpha));
		m_pNewLayer->SetColor(D3DXCOLOR(1, 1, 1, fAlpha));
	}

	//------------------------------------------------------
	//grd 1
	if(nTick <= _TICK_IN_FORM_LEFT)
	{	
		float frate = 1.f - ((float)nTick / _TICK_IN_FORM_LEFT);
		int nX = (int)(_START_POS_X_LIB1 * frate);
		m_pLibLayer->SetWindowPosX(nX);

		float fAlpha = 1.f - frate;
		m_pLibLayer->SetColor(D3DXCOLOR(1, 1, 1, fAlpha));
	}

	//------------------------------------------------------
	//point
	if(nTick <= _TICK_IN_FORM_RIGHT)
	{
		float frate = (float)nTick / _TICK_IN_FORM_RIGHT;
		int nX = _END_POS_X_LIB2 + int((_START_POS_X_LIB2 - _END_POS_X_LIB2) * (1.f - frate));
		m_pPointLayer->SetWindowPosX(nX);

		float fAlpha = frate;
		m_pPointLayer->SetColor(D3DXCOLOR(1, 1, 1, fAlpha));
	}
}


//-----------------------------------------------------------------------------------------------------
DWORD CFcMenuLibLayer::ProcMessage(xwMessageToken* pMsgToken)
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
					m_pMenu->SetStatus(_MS_CLOSING);
					g_MenuHandle->PostMessage(fcMSG_TITLE_START);
					break;
				}
			case MENU_INPUT_UP:
				{
					BsUiWindow* pPrev = SetFocusPrevChileWindow(pMsgToken->hWnd, true);
					if(pPrev != NULL && pPrev->GetHWnd() != pMsgToken->hWnd){
						g_pSoundManager->PlaySystemSound( SB_COMMON,"SYS_CURSOR");
					}
					break;
				}	
			case MENU_INPUT_DOWN:
				{
					BsUiWindow* pNext = SetFocusNextChileWindow(pMsgToken->hWnd, true);
					if(pNext != NULL && pNext->GetHWnd() != pMsgToken->hWnd){
						g_pSoundManager->PlaySystemSound( SB_COMMON,"SYS_CURSOR");
					}
					break;
				}
			case MENU_INPUT_A:
				{
					m_pMenu->SetStatus(_MS_CLOSING);

					BsUiCLASS hClass = BsUi::BsUiGetClass(pMsgToken->hWnd);
					switch(hClass)
					{
					case BT_LibChar:		g_MenuHandle->PostMessage(fcMSG_LIB_CHAR_START); break;
					case BT_LibMission:		g_MenuHandle->PostMessage(fcMSG_LIB_MISSION_START); break;
					case BT_LibArtwork:		g_MenuHandle->PostMessage(fcMSG_LIB_ART_START); break;
					}
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
//------------------------------------------------------------------------------
DWORD CFcMenuLibNewLayer::ProcMessage(xwMessageToken* pMsgToken)
{
	BsAssert(pMsgToken);
	switch(pMsgToken->message)
	{
	case XWMSG_KEYDOWN:
		{
			switch (pMsgToken->lParam)
			{
			case MENU_INPUT_B:
			case MENU_INPUT_A:
				{
					g_BsUiSystem.SetFocusWindow(m_pMenu->m_pLibLayer);
					SetWindowAttr(XWATTR_SHOWWINDOW, false);
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


