#include "stdafx.h"

#include "FcMenuSelectChar.h"
#include "BSuiSystem.h"

#include "BsUiClassIDManager.h"
#include "BsUiFontAttrManager.h"

#include "FcInterfaceManager.h"
#include "FcMessageDef.h"

#include "FcGlobal.h"
#include "FcSpeech.h"

#include "FcMenu3DObject.h"
#include "BsSinTable.h"

#include "BsUiUVManager.h"
#include ".\\data\\Menu\\BsUiUVID.h"
#include "TextTable.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG


#define _CAMERA_POS					D3DXVECTOR3(0, 100.f, -600.f)
#define _CAMERA_TARGET_POS			D3DXVECTOR3(0, 100.f, 0.f)
#define _LIGHT_POS					D3DXVECTOR3(0, 0.f, -400.f)
#define _LIGHT_TARGET_POS			D3DXVECTOR3(0, 100.f, 0.f)
#define _CAM_TARGET_PER_CHAR_POS	D3DXVECTOR3(-20.f, 80.f, 0.f)

#define _MAX_CHAR_COUNT		7

#define _CHAR_RADIUS		250
#define _PI					3.1415927f
#define _MAX_ANGLE			360

#define _PER_ANGLE			3
#define _POS_SPACE_ANGLE	_MAX_ANGLE/_MAX_CHAR_COUNT

#define _PER_YAW_ANGLE			5
#define _PER_YAW_FIX_ANGLE		45
#define _POS_SPACE_YAW_ANGLE	_MAX_ANGLE/_PER_YAW_FIX_ANGLE
 
//char position 순으로 정렬
SelCharInfo g_SelCharInfo[_MAX_CHAR_COUNT] = {
	{ CHAR_ID_INPHYY,	MISSION_TYPE_INPHYY,	D3DXVECTOR3(120.f, 120.f, -550.f),   1.f, false, _TEX_Inphyy,   80 },
	{ CHAR_ID_MYIFEE,	MISSION_TYPE_MYIFEE,	D3DXVECTOR3(120.f, 130.f, -550.f),   1.f, false, _TEX_Myifee,   82 },
	{ CHAR_ID_TYURRU,	MISSION_TYPE_TYURRU,	D3DXVECTOR3(120.f, 120.f, -550.f),   1.f, false, _TEX_Tyurru,   83 },
	{ CHAR_ID_DWINGVATT,MISSION_TYPE_DWINGVATT,	D3DXVECTOR3(120.f, 120.f, -550.f),   1.f, false, _TEX_Dwingvatt,85 },
	{ CHAR_ID_VIGKVAGK,	MISSION_TYPE_VIGKVAGK,	D3DXVECTOR3(120.f, 120.f, -550.f), 0.65f, false, _TEX_Vigkvagk, 86 },
	{ CHAR_ID_KLARRANN,	MISSION_TYPE_KLARRANN,	D3DXVECTOR3(120.f, 120.f, -550.f),   1.f, false, _TEX_Klarrann, 84 },
	{ CHAR_ID_ASPHARR,	MISSION_TYPE_ASPHARR,	D3DXVECTOR3(120.f, 120.f, -550.f),   1.f, false, _TEX_Aspharr,  81 },
	
	
};

CFcMenuSelChar::CFcMenuSelChar(_FC_MENU_TYPE nType)
: CFcMenuForm(nType)
{
	m_dwRotateDir = NULL;
	m_nRotateYawAngle = 0;
	m_dwRotateDir = 0;
	m_nCumulationAngle = 0;
	m_fLerp = 0.f;
	m_dwBaseCircleHandle = NULL;
	m_nCurObjIndex = 0;
	m_pCharLayer = NULL;
	m_nSkinforInphyyWeapon = -1;
		
	
	//cheat..
	if(g_FCGameData.bCharAllOpen == true)
	{
		for(int i=0; i<_MAX_CHAR_COUNT; i++){
			g_SelCharInfo[i].bOpenOffset = true;
		}
	}
	else
	{
		//이전에 play한 결과에 따라 open된다.
		for(int nCharPos=0; nCharPos<_MAX_CHAR_COUNT; nCharPos++){
			CheckEnablePlayer(nCharPos);
		}
	}

	int nValue = _POS_SPACE_YAW_ANGLE;
	m_nFomationRotateAngle = (_MAX_ANGLE / _MAX_CHAR_COUNT) * (_MAX_CHAR_COUNT - m_nCurObjIndex);

	m_nSection = _SEL_SECTION_SEL_CHAR;

	m_pCharLayer = new CFcMenuSelCharLayer(this);
	AddUiLayer(m_pCharLayer, SN_SelChar0, -1, -1);
	m_pCharLayer->Initialize();
	m_pCharLayer->SetWindowAttr(XWATTR_SHOWWINDOW, false);

	CreateObject();

	m_pCharLayer->UpdateCharInfo(m_nCurObjIndex);
	SetStatus(_MS_NORMAL);
}

