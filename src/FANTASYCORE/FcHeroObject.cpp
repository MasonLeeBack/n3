#include "StdAfx.h"
#include "BsKernel.h"
#include "Input.h"
#include "InputPad.h"
#include "BsKernel.h"

#include "AsData.h"
#include ".\\data\\AnimType.h"
#include ".\\data\\SignalType.h"
#include ".\\data\\RequiredExp.h"
#include ".\\data\\KillItem.h"
#include "FcGameObject.h"
#include "FcHeroObject.h"
#include "FcAdjutantObject.h"
#include "FcFxManager.h"
#include "FcAIObject.h"

#include "FcProp.h"

#include "FcCameraObject.h"
#include "FcAbilityManager.h"
#include "FcSOXLoader.h"

#include "FcGlobal.h"
#include "FcSoundManager.h"

#include "FcEventSequencer.h"
#include "FcHeroSeqEvent.h"
#include "DebugUtil.h"

#include "FcTroopObject.h"
#include "FcVelocityCtrl.h"

#include "FcUtil.h" //TGS 땜빵을 위한 헤더
#include "FcItem.h" //TGS 땜빵을 위한 헤더

#include "Ray3.h"
#include "Box3.h"
#include "IntLin3Box3.h"

#define HERO_RENEW_FRAME 800
#define HERO_RENEW_RECOVERY_HP_RATIO 0.5

#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG


CFcHeroObject::CFcHeroObject(CCrossVector *pCross)
: CFcUnitObject(pCross)
{
	SetClassID( Class_ID_Hero );
	m_nPlayerIndex = -1;

	m_nFallFrame = FALL_SUCCESS_FRAME + 1;
	m_nLoopInputCount = 0;
	m_bUsedInput = false;
	m_nInput = -1;
	memset( m_nInputPressCount, 0, sizeof( int ) * PAD_INPUT_COUNT );

	m_nComboCount = 0;
	m_nComboKeepFrame = 0;
	m_nOrbKillCount = 0;
	m_nMaxOrbKillCount = 0;
	m_nKillCount = 0;
	m_fOrbSparkFrame = 0.f;
	m_nMotionBlurIndex = -1;
//	m_nFlockingInterval = 1;
	m_nOrbSparkType = FC_ABILITY_LIGHT_1;
	s_nEffectLightFrame = 0;

	m_nTrueOrbSpark = 0;
	m_nOrbSpark = 0;
	m_nMaxOrbSparkAdd = 0;
	m_nOrbSparkAdd = 0;
	m_nOrbSparkAddWithTrueOrb = 0;
	m_nTrueOrbSparkAdd = 0;
	m_nOrbSplitPercent = 0;
	m_nTotalOrbSpark = 0;
	m_fOrbAttackKeepSpeed = 0.0f;

	m_bEnableTrueOrbSpark = true;

	m_nOrbSparkOn = 0;

	m_nDashFrame = 0;
	m_nWeaponTrailOffsetTexID = -1;

	m_nMoveSpeedAdd = 0;
	m_fJumpSpeed = 0.0f;
	m_nExp = 0;
	m_bOrbSpark = false;
	m_bChargeMode = false;

	m_bTrampleDamage = false;
	m_nDashLengthAdd = 0;
	m_bEnableDash = true;
	m_bEnableJump = true;
	m_bEnableDefense = true;
	m_bNotUseOrbSpecialAttack = false;
	m_bAlwaysSmallDamage = false;
	m_nSpecialAttackUseOrb = 0;
	m_bChargeModeGod = false;
	m_bAutoOrbAttack = false;
	m_nOrbAttackPowerAdd = 0;

	m_bStopOrb = false;

	m_nDieRenewFrame = 0;
	m_nDieType = 0;
	m_fOnlyPlayerDamage = 0.f;
	SetOrbSparkActivity(false);
}

void CalcVelocityValue( D3DXVECTOR3 &vCenter, const CCrossVector &ObjCross, float &fXVal, float &fZVal ) //aleksger: prefix bug 575: Pass large objects by reference
{
	D3DXVECTOR3 vXVec, vYVec, vZVec;

	vZVec = ObjCross.m_PosVector - vCenter;
	D3DXVec3Normalize( &vZVec, &vZVec );

	D3DXVec3Cross( &vXVec, &vZVec, &D3DXVECTOR3( 0.f, 1.f, 0.f ) );
	D3DXVec3Normalize( &vXVec, &vXVec );

	fZVal = D3DXVec3Dot( &ObjCross.m_ZVector, &vZVec );
	fXVal = D3DXVec3Dot( &ObjCross.m_XVector, &vXVec );
}

CFcHeroObject::~CFcHeroObject()
{	
	if( m_CatchHandle )
	{
		SendUnlinkCatchMsg();
	}
	g_pFcFXManager->SendMessage( m_nMotionBlurIndex, FX_DELETE_OBJECT );
	m_nMotionBlurIndex = -1;

	ReleaseWeaponTrailTexture();
}


void CFcHeroObject::Initialize( INITGAMEOBJECTDATA* pData, int nForce, int nTeam, TroopObjHandle hTroop, float fScaleMin, float fScaleMax )
{
	CFcUnitObject::Initialize( pData, nForce, nTeam, hTroop, fScaleMin, fScaleMax );
	//	m_nMotionBlurIndex = g_pFcFXManager->Create( FX_TYPE_MOTIONBLUR );
	//	g_pFcFXManager->SendMessage( m_nMotionBlurIndex, FX_INIT_OBJECT );
	InitOrbAttack2DFx();
	m_nTrailHandle[ 0 ] = g_pFcFXManager->Create( FX_TYPE_WEAPONTRAIL );
	g_pFcFXManager->SendMessage( m_nTrailHandle[ 0 ], FX_INIT_OBJECT );

	
	m_pRotateCtrl->SetMaximumSpeed( 20.f );
	m_pRotateCtrl->SetMinimumSpeed( 20.f );
	m_pRotateCtrl->SetStartSpeed( 20.f );
	
	BsAssert( m_WeaponTrailTexIDs.empty() );
	BsAssert( m_nWeaponTrailOffsetTexID == -1 );

	LoadWeaponTrailTexture();

	g_BsKernel.SendMessage( m_nEngineIndex, BS_USE_ANI_CACHE, 0 );

}



void CFcHeroObject::InitOrbAttack2DFx()
{
	m_nMotionBlurIndex = g_pFcFXManager->Create( FX_TYPE_MOTIONBLUR );
	g_pFcFXManager->SendMessage( m_nMotionBlurIndex, FX_INIT_OBJECT );
}



#include "FcWorld.h"
#include "FcPropManager.h"
void CFcHeroObject::ProcessKeyEvent(int nKeyCode, KEY_EVENT_PARAM *pKeyParam)
{
	s_nEffectLightFrame--;
	if( nKeyCode == PAD_INPUT_EVENT_RESTORE_KEY ) {
		ResetKeyProcess();
		if( IsMove() ) CmdStop();
		m_MoveTargetPos = D3DXVECTOR3( 0.f, 0.f, 0.f );
		return;
	}

/// 카메라 조작시에 입력이 안들어 오도록 임시로 막았는데 위치가 여기가 맞는 지는 모르겠습니다. folded
	if( m_nPlayerIndex != -1 ) {
		CameraObjHandle CamHandle = CFcBaseObject::GetCameraObjectHandle( m_nPlayerIndex );
		if( CamHandle->IsEditMode() || CamHandle->IsFreeCamMode() ) return;
	}

////////////////////////////////////////////////////////////////////////////////////////
	if( IsPlayEvent() || IsPlayRealMovie() )
	{
		return;
	}

	bool bRelease=false;

	if(nKeyCode&0x80000000){
		bRelease=true;
	}
	nKeyCode&=0x7fffffff;

	if(nKeyCode==PAD_INPUT_LSTICK){
		int nFindAni;

		ResetKeyProcess();
		if( IsMovable() )
		{
			if(bRelease){
				nFindAni = FindExactAni( m_pUnitInfoData->GetAniIndex( ANI_TYPE_STAND, 0 ) );
				if( nFindAni != -1 ) ChangeAnimationByIndex( nFindAni );
			}
			else {
				D3DXVECTOR2 vVec = D3DXVECTOR2( (float)pKeyParam->nPosX, (float)pKeyParam->nPosY );
				float fLength = D3DXVec2Length( &vVec );
				int nAniType;
				if( fLength < 25000.f ) nAniType = ANI_TYPE_WALK;
				else nAniType = ANI_TYPE_RUN;

				nFindAni = FindExactAni( m_pUnitInfoData->GetAniIndex( nAniType, m_nCurAniTypeIndex ) );
				if( nFindAni != -1 ) ChangeAnimationByIndex( nFindAni );
				CalculateMoveDir( pKeyParam, m_nPlayerIndex );
				nAniType = m_nCurAniType;

				if( nAniType == ANI_TYPE_WALK )
					m_fMoveSpeed = ( ( float )m_pUnitInfoData->GetMoveSpeed( m_nAniIndex ) ) * GetFrameAdd(); 
				else m_fMoveSpeed = CHeroLevelTableLoader::GetInstance().GetMoveSpeed( m_nUnitSOXIndex, m_nLevel ) * GetFrameAdd() * CalcMoveSpeedAdd(); 
//				m_nInput = -1;
//				m_bUsedInput = true;
			}
		}
		else if( ( m_nCurAniType == ANI_TYPE_JUMP_UP ) || ( m_nCurAniType == ANI_TYPE_JUMP_ATTACK ) 
			|| ( m_nCurAniType == ANI_TYPE_JUMP_DOWN ) )
		{
			float fDot;

			D3DXVECTOR3 KeyVector((float)pKeyParam->nPosX, 0.0f, (float)pKeyParam->nPosY);

			D3DXVec3Normalize(&KeyVector, &KeyVector);
			D3DXVec3TransformNormal(&KeyVector, &KeyVector, *s_CameraHandle[m_nPlayerIndex]->GetCrossVector());
			fDot=D3DXVec3Dot(&KeyVector, &m_Cross.m_ZVector);
			if(fDot>0.6f){
				float fHeight;

				fHeight = g_BsKernel.GetLandHeight( m_Cross.m_PosVector.x, m_Cross.m_PosVector.z );
				if( ( fHeight < m_Cross.m_PosVector.y ) || ( GetVelocityY() > 0.0f ) )
				{
					m_fJumpSpeed = ( float )m_pUnitInfoData->GetMoveSpeed( m_nAniIndex );
//					MoveZ( ( float )m_pUnitInfoData->GetMoveSpeed( m_nAniIndex ) );
				}
			}
		}
	}
	else if( nKeyCode != PAD_INPUT_RSTICK )
	{
		if( !bRelease )
		{
			if( nKeyCode != PAD_INPUT_RSTICK )
				ResetKeyProcess();
			if( nKeyCode == PAD_INPUT_A )
			{
				if( RideHorse() )
				{
					return;
				}
				else if( ProcessFall() )
				{
					return;
				}
			}
			else if( nKeyCode == PAD_INPUT_B )
			{
				if( !m_bOrbSpark )
				{
					if( ( IsMovable() ) || ( m_nCurAniType == ANI_TYPE_ATTACK ) )
					{
						if( GetHeightFromLand() <= 20.0f )
						{
							if( ( m_bEnableTrueOrbSpark ) && ( m_nTrueOrbSpark == MAX_TRUE_ORB_SPARK ) )
							{
								UnlinkCatchObject( 0, -1, 0 );
								ChangeAnimation( ANI_TYPE_SPECIAL_ATTACK, 2 );
							}
							else if( m_nOrbSpark == CalcMaxOrbSpark() )
							{
								StartOrbSpark();
							}
						}
					}
				}
			}
			else if( nKeyCode == PAD_INPUT_Y && m_bOrbSpark )
			{
				OrbAttack();
			}
			else if( nKeyCode == PAD_INPUT_X )
			{
				NormalAttack();
			}
			if( CInputPad::GetInstance().GetKeyPressTick( g_FCGameData.nEnablePadID, nKeyCode ) == 1 )
			{
				m_nInputPressCount[ nKeyCode ]++;
			}
			m_nInput=nKeyCode;
			m_bUsedInput = false;
		}
	}
}

