#include "StdAfx.h"
#include "FcAdjutantObject.h"

#include "FcFXCommon.h"
#include "FcFxManager.h"

#include "FcWorld.h"
#include "./Data/SignalType.h"
#include "FcAIObject.h"
#include "FcAISearchSlot.h"
#include "FcAdjutantGoblinWizard.h"
#include "FcUtil.h"
#include "Data/FXList.h"
#include "FcProjectileTimeBomb.h"
#include "FcTroopObject.h"


CFcAdjutantGoblinWizard::CFcAdjutantGoblinWizard(CCrossVector *pCross)
: CFcAdjutantObject( pCross )
{
	SetAdjutantClassID( Class_ID_Adjutant_GoblinWizard );
}

CFcAdjutantGoblinWizard::~CFcAdjutantGoblinWizard()
{
}

void CFcAdjutantGoblinWizard::SignalCheck( ASSignalData *pSignal )
{
	switch( pSignal->m_nID ) {
		case SIGNAL_TYPE_SHOOT_PROJECTILE:
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

				CCrossVector Cross;
				PROJECTILE_PARAM Param;
				ProjetileHandle Handle;

				Param.hParent = m_Handle;
				Param.pSignal = pSignal;
				Param.fSize = 100.f + ( 30.f * (float)m_nLevel );

				Cross = *hUnit->GetCrossVector();
				Cross.RotateYaw( Random(1024) );
				Cross.MoveFrontBack( (float)Random( 200 ) );
				Cross.m_PosVector.y = g_FcWorld.GetLandHeight( Cross.m_PosVector.x, Cross.m_PosVector.z );

				Handle = CFcProjectile::CreateObject( CFcProjectile::TIMEBOMB, &Cross );
				((CFcProjectileTimeBomb*)Handle.GetPointer())->Initialize( FX_GBMADO01_2, FX_GBMADO01, -1, 1, 0, &Param );
				
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

				GetAIHandle()->GetSearchSlot()->SetHandle( 0, NULL );

				switch( pSignal->m_pParam[0] ) {
					case 0:		// »∏∫π
						{
							std::vector<GameObjHandle> vecList;
							float fRadius = 300.f + ( 100 * (float)m_nLevel );
							int nRecoveryRatio = 30;
							g_FcWorld.GetObjectListInRangeByTeam( &hUnit->GetCrossVector()->m_PosVector, fRadius, vecList, GetTeam() );

							for( DWORD i=0; i<vecList.size(); i++ ) {
								vecList[i]->BeginStatus( CFcStatusObject::STATUS_RECOVERY_HP, nRecoveryRatio );
							}
						}
						break;
					case 1:		// »•µ∑
						{
							std::vector<GameObjHandle> vecList;
							float fRadius = 300.f + ( 100 * (float)m_nLevel );
							int nTime = 200 + ( 200 * m_nLevel );
							g_FcWorld.GetObjectListInRange( &hUnit->GetCrossVector()->m_PosVector, fRadius, vecList );
							for( DWORD i=0; i<vecList.size(); i++ ) {
								if( vecList[i]->GetTeam() == GetTeam() ) continue;
								if( vecList[i]->GetClassID() == Class_ID_Hero ) continue;
								vecList[i]->BeginStatus( CFcStatusObject::STATUS_CHAOS, nTime );
							}
						}
						break;
					case 2:		// ¿Ãµøº”µµ
						{
							std::vector<GameObjHandle> vecList;
							float fRadius = 300.f + ( 75 * (float)m_nLevel );
							int nTime = 200 + ( 40 * m_nLevel );
							int nDownSpeedRatio = 75 - m_nLevel;
							if( nDownSpeedRatio < 10 ) nDownSpeedRatio = 10;

							g_FcWorld.GetObjectListInRange( &hUnit->GetCrossVector()->m_PosVector, fRadius, vecList );
							for( DWORD i=0; i<vecList.size(); i++ ) {
								if( vecList[i]->GetTeam() == GetTeam() ) continue;
								vecList[i]->BeginStatus( CFcStatusObject::STATUS_DOWN_SPEED, nTime, nDownSpeedRatio );
							}
						}
						break;
				}
			}
			break;
		default: 
			break;
	}
	CFcAdjutantObject::SignalCheck( pSignal );
}