CFcMenuSelChar::~CFcMenuSelChar()
{
	CloseObject();
	SAFE_RELEASE_SKIN(m_nSkinforInphyyWeapon);
}

void CFcMenuSelChar::RenderProcess()
{
	switch(GetStatus())
	{
	case _MS_OPEN:
		{
			m_pCharLayer->SetColor(D3DXCOLOR(1, 1, 1, 0.f));

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
			//RenderMoveLayer(_TICK_END_OPENING_CLOSING - nTick);

			if(nTick >= _TICK_END_OPENING_CLOSING){
				SetStatus(_MS_CLOSE);
			}
			break;
		}
	case _MS_CLOSE:		break;
	}
}

void CFcMenuSelChar::RenderMoveLayer(int nTick)
{	
	//------------------------------------------------------
	//grd 0
	if(nTick <= _TICK_FADE_IN_BASE)
	{
		float fAlpha = (float)nTick / _TICK_FADE_IN_BASE;
		m_pCharLayer->SetColor(D3DXCOLOR(1, 1, 1, fAlpha));
	}
}

void CFcMenuSelChar::RenderNormal()
{	
	if(m_pCharLayer != NULL)
	{
		switch(m_nSection)
		{
		case _SEL_SECTION_SEL_CHAR:			CircleRotate(); break;
		case _SEL_SECTION_FIX_CHAR:			ObjRotateYaw(); break;
		case _SEL_SECTION_SEL_TO_FIX_CHAR:
		case _SEL_SECTION_FIX_TO_SEL_CHAR:	ChangeFormation(); break;
		}
	}
}

void CFcMenuSelChar::ProcessMsg(int nMsgAsk, BsUiHANDLE hWnd, int nMsgRsp)
{
	if(nMsgAsk == fcMT_MissionStart)
	{
		if(nMsgRsp == ID_YES)
		{
			g_MenuHandle->GetMenuWorldMapInfo()->Load();
			WorldMapPointLIST* pPointList = g_MenuHandle->GetMenuWorldMapInfo()->GetPointList();
			
			g_FCGameData.PrevStageId = STAGE_ID_NONE;
			g_FCGameData.SelStageId = (GAME_STAGE_ID)((*pPointList)[0].nId);
			strcpy(g_FCGameData.cMapFileName, (*pPointList)[0].szStage);

			SetStatus(_MS_CLOSING);
			g_MenuHandle->PostMessage(fcMSG_FIRST_MISSION_START);

			// char animation
			CFcMenu3DObjManager* p3DObjMgr = g_MenuHandle->Get3DObjectMgr();
			p3DObjMgr->SetAniIndex( m_nCurObjIndex, 1 );		// 선택 동작

			//char select에 ani가 없는 관계로 정상적인 next ani는 안된다.
			//next가 들어가면 ani가 멈춘다.
			p3DObjMgr->SetNextAniIndex( m_nCurObjIndex, 2 );	// 선택 동작 루프
		}

		g_BsUiSystem.SetFocusWindow(BsUi::BsUiGetWindow(hWnd));
	}
}

