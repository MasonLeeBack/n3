#include "stdafx.h"

#include "FcMenuSaveData.h"
#include "BSuiSystem.h"

#include "BsUiClassIDManager.h"
#include "BsUiFontAttrManager.h"

#include "FcInterfaceManager.h"
#include "FcMessageDef.h"
#include "FcItem.h"
#include "LocalLanguage.h"


#include "FcSoundManager.h"
#include "FantasyCore.h"
#include "TextTable.h"

#include "FcMenuMessageBox.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG

#define _WAIT_SAVE_TIME	80

CFcMenuSaveData::CFcMenuSaveData(_FC_MENU_TYPE nType, _FC_MENU_TYPE nPrevType, DWORD dwClearChar)
: CFcMenuForm(nType)
{
	m_nPrevType = nPrevType;
	m_dwClearChar = dwClearChar;
	m_nSaveState = SAVE_STATE_NONE;
	m_bNowSaving = false;
	m_bWaitSaving = false;
	m_bConfirmDevice = true;
	m_bNeedShowProfileDeleteWarn = false;
	m_bCanExit = true;

	m_pLayer = new CFcMenuSaveDataLayer(this);
	AddUiLayer(m_pLayer, SN_SaveData, -1, -1);
	m_pLayer->Initialize();

	g_FCGameData.bCheckFreeSpace = false;
	
}

void CFcMenuSaveData::ProcessMsg(int nMsgAsk, BsUiHANDLE hWnd, int nMsgRsp)
{
#ifdef _XBOX
	if(nMsgAsk == fcMT_SaveGameData)
	{
		if(nMsgRsp == ID_YES)
		{	
			if(g_FCGameData.m_DeviceID == XCONTENTDEVICE_ANY) {
				g_BsUiSystem.SetFocusWindow(BsUi::BsUiGetWindow(hWnd));
				return;
			}

			m_bNowSaving  = true;

            m_hFocusedWnd = hWnd;
			m_nSaveState = SAVE_STATE_NEWDATA;

			g_MenuHandle->EnableKey(false);
		}
		
		g_BsUiSystem.SetFocusWindow(BsUi::BsUiGetWindow(hWnd));
	}
	else if(nMsgAsk == fcMT_OverWriteGameData)
	{
		if(nMsgRsp == ID_YES)
		{
			if(g_FCGameData.m_DeviceID == XCONTENTDEVICE_ANY) {
				g_BsUiSystem.SetFocusWindow(BsUi::BsUiGetWindow(hWnd));
				return;
			}

			m_bNowSaving = true;

			m_hFocusedWnd = hWnd;
			m_nSaveState = SAVE_STATE_OVERWRITE;

			g_MenuHandle->EnableKey(false);
			
		}
		g_BsUiSystem.SetFocusWindow(BsUi::BsUiGetWindow(hWnd));
	}
	else if(nMsgAsk == fcMT_ExitClearStageSave)
	{
		if(m_bCanExit==false)
			return;
		if(nMsgRsp == ID_YES)
		{
			SetStatus(_MS_CLOSING);
			g_MenuHandle->PostMessage(fcMSG_GO_TO_SEL_STAGE);
		}else{
			g_BsUiSystem.SetFocusWindow(BsUi::BsUiGetWindow(hWnd));
		}
	}
	else if(nMsgAsk == fcMT_ExitClearAllSave)
	{
		if(nMsgRsp == ID_YES)
		{
			SetStatus(_MS_CLOSING);
			g_MenuHandle->PostMessage(fcMSG_ENDING_CHAR_START);
		}else{
			g_BsUiSystem.SetFocusWindow(BsUi::BsUiGetWindow(hWnd));
		}
	}	
	else if(nMsgAsk == fcMT_DiskFull || nMsgAsk == fcMT_ContentsLimit )
	{
		if(nMsgRsp == ID_YES)
		{ 
			g_FCGameData.bShowDeviceUI = false;				
			g_FCGameData.bChangeDevice = true;
			
		}else{
			m_bConfirmDevice = true;
			if(g_FCGameData.bFixedSaveData == false)
			{		
				m_bNowSaving  = true;
				//m_hFocusedWnd = hWnd;
				m_nSaveState = SAVE_STATE_NEWFIXED;
				g_MenuHandle->EnableKey(false);		
			}
			g_BsUiSystem.SetFocusWindow(BsUi::BsUiGetWindow(hWnd));
		}
	} 
	else if( nMsgAsk == fcMT_DiskFree )
	{
		if(nMsgRsp == ID_YES)
		{ 
			m_bConfirmDevice = true;
			//if(g_FCGameData.bFixedSaveData == false)
			//{		
			//	m_bNowSaving  = true;
			//	//m_hFocusedWnd = hWnd;
			//	m_nSaveState = SAVE_STATE_NEWFIXED;
			//	g_MenuHandle->EnableKey(false);
			//}
		}
		else		
		{
            g_FCGameData.bShowDeviceUI = false;
			g_FCGameData.bChangeDevice = true;
		}
	}
	else if( nMsgAsk == fcMT_ProfileDelete )
	{
		if(nMsgRsp == ID_YES)
		{ 
			m_bConfirmDevice = true;	
				m_bNowSaving  = true;

				//m_hFocusedWnd = hWnd;
				m_nSaveState = SAVE_STATE_NEWFIXED;

				g_MenuHandle->EnableKey(false);

		}
		else		
		{
            g_FCGameData.bShowDeviceUI = false;
			g_FCGameData.bChangeDevice = true;
		}
	}
	else if( nMsgAsk == fcMT_RemoveStorage )
	{
		m_bCanExit = true;
	}


#endif //_XBOX
}

