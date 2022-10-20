#include "StdAfx.h"
#include "InputPad.h"
#include ".\inphy.h"
#include "FcCameraObject.h"
#include ".\\data\\AnimType.h"
#include ".\\data\\SignalType.h"

#include "FcGlobal.h"
#include "FcWorld.h"
#include "FcFxCommon.h"
#include "FcFxManager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG


CInphy::CInphy( CCrossVector *pCross )
	: CFcHeroObject( pCross )
{
	SetHeroClassID( Class_ID_Hero_Inphy );
	m_FindHandle.Identity();
	m_LockHandle.Identity();
	m_nLockonMoveFrame = 0;
	m_bInitFound = false;
	m_bFreezCamera = false;
	m_fRagdollDistY = 0.f;
}

CInphy::~CInphy(void)
{
}

bool CInphy::Render()
{
	if( m_CatchHandle ) {    		
		if( m_fRagdollDistY > 0.f ) {
			float fAngleScale = ( (m_fRagdollDistY) / 85.f );
			fAngleScale = BsMin(1.2f, fAngleScale);	
			g_BsKernel.SendMessage( m_nEngineIndex, BS_SET_CLEAR_BONE_ROTATION );
			g_BsKernel.SendMessage( m_nEngineIndex, BS_SET_BONE_ROTATION, (DWORD)"Bip01 Spine", (DWORD)&D3DXVECTOR3( -10.f*fAngleScale, 0.f, 0.f ) );
			g_BsKernel.SendMessage( m_nEngineIndex, BS_SET_BONE_ROTATION, (DWORD)"Bip01 Spine1", (DWORD)&D3DXVECTOR3( -10.f*fAngleScale, 0.f, 0.f ) );
			g_BsKernel.SendMessage( m_nEngineIndex, BS_SET_BONE_ROTATION, (DWORD)"Bip01 Spine2", (DWORD)&D3DXVECTOR3( -10.f*fAngleScale, 0.f, 0.f ) );
		}
		else {
			g_BsKernel.SendMessage( m_nEngineIndex, BS_SET_CLEAR_BONE_ROTATION );
		}
	}
	return CFcHeroObject::Render();
}

