#include "stdafx.h"
#include "FcAbility.h"
#include "BSKernel.h"
#include "Input.h"
#include "InputPad.h"
#include "DebugUtil.h"
#include "FcWorld.h"
#include "FcInterfaceDataDefine.h"
#include "FcInterfaceManager.h"
#include "FcTroopManager.h"
#include "FcTroopObject.h"
#include "FcGlobal.h"
#include "FcCameraObject.h"
#include "TextTable.h"
#include "FcHeroObject.h"
#include "FcSoundManager.h"
#include "FcRealtimeMovie.h"
#include "FcItem.h"

#include "BsUiUVManager.h"
#include ".\\data\\Menu\\BsUiUVID.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG

#define GUARDIAN_ANSWER_DELAY_TICK		20

#define ABILITY_DELAY_TICK				0
#define _GUARDIAN_MAX_COUNT				2

#define _GUARDIAN_DECAL_TICK			0

#define _CMD_ANSWER_TICK				70

#define _GUARDIAN_ICON_BASE_POS_X		337
#define _GUARDIAN_ICON_BASE_POS_Y		618
#define _GUARDIAN_ICON_BASE_POS_X_GAP	76
#define _GUARDIAN_ICON_POS_X_GAP		11

#define _DURATION_ITEM_SHOW_COUNT		5
#define _DURATION_ITEM_POS_X			530
#define _DURATION_ITEM_POS_Y			626
#define _DURATION_ITEM_POS_X_GAP		46


//-----------------------------------------------------------------------------------------------





struct GUARDIAN_VOICEQUE_INFO
{
	char cName[32];
	int nNum;
};


GUARDIAN_VOICEQUE_INFO g_HeroGuardianVoiceQueInfo[4][7] = {
	// 장착
	{
		{ "A_SYS_SC0", 3 },	
		{ "I_SYS_SC0", 5 },
		{ "K_SYS_SC0", 3 },
		{ "", 0 },			// 비그바그는 호위병 처리 없음
		{ "M_SYS_SC0", 3 },
		{ "D_SYS_SC0", 4 },
		{ "T_SYS_SC0", 3 }
	},
	// 해제
	{
		{ "A_SYS_TM0", 5 },	
		{ "I_SYS_TM0", 4 },
		{ "K_SYS_TM0", 2 },
		{ "", 0 },
		{ "M_SYS_TM0", 4 },
		{ "D_SYS_TM0", 3 },
		{ "T_SYS_TM0", 4 }
	},
	// 공격
	{
		{ "A_SYS_KM0", 4 },	
		{ "I_SYS_KM0", 6 },
		{ "K_SYS_KM0", 4 },
		{ "", 0 },
		{ "M_SYS_KM0", 4 },
		{ "D_SYS_KM0", 4 },
		{ "T_SYS_KM0", 4 }
	},
	// 방어
	{
		{ "A_SYS_BM0", 5 },	
		{ "I_SYS_BM0", 5 },
		{ "K_SYS_BM0", 3 },
		{ "", 0 },
		{ "M_SYS_BM0", 5 },
		{ "D_SYS_BM0", 3 },
		{ "T_SYS_BM0", 4 }
	}
};

GUARDIAN_VOICEQUE_INFO g_GuardianVoiceQueInfo[4] =
{
	{ "GEH_SYS_SC0", 5 },
	{ "GEH_SYS_ANT0", 4 },
	{ "GEH_SYS_KM0", 5 },
	{ "GEH_SYS_BM0", 5 },
};




FcAbilInterface::FcAbilInterface(CFcPortrait *pPortrait)
{
	m_bPlay = false;
	m_pEnemyTarget = NULL;
	m_nDecalTexIndex = -1;
	m_nDecalTargetTexIndex = -1;

	m_pPortrait = NULL;
	m_nCmdPortraitId = -1;
	m_nAnswerPortraitId = -1;

	memset(m_cCmdText, -1, sizeof(char)*256);
	memset(m_cAnswerText, -1, sizeof(char)*256);

	m_nGuadianSignTick = 0;

	m_pPortrait = pPortrait;

	m_pEnemyTarget = new CFcMenu3DObject();
	m_pEnemyTarget->CreateObject(_ENEMY_TARGET_SKIN, _ENEMY_TARGET_ANI, NULL, 0, D3DXVECTOR3(0.f, 0.f, 0.f), 2.f);
	g_BsKernel.SendMessage(m_pEnemyTarget->GetEngineIndex(), BS_SHADOW_CAST);

	g_BsKernel.chdir("interface");
	m_nDecalTexIndex = g_BsKernel.LoadTexture( _GUARDIAN_DECAL_TEXTURE );
	m_nDecalTargetTexIndex = g_BsKernel.LoadTexture( _GUARDIAN_TARGET_DECAL_TEXTURE );
	g_BsKernel.chdir("..");

	for( int i=0; i<4; i++ )
	{
		m_nPlayerVoiceHandle[i] = -1;
		m_nGuardianVoiceHandle[i] = -1;
	}
}

