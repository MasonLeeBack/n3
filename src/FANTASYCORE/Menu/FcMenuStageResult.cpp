#include "stdafx.h"

#include "FcMenuStageResult.h"
#include "BSuiSystem.h"

#include "BsUiClassIDManager.h"
#include "BsUiFontAttrManager.h"

#include "FcInterfaceManager.h"
#include "FcHeroObject.h"
#include "FantasyCore.h"
#include "FcSoundManager.h"
#include "FcAchievement.h"
#include "FcLiveManager.h"
#include "FcItem.h"

#include "FcWorld.h"
#include "FcMessageDef.h"
#include "TextTable.h"

extern int g_nRequiredExp[9];

#ifdef _XBOX
#include "AtgSignIn.h"
#endif // _XBOX

#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG

#define _PROCESS_FRAME		40
#define _TIME_RATE			60
#define _WAIT_LIMIT_TIME	25 * 04

extern CItemDescTable	g_FcItemDescTable;

CFcMenuStageResult::CFcMenuStageResult(_FC_MENU_TYPE nType)
: CFcMenuForm(nType)
{
	m_pLayer = new CFcMenuStageResultLayer(this);
	AddUiLayer(m_pLayer, SN_StageResult, -1, -1);
	m_pLayer->Initialize();

	{
		bool bFirstStageClear = true;
		std::vector<int>* pClearStageList = &(g_FCGameData.tempUserSave.ClearStageList);
		DWORD dwCount = pClearStageList->size();
		for(DWORD i=0; i<dwCount; i++)
		{
			if((*pClearStageList)[i] == g_FCGameData.SelStageId)
			{
				bFirstStageClear = false;
				break;
			}
		}

		if(bFirstStageClear){
			g_FCGameData.tempUserSave.ClearStageList.push_back(g_FCGameData.SelStageId);
		}

		if(g_FC.GetLiveManager()->IsUserSignedIn() && g_FCGameData.m_DeviceID != XCONTENTDEVICE_ANY )
			g_FCGameData.SaveFixedGameData();
	}

	m_ntime = g_BsUiSystem.GetMenuTick();

	m_dwClearMission = (DWORD)CheckClearChar();
	CheckHighestLevelAllClearAchievement();
}

void CFcMenuStageResult::ProcessMsg(int nMsgAsk, BsUiHANDLE hWnd, int nMsgRsp)
{
	if(nMsgAsk == fcMT_AskClearStageSave)
	{
		if(nMsgRsp == ID_YES){
			g_MenuHandle->PostMessage(fcMSG_SAVEDATA_START);
		}
		else{
			g_MenuHandle->PostMessage(fcMSG_ENDING_CHAR_START);
		}
	}
}


void CFcMenuStageResult::RenderProcess()
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
	case _MS_NORMAL:	RenderNormal(); break;
	case _MS_CLOSING:
		{
			int nTick = g_BsUiSystem.GetMenuTick() - m_nFormTick;

			if(nTick >= _TICK_END_OPENING_CLOSING){
				SetStatus(_MS_CLOSE);
			}
			break;
		}
	case _MS_CLOSE:	break;
	}
}


void CFcMenuStageResult::RenderNormal()
{
	;
}

void CFcMenuStageResult::RenderMoveLayer(int nTick)
{
	if(nTick <= _TICK_FADE_IN_BASE)
	{
		float fAlpha = (float)nTick / _TICK_FADE_IN_BASE;
		m_pLayer->SetColor(D3DXCOLOR(1, 1, 1, fAlpha));
	}
}

