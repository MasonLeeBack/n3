#include "stdafx.h"

#include "FcMenuGuardian.h"
#include "BSuiSystem.h"
#include "FcGlobal.h"

#include "BsUiClassIDManager.h"
#include "BsUiFontAttrManager.h"

#include "FcMessageDef.h"

#include "FcInterfaceManager.h"
#include "FcTroopObject.h"

#include "FcSoundManager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG

#define _SEL_GRD_TICK				3
#define _SEL_GRD_INCREASE_GAP		0
//Center pos
const D3DXVECTOR2 g_vImagePos[3] = 
{
	D3DXVECTOR2(192, 338),
	D3DXVECTOR2(320, 338),
	D3DXVECTOR2(448, 338),
};


int GetGuardianImgTexID(int nMissionType, int nType, bool bSel)
{
	char cFileName[256];
	switch(nType)
	{
	case GT_ONE_HAND:
		{
			if(nMissionType == MISSION_TYPE_MYIFEE){
				strcpy(cFileName, "mn_gurdian_MOH");
			}else{
				strcpy(cFileName, "mn_gurdian_KOH");
			}
			break;
		}	
	case GT_TWO_HAND:
		{
			if(nMissionType == MISSION_TYPE_MYIFEE){
				strcpy(cFileName, "mn_gurdian_MTH");
			}else{
				strcpy(cFileName, "mn_gurdian_KTH");
			}
			break;
		}
	case GT_SPEAR:	  strcpy(cFileName, "mn_gurdian_S"); break;
	case GT_RANGE:	  strcpy(cFileName, "mn_gurdian_A"); break;
	}
	
	if(bSel == true){
		strcat(cFileName, "_selct.dds");
	}else{
		strcat(cFileName, ".dds");
	}

	CBsKernel::GetInstance().chdir("Interface");
	int nTexID = g_BsKernel.LoadTexture(cFileName);
	CBsKernel::GetInstance().chdir("..");
	BsAssert(nTexID != -1);

	return nTexID;
}

//-------------------------------------------------------
CFcMenuGuardian::CFcMenuGuardian(_FC_MENU_TYPE nType)
: CFcMenuForm(nType)
{
	Initialize();
		
	m_pGrd0Layer = new CFcMenuGrd0Layer();
	AddUiLayer(m_pGrd0Layer, SN_Guardian, -1, -1);	
	m_pGrd0Layer->SetColor(D3DXCOLOR(1, 1, 1, 0.f));

	m_pGrd2Layer = new CFcMenuGrd2Layer(this);
	AddUiLayer(m_pGrd2Layer, SN_Guardian2, -1, -1);
	m_pGrd2Layer->Initialize();

	m_pGrd1Layer = new CFcMenuGrd1Layer(this);
	AddUiLayer(m_pGrd1Layer, SN_Guardian1, -1, -1);
	m_pGrd1Layer->Initialize();

	m_nTick = g_BsUiSystem.GetMenuTick();
}

CFcMenuGuardian::~CFcMenuGuardian()
{
	/*
	SAFE_DELETE(m_pGrd0Layer);
	SAFE_DELETE(m_pGrd1Layer);
	SAFE_DELETE(m_pGrd2Layer);
	*/
}

