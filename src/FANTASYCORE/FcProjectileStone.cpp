#include "StdAfx.h"
#include "FcProjectile.h"
#include "FcProjectileStone.h"
#include "FcFXManager.h"
#include "FcUnitObject.h"
#include "FcTroopObject.h"
#include "FcGlobal.h"
#include "ASData.h"
#include "DebugUtil.h"
#include "Data/SignalType.h"
#include "fcWorld.h"
#include "FcProp.h"
#include "Box3.h"
#include "Ray3.h"
#include "IntLin3Box3.h"
#include "Data/FXList.h"
#include "FcSoundManager.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG
#define SOUND_MAXIMUM_DISTANCE	10000.f

ASSignalData *CFcProjectileStone::s_pHitSignal = NULL;

CFcProjectileStone::CFcProjectileStone( CCrossVector *pCross )
: CFcProjectile( pCross )
{
	m_bCheckGround = false;
	m_nDestroyTick = -1;
	m_nFXIndex = -1;
	m_nDestroyFXIndex = -1;

	if( s_pHitSignal == NULL ) {
		s_pHitSignal = new ASSignalData;
		s_pHitSignal->m_nID = SIGNAL_TYPE_HIT_TEMP;
		s_pHitSignal->m_NumParam = HIT2_SIGNAL_LENGTH;
		s_pHitSignal->m_pParam = new int[s_pHitSignal->m_NumParam];

		memset( s_pHitSignal->m_pParam, 0, sizeof(int) * s_pHitSignal->m_NumParam );

		/*
		s_pHitSignal->m_pParam[2] = ANI_TYPE_DOWN;
		s_pHitSignal->m_pParam[3] = 0;
		s_pHitSignal->m_pParam[6] = -1;
		*/
		s_pHitSignal->m_Hit2AniType = ANI_TYPE_DOWN;
		s_pHitSignal->m_Hit2AniIndex = 0;
		s_pHitSignal->m_Hit2PushY = 20;
		s_pHitSignal->m_Hit2PushZ = -12;


	}
}

CFcProjectileStone::~CFcProjectileStone()
{
	if( m_nFXIndex != -1 ) {
		g_BsKernel.DeleteObject( m_nFXIndex );
		m_nFXIndex = -1;
	}
	if( m_nDestroyFXIndex != -1 ) {
		g_BsKernel.DeleteObject( m_nDestroyFXIndex );
		m_nDestroyFXIndex = -1;
	}
}

int CFcProjectileStone::Initialize( int nSkinIndex, PROJECTILE_PARAM *pParam )
{
	int nResult = CFcProjectile::Initialize( nSkinIndex, pParam );

	m_bCheckGround = true;
	if( pParam->hParent ) {
		TroopObjHandle hTroop = pParam->hParent->GetTroop()->GetTargetTroop();
		if( hTroop && hTroop->GetType() == TROOPTYPE_WALL_DUMMY ) m_bCheckGround = false;
	}
	m_Velocity = GetVelocity( m_Cross.m_PosVector, CFcProjectile::s_gravity, sin( pParam->fAngle / 180.f * D3DX_PI ), m_Param.vTarget );
	m_Accel = s_gravity;		// 화살은 중력만 있음

	m_PrevPos = m_Cross.m_PosVector;
	m_nParentTeam = m_Param.hParent->GetTeam();

	m_nFXIndex = g_BsKernel.CreateFXObject( FX_CAT_BOOM );
	if( m_nFXIndex != -1 ) {
		g_BsKernel.SendMessage( m_nFXIndex, BS_ENABLE_OBJECT_CULL, FALSE );
		g_BsKernel.SetFXObjectState( m_nFXIndex, CBsFXObject::PLAY, -1 );
	}

	return nResult;
}

