#include "StdAfx.h"
#include "AsData.h"
#include ".\\data\\AnimType.h"
#include ".\\data\\SignalType.h"
#include ".\Myifee.h"
#include "FcFxManager.h"
#include "FcWorld.h"
#include "Sphere.h"
#include "IntBox3Frustum.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG


CMyifee::CMyifee( CCrossVector *pCross )
	: CFcHeroObject( pCross )
{
	SetHeroClassID( Class_ID_Hero_Myifee );
	m_fMaxThrowLength = 1000.0f;
	m_nLineHandle = -1;
	m_bThrowMode = false;
	m_fThrowLength = 0.0f;
	m_pThrowTrailSignal = NULL;
}

CMyifee::~CMyifee(void)
{
}

void CMyifee::Initialize( INITGAMEOBJECTDATA* pData, int nForce, int nTeam, TroopObjHandle hTroop, float fScaleMin, float fScaleMax )
{
	int nPointCount;
	float fDistant, fLineWidth;

	m_nLineHandle = g_pFcFXManager->Create( FX_TYPE_LINECURVE );
	g_pFcFXManager->SendMessage( m_nLineHandle, FX_SET_TEXTURE, ( DWORD )"C_LP_WM_CHIAN_Fx.dds" );
	nPointCount = 3;
	fDistant = 5.0f;
	fLineWidth = 5.0f;
	g_pFcFXManager->SendMessage( m_nLineHandle, FX_INIT_OBJECT, ( DWORD )nPointCount, ( DWORD )&fDistant, ( DWORD )&fLineWidth );
	g_pFcFXManager->SendMessage( m_nLineHandle, FX_PLAY_OBJECT );

	m_nTrailHandle[ 1 ] = g_pFcFXManager->Create( FX_TYPE_WEAPONTRAIL );
	g_pFcFXManager->SendMessage( m_nTrailHandle[ 1 ], FX_INIT_OBJECT );

	CFcHeroObject::Initialize( pData, nForce, nTeam, hTroop, fScaleMin, fScaleMax );
}

void CMyifee::ProcessThrowWeapon()
{
	float m_fBackup;

	m_fBackup = m_fThrowLength;
	m_fThrowLength += m_fMaxThrowLength * m_fThrowSpeed;
	if( m_fThrowLength < 0.f )
	{
		m_fThrowLength = 0.f;
		ThrowWeapon( false );
	}
	else
	{
		D3DXVECTOR3 WeaponPos;
		float fHeight;

		if( m_fThrowLength > m_fMaxThrowLength )
		{
			m_fThrowSpeed = 0.0f;
			m_fThrowLength = m_fMaxThrowLength;
		}

		D3DXVECTOR3 Direction, Positions[ 3 ];

		Direction.x = m_WeaponMat._41 - m_HandMat._41;
		Direction.y = m_WeaponMat._42 - m_HandMat._42;
		Direction.z = m_WeaponMat._43 - m_HandMat._43;
		D3DXVec3Normalize( &Direction, &Direction );

		WeaponPos.x = m_HandMat._41 + Direction.x * ( m_fThrowLength + 100.0f );
		WeaponPos.y = m_HandMat._42 + Direction.y * m_fThrowLength;
		WeaponPos.z = m_HandMat._43 + Direction.z * ( m_fThrowLength + 100.0f );
		
		fHeight = CFcWorld::GetInstance().GetLandHeight( WeaponPos.x, WeaponPos.z );
		if( fHeight > WeaponPos.y - 30.0f )
		{
			m_fThrowLength = m_fBackup;
		}
		m_WeaponMat._41 = m_HandMat._41 + Direction.x * m_fThrowLength;
		m_WeaponMat._42 = m_HandMat._42 + Direction.y * m_fThrowLength;
		m_WeaponMat._43 = m_HandMat._43 + Direction.z * m_fThrowLength;

		memcpy( Positions, &m_HandMat._41, sizeof( D3DXVECTOR3 ) );
		memcpy( Positions + 2, &m_WeaponMat._41, sizeof( D3DXVECTOR3 ) );
		memcpy( Positions + 1, &m_CenterPos, sizeof( D3DXVECTOR3 ) );
		g_pFcFXManager->SendMessage( m_nLineHandle, FX_UPDATE_OBJECT, 3, ( DWORD )Positions );
	}
}