void CFcHeroObject::Enable( bool bEnable, bool bFullHP /*= false*/ )
{
	if( bEnable == false )
	{
		StopOrbSpark();
	}

	CFcUnitObject::Enable( bEnable, bFullHP );
}

void CFcHeroObject::ResetKeyProcess()
{
	m_pEventSeq->Reset();
//	m_MoveTargetPos = D3DXVECTOR3( 0.f, 0.f, 0.f );
//	if( m_ParentLinkObjHandle ) *m_ParentLinkObjHandle->GetMoveTargetPos() = D3DXVECTOR3( 0.f, 0.f, 0.f );
}

/*
void CFcHeroObject::CreateParts()
{
	CFcUnitObject::CreateParts();
}
*/

void CFcHeroObject::Process()
{
	if( IsPlayEvent() || IsPlayRealMovie() )
	{
		if( m_bStopOrb == false )
		{
			StopOrbSpark();
			m_bStopOrb = true;
		}
	}
	else
	{
		m_bStopOrb = false;
	}

	if( m_nComboKeepFrame > 0 )
	{
		m_nComboKeepFrame--;
	}
	else
	{
		m_nComboCount = 0;
	}
	m_nDashFrame--;
	CheckFall();
	if( m_nCurAniAttr == ANI_ATTR_NONE && m_nCurAniType == ANI_TYPE_JUMP_UP )
	{
		float fHeight;
		fHeight = CFcWorld::GetInstance().GetLandHeight( m_Cross.m_PosVector.x, m_Cross.m_PosVector.z );
		if( m_Cross.m_PosVector.y > fHeight + AIR_HEIGHT_GAP )
		{
			MoveZ( m_fJumpSpeed );
		}
		m_fJumpSpeed -= 0.5f;
		if( m_fJumpSpeed <= 0.0f )
		{
			m_fJumpSpeed = 0.0f;
		}
	}
	else
	{
		m_fJumpSpeed = 0.0f;
	}

#if 0	// tohoshi
	if( GetPlayerIndex() != -1 ) //플레이어 캐릭터만
	{	
		CameraObjHandle CamHandle = CFcBaseObject::GetCameraObjectHandle( GetPlayerIndex() );
		BsAssert( CamHandle );
		D3DXVECTOR3 camDirV3 = CamHandle->GetCrossVector()->m_ZVector;
		float fDist = D3DXVec3LengthSq( &camDirV3);
		if( fDist == 0.f )
			camDirV3 = D3DXVECTOR3( 0.f, 0.f, 1.f );
		else
			D3DXVec3Normalize( &camDirV3, &camDirV3 );

		D3DXVECTOR3 Pos = m_Cross.m_PosVector;
		Pos.y += GetUnitHeight();
		g_pSoundManager->SetListenerPos( Pos, camDirV3);
	}
#endif
	
	PreProcessSignal();
	CFcUnitObject::Process();

	if( ( m_nDashLengthAdd ) && ( m_nCurAniType == ANI_TYPE_DASH ) )
	{
		GET_ANIDISTANCE_INFO Info;
		D3DXVECTOR3 vPos = D3DXVECTOR3( 0.f, 0.f, 0.f );

		Info.fFrame1 = m_fFrame;
		Info.fFrame2 = m_fPrevFrame;
		Info.nAni1 = m_nAniIndex;
		Info.nAni2 = m_nAniIndex;
		Info.pVector = &vPos;
		g_BsKernel.GetAniDistance( m_nEngineIndex, &Info );
		MoveZ( vPos.z * ( m_nDashLengthAdd / 100.0f ) );
	}

	switch( m_nDieType ) {
		case 1:
			if( m_nDieRenewFrame != 0 )		// 같은 편일 때만 다시 일어나게 처리
				++m_nDieRenewFrame;

			if( m_nDieRenewFrame > HERO_RENEW_FRAME ) {
				m_fHP = (float)(GetMaxHP() * HERO_RENEW_RECOVERY_HP_RATIO);

				// 일단 죽었따가 일어나기가 없으니 걍 일어나기루 처리..
				int nIndex = GetCurAniIndex();
				ChangeAnimation( ANI_TYPE_RISE, nIndex );
				m_nDieRenewFrame = 0;
			}
			break;

		default:	break;
	}
}

void CFcHeroObject::StartOrbSpark()
{
	UnlinkCatchObject( 0, -1, 0 );
	OrbSparkOn();
	OrbAttackFxOn();
	m_bOrbSpark = true;
	m_nOrbKillCount = 0;
	g_pFcFXManager->SendMessage( m_nMotionBlurIndex, FX_PLAY_OBJECT );
	m_fOrbSparkFrame = ( float )m_nOrbSpark;

	int nAniIndex = FindExactAni( m_pUnitInfoData->GetAniIndex( ANI_TYPE_SPECIAL_ATTACK, 0 ) );
	ChangeAnimation( m_pUnitInfoData->GetAniType(nAniIndex), m_pUnitInfoData->GetAniTypeIndex(nAniIndex), m_pUnitInfoData->GetAniAttr(nAniIndex) );
	++m_nOrbSparkOn;
}

void CFcHeroObject::StopOrbSpark()
{
	if( m_bOrbSpark )
	{
		g_pFcFXManager->SendMessage( m_nMotionBlurIndex, FX_STOP_OBJECT );
		OrbAttackFxOff();
		OrbSparkOff();
		m_bOrbSpark = false;
		if( m_fOrbSparkFrame <= 0.f )
		{
			m_fOrbSparkFrame = 0.0f;
		}
		SetOrbSpark( ( int )m_fOrbSparkFrame );
		m_fOrbSparkFrame = 0.0f;
	}
	Resume();
	UnlinkCatchObject( 0, -1, 0, true );
}

void CFcHeroObject::PostProcess()
{
	if( m_bOrbSpark )
	{
		m_fOrbSparkFrame -= 0.01f * (float)CHeroLevelTableLoader::GetInstance().GetOrbDecrease( m_nUnitSOXIndex, m_nLevel )*1.5f / ( m_fOrbAttackKeepSpeed / 100.0f + 1.0f );
		if( m_fOrbSparkFrame <= 0.f )
		{
			int nAniIndex = FindExactAni( m_pUnitInfoData->GetAniIndex( ANI_TYPE_SPECIAL_ATTACK, 1 ) );
			ChangeAnimation( m_pUnitInfoData->GetAniType(nAniIndex), m_pUnitInfoData->GetAniTypeIndex(nAniIndex), m_pUnitInfoData->GetAniAttr(nAniIndex) );
//			ChangeAnimation( ANI_TYPE_SPECIAL_ATTACK, 1 );
			StopOrbSpark();
			m_bKeepOrbSpark = true;
		}
	}
	if( m_nCurAniType == ANI_TYPE_DASH )
	{
		float fMoveY = CFcWorld::GetInstance().GetLandHeight( m_Cross.m_PosVector.x, m_Cross.m_PosVector.z ) - m_Cross.m_PosVector.y;
		if( fMoveY < 80.f ) {		
			MoveY( fMoveY );
		}
	}
	if( m_hUnitAIHandle && !m_hUnitAIHandle->IsEnable() && m_nPlayerIndex != -1 ) m_bUsePathFind = false;
	else m_bUsePathFind = true;

	CFcUnitObject::PostProcess();

}

