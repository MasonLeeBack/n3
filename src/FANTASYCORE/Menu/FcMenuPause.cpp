#include "stdafx.h"

#include "FcMenuPause.h"
#include "BSuiSystem.h"

#include "BsUiClassIDManager.h"
#include "BsUiFontAttrManager.h"
#include "FantasyCore.h"
#include "FcInterfaceManager.h"
#include "FcMessageDef.h"

#include "FcWorld.h"
#include "FcSoundManager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG



CFcMenuPause::CFcMenuPause(_FC_MENU_TYPE nType)
: CFcMenuForm(nType)
{
	CFcMenuPauseLayer* pLayer = new CFcMenuPauseLayer(this);
	AddUiLayer(pLayer, SN_Pause, -1, -1);
	pLayer->Initialize();
}


void CFcMenuPause::SetFocus(int nMenuType)
{
	BsUiHANDLE hLayer = GetUiLayer(SN_Pause);
	BsUiLayer* pLayer = (BsUiLayer*)BsUi::BsUiGetWindow(hLayer);
	switch(nMenuType)
	{
	case _FC_MENU_NONE:			pLayer->SetFocusWindowClass(BT_PauseCountinue); break;
	case _FC_MENU_STATUS:		pLayer->SetFocusWindowClass(BT_PauseStatus); break;
	case _FC_MENU_ABILITY:		pLayer->SetFocusWindowClass(BT_PauseAbility); break;
	case _FC_MENU_OPTION:		pLayer->SetFocusWindowClass(BT_PauseOption); break;
	case _FC_MENU_CONTROL:		pLayer->SetFocusWindowClass(BT_PauseControl); break;
	case _FC_MENU_MISSION_OBJ:	pLayer->SetFocusWindowClass(BT_PauseObjective); break;
	}
}

void CFcMenuPause::ProcessMsg(int nMsgAsk, BsUiHANDLE hWnd, int nMsgRsp)
{
	if(nMsgAsk == fcMT_GotoSelectStage)
	{
		if(nMsgRsp == ID_YES)
		{
			g_FCGameData.RollBack();
			g_FCGameData.SelStageId = g_FCGameData.PrevStageId;
			g_MenuHandle->PostMessage(fcMSG_GO_TO_SEL_STAGE);
		}
		else
		{
			g_BsUiSystem.SetFocusWindow(BsUi::BsUiGetWindow(hWnd));

			BsUiHANDLE hLayer = GetUiLayer(SN_Pause);
			BsUiLayer* pLayer = (BsUiLayer*)BsUi::BsUiGetWindow(hLayer);
			pLayer->SetWindowAttr(XWATTR_SHOWWINDOW, true);
		}
	}
	else if(nMsgAsk == fcMT_ExitMissionToTitle)
	{
		if(nMsgRsp == ID_YES)
		{
			g_MenuHandle->PostMessage(fcMSG_GO_TO_TITLE);
		}
		else
		{
			g_BsUiSystem.SetFocusWindow(BsUi::BsUiGetWindow(hWnd));

			BsUiHANDLE hLayer = GetUiLayer(SN_Pause);
			BsUiLayer* pLayer = (BsUiLayer*)BsUi::BsUiGetWindow(hLayer);
			pLayer->SetWindowAttr(XWATTR_SHOWWINDOW, true);
		}
	}
}

void CFcMenuPause::RenderProcess()
{
	switch(GetStatus())
	{
	case _MS_OPEN:		SetStatus(_MS_OPENING); break;
	case _MS_OPENING:	SetStatus(_MS_NORMAL); break;
	case _MS_NORMAL:	break;
	case _MS_CLOSING:
		{
			SetStatus(_MS_CLOSE);
			g_MenuHandle->PostMessage(fcMSG_PAUSE_END);
			break;
		}
	case _MS_CLOSE:	break;
	}
}

//-----------------------------------------------------------------------------------------------------
CFcMenuPauseLayer::CFcMenuPauseLayer(CFcMenuForm* pMenu)
{
	m_pMenu = pMenu;
}

