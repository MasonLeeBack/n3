#include "stdafx.h"

#include "FcMenuLoadData.h"
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

#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG


#define _WAIT_SAVE_TIME	80



CFcMenuLoadData::CFcMenuLoadData(_FC_MENU_TYPE nType, _FC_MENU_TYPE nPrevType)
: CFcMenuForm(nType)
{
	m_pLayer = new CFcMenuLoadDataLayer(this);
	AddUiLayer(m_pLayer, SN_LoadData);
	m_pLayer->Initialize();

	m_nPrevType = nPrevType;

	g_FCGameData.bCheckFreeSpace = false;
	m_bConfirmDevice = true;
	m_bNeedShowProfileDeleteWarn = false;

	m_bNeedSaveFixed = false;
	m_bWaitSaving = false;

}
CFcMenuLoadData::~CFcMenuLoadData() 
{
	g_FCGameData.bCheckFreeSpace = true;
}

void CFcMenuLoadData::Process()
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

			if( data.ulDeviceFreeBytes < STORAGE_ENOUGHSIZE)		{
				g_MenuHandle->PostMessage(fcMSG_MESSAGEBOX_START,
					fcMT_DiskFree,	// yes, no
					(DWORD)this, NULL);
				m_bConfirmDevice = false;
			}

			if(g_FCGameData.bFixedSaveData == true) 
			{
                m_bNeedShowProfileDeleteWarn = true;
			}
			else 
			{		
				m_bNeedShowProfileDeleteWarn = false;
				//g_FCGameData.SaveFixedGameData();	
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

		BsUiWindow* pWnd = NULL;
		if(m_pLayer->IsFocusedOnListBtn()==false)
			pWnd = g_BsUiSystem.GetFocusWindow();
		m_pLayer->Initialize();
		if(pWnd)
			g_BsUiSystem.SetFocusWindow(pWnd);
		g_FCGameData.m_OldDeviceID = g_FCGameData.m_DeviceID;
		g_FCGameData.bChangeDevice = false;

	}

	if(g_FC.GetMainState() != GAME_STATE_SYSTEM_MESSAGE)
	{

		if(g_FCGameData.bRemovedStorageDevice )
		{
			CloseAllAboutStorage();
			g_MenuHandle->PostMessage(fcMSG_MESSAGEBOX_START,
				fcMT_RemoveStorage,	// yesno
				(DWORD)this, NULL);
			g_FCGameData.bRemovedStorageDevice = false;
		}
		if(g_FCGameData.bCancelDeviceSelector )
		{
			g_MenuHandle->PostMessage(fcMSG_MESSAGEBOX_START,
				fcMT_CancelDeviceSelector,	// ok
				(DWORD)this, NULL);
			g_FCGameData.bCancelDeviceSelector = false;				
		}

		if(m_bNeedShowProfileDeleteWarn && m_bConfirmDevice )
		{
			g_MenuHandle->PostMessage(fcMSG_MESSAGEBOX_START,
				fcMT_ProfileDelete,	// ok
				(DWORD)this, NULL);
			m_bNeedShowProfileDeleteWarn = false;	
			m_bConfirmDevice = false;
		}

		if(m_bNeedShowProfileDeleteWarn==false 
			&& m_bConfirmDevice
			&& m_bNeedSaveFixed
			&& m_bWaitSaving == false)
		{
			g_MenuHandle->PostMessage(fcMSG_MESSAGEBOX_START,
				fcMT_SavingNow,
				(DWORD)this, NULL);

			m_nSaveProcessTick = GetProcessTick();
			m_bWaitSaving = true;
			return;

		}

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
	}
#endif // _XBOX
}

