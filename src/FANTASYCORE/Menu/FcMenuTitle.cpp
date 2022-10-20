#include "stdafx.h"

#include "FantasyCore.h"

#include "FcMenuTitle.h"
#include "BSuiSystem.h"

#include "BsUiClassIDManager.h"
#include "BsUiFontAttrManager.h"

#include "FcInterfaceManager.h"
#include "FcMessageDef.h"

#include "InputPad.h"
#include "FcGlobal.h"
#include "FcItem.h"

#include "BsMoviePlayer.h"
#include "FcSoundManager.h"
#include "FcRealtimeMovie.h"

#ifdef _XBOX
#include "atgsignin.h"
#include "FcLiveManager.h"
#endif //_XBOX

#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG

#define _WAIT_LIMIT_TIME	45 * 40
#define _WAIT_START_TIME	40

#define _WAVE_COUNT		3
#define _WAVE_SIZE		512
#define _WAVE_SPEED		1


#define _WAIT_SAVE_TIME	80


bool s_bCheckSaveDataCount = false;

CFcMenuTitle::CFcMenuTitle(_FC_MENU_TYPE nType, _FC_MENU_TYPE nPrevType)
: CFcMenuForm(nType)
{	
	g_FCGameData.nPlayerType = MISSION_TYPE_NONE;
	m_ntime = g_BsUiSystem.GetMenuTick() - 10;
	m_PrevMenu = nPrevType;
	m_bConfirmDevice = true;
	m_bNeedSaveFixed = false;
	m_bWaitSaving = false;
	m_bRemovedStorage = false;
	m_bNeedShowProfileDeleteWarn = false;

	m_pBaseLayer = new BsUiLayer();
	AddUiLayer(m_pBaseLayer, SN_Title0, -1, -1);

	m_pTitleLayer = new CFcMenuTitleLayer(this);
	AddUiLayer(m_pTitleLayer, SN_Title2, -1, -1);
	m_pTitleLayer->Initialize();

	m_pStartLayer = new CFcMenuTitleStartLayer(this);
	AddUiLayer(m_pStartLayer, SN_Title1, -1, -1);
	
	g_FCGameData.bCheckFreeSpace = false;
	s_bCheckSaveDataCount = false;
    
	UpdateLayer(nPrevType);
	
	SetStatus(_MS_NORMAL);
}

CFcMenuTitle::~CFcMenuTitle() 
{
	g_FCGameData.bCheckFreeSpace = true;
}


void CFcMenuTitle::InitSavedMachine()
{
#ifdef _XBOX
	if(g_FC.GetLiveManager()->IsUserSignedIn() 
		&& !ATG::SignIn::IsSystemUIShowing() 
		&& g_FCGameData.m_DeviceID != XCONTENTDEVICE_ANY 
		&& g_FCGameData.bShowDeviceUI 
		&& s_bCheckSaveDataCount == false ) 
	{
		s_bCheckSaveDataCount = true;
		
		if(g_FCGameData.UpdateGetContentData()==false) {
			g_MenuHandle->PostMessage(fcMSG_MESSAGEBOX_START,
				fcMT_FailEnumerateContents,	// ok
				(DWORD)this, NULL);
		}
		

		g_FCGameData.m_OldDeviceID = g_FCGameData.m_DeviceID;

		if(g_FCGameData.IsFixedSavedGame()){
			g_FCGameData.LoadFixedGameData();
		}
		else{
			XDEVICE_DATA  data={0,};
			if(ERROR_SUCCESS != XContentGetDeviceData(g_FCGameData.m_DeviceID, &data))
			{
				// 중간에 디바이스 장치가 제거 되었거나 기타 오류 처리
				return;
			}		

			if( data.ulDeviceFreeBytes >= FIXEDSAVEDATA_ENOUGHSIZE)						
				m_bNeedSaveFixed = true;
		}
	}

#endif //_XBOX
}