void CFcMenuStageResult::GoToTitleorWorldMap()
{
	//현재의 mission이 마지막인지 check한다.
	BsAssert(g_FCGameData.SelStageId != -1);
	g_MenuHandle->PostMessage(fcMSG_CLICK_FX_EFFECT, NULL);

#ifdef _XBOX
	if(ATG::SignIn::IsUserSignedIn(g_FCGameData.nEnablePadID)==FALSE)
#else
	if(true)
#endif
	{
		if(m_dwClearMission != MAX_MISSION_TYPE)
		{
			g_MenuHandle->PostMessage(fcMSG_ENDING_CHAR_START);
		}else
		{
			g_MenuHandle->PostMessage(fcMSG_GO_TO_SEL_STAGE);
		}
	}
	else 
		g_MenuHandle->PostMessage(fcMSG_SAVEDATA_START, (DWORD)m_nType, m_dwClearMission );

	g_FCGameData.stageInfo.Clear();
}


int CFcMenuStageResult::CheckClearChar()
{	
	WorldMapPassList* pPathList = g_MenuHandle->GetMenuWorldMapInfo()->GetPathList();
	DWORD dwCount = pPathList->size();
	for(DWORD i=0; i<dwCount; i++)
	{
		if((*pPathList)[i].nStartId == g_FCGameData.SelStageId){
			return MAX_MISSION_TYPE;
		}
	}
	
	BsAssert(g_FCGameData.nPlayerType < MAX_MISSION_TYPE);
	int nHeroId = g_MissionToSoxID[g_FCGameData.nPlayerType];
	HeroRecordInfo* pRecordInfo = g_FCGameData.tempFixedSave.GetHeroRecordInfo(nHeroId);
	BsAssert(pRecordInfo);
	pRecordInfo->bAllStageClear = true;

	//마계 열릴 조건 중 하나
	//1. g_FCGameData.nTrgGlobalVar[0] 값은 아바리아에서 최초 setting되고,
	//2. 0이면 bad path, 1이면 good path이고
	//3. 야페왈에서 good path ending이면 2를 setting한다.
	if(nHeroId == CHAR_ID_ASPHARR && g_FCGameData.nTrgGlobalVar[0] == 2){
		g_FCGameData.tempFixedSave.bGoodEndingforAspharr = true;
	}

	//마계가 열릴지 check;
	//1. 모든 미션을 clear해야 한다.
	//2. 특히 아스파는 good ending으로 clear해야 한다.(bed면 인피가 죽는다)
	//3. 모든 캐릭터 clear하면
	//4. g_FCGameData.tempFixedSave.bOpenBonusStage = true로 setting한다.
	if(g_FCGameData.tempFixedSave.bOpenBonusStage == false)
	{
		g_FCGameData.tempFixedSave.bOpenBonusStage = true;
		for(int i=0; i<CHAR_ID_MAX; i++)
		{
			pRecordInfo = g_FCGameData.tempFixedSave.GetHeroRecordInfo(i);
			if(pRecordInfo == NULL || pRecordInfo->bAllStageClear == false)
			{
				g_FCGameData.tempFixedSave.bOpenBonusStage = false;
				break;
			}
		}
	}
#ifdef _XBOX
	if(ATG::SignIn::IsUserSignedIn(g_FCGameData.nEnablePadID)==TRUE)
		CheckAchievement();
#endif // _XBOX
	return g_FCGameData.nPlayerType;
}


void CFcMenuStageResult::CheckAchievement()
{
	if(g_FCGameData.bLockAchievement)
		return;
#ifdef _XBOX
	CFcAchievement* pAchieve = g_FC.GetLiveManager()->GetAchievement();
	pAchieve->SetUserIndex(g_FCGameData.nEnablePadID);
	switch( g_MissionToSoxID[g_FCGameData.nPlayerType] )
	{
	case CHAR_ID_INPHYY:
		pAchieve->SetInphyyClear();
		break;
	case CHAR_ID_ASPHARR:
		pAchieve->SetAspharrClear();
		break;
	case CHAR_ID_MYIFEE:
		pAchieve->SetMyifeeClear();
		break;
	case CHAR_ID_TYURRU:
		pAchieve->SetTyurruClear();
		break;
	case CHAR_ID_KLARRANN:
		pAchieve->SetKlarrannClear();
		break;
	case CHAR_ID_DWINGVATT:
		pAchieve->SetDwingvattClear();
		break;
	case CHAR_ID_VIGKVAGK:
		pAchieve->SetVigkvagkClear();
		break;
	}

	// 마계클리어 여부
	if(g_FCGameData.tempFixedSave.bOpenBonusStage && g_FCGameData.stageInfo.nStageID == STAGE_ID_ANOTHER_WORLD)
		pAchieve->SetHellClear();

	

	

#endif //_XBOX
}