void CFcHeroObject::AddExp( int nExp, GameObjHandle Handle, int nKillCount/* = 0*/ )
{
	int i, nPercent;
	bool bLevelUp;

	if( nExp == 0 ) return;
	nPercent = GetOrbPercent() + 100;
	nExp = ( int )( nExp * ( nPercent / 100.0f ) );

	//TGS Build temporary code//////////////////////////////////////////////////////////////////
	if( nKillCount )
	{
		AddKillCount( nKillCount, Handle );
	}
	/*
	if( GetKillCount() % 1000 == 0){ //나중에 트리거로 빼야함
	g_pSoundManager->PlaySound( SB_ALL_KAKEGOE, "I_U800_01" );
	}
	else if( GetKillCount() % 500 == 0 ){
	g_pSoundManager->PlaySound( SB_ALL_KAKEGOE, "I_U1000_03" );        
	}

	if(GetKillCount() == 1000)
	{

	CCrossVector Cross = *GetCrossVector();
	Cross.MoveFrontBack(500);
	g_FcItemManager.CreateItemToWorld( ITEM_DYING_GLOVE,Cross.GetPosition());
	}
	else if(GetKillCount() == 2000)
	{
	CCrossVector Cross = *GetCrossVector();
	Cross.MoveFrontBack(500);
	g_FcItemManager.CreateItemToWorld( ITEM_ASSASIN_GLOVE_1,Cross.GetPosition());
	}*/
	/////////////////////////////////////////////////////////////////////////////////////////////

	CFcUnitObject::AddExp( nExp, Handle );
	bLevelUp = false;
	for( i = m_nLevel; i < MAX_HERO_LEVEL - 1; i++ )
	{
		if( m_nExp >= g_nRequiredExp[ i ] )
		{
			m_nLevel = i + 1;
			m_hTroop->SetLevel( m_nLevel );
			bLevelUp = true;
		}
		else
		{
			break;
		}
	}
	if( bLevelUp )
	{
		// Level up Fx 필요
		SetHP(GetMaxHP());
		AddOrbSpark(CalcMaxOrbSpark());
		g_pSoundManager->PlaySystemSound( SB_COMMON, "SYS_LEVEL_UP" );
	}
}

int CFcHeroObject::GetNextExp() 
{ 
	return g_nRequiredExp[ m_nLevel ]; 
}

bool CFcHeroObject::CalculateDamage(int nAttackPower)
{
	bool bRet;

	if( m_fOnlyPlayerDamage > 0.f ) {
		float fHP = m_fHP;
		bRet = CFcGameObject::CalculateDamage( nAttackPower );
		if( bRet == true ) {
			if( m_fHP < (float)(GetMaxHP() * m_fOnlyPlayerDamage) ) {
				if( IsPlayerDamage() == false )
					m_fHP = fHP;
			}
		}
		return bRet;
	}

	bRet = CFcUnitObject::CalculateDamage( nAttackPower );

	if( ( m_fHP <= 0.f ) && ( m_bAutoOrbAttack ) )
	{
		m_fHP = 1.f;
		g_FcItemManager.NotifyAutoOrbAttack();		
		StartOrbSpark();
	}
	return bRet;
}

void CFcHeroObject::SignalCheck( ASSignalData *pSignal )
{
	switch( pSignal->m_nID )
	{
	case SIGNAL_TYPE_HIT_TEMP:		// 2ND 빌드용 임시 HitCheck다..
		HitCheckTempProp( pSignal );
		CFcUnitObject::SignalCheck( pSignal );
		break;
	case SIGNAL_TYPE_INPUT:
	case SIGNAL_TYPE_ORB_INPUT:
		if( m_nPlayerIndex != -1 )
			InputCheck( pSignal );
		break;
	case SIGNAL_TYPE_GOTO_PRESS_COUNT:
		if( m_nPlayerIndex != -1 )
			GotoPressCount( pSignal );
		break;
	case SIGNAL_TYPE_GOTO_PRESS_CHECK:
		if( m_nPlayerIndex != -1 )
			GotoPressCheck( pSignal );
		break;
	case SIGNAL_TYPE_CHECK_PHYSICS:
		PhysicsCheck( pSignal );
		break;
	case SIGNAL_TYPE_TRUE_ORB_SPARK:
		TrueOrbCheck( pSignal );
		break;

	case SIGNAL_TYPE_UNLINK_CATCH:
		UnlinkCatch( pSignal );
		break;
	case SIGNAL_TYPE_AI_CONTROL_CHECK:
	case SIGNAL_TYPE_AI_CONTROL_RELEASE:
	case SIGNAL_TYPE_AI_CONTROL_GOTO:
	case SIGNAL_TYPE_AI_CONTROL_CALLBACK:
		if( m_nPlayerIndex != -1 ) return;
		CFcUnitObject::SignalCheck( pSignal );
		break;
	case SIGNAL_TYPE_CHARGE:
		m_bChargeMode = pSignal->m_pParam[ 0 ] != 0;
		break;
	case SIGNAL_TYPE_SPECIAL_CAMERA:
		if( ( pSignal->m_nFrame >= m_fFrame ) && ( pSignal->m_nFrame < m_fFrame + GetFrameAdd() ) )
		{
			CameraObjHandle CamHandle = CFcBaseObject::GetCameraObjectHandle( 0 );
			CamHandle->SpecialCam( pSignal->m_pParam[ 0 ], pSignal->m_pParam[ 1 ] );
		}
		break;
	default:
		CFcUnitObject::SignalCheck( pSignal );
		break;
	}
}


void CFcHeroObject::GotoPressCount( ASSignalData *pSignal )
{
	if( pSignal->m_GotoKey )
	{
		if( ( pSignal->m_nFrame >= m_fFrame ) && ( pSignal->m_nFrame < m_fFrame + GetFrameAdd() ) )
		{
			if( m_nInputPressCount[ pSignal->m_GotoKey - 1 ] >= pSignal->m_GotoKeyCount + pSignal->m_GotoAddKeyCount * m_nLoopInputCount )
			{
				if( m_nLoopInputCount >= pSignal->m_GotoMaxLoop )
				{
					return;
				}
				if( m_nLevel < pSignal->m_GotoLoopLevel )
				{
					return;
				}
				m_fFrame = ( float )( pSignal->m_GotoFrame );
				if( m_fFrame > m_nCurAniLength - 1 )
				{
					m_fFrame = ( float )( m_nCurAniLength - 1 );
				}
				m_fPrevFrame = m_fFrame;
				m_nInputPressCount[ pSignal->m_GotoKey - 1 ] = 0;
				m_nLoopInputCount++;
				if( pSignal->m_GotoDirection )
				{
// [beginmodify] 2006/1/16 junyash PS#1259,3249,3251, TCR#033PER multi controller
					#if 0
					LookAtNearestEnemy( CInputPad::GetInstance().GetKeyParam( m_nPlayerIndex, 0 ), m_nPlayerIndex);
					#else
					LookAtNearestEnemy( CInputPad::GetInstance().GetKeyParam( g_FCGameData.nEnablePadID, 0 ), m_nPlayerIndex);
					#endif
// [endmodify] junyash
				}
			}
		}
		return;
	}
}

bool CFcHeroObject::IsInHitRange( ASSignalData *pSignal, D3DXVECTOR3 *pPosition )
{
	D3DXVECTOR3 Direction, Orient;
	float fLength, fDot, fCos, fSin;

	Direction = *pPosition - m_Cross.m_PosVector;
	Direction.y = 0.0f;
	fLength = D3DXVec3Length( &Direction );
	if( fLength < pSignal->m_Hit2MinDist )
	{
		return false;
	}
	if( fLength > pSignal->m_Hit2MaxDist )
	{
		return false;
	}
	if( pSignal->m_Hit2Angle != 0 )
	{
		fCos = cos( D3DXToRadian( pSignal->m_Hit2CenterAngle ) );
		fSin = sin( D3DXToRadian( pSignal->m_Hit2CenterAngle ) );
		Orient.x = m_Cross.m_ZVector.x * fCos - m_Cross.m_ZVector.z * fSin;
		Orient.y = 0.0f;
		Orient.z = m_Cross.m_ZVector.x * fSin + m_Cross.m_ZVector.z * fCos;
		D3DXVec3Normalize( &Direction, &Direction );
		fDot = D3DXVec3Dot( &Direction, &Orient );
	}
	else
	{
		return true;
	}
	if( fDot > cos( D3DXToRadian( pSignal->m_Hit2Angle ) ) )
	{
		return true;
	}

	return false;
}

bool CFcHeroObject::AddDynamicForce( CFcProp *pProp, int nAttackPoint, PROP_BREAK_TYPE Type )
{
	D3DXVECTOR3 Force;

	if( pProp->IsDynamicProp() )
	{
		return false;
//		Force = D3DXVECTOR3( 0.0f, 0.0f, 5.0f );
	}
	else
	{
		Force = D3DXVECTOR3( 0.0f, 2.0f, 6.0f );
	}
	D3DXVec3TransformNormal( &Force, &Force, ( D3DXMATRIX * )m_Cross );

	return pProp->AddForce( &Force, &(m_Cross.m_PosVector), nAttackPoint, Type );
}

void CFcHeroObject::HitCheckTempProp( ASSignalData *pSignal )
{
	if( ( pSignal->m_nFrame >= m_fFrame ) && ( pSignal->m_nFrame < m_fFrame + GetFrameAdd() ) )
	{
		bool bCollision;
		int i, nSize;
		std::vector< CFcProp * > Result;
		Box3 BoundingBox;
		Segment3 Line;

		CFcWorld::GetInstance().GetActiveProps( m_Cross.m_PosVector, pSignal->m_Hit2MaxDist * 3.0f, Result );
		nSize = Result.size();
		for( i = 0; i < nSize; i++)
		{
			if( Result[ i ]->IsBreak() )
			{
				continue;
			}
			bCollision = false;
			Result[ i ]->GetBox3( BoundingBox );
			if( ( BoundingBox.E[ 0 ] > 150.0f ) || ( BoundingBox.E[ 2 ] > 150.0f ) )
			{
				memcpy( &( Line.P ), &m_Cross.m_PosVector, sizeof( D3DXVECTOR3 ) );
				Line.P.y += 100.0f;
				memcpy( &( Line.D ), &( m_Cross.m_ZVector * ( float )pSignal->m_Hit2MaxDist ), sizeof( D3DXVECTOR3 ) );
				bCollision = TestIntersection( Line, BoundingBox );
			}
			else
			{
				bCollision = IsInHitRange( pSignal, &Result[ i ]->GetPos() );
			}
			if( bCollision )
			{
				if( AddDynamicForce( Result[ i ], pSignal->m_Hit2AttackPoint, PROP_BREAK_TYPE_NORMAL ) )
				{
					if( Result[ i ]->GetCrushParticleIndex() > 0 )
					{
						CCrossVector Cross;
						D3DXVECTOR3 Direction;
						float fLength;

						Direction = Result[ i ]->GetPos() - m_Cross.m_PosVector;
						fLength = D3DXVec3Length( &Direction );
						D3DXVec3Normalize( &Direction, &Direction );
						Cross = m_Cross;
						Cross.MoveFrontBack( D3DXVec3Dot( &Direction, &m_Cross.m_ZVector ) * fLength );
						Cross.m_PosVector.y += pSignal->m_Hit2ParticleY;

						g_BsKernel.CreateParticleObject( Result[ i ]->GetCrushParticleIndex(), false, false, Cross );
					}
				}
			}
		}
	}
}