void CFcMenuSaveData::Process()
{
	
#ifdef _XBOX
	if(g_FCGameData.bShowDeviceUI 
		&& ( (g_FCGameData.m_OldDeviceID != g_FCGameData.m_DeviceID) || g_FCGameData.bChangeDevice) ) 
	{
		if(g_FCGameData.UpdateGetContentData()==false) {
			g_MenuHandle->PostMessage(fcMSG_MESSAGEBOX_START,
				fcMT_FailEnumerateContents,	// ok
				(DWORD)this, NULL);
		}        

		// if a selected storage device has not enough space to save a gamedata, send a message to user "is it ok?"
		if(g_FCGameData.m_DeviceID != XCONTENTDEVICE_ANY)
		{
			XDEVICE_DATA  data={0,};
			m_bConfirmDevice = true;

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
			else if(g_FCGameData.GetUserSaveGameCount() >= _MAX_USER_SAVE_DATA_COUNT) // 세이브 데이타 50개 제한 
			{
				g_MenuHandle->PostMessage(fcMSG_MESSAGEBOX_START,
					fcMT_ContentsLimit,	// ok
					(DWORD)this, NULL);
				m_bConfirmDevice = false;
			}

			if(g_FCGameData.bFixedSaveData == true) 
			{
                m_bNeedShowProfileDeleteWarn = true;
			}
			else if(m_bConfirmDevice)
			{		
				m_bNowSaving  = true;

				//m_hFocusedWnd = hWnd;
				m_nSaveState = SAVE_STATE_NEWFIXED;

				g_MenuHandle->EnableKey(false);
			}
		}
		BsUiWindow* pWnd = NULL;
		if(m_pLayer->IsFocusedOnListBtn()==false)
			pWnd = g_BsUiSystem.GetFocusWindow();
		m_pLayer->Initialize();
		if(pWnd)
			g_BsUiSystem.SetFocusWindow(pWnd);
		g_FCGameData.m_OldDeviceID = g_FCGameData.m_DeviceID;
		g_FCGameData.bChangeDevice = false;
	}

	if(m_bNowSaving && g_FC.GetMainState() != GAME_STATE_SYSTEM_MESSAGE)
	{
		g_MenuHandle->PostMessage(fcMSG_MESSAGEBOX_START,
			fcMT_SavingNow,
			(DWORD)this, m_hFocusedWnd);

		m_bNowSaving = false;

		m_nSaveProcessTick = GetProcessTick();
		m_bWaitSaving = true;
		return;
	}

	if(m_bWaitSaving && g_FC.GetMainState() != GAME_STATE_SYSTEM_MESSAGE)
	{
		if(m_nSaveProcessTick + _WAIT_SAVE_TIME <= GetProcessTick())
		{
			m_bWaitSaving = false;
			//g_BsUiSystem.TranslateMessage(XWMSG_KEYDOWN, 0, MENU_INPUT_A, NULL);

			CFcMenuForm* pMsgBoxMenu = g_MenuHandle->GetMessageBoxMenu(fcMT_SavingNow);
			BsAssert(pMsgBoxMenu != NULL);
			g_MenuHandle->PostMessage(fcMSG_MESSAGEBOX_END,
					(DWORD)pMsgBoxMenu,
					(DWORD)g_FC.GetMainState(), //(DWORD)m_dwMainState,
					(DWORD)this);				//(DWORD)m_pCmdMenu,
			// 세이브 데이타 50개 제한 
			if(g_FCGameData.GetUserSaveGameCount() >= _MAX_USER_SAVE_DATA_COUNT)
			{
				g_MenuHandle->PostMessage(fcMSG_MESSAGEBOX_START,
					fcMT_ContentsLimit,	// ok
					(DWORD)this, NULL);
			}
			else if(g_FCGameData.m_DeviceID != XCONTENTDEVICE_ANY)
			{
				ULARGE_INTEGER iBytesRequested = {0};			
				int nDataSize = g_FCGameData.tempUserSave.GetSaveDataSize();				
				//if(!g_FCGameData.bFixedSaveData) // 유저 프로필 파일이 없을 경우 이를 생성하기 위한 용량
				//	nDataSize+=FIXEDSAVEDATA_ENOUGHSIZE;
				if(!g_FCGameData.bFixedSaveData && g_FCGameData.nSaveGameCount==0) // 새로운 파일을 위한 폴더 생성 요구 용량
					nDataSize+=FIRSTCREATE_ENOUGHSIZE;
				iBytesRequested.QuadPart = XContentCalculateSize(nDataSize, 1);
				XDEVICE_DATA  data={0,};
				if(ERROR_SUCCESS != XContentGetDeviceData(g_FCGameData.m_DeviceID, &data))
				{
					// 중간에 디바이스 장치가 제거 되었거나 기타 오류 처리
					return;
				}		

				if( data.ulDeviceFreeBytes < iBytesRequested.QuadPart)		
					g_MenuHandle->PostMessage(fcMSG_MESSAGEBOX_START,
					fcMT_DiskFree,	// yes, no
					(DWORD)this, NULL);
			}

			return;
		}
	}

	else if(g_FCGameData.bRemovedStorageDevice && g_FC.GetMainState() != GAME_STATE_SYSTEM_MESSAGE )
	{
		CloseAllAboutStorage();
		g_MenuHandle->PostMessage(fcMSG_MESSAGEBOX_START,
				fcMT_RemoveStorage,	// yesno
				(DWORD)this, NULL);
		g_FCGameData.bRemovedStorageDevice = false;
		m_bCanExit = false;
	}
	if(g_FCGameData.bCancelDeviceSelector )
	{
		g_MenuHandle->PostMessage(fcMSG_MESSAGEBOX_START,
				fcMT_CancelDeviceSelector,	// ok
				(DWORD)this, NULL);
		g_FCGameData.bCancelDeviceSelector = false;				
	}
	if(m_bNeedShowProfileDeleteWarn && m_bConfirmDevice)
	{
		g_MenuHandle->PostMessage(fcMSG_MESSAGEBOX_START,
				fcMT_ProfileDelete,	// ok
				(DWORD)this, NULL);
		m_bNeedShowProfileDeleteWarn = false;				
	}
	

#endif // _XBOX

	if(g_FC.GetMainState() != GAME_STATE_SYSTEM_MESSAGE)
	{
		switch( m_nSaveState )
		{
		case SAVE_STATE_NEWDATA: 
			{
				g_FCGameData.SaveUserGameData(NULL);
				
				BsUiWindow* pWnd = g_BsUiSystem.GetFocusWindow();

				m_pLayer->UpdateListBox();

				g_BsUiSystem.SetFocusWindow(pWnd);	

				m_nSaveState = SAVE_STATE_NONE;

			}
			break;
		case SAVE_STATE_OVERWRITE:
			{	
#ifdef _XBOX
				g_FCGameData.SaveUserGameData(m_pLayer->GetSelectedContent());
#endif
		
				BsUiWindow* pWnd = g_BsUiSystem.GetFocusWindow();

				m_pLayer->UpdateListBox();
				m_pLayer->UpdateLoadInfo();

				g_BsUiSystem.SetFocusWindow(pWnd);				

				m_nSaveState = SAVE_STATE_NONE;
			}
			break;
		case SAVE_STATE_NEWFIXED:
			{	
				g_FCGameData.SaveFixedGameData();
		
				m_nSaveState = SAVE_STATE_NONE;
			}
			break;
		case SAVE_STATE_NONE:
		default:
			break;
		};
	}

}


