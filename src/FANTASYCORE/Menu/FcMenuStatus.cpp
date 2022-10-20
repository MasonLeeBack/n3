#include "stdafx.h"

#include "FcMenuStatus.h"
#include "BSuiSystem.h"

#include "BsUiClassIDManager.h"
#include "BsUiFontAttrManager.h"
#include "FcInterfaceManager.h"
#include "FcMessageDef.h"
#include "FcWorld.h"
#include "FcHeroObject.h"
#include "Fcitem.h"
#include "LocalLanguage.h"

#include "BsUiUVManager.h"
#include ".\\data\\Menu\\BsUiUVID.h"
#include "TextTable.h"
#include "FcGlobal.h"
#include "FcSoundManager.h"

//#include ".\\data\\RequiredExp.h"
extern int g_nRequiredExp[9];

#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG

enum _STATUS_CHANGE_PAGE
{
	_NONE_PAGE = -1,
	_STATUS_PAGE,
	_WEAPON_PAGE,
	_ACCESSORY_PAGE,
};



CFcMenuStatus::CFcMenuStatus(_FC_MENU_TYPE nType)
: CFcMenuForm(nType)
{
	int nLevel = g_FcWorld.GetHeroHandle(0)->GetLevel();
	if(nLevel <= 0){
		nLevel = 0;
	}
	m_nAccessorySlotCount = g_AccessorySlotCountByLevel[nLevel];
	BsAssert(m_nAccessorySlotCount < _ITEM_ICON_MAX_COUNT);

	UpdateItemSlot();

	m_nCurPage = _NONE_PAGE;
	m_nCurSlot = 0;
	int nHeroID = g_FcWorld.GetHeroHandle()->GetUnitSoxIndex();
	if(nHeroID == CHAR_ID_VIGKVAGK){
		m_nCurSlot = 1;
	}

	m_pBaseLayer = NULL;
	m_pHeadLineLayer = NULL;
	m_pCharLayer = NULL;
	m_pCompareLayer = NULL;
	m_pInfoLayer = NULL;
	m_pNameLayer = NULL;
	m_pGaugeLayer = NULL;
	m_pStateLayer = NULL;
	m_pListLayer = NULL;
	m_pTabLayer = NULL;

	CreateObject();
	CreateLayer();	
	
	UpdateHeroNameLayer();
	UpdateStatusPage(_STATUS_PAGE);

	m_nTick = g_BsUiSystem.GetMenuTick();
}

CFcMenuStatus::~CFcMenuStatus()
{	
	CloseObject();
}

#define _CAMERA_POS			D3DXVECTOR3(0, 80.f, -450.f)
#define _CAMERA_TARGET_POS	D3DXVECTOR3(0, 60.f, 0.f)


static int g_nSoxIndex = 0;
static float g_fScale = 1.f;
static float g_fcx = 0.f;
static float g_fcy = 80.f;
static float g_fcz = -250.f;

static float g_fctx = 0;
static float g_fcty = 80.f;
static float g_fctz = 0.f;

static int g_nAngle = 512;
static int g_aniID = 0;

//#define _TEXT_CHAR_POS

void CFcMenuStatus::CreateObject()
{
	CFcMenu3DObjManager* p3DObjMgr = g_MenuHandle->Get3DObjectMgr();

	if(p3DObjMgr == NULL){
		return;
	}

	int nAccouteredWeapon(-1), nItemSoxID(-1), nSkinIndex(-1);
	nAccouteredWeapon = m_ItemSlot[0];
	if(nAccouteredWeapon >= 0)
	{
		nItemSoxID = g_FcItemManager.GetHeroEquip(nAccouteredWeapon)->nItemSoxID;
		if(nItemSoxID){
			nSkinIndex = g_FcItemManager.GetSkinIndex(nItemSoxID);
		}
	}

#ifndef _TEXT_CHAR_POS

	CCrossVector* pCameraVector = p3DObjMgr->GetCameraCrossVector();
	int nSoxIndex = g_FcWorld.GetHeroHandle(0)->GetUnitSoxIndex();
	
	D3DXVECTOR3 vCamPos = D3DXVECTOR3 (g_CharStandInfo[nSoxIndex].fcx, g_CharStandInfo[nSoxIndex].fcy, g_CharStandInfo[nSoxIndex].fcz);
	pCameraVector->SetPosition(vCamPos);
	
	D3DXVECTOR3 vCamTargetPos = D3DXVECTOR3 (g_CharStandInfo[nSoxIndex].fctx, g_CharStandInfo[nSoxIndex].fcty, g_CharStandInfo[nSoxIndex].fctz);
	p3DObjMgr->SetCameraTargetPos(vCamTargetPos);

	D3DXVECTOR3 pos = D3DXVECTOR3(0.f, 0.f, 0.f);
	int nWeapon(0), nWeaponCount(1);
	if(nSoxIndex == CHAR_ID_VIGKVAGK){
		nWeapon = -1;
		nWeaponCount = 0;
	}
	else if(nSoxIndex == CHAR_ID_DWINGVATT){
		nWeaponCount = 2;
	}

	m_h3DObject = p3DObjMgr->AddObject(nSoxIndex,
		g_CharStandInfo[nSoxIndex].aniID,
		pos,
		g_CharStandInfo[nSoxIndex].fScale,
		nWeapon,
		nWeaponCount);
	p3DObjMgr->ChangeWeapon(m_h3DObject, nSkinIndex);

	CCrossVector* pObjCross = p3DObjMgr->GetObjCrossVector(m_h3DObject);
	BsAssert(pObjCross && "Unable to get ObjectCrossVector"); //aleksger: prefix bug 803: GetObjCrossVector() may return NULL.
	pObjCross->RotateYaw(g_CharStandInfo[nSoxIndex].Angle);

#else

	CCrossVector* pCameraVector = p3DObjMgr->GetCameraCrossVector();
	pCameraVector->SetPosition(D3DXVECTOR3(g_fcx, g_fcy, g_fcz));
	p3DObjMgr->SetCameraTargetPos(D3DXVECTOR3(g_fctx, g_fcty, g_fctz));
		
	D3DXVECTOR3 pos = D3DXVECTOR3(0.f, 0.f, 0.f);
	int nWeapon = 0;
	if(g_nSoxIndex == CHAR_ID_VIGKVAGK){
		nWeapon = -1
	}
	m_h3DObject = p3DObjMgr->AddObject(g_nSoxIndex, g_aniID, pos, g_fScale, nWeapon);
	p3DObjMgr->ChangeWeapon(m_h3DObject, nSkinIndex);

	CCrossVector* pObjCross = p3DObjMgr->GetObjCrossVector(m_h3DObject);
	BsAssert(pObjCross && "Unable to get ObjectCrossVector"); //aleksger: prefix bug 803: GetObjCrossVector() may return NULL.
	pObjCross->RotateYaw(g_nAngle);
#endif //_TEXT_CHAR_POS

}