void CFcMenuSelChar::CreateObject()
{	
	CFcMenu3DObjManager* p3DObjMgr = g_MenuHandle->Get3DObjectMgr();
	if(p3DObjMgr == NULL){
		return;
	}

	g_BsKernel.chdir("Weapons");
	m_nSkinforInphyyWeapon = g_BsKernel.LoadSkin(-1, "C_LP_KF_W1.SKIN");
	BsAssert(m_nSkinforInphyyWeapon != -1);
	g_BsKernel.chdir("..");

	//fx loading
	int nFxTemplateIndex = p3DObjMgr->LoadFXTemplate("NonSelChar.bfx");
	BsAssert(nFxTemplateIndex != -1);

	CCrossVector* pCameraVector = p3DObjMgr->GetCameraCrossVector();
	pCameraVector->SetPosition(_CAMERA_POS);
	p3DObjMgr->SetCameraTargetPos(_CAMERA_TARGET_POS);

	CCrossVector* pLightVector = p3DObjMgr->GetLightCrossVector();
	pLightVector->SetPosition(_LIGHT_POS);
	p3DObjMgr->SetCameraTargetPos(_LIGHT_TARGET_POS);

	D3DXVECTOR3 vecCameraViewPos = p3DObjMgr->GetCameraTargetPos();
	for(int nCharPos=0; nCharPos<_MAX_CHAR_COUNT; nCharPos++)
	{
		int nAngle = NUM_SINTABLE / _MAX_CHAR_COUNT * nCharPos;

		D3DXVECTOR3 pos;
		pos.x = _CHAR_RADIUS * FSIN(nAngle);
		pos.y = 0.f;
		pos.z = -(_CHAR_RADIUS * FCOS(nAngle)) + vecCameraViewPos.z;
		
		int nPerRotate = 512 + (NUM_SINTABLE / _MAX_CHAR_COUNT) * nCharPos;
		if(nPerRotate > NUM_SINTABLE){
			nPerRotate -= NUM_SINTABLE;
		}
		
		DWORD dwHandle = NULL;
		if(g_SelCharInfo[nCharPos].bOpenOffset == false)	//비선택일 경우 FX
		{
			pos.y = 100.f;
			dwHandle = p3DObjMgr->AddFXObject(nFxTemplateIndex, pos, 1.f, true);
		}
		else
		{
			int nUnitSox = g_SelCharInfo[nCharPos].nSoxID;
			UnitDataInfo* pUnitData =CUnitSOXLoader::GetInstance().GetUnitData(nUnitSox);
			BsAssert( pUnitData->cSkinVariationNum > 0 && "Invalid unit type" );

			char szSkinName[_MAX_PATH];
			sprintf(szSkinName, "%s%s%d.skin", pUnitData->cSkinDir, pUnitData->cSkinFileName, 1);

			char cAniName[_MAX_PATH];
			strcpy(cAniName, pUnitData->cAniName);
			int nlen = strlen(cAniName);
			cAniName[nlen-3] = '\0';
			sprintf(cAniName, "%s_menu.ba", cAniName);

			char cUnitInfoFileName[_MAX_PATH];
			sprintf(cUnitInfoFileName, "as\\%s", pUnitData->cUnitInfoFileName );

			int nWeapon(0), nWeaponCount(1);
			if(nUnitSox == CHAR_ID_DWINGVATT){
				nWeaponCount = 2;
			}
			else if(nUnitSox == CHAR_ID_VIGKVAGK){
				nWeapon = -1;
				nWeaponCount = 0;
			}

			dwHandle = p3DObjMgr->AddObject(szSkinName,
				cAniName,
				cUnitInfoFileName,
				0, pos,
				g_SelCharInfo[nCharPos].fObjScale,
				nWeapon,
				nWeaponCount);

			if(nUnitSox == CHAR_ID_INPHYY){
				p3DObjMgr->ChangeWeapon(dwHandle, m_nSkinforInphyyWeapon);
			}
		}

		CCrossVector* pObjCross = p3DObjMgr->GetObjCrossVector(dwHandle);
		BsAssert(pObjCross && "GetObjCrossVector may return NULL");
		if (pObjCross){
			pObjCross->RotateYaw(nPerRotate);
		}
		m_3DObjHandle.push_back(dwHandle);		
	}

	int nFxTemplatePlanIndex = p3DObjMgr->LoadFXTemplate("NoneSelChar_plan.bfx");
	BsAssert(nFxTemplatePlanIndex != -1);
	D3DXVECTOR3 pos(0.f, 0.f, 0.f);
	DWORD dwHandle2 = p3DObjMgr->AddFXObject(nFxTemplatePlanIndex, pos, 1.f, true);
	CCrossVector* pObjCross = p3DObjMgr->GetObjCrossVector(dwHandle2);
	BsAssert(pObjCross && "GetObjCrossVector may return NULL");
	if (pObjCross){
		pObjCross->RotateYaw(73);
	}

	UpdateCameraLight(_CAMERA_POS, _LIGHT_POS);
	m_pCharLayer->SetWindowAttr(XWATTR_SHOWWINDOW, true);
}

