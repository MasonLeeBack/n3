#include "StdAfx.h"
#include "FcAdjutantMylarrvaryss.h"
#include "./Data/SignalType.h"
#include "FcProjectile.h"
#include "Data/FXList.h"
#include "ASData.h"
#include "FcAIObject.h"
#include "FcAISearchSlot.h"
#include "FcProjectileMagicFx.h"
#include "FcProjectileTimeBomb.h"
#include "FcTroopObject.h"
#include "FcWorld.h"
#include "FcVelocityCtrl.h"

CFcAdjutantMylarrvaryss::CFcAdjutantMylarrvaryss(CCrossVector *pCross)
: CFcAdjutantObject( pCross )
{
	SetAdjutantClassID( Class_ID_Adjutant_Mylarrvaryss );
}

CFcAdjutantMylarrvaryss::~CFcAdjutantMylarrvaryss()
{
}

void CFcAdjutantMylarrvaryss::SignalCheck( ASSignalData *pSignal )
{
	switch( pSignal->m_nID ) {
		case SIGNAL_TYPE_SHOOT_PROJECTILE:
			if( ( pSignal->m_nFrame >= m_fFrame ) && ( pSignal->m_nFrame < m_fFrame + GetFrameAdd() ) ) {
				switch( m_nCurAniTypeIndex ) {
					case 0:
						ProjectileFireball( pSignal );
						break;
					case 1:
						ProjectileAirBomb( pSignal );
						break;
				}
			}
			break;
		case SIGNAL_TYPE_CUSTOM_MESSAGE:
			if( ( pSignal->m_nFrame >= m_fFrame ) && ( pSignal->m_nFrame < m_fFrame + GetFrameAdd() ) ) {
				GameObjHandle hUnit;
				if( GetAIHandle() ) {
					GameObjHandle *pSearchHandle = GetAIHandle()->GetSearchSlot()->GetHandle(0);
					if( pSearchHandle )
						hUnit = *pSearchHandle;
					else break;
				}
				else break;
				if( !CFcBaseObject::IsValid( hUnit ) ) break;

				if( CFcTroopObject::s_nTroopStateTable[ m_hTroop->GetState() ] & UNIT_RANGE ) {
					GetAIHandle()->GetSearchSlot()->SetHandle( 0, NULL );
				}

				switch( pSignal->m_pParam[0] ) {
					case 0:		// 자연발화
						{
							std::vector<GameObjHandle> vecList;
							float fRadius = 800.f;
							g_FcWorld.GetEnemyObjectListInRange( GetTeam() ,&hUnit->GetCrossVector()->m_PosVector, fRadius, vecList );

							for( DWORD i=0; i<vecList.size(); i++ ) {
								vecList[i]->BeginStatus( CFcStatusObject::STATUS_IGNITION, pSignal->m_pParam[1], pSignal->m_pParam[2] );
							}
						}
						break;
					default:
						break;
				}
			}
			break;
		default:
			CFcAdjutantObject::SignalCheck( pSignal );
			break;
	}
}


void CFcAdjutantMylarrvaryss::ProjectileFireball( ASSignalData *pSignal )
{
	GameObjHandle hUnit;
	if( GetAIHandle() ) {
		GameObjHandle *pSearchHandle = GetAIHandle()->GetSearchSlot()->GetHandle(0);
		if( pSearchHandle )
			hUnit = *pSearchHandle;
		else return;
	}
	else return;
	if( !CFcBaseObject::IsValid( hUnit ) ) return;

	if( CFcTroopObject::s_nTroopStateTable[ m_hTroop->GetState() ] & UNIT_RANGE ) {
		GetAIHandle()->GetSearchSlot()->SetHandle( 0, NULL );
	}

	CCrossVector Cross;
	D3DXVECTOR3 Position;
	PROJECTILE_PARAM Param;
	ProjetileHandle Handle;

	Param.hParent = m_Handle;
	Param.pSignal = pSignal;
	Param.vTarget = hUnit->GetPos();
	Param.vTarget.y += 25.f;
	Param.fSize = 250.f;

	Cross = m_Cross;
	Position.x = ( float )pSignal->m_pParam[ 1 ];
	Position.y = ( float )pSignal->m_pParam[ 2 ];
	Position.z = ( float )pSignal->m_pParam[ 3 ];
	D3DXVec3TransformNormal( &Position, &Position, m_Cross );
	Cross.m_PosVector += Position;

	Handle = CFcProjectile::CreateObject( CFcProjectile::MAGIC_FX, &Cross );
	((CFcProjectileMagicFx*)Handle.GetPointer())->Initialize( FX_MIRA01FIREBALL, FX_MIRA01BOOM, &Param );
	((CFcProjectileMagicFx*)Handle.GetPointer())->SetDirectHideThrowFx( false );

	CFcVelocityCtrl *pCtrl = ((CFcProjectileMagicFx*)Handle.GetPointer())->GetVelocityCtrl();
	pCtrl->SetStartSpeed( 40.f );
	pCtrl->SetMaximumSpeed( 40.f );
	pCtrl->SetMinimumSpeed( 40.f );
}

void CFcAdjutantMylarrvaryss::ProjectileAirBomb( ASSignalData *pSignal )
{
	GameObjHandle hUnit;
	if( GetAIHandle() ) {
		GameObjHandle *pSearchHandle = GetAIHandle()->GetSearchSlot()->GetHandle(0);
		if( pSearchHandle )
			hUnit = *pSearchHandle;
		else return;
	}
	else return;
	if( !CFcBaseObject::IsValid( hUnit ) ) return;

	CCrossVector Cross;
	PROJECTILE_PARAM Param;
	ProjetileHandle Handle;

	Param.hParent = m_Handle;
	Param.pSignal = pSignal;
	Param.fSize = 400.f;

	Cross = *hUnit->GetCrossVector();
	Cross.m_PosVector.y += ( float )pSignal->m_pParam[ 2 ];

	Handle = CFcProjectile::CreateObject( CFcProjectile::TIMEBOMB, &Cross );
	((CFcProjectileTimeBomb*)Handle.GetPointer())->Initialize( FX_MIRA02AIRBOOM, FX_MIRA02_2AIRBOOM, -1, 1, 0, &Param );
}