void CMyifee::Process()
{
	if( ( m_nAniIndex == 72 ) && ( m_fFrame > 54.0f ) && ( m_CatchHandle ) )
	{
		ChangeAnimationByIndex( 73 );
	}

	CFcHeroObject::Process();

	if( m_bThrowMode )
	{
		if( ( m_nCurAniType != ANI_TYPE_ATTACK ) && ( m_nCurAniType != ANI_TYPE_JUMP_ATTACK ) )
		{
			CancelThrow();
		}
		ProcessThrowWeapon();
	}
	if( m_pThrowTrailSignal )
	{
		AddThrowTrail( ( int )( m_fFrame - m_pThrowTrailSignal->m_nFrame ), m_pThrowTrailSignal, 0 );
	}
}

ASSignalData *g_pDebugSignal;
void CMyifee::SignalCheck( ASSignalData *pSignal )
{
	g_pDebugSignal = pSignal;
	switch( pSignal->m_nID )
	{
		case SIGNAL_TYPE_HIT_TEMP:
			if( m_bThrowMode )
			{
				HitCheckTempMyifee( pSignal );
				HitCheckTempMyifeeProp( pSignal );
			}
			else
			{
				CFcHeroObject::SignalCheck( pSignal );
			}
			break;
		case SIGNAL_TYPE_THROW_WEAPON:
			ThrowWeapon( pSignal );
			break;
		case SIGNAL_TYPE_RETURN_WEAPON:
			ReturnWeapon( pSignal );
			break;
		case SIGNAL_TYPE_SHOW_TRAIL:
			MyifeeTrailCheck( pSignal );
			if( ( !m_bThrowMode ) && ( !m_pThrowTrailSignal ) )
			{
				TrailCheck( pSignal );
			}
			break;
		default:
			CFcHeroObject::SignalCheck( pSignal );
			break;
	}
}

void CMyifee::GetNextFrameCenterPosition( D3DXVECTOR3 *pPosition, int nCurFrame, ASSignalData *pSignal, int nThrowBending )
{
	D3DXMATRIX BoneMatrix;
	D3DXVECTOR3 HandPos, WeaponPos, Direction;
	float fThrowLength;

	nCurFrame += nThrowBending;
	if( nCurFrame > ( pSignal->m_ThrowWeaponLength - 1 ) )
	{
		nCurFrame = pSignal->m_ThrowWeaponLength - 1;
	}

	memcpy( &BoneMatrix, m_pASData->GetHitMatrix( pSignal->m_ThrowWeaponMatrixIndex + nCurFrame * 2 ), 
		sizeof( D3DXMATRIX ) );
	D3DXMatrixMultiply( &BoneMatrix, &BoneMatrix, m_Cross );
	memcpy( &HandPos, &BoneMatrix._41, sizeof( D3DXVECTOR3 ) );
	memcpy( &BoneMatrix, m_pASData->GetHitMatrix( pSignal->m_ThrowWeaponMatrixIndex + nCurFrame * 2 + 1 ), 
		sizeof( D3DXMATRIX ) );
	D3DXMatrixMultiply( &BoneMatrix, &BoneMatrix, m_Cross );
	memcpy( &WeaponPos, &BoneMatrix._41, sizeof( D3DXVECTOR3 ) );

	Direction = WeaponPos - HandPos;
	D3DXVec3Normalize( &Direction, &Direction );
	fThrowLength = m_fThrowLength + m_fMaxThrowLength * m_fThrowSpeed;
	if( fThrowLength < 0.0f )
	{
		fThrowLength = 0.0f;
	}
	if( fThrowLength > m_fMaxThrowLength )
	{
		fThrowLength = m_fMaxThrowLength;
	}
	m_NextWeaponPos = HandPos + Direction * fThrowLength;

	D3DXVec3Lerp( pPosition, &HandPos, &m_NextWeaponPos, pSignal->m_ThrowWeaponBendPos / 100.0f );
//	*pPosition = ( WeaponPos + HandPos ) / 2;
}