void	CFcMenuStageResult::CheckHighestLevelAllClearAchievement()
{
	if(g_FCGameData.bLockAchievement)
		return;
#ifdef _XBOX
	CFcAchievement* pAchieve = g_FC.GetLiveManager()->GetAchievement();
	pAchieve->SetUserIndex(g_FCGameData.nEnablePadID);

	// 올 A 클리어 여부 & 히어로 레벨 9로 모두 클리어 여부	
	bool bAllAClear=true, bAll9Clear=true;

	for(int i=0; i<CHAR_ID_MAX; i++)
	{	
		HeroRecordInfo* pRecordInfo = g_FCGameData.tempFixedSave.GetHeroRecordInfo(i);
		if(pRecordInfo == NULL || pRecordInfo->bAllStageClear == false)
		{
			bAllAClear = false;
			bAll9Clear = false;
			break;
		}
		int nStages = pRecordInfo->StageResultList.size();
		for(int j=0; j<nStages; ++j)
		{
			if(pRecordInfo->StageResultList[j].nStageID == STAGE_ID_ANOTHER_WORLD)
				continue;
			if(pRecordInfo->StageResultList[j].nPlayGrade>1)
				bAllAClear = false;
			if(pRecordInfo->StageResultList[j].nHeroLevel<8)
				bAll9Clear = false;				
		}
	}

	if(bAllAClear)
		pAchieve->SetAllARankClear();
	if(bAll9Clear)
		pAchieve->SetAllHeroOfLevel9Clear();

#endif // _XBOX

}
//-----------------------------------------------------------------------------------------------------
CFcMenuStageResultLayer::CFcMenuStageResultLayer(CFcMenuForm* pMenu)
{
	m_pMenu = pMenu;
	m_bLevelUp = false;
	m_bSpecialItem = false;
	m_nItemCount = 0;
	
	memset(m_cFullItem, 0, sizeof(char)*1024);
}


void CFcMenuStageResultLayer::Initialize()
{
	StageResultInfo* pStageInfo = &(g_FCGameData.stageInfo);
    	
	UpdateStageResultInfo();

	//-----------------------------------------------------------------------
	UpdateClearTime(pStageInfo);	

	UpdateTextCtrl(TT_Result_KillCount, pStageInfo->nKillCount);
	UpdateTextCtrl(TT_Result_NormalKill, pStageInfo->nNormalKillCount);
	UpdateTextCtrl(TT_Result_OrbSparkKill, pStageInfo->nOrbSparkKillCount);
	UpdateTextCtrl(TT_Result_GetOrb, pStageInfo->nGetOrbEXP);
	UpdateTextCtrl(TT_Result_MaxCombo, pStageInfo->nMaxCombo);
	UpdateTextCtrl(TT_Result_GuardianEx, pStageInfo->nGuardianLiveRate);
	UpdateTextCtrl(TT_Result_Clear, pStageInfo->nMissionClearCount);
	UpdateTextCtrl(TT_Result_Bonus, pStageInfo->nBonusPoint);
	UpdatePlayGrade(pStageInfo);

	//-----------------------------------------------------------------------
	// total point
	char cTempText[256];
	//14 : DefaultTextTable - Points
	g_TextTable->GetText(14, cTempText, _countof(cTempText));

	char szText[256];
	sprintf(szText, "%s : %d", cTempText, g_FCGameData.tempFixedSave.nTotalPoint);
	GetWindow(TT_Result_GetPoint_Txt)->SetItemText(szText);
	
	//-----------------------------------------------------------------------
	// add new item

	UpdateSpecialLevelUp();
	UpdateSpecialItem();

	if(m_bLevelUp)
	{
		UpdateSpecialMessage(true, 0);
		GetWindow(TT_Result_GetItem_Text)->SetWindowAttr(XWATTR_SHOWWINDOW, false);
		GetWindow(IM_Result_LvUp)->SetWindowAttr(XWATTR_SHOWWINDOW, true);
	}
	else if(m_bSpecialItem)
	{
		UpdateSpecialMessage(true, m_nItemCount);
		GetWindow(IM_Result_LvUp)->SetWindowAttr(XWATTR_SHOWWINDOW, false);
	}

	UpdateBackground();
}