void CFcMenuSaveData::RenderProcess()
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

void CFcMenuSaveData::RenderMoveLayer(int nTick)
{
	if(nTick <= _TICK_FADE_IN_BASE)
	{
		float fAlpha = (float)nTick / _TICK_FADE_IN_BASE;
		m_pLayer->SetColor(D3DXCOLOR(1, 1, 1, fAlpha));
	}
}


const int STORAGE_MSGS[] = 
{
	fcMT_SaveGameData,
	fcMT_OverWriteGameData,
	fcMT_DiskFree,
	fcMT_ProfileDelete,
	fcMT_ExitClearAllSave,
	fcMT_ExitClearStageSave,
};

void CFcMenuSaveData::CloseAllAboutStorage() 
{	
	for(int i=0; i<sizeof(STORAGE_MSGS); ++i)
	{	
		CFcMenuForm* pMsgBoxMenu = g_MenuHandle->GetMessageBoxMenu(STORAGE_MSGS[i]);
		if(pMsgBoxMenu!=NULL)
			g_MenuHandle->PostMessage(fcMSG_MESSAGEBOX_END,
					(DWORD)pMsgBoxMenu,
					(DWORD)g_FC.GetMainState(), //(DWORD)m_dwMainState,
					(DWORD)this);				//(DWORD)m_pCmdMenu,
	}
}
//-----------------------------------------------------------------------------------------------------
CFcMenuSaveDataLayer::CFcMenuSaveDataLayer(CFcMenuSaveData* pMenu)
{
	m_pMenu = pMenu;
	m_nLoadImageStageID = -1;

	for(int i=0; i<SAVEMENU_SLOTS_MAX; ++i)
		m_pBtns[i] = NULL;

	m_nStartLine=0;
	m_nFocusedBtn=0;
	m_bEnableNewSaveData = true;
}