void CFcMenuPauseLayer::Initialize()
{
	FcMissionObjList* pFcMissionObjList = g_FcWorld.GetMIssionObjective();
	if(pFcMissionObjList->size() == 0){
		GetWindow(BT_PauseObjective)->SetWindowAttr(XWATTR_DISABLE, true);
	}

	if(g_FCGameData.SelStageId == STAGE_ID_TUTORIAL){
		GetWindow(BT_PauseSelect)->SetWindowAttr(XWATTR_DISABLE, true);
	}

	std::vector<int>* pClearStageList = &(g_FCGameData.tempUserSave.ClearStageList);
	if(pClearStageList->size() == 0){
		GetWindow(BT_PauseSelect)->SetWindowAttr(XWATTR_DISABLE, true);
	}
}

DWORD CFcMenuPauseLayer::ProcMessage(xwMessageToken* pMsgToken)
{
	BsAssert(pMsgToken);
	switch(pMsgToken->message)
	{
	case XWMSG_KEYDOWN:
		{
			switch (pMsgToken->lParam)
			{
			case MENU_INPUT_B:
			case MENU_INPUT_START:
				{
					m_pMenu->SetStatus(_MS_CLOSING);
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
					case BT_PauseCountinue: OnKeyDownBtContinue(pMsgToken); break;
					case BT_PauseStatus:    OnKeyDownBtStatus(pMsgToken); break;
					case BT_PauseAbility:   OnKeyDownBtAbility(pMsgToken); break;
					case BT_PauseControl:   OnKeyDownBtControl(pMsgToken); break;
					case BT_PauseObjective:	OnKeyDownBtMission(pMsgToken); break;
					case BT_PauseOption:    OnKeyDownBtOption(pMsgToken); break;
					case BT_PauseSelect:    OnKeyDownBtSelStage(pMsgToken); break;
					case BT_PauseExit:      OnKeyDownBtExit(pMsgToken); break;
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


void CFcMenuPauseLayer::OnKeyDownBtContinue(xwMessageToken* pMsgToken)
{
	switch (pMsgToken->lParam)
	{
	case MENU_INPUT_A:
		{	
			g_MenuHandle->PostMessage(fcMSG_PAUSE_END);
			break;
		}
	}
}

void CFcMenuPauseLayer::OnKeyDownBtStatus(xwMessageToken* pMsgToken)
{
	switch (pMsgToken->lParam)
	{
	case MENU_INPUT_A:
		{	
			g_MenuHandle->PostMessage(fcMSG_STATUS_START);
			break;
		}
	}
}

void CFcMenuPauseLayer::OnKeyDownBtAbility(xwMessageToken* pMsgToken)
{
	switch (pMsgToken->lParam)
	{
	case MENU_INPUT_A:
		{	
			g_MenuHandle->PostMessage(fcMSG_ABILITY_START);
			break;
		}
	}
}

void CFcMenuPauseLayer::OnKeyDownBtControl(xwMessageToken* pMsgToken)
{
	switch (pMsgToken->lParam)
	{
	case MENU_INPUT_A:
		{
			g_MenuHandle->PostMessage(fcMSG_CONTROL_START);
			break;
		}
	}
}


void CFcMenuPauseLayer::OnKeyDownBtMission(xwMessageToken* pMsgToken)
{
	switch (pMsgToken->lParam)
	{
	case MENU_INPUT_A:
		{
			g_MenuHandle->PostMessage(fcMSG_MISSION_OBJ_START);
			break;
		}
	}
}

void CFcMenuPauseLayer::OnKeyDownBtOption(xwMessageToken* pMsgToken)
{
	switch (pMsgToken->lParam)
	{
	case MENU_INPUT_A:
		{
			g_MenuHandle->PostMessage(fcMSG_OPTION_START);
			break;
		}
	}
}

void CFcMenuPauseLayer::OnKeyDownBtSelStage(xwMessageToken* pMsgToken)
{
	switch (pMsgToken->lParam)
	{
	case MENU_INPUT_A:
		{
			g_MenuHandle->PostMessage(fcMSG_MESSAGEBOX_START,
				fcMT_GotoSelectStage,
				(DWORD)m_pMenu, pMsgToken->hWnd, 1);
			SetWindowAttr(XWATTR_SHOWWINDOW, false);
			break;
		}
	}
}

void CFcMenuPauseLayer::OnKeyDownBtExit(xwMessageToken* pMsgToken)
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



//-----------------------------------------------------------------
//-----------------------------------------------------------------
//-----------------------------------------------------------------
//-----------------------------------------------------------------
#define _SEL_SPECIAL_STAGE_TICK		10
#define _SEL_SPECIAL_STAGE_1		STAGE_ID_WYANDEEK
#define _SEL_SPECIAL_STAGE_2		STAGE_ID_VARRVAZZAR

CFcMenuSpecialSelStage::CFcMenuSpecialSelStage(_FC_MENU_TYPE nType)
: CFcMenuForm(nType)
{
	CFcMenuSpecialSelStageLayer* pLayer = new CFcMenuSpecialSelStageLayer(this);
	AddUiLayer(pLayer, SN_SSS, -1, -1);
	pLayer->Initialize();

	m_ntime = _SEL_SPECIAL_STAGE_TICK;
	SetStatus(_MS_NORMAL);
}


void CFcMenuSpecialSelStage::RenderProcess()
{
	//이미지 변화 비선택은 어두운 판을 위에 씌우고 글씨 이미지는 안보인다.
	BsUiHANDLE hLayer = GetUiLayer(SN_SSS);
	BsUiLayer* pLayer = (BsUiLayer*)BsUi::BsUiGetWindow(hLayer);

	int nMission = ((CFcMenuSpecialSelStageLayer*)pLayer)->GetStage();

	BsUiWindow* pWindowE = NULL;
	BsUiWindow* pWindowJ = NULL;
	RECT rectRightIn, rectLeftIn;
	if(nMission == _SEL_SPECIAL_STAGE_1)
	{
		pWindowE= pLayer->GetChildWindow(IM_SSS_Wdk_E);
		pWindowJ = pLayer->GetChildWindow(IM_SSS_Wdk_J);

		rectLeftIn = ((CFcMenuSpecialSelStageLayer*)pLayer)->GetIErect();
		rectRightIn = ((CFcMenuSpecialSelStageLayer*)pLayer)->GetIJrect();
	}	
	else
	{	
		pWindowE = pLayer->GetChildWindow(IM_SSS_Vz_E);
		pWindowJ = pLayer->GetChildWindow(IM_SSS_Vz_J);

		rectLeftIn = ((CFcMenuSpecialSelStageLayer*)pLayer)->GetAErect();
		rectRightIn = ((CFcMenuSpecialSelStageLayer*)pLayer)->GetAJrect();
	}
	
	int nTick = _SEL_SPECIAL_STAGE_TICK - m_ntime;
	if(nTick >= 0)
	{
		float frate = (float)nTick / _SEL_SPECIAL_STAGE_TICK;
		D3DXVECTOR2 Out;
		D3DXVec2Lerp(&Out,
			&D3DXVECTOR2((float)rectLeftIn.left, (float)rectLeftIn.top),
			&D3DXVECTOR2((float)(-rectLeftIn.right + rectLeftIn.left), (float)rectLeftIn.top),
			frate);

		pWindowE->SetWindowPos((int)Out.x, (int)Out.y);
		pWindowE->SetColor(D3DXCOLOR(1, 1, 1, 1.f - frate));

		D3DXVec2Lerp(&Out,
			&D3DXVECTOR2((float)rectRightIn.left, (float)rectRightIn.top),
			&D3DXVECTOR2(_SCREEN_WIDTH, (float)rectRightIn.top),
			(float)nTick / _SEL_SPECIAL_STAGE_TICK);

		pWindowJ->SetWindowPos((int)Out.x, (int)Out.y);
		pWindowJ->SetColor(D3DXCOLOR(1, 1, 1, 1.f - frate));
		m_ntime++;
	}
}


void CFcMenuSpecialSelStage::ProcessMsg(int nMsgAsk, BsUiHANDLE hWnd, int nMsgRsp)
{
	if(nMsgAsk == fcMT_SpecialSelStage)
	{
		if(nMsgRsp == ID_YES)
		{
			BsUiHANDLE hLayer = GetUiLayer(SN_SSS);
			BsUiLayer* pLayer = (BsUiLayer*)BsUi::BsUiGetWindow(hLayer);
			int nMission = ((CFcMenuSpecialSelStageLayer*)pLayer)->GetStage();
			g_FCGameData.SpeaialSelStageId = (GAME_STAGE_ID)nMission;
			g_FCGameData.SpecialSelStageIdforTrigger = (GAME_STAGE_ID)nMission;
			g_MenuHandle->PostMessage(fcMSG_SPECIAL_SELSTAGE_END);
		}

        g_BsUiSystem.SetFocusWindow(BsUi::BsUiGetWindow(hWnd));
	}
}

//-----------------------------------------------------------------------------------------------------
CFcMenuSpecialSelStageLayer::CFcMenuSpecialSelStageLayer(CFcMenuForm* pMenu)
{
	m_pMenu = pMenu;
	m_nStageId = _SEL_SPECIAL_STAGE_1;
}

void CFcMenuSpecialSelStageLayer::Initialize()
{	
	GetChildWindow(IM_SSS_black)->SetWindowPos(641, 0);
	GetChildWindow(IM_SSS_black)->SetColor(D3DXCOLOR(0, 0, 0, 0.6f));

	GetChildWindow(IM_SSS_Wdk_E)->GetWindowRect(&m_rectI_E);
	GetChildWindow(IM_SSS_Wdk_J)->GetWindowRect(&m_rectI_J);

	GetChildWindow(IM_SSS_Vz_E)->GetWindowRect(&m_rectA_E);
	GetChildWindow(IM_SSS_Vz_J)->GetWindowRect(&m_rectA_J);

	GetChildWindow(IM_SSS_Wdk_E)->SetWindowAttr(XWATTR_SHOWWINDOW, true);
	GetChildWindow(IM_SSS_Wdk_J)->SetWindowAttr(XWATTR_SHOWWINDOW, true);

	GetChildWindow(IM_SSS_Vz_E)->SetWindowAttr(XWATTR_SHOWWINDOW, false);
	GetChildWindow(IM_SSS_Vz_J)->SetWindowAttr(XWATTR_SHOWWINDOW, false);

}

DWORD CFcMenuSpecialSelStageLayer::ProcMessage(xwMessageToken* pMsgToken)
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
					break;
				}
			case MENU_INPUT_A:
				{	
					g_MenuHandle->PostMessage(fcMSG_MESSAGEBOX_START,
						fcMT_SpecialSelStage,
						(DWORD)m_pMenu, pMsgToken->hWnd, 1);
					break;
				}
			case MENU_INPUT_LEFT:
				{
					if(m_nStageId == _SEL_SPECIAL_STAGE_1){
						break;
					}

					m_nStageId = _SEL_SPECIAL_STAGE_1;
					((CFcMenuSpecialSelStage*)m_pMenu)->SetTime(0);

					GetChildWindow(IM_SSS_Wdk_E)->SetWindowAttr(XWATTR_SHOWWINDOW, true);
					GetChildWindow(IM_SSS_Wdk_J)->SetWindowAttr(XWATTR_SHOWWINDOW, true);

					GetChildWindow(IM_SSS_Vz_E)->SetWindowAttr(XWATTR_SHOWWINDOW, false);
					GetChildWindow(IM_SSS_Vz_J)->SetWindowAttr(XWATTR_SHOWWINDOW, false);

					GetChildWindow(IM_SSS_black)->SetWindowPos(641, 0);
					break;
				}
			case MENU_INPUT_RIGHT:
				{
					if(m_nStageId == _SEL_SPECIAL_STAGE_2){
						break;
					}

					m_nStageId = _SEL_SPECIAL_STAGE_2;
					((CFcMenuSpecialSelStage*)m_pMenu)->SetTime(0);

					GetChildWindow(IM_SSS_Wdk_E)->SetWindowAttr(XWATTR_SHOWWINDOW, false);
					GetChildWindow(IM_SSS_Wdk_J)->SetWindowAttr(XWATTR_SHOWWINDOW, false);

					GetChildWindow(IM_SSS_Vz_E)->SetWindowAttr(XWATTR_SHOWWINDOW, true);
					GetChildWindow(IM_SSS_Vz_J)->SetWindowAttr(XWATTR_SHOWWINDOW, true);

					GetChildWindow(IM_SSS_black)->SetWindowPos(0, 0);
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