FcAbilInterface::~FcAbilInterface()
{
	if(m_pEnemyTarget != NULL)
	{
		delete m_pEnemyTarget;
		m_pEnemyTarget = NULL;
	}
}



void FcAbilInterface::ReleaseData()
{
	if( m_nDecalTexIndex != -1 )
		g_BsKernel.ReleaseTexture( m_nDecalTexIndex );

	if( m_nDecalTargetTexIndex != -1 )
		g_BsKernel.ReleaseTexture( m_nDecalTargetTexIndex );

	m_nDecalTexIndex = -1;
	m_nDecalTargetTexIndex = -1;
}

void FcAbilInterface::DrawGuardianTarget()
{
	CFcTroopObject* pTargetObj = g_FcWorld.GetTroopmanager()->GetGuardianTarget();
	if(pTargetObj == NULL){
		return;
	}

	// 2P인 경우 처리 없음
	for(int j=0; j<_GUARDIAN_MAX_COUNT; j++)
	{
		CFcTroopObject* pGuardianObj = g_FcWorld.GetTroopmanager()->GetGuardianTroop(j);
		if(pGuardianObj != NULL && pGuardianObj == pTargetObj){
			return;
		}
	}

	pTargetObj->ShowObjectBaseDecal(m_nDecalTargetTexIndex, _GUARDIAN_DECAL_TICK, 1);
}

void FcAbilInterface::DrawEnemyTarget()
{
	CFcTroopObject* pTargetObj = g_FcWorld.GetTroopmanager()->GetLockOnTarget();
	if(pTargetObj == NULL){
		return;
	}

	CCrossVector* pCross = m_pEnemyTarget->GetCrossVector();
//	D3DXVECTOR3 vPos = pTargetObj->GetPos();
	D3DXVECTOR2 vPos2;
	bool bRet = pTargetObj->GetUnitCenterPos( &vPos2 );
	if( bRet == false )
		return;

	D3DXVECTOR3 vPos;
	vPos.x = vPos2.x;
	vPos.z = vPos2.y;
	vPos.y = g_BsKernel.GetLandHeight( vPos2.x, vPos2.y );
	vPos.y += 300.f;
	pCross->SetPosition(vPos);
	m_pEnemyTarget->Update();

	if( (pCross->m_PosVector.x == pCross->m_PosVector.x) == false )
	{
		BsAssert(0);
	}
}

int g_AttackIconFrame[4] = 
{
	_UV_Guardian_Sword_Attack1,
	_UV_Guardian_Sword_Attack2,
	_UV_Guardian_Sword_Attack1,
	_UV_Guardian_Sword_Attack2,
};

int g_ReadyIconFrame[4] = 
{	
	_UV_Guardian_Sword_Ready1,
	_UV_Guardian_Sword_Ready2,
	_UV_Guardian_Sword_Ready3,
	_UV_Guardian_Sword_Ready2,
};