void CFcMenuSaveDataLayer::Initialize()
{
	// Btn 컨트롤러를 얻어와 변수에 저장해 둔다.
	m_pBtns[0] = (BsUiButton*)GetWindow(BT_Save_1);
	m_pBtns[1] = (BsUiButton*)GetWindow(BT_Save_2);
	m_pBtns[2] = (BsUiButton*)GetWindow(BT_Save_3);

	m_pImages[0] = (BsUiImageCtrl*)GetWindow(IM_Save_1);		
	m_pImages[1] = (BsUiImageCtrl*)GetWindow(IM_Save_2);		
	m_pImages[2] = (BsUiImageCtrl*)GetWindow(IM_Save_3);		
	

	UpdateListBox();
	UpdateLoadInfo();

	if(m_items.empty() || (int)m_items.size() < m_nStartLine + m_nFocusedBtn + 1) {
		m_nStartLine =0;
		m_nFocusedBtn=0;
	}

	SetFocusWindowClass(m_pBtns[m_nFocusedBtn]->GetClass());
}

DWORD CFcMenuSaveDataLayer::ProcMessage(xwMessageToken* pMsgToken)
{
	BsAssert(pMsgToken);

	BsUiCLASS hClass = BsUi::BsUiGetClass(pMsgToken->hWnd);

	switch(pMsgToken->message)
	{
	case XWMSG_KEYDOWN:
		{
			switch (pMsgToken->lParam)
			{
			case MENU_INPUT_B:
				{
					_FC_MENU_TYPE nPrevType = ((CFcMenuSaveData*)m_pMenu)->GetPrevType();
					switch(nPrevType)
					{
					case _FC_MENU_USUAL:
						{
							m_pMenu->SetStatus(_MS_CLOSING);
							g_MenuHandle->PostMessage(fcMSG_SUB_TO_USUAL);
							break;
						}
					default:
						{
							if(m_pMenu->GetClearChar() != MAX_MISSION_TYPE)
							{
								g_MenuHandle->PostMessage(fcMSG_MESSAGEBOX_START,
									fcMT_ExitClearAllSave,
									(DWORD)m_pMenu, pMsgToken->hWnd);
							}
							else
							{
								g_MenuHandle->PostMessage(fcMSG_MESSAGEBOX_START,
									fcMT_ExitClearStageSave,
									(DWORD)m_pMenu, pMsgToken->hWnd);
							}
							break;
						}
					}
					
					break;
				}
			case MENU_INPUT_Y:
				{
					SetFocusWindowClass(m_pBtns[0]->GetClass());
					m_nFocusedBtn = 0;
					m_nStartLine = 0;
					g_FCGameData.bShowDeviceUI = false;
					g_FCGameData.bChangeDevice = true;
					((CFcMenuSaveData*)m_pMenu)->NotifyDeviceChanged(pMsgToken->hWnd);
					break;
				}
			case MENU_INPUT_UP:
				{
					if(m_nFocusedBtn > 0) 
					{
						BsUiWindow* pPrev = SetFocusPrevChileWindow(pMsgToken->hWnd, true);					
						--m_nFocusedBtn;
						if(pPrev != NULL && pPrev->GetHWnd() != pMsgToken->hWnd  )
							g_pSoundManager->PlaySystemSound( SB_COMMON,"SYS_CURSOR");
					}
					else if(m_nStartLine > 0)
					{
						--m_nStartLine;
						g_pSoundManager->PlaySystemSound( SB_COMMON,"SYS_CURSOR");
					}
					UpdateSlots();
					UpdateLoadInfo();
					break;
				}	
			case MENU_INPUT_DOWN:
				{
					if(m_nFocusedBtn < int(__min(SAVEMENU_SLOTS_MAX, m_items.size())-1) )
					{
						BsUiWindow* pNext = SetFocusNextChileWindow(pMsgToken->hWnd, true);
						++m_nFocusedBtn;
						if(pNext != NULL && pNext->GetHWnd() != pMsgToken->hWnd )
                            g_pSoundManager->PlaySystemSound( SB_COMMON,"SYS_CURSOR");
					}
					else if(m_nStartLine+m_nFocusedBtn +1 < int(m_items.size()) )
					{
						++m_nStartLine;
						g_pSoundManager->PlaySystemSound( SB_COMMON,"SYS_CURSOR");
					}
					UpdateSlots();
					UpdateLoadInfo();
					break;
				}
			default:
				{
					if(m_items.empty() || (int)m_items.size() < m_nStartLine + m_nFocusedBtn + 1) 
						break;
					BsUiCLASS hClass = BsUi::BsUiGetClass(pMsgToken->hWnd);
					switch(hClass)
					{
					case BT_Save_1:
					case BT_Save_2:
					case BT_Save_3:	
						OnKeyDownLbSaveData(pMsgToken); break;
					}
				}
			}
			break;
		}
	}

	return BsUiWindow::ProcMessage(pMsgToken);
}