void CInphy::TrampleCheck()
{
	GameObjHandle Handle;
	D3DXVECTOR3 Dist;

	if( ( m_nCurAniTypeIndex == 1 ) && ( m_fFrame == 0.0f ) )	
	{
		if( GetPlayerIndex() != -1 )
		{
			if( CInputPad::GetInstance().GetKeyPressTick( g_FCGameData.nEnablePadID, PAD_INPUT_LSTICK ) == 0 )
			{
				m_nCurAniTypeIndex = 0;
				m_nAniIndex = m_pUnitInfoData->GetAniIndex( ANI_TYPE_JUMP_UP, m_nCurAniTypeIndex, 0);
			}
		}
		return;
	}
/*	if( CInputPad::GetInstance().GetKeyPressTick( g_FCGameData.nEnablePadID, PAD_INPUT_A ) == 1 )
	{
		FindTramplableObject( &m_Cross.m_PosVector, Handle, 150.0f, -50.0f, 50.0f );
		if( Handle )
		{
			m_Cross.m_PosVector.y = Handle->GetPos().y + Handle->GetUnitHeight();
			ChangeAnimation( ANI_TYPE_JUMP_UP, 0, 0, true );
			Handle->ChangeAnimation( ANI_TYPE_HIT, 0 );
		}
	}
	else*/ if( CInputPad::GetInstance().GetKeyPressTick( g_FCGameData.nEnablePadID, PAD_INPUT_LSTICK ) )
	{
		if( ( m_nCurAniTypeIndex == 0 ) || ( m_nCurAniTypeIndex == 1 ) )
		{
			if( m_vVelocity.y < 0.0f )
			{
				FindTramplableObject( &m_Cross.m_PosVector, Handle, 150.0f, -50.0f, 0.0f );
				if( Handle )
				{
					Dist = Handle->GetPos() - m_Cross.m_PosVector;
					Dist.y = Handle->GetPos().y + Handle->GetUnitHeight() - m_Cross.m_PosVector.y;
					m_MoveVector += Dist;
					ChangeAnimation( ANI_TYPE_JUMP_UP, 3, 0, true );
					m_fJumpSpeed = 0.0f;
					m_fGravity = 0.0f;
					m_vVelocity.y = 0.0f;
					m_FindHandle = Handle;
				}
			}
		}
		else if( m_nCurAniTypeIndex == 4 )
		{
			if( m_fFrame >= m_nCurAniLength - 1 )
			{
				FindTramplableObject( &m_Cross.m_PosVector, Handle, 150.0f, -50.0f, 0.0f );
				if( Handle )
				{
					Dist = Handle->GetPos() - m_Cross.m_PosVector;
					Dist.y = Handle->GetPos().y + Handle->GetUnitHeight() - m_Cross.m_PosVector.y;
					m_MoveVector += Dist;
					ChangeAnimation( ANI_TYPE_JUMP_UP, 3, 0, true );
					m_fJumpSpeed = 0.0f;
					m_fGravity = 0.0f;
					m_vVelocity.y = 0.0f;
					m_FindHandle = Handle;
				}
			}
		}
		else if( m_nCurAniTypeIndex == 3 )
		{
			if( m_fFrame >= m_nCurAniLength - 1 )
			{
				GameObjHandle NextHandle;
				D3DXVECTOR3 NextPos;

				NextPos = m_Cross.m_PosVector + m_Cross.m_ZVector * ( 21 * TRAMPLE_STEP );
				FindTramplableObject( &NextPos, NextHandle, 100.0f, -100.0f, 100.0f );
				if( NextHandle )
				{
					NextPos = NextHandle->GetPos() - m_Cross.m_PosVector;
					NextPos.y += NextHandle->GetUnitHeight();
					m_CustomMoveStep.y = NextPos.y / ( float )m_nCurAniLength;
					NextPos.y = 0.0f;
					m_CustomMoveStep.z = D3DXVec3Length( &NextPos ) / ( float )m_nCurAniLength;
					m_CustomMoveStep.x = 0.0f;

					NextPos = NextHandle->GetPos();
					NextPos.y = m_Cross.m_PosVector.y;
					m_Cross.LookAt( &NextPos );
					ChangeAnimation( ANI_TYPE_JUMP_UP, 4, 0, true );
					m_fGravity = 0.0f;
					m_vVelocity.y = 0.0f;
				}
				else
				{
					ChangeAnimation( ANI_TYPE_JUMP_UP, 5, 0, true );
				}
			}
		}
		if( Handle )
		{
			Handle->ChangeAnimation( ANI_TYPE_HIT, 0 );

		}
	}
	else if( m_nCurAniTypeIndex == 3 )
	{
		if( m_fFrame >= m_nCurAniLength - 1 )
		{
			ChangeAnimationByIndex( 98 );
		}
	}
	if( m_nCurAniTypeIndex == 4 )
	{
		MoveZ( m_CustomMoveStep.z );
		MoveY( m_CustomMoveStep.y );
	}
}

