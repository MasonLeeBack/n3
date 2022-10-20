#include "StdAfx.h"
#include "BsKernel.h"
#include "BsSinTable.h"
#include "FcProjectile.h"
#include "FcGameObject.h"
#include "FcUtil.h"
#include "FcBaseObject.h"
#include "ASData.h"

#include "FcProjectileArrow.h"
#include "FcProjectileStone.h"
#include "FcProjectilePhysicsThrow.h"
#include "FcProjectileThrowStone.h"
#include "FcProjectileDirectArrow.h"
#include "FcProjectileWaterBomb.h"
#include "FcProjectileMagicFx.h"
#include "FcProjectileTimeBomb.h"
#include "FcProjectileRoots.h"
#include "FcProjectileBlackhall.h"
#include "FcWorld.h"

typedef CSmartPtr<CFcGameObject> GameObjHandle;


#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG


D3DXVECTOR3 CFcProjectile::s_gravity = D3DXVECTOR3( 0.f, -0.8f, 0.f );	// cm^2 / frame
std::vector< CFcProjectile* > CFcProjectile::s_Objs;


CFcProjectile::CFcProjectile( CCrossVector *pCross )
	: CFcBaseObject( pCross )
{
	
}

CFcProjectile::~CFcProjectile(void)
{
	int nCnt = s_Objs.size();
	for( int i=0; i<nCnt; i++ )
	{
		if( s_Objs[i] == this )
		{
			s_Objs.erase( s_Objs.begin() + i );
			break;
		}
	}
	m_nExplotionFxIndex = -1;
}

CSmartPtr< CFcProjectile > CFcProjectile::CreateObject( PROJECTILE_TYPE Type, CCrossVector *pCross )
{
	CSmartPtr< CFcProjectile > Obj;

	switch( Type ) {
		case ARROW:			Obj = CFcBaseObject::CreateObject< CFcProjectileArrow >( pCross );			break;
		case STONE:			Obj =  CFcBaseObject::CreateObject< CFcProjectileStone >( pCross );			break;
		case PHYSICS_THROW: Obj = CFcBaseObject::CreateObject< CFcProjectilePhysicsThrow >( pCross );	break;
		case THROW_STONE:	Obj = CFcBaseObject::CreateObject< CFcProjectileThrowStone >( pCross );		break;
		case DIRECT_ARROW:	Obj = CFcBaseObject::CreateObject< CFcProjectileDirectArrow >( pCross );	break;
		case WATERBOMB:		Obj = CFcBaseObject::CreateObject< CFcProjectileWaterBomb >( pCross );	break;
		case MAGIC_FX:		Obj = CFcBaseObject::CreateObject< CFcProjectileMagicFx >( pCross );	break;
		case TIMEBOMB:		Obj = CFcBaseObject::CreateObject< CFcProjectileTimeBomb >( pCross );	break;
		case ROOTS:			Obj = CFcBaseObject::CreateObject< CFcProjectileRoots >( pCross );	break;
		case BLACKHALL:		Obj = CFcBaseObject::CreateObject< CFcProjectileBlackHall >( pCross );	break;

	}
	if( Obj )
	{
		s_Objs.push_back( Obj );
	}
	return Obj;
}

#define SAFE_DELETE_SIGNAL_INFO( a ) { if( a ) { delete a; a = NULL; } }

void CFcProjectile::ReleaseSignal()
{
	SAFE_DELETE_SIGNAL_INFO( CFcProjectileArrow::s_pHitSignal );
	SAFE_DELETE_SIGNAL_INFO( CFcProjectileStone::s_pHitSignal );
	SAFE_DELETE_SIGNAL_INFO( CFcProjectilePhysicsThrow::s_pHitSignal );
	SAFE_DELETE_SIGNAL_INFO( CFcProjectileThrowStone::s_pHitSignal );
	SAFE_DELETE_SIGNAL_INFO( CFcProjectileDirectArrow::s_pHitSignal );
	SAFE_DELETE_SIGNAL_INFO( CFcProjectileWaterBomb::s_pHitSignal );
	SAFE_DELETE_SIGNAL_INFO( CFcProjectileMagicFx::s_pHitSignal );
	SAFE_DELETE_SIGNAL_INFO( CFcProjectileTimeBomb::s_pHitSignal );
	SAFE_DELETE_SIGNAL_INFO( CFcProjectileRoots::s_pHitSignal );
	SAFE_DELETE_SIGNAL_INFO( CFcProjectileBlackHall::s_pHitSignal );
}

CFcProjectile* CFcProjectile::GetOneMovingProjectile( TroopObjHandle hTargetTroop )
{
	int nCnt = s_Objs.size();
	for( int i=0; i<nCnt; i++ )
	{
		CFcProjectile* Obj = s_Objs[i];
		if( Obj->GetTargetTroop() == hTargetTroop )
			return Obj;
	}
	return NULL;
}


int CFcProjectile::Initialize( int nSkinIndex, PROJECTILE_PARAM *pParam )
{
	int nResult = CFcBaseObject::Initialize( nSkinIndex );
	m_Param = *pParam;

	return nResult;
}

void CFcProjectile::Process()
{
	CFcBaseObject::Process();
}

TroopObjHandle CFcProjectile::GetTargetTroop()
{
	TroopObjHandle hTroop;
	if( m_Param.hTarget )
	{
		hTroop = m_Param.hTarget->GetTroop();
	}
	return hTroop;
}