void CFcMenuTitle::ProcessMsg(int nMsgAsk, BsUiHANDLE hWnd, int nMsgRsp)
{
	if( nMsgAsk == fcMT_NoSignInWarning )
	{
		if(nMsgRsp == ID_YES)
		{
			BsUiWindow* pFocus = g_BsUiSystem.GetFocusWindow();
			if(pFocus != NULL ){	
				g_MenuHandle->PostMessage(fcMSG_CLICK_FX_EFFECT, (DWORD)pFocus, _WAVE_COUNT, _WAVE_SIZE, _WAVE_SPEED);
			}

			g_FCGameData.nPlayerCount = 1;
			g_MenuHandle->PostMessage(fcMSG_SEL_CHAR_START);
		}
		else
		{
			m_ntime = g_BsUiSystem.GetMenuTick() - 10;
			g_BsUiSystem.SetFocusWindow(BsUi::BsUiGetWindow(hWnd));
		}
	} else if( nMsgAsk == fcMT_NoDeviceWarning )
	{
		if(nMsgRsp == ID_YES)
		{
			BsUiWindow* pFocus = g_BsUiSystem.GetFocusWindow();
			if(pFocus != NULL ){	
				g_MenuHandle->PostMessage(fcMSG_CLICK_FX_EFFECT, (DWORD)pFocus, _WAVE_COUNT, _WAVE_SIZE, _WAVE_SPEED);
			}

			g_FCGameData.nPlayerCount = 1;
			g_MenuHandle->PostMessage(fcMSG_SEL_CHAR_START);
		}
		else
		{
			m_ntime = g_BsUiSystem.GetMenuTick() - 10;
			g_BsUiSystem.SetFocusWindow(BsUi::BsUiGetWindow(hWnd));
		}
	} else if( nMsgAsk == fcMT_DiskFree )
	{
		if(nMsgRsp == ID_YES)
		{ 
			m_bConfirmDevice = true;		
		}
		else		
		{
            g_FCGameData.bShowDeviceUI = false;
		}
	} else if( nMsgAsk == fcMT_CancelDeviceSelector )
	{
		m_bConfirmDevice = true;
		m_pTitleLayer->EnableKeyDown();
	}
	else if( nMsgAsk == fcMT_ProfileDelete )
	{
		if(nMsgRsp == ID_YES)
		{ 
			m_bRemovedStorage = false;
			m_bConfirmDevice = true;
			g_FCGameData.LoadFixedGameData();			
		}
		else		
		{
            g_FCGameData.bShowDeviceUI = false;
			g_FCGameData.bChangeDevice = true;
		}
	}
}