void FcAbilInterface::DrawGuardianList()
{
	for(int i=0; i<_GUARDIAN_MAX_COUNT; i++)
	{
		int nX = _GUARDIAN_ICON_BASE_POS_X + _GUARDIAN_ICON_BASE_POS_X_GAP * i;
		int nY = _GUARDIAN_ICON_BASE_POS_Y;

		CFcTroopObject* pTroopObj = g_FcWorld.GetTroopmanager()->GetGuardianTroop( i );
		
		if(pTroopObj == NULL)
		{
			int nUVId = _UV_Guardian_None_L;
			if(i > 0){
				nUVId = _UV_Guardian_None_R;
			}

			UVImage*  pImg = g_UVMgr.GetUVImage(nUVId);
			BsAssert(pImg != NULL);
			g_BsKernel.DrawUIBox_s(_Ui_Mode_Image,
				nX, nY, pImg->u2 - pImg->u1, pImg->v2 - pImg->v1,
				0.f, D3DXCOLOR(1, 1, 1, 1),
				0.f, pImg->nTexId,
				pImg->u1, pImg->v1, pImg->u2, pImg->v2);
		}
		else
		{
			int nHP = pTroopObj->GetHP();
			int nMaxHP = pTroopObj->GetMaxHP();
			BsAssert(nMaxHP >= nHP);
			float fHp = (float)nHP / nMaxHP;
			int nStep = (int)(fHp / 0.25f);
			if(nStep == 4){
				nStep = 3;
			}
			UVImage* pBaseImg = g_UVMgr.GetUVImage(_UV_Guardian_Red_Base + nStep);
			BsAssert(pBaseImg != NULL);
			g_BsKernel.DrawUIBox_s(_Ui_Mode_Image,
				nX, nY, pBaseImg->u2 - pBaseImg->u1, pBaseImg->v2 - pBaseImg->v1,
				0.f, D3DXCOLOR(1, 1, 1, 1),
				0.f, pBaseImg->nTexId,
				pBaseImg->u1, pBaseImg->v1, pBaseImg->u2, pBaseImg->v2);

			if(pTroopObj->IsEnable() == false){
				continue;
			}

			GameObjHandle hHandle = pTroopObj->GetFirstLiveUnit();
			if( hHandle == NULL ){
				continue;
			}

			int nSoxId = hHandle->GetUnitSoxIndex();

			int nUVID = -1;
			switch(GetGuardianType(nSoxId))
			{
			case GT_ONE_HAND:	nUVID = _UV_Guardian_Sword_Attack1; break;
			case GT_TWO_HAND:	nUVID = _UV_Guardian_TH_Sword_Attack1; break;
			case GT_SPEAR:		nUVID = _UV_Guardian_Spear_Attack1; break;
			case GT_RANGE:		nUVID = _UV_Guardian_Arrow_Attack1; break;
			default: BsAssert(0);
			}

			switch(pTroopObj->GetState())
			{
			case TROOPSTATE_RANGE_ATTACK:
			case TROOPSTATE_MELEE_ATTACK:
			case TROOPSTATE_MOVE_ATTACK:
				{
					int nFrame = (GetProcessTick() % 16)/4;
					nUVID = nUVID  + (g_AttackIconFrame[nFrame] - _UV_Guardian_Sword_Attack1);
				}
				break;
			default:
				{
					int nFrame = (GetProcessTick() % 16)/4;
					nUVID += _UV_Guardian_Sword_Ready1 - _UV_Guardian_Sword_Attack1;
					nUVID = nUVID + (g_ReadyIconFrame[nFrame] - _UV_Guardian_Sword_Ready1);
				}
				break;
			}

			UVImage* pImg = g_UVMgr.GetUVImage(nUVID);

			BsAssert(pImg != NULL);
			g_BsKernel.DrawUIBox_s(_Ui_Mode_Image,
				nX + _GUARDIAN_ICON_POS_X_GAP, nY,
				pImg->u2 - pImg->u1, pImg->v2 - pImg->v1,
				0.f, D3DXCOLOR(1, 1, 1, 1),
				0.f, pImg->nTexId,
				pImg->u1, pImg->v1, pImg->u2, pImg->v2);

			//object에 직접 그린다.
			pTroopObj->ShowObjectBaseDecal(m_nDecalTexIndex, _GUARDIAN_DECAL_TICK, 1);
		}
	}
}