DWORD CFcMenuStageResultLayer::ProcMessage(xwMessageToken* pMsgToken)
{
	BsAssert(pMsgToken);

	BsUiCLASS hClass = BsUi::BsUiGetClass(pMsgToken->hWnd);

	switch(pMsgToken->message)
	{
	case XWMSG_KEYDOWN:
		{
			if(pMsgToken->lParam == MENU_INPUT_A)
			{	
				if(m_bLevelUp == true)
				{
					g_pSoundManager->PlaySystemSound( SB_COMMON,"SYS_OK");
					
					UpdateSpecialMessage(m_bSpecialItem, m_nItemCount);
					m_bLevelUp = false;
					GetWindow(IM_Result_LvUp)->SetWindowAttr(XWATTR_SHOWWINDOW, false);
				}
				else if(m_bSpecialItem == true)
				{
					g_pSoundManager->PlaySystemSound( SB_COMMON,"SYS_OK");
					UpdateSpecialMessage(false);
					m_bSpecialItem = false;
				}
				else
				{
					((CFcMenuStageResult*)m_pMenu)->GoToTitleorWorldMap();
				}
			}
			else if(pMsgToken->lParam == MENU_INPUT_B)
			{
				;
			}

			//wait time을 갱신한다.
			((CFcMenuStageResult*)m_pMenu)->SetTime(g_BsUiSystem.GetMenuTick());
			break;
		}
	}

	return BsUiWindow::ProcMessage(pMsgToken);
}


