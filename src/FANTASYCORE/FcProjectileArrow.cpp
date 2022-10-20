#include "StdAfx.h"
#include "FcProjectileArrow.h"
#include "FcFXManager.h"
#include "FcUnitObject.h"
#include "FcTroopObject.h"
#include "FcGlobal.h"
#include "ASData.h"
#include "DebugUtil.h"
#include "Data/SignalType.h"
#include "Box3.h"
#include "Ray3.h"
#include "IntLin3Box3.h"
#include "FcArcherUnitObject.h"
#include "FcVelocityCtrl.h"
#include "FcFlyUnitObject.h"
#include ".\\data\\Sound\\FcSoundPlayDefinitions.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG


ASSignalData *CFcProjectileArrow::s_pHitSignal = NULL;

CFcProjectileArrow::CFcProjectileArrow( CCrossVector *pCross )
: CFcProjectile( pCross )
{
	m_nFXLineIndex = -1;
	m_nDestroyTick = -1;

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
		s_pHitSignal->m_Hit2SoundAction = ATT_NONE;
		s_pHitSignal->m_Hit2WeaponMaterial = WT_BOW;

		/*
		s_pHitSignal->m_pParam[2] = ANI_TYPE_HIT;
		s_pHitSignal->m_pParam[3] = 0;
		s_pHitSignal->m_pParam[6] = -1;
		s_pHitSignal->m_pParam[10] = 6;
		*/
	}
}

CFcProjectileArrow::~CFcProjectileArrow()
{
	if( m_nFXLineIndex != -1 ) {
		SAFE_DELETE_FX(m_nFXLineIndex,FX_TYPE_LINETRAIL);
		/*g_pFcFXManager->SendMessage(m_nFXLineIndex, FX_DELETE_OBJECT);
		m_nFXLineIndex = -1;*/
	}
}

int CFcProjectileArrow::Initialize( int nSkinIndex, PROJECTILE_PARAM *pParam )
{
	int nResult = CFcProjectile::Initialize( nSkinIndex, pParam );

	D3DXVECTOR3 vTarget = pParam->hTarget->GetPos();
	float fTargetHeight = pParam->hTarget->GetUnitHeight() / 2.f;
	float fTargetWidth = pParam->hTarget->GetUnitRadius() / 2.f;

	if( pParam->hTarget->GetClassID() == CFcGameObject::Class_ID_Fly ) {	// 예측사격
		float fSpeed = ((CFcFlyUnitObject*)pParam->hTarget.GetPointer())->GetVelocityCtrl()->GetCurSpeed() * RandomNumberInRange( 30.f, 60.f );
		vTarget += pParam->hTarget->GetCrossVector()->m_ZVector * 1000.f;
	}
	if( Random(100) < pParam->nHitProb ) {
		vTarget.y += fTargetHeight + Random( (int)fTargetHeight );
		vTarget.x += -fTargetWidth + Random( (int)fTargetWidth*2 );
		vTarget.z += -fTargetWidth + Random( (int)fTargetWidth*2 );
	}
	else {
		if( Random(2) == 0 )
			vTarget.y += ( fTargetHeight * 2 ) + Random(200);
		else vTarget.y -= ( fTargetHeight * 2 ) + Random(200);

		if( Random(2) == 0 )
			vTarget.x += ( fTargetWidth * 2 ) + Random(200);
		else vTarget.x -= ( fTargetWidth * 2 ) + Random(200);

		if( Random(2) == 0 )
			vTarget.z += ( fTargetWidth * 2 ) + Random(200);
		else vTarget.z -= ( fTargetWidth * 2 ) + Random(200);
	}

	m_Velocity = GetVelocity( m_Cross.m_PosVector, CFcProjectile::s_gravity, sin( pParam->fAngle / 180.f * D3DX_PI ), vTarget );
	m_Accel = s_gravity;		// 화살은 중력만 있음
	m_nTrailLifeTick = 20;

	m_nFXLineIndex = g_pFcFXManager->Create(FX_TYPE_LINETRAIL);
	if( m_nFXLineIndex == -1 ) return nResult;

	float nE = 25.f;
	float nW = 0.8f;
	static char szFile[128] = "arrowline.dds";
	g_BsKernel.LoadTexture( szFile );

	g_pFcFXManager->SendMessage( m_nFXLineIndex, FX_INIT_OBJECT, m_nTrailLifeTick, (DWORD)&nW, (DWORD)&nE );
	g_pFcFXManager->SendMessage( m_nFXLineIndex, FX_SET_TEXTURE, (DWORD)szFile );
	g_pFcFXManager->SendMessage( m_nFXLineIndex, FX_PLAY_OBJECT );

	m_PrevPos = m_Cross.m_PosVector;
	return nResult;
}