void CFcMenuSelChar::CloseObject()
{
	CFcMenu3DObjManager* p3DObjMgr = g_MenuHandle->Get3DObjectMgr();
	if(p3DObjMgr != NULL){
		p3DObjMgr->RemoveAllObject();
	}
}


void CFcMenuSelChar::SetCircleRotateDir(DWORD dwKeyValue)
{
	if(m_dwRotateDir == MENU_INPUT_LEFT || m_dwRotateDir == MENU_INPUT_RIGHT){
		return;
	}

	switch(dwKeyValue)
	{
	case MENU_INPUT_LEFT:
		{
			m_nCurObjIndex--;

			if(m_nCurObjIndex < 0){
				m_nCurObjIndex = _MAX_CHAR_COUNT-1;
			}
			break;
		}
	case MENU_INPUT_RIGHT:
		{
			m_nCurObjIndex++;

			if(m_nCurObjIndex == _MAX_CHAR_COUNT){
				m_nCurObjIndex = 0;
			}
			break;
		}
	default: return;
	}

	m_dwRotateDir = dwKeyValue;
	m_pCharLayer->UpdateCharInfo(m_nCurObjIndex);
}

void CFcMenuSelChar::CircleRotate()
{	
	if(m_3DObjHandle.size() == 0){
		return;
	}

	switch(m_dwRotateDir)
	{
	case MENU_INPUT_LEFT:
		{
			m_nFomationRotateAngle += _PER_ANGLE;

			if(m_nFomationRotateAngle >= _MAX_ANGLE){
				m_nFomationRotateAngle = 0;
			}
			break;
		}
	case MENU_INPUT_RIGHT:
		{
			m_nFomationRotateAngle -= _PER_ANGLE;

			if(m_nFomationRotateAngle < 0){
				m_nFomationRotateAngle = _MAX_ANGLE-_PER_ANGLE;
			}
			break;
		}
	default: return;
	}

	UpdateCameraLight(_CAMERA_POS, _LIGHT_POS);

	int nInverseAngle = _MAX_ANGLE - m_nFomationRotateAngle;
	if(nInverseAngle == _MAX_ANGLE){
		nInverseAngle = 0;
	}
	
	int nValue = _POS_SPACE_ANGLE;
	int nShare = nInverseAngle / nValue;
	int nRest = nInverseAngle % nValue;	
	
	if( nShare == m_nCurObjIndex &&	nRest == 0){
		m_dwRotateDir = NULL;
	}
}


void CFcMenuSelChar::SetRotateYawDir(DWORD dwKeyValue)
{ 
	if(m_dwRotateDir == MENU_INPUT_LEFT || m_dwRotateDir == MENU_INPUT_RIGHT){
		return;
	}

	int nValue = _POS_SPACE_YAW_ANGLE;
	int nAngle = NUM_SINTABLE / nValue;

	switch(dwKeyValue)
	{
	case MENU_INPUT_LEFT:
		{
			m_nCumulationAngle -= nAngle;

			if(m_nCumulationAngle < 0){
				m_nCumulationAngle += NUM_SINTABLE;
			}
			break;
		}
	case MENU_INPUT_RIGHT:
		{
			m_nCumulationAngle += nAngle;

			if(m_nCumulationAngle >= NUM_SINTABLE){
				m_nCumulationAngle -= NUM_SINTABLE;
			}
			break;
		}
	default: return;
	}

	m_dwRotateDir = dwKeyValue;
	m_nRotateYawAngle = nAngle;
}

