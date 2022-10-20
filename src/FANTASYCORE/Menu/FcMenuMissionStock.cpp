#include "stdafx.h"

#include "FcMenuMissionStock.h"
#include "BSuiSystem.h"

#include "BsUiClassIDManager.h"
#include "BsUiFontAttrManager.h"

#include "FcInterfaceManager.h"
#include "FcGlobal.h"

#include "FcSoundManager.h"
#include "LocalLanguage.h"
#include "FantasyCore.h"
#include "FcLiveManager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG

#define _CHAR_DATA_NONE		 "mn_chardata_no.dds"

CFcMenuMissionDB::CFcMenuMissionDB(_FC_MENU_TYPE nType, DWORD dwItemID)
: CFcMenuForm(nType)
{
	DWORD dwHeroType(0), dwStageType(0);
	if(dwItemID == LIB_MAX_COUNT * LIBRARY_SECTION_GAP)
	{
		dwHeroType = MISSION_TYPE_NONE;
		dwStageType = STAGE_ID_NONE;
	}
	else
	{
		CFcMenuLibraryManager* pLibMgr = g_MenuHandle->GetLibManager();
		LibraryTable* pItem = pLibMgr->GetItemData(dwItemID);
		BsAssert(pItem);
		dwHeroType = g_SoxToMissionID[pItem->nCharCondition];
		dwStageType = pItem->nStageCondition;
	}
	
	m_pBaseLayer = new BsUiLayer();
	AddUiLayer(m_pBaseLayer, SN_Msn0, -1, -1);

	m_pHeroLayer = new CFcMenuMssnHeroLayer(this);
	AddUiLayer(m_pHeroLayer, SN_Msn1, -1, -1);

	m_pStageLayer = new CFcMenuMssnStageLayer(this);
	AddUiLayer(m_pStageLayer, SN_Msn2, -1, -1);

	UpdateMissionData();
	UpdateHeroLayer(dwHeroType);
	UpdateStageLayer(dwHeroType, dwStageType);

	BsUiListBox* pHeroList = (BsUiListBox*)(m_pHeroLayer->GetWindow(LB_Msn));
	BsUiListBox* pStageList = (BsUiListBox*)(m_pStageLayer->GetWindow(LB_Msn));

	if((int)dwStageType == STAGE_ID_NONE){
		g_BsUiSystem.SetFocusWindow(pHeroList);
	}else{
		g_BsUiSystem.SetFocusWindow(pStageList);
	}
}