void CFcProjectileArrow::Process()
{
//	CFcProjectile::Process();
	if( m_nDestroyTick != -1 ) {
		--m_nDestroyTick;
		if( m_nDestroyTick == 0 ) { Delete(); }

		return;
	}
	m_Velocity += m_Accel;

	m_PrevPos = m_Cross.m_PosVector;
	D3DXVec3Normalize( &(m_Cross.m_ZVector), &m_Velocity );
	D3DXVec3Cross( &(m_Cross.m_YVector), &(m_Cross.m_ZVector), &(m_Cross.m_XVector) );
	D3DXVec3Normalize( &(m_Cross.m_YVector), &(m_Cross.m_YVector) );
	D3DXVECTOR3 newPos = m_Cross.m_PosVector + m_Velocity;

	// Check Hit Unit;
	if( CheckHitGround() || !CFcBaseObject::IsValid( m_Param.hParent ) )
	{
		m_nDestroyTick = m_nTrailLifeTick * 5;

		m_Cross.m_PosVector = newPos;

//		m_Cross.m_PosVector.y = g_BsKernel.GetLandHeight( m_Cross.m_PosVector.x, m_Cross.m_PosVector.z ) - 10.f;  
		m_Cross.m_PosVector.y = GetLandHeight() - 10.f;
	}
	else if( CheckHitTarget() && m_Param.hTarget->IsHittable( m_Param.hParent ) )
	{
		m_Param.hParent->GetTroop()->OnAttack( TROOP_ATTACK_TYPE_RANGE, m_Param.hTarget );

		HIT_PARAM HitParam = MakeHitParam( s_pHitSignal );

		HitParam.nUseHitDir = 1;
		HitParam.HitDir = newPos - m_PrevPos;
		HitParam.HitDir.y = 0.f;
		D3DXVec3Normalize( &HitParam.HitDir, &HitParam.HitDir );

		m_Param.hTarget->CmdHit( &HitParam );

		m_bUpdateObject = false;
		m_nDestroyTick = m_nTrailLifeTick * 5;

		m_Cross.m_PosVector = newPos;
	}
	else m_Cross.m_PosVector = newPos;

	g_pFcFXManager->SendMessage( m_nFXLineIndex, FX_UPDATE_OBJECT, (DWORD)&m_Cross.m_PosVector );
}

bool CFcProjectileArrow::CheckHitTarget()
{
	if( !CFcBaseObject::IsValid( m_Param.hTarget ) ) return false;

	Box3 Box;
	Segment3 Seg3;
	m_Param.hTarget->GetBox3( Box, m_Param.hTarget->GetBoundingBox() );

	Seg3.P.x = m_Cross.m_PosVector.x;
	Seg3.P.y = m_Cross.m_PosVector.y;
	Seg3.P.z = m_Cross.m_PosVector.z;

	Seg3.D.x = m_Cross.m_PosVector.x - m_PrevPos.x;
	Seg3.D.y = m_Cross.m_PosVector.y - m_PrevPos.y;
	Seg3.D.z = m_Cross.m_PosVector.z - m_PrevPos.z;
	if( TestIntersection( Seg3, Box ) == true ) {
		return true;
	}

	return false;
}