void CFcMenuSelChar::ObjRotateYaw()
{
	if(m_3DObjHandle.size() == 0){
		return;
	}

	int nRotateAngle = 0;
	switch(m_dwRotateDir)
	{
	case MENU_INPUT_LEFT: nRotateAngle = _PER_YAW_ANGLE * -1; break;
	case MENU_INPUT_RIGHT: nRotateAngle = _PER_YAW_ANGLE; break;
	default: return;
	}

	CFcMenu3DObjManager* p3DObjMgr = g_MenuHandle->Get3DObjectMgr();
	if(p3DObjMgr == NULL){
		return;
	}

	CCrossVector* pObjCross = p3DObjMgr->GetObjCrossVector(m_3DObjHandle[m_nCurObjIndex]);
	BsAssert(pObjCross && "GetObjCrossVector may return NULL");
	if (pObjCross){
		pObjCross->RotateYaw(nRotateAngle);
	}

	m_nRotateYawAngle -= _PER_YAW_ANGLE;
	if(m_nRotateYawAngle <= 0){
		m_dwRotateDir = NULL;
	}
}


void CFcMenuSelChar::ChangeFormation()
{
	if(m_nSection != _SEL_SECTION_SEL_TO_FIX_CHAR && m_nSection != _SEL_SECTION_FIX_TO_SEL_CHAR){
		return;
	}
	
	CFcMenu3DObjManager* p3DObjMgr = g_MenuHandle->Get3DObjectMgr();
	if(p3DObjMgr == NULL){
		return;
	}

	switch(m_nSection)
	{
	case _SEL_SECTION_SEL_TO_FIX_CHAR:
		{
			if(m_fLerp == 0.f)
			{
				int nValue = _POS_SPACE_YAW_ANGLE;
				m_nRotateYawAngle = NUM_SINTABLE / nValue;
			}

			m_fLerp += 0.05f;

			D3DXVECTOR3 vOut;
			D3DXVECTOR3 vCameraTargetPos = _CAMERA_TARGET_POS;
			D3DXVECTOR3 vCameraFixTargetPos = _CAM_TARGET_PER_CHAR_POS;
			D3DXVec3Lerp(&vOut, &vCameraTargetPos, &vCameraFixTargetPos, m_fLerp);
			p3DObjMgr->SetCameraTargetPos(vOut);		

			D3DXVECTOR3 vCameraPos = _CAMERA_POS;
			D3DXVECTOR3 vCameraFixPos = g_SelCharInfo[m_nCurObjIndex].vCameraFixPosPerChar;
			D3DXVec3Lerp(&vOut, &vCameraPos, &vCameraFixPos, m_fLerp);
			UpdateCameraLight(vOut, _LIGHT_POS);
			
			if(m_nRotateYawAngle > 0)
			{
				m_nRotateYawAngle -= _PER_YAW_ANGLE;
				CCrossVector* pObjCross = p3DObjMgr->GetObjCrossVector(m_3DObjHandle[m_nCurObjIndex]);
				pObjCross->RotateYaw(_PER_YAW_ANGLE);
			}
					
			if(m_fLerp >= 1.f && m_nRotateYawAngle <= 0)
			{	
				m_nSection = _SEL_SECTION_FIX_CHAR;
				m_fLerp = 0.f;
				int nValue = _POS_SPACE_YAW_ANGLE;
				m_nCumulationAngle = NUM_SINTABLE / nValue;
				m_nRotateYawAngle = 0;
			}

			break;
		}
	case _SEL_SECTION_FIX_TO_SEL_CHAR:
		{	
			if(m_nCumulationAngle != 0)
			{
				int nAngle = 0;
				if(m_nCumulationAngle <= NUM_SINTABLE / 2){
					nAngle = _PER_YAW_ANGLE * -2;
				}
				else{
					nAngle = _PER_YAW_ANGLE * 2;
				}

				m_nCumulationAngle += nAngle;
				if(m_nCumulationAngle <= 0 || m_nCumulationAngle >= NUM_SINTABLE)
				{
					m_nCumulationAngle = 0;
				}

				CCrossVector* pObjCross = p3DObjMgr->GetObjCrossVector(m_3DObjHandle[m_nCurObjIndex]);
				BsAssert(pObjCross && "GetObjCrossVector may return NULL");
				if (pObjCross){
					pObjCross->RotateYaw(nAngle);
				}
			}
			
			if(m_fLerp < 1.f)
			{
				D3DXVECTOR3 vOut;
				D3DXVECTOR3 vCameraTargetPos = _CAMERA_TARGET_POS;
				D3DXVECTOR3 vCameraFixTargetPos = _CAM_TARGET_PER_CHAR_POS;
				D3DXVec3Lerp(&vOut, &vCameraFixTargetPos, &vCameraTargetPos, m_fLerp);
				p3DObjMgr->SetCameraTargetPos(vOut);

				D3DXVECTOR3 vCameraPos = _CAMERA_POS;
				D3DXVECTOR3 vCameraFixPos = g_SelCharInfo[m_nCurObjIndex].vCameraFixPosPerChar;
				D3DXVec3Lerp(&vOut, &vCameraFixPos, &vCameraPos, m_fLerp);
				UpdateCameraLight(vOut, _LIGHT_POS);

				m_fLerp += 0.05f;
			}

			if(m_nCumulationAngle == 0 && m_fLerp >= 1.f){
				m_nSection = _SEL_SECTION_SEL_CHAR;
				m_fLerp = 0.f;
			}

			break;
		}
	}
}