bool			CFcMenuSaveDataLayer::IsFocusedOnListBtn()  
{
	BsUiWindow* pWnd = g_BsUiSystem.GetFocusWindow();

	if( pWnd == GetWindow(BT_Save_1) 
		|| pWnd == GetWindow(BT_Save_2)
		|| pWnd == GetWindow(BT_Save_3) )
		return true;
	return false;
}

void CFcMenuSaveDataLayer::OnKeyDownLbSaveData(xwMessageToken* pMsgToken)
{
	switch (pMsgToken->lParam)
	{
	case PAD_INPUT_A:
		{
			if(g_FCGameData.m_DeviceID == XCONTENTDEVICE_ANY
				|| g_FCGameData.bChangeDevice) 
				break;
			
			if(m_nStartLine+m_nFocusedBtn==0 && m_bEnableNewSaveData){
				g_MenuHandle->PostMessage(fcMSG_MESSAGEBOX_START,
					fcMT_SaveGameData,
					(DWORD)m_pMenu, pMsgToken->hWnd);
			}
			else{
				g_MenuHandle->PostMessage(fcMSG_MESSAGEBOX_START,
					fcMT_OverWriteGameData,
					(DWORD)m_pMenu, pMsgToken->hWnd);
			}

			break;
		}
	case PAD_INPUT_UP:	UpdateLoadInfo(); break;
	case PAD_INPUT_DOWN:UpdateLoadInfo(); break;
	}
}


