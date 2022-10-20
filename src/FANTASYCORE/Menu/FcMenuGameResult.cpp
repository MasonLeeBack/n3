#include "stdafx.h"

#include "FcMenuGameResult.h"
#include "BSuiSystem.h"

#include "BsUiClassIDManager.h"
#include "BsUiFontAttrManager.h"

#include "FcInterfaceManager.h"
#include "FcMessageDef.h"
#include "FantasyCore.h"

#include "FcRealtimeMovie.h"
#include "FcSoundManager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG


#define SUCCESS_GAME_RESULT_TIME	4 * 40
#define GAME_RESULT_TIME			4 * 40

CFcMenuGameResult::CFcMenuGameResult(_FC_MENU_TYPE nType)
: CFcMenuForm(nType)
{
	if(nType == _FC_MENU_GAME_CLEAR)
	{
		AddUiLayer(SN_GameClear, -1, -1);
		if( g_FCGameData.SelStageId != STAGE_ID_TUTORIAL )
			m_nLimitTick = SUCCESS_GAME_RESULT_TIME;
		else
			m_nLimitTick = 1 * 40;
	}
	else if(nType == _FC_MENU_GAME_OVER)
	{
		AddUiLayer(SN_GameOver, -1, -1);
		m_nLimitTick = GAME_RESULT_TIME;
	}

	m_ntime = g_BsUiSystem.GetMenuTick();
	m_bLoadRealMovie = false;
	m_bPlayRealMovie = false;

	DebugString("__ CFcMenuGameResult class Set time : %d\n",
		g_BsUiSystem.GetSkin()->GetTime() );

	SetStatus(_MS_NORMAL);
}


void CFcMenuGameResult::RenderProcess()
{
	if(m_bLoadRealMovie == false)
	{
		if(g_FCGameData.cFinishRealtimeMovieFileName[0] != NULL)
		{
			DebugString("__ PreLoadRealMovie CFcMenuGameResult::RenderProcess Set time : %d",
				g_BsUiSystem.GetSkin()->GetTime() );

			g_pFcRealMovie->PreLoadRealMovie(g_FCGameData.cFinishRealtimeMovieFileName);
		}
		m_bLoadRealMovie = true;
	}

	int ntime = g_BsUiSystem.GetMenuTick();
	// by cizi
	// DebugString("__ CFcMenuGameResult::RenderProcess : m_nLimitTime : %d, ntime : %d\n", m_ntime + m_nLimitTick, ntime );

	if(m_ntime + m_nLimitTick < ntime)
	{
		if(GetMenuType() == _FC_MENU_GAME_CLEAR)
		{
			//mission이 성공하면 real movie 작동.
			if(g_FCGameData.cFinishRealtimeMovieFileName[0] != NULL)
			{
				if(!m_bPlayRealMovie && g_pFcRealMovie->IsLoadComplete())
				{
					DebugString("____ StartRealMovie ( LimitTime < ntime ) : m_nLimitTime : %d, ntime : %d\n",
						m_ntime+m_nLimitTick, ntime );

					g_pFcRealMovie->StartRealMovie();
					BsUiHANDLE hLayer = GetUiLayer(SN_GameClear);
					BsUiLayer* pLayer = (BsUiLayer*)BsUi::BsUiGetWindow(hLayer);
					pLayer->SetWindowAttr(XWATTR_SHOWWINDOW, false);
					BsAssert(pLayer &&"BsUiGetWindow may return NULL"); //aleksger: prefix bug 789: BsUiGetWindow may return NULL;
					m_bPlayRealMovie = true;
				}
				else if(m_bPlayRealMovie && g_pFcRealMovie->IsPlay() == false){
					SetNextMenufromSuccess();
				}
			}
			else{
				SetNextMenufromSuccess();
			}
		}
		else if(GetMenuType() == _FC_MENU_GAME_OVER){
			SetNextMenufromFailed();
		}
	}
}