void CFcMenuSelChar::UpdateCameraLight(D3DXVECTOR3 vCameraPos, D3DXVECTOR3 vLightPos)
{
	D3DXMATRIX mat;
	D3DXMatrixIdentity(&mat);
	float fAngle = 2 * PI * m_nFomationRotateAngle / _MAX_ANGLE;
	D3DXMatrixRotationY(&mat, fAngle);

	D3DXVECTOR3 vOut;
	D3DXVec3TransformCoord(&vOut, &vCameraPos, &mat);

	CFcMenu3DObjManager* p3DObjMgr = g_MenuHandle->Get3DObjectMgr();
	if(p3DObjMgr == NULL){
		return;
	}

	CCrossVector* pCameraCross = p3DObjMgr->GetCameraCrossVector();
	pCameraCross->SetPosition(vOut);
	
	D3DXVec3TransformCoord(&vOut, &vLightPos, &mat);
	CCrossVector* pLightCross = p3DObjMgr->GetLightCrossVector();
	pLightCross->SetPosition(vOut);
}

void CFcMenuSelChar::SetSection(int nSection)
{
	if(m_nSection == nSection){
		return;
	}

	m_nSection = nSection;

	CFcMenu3DObjManager* p3DObjMgr = g_MenuHandle->Get3DObjectMgr();
	if(p3DObjMgr == NULL){
		return;
	}
	
	BsUiWindow* pWindow = m_pCharLayer->GetWindow(TT_SelCharHelp);

	switch(m_nSection)
	{
	case _SEL_SECTION_SEL_CHAR: break;
	case _SEL_SECTION_SEL_TO_FIX_CHAR:
		{	
			pWindow->SetItemTextID(_TEX_HELP_SEL_CHAR_FIX);
			break;
		}
	case _SEL_SECTION_FIX_CHAR: break;
	case _SEL_SECTION_FIX_TO_SEL_CHAR:
		{
			pWindow->SetItemTextID(_TEX_HELP_SEL_CHAR);
			break;
		}
	}
}

bool CFcMenuSelChar::CheckEnablePlayer(int nCharPos)
{
	int nMissionType = g_SelCharInfo[nCharPos].nMissionType;
	for(int i=0; i<MAX_MISSION_TYPE; i++)
	{
		int nConditionMission = g_NewCharEnableCondition[nMissionType][i];
		if(nConditionMission == -1){
			continue;
		}

		int nHeroId = g_MissionToSoxID[nConditionMission];
		HeroRecordInfo* pRecordInfo = g_FCGameData.tempFixedSave.GetHeroRecordInfo(nHeroId);
		if(pRecordInfo != NULL)
		{
			switch(nHeroId)
			{
			case CHAR_ID_ASPHARR:
			case CHAR_ID_INPHYY:
				{
					switch(nMissionType)
					{
					case MISSION_TYPE_ASPHARR:
						{
							StageResultInfo* pStageInfo = pRecordInfo->GetStageResultInfo(STAGE_ID_WYANDEEK);
							if(pStageInfo != NULL && pStageInfo->nMissionClearCount > 0){
								continue;
							}
							break;
						}
					case MISSION_TYPE_MYIFEE:
						{
							StageResultInfo* pStageInfo = pRecordInfo->GetStageResultInfo(STAGE_ID_PHOLYA);
							if(pStageInfo != NULL && pStageInfo->nMissionClearCount > 0){
								continue;
							}
							break;
						}
					}
					break;
				}
			case CHAR_ID_TYURRU:
			case CHAR_ID_KLARRANN:
			case CHAR_ID_MYIFEE:
			case CHAR_ID_VIGKVAGK:
			case CHAR_ID_DWINGVATT: break;
			}

			if(pRecordInfo->bAllStageClear == true){
				continue;
			}
		}			
		
		g_SelCharInfo[nCharPos].bOpenOffset = false;
		return false;
		
	}

	g_SelCharInfo[nCharPos].bOpenOffset = true;
	return true;
}