void CFcMenuStatus::CloseObject()
{
	CFcMenu3DObjManager* p3DObjMgr = g_MenuHandle->Get3DObjectMgr();
	if(p3DObjMgr != NULL){
		p3DObjMgr->RemoveAllObject();
	}
}

void CFcMenuStatus::CreateLayer()
{
	//StatusBase
	m_pBaseLayer = new BsUiLayer();
	AddUiLayer(m_pBaseLayer, SN_Status0, -1, -1);

	//StatusHeadLine
	m_pHeadLineLayer = new BsUiLayer();
	AddUiLayer(m_pHeadLineLayer, SN_Status1, -1, -1);

	//Status3DImg
	m_pCharLayer = new BsUiLayer();
	AddUiLayer(m_pCharLayer, SN_Status2, -1, -1);

	//StatusItemImg
	m_pCompareLayer = new BsUiLayer();
	AddUiLayer(m_pCompareLayer, SN_Status3, -1, -1);
	m_pCompareLayer->SetWindowAttr(XWATTR_SHOWWINDOW, false);

	//StatusIteminfo
	m_pInfoLayer = new BsUiLayer();
	AddUiLayer(m_pInfoLayer, SN_Status4, -1, -1);

	//StatusHeroname
	m_pNameLayer = new BsUiLayer();
	AddUiLayer(m_pNameLayer, SN_Status5_1, -1, -1);

	//StatusGauge
	m_pGaugeLayer = new BsUiLayer();
	AddUiLayer(m_pGaugeLayer, SN_Status5_2, -1, -1);

	//Status account item
	m_pStateLayer = new BsUiLayer();
	AddUiLayer(m_pStateLayer, SN_Status6, -1, -1);
	
	//Status Item List
	m_pListLayer = new CFcMenuStatusItemListLayer(this);
	AddUiLayer(m_pListLayer, SN_Status7, -1, -1);
	m_pListLayer->SetWindowAttr(XWATTR_SHOWWINDOW, false);

	m_pTabLayer = new CFcMenuStatusItemTabLayer(this);
	AddUiLayer(m_pTabLayer, SN_Status8, -1, -1);
}


void CFcMenuStatus::RenderProcess()
{
	switch(GetStatus())
	{
	case _MS_OPEN:
		{
			m_pBaseLayer->SetColor(D3DXCOLOR(1, 1, 1, 0.f));
			m_pHeadLineLayer->SetColor(D3DXCOLOR(1, 1, 1, 0.f));
			m_pCharLayer->SetColor(D3DXCOLOR(1, 1, 1, 0.f));

			m_pNameLayer->SetColor(D3DXCOLOR(1, 1, 1, 0.f));
			m_pGaugeLayer->SetColor(D3DXCOLOR(1, 1, 1, 0.f));

			m_pInfoLayer->SetColor(D3DXCOLOR(1, 1, 1, 0.f));
			m_pStateLayer->SetColor(D3DXCOLOR(1, 1, 1, 0.f));
			m_pTabLayer->SetColor(D3DXCOLOR(1, 1, 1, 0.f));

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

			if(nTick >= _TICK_END_OPENING_CLOSING)
			{
				SetStatus(_MS_CLOSE);
				switch(g_MenuHandle->GetGamePauseMenu())
				{
				case _FC_MENU_PAUSE:	g_MenuHandle->PostMessage(fcMSG_PAUSE_START); break;
				case _FC_MENU_STATUS:
				default:				g_MenuHandle->PostMessage(fcMSG_STATUS_END);
				}
			}
			break;
		}
	case _MS_CLOSE:	break;
	}
}


#define _START_POS_X_STS1				-540
#define _END_POS_X_STS1					0
#define _START_POS_X_STS2				520
#define _END_POS_X_STS2					0
void CFcMenuStatus::RenderMoveLayer(int nTick)
{	
	//------------------------------------------------------
	//m_pBaseLayer, m_pHeadLineLayer, m_pCharLayer
	if(nTick <= _TICK_FADE_IN_BASE)
	{
		float fAlpha = (float)nTick / _TICK_FADE_IN_BASE;
		m_pBaseLayer->SetColor(D3DXCOLOR(1, 1, 1, fAlpha));
		m_pHeadLineLayer->SetColor(D3DXCOLOR(1, 1, 1, fAlpha));
		m_pCharLayer->SetColor(D3DXCOLOR(1, 1, 1, fAlpha));
	}

	//------------------------------------------------------
	//m_pNameLayer, m_pGaugeLayer
	if(nTick <= _TICK_IN_FORM_LEFT)
	{	
		float frate = 1.f - ((float)nTick / _TICK_IN_FORM_LEFT);
		int nX = (int)(_START_POS_X_STS1 * frate);
		m_pNameLayer->SetWindowPosX(nX);
		m_pGaugeLayer->SetWindowPosX(nX);

		float fAlpha = 1.f - frate;
		m_pNameLayer->SetColor(D3DXCOLOR(1, 1, 1, fAlpha));
		m_pGaugeLayer->SetColor(D3DXCOLOR(1, 1, 1, fAlpha));
	}

	//------------------------------------------------------
	//m_pInfoLayer, m_pStateLayer, m_pTabLayer
	if(nTick <= _TICK_IN_FORM_RIGHT)
	{
		float frate = (float)nTick / _TICK_IN_FORM_RIGHT;
		int nX = _END_POS_X_STS2 + int((_START_POS_X_STS2 - _END_POS_X_STS2) * (1.f - frate));
		m_pInfoLayer->SetWindowPosX(nX);
		m_pStateLayer->SetWindowPosX(nX);
		m_pTabLayer->SetWindowPosX(nX);

		float fAlpha = frate;
		m_pInfoLayer->SetColor(D3DXCOLOR(1, 1, 1, fAlpha));
		m_pStateLayer->SetColor(D3DXCOLOR(1, 1, 1, fAlpha));
		m_pTabLayer->SetColor(D3DXCOLOR(1, 1, 1, fAlpha));
	}
}



void CFcMenuStatus::UpdateItemSlot()
{
	memset(m_ItemSlot, -1, sizeof(int) * _ITEM_ICON_MAX_COUNT);

	int nWeapon(0), nAcce(0);
	for(int i=0; i<g_FcItemManager.GetItemCount(); i++)
	{
		ItemDataTable* pItem = g_FcItemManager.GetItemData(i);
		BsAssert(pItem != NULL);
		BsAssert(g_FcItemManager.GetItemName(i) != NULL);

		if(g_FcItemManager.IsAccoutered(i) == false){
			continue;
		}

		switch(pItem->nItemType)
		{
		case ITP_WEAPON:
			{
				BsAssert(nWeapon == 0);
				m_ItemSlot[0] = i;
				nWeapon++;
				break;
			}
		case ITP_ACCE:
		case ITP_ACCE_SET:
		case ITP_ACCE_SET_RLT:
		case ITP_ACCE_UNIQUE:
		case ITP_ACCE_COL:
		case ITP_ACCE_COL_RLT:
			{
				BsAssert(nAcce < m_nAccessorySlotCount);
				BsAssert(nAcce < _ACCESSORY_MAX_COUNT);
				m_ItemSlot[nAcce+1] = i;
				nAcce++;
				break;
			}
		}
	}
}