void CMyifee::ThrowWeapon( ASSignalData *pSignal )
{
	if( ( m_fFrame >= pSignal->m_nFrame ) && ( m_fFrame < pSignal->m_nFrame + pSignal->m_ThrowWeaponLength ) )
	{
		int nCurFrame;

		nCurFrame = ( int )( m_fFrame - pSignal->m_nFrame );
		if( nCurFrame == 0 )
		{
			m_fThrowSpeed = pSignal->m_ThrowWeaponSpeed / 100.0f;
			m_fThrowLength = 0.0f;
			m_fMaxThrowLength = ( float )pSignal->m_ThrowWeaponThrowLength;
			ThrowWeapon( true );
		}

		memcpy( &m_HandMat, m_pASData->GetHitMatrix( pSignal->m_ThrowWeaponMatrixIndex + nCurFrame * 2 ), 
			sizeof( D3DXMATRIX ) );
		D3DXMatrixMultiply( &m_HandMat, &m_HandMat, m_Cross );
		memcpy( &m_WeaponMat, m_pASData->GetHitMatrix( pSignal->m_ThrowWeaponMatrixIndex + nCurFrame * 2 + 1 ), 
			sizeof( D3DXMATRIX ) );
		D3DXMatrixMultiply( &m_WeaponMat, &m_WeaponMat, m_Cross );

		GetNextFrameCenterPosition( &m_CenterPos, nCurFrame, pSignal, pSignal->m_ThrowWeaponBending );

		D3DXVECTOR3 DummyPos;
		GetNextFrameCenterPosition( &DummyPos, nCurFrame, pSignal, 1 );
	}
	else
	{
		if( m_bThrowMode )
		{
			ThrowWeapon( false );
		}
	}
}

void CMyifee::ReturnWeapon( ASSignalData *pSignal )
{
	if( ( pSignal->m_nFrame >= m_fFrame ) && ( pSignal->m_nFrame < m_fFrame + GetFrameAdd() ) )
	{
		m_fThrowSpeed = -pSignal->m_pParam[ 0 ] / 100.0f;
	}
}

void CMyifee::ThrowWeapon( bool bThrow )
{
	m_bThrowMode = bThrow;
	if( m_bThrowMode )
	{
		g_BsKernel.SendMessage( m_nEngineIndex, BS_UNLINKOBJECT, m_WeaponList[ m_nCurrentWeaponIndex ].nObjectIndex );
		g_BsKernel.SetPreCalcAni( m_WeaponList[ m_nCurrentWeaponIndex ].nObjectIndex, true );
	}
	else
	{
		g_BsKernel.SendMessage( m_nEngineIndex, BS_LINKOBJECT, m_WeaponList[ m_nCurrentWeaponIndex ].nLinkBoneIndex, m_WeaponList[ m_nCurrentWeaponIndex ].nObjectIndex );
	}
}

void CMyifee::SendCatchMsg()
{
	if( m_CatchHandle )
	{
		int nCatchEngineIndex = m_CatchHandle->GetEngineIndex();
		m_CatchHandle->SetCatchedHero( true );
		DebugString("CMyifee::Render() Catch : LinkObject(%d), CatchedHandle(%d)\n", nCatchEngineIndex, m_CatchHandle);
		g_BsKernel.SendMessage( m_WeaponList[ m_nCurrentWeaponIndex ].nObjectIndex, BS_LINKOBJECT, 0, nCatchEngineIndex );
		g_BsKernel.SendMessage(  nCatchEngineIndex, BS_SET_RAGDOLL_CATCH_BONE, m_nCatchBoneIndex);
	}
}

