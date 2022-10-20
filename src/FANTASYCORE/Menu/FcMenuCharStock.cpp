#include "stdafx.h"

#include "FcMenuCharStock.h"
#include "BSuiSystem.h"

#include "BsUiClassIDManager.h"
#include "BsUiFontAttrManager.h"

#include "FcInterfaceManager.h"

#include "FcMessageDef.h"
#include "FcGlobal.h"
#include "FcSoundManager.h"

#include "FcMenuLibraryManager.h"
#include "TextTable.h"

#include "FantasyCore.h"
#include "FcLiveManager.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG

#define _CHAR_DATA_NONE		 "mn_chardata_no.dds"


CFcMenuCharStock::CFcMenuCharStock(_FC_MENU_TYPE nType, DWORD dwMissionType)
: CFcMenuForm(nType)
{
	BsAssert(dwMissionType >= MISSION_TYPE_INPHYY && dwMissionType < MAX_MISSION_TYPE);

	m_pBaseLayer = new BsUiLayer();
	AddUiLayer(m_pBaseLayer, SN_CharStockDB1, -1, -1);

	m_pPointLayer = new BsUiLayer();
	AddUiLayer(m_pPointLayer, SN_CharStockDB2, -1, -1);

	m_pCharLeftLayer = new CFcMenuCharLeftLayer(this);
	AddUiLayer(m_pCharLeftLayer, SN_CharStockDB3, -1, -1);

	m_pCharRightLayer = new CFcMenuCharRightLayer(this);
	AddUiLayer(m_pCharRightLayer, SN_CharStockDB4, -1, -1);
		
	UpdatePointLayer();
	UpdateLeftLayer();
	UpdateRightLayer();
	SetCharFocus(dwMissionType);
}

void CFcMenuCharStock::ProcessMsg(int nMsgAsk, BsUiHANDLE hWnd, int nMsgRsp)
{
	if(nMsgAsk == fcMT_LibOpenItem)
	{
		if(nMsgRsp == ID_YES)
		{
			DWORD dwData = (BsUi::BsUiGetWindow(hWnd))->GetItemData();

			CFcMenuLibraryManager* pLibMgr = g_MenuHandle->GetLibManager();
			int nItemID = pLibMgr->GetItemID(LIB_CHAR, dwData);

			LibraryTable* pItem = pLibMgr->GetItemData(nItemID);

			g_FCGameData.tempFixedSave.nTotalPoint -= pItem->nPoint;
			LibraryItem* pLibItem = g_FCGameData.tempFixedSave.GetLibraryData(nItemID);
			pLibItem->nState = LIS_OPEN;

			if(g_FC.GetLiveManager()->IsUserSignedIn() && g_FCGameData.m_DeviceID != XCONTENTDEVICE_ANY ){
				g_FCGameData.SaveFixedGameData();
			}

			g_MenuHandle->PostMessage(fcMSG_LIB_CHAR_SUB_START, dwData);
		}
		else{
			g_BsUiSystem.SetFocusWindow(BsUi::BsUiGetWindow(hWnd));
		}
	}
	else if(nMsgAsk == fcMT_LibOpenItemInWarning){
		g_BsUiSystem.SetFocusWindow(BsUi::BsUiGetWindow(hWnd));
	}
}

void CFcMenuCharStock::UpdatePointLayer()
{
	char cPoint[256];
	sprintf(cPoint, "%d", g_FCGameData.tempFixedSave.nTotalPoint);
	BsUiWindow* pWindow = m_pPointLayer->GetWindow(TT_CharStockDB_point_num);
	pWindow->SetItemText(cPoint);
}