void CFcMenuStatus::ProcessMsg(int nMsgAsk, BsUiHANDLE hWnd, int nMsgRsp)
{
	if(nMsgAsk == fcMT_FailLevelItem){
		g_BsUiSystem.SetFocusWindow(BsUi::BsUiGetWindow(hWnd));
	}
	else if(nMsgAsk == fcMT_NotStateChangeItem){
		g_BsUiSystem.SetFocusWindow(BsUi::BsUiGetWindow(hWnd));
	}
	else if(nMsgAsk == fcMT_NoneItemInSlot){
		g_BsUiSystem.SetFocusWindow(BsUi::BsUiGetWindow(hWnd));
	}
	else if(nMsgAsk == fcMT_SucessChangeItem){
		UpdateStatusPage(_STATUS_PAGE);
	}
	else if(nMsgAsk == fcMT_NotAccountWeapon){
		g_BsUiSystem.SetFocusWindow(BsUi::BsUiGetWindow(hWnd));
	}
	else if(nMsgAsk == fcMT_OneEquipThisMission){
		g_BsUiSystem.SetFocusWindow(BsUi::BsUiGetWindow(hWnd));
	}
}

void CFcMenuStatus::UpdateStatusPage(int nPage)
{
	if(nPage == m_nCurPage){
		return;
	}

	m_nCurPage = nPage;

	UpdateHeadLineLayer();
	UpdateCharLayer();
	UpdateHeroNameLayer();
	UpdateGaugeLayer();
	
	UpdateItemTabLayer();
	UpdateStateLayer();

	UpdateItemListLayer();
	UpdateCompareLayer();

	switch(m_nCurPage)
	{
	case _STATUS_PAGE:
		{
			BsUiCLASS hClass = BT_Status_weapon + m_nCurSlot;
			BsUiWindow* pWindow = m_pTabLayer->GetWindow(hClass);
			BsAssert(pWindow);
			g_BsUiSystem.SetFocusWindow(pWindow);

			UpdateInfoLayer(m_ItemSlot[m_nCurSlot]);
			break;
		}
	case _WEAPON_PAGE:
	case _ACCESSORY_PAGE:
		{
			BsUiWindow* pWindow = m_pListLayer->GetWindow(LB_Status_list);
			BsAssert(pWindow);
			g_BsUiSystem.SetFocusWindow(pWindow);

			BsUiListBox* pListBox = (BsUiListBox*)(m_pListLayer->GetWindow(LB_Status_list));
			int nItemID = pListBox->GetItemData(pListBox->GetCurSel());
			
			UpdateInfoLayer(nItemID);
			break;
		}
	}
}

void CFcMenuStatus::UpdateHeadLineLayer()
{
	BsUiImageCtrl* pImg0 = (BsUiImageCtrl*)(m_pHeadLineLayer->GetWindow(IM_Status_Headline));
	BsUiImageCtrl* pImg1 = (BsUiImageCtrl*)(m_pHeadLineLayer->GetWindow(IM_StatusHeadline_Item));
	BsUiText* pText = (BsUiText*)(m_pHeadLineLayer->GetWindow(TT_StatusHelp));
	
	switch(m_nCurPage)
	{
	case _STATUS_PAGE:
		{
			pImg0->SetWindowAttr(XWATTR_SHOWWINDOW, true);
			pImg1->SetWindowAttr(XWATTR_SHOWWINDOW, false);

			if(g_MenuHandle->GetGamePauseMenu() == _FC_MENU_PAUSE){
				pText->SetItemTextID(_TEX_HELP_STATUS);
			}else{
				pText->SetItemTextID(_TEX_HELP_STATUS_Q);
			}

			break;
		}
	case _WEAPON_PAGE:
	case _ACCESSORY_PAGE:
		{
			pImg0->SetWindowAttr(XWATTR_SHOWWINDOW, false);
			pImg1->SetWindowAttr(XWATTR_SHOWWINDOW, true);
			pText->SetItemTextID(_TEX_HELP_ITEM);
			break;
		}
	}
}

void CFcMenuStatus::UpdateCharLayer()
{
	switch(m_nCurPage)
	{
	case _STATUS_PAGE:
		{
			m_pCharLayer->SetWindowAttr(XWATTR_SHOWWINDOW, true);
			break;
		}
	case _WEAPON_PAGE:
	case _ACCESSORY_PAGE:
		{
			m_pCharLayer->SetWindowAttr(XWATTR_SHOWWINDOW, false);
			return;
		}
	}

	BsUiImageCtrl* pImgCtrl = (BsUiImageCtrl*)(m_pCharLayer->GetWindow(IM_Status_Char));

	int nTexID = -1;
	CFcMenu3DObjManager* p3DObjMgr = g_MenuHandle->Get3DObjectMgr();
	if(p3DObjMgr != NULL){
		nTexID = p3DObjMgr->GetTextureID();
	}

	if(nTexID != -1){
		pImgCtrl->SetWindowAttr(XWATTR_SHOWWINDOW, true);
	}
	else
	{
		pImgCtrl->SetWindowAttr(XWATTR_SHOWWINDOW, false);
		return;
	}

	ImageCtrlInfo ImgCtrlInfo;
	ImgCtrlInfo.nTexId = nTexID; //3D object에 대한 RTT를 얻어야 한다.
	ImgCtrlInfo.u2 = 512;		//RTT생성은 openmenu(_FC_MENU_STATUS)에서 한다.
	ImgCtrlInfo.v2 = 560;
	
	pImgCtrl->SetImageInfo(&ImgCtrlInfo);
	//pImgCtrl->SetWindowPos(0, 110);
	pImgCtrl->SetWindowSize(512, 560);
	pImgCtrl->SetRenderTargetTexture(true);
}