void CFcMenuTitle::Process()
{
	//InitSavedMachine();
#ifdef _XBOX
	if(g_FCGameData.m_OldDeviceID != g_FCGameData.m_DeviceID && g_FC.GetMainState() != GAME_STATE_SYSTEM_MESSAGE) 
	{
		if(g_FCGameData.UpdateGetContentData()==false) {
			g_MenuHandle->PostMessage(fcMSG_MESSAGEBOX_START,
				fcMT_FailEnumerateContents,	// ok
				(DWORD)this, NULL);
		}		

		// if a selected storage device has not enough space to save a gamedata, send a message to user "is it ok?"
		if(g_FCGameData.m_DeviceID != XCONTENTDEVICE_ANY)
		{
			m_bConfirmDevice = true;

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
				m_bConfirmDevice = false;
			}
			
			if(g_FCGameData.bFixedSaveData == false)
			{
				m_bNeedSaveFixed = true;				
				m_bNeedShowProfileDeleteWarn = false;
			}
			else
			{
				m_bNeedSaveFixed = false;
				
				if(m_bRemovedStorage)
				{
					m_bNeedShowProfileDeleteWarn = true;					
				}
				else
					g_FCGameData.LoadFixedGameData();
			}
		}        

		if(!m_pStartLayer->IsShowWindow()) {

			g_FCGameData.m_OldDeviceID = g_FCGameData.m_DeviceID;

			BsUiWindow* pFocusedWnd = g_BsUiSystem.GetFocusWindow();		

			if(m_pStartLayer->GetWindow(BT_Title_Start) == pFocusedWnd)
				pFocusedWnd = m_pTitleLayer->GetWindow(BT_Title1P);

			BsUiWindow* pWindow = m_pTitleLayer->GetWindow(BT_TitleLoadData);
			
			if(g_FCGameData.GetUserSaveGameCount() > 0 && g_FCGameData.m_DeviceID != XCONTENTDEVICE_ANY){
				pWindow->SetWindowAttr(XWATTR_DISABLE, false );
			}else{
				pWindow->SetWindowAttr(XWATTR_DISABLE, true);
				m_pTitleLayer->SetFocusWindowClass(BT_Title1P);
			}

			if(pWindow->IsEnableWindow() && g_FCGameData.GetUserSaveGameCount() > 0){
				m_pTitleLayer->SetFocusWindowClass(BT_TitleLoadData);
			}

			if(pFocusedWnd != pWindow)
				g_BsUiSystem.SetFocusWindow(pFocusedWnd);
		}
	}
	if(g_FCGameData.bRemovedStorageDevice && g_FC.GetMainState() != GAME_STATE_SYSTEM_MESSAGE )
	{
		CFcMenuForm* pMsgBoxMenu = g_MenuHandle->GetMessageBoxMenu(fcMT_DiskFree);
		if(pMsgBoxMenu!=NULL)
			g_MenuHandle->PostMessage(fcMSG_MESSAGEBOX_END,
					(DWORD)pMsgBoxMenu,
					(DWORD)g_FC.GetMainState(), //(DWORD)m_dwMainState,
					(DWORD)this);				//(DWORD)m_pCmdMenu,
		pMsgBoxMenu = g_MenuHandle->GetMessageBoxMenu(fcMT_ProfileDelete);
			if(pMsgBoxMenu!=NULL)
				g_MenuHandle->PostMessage(fcMSG_MESSAGEBOX_END,
				(DWORD)pMsgBoxMenu,
				(DWORD)g_FC.GetMainState(), //(DWORD)m_dwMainState,
				(DWORD)this);				//(DWORD)m_pCmdMenu,
        g_MenuHandle->PostMessage(fcMSG_MESSAGEBOX_START,
				fcMT_RemoveStorage,	// yesno
				(DWORD)this, NULL);
		g_FCGameData.bRemovedStorageDevice = false;
		m_bRemovedStorage = true;
	}
	if(g_FCGameData.bCancelDeviceSelector )
	{
		g_MenuHandle->PostMessage(fcMSG_MESSAGEBOX_START,
				fcMT_CancelDeviceSelector,	// ok
				(DWORD)this, NULL);
		g_FCGameData.bCancelDeviceSelector = false;
		m_bConfirmDevice=false;
		m_pTitleLayer->EnableKeyDown(false);
		
	}

	if( m_bConfirmDevice 
		&& m_bNeedSaveFixed 
		&& m_bWaitSaving == false
		&& g_FC.GetMainState() != GAME_STATE_SYSTEM_MESSAGE)
	{
		g_MenuHandle->PostMessage(fcMSG_MESSAGEBOX_START,
			fcMT_SavingNow,
			(DWORD)this, NULL);

		m_nSaveProcessTick = GetProcessTick();
		m_bWaitSaving = true;
		return;
	}

	if(g_FC.GetMainState() != GAME_STATE_SYSTEM_MESSAGE)
	{
		if(m_bNeedSaveFixed && m_bWaitSaving) {
			g_FCGameData.SaveFixedGameData();			
			m_bNeedSaveFixed = false;
		}

		if(m_bNeedSaveFixed == false 
			&& m_bWaitSaving 
			&& m_nSaveProcessTick + _WAIT_SAVE_TIME <= GetProcessTick())
		{
			m_bWaitSaving = false;
			//g_BsUiSystem.TranslateMessage(XWMSG_KEYDOWN, 0, MENU_INPUT_A, NULL);

			CFcMenuForm* pMsgBoxMenu = g_MenuHandle->GetMessageBoxMenu(fcMT_SavingNow);
			BsAssert(pMsgBoxMenu != NULL);
			g_MenuHandle->PostMessage(fcMSG_MESSAGEBOX_END,
					(DWORD)pMsgBoxMenu,
					(DWORD)g_FC.GetMainState(), //(DWORD)m_dwMainState,
					(DWORD)this);				//(DWORD)m_pCmdMenu,
		}

		// 스토리지가 삭제된 후 새로 선택이 되었을 시에 경고 메시지(설정파일이 이미 있는경우)
		if(
			m_bNeedShowProfileDeleteWarn 
			&& m_bConfirmDevice
			&& m_bNeedSaveFixed == false
			&& m_bWaitSaving ==false )
		{
			g_MenuHandle->PostMessage(fcMSG_MESSAGEBOX_START,
			fcMT_ProfileDelete,
			(DWORD)this, NULL);

			m_bNeedShowProfileDeleteWarn = false;
			m_bConfirmDevice = false;
		}			

	}
#endif // _XBOX
}

