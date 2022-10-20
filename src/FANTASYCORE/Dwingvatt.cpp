#include "StdAfx.h"
#include "InputPad.h"
#include "Dwingvatt.h"
#include "AsData.h"
#include "FcWorld.h"
#include "FcFxManager.h"
#include "BsImageProcess.h"
#include "FcUtil.h"

#include "FcGlobal.h"
#include ".\\data\\AnimType.h"
#include ".\\data\\SignalType.h"

CDwingvatt::CDwingvatt( CCrossVector *pCross )
: CFcHeroObject( pCross )
{
	SetHeroClassID( Class_ID_Hero_Dwingvatt );

	for ( int i = 0; i < 5; ++i )
	{
		m_nIllusionEidx[i] = -1;
	}

	m_nIllusion = 0;
	m_nIdxcrossIllusion = 0;
}

CDwingvatt::~CDwingvatt(void)
{
	for ( int i = 0; i < 5; ++i )
	{
		if ( m_nIllusionEidx[i] != -1 ) {
			g_BsKernel.DeleteObject( m_nIllusionEidx[i] );
			m_nIllusionEidx[i] = -1;
		}
	}
}

void CDwingvatt::Initialize( INITGAMEOBJECTDATA* pData, int nForce, int nTeam, TroopObjHandle hTroop, float fScaleMin, float fScaleMax )
{
	m_nTrailHandle[ 1 ] = g_pFcFXManager->Create( FX_TYPE_WEAPONTRAIL );
	g_pFcFXManager->SendMessage( m_nTrailHandle[ 1 ], FX_INIT_OBJECT );

	CFcHeroObject::Initialize( pData, nForce, nTeam, hTroop, fScaleMin, fScaleMax );

	if( m_WeaponList[ 1 ].nObjectIndex != -1 )
	{
		g_BsKernel.ShowObject( m_WeaponList[ 1 ].nObjectIndex, true );
	}

	int nRand = Random( m_pUnitSOX->cSkinVariationNum );
	float fAlpha = 1.f;
	int nSub = 0;
	for(int ii = 0; ii < 5; ++ii)
	{
		m_nIllusionEidx[ii] = g_BsKernel.CreateAniObjectFromSkin( pData->nSkinIndex[nRand], pData->nAniIndex );
		nSub = g_BsKernel.SendMessage(m_nIllusionEidx[ii], BS_GET_SUBMESH_COUNT);
		fAlpha = float(5-ii)/5.f;
		for(int ij = 0; ij < nSub; ++ij)
		{
			g_BsKernel.SendMessage(m_nIllusionEidx[ii], BS_SET_SUBMESH_ALPHABLENDENABLE, ij, true);			
			g_BsKernel.SendMessage(m_nIllusionEidx[ii], BS_SET_SUBMESH_ALPHA, ij, (DWORD)&fAlpha);
		}
	}
	m_nIllusion = 0;
	m_nIdxcrossIllusion = 0;
}



void CDwingvatt::FindTramplableObject( D3DXVECTOR3 *pPosition, GameObjHandle &Handle, float fCheckRadius, float fSearchMin, float fSearchMax )
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

void CDwingvatt::TrampleCheck()
{
	GameObjHandle Handle;
	D3DXVECTOR3 Dist;

	if( CInputPad::GetInstance().GetKeyPressTick( g_FCGameData.nEnablePadID, PAD_INPUT_LSTICK ) )
	{
		if( ( m_nCurAniTypeIndex >= 0 ) && ( m_nCurAniTypeIndex <= 2 ) )
		{
			if( m_vVelocity.y < 0.0f )
			{
				FindTramplableObject( &m_Cross.m_PosVector, Handle, 150.0f, -50.0f, 0.0f );
				if( Handle )
				{
					Dist = Handle->GetPos() - m_Cross.m_PosVector;
					Dist.y = Handle->GetPos().y + Handle->GetUnitHeight() - m_Cross.m_PosVector.y;
					m_MoveVector += Dist;
					ChangeAnimation( ANI_TYPE_JUMP_UP, 4, 0, true );
					m_fJumpSpeed = 0.0f;
					m_fGravity = 0.0f;
					m_vVelocity.y = 0.0f;
					m_FindHandle = Handle;
				}
			}
		}
		else if( m_nCurAniTypeIndex == 7 )
		{
			if( m_fFrame >= m_nCurAniLength - 1 )
			{
				FindTramplableObject( &m_Cross.m_PosVector, Handle, 150.0f, -50.0f, 0.0f );
				if( Handle )
				{
					Dist = Handle->GetPos() - m_Cross.m_PosVector;
					Dist.y = Handle->GetPos().y + Handle->GetUnitHeight() - m_Cross.m_PosVector.y;
					m_MoveVector += Dist;
					ChangeAnimation( ANI_TYPE_JUMP_UP, 4, 0, true );
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
					ChangeAnimation( ANI_TYPE_JUMP_UP, 7, 0, true );
					m_fGravity = 0.0f;
					m_vVelocity.y = 0.0f;
				}
				else
				{
					ChangeAnimation( ANI_TYPE_JUMP_UP, 6, 0, true );
				}
			}
		}
		if( Handle )
		{
			Handle->ChangeAnimation( ANI_TYPE_HIT, 0 );

		}
	}
	else if( m_nCurAniTypeIndex == 4 )
	{
		if( m_fFrame >= m_nCurAniLength - 1 )
		{
			ChangeAnimationByIndex( 93 );
		}
	}
	if( m_nCurAniTypeIndex == 7 )
	{
		MoveZ( m_CustomMoveStep.z );
		MoveY( m_CustomMoveStep.y );
	}
}