void CFcMenuStatus::UpdateStateLayer()
{
	switch(m_nCurPage)
	{
	case _STATUS_PAGE:
		{
			m_pStateLayer->SetWindowAttr(XWATTR_SHOWWINDOW, true);
			break;
		}
	case _WEAPON_PAGE:
	case _ACCESSORY_PAGE:
		{
			m_pStateLayer->SetWindowAttr(XWATTR_SHOWWINDOW, false);
			return;
		}
	}

	int nItemID = m_ItemSlot[m_nCurSlot];

	//image
	BsUiImageCtrl* pImg = (BsUiImageCtrl*)(m_pStateLayer->GetWindow(IM_Status_iconitem));
	int nPage = _ACCESSORY_PAGE;
	if(m_nCurSlot == 0){
		nPage = _WEAPON_PAGE;
	}
	UpdateItemImage(pImg, nItemID, nPage, false);
	
	//tab item name & state
	char ctemp[256];
	BsUiText* pName = (BsUiText*)(m_pStateLayer->GetWindow(TT_Status_EquipiconItem_Name));
	BsUiListBox* pListNum = (BsUiListBox*)(m_pStateLayer->GetWindow(LB_Status_Stat_Item));
	if(nItemID != -1)
	{	
		pListNum->ClearItem();
		ItemDataTable* pInfo = g_FcItemManager.GetItemData(nItemID);
		GetCompareValue(ctemp, 256, pInfo->nHP);
		pListNum->AddItem(ctemp, 0);
		GetCompareValue(ctemp, 256, pInfo->nAttackPoint);
		pListNum->AddItem(ctemp, 1);
		GetCompareValue(ctemp, 256, pInfo->nDefensePoint);
		pListNum->AddItem(ctemp, 2);
		GetCompareValue(ctemp, 256, pInfo->nMoveSpeed);
		pListNum->AddItem(ctemp, 3);
		GetCompareValue(ctemp, 256, pInfo->nCriticalProbbt);
		pListNum->AddItem(ctemp, 4);

		char cItemName[256];
		sprintf(cItemName, "%s", g_FcItemManager.GetItemName(nItemID));
		pName->SetItemText(cItemName);

		pListNum->SetWindowAttr(XWATTR_SHOWWINDOW, true);
		pName->SetWindowAttr(XWATTR_SHOWWINDOW, true);
	}
	else
	{
		pListNum->SetWindowAttr(XWATTR_SHOWWINDOW, false);
		pName->SetWindowAttr(XWATTR_SHOWWINDOW, false);
	}
	
	//caption
	BsUiListBox* pListCap = (BsUiListBox*)(m_pStateLayer->GetWindow(LB_Status_Stat_list));
	pListCap->ClearItem();
	pListCap->AddItem(_TEX_SIMPLER_HP, 0);
	pListCap->AddItem(_TEX_SIMPLER_ATTACK, 1);
	pListCap->AddItem(_TEX_SIMPLER_DEFENCE, 2);
	pListCap->AddItem(_TEX_SIMPLER_SPEED, 3);
	pListCap->AddItem(_TEX_SIMPLER_CRITICAL, 4);

	//hero value
	BsUiListBox* pListState = (BsUiListBox*)(m_pStateLayer->GetWindow(LB_Status_Stat_CharNum));
	HeroObjHandle hHero = g_FcWorld.GetHeroHandle();
	pListState->ClearItem();
	sprintf(ctemp, "%d", hHero->GetMaxHP());
	pListState->AddItem(ctemp, 0);
	sprintf(ctemp, "%d", hHero->GetAttackPower());
	pListState->AddItem(ctemp, 1);
	sprintf(ctemp, "%d", hHero->GetDefense());
	pListState->AddItem(ctemp, 2);
	sprintf(ctemp, "%.1f", (hHero->GetMoveSpeed() * hHero->CalcMoveSpeedAdd() ));
	pListState->AddItem(ctemp, 3);
	sprintf(ctemp, "%d", hHero->GetCritical());
	pListState->AddItem(ctemp, 4);
}


void CFcMenuStatus::UpdateCompareLayer()
{
	switch(m_nCurPage)
	{
	case _STATUS_PAGE:
		{
			m_pCompareLayer->SetWindowAttr(XWATTR_SHOWWINDOW, false);
			return;
		}
	case _WEAPON_PAGE:
	case _ACCESSORY_PAGE:
		{
			m_pCompareLayer->SetWindowAttr(XWATTR_SHOWWINDOW, true);
			break;
		}
	}

	int nItemID = m_ItemSlot[m_nCurSlot];
	UpdateCompareInfo(TT_Status_ItemName_equip,
		IM_Status_Item_equip,
		LB_Status_Item_equip,
		LB_Status_Item_equip_Num,
		nItemID);


	BsUiListBox* pList = (BsUiListBox*)(m_pListLayer->GetWindow(LB_Status_list));
	BsAssert(pList);
	int nChangeItemID = pList->GetItemData(pList->GetCurSel());
	UpdateCompareInfo(TT_Status_ItemName_Change,
		IM_Status_Item_Change,
		LB_Status_Item_Change,
		LB_Status_Item_Change_Num,
		nChangeItemID);
}


void CFcMenuStatus::UpdateCompareInfo(BsUiCLASS hName, BsUiCLASS hImage,
									  BsUiCLASS hListCaption, BsUiCLASS hListValue, int nItemID)
{
	BsUiText* pName = (BsUiText*)(m_pCompareLayer->GetWindow(hName));
	BsUiImageCtrl* pImg = (BsUiImageCtrl*)(m_pCompareLayer->GetWindow(hImage));
	BsUiListBox* pListBox = (BsUiListBox*)(m_pCompareLayer->GetWindow(hListCaption));
	BsUiListBox* pListNum = (BsUiListBox*)(m_pCompareLayer->GetWindow(hListValue));

	pListBox->ClearItem();
	pListNum->ClearItem();
	
	if(nItemID != -1)
	{	
		pName->SetWindowAttr(XWATTR_SHOWWINDOW, true);
		pImg->SetWindowAttr(XWATTR_SHOWWINDOW, true);
		pListBox->SetWindowAttr(XWATTR_SHOWWINDOW, true);
		pListNum->SetWindowAttr(XWATTR_SHOWWINDOW, true);

		char cItemName[256];
		sprintf(cItemName, "%s", g_FcItemManager.GetItemName(nItemID));
		pName->SetItemText(cItemName);
		
		//Item image
		int nPage = _ACCESSORY_PAGE;
		if(m_nCurSlot == 0){
			nPage = _WEAPON_PAGE;
		}
		UpdateItemImage(pImg, nItemID, nPage, false);

		char cValue[256];
		ItemDataTable* pInfo = g_FcItemManager.GetItemData(nItemID);
		if(pInfo->nHP != 0)
		{
			pListBox->AddItem("HP", 0);
			GetCompareValue(cValue, 256, pInfo->nHP);
			pListNum->AddItem(cValue, 0);
		}

		if(pInfo->nAttackPoint != 0)
		{
			pListBox->AddItem("Atk", 0);
			GetCompareValue(cValue, 256, pInfo->nAttackPoint);
			pListNum->AddItem(cValue, 0);
		}

		if(pInfo->nDefensePoint != 0)
		{
			pListBox->AddItem("Def", 0);
			GetCompareValue(cValue, 256, pInfo->nDefensePoint);
			pListNum->AddItem(cValue, 0);
		}

		if(pInfo->nMoveSpeed != 0)
		{
			pListBox->AddItem("Speed", 0);
			GetCompareValue(cValue, 256, pInfo->nMoveSpeed);
			pListNum->AddItem(cValue, 0);
		}
		
		if(pInfo->nCriticalProbbt != 0)
		{
			pListBox->AddItem("Crit", 0);
			GetCompareValue(cValue, 256, pInfo->nCriticalProbbt);
			pListNum->AddItem(cValue, 0);
		}
	}
	else
	{
		pName->SetWindowAttr(XWATTR_SHOWWINDOW, false);
		pImg->SetWindowAttr(XWATTR_SHOWWINDOW, false);
		pListBox->SetWindowAttr(XWATTR_SHOWWINDOW, false);
		pListNum->SetWindowAttr(XWATTR_SHOWWINDOW, false);
	}
}