D3DXVECTOR3	CFcProjectile::GetVelocity( const D3DXVECTOR3& pos, const D3DXVECTOR3& accel, float angle, D3DXVECTOR3 vTarget, bool bAverageDir )
{
	D3DXVECTOR2 troopPosV2( m_Cross.m_PosVector.x, m_Cross.m_PosVector.z );
	D3DXVECTOR3 diff = vTarget - pos;

	// horizontal dist
	float dist = sqrtf(diff.x * diff.x + diff.z * diff.z);

	float sine	  = sinf( angle );
	float cosine  = cosf( angle );
	float tangent = sine / cosine;

	float speed = (dist/cosine) * sqrtf( fabsf( accel.y / (2.f * (diff.y - (dist * tangent))) ));

	// 안맞는경우 있다..
	float fTemp = speed;
	if( speed < 40.f ) {
		fTemp = speed;
		speed = 40.f;
	}

	// get x,y,z speed components
	float s = speed * cosine / dist;

	D3DXVECTOR3 vel;
	vel.x = s * diff.x;
	vel.z = s * diff.z;
	vel.y = fTemp * sine;

	return vel;
}

float CFcProjectile::GetLandHeight()
{
	float fWidth, fHeight;
	g_FcWorld.GetMapSize( fWidth, fHeight );
	if( m_Cross.m_PosVector.x < 0.f || m_Cross.m_PosVector.x > fWidth || m_Cross.m_PosVector.z < 0.f || m_Cross.m_PosVector.z > fHeight ) return m_Cross.m_PosVector.y;

	return g_BsKernel.GetLandHeight( m_Cross.m_PosVector.x, m_Cross.m_PosVector.z );
}

bool CFcProjectile::CheckHitGround()
{
	float fWidth, fHeight;
	g_FcWorld.GetMapSize( fWidth, fHeight );
	if( m_Cross.m_PosVector.x < 0.f || m_Cross.m_PosVector.x > fWidth || m_Cross.m_PosVector.z < 0.f || m_Cross.m_PosVector.z > fHeight ) return true;

	if( m_Cross.m_PosVector.y < g_BsKernel.GetLandHeight( m_Cross.m_PosVector.x, m_Cross.m_PosVector.z ) ) return true;
	return false;
}

void CFcProjectile::GetProjectileList(int p_nRtti, std::vector<CFcProjectile*> &p_vecPro )
{
	int nCnt = s_Objs.size();
	for( int i=0; i<nCnt; i++ )
	{
		CFcProjectile* Obj = s_Objs[i];
		if( Obj->GetRtti() == p_nRtti )
		{
			p_vecPro.push_back(Obj);
		}
	}
}

void CFcProjectile::GetProjectileList2(int p_nRtti1, int p_nRtti2, std::vector<CFcProjectile*> &p_vecPro )
{
	int nCnt = s_Objs.size();
	for( int i=0; i<nCnt; i++ )
	{
		CFcProjectile* Obj = s_Objs[i];
		if( Obj->GetRtti() == p_nRtti1 || Obj->GetRtti() == p_nRtti2 )
		{
			p_vecPro.push_back(Obj);
		}
	}
}


void CFcProjectile::PhysicsCheck( ASSignalData *pSignal, GameObjHandle Handle, bool p_bPush /*= false*/ )
{
//	if( ( pSignal->m_nFrame >= m_fFrame ) && ( pSignal->m_nFrame < m_fFrame + GetFrameAdd() ) )
//	{
		float fLength, fDot;

//		std::vector< CFcProp * > Result;
		D3DXVECTOR3 vPosGameObj = Handle->GetCrossVector()->m_PosVector;
		D3DXVECTOR3 vDirection;

//		CFcWorld::GetInstance().GetActiveProps( m_Cross.m_PosVector, pSignal->m_PhysicsDistance + 1000.0f, Result );
		
		std::vector<CFcProjectile*> vecPro;

		CFcProjectile::GetProjectileList2(1000,1001, vecPro);

		int nCnt = vecPro.size();

		for( int i=0; i<nCnt; i++ )
		{
			CFcProjectile* Obj = vecPro[i];

			vDirection = Obj->GetPos() - vPosGameObj;
			fLength = D3DXVec3Length( &(vDirection) );
			float fDist = ((CFcProjectileWaterBomb*)Obj)->GetScale();
			if(  fLength < fDist*4.f )
			{
				D3DXVec3Normalize( &vDirection, &vDirection );
				fDot = D3DXVec3Dot( &vDirection, &Handle->GetCrossVector()->m_ZVector );
				if( fDot > cos( ( float )pSignal->m_PhysicsAngle ) )
				{
					if( p_bPush )
					{
						vDirection = Handle->GetCrossVector()->m_XVector;
						Obj->AddForce2( &( vDirection * ( float )pSignal->m_PhysicsPower ) );
					}
					else
					{
						vDirection = Handle->GetCrossVector()->m_ZVector;
						vDirection.y += 0.35f;
						D3DXVec3Normalize( &vDirection, &vDirection );
						Obj->AddForce( &( vDirection * ( float )pSignal->m_PhysicsPower ) );
					}
					
				}
			}
		}
//	}

}



HIT_PARAM CFcProjectile::MakeHitParam( ASSignalData *pSignal )
{
	HIT_PARAM HitParam;
	HitParam.ObjectHandle = m_Param.hParent;
	HitParam.nAttackPower = m_Param.hParent->GetAttackPower();
	HitParam.pHitSignal = pSignal;
	HitParam.Position = m_Cross.m_PosVector;
	HitParam.nHitRemainFrame = ( pSignal->m_Hit2Length > 0 ) ? pSignal->m_Hit2Length : 20;

	pSignal->m_Hit2AttackPoint = m_Param.pSignal->m_pParam[4];
	pSignal->m_Hit2Particle = m_Param.pSignal->m_pParam[5];

	HitParam.fVelocityY = (float)pSignal->m_Hit2PushY;
	HitParam.fVelocityZ = (float)pSignal->m_Hit2PushZ;

	return HitParam;
}

