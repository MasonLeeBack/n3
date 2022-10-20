#include "StdAfx.h"
#include "FcProjectileTimeBomb.h"
#include "FcGlobal.h"
#include "FcGameObject.h"
#include "BsFXObject.h"
#include "Data/SignalType.h"
#include "FcWorld.h"
#include "ASData.h"
#include "FcSoundManager.h"


ASSignalData *CFcProjectileTimeBomb::s_pHitSignal = NULL;

CFcProjectileTimeBomb::CFcProjectileTimeBomb( CCrossVector *pCross )
: CFcProjectile( pCross )
{
	memset( m_nFxObjectIndex, -1, sizeof(m_nFxObjectIndex) );

	if( s_pHitSignal == NULL ) {
		s_pHitSignal = new ASSignalData;
		s_pHitSignal->m_nID = SIGNAL_TYPE_HIT_TEMP;
		s_pHitSignal->m_NumParam = HIT2_SIGNAL_LENGTH;
		s_pHitSignal->m_pParam = new int[s_pHitSignal->m_NumParam];

		memset( s_pHitSignal->m_pParam, 0, sizeof(int) * s_pHitSignal->m_NumParam );

		s_pHitSignal->m_Hit2AniType = ANI_TYPE_DOWN;
		s_pHitSignal->m_Hit2Length = 40;
		s_pHitSignal->m_Hit2AniIndex = 0;
		s_pHitSignal->m_Hit2PushY = 15;
		s_pHitSignal->m_Hit2PushZ = -20;
	}
	m_nDamageCount = 1;
	m_nDamageDelay = 0;
}

CFcProjectileTimeBomb::~CFcProjectileTimeBomb()
{
	for( int i=0; i<2; i++ ) {
		if( m_nFxObjectIndex[i] != -1 ) {
			g_BsKernel.DeleteObject( m_nFxObjectIndex[i] );
			m_nFxObjectIndex[i] = -1;
		}
	}
}

int CFcProjectileTimeBomb::Initialize( int nStartFxIndex, int nExpFxIndex, int nStartDamageTime, int nDamageCount, int nDamageTick, PROJECTILE_PARAM *pParam )
{
	CFcProjectile::Initialize( -1, pParam );
	if( nStartFxIndex != -1 ) {
#ifdef _XBOX
		g_pSoundManager->PlayFxSound( nStartFxIndex, &m_Cross.m_PosVector );
#endif
		m_nFxObjectIndex[0] = g_BsKernel.CreateFXObject( nStartFxIndex );
	}
	if( nExpFxIndex != -1 ) {
		m_nExplotionFxIndex = nExpFxIndex;
		m_nFxObjectIndex[1] = g_BsKernel.CreateFXObject( nExpFxIndex );
	}

	m_nDestroyTick = 1;
	m_nDamageTick = 1;
	if( m_nFxObjectIndex[0] != -1 ) {
		CBsFXObject *pObject = (CBsFXObject *)g_BsKernel.GetEngineObjectPtr( m_nFxObjectIndex[0] );
		if( pObject == NULL ) m_nDestroyTick = 1;
		else {
			m_nDestroyTick = pObject->GetTotalFrame();
			m_nDamageTick = m_nDestroyTick;
		}
	}
	if( m_nFxObjectIndex[1] != -1 ) {
		CBsFXObject *pObject = (CBsFXObject *)g_BsKernel.GetEngineObjectPtr( m_nFxObjectIndex[1] );
		if( pObject == NULL ) m_nDestroyTick += 1;
		else m_nDestroyTick += pObject->GetTotalFrame();
	}

	if( nStartDamageTime == -1 )
		m_nDamageTick = m_nDestroyTick - m_nDamageTick;
	else m_nDamageTick = m_nDestroyTick - nStartDamageTime;

	m_nParentTeam = m_Param.hParent->GetTeam();

	if( m_nFxObjectIndex[0] != -1 )
		g_BsKernel.SetFXObjectState( m_nFxObjectIndex[0], CBsFXObject::PLAY );

	m_nDamageCount = nDamageCount;
	m_nDamageDelay = nDamageTick;
	return 1;
}

void CFcProjectileTimeBomb::Process()
{
	if( m_nDestroyTick <= 0 ) {
		Delete();
		return;
	}

	if( m_nDestroyTick == m_nDamageTick ) {
		if( m_nFxObjectIndex[1] != -1 ){
			g_BsKernel.SetFXObjectState( m_nFxObjectIndex[1], CBsFXObject::PLAY );
#ifdef _XBOX
			g_pSoundManager->PlayFxSound( m_nExplotionFxIndex, &m_Cross.m_PosVector );
#endif
		}

	}
	if( m_nDamageTick >= m_nDestroyTick ) ProcessDamage();

	--m_nDestroyTick;
}

bool CFcProjectileTimeBomb::Render()
{
	for( int i=0; i<2; i++ ) {
		if( m_nFxObjectIndex[i] != -1 ) {
			if( g_BsKernel.GetFXObjectState( m_nFxObjectIndex[i] ) != CBsFXObject::PLAY ) continue;
			g_BsKernel.UpdateObject( m_nFxObjectIndex[i], m_Cross );
		}
	}
	return true;
}

void CFcProjectileTimeBomb::ProcessDamage()
{
	if( m_nDamageCount <= 0 ) return;
	int nTick = m_nDamageTick - m_nDestroyTick;

	if( nTick != 0 && nTick % m_nDamageDelay != 0 ) return;

	--m_nDamageCount;
	HIT_PARAM HitParam;
	HitParam = MakeHitParam( s_pHitSignal );

	std::vector< GameObjHandle > VecList;
	g_FcWorld.GetEnemyObjectListInRange( m_nParentTeam, &m_Cross.m_PosVector, m_Param.fSize, VecList );

	for( DWORD i=0; i<VecList.size(); i++ ) {
		if( VecList[i]->IsHittable( HitParam.ObjectHandle ) )
			VecList[i]->CmdHit( &HitParam );
	}
}