//-----------------------------------------------------------------------------------------------------
void CFcMenuSelCharLayer::Initialize()
{
	int nTexID = -1;
	CFcMenu3DObjManager* p3DObjMgr = g_MenuHandle->Get3DObjectMgr();
	if(p3DObjMgr != NULL){
		nTexID = p3DObjMgr->GetTextureID();
	}

	ImageCtrlInfo ImgCtrlInfo;

	// 소스 Rect 지정
	ImgCtrlInfo.nTexId = nTexID; //3D object에 대한 RTT를 얻어야 한다.
	ImgCtrlInfo.u1 = 1;
	ImgCtrlInfo.v1 = 1;
	ImgCtrlInfo.u2 = 1479;
	ImgCtrlInfo.v2 = 839;
	
	BsUiImageCtrl* pImgCtrl = (BsUiImageCtrl*)GetWindow(IM_SelectChar);
	pImgCtrl->SetImageInfo(&ImgCtrlInfo);
	pImgCtrl->SetWindowPos(0, 0);
	pImgCtrl->SetWindowSize(_SCREEN_WIDTH, _SCREEN_HEIGHT);
	pImgCtrl->SetRenderTargetTexture(true);

	ShowCharInfo(false);
	g_BsUiSystem.SetFocusWindow(GetWindow(BT_SelectChar));
}

DWORD CFcMenuSelCharLayer::ProcMessage(xwMessageToken* pMsgToken)
{
	BsAssert(pMsgToken);

	BsUiCLASS hClass = BsUi::BsUiGetClass(pMsgToken->hWnd);

	switch(pMsgToken->message)
	{
	case XWMSG_KEYDOWN:
		{
			if(hClass == BT_SelectChar){
				OnKeyDownBtSelectChar(pMsgToken);
			}
		}
		break;
	case XWMSG_BN_CLICKED:
		{
			;
		}
		break;
	}

	return BsUiWindow::ProcMessage(pMsgToken);
}