void CFcMenuStatus::GetCompareValue(char* szStr, const size_t szStr_len, int nValue)
{	
	if(nValue > 0){
		sprintf_s(szStr, szStr_len, "@(color=0,255,0,255)%d @(up)", nValue);
	}else if(nValue < 0){
		sprintf_s(szStr, szStr_len, "@(color=255,0,0,255)%d @(down)", nValue);
	}else{
		sprintf_s(szStr, szStr_len, "@(color=0,0,255,255)-");
	}
}

void CFcMenuStatus::UpdateHeroNameLayer()
{
	// Name & Lavel
	HeroObjHandle hHandle = g_FcWorld.GetHeroHandle();
	int nUnitIndex = hHandle->GetUnitSoxIndex();
	int nLevel = hHandle->GetLevel() + 1;

	int nTextId = -1;
	switch(nUnitIndex)
	{
	case CHAR_ID_ASPHARR:	nTextId = _TEX_Aspharr; break;
	case CHAR_ID_INPHYY:	nTextId = _TEX_Inphyy; break;
	case CHAR_ID_TYURRU:	nTextId = _TEX_Tyurru; break;
	case CHAR_ID_KLARRANN:	nTextId = _TEX_Klarrann; break;
	case CHAR_ID_MYIFEE:	nTextId = _TEX_Myifee; break;
	case CHAR_ID_VIGKVAGK:	nTextId = _TEX_Vigkvagk; break;
	case CHAR_ID_DWINGVATT:	nTextId = _TEX_Dwingvatt; break;
	default: BsAssert(0);
	}

	char cName[256];
	g_TextTable->GetText(nTextId, cName, _countof(cName));

	char cLevel[64];
	g_TextTable->GetText(_TEX_SUM_LEVEL, cLevel, _countof(cLevel));

	char cFullName[256];
	sprintf(cFullName, "%s %s %d", cName, cLevel, nLevel);

	m_pNameLayer->GetWindow(TT_Status_CharName)->SetItemText(cFullName);
	m_pNameLayer->GetWindow(TT_Status_CharName)->SetItemData(nUnitIndex);
}

void CFcMenuStatus::UpdateGaugeLayer()
{
	switch(m_nCurPage)
	{
	case _STATUS_PAGE:
		{
			m_pGaugeLayer->SetWindowAttr(XWATTR_SHOWWINDOW, true);
			break;
		}
	case _WEAPON_PAGE:
	case _ACCESSORY_PAGE:
		{
			m_pGaugeLayer->SetWindowAttr(XWATTR_SHOWWINDOW, false);
			return;
		}
	}
	
	// Hp gauge
	BsUiText* pHpValue = (BsUiText*)(m_pGaugeLayer->GetWindow(TT_Status_Hp_Num));
	BsUiImageCtrl* pHpBar = (BsUiImageCtrl*)(m_pGaugeLayer->GetWindow(IM_Status_Hp_Bar));

	HeroObjHandle hHero = g_FcWorld.GetHeroHandle();
	int nHP = hHero->GetHP();
	int nMaxHP = hHero->GetMaxHP();
	
	char cValue[256];
	sprintf(cValue, "%d / %d", nHP, nMaxHP);
	pHpValue->SetItemText(cValue);

	int nW(0), nH(0);
	pHpBar->GetWindowSize(nW, nH);
	pHpBar->SetWindowSize(nHP * 300 / nMaxHP, nH);

	// orbs gauge
	BsUiText* pOrbValue = (BsUiText*)(m_pGaugeLayer->GetWindow(TT_Status_Orb_Num));
	BsUiImageCtrl* pOrbBar = (BsUiImageCtrl*)(m_pGaugeLayer->GetWindow(IM_Status_Orb_Ber));

	int nOrbExp = hHero->GetEXP();
	int nLv = hHero->GetLevel();
	int nOrbW(0);
	if(nLv + 1 >= MAX_HERO_LEVEL)
	{
		sprintf(cValue, "%d", nOrbExp);
		nOrbW = 300;
	}
	else
	{
		sprintf(cValue, "%d / %d", nOrbExp, g_nRequiredExp[nLv]);
		nOrbW = nOrbExp * 300 / g_nRequiredExp[nLv];
	}                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               
	pOrbValue->SetItemText(cValue);
	
	pOrbBar->GetWindowSize(nW, nH);
	pOrbBar->SetWindowSize(nOrbW, nH);
}


void CFcMenuStatus::UpdateInfoLayer(int nItemID)
{	
	BsUiText* pTextItem = (BsUiText*)(m_pInfoLayer->GetWindow(TT_Status_Item_info));
	if(nItemID != -1)
	{
		pTextItem->SetWindowAttr(XWATTR_SHOWWINDOW, true);

		int nPage = _ACCESSORY_PAGE;
		if(m_nCurSlot == 0){
			nPage = _WEAPON_PAGE;
		}
		
		//Item info
		pTextItem->SetItemText(g_FcItemManager.GetItemDesc(nItemID));
	}
	else{
		pTextItem->SetWindowAttr(XWATTR_SHOWWINDOW, false);
	}
}


void CFcMenuStatus::UpdateItemListLayer()
{
	switch(m_nCurPage)
	{
	case _STATUS_PAGE:
		{
			m_pListLayer->SetWindowAttr(XWATTR_SHOWWINDOW, false);
			return;
		}
	case _WEAPON_PAGE:
	case _ACCESSORY_PAGE:
		{
			m_pListLayer->SetWindowAttr(XWATTR_SHOWWINDOW, true);
			break;
		}
	}

	UpdateItemList();
}


void CFcMenuStatus::UpdateItemList()
{
	BsUiListBox* pListBox = (BsUiListBox*)(m_pListLayer->GetWindow(LB_Status_list));
	BsAssert( pListBox );
	BsUiListBox* pIconListBox = (BsUiListBox*)(m_pListLayer->GetWindow(LB_Status_list_icon));
	BsAssert( pIconListBox );

	pListBox->ClearItem();
	pIconListBox->ClearItem();

	pListBox->SetCurSel(0);
	pIconListBox->SetCurSel(0);

	for(int i=0; i<g_FcItemManager.GetItemCount(); i++)
	{
		ItemDataTable* pItem = g_FcItemManager.GetItemData(i);
		BsAssert(pItem != NULL);
		BsAssert(g_FcItemManager.GetItemName(i) != NULL);

		switch(m_nCurPage)
		{
		case _WEAPON_PAGE:
			{
				if(pItem->nItemType != ITP_WEAPON){
					continue;
				}
				break;
			}
		case _ACCESSORY_PAGE:
			{
				if(pItem->nItemType == ITP_NORMAL){
					continue;
				}

				if(pItem->nItemType == ITP_WEAPON){
					continue;
				}

				if(pItem->nItemType == ITP_LAST){
					continue;
				}
				break;
			}
		default: BsAssert(0);
		}
			
		if(g_FcItemManager.IsAccoutered(i) == false)
		{
			pListBox->AddItem(g_FcItemManager.GetItemName(i), i);
			if(g_FcItemManager.IsNewItem(i)){
				pIconListBox->AddItem("@(New_icon)", 1);
			}else{
				pIconListBox->AddItem(" ", 1);
			}
		}
	}

	if(pListBox->GetItemCount() == 0)
	{
		char cItemName[256];
		g_TextTable->GetText(_TEX_EQUIP_NONE, cItemName, _countof(cItemName));
		pListBox->AddItem(cItemName, -1);
	}
}