void CFcMenuGuardian::Initialize()
{
	if(g_bIsStartTitleMenu)
	{
		switch(g_FCGameData.nPlayerType)
		{
		case MISSION_TYPE_INPHYY:
		case MISSION_TYPE_ASPHARR:
			{
				g_GrdTypeList[0] = GT_ONE_HAND;
				g_GrdTypeList[1] = GT_TWO_HAND;
				g_GrdTypeList[2] = GT_SPEAR;
				g_GrdTypeList[3] = GT_RANGE;
				break;
			}
		case MISSION_TYPE_MYIFEE:
			{
				g_GrdTypeList[0] = GT_ONE_HAND;
				g_GrdTypeList[1] = GT_TWO_HAND;
				g_GrdTypeList[2] = GT_RANGE;
				g_GrdTypeList[3] = GT_NONE;
				break;
			}
		default: BsAssert(0);
		}
	}else
	{
		g_GrdTypeList[0] = GT_ONE_HAND;
		g_GrdTypeList[1] = GT_TWO_HAND;
		g_GrdTypeList[2] = GT_SPEAR;
		g_GrdTypeList[3] = GT_RANGE;
	}

	g_FCGameData.GuardianInfo[0].clear();
	g_FCGameData.GuardianInfo[1].clear();

	g_MenuHandle->GetMenuWorldMapInfo()->Load();
	WorldMapPointLIST* pPointList = g_MenuHandle->GetMenuWorldMapInfo()->GetPointList();
	BsAssert(pPointList);

	m_pPointInfo = NULL;
	for(DWORD i=0; i<pPointList->size(); i++)
	{
		GAME_STAGE_ID StageId = (GAME_STAGE_ID)((*pPointList)[i].nId);
		if(g_FCGameData.SelStageId == StageId)
		{
			m_pPointInfo = &((*pPointList)[i]);
			break;
		}
	}
	BsAssert(m_pPointInfo);

	for(int i=0; i<2; i++)
	{
		int nSoxID = m_pPointInfo->nGuadian_L;
		if(i == 1){
			nSoxID = m_pPointInfo->nGuadian_R;
		}

		bool bValue = false;
		for(int j=0; j<GT_MAX; j++)
		{	
			if(m_pPointInfo->GuadianType[j].nID == nSoxID)
			{
				bValue = true;
				break;
			}
		}

		BsAssert(bValue);
	}

	m_nGrdType_L = GetGuardianType(m_pPointInfo->nGuadian_L);
	m_nGrdType_R = GetGuardianType(m_pPointInfo->nGuadian_R);

	m_nSelGrdTick_L = 0;
	m_nSelGrdTick_R = 0;
}