void CFcHeroObject::PhysicsCheck( ASSignalData *pSignal )
{
	if( ( pSignal->m_nFrame >= m_fFrame ) && ( pSignal->m_nFrame < m_fFrame + GetFrameAdd() ) )
	{
		bool bCollision;
		int i, nCount;
		float fLength, fDot;
		D3DXVECTOR3 Direction;
		std::vector< CFcProp * > Result;
		Box3 BoundingBox;
		Segment3 Line;

		CFcWorld::GetInstance().GetActiveProps( m_Cross.m_PosVector, pSignal->m_PhysicsDistance * 3.0f, Result );
		nCount = ( int ) Result.size();
		for( i = 0; i < nCount; i++ )
		{
			if( pSignal->m_PhysicsType == 0 ) 
			{
				if( ( !Result[ i ]->IsCrushProp() ) || ( Result[ i ]->IsBreak() ) )
				{
					continue;
				}
			}
			if( ( pSignal->m_PhysicsType == 1 ) && ( !Result[ i ]->IsDynamicProp() ) )
			{
				continue;
			}

			bCollision = false;
			Result[ i ]->GetBox3( BoundingBox );
			Direction = Result[ i ]->GetPos() - m_Cross.m_PosVector;
			fLength = D3DXVec3Length( &Direction );
			if( ( BoundingBox.E[ 0 ] > 150.0f ) || ( BoundingBox.E[ 2 ] > 150.0f ) )
			{
				memcpy( &( Line.P ), &m_Cross.m_PosVector, sizeof( D3DXVECTOR3 ) );
				Line.P.y += 100.0f;
				memcpy( &( Line.D ), &( m_Cross.m_ZVector * ( float )pSignal->m_PhysicsDistance ), sizeof( D3DXVECTOR3 ) );
				bCollision = TestIntersection( Line, BoundingBox );
				D3DXVec3Normalize( &Direction, &Direction );
			}
			else
			{
				if( fLength < pSignal->m_PhysicsDistance )
				{
					D3DXVec3Normalize( &Direction, &Direction );
					fDot = D3DXVec3Dot( &Direction, &m_Cross.m_ZVector );
					if( fDot > cos( ( float )pSignal->m_PhysicsAngle ) )
					{
						bCollision = true;
					}
				}
			}
			if( bCollision )
			{
				if( AddDynamicForce( Result[ i ], pSignal->m_PhysicsPower, PROP_BREAK_TYPE_SPECIAL_ATTACK ) )
				{
					if( Result[ i ]->GetCrushParticleIndex() > 0 )
					{
						CCrossVector Cross;

						Cross = m_Cross;
						Cross.MoveFrontBack( D3DXVec3Dot( &Direction, &m_Cross.m_ZVector ) * fLength );
						Cross.m_PosVector.y += pSignal->m_PhysicsParticleY;
						g_BsKernel.CreateParticleObject( Result[ i ]->GetCrushParticleIndex(), false, false, Cross );
					}
				}
			}
		}
	}
}

void CFcHeroObject::InputCheck( ASSignalData *pSignal )
{
	int nKeyPressTick, nAttr;

	if( m_bOrbSpark ) 
	{
		if( pSignal->m_nID == SIGNAL_TYPE_INPUT )
		{
			return;
		}
	}
	else
	{
		if( pSignal->m_nID == SIGNAL_TYPE_ORB_INPUT )
		{
			return;
		}
	}
	if( ( m_fFrame >= pSignal->m_nFrame ) && ( m_fFrame < pSignal->m_nFrame + pSignal->m_InputLength ) )
	{
		if( pSignal->m_InputClear )
		{
			m_bUsedInput = true;
			return;
		}
		if( GetLevel() < pSignal->m_InputLevel )
		{
			return;
		}
		// 잠깐 이렇게 해놓자.. 입력 안된다..
		if( pSignal->m_NumParam >= 9 )
		{
			if( !IsNotUseOrbSpecialAttack() )
			{
				if( GetOrbSpark() < pSignal->m_NeedOrb * ( ( 100 - m_nSpecialAttackUseOrb ) / 100.0f ) )
				{
					return;
				}
			}
		}

		if( ( pSignal->m_InputKey - 1 == m_nInput ) || ( pSignal->m_InputKey - 1 == PAD_INPUT_LSTICK ) )
		{
			nKeyPressTick = CInputPad::GetInstance().GetKeyPressTick( g_FCGameData.nEnablePadID, pSignal->m_InputKey - 1 );
			if( pSignal->m_InputPressFrame )
			{
				if( nKeyPressTick <= pSignal->m_InputPressFrame )
				{
					return;
				}
			}
			else 
			{
				if( ( m_bUsedInput ) && ( pSignal->m_InputKey - 1 != PAD_INPUT_LSTICK ) )
				{
					return;
				}
			}
			if( m_ParentLinkObjHandle )
			{
				nAttr = ANI_ATTR_HORSE;
			}
			else
			{
				nAttr = 0;
			}
			if( ( pSignal->m_InputPressFrame == 0 ) && ( nKeyPressTick > m_fFrame ) )	// 이전부터 꾹 눌러온 키 입력이면 무시한다..
			{
				return;
			}
			memset( m_nInputPressCount, 0, sizeof( int ) * PAD_INPUT_COUNT );
			m_bUsedInput = true;

			int nChangeAni, nChangeAniType;
			if( pSignal->m_InputDirectIndex - 1 < 0 )
			{
				nChangeAniType = pSignal->m_InputAniType;
				nChangeAni = m_pUnitInfoData->GetAniIndex( pSignal->m_InputAniType, pSignal->m_InputAniIndex, nAttr );
			}
			else
			{
				nChangeAniType = m_pUnitInfoData->GetAniType( pSignal->m_InputDirectIndex - 1 );
				nChangeAni = pSignal->m_InputDirectIndex - 1;
			}
			if( m_ParentLinkObjHandle && nChangeAniType == ANI_TYPE_JUMP_UP ) {
				m_ParentLinkObjHandle->ChangeAnimation( ANI_TYPE_JUMP_UP, -1 );
			}
			if( !IsEnableDash() && ( nChangeAniType == ANI_TYPE_DASH ) )
			{
				if( m_pUnitInfoData->GetAniTypeIndex( nChangeAni ) == 0 )
				{
					return;
				}
			}
			if( !IsEnableJump() && ( nChangeAniType == ANI_TYPE_JUMP_UP ) )
			{
				return;
			}
			if( !IsEnableDefense() && ( nChangeAniType == ANI_TYPE_DEFENSE ) )
			{
				return;
			}
			/*			if( nChangeAniType == ANI_TYPE_DASH )
			{
			if( m_nDashFrame <= 0 )
			{
			m_nDashFrame = DASH_STOP_FRAME;
			}
			else
			{
			return;
			}
			}*/
			// 잠깐 이렇게 해놓자.. 입력 안된다..
			if( pSignal->m_NumParam >= 9 )
			{
				if( ( pSignal->m_NeedOrb > 0 ) && ( !IsNotUseOrbSpecialAttack() ) )
				{
					AddOrbSpark( ( int )( -pSignal->m_NeedOrb * ( ( 100 - m_nSpecialAttackUseOrb ) / 100.0f ) ) );
					m_bNowSpecialAttack = true;
				}
				else
				{
					m_bNowSpecialAttack = false;
				}
			}
			ChangeAnimationByIndex( nChangeAni, true );
// [beginmodify] 2006/1/16 junyash PS#1259,3249,3251, TCR#033PER multi controller
			#if 0
			LookAtNearestEnemy( CInputPad::GetInstance().GetKeyParam( m_nPlayerIndex, 0 ), m_nPlayerIndex);
			#else
			LookAtNearestEnemy( CInputPad::GetInstance().GetKeyParam( g_FCGameData.nEnablePadID, 0 ), m_nPlayerIndex);
			#endif
// [endmodify] junyash
		}
	}
}

/*
void CFcHeroObject::AIInputCheck( ASSignalData *pSignal )
{
int nAttr;

if( pSignal->m_nID == SIGNAL_TYPE_ORB_INPUT )
{
return;
}
if( m_bUsedInput )
{
return;
}
if( ( m_fFrame >= pSignal->m_nFrame ) && ( m_fFrame < pSignal->m_nFrame + pSignal->m_InputLength ) )
{
if( ( pSignal->m_InputKey - 1 == m_nInput ) && ( pSignal->m_InputPressFrame == 0 ) )
{
if( m_ParentLinkObjHandle )
{
nAttr = ANI_ATTR_HORSE;
}
else
{
nAttr = 0;
}
m_bUsedInput = true;
if( pSignal->m_InputDirectIndex - 1 < 0 )
{
ChangeAnimation( pSignal->m_InputAniType, pSignal->m_InputAniIndex, nAttr, true);
}
else
{
ChangeAnimationByIndex( pSignal->m_InputDirectIndex - 1, true );
}
}
}
}
*/