void CInphy::LockAttack()
{
	CameraObjHandle CamHandle;
	CamHandle = CFcBaseObject::GetCameraObjectHandle( m_nPlayerIndex );
	if( CamHandle )
	{
		CamHandle->SetFreeze( true );
		m_bFreezCamera = true;
	}
	if( !m_bInitFound )
	{
		m_FindResult.clear();
		CFcWorld::GetInstance().GetObjectListInRange( (D3DXVECTOR3*)&GetPos(), 600.0f, m_FindResult );
		std::vector< GameObjHandle >::iterator it;

		it = m_FindResult.begin();
		while( it != m_FindResult.end() )
		{
			if( *it == m_Handle )
			{
				it = m_FindResult.erase( it );
				continue;
			}
			if( ( *( it ) )->GetGravity() == 0.0f )
			{
				it++;
				continue;
			}
			it = m_FindResult.erase( it );
		}
		m_bInitFound = true;
	}
	if( !m_LockHandle )
	{
		int i, nCount, nMaxIndex;
		float fMaxDist, fLength;
		D3DXVECTOR3 LookAtPos;

		nCount = ( int )m_FindResult.size();
		fMaxDist = 0.0f;
		nMaxIndex = -1;
		for( i = 0; i < nCount; i++ )
		{
			if( m_FindResult[ i ]->GetGravity() != 0.0f )
			{
				continue;
			}
			fLength = D3DXVec3Length( &( m_Cross.m_PosVector - m_FindResult[ i ]->GetPos() ) );
			if( fLength > fMaxDist )
			{
				fMaxDist = fLength;
				nMaxIndex = i;
			}
		}
		if( nMaxIndex != -1 )
		{
			m_LockHandle = m_FindResult[ nMaxIndex ];
			m_LockTargetPos = m_LockHandle->GetPos();
			LookAtPos = m_LockTargetPos;
			LookAtPos.y = m_Cross.m_PosVector.y;
			m_Cross.LookAt( &LookAtPos );
			m_CustomMoveStep.z = 60.0f;
			fLength = D3DXVec3Length( &( m_Cross.m_PosVector - m_LockTargetPos ) );
			m_nLockonMoveFrame = ( int )( fLength / 60.0f ) + 1;
			if( m_nLockonMoveFrame <= 2 )
			{
				m_CustomMoveStep.z = 40.0f;
				m_nLockonMoveFrame = 3;
			}
			m_FindResult.erase( m_FindResult.begin() + nMaxIndex );
			m_CustomMoveStep.y = ( m_LockTargetPos.y - m_Cross.m_PosVector.y - 100.0f ) / m_nLockonMoveFrame;
		}
		if( !m_LockHandle )
		{
			ChangeAnimationByIndex( LOCKON_ATTACK_FALL );
			m_bInitFound = false;
		}
	}
	if( m_LockHandle )
	{
		MoveZ( m_CustomMoveStep.z );
		MoveY( m_CustomMoveStep.y );
		if( m_nLockonMoveFrame <= 0 )
		{
			m_LockHandle.Identity();
			if( m_FindResult.size() == 0 )
			{
				ChangeAnimationByIndex( LOCKON_ATTACK_FALL );
				m_bInitFound = false;
			}
		}
	}
	m_nLockonMoveFrame--;
}

void CInphy::CheckSAttack()
{
	if( ( GetLevel() >= 3 ) && ( GetLevel() <= 6 ) )
	{
		if( m_fFrame > m_nCurAniLength - 1 )
		{
			ChangeAnimationByIndex( 106 );
		}
	}
	else if( GetLevel() >= 7 )
	{
		if( m_fFrame > m_nCurAniLength - 1 )
		{
			ChangeAnimationByIndex( 113 );
		}
		if( CInputPad::GetInstance().GetKeyPressTick( g_FCGameData.nEnablePadID, PAD_INPUT_Y ) == 0 )
		{
			ChangeAnimationByIndex( 106 );
		}
	}
}