void CFcMenuLoadData::ProcessMsg(int nMsgAsk, BsUiHANDLE hWnd, int nMsgRsp)
{
	if(nMsgAsk == fcMT_LoadGameData || nMsgAsk == fcMT_LoadGameDataWarning )
	{
		if(nMsgRsp == ID_YES)
		{	
			//BsUiListBox* pListBox = (BsUiListBox*)(m_pLayer->GetWindow(LB_LoadData));
#ifdef _XBOX
			XCONTENT_DATA *pContent = m_pLayer->GetSelectedContent();
			if(!pContent)
			{
				g_BsUiSystem.SetFocusWindow(BsUi::BsUiGetWindow(hWnd));
				return;		
			}

			if(g_FCGameData.LoadUserGameData(pContent) == false)
			{
				g_MenuHandle->PostMessage(fcMSG_MESSAGEBOX_START,
					fcMT_FailLoadContent,	// ok
					(DWORD)this, hWnd);
			}
			else
			{
				if(g_FCGameData.bRemovedStorageDevice) 
					return;
				g_MenuHandle->PostMessage(fcMSG_GO_TO_LOAD_SELECT_STAGE);
			}
#endif //_XBOX
		}
		else
		{
			g_BsUiSystem.SetFocusWindow(BsUi::BsUiGetWindow(hWnd));
		}
	}
	else if(nMsgAsk == fcMT_FailLevelItem)
	{
		g_BsUiSystem.SetFocusWindow(BsUi::BsUiGetWindow(hWnd));
	} 
	else if( nMsgAsk == fcMT_DiskFree )	
	{
		if(nMsgRsp == ID_YES)
		{ 
			m_bConfirmDevice = true;			
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
		}
		else		
		{
            g_FCGameData.bShowDeviceUI = false;
			g_FCGameData.bChangeDevice = true;
		}
	}
}


void CFcMenuLoadData::RenderProcess()
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

void CFcMenuLoadData::RenderMoveLayer(int nTick)
{
	if(nTick <= _TICK_FADE_IN_BASE)
	{
		float fAlpha = (float)nTick / _TICK_FADE_IN_BASE;
		m_pLayer->SetColor(D3DXCOLOR(1, 1, 1, fAlpha));
	}
}


const int STORAGE_MSGS[] = 
{
	fcMT_LoadGameData,
	fcMT_LoadGameDataWarning,
	fcMT_DiskFree,
	fcMT_ProfileDelete,	
};

void CFcMenuLoadData::CloseAllAboutStorage() 
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
CFcMenuLoadDataLayer::CFcMenuLoadDataLayer(CFcMenuForm* pMenu)
{
	m_pMenu = pMenu;
	m_nLoadImageStageID = -1;

	for(int i=0; i<LOADMENU_SLOTS_MAX; ++i)
		m_pBtns[i] = NULL;

	m_nStartLine=0;
	m_nFocusedBtn=0;
}

void CFcMenuLoadDataLayer::Initialize()
{	
	// Btn 컨트롤러를 얻어와 변수에 저장해 둔다.
	m_pBtns[0] = (BsUiButton*)GetWindow(BT_Load_1);
	m_pBtns[1] = (BsUiButton*)GetWindow(BT_Load_2);
	m_pBtns[2] = (BsUiButton*)GetWindow(BT_Load_3);

	m_pImages[0] = (BsUiImageCtrl*)GetWindow(IM_Load_1);		
	m_pImages[1] = (BsUiImageCtrl*)GetWindow(IM_Load_2);		
	m_pImages[2] = (BsUiImageCtrl*)GetWindow(IM_Load_3);		
	

	UpdateListBox();
	UpdateLoadInfo();

	if(m_items.empty() || (int)m_items.size() < m_nStartLine + m_nFocusedBtn + 1) {
		m_nStartLine =0;
		m_nFocusedBtn=0;
	}
	
	SetFocusWindowClass(m_pBtns[m_nFocusedBtn]->GetClass());
}