void CFcHeroObject::GotoPressCheck( ASSignalData *pSignal )
{
	if( ( m_fFrame >= pSignal->m_nFrame ) && ( m_fFrame < pSignal->m_nFrame + pSignal->m_GotoPressCheckLength ) )
	{
		if( GetLevel() < pSignal->m_GotoRequireLevel )
		{
			return;
		}
		if( CInputPad::GetInstance().GetKeyPressTick( g_FCGameData.nEnablePadID, pSignal->m_GotoPressCheckKey - 1 ) )
		{
			if( pSignal->m_GotoPressCheck )
			{
				m_nLoopInputCount++;
				m_fFrame = ( float )pSignal->m_GotoPressCheckFrame;
				m_fPrevFrame = m_fFrame;
			}
		}
		else
		{
			if( !pSignal->m_GotoPressCheck )
			{
				CFcUnitObject::GotoCheck( pSignal );
			}
		}
	}
}

/*
void CFcHeroObject::AIGotoPressCheck( ASSignalData *pSignal )
{
if( ( m_fFrame >= pSignal->m_nFrame ) && ( m_fFrame < pSignal->m_nFrame + pSignal->m_GotoPressCheckLength ) )
{
if( m_nInput == pSignal->m_GotoPressCheckKey - 1 )
{
if( pSignal->m_GotoPressCheck )
{
m_fFrame = ( float )pSignal->m_GotoPressCheckFrame;
m_fPrevFrame = m_fFrame;
}
}
else
{
if( !pSignal->m_GotoPressCheck )
{
CFcUnitObject::GotoCheck( pSignal );
}
}
}
}
*/

void CFcHeroObject::TrueOrbCheck( ASSignalData *pSignal )
{
	if( ( pSignal->m_nFrame >= m_fFrame ) && ( pSignal->m_nFrame < m_fFrame + GetFrameAdd() ) )
	{
		if( m_nTrueOrbSpark == MAX_TRUE_ORB_SPARK )
		{
			CFcWorld::GetInstance().SetAbility( pSignal->m_pParam[0], m_Handle );
			m_nTrueOrbSpark = 0;
			SetOrbSparkActivity(true);
		}
	}
}

void CFcHeroObject::AddOrbSparkPercent( int nPercent )
{
	int nAddOrb;

	nAddOrb = ( int )( CalcMaxOrbSpark() * ( nPercent / 100.0f ) );
	AddOrbSpark( nAddOrb, true, false );
}

void CFcHeroObject::AddOrbSpark( int nOrb, bool bUseSplit/* = true*/, bool bAddTotal /*= true*/ )
{
	if( nOrb > 0 )
	{
		int nMax;

		if( ( bUseSplit ) && ( m_nOrbSplitPercent > 0 ) )
		{
			AddTrueOrbSpark( ( int )( nOrb * ( m_nOrbSplitPercent / 100.0f ) ), false, bAddTotal );
		}
		nOrb += ( int )( nOrb * ( m_nOrbSparkAdd / 100.0f ) );
		m_nOrbSpark += nOrb;
		AddTrueOrbSpark( ( int )(nOrb * ( m_nOrbSparkAddWithTrueOrb / 100.0f) ) , false );		
		nMax = CalcMaxOrbSpark();
		if( m_nOrbSpark >= nMax )
		{
			m_nOrbSpark = nMax;
		}
		if( bAddTotal )
		{
			m_nTotalOrbSpark += nOrb;
		}
	}
	else
	{
		m_nOrbSpark += nOrb;
		if( m_nOrbSpark < 0 )
		{
			m_nOrbSpark = 0;
		}
	}
}

void CFcHeroObject::AddTrueOrbSpark( int nOrb, bool bUseSplit/* = true*/, bool bAddTotal /*= true*/ )
{
	if( !m_bEnableTrueOrbSpark )
	{
		return;
	}
	if( nOrb > 0 )
	{
		if( ( bUseSplit ) && ( m_nOrbSplitPercent > 0 ) )
		{
			AddOrbSpark( ( int )( nOrb * ( m_nOrbSplitPercent / 100.0f ) ), false );
		}
		nOrb += ( int )( nOrb * ( m_nTrueOrbSparkAdd / 100.0f ) );
		if( bAddTotal )
		{
			m_nTotalOrbSpark += nOrb;
		}
		m_nTrueOrbSpark += nOrb;
		if( m_nTrueOrbSpark >= MAX_TRUE_ORB_SPARK )
		{
			m_nTrueOrbSpark = MAX_TRUE_ORB_SPARK;
		}
	}
	else
	{
		m_nTrueOrbSpark += nOrb;
		if( m_nTrueOrbSpark < 0 )
		{
			m_nTrueOrbSpark = 0;
		}
	}
}

void CFcHeroObject::SetMaxOrbSparkAdd( int nAdd ) 
{ 
	int nMax;

	m_nMaxOrbSparkAdd = nAdd; 
	nMax = CalcMaxOrbSpark();
	if( m_nOrbSpark > nMax )
	{
		m_nOrbSpark = nMax;
	}
}

void CFcHeroObject::NextAnimation()
{
	int nAniIndex, nAniType;

	nAniIndex = m_pUnitInfoData->GetNextAni( m_nAniIndex ) - 1;
	nAniType = m_pUnitInfoData->GetAniType( nAniIndex );
	if( ( nAniType == ANI_TYPE_RUN ) || ( nAniType == ANI_TYPE_STAND ) )
	{
		m_bUsedInput = true;
	}

	CFcUnitObject::NextAnimation();

	m_nLoopInputCount = 0;
	memset( m_nInputPressCount, 0, sizeof( int ) * PAD_INPUT_COUNT );
}

float CFcHeroObject::GetFrameAdd()
{
	if( ( m_bOrbSpark ) && ( m_nCurAniType == ANI_TYPE_RUN ) )
	{
		return 1.5f;
	}

	return 1.0f;
}

bool CFcHeroObject::RideHorse()
{
	int i, nCount;

	if( m_ParentLinkObjHandle )
	{
		if( m_nCurAniType == ANI_TYPE_STAND )
		{
			D3DXVECTOR3 vVec = m_ParentLinkObjHandle->GetCrossVector()->m_ZVector;
			if( RideOut() )
			{
				m_Cross.m_PosVector.y = g_BsKernel.GetLandHeight( m_Cross.m_PosVector.x, m_Cross.m_PosVector.z );
				m_Cross.m_ZVector = vVec;
				m_MoveDir = vVec;
				BsAssert( (m_MoveDir.x == m_MoveDir.x) && (m_MoveDir.y == m_MoveDir.y) && (m_MoveDir.z == m_MoveDir.z) );
				m_Cross.UpdateVectors();
				m_ParentLinkObjHandle.Identity();
				ChangeAnimation( ANI_TYPE_RIDE_HORSE, 0, ANI_ATTR_HORSE );
				m_nBlendFrame = 0;
				return true;
			}
		}
		else return false;
	}
	else
	{
		if( !IsRidable() ) return false;

		std::vector< GameObjHandle > Objects;

		CFcWorld::GetInstance().GetObjectListInRange( &m_Cross.m_PosVector, 220.0f, Objects );
		nCount = Objects.size();
		if( nCount >= 2 )
		{
			for( i = 0; i < nCount; i++ )
			{
				if( Objects[ i ]->GetClassID() == CFcGameObject::Class_ID_Horse )
				{
					if( Objects[i]->GetLinkObjCount() == 0 ) {
						CFcHeroSeqRideHorseEvent *pElement = new CFcHeroSeqRideHorseEvent( m_pEventSeq );
						pElement->m_nFlag = 0;
						pElement->m_nTime = -1;
						pElement->m_Handle = Objects[i];
						m_pEventSeq->AddEvent( pElement );

						m_nInput = -1;
						return true;
					}
				}
			}
		}
	}

	return false;
}

void CFcHeroObject::CheckFall()
{
	float fHeight;

	m_nFallFrame++;
	if( m_fHP <= 0.f )
	{
		return;
	}
	if( ( m_nCurAniType == ANI_TYPE_DOWN ) || ( m_nCurAniType == ANI_TYPE_DOWN_HIT ) )
	{
		if( m_nFallFrame > FALL_SUCCESS_FRAME )
		{
			fHeight = GetHeightFromLand();
			if( ( GetVelocityY() < 0.0f ) && ( fHeight < 50.0f ) )
			{
				m_nFallFrame = 0;
			}
		}
	}
}

bool CFcHeroObject::ProcessFall()
{
	if( m_nFallFrame <= FALL_SUCCESS_FRAME )
	{
		ChangeAnimation( ANI_TYPE_FALL, 0 );
		return true;
	}

	return false;
}

