#include "stdafx.h"

#include "FcMenuUsual.h"
#include "BSuiSystem.h"

#include "BsUiClassIDManager.h"
#include "BsUiFontAttrManager.h"
#include "FantasyCore.h"
#include "FcInterfaceManager.h"
#include "FcMessageDef.h"

#include "FcSoundManager.h"


#ifdef _XBOX
#include "atgsignin.h"
#include "FcLiveManager.h"
#endif //_XBOX

#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG



CFcMenuUsual::CFcMenuUsual(_FC_MENU_TYPE nType)
: CFcMenuForm(nType)
{
	m_pUsual = new CFcMenuUsualLayer(this);
	AddUiLayer(m_pUsual, SN_Usual, -1, -1);
	g_FCGameData.bCheckFreeSpace = false;
	m_pUsual->Initialize();	
}

CFcMenuUsual::~CFcMenuUsual()
{
	g_FCGameData.bCheckFreeSpace = true;
}

void CFcMenuUsual::Reinit()
{
	g_FCGameData.bCheckFreeSpace = false;
	m_pUsual->Initialize();
	
}


void CFcMenuUsual::SetFocus(int nMenuType)
{
	switch(nMenuType)
	{
	case _FC_MENU_NONE:			m_pUsual->SetFocusWindowClass(BT_UsualContinue); break;
	case _FC_MENU_SAVEDATA:		m_pUsual->SetFocusWindowClass(BT_UsualSave); break;
	case _FC_MENU_LOADDATA:		m_pUsual->SetFocusWindowClass(BT_UsualLoad); break;
	case _FC_MENU_OPTION:		m_pUsual->SetFocusWindowClass(BT_UsualOption); break;
	}
}

void CFcMenuUsual::ProcessMsg(int nMsgAsk, BsUiHANDLE hWnd, int nMsgRsp)
{
	if(nMsgAsk == fcMT_ExitMissionToTitle)
	{
		if(nMsgRsp == ID_YES){
			g_MenuHandle->PostMessage(fcMSG_GO_TO_TITLE);
		}
		else
		{
			g_BsUiSystem.SetFocusWindow(BsUi::BsUiGetWindow(hWnd));

			BsUiHANDLE hLayer = GetUiLayer(SN_Usual);
			BsUiLayer* pLayer = (BsUiLayer*)BsUi::BsUiGetWindow(hLayer);
			pLayer->SetWindowAttr(XWATTR_SHOWWINDOW, true);
		}
	}else if( nMsgAsk == fcMT_DiskFree )
	{
		if(nMsgRsp == ID_YES)
		{
			if(g_FCGameData.m_DeviceID != XCONTENTDEVICE_ANY
				&& g_FCGameData.bFixedSaveData == true )				
			{
				g_MenuHandle->PostMessage(fcMSG_MESSAGEBOX_START,
					fcMT_ProfileDelete,	// yes, no
					(DWORD)this, NULL);
			}
		}
		else
		{
            g_FCGameData.bShowDeviceUI = false;
		}
	}
	else if( nMsgAsk == fcMT_ProfileDelete )
	{
		if(nMsgRsp == ID_YES)
		{ 			
		}
		else		
		{
            g_FCGameData.bShowDeviceUI = false;
			g_FCGameData.bChangeDevice = true;
		}
	}
}

void CFcMenuUsual::Process() 
{
#ifdef _XBOX
	if(g_MenuHandle->GetCurMenuType() == _FC_MENU_USUAL)
	{
		if( g_FCGameData.m_OldDeviceID != g_FCGameData.m_DeviceID
			&& g_FC.GetMainState() != GAME_STATE_SYSTEM_MESSAGE ) 
		{
			g_FCGameData.m_OldDeviceID = g_FCGameData.m_DeviceID;

			if(g_FCGameData.UpdateGetContentData()==false) {
				g_MenuHandle->PostMessage(fcMSG_MESSAGEBOX_START,
					fcMT_FailEnumerateContents,	// ok
					(DWORD)this, NULL);
			}        

			// if a selected storage device has not enough space to save a gamedata, send a message to user "is it ok?"
			if(g_FCGameData.m_DeviceID != XCONTENTDEVICE_ANY)
			{
				bool bConfirmDevice = true;
				XDEVICE_DATA  data={0,};
				if(ERROR_SUCCESS != XContentGetDeviceData(g_FCGameData.m_DeviceID, &data))
				{
					// 중간에 디바이스 장치가 제거 되었거나 기타 오류 처리
					return;
				}		

				if( data.ulDeviceFreeBytes < STORAGE_ENOUGHSIZE)		
				{
					g_MenuHandle->PostMessage(fcMSG_MESSAGEBOX_START,
					fcMT_DiskFree,	// yes, no
					(DWORD)this, NULL);
					bConfirmDevice = false;
				}

				if(g_FCGameData.bFixedSaveData == true && bConfirmDevice)			
				{
					g_MenuHandle->PostMessage(fcMSG_MESSAGEBOX_START,
					fcMT_ProfileDelete,	// yes, no
					(DWORD)this, NULL);
				}
			}        

			
			BsUiWindow* pWnd = g_BsUiSystem.GetFocusWindow();
			m_pUsual->Initialize();	
			g_BsUiSystem.SetFocusWindow(pWnd);	
			g_FCGameData.m_OldDeviceID = g_FCGameData.m_DeviceID;
		}

		if(g_FCGameData.bRemovedStorageDevice && g_FC.GetMainState() != GAME_STATE_SYSTEM_MESSAGE )
		{
			CFcMenuForm* pMsgBoxMenu = g_MenuHandle->GetMessageBoxMenu(fcMT_DiskFree);
			if(pMsgBoxMenu!=NULL)
				g_MenuHandle->PostMessage(fcMSG_MESSAGEBOX_END,
				(DWORD)pMsgBoxMenu,
				(DWORD)g_FC.GetMainState(), //(DWORD)m_dwMainState,
				(DWORD)this);				//(DWORD)m_pCmdMenu,

			g_MenuHandle->PostMessage(fcMSG_MESSAGEBOX_START,
				fcMT_RemoveStorage,	// yesno
				(DWORD)this, NULL);
			g_FCGameData.bRemovedStorageDevice = false;
		}
		if(g_FCGameData.bCancelDeviceSelector )
		{
			g_MenuHandle->PostMessage(fcMSG_MESSAGEBOX_START,
				fcMT_CancelDeviceSelector,	// yesno
				(DWORD)this, NULL);
			g_FCGameData.bCancelDeviceSelector = false;		
		}
	}

#endif // _XBOX
}