void CMyifee::SendUnlinkCatchMsg( int nHandIndex /*= -1*/ )
{
	if( m_CatchHandle )
	{
		int nCatchEngineIndex = m_CatchHandle->GetEngineIndex();
		m_CatchHandle->SetCatchedHero( false );
		DebugString("CMyifee::Render() Uncatch : UnLinkObject(%d)\n", nCatchEngineIndex);
		g_BsKernel.SendMessage( m_WeaponList[ m_nCurrentWeaponIndex ].nObjectIndex, BS_UNLINKOBJECT, nCatchEngineIndex, ( DWORD )&m_UnlinkVelocity );
		m_CatchHandle->UnlinkRagdoll();
		m_CatchHandle.Identity();
	}
}

bool CMyifee::Render()
{
	if( ( m_bThrowMode ) && ( m_nCurrentWeaponIndex != -1 ) )
	{
        g_BsKernel.UpdateObject( m_WeaponList[ m_nCurrentWeaponIndex ].nObjectIndex, &m_WeaponMat );
	}

	return CFcHeroObject::Render();
}

bool CMyifee::Catch( GameObjHandle Handle )
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
	m_CatchHandle->ChangeAnimation( ANI_TYPE_DOWN, 0 );	// Ragdoll 상태일때 AI 못돌게 할려고..
	m_nCatchBoneIndex = g_BsKernel.SendMessage( m_CatchHandle->GetEngineIndex(), BS_GET_BONE_INDEX, ( DWORD )"Bip01 Spine1" );

	return true;
}