void CFcMenuSaveDataLayer::UpdateListBox()
{	
#ifdef _XBOX
	m_bEnableNewSaveData = false;
	m_items.clear();	

	// 리스트 StartLine 로부터 3개를 읽어와 버튼 업데이트
	if(g_FCGameData.m_DeviceID == XCONTENTDEVICE_ANY) {
		m_nStartLine = 0;
		m_nFocusedBtn = 0;
		UpdateSlots();
		return;
	}

	if( g_FCGameData.m_DeviceID != XCONTENTDEVICE_ANY) 
	{
		ULARGE_INTEGER iBytesRequested = {0};			
		int nDataSize = g_FCGameData.tempUserSave.GetSaveDataSize();				
		//if(!g_FCGameData.bFixedSaveData) // 유저 프로필 파일이 없을 경우 이를 생성하기 위한 용량
		//	nDataSize+=FIXEDSAVEDATA_ENOUGHSIZE;
		if(!g_FCGameData.bFixedSaveData && g_FCGameData.nSaveGameCount==0) // 새로운 파일을 위한 폴더 생성 요구 용량
			nDataSize+=FIRSTCREATE_ENOUGHSIZE;
		iBytesRequested.QuadPart = XContentCalculateSize(nDataSize, 1);
		XDEVICE_DATA  data={0,};
		if(ERROR_SUCCESS != XContentGetDeviceData(g_FCGameData.m_DeviceID, &data))
		{
			// 중간에 디바이스 장치가 제거 되었거나 기타 오류 처리
			return;
		}		

		if(g_FCGameData.GetUserSaveGameCount() < _MAX_USER_SAVE_DATA_COUNT
			&& data.ulDeviceFreeBytes >= iBytesRequested.QuadPart)
			m_bEnableNewSaveData = true;
		// 용량이 없음
		if( data.ulDeviceFreeBytes < iBytesRequested.QuadPart)
		{
			//g_MenuHandle->PostMessage(fcMSG_MESSAGEBOX_START,
			//	fcMT_DiskFull,	// ok
			//	(DWORD)this, NULL);
		}
	}
	if(m_bEnableNewSaveData)
		m_items.push_back(std::make_pair("New Save Data", (XCONTENT_DATA*)0));

	DWORD dwStartCount = g_FCGameData.nSaveGameCount;
	for(DWORD i=0; i<dwStartCount; ++i)
	{
		XCONTENT_DATA* pContentData = g_FCGameData.GetContentData(i);
		BsAssert(pContentData);

		if(strcmp(pContentData->szFileName, _FIXED_SAVE_FILENAME) == 0){
			continue;
		}

		SavedDataInfo DataInfo;
		g_FCGameData.GetSavedFileInfo(pContentData, &DataInfo);

		//hero---------------------------------------------------
		char cHeroName[64];
		int nTextID = _TEX_Inphyy + g_SoxToMissionID[DataInfo.nHeroID];
		g_TextTable->GetText(nTextID, cHeroName, _countof(cHeroName));

		//
		char cLevel[64];
		g_TextTable->GetText(_TEX_SUM_LEVEL, cLevel, _countof(cLevel));


		//day----------------------------------------------------
		char cDay[256];
		switch(g_LocalLanguage.GetLanguage())
		{
		case LANGUAGE_KOR:
		case LANGUAGE_JPN:
		case LANGUAGE_CHN:
			{
				sprintf(cDay, "%d / %d / %d", DataInfo.nYear, DataInfo.nMonth, DataInfo.nDay);
				break;
			}
		case LANGUAGE_ENG:
		case LANGUAGE_GER:
		case LANGUAGE_FRA:
		case LANGUAGE_SPA:
		case LANGUAGE_ITA:
		default:
			{
				sprintf(cDay, "%d / %d / %d", DataInfo.nMonth, DataInfo.nDay, DataInfo.nYear);
				break;
			}
		}
		//time----------------------------------------------------
		int nH12 = DataInfo.nHour;
		nH12 += ( DataInfo.nHour == 0 ) ? 12 : ( ( DataInfo.nHour > 12 ) ? -12 : 0 );
		int nAMPM = ( DataInfo.nHour < 12 ) ? _TEX_AM : _TEX_PM;

		char cAMPM[64];
		g_TextTable->GetText(nAMPM, cAMPM, _countof(cAMPM));

		char cTime[256];
		switch(g_LocalLanguage.GetLanguage())
		{
		case LANGUAGE_KOR:
		case LANGUAGE_JPN:
		case LANGUAGE_CHN:
			{
				sprintf(cTime, "%s %d:%02d", cAMPM, nH12, DataInfo.nMin);
				break;
			}
		case LANGUAGE_ENG:
		case LANGUAGE_GER:
		case LANGUAGE_FRA:
		case LANGUAGE_SPA:
		case LANGUAGE_ITA:
		default:
			{
				sprintf(cTime, "%d : %02d %s", nH12, DataInfo.nMin, cAMPM);
				break;
			}
		}
	
		//stage name----------------------------------------------------
		char cStageName[64];
		g_FCGameData.GetStageName(DataInfo.nStageID, cStageName, _countof(cStageName), DataInfo.nHeroID);	

		//
		char cCaption[256];
		sprintf_s(cCaption, _countof(cCaption), "@(space=-3)%s  %s %d \n@(space=+3)@(scale=0.8,0.8)%s\n%s   %s",	cHeroName, cLevel, DataInfo.nLevel+1, cStageName , cDay, cTime);

		m_items.push_back(std::make_pair(cCaption, pContentData));
	}
	
//	ITEMS_TYPE reversed_items(m_items.rbegin(), m_items.rend());
//	std::swap(m_items, reversed_items);
	

	// 버튼들을 업데이트 한다.
	UpdateSlots();

	
#endif //_XBOX

}