void CFcMenuTitle::RenderProcess()
{
	if(m_pStartLayer->IsShowWindow())
	{
		int ntime = g_BsUiSystem.GetMenuTick();

		if(m_pStartLayer->IsFirstStartInput())
		{
			if(g_BsKernel.GetClickFxEffect() != NULL)
			{
				int nWave = g_BsKernel.GetClickFxEffect()->GetWaveCount();
				if(nWave == 0 && (m_ntime + _WAIT_START_TIME < ntime))
				{
				//	BsUiWindow* pWnd = g_BsUiSystem.GetFocusWindow();
					UpdateLayer(_FC_MENU_SEL_CHAR);
					ClickStartPage();
				//	g_BsUiSystem.SetFocusWindow(pWnd);
				}
			}
			else{
				//BsUiWindow* pWnd = g_BsUiSystem.GetFocusWindow();
				UpdateLayer(_FC_MENU_SEL_CHAR);
				ClickStartPage();
				//g_BsUiSystem.SetFocusWindow(pWnd);
			}
		}
							
		if(m_ntime + _WAIT_LIMIT_TIME < ntime){
			g_MenuHandle->PostMessage(fcMSG_OPENING_START);
		}
		
	}
	else if(g_BsKernel.GetClickFxEffect() != NULL)
	{	
		if(g_BsUiSystem.GetFocusWindow())
		{
			if(g_BsUiSystem.GetFocusWindow()->GetClass() == BT_Title_Start){
				m_pTitleLayer->SetFocusWindowClass(BT_Title1P);
			}
		}

		int nWave = g_BsKernel.GetClickFxEffect()->GetWaveCount();

		if(nWave > 0)
		{
			float fAlpha = 0.f;
			if(g_BsKernel.GetClickFxEffect() != NULL)
			{
				int nTotalFrame = g_BsKernel.GetClickFxEffect()->GetWaveFrame();
				int nCurFrame = g_BsKernel.GetClickFxEffect()->GetLastWaveRemainFrame();

				fAlpha = (float)nCurFrame / nTotalFrame;

				if(fAlpha <= 0.f){
					fAlpha = 0.f;
				}
				else if(fAlpha >= 1.f){
					fAlpha = 1.f;
				}
			}
					
			m_pBaseLayer->GetWindow(IM_TitleLogo)->SetColor(D3DXCOLOR(1, 1, 1, 1 - fAlpha));
			m_pBaseLayer->GetWindow(TT_Title_Copyright)->SetColor(D3DXCOLOR(1, 1, 1, 1 - fAlpha));

			m_pTitleLayer->GetWindow(BT_Title1P)->SetColor(D3DXCOLOR(1, 1, 1, 1 - fAlpha));
			m_pTitleLayer->GetWindow(BT_TitleLoadData)->SetColor(D3DXCOLOR(1, 1, 1, 1 - fAlpha));
			m_pTitleLayer->GetWindow(BT_TitleLibrary)->SetColor(D3DXCOLOR(1, 1, 1, 1 - fAlpha));
			m_pTitleLayer->GetWindow(BT_TitleOption)->SetColor(D3DXCOLOR(1, 1, 1, 1 - fAlpha));
			m_pTitleLayer->GetWindow(BT_Tutorial)->SetColor(D3DXCOLOR(1, 1, 1, 1 - fAlpha));
			
			m_pTitleLayer->GetWindow(IM_FadeBox)->SetColor(D3DXCOLOR(1, 1, 1, fAlpha));
		}
	}
}