void CMyifee::AddThrowTrail( int nTrailFrame, ASSignalData *pSignal, int nTrailHandle/* = 0*/ )
{
	if( m_nCurrentWeaponIndex == -1 )
	{
		return;
	}
	if( !m_bTrailOn[ nTrailHandle ] )
	{
		float fLifeTime;
		D3DXVECTOR3 Color;

		fLifeTime = ( float )( pSignal->m_TrailLifeTime );
		if( fLifeTime == 0.0f )
		{
			fLifeTime = 15.0f;
		}
		Color.x = pSignal->m_TrailR / 255.0f;
		Color.y = pSignal->m_TrailG / 255.0f;
		Color.z = pSignal->m_TrailB / 255.0f;
		g_pFcFXManager->SendMessage( m_nTrailHandle[ nTrailHandle ], FX_SETLIFE, ( DWORD )&fLifeTime );
		g_pFcFXManager->SendMessage( m_nTrailHandle[ nTrailHandle ], FX_SETCOLOR, ( DWORD )&Color );
	}

	int nBookingCount;
	D3DXVECTOR3 *pStart, *pEnd;
	D3DXMATRIX HitMatrix, HitMatrix2;
	D3DXVECTOR3 TrailPos[ 4 ], AddPos;

	nBookingCount = g_BsKernel.SendMessage( m_WeaponList[ m_nCurrentWeaponIndex ].nObjectIndex, BS_GET_BOOKING_COUNT );
	BsAssert( nBookingCount >= 4 );
	if( nBookingCount )
	{
		pStart = ( D3DXVECTOR3 * )g_BsKernel.SendMessage( m_WeaponList[ m_nCurrentWeaponIndex ].nObjectIndex, BS_GET_BOOKING_POSITION, 3 );
		pEnd = ( D3DXVECTOR3 * )g_BsKernel.SendMessage( m_WeaponList[ m_nCurrentWeaponIndex ].nObjectIndex, BS_GET_BOOKING_POSITION, 1 );

		if( m_bThrowMode )
		{
			memcpy( &HitMatrix, m_WeaponMat, sizeof( D3DXMATRIX ) );
			memcpy( &HitMatrix2, m_WeaponMat, sizeof( D3DXMATRIX ) );
			memcpy( &HitMatrix2._41, &m_NextWeaponPos, sizeof( D3DXVECTOR3 ) );
		}
		else
		{
			memcpy( &HitMatrix, m_pASData->GetHitMatrix( pSignal->m_TrailMatrixIndex + nTrailFrame ), 
				sizeof( D3DXMATRIX ) );
			D3DXMatrixMultiply( &HitMatrix, &HitMatrix, m_Cross );
			if( nTrailFrame < pSignal->m_HitLength -1 )
			{
				memcpy( &HitMatrix2, m_pASData->GetHitMatrix( pSignal->m_TrailMatrixIndex + nTrailFrame + 1 ), sizeof( D3DXMATRIX ));
			}
			else
			{
				memcpy( &HitMatrix2, m_pASData->GetHitMatrix( pSignal->m_TrailMatrixIndex + nTrailFrame ), sizeof( D3DXMATRIX ));
			}
			D3DXMatrixMultiply( &HitMatrix2, &HitMatrix2, m_Cross );
		}
//		D3DXMatrixMultiply( &HitMatrix, &HitMatrix, m_Cross );
		D3DXVec3TransformCoord( TrailPos, pStart, &HitMatrix );
		D3DXVec3TransformCoord( TrailPos + 1, pEnd, &HitMatrix );
		AddPos = TrailPos[ 1 ] - TrailPos[ 0 ];
		D3DXVec3Normalize( &AddPos, &AddPos );
		TrailPos[ 1 ] += AddPos * ( float )pSignal->m_TrailAddLength;

//		D3DXMatrixMultiply( &HitMatrix2, &HitMatrix2, m_Cross );
		D3DXVec3TransformCoord( TrailPos +2, pStart, &HitMatrix2 );
		D3DXVec3TransformCoord( TrailPos + 3, pEnd, &HitMatrix2 );
		AddPos = TrailPos[ 3 ] - TrailPos[ 2 ];
		D3DXVec3Normalize( &AddPos, &AddPos );
		TrailPos[ 3 ] += AddPos * ( float )pSignal->m_TrailAddLength;

		int nTexIdx = (int)(pSignal->m_TrailKind);
		g_pFcFXManager->SendMessage( m_nTrailHandle[ nTrailHandle ], FX_SET_DIFFTEXTURE, ( DWORD )GetWeaponTrailTextureID( nTexIdx ) );
		g_pFcFXManager->SendMessage( m_nTrailHandle[ nTrailHandle ], FX_SET_OFFSETTEXTURE, ( DWORD )GetWeaponTrailOffsetTextureID() );
		g_pFcFXManager->SendMessage( m_nTrailHandle[ nTrailHandle ], FX_ADD_TRAIL, ( DWORD )TrailPos );
	}
	m_bTrailOn[ nTrailHandle ] = true;
}

void CMyifee::MyifeeTrailCheck( ASSignalData *pSignal )
{
	if( ( m_fFrame >= pSignal->m_nFrame ) && ( m_fFrame < pSignal->m_nFrame + pSignal->m_HitLength ) )
	{
		if( ( m_bThrowMode ) || ( m_pThrowTrailSignal ) )
		{
			m_pThrowTrailSignal = pSignal;
		}
		else
		{
			AddTrail( ( int )( m_fFrame - pSignal->m_nFrame ), pSignal, 1, 1 );
		}
		return;
	}
	m_pThrowTrailSignal = NULL;
	if( m_bTrailOn[ 1 ] )
	{
		g_pFcFXManager->SendMessage( m_nTrailHandle[ 1 ], FX_STOP_OBJECT );
	}
}