void FcAbilInterface::DrawDurationItem()
{
	int nCount = g_FcItemManager.GetDurationItemCount();
	if(nCount == 0){
		return;
	}

	for(int i=0; i<nCount; i++)
	{
		if(i == _DURATION_ITEM_SHOW_COUNT){
			break;
		}
				
		CFcItemManager::ITEM_DURATION_INFO* pDuration = NULL;
		pDuration = g_FcItemManager.GetDurationItem(i);
		BsAssert(pDuration);

		int nX = _DURATION_ITEM_POS_X + _DURATION_ITEM_POS_X_GAP * i;
		int nY = _DURATION_ITEM_POS_Y;	                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                        

		int nUVId = -1;
		switch(pDuration->nItemSoxID)
		{
		case ITEM_ORB_MARK:			nUVId = _UV_Item_OrbMark_0; break;
		case ITEM_GRAIL:			nUVId = _UV_Item_Grail_0; break;
		case ITEM_WEAPON_MARK:		nUVId = _UV_Item_WeaponMark_0; break;
		case ITEM_ARMOR_MARK:		nUVId = _UV_Item_ArmorMark_0; break;
		case ITEM_FLYING_SHOES_MARK:nUVId = _UV_Item_FlyingShoes_0; break;
		case ITEM_ANGLE_WING_MARK:	nUVId = _UV_Item_AngelWing_0; break;
		case ITEM_EXP_2_MARK:		nUVId = _UV_Item_Exp2Mark_0; break;
		case ITEM_WHISTLE:			nUVId = _UV_Item_Whistle_0; break;
		case ITEM_BENUS_STATUE:		nUVId = _UV_Item_VenusStatue_0; break;
		case ITEM_WHITE_WOOD:		break;
		default:	break;
		}

		if(nUVId == -1){
			continue;
		}

		//base
		UVImage* pImg = g_UVMgr.GetUVImage(nUVId);
		
		if (pImg == NULL) {
			DebugString("Unable to find UV ID for ability: %i", nUVId);
			return;
		}

		BsAssert(pImg != NULL);
		g_BsKernel.DrawUIBox_s(_Ui_Mode_Image,
			nX, nY,
			pImg->u2 - pImg->u1, pImg->v2 - pImg->v1,
			0.f, D3DXCOLOR(1, 1, 1, 1),
			0.f, pImg->nTexId,
			pImg->u1, pImg->v1, pImg->u2, pImg->v2);


		//light
		int nRemainTick = pDuration->nDestTime - pDuration->nPastTime;
		int nValue = 1;
		if(nRemainTick < 200){
			nValue = 8;
		}else if(nRemainTick < 400){
			nValue = 16;
		}else{
			nValue = 32;
		}
		float fAlpha =(GetProcessTick() % nValue)/(nValue * 1.f);
		
		UVImage* pLightImg = g_UVMgr.GetUVImage(nUVId + 1);
		BsAssert(pLightImg != NULL);
		g_BsKernel.DrawUIBox_s(_Ui_Mode_Image,
			nX, nY,
			pLightImg->u2 - pLightImg->u1, pLightImg->v2 - pLightImg->v1,
			0.f, D3DXCOLOR(1, 1, 1, fAlpha),
			0.f, pLightImg->nTexId,
			pLightImg->u1, pLightImg->v1, pLightImg->u2, pLightImg->v2);

	}
}

void FcAbilInterface::ProcessGuardianVoice()
{
	if(g_InterfaceManager.IsShowLetterBox() == true){
		return;
	}
	if( g_pFcRealMovie && g_pFcRealMovie->IsPlay() == true ){
		return;
	}

	for( int i=0; i<4; i++ )
	{
		if( m_nGuardianVoiceHandle[i] != -1 )
		{
			if( g_pSoundManager->IsOwner( this, m_nGuardianVoiceHandle[i] ) == false )
				m_nGuardianVoiceHandle[i] = -1;
		}
		if( m_nPlayerVoiceHandle[i] != -1 )
		{
			if( g_pSoundManager->IsOwner( this, m_nPlayerVoiceHandle[i] ) == false )
				m_nPlayerVoiceHandle[i] = -1;
		}
	}
	
	int nCnt = m_vecGuardianVoice.size();
	for( int i=0; i<nCnt; i++ )
	{
		VOICEQUE_INFO* pInfo = &m_vecGuardianVoice[i];
		if( pInfo->nCurTick == -1 )
		{
			if( pInfo->nPlayerSoundID != m_nPlayerVoiceHandle[pInfo->Type] )
			{
				pInfo->nCurTick = GUARDIAN_ANSWER_DELAY_TICK;
			}
		}
		else
		{
			if( pInfo->nCurTick <= 0 )
			{
				if( m_nGuardianVoiceHandle[pInfo->Type] == -1 && !IsPause())
					m_nGuardianVoiceHandle[pInfo->Type] = g_pSoundManager->PlaySystemSound( SB_VOICE_IN_GAME, pInfo->cName, SC_COMMON, NULL, this );

				m_vecGuardianVoice.erase( m_vecGuardianVoice.begin() + i );
				--i;
				--nCnt;
				continue;
				
			}
			--pInfo->nCurTick;
		}
	}		
}