void CFcMenuCharStock::UpdateLeftLayer()
{
	CFcMenuLibraryManager* pLibMgr = g_MenuHandle->GetLibManager();

	int nEnd = MISSION_TYPE_TYURRU + 1;
	for(int i=0; i<nEnd; i++)
	{
		int nItemID = pLibMgr->GetItemID(LIB_CHAR, i);

		BsUiWindow* pButton = m_pCharLeftLayer->GetWindow(BT_CharStockDB_Name_1 + i);
		BsUiWindow* pImg = m_pCharLeftLayer->GetWindow(IM_CharStockDB_Name_1 + i);
		BsUiWindow* pNew = m_pCharLeftLayer->GetWindow(TT_CharStockDB_New1 + i);
		
		pButton->SetItemData(MISSION_TYPE_INPHYY + i);
		pButton->SetViewChildFocus(false);

		UpdateSetButton(nItemID, pButton, pImg, pNew);
	}
}

void CFcMenuCharStock::UpdateRightLayer()
{
	CFcMenuLibraryManager* pLibMgr = g_MenuHandle->GetLibManager();

	int nEnd = MISSION_TYPE_VIGKVAGK - MISSION_TYPE_KLARRANN + 1;
	for(int i=0; i<nEnd; i++)
	{
		int nItemID = pLibMgr->GetItemID(LIB_CHAR, i + MISSION_TYPE_KLARRANN);

		BsUiWindow* pButton = m_pCharRightLayer->GetWindow(BT_CharStockDB_Name_5 + i);
		BsUiWindow* pImg = m_pCharRightLayer->GetWindow(IM_CharStockDB_Name_5 + i);
		BsUiWindow* pNew = m_pCharRightLayer->GetWindow(TT_CharStockDB_New5 + i);

		pButton->SetItemData(MISSION_TYPE_KLARRANN + i);
		pButton->SetViewChildFocus(false);

		UpdateSetButton(nItemID, pButton, pImg, pNew);
	}
}

void CFcMenuCharStock::UpdateSetButton(int nItemID, BsUiWindow* pButton, BsUiWindow* pImg, BsUiWindow* pNew)
{
	CFcMenuLibraryManager* pLibMgr = g_MenuHandle->GetLibManager();
	switch(pLibMgr->GetState(nItemID))
	{
	case LIS_DEFAULT:
		{	
			pButton->SetItemTextID(_TEX_NOT_OPEN_DATA);

			CBsKernel::GetInstance().chdir("Interface");
			int nTexID = g_BsKernel.LoadTexture(_CHAR_DATA_NONE);
			BsAssert(nTexID != -1);
			CBsKernel::GetInstance().chdir("..");

			SIZE size = g_BsKernel.GetTextureSize(nTexID);
			ImageCtrlInfo ImgCtrlInfo;
			ImgCtrlInfo.nTexId = nTexID;
			ImgCtrlInfo.u2 = size.cx;
			ImgCtrlInfo.v2 = size.cy;
			
			((BsUiImageCtrl*)pImg)->SetImageInfo(&ImgCtrlInfo);

			break;
		}
	case LIS_NEW:
		{	
			pNew->SetWindowAttr(XWATTR_SHOWWINDOW, true);
			break;
		}
	case LIS_OPEN: break;
	}
}

void CFcMenuCharStock::SetCharFocus(DWORD dwMissionType)
{
	BsUiWindow* pWindow = NULL;
	switch(dwMissionType)
	{
	case MISSION_TYPE_INPHYY:	pWindow = m_pCharLeftLayer->GetWindow(BT_CharStockDB_Name_1); break;
	case MISSION_TYPE_ASPHARR:	pWindow = m_pCharLeftLayer->GetWindow(BT_CharStockDB_Name_2); break;
	case MISSION_TYPE_MYIFEE:	pWindow = m_pCharLeftLayer->GetWindow(BT_CharStockDB_Name_3); break;
	case MISSION_TYPE_TYURRU:	pWindow = m_pCharLeftLayer->GetWindow(BT_CharStockDB_Name_4); break;
	case MISSION_TYPE_KLARRANN:	pWindow = m_pCharRightLayer->GetWindow(BT_CharStockDB_Name_5); break;
	case MISSION_TYPE_DWINGVATT:pWindow = m_pCharRightLayer->GetWindow(BT_CharStockDB_Name_6); break;
	case MISSION_TYPE_VIGKVAGK:	pWindow = m_pCharRightLayer->GetWindow(BT_CharStockDB_Name_7); break;
	}

	BsAssert(pWindow);
	g_BsUiSystem.SetFocusWindow(pWindow);
	UpdateNeedPoint(dwMissionType);
}