void CMyifee::HitCheckTempMyifee( ASSignalData *pSignal )
{
	if( ( pSignal->m_nFrame >= m_fFrame ) && ( pSignal->m_nFrame < m_fFrame + GetFrameAdd() ) )
	{
		int i, nSize;
//		float fLength, fDot, fCos, fSin;
		float fLength;
		std::vector<GameObjHandle> Result;
		D3DXVECTOR3 Direction, Orient, WeaponPosition;

		memcpy( &WeaponPosition, &m_WeaponMat._41, sizeof( D3DXVECTOR3 ) );
		CFcWorld::GetInstance().GetEnemyObjectListInRange( m_nTeam, &WeaponPosition, ( float )pSignal->m_Hit2MinDist, Result );
		nSize = Result.size();
		for( i = 0; i < nSize; i++)
		{
			if( !Result[ i ]->IsHittable( m_Handle ) )
			{
				continue;
			}
			bool bCheck = true;
			for( DWORD j=0; j<m_VecLinkObjHandle.size(); j++ ) {
				if( Result[i] == m_VecLinkObjHandle[j] ) {
					bCheck = false;
					break;
				}
			}
			if( bCheck == false ) continue;

			Direction = Result[ i ]->GetPos() - WeaponPosition;
			Direction.y = 0.0f;
			fLength = D3DXVec3Length( &Direction );
			if( fLength > pSignal->m_Hit2MinDist )
			{
				continue;
			}

			D3DXVECTOR3 CheckStart, CheckEnd;

			CheckStart = Result[ i ]->GetPos();
			CheckEnd = CheckStart;
			CheckStart.y = WeaponPosition.y + pSignal->m_Hit2MinHeight;
			CheckEnd.y = WeaponPosition.y + pSignal->m_Hit2MaxHeight;
			if( CollisionCheck( Result[ i ], &CheckStart, &CheckEnd, 0.0f ) )
			{
				HIT_PARAM HitParam;
				HitParam.ObjectHandle = m_Handle;
				HitParam.pHitSignal = pSignal;
				HitParam.nHitRemainFrame = pSignal->m_Hit2Length;
				HitParam.nAttackPower = GetAttackPower();
				HitParam.Position = WeaponPosition;
				HitParam.nGroundResist = pSignal->m_Hit2GroundResist;
				HitParam.fVelocityY = ( float )pSignal->m_HitPushY;
				HitParam.fVelocityZ = ( float )pSignal->m_HitPushZ;
				SendHitSignal( &HitParam, Result[ i ] );
			}
		}
	}
}

void CMyifee::HitCheckTempMyifeeProp( ASSignalData *pSignal )
{
	if( ( pSignal->m_nFrame >= m_fFrame ) && ( pSignal->m_nFrame < m_fFrame + GetFrameAdd() ) )
	{
		bool bCollision;
		int i, nSize;
		std::vector< CFcProp * > Result;
		Box3 BoundingBox;
		Sphere WeaponSphere;

		WeaponSphere.C = BSVECTOR( m_WeaponMat._41, m_WeaponMat._42, m_WeaponMat._43 );
		WeaponSphere.R = 50.0f;

		CFcWorld::GetInstance().GetActiveProps( *( D3DXVECTOR3 * )&m_WeaponMat._41, pSignal->m_Hit2MaxDist * 3.0f, Result );
		nSize = Result.size();
		for( i = 0; i < nSize; i++)
		{
			if( Result[ i ]->IsBreak() )
			{
				continue;
			}
			Result[ i ]->GetBox3( BoundingBox );
			bCollision = TestIntersection( WeaponSphere, BoundingBox );
			if( bCollision )
			{
				if( AddDynamicForce( Result[ i ], pSignal->m_Hit2AttackPoint, PROP_BREAK_TYPE_NORMAL ) )
				{
					if( Result[ i ]->GetCrushParticleIndex() > 0 )
					{
						CCrossVector Cross;

						Cross = m_Cross;
						Cross.m_PosVector.x = m_WeaponMat._41;
						Cross.m_PosVector.y = m_WeaponMat._42;
						Cross.m_PosVector.z = m_WeaponMat._43;
						g_BsKernel.CreateParticleObject( Result[ i ]->GetCrushParticleIndex(), false, false, Cross );
					}
				}
			}
		}
	}
}


void CMyifee::CancelThrow()
{
	int i;

	ThrowWeapon( false );
	m_pThrowTrailSignal = NULL;
	for( i = 0; i < 2; i++ )
	{
		if( m_bTrailOn[ i ] )
		{
			g_pFcFXManager->SendMessage( m_nTrailHandle[ i ], FX_STOP_OBJECT );
		}
	}
}