DWORD CFcMenuLoadDataLayer::ProcMessage(xwMessageToken* pMsgToken)
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
					m_pMenu->SetStatus(_MS_CLOSING);
					_FC_MENU_TYPE nPrevType = ((CFcMenuLoadData*)m_pMenu)->GetPrevType();
					switch(nPrevType)
					{
					case _FC_MENU_TITLE:	g_MenuHandle->PostMessage(fcMSG_TITLE_START); break;
					case _FC_MENU_USUAL:	g_MenuHandle->PostMessage(fcMSG_SUB_TO_USUAL); break;
					default: BsAssert(0);
					}
					break;
				}
			case MENU_INPUT_Y:
				{
					SetFocusWindowClass(m_pBtns[0]->GetClass());
					m_nFocusedBtn = 0;
					g_FCGameData.bShowDeviceUI = false;
					g_FCGameData.bChangeDevice = true;
					((CFcMenuLoadData*)m_pMenu)->NotifyDeviceChanged(pMsgToken->hWnd);
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
					if(m_nFocusedBtn < int(__min(LOADMENU_SLOTS_MAX, m_items.size())-1) )
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
					case BT_Load_1:
					case BT_Load_2:
					case BT_Load_3:	
						OnKeyDownLbLoadData(pMsgToken); break;
					}
				}
			}
			break;
		}
	}

	return BsUiWindow::ProcMessage(pMsgToken);
}

bool			CFcMenuLoadDataLayer::IsFocusedOnListBtn()  
{
	BsUiWindow* pWnd = g_BsUiSystem.GetFocusWindow();

	if( pWnd == GetWindow(BT_Load_1) 
		|| pWnd == GetWindow(BT_Load_2)
		|| pWnd == GetWindow(BT_Load_3) )
		return true;
	return false;
}

void CFcMenuLoadDataLayer::OnKeyDownLbLoadData(xwMessageToken* pMsgToken)
{
	switch (pMsgToken->lParam)
	{
	case PAD_INPUT_A:
		{
			if(g_FCGameData.m_DeviceID == XCONTENTDEVICE_ANY) 
				break;

			_FC_MENU_TYPE nPrevType = ((CFcMenuLoadData*)m_pMenu)->GetPrevType();
			switch(nPrevType)
			{
			case _FC_MENU_TITLE: 
				g_MenuHandle->PostMessage(fcMSG_MESSAGEBOX_START,
					fcMT_LoadGameData,
					(DWORD)m_pMenu, pMsgToken->hWnd);
				break;
			case _FC_MENU_USUAL:	
				//!msg - 로드시에 현재 게임 데이타가 날라 갈 수 있음을 경고 확인 메시지를 띄움
				g_MenuHandle->PostMessage(fcMSG_MESSAGEBOX_START,
					fcMT_LoadGameDataWarning,	// yes no
					(DWORD)m_pMenu, pMsgToken->hWnd);
				break;
			default: BsAssert(0);
			}
			break;			
		}
	case PAD_INPUT_UP:	UpdateLoadInfo(); break;
	case PAD_INPUT_DOWN:UpdateLoadInfo(); break;
	}
}

void CFcMenuLoadDataLayer::UpdateListBox()
{
#ifdef _XBOX
	m_items.clear();
	// 리스트 StartLine 로부터 3개를 읽어와 버튼 업데이트
	if(g_FCGameData.m_DeviceID == XCONTENTDEVICE_ANY ) {
		m_nStartLine = 0;
		m_nFocusedBtn = 0;
		UpdateSlots();
		return;
	}


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
		sprintf_s(cCaption, _countof(cCaption), "@(space=-3)%s  %s %d \n@(space=+3)@(scale=0.8,0.8)%s\n%s   %s",	cHeroName, cLevel, DataInfo.nLevel+1, cStageName, cDay, cTime);

		m_items.push_back(std::make_pair(cCaption, pContentData));
	}

	//ITEMS_TYPE reversed_items(m_items.rbegin(), m_items.rend());
	//std::swap(m_items, reversed_items);


	// 버튼들을 업데이트 한다.
	UpdateSlots();

	
#endif //_XBOX

}