void CFcMenuUsual::RenderProcess()
{
	switch(GetStatus())
	{
	case _MS_OPEN:
		{
			m_pUsual->SetColor(D3DXCOLOR(1, 1, 1, 0.f));

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


void CFcMenuUsual::RenderMoveLayer(int nTick)
{
	if(nTick <= _TICK_FADE_IN_BASE)
	{
		float fAlpha = (float)nTick / _TICK_FADE_IN_BASE;
		m_pUsual->SetColor(D3DXCOLOR(1, 1, 1, fAlpha));
	}
}

//-----------------------------------------------------------------------------------------------------
void CFcMenuUsualLayer::Initialize()
{
	GetWindow(BT_UsualLoad)->SetWindowAttr(XWATTR_DISABLE, false);
	GetWindow(BT_UsualSave)->SetWindowAttr(XWATTR_DISABLE, false);
	if(g_FCGameData.GetUserSaveGameCount() == 0 ){
		GetWindow(BT_UsualLoad)->SetWindowAttr(XWATTR_DISABLE, true);
	}

	// check signed-in 
#ifdef _XBOX
	if(g_FC.GetLiveManager()->IsUserSignedIn()==false || g_FCGameData.m_DeviceID == XCONTENTDEVICE_ANY) 
	{
		GetWindow(BT_UsualLoad)->SetWindowAttr(XWATTR_DISABLE, true);
		GetWindow(BT_UsualSave)->SetWindowAttr(XWATTR_DISABLE, true);
	}
#endif //_XBOX

}

DWORD CFcMenuUsualLayer::ProcMessage(xwMessageToken* pMsgToken)
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
					g_MenuHandle->PostMessage(fcMSG_USUAL_END);
					break;
				}
			case MENU_INPUT_START:
				{
					if(g_bIsStartTitleMenu)
					{
						g_MenuHandle->PostMessage(fcMSG_USUAL_END);
					}
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
			default:
				{
					BsUiCLASS hClass = BsUi::BsUiGetClass(pMsgToken->hWnd);
					switch(hClass)
					{
					case BT_UsualContinue:  OnKeyDownBtContinue(pMsgToken); break;
					case BT_UsualSave:	    OnKeyDownBtSave(pMsgToken); break;
					case BT_UsualLoad:	    OnKeyDownBtLoad(pMsgToken); break;
					case BT_UsualOption:    OnKeyDownBtOption(pMsgToken); break;
					case BT_UsualExit:      OnKeyDownBtExit(pMsgToken); break;
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


void CFcMenuUsualLayer::OnKeyDownBtContinue(xwMessageToken* pMsgToken)
{
	switch (pMsgToken->lParam)
	{
	case MENU_INPUT_A:
		{	
			g_MenuHandle->PostMessage(fcMSG_USUAL_END);
			break;
		}
	}
}

void CFcMenuUsualLayer::OnKeyDownBtSave(xwMessageToken* pMsgToken)
{
	switch (pMsgToken->lParam)
	{
	case MENU_INPUT_A:
		{	
			g_MenuHandle->PostMessage(fcMSG_USUAL_TO_SAVEDATA_START);
			break;
		}
	}
}

void CFcMenuUsualLayer::OnKeyDownBtLoad(xwMessageToken* pMsgToken)
{
	switch (pMsgToken->lParam)
	{
	case MENU_INPUT_A:
		{	
			g_MenuHandle->PostMessage(fcMSG_USUAL_TO_LOAD_START);
			break;
		}
	}
}

void CFcMenuUsualLayer::OnKeyDownBtOption(xwMessageToken* pMsgToken)
{
	switch (pMsgToken->lParam)
	{
	case MENU_INPUT_A:
		{
			g_MenuHandle->PostMessage(fcMSG_USUAL_TO_OPTION_START);
			break;
		}
	}
}

void CFcMenuUsualLayer::OnKeyDownBtExit(xwMessageToken* pMsgToken)
{
	switch (pMsgToken->lParam)
	{
	case MENU_INPUT_A:
		{
			g_MenuHandle->PostMessage(fcMSG_MESSAGEBOX_START,
				fcMT_ExitMissionToTitle,
				(DWORD)m_pMenu, pMsgToken->hWnd, 1);

			SetWindowAttr(XWATTR_SHOWWINDOW, false);
			break;
		}
	}
}