void CFcProjectileStone::Process()
{
	if( m_nDestroyTick != -1 ) {
		--m_nDestroyTick;
		if( m_nDestroyTick == 0 ) Delete();

		return;
	}
	m_Velocity += m_Accel;

	m_PrevPos = m_Cross.m_PosVector;
	D3DXVec3Normalize( &(m_Cross.m_ZVector), &m_Velocity );
	D3DXVec3Cross( &(m_Cross.m_YVector), &(m_Cross.m_ZVector), &(m_Cross.m_XVector) );
	D3DXVec3Normalize( &(m_Cross.m_YVector), &(m_Cross.m_YVector) );
	D3DXVECTOR3 newPos = m_Cross.m_PosVector + m_Velocity;

	if( m_bCheckGround == false ) {
		if( CheckHitTarget() == true ) {
			m_nDestroyTick = 40;
			m_Cross.m_PosVector = newPos;
			D3DXVECTOR3 SoundPos( m_Cross.m_PosVector.x, 0.f , m_Cross.m_PosVector.z );
			if( D3DXVec3Length(&(g_pSoundManager->GetListenerPos() - SoundPos)) < SOUND_MAXIMUM_DISTANCE ) {			
				g_pSoundManager->Play3DSound(NULL,SB_COMMON,"OB_WALL_HIT", &SoundPos );
			}


			// 여기서 파티클 튀겨준다!!
			m_Cross.m_YVector = D3DXVECTOR3( 0.f, 1.f, 0.f );
			m_Cross.UpdateVectorsY();
			m_Cross.m_ZVector = m_WallVector;
			m_Cross.UpdateVectors();

			m_DestroyCross = m_Cross;
			m_nDestroyFXIndex = g_BsKernel.CreateFXObject( FX_STONE_ATTACK );
			if( m_nDestroyFXIndex != -1 ) {
				g_BsKernel.SendMessage( m_nDestroyFXIndex, BS_ENABLE_OBJECT_CULL, FALSE );
				g_BsKernel.SetFXObjectState( m_nDestroyFXIndex, CBsFXObject::PLAY );
			}

			m_Cross.m_PosVector.y = -10000.f;			
			return;
		}
	}
	if( CheckHitGround() )
	{
		m_nDestroyTick = 100;

		m_Cross.m_PosVector = newPos;
//		m_Cross.m_PosVector.y = g_BsKernel.GetLandHeight( m_Cross.m_PosVector.x, m_Cross.m_PosVector.z ) - 10.f;  
		m_Cross.m_PosVector.y = GetLandHeight() - 10.f;

		if( CFcBaseObject::IsValid( m_Param.hParent ) ) {
			s_pHitSignal->m_pParam[1] = m_Param.pSignal->m_pParam[4];

			HIT_PARAM HitParam = MakeHitParam( s_pHitSignal );

			std::vector< GameObjHandle > VecList;
			g_FcWorld.GetEnemyObjectListInRange( m_nParentTeam, &m_Cross.m_PosVector, 500.f, VecList );
			for( DWORD i=0; i<VecList.size(); i++ ) {
				if( VecList[i]->IsHittable( HitParam.ObjectHandle ) )
					VecList[i]->CmdHit( &HitParam );
			}
		}
		
		m_nDestroyTick = 100;
		m_Cross.m_PosVector = newPos;

	}
	else  {
		m_Cross.RotateYaw( 10 );
		m_Cross.RotatePitch( 20 );
		m_Cross.RotateRoll( 25 );
		m_Cross.m_PosVector = newPos;
	}
}

bool CFcProjectileStone::CheckHitTarget()
{
	if( !CFcBaseObject::IsValid( m_Param.hParent ) ) return false;

	TroopObjHandle hTroop = m_Param.hParent->GetTroop()->GetTargetTroop();
	if( !hTroop || hTroop->GetType() != TROOPTYPE_WALL_DUMMY ) return false;
	if( hTroop->GetState() == TROOPSTATE_ELIMINATED ) {
		m_bCheckGround = true;
		return false;
	}

	CFcWallDummyTroop *pDummyTroop = (CFcWallDummyTroop *)hTroop.GetPointer();
	CFcProp *pProp = pDummyTroop->GetParentProp();

	Box3 Box;
	Segment3 Seg3;
	pProp->GetBox3( Box );

	Seg3.P.x = m_Cross.m_PosVector.x;
	Seg3.P.y = m_Cross.m_PosVector.y;
	Seg3.P.z = m_Cross.m_PosVector.z;

	Seg3.D.x = m_Cross.m_PosVector.x - m_PrevPos.x;
	Seg3.D.y = m_Cross.m_PosVector.y - m_PrevPos.y;
	Seg3.D.z = m_Cross.m_PosVector.z - m_PrevPos.z;
	if( TestIntersection( Seg3, Box ) == true ) {
		pDummyTroop->AddHP( -m_Param.hParent->GetAttackPower() );
		memcpy( &m_WallVector, &Box.A[2], sizeof( D3DXVECTOR3 ) );
		
//		memcpy( &m_WallVector, &Seg3.D, sizeof( D3DXVECTOR3 ) );
		return true;
	}
	return false;
}
bool CFcProjectileStone::Render()
{
	if( m_nFXIndex != -1 ) {
		g_BsKernel.UpdateObject( m_nFXIndex, m_Cross );
	}
	if( m_nDestroyFXIndex != -1 ) {
		g_BsKernel.UpdateObject( m_nDestroyFXIndex, m_DestroyCross );
	}
	if( m_nDestroyTick != -1 ) return true;
	return CFcProjectile::Render();
}