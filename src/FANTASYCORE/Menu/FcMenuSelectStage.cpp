#include "stdafx.h"

#include "FcMenuSelectStage.h"
#include "BSuiSystem.h"

#include "BsUiClassIDManager.h"
#include "BsUiFontAttrManager.h"

#include "FcInterfaceManager.h"
#include "FcMessageDef.h"

#include "LocalLanguage.h"
#include "FcGlobal.h"
#include "TextTable.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG

CFcMenuSelectStage::CFcMenuSelectStage(_FC_MENU_TYPE nType)
: CFcMenuForm(nType)
{
	m_pBaseLayer = new BsUiLayer();
	AddUiLayer(m_pBaseLayer, SN_SelectStage0, -1, -1);

	m_pFrameLayer = new BsUiLayer();
	AddUiLayer(m_pFrameLayer, SN_SelectStage1, -1, -1);

	m_pInfoLayer = new CFcMenuSelStageInfoLayer(this);
	AddUiLayer(m_pInfoLayer, SN_SelectStage2, -1, -1);
	g_BsUiSystem.SetFocusWindow(m_pInfoLayer);

	UpdateInfoLayer(g_FCGameData.SelStageId);

	m_bCheckBriefing = false;
	m_nTargetID = -1;
}


void CFcMenuSelectStage::ProcessMsg(int nMsgAsk, BsUiHANDLE hWnd, int nMsgRsp)
{
	if(nMsgAsk == fcMT_MissionStart)
	{
		if(nMsgRsp == ID_YES)
		{
			SetStatus(_MS_CLOSING);
			g_MenuHandle->PostMessage(fcMSG_SELECT_STAGE_TO_MISSION_START);
		}
		else
		{
			g_BsUiSystem.SetFocusWindow(BsUi::BsUiGetWindow(hWnd));
			g_MenuHandle->GetMenuWorldMap()->SetEnableKey(true);
		}
	}
}


void CFcMenuSelectStage::Process()
{
	ProcessNextPage();
}

void CFcMenuSelectStage::ProcessNextPage()
{
	if(g_MenuHandle->GetMenuWorldMap()->IsExitToNext())
	{
		g_MenuHandle->GetMenuWorldMap()->SetExitToNext(false);
		g_MenuHandle->GetMenuWorldMap()->SetEnableKey(false);

		bool bEnableGuadianMenu = g_MenuHandle->GetMenuWorldMap()->IsEnableGuadianMenu();
		if(bEnableGuadianMenu)
		{
			SetStatus(_MS_CLOSING);
			g_MenuHandle->PostMessage(fcMSG_GUARDIAN_START);
		}
		else
		{
			BsUiHANDLE hWnd = m_pInfoLayer->GetHWnd();
			g_MenuHandle->PostMessage(fcMSG_MESSAGEBOX_START,
				fcMT_MissionStart,
				(DWORD)this,
				hWnd);
		}
	}
}

void CFcMenuSelectStage::RenderProcess()
{
	switch(GetStatus())
	{
	case _MS_OPEN:
		{
			m_pBaseLayer->SetColor(D3DXCOLOR(1, 1, 1, 0.f));
			m_pFrameLayer->SetColor(D3DXCOLOR(1, 1, 1, 0.f));
			m_pInfoLayer->SetColor(D3DXCOLOR(1, 1, 1, 0.f));

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
			RenderMoveLayer(_TICK_END_OPENING_CLOSING - nTick);

			if(nTick >= _TICK_END_OPENING_CLOSING){
				SetStatus(_MS_CLOSE);
			}
			break;
		}
	case _MS_CLOSE:	break;
	}
}


void CFcMenuSelectStage::RenderMoveLayer(int nTick)
{
	if(nTick <= _TICK_FADE_IN_BASE)
	{
		float fAlpha = (float)nTick / _TICK_FADE_IN_BASE;
		m_pBaseLayer->SetColor(D3DXCOLOR(1, 1, 1, fAlpha));
		m_pFrameLayer->SetColor(D3DXCOLOR(1, 1, 1, fAlpha));
		m_pInfoLayer->SetColor(D3DXCOLOR(1, 1, 1, fAlpha));
	}
}