void CFcMenuMissionDB::RenderProcess()
{
	switch(GetStatus())
	{
	case _MS_OPEN:
		{
			m_pBaseLayer->SetColor(D3DXCOLOR(1, 1, 1, 0.f));
			m_pHeroLayer->SetColor(D3DXCOLOR(1, 1, 1, 0.f));
			m_pStageLayer->SetColor(D3DXCOLOR(1, 1, 1, 0.f));

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
	case _MS_CLOSE:	break;
	}
}

#define _START_POS_X_MSSN1				-640
#define _END_POS_X_MSSN1				0
#define _START_POS_X_MSSN2				1280
#define _END_POS_X_MSSN2				640
void CFcMenuMissionDB::RenderMoveLayer(int nTick)
{
	//------------------------------------------------------
	if(nTick <= _TICK_FADE_IN_BASE)
	{
		float fAlpha = (float)nTick / _TICK_FADE_IN_BASE;
		m_pBaseLayer->SetColor(D3DXCOLOR(1, 1, 1, fAlpha));
	}

	//------------------------------------------------------
	if(nTick <= _TICK_IN_FORM_LEFT)
	{	
		float frate = 1.f - ((float)nTick / _TICK_IN_FORM_LEFT);
		int nX = (int)(_START_POS_X_MSSN1 * frate);
		m_pHeroLayer->SetWindowPosX(nX);

		float fAlpha = 1.f - frate;
		m_pHeroLayer->SetColor(D3DXCOLOR(1, 1, 1, fAlpha));
	}

	//------------------------------------------------------
	if(nTick <= _TICK_IN_FORM_RIGHT)
	{
		float frate = (float)nTick / _TICK_IN_FORM_RIGHT;
		int nX = _END_POS_X_MSSN2 + int((_START_POS_X_MSSN2 - _END_POS_X_MSSN2) * (1.f - frate));
		m_pStageLayer->SetWindowPosX(nX);

		float fAlpha = frate;
		m_pStageLayer->SetColor(D3DXCOLOR(1, 1, 1, fAlpha));
	}
}

void CFcMenuMissionDB::UpdateMissionData()
{	
	for(int i=0; i<MAX_MISSION_TYPE; i++)
	{
		for(int j=0; j<MAX_STAGE; j++)
		{	
			m_nStageOpenList[i][j] = -1;
			m_nStageItemList[i][j] = -1;
			m_nStageSortList[i][j] = -1;
		}
	}

	CFcMenuLibraryManager* pLibMgr = g_MenuHandle->GetLibManager();
	int nEndItemID = pLibMgr->GetItemID(LIB_ART, 0) - 1;
	int nStartMissionType = MISSION_TYPE_INPHYY;
	int nCount = 0;
	for(int i=0; i<LIBRARY_SECTION_GAP;i++)
	{
		int nItemIndex = pLibMgr->GetItemIndex(LIB_MISSION, i);
		LibraryTable* pItem = pLibMgr->GetItemDataforIndex(nItemIndex);
		if(pItem->nID >= nEndItemID){
			break;
		}

		int nState = pLibMgr->GetState(pItem->nID);

		int nMissionType = g_SoxToMissionID[pItem->nCharCondition];
		if(nStartMissionType != nMissionType)
		{
			nStartMissionType = nMissionType;
			nCount = 0;
		}

		m_nStageOpenList[nMissionType][pItem->nStageCondition] = nState;
		m_nStageItemList[nMissionType][pItem->nStageCondition] = pItem->nID;
		m_nStageSortList[nMissionType][nCount] = pItem->nStageCondition;
		nCount++;
	}
}


void CFcMenuMissionDB::UpdateHeroLayer(DWORD dwHeroType)
{
	if((int)dwHeroType == MISSION_TYPE_NONE){
		dwHeroType = MISSION_TYPE_INPHYY;
	}
	
	BsUiListBox* pList = (BsUiListBox*)(m_pHeroLayer->GetWindow(LB_Msn));
	BsUiListBox* pIconList = (BsUiListBox*)(m_pHeroLayer->GetWindow(LB_Msn_icon));

	pList->ClearItem();
	pIconList->ClearItem();

	for(int i=0; i<MAX_MISSION_TYPE; i++)
	{
		int nState = LIS_DEFAULT;
		for(int j=0; j<MAX_STAGE; j++)
		{
			if(m_nStageOpenList[i][j] == LIS_DEFAULT){
				;
			}else if(m_nStageOpenList[i][j] == LIS_NEW){
				nState = LIS_NEW;
			}else if(m_nStageOpenList[i][j] == LIS_OPEN){
				if(nState != LIS_NEW){
					nState = LIS_OPEN;
				}
			}
		}

		switch(nState)
		{
		case LIS_DEFAULT:
			{	
				pIconList->AddItem("", LIS_DEFAULT);
				pList->AddItem(_TEX_NOT_OPEN_DATA, i);
				break;
			}
		case LIS_NEW:
			{
				pIconList->AddItem("@(New_icon)", LIS_NEW);
				pList->AddItem(_TEX_Inphyy + i, i);
				break;
			}
		case LIS_OPEN:
			{
				pIconList->AddItem("", LIS_OPEN);
				pList->AddItem(_TEX_Inphyy + i, i);
				break;
			}
		}
	}

	pList->SetCurSel(dwHeroType);
	pIconList->SetCurSel(dwHeroType);
}

void CFcMenuMissionDB::UpdateStageLayer(DWORD dwHeroType, DWORD dwStageType)
{
	if((int)dwHeroType == MISSION_TYPE_NONE){
		dwHeroType = MISSION_TYPE_INPHYY;
	}

	BsUiListBox* pList = (BsUiListBox*)(m_pStageLayer->GetWindow(LB_Msn_stage));
	BsUiListBox* pIconList = (BsUiListBox*)(m_pStageLayer->GetWindow(LB_Msn_stage_icon));

	pList->ClearItem();
	pIconList->ClearItem();

	int nCurSelPos = 0;
	int nCount = 0;
	int bNewOpen = false;
	for(int i=0; i<MAX_STAGE; i++)
	{
		if(m_nStageSortList[dwHeroType][i] == -1){
			continue;
		}

		int nStageID = m_nStageSortList[dwHeroType][i];
		int nHeroSoxID = g_MissionToSoxID[dwHeroType];

		int nTexID = _TEX_VARRGANDD + nStageID;
		//STAGE_ID_VARRVAZZAR와 STAGE_ID_VARRVAZZAR_2가 서로 엮이는 문제..(기획 쪽 ㅡ.ㅡ)로 하드 코드 합니다.
		switch(nHeroSoxID)
		{
		case CHAR_ID_MYIFEE:
		case CHAR_ID_DWINGVATT:
			{
				if(nStageID == STAGE_ID_VARRVAZZAR){
					nTexID = _TEX_VARRGANDD + STAGE_ID_VARRVAZZAR_2;
				}
				else if(nStageID == STAGE_ID_VARRVAZZAR_2){
					nTexID = _TEX_VARRGANDD + STAGE_ID_VARRVAZZAR;
				}
				break;
			}
		}
		
		switch(m_nStageOpenList[dwHeroType][nStageID])
		{
		case LIS_DEFAULT:
			{	
				pIconList->AddItem("", i);
				pList->AddItem(_TEX_NOT_OPEN_DATA, m_nStageItemList[dwHeroType][nStageID]);
				break;
			}
		case LIS_NEW:
			{
				pIconList->AddItem("@(New_icon)", i);
				pList->AddItem(nTexID, m_nStageItemList[dwHeroType][nStageID]);
				bNewOpen = true;
				break;
			}
		case LIS_OPEN:
			{
				pIconList->AddItem("", i);
				pList->AddItem(nTexID, m_nStageItemList[dwHeroType][nStageID]);
				bNewOpen = true;
				break;
			}
		}

		if(dwStageType == nStageID){
			nCurSelPos = nCount;
		}
		nCount++;
	}

	pList->SetCurSel(nCurSelPos);
	pIconList->SetCurSel(nCurSelPos);

	BsUiWindow* pListUP = m_pStageLayer->GetWindow(IM_Msn_stage_listUp);
	pListUP->SetWindowAttr(XWATTR_SHOWWINDOW, true);
	if(pList->GetCurSel() == 0 ){
		pListUP->SetWindowAttr(XWATTR_SHOWWINDOW, false);
	}

	BsUiWindow* pListDOWN = m_pStageLayer->GetWindow(IM_Msn_stage_listdown);
	pListDOWN->SetWindowAttr(XWATTR_SHOWWINDOW, true);
	if(pList->GetCurSel() == pList->GetItemCount()-1){
		pListDOWN->SetWindowAttr(XWATTR_SHOWWINDOW, false);
	}


	BsUiWindow* pName = m_pStageLayer->GetWindow(BT_Msn_char);
	BsUiImageCtrl* pImg = (BsUiImageCtrl*)(m_pStageLayer->GetWindow(IM_Msn_char));

	char cImgName[256];
	if(bNewOpen == false)
	{
		pName->SetItemTextID(_TEX_NOT_OPEN_DATA);
		strcpy(cImgName, _CHAR_DATA_NONE);
	}
	else
	{
		pName->SetItemTextID(_TEX_Inphyy + dwHeroType);

		switch(dwHeroType)
		{
		case MISSION_TYPE_INPHYY:	strcpy(cImgName, "mn_chardata_in.dds"); break;
		case MISSION_TYPE_ASPHARR:	strcpy(cImgName, "mn_chardata_as.dds"); break;
		case MISSION_TYPE_MYIFEE:	strcpy(cImgName, "mn_chardata_my.dds"); break;
		case MISSION_TYPE_TYURRU:	strcpy(cImgName, "mn_chardata_ty.dds"); break;
		case MISSION_TYPE_KLARRANN:	strcpy(cImgName, "mn_chardata_kl.dds"); break;
		case MISSION_TYPE_DWINGVATT:strcpy(cImgName, "mn_chardata_dw.dds"); break;
		case MISSION_TYPE_VIGKVAGK:	strcpy(cImgName, "mn_chardata_vi.dds"); break;
		}
	}

	CBsKernel::GetInstance().chdir("Interface");
	int nTexId = g_BsKernel.LoadTexture(cImgName);
	BsAssert(nTexId != -1);
	CBsKernel::GetInstance().chdir("..");

	ImageCtrlInfo info;
	info.nTexId = nTexId;
	info.u2 = 518;
	info.v2 = 104;

	pImg->SetImageInfo(&info);
}


void CFcMenuMissionDB::ClickAItem(BsUiHANDLE hWnd)
{
	BsUiListBox* pList = (BsUiListBox*)(BsUi::BsUiGetWindow(hWnd));
	BsAssert(pList->GetClass() == LB_Msn_stage);
	DWORD dwItemID = pList->GetItemData(pList->GetCurSel());
	
	CFcMenuLibraryManager* pLibMgr = g_MenuHandle->GetLibManager();
	int nState = pLibMgr->GetState(dwItemID);
	switch(nState)
	{
	case LIS_DEFAULT:
		{
			g_pSoundManager->PlaySystemSound( SB_COMMON,"SYS_CANCEL");
			break;
		}
	case LIS_NEW:
	case LIS_OPEN:
		{
			SetStatus(_MS_CLOSING);

			LibraryTable* pItem = pLibMgr->GetItemData(dwItemID);
			if(nState == LIS_NEW)
			{
				LibraryItem* pLibItem = g_FCGameData.tempFixedSave.GetLibraryData(dwItemID);
				pLibItem->nState = LIS_OPEN;

				if(g_FC.GetLiveManager()->IsUserSignedIn() && g_FCGameData.m_DeviceID != XCONTENTDEVICE_ANY ){
					g_FCGameData.SaveFixedGameData();
				}
			}

			g_MenuHandle->PostMessage(fcMSG_LIB_MISSION_SUB_START, dwItemID);
			break;
		}
	default: BsAssert(0);
	}
}

//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------
DWORD CFcMenuMssnHeroLayer::ProcMessage(xwMessageToken* pMsgToken)
{
	BsAssert(pMsgToken);
	switch(pMsgToken->message)
	{
	case XWMSG_KEYDOWN:
		{
			if(pMsgToken->lParam == MENU_INPUT_B)
			{
				m_pMenu->SetStatus(_MS_CLOSING);
				g_MenuHandle->PostMessage(fcMSG_LIBRARY_START);
			}
			else
			{
				BsUiCLASS hClass = BsUi::BsUiGetClass(pMsgToken->hWnd);
				switch(hClass)
				{
				case LB_Msn:	OnKeyDownLbMissionDB(pMsgToken); break;
				}
			}
			break;
		}
	}

	return BsUiWindow::ProcMessage(pMsgToken);
}

void CFcMenuMssnHeroLayer::OnKeyDownLbMissionDB(xwMessageToken* pMsgToken)
{
	BsUiListBox* pListBox = ( BsUiListBox * )GetWindow( LB_Msn );
	BsAssert( pListBox );

	switch (pMsgToken->lParam)
	{
	case MENU_INPUT_A:
		{
			int nCurPos = pListBox->GetCurSel();
			BsUiListBox* pIconListBox = (BsUiListBox*)GetWindow( LB_Msn_icon );
			DWORD dwItemType = pIconListBox->GetItemData(nCurPos);
			if(dwItemType != LIS_DEFAULT)
			{
				g_pSoundManager->PlaySystemSound( SB_COMMON,"SYS_OK");
				g_BsUiSystem.SetFocusWindow(m_pMenu->m_pStageLayer);
			}
			else{
				g_pSoundManager->PlaySystemSound( SB_COMMON,"SYS_CANCEL");
			}
			break;
		}
	case MENU_INPUT_UP:
		{	
			int nCurPos = pListBox->GetCurSel();
			BsUiListBox* pIconListBox = (BsUiListBox*)GetWindow( LB_Msn_icon );
			pIconListBox->SetCurSel(nCurPos);

			DWORD dwHeroType = pListBox->GetItemData(nCurPos);
			m_pMenu->UpdateStageLayer(dwHeroType, STAGE_ID_NONE);

			break;
		}
	case MENU_INPUT_DOWN:
		{
			int nCurPos = pListBox->GetCurSel();
			BsUiListBox* pIconListBox = (BsUiListBox*)GetWindow( LB_Msn_icon );
			pIconListBox->SetCurSel(nCurPos);

			DWORD dwHeroType = pListBox->GetItemData(nCurPos);
			m_pMenu->UpdateStageLayer(dwHeroType, STAGE_ID_NONE);

			break;
		}
	}
}

//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------
DWORD CFcMenuMssnStageLayer::ProcMessage(xwMessageToken* pMsgToken)
{
	BsAssert(pMsgToken);
	switch(pMsgToken->message)
	{
	case XWMSG_KEYDOWN:
		{
			if(pMsgToken->lParam == MENU_INPUT_B)
			{
				g_pSoundManager->PlaySystemSound( SB_COMMON,"SYS_OK");
				g_BsUiSystem.SetFocusWindow(m_pMenu->m_pHeroLayer);
			}
			else
			{
				BsUiCLASS hClass = BsUi::BsUiGetClass(pMsgToken->hWnd);
				switch(hClass)
				{
				case LB_Msn_stage: OnKeyDownLbStageDB(pMsgToken); break;
				}
			}
			break;
		}
	}

	return BsUiWindow::ProcMessage(pMsgToken);
}

void CFcMenuMssnStageLayer::OnKeyDownLbStageDB(xwMessageToken* pMsgToken)
{
	BsUiListBox* pListBox = ( BsUiListBox * )GetWindow( LB_Msn_stage );
	BsAssert( pListBox );

	switch (pMsgToken->lParam)
	{
	case MENU_INPUT_A:
		{
			m_pMenu->ClickAItem(pMsgToken->hWnd);
			break;
		}
	case MENU_INPUT_UP:
		{	
			BsUiWindow* pListUP = GetWindow(IM_Msn_stage_listUp);
			pListUP->SetWindowAttr(XWATTR_SHOWWINDOW, true);
			if(pListBox->GetCurSel() == 0 ){
				pListUP->SetWindowAttr(XWATTR_SHOWWINDOW, false);
			}

			BsUiWindow* pListDOWN = GetWindow(IM_Msn_stage_listdown);
			pListDOWN->SetWindowAttr(XWATTR_SHOWWINDOW, true);
			if(pListBox->GetCurSel() == pListBox->GetItemCount()-1){
				pListDOWN->SetWindowAttr(XWATTR_SHOWWINDOW, false);
			}

			int nCurPos = pListBox->GetCurSel();
			BsUiListBox* pIconListBox = (BsUiListBox*)GetWindow( LB_Msn_stage_icon );
			pIconListBox->SetCurSel(nCurPos);
			break;
		}
	case MENU_INPUT_DOWN:
		{
			BsUiWindow* pListUP = GetWindow(IM_Msn_stage_listUp);
			pListUP->SetWindowAttr(XWATTR_SHOWWINDOW, true);
			if(pListBox->GetCurSel() == 0 ){
				pListUP->SetWindowAttr(XWATTR_SHOWWINDOW, false);
			}

			BsUiWindow* pListDOWN = GetWindow(IM_Msn_stage_listdown);
			pListDOWN->SetWindowAttr(XWATTR_SHOWWINDOW, true);
			if(pListBox->GetCurSel() == pListBox->GetItemCount()-1){
				pListDOWN->SetWindowAttr(XWATTR_SHOWWINDOW, false);
			}

			int nCurPos = pListBox->GetCurSel();
			BsUiListBox* pIconListBox = (BsUiListBox*)GetWindow( LB_Msn_stage_icon );
			pIconListBox->SetCurSel(nCurPos);
			break;
		}
	}
}

//--------------------------------------------------------------------
//--------------------------------------------------------------------
//--------------------------------------------------------------------
CFcMenuMissionDBSub::CFcMenuMissionDBSub(_FC_MENU_TYPE nType, DWORD dwItemID)
: CFcMenuForm(nType)
{
	m_dwItemID = dwItemID;

	m_pLayer = new CFcMenuMissionDBSubLayer(this);
	AddUiLayer(m_pLayer, SN_Msn_Sub, -1, -1);

	UpdateMissionDB_Sub(dwItemID);
}

void CFcMenuMissionDBSub::RenderProcess()
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
	case _MS_CLOSE:	break;
	}
}