void CFcMenuStatus::UpdateItemTabLayer()
{	
	int nHeroID = g_FcWorld.GetHeroHandle()->GetUnitSoxIndex();

	for(int i=0; i<_ITEM_ICON_MAX_COUNT; i++)
	{
		BsUiCLASS hWnd = BT_Status_weapon + i;
		BsUiWindow* pButton = m_pTabLayer->GetWindow(hWnd);
		BsAssert(pButton != NULL);

		BsUiCLASS hImg = IM_Status_weapon + i;
		BsUiImageCtrl* pImg = (BsUiImageCtrl*)(m_pTabLayer->GetWindow(hImg));
		BsAssert(pImg != NULL);

		pButton->SetWindowAttr(XWATTR_DISABLE, false);
		pImg->SetWindowAttr(XWATTR_SHOWWINDOW, true);

		if(i == 0 && nHeroID == CHAR_ID_VIGKVAGK)
		{
			pButton->SetWindowAttr(XWATTR_DISABLE, true);
			pImg->SetWindowAttr(XWATTR_SHOWWINDOW, false);
		}
		if(i < m_nAccessorySlotCount + 1)
		{
			pButton->SetItemData(m_ItemSlot[i]);
		
			int nPage = _ACCESSORY_PAGE;
			if(i == 0){
				nPage = _WEAPON_PAGE;
			}
			UpdateItemImage(pImg, m_ItemSlot[i], nPage, true);
		}
		else
		{
			pButton->SetWindowAttr(XWATTR_DISABLE, true);
			pImg->SetWindowAttr(XWATTR_SHOWWINDOW, false);
		}
	}
}

void CFcMenuStatus::UpdateItemImage(BsUiImageCtrl* pImg, int nItemID, int nPage, bool bIcon)
{
	if( nItemID == -1 ){
		pImg->SetWindowAttr(XWATTR_SHOWWINDOW, false);
		return;
	}

	//Item Image
	ItemDataTable* pItem = g_FcItemManager.GetItemData(nItemID);
	BsAssert(pItem != NULL);
	if(pItem->szImageFile == NULL || strlen(pItem->szImageFile) == 0){
		return;
	}

	if(strcmp(pItem->szImageFile, "noname") == 0)
	{
		pImg->SetWindowAttr(XWATTR_SHOWWINDOW, false);
		return;
	}

	int nTexId = -1;

	CBsKernel::GetInstance().chdir("Interface");
	CBsKernel::GetInstance().chdir("Item");

	switch(nPage)
	{
	case _WEAPON_PAGE:		CBsKernel::GetInstance().chdir("Weapon"); break;
	case _ACCESSORY_PAGE:	CBsKernel::GetInstance().chdir("Accessory"); break;
	default: BsAssert(0);
	}

	if(bIcon)
	{
		CBsKernel::GetInstance().chdir("Icon");

		char cItemName[256];
		sprintf_s(cItemName, _countof(cItemName), "I_%s", pItem->szImageFile);
		nTexId = g_BsKernel.LoadTexture(cItemName);

		CBsKernel::GetInstance().chdir("..");
	}
	else{
		nTexId = g_BsKernel.LoadTexture(pItem->szImageFile);
	}
	
	CBsKernel::GetInstance().chdir("..");
	CBsKernel::GetInstance().chdir("..");
	CBsKernel::GetInstance().chdir("..");

	if(nTexId == -1){
		pImg->SetWindowAttr(XWATTR_SHOWWINDOW, false);
	}
	else
	{
		pImg->SetWindowAttr(XWATTR_SHOWWINDOW, true);
		SIZE size = g_BsKernel.GetTextureSize(nTexId);

		ImageCtrlInfo info;
		info.nTexId = nTexId;
		if(bIcon)
		{
			info.u1 = 20;
			info.v1 = 20;
			info.u2 = 108;
			info.v2 = 108;
		}
		else
		{
			info.u2 = size.cx;
			info.v2 = size.cy;
		}
		pImg->SetImageInfo(&info);
	}
}


void CFcMenuStatus::SetItemSlot(int nSlot, int nItemID)
{
	if(nSlot < 0 || nSlot >= _ITEM_ICON_MAX_COUNT){
		BsAssert(0);
	}

	m_ItemSlot[nSlot] = nItemID;
}

int	CFcMenuStatus::GetItemSlot(int nSlot)
{
	if(nSlot < 0 || nSlot >= _ITEM_ICON_MAX_COUNT){
		BsAssert(0);
	}

	return m_ItemSlot[nSlot];
}

void CFcMenuStatus::SetCurSlot(int nSlot)
{ 
	if(nSlot < 0 || nSlot >= _ITEM_ICON_MAX_COUNT)
	{
		BsAssert(0);
		return;
	}

	m_nCurSlot = nSlot;
}

//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------
CFcMenuStatusItemListLayer::CFcMenuStatusItemListLayer(CFcMenuStatus* pMenu)
{
	m_pMenu = pMenu;
}

DWORD CFcMenuStatusItemListLayer::ProcMessage(xwMessageToken* pMsgToken)
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
					int nHeroID = g_FcWorld.GetHeroHandle()->GetUnitSoxIndex();
					if(nHeroID != CHAR_ID_VIGKVAGK && m_pMenu->GetItemSlot(0) == -1)
					{
						g_MenuHandle->PostMessage(fcMSG_MESSAGEBOX_START,
							fcMT_NotAccountWeapon,
							(DWORD)m_pMenu, pMsgToken->hWnd);
					}
					else
					{
						g_pSoundManager->PlaySystemSound( SB_COMMON,"SYS_OK");
						m_pMenu->UpdateStatusPage(_STATUS_PAGE);
					}
					break;
				}
			default:
				{
					BsUiCLASS hClass = BsUi::BsUiGetClass(pMsgToken->hWnd);
					switch(hClass)
					{
					case LB_Status_list: OnKeyDownLbItemTab(pMsgToken); break;
					}
				}
			}
		}
	case XWMSG_BN_CLICKED:
		{
			break;
		}
	}

	return BsUiWindow::ProcMessage(pMsgToken);
}