void CFcMenuGameResult::SetNextMenufromSuccess()
{
	if(g_FC.IsPause() == true){
		return;
	}
	
	if(g_FCGameData.SelStageId == STAGE_ID_TUTORIAL)
	{
		g_MenuHandle->PostMessage(fcMSG_GO_TO_TITLE);
		g_FCGameData.stageInfo.Clear();
		return;
	}
	else
	{
		g_FC.SetMainState( GAME_STATE_INIT_RESULT );
		g_MenuHandle->PostMessage(fcMSG_STAGE_RESULT_START);
	}
}

void CFcMenuGameResult::SetNextMenufromFailed()
{
	if(g_FC.IsPause() == true){
		return;
	}

	g_MenuHandle->PostMessage(fcMSG_GAME_FAILED_START);
}

//---------------------------------------------------------
//---------------------------------------------------------
CFcMenuFailed::CFcMenuFailed(_FC_MENU_TYPE nType)
: CFcMenuForm(nType)
{
	m_pLayer = new CFcMenuFailedLayer(this);
	AddUiLayer(m_pLayer, SN_Failed, -1, -1);
	m_pLayer->Initialize();
}

void CFcMenuFailed::ProcessMsg(int nMsgAsk, BsUiHANDLE hWnd, int nMsgRsp)
{
#ifdef _XBOX
	switch(nMsgAsk)
	{
	case fcMT_RestartMission:
		{
			if(nMsgRsp == ID_YES)
			{
				g_FCGameData.RollBack();

				if( g_FCGameData.nNumMissionFail % 3 == 0 && g_FCGameData.nNumMissionFail > 0 )
				{
					int nHeroID = g_MissionToSoxID[g_FCGameData.nPlayerType];
					HeroRecordInfo* pHeroRecordInfo = g_FCGameData.tempFixedSave.GetHeroRecordInfo(nHeroID);
					StageResultInfo* pStageResultInfo = NULL;
					if(pHeroRecordInfo)
						pStageResultInfo = pHeroRecordInfo->GetStageResultInfo(g_FCGameData.SelStageId);

					if( pStageResultInfo )
					{
						if( pStageResultInfo->nEnemyLevelUp > -2 )
							pStageResultInfo->nEnemyLevelUp--;
					}
					else
					{
						if( g_FCGameData.stageInfo.nEnemyLevelUp > -2 )
							g_FCGameData.stageInfo.nEnemyLevelUp--;
					}
				}

				SetStatus(_MS_CLOSING);
				g_MenuHandle->PostMessage(fcMSG_MISSION_START);
			}
			else
			{
				g_BsUiSystem.SetFocusWindow(BsUi::BsUiGetWindow(hWnd));
				m_pLayer->SetWindowAttr(XWATTR_SHOWWINDOW, true);
			}
			break;
		}
	case fcMT_GotoSelectStage:
		{
			if(nMsgRsp == ID_YES)
			{
				g_FCGameData.RollBack();
				g_FCGameData.SelStageId = g_FCGameData.PrevStageId;

				SetStatus(_MS_CLOSING);
				g_MenuHandle->PostMessage(fcMSG_GO_TO_SEL_STAGE);
			}
			else
			{
				g_BsUiSystem.SetFocusWindow(BsUi::BsUiGetWindow(hWnd));
				m_pLayer->SetWindowAttr(XWATTR_SHOWWINDOW, true);
			}
			break;
		}
	case fcMT_ExitMissionToTitle:
		{
			if(nMsgRsp == ID_YES)
			{
				SetStatus(_MS_CLOSING);
				g_MenuHandle->PostMessage(fcMSG_GO_TO_TITLE);
			}
			else
			{
				g_BsUiSystem.SetFocusWindow(BsUi::BsUiGetWindow(hWnd));
				m_pLayer->SetWindowAttr(XWATTR_SHOWWINDOW, true);
			}
			break;
		}
	}
#endif //_XBOX
}



void CFcMenuFailed::RenderProcess()
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

