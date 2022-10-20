#include "StdAfx.h"
#include "FcProjectile.h"
#include "FcProjectilePhysicsThrow.h"
#include "FcFXManager.h"
#include "FcUnitObject.h"
#include "FcTroopObject.h"
#include "FcGlobal.h"
#include "ASData.h"
#include "DebugUtil.h"
#include "Data/SignalType.h"
#include "fcWorld.h"
#include "BsPhysicsMgr.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG


ASSignalData *CFcProjectilePhysicsThrow::s_pHitSignal = NULL;
int CFcProjectilePhysicsThrow::s_nDeleteTick = 400;
int CFcProjectilePhysicsThrow::s_nAlphaTick = 80;

CFcProjectilePhysicsThrow::CFcProjectilePhysicsThrow( CCrossVector *pCross )
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

		s_pHitSignal->m_Hit2AniType = ANI_TYPE_DOWN;
		s_pHitSignal->m_Hit2AniIndex = 0;
		s_pHitSignal->m_Hit2PushY = 10;
		s_pHitSignal->m_Hit2PushZ = -25;

	}

}

CFcProjectilePhysicsThrow::~CFcProjectilePhysicsThrow()
{
	SAFE_DELETE( m_pActor );

}

int CFcProjectilePhysicsThrow::Initialize( int nSkinIndex, PROJECTILE_PARAM *pParam )
{
	int nResult = CFcProjectile::Initialize( nSkinIndex, pParam );

	g_BsKernel.SendMessage( m_nEngineIndex, BS_SHADOW_CAST, BS_SHADOW_BUFFER );

	AABB *pBox;
	D3DXVECTOR3 Size, AngVel;

	pBox = ( AABB * )g_BsKernel.SendMessage( m_nEngineIndex, BS_GET_BOUNDING_BOX );
	Size = *(D3DXVECTOR3*)&(( pBox->Vmax - pBox->Vmin ) * 0.5f);
	m_fRadius = D3DXVec3Length( &Size );

	
	D3DXMatrixTranslation(&m_matLocal, -pBox->GetCenter().x, -pBox->GetCenter().y, -pBox->GetCenter().z);
	
	// temp
	D3DXMATRIX matRot;
	D3DXMatrixRotationZ(&matRot, D3DX_PI * 0.045f );	
	D3DXMatrixMultiply( &m_matLocal, &m_matLocal, &matRot);

	m_nDestroyTick = s_nDeleteTick;

	// Temp
	//Size.x -= 20.f;
	//Size.z -= 20.f;
	/////////////
	if( fabs( 1.f - (Size.x / Size.y) ) < 0.1f &&
		fabs( 1.f - (Size.x / Size.z) ) < 0.1f ) {
		m_pActor = CBsPhysicsStone::Create( Size.x, *( ( D3DXMATRIX *)m_Cross ) );
	}
	else {
		float fHeight = BsMax(BsMax( Size.x, Size.y), Size.z);
		float fRadius = (Size.x + Size. y + Size.z - fHeight) * 0.5f;
		fHeight *= 2.f;
		fRadius *= 0.5f; // temp
		m_pActor = CBsPhysicsCapsule::Create( fRadius, fHeight, *( ( D3DXMATRIX *)m_Cross ) );
	}

	if( !m_pActor->GetActor() ) return -1;
	m_pActor->SetGroup("ThrowBox");

	m_bApplyDamage = true;
	m_vPrevPos = m_Cross.m_PosVector;

	m_pActor->GetActor()->setLinearDamping( 0.1f );
	m_pActor->GetActor()->setAngularDamping( 0.1f );
	m_pActor->GetActor()->setMaxAngularVelocity( FLT_MAX );
	
	D3DXVECTOR3 LinVel = m_Param.vTarget * 20.f;
	AngVel = m_Cross.m_XVector * 5.f;

	m_pActor->AddVelocity( &LinVel, &AngVel );

	m_nParentTeam = m_Param.hParent->GetTeam();

	return nResult;
}

void CFcProjectilePhysicsThrow::Process()
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

	D3DXMatrixMultiply( &ActorMat, &m_matLocal, &ActorMat);
	memcpy( &m_Cross.m_XVector, &ActorMat._11, sizeof( D3DXVECTOR3 ) );
	memcpy( &m_Cross.m_YVector, &ActorMat._21, sizeof( D3DXVECTOR3 ) );
	memcpy( &m_Cross.m_ZVector, &ActorMat._31, sizeof( D3DXVECTOR3 ) );
	memcpy( &m_Cross.m_PosVector, &ActorMat._41, sizeof( D3DXVECTOR3 ) );
	if( m_bApplyDamage && s_nDeleteTick - m_nDestroyTick > 40 && D3DXVec3LengthSq( (D3DXVECTOR3*)&(m_Cross.m_PosVector - m_vPrevPos) ) < 100.f * 100.f ) {
		m_bApplyDamage = false;
	}
	m_vPrevPos = m_Cross.m_PosVector;

	if( m_bApplyDamage == true ) {
		ProcessDamage();
	}
}


void CFcProjectilePhysicsThrow::ProcessDamage()
{
	if( m_nDestroyTick % 5 != 0 ) return;

	g_FcWorld.GivePhysicsRange( m_Handle, &m_Cross.m_PosVector, m_fRadius, 20.0f, 500, PROP_BREAK_TYPE_NORMAL );

	std::vector<GameObjHandle> VecList;
	g_FcWorld.GetEnemyObjectListInRange( m_nParentTeam, &m_Cross.m_PosVector, m_fRadius, VecList );
	if( VecList.empty() ) return;

	/*
	HIT_PARAM HitParam;

	HitParam.ObjectHandle = m_Param.hParent;
	HitParam.nAttackPower = m_Param.hParent->GetAttackPower();
	HitParam.pHitSignal = s_pHitSignal;
	HitParam.Position = m_Cross.m_PosVector;
	HitParam.nHitRemainFrame = 6;
	HitParam.fVelocityY = 10.f;
	HitParam.fVelocityZ = -25.f;

	s_pHitSignal->m_pParam[6] = m_Param.pSignal->m_pParam[5];
	s_pHitSignal->m_pParam[1] = m_Param.pSignal->m_pParam[4];
	*/
	HIT_PARAM HitParam = MakeHitParam( s_pHitSignal );


	for( DWORD i=0; i<VecList.size(); i++ ) {
		if( VecList[i] == m_Param.hParent ) continue;
		//		if( VecList[i]->GetTeam() == m_Param.hParent->GetTeam() ) continue;
		m_Param.hParent->GetTroop()->OnAttack( TROOP_ATTACK_TYPE_RANGE, VecList[i] );

		if( VecList[i]->IsHittable( m_Param.hParent ) )
			VecList[i]->CmdHit( &HitParam );
	}
}