void CFcMenuSelCharLayer::OnKeyDownBtSelectChar(xwMessageToken* pMsgToken)
{
	switch(pMsgToken->lParam)
	{
	case MENU_INPUT_LEFT:
	case MENU_INPUT_RIGHT:
		{
			switch(m_pMenu->GetSection())
			{
			case _SEL_SECTION_SEL_CHAR:	m_pMenu->SetCircleRotateDir(pMsgToken->lParam); break;
			case _SEL_SECTION_FIX_CHAR: break;
			}
		}
		break;
	case MENU_INPUT_START:
		{
			bool bOpen = g_SelCharInfo[m_pMenu->GetCurObjIndex()].bOpenOffset;
			if(bOpen == false){
				break;
			}

			//여기서 못 바꾸게 해야 한다.
			if(m_pMenu->GetSection() == _SEL_SECTION_FIX_CHAR)
			{
				if(m_pMenu->GetRotateYawDir() == NULL)
				{	
					DWORD dwMissionData = BsUi::BsUiGetWindow(pMsgToken->hWnd)->GetItemData();
					BsAssert(dwMissionData < MAX_MISSION_TYPE);
					g_FCGameData.nPlayerType = (GAME_MISSION_TYPE)dwMissionData;

					g_MenuHandle->PostMessage(fcMSG_MESSAGEBOX_START,
						fcMT_MissionStart,
						(DWORD)m_pMenu, pMsgToken->hWnd);
				}
			}
			break;
		}
	case MENU_INPUT_A:
		{
			bool bOpen = g_SelCharInfo[m_pMenu->GetCurObjIndex()].bOpenOffset;
			if(bOpen == false){
				break;
			}

			//여기서 못 바꾸게 해야 한다.
			if(m_pMenu->GetSection() == _SEL_SECTION_SEL_CHAR)
			{
				DWORD dwMissionData = BsUi::BsUiGetWindow(pMsgToken->hWnd)->GetItemData();
				if(dwMissionData < MAX_MISSION_TYPE && m_pMenu->GetRotateYawDir() == NULL)
				{
					BsAssert(GetWindow(TT_SelectCharInfo));
					ShowCharInfo(true);
					m_pMenu->SetSection(_SEL_SECTION_SEL_TO_FIX_CHAR);
				}
			}
			
			break;
		}
	case MENU_INPUT_B:
		{
			switch(m_pMenu->GetSection())
			{
			case _SEL_SECTION_SEL_CHAR:
				{
					if(m_pMenu->GetRotateYawDir() == NULL)
					{
						m_pMenu->SetStatus(_MS_CLOSING);
						g_MenuHandle->PostMessage(fcMSG_TITLE_START);
					}
				}
				break;
			case _SEL_SECTION_FIX_CHAR:
				{
					if(m_pMenu->GetRotateYawDir() == NULL){
						m_pMenu->SetSection(_SEL_SECTION_FIX_TO_SEL_CHAR);
						m_pMenu->SetRotateYawDir(NULL);
						ShowCharInfo(false);
					}
				}
				break;
			}
			break;
		}
	}
}

void CFcMenuSelCharLayer::UpdateCharInfo(int nCharPos)
{
	bool bOpen = g_SelCharInfo[nCharPos].bOpenOffset;
	if(bOpen == false)
	{	
		GetWindow(BT_SelectChar)->SetWindowAttr(XWATTR_SHOWWINDOW, false);
		return;
	}else{
		GetWindow(BT_SelectChar)->SetWindowAttr(XWATTR_SHOWWINDOW, true);
	}

	int nNameTextID = g_SelCharInfo[nCharPos].nNameTextID;
	int nInfoTextID = g_SelCharInfo[nCharPos].nInfoTextID;
	DWORD dwMissionData = g_SelCharInfo[nCharPos].nMissionType;
	
	// name
	GetWindow(BT_SelectChar)->SetItemTextID(nNameTextID);
	GetWindow(BT_SelectChar)->SetItemData(dwMissionData);

	// info
	GetWindow(TT_SelectCharInfo)->SetItemTextID(nInfoTextID);
}

void CFcMenuSelCharLayer::ShowCharInfo(bool bShow)
{
	GetWindow(TT_SelectCharInfo)->SetWindowAttr(XWATTR_SHOWWINDOW, bShow);
	GetWindow(IM_SelChar_info_left_up)->SetWindowAttr(XWATTR_SHOWWINDOW, bShow);
	GetWindow(IM_SelChar_info_center_up)->SetWindowAttr(XWATTR_SHOWWINDOW, bShow);
	GetWindow(IM_SelChar_info_right_up)->SetWindowAttr(XWATTR_SHOWWINDOW, bShow);
	GetWindow(IM_SelChar_info_left_center)->SetWindowAttr(XWATTR_SHOWWINDOW, bShow);
	GetWindow(IM_SelChar_info_center_center)->SetWindowAttr(XWATTR_SHOWWINDOW, bShow);
	GetWindow(IM_SelChar_info_right_center)->SetWindowAttr(XWATTR_SHOWWINDOW, bShow);
	GetWindow(IM_SelChar_info_left_down)->SetWindowAttr(XWATTR_SHOWWINDOW, bShow);
	GetWindow(IM_SelChar_info_center_down)->SetWindowAttr(XWATTR_SHOWWINDOW, bShow);
	GetWindow(IM_SelChar_info_right_down)->SetWindowAttr(XWATTR_SHOWWINDOW, bShow);

	GetWindow(BT_SelectChar)->SetWindowAttr(XWATTR_SHOWWINDOW, !bShow);
}