void CFcMenuFailed::RenderMoveLayer(int nTick)
{
	if(nTick <= _TICK_FADE_IN_BASE)
	{
		float fAlpha = (float)nTick / _TICK_FADE_IN_BASE;
		m_pLayer->SetColor(D3DXCOLOR(1, 1, 1, fAlpha));
	}
}

//-----------------------------------------------------------------------------------------------------
void CFcMenuFailedLayer::Initialize()
{
	if(g_FCGameData.SelStageId == STAGE_ID_TUTORIAL)
	{
		GetWindow(BT_Failed_SelectStage)->SetWindowAttr(XWATTR_DISABLE, true);
		return;
	}

	//go to world map 버튼의 활성화 결정
	if(g_bIsStartTitleMenu)
	{
		std::vector<int>* pClearStageList = &(g_FCGameData.tempUserSave.ClearStageList);
		if(pClearStageList->size() == 0){
			GetWindow(BT_Failed_SelectStage)->SetWindowAttr(XWATTR_DISABLE, true);
		}		
	}

	//실패시 값을 저장한다.---------------------------------------------------------------------
	int nHeroSoxId = g_MissionToSoxID[g_FCGameData.nPlayerType];
	HeroRecordInfo* pHeroRecordInfo = g_FCGameData.tempFixedSave.GetHeroRecordInfo(nHeroSoxId);
	if(pHeroRecordInfo == NULL)
	{
		HeroRecordInfo HeroInfo;
		HeroInfo.nHeroID = nHeroSoxId;
		g_FCGameData.tempFixedSave.HeroRecordList.push_back(HeroInfo);
		pHeroRecordInfo = g_FCGameData.tempFixedSave.GetHeroRecordInfo(nHeroSoxId);
	}

// [PREFIX:beginmodify] 2006/2/25 junyash PS#5823 dereferencing NULL pointer
	BsAssert(pHeroRecordInfo);
// [PREFIX:endmodify] junyash
	StageResultInfo* pRecordStageInfo = pHeroRecordInfo->GetStageResultInfo(g_FCGameData.SelStageId);
	if(pRecordStageInfo == NULL)
	{
		StageResultInfo StageInfo;
		StageInfo.nStageID = g_FCGameData.SelStageId;
		StageInfo.nMissionFaultCount = 1;
		pHeroRecordInfo->StageResultList.push_back(StageInfo);
	}
	else{
		pRecordStageInfo->nMissionFaultCount++;
	}

	if(g_FCGameData.m_DeviceID != XCONTENTDEVICE_ANY)
		g_FCGameData.SaveFixedGameData();
	g_FCGameData.stageInfo.Clear();
}

DWORD CFcMenuFailedLayer::ProcMessage(xwMessageToken* pMsgToken)
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
					case BT_Failed_Continue:		OnKeyDownBtRestart(pMsgToken); break;
					case BT_Failed_SelectStage:		OnKeyDownBtStageSel(pMsgToken); break;
					case BT_Failed_Exit:			OnKeyDownBtGotoTitle(pMsgToken); break;
					default:
						BsAssert(0);
					}
					break;
				}
			}
			break;
		}
	}

	return BsUiWindow::ProcMessage(pMsgToken);
}

void CFcMenuFailedLayer::OnKeyDownBtRestart(xwMessageToken* pMsgToken)
{
	switch (pMsgToken->lParam)
	{
	case MENU_INPUT_A:
		{
			g_MenuHandle->PostMessage(fcMSG_MESSAGEBOX_START,
				fcMT_RestartMission,
				(DWORD)m_pMenu, pMsgToken->hWnd, 1);
			SetWindowAttr(XWATTR_SHOWWINDOW, false);
			break;
		}
	}
}

void CFcMenuFailedLayer::OnKeyDownBtStageSel(xwMessageToken* pMsgToken)
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

void CFcMenuFailedLayer::OnKeyDownBtGotoTitle(xwMessageToken* pMsgToken)
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