void CFcMenuStageResultLayer::UpdateStageResultInfo()
{	
	StageResultInfo* pStageInfo = &(g_FCGameData.stageInfo);

	//add total point
	int nAddPoint = 0;
	char cGrade[256];
	switch(pStageInfo->nPlayGrade)
	{
	case 0:		nAddPoint = _TOTAL_POINT_ADD_RANK_S; strcpy(cGrade, "S : 5"); break;
	case 1:		nAddPoint = _TOTAL_POINT_ADD_RANK_A; strcpy(cGrade, "A : 4"); break;
	case 2:		nAddPoint = _TOTAL_POINT_ADD_RANK_B; strcpy(cGrade, "B : 3"); break;
	case 3:		nAddPoint = _TOTAL_POINT_ADD_RANK_C; strcpy(cGrade, "C : 2"); break;
	case 4:		nAddPoint = _TOTAL_POINT_ADD_RANK_D; strcpy(cGrade, "D : 1"); break;
	default:	nAddPoint = _TOTAL_POINT_ADD_RANK_F; strcpy(cGrade, "F : 0"); break;
	}
	
	int nTempPoint = g_FCGameData.tempFixedSave.nTotalPoint;
	g_FCGameData.tempFixedSave.nTotalPoint += nAddPoint;

	DebugString( "//----------------------------\nMission success\n");
	DebugString( "pre total point %d / Add point %d / Rank %s\n", nTempPoint, nAddPoint, cGrade );
	//-----------------------------------------

	int nHeroSoxId = g_MissionToSoxID[g_FCGameData.nPlayerType];
	HeroRecordInfo* pHeroRecordInfo = g_FCGameData.tempFixedSave.GetHeroRecordInfo(nHeroSoxId);
	if(pHeroRecordInfo == NULL)
	{
		HeroRecordInfo HeroInfo;
		HeroInfo.nHeroID = nHeroSoxId;
		g_FCGameData.tempFixedSave.HeroRecordList.push_back(HeroInfo);
		pHeroRecordInfo = g_FCGameData.tempFixedSave.GetHeroRecordInfo(nHeroSoxId);
	}

	StageResultInfo* pRecordStageInfo = pHeroRecordInfo->GetStageResultInfo(g_FCGameData.SelStageId);
	if(pRecordStageInfo == NULL)
	{
		g_FCGameData.stageInfo.nMissionClearCount++;

		//A rank이상 나오면 nEnemyLevelUp이 올라간다.
		if(g_FCGameData.stageInfo.nPlayGrade <= 1 ){
			if( g_FCGameData.stageInfo.nEnemyLevelUp < 2 )
				g_FCGameData.stageInfo.nEnemyLevelUp++;
		}

		pHeroRecordInfo->StageResultList.push_back(g_FCGameData.stageInfo);
		return;
	}

	
	BsAssert(pStageInfo->nStageID == pRecordStageInfo->nStageID);
	if(pStageInfo->nClearTime < pRecordStageInfo->nClearTime)
	{
		pRecordStageInfo->nClearTime = pStageInfo->nClearTime;
		UpdateNewIcon(TT_Result_ClearTime_Txt);
	}

	if(pStageInfo->nKillCount > pRecordStageInfo->nKillCount)
	{
		pRecordStageInfo->nKillCount = pStageInfo->nKillCount;
		UpdateNewIcon(TT_Result_KillCount_Txt);
	}

	if(pStageInfo->nNormalKillCount > pRecordStageInfo->nNormalKillCount)
	{
		pRecordStageInfo->nNormalKillCount = pStageInfo->nNormalKillCount;
		UpdateNewIcon(TT_Result_NormalKill_Txt);
	}

	if(pStageInfo->nOrbSparkKillCount > pRecordStageInfo->nOrbSparkKillCount)
	{
		pRecordStageInfo->nOrbSparkKillCount = pStageInfo->nOrbSparkKillCount;
		UpdateNewIcon(TT_Result_OrbSparkKill_Txt);
	}

	if(pStageInfo->nGetOrbEXP > pRecordStageInfo->nGetOrbEXP)
	{
		pRecordStageInfo->nGetOrbEXP = pStageInfo->nGetOrbEXP;
		UpdateNewIcon(TT_Result_GetOrb_Txt);
	}

	if(pStageInfo->nMaxCombo > pRecordStageInfo->nMaxCombo)
	{
		pRecordStageInfo->nMaxCombo = pStageInfo->nMaxCombo;
		UpdateNewIcon(TT_Result_MaxCombo_Txt);
	}
	
	if(pStageInfo->nGuardianLiveRate > pRecordStageInfo->nGuardianLiveRate)
	{
		pRecordStageInfo->nGuardianLiveRate = pStageInfo->nGuardianLiveRate;
		UpdateNewIcon(TT_Result_GuardianEx_Txt);
	}

	if(pStageInfo->nPlayGrade < pRecordStageInfo->nPlayGrade)
	{
		pRecordStageInfo->nPlayGrade = pStageInfo->nPlayGrade;
		UpdateNewIcon(TT_Result_Bonus_rank);
	}

	if(pStageInfo->nHeroLevel > pRecordStageInfo->nHeroLevel)
	{
		pRecordStageInfo->nHeroLevel = pStageInfo->nHeroLevel;		
	}

	pRecordStageInfo->nMissionClearCount++;
	
	//A rank이상 나오면 nEnemyLevelUp이 올라간다.
	if(pStageInfo->nPlayGrade <= 1 ){
		if( pRecordStageInfo->nEnemyLevelUp < 2 )
			pRecordStageInfo->nEnemyLevelUp++;
	}

	pStageInfo->nMissionClearCount = pRecordStageInfo->nMissionClearCount;
	pStageInfo->nMissionFaultCount = pRecordStageInfo->nMissionFaultCount;
}