void CFcMenuStatusItemListLayer::OnKeyDownLbItemTab(xwMessageToken* pMsgToken)
{
	switch (pMsgToken->lParam)
	{
	case PAD_INPUT_UP:
	case PAD_INPUT_DOWN:
		{
			BsUiListBox* pListBox = (BsUiListBox*)GetWindow( LB_Status_list );
			BsAssert( pListBox );

			int nCurPos = pListBox->GetCurSel();

			BsUiListBox* pIconListBox = (BsUiListBox*)GetWindow( LB_Status_list_icon );
			pIconListBox->SetCurSel(nCurPos);

			int nItemID = pListBox->GetItemData(nCurPos);
			m_pMenu->UpdateInfoLayer(nItemID);
			m_pMenu->UpdateCompareLayer();

			break;
		}
	case MENU_INPUT_A:
		{			
			if(AccountItem(pMsgToken))
			{
				g_MenuHandle->PostMessage(fcMSG_MESSAGEBOX_START,
					fcMT_SucessChangeItem,
					(DWORD)m_pMenu, pMsgToken->hWnd);
			}
			break;
		}
	case MENU_INPUT_Y:
		{
			if(CancelItem(pMsgToken))
			{
				g_pSoundManager->PlaySystemSound( SB_COMMON,"SYS_OK");

				m_pMenu->UpdateCompareLayer();
				m_pMenu->UpdateItemListLayer();

				BsUiListBox* pListBox = (BsUiListBox*)GetWindow( LB_Status_list );
				BsAssert( pListBox );

				int nCurPos = pListBox->GetCurSel();
				int nItemID = pListBox->GetItemData(nCurPos);
				m_pMenu->UpdateInfoLayer(nItemID);
				m_pMenu->UpdateItemTabLayer();
				m_pMenu->UpdateCompareLayer();
			}
			break;
		}
	}
}

bool CFcMenuStatusItemListLayer::AccountItem(xwMessageToken* pMsgToken)
{
	BsUiListBox* pListBox = (BsUiListBox*)GetWindow(LB_Status_list);
	BsAssert( pListBox );
	int nSelItemID = pListBox->GetItemData(pListBox->GetCurSel());
	if(nSelItemID == -1){
		return false;
	}

	//slot에 아템이 없어서 삭제 못하는 경우
	if(m_pMenu->GetCurSlot() == 0)
	{
		if(g_FcWorld.GetHeroHandle()->IsChangableWeapon() == false)
		{
			g_MenuHandle->PostMessage(fcMSG_MESSAGEBOX_START,
				fcMT_NotStateChangeItem,
				(DWORD)m_pMenu, pMsgToken->hWnd);
			return false;
		}
	}
	
	_ACCOUTERING_MSG nMsg = g_FcItemManager.CheckAccoutering(nSelItemID);
	switch(nMsg)
	{
	case ACCT_MSG_ABLE: break;
	case ACCT_MSG_FAIL_LEVEL:
		{
			g_MenuHandle->PostMessage(fcMSG_MESSAGEBOX_START,
				fcMT_FailLevelItem,
				(DWORD)m_pMenu, pMsgToken->hWnd);
			return false;
		}
	case ACCT_MSG_ONE_EQUIP_THIS_MISSION:
		{
			g_MenuHandle->PostMessage(fcMSG_MESSAGEBOX_START,
				fcMT_OneEquipThisMission,
				(DWORD)m_pMenu, pMsgToken->hWnd);
			return false;
		}
	}

	int nItemID = m_pMenu->GetItemSlot(m_pMenu->GetCurSlot());
	if(nItemID != -1){
		g_FcItemManager.SetAccoutered(nItemID, false);
	}
	
	g_FcItemManager.SetAccoutered(nSelItemID, true);
	m_pMenu->SetItemSlot(m_pMenu->GetCurSlot(), nSelItemID);

	//변경되는 item이 무기인 경우.. 3D캐릭터의 무기를 바꿔 준다.
	int nHeroID = g_FcWorld.GetHeroHandle()->GetUnitSoxIndex();
	if(m_pMenu->GetCurSlot() == 0)
	{
		int nItemSoxID = g_FcItemManager.GetHeroEquip(nSelItemID)->nItemSoxID;
		int nSkinIndex = g_FcItemManager.GetSkinIndex(nItemSoxID);
		CFcMenu3DObjManager* p3DObjMgr = g_MenuHandle->Get3DObjectMgr();
		if(p3DObjMgr != NULL)
		{
			DWORD hHandle = ((CFcMenuStatus*)m_pMenu)->Get3DObject();
			p3DObjMgr->ChangeWeapon(hHandle, nSkinIndex);
		}

		g_FcWorld.GetHeroHandle()->ChangeWeapon( nSkinIndex, 0 );
	}
	else if(nHeroID != CHAR_ID_VIGKVAGK)
	{
		int nBeforeItemEquipLevelDown(0), nSelItemEquipLevelDown(0);
		if(nItemID != -1)
		{
			ItemDataTable* pBeforeItem = g_FcItemManager.GetItemData(nItemID);
			nBeforeItemEquipLevelDown = pBeforeItem->nItemEquipLevelDown;
		}
		
		if(nSelItemID != -1)
		{
			ItemDataTable* pSelItem = g_FcItemManager.GetItemData(nSelItemID);
			nSelItemEquipLevelDown = pSelItem->nItemEquipLevelDown;
		}

		if(nSelItemEquipLevelDown - nBeforeItemEquipLevelDown < 0){
			CheckItemEquipLevelDown();
		}
	}

	return true;
}

bool CFcMenuStatusItemListLayer::CancelItem(xwMessageToken* pMsgToken)
{
	int nItemID = m_pMenu->GetItemSlot(m_pMenu->GetCurSlot());
	if(nItemID == -1)
	{
		//빈 slot인데 삭제하라고 하면
		g_MenuHandle->PostMessage(fcMSG_MESSAGEBOX_START,
			fcMT_NoneItemInSlot,
			(DWORD)m_pMenu, pMsgToken->hWnd);

		return false;
	}	

	//삭제 아이템이 무기인데..못바꾸는 상황이라면..
	if(m_pMenu->GetCurSlot() == 0)
	{
		if(g_FcWorld.GetHeroHandle()->IsChangableWeapon() == false)
		{
			g_MenuHandle->PostMessage(fcMSG_MESSAGEBOX_START,
				fcMT_NotStateChangeItem,
				(DWORD)m_pMenu, pMsgToken->hWnd);
			return false;
		}
	}
	
	g_FcItemManager.SetAccoutered(nItemID, false);
	m_pMenu->SetItemSlot(m_pMenu->GetCurSlot(), -1);

	int nHeroID = g_FcWorld.GetHeroHandle()->GetUnitSoxIndex();
	if(nHeroID != CHAR_ID_VIGKVAGK && m_pMenu->GetCurSlot() != 0){
		CheckItemEquipLevelDown();
	}

	return true;
}

