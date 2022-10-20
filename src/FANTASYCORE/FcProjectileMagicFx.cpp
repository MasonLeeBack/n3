#include "StdAfx.h"
#include "FcProjectileMagicFx.h"
#include "FcGlobal.h"
#include "FcGameObject.h"
#include "FcVelocityCtrl.h"
#include "BsFXObject.h"
#include "Data/SignalType.h"
#include "FcWorld.h"
#include "FcSoundManager.h"

ASSignalData *CFcProjectileMagicFx::s_pHitSignal = NULL;

CFcProjectileMagicFx::CFcProjectileMagicFx( CCrossVector *pCross )
: CFcProjectile( pCross )
{
	memset( m_nFxObjectIndex, -1, sizeof(m_nFxObjectIndex) );
	m_pVelocity = new CFcVelocityCtrl( 3.0f, 32.f, 2.f, 1.03f, 0.98f );

	if( s_pHitSignal == NULL ) {
		s_pHitSignal = new ASSignalData;
		s_pHitSignal->m_nID = SIGNAL_TYPE_HIT_TEMP;
		s_pHitSignal->m_NumParam = HIT2_SIGNAL_LENGTH;
		s_pHitSignal->m_pParam = new int[s_pHitSignal->m_NumParam];

		memset( s_pHitSignal->m_pParam, 0, sizeof(int) * s_pHitSignal->m_NumParam );

		s_pHitSignal->m_Hit2Length = 30;
		s_pHitSignal->m_Hit2AniType = ANI_TYPE_HIT;
		s_pHitSignal->m_Hit2AniIndex = 0;
		s_pHitSignal->m_Hit2PushY = 5;
		s_pHitSignal->m_Hit2PushZ = -10;

	}
	m_bHidePosition = false;
	m_bExplosion = false;
	m_nHitType = ANI_TYPE_HIT;
}

CFcProjectileMagicFx::~CFcProjectileMagicFx()
{
	for( int i=0; i<2; i++ ) {
		if( m_nFxObjectIndex[i] != -1 ) {
			g_BsKernel.DeleteObject( m_nFxObjectIndex[i] );
			m_nFxObjectIndex[i] = -1;
		}
	}
}

int CFcProjectileMagicFx::Initialize( int nThrowFxIndex, int nExpFxIndex, PROJECTILE_PARAM *pParam )
{
	CFcProjectile::Initialize( -1, pParam );
	if( nThrowFxIndex != -1 )
		m_nFxObjectIndex[0] = g_BsKernel.CreateFXObject( nThrowFxIndex );
	if( nExpFxIndex != -1 ){
		m_nExplotionFxIndex = nExpFxIndex;
		m_nFxObjectIndex[1] = g_BsKernel.CreateFXObject( nExpFxIndex );
	}

	m_nDestroyTick = 300;
	m_bDestroy = false;
	m_nParentTeam = m_Param.hParent->GetTeam();

	if( m_Param.hTarget ) m_nTargetType = 0;
	else if( D3DXVec3Length( &m_Param.vTarget ) > 0.f ) m_nTargetType = 1;
	else {
		m_nTargetType = 2;
		CBsFXObject *pObject = (CBsFXObject *)g_BsKernel.GetEngineObjectPtr( m_nFxObjectIndex[0] );
		if( pObject ) {
			m_nDestroyTick = (int)pObject->GetTotalFrame() - 2;
		}
	}

	if( m_nFxObjectIndex[0] != -1 ) {
		g_BsKernel.SetFXObjectState( m_nFxObjectIndex[0], CBsFXObject::PLAY, -1 );
	}

	return 1;
}