void CFcMenuSaveDataLayer::UpdateLoadInfo()
{
#ifdef _XBOX

	
	BsUiImageCtrl* pImg = (BsUiImageCtrl*)GetWindow(IM_SaveStage);
	pImg->SetWindowAttr(XWATTR_SHOWWINDOW, true);
	BsUiImageCtrl* pRankImg = (BsUiImageCtrl*)GetWindow(IM_SaveRank);
	pRankImg->SetWindowAttr(XWATTR_SHOWWINDOW, true);

	int index = m_nStartLine + m_nFocusedBtn;	
	XCONTENT_DATA *pContent = NULL;
	if(int(m_items.size()) > index)
		pContent = m_items[index].second;
	if(g_FCGameData.m_DeviceID ==  XCONTENTDEVICE_ANY
		|| !pContent
		|| m_items.size() == 0)
	{		
		GetWindow(TT_SaveStage)->SetItemTextID(-1);
		GetWindow(TT_SaveClear_num)->SetItemTextID(-1);		
		pImg->SetWindowAttr(XWATTR_SHOWWINDOW, false);
		pRankImg->SetWindowAttr(XWATTR_SHOWWINDOW, false);
		return;
	}
	
	SavedDataInfo DataInfo;
		
	g_FCGameData.GetSavedFileInfo(pContent, &DataInfo);

	//stage name----------------------------------------------------
	char cStageName[64];
	g_FCGameData.GetStageName(DataInfo.nStageID, cStageName, _countof(cStageName), DataInfo.nHeroID);	
	GetWindow(TT_SaveStage)->SetItemText(cStageName);

	//clear count
	char cClearCount[64] = {0,};
	HeroRecordInfo *pHeroRecord = g_FCGameData.tempFixedSave.GetHeroRecordInfo(DataInfo.nHeroID);
	StageResultInfo *pRecord = NULL;
	if(pHeroRecord)
		pRecord = pHeroRecord->GetStageResultInfo(DataInfo.nStageID);
	if(pHeroRecord && pRecord) 
	{
		sprintf(cClearCount, "%d", pRecord->nMissionClearCount);
	}
	else
		sprintf(cClearCount, "%d", 0);
	GetWindow(TT_SaveClear_num)->SetItemText(cClearCount);

	//image----------------------------------------------------
	int nPlayGrade = 5;	
	if(pRecord)
		nPlayGrade = pRecord->nPlayGrade;
	UpdateLoadRankImage(nPlayGrade);
	UpdateLoadStageImage(DataInfo.nHeroID, DataInfo.nStageID);

#endif

}
void CFcMenuSaveDataLayer::UpdateLoadRankImage(int nRank) 
{
    // image
	std::string filename;	
	switch(nRank) 
	{
	case 0: filename = "mn_result_s.dds";		break;
	case 1:	filename = "mn_result_a.dds";		break;
	case 2:	filename = "mn_result_b.dds";		break;
	case 3:	filename = "mn_result_c.dds";		break;
	case 4:	filename = "mn_result_d.dds";		break;
	case 5:	filename = "mn_result_f.dds";		break;	
	default:
		BsAssert(0);		
		break;
	};
	CBsKernel::GetInstance().chdir("Interface");	
	int nTexId = g_BsKernel.LoadTexture(filename.c_str());
	BsAssert(nTexId != -1);	
	CBsKernel::GetInstance().chdir("..");

	BsUiImageCtrl* pImg = (BsUiImageCtrl*)GetWindow(IM_SaveRank);
	ImageCtrlInfo info;
	info.nTexId = nTexId;
	info.u1 = 0;	info.v1 = 0;
	info.u2 = 150;	info.v2 = 150;
	pImg->SetImageInfo(&info);
}