void CFcMenuStageResultLayer::UpdateClearTime(StageResultInfo* pStageInfo)
{
	int nFullSec = pStageInfo->nClearTime;
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
	GetWindow(TT_Result_ClearTime)->SetItemText(szText);
}


void CFcMenuStageResultLayer::UpdatePlayGrade(StageResultInfo* pStageInfo)
{	
	char cImageFile[256];
	switch(pStageInfo->nPlayGrade)
	{
	case 0:		strcpy(cImageFile, "mn_result_S.dds"); break;
	case 1:		strcpy(cImageFile, "mn_result_A.dds"); break;
	case 2:		strcpy(cImageFile, "mn_result_B.dds"); break;
	case 3:		strcpy(cImageFile, "mn_result_C.dds"); break;
	case 4:		strcpy(cImageFile, "mn_result_D.dds"); break;
	default:	strcpy(cImageFile, "mn_result_F.dds"); break;
	}

	CBsKernel::GetInstance().chdir("Interface");
	int nTexId = g_BsKernel.LoadTexture(cImageFile);
	BsAssert(nTexId != -1);
	CBsKernel::GetInstance().chdir("..");

	BsUiImageCtrl* pImg = (BsUiImageCtrl*)GetWindow(IM_Result_Grade);
	ImageCtrlInfo info;
	info.nTexId = nTexId;
	info.u1 = 0;	info.v1 = 0;
	info.u2 = 150;	info.v2 = 150;
	pImg->SetImageInfo(&info);
}


void CFcMenuStageResultLayer::UpdateBackground()
{
	char cImageFile[256];
	switch(g_FCGameData.nPlayerType)
	{
	case MISSION_TYPE_INPHYY:		strcpy(cImageFile, "mn_resultBG_in.dds"); break;
	case MISSION_TYPE_ASPHARR:		strcpy(cImageFile, "mn_resultBG_as.dds"); break;
	case MISSION_TYPE_MYIFEE:		strcpy(cImageFile, "mn_resultBG_my.dds"); break;
	case MISSION_TYPE_TYURRU:		strcpy(cImageFile, "mn_resultBG_ty.dds"); break;
	case MISSION_TYPE_KLARRANN:		strcpy(cImageFile, "mn_resultBG_kl.dds"); break;
	case MISSION_TYPE_DWINGVATT:	strcpy(cImageFile, "mn_resultBG_dw.dds"); break;
	case MISSION_TYPE_VIGKVAGK:		strcpy(cImageFile, "mn_resultBG_vi.dds"); break;
	default:
		{
			strcpy(cImageFile, "mn_resultBG_in.dds"); break;
			BsAssert(0);
		}
	}

	CBsKernel::GetInstance().chdir("Interface");
	int nTexId = g_BsKernel.LoadTexture(cImageFile);
	BsAssert(nTexId != -1);
	CBsKernel::GetInstance().chdir("..");

	BsUiImageCtrl* pImg = (BsUiImageCtrl*)GetWindow(IM_StageResult_BG);
	ImageCtrlInfo info;
	info.nTexId = nTexId;
	info.u1 = 0;	info.v1 = 0;
	info.u2 = 1024;	info.v2 = 512;
	pImg->SetImageInfo(&info);
}

void CFcMenuStageResultLayer::UpdateTextCtrl(BsUiCLASS hClassID, int nValue)
{
	char szText[256];
	sprintf(szText, "%d", nValue);
	GetWindow(hClassID)->SetItemText(szText);
}

void CFcMenuStageResultLayer::UpdateNewIcon(BsUiCLASS hClassID)
{
	char szText[256];
	sprintf(szText, "@(New_icon)");
	GetWindow(hClassID)->SetItemText(szText);
}