void CFcHeroObject::LookAtNearestEnemy( KEY_EVENT_PARAM *pParam, int nCamIndex )
{
	int i, nCount, nFindIndex;
	float fDot, fMaxDot;
	D3DXVECTOR3 KeyVector, ZDir, XDir, UpVec, LookVec, MaxVec;
	CCrossVector *pCross;
	std::vector< GameObjHandle > Result;

	KeyVector.x = ( float )pParam->nPosX;
	KeyVector.y = 0.0f;
	KeyVector.z = ( float )pParam->nPosY;
	D3DXVec3Normalize( &KeyVector, &KeyVector );

	if( s_CameraHandle[nCamIndex] )
	{
		pCross = s_CameraHandle[nCamIndex]->GetCrossVector();
		if( D3DXVec3Length( &KeyVector ) <= 0.0f )
		{
			m_MoveDir = m_Cross.m_ZVector;
			BsAssert( (m_MoveDir.x == m_MoveDir.x) && (m_MoveDir.y == m_MoveDir.y) && (m_MoveDir.z == m_MoveDir.z) );
		}
		else
		{
			ZDir = pCross->m_ZVector;
			ZDir.y = 0;
			D3DXVec3Normalize( &ZDir, &ZDir );
			UpVec=D3DXVECTOR3( 0, 1, 0 );
			D3DXVec3Cross( &XDir, &UpVec, &ZDir );
			m_MoveDir = XDir * KeyVector.x;
			BsAssert( (m_MoveDir.x == m_MoveDir.x) && (m_MoveDir.y == m_MoveDir.y) && (m_MoveDir.z == m_MoveDir.z) );
			m_MoveDir += ZDir * KeyVector.z;
			BsAssert( (m_MoveDir.x == m_MoveDir.x) && (m_MoveDir.y == m_MoveDir.y) && (m_MoveDir.z == m_MoveDir.z) );
			D3DXVec3Normalize( &m_MoveDir, &m_MoveDir );
		}

		if( GetCurAniType() == ANI_TYPE_ATTACK )
		{
			CFcWorld::GetInstance().GetEnemyObjectListInRange( m_nTeam, &m_Cross.m_PosVector, 300.0f, Result );
			nCount = ( int )Result.size();
			nFindIndex = -1;
			fMaxDot = -FLT_MAX;
			for( i = 0; i < nCount; i++ )
			{
				if( Result[ i ]->GetClassID() == CFcGameObject::Class_ID_Horse )
				{
					continue;
				}
				if( Result[ i ]->IsDie() )
				{
					continue;
				}
				LookVec = Result[ i ]->GetPos() - m_Cross.m_PosVector;
				D3DXVec3Normalize( &LookVec, &LookVec );
				fDot = D3DXVec3Dot( &LookVec, &m_MoveDir );
				if( fDot < 0.5f )
				{
					continue;
				}
				if( fDot > fMaxDot )
				{
					fMaxDot = fDot;
					nFindIndex = i;
					MaxVec = LookVec;
				}
			}
			if( nFindIndex != -1 )
			{
				m_MoveDir = MaxVec;
				m_MoveDir.y = 0.0f;
				BsAssert( (m_MoveDir.x == m_MoveDir.x) && (m_MoveDir.y == m_MoveDir.y) && (m_MoveDir.z == m_MoveDir.z) );
				D3DXVec3Normalize( &m_MoveDir, &m_MoveDir );
			}
		}
	}
	else
	{
		m_MoveDir = KeyVector;
		BsAssert( (m_MoveDir.x == m_MoveDir.x) && (m_MoveDir.y == m_MoveDir.y) && (m_MoveDir.z == m_MoveDir.z) );
	}
}

void CFcHeroObject::ProcessFlocking()
{
	if( m_bDead ) return;
	if( m_nCurAniType == ANI_TYPE_RIDE_HORSE ) return;
	if( IsJump() ) return;

	float fScanRange = GetUnitRadius() * 3.f;
	m_VecFlocking.erase( m_VecFlocking.begin(), m_VecFlocking.end() );
	CFcWorld::GetInstance().GetObjectListInRange( &GetDummyPos(), fScanRange, m_VecFlocking );

	D3DXVECTOR3 Direction;
	float fSumRadius;
	float fSpeed;
	float fLength;

	for( int i = 0; i <(int)m_VecFlocking.size(); i++ ) {
		if( !CFcGameObject::IsValid( m_VecFlocking[i] ) ) continue;
		if( !m_VecFlocking[i]->IsFlockable( m_Handle ) ) continue;

		Direction = m_VecFlocking[i]->GetPos() - GetPos();
		Direction.y = 0.f;

		fSumRadius = GetUnitRadius() + m_VecFlocking[ i ]->GetUnitRadius();
		fLength = D3DXVec3LengthSq( &Direction );
		if( fLength > fSumRadius * fSumRadius ) continue;

		if( m_VecFlocking[i]->GetClassID() >= Class_ID_Adjutant ) {
			fSpeed = ( fSumRadius - sqrtf( fLength ) );
		}
		else fSpeed = 5.f;


		D3DXVec3Normalize( &Direction, &Direction );
		m_VecFlocking[i]->Move( &( Direction * fSpeed ) );
		m_VecFlocking[i]->SetFlocking( true, fSumRadius );
	}

}

bool CFcHeroObject::IsHittable( GameObjHandle Handle )
{
	if( ( m_bChargeModeGod ) && ( m_bChargeMode ) )
	{
		return false;
	}
	if( ( m_nPlayerIndex != -1 ) && ( g_FcWorld.GetAbilityMng()->GetRealAbilityCount()>0/*g_FcWorld.IsProcessTrueOrb( -1 )*/ ) )
	{
		return false;
	}
	if( g_FCGameData.bGod == true && m_nPlayerIndex != -1 ) return false;
	if( !m_bOrbSpark )
	{
		return CFcUnitObject::IsHittable( Handle );
	}

	return false;
}

void CFcHeroObject::SendHitSignal( HIT_PARAM *pHitParam, GameObjHandle Handle )
{
	if( CFcBaseObject::IsValid( Handle ) 
		&& ( Handle->GetClassID() == Class_ID_Unit || Handle->GetClassID() == Class_ID_Archer ) 
		&& !CFcBaseObject::IsValid( Handle->GetParentLinkHandle() ) ) 
	{
		if( ( !m_HitParam.pHitSignal ) && ( Handle->IsCatchable() ) )
		{
			if( pHitParam->pHitSignal->m_Hit2Catch )
			{			
				if( Catch(Handle) )
				{
					Handle->SetRagDollMode( RAGDOLL_MODE_LINK );
					SendCatchMsg();
				}
			}
		}
	}

	CFcUnitObject::SendHitSignal( pHitParam, Handle );
}

void CFcHeroObject::HitSuccess()
{
	m_nComboCount++;
	m_nComboKeepFrame = DEFAULT_COMBO_KEEP_FRAME;

	int i;

	for( i = 0; i < KILL_COUNT_TABLE; i++ )
	{
		if( g_nComboCountTable[ i ] == m_nComboCount )
		{
			DropPrizeItem( g_nComboItemTable[ i ] );
		}
	}
}

void CFcHeroObject::AddKillCount( int nAdd, GameObjHandle Handle )
{ 
	if( g_FcWorld.GetAbilityMng()->GetRealAbilityCount() > 0 /*g_FcWorld.IsProcessTrueOrb( -1 )*/ )
	{
		m_nMaxOrbKillCount += nAdd;
	}	
	g_FcItemManager.NotifyUnitKill( Handle->GetCrossVector() );
	/*	if( m_bOrbSpark )
	{
	m_nOrbKillCount += nAdd;
	if( m_nMaxOrbKillCount < m_nOrbKillCount )
	{
	m_nMaxOrbKillCount = m_nOrbKillCount;
	}
	}*/
	m_nKillCount += nAdd; 

	int i;

	for( i = 0; i < KILL_COUNT_TABLE; i++ )
	{
		if( g_nKillCountTable[ i ] == m_nKillCount )
		{
			DropPrizeItem( g_nKillItemTable[ i ] );
		}
	}
}

void CFcHeroObject::DropPrizeItem( int nItemIndex )
{
	static float fDropPosX[ 8 ] = { 0.0f, 140.0f, 200.0f, 140.0f, 0.0f, -140.0f, -200.0f, -140.0f };
	static float fDropPosZ[ 8 ] = { 200.0f, 140.0f, 0.0f, -140.0f, -200.0f, -140.0f, 0.0f, 140.0f };
	int i;
	D3DXVECTOR3 Pos;

	for( i = 0; i < 8; i++ )
	{
		Pos = m_Cross.m_PosVector;
		Pos.x += fDropPosX[ i ];
		Pos.y += 100.0f;
		Pos.z += fDropPosZ[ i ];
		if( CFcWorld::GetInstance().GetAttr( Pos.x, Pos.z ) == 0 )
		{
			break;
		}
	}
	g_FcItemManager.CreateItemToWorld( nItemIndex, Pos );
}

void CFcHeroObject::SetEnableAI( bool bEnable )
{
	if( m_hUnitAIHandle == NULL )
		return;

	if( bEnable == m_hUnitAIHandle->IsEnable() ) return;

	if( bEnable == false ) {
		//		ChangeAnimation( ANI_TYPE_STAND, 0 );
		m_dwTickInterval = 1;
	}
	if( m_ParentLinkObjHandle && m_ParentLinkObjHandle->GetAIHandle() ) {
		m_ParentLinkObjHandle->GetAIHandle()->SetEnable( bEnable );
		m_hUnitAIHandle->SetEnable( false );
	}
	else m_hUnitAIHandle->SetEnable( bEnable );
}

void CFcHeroObject::AIInput( int nKeyCode )
{
	KEY_EVENT_PARAM Param;
	Param.nPosX = 0;
	Param.nPosY = 0;
	if( nKeyCode & 0x80000000 )
	{
		m_nInput = -1;
	}
	else
	{
		ProcessKeyEvent( nKeyCode, &Param );
	}
}