void CFcMenuStatusItemListLayer::CheckItemEquipLevelDown()
{
	int nLevel = g_FcWorld.GetHeroHandle(0)->GetLevel();
	if(nLevel <= 0){
		nLevel = 0;
	}

	int nWeaponItemID = m_pMenu->GetItemSlot(0);
	ItemDataTable* pWeaponItem = g_FcItemManager.GetItemData(nWeaponItemID);
	BsAssert(pWeaponItem != NULL);
	int nWeaponEquipGap = nLevel - (pWeaponItem->nLevelMinimum - 1);
	if(nWeaponEquipGap >= 0){
		return;
	}
    
	//착용하고 있는 item중 전체 ItemEquipLevelDown값을 얻는다.
	int nItemEquipLevelDown = 0;
	for(int i=1; i<_ITEM_ICON_MAX_COUNT; i++)
	{
		int nItemID = m_pMenu->GetItemSlot(i);
		if(nItemID == -1){
			continue;
		}

		ItemDataTable* pItem = g_FcItemManager.GetItemData(nItemID);
		BsAssert(pItem != NULL);
		if(pItem->nItemEquipLevelDown == 0){
			continue;
		}

		nItemEquipLevelDown += pItem->nItemEquipLevelDown;
	}

	//현재 레벨보다 높은 아이템을 착용하고 있다면 문제 처리.
	int nCurWeaponEquipGap = nItemEquipLevelDown + nWeaponEquipGap;
	if(nCurWeaponEquipGap >= 0){
		return;
	}

	//--------------------------------------------------------------------------
	//현재 착용하고 있는 아이템을 풀어준다.
	g_FcItemManager.SetAccoutered(nWeaponItemID, false);

	int nEnableWeaponEquipLevel = nLevel + nItemEquipLevelDown;
	int nEquipWeaponItemID = -1;
	int nEquipWeaponLevel = 0;

	for(int i=0; i<g_FcItemManager.GetItemCount(); i++)
	{
		ItemDataTable* pItem = g_FcItemManager.GetItemData(i);
		if(pItem->nItemType != ITP_WEAPON){
			continue;
		}

		int nLevelMinimum = pItem->nLevelMinimum - 1;
		if(nLevelMinimum > nEnableWeaponEquipLevel){
			continue;
		}

		if(nEquipWeaponLevel > nLevelMinimum ){
			continue;
		}

		nEquipWeaponItemID = i;
		nEquipWeaponLevel = nLevelMinimum;
	}

	BsAssert(nEquipWeaponItemID != -1);

	//적당한 새 아이템을 착용한다.
	g_FcItemManager.SetAccoutered(nEquipWeaponItemID, true);
	m_pMenu->SetItemSlot(0, nEquipWeaponItemID);

	//--------------------------------------------------------------------------
	//
	int nItemSoxID = g_FcItemManager.GetHeroEquip(nEquipWeaponItemID)->nItemSoxID;
	int nSkinIndex = g_FcItemManager.GetSkinIndex(nItemSoxID);
	CFcMenu3DObjManager* p3DObjMgr = g_MenuHandle->Get3DObjectMgr();
	if(p3DObjMgr != NULL)
	{
		DWORD hHandle = ((CFcMenuStatus*)m_pMenu)->Get3DObject();
		p3DObjMgr->ChangeWeapon(hHandle, nSkinIndex);
	}

	g_FcWorld.GetHeroHandle()->ChangeWeapon( nSkinIndex, 0 );
}


//-----------------------------------------------------------------
CFcMenuStatusItemTabLayer::CFcMenuStatusItemTabLayer(CFcMenuStatus* pMenu)
{
	m_pMenu = pMenu;
}

DWORD CFcMenuStatusItemTabLayer::ProcMessage(xwMessageToken* pMsgToken)
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
					break;
				}
			case MENU_INPUT_BACK:
				{
					if(g_MenuHandle->GetGamePauseMenu() == _FC_MENU_STATUS){
						m_pMenu->SetStatus(_MS_CLOSING);
					}
					break;
				}
			case MENU_INPUT_A:
				{
					BsUiCLASS hClass = BsUi::BsUiGetClass(pMsgToken->hWnd);
					switch(hClass)
					{
					case BT_Status_weapon:
						{
							m_pMenu->UpdateStatusPage(_WEAPON_PAGE);
							break;
						}
					case BT_Status_accessory_1:
					case BT_Status_accessory_2:
					case BT_Status_accessory_3:
					case BT_Status_accessory_4:
					case BT_Status_accessory_5:
						{
							m_pMenu->UpdateStatusPage(_ACCESSORY_PAGE);
							break;
						}
					default:
						{
							BsAssert(0);
						}
					}
					break;
				}
			case MENU_INPUT_LEFT:
				{
					BsUiWindow* pPrev = SetFocusPrevChileWindow(pMsgToken->hWnd, true);
					if(pPrev != NULL && pPrev->GetHWnd() != pMsgToken->hWnd)
					{
						g_pSoundManager->PlaySystemSound( SB_COMMON,"SYS_CURSOR");

						int nCurSlot = m_pMenu->GetCurSlot() - 1;
						int nAccessorySlotCount = m_pMenu->GetAccessorySlotCount();
						int nHeroID = g_FcWorld.GetHeroHandle()->GetUnitSoxIndex();

						if(nHeroID == CHAR_ID_VIGKVAGK && nCurSlot == 0){
							m_pMenu->SetCurSlot(nAccessorySlotCount);
						}else if(nCurSlot == -1){
							m_pMenu->SetCurSlot(nAccessorySlotCount);
						}else{
							m_pMenu->SetCurSlot(nCurSlot);
						}

						int nItemID = m_pMenu->GetItemSlot(m_pMenu->GetCurSlot());
						m_pMenu->UpdateInfoLayer(nItemID);
						m_pMenu->UpdateStateLayer();
					}
					break;
				}
			case MENU_INPUT_RIGHT:
				{
					BsUiWindow* pNext = SetFocusNextChileWindow(pMsgToken->hWnd, true);
					if(pNext != NULL && pNext->GetHWnd() != pMsgToken->hWnd)
					{
						g_pSoundManager->PlaySystemSound( SB_COMMON,"SYS_CURSOR");
						
						int nCurSlot = m_pMenu->GetCurSlot() + 1;
						int nAccessorySlotCount = m_pMenu->GetAccessorySlotCount();
						int nHeroID = g_FcWorld.GetHeroHandle()->GetUnitSoxIndex();

						if(nHeroID == CHAR_ID_VIGKVAGK && nCurSlot == nAccessorySlotCount + 1){
							m_pMenu->SetCurSlot(1);
						}else if(nCurSlot == nAccessorySlotCount + 1){
							m_pMenu->SetCurSlot(0);
						}else{
							m_pMenu->SetCurSlot(nCurSlot);
						}

						int nItemID = m_pMenu->GetItemSlot(m_pMenu->GetCurSlot());
						m_pMenu->UpdateInfoLayer(nItemID);
						m_pMenu->UpdateStateLayer();
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