#include "StdAfx.h"
#include "FcProjectileBlackHall.h"
#include "FcFXManager.h"
#include "FcUnitObject.h"
#include "ASData.h"
#include "Data/SignalType.h"
#include "Data/FXList.h"
#include "BsKernel.h"
#include "BsFXObject.h"
#include "FcGameObject.h"
#include "FcWorld.h"
#include "FcGlobal.h"
#include "FcTroopObject.h"

ASSignalData *CFcProjectileBlackHall::s_pHitSignal = NULL;

CFcProjectileBlackHall::CFcProjectileBlackHall( CCrossVector *pCross )
: CFcProjectile( pCross )
{
	if( s_pHitSignal == NULL ) {
		s_pHitSignal = new ASSignalData;
		s_pHitSignal->m_nID = SIGNAL_TYPE_HIT_TEMP;
		s_pHitSignal->m_NumParam = HIT2_SIGNAL_LENGTH;
		s_pHitSignal->m_pParam = new int[s_pHitSignal->m_NumParam];

		memset( s_pHitSignal->m_pParam, 0, sizeof(int) * s_pHitSignal->m_NumParam );

		s_pHitSignal->m_Hit2Length = 120;
		s_pHitSignal->m_Hit2AniType = ANI_TYPE_DOWN;
		s_pHitSignal->m_Hit2AniIndex = 0;
		s_pHitSignal->m_Hit2PushY = 30;
		s_pHitSignal->m_Hit2PushZ = -40;
	}

	memset( m_nFXIndex, -1 ,sizeof(m_nFXIndex) );
	m_nCreateTick = -1;
	m_nState = 0;
	m_bFinish = false;
}

CFcProjectileBlackHall::~CFcProjectileBlackHall()
{
	for( int i=0; i<3; i++ ) {
		if( m_nFXIndex[i] == -1 ) continue;
		g_BsKernel.DeleteObject( m_nFXIndex[i] );
	}
}

int CFcProjectileBlackHall::Initialize( int nSkinIndex, PROJECTILE_PARAM *pParam )
{
	int nResult = CFcProjectile::Initialize( nSkinIndex, pParam );

	m_nFXIndex[0] = g_BsKernel.CreateFXObject( FX_99K06BLACKHALLS );
	m_nFXIndex[1] = g_BsKernel.CreateFXObject( FX_99K06BLACKHALL );
	m_nFXIndex[2] = g_BsKernel.CreateFXObject( FX_99K06BLACKHALLE );

	g_BsKernel.SetFXObjectState( m_nFXIndex[0], CBsFXObject::PLAY );

	CBsFXObject *pObject = (CBsFXObject *)g_BsKernel.GetEngineObjectPtr( m_nFXIndex[0] );
	if( pObject ) {
		m_nNextTick = pObject->GetTotalFrame();
	}

	m_nParentTeam = m_Param.hParent->GetTeam();
	m_nCreateTick = GetProcessTick();
	return nResult;
}