void CInphy::Process()
{
	if( m_nCurAniType == ANI_TYPE_JUMP_UP )
	{
		TrampleCheck();
	}
	if( m_nAniIndex == LOCKON_ATTACK_START )
	{
		LockAttack();
	}
	else if( m_nAniIndex == 117 )
	{
		if( !m_bFreezCamera )	// 우선 이렇게 땜빵으로 하자.. 나중에 정리
		{
			CameraObjHandle CamHandle;
			CamHandle = CFcBaseObject::GetCameraObjectHandle( m_nPlayerIndex );
			if( CamHandle )
			{
				CamHandle->SetFreeze( true );
				m_bFreezCamera = true;
			}
		}
	}
	else if( m_bFreezCamera )
	{
		CameraObjHandle CamHandle;
		CamHandle = CFcBaseObject::GetCameraObjectHandle( m_nPlayerIndex );
		if( CamHandle )
		{
			CamHandle->SetFreeze( false );
			m_bFreezCamera = false;
		}
	}
/*	else if( m_nAniIndex == INPHY_SATTACK_Y )
	{
		CheckSAttack();
	}*/
/*	if( ( m_nCurAniType == ANI_TYPE_JUMP_UP ) && ( m_fFrame <= 1.0f ) )
		if( ( m_nCurAniTypeIndex == 0 ) && ( m_bFrontJump) )
	{
		m_nAniIndex = m_pUnitInfoData->GetAniIndex( ANI_TYPE_JUMP_UP, 1 );
		m_nCurAniTypeIndex = 1;
		m_bFrontJump = false;
	}*/

	CFcHeroObject::Process();
}

void CInphy::FindTramplableObject( D3DXVECTOR3 *pPosition, GameObjHandle &Handle, float fCheckRadius, float fSearchMin, float fSearchMax )
{
	int i, nSize, nMinIndex;
	float fDist, fMinDist;
	D3DXVECTOR3 Direction;
	std::vector< GameObjHandle > Result;

	CFcWorld::GetInstance().GetObjectListInRange( pPosition, fCheckRadius, Result );
	nSize = ( int )Result.size();
	fMinDist = FLT_MAX;
	nMinIndex = -1;
	for( i = 0; i < nSize; i++ )
	{
		if( !Result[ i ]->IsTramplable() )
		{
			continue;
		}
		if( Result[ i ] == m_FindHandle )
		{
			continue;
		}
		Direction = *pPosition - Result[ i ]->GetPos();
		Direction.y = 0.0f;
		fDist = D3DXVec3LengthSq( &Direction );
		if( fDist < fMinDist )
		{
			fMinDist = fDist;
			nMinIndex = i;
		}
	}
	if( nMinIndex != -1 )
	{
		float fUnitHeight;

		fUnitHeight = Result[ nMinIndex ]->GetPos().y + Result[ nMinIndex ]->GetUnitHeight();
		if( ( pPosition->y > fUnitHeight + fSearchMax ) || ( pPosition->y < fUnitHeight + fSearchMin ) )
		{
			return;
		}
		Handle = Result[ nMinIndex ];
	}
}


void CInphy::LoadWeaponTrailTexture()
{
	int nTextureId;
	g_BsKernel.chdir("Fx");
	
	nTextureId = g_BsKernel.LoadTexture( "Inphyy_SwordTrail01.dds" );
	if( nTextureId != -1 ) 
		m_WeaponTrailTexIDs.push_back(nTextureId);
	nTextureId = -1;

	nTextureId = g_BsKernel.LoadTexture( "Inphyy_SwordTrail02.dds" );
	if( nTextureId != -1 ) 
		m_WeaponTrailTexIDs.push_back(nTextureId);
	nTextureId = -1;

	nTextureId = g_BsKernel.LoadTexture( "Inphyy_SwordTrail03.dds" );
	if( nTextureId != -1 ) 
		m_WeaponTrailTexIDs.push_back(nTextureId);

	m_nWeaponTrailOffsetTexID = g_BsKernel.LoadTexture( "Inphyy_SwordTrail_Normals.dds" );
	g_BsKernel.chdir("..");
}