void CDwingvatt::Process()
{
	if( m_nCurAniType == ANI_TYPE_JUMP_UP )
	{
		TrampleCheck();
	}

	CFcHeroObject::Process();
}

void CDwingvatt::DwingvattTrailCheck( ASSignalData *pSignal )
{
	if( ( m_fFrame >= pSignal->m_nFrame ) && ( m_fFrame < pSignal->m_nFrame + pSignal->m_HitLength ) )
	{
		m_nCurrentWeaponIndex = pSignal->m_TrailWeaponGroup;
		BsAssert( m_nCurrentWeaponIndex <= 1 );
		AddTrail( ( int )( m_fFrame - pSignal->m_nFrame ), pSignal, 0, m_nCurrentWeaponIndex );
		return;
	}
	if( m_bTrailOn[ 0 ] )
	{
		g_pFcFXManager->SendMessage( m_nTrailHandle[ 0 ], FX_STOP_OBJECT );
	}
	m_bTrailOn[ 0 ] = false;
}

void CDwingvatt::SignalCheck( ASSignalData *pSignal )
{
	switch( pSignal->m_nID )
	{
		case SIGNAL_TYPE_SHOW_TRAIL:
			DwingvattTrailCheck( pSignal );
			break;
		case SIGNAL_TYPE_DASH_TRAIL:
			DashTrailCheck( pSignal );
			break;

		default:
			CFcHeroObject::SignalCheck( pSignal );
			break;
	}
}


void CDwingvatt::InitOrbAttack2DFx()
{
	m_nMotionBlurIndex = g_pFcFXManager->Create(FX_TYPE_2DFSEFFECT);
	g_pFcFXManager->SendMessage(m_nMotionBlurIndex, FX_INIT_OBJECT, 1);
	g_pFcFXManager->SendMessage(m_nMotionBlurIndex, FX_SET_FSDEVIDE, 1);
	float fAlpha = 0.7f;
	g_pFcFXManager->SendMessage(m_nMotionBlurIndex, FX_SET_FSALPHA, (DWORD)&fAlpha);
	g_pFcFXManager->SendMessage(m_nMotionBlurIndex, FX_SET_TEXTURE, g_BsKernel.GetImageProcess()->GetBackBufferTexture() );
//	g_pFcFXManager->SendMessage(m_nMotionBlurIndex, FX_PLAY_OBJECT);
}

int CDwingvatt::FindExactAni( int nAniIndex )
{
	int nAniType = m_pUnitInfoData->GetAniType( nAniIndex );
	int nAniTypeIndex = m_pUnitInfoData->GetAniTypeIndex( nAniIndex );
	int nAniAttr = m_pUnitInfoData->GetAniAttr( nAniIndex );

	switch( m_nAniIndex )
	{
	case 70:
		if( nAniType == ANI_TYPE_RUN )
		{
			return 72;
		}
		else return 70;
		break;
	case 72:
		if( nAniType == ANI_TYPE_STAND )
		{
			return 70;
		}
		else if( nAniType == ANI_TYPE_RUN  || nAniType == ANI_TYPE_WALK )
		{
			return 72;
		}
		break;
	}
	return nAniIndex;

}

void CDwingvatt::LoadWeaponTrailTexture()
{
	int nTextureId;
	g_BsKernel.chdir("Fx");

	nTextureId = g_BsKernel.LoadTexture( "Dwingvatt_SwordTrail01.dds" );
	if( nTextureId != -1 ) 
		m_WeaponTrailTexIDs.push_back(nTextureId);
	nTextureId = -1;

	nTextureId = g_BsKernel.LoadTexture( "Dwingvatt_SwordTrail02.dds" );
	if( nTextureId != -1 ) 
		m_WeaponTrailTexIDs.push_back(nTextureId);
	nTextureId = -1;

	nTextureId = g_BsKernel.LoadTexture( "Dwingvatt_SwordTrail03.dds" );
	if( nTextureId != -1 ) 
		m_WeaponTrailTexIDs.push_back(nTextureId);
	nTextureId = -1;

	m_nWeaponTrailOffsetTexID = g_BsKernel.LoadTexture( "Dwingvatt_SwordTrail_Normals.dds" );

	g_BsKernel.chdir("..");
}


void CDwingvatt::DashTrailCheck( ASSignalData *pSignal )
{
	if( ( m_fFrame >= pSignal->m_nFrame ) && ( m_fFrame < pSignal->m_nFrame + pSignal->m_DashTrailLength ) )
	{
		for(int ii = 0; ii < m_nIllusion; ++ii)
		{
			float fFrame = ( float )( int )( m_fFrame - 1.f- float(ii));
			if(fFrame < 0.f)
				fFrame = 0.f;
			g_BsKernel.SetCurrentAni( m_nIllusionEidx[ii], m_nAniIndex, fFrame );
			
			D3DXMATRIX RenderMat;
			D3DXMatrixMultiply( &RenderMat, &m_ScaleMat, m_crossIllusion[ ((m_nIdxcrossIllusion-ii+4)%5) ] );
			CBsKernel::GetInstance().UpdateObject( m_nIllusionEidx[ii], &RenderMat );
		}

		if(m_nIllusion < 5)
			m_nIllusion++;

		m_crossIllusion[m_nIdxcrossIllusion] = m_Cross;

		m_nIdxcrossIllusion++;
		if(m_nIdxcrossIllusion >= 5)
			m_nIdxcrossIllusion = 0;


		return;
	}
	if( m_nIllusion )
	{
		m_nIllusion = 0;
		m_nIdxcrossIllusion = 0;
	}
}

bool CDwingvatt::ChangeWeapon( int nSkinIndex, int nAttackPower )
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

	return true;
}
