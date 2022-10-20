#include "stdafx.h"

#include "FcMenuAbility.h"
#include "BSuiSystem.h"

#include "BsUiClassIDManager.h"
#include "BsUiFontAttrManager.h"
#include "FcInterfaceManager.h"
#include "FcMessageDef.h"
#include "FcWorld.h"
#include "FcHeroObject.h"
#include "FcSOXLoader.h"
#include "Fcitem.h"
#include "TextTable.h"

#include "BsUiUVManager.h"
#include ".\\data\\Menu\\BsUiUVID.h"

CAbilityTableLoader g_AbiltyTable;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG

CFcMenuAbility::CFcMenuAbility(_FC_MENU_TYPE nType, _FC_MENU_TYPE nPreType)
: CFcMenuForm(nType)
{	
	HeroObjHandle Handle = g_FcWorld.GetHeroHandle();
	g_AbiltyTable.Load(Handle->GetUnitSoxIndex());
	CreateObject();

	m_pBaseLayer = new BsUiLayer();
	AddUiLayer(m_pBaseLayer, SN_Ability0, -1, -1);

	m_pNameLayer = new BsUiLayer();
	AddUiLayer(m_pNameLayer, SN_Ability1, -1, -1);

	m_pListLayer = new CFcMenuAbilityLayer(this);
	AddUiLayer(m_pListLayer, SN_Ability2, -1, -1);


	UpdateBaseLayer();
	UpdateNameLayer();
	m_pListLayer->Initialize();
}

CFcMenuAbility::~CFcMenuAbility()
{
	CloseObject();
	g_AbiltyTable.Release();
}

void CFcMenuAbility::CreateObject()
{
	CFcMenu3DObjManager* p3DObjMgr = g_MenuHandle->Get3DObjectMgr();

	if(p3DObjMgr == NULL){
		return;
	}

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
		
	CCrossVector* pObjCross = p3DObjMgr->GetObjCrossVector(m_h3DObject);
	BsAssert(pObjCross && "Unable to get ObjectCrossVector"); //aleksger: prefix bug 803: GetObjCrossVector() may return NULL.
	pObjCross->RotateYaw(g_CharStandInfo[nSoxIndex].Angle);

	int nSkinIndex = -1;
	for(int i=0; i<g_FcItemManager.GetItemCount(); i++)
	{
		ItemDataTable* pItem = g_FcItemManager.GetItemData(i);
		BsAssert(pItem != NULL);
		BsAssert(g_FcItemManager.GetItemName(i) != NULL);

		if(pItem->nItemType != ITP_WEAPON){
			continue;
		}

		if(g_FcItemManager.IsAccoutered(i) == false){	
			continue;
		}

		int nItemSoxID = g_FcItemManager.GetHeroEquip(i)->nItemSoxID;
		nSkinIndex = g_FcItemManager.GetSkinIndex(nItemSoxID);

		break;
	}

	p3DObjMgr->ChangeWeapon(m_h3DObject, nSkinIndex);
}


void CFcMenuAbility::CloseObject()
{
	CFcMenu3DObjManager* p3DObjMgr = g_MenuHandle->Get3DObjectMgr();
	if(p3DObjMgr != NULL){
		p3DObjMgr->RemoveAllObject();
	}
}