void CFcMenuGuardian::RenderProcess()
{
	switch(GetStatus())
	{
	case _MS_OPEN:
		{
			m_pGrd0Layer->SetColor(D3DXCOLOR(1, 1, 1, 0.f));
			m_pGrd1Layer->SetColor(D3DXCOLOR(1, 1, 1, 0.f));
			m_pGrd2Layer->SetColor(D3DXCOLOR(1, 1, 1, 0.f));

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


#define _START_POS_X_GRD1				-640
#define _END_POS_X_GRD1					0
#define _START_POS_X_GRD2				1280
#define _END_POS_X_GRD2					640
void CFcMenuGuardian::RenderMoveLayer(int nTick)
{	
	//------------------------------------------------------
	//grd 0
	if(nTick <= _TICK_FADE_IN_BASE)
	{
		float fAlpha = (float)nTick / _TICK_FADE_IN_BASE;
		m_pGrd0Layer->SetColor(D3DXCOLOR(1, 1, 1, fAlpha));
	}

	//------------------------------------------------------
	//grd 1
	if(nTick <= _TICK_IN_FORM_LEFT)
	{	
		float frate = 1.f - ((float)nTick / _TICK_IN_FORM_LEFT);
		int nX = (int)(_START_POS_X_GRD1 * frate);
		m_pGrd1Layer->SetWindowPosX(nX);

		float fAlpha = 1.f - frate;
		m_pGrd1Layer->SetColor(D3DXCOLOR(1, 1, 1, fAlpha));
	}

	//------------------------------------------------------
	//grd 2
	if(nTick <= _TICK_IN_FORM_RIGHT)
	{
		float frate = (float)nTick / _TICK_IN_FORM_RIGHT;
		int nX = _END_POS_X_GRD2 + int((_START_POS_X_GRD2 - _END_POS_X_GRD2) * (1.f - frate));
		m_pGrd2Layer->SetWindowPosX(nX);

		float fAlpha = frate;
		m_pGrd2Layer->SetColor(D3DXCOLOR(1, 1, 1, fAlpha));
	}
}


void CFcMenuGuardian::RenderNormal()
{	
	int nTick = g_BsUiSystem.GetMenuTick();

	if(m_nSelGrdTick_L + _SEL_GRD_TICK >= nTick)
	{
		BsUiImageCtrl* pImgCtrl = (BsUiImageCtrl*)(m_pGrd1Layer->GetWindow(IM_Guardian_L_1));
		UpdateSelGrdImg(m_pGrd1Layer, pImgCtrl, nTick - m_nSelGrdTick_L);
	}

	if(m_nSelGrdTick_R + _SEL_GRD_TICK >= nTick)
	{
		BsUiImageCtrl* pImgCtrl = (BsUiImageCtrl*)(m_pGrd2Layer->GetWindow(IM_Guardian_R_1));
		UpdateSelGrdImg(m_pGrd2Layer, pImgCtrl, nTick - m_nSelGrdTick_R);
	}	
}

void CFcMenuGuardian::UpdateSelGrdImg(BsUiLayer* pLayer, BsUiImageCtrl* pImgCtrl, int ntempTick)
{	
	int nIncrease = (int)(128 * (ntempTick+_SEL_GRD_INCREASE_GAP) / (_SEL_GRD_TICK+_SEL_GRD_INCREASE_GAP));

	int nX = (int)(pLayer->GetWindowPos().x + g_vImagePos[1].x - 128 - nIncrease);
	int nY = (int)(pLayer->GetWindowPos().y + g_vImagePos[1].y - 128 - nIncrease);
	pImgCtrl->SetWindowPos(nX, nY);

	int nSize = 256 + nIncrease * 2;
	pImgCtrl->SetWindowSize(nSize, nSize);
}


void CFcMenuGuardian::UpdateGrdImgList(BsUiCLASS hImg, int nSelType)
{
	int nType = -1;
	if(hImg == IM_Guardian_L_1){
		nType = m_nGrdType_L;
	}else{
		nType = m_nGrdType_R;
	}

	int nCount = GT_MAX;
	if(g_FCGameData.nPlayerType == MISSION_TYPE_MYIFEE){
		nCount = GT_MAX - 1;
	}

	for(int i=0; i<nCount; i++)
	{	
		BsUiCLASS hClass = hImg + i;
		if(g_FCGameData.nPlayerType == MISSION_TYPE_MYIFEE)
		{
			if(hClass == hImg + 2){
				hClass = hImg + 3;
			}

			if(nType == GT_SPEAR){
				nType = GT_RANGE;
			}
		}

		BsUiImageCtrl* pImgCtrl = NULL;
		if(hImg == IM_Guardian_L_1){
			pImgCtrl = (BsUiImageCtrl*)(m_pGrd1Layer->GetWindow(hClass));
		}else{
			pImgCtrl = (BsUiImageCtrl*)(m_pGrd2Layer->GetWindow(hClass));
		}

		BsAssert(pImgCtrl != NULL);
		UpdateGrdImg(pImgCtrl, nType, i ? false : true);

		nType++;
		if(nType > GT_RANGE){
			nType = GT_ONE_HAND;
		}
	}
}


void CFcMenuGuardian::UpdateGrdImg(BsUiImageCtrl* pImgCtrl, int nType, bool bSel)
{
	int nTexID = GetGuardianImgTexID(g_FCGameData.nPlayerType, nType, bSel);
	BsAssert(nTexID != -1);

	ImageCtrlInfo ImgCtrlInfo;
	ImgCtrlInfo.nTexId = nTexID;
	ImgCtrlInfo.u1 = 0;
	ImgCtrlInfo.v1 = 0;
	ImgCtrlInfo.u2 = 512;
	ImgCtrlInfo.v2 = 512;

	pImgCtrl->SetImageInfo(&ImgCtrlInfo);
}


void CFcMenuGuardian::ProcessMsg(int nMsgAsk, BsUiHANDLE hWnd, int nMsgRsp)
{
	if(nMsgAsk == fcMT_MissionStart)
	{
		if(nMsgRsp == ID_YES)
		{	
			SetStatus(_MS_CLOSING);
			g_MenuHandle->PostMessage(fcMSG_MISSION_START);

			if(m_nGrdType_L != -1){
				g_FCGameData.GuardianInfo[0] = m_pPointInfo->GuadianType[m_nGrdType_L];
			}

			if(m_nGrdType_R != -1){
				g_FCGameData.GuardianInfo[1] = m_pPointInfo->GuadianType[m_nGrdType_R];
			}
		}
		else
		{
			g_BsUiSystem.SetFocusWindow(BsUi::BsUiGetWindow(hWnd));
		}
	}
}

//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------
CFcMenuGrd1Layer::CFcMenuGrd1Layer(CFcMenuGuardian* pMenu)
{
	m_pMenu = pMenu;
}

void CFcMenuGrd1Layer::Initialize()
{
	for(int i=0; i<GT_MAX; i++)
	{
		int nType = m_pMenu->g_GrdTypeList[i];

		BsUiHANDLE hClass = BT_Guardian_L_list_1 + i;
		if(nType == -1)
		{
			GetWindow(hClass)->SetWindowAttr(XWATTR_DISABLE, true);
			GetWindow(hClass)->SetWindowAttr(XWATTR_SHOWWINDOW, false);
		}
		else
		{
			int nTextId = _TEX_GUADIAN_ONE_HAND + nType;
			GetWindow(hClass)->SetItemTextID(nTextId);
			GetWindow(hClass)->SetItemData(nType);

			if(m_pMenu->m_nGrdType_L == nType){
				g_BsUiSystem.SetFocusWindow(GetWindow(hClass));
			}
		}
	}

	m_pMenu->UpdateGrdImgList(IM_Guardian_L_1, m_pMenu->m_nGrdType_L);
	SetColor(D3DXCOLOR(1, 1, 1, 0.f));
}

DWORD CFcMenuGrd1Layer::ProcMessage(xwMessageToken* pMsgToken)
{
	BsAssert(pMsgToken);

	switch(pMsgToken->message)
	{
	case XWMSG_KEYDOWN:
		{
			switch (pMsgToken->lParam)
			{
			case MENU_INPUT_START:
				{	
					g_MenuHandle->PostMessage(fcMSG_MESSAGEBOX_START,
						fcMT_MissionStart,
						(DWORD)m_pMenu,
						pMsgToken->hWnd);
					break;
				}
			case MENU_INPUT_B:
				{	
					m_pMenu->SetStatus(_MS_CLOSING);
					g_MenuHandle->PostMessage(fcMSG_SELECT_STAGE_START);
					break;
				}
			case MENU_INPUT_UP:
				{
					BsUiWindow* pPrev = SetFocusPrevChileWindow(pMsgToken->hWnd, true);
					if(pPrev != NULL && pPrev->GetHWnd() != pMsgToken->hWnd)
					{
						g_pSoundManager->PlaySystemSound( SB_COMMON,"SYS_CURSOR");

						m_pMenu->m_nGrdType_L = pPrev->GetItemData();
						m_pMenu->m_nSelGrdTick_L = g_BsUiSystem.GetMenuTick();
						m_pMenu->UpdateGrdImgList(IM_Guardian_L_1, m_pMenu->m_nGrdType_L);
					}
					break;
				}	
			case MENU_INPUT_DOWN:
				{
					BsUiWindow* pNext = SetFocusNextChileWindow(pMsgToken->hWnd, true);
					if(pNext != NULL && pNext->GetHWnd() != pMsgToken->hWnd)
					{
						g_pSoundManager->PlaySystemSound( SB_COMMON,"SYS_CURSOR");
						
						m_pMenu->m_nGrdType_L = pNext->GetItemData();
						m_pMenu->m_nSelGrdTick_L = g_BsUiSystem.GetMenuTick();
						m_pMenu->UpdateGrdImgList(IM_Guardian_L_1, m_pMenu->m_nGrdType_L);
					}
					break;
				}
			case MENU_INPUT_RIGHT:
				{
					if(m_pMenu->m_nGrdType_R != -1)
					{
						CFcMenuGrd2Layer* pGrd2Layer = m_pMenu->m_pGrd2Layer;
						g_BsUiSystem.SetFocusWindow(pGrd2Layer);
					}
					break;
				}
			}
		}
		break;
	case XWMSG_BN_CLICKED:
		{	
			break;
		}
	}

	return BsUiWindow::ProcMessage(pMsgToken);
}

//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------
CFcMenuGrd2Layer::CFcMenuGrd2Layer(CFcMenuGuardian* pMenu)
{
	m_pMenu = pMenu;
}

void CFcMenuGrd2Layer::Initialize()
{
	for(int i=0; i<GT_MAX; i++)
	{
		int nType = m_pMenu->g_GrdTypeList[i];

		BsUiHANDLE hClass = BT_Guardian_R_list_1 + i;
		if(nType == -1)
		{
			GetWindow(hClass)->SetWindowAttr(XWATTR_DISABLE, true);
			GetWindow(hClass)->SetWindowAttr(XWATTR_SHOWWINDOW, false);
		}
		else
		{
			int nTextId = _TEX_GUADIAN_ONE_HAND + nType;
			GetWindow(hClass)->SetItemTextID(nTextId);
			GetWindow(hClass)->SetItemData(nType);

			if(m_pMenu->m_nGrdType_R == nType){
				g_BsUiSystem.SetFocusWindow(GetWindow(hClass));
			}
		}
	}

	m_pMenu->UpdateGrdImgList(IM_Guardian_R_1, m_pMenu->m_nGrdType_R);
	SetColor(D3DXCOLOR(1, 1, 1, 0.f));
}

DWORD CFcMenuGrd2Layer::ProcMessage(xwMessageToken* pMsgToken)
{
	BsAssert(pMsgToken);

	BsUiCLASS hClass = BsUi::BsUiGetClass(pMsgToken->hWnd);

	switch(pMsgToken->message)
	{
	case XWMSG_KEYDOWN:
		{
			switch (pMsgToken->lParam)
			{
			case MENU_INPUT_START:
				{	
					g_MenuHandle->PostMessage(fcMSG_MESSAGEBOX_START,
						fcMT_MissionStart,
						(DWORD)m_pMenu,
						pMsgToken->hWnd);
					break;
				}
			case MENU_INPUT_B:
				{	
					m_pMenu->SetStatus(_MS_CLOSING);
					g_MenuHandle->PostMessage(fcMSG_SELECT_STAGE_START);
					break;
				}
			case MENU_INPUT_UP:
				{
					BsUiWindow* pPrev = SetFocusPrevChileWindow(pMsgToken->hWnd, true);
					if(pPrev != NULL && pPrev->GetHWnd() != pMsgToken->hWnd)
					{
						g_pSoundManager->PlaySystemSound( SB_COMMON,"SYS_CURSOR");

						m_pMenu->m_nGrdType_R = pPrev->GetItemData();
						m_pMenu->m_nSelGrdTick_R = g_BsUiSystem.GetMenuTick();
						m_pMenu->UpdateGrdImgList(IM_Guardian_R_1, m_pMenu->m_nGrdType_R);
					}
					break;
				}	
			case MENU_INPUT_DOWN:
				{
					BsUiWindow* pNext = SetFocusNextChileWindow(pMsgToken->hWnd, true);
					if(pNext != NULL && pNext->GetHWnd() != pMsgToken->hWnd)
					{
						g_pSoundManager->PlaySystemSound( SB_COMMON,"SYS_CURSOR");

						m_pMenu->m_nGrdType_R = pNext->GetItemData();
						m_pMenu->m_nSelGrdTick_R = g_BsUiSystem.GetMenuTick();
						m_pMenu->UpdateGrdImgList(IM_Guardian_R_1, m_pMenu->m_nGrdType_R);
					}
					break;
				}
			case MENU_INPUT_LEFT:
				{
					if(m_pMenu->m_nGrdType_L != -1)
					{
						CFcMenuGrd1Layer* pGrd1Layer = m_pMenu->m_pGrd1Layer;
						g_BsUiSystem.SetFocusWindow(pGrd1Layer);
					}
					break;
				}
			}
		}
		break;
	case XWMSG_BN_CLICKED:
		{	
			break;
		}
	}

	return BsUiWindow::ProcMessage(pMsgToken);
}