void CFcMenuSaveDataLayer::UpdateLoadStageImage(int nHeroSoxID, int nStageId)
{	
	//STAGE_ID_VARRVAZZAR와 STAGE_ID_VARRVAZZAR_2가 서로 엮이는 문제..(기획 쪽 ㅡ.ㅡ)로 하드 코드 합니다.
	switch(nHeroSoxID)
	{
	case CHAR_ID_MYIFEE:
	case CHAR_ID_DWINGVATT:
		{
			if(nStageId == STAGE_ID_VARRVAZZAR){
				nStageId = STAGE_ID_VARRVAZZAR_2;
			}
			else if(nStageId == STAGE_ID_VARRVAZZAR_2){
				nStageId = STAGE_ID_VARRVAZZAR;
			}
			break;
		}
	}

	if(m_nLoadImageStageID == nStageId){
		return;
	}

	m_nLoadImageStageID = nStageId;

	char cImageFile[256];
	switch(nStageId)
	{
	case STAGE_ID_VARRGANDD:			strcpy(cImageFile, "mn_Save_vg.dds"); break;
	case STAGE_ID_OUT_VARRGANDD:		strcpy(cImageFile, "mn_Save_vg.dds"); break;
	case STAGE_ID_VARRFARRINN:			strcpy(cImageFile, "mn_Save_vf.dds"); break;
	case STAGE_ID_OUT_VARRFARRINN:		strcpy(cImageFile, "mn_Save_vf_outside.dds"); break;
	case STAGE_ID_VARRVAZZAR:			strcpy(cImageFile, "mn_Save_vz.dds"); break;
	case STAGE_ID_OUT_VARRVAZZAR:		strcpy(cImageFile, "mn_Save_vz_outside.dds"); break;
	case STAGE_ID_WYANDEEK:				strcpy(cImageFile, "mn_Save_wy.dds"); break;
	case STAGE_ID_WYANDEEK_VILLAGE:		strcpy(cImageFile, "mn_Save_wy.dds"); break;
	case STAGE_ID_YWA_UE_UAR:			strcpy(cImageFile, "mn_Save_yw.dds"); break;
	case STAGE_ID_EAURVARRIA:			strcpy(cImageFile, "mn_Save_ev.dds"); break;
	case STAGE_ID_FELPPE:				strcpy(cImageFile, "mn_Save_he.dds"); break;
	case STAGE_ID_PHOLYA:				strcpy(cImageFile, "mn_Save_ph.dds"); break;
	case STAGE_ID_BEFORE_PHOLYA_1:		strcpy(cImageFile, "mn_Save_ph.dds"); break;
	case STAGE_ID_BEFORE_PHOLYA_2:		strcpy(cImageFile, "mn_Save_ph.dds"); break;
	case STAGE_ID_ANOTHER_WORLD:		strcpy(cImageFile, "mn_Save_ma.dds"); break;
	case STAGE_ID_VARRVAZZAR_2:			strcpy(cImageFile, "mn_Save_vz_entrance.dds"); break;
	default: BsAssert(0);
	}

	CBsKernel::GetInstance().chdir("Interface");
	CBsKernel::GetInstance().chdir("SaveImage");
	int nTexId = g_BsKernel.LoadTexture(cImageFile);
	BsAssert(nTexId != -1);
	CBsKernel::GetInstance().chdir("..");
	CBsKernel::GetInstance().chdir("..");

	BsUiImageCtrl* pImg = (BsUiImageCtrl*)GetWindow(IM_SaveStage);
	ImageCtrlInfo info;
	info.nTexId = nTexId;
	info.u1 = 0;	info.v1 = 0;
	info.u2 = 256;	info.v2 = 128;
	pImg->SetImageInfo(&info);
}


void CFcMenuSaveDataLayer::UpdateLoadHeroImage(int nSlot, int nHeroId)
{	
	// image
	std::string filename ="mn_save_none.dds";	
	switch(nHeroId) 
	{
	case _TEX_Inphyy: 	filename = "mn_save_in.dds";		break;
	case _TEX_Aspharr:	filename = "mn_save_as.dds";		break;
	case _TEX_Myifee:	filename = "mn_save_my.dds";		break;
	case _TEX_Tyurru:	filename = "mn_save_ty.dds";		break;
	case _TEX_Klarrann:	filename = "mn_save_kl.dds";		break;
	case _TEX_Dwingvatt:filename = "mn_save_dw.dds";		break;
	case _TEX_Vigkvagk:	filename = "mn_save_vi.dds";		break;
	default:	
		filename = "mn_save_none.dds";
		break;
	};
	CBsKernel::GetInstance().chdir("Interface");
	CBsKernel::GetInstance().chdir("SaveImage");
	int nTexId = g_BsKernel.LoadTexture(filename.c_str());
	BsAssert(nTexId != -1);
	CBsKernel::GetInstance().chdir("..");
	CBsKernel::GetInstance().chdir("..");

	ImageCtrlInfo info;
	info.nTexId = nTexId;
	info.u1 = 0;	info.v1 = 0;
	info.u2 = 256;	info.v2 = 113;		
	m_pImages[nSlot]->SetImageInfo(&info);

}
void CFcMenuSaveDataLayer::UpdateSlots()
{	

	for(DWORD i=0; i<SAVEMENU_SLOTS_MAX; ++i) 
	{
        size_t index = m_nStartLine + i;
		if(index < m_items.size()) 
		{
			// caption
			std::string caption = m_items[index].first;
			m_pBtns[i]->SetItemText((char*)caption.c_str());		
			
			// image
			D3DXCOLOR col = m_pImages[i]->GetColor();
			if(m_items[index].second == NULL) {
				UpdateLoadHeroImage(i, -1);				
			}
			else {
				SavedDataInfo DataInfo;
				g_FCGameData.GetSavedFileInfo(m_items[index].second, &DataInfo);
				int nTextID = _TEX_Inphyy + g_SoxToMissionID[DataInfo.nHeroID];			
				UpdateLoadHeroImage(i, nTextID);
			}
			// alpha			
			if( i == m_nFocusedBtn )
				col.a = 1.0f;
			else
				col.a = 0.4f;
			m_pImages[i]->SetColor(col);
		}
		else
		{
            m_pBtns[i]->SetItemText("");
			D3DXCOLOR col = m_pImages[i]->GetColor();
			col.a = 0.0f;
			m_pImages[i]->SetColor(col);
		}
	}
}