void CFcMenuSelectStage::RenderNormal()
{	
	if(m_bShowOn == false){
		return;
	}

	FcMenuWorldMap* pWorldmap = g_MenuHandle->GetMenuWorldMap();
	bool bBriefing = pWorldmap->IsBriefing();
	if(bBriefing == false)
	{	
		int nTargetId = g_MenuHandle->GetMenuWorldMap()->GetTargetPointId();
		if(m_nTargetID != nTargetId){
			m_nTargetID = nTargetId;
		}

		if(m_nTargetID != -1)
		{
			WorldMapPointLIST* pPointList = g_MenuHandle->GetMenuWorldMapInfo()->GetPointList();
			UpdateInfoLayer((*pPointList)[nTargetId].nId);
			m_pInfoLayer->SetWindowAttr(XWATTR_SHOWWINDOW, true);
		}
		else{
			m_pInfoLayer->SetWindowAttr(XWATTR_SHOWWINDOW, false);
		}
	}
	else{
		m_pInfoLayer->SetWindowAttr(XWATTR_SHOWWINDOW, false);
	}

	if(m_bCheckBriefing != bBriefing)
	{	
		m_bCheckBriefing = bBriefing;

		UpdateHeadLineImage();
		UpdateHelpText();
	}
}

void CFcMenuSelectStage::UpdateHelpText()
{	
	BsUiText* pText = (BsUiText*)(m_pBaseLayer->GetWindow(TT_SelStageHelp));
	int nTexID = pText->GetItemTextID();

	if(m_bCheckBriefing == false)
	{
		if(nTexID != _TEX_HELP_SEL_STAGE){
			pText->SetItemTextID(_TEX_HELP_SEL_STAGE);
		}
	}
	else
	{
		FcMenuWorldMap* pWorldmap = g_MenuHandle->GetMenuWorldMap();
		bool bEnableGuadianMenu = pWorldmap->IsEnableGuadianMenu();
		if(bEnableGuadianMenu)
		{
			if(nTexID != _TEX_HELP_BRIEFING_0){
				pText->SetItemTextID(_TEX_HELP_BRIEFING_0);
			}
		}
		else
		{
			if(nTexID != _TEX_HELP_BRIEFING_1){
				pText->SetItemTextID(_TEX_HELP_BRIEFING_1);
			}
		}
	}
}


void CFcMenuSelectStage::UpdateHeadLineImage()
{	
	BsUiImageCtrl* pImgCtrl = (BsUiImageCtrl*)(m_pBaseLayer->GetWindow(IM_SelStage_Headline));
	BsAssert(pImgCtrl && "GetChildWindow(IM_SelStage_Headline) returned a NULL");
		
	int nAreaNameTexID = -1;
	if(m_bCheckBriefing == true)
	{	
		WorldMapPointLIST* pPointList = g_MenuHandle->GetMenuWorldMapInfo()->GetPointList();
		int nTargetId = g_MenuHandle->GetMenuWorldMap()->GetTargetPointId();
		BsAssert(nTargetId != -1);

		char cAreaName[256];
		sprintf(cAreaName, "interface\\Local\\%s\\%s_%s.dds",
			g_LocalLanguage.GetLanguageDir(),
			(*pPointList)[nTargetId].szImageNM,
			g_LocalLanguage.GetLanguageStr());

		nAreaNameTexID = pImgCtrl->LoadImage(cAreaName, true);
	}
	else
	{	
		char cAreaName[256];
		sprintf(cAreaName, "interface\\Local\\%s\\HeadLine\\mn_headline_stageselect_%s.dds",
			g_LocalLanguage.GetLanguageDir(),
			g_LocalLanguage.GetLanguageStr());

		nAreaNameTexID = pImgCtrl->LoadImage(cAreaName, true);
	}

	BsAssert(nAreaNameTexID != -1);
	SIZE size = g_BsKernel.GetTextureSize(nAreaNameTexID);
	pImgCtrl->SetWindowSize(size.cx, size.cy);
}