void CFcMenuCharStock::RenderProcess()
{
	switch(GetStatus())
	{
	case _MS_OPEN:
		{
			m_pBaseLayer->SetColor(D3DXCOLOR(1, 1, 1, 0.f));
			m_pPointLayer->SetColor(D3DXCOLOR(1, 1, 1, 0.f));
			m_pCharLeftLayer->SetColor(D3DXCOLOR(1, 1, 1, 0.f));
			m_pCharRightLayer->SetColor(D3DXCOLOR(1, 1, 1, 0.f));

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

#define _START_POS_X_CHAR1				-640
#define _END_POS_X_CHAR1				0
#define _START_POS_X_CHAR2				1280
#define _END_POS_X_CHAR2				640
void CFcMenuCharStock::RenderMoveLayer(int nTick)
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
		int nX = (int)(_START_POS_X_CHAR1 * frate);
		m_pCharLeftLayer->SetWindowPosX(nX);

		float fAlpha = 1.f - frate;
		m_pCharLeftLayer->SetColor(D3DXCOLOR(1, 1, 1, fAlpha));
	}

	//------------------------------------------------------
	if(nTick <= _TICK_IN_FORM_RIGHT)
	{
		float frate = (float)nTick / _TICK_IN_FORM_RIGHT;
		int nX = _END_POS_X_CHAR2 + int((_START_POS_X_CHAR2 - _END_POS_X_CHAR2) * (1.f - frate));
		m_pCharRightLayer->SetWindowPosX(nX);
		m_pPointLayer->SetWindowPosX(nX);

		float fAlpha = frate;
		m_pCharRightLayer->SetColor(D3DXCOLOR(1, 1, 1, fAlpha));
		m_pPointLayer->SetColor(D3DXCOLOR(1, 1, 1, fAlpha));
	}
}


void CFcMenuCharStock::ClickAItem(BsUiHANDLE hWnd)
{
	DWORD dwData = (BsUi::BsUiGetWindow(hWnd))->GetItemData();

	CFcMenuLibraryManager* pLibMgr = g_MenuHandle->GetLibManager();
	int nItemID = pLibMgr->GetItemID(LIB_CHAR, dwData);

	switch(pLibMgr->GetState(nItemID))
	{
	case LIS_DEFAULT:
		{
			g_pSoundManager->PlaySystemSound( SB_COMMON,"SYS_CANCEL");
			break;
		}
	case LIS_NEW:
		{
			LibraryTable* pItem = pLibMgr->GetItemData(nItemID);
			if(pItem->nPoint == 0)
			{	
				LibraryItem* pLibItem = g_FCGameData.tempFixedSave.GetLibraryData(nItemID);
				pLibItem->nState = LIS_OPEN;

				if(g_FC.GetLiveManager()->IsUserSignedIn() && g_FCGameData.m_DeviceID != XCONTENTDEVICE_ANY ){
					g_FCGameData.SaveFixedGameData();
				}
				
				SetStatus(_MS_CLOSING);
				g_MenuHandle->PostMessage(fcMSG_LIB_CHAR_SUB_START, dwData);
			}
			else if(g_FCGameData.tempFixedSave.nTotalPoint >= pItem->nPoint)
			{
				g_MenuHandle->PostMessage(fcMSG_MESSAGEBOX_START,
					fcMT_LibOpenItem,
					(DWORD)this,
					hWnd,
					NULL, (DWORD)pItem->nPoint);
			}else{
				g_MenuHandle->PostMessage(fcMSG_MESSAGEBOX_START,
					fcMT_LibOpenItemInWarning,
					(DWORD)this, hWnd);
			}
			break;
		}
	case LIS_OPEN:
		{
			SetStatus(_MS_CLOSING);
			g_MenuHandle->PostMessage(fcMSG_LIB_CHAR_SUB_START, dwData);
			break;
		}
	default: BsAssert(0);
	}
}

void CFcMenuCharStock::UpdateNeedPoint(DWORD dwMissionType)
{
	CFcMenuLibraryManager* pLibMgr = g_MenuHandle->GetLibManager();
	int nItemID = pLibMgr->GetItemID(LIB_CHAR, dwMissionType);
	LibraryTable* pItem = pLibMgr->GetItemData(nItemID);
	
	BsUiWindow* pWindow = m_pCharRightLayer->GetWindow(TT_CharStockDB_Pointinfo_num);
	pWindow->SetWindowAttr(XWATTR_SHOWWINDOW, true);

	int nNeedPoint(0);
	switch(pLibMgr->GetState(nItemID))
	{
	case LIS_DEFAULT:
		{
			pWindow->SetWindowAttr(XWATTR_SHOWWINDOW, false);
			return;
		}
	case LIS_NEW:
		{
			nNeedPoint = pItem->nPoint;
			break;
		}
	case LIS_OPEN: break;
	}

	char cNeedPoint[256];
	sprintf(cNeedPoint, "%d", nNeedPoint);
	pWindow->SetItemText(cNeedPoint);
}

//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------
DWORD CFcMenuCharLeftLayer::ProcMessage(xwMessageToken* pMsgToken)
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
					g_MenuHandle->PostMessage(fcMSG_LIBRARY_START);
					break;
				}
			case MENU_INPUT_A:
				{
					m_pMenu->ClickAItem(pMsgToken->hWnd);
					break;
				}
			case MENU_INPUT_UP:
				{
					DWORD dwData = (BsUi::BsUiGetWindow(pMsgToken->hWnd))->GetItemData();
					
					BsAssert(dwData <= MISSION_TYPE_TYURRU);

					if(dwData == MISSION_TYPE_INPHYY){
						dwData = MISSION_TYPE_TYURRU;
					}else{
						dwData--;
					}
					m_pMenu->SetCharFocus(dwData);
					g_pSoundManager->PlaySystemSound( SB_COMMON,"SYS_CURSOR");
					break;
				}
			case MENU_INPUT_DOWN:
				{
					DWORD dwData = (BsUi::BsUiGetWindow(pMsgToken->hWnd))->GetItemData();

					BsAssert(dwData <= MISSION_TYPE_TYURRU);

					if(dwData == MISSION_TYPE_TYURRU){
						dwData = MISSION_TYPE_INPHYY;
					}else{
						dwData++;
					}
					m_pMenu->SetCharFocus(dwData);
					g_pSoundManager->PlaySystemSound( SB_COMMON,"SYS_CURSOR");
					break;
				}
			case MENU_INPUT_LEFT:
				{
					DWORD dwData = (BsUi::BsUiGetWindow(pMsgToken->hWnd))->GetItemData();

					BsAssert(dwData <= MISSION_TYPE_TYURRU);
					switch(dwData)
					{
					case MISSION_TYPE_INPHYY:	break;
					case MISSION_TYPE_ASPHARR:	m_pMenu->SetCharFocus(MISSION_TYPE_KLARRANN); break;
					case MISSION_TYPE_MYIFEE:	m_pMenu->SetCharFocus(MISSION_TYPE_DWINGVATT); break;
					case MISSION_TYPE_TYURRU:	m_pMenu->SetCharFocus(MISSION_TYPE_VIGKVAGK); break;
					}
				
					if(dwData != MISSION_TYPE_INPHYY){
						g_pSoundManager->PlaySystemSound( SB_COMMON,"SYS_CURSOR");
					}
					break;
				}
			case MENU_INPUT_RIGHT:
				{
					DWORD dwData = (BsUi::BsUiGetWindow(pMsgToken->hWnd))->GetItemData();

					BsAssert(dwData <= MISSION_TYPE_TYURRU);
					switch(dwData)
					{
					case MISSION_TYPE_INPHYY:	break;
					case MISSION_TYPE_ASPHARR:	m_pMenu->SetCharFocus(MISSION_TYPE_KLARRANN); break;
					case MISSION_TYPE_MYIFEE:	m_pMenu->SetCharFocus(MISSION_TYPE_DWINGVATT); break;
					case MISSION_TYPE_TYURRU:	m_pMenu->SetCharFocus(MISSION_TYPE_VIGKVAGK); break;
					}

					if(dwData != MISSION_TYPE_INPHYY){
						g_pSoundManager->PlaySystemSound( SB_COMMON,"SYS_CURSOR");
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
//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------
DWORD CFcMenuCharRightLayer::ProcMessage(xwMessageToken* pMsgToken)
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
					g_MenuHandle->PostMessage(fcMSG_LIBRARY_START);
					break;
				}
			case MENU_INPUT_A:
				{
					m_pMenu->ClickAItem(pMsgToken->hWnd);
					break;
				}
			case MENU_INPUT_UP:
				{
					DWORD dwData = (BsUi::BsUiGetWindow(pMsgToken->hWnd))->GetItemData();
					BsAssert(dwData >= MISSION_TYPE_KLARRANN && dwData <= MISSION_TYPE_VIGKVAGK);

					if(dwData == MISSION_TYPE_KLARRANN){
						dwData = MISSION_TYPE_VIGKVAGK;
					}else{
						dwData--;
					}
					m_pMenu->SetCharFocus(dwData);
					g_pSoundManager->PlaySystemSound( SB_COMMON,"SYS_CURSOR");
					break;
				}
			case MENU_INPUT_DOWN:
				{
					DWORD dwData = (BsUi::BsUiGetWindow(pMsgToken->hWnd))->GetItemData();
					BsAssert(dwData >= MISSION_TYPE_KLARRANN && dwData <= MISSION_TYPE_VIGKVAGK);

					if(dwData == MISSION_TYPE_VIGKVAGK){
						dwData = MISSION_TYPE_KLARRANN;
					}else{
						dwData++;
					}
					m_pMenu->SetCharFocus(dwData);
					g_pSoundManager->PlaySystemSound( SB_COMMON,"SYS_CURSOR");
					break;
				}
			case MENU_INPUT_LEFT:
				{
					DWORD dwData = (BsUi::BsUiGetWindow(pMsgToken->hWnd))->GetItemData();
					BsAssert(dwData >= MISSION_TYPE_KLARRANN && dwData <= MISSION_TYPE_VIGKVAGK);

					switch(dwData)
					{
					case MISSION_TYPE_KLARRANN:	m_pMenu->SetCharFocus(MISSION_TYPE_ASPHARR); break;
					case MISSION_TYPE_DWINGVATT:m_pMenu->SetCharFocus(MISSION_TYPE_MYIFEE); break;
					case MISSION_TYPE_VIGKVAGK:	m_pMenu->SetCharFocus(MISSION_TYPE_TYURRU); break;
					}

					g_pSoundManager->PlaySystemSound( SB_COMMON,"SYS_CURSOR");
					break;
				}
			case MENU_INPUT_RIGHT:
				{
					DWORD dwData = (BsUi::BsUiGetWindow(pMsgToken->hWnd))->GetItemData();
					BsAssert(dwData >= MISSION_TYPE_KLARRANN && dwData <= MISSION_TYPE_VIGKVAGK);

					switch(dwData)
					{
					case MISSION_TYPE_KLARRANN:	m_pMenu->SetCharFocus(MISSION_TYPE_ASPHARR); break;
					case MISSION_TYPE_DWINGVATT:m_pMenu->SetCharFocus(MISSION_TYPE_MYIFEE); break;
					case MISSION_TYPE_VIGKVAGK:	m_pMenu->SetCharFocus(MISSION_TYPE_TYURRU); break;
					}

					g_pSoundManager->PlaySystemSound( SB_COMMON,"SYS_CURSOR");
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




//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
CFcMenuCharStockSub::CFcMenuCharStockSub(_FC_MENU_TYPE nType, DWORD dwData)
: CFcMenuForm(nType)
{
	m_pBaseLayer = new BsUiLayer();
	AddUiLayer(m_pBaseLayer, SN_CharStockDB_Sub1, -1, -1);

	m_pLeftLayer = new BsUiLayer();
	AddUiLayer(m_pLeftLayer, SN_CharStockDB_Sub2, -1, -1);
	
	m_pRightLayer = new CFcMenuCharStockSubLayer(this);
	AddUiLayer(m_pRightLayer, SN_CharStockDB_Sub3, -1, -1);


	UpdateBaseLayer(dwData);
	UpdateLeftLayer(dwData);
	UpdateRightLayer(dwData);
	
	g_BsUiSystem.SetFocusWindow(m_pRightLayer);
}

void CFcMenuCharStockSub::UpdateBaseLayer(DWORD dwMissionType)
{
	m_dwMissionType = dwMissionType;

	int nPortraitID = PORTRAIT_Aspharr;
	switch(dwMissionType)
	{
	case MISSION_TYPE_INPHYY:	nPortraitID = PORTRAIT_Inphyy; break;
	case MISSION_TYPE_ASPHARR:	nPortraitID = PORTRAIT_Aspharr; break;
	case MISSION_TYPE_MYIFEE:	nPortraitID = PORTRAIT_Myifee; break;
	case MISSION_TYPE_TYURRU:	nPortraitID = PORTRAIT_Tyurru; break;
	case MISSION_TYPE_KLARRANN:	nPortraitID = PORTRAIT_Klarrann; break;
	case MISSION_TYPE_DWINGVATT:nPortraitID = PORTRAIT_Dwingvatt; break;
	case MISSION_TYPE_VIGKVAGK:	nPortraitID = PORTRAIT_VigkVagk; break;
	}

	UVImage* pUVInfo = g_InterfaceManager.GetPortrait()->GetUVInfoforTex(nPortraitID);

	ImageCtrlInfo ImgCtrlInfo;
	ImgCtrlInfo.nTexId = pUVInfo->nTexId;
	ImgCtrlInfo.u1 = pUVInfo->u1;	ImgCtrlInfo.v1 = pUVInfo->v1;
	ImgCtrlInfo.u2 = pUVInfo->u2;	ImgCtrlInfo.v2 = pUVInfo->v2;
	int nSizeX = (int)((pUVInfo->u2 - pUVInfo->u1) * 1.4f);
	int nSizeY = (int)((pUVInfo->v2 - pUVInfo->v1) * 1.4f);

	BsUiImageCtrl* pImgCtrl = (BsUiImageCtrl*)(m_pBaseLayer->GetWindow(IM_CharStockDB_Sub_Model));
	pImgCtrl->SetImageInfo(&ImgCtrlInfo);
	pImgCtrl->SetWindowSize(nSizeX, nSizeY);
	pImgCtrl->SetPortraitTexture(true);

	int nLibCharInfo = g_MissionToLibCharInfo[dwMissionType];
	CharLocalTable* pInfo = CLibCharLocalLoader::GetInstance().GetItemData(nLibCharInfo);
	m_pBaseLayer->GetWindow(TT_CharStockDB_Sub_CharEx)->SetItemText(pInfo->pCaption);
}

void CFcMenuCharStockSub::UpdateLeftLayer(DWORD dwMissionType)
{	
	int nLibCharInfo = g_MissionToLibCharInfo[dwMissionType];
	CharLocalTable* pInfo = CLibCharLocalLoader::GetInstance().GetItemData(nLibCharInfo);
	
	m_pLeftLayer->GetWindow(TT_CharStockDB_Sub_Name)->SetItemText(pInfo->cName);
}

void CFcMenuCharStockSub::UpdateRightLayer(DWORD dwMissionType)
{
	char cPoint[256];
	sprintf(cPoint, "%d", g_FCGameData.tempFixedSave.nTotalPoint);
	BsUiWindow* pWindow = m_pRightLayer->GetWindow(TT_CharStockDB_Sub_point_num);
	pWindow->SetItemText(cPoint);

	int nLibCharInfo = g_MissionToLibCharInfo[dwMissionType];
	CharLocalTable* pInfo = CLibCharLocalLoader::GetInstance().GetItemData(nLibCharInfo);
	
	m_pRightLayer->GetWindow(TT_CharStockDB_Sub_CharAge)->SetItemText(pInfo->cAge);
	m_pRightLayer->GetWindow(TT_CharStockDB_Sub_CharPart)->SetItemText(pInfo->cPosition);
	m_pRightLayer->GetWindow(TT_CharStockDB_Sub_CharWeapon)->SetItemText(pInfo->cWeapon);
	m_pRightLayer->GetWindow(TT_CharStockDB_Sub_CharOrbspark)->SetItemText(pInfo->cOrbSpark);
}

void CFcMenuCharStockSub::RenderProcess()
{
	switch(GetStatus())
	{
	case _MS_OPEN:
		{
			m_pBaseLayer->SetColor(D3DXCOLOR(1, 1, 1, 0.f));
			m_pLeftLayer->SetColor(D3DXCOLOR(1, 1, 1, 0.f));
			m_pRightLayer->SetColor(D3DXCOLOR(1, 1, 1, 0.f));

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

#define _START_POS_X_CHAR_SUB1				-540
#define _END_POS_X_CHAR_SUB1				0
#define _START_POS_X_CHAR_SUB2				1280
#define _END_POS_X_CHAR_SUB2				500
void CFcMenuCharStockSub::RenderMoveLayer(int nTick)
{
	//------------------------------------------------------
	if(nTick <= _TICK_FADE_IN_BASE)
	{
		float fAlpha = (float)nTick / _TICK_FADE_IN_BASE;
		m_pBaseLayer->SetColor(D3DXCOLOR(1, 1, 1, fAlpha));
		m_pLeftLayer->SetColor(D3DXCOLOR(1, 1, 1, fAlpha));
		m_pRightLayer->SetColor(D3DXCOLOR(1, 1, 1, fAlpha));
	}

	//------------------------------------------------------
	if(nTick <= _TICK_IN_FORM_LEFT)
	{	
		float frate = 1.f - ((float)nTick / _TICK_IN_FORM_LEFT);
		int nX = (int)(_START_POS_X_CHAR_SUB1 * frate);
		m_pLeftLayer->SetWindowPosX(nX);

		float fAlpha = 1.f - frate;
		m_pLeftLayer->SetColor(D3DXCOLOR(1, 1, 1, fAlpha));
	}

	//------------------------------------------------------
	//point
	if(nTick <= _TICK_IN_FORM_RIGHT)
	{
		float frate = (float)nTick / _TICK_IN_FORM_RIGHT;
		int nX = _END_POS_X_CHAR_SUB2 + int((_START_POS_X_CHAR_SUB2 - _END_POS_X_CHAR_SUB2) * (1.f - frate));
		m_pRightLayer->SetWindowPosX(nX);

		float fAlpha = frate;
		m_pRightLayer->SetColor(D3DXCOLOR(1, 1, 1, fAlpha));
	}
}

//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------
DWORD CFcMenuCharStockSubLayer::ProcMessage(xwMessageToken* pMsgToken)
{
	BsAssert(pMsgToken);
	switch(pMsgToken->message)
	{
	case XWMSG_KEYDOWN:
		{
			if(pMsgToken->lParam == MENU_INPUT_B)
			{
				m_pMenu->SetStatus(_MS_CLOSING);
				g_MenuHandle->PostMessage(fcMSG_LIB_CHAR_START, m_pMenu->GetMissionType());
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