void CFcMenuTitle::UpdateLayer(_FC_MENU_TYPE nPrevType)
{
	BsUiWindow* pWindow = m_pTitleLayer->GetWindow(BT_TitleLoadData);
#ifdef _XBOX
	if(g_FCGameData.GetUserSaveGameCount() > 0 
		&& g_FCGameData.m_DeviceID != XCONTENTDEVICE_ANY
		&& ATG::SignIn::IsUserSignedIn(g_FCGameData.nEnablePadID))
#else
	if (g_FCGameData.GetUserSaveGameCount() > 0
		&& g_FCGameData.m_DeviceID != XCONTENTDEVICE_ANY)
#endif
	{
		pWindow->SetWindowAttr(XWATTR_DISABLE, false );
	}else{
		pWindow->SetWindowAttr(XWATTR_DISABLE, true);
		m_pTitleLayer->SetFocusWindowClass(BT_Title1P);
	}

	switch(nPrevType)
	{
	case _FC_MENU_TITLE:
	case _FC_MENU_OPENING:
		{	
			g_FCGameData.nEnablePadID = -1;
			m_pStartLayer->SetFocusWindowClass(BT_Title_Start);
			break;
		}
	case _FC_MENU_SEL_CHAR:		m_pTitleLayer->SetFocusWindowClass(BT_Title1P); break;
	case _FC_MENU_LOADDATA:		m_pTitleLayer->SetFocusWindowClass(BT_TitleLoadData); break;
	case _FC_MENU_LIBRARY:		m_pTitleLayer->SetFocusWindowClass(BT_TitleLibrary); break;
	case _FC_MENU_OPTION:		m_pTitleLayer->SetFocusWindowClass(BT_TitleOption); break;
	case _FC_MENU_TUTORIAL:		m_pTitleLayer->SetFocusWindowClass(BT_Tutorial); break;
	default:
		{
			if(g_FCGameData.GetUserSaveGameCount() > 0)
			{
				nPrevType = _FC_MENU_LOADDATA;
				m_pTitleLayer->SetFocusWindowClass(BT_TitleLoadData);
			}
			else
			{
				nPrevType = _FC_MENU_SEL_CHAR;
				m_pTitleLayer->SetFocusWindowClass(BT_Title1P);
			}
		}
	}
	
	switch(nPrevType)
	{
	case _FC_MENU_TITLE:
	case _FC_MENU_OPENING:
		{
			m_pTitleLayer->SetWindowAttr(XWATTR_SHOWWINDOW, false);
			m_pStartLayer->SetWindowAttr(XWATTR_SHOWWINDOW, true);
			break;
		}
	case _FC_MENU_SEL_CHAR:		
	case _FC_MENU_LOADDATA:
	case _FC_MENU_LIBRARY:
	case _FC_MENU_OPTION:
	case _FC_MENU_TUTORIAL:
	default:
		{
			m_pTitleLayer->SetWindowAttr(XWATTR_SHOWWINDOW, true);
			m_pStartLayer->SetWindowAttr(XWATTR_SHOWWINDOW, false);



			break;
		}
	}
}


void CFcMenuTitle::ClickStartPage()
{
	if(g_FCGameData.GetUserSaveGameCount() > 0){
		m_pTitleLayer->SetFocusWindowClass(BT_TitleLoadData);
	}

	BsUiWindow* pWindow = m_pTitleLayer->GetWindow(BT_TitleLoadData);
#ifdef _XBOX
	if(g_FCGameData.GetUserSaveGameCount() > 0 
		&& g_FCGameData.m_DeviceID != XCONTENTDEVICE_ANY
		&& ATG::SignIn::IsUserSignedIn(g_FCGameData.nEnablePadID))
#else
	if (g_FCGameData.GetUserSaveGameCount() > 0
		&& g_FCGameData.m_DeviceID != XCONTENTDEVICE_ANY)
#endif
	{
		pWindow->SetWindowAttr(XWATTR_DISABLE, false );
	}else{
		pWindow->SetWindowAttr(XWATTR_DISABLE, true);
		m_pTitleLayer->SetFocusWindowClass(BT_Title1P);
	}

	//show device selector
#ifdef _XBOX
	if(ATG::SignIn::IsUserSignedIn(g_FCGameData.nEnablePadID))
		g_FCGameData.bShowDeviceUI = false;
#endif
}


void CFcMenuTitle::ClickBackPage()
{
	g_FCGameData.nEnablePadID = -1;
	g_FCGameData.tempFixedSave.Clear();
	s_bCheckSaveDataCount = false;
	g_FCGameData.m_DeviceID = XCONTENTDEVICE_ANY;
#ifdef _XBOX
	g_FCGameData.m_OldDeviceID = XCONTENTDEVICE_ANY;
#endif
	m_pStartLayer->SetFirstStartInput(false);

	UpdateLayer(_FC_MENU_OPENING);
}

bool CFcMenuTitle::IsStartLayer() { return m_pStartLayer->IsShowWindow(); }
//-----------------------------------------------------------------------------------------------------
CFcMenuTitleStartLayer::CFcMenuTitleStartLayer(CFcMenuTitle* pMenu)
{ 
	m_pMenu = pMenu;
	m_bFirstStartInput = false;
}