void CFcMenuSelectStage::UpdateInfoLayer(int nStageID)
{
	if(nStageID == -1)
	{
		m_pInfoLayer->SetWindowAttr(XWATTR_SHOWWINDOW, false);
		return;
	}
	else{
		m_pInfoLayer->SetWindowAttr(XWATTR_SHOWWINDOW, true);
	}

	//name
	char cHeroName[256];
	int nTextID = _TEX_Inphyy + g_SoxToMissionID[g_FCGameData.tempUserSave.nHeroID];
	g_TextTable->GetText(nTextID, cHeroName, _countof(cHeroName));

	char cLevel[256];
	g_TextTable->GetText(_TEX_SUM_LEVEL, cLevel, _countof(cLevel));

	char cName[256];
	int nLevel = g_FCGameData.tempUserSave.nLevel + 1;
	sprintf(cName, "%s %s %d", cHeroName, cLevel, nLevel);
	
	BsUiWindow* pName = m_pInfoLayer->GetWindow(TT_SelStage_CharName);
	pName->SetItemText(cName);


	BsUiImageCtrl* pImgCtrl = (BsUiImageCtrl*)(m_pInfoLayer->GetWindow(IM_SelStage_Rank));

	//rank
	int nSoxID = g_MissionToSoxID[g_FCGameData.nPlayerType];
	HeroRecordInfo* pHeroInfo = g_FCGameData.tempFixedSave.GetHeroRecordInfo(nSoxID);	
	StageResultInfo* pStageInfo = NULL;
	if(pHeroInfo) {
		pStageInfo = pHeroInfo->GetStageResultInfo(nStageID);
	}
	
	if(pStageInfo == NULL || pStageInfo->nMissionClearCount == 0)
	{
		pImgCtrl->SetWindowAttr(XWATTR_SHOWWINDOW, false);
		return;
	}
	else{
		pImgCtrl->SetWindowAttr(XWATTR_SHOWWINDOW, true);
	}
	
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

	
	ImageCtrlInfo info;
	info.nTexId = nTexId;
	info.u1 = 0;	info.v1 = 0;
	info.u2 = 150;	info.v2 = 150;
	pImgCtrl->SetImageInfo(&info);
}



//-----------------------------------------------------------------------------------------------------
DWORD CFcMenuSelStageInfoLayer::ProcMessage(xwMessageToken* pMsgToken)
{
	BsAssert(pMsgToken);

	BsUiCLASS hClass = BsUi::BsUiGetClass(pMsgToken->hWnd);

	switch(pMsgToken->message)
	{
	case XWMSG_KEYDOWN:
		{
			switch (pMsgToken->lParam)
			{
			case MENU_INPUT_B: break;
			case MENU_INPUT_START:
				{
					if(g_bIsStartTitleMenu)
					{
						bool bBriefing = g_MenuHandle->GetMenuWorldMap()->IsBriefing();
						if(bBriefing == false){
							g_MenuHandle->PostMessage(fcMSG_USUAL_START);
						}
					}
					break;
				}
			case MENU_INPUT_Y:
				{
					if(g_bIsStartTitleMenu == false)
					{
						bool bBriefing = g_MenuHandle->GetMenuWorldMap()->IsBriefing();
						if(bBriefing == false){
							g_MenuHandle->PostMessage(fcMSG_USUAL_START);
						}
					}
					break;
				}
			case MENU_INPUT_A: break;
			}
		}
	case XWMSG_BN_CLICKED:
		{	
			break;
		}
	}

	return BsUiWindow::ProcMessage(pMsgToken);
}