void FcAbilInterface::Process()
{
	m_pEnemyTarget->Process();

	ProcessGuardianVoice();
}

void FcAbilInterface::Update()
{
	if(m_bShow == false){
		return;
	}
	
	if(IsEnableGuardian())
	{
		DrawGuardianTarget();

		DrawEnemyTarget();

		DrawGuardianList();

		DrawGuardianSign();
	}

	DrawDurationItem();
}

void FcAbilInterface::KeyInput(int nPlayerID, int KeyType,int PressTick)
{
	if(g_MenuHandle->GetCurMenuType() == _FC_MENU_DEBUG){
		return;
	}

	if(g_FCGameData.State != GAME_STATE_LOOP_STAGE){
		return;
	}
	
	if(IsEnableGuardian() == false){
		return;
	}

	CameraObjHandle CamHandle = CFcBaseObject::GetCameraObjectHandle( 0 );
	if( CamHandle->IsEditMode() )
		return;

	if( nPlayerID != 0 )		// 나랑 관계 없는 입력
		return;

	if( ::IsPause() )
		return;

	if( KeyType == PAD_INPUT_BLACK && PressTick == 1 )		// L shoulder
	{
		CFcTroopObject* pTroopObj = g_FcWorld.GetTroopmanager()->GetGuardianTroop(0);
		bool bRet = g_FcWorld.CmdRegisterGuardian( 0 );

		if( bRet )
		{
			if(pTroopObj != NULL){
				// 뺀다
				if( m_nPlayerVoiceHandle[GUARDIAN_REMOVE] == -1 )
				{
					char cStr[256];
					GetGuardianVoiceQueStr( GUARDIAN_REMOVE, cStr );
					if( cStr[0] != NULL && IsEnablePlayerVoice() && !IsPause())
						m_nPlayerVoiceHandle[GUARDIAN_REMOVE] = g_pSoundManager->PlaySystemSound( SB_VOICE_IN_GAME, cStr, SC_COMMON, NULL, this );

					if( m_nPlayerVoiceHandle[GUARDIAN_REMOVE] != -1 )
					{
						VOICEQUE_INFO Info;
						int nRandomIndex = Random( g_GuardianVoiceQueInfo[GUARDIAN_REMOVE].nNum ) + 1;
						sprintf_s( Info.cName, 32, "%s%d", g_GuardianVoiceQueInfo[GUARDIAN_REMOVE].cName, nRandomIndex );
						Info.nCurTick = -1;
						Info.Type = GUARDIAN_REMOVE;
						Info.nPlayerSoundID = m_nPlayerVoiceHandle[GUARDIAN_REMOVE];
						m_vecGuardianVoice.push_back( Info );
					}
				}
			}
			else
			{
				// 등록한다
				if( m_nPlayerVoiceHandle[GUARDIAN_ADD] == -1)
				{
					char cStr[256];
					GetGuardianVoiceQueStr( GUARDIAN_ADD, cStr );
					if( cStr[0] != NULL && IsEnablePlayerVoice() && !IsPause())
						m_nPlayerVoiceHandle[GUARDIAN_ADD] = g_pSoundManager->PlaySystemSound( SB_VOICE_IN_GAME, cStr, SC_COMMON, NULL, this );

					if( m_nPlayerVoiceHandle[GUARDIAN_ADD] != -1 )
					{
						VOICEQUE_INFO Info;
						int nRandomIndex = Random( g_GuardianVoiceQueInfo[GUARDIAN_ADD].nNum ) + 1;
						sprintf_s( Info.cName, 32, "%s%d", g_GuardianVoiceQueInfo[GUARDIAN_ADD].cName, nRandomIndex );
						Info.nCurTick = -1;
						Info.Type = GUARDIAN_ADD;
						Info.nPlayerSoundID = m_nPlayerVoiceHandle[GUARDIAN_ADD];

						m_vecGuardianVoice.push_back( Info );
					}
					if(!IsPause()) {					
						g_pSoundManager->PlaySystemSound( SB_COMMON, "SYS_FORCE_ASSIGN" );
					}
				}
			}
		}
	}
	else if( KeyType == PAD_INPUT_WHITE && PressTick == 1 )		// R shoulder
	{
		CFcTroopObject* pTroopObj = g_FcWorld.GetTroopmanager()->GetGuardianTroop( 1 );
		bool bRet = g_FcWorld.CmdRegisterGuardian( 1 );

		if( bRet )
		{
			if(pTroopObj != NULL){
				// 뺀다
				if( m_nPlayerVoiceHandle[GUARDIAN_REMOVE] == -1)
				{
					char cStr[256];
					GetGuardianVoiceQueStr( GUARDIAN_REMOVE, cStr );
					if( cStr[0] != NULL && IsEnablePlayerVoice() && !IsPause())
						m_nPlayerVoiceHandle[GUARDIAN_REMOVE] = g_pSoundManager->PlaySystemSound( SB_VOICE_IN_GAME, cStr, SC_COMMON, NULL, this );

					if( m_nPlayerVoiceHandle[GUARDIAN_REMOVE] != -1 )
					{
						VOICEQUE_INFO Info;
						int nRandomIndex = Random( g_GuardianVoiceQueInfo[GUARDIAN_REMOVE].nNum ) + 1;
						sprintf_s( Info.cName, 32, "%s%d", g_GuardianVoiceQueInfo[GUARDIAN_REMOVE].cName, nRandomIndex );
						Info.nCurTick = -1;
						Info.Type = GUARDIAN_REMOVE;
						Info.nPlayerSoundID = m_nPlayerVoiceHandle[GUARDIAN_REMOVE];
						m_vecGuardianVoice.push_back( Info );
					}
				}
			}
			else
			{
				// 등록한다
				if( m_nPlayerVoiceHandle[GUARDIAN_ADD] == -1 )
				{
					char cStr[256];
					GetGuardianVoiceQueStr( GUARDIAN_ADD, cStr );
					if( cStr[0] != NULL && IsEnablePlayerVoice() && !IsPause())
						m_nPlayerVoiceHandle[GUARDIAN_ADD] = g_pSoundManager->PlaySystemSound( SB_VOICE_IN_GAME, cStr, SC_COMMON, NULL, this );

					if( m_nPlayerVoiceHandle[GUARDIAN_ADD] != -1 )
					{
						VOICEQUE_INFO Info;
						int nRandomIndex = Random( g_GuardianVoiceQueInfo[GUARDIAN_ADD].nNum ) + 1;
						sprintf_s( Info.cName, 32, "%s%d", g_GuardianVoiceQueInfo[GUARDIAN_ADD].cName, nRandomIndex );
						Info.nCurTick = -1;
						Info.Type = GUARDIAN_ADD;
						Info.nPlayerSoundID = m_nPlayerVoiceHandle[GUARDIAN_ADD];
						m_vecGuardianVoice.push_back( Info );
					}
					if(!IsPause()){
						g_pSoundManager->PlaySystemSound( SB_COMMON, "SYS_FORCE_ASSIGN");
					}
				}
			}
		}
	}

	if( KeyType == PAD_INPUT_UP && PressTick == 1 )
	{
		g_FcWorld.CmdGuardianAttack();

		CFcTroopObject* pTroopObj = g_FcWorld.GetTroopmanager()->GetGuardianTroop( 0 );
		if(pTroopObj == NULL)
		{
			pTroopObj = g_FcWorld.GetTroopmanager()->GetGuardianTroop( 1 );
		}
		if(pTroopObj != NULL)
		{
			// 공격한다
			if( m_nPlayerVoiceHandle[GUARDIAN_ATTACK] == -1)
			{
				char cStr[256];
				GetGuardianVoiceQueStr( GUARDIAN_ATTACK, cStr );
				if( cStr[0] != NULL && IsEnablePlayerVoice() && !IsPause())
					m_nPlayerVoiceHandle[GUARDIAN_ATTACK] = g_pSoundManager->PlaySystemSound( SB_VOICE_IN_GAME, cStr, SC_COMMON, NULL, this );

				if( m_nPlayerVoiceHandle[GUARDIAN_ATTACK] != -1 )
				{
					VOICEQUE_INFO Info;
					int nRandomIndex = Random( g_GuardianVoiceQueInfo[GUARDIAN_ATTACK].nNum ) + 1;
					sprintf_s( Info.cName, 32, "%s%d", g_GuardianVoiceQueInfo[GUARDIAN_ATTACK].cName, nRandomIndex );
					Info.nCurTick = -1;
					Info.Type = GUARDIAN_ATTACK;
					Info.nPlayerSoundID = m_nPlayerVoiceHandle[GUARDIAN_ATTACK];
					m_vecGuardianVoice.push_back( Info );
				}

				if(!IsPause())
				{
					g_pSoundManager->PlaySystemSound( SB_COMMON, "SYS_FORCE_INSTRUCTION");
				}
			}
		}
	}
	else if( KeyType == PAD_INPUT_DOWN && PressTick == 1 )
	{
		g_FcWorld.CmdGuardianDefense();

		CFcTroopObject* pTroopObj = g_FcWorld.GetTroopmanager()->GetGuardianTroop( 0 );
		if(pTroopObj == NULL)
		{
			pTroopObj = g_FcWorld.GetTroopmanager()->GetGuardianTroop( 1 );
		}
		if(pTroopObj != NULL)
		{
			// 방어한다.
			if( m_nPlayerVoiceHandle[GUARDIAN_DEFENSE] == -1)
			{
				char cStr[256];
				GetGuardianVoiceQueStr( GUARDIAN_DEFENSE, cStr );
				if( cStr[0] != NULL && IsEnablePlayerVoice() && !IsPause() ){
					m_nPlayerVoiceHandle[GUARDIAN_DEFENSE] = g_pSoundManager->PlaySystemSound( SB_VOICE_IN_GAME, cStr, SC_COMMON, NULL, this );
				}

				if(!IsPause()){
					g_pSoundManager->PlaySystemSound( SB_COMMON, "SYS_FORCE_INSTRUCTION");
				}

				if( m_nPlayerVoiceHandle[GUARDIAN_DEFENSE] != -1 )
				{
					VOICEQUE_INFO Info;
					int nRandomIndex = Random( g_GuardianVoiceQueInfo[GUARDIAN_DEFENSE].nNum ) + 1;
					sprintf_s( Info.cName, 32, "%s%d", g_GuardianVoiceQueInfo[GUARDIAN_DEFENSE].cName, nRandomIndex );
					Info.nCurTick = -1;
					Info.Type = GUARDIAN_DEFENSE;
					Info.nPlayerSoundID = m_nPlayerVoiceHandle[GUARDIAN_DEFENSE];
					m_vecGuardianVoice.push_back( Info );
				}
			}
		}
	}
}