void CFcProjectileMagicFx::Process()
{
	if( m_nDestroyTick <= 0 ) {
		Delete();
		return;
	}
	--m_nDestroyTick;
	if( m_bDestroy == true ) return;

	switch( m_nTargetType ) {
		case 0:
			if( !CFcBaseObject::IsValid( m_Param.hTarget ) || CheckExp() == true ) {
				m_bExplosion = true;
				ProcessDamage();

				m_nDestroyTick = 0;
				if( m_bHidePosition == false ) {
					g_BsKernel.DeleteObject( m_nFxObjectIndex[0] );
					m_nFxObjectIndex[0] = -1;
				}
				else {
					g_BsKernel.SetFXObjectState( m_nFxObjectIndex[0], CBsFXObject::STOPLOOP );
					CBsFXObject *pObject = (CBsFXObject *)g_BsKernel.GetEngineObjectPtr( m_nFxObjectIndex[0] );
					if( pObject ) {
						m_nDestroyTick += pObject->GetTotalFrame();
					}
				}

				if( m_nFxObjectIndex[1] != -1 ) {
					g_BsKernel.SetFXObjectState( m_nFxObjectIndex[1], CBsFXObject::PLAY );
					CBsFXObject *pObject = (CBsFXObject *)g_BsKernel.GetEngineObjectPtr( m_nFxObjectIndex[1] );

#ifdef _XBOX
					g_pSoundManager->PlayFxSound( m_nExplotionFxIndex,&m_Cross.m_PosVector );
#endif

					if( pObject ) {
						if( m_nDestroyTick < (int)pObject->GetTotalFrame() )
							m_nDestroyTick = (int)pObject->GetTotalFrame();
					}
					m_bDestroy = true;
				}
			}
			else {
				float fSpeed = 0.f;
				D3DXVECTOR3 vCross;
				D3DXVECTOR3 vVec = m_Param.hTarget->GetPos() - m_Cross.m_PosVector;
				vVec.y = 0.f;
				D3DXVec3Normalize( &vVec, &vVec );
				float fDot = D3DXVec3Dot( &vVec, &m_Cross.m_ZVector );
				float fAngle = acos( fDot ) * 180.f / 3.1415926f;

				if( fAngle > 5.f ) {
					D3DXVec3Cross( &vCross, &vVec, &m_Cross.m_ZVector );
					if( vCross.y > 0 ) {
						m_Cross.RotateYaw( int(5.f * 2.84f) );
					}
					else {
						m_Cross.RotateYaw( -int(5.f * 2.84f) );
					}
					fSpeed = m_pVelocity->GetSpeed( CFcVelocityCtrl::DECELERATION );

					fDot = D3DXVec3Dot( &vVec, &m_Cross.m_ZVector );
					fAngle = acos( fDot ) * 180.f / 3.1415926f;
					if( fAngle <= 5.f ) {
						m_Cross.m_ZVector = vVec;
						m_Cross.UpdateVectors();
					}
				}
				else {
					m_Cross.m_ZVector = vVec;
					m_Cross.UpdateVectors();
					fSpeed = m_pVelocity->GetSpeed( CFcVelocityCtrl::ACCELERATION );
				}

				//		m_Cross.m_PosVector.y = g_FcWorld.GetLandHeight( m_Cross.m_PosVector.x, m_Cross.m_PosVector.z ) + m_Param.pSignal->m_pParam[2];
				m_Cross.m_PosVector.y = GetLandHeight() + m_Param.pSignal->m_pParam[2];
				m_Cross.MoveFrontBack( fSpeed );
			}
			break;
		case 1:
			if( CheckExp() == true ) {
				m_bExplosion = true;

				D3DXVECTOR3 vVec = m_Param.vTarget - m_Cross.m_PosVector;
				vVec.y = 0.f;
				D3DXVec3Normalize( &vVec, &vVec );
				m_Cross.m_ZVector = vVec;
				m_Cross.UpdateVectors();

				ProcessDamage();

				m_nDestroyTick = 0;

				if( m_bHidePosition == false ) {
					g_BsKernel.DeleteObject( m_nFxObjectIndex[0] );
					m_nFxObjectIndex[0] = -1;
				}
				else {
//					g_BsKernel.SetFXObjectState( m_nFxObjectIndex[0], CBsFXObject::STOPLOOP );
					CBsFXObject *pObject = (CBsFXObject *)g_BsKernel.GetEngineObjectPtr( m_nFxObjectIndex[0] );
					if( pObject ) {
						m_nDestroyTick += pObject->GetTotalFrame();
					}
				}

				if( m_nFxObjectIndex[1] != -1 ) {
#ifdef _XBOX
					g_pSoundManager->PlayFxSound( m_nExplotionFxIndex,&m_Cross.m_PosVector );
#endif
					g_BsKernel.SetFXObjectState( m_nFxObjectIndex[1], CBsFXObject::PLAY );
					CBsFXObject *pObject = (CBsFXObject *)g_BsKernel.GetEngineObjectPtr( m_nFxObjectIndex[1] );
					if( pObject ) {
						if( m_nDestroyTick < (int)pObject->GetTotalFrame() )
							m_nDestroyTick = (int)pObject->GetTotalFrame();
					}
					m_bDestroy = true;
				}
			}
			else {
				float fSpeed = 0.f;
				D3DXVECTOR3 vCross;
				D3DXVECTOR3 vVec = m_Param.vTarget - m_Cross.m_PosVector;
				D3DXVec3Normalize( &vVec, &vVec );

				m_Cross.m_ZVector = vVec;
				m_Cross.UpdateVectors();
				m_Cross.MoveFrontBack( m_pVelocity->GetSpeed( CFcVelocityCtrl::ACCELERATION ) );
			}
			break;
		case 2:
			{
				m_Cross.MoveFrontBack( m_pVelocity->GetSpeed( CFcVelocityCtrl::ACCELERATION ) );
				if( GetProcessTick() % 5 == 0 ) {
					ProcessDamage();
				}
			}
			break;
	}
}