bool CFcHeroObject::CheckDefense()
{
	if( m_bDead ) return false;
	if( GetPlayerIndex() == -1 ) {
		if( !m_HitParam.ObjectHandle ) return false;

		// 진오브일 경우 걍 무시!!
		if( g_FcWorld.IsProcessTrueOrb( GetTeam() ) ) {
			ChangeAnimation( ANI_TYPE_DEFENSE, 3, ANI_ATTR_NONE, false );
			return true;
		}

		if( m_HitParam.ObjectHandle->IsGuardBreak( m_Handle ) || Random(100) < m_HitParam.nGuardBreak ) {
			ChangeAnimation( ANI_TYPE_DEFENSE, 4, ANI_ATTR_NONE, true );
			return false;
		}

		bool bResult = false;
		if( m_nCurAniType == ANI_TYPE_DEFENSE ) {
			ChangeAnimation( ANI_TYPE_DEFENSE, 3, ANI_ATTR_NONE, true );
			bResult = true;
		}
		if( bResult == false && Random( 100 ) < GetDefenseProb() ) {
			ChangeAnimation( ANI_TYPE_DEFENSE, 3, ANI_ATTR_NONE, false );
			bResult = true;
		}
		if( bResult == true ) {
			D3DXVECTOR3 vDir = m_HitParam.ObjectHandle->GetPos() - GetPos();
			vDir.y = 0.f;
			D3DXVec3Normalize( &vDir, &vDir );
			m_MoveDir = vDir;
			BsAssert( (m_MoveDir.x == m_MoveDir.x) && (m_MoveDir.y == m_MoveDir.y) && (m_MoveDir.z == m_MoveDir.z) );

			float fHitVelocityZ = ( float )m_HitParam.fVelocityZ * 0.6f;
			float fHitVelocityY = ( float )m_HitParam.fVelocityY * 0.6f;
			D3DXVECTOR3 Direction = m_HitParam.ObjectHandle->GetCrossVector()->m_PosVector - m_Cross.m_PosVector;

			Direction.y = 0.0f;
			D3DXVec3Normalize( &Direction, &Direction );

			GetExactHitVelocity( fHitVelocityZ, fHitVelocityY );

			SetVelocityX( Direction.x * fHitVelocityZ );
			SetVelocityZ( Direction.z * fHitVelocityZ );
			SetVelocityY( fHitVelocityY );
			return true;
		}
		return bResult;
	}
	else {
		if( m_nCurAniType == ANI_TYPE_STAND && IsAutoGuard() ) {
			ChangeAnimation( ANI_TYPE_DEFENSE, 3, ANI_ATTR_NONE, false );
		}
		if( m_nCurAniType != ANI_TYPE_DEFENSE ) return false;

		int nKindOf = ( m_nCurAniTypeIndex + 1 ) / 5;
		int nIndex = m_nCurAniTypeIndex % 5;
		switch( nIndex ) {
			case 1:	// 루프
				{
					if( !CFcBaseObject::IsValid( m_HitParam.ObjectHandle ) ) break;

					if( m_HitParam.ObjectHandle->IsGuardBreak( m_Handle ) || Random(100) < m_HitParam.nGuardBreak ) {
						if( m_HitParam.nGuardBreak == 1000 ) return false;
						ChangeAnimation( ANI_TYPE_DEFENSE, ( nKindOf * 5 ) + 4, ANI_ATTR_NONE, false );
						return true;
					}

					ChangeAnimation( ANI_TYPE_DEFENSE, ( nKindOf * 5 ) + 3, ANI_ATTR_NONE, false );

					D3DXVECTOR3 vDir = m_HitParam.ObjectHandle->GetPos() - GetPos();
					vDir.y = 0.f;
					D3DXVec3Normalize( &vDir, &vDir );
					m_MoveDir = vDir;
					BsAssert( (m_MoveDir.x == m_MoveDir.x) && (m_MoveDir.y == m_MoveDir.y) && (m_MoveDir.z == m_MoveDir.z) );

					float fHitVelocityZ = ( float )m_HitParam.fVelocityZ * 0.6f;
					float fHitVelocityY = ( float )m_HitParam.fVelocityY * 0.6f;
					D3DXVECTOR3 Direction = m_HitParam.ObjectHandle->GetCrossVector()->m_PosVector - m_Cross.m_PosVector;

					Direction.y = 0.0f;
					D3DXVec3Normalize( &Direction, &Direction );

					GetExactHitVelocity( fHitVelocityZ, fHitVelocityY );

					SetVelocityX( Direction.x * fHitVelocityZ );
					SetVelocityZ( Direction.z * fHitVelocityZ );
					SetVelocityY( fHitVelocityY );
					return true;
				}
				break;
			case 3:	// 타격
				{
					if( !CFcBaseObject::IsValid( m_HitParam.ObjectHandle ) ) break;

					if( m_HitParam.ObjectHandle->IsGuardBreak( m_Handle ) || Random(100) < m_HitParam.nGuardBreak ) {
						ChangeAnimation( ANI_TYPE_DEFENSE, ( nKindOf * 5 ) + 4, ANI_ATTR_NONE, false );
						return true;
					}

					ChangeAnimation( ANI_TYPE_DEFENSE, ( nKindOf * 5 ) + 3, ANI_ATTR_NONE, true );

					D3DXVECTOR3 vDir = m_HitParam.ObjectHandle->GetPos() - GetPos();
					vDir.y = 0.f;
					D3DXVec3Normalize( &vDir, &vDir );
					m_MoveDir = vDir;
					BsAssert( (m_MoveDir.x == m_MoveDir.x) && (m_MoveDir.y == m_MoveDir.y) && (m_MoveDir.z == m_MoveDir.z) );

					float fHitVelocityZ = ( float )m_HitParam.fVelocityZ * 0.6f;
					float fHitVelocityY = ( float )m_HitParam.fVelocityY * 0.6f;
					D3DXVECTOR3 Direction = m_HitParam.ObjectHandle->GetCrossVector()->m_PosVector - m_Cross.m_PosVector;

					Direction.y = 0.0f;
					D3DXVec3Normalize( &Direction, &Direction );

					GetExactHitVelocity( fHitVelocityZ, fHitVelocityY );

					SetVelocityX( Direction.x * fHitVelocityZ );
					SetVelocityZ( Direction.z * fHitVelocityZ );
					SetVelocityY( fHitVelocityY );

					return true;
				}
				break;
		}
		return false;
	}
	return false;
}

bool CFcHeroObject::RideOn( GameObjHandle RiderHandle )
{
	if( RiderHandle->LinkMe( m_Handle, m_nPlayerIndex ) == false ) return false;

	m_ParentLinkObjHandle = RiderHandle;

	// AI Change
	if( m_hUnitAIHandle && m_hUnitAIHandle->IsEnable() == true ) {
		m_hUnitAIHandle->SetEnable( false );
		if( m_ParentLinkObjHandle->GetAIHandle() )
			m_ParentLinkObjHandle->GetAIHandle()->SetEnable( true );
	}
	else {
		if( m_ParentLinkObjHandle->GetAIHandle() )
			m_ParentLinkObjHandle->GetAIHandle()->SetEnable( false );
	}

	return true;
}

bool CFcHeroObject::RideOut()
{
	if( !m_ParentLinkObjHandle ) return false;

	if( m_ParentLinkObjHandle->UnLinkMe( m_Handle ) == false ) return false;

	if( m_hUnitAIHandle && m_hUnitAIHandle->IsEnable() == true ) {
		m_hUnitAIHandle->SetEnable( true );
		if( m_ParentLinkObjHandle->GetAIHandle() ) 
			m_ParentLinkObjHandle->GetAIHandle()->SetEnable( false );
	}
	else {
		if( m_ParentLinkObjHandle->GetAIHandle() ) 
			m_ParentLinkObjHandle->GetAIHandle()->SetEnable( false );
	}

	m_ParentLinkObjHandle.Identity();
	return true;
}

void CFcHeroObject::ProcessUnlink( GameObjHandle Handle, int nX, int nY, int nZ )
{
	float fLength;

	D3DXMATRIX *pMatrix = (D3DXMATRIX *)g_BsKernel.SendMessage( Handle->GetEngineIndex(), BS_GET_OBJECT_MAT );
// [PREFIX:beginmodify] 2006/2/25 junyash PS#5815 dereferencing NULL pointer
	BsAssert(pMatrix);
// [PREFIX:endmodify] junyash
	Handle->GetCrossVector()->m_PosVector.x = pMatrix->_41;
	Handle->GetCrossVector()->m_PosVector.y = pMatrix->_42;
	Handle->GetCrossVector()->m_PosVector.z = pMatrix->_43;

	if( CFcWorld::GetInstance().GetHeroHandle( 0 ) == m_Handle )
	{
		Handle->CreateOrbObject( m_Handle, GENERATE_ORB_NORMAL, 100 );
	}
	AddExp( Handle->GetDischargeOrbSpark(), Handle, 1 );
	Handle->SetDie();
	Handle->SetRagDollMode( RAGDOLL_MODE_FREE );

	m_UnlinkVelocity = D3DXVECTOR3( ( float )nX, ( float )nY, ( float )nZ );
	fLength = D3DXVec3Length( &m_UnlinkVelocity );
	fLength = BsMin( fLength, 100.f );
	D3DXVec3Normalize( &m_UnlinkVelocity, &m_UnlinkVelocity );
	D3DXVec3TransformNormal( &m_UnlinkVelocity, &m_UnlinkVelocity, m_Cross );
	m_UnlinkVelocity *= fLength;
}

void CFcHeroObject::UnlinkCatchObject( int nX, int nY, int nZ, bool bEvent /*= false*/, int nHandIndex /*= -1*/ )
{
	if( ( m_CatchHandle ) && ( nHandIndex != 1 ) )
		{
		ProcessUnlink( m_CatchHandle, nX, nY, nZ );
	}
	SendUnlinkCatchMsg( nHandIndex );
}

void CFcHeroObject::UnlinkCatch( ASSignalData *pSignal )
{
	if( ( pSignal->m_nFrame >= m_fFrame ) && ( pSignal->m_nFrame < m_fFrame + GetFrameAdd() ) )
	{
		if( pSignal->m_NumParam >= 5 )	// 삭제 요망 by mapping
		{
			UnlinkCatchObject( pSignal->m_pParam[ 1 ], pSignal->m_pParam[ 2 ], pSignal->m_pParam[ 3 ], false, pSignal->m_pParam[ 4 ] );
		}
		else
		{
			UnlinkCatchObject( pSignal->m_pParam[ 1 ], pSignal->m_pParam[ 2 ], pSignal->m_pParam[ 3 ] );
		}
	}
}

void CFcHeroObject::CmdHit( HIT_PARAM *pHitParam, D3DXVECTOR3 *pHitDir/* = NULL*/ )
{
	UnlinkCatchObject( 0, -1, 0 );

	CFcUnitObject::CmdHit( pHitParam, pHitDir );
}

void CFcHeroObject::CmdPush()
{
	UnlinkCatchObject( 0, -1, 0 );
}

