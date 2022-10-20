#include "stdafx.h"

#include "FcMenuArtWork.h"
#include "BSuiSystem.h"

#include "BsUiClassIDManager.h"
#include "BsUiFontAttrManager.h"

#include "FcInterfaceManager.h"

#include "FcMessageDef.h"
#include "FcSoundManager.h"
#include "FcGlobal.h"
#include "TextTable.h"

#include "FantasyCore.h"
#include "FcLiveManager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG

#define _MAX_IMG_COUNT			60

//--------------------------------------------------------------------
//--------------------------------------------------------------------
CFcMenuArtWork::CFcMenuArtWork(_FC_MENU_TYPE nType)
: CFcMenuForm(nType)
{
	m_hClassCurLayer = SN_illust2;

	m_pViewerLayer = new CFcMenuArtViewerLayer(this);
	AddUiLayer(m_pViewerLayer, SN_Illustviewer, -1, -1);
	m_pViewerLayer->SetWindowAttr(XWATTR_SHOWWINDOW, false);

	m_pBaseLayer = new BsUiLayer();
	AddUiLayer(m_pBaseLayer, SN_illust0, -1, -1);

	m_pPointLayer = new BsUiLayer();
	AddUiLayer(m_pPointLayer, SN_illust1, -1, -1);

	m_pArtWorkLayer = new CFcMenuArtWorkLayer(this);
	AddUiLayer(m_pArtWorkLayer, SN_illust2, -1, -1);
	m_pArtWorkLayer->Initialize();

	
	BsUiWindow* pWindow = m_pArtWorkLayer->GetWindow(BT_illust_00);
	BsAssert(pWindow);
	UpdateNeedPoint(pWindow->GetItemData());

	UpdatePointLayer();
}


void CFcMenuArtWork::ProcessMsg(int nMsgAsk, BsUiHANDLE hWnd, int nMsgRsp)
{
	if(nMsgAsk == fcMT_LibOpenItem)
	{
		if(nMsgRsp == ID_YES)
		{
			BsUiWindow* pWindow = BsUi::BsUiGetWindow(hWnd);
			DWORD dwItemID = pWindow->GetItemData();

			CFcMenuLibraryManager* pLibMgr = g_MenuHandle->GetLibManager();
			LibraryTable* pItem = pLibMgr->GetItemData(dwItemID);

			g_FCGameData.tempFixedSave.nTotalPoint -= pItem->nPoint;
			LibraryItem* pLibItem = g_FCGameData.tempFixedSave.GetLibraryData(dwItemID);
			pLibItem->nState = LIS_OPEN;

			if(g_FC.GetLiveManager()->IsUserSignedIn() && g_FCGameData.m_DeviceID != XCONTENTDEVICE_ANY ){
				g_FCGameData.SaveFixedGameData();
			}

			pWindow->SetItemText("");
			UpdatePointLayer();

			ChangePage(SN_Illustviewer, dwItemID);
		}
		else{
			g_BsUiSystem.SetFocusWindow(BsUi::BsUiGetWindow(hWnd));
		}
	}
	else if(nMsgAsk == fcMT_LibOpenItemInWarning){
		g_BsUiSystem.SetFocusWindow(BsUi::BsUiGetWindow(hWnd));
	}
}


