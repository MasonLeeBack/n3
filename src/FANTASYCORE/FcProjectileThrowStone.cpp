#include "StdAfx.h"
#include "FcProjectile.h"
#include "FcProjectileThrowStone.h"
#include "FcFXManager.h"
#include "FcUnitObject.h"
#include "FcTroopObject.h"
#include "FcGlobal.h"
#include "ASData.h"
#include "DebugUtil.h"
#include "Data/SignalType.h"
#include "fcWorld.h"
#include "BsPhysicsMgr.h"
#include "Ray3.h"
#include "IntLin3Box3.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG


ASSignalData *CFcProjectileThrowStone::s_pHitSignal = NULL;
int CFcProjectileThrowStone::s_nDeleteTick = 200;
int CFcProjectileThrowStone::s_nAlphaTick = 40;

CFcProjectileThrowStone::CFcProjectileThrowStone( CCrossVector *pCross )
: CFcProjectile( pCross )
{
	m_nDestroyTick = -1;
	m_pActor = NULL;

	if( s_pHitSignal == NULL ) {
		s_pHitSignal = new ASSignalData;
		s_pHitSignal->m_nID = SIGNAL_TYPE_HIT_TEMP;
		s_pHitSignal->m_NumParam = HIT2_SIGNAL_LENGTH;
		s_pHitSignal->m_pParam = new int[s_pHitSignal->m_NumParam];

		memset( s_pHitSignal->m_pParam, 0, sizeof(int) * s_pHitSignal->m_NumParam );

		s_pHitSignal->m_Hit2AniType = ANI_TYPE_HIT;
		s_pHitSignal->m_Hit2AniIndex = 0;
		s_pHitSignal->m_Hit2PushY = 5;
		s_pHitSignal->m_Hit2PushZ = -10;
	}

}

CFcProjectileThrowStone::~CFcProjectileThrowStone()
{
	SAFE_DELETE( m_pActor );

}

int CFcProjectileThrowStone::Initialize( int nSkinIndex, PROJECTILE_PARAM *pParam )
{
	int nResult = CFcProjectile::Initialize( nSkinIndex, pParam );

	AABB *pBox;
	D3DXVECTOR3 Size, AngVel;

	pBox = ( AABB * )g_BsKernel.SendMessage( m_nEngineIndex, BS_GET_BOUNDING_BOX );
	Size = *(D3DXVECTOR3*)&(( pBox->Vmax - pBox->Vmin ) * 0.5f);
	m_fRadius = D3DXVec3Length( &Size );

	m_nDestroyTick = s_nDeleteTick;

	m_pActor = CBsPhysicsBox::Create( Size, *( ( D3DXMATRIX *)m_Cross ) );

	if( !m_pActor->GetActor() ) return -1;
	m_pActor->SetGroup("ThrowBox");

	m_bApplyDamage = true;
	m_vPrevPos = m_Cross.m_PosVector;

	m_pActor->GetActor()->setLinearDamping( 1.0f );
	m_pActor->GetActor()->setAngularDamping( 1.0f );
	m_pActor->GetActor()->setMaxAngularVelocity( FLT_MAX );

	AngVel = D3DXVECTOR3( 0.0f, 0.0f, 0.0f );
//	AngVel = m_Cross.m_YVector * 2.f;
	m_pActor->AddVelocity( &m_Param.vTarget, &AngVel );

	return nResult;
}

void CFcProjectileThrowStone::Process()
{
	--m_nDestroyTick;
	if( m_nDestroyTick == 0 ) {
		Delete();
		return;
	}
	if( m_nDestroyTick < s_nAlphaTick ) {
		float fAlphaWeight;

		fAlphaWeight = m_nDestroyTick / ( float )s_nAlphaTick;
		g_BsKernel.SendMessage( m_nEngineIndex, BS_ENABLE_OBJECT_ALPHABLEND, 1 );
		g_BsKernel.SendMessage( m_nEngineIndex, BS_SET_OBJECT_ALPHA, ( DWORD )&fAlphaWeight );
	}

	D3DXMATRIX ActorMat;

	m_pActor->GetGlobalPose( ActorMat );
	memcpy( &m_Cross.m_XVector, &ActorMat._11, sizeof( D3DXVECTOR3 ) );
	memcpy( &m_Cross.m_YVector, &ActorMat._21, sizeof( D3DXVECTOR3 ) );
	memcpy( &m_Cross.m_ZVector, &ActorMat._31, sizeof( D3DXVECTOR3 ) );
	memcpy( &m_Cross.m_PosVector, &ActorMat._41, sizeof( D3DXVECTOR3 ) );
//	m_Cross.RotateRoll( -256 );
	m_Cross.m_YVector = m_Cross.m_PosVector - m_vPrevPos;
	D3DXVec3Normalize( &m_Cross.m_YVector, &m_Cross.m_YVector );
	m_Cross.UpdateVectorsY();
	m_Cross.RotateYaw( m_nDestroyTick );
	if( m_bApplyDamage && s_nDeleteTick - m_nDestroyTick > 40 && D3DXVec3LengthSq( (D3DXVECTOR3*)&(m_Cross.m_PosVector - m_vPrevPos) ) < 100.f * 100.f ) {
		m_bApplyDamage = false;
	}
	m_vPrevPos = m_Cross.m_PosVector;

	if( m_bApplyDamage == true ) {
		ProcessDamage();
	}
}


void CFcProjectileThrowStone::ProcessDamage()
{
//	if( m_nDestroyTick % 5 != 0 ) return;

	float fLength = 200.f;//D3DXVec3Length( (D3DXVECTOR3*)&( m_Cross.m_PosVector - m_vPrevPos ) );
	std::vector<GameObjHandle> VecList;
	g_FcWorld.GetObjectListInRange( &m_Cross.m_PosVector, fLength, VecList );
	if( VecList.empty() ) return;

	Box3 Box;
	Segment3 Seg3;
	for( DWORD i=0; i<VecList.size(); i++ ) {
		if( VecList[i] == m_Param.hParent ) continue;
		if( !VecList[i]->IsHittable( m_Param.hParent ) ) continue;

		VecList[i]->GetBox3( Box, VecList[i]->GetBoundingBox() );
		Box.E[0] += 20.f;
		Box.E[1] += 20.f;
		Box.E[2] += 20.f;
		Seg3.P.x = m_Cross.m_PosVector.x;
		Seg3.P.y = m_Cross.m_PosVector.y;
		Seg3.P.z = m_Cross.m_PosVector.z;

		Seg3.D.x = m_Cross.m_PosVector.x - m_vPrevPos.x;
		Seg3.D.y = m_Cross.m_PosVector.y - m_vPrevPos.y;
		Seg3.D.z = m_Cross.m_PosVector.z - m_vPrevPos.z;
		if( TestIntersection( Seg3, Box ) == false ) continue;

		HIT_PARAM HitParam = MakeHitParam( s_pHitSignal );

		VecList[i]->CmdHit( &HitParam );

		D3DXVECTOR3 vVel = m_pActor->GetVelocity();
		vVel.y = -vVel.y;
		vVel = -vVel * 0.2f;
		m_pActor->GetActor()->setLinearVelocity( *(NxVec3*)&(vVel*0.01f) );
		m_bApplyDamage = false;
		break;
	}
}