DWORD CFcMenuTitleStartLayer::ProcMessage(xwMessageToken* pMsgToken)
{
	BsAssert(pMsgToken);

	switch(pMsgToken->message)
	{
	case XWMSG_KEYDOWN:
		{
			m_pMenu->SetItem(g_BsUiSystem.GetMenuTick() - 10);

			switch (pMsgToken->lParam)
			{
			case MENU_INPUT_START:
				{
					if(m_bFirstStartInput == false)
					{
						g_MenuHandle->PostMessage(fcMSG_CLICK_FX_EFFECT, NULL, _WAVE_COUNT, _WAVE_SIZE, _WAVE_SPEED);

						//key setting
						g_FCGameData.nEnablePadID = CInputPad::GetInstance().GetLastKeyPressPort();						
						g_FCGameData.m_DeviceID = XCONTENTDEVICE_ANY;


#ifdef _XBOX
						if (ATG::SignIn::IsUserSignedIn(g_FCGameData.nEnablePadID) == TRUE)
							XUserGetName(g_FCGameData.nEnablePadID, g_FCGameData.szProfileName, sizeof(g_FCGameData.szProfileName));

						g_FCGameData.ConfigInfo.bCameraUD_Reverse = g_FC.GetLiveManager()->m_nYAxis_Inversion ? true : false;
						g_FCGameData.ConfigInfo.bVibration = g_FC.GetLiveManager()->m_bController_Vibration;
#else
						g_FCGameData.ConfigInfo.bCameraUD_Reverse = false;
						g_FCGameData.ConfigInfo.bVibration = false;
#endif
						CInputPad::GetInstance().SetVibrationOn(g_FCGameData.ConfigInfo.bVibration);

						m_bFirstStartInput = true;
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



//-----------------------------------------------------------------------------------------------------
void CFcMenuTitleLayer::Initialize()
{
	GetWindow(IM_FadeBox)->SetWindowAttr(XWATTR_SHOWWINDOW, true);
	GetWindow(IM_FadeBox)->SetColor(D3DXCOLOR(1, 1, 1, 0));
	
	bool bDisable = true;
	if(g_FCGameData.GetUserSaveGameCount() > 0){
		bDisable = false;
	}

	m_bEnableKeyDown = true;

	GetWindow(BT_TitleLoadData)->SetWindowAttr(XWATTR_DISABLE, bDisable);
	if(bDisable)
		SetFocusWindowClass(BT_Title1P);

}

DWORD CFcMenuTitleLayer::ProcMessage(xwMessageToken* pMsgToken)
{
	BsAssert(pMsgToken);

	switch(pMsgToken->message)
	{
	case XWMSG_KEYDOWN:
		{	
			// Device Selector UI 를 띄울때에는 키 입력을 받지 못하게
#ifdef _XBOX
			if(g_FCGameData.bShowDeviceUI==false
				&& g_FCGameData.nEnablePadID != -1
				&& ATG::SignIn::IsUserSignedIn(g_FCGameData.nEnablePadID)==TRUE)
				break;							
#endif

			//wait time을 갱신한다.
			m_pMenu->SetItem(g_BsUiSystem.GetMenuTick() - 10);

			switch (pMsgToken->lParam)
			{
			case MENU_INPUT_B:
				{
					if(m_pMenu->IsDeviceConfirmed()==false || g_FCGameData.bCancelDeviceSelector)
						break;

					g_pSoundManager->PlaySystemSound(SB_COMMON,"SYS_CANCEL");
					m_pMenu->ClickBackPage();
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
					if(!m_bEnableKeyDown)				
						break;
					

					BsUiCLASS hClass = BsUi::BsUiGetClass(pMsgToken->hWnd);
					switch(hClass)
					{
					case BT_Title1P:		OnKeyDownBt1P(pMsgToken); break;
					case BT_TitleLoadData:	OnKeyDownBtLoadData(pMsgToken); break;
					case BT_TitleLibrary:	OnKeyDownBtLibrary(pMsgToken); break;
					case BT_TitleOption:	OnKeyDownBtOption(pMsgToken); break;
					case BT_Tutorial:		OnKeyDownBtTutorial(pMsgToken); break;
					}
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


void CFcMenuTitleLayer::OnKeyDownBt1P(xwMessageToken* pMsgToken)
{
	switch (pMsgToken->lParam)
	{
	case MENU_INPUT_A:
		{
#ifdef _XBOX
			if(ATG::SignIn::IsUserSignedIn(g_FCGameData.nEnablePadID)==FALSE) 
			{
				g_MenuHandle->PostMessage(fcMSG_MESSAGEBOX_START,
					fcMT_NoSignInWarning,	// yes no
					(DWORD)m_pMenu, pMsgToken->hWnd);
			}
			else if(g_FCGameData.m_DeviceID == XCONTENTDEVICE_ANY)
			{
				g_MenuHandle->PostMessage(fcMSG_MESSAGEBOX_START,
					fcMT_NoDeviceWarning,	// yes no
					(DWORD)m_pMenu, pMsgToken->hWnd);
			}
			else
#endif // _XBOX
			{
				BsUiWindow* pFocus = g_BsUiSystem.GetFocusWindow();
				if(pFocus != NULL && pMsgToken->lParam == PAD_INPUT_A){	
					g_MenuHandle->PostMessage(fcMSG_CLICK_FX_EFFECT, (DWORD)pFocus, _WAVE_COUNT, _WAVE_SIZE, _WAVE_SPEED);
				}

				g_FCGameData.nPlayerCount = 1;
				g_MenuHandle->PostMessage(fcMSG_SEL_CHAR_START);
			}
			break;
		}
	}	
}

void CFcMenuTitleLayer::OnKeyDownBtLoadData(xwMessageToken* pMsgToken)
{
	switch (pMsgToken->lParam)
	{
	case MENU_INPUT_A:
		{
			BsUiWindow* pFocus = g_BsUiSystem.GetFocusWindow();
			if(pFocus != NULL && pMsgToken->lParam == PAD_INPUT_A){	
				g_MenuHandle->PostMessage(fcMSG_CLICK_FX_EFFECT, (DWORD)pFocus);
			}

			g_MenuHandle->PostMessage(fcMSG_LOADDATA_START);
			break;
		}
	}
}
void CFcMenuTitleLayer::OnKeyDownBtLibrary(xwMessageToken* pMsgToken)
{
	switch (pMsgToken->lParam)
	{
	case MENU_INPUT_A:
		{
			BsUiWindow* pFocus = g_BsUiSystem.GetFocusWindow();
			if(pFocus != NULL && pMsgToken->lParam == PAD_INPUT_A){	
				g_MenuHandle->PostMessage(fcMSG_CLICK_FX_EFFECT, (DWORD)pFocus);
			}
			g_MenuHandle->PostMessage(fcMSG_LIBRARY_START);
			break;
		}
	}
}

void CFcMenuTitleLayer::OnKeyDownBtOption(xwMessageToken* pMsgToken)
{
	switch (pMsgToken->lParam)
	{
	case MENU_INPUT_A:
		{
			BsUiWindow* pFocus = g_BsUiSystem.GetFocusWindow();
			if(pFocus != NULL && pMsgToken->lParam == PAD_INPUT_A){	
				g_MenuHandle->PostMessage(fcMSG_CLICK_FX_EFFECT, (DWORD)pFocus);
			}
			
			g_MenuHandle->PostMessage(fcMSG_OPTION_START);
			break;
		}
	}
}

void CFcMenuTitleLayer::OnKeyDownBtCreadit(xwMessageToken* pMsgToken)
{
	switch (pMsgToken->lParam)
	{
	case MENU_INPUT_A:
		{
			BsUiWindow* pFocus = g_BsUiSystem.GetFocusWindow();
			if(pFocus != NULL && pMsgToken->lParam == PAD_INPUT_A){	
				g_MenuHandle->PostMessage(fcMSG_CLICK_FX_EFFECT, (DWORD)pFocus);
			}

			g_MenuHandle->PostMessage(fcMSG_CREDITS_START);
			break;
		}
	}
}

void CFcMenuTitleLayer::OnKeyDownBtTutorial(xwMessageToken* pMsgToken)
{
	switch (pMsgToken->lParam)
	{
	case MENU_INPUT_A:
		{
			BsUiWindow* pFocus = g_BsUiSystem.GetFocusWindow();
			if(pFocus != NULL && pMsgToken->lParam == PAD_INPUT_A){	
				g_MenuHandle->PostMessage(fcMSG_CLICK_FX_EFFECT, (DWORD)pFocus);
			}
			
			g_MenuHandle->PostMessage(fcMSG_TUTORIAL_START);
			break;
		}
	}
}