bool CInphy::Catch( GameObjHandle Handle )
{
	if( ( m_CatchHandle ) || ( m_nCurrentWeaponIndex == -1 ) )
	{
		return false;
	}
	if( Handle->IsDie() ) return false;

	int nLinkCount;

	nLinkCount = g_BsKernel.SendMessage( m_WeaponList[ m_nCurrentWeaponIndex ].nObjectIndex, BS_GET_LINKDUMMY_COUNT );
	if( nLinkCount <= 0 )
	{
		return false;
	}
	Handle->RideOut();

	m_CatchHandle = Handle;
	m_CatchHandle->RunRagdoll();
	g_BsKernel.SendMessage( m_CatchHandle->GetEngineIndex(), BS_PHYSICS_SIMULATION, TRUE );

	//g_BsKernel.SendMessage( m_WeaponList[ m_nCurrentWeaponIndex ].nObjectIndex, BS_LINKOBJECT, 0, 
		//m_CatchHandle->GetEngineIndex(), nBoneIndex );
	if( m_nAniIndex == 86 )
	{
		ChangeAnimation( ANI_TYPE_ATTACK, 19 );
	}
	m_CatchHandle->ChangeAnimation( ANI_TYPE_DOWN, 0 );	// Ragdoll 상태일때 AI 못돌게 할려고..
	m_nCatchBoneIndex = g_BsKernel.SendMessage( m_CatchHandle->GetEngineIndex(), BS_GET_BONE_INDEX, ( DWORD )"Bip01 Spine1" );

	return true;
}

void CInphy::SendCatchMsg()
{
	if( m_CatchHandle )
	{
		int nCatchEngineIndex = m_CatchHandle->GetEngineIndex();

		BSMATRIX *pMatrix = (BSMATRIX*)g_BsKernel.SendMessage( nCatchEngineIndex, BS_GET_BONE_WORLD_MATRIX, (DWORD)"Bip01 Spine1");
		m_fRagdollDistY = (m_CatchHandle->GetCrossVector()->GetPosition().y - GetCrossVector()->GetPosition().y) + (pMatrix->_42 - 85.f);
		
		m_CatchHandle->SetCatchedHero( true );
		DebugString("CInphy::Render() Catch : LinkObject(%d)\n", nCatchEngineIndex);
		g_BsKernel.SendMessage( m_WeaponList[ m_nCurrentWeaponIndex ].nObjectIndex, BS_LINKOBJECT, 0, nCatchEngineIndex, m_nCatchBoneIndex );
		g_BsKernel.SendMessage( nCatchEngineIndex, BS_SET_RAGDOLL_CATCH_BONE, m_nCatchBoneIndex);
	}
}

void CInphy::SendUnlinkCatchMsg( int nHandIndex /*= -1*/ )
{
	if( m_CatchHandle )
	{
		int nCatchEngineIndex = m_CatchHandle->GetEngineIndex();
		m_CatchHandle->SetCatchedHero( false );
		DebugString("CInphy::Render() Uncatch : UnLinkObject(%d)\n", nCatchEngineIndex);
		g_BsKernel.SendMessage( m_WeaponList[ m_nCurrentWeaponIndex ].nObjectIndex, BS_UNLINKOBJECT, nCatchEngineIndex, ( DWORD )&m_UnlinkVelocity );
		m_CatchHandle->UnlinkRagdoll();
		m_CatchHandle.Identity();
		g_BsKernel.SendMessage( m_nEngineIndex, BS_SET_CLEAR_BONE_ROTATION );
	}
}

void CInphy::InitOrbAttack2DFx()
{
	m_nMotionBlurIndex = g_pFcFXManager->Create( FX_TYPE_INPHYMOTIONBLUR );
	g_pFcFXManager->SendMessage( m_nMotionBlurIndex, FX_INIT_OBJECT );
}

void CInphy::SignalCheck( ASSignalData *pSignal )
{
	switch( pSignal->m_nID )
	{
		case SIGNAL_TYPE_HIT_TEMP:
			if( ( m_nAniIndex == 95 ) && ( !m_bTrampleDamage ) )
			{
				return;
			}
			else
			{
				CFcHeroObject::SignalCheck( pSignal );
			}
			break;
		default:
			CFcHeroObject::SignalCheck( pSignal );
			break;
	}
}