void CFcMenuMissionDBSub::RenderMoveLayer(int nTick)
{
	//------------------------------------------------------
	if(nTick <= _TICK_FADE_IN_BASE)
	{
		float fAlpha = (float)nTick / _TICK_FADE_IN_BASE;
		m_pLayer->SetColor(D3DXCOLOR(1, 1, 1, fAlpha));
	}
}

void CFcMenuMissionDBSub::UpdateMissionDB_Sub(DWORD dwItemID)
{	
	CFcMenuLibraryManager* pLibMgr = g_MenuHandle->GetLibManager();
	LibraryTable* pItem = pLibMgr->GetItemData(dwItemID);

	//---------------------------------------------------------------
	CBsKernel::GetInstance().chdir("Interface");
	CBsKernel::GetInstance().chdir("WorldMap");
	int nTexId = g_BsKernel.LoadTexture(pItem->cImageFileName);
	BsAssert(nTexId != -1);
	CBsKernel::GetInstance().chdir("..");
	CBsKernel::GetInstance().chdir("..");

	ImageCtrlInfo BGinfo;
	BGinfo.nTexId = nTexId;
	BGinfo.u2 = 1024;
	BGinfo.v2 = 512;

	BsUiImageCtrl* pBGImg = (BsUiImageCtrl*)(m_pLayer->GetWindow(IM_MissionClear_BG));
	pBGImg->SetImageInfo(&BGinfo);
	
	//---------------------------------------------------------------
	char cFullname[256];
	sprintf(cFullname, "%s\\%s_%s.dds",
		g_LocalLanguage.GetLanguageDir(),
		pItem->cIconFileName,
		g_LocalLanguage.GetLanguageStr());
	
	CBsKernel::GetInstance().chdir("Interface");
	CBsKernel::GetInstance().chdir("Local");
	int nHeadTexId = g_BsKernel.LoadTexture(cFullname);
	BsAssert(nHeadTexId != -1);
	CBsKernel::GetInstance().chdir("..");
	CBsKernel::GetInstance().chdir("..");

	ImageCtrlInfo Headinfo;
	Headinfo.nTexId = nHeadTexId;
	Headinfo.u2 = 1024;
	Headinfo.v2 = 128;

	BsUiImageCtrl* pHeadImg = (BsUiImageCtrl*)(m_pLayer->GetWindow(IM_Msn_Sub_Headline));
	pHeadImg->SetImageInfo(&Headinfo);

	//---------------------------------------------------------------
	int nHeroID = pItem->nCharCondition;
	HeroRecordInfo* pHeroRecordInfo = g_FCGameData.tempFixedSave.GetHeroRecordInfo(nHeroID);

	int nStageID = pItem->nStageCondition;
	StageResultInfo* pStageResultInfo = pHeroRecordInfo->GetStageResultInfo(nStageID);

	UpdateClearTime(pStageResultInfo->nClearTime);
	UpdateText(TT_Msn_Sub_KillCount, pStageResultInfo->nKillCount);
	UpdateText(TT_Msn_Sub_NormalKill, pStageResultInfo->nNormalKillCount);
	UpdateText(TT_Msn_Sub_OrbSparkKill, pStageResultInfo->nOrbSparkKillCount);
	UpdateText(TT_Msn_Sub_GetOrb, pStageResultInfo->nGetOrbEXP);
	UpdateText(TT_Msn_Sub_MaxCombo, pStageResultInfo->nMaxCombo);
	UpdateText(TT_Msn_Sub_GuardianEx, pStageResultInfo->nGuardianLiveRate);
	UpdateText(TT_Msn_Sub_Clear, pStageResultInfo->nMissionClearCount);
	UpdateText(TT_Msn_Sub_Bonus, pStageResultInfo->nBonusPoint);

	
	char cGradeFile[256];
	switch(pStageResultInfo->nPlayGrade)
	{
	case 0:		strcpy(cGradeFile, "mn_result_S.dds"); break;
	case 1:		strcpy(cGradeFile, "mn_result_A.dds"); break;
	case 2:		strcpy(cGradeFile, "mn_result_B.dds"); break;
	case 3:		strcpy(cGradeFile, "mn_result_C.dds"); break;
	case 4:		strcpy(cGradeFile, "mn_result_D.dds"); break;
	default:	strcpy(cGradeFile, "mn_result_F.dds"); break;
	}

	CBsKernel::GetInstance().chdir("Interface");
	int nGradeTexId = g_BsKernel.LoadTexture(cGradeFile);
	BsAssert(nGradeTexId != -1);
	CBsKernel::GetInstance().chdir("..");
	
	ImageCtrlInfo Gradeinfo;
	Gradeinfo.nTexId = nGradeTexId;
	Gradeinfo.u2 = 150;
	Gradeinfo.v2 = 150;

	BsUiImageCtrl* pGradeImg = (BsUiImageCtrl*)(m_pLayer->GetWindow(IM_Msn_Sub_Grade));
	pGradeImg->SetImageInfo(&Gradeinfo);
}