void CMyifee::CmdHit( HIT_PARAM *pHitParam, D3DXVECTOR3 *pHitDir /*= NULL*/ )
{
	if( ( m_bThrowMode ) || ( m_pThrowTrailSignal ) )
	{
		CancelThrow();
	}
	CFcHeroObject::CmdHit( pHitParam );
	InitWeapon();
}

void CMyifee::LoadWeaponTrailTexture()
{
	int nTextureId;
	g_BsKernel.chdir("Fx");
	
	nTextureId = g_BsKernel.LoadTexture( "Myifee_SwordTrail01.dds" );
	if( nTextureId != -1 ) 
		m_WeaponTrailTexIDs.push_back(nTextureId);
	nTextureId = -1;

	nTextureId = g_BsKernel.LoadTexture( "Myifee_SwordTrail02.dds" );
	if( nTextureId != -1 ) 
		m_WeaponTrailTexIDs.push_back(nTextureId);
	nTextureId = -1;

	nTextureId = g_BsKernel.LoadTexture( "Myifee_SwordTrail03.dds" );
	if( nTextureId != -1 ) 
		m_WeaponTrailTexIDs.push_back(nTextureId);

	m_nWeaponTrailOffsetTexID = g_BsKernel.LoadTexture( "Myifee_SwordTrail_Normals.dds" );

	g_BsKernel.chdir("..");
}

void CMyifee::InitOrbAttack2DFx()
{
	m_nMotionBlurIndex = g_pFcFXManager->Create( FX_TYPE_INPHYMOTIONBLUR );
	g_pFcFXManager->SendMessage( m_nMotionBlurIndex, FX_INIT_OBJECT );
}

bool CMyifee::ChangeWeapon( int nSkinIndex, int nAttackPower )
{
	BsAssert( nSkinIndex >= 0 );

	int i;

	if( m_CatchHandle )
	{
		return false;
	}

	for( i = 0; i < 2; i++ )
	{
		m_WeaponList[ i ].nEngineIndex = nSkinIndex;
		int nObjectIndex = g_BsKernel.CreateStaticObjectFromSkin( nSkinIndex );
		g_BsKernel.SendMessage( m_nEngineIndex, BS_UNLINKOBJECT_BONE, m_WeaponList[ i ].nLinkBoneIndex );
		g_BsKernel.DeleteObject( m_WeaponList[ i ].nObjectIndex );
		m_WeaponList[ i ].nObjectIndex = -1;

		g_BsKernel.SendMessage( m_nEngineIndex, BS_LINKOBJECT, 
			( DWORD )m_WeaponList[ i ].nLinkBoneIndex, nObjectIndex );
		g_BsKernel.ShowObject( nObjectIndex, true );
		m_WeaponList[ i ].nObjectIndex = nObjectIndex;
	}
	g_BsKernel.ShowObject( m_WeaponList[ 1 ].nObjectIndex, false );

	return true;
}

void CMyifee::InitWeapon()
{
	if(m_WeaponList[0].nObjectIndex != -1)
        g_BsKernel.ShowObject( m_WeaponList[ 0 ].nObjectIndex, true );
	if(m_WeaponList[1].nObjectIndex != -1)
        g_BsKernel.ShowObject( m_WeaponList[ 1 ].nObjectIndex, false );
	m_nCurrentWeaponIndex = 0;
}

void CMyifee::StartOrbSpark()
{
	if( ( m_bThrowMode ) || ( m_pThrowTrailSignal ) )
	{
		CancelThrow();
	}
	InitWeapon();

	CFcHeroObject::StartOrbSpark();
}

void CMyifee::StopOrbSpark()
{
	if( ( m_bThrowMode ) || ( m_pThrowTrailSignal ) )
	{
		CancelThrow();
	}
	InitWeapon();

	CFcHeroObject::StopOrbSpark();
}