void FcAbilInterface::SetGuardianSign(CFcTroopObject* pTroopObj, int nState, bool bLeft)
{
	return;

	if(m_nGuadianSignTick > 0){
		return;
	}
		
	if(pTroopObj == NULL){
		return;
	}

	HeroObjHandle Handle = g_FcWorld.GetHeroHandle();
	BsAssert(Handle != NULL);
	int nUnitId = Handle->GetUnitSoxIndex();
	switch(nUnitId)
	{
	case 0: m_nCmdPortraitId = PORTRAIT_Aspharr; break;
	case 1: m_nCmdPortraitId = PORTRAIT_Inphyy; break;
	case 2: m_nCmdPortraitId = PORTRAIT_Myifee; break;
	case 3: m_nCmdPortraitId = PORTRAIT_Tyurru; break;
	case 4: m_nCmdPortraitId = PORTRAIT_Inphyy; break;
	case 5: m_nCmdPortraitId = PORTRAIT_Inphyy; break;
	case 6: m_nCmdPortraitId = PORTRAIT_Inphyy; break;
	default:
		{
			BsAssert(0);
		}
	}
	
	m_nAnswerPortraitId = PORTRAIT_HolyKnight;

	//nState, bLeft : text얻기
	int nTexId = -1;
	switch(nState)
	{
	case _GUADIAN_IN:
		{
			if(bLeft){
                nTexId = _TEX_CMD_GUADIAN_L_IN;
			}else{
				nTexId = _TEX_CMD_GUADIAN_R_IN;
			}
			break;
		}
	case _GUADIAN_OUT:
		{
			if(bLeft){
				nTexId = _TEX_CMD_GUADIAN_L_OUT;
			}else{
				nTexId = _TEX_CMD_GUADIAN_R_OUT;
			}
			break;
		}
	case _GUADIAN_ATTACK:	nTexId = _TEX_CMD_GUADIAN_ATTACK; break;
	case _GUADIAN_DEFANCE:	nTexId = _TEX_CMD_GUADIAN_DEFENSE; break;
	default:
		{
			BsAssert(0);
		}
	}

	g_TextTable->GetText(nTexId, m_cCmdText, _countof(m_cCmdText));
	g_TextTable->GetText(nTexId+1, m_cAnswerText, _countof(m_cAnswerText));
	m_nGuadianSignTick = _CMD_ANSWER_TICK;
}