void CFcMenuAbility::RenderProcess()
{
	switch(GetStatus())
	{
	case _MS_OPEN:
		{	
			m_pBaseLayer->SetColor(D3DXCOLOR(1, 1, 1, 0.f));
			m_pNameLayer->SetColor(D3DXCOLOR(1, 1, 1, 0.f));
			m_pListLayer->SetColor(D3DXCOLOR(1, 1, 1, 0.f));

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


#define _START_POS_X_ABT1				-520
#define _END_POS_X_ABT1					0
#define _START_POS_X_ABT2				520
#define _END_POS_X_ABT2					0
void CFcMenuAbility::RenderMoveLayer(int nTick)
{	
	//------------------------------------------------------
	//m_pBaseLayer, m_pHeadLineLayer, m_pCharLayer
	if(nTick <= _TICK_FADE_IN_BASE)
	{
		float fAlpha = (float)nTick / _TICK_FADE_IN_BASE;
		m_pBaseLayer->SetColor(D3DXCOLOR(1, 1, 1, fAlpha));
	}

	//------------------------------------------------------
	//m_pNameLayer, m_pGaugeLayer
	if(nTick <= _TICK_IN_FORM_LEFT)
	{	
		float frate = 1.f - ((float)nTick / _TICK_IN_FORM_LEFT);
		int nX = (int)(_START_POS_X_ABT1 * frate);
		m_pNameLayer->SetWindowPosX(nX);

		float fAlpha = 1.f - frate;
		m_pNameLayer->SetColor(D3DXCOLOR(1, 1, 1, fAlpha));
	}

	//------------------------------------------------------
	//m_pInfoLayer, m_pStateLayer, m_pTabLayer
	if(nTick <= _TICK_IN_FORM_RIGHT)
	{
		float frate = (float)nTick / _TICK_IN_FORM_RIGHT;
		int nX = _END_POS_X_ABT2 + int((_START_POS_X_ABT2 - _END_POS_X_ABT2) * (1.f - frate));
		m_pListLayer->SetWindowPosX(nX);

		float fAlpha = frate;
		m_pListLayer->SetColor(D3DXCOLOR(1, 1, 1, fAlpha));
	}
}


void CFcMenuAbility::UpdateBaseLayer()
{
	int nTexID = -1;
	CFcMenu3DObjManager* p3DObjMgr = g_MenuHandle->Get3DObjectMgr();
	if(p3DObjMgr != NULL){
		nTexID = p3DObjMgr->GetTextureID();
	}

	ImageCtrlInfo ImgCtrlInfo;
	ImgCtrlInfo.nTexId = nTexID; //3D object에 대한 RTT를 얻어야 한다.
	ImgCtrlInfo.u1 = 0;
	ImgCtrlInfo.v1 = 0;
	ImgCtrlInfo.u2 = 512;
	ImgCtrlInfo.v2 = 560;

	BsUiImageCtrl* pImgCtrl = (BsUiImageCtrl*)(m_pBaseLayer->GetWindow(IM_AbilityChar));
	pImgCtrl->SetImageInfo(&ImgCtrlInfo);
	//pImgCtrl->SetWindowPos(-12, 60);
	pImgCtrl->SetWindowSize(512, 560);
	pImgCtrl->SetRenderTargetTexture(true);
}

void CFcMenuAbility::UpdateNameLayer()
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

	m_pNameLayer->GetWindow(TT_Ability_CharName)->SetItemText(cFullName);
	m_pNameLayer->GetWindow(TT_Ability_CharName)->SetItemData(nUnitIndex);
}

//-----------------------------------------------------------------------------------------------------
CFcMenuAbilityLayer::CFcMenuAbilityLayer(CFcMenuForm* pMenu)
{
	m_pMenu = pMenu;
}

#define _TAB_WEAPON			0
#define _TAB_ACCESSORY		1
void CFcMenuAbilityLayer::Initialize()
{
	UpdateLbAbility();
	UpdateData(0);

	SetFocusWindowClass(LB_Abilitylist);
}

DWORD CFcMenuAbilityLayer::ProcMessage(xwMessageToken* pMsgToken)
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
					g_MenuHandle->PostMessage(fcMSG_PAUSE_START);
					break;
				}
			default:
				{
					BsUiCLASS hClass = BsUi::BsUiGetClass(pMsgToken->hWnd);
					switch(hClass)
					{
					case LB_Abilitylist:	OnKeyDownLbEquip(pMsgToken); break;
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


void CFcMenuAbilityLayer::OnKeyDownLbEquip(xwMessageToken* pMsgToken)
{
	switch (pMsgToken->lParam)
	{
	case PAD_INPUT_UP:
	case PAD_INPUT_DOWN:
		{
			BsUiListBox* pListBox = (BsUiListBox*)GetWindow( LB_Abilitylist );
			BsAssert( pListBox );
			
			UpdateData(pListBox->GetCurSel());
			
			break;
		}
	}
}


void CFcMenuAbilityLayer::UpdateLbAbility()
{
	BsUiListBox* pList = (BsUiListBox*)GetWindow(LB_Abilitylist);
	pList->ClearItem();

	HeroObjHandle Handle = g_FcWorld.GetHeroHandle();
	int nUnitID = Handle->GetUnitSoxIndex();
	int nLevel = Handle->GetLevel() + 1;

	int nCount = g_AbiltyTable.GetItemTableCount();
	for(int i=nCount-1; i>=0; i--)
	{
		AbilityLocalTable* pTable = g_AbiltyTable.GetItemData(i);
		BsAssert(pTable);
		
		if(nLevel < pTable->nLevel){
			continue;
		}
		
		pList->AddItem(pTable->cName, i);
	}
}

void CFcMenuAbilityLayer::UpdateData(int nPos)
{
	BsUiListBox* pListBox = (BsUiListBox*)GetWindow(LB_Abilitylist);
	DWORD nID = pListBox->GetItemData(nPos);

	AbilityLocalTable* pTable = g_AbiltyTable.GetItemData(nID);

	//GetWindow(TT_Ability_control)->SetItemText();
	GetWindow(TT_AbilityInfo)->SetItemText(pTable->cCommand);
}