void CFcMenuStageResultLayer::UpdateSpecialLevelUp()
{	
	StageResultInfo* pStageInfo = &(g_FCGameData.stageInfo);
	g_FCGameData.tempUserSave.nExp += pStageInfo->nGuardianOrbs;

	int nLevel = g_FCGameData.tempUserSave.nLevel;
	int nRequiredExp = g_nRequiredExp[nLevel];
	if(g_FCGameData.tempUserSave.nExp >= nRequiredExp)
	{	
		if(nLevel < _LIMIT_LEVEL - 1)
		{
			m_bLevelUp = true;
			g_FCGameData.tempUserSave.nLevel++;	
		}
	}

	if( g_FCGameData.tempUserSave.nExp > _LIMIT_EXP ){
		g_FCGameData.tempUserSave.nExp = _LIMIT_EXP;
	}
	
	if(g_FCGameData.tempUserSave.nLevel >= _LIMIT_LEVEL)
	{
		BsAssert(0);
		g_FCGameData.tempUserSave.nLevel = _LIMIT_LEVEL - 1;
	}
}


void CFcMenuStageResultLayer::UpdateSpecialItem()
{
	StageResultInfo* pStageInfo = &(g_FCGameData.stageInfo);

	char cGetItem[256];
	g_TextTable->GetText(_TEX_GET_ITEM, cGetItem, _countof(cGetItem));

	for(int i=0; i<PRIZE_ITEM_MAX; i++)
	{
		int nItemID = pStageInfo->nSpecialItem[i];
		if(nItemID == -1){
			continue;
		}

		HeroEquip Item;
		Item.bAccoutered = false;
		Item.bNew = true;
		Item.nItemSoxID = nItemID;
		g_FcItemManager.AddItem(Item);

		char ctempFullItem[256];
		sprintf(ctempFullItem, cGetItem, g_FcItemDescTable.GetItemName(nItemID));
		strcat(ctempFullItem, "\n");
		strcat(m_cFullItem, ctempFullItem);
		m_nItemCount++;
	}

	if(m_nItemCount > 0)
	{	
		m_bSpecialItem = true;
		GetWindow(TT_Result_GetItem_Text)->SetItemText(m_cFullItem);
	}
}

void CFcMenuStageResultLayer::UpdateSpecialMessage(bool bOn, int nLineCount)
{
	int AddValue = nLineCount * 10;
	
	BsUiWindow* pImgUp = GetWindow(IM_Result_GetItem_bg_up);
	BsUiWindow* pImgDown = GetWindow(IM_Result_GetItem_bg_down);

	BsUiWindow* pImgCenter = GetWindow(IM_Result_GetItem_bg_Center);
	BsUiWindow* pText = GetWindow(TT_Result_GetItem_Text);
	
	pImgUp->SetWindowAttr(XWATTR_SHOWWINDOW, bOn);
	pImgDown->SetWindowAttr(XWATTR_SHOWWINDOW, bOn);

	pImgCenter->SetWindowAttr(XWATTR_SHOWWINDOW, bOn);
	pText->SetWindowAttr(XWATTR_SHOWWINDOW, bOn);

	int nX(0), nY(0), nW(0), nH(0);
	pImgUp->GetWindowPos(nX, nY);
	pImgUp->SetWindowPos(nX, nY - AddValue);

	pImgDown->GetWindowPos(nX, nY);
	pImgDown->SetWindowPos(nX, nY + AddValue);
	
	pImgCenter->GetWindowPos(nX, nY);
	pImgCenter->SetWindowPos(nX, nY - AddValue);
	pImgCenter->GetWindowSize(nW, nH);
	pImgCenter->SetWindowSize(nW, nH + AddValue * 2);

	pText->GetWindowPos(nX, nY);
	pText->SetWindowPos(nX, nY - AddValue);
	pText->GetWindowSize(nW, nH);
	pText->SetWindowSize(nW, nH + AddValue);
}