#define _GUARDIAN_SIGN_POS_X		96
#define _GUARDIAN_SIGN_POS_Y		428
#define _GUARDIAN_PORTRAIT_WIDTH	80
#define _GUARDIAN_PORTRAIT_HEIGHT	102
void FcAbilInterface::DrawGuardianSign()
{
	if(m_nGuadianSignTick <= 0){
		return;
	}

	int nPortraitId(-1);
	char cText[256];
	if(m_nGuadianSignTick > _CMD_ANSWER_TICK/2)
	{
		nPortraitId = m_nCmdPortraitId;
		strcpy(cText, m_cCmdText);
	}
	else
	{
		nPortraitId = m_nAnswerPortraitId;
		strcpy(cText, m_cAnswerText);
	}

	//length
	g_pFont->DrawUIText(0, 0, -1, -1, "@(reset)@(scale=0.8,0.8)@(space=-2)");
	int nWidth=0, nHeight=0, nLine;  //aleksger: prefix bug 752: GetTextLength may not initialize the variables.
	int nWordCount(0);
	g_pFont->GetTextLengthInfo(nWidth, nHeight, nLine, nWordCount, 0, 0, -1, -1, cText);

	int sizex = 50 + _GUARDIAN_SIGN_POS_X + _GUARDIAN_PORTRAIT_WIDTH + nWidth;
	UVImage* pImg = g_UVMgr.GetUVImage(_UV_SpeechMessage);
	g_BsKernel.DrawUIBox_s(_Ui_Mode_Image,
		-50, 428, sizex, 48,
		0.f, D3DXCOLOR(1, 1, 1, 1),
		0.f, pImg->nTexId,
		pImg->u1, pImg->v1, pImg->u2, pImg->v2);
	
	//portrait
	BsAssert(m_pPortrait != NULL);
	m_pPortrait->DrawPortrait(nPortraitId,
        _GUARDIAN_SIGN_POS_X - 10, _GUARDIAN_SIGN_POS_Y - 50,
        _GUARDIAN_PORTRAIT_WIDTH, _GUARDIAN_PORTRAIT_HEIGHT, 1.f);
	
	//Text
	g_pFont->DrawUIText(_GUARDIAN_SIGN_POS_X + _GUARDIAN_PORTRAIT_WIDTH,
		_GUARDIAN_SIGN_POS_Y + 14, 108+ nWidth, -1, cText);

	m_nGuadianSignTick--;
}