void CFcMenuMissionDBSub::UpdateText(BsUiCLASS hClass, int nValue)
{	
	BsUiWindow* pWindow = m_pLayer->GetWindow(hClass);
	BsAssert(pWindow);

	char cValue[256];
	sprintf(cValue, "%d", nValue);
	pWindow->SetItemText(cValue);
}

#define _TIME_RATE			60
void CFcMenuMissionDBSub::UpdateClearTime(int nFullSec)
{	
	int nHour(0), nMin(0), nSec(0);
	nSec = nFullSec % _TIME_RATE;
	nMin = (nFullSec / _TIME_RATE) % _TIME_RATE;
	nHour = (nFullSec / _TIME_RATE) / _TIME_RATE;

	char cHour[64], cMin[64], cSec[64];
	if(nHour < 10)	sprintf(cHour, "0%d", nHour);
	else			sprintf(cHour, "%d", nHour);

	if(nMin < 10)	sprintf(cMin, "0%d", nMin);
	else			sprintf(cMin, "%d", nMin);

	if(nSec < 10)	sprintf(cSec, "0%d", nSec);
	else			sprintf(cSec, "%d", nSec);

	char szText[128];
	sprintf(szText, "%s : %s : %s", cHour, cMin, cSec);
	m_pLayer->GetWindow(TT_Msn_Sub_ClearTime)->SetItemText(szText);
}

//--------------------------------------------------------------------
DWORD CFcMenuMissionDBSubLayer::ProcMessage(xwMessageToken* pMsgToken)
{
	BsAssert(pMsgToken);
	switch(pMsgToken->message)
	{
	case XWMSG_KEYDOWN:
		{
			switch(pMsgToken->lParam)
			{
			case MENU_INPUT_B:
				{
					m_pMenu->SetStatus(_MS_CLOSING);
					g_MenuHandle->PostMessage(fcMSG_LIB_MISSION_START, m_pMenu->GetItemID());
					break;
				}
			case MENU_INPUT_A:
				{	
					break;
				}
			}
			break;
		}
	}

	return BsUiWindow::ProcMessage(pMsgToken);
}