void CFcMenuLoadDataLayer::UpdateLoadInfo()
{
	
#ifdef _XBOX
	//BsUiListBox* pListBox = (BsUiListBox*)GetWindow(LB_LoadData);
	//BsAssert(pListBox);

	int index = m_nStartLine + m_nFocusedBtn;

	BsUiImageCtrl* pImg = (BsUiImageCtrl*)GetWindow(IM_LoadStage);
	pImg->SetWindowAttr(XWATTR_SHOWWINDOW, true);
	BsUiImageCtrl* pRankImg = (BsUiImageCtrl*)GetWindow(IM_LoadRank);
	pRankImg->SetWindowAttr(XWATTR_SHOWWINDOW, true);
	
	if(g_FCGameData.m_DeviceID == XCONTENTDEVICE_ANY || m_items.size() == 0)
	{		
		GetWindow(TT_LoadStage)->SetItemTextID(-1);
		GetWindow(TT_LoadClear_num)->SetItemTextID(-1);		
		pImg->SetWindowAttr(XWATTR_SHOWWINDOW, false);
		pRankImg->SetWindowAttr(XWATTR_SHOWWINDOW, false);
		return;
	}
	
	SavedDataInfo DataInfo;
	XCONTENT_DATA *pContent = m_items[index].second;
	g_FCGameData.GetSavedFileInfo(pContent, &DataInfo);

	//stage name----------------------------------------------------
	char cStageName[64];
	g_FCGameData.GetStageName(DataInfo.nStageID, cStageName, _countof(cStageName), DataInfo.nHeroID);	
	GetWindow(TT_LoadStage)->SetItemText(cStageName);

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
	GetWindow(TT_LoadClear_num)->SetItemText(cClearCount);

	//image----------------------------------------------------	
	int nPlayGrade = 5;
	if(pRecord)
		nPlayGrade = pRecord->nPlayGrade;
	UpdateLoadRankImage(nPlayGrade);
	UpdateLoadStageImage(DataInfo.nHeroID, DataInfo.nStageID);

#endif
	
}
void CFcMenuLoadDataLayer::UpdateLoadRankImage(int nRank) 
{
    // image
	std::string filename;	
	switch(nRank) 
	{
	case 0: 	filename = "mn_result_s.dds";		break;
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

	BsUiImageCtrl* pImg = (BsUiImageCtrl*)GetWindow(IM_LoadRank);
	ImageCtrlInfo info;
	info.nTexId = nTexId;
	info.u1 = 0;	info.v1 = 0;
	info.u2 = 150;	info.v2 = 150;
	pImg->SetImageInfo(&info);
}
void CFcMenuLoadDataLayer::UpdateLoadStageImage(int nHeroSoxID, int nStageId)
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

	BsUiImageCtrl* pImg = (BsUiImageCtrl*)GetWindow(IM_LoadStage);
	ImageCtrlInfo info;
	info.nTexId = nTexId;
	info.u1 = 0;	info.v1 = 0;
	info.u2 = 256;	info.v2 = 128;
	pImg->SetImageInfo(&info);
}


void CFcMenuLoadDataLayer::UpdateLoadHeroImage(int nSlot, int nHeroId)
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
		BsAssert(0);
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
void CFcMenuLoadDataLayer::UpdateSlots()
{	

	for(DWORD i=0; i<LOADMENU_SLOTS_MAX; ++i) 
	{
        size_t index = m_nStartLine + i;
		if(index < m_items.size()) 
		{
			// caption
			std::string caption = m_items[index].first;
			m_pBtns[i]->SetItemText((char*)caption.c_str());								
			// image
			SavedDataInfo DataInfo;
			g_FCGameData.GetSavedFileInfo(m_items[index].second, &DataInfo);
			int nTextID = _TEX_Inphyy + g_SoxToMissionID[DataInfo.nHeroID];			
			UpdateLoadHeroImage(i, nTextID);
			// alpha
			D3DXCOLOR col = m_pImages[i]->GetColor();
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