void FcAbilInterface::GetGuardianVoiceQueStr( GUARDIAN_VOICE_TYPE Type, char* pStr )
{
	int nIndex = (int)Type;
	int nHero = g_FcWorld.GetHeroHandle()->GetHeroClassID() - CFcGameObject::Class_ID_Hero_Aspharr;

	if( g_HeroGuardianVoiceQueInfo[nIndex][nHero].nNum == 0 )
	{
		pStr[0] = NULL;
		return;
	}
	char* pDest = g_HeroGuardianVoiceQueInfo[nIndex][nHero].cName;
	int nRandomIndex = Random( g_HeroGuardianVoiceQueInfo[nIndex][nHero].nNum ) + 1;
	sprintf_s( pStr, 256, "%s%d", pDest, nRandomIndex );
}


bool FcAbilInterface::IsEnableGuardian()
{
	if(g_FCGameData.nPlayerType == MISSION_TYPE_DWINGVATT){
		return false;
	}

	if(g_FCGameData.nPlayerType == MISSION_TYPE_VIGKVAGK){
		return false;
	}

	if(g_FCGameData.nPlayerType == MISSION_TYPE_INPHYY &&
		g_FCGameData.SelStageId == STAGE_ID_ANOTHER_WORLD)
	{
		return false;
	}

	return true;
}

bool FcAbilInterface::IsEnablePlayerVoice()
{
	for( int i=0; i<4; i++ )
	{
		if( m_nPlayerVoiceHandle[i] != -1 )
		{
			return false;
		}
	}
	return true;
}