bool CFcHeroObject::IsFlockable( GameObjHandle Handle )
{
	if( Handle == m_Handle ) return false;
	if( Handle->GetParentLinkHandle() == m_Handle ) return false;

	switch( Handle->GetClassID() ) {
		case Class_ID_Hero:	return true;
			/*
			if( ((CFcHeroObject*)Handle.GetPointer())->GetPlayerIndex() != -1 ) {
				switch( ((CFcHeroObject*)Handle.GetPointer())->GetHeroClassID() ) {
					case Class_ID_Hero_VigkVagk:	return true;
					default:	return false;
				}
				return true;
			}
			return true;
			*/

		case Class_ID_Catapult: return true;
		case Class_ID_Adjutant:	
			switch( ((CFcAdjutantObject*)Handle.GetPointer())->GetAdjutantClassID() ) {
				case Class_ID_Adjutant_KingFrog:	return true;
				case Class_ID_Adjutant_99King:		return true;
				default:
					if( IsJump() ) return false;
					if( Handle->GetTeam() == GetTeam() ) return false;
					return true;
					break;
			}
			break;
		default:
			break;
	}
	return false;
}

void CFcHeroObject::ReleaseWeaponTrailTexture()
{
	int nSize = m_WeaponTrailTexIDs.size();
	for(int ii = 0; ii <  nSize ; ++ii)
	{
		SAFE_RELEASE_TEXTURE(m_WeaponTrailTexIDs[ii]);
	}
	m_WeaponTrailTexIDs.clear();

	SAFE_RELEASE_TEXTURE(m_nWeaponTrailOffsetTexID);
}

int	CFcHeroObject::GetWeaponTrailTextureID(int nIdx)
{
	if(nIdx >= (int)m_WeaponTrailTexIDs.size())
		return -1;

	return m_WeaponTrailTexIDs[nIdx];
}

bool CFcHeroObject::GetExactHitAni( int &nAniType, int &nAniIndex )
{
	if( m_bAlwaysSmallDamage )
	{
		if( nAniType == ANI_TYPE_HIT )
		{
			nAniIndex = 0;
		}
		else if( nAniType == ANI_TYPE_DOWN ) 
		{
			nAniType = ANI_TYPE_HIT;
			nAniIndex = 0;
		}
	}

	return true;
}


int CFcHeroObject::FindExactAni( int nAniIndex )
{
	return nAniIndex;
}

/*
void CFcHeroObject::ChangeWeapon( char *pSkinName, int nAttackPower )
{
int nSkinIndex, nObjectIndex;

nSkinIndex = g_BsKernel.LoadSkin( -1, pSkinName );

if(nSkinIndex == -1)
{
BsAssert(0);
return;
}

nObjectIndex = g_BsKernel.CreateStaticObjectFromSkin( nSkinIndex );
g_BsKernel.SendMessage( m_nEngineIndex, BS_UNLINKOBJECT_BONE, m_WeaponList[ m_nCurrentWeaponIndex ].nLinkBoneIndex );
g_BsKernel.DeleteObject( m_WeaponList[ m_nCurrentWeaponIndex ].nObjectIndex );
m_WeaponList[m_nCurrentWeaponIndex].nObjectIndex = -1;

g_BsKernel.SendMessage( m_nEngineIndex, BS_LINKOBJECT, 
( DWORD )m_WeaponList[ m_nCurrentWeaponIndex ].nLinkBoneIndex, nObjectIndex );
g_BsKernel.ShowObject( nObjectIndex, true );
m_WeaponList[ m_nCurrentWeaponIndex ].nObjectIndex = nObjectIndex;
}
*/

bool CFcHeroObject::IsChangableWeapon()
{
	if( m_CatchHandle )
	{
		return false;
	}
	if( IsOrbSparkOn() )
	{
		return false;
	}
	if( ( m_nCurAniType != ANI_TYPE_STAND ) && ( m_nCurAniType != ANI_TYPE_WALK ) && ( m_nCurAniType != ANI_TYPE_RUN ) )
	{
		return false;
	}

	return true;
}

bool CFcHeroObject::ChangeWeapon( int nSkinIndex, int nAttackPower )
{
	BsAssert( nSkinIndex >= 0 );

	if( m_CatchHandle )
	{
		return false;
	}

	m_WeaponList[m_nCurrentWeaponIndex].nEngineIndex = nSkinIndex;
	int nObjectIndex = g_BsKernel.CreateStaticObjectFromSkin( nSkinIndex );
	g_BsKernel.SendMessage( m_nEngineIndex, BS_UNLINKOBJECT_BONE, m_WeaponList[ m_nCurrentWeaponIndex ].nLinkBoneIndex );
	g_BsKernel.DeleteObject( m_WeaponList[ m_nCurrentWeaponIndex ].nObjectIndex );
	m_WeaponList[m_nCurrentWeaponIndex].nObjectIndex = -1;

	g_BsKernel.SendMessage( m_nEngineIndex, BS_LINKOBJECT, 
		( DWORD )m_WeaponList[ m_nCurrentWeaponIndex ].nLinkBoneIndex, nObjectIndex );
	g_BsKernel.ShowObject( nObjectIndex, true );
	m_WeaponList[ m_nCurrentWeaponIndex ].nObjectIndex = nObjectIndex;

	return true;
}

int CFcHeroObject::GetAttackPower()
{
	int nPower;

	nPower = CFcUnitObject::GetAttackPower();
	if( IsOrbSparkOn() )
	{
		nPower += ( int )( nPower * ( m_nOrbAttackPowerAdd / 100.0f ) );
	}

	return nPower;
}

void CFcHeroObject::CmdMove( int nX, int nY, float fSpeed, int nAniType, int nAniIndex )
{
	if( IsDie() ) return;
	if( !IsMovable() ) return;

	D3DXVECTOR3 vTemp = D3DXVECTOR3( (float)nX, 0.f, (float)nY );
	D3DXVECTOR3 Pos = m_Cross.GetPosition();

	ResetMoveVector();

	float fTemp;
	if( fSpeed == -1.f ) {
		//		if( nAniType == ANI_TYPE_WALK || GetPlayerIndex() == -1 ) {
		if( nAniType == ANI_TYPE_WALK ) {
			int nIndex = m_pUnitInfoData->GetAniIndex( nAniType & 0x00FF, nAniIndex, nAniType & 0xFF00 );
			fTemp = (float)m_pUnitInfoData->GetMoveSpeed( nIndex );
		}
		else {
			fTemp = CHeroLevelTableLoader::GetInstance().GetMoveSpeed( m_nUnitSOXIndex, m_nLevel ) * GetFrameAdd() * CalcMoveSpeedAdd();
		}
	}
	else fTemp = fSpeed;

	float fLength = D3DXVec3Length( (D3DXVECTOR3*)&(vTemp - D3DXVECTOR3( Pos.x, 0.f, Pos.z )) );
	if( fLength < fTemp || fLength < (float)GetUnitRadius() ) {
		m_MoveTargetPos = Pos;
		return;
	}

	m_fMoveSpeed = fTemp;
	m_MoveTargetPos = vTemp;
	m_MoveDir = vTemp - D3DXVECTOR3( Pos.x, 0.f, Pos.z );

	BsAssert( (m_MoveDir.x == m_MoveDir.x) && (m_MoveDir.y == m_MoveDir.y) && (m_MoveDir.z == m_MoveDir.z) );
	D3DXVec3Normalize(&m_MoveDir, &m_MoveDir);

	m_nChangeMoveAniIndex = m_pUnitInfoData->GetAniIndex( nAniType & 0x00FF, nAniIndex, nAniType & 0xFF00 );

	SetFlocking( false );
}

void CFcHeroObject::CmdStop( int nAniType, int nAniIndex )
{
	if( nAniIndex == -1 ) {
		nAniIndex = 0;
	}
	CFcUnitObject::CmdStop( nAniType, nAniIndex );
}

void CFcHeroObject::Resume()
{
	if( ( m_nCurAniType == ANI_TYPE_RUN ) || ( m_nCurAniType == ANI_TYPE_WALK ) )
	{
		if( CInputPad::GetInstance().GetKeyPressTick( g_FCGameData.nEnablePadID, PAD_INPUT_LSTICK ) == 0 )
		{
			ChangeAnimation( ANI_TYPE_STAND, 0 );
		}
	}
}

int CFcHeroObject::GetOrbPercent()
{
	int nCombo, nPercent;

	nPercent = 0;
	nCombo = GetComboCount();
	if( nCombo > 500 )
	{
		nPercent = nCombo / 500 * 20;
		if( nPercent > 100 )
		{
			nPercent = 100;
		}
	}
	else if( nCombo > 300 )
	{
		nPercent = 10;
	}

	return nPercent;
}


void CFcHeroObject::SetDie( int nWeaponDropPercent )
{
	switch( m_nDieType ) {
		case 0:
			if( m_nDieFrame == 0 ) {
				m_nDieFrame = 1;
				if( m_hTroop ) m_hTroop->CmdDie( m_Handle );
			}
			m_bDead = true;
			SetInScanner( false );
			break;
		case 1:
			if( m_nDieRenewFrame == 0 ) m_nDieRenewFrame = 1;
			break;
		case 2:
			if( m_nDieRenewFrame == 0 ) {
				m_hTroop->CmdDie( m_Handle );
				m_nDieRenewFrame = 1;

				if( m_hTroop ) {
					m_hTroop->CmdDie( m_Handle );
				}
			}
			SetInScanner( false );
			break;
	}
}

bool CFcHeroObject::IsDie( bool bCheckEnable )
{
	switch( m_nDieType ) {
		case 0:	
			if( bCheckEnable && !m_bEnable ) return true;
			return m_bDead;
		case 1:	return false;
		case 2:	return ( m_nDieRenewFrame == 0 ) ? false : true;
	}
	return true;
}


bool CFcHeroObject::Render()
{
	switch( m_nDieType ) {
		case 1:
			m_bDead = false;
			break;
		default:	break;
	}
	return CFcUnitObject::Render();
}