bool CFcProjectileMagicFx::Render()
{
	for( int i=0; i<2; i++ ) {
		if( m_nFxObjectIndex[i] != -1 ) {
			if( g_BsKernel.GetFXObjectState( m_nFxObjectIndex[i] ) != CBsFXObject::PLAY ) continue;
			if( i == 0 && m_bHidePosition == true && m_bExplosion == true ) {
				float fTemp = m_Cross.m_PosVector.y;
				m_Cross.m_PosVector.y = g_FcWorld.GetLandHeight( m_Cross.m_PosVector.x, m_Cross.m_PosVector.z ) - 2000.f;
				g_BsKernel.UpdateObject( m_nFxObjectIndex[i], m_Cross );
				m_Cross.m_PosVector.y = fTemp;
				g_BsKernel.SendMessage( m_nFxObjectIndex[i], BS_ENABLE_OBJECT_CULL, FALSE );
			}
			else g_BsKernel.UpdateObject( m_nFxObjectIndex[i], m_Cross );
		}
	}
	return true;
}

void CFcProjectileMagicFx::ProcessDamage()
{
	if(!CFcBaseObject::IsValid( m_Param.hParent ))
		return;

	switch( m_nHitType ) {
		case ANI_TYPE_HIT:
			s_pHitSignal->m_Hit2AniType = ANI_TYPE_HIT;
			s_pHitSignal->m_Hit2AniIndex = 0;
			s_pHitSignal->m_Hit2PushY = 5;
			s_pHitSignal->m_Hit2PushZ = -10;
			break;
		case ANI_TYPE_DOWN:
			s_pHitSignal->m_Hit2AniType = ANI_TYPE_DOWN;
			s_pHitSignal->m_Hit2Length = 40;
			s_pHitSignal->m_Hit2AniIndex = 0;
			s_pHitSignal->m_Hit2PushY = 15;
			s_pHitSignal->m_Hit2PushZ = -20;
			break;
	}
	s_pHitSignal->m_pParam[1] = m_Param.pSignal->m_pParam[4];

	HIT_PARAM HitParam = MakeHitParam( s_pHitSignal );

	std::vector< GameObjHandle > VecList;
	g_FcWorld.GetEnemyObjectListInRange( m_nParentTeam, &m_Cross.m_PosVector, m_Param.fSize, VecList );
	for( DWORD i=0; i<VecList.size(); i++ ) {
		if( VecList[i]->IsHittable( HitParam.ObjectHandle ) )
			VecList[i]->CmdHit( &HitParam );
	}
}

bool CFcProjectileMagicFx::CheckExp()
{
	if( m_nDestroyTick == 0 ) return true;
	switch( m_nTargetType ) {
		case 0:
			{
				float fExpLength = m_Param.hTarget->GetUnitRadius();
				D3DXVECTOR3 vTemp = m_Param.hTarget->GetPos() - m_Cross.m_PosVector;
				vTemp.y = 0.f;
				if( D3DXVec3LengthSq( &vTemp ) < fExpLength * fExpLength ) return true;
				return false;
			}
			break;
		case 1:
			{
				D3DXVECTOR3 vTemp = m_Param.vTarget - m_Cross.m_PosVector;
				vTemp.y = 0.f;
				if( D3DXVec3LengthSq( &vTemp ) <= m_pVelocity->GetCurSpeed() * m_pVelocity->GetCurSpeed() ) return true;
				return false;
			}
			break;
	}
	return true;
}

void CFcProjectileMagicFx::SetHitType( int nAniType )
{
	m_nHitType = nAniType;
}