void CFcMenuArtWork::RenderProcess()
{
	switch(GetStatus())
	{
	case _MS_OPEN:
		{
			m_pBaseLayer->SetColor(D3DXCOLOR(1, 1, 1, 0.f));
			m_pPointLayer->SetColor(D3DXCOLOR(1, 1, 1, 0.f));
			m_pArtWorkLayer->SetColor(D3DXCOLOR(1, 1, 1, 0.f));
			m_pViewerLayer->SetColor(D3DXCOLOR(1, 1, 1, 0.f));

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

#define _START_POS_X_ART1				-640
#define _END_POS_X_ART1					0
#define _START_POS_X_ART2				1280
#define _END_POS_X_ART2					540
void CFcMenuArtWork::RenderMoveLayer(int nTick)
{
	//------------------------------------------------------
	//base
	if(nTick <= _TICK_FADE_IN_BASE)
	{
		float fAlpha = (float)nTick / _TICK_FADE_IN_BASE;
		m_pBaseLayer->SetColor(D3DXCOLOR(1, 1, 1, fAlpha));
		m_pArtWorkLayer->SetColor(D3DXCOLOR(1, 1, 1, fAlpha));
		m_pViewerLayer->SetColor(D3DXCOLOR(1, 1, 1, fAlpha));
	}

	//------------------------------------------------------
	//point
	if(nTick <= _TICK_IN_FORM_RIGHT)
	{
		float frate = (float)nTick / _TICK_IN_FORM_RIGHT;
		int nX = _END_POS_X_ART2 + int((_START_POS_X_ART2 - _END_POS_X_ART2) * (1.f - frate));
		m_pPointLayer->SetWindowPosX(nX);

		float fAlpha = frate;
		m_pPointLayer->SetColor(D3DXCOLOR(1, 1, 1, fAlpha));
	}
}

void CFcMenuArtWork::ChangePage(BsUiCLASS hClass, int nItemID)
{
	switch(hClass)
	{
	case SN_Illustviewer:
		{
			m_pArtWorkLayer->SetWindowAttr(XWATTR_SHOWWINDOW, false);
			m_pViewerLayer->SetWindowAttr(XWATTR_SHOWWINDOW, true);

			m_hClassCurLayer = SN_Illustviewer;
			m_pViewerLayer->UpdateViewer(nItemID);
			g_BsUiSystem.SetFocusWindow(m_pViewerLayer);
			break;
		}
	case SN_illust2:
		{
			m_pArtWorkLayer->SetWindowAttr(XWATTR_SHOWWINDOW, true);
			m_pViewerLayer->SetWindowAttr(XWATTR_SHOWWINDOW, false);

			m_hClassCurLayer = SN_illust2;
			g_BsUiSystem.SetFocusWindow(m_pArtWorkLayer);
			break;
		}
	}
}

void CFcMenuArtWork::UpdatePointLayer()
{
	char cPoint[256];
	sprintf(cPoint, "%d", g_FCGameData.tempFixedSave.nTotalPoint);
	m_pPointLayer->GetWindow(TT_illust_num)->SetItemText(cPoint);
}

void CFcMenuArtWork::UpdateNeedPoint(DWORD dwItemID)
{
	if(dwItemID == _MAX_IMG_COUNT){
		return;
	}
	
	CFcMenuLibraryManager* pLibMgr = g_MenuHandle->GetLibManager();
	LibraryTable* pItem = pLibMgr->GetItemData(dwItemID);

	BsUiWindow* pWindow = m_pArtWorkLayer->GetWindow(TT_illust_point_caution_num);
	pWindow->SetWindowAttr(XWATTR_SHOWWINDOW, true);

	int nNeedPoint(0);
	switch(pLibMgr->GetState(dwItemID))
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
#define _ARTWORW_ILL_Row		3
#define _ARTWORW_ILL_Column		7
void CFcMenuArtWorkLayer::Initialize()
{
	DWORD dwClass = BT_illust_00;
	BsUiWindow* pWindow = GetWindow(dwClass);
	BsAssert(pWindow);

	g_BsUiSystem.SetFocusWindow(pWindow);

	m_nCurPage = 0;
	UpdateImageInfo();
}

DWORD CFcMenuArtWorkLayer::ProcMessage(xwMessageToken* pMsgToken)
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
					m_pMenu->SetStatus(_MS_CLOSING);
					g_MenuHandle->PostMessage(fcMSG_LIBRARY_START);
					break;
				}
			case MENU_INPUT_A:
				{
					BsUiWindow* pWindow = g_BsUiSystem.GetFocusWindow();
					BsUiCLASS hClass = pWindow->GetClass();
					BsAssert(hClass >= BT_illust_00);
					BsAssert(hClass <= BT_illust_20);

					ClickAItem(pWindow);
					break;
				}
			case MENU_INPUT_UP:		OnKeyDown_UP(pMsgToken); break;
			case MENU_INPUT_DOWN:	OnKeyDown_DOWN(pMsgToken); break;
			case MENU_INPUT_LEFT:	OnKeyDown_LEFT(pMsgToken); break;
			case MENU_INPUT_RIGHT:	OnKeyDown_RIGHT(pMsgToken); break;
			case MENU_INPUT_BLACK:	OnKeyDown_SHOULDER_LEFT(pMsgToken); break;
			case MENU_INPUT_WHITE:	OnKeyDown_SHOULDER_RIGHT(pMsgToken); break;
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


void CFcMenuArtWorkLayer::ClickAItem(BsUiWindow* pWindow)
{	
	CFcMenuLibraryManager* pLibMgr = g_MenuHandle->GetLibManager();
	int nItemID = (int)(pWindow->GetItemData());
	if(nItemID == _MAX_IMG_COUNT)
	{
		g_pSoundManager->PlaySystemSound( SB_COMMON,"SYS_CANCEL");
		return;
	}

	LibraryTable* pItem = pLibMgr->GetItemData(nItemID);
	if(pItem == NULL)
	{	
		g_pSoundManager->PlaySystemSound( SB_COMMON,"SYS_CANCEL");
		return;
	}
	
	switch(pLibMgr->GetState(nItemID))
	{
	case LIS_DEFAULT:
		{
			g_pSoundManager->PlaySystemSound( SB_COMMON,"SYS_CANCEL");
			break;
		}
	case LIS_NEW:
		{
			if(pItem->nPoint == 0)
			{
				g_pSoundManager->PlaySystemSound( SB_COMMON,"SYS_OK");
				LibraryItem* pLibItem = g_FCGameData.tempFixedSave.GetLibraryData(nItemID);
				pLibItem->nState = LIS_OPEN;

				pWindow->SetItemText("");
				m_pMenu->ChangePage(SN_Illustviewer, nItemID);
			}
			else if(g_FCGameData.tempFixedSave.nTotalPoint >= pItem->nPoint)
			{
				g_MenuHandle->PostMessage(fcMSG_MESSAGEBOX_START,
					fcMT_LibOpenItem,
					(DWORD)m_pMenu,
					pWindow->GetHWnd(),
					NULL, (DWORD)(pItem->nPoint));
			}else{
				g_MenuHandle->PostMessage(fcMSG_MESSAGEBOX_START,
					fcMT_LibOpenItemInWarning,
					(DWORD)m_pMenu, pWindow->GetHWnd());
			}
			break;
		}
	case LIS_OPEN:
		{
			g_pSoundManager->PlaySystemSound( SB_COMMON,"SYS_OK");
			m_pMenu->ChangePage(SN_Illustviewer, nItemID);
			break;
		}
	}
}

void CFcMenuArtWorkLayer::OnKeyDown_UP(xwMessageToken* pMsgToken)
{
	BsUiWindow* pWindow = BsUi::BsUiGetWindow(pMsgToken->hWnd);
	BsUiCLASS hClass = pWindow->GetClass();
	if(hClass - _ARTWORW_ILL_Column < BT_illust_00){
		hClass = hClass + (_ARTWORW_ILL_Column * (_ARTWORW_ILL_Row - 1));
	}
	else{
		hClass -= _ARTWORW_ILL_Column;
	}

	BsAssert(hClass >= BT_illust_00 && hClass <= BT_illust_20);
	g_BsUiSystem.SetFocusWindow(GetWindow(hClass));
	g_pSoundManager->PlaySystemSound( SB_COMMON,"SYS_CURSOR");

	m_pMenu->UpdateNeedPoint(GetWindow(hClass)->GetItemData());
}

void CFcMenuArtWorkLayer::OnKeyDown_DOWN(xwMessageToken* pMsgToken)
{
	BsUiWindow* pWindow = BsUi::BsUiGetWindow(pMsgToken->hWnd);
	BsUiCLASS hClass = pWindow->GetClass();
	if(hClass + _ARTWORW_ILL_Column > BT_illust_20){
		hClass =hClass - (_ARTWORW_ILL_Column * (_ARTWORW_ILL_Row - 1));
	}
	else{
		hClass += _ARTWORW_ILL_Column;
	}

	BsAssert(hClass >= BT_illust_00 && hClass <= BT_illust_20);
	g_BsUiSystem.SetFocusWindow(GetWindow(hClass));
	g_pSoundManager->PlaySystemSound( SB_COMMON,"SYS_CURSOR");

	m_pMenu->UpdateNeedPoint(GetWindow(hClass)->GetItemData());
}

void CFcMenuArtWorkLayer::OnKeyDown_LEFT(xwMessageToken* pMsgToken)
{
	BsUiWindow* pWindow = BsUi::BsUiGetWindow(pMsgToken->hWnd);
	BsUiCLASS hClass = pWindow->GetClass();
	DWORD dwIndex = hClass - BT_illust_00; 
	int nRow = dwIndex / _ARTWORW_ILL_Column;
	if(nRow == (dwIndex - 1) / _ARTWORW_ILL_Column){
		hClass--;
	}
	else{
		hClass = BT_illust_00 + ((nRow + 1) * _ARTWORW_ILL_Column) - 1;
	}

	BsAssert(hClass >= BT_illust_00 && hClass <= BT_illust_20);
	g_BsUiSystem.SetFocusWindow(GetWindow(hClass));
	g_pSoundManager->PlaySystemSound( SB_COMMON,"SYS_CURSOR");

	m_pMenu->UpdateNeedPoint(GetWindow(hClass)->GetItemData());
}


void CFcMenuArtWorkLayer::OnKeyDown_RIGHT(xwMessageToken* pMsgToken)
{
	BsUiWindow* pWindow = BsUi::BsUiGetWindow(pMsgToken->hWnd);
	BsUiCLASS hClass = pWindow->GetClass();
	DWORD dwIndex = hClass - BT_illust_00; 
	int nRow = dwIndex / _ARTWORW_ILL_Column;
	if(nRow == (dwIndex + 1) / _ARTWORW_ILL_Column){
		hClass++;
	}
	else{
		hClass = BT_illust_00 + (nRow * _ARTWORW_ILL_Column);
	}

	BsAssert(hClass >= BT_illust_00 && hClass <= BT_illust_20);
	g_BsUiSystem.SetFocusWindow(GetWindow(hClass));
	g_pSoundManager->PlaySystemSound( SB_COMMON,"SYS_CURSOR");

	m_pMenu->UpdateNeedPoint(GetWindow(hClass)->GetItemData());
}

void CFcMenuArtWorkLayer::OnKeyDown_SHOULDER_LEFT(xwMessageToken* pMsgToken)
{
	if(m_nCurPage > 0)
	{
		m_nCurPage--;
		UpdateImageInfo();

		g_pSoundManager->PlaySystemSound( SB_COMMON,"SYS_CURSOR");

		BsUiWindow* pWindow = BsUi::BsUiGetWindow(pMsgToken->hWnd);
		m_pMenu->UpdateNeedPoint(pWindow->GetItemData());
	}
}

void CFcMenuArtWorkLayer::OnKeyDown_SHOULDER_RIGHT(xwMessageToken* pMsgToken)
{	
	int nMaxSlot = _ARTWORW_ILL_Row * _ARTWORW_ILL_Column;
	int nPageCount = m_nMaxItamCount / nMaxSlot;
	if(m_nMaxItamCount % nMaxSlot > 0){
		nPageCount += 1;
	}

	if(m_nCurPage+1 < nPageCount){
		m_nCurPage++;
		UpdateImageInfo();

		g_pSoundManager->PlaySystemSound( SB_COMMON,"SYS_CURSOR");

		BsUiWindow* pWindow = BsUi::BsUiGetWindow(pMsgToken->hWnd);
		m_pMenu->UpdateNeedPoint(pWindow->GetItemData());
	}
	
}


void CFcMenuArtWorkLayer::UpdateImageInfo()
{
	int nMaxSlot = _ARTWORW_ILL_Row * _ARTWORW_ILL_Column;
	m_nMaxItamCount = _MAX_IMG_COUNT;
	int nPageCount = m_nMaxItamCount / nMaxSlot;
	if(m_nMaxItamCount % nMaxSlot > 0){
		nPageCount += 1;
	}

	int nSlotCount = nMaxSlot;
	if(nPageCount == m_nCurPage + 1){
		nSlotCount = m_nMaxItamCount % nMaxSlot - 1;
	}

	char cText[64];
	sprintf(cText, "%d/%d", m_nCurPage+1, nPageCount);
	GetWindow(TT_illust_imgName)->SetItemText(cText);

	bool bLeftShow = true;
	if(m_nCurPage == 0){
		bLeftShow = false;
	}
	GetWindow(IM_illustList_left)->SetWindowAttr(XWATTR_SHOWWINDOW, bLeftShow);

	bool bRightShow = true;
	if(nPageCount == m_nCurPage+1){
		bRightShow = false;
	}
	GetWindow(IM_illustList_right)->SetWindowAttr(XWATTR_SHOWWINDOW, bRightShow);

	CFcMenuLibraryManager* pLibMgr = g_MenuHandle->GetLibManager();
	
	for(int i=0; i< nMaxSlot; i++)
	{
		DWORD hIMClass = IM_illust_00 + i;
		DWORD hBTClass = BT_illust_00 + i;
		if(i >= nSlotCount)
		{	
			GetWindow(hIMClass)->SetWindowAttr(XWATTR_SHOWWINDOW, false);
			GetWindow(hBTClass)->SetItemData(_MAX_IMG_COUNT);
			GetWindow(hBTClass)->SetItemText("");
			continue;
		}

		
		int nIndex = nMaxSlot * m_nCurPage + i;
		int nItemID = pLibMgr->GetItemID(LIB_ART, nIndex);
		LibraryTable* pItem = pLibMgr->GetItemData(nItemID);
		BsAssert(pItem);

		//-----------------------------
		//button
		GetWindow(hBTClass)->SetItemData(nItemID);
		switch(pLibMgr->GetState(nItemID))
		{
		case LIS_DEFAULT:
		case LIS_OPEN:
			{
				GetWindow(hBTClass)->SetItemText("");
				break;
			}
		case LIS_NEW:
			{
				GetWindow(hBTClass)->SetItemText("@(New_icon)");
				break;
			}
		}


		//-----------------------------
		//image
		int nTexId = -1;
		switch(pLibMgr->GetState(nItemID))
		{
		case LIS_DEFAULT:
			{
				g_BsKernel.chdir("interface");
				nTexId = g_BsKernel.LoadTexture("MN_img_disable.dds");
				g_BsKernel.chdir("..");
				break;
			}
		case LIS_NEW:
		case LIS_OPEN:
			{	
				g_BsKernel.chdir("interface");
				g_BsKernel.chdir("Artwork");
				nTexId = g_BsKernel.LoadTexture(pItem->cIconFileName);
				g_BsKernel.chdir("..");
				g_BsKernel.chdir("..");
			}
		}
		
		BsAssert(nTexId != -1);
		SIZE size = g_BsKernel.GetTextureSize(nTexId);

		ImageCtrlInfo ImgCtrlInfo;
		ImgCtrlInfo.nTexId = nTexId;
		ImgCtrlInfo.u2 = size.cx;
		ImgCtrlInfo.v2 = size.cy;
		
		BsUiImageCtrl* pImgCtrl = (BsUiImageCtrl*)GetWindow(hIMClass);
		pImgCtrl->SetImageInfo(&ImgCtrlInfo);
		GetWindow(hIMClass)->SetWindowAttr(XWATTR_SHOWWINDOW, true);
	}
}

//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------
DWORD CFcMenuArtViewerLayer::ProcMessage(xwMessageToken* pMsgToken)
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
					g_pSoundManager->PlaySystemSound( SB_COMMON,"SYS_OK");
					m_pMenu->ChangePage(SN_illust2);
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

void CFcMenuArtViewerLayer::UpdateViewer(int nItemID)
{
	m_nItemID = nItemID;

	CFcMenuLibraryManager* pLibMgr = g_MenuHandle->GetLibManager();
	LibraryTable* pItem = pLibMgr->GetItemData(nItemID);

	g_BsKernel.chdir("interface");
	g_BsKernel.chdir("Artwork");
	int nTexId = g_BsKernel.LoadTexture(pItem->cImageFileName);
	g_BsKernel.chdir("..");
	g_BsKernel.chdir("..");

	BsAssert(nTexId != -1);
	SIZE size = g_BsKernel.GetTextureSize(nTexId);

	ImageCtrlInfo ImgCtrlInfo;
	ImgCtrlInfo.nTexId = nTexId;
	ImgCtrlInfo.u2 = size.cx;
	ImgCtrlInfo.v2 = size.cy;

	BsUiImageCtrl* pImgCtrl = (BsUiImageCtrl*)GetWindow(IM_Illustviewer_img);
	pImgCtrl->SetImageInfo(&ImgCtrlInfo);
}