void CFcProjectileBlackHall::Process()
{
	switch( m_nState ) {
		case 0:	// 시작
			if( GetProcessTick() - m_nCreateTick >= m_nNextTick ) {
				g_BsKernel.DeleteObject( m_nFXIndex[0] );
				m_nFXIndex[0] = -1;

				g_BsKernel.SetFXObjectState( m_nFXIndex[1], CBsFXObject::PLAY, -1 );
				m_nState = 1;
			}
			break;
		case 1:	// 루프
			if( m_bFinish == true ) {
				g_BsKernel.SetFXObjectState( m_nFXIndex[1], CBsFXObject::STOPLOOP );
				CBsFXObject *pObject = (CBsFXObject *)g_BsKernel.GetEngineObjectPtr( m_nFXIndex[1] );
				if( pObject ) {
					m_nNextTick = pObject->GetTotalFrame() - pObject->GetCurFrame();
				}
				m_nCreateTick = GetProcessTick();

				m_nState = 2;
			}
			break;
		case 2:
			if( GetProcessTick() - m_nCreateTick >= m_nNextTick ) {
				g_BsKernel.DeleteObject( m_nFXIndex[1] );
				m_nFXIndex[1] = -1;

				g_BsKernel.SetFXObjectState( m_nFXIndex[2], CBsFXObject::PLAY );
				CBsFXObject *pObject = (CBsFXObject *)g_BsKernel.GetEngineObjectPtr( m_nFXIndex[2] );
				if( pObject ) {
					m_nNextTick = pObject->GetTotalFrame();
				}
				m_nCreateTick = GetProcessTick();

				m_nState = 3;

				// 남아있던 놈들은 Hit로 처리해서 날려줘야한다..
				HIT_PARAM HitParam;
				D3DXVECTOR3 vVec;
				for( DWORD i=0; i<m_vecList.size(); i++ ) {
					if( !CFcBaseObject::IsValid( m_vecList[i] ) ) continue;
					vVec = m_Cross.m_PosVector - m_vecList[i]->GetPos();
					vVec.y = 0.f;
					if( D3DXVec3Length( &vVec ) > 120.f ) continue;

					HitParam = MakeHitParam( s_pHitSignal );

					HitParam.nGroundResist = 150;
					HitParam.nUseHitDir = 1;
					HitParam.HitDir = -vVec;
					HitParam.HitDir.y = 0.f;
					HitParam.nGuardBreak = 1000;
					D3DXVec3Normalize( &HitParam.HitDir, &HitParam.HitDir );
					if( !m_vecList[i]->IsDie() && m_vecList[i]->CheckHit() )
						m_vecList[i]->CmdHit( &HitParam );
				}
			}
			break;
		case 3:
			if( GetProcessTick() - m_nCreateTick >= m_nNextTick ) {
				Delete();
				return;
			}
			break;
	}
	if( m_nState != 1 ) return;

	float fDistance = 2000.f;
	m_vecList.clear();
	g_FcWorld.GetEnemyObjectListInRange( m_nParentTeam, &m_Cross.m_PosVector, fDistance, m_vecList );
	if( m_vecList.size() == 0 ) return;

	D3DXVECTOR3 vVec, vVelocity;
	float fMinVelocity = 5.f;
	float fMaxVelocity = 20.f;
	float fLength, fLength2;
	float fTemp, fTemp2;
	for( DWORD i=0; i<m_vecList.size(); i++ ) {
		vVec = m_Cross.m_PosVector - m_vecList[i]->GetPos();
		fLength = D3DXVec3Length( &vVec );
		fLength2 = D3DXVec3Length( &D3DXVECTOR3( vVec.x, 0.f, vVec.z ) );
		if( fLength < 100.f ) {
			HIT_PARAM HitParam = MakeHitParam( s_pHitSignal );

			HitParam.nGroundResist = 150;

			HitParam.nUseHitDir = 1;
			HitParam.HitDir = -vVec;
			HitParam.HitDir.y = 0.f;
			HitParam.nGuardBreak = 1000;
			D3DXVec3Normalize( &HitParam.HitDir, &HitParam.HitDir );

			if( m_vecList[i]->IsHittable( m_Param.hParent ) ||
				( !m_vecList[i]->IsHittable( m_Param.hParent ) && m_vecList[i]->IsOrbSparkGodMode() && m_vecList[i]->GetHitParam()->nHitRemainFrame <= 0 ) )
				m_vecList[i]->CmdHit( &HitParam );

		}
		else {
			D3DXVec3Normalize( &vVec, &vVec );
			fTemp = fMinVelocity + ( ( (fMaxVelocity - fMinVelocity) / fDistance ) * ( fDistance - fLength2 ) );
			vVec *= fTemp;

			vVelocity = *m_vecList[i]->GetVelocity();
			fTemp2 = D3DXVec3Length( &vVelocity );
			if( fLength2 > 200.f && fTemp2 > fTemp ) continue;
			if( fLength2 <= 200.f && fTemp2 > 100.f ) continue;
			vVelocity += vVec;
			m_vecList[i]->SetVelocity( &vVelocity );
		}
	}
}

bool CFcProjectileBlackHall::Render()
{
	for( DWORD i=0; i<3; i++ ) {
		if( m_nFXIndex[i] == -1 ) continue;
		g_BsKernel.UpdateObject( m_nFXIndex[i], m_Cross );
	}
	return true;
}