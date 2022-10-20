#include "StdAfx.h"
#include "FcAIHardCodingFunctionAdditional.h"
#include "FcUtil.h"
#include "FcAIGlobalParam.h"
#include "FcGameObject.h"
#include "FcAIObject.h"
#include "FcAISearchGameObject.h"
#include "FcAISearchSlot.h"
#include "FcCorrelation.h"
#include "FcTroopObject.h"
#include "./Data/AIDef.h"
#include "FcHeroObject.h"
#include "PerfCheck.h"


extern int GetProcessTick();
#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG


const int DELAYORDER_MAX = 120;
const int DELAYORDER_ADD_RANGE = 120;













/*
 *	BattleMelleType1
 */
CFcAIHardCodingFunctionBattleMeleeType1::CFcAIHardCodingFunctionBattleMeleeType1()
{
	m_pSearch = NULL;
}

CFcAIHardCodingFunctionBattleMeleeType1::~CFcAIHardCodingFunctionBattleMeleeType1()
{
	SAFE_DELETE( m_pSearch );
}

void CFcAIHardCodingFunctionBattleMeleeType1::SetNullFuncParam( CFcParamVariable *pParam )
{
	m_pSearch->SetNullFuncParam( pParam );
}

void CFcAIHardCodingFunctionBattleMeleeType1::Initialize()
{
	m_nState = 0;

	m_pSearch = new CFcAISearchGameObject( m_pParent );
	m_pSearch->SetProcessTick( 40 );
	m_pSearch->SetType( AI_SEARCH_ALL_TROOP_OFFSET );

	m_pSearch->AddParam( (*m_pVecParam)[0] );

	m_pSearch->AddFilter( AI_SEARCH_FILTER_TARGETING_COUNT );
	m_pSearch->AddParam( (*m_pVecParam)[3] );
	m_pSearch->AddParamAlloc( CFcParamVariable( (int)AI_OP_LESS_THAN ) );

	m_pSearch->AddFilter( AI_SEARCH_FILTER_NEAREST );

	m_pSearch->AddCondition( AI_SEARCH_CON_IS_INVALID_SLOT );
	m_pSearch->AddParamAlloc( CFcParamVariable( (int)0 ) );

	m_pParent->CheckParentType( NULL );
}

void CFcAIHardCodingFunctionBattleMeleeType1::ExecuteFunction()
{
	GameObjHandle hUnit = m_pParent->GetUnitObjectHandle();
	CFcAISearchSlot *pSearchSlot = m_pParent->GetSearchSlot();

	int nTroopState = hUnit->GetParentTroop()->GetState();
	int nTroopStateTable = CFcTroopObject::s_nTroopStateTable[ nTroopState ];
	if( !( nTroopStateTable & UNIT_ATTACKABLE ) ) {
		GameObjHandle *pSlotHandle = pSearchSlot->GetHandle( 0 );
		if( pSlotHandle && CFcBaseObject::IsValid( (*pSlotHandle) ) ) {
			CFcCorrelationTargeting *pCorrelation = (CFcCorrelationTargeting *)(*pSlotHandle)->GetCorrelation( CFcCorrelation::CT_TARGETING_COUNTER );
			pCorrelation->InfluenceTargeting( &hUnit, false );
		}
		pSearchSlot->SetHandle( 0, NULL );
		return;
	}

	// Element 1
	if( m_pSearch->Process( CFcAIObject::s_dwLocalTick ) == true ) {
		bool bFlag = false;

		int nProcessCount = m_pSearch->GetProcessCount();
		if( nProcessCount != 0 ) {
			for( int j=0; j<nProcessCount; j++ ) {
				// mruete: prefx bug 549: added intermediate variable and assert
				GameObjHandle * pHandle = (GameObjHandle*)m_pSearch->GetSearchProcess( j );
				BsAssert( NULL != pHandle );
				m_hTarget = *pHandle;
				if( m_hTarget == hUnit ) continue;

				if( pSearchSlot->GetHandle(0) ) continue;
				if( m_hTarget->IsDie(true) ) continue;
				if( m_hTarget->GetTeam() == hUnit->GetTeam() ) continue;
				if( m_hTarget->GetTeam() == -1 ) continue;

				pSearchSlot->SetHandle( 0, &m_hTarget );

				CFcCorrelationTargeting *pCorrelation = (CFcCorrelationTargeting *)m_hTarget->GetCorrelation( CFcCorrelation::CT_TARGETING_COUNTER );
				BsAssert( NULL != pCorrelation );	// mruete: prefix bug 550: added assert
				pCorrelation->InfluenceTargeting( &hUnit, true );
				m_nState = 1;
				bFlag = true;
				break;
			}
		}
		if( bFlag == false ) m_hTarget.Identity();
	}

//	if( bFlag == false ) return;

	int nAniAttr = hUnit->GetCurAniAttr();
	int nAniType = hUnit->GetCurAniType() & 0x00FF;
	int nAniIndex = hUnit->GetCurAniIndex();

	// Element 2
	if( !hUnit->IsDie(true) && !pSearchSlot->GetHandle(0) && nAniAttr == ANI_ATTR_BATTLE && nAniType == ANI_TYPE_STAND ) 
	{
		hUnit->ChangeAnimation( ANI_TYPE_STAND, -1, ANI_ATTR_BATTLE );
	}

	GameObjHandle *phTarget = pSearchSlot->GetHandle(0);
	if( phTarget && CFcBaseObject::IsValid( *phTarget ) ) {
		m_hTarget = *phTarget;
		CFcCorrelationTargeting *pCorTargeting = (CFcCorrelationTargeting *)m_hTarget->GetCorrelation( CFcCorrelation::CT_TARGETING_COUNTER );
		// Element 3, 4
		if( hUnit->IsDie(true) || m_hTarget->IsDie(true) ) {
			pCorTargeting->InfluenceTargeting( &hUnit, false );
			pSearchSlot->SetHandle( 0, NULL );
			m_nState = 0;
		}
		// Element 5
		else {
			// CheckTroopOffset
			D3DXVECTOR2 vPos = hUnit->GetTroopOffset();
			D3DXVECTOR2 vTarget = hUnit->GetParentTroop()->GetPosV2();
			D3DXVECTOR2 vTempV2;

			vTarget += vPos;
			vTempV2 = vTarget - D3DXVECTOR2( hUnit->GetDummyPos().x, hUnit->GetDummyPos().z );

			float fDist = D3DXVec2LengthSq( &vTempV2 );

			int nLength = (*m_pVecParam)[0]->GetVariableInt() + 100;
			if( (int)fDist > nLength * nLength ) {
				hUnit->CmdMove( (int)vTarget.x, (int)vTarget.y, -1.f, ANI_TYPE_RUN, -1 );
				pCorTargeting->InfluenceTargeting( &hUnit, false );
				pSearchSlot->SetHandle( 0, NULL );
			}
			//////////////////////

			// Element 0, 1
			D3DXVECTOR3 vTemp = hUnit->GetPos() - m_hTarget->GetPos();
			vTemp.y = 0.f;
			float fCurLength = D3DXVec3Length( &vTemp ) - m_hTarget->GetUnitRadius();
			if( fCurLength > (float)(*m_pVecParam)[0]->GetVariableInt() ) {
				pCorTargeting->InfluenceTargeting( &hUnit, false );
				pSearchSlot->SetHandle( 0, NULL );
				m_nState = 0;
			}
			// Element 6
			else {
				if( fCurLength < (*m_pVecParam)[1]->GetVariableInt() && fCurLength > (*m_pVecParam)[2]->GetVariableInt() ) {
					m_nState = 2;
					// Element 9
					if( nAniType != ANI_TYPE_STAND && nAniType != ANI_TYPE_MOVE && fCurLength < (*m_pVecParam)[1]->GetVariableInt() - 20.f ) {
						hUnit->CmdStop( ANI_ATTR_BATTLE | ANI_TYPE_STAND, -1 );
					}
					// Element 11
					if( !IsDelay(0) && Random( 0, 2400 ) < (*m_pVecParam)[3]->GetVariableInt() && nAniType == ANI_TYPE_STAND ) {
						hUnit->ChangeAnimation( ANI_TYPE_ATTACK, -1, ANI_ATTR_BATTLE );
						SetDelay( 0, (*m_pVecParam)[4]->GetVariableInt() );
					}
				}
				else {
					m_nState = 1;
					// Element 7, 8
					float fTemp = (*m_pVecParam)[0]->GetVariableInt() / 2.f;
					if( nAniType == ANI_TYPE_STAND && fCurLength <= (*m_pVecParam)[2]->GetVariableInt() ) {
						int nMoveAmount = ::Random( hUnit->GetAniTypeCount( ANI_ATTR_BATTLE, ANI_TYPE_MOVE ) / 4 ) * 4;
						hUnit->ChangeAnimation( ANI_TYPE_MOVE, nMoveAmount + 1, ANI_ATTR_BATTLE, false );
					}
					else if( nAniType != ANI_TYPE_RUN && ( fCurLength > fTemp ) ) {
						D3DXVECTOR3 vVec, vPos;
						float fNearLength = (*m_pVecParam)[1]->GetVariableInt() - 20.f;
						if( fCurLength < fNearLength ) {
							hUnit->CmdStop( hUnit->GetCurAniAttr() | ANI_TYPE_STAND, -1 );
						}
						else {
							/*
							vPos = m_hTarget->GetDummyPos();
							vPos.y = 0.f;
							vVec = vTemp;

							D3DXVec3Normalize( &vVec, &vVec );

							vVec *= fNearLength;
							vPos -= vVec;
							hUnit->CmdMove( (int)vPos.x, (int)vPos.z, -1.0, ANI_ATTR_BATTLE | ANI_TYPE_RUN, -1 );
							*/
							hUnit->CmdMove( m_hTarget, fNearLength, -1.0, ANI_ATTR_BATTLE | ANI_TYPE_WALK, -1 );
						}
					}
					else if( nAniType == ANI_TYPE_STAND && ( fCurLength < fTemp ) ) {
						D3DXVECTOR3 vVec, vPos;
						float fNearLength = (*m_pVecParam)[1]->GetVariableInt() - 20.f;
						if( fCurLength < (*m_pVecParam)[1]->GetVariableInt() - 20.f ) {
							hUnit->CmdStop( hUnit->GetCurAniAttr() | ANI_TYPE_STAND, -1 );
						}
						else {
							/*
							vPos = m_hTarget->GetDummyPos();
							vPos.y = 0.f;
							vVec = vTemp;

							D3DXVec3Normalize( &vVec, &vVec );

							vVec *= (float)(*m_pVecParam)[1]->GetVariableInt();
							vPos -= vVec;
							hUnit->CmdMove( (int)vPos.x, (int)vPos.z, -1.0, ANI_ATTR_BATTLE | ANI_TYPE_WALK, -1 );
							*/
							hUnit->CmdMove( m_hTarget, fNearLength, -1.0, ANI_ATTR_BATTLE | ANI_TYPE_WALK, -1 );
						}
					}
				}
				// Element 10
				if( nAniType != ANI_TYPE_MOVE && nAniType != ANI_TYPE_WALK && nAniType != ANI_TYPE_RUN && nAniType != ANI_TYPE_STAND ) {
					pSearchSlot->SetLookAt( 0, false );
				}
				else pSearchSlot->SetLookAt( 0, true );

				// Element 12
				if( fCurLength <= (*m_pVecParam)[1]->GetVariableInt() + 100 && nAniAttr == ANI_ATTR_BATTLE && nAniType == ANI_TYPE_RUN && Random( 0, 2400 ) < (*m_pVecParam)[3]->GetVariableInt() * 4 ) {
					hUnit->ChangeAnimation( ANI_TYPE_RUN_ATTACK, -1, ANI_ATTR_BATTLE );
				}
			}
		}
	}
}

/*
*	BattleMeleeTroopType1
*/
CFcAIHardCodingFunctionBattleMeleeTroopType1::CFcAIHardCodingFunctionBattleMeleeTroopType1()
{
	m_pSearch = NULL;
}

CFcAIHardCodingFunctionBattleMeleeTroopType1::~CFcAIHardCodingFunctionBattleMeleeTroopType1()
{
	SAFE_DELETE( m_pSearch );
}

void CFcAIHardCodingFunctionBattleMeleeTroopType1::SetNullFuncParam( CFcParamVariable *pParam )
{
	m_pSearch->SetNullFuncParam( pParam );
}

void CFcAIHardCodingFunctionBattleMeleeTroopType1::Initialize()
{
	m_nState = 0;

	m_pSearch = new CFcAISearchGameObject( m_pParent );
	m_pSearch->SetProcessTick( 40 );
	m_pSearch->SetType( AI_SEARCH_ALL_TARGET_TROOP_OFFSET );

	m_pSearch->AddParamAlloc( CFcParamVariable( (int)-1 ) );

	m_pSearch->AddFilter( AI_SEARCH_FILTER_TARGETING_COUNT );
	m_pSearch->AddParam( (*m_pVecParam)[4] );
	m_pSearch->AddParamAlloc( CFcParamVariable( (int)AI_OP_LESS_THAN ) );

	m_pSearch->AddFilter( AI_SEARCH_FILTER_TROOP_OFFSET_NEAREST );

	m_pSearch->AddCondition( AI_SEARCH_CON_IS_INVALID_SLOT );
	m_pSearch->AddParamAlloc( CFcParamVariable( (int)0 ) );

	m_pParent->CheckParentType( NULL );
}

void CFcAIHardCodingFunctionBattleMeleeTroopType1::ExecuteFunction()
{
	GameObjHandle hUnit = m_pParent->GetUnitObjectHandle();
	CFcAISearchSlot *pSearchSlot = m_pParent->GetSearchSlot();

	int nTroopState = hUnit->GetParentTroop()->GetState();
	int nTroopStateTable = CFcTroopObject::s_nTroopStateTable[ nTroopState ];
	if( nTroopState == TROOPSTATE_MELEE_READY ) return;
	if( !( nTroopStateTable & UNIT_ATTACKABLE ) || hUnit->IsDie(true) ) {
		GameObjHandle *pSlotHandle = pSearchSlot->GetHandle( 0 );
		if( pSlotHandle && CFcBaseObject::IsValid( (*pSlotHandle ) ) ) {
			CFcCorrelationTargeting *pCorrelation = (CFcCorrelationTargeting *)(*pSlotHandle)->GetCorrelation( CFcCorrelation::CT_TARGETING_COUNTER );
			pCorrelation->InfluenceTargeting( &hUnit, false );
		}
		pSearchSlot->SetHandle( 0, NULL );
		return;
	}

	// Element 1
	if( m_pSearch->Process( CFcAIObject::s_dwLocalTick ) == true ) {
		bool bFlag = false;

		int nProcessCount = m_pSearch->GetProcessCount();
		if( nProcessCount != 0 ) {
			for( int j=0; j<nProcessCount; j++ ) {
				// mruete: prefx bug 551: added intermediate variable and assert
				GameObjHandle * pHandle = (GameObjHandle*)m_pSearch->GetSearchProcess( j );
				BsAssert( NULL != pHandle );
				m_hTarget = *pHandle;
				if( m_hTarget == hUnit ) continue;

				if( pSearchSlot->GetHandle(0) ) continue;
				if( m_hTarget->IsDie(true) ) continue;
				if( m_hTarget->GetTeam() == hUnit->GetTeam() ) continue;
				if( m_hTarget->GetTeam() == -1 ) continue;

				CFcCorrelationTargeting *pCorrelation = (CFcCorrelationTargeting *)m_hTarget->GetCorrelation( CFcCorrelation::CT_TARGETING_COUNTER );
				BsAssert( NULL != pCorrelation );	// mruete: prefix bug 552: added assert

				if( *(int*)pCorrelation->GetResult() >= (*m_pVecParam)[4]->GetVariableInt() ) continue;

				if( D3DXVec2LengthSq( (D3DXVECTOR2*)&(hUnit->GetParentTroop()->GetPosV2() - m_hTarget->GetDummyPosV2()) ) > hUnit->GetParentTroop()->GetRadius() * hUnit->GetParentTroop()->GetRadius() ) continue;

				pSearchSlot->SetHandle( 0, &m_hTarget );
				pCorrelation->InfluenceTargeting( &hUnit, true );
				hUnit->ResetMoveVector();
				m_nState = 1;
				bFlag = true;
				break;
			}
		}
		if( bFlag == false ) m_hTarget.Identity();
	}

	//	if( bFlag == false ) return;

	int nAniAttr = hUnit->GetCurAniAttr();
	int nAniType = hUnit->GetCurAniType() & 0x00FF;
	int nAniIndex = hUnit->GetCurAniIndex();

	// Element 2
	if( !hUnit->IsDie(true) && !pSearchSlot->GetHandle(0) && !pSearchSlot->GetHandle(1) ) {
		if( nAniAttr == ANI_ATTR_NONE && nAniType == ANI_TYPE_STAND ) {
			hUnit->ChangeAnimation( ANI_TYPE_STAND, -1, ANI_ATTR_BATTLE );
		}

		float fTroopOffset = D3DXVec2LengthSq( (D3DXVECTOR2*)&(hUnit->GetParentTroop()->GetPosV2() - hUnit->GetDummyPosV2()) );
		float fRadius = hUnit->GetParentTroop()->GetRadius();

		if( nAniType == ANI_TYPE_RUN || nAniType == ANI_TYPE_WALK || fTroopOffset > fRadius * fRadius ) {
			D3DXVECTOR2 vTarget = hUnit->GetParentTroop()->GetPosV2() + hUnit->GetTroopOffset();


			hUnit->CmdMove( (int)vTarget.x, (int)vTarget.y, -1.f, ANI_ATTR_BATTLE | ANI_TYPE_RUN, -1 );
		}
	}

	GameObjHandle *phTarget = pSearchSlot->GetHandle(0);
	if( phTarget && CFcBaseObject::IsValid( *phTarget ) ) {
		m_hTarget = *phTarget;
		CFcCorrelationTargeting *pCorTargeting = (CFcCorrelationTargeting *)m_hTarget->GetCorrelation( CFcCorrelation::CT_TARGETING_COUNTER );
		// Element 3, 4
		if( m_hTarget->IsDie(true) ) {
			pCorTargeting->InfluenceTargeting( &hUnit, false );
			pSearchSlot->SetHandle( 0, NULL );
			hUnit->CmdStop( ANI_ATTR_BATTLE | ANI_TYPE_STAND, -1 );
			m_nState = 0;
		}
		// Element 5
		else {
			// CheckTroopOffset
			float fTroopOffset = D3DXVec2LengthSq( (D3DXVECTOR2*)&(hUnit->GetParentTroop()->GetPosV2() - hUnit->GetDummyPosV2()) );
			float fRadius = hUnit->GetParentTroop()->GetRadius();
			if( fTroopOffset > fRadius * fRadius ) {
				D3DXVECTOR2 vTarget = hUnit->GetParentTroop()->GetPosV2() + hUnit->GetTroopOffset();

				hUnit->CmdMove( (int)vTarget.x, (int)vTarget.y, -1.f, ANI_ATTR_BATTLE | ANI_TYPE_RUN, -1 );

				pCorTargeting->InfluenceTargeting( &hUnit, false );
				pSearchSlot->SetHandle( 0, NULL );
//				DebugString( "나갔데~~, %.2f, %.2f, %s\n", fRadius, sqrtf( fTroopOffset ), hUnit->GetParentTroop()->GetName() );

				m_nState = 0;
			}
			//////////////////////

			// Element 0, 1
			D3DXVECTOR3 vTemp = hUnit->GetPos() - m_hTarget->GetPos();
			vTemp.y = 0.f;
			float fCurLength = D3DXVec3Length( &vTemp ) - m_hTarget->GetUnitRadius();

			fTroopOffset = D3DXVec2LengthSq( (D3DXVECTOR2*)&(hUnit->GetParentTroop()->GetPosV2() - m_hTarget->GetDummyPosV2()) );
			if( fTroopOffset > fRadius * fRadius ) {
				D3DXVECTOR2 vTarget = hUnit->GetParentTroop()->GetPosV2() + hUnit->GetTroopOffset();

				hUnit->CmdMove( (int)vTarget.x, (int)vTarget.y, -1.f, ANI_ATTR_BATTLE | ANI_TYPE_RUN, -1 );

				pCorTargeting->InfluenceTargeting( &hUnit, false );
				pSearchSlot->SetHandle( 0, NULL );

//				DebugString( "적이 나갔데~~, %.2f, %.2f, %s\n", fRadius, sqrtf( fTroopOffset ), hUnit->GetParentTroop()->GetName() );
				m_nState = 0;
			}

			// Element 6
			else {
				if( fCurLength < (*m_pVecParam)[1]->GetVariableInt() && fCurLength > (*m_pVecParam)[2]->GetVariableInt() ) {
					m_nState = 2;
					// Element 9
					if( nAniType != ANI_TYPE_STAND && nAniType != ANI_TYPE_MOVE && fCurLength < (*m_pVecParam)[1]->GetVariableInt() - 20.f ) {
						hUnit->CmdStop( ANI_ATTR_BATTLE | ANI_TYPE_STAND, -1 );
					}
				}
				else {
					m_nState = 1;
					// Element 7, 8
					float fTemp = (float)(*m_pVecParam)[0]->GetVariableInt();
					if( nAniType == ANI_TYPE_STAND && fCurLength <= (*m_pVecParam)[2]->GetVariableInt() ) {
						int nMoveAmount = ::Random( hUnit->GetAniTypeCount( ANI_ATTR_BATTLE, ANI_TYPE_MOVE ) / 4 ) * 4;
						hUnit->ChangeAnimation( ANI_TYPE_MOVE, nMoveAmount + 1, ANI_ATTR_BATTLE, false );
					}
					else if( nAniType != ANI_TYPE_RUN && ( fCurLength > fTemp ) ) {
						D3DXVECTOR3 vVec, vPos;
						float fNearLength = (*m_pVecParam)[1]->GetVariableInt() - 20.f;
						if( fCurLength <= fNearLength ) {
							hUnit->CmdStop( hUnit->GetCurAniAttr() | ANI_TYPE_STAND, -1 );
						}
						else {
							hUnit->CmdMove( m_hTarget, fNearLength, -1.0, ANI_ATTR_BATTLE | ANI_TYPE_RUN, -1 );
						}
					}
					else if( nAniType == ANI_TYPE_STAND && ( fCurLength < fTemp ) ) {
						D3DXVECTOR3 vVec, vPos;
						float fNearLength = (*m_pVecParam)[1]->GetVariableInt() - 20.f;
						if( fCurLength <= fNearLength ) {
							hUnit->CmdStop( hUnit->GetCurAniAttr() | ANI_TYPE_STAND, -1 );
						}
						else {
							hUnit->CmdMove( m_hTarget, fNearLength, -1.0, ANI_ATTR_BATTLE | ANI_TYPE_WALK, -1 );
						}
					}
				}
				// Element 10
				if( nAniType != ANI_TYPE_MOVE && nAniType != ANI_TYPE_WALK && nAniType != ANI_TYPE_RUN && nAniType != ANI_TYPE_STAND ) {
					pSearchSlot->SetLookAt( 0, false );
				}
				else {
					D3DXVec3Normalize( &vTemp, &vTemp );
					float fAngle = acos( D3DXVec3Dot( &hUnit->GetCrossVector()->m_ZVector, &(-vTemp) ) ) * 180.f / 3.1415926f;
					if( fAngle < (float)(*m_pVecParam)[3]->GetVariableInt() )
						pSearchSlot->SetLookAt( 0, false );
					else pSearchSlot->SetLookAt( 0, true );
				}
			}
		}
	}
}

/*
 *	BattleIdle
 */

CFcAIHardCodingFunctionBattleIdle::CFcAIHardCodingFunctionBattleIdle()
{
}

CFcAIHardCodingFunctionBattleIdle::~CFcAIHardCodingFunctionBattleIdle()
{
}

void CFcAIHardCodingFunctionBattleIdle::SetNullFuncParam( CFcParamVariable *pParam )
{
}

void CFcAIHardCodingFunctionBattleIdle::Initialize()
{
	m_pParent->CheckParentType( NULL );
}

void CFcAIHardCodingFunctionBattleIdle::ExecuteFunction()
{
	GameObjHandle hUnit = m_pParent->GetUnitObjectHandle();

	int nTroopState = hUnit->GetParentTroop()->GetState();
	int nTroopStateTable = CFcTroopObject::s_nTroopStateTable[ nTroopState ];
	if( !( nTroopStateTable & UNIT_ATTACKABLE ) ) return;
	if( hUnit->IsDie(true) ) return;

	CFcAISearchSlot *pSearchSlot = m_pParent->GetSearchSlot();
	GameObjHandle *phSlot[2];
	int nSlotIndex;
	phSlot[0] = pSearchSlot->GetHandle(0);
	phSlot[1] = pSearchSlot->GetHandle(1);

	if( ( phSlot[0] && CFcBaseObject::IsValid( *phSlot[0] ) ) && ( !phSlot[1] || !CFcBaseObject::IsValid( *phSlot[1] ) ) ) {
		nSlotIndex = 0;
	}
	else if( ( phSlot[1] && CFcBaseObject::IsValid( *phSlot[1] ) ) && ( !phSlot[0] || !CFcBaseObject::IsValid( *phSlot[0] ) ) ) {
		nSlotIndex = 1;
	}
	else {	// 이러면 안되지잉~~
		nSlotIndex = 0;
	}

	GameObjHandle *phTarget = phSlot[nSlotIndex];

	if( phTarget == NULL || !CFcBaseObject::IsValid( *phTarget ) ) {
		// Element 3
		if( Random( 0, 2400 ) < ( (*m_pVecParam)[0]->GetVariableInt() * 2 ) && ( hUnit->GetCurAniType() & 0x00FF ) == ANI_TYPE_STAND ) {
			hUnit->ChangeAnimation( ANI_TYPE_VICTORY, -1 );
		}
		return;
	}
	GameObjHandle hTarget = *phTarget;

	/*
	D3DXVECTOR3 vTemp = hUnit->GetDummyPos() - hTarget->GetDummyPos();
	vTemp.y = 0.f;
	float fCurLength = D3DXVec3Length( &vTemp );
	if( fCurLength >= (*m_pVecParam)[0]->GetVariableInt() ) return;
	*/

	int nAniAttr = hUnit->GetCurAniAttr();
	int nAniType = hUnit->GetCurAniType() & 0x00FF;
	int nAniIndex = hUnit->GetCurAniIndex();

	int nProb = (*m_pVecParam)[0]->GetVariableInt();

	int nLeftRightRandom, nUpDownRandom;
	switch( nSlotIndex ) {
		case 0:
			nLeftRightRandom = RandomNumberInRange( 1, 3 );
			nUpDownRandom = 1;
			break;
		case 1:
			nLeftRightRandom = RandomNumberInRange( 4, 5 );
			nUpDownRandom = RandomNumberInRange( 1, 2 );
			break;
	}

	int nMoveAmount = ::Random( hUnit->GetAniTypeCount( ANI_ATTR_BATTLE, ANI_TYPE_MOVE ) / 4 ) * 4;
	// 앞
	if( !IsDelay(0) && Random( 0, 2400 ) < nProb && nAniAttr == ANI_ATTR_BATTLE && nAniType == ANI_TYPE_STAND && nSlotIndex == 1 ) {
		hUnit->ChangeAnimation( ANI_TYPE_MOVE, nMoveAmount + 0, ANI_ATTR_BATTLE, false, nUpDownRandom );
		SetDelay( 0, (*m_pVecParam)[1]->GetVariableInt() );
	}
	// 뒤
	if( !IsDelay(1) && Random( 0, 2400 ) < nProb && nAniAttr == ANI_ATTR_BATTLE && nAniType == ANI_TYPE_STAND ) {
		hUnit->ChangeAnimation( ANI_TYPE_MOVE, nMoveAmount + 1, ANI_ATTR_BATTLE, false, nUpDownRandom );
		SetDelay( 1, (*m_pVecParam)[1]->GetVariableInt() );
	}

	// Element 1
	if( !IsDelay(2) && Random( 0, 2400 ) < nProb && nAniAttr == ANI_ATTR_BATTLE && nAniType == ANI_TYPE_STAND ) {
		hUnit->ChangeAnimation( ANI_TYPE_MOVE, nMoveAmount + 2, ANI_ATTR_BATTLE, false, nLeftRightRandom );
		SetDelay( 2, (*m_pVecParam)[1]->GetVariableInt() );
	}
	
	// Element 2
	if( !IsDelay(3) && Random( 0, 2400 ) < nProb && nAniAttr == ANI_ATTR_BATTLE && nAniType == ANI_TYPE_STAND ) {
		hUnit->ChangeAnimation( ANI_TYPE_MOVE, nMoveAmount + 3, ANI_ATTR_BATTLE, false, nLeftRightRandom );
		SetDelay( 3, (*m_pVecParam)[1]->GetVariableInt() );
	}
}

/*
 *	Correlation_LastHit
 */
CFcAIHardCodingFunctionCorrelationLastHit::CFcAIHardCodingFunctionCorrelationLastHit()
{
	m_pSearch = NULL;
}

CFcAIHardCodingFunctionCorrelationLastHit::~CFcAIHardCodingFunctionCorrelationLastHit()
{
	SAFE_DELETE( m_pSearch );
}

void CFcAIHardCodingFunctionCorrelationLastHit::SetNullFuncParam( CFcParamVariable *pParam )
{
	m_pSearch->SetNullFuncParam( pParam );
}

void CFcAIHardCodingFunctionCorrelationLastHit::Initialize()
{
	/*
	m_pSearch = new CFcAISearchGameObject( m_pParent );
	m_pSearch->SetProcessTick( 40 );
	m_pSearch->SetType( AI_SEARCH_CORRELATION );
	m_pSearch->AddParamAlloc( CFcParamVariable( (int)CFcCorrelation::CT_HIT ) );
	*/

	m_pSearch = new CFcAISearchGameObject( m_pParent );
//	m_pSearch->SetProcessTick( 40 );
	m_pSearch->SetType( AI_SEARCH_CORRELATION );
	m_pSearch->AddParamAlloc( CFcParamVariable( (int)CFcCorrelation::CT_HIT ) );

	m_pParent->CheckParentType( NULL );
}

void CFcAIHardCodingFunctionCorrelationLastHit::ExecuteCallbackFunction( int nIndex )
{
	GameObjHandle hUnit = m_pParent->GetUnitObjectHandle();

	int nTroopState = hUnit->GetParentTroop()->GetState();
	int nTroopStateTable = CFcTroopObject::s_nTroopStateTable[ nTroopState ];
	if( !( nTroopStateTable & UNIT_ATTACKABLE ) ) return;
	if( hUnit->IsDie(true) ) return;

	bool bSearchFlag = false;
	int nAniType;
	if( m_pSearch->Process( CFcAIObject::s_dwLocalTick ) == true ) {
		bool bFlag = false;
		int nProcessCount = m_pSearch->GetProcessCount();
		if( nProcessCount != 0 ) {
			for( int j=0; j<nProcessCount; j++ ) {
				// mruete: prefx bug 553: added intermediate variable and assert
				GameObjHandle * pHandle = (GameObjHandle*)m_pSearch->GetSearchProcess( j );
				BsAssert( NULL != pHandle );
				m_hTarget = *pHandle;
				if( m_hTarget == hUnit ) continue;

				if( m_hTarget->IsDie(true) ) continue;
				if( ::Random( 100 ) >= (*m_pVecParam)[0]->GetVariableInt() ) continue;

				nAniType = hUnit->GetCurAniType() & 0x00FF;
				if( D3DXVec2LengthSq( &( hUnit->GetPosV2() - m_hTarget->GetPosV2() ) ) > 640000.f /*800*800*/) continue;

				bFlag = true;
				break;
			}
		}
		if( bFlag == false ) m_hTarget.Identity();
		bSearchFlag = true;
	}
	else {
		assert(0);
	}

	if( !CFcBaseObject::IsValid( m_hTarget ) ) return;

	// Element 0
	if( bSearchFlag == false ) {
		if( m_hTarget->IsDie(true) ) return;
		if( ::Random( 100 ) >= (*m_pVecParam)[0]->GetVariableInt() ) return;
		nAniType = hUnit->GetCurAniType() & 0x00FF;
		if( D3DXVec2LengthSq( &( hUnit->GetPosV2() - m_hTarget->GetPosV2() ) ) > 640000.f /*800*800*/) return;
	}

	CFcAISearchSlot *pSearchSlot = m_pParent->GetSearchSlot();


	GameObjHandle *pSlotHandle = pSearchSlot->GetHandle( 0 );
	if( pSlotHandle && CFcBaseObject::IsValid( (*pSlotHandle) ) ) {
		CFcCorrelationTargeting *pCorrelation = (CFcCorrelationTargeting *)(*pSlotHandle)->GetCorrelation( CFcCorrelation::CT_TARGETING_COUNTER );
		pCorrelation->InfluenceTargeting( &hUnit, false );

	}

	pSearchSlot->SetHandle( 0, &m_hTarget );
	CFcCorrelationTargeting *pCorrelation = (CFcCorrelationTargeting *)m_hTarget->GetCorrelation( CFcCorrelation::CT_TARGETING_COUNTER );
	pCorrelation->InfluenceTargeting( &hUnit, true );
	hUnit->ResetMoveVector();
}

/*
 *	TroopCommand
 */

CFcAIHardCodingFunctionTroopCommand::CFcAIHardCodingFunctionTroopCommand()
{
}

CFcAIHardCodingFunctionTroopCommand::~CFcAIHardCodingFunctionTroopCommand()
{
}

void CFcAIHardCodingFunctionTroopCommand::SetNullFuncParam( CFcParamVariable *pParam )
{
}

void CFcAIHardCodingFunctionTroopCommand::Initialize()
{
	m_pParent->CheckParentType( NULL );
}



void CFcAIHardCodingFunctionTroopCommand::ExecuteFunction()
{
	GameObjHandle hUnit = m_pParent->GetUnitObjectHandle();

	int nTroopState = hUnit->GetParentTroop()->GetState();
	int nTroopStateTable = CFcTroopObject::s_nTroopStateTable[ nTroopState ];
	int nTroopSubState = hUnit->GetParentTroop()->GetSubState();

	CFcAISearchSlot *pSearchSlot = m_pParent->GetSearchSlot();
	GameObjHandle *pSlotHandle = pSearchSlot->GetHandle( 0 );
	if( nTroopState == TROOPSTATE_MELEE_READY && pSlotHandle && CFcBaseObject::IsValid( (*pSlotHandle) ) ) return;

	// Element 0, 1
	D3DXVECTOR2 vPos = hUnit->GetTroopOffset();
	D3DXVECTOR2 vTarget = hUnit->GetParentTroop()->GetPosV2();



	D3DXVECTOR2 vTemp;

	vTarget += vPos;
	vTemp = vTarget - D3DXVECTOR2( hUnit->GetDummyPos().x, hUnit->GetDummyPos().z );

	float fDist = D3DXVec2LengthSq( &vTemp );

	int nLength = (*m_pVecParam)[0]->GetVariableInt() + (int)hUnit->GetUnitRadius();
	if( (int)fDist <= nLength * nLength ) {
		// STand Element
		if( nTroopState == 0  && ( hUnit->GetCurAniType() & 0x00FF ) == ANI_TYPE_STAND ) {
			hUnit->ChangeAnimation( ANI_TYPE_STAND, -1, ANI_ATTR_NONE ); // nStandRandom 인덱스 랜덤하게 너줘야하는데.. 자꾸 바껴서..나중에 AniObject 내에서 컨트롤 할 수 있게 하자..
			/*
			if( hUnit->GetClassID() == Class_ID_Hero && ((CFcHeroObject*)hUnit.m_pInstance)->GetHeroClassID() == Class_ID_Hero_Inphy ) {
				DebugString( "4Cmd Move AI Coding~~~ %.2f, %.2f \n", vTarget.x, vTarget.y );
			}
			*/
		}
		return;
	}

	bool bMove = false;
	if( ( nTroopStateTable & UNIT_MOVABLE ) && !( nTroopStateTable & UNIT_ATTACKABLE ) ) bMove = true;
	if( nTroopState == 0 ) bMove = true;

	/*
	if( hUnit->GetClassID() == Class_ID_Hero && ((CFcHeroObject*)hUnit.m_pInstance)->GetHeroClassID() == Class_ID_Hero_Inphy ) {
		DebugString( "5Cmd Move AI Coding~~~ %.2f, %.2f \n", vTarget.x, vTarget.y );
	}
	*/
	if( bMove ) {
		//if(DelayOrder(DELAYORDER_MAX))
		//	return ;

		switch( nTroopSubState ) {
			case TROOPSUBSTATE_WALK:
				hUnit->CmdMove( (int)vTarget.x, (int)vTarget.y, -1.f, ANI_TYPE_WALK, -1 );
				break;
			case TROOPSUBSTATE_RUN:
				hUnit->CmdMove( (int)vTarget.x, (int)vTarget.y, -1.f, ANI_TYPE_RUN, -1 );
				break;
			case TROOPSUBSTATE_BATTLERUN:
				hUnit->CmdMove( (int)vTarget.x, (int)vTarget.y, -1.f, ANI_ATTR_BATTLE | ANI_TYPE_RUN, -1 );
				break;
		}
		/*
		nLength = (*m_pVecParam)[1]->GetVariableInt();
		if( fDist <= nLength * nLength )
			hUnit->CmdMove( (int)vTarget.x, (int)vTarget.y, -1.f, ANI_TYPE_WALK, -1 );
		else hUnit->CmdMove( (int)vTarget.x, (int)vTarget.y, -1.f, ANI_TYPE_RUN, -1 );
		*/
	}
}

bool CFcAIHardCodingFunctionTroopCommand::DelayOrder(int nMaxDelay) 
{
	if(IsDelay(0))
		return true;

	GameObjHandle hMe = m_pParent->GetUnitObjectHandle();
	if(hMe->IsMove() || hMe->IsEnabledMoveDelay()==false)
		return false;
	TroopObjHandle hTroop = hMe->GetTroop();
	
	D3DXVECTOR2 vDest = hTroop->GetTargetPosV2();
	float nearest, farthest, dis;
	D3DXVECTOR2 vDis = vDest - hMe->GetPosV2();
	nearest = farthest = dis = D3DXVec2Length( &vDis );	

	int nCnt = hTroop->GetUnitCount();
	for( int i=0; i<nCnt; ++i )
	{
		GameObjHandle hUnit = hTroop->GetUnit( i );
		D3DXVECTOR2 vDis = vDest - hUnit->GetPosV2();
		float len = D3DXVec2Length( &vDis );
		if(nearest > len)
			nearest = len;
		if(farthest < len)
			farthest = len;
	}

// [PREFIX:beginmodify] 2006/2/16 junyash PS#5194 reports dividing by zero using 'farthest-nearest'
	//SetDelay( 0, int( ((dis-nearest)/(farthest-nearest))*nMaxDelay) + RandomNumberInRange(0, DELAYORDER_ADD_RANGE) );
	// modify '(dis-nearest)/(farthest-nearest)' to 'if farthest-nearest == 0 then 0', because it seems '(dis-nearest) < (farthest-nearest)' and result is 0~1
	SetDelay( 0, ( (farthest-nearest) ? int( ((dis-nearest)/(farthest-nearest))*nMaxDelay) : 0 ) + RandomNumberInRange(0, DELAYORDER_ADD_RANGE) );
// [PREFIX:endmodify] junyash
	hMe->EnableMoveDelay(false);

    return true;
}
/*
 *	CheckTroopOffset
 */
/*
CFcAIHardCodingFunctionCheckTroopOffset::CFcAIHardCodingFunctionCheckTroopOffset()
{
}

CFcAIHardCodingFunctionCheckTroopOffset::~CFcAIHardCodingFunctionCheckTroopOffset()
{
}

void CFcAIHardCodingFunctionCheckTroopOffset::SetNullFuncParam( CFcParamVariable *pParam )
{
}

void CFcAIHardCodingFunctionCheckTroopOffset::Initialize()
{
	m_pParent->CheckParentType( NULL );
}

void CFcAIHardCodingFunctionCheckTroopOffset::ExecuteFunction()
{
	GameObjHandle hUnit = m_pParent->GetUnitObjectHandle();

	int nTroopState = hUnit->GetParentTroop()->GetState();
	int nTroopStateTable = CFcTroopObject::s_nTroopStateTable[ nTroopState ];

	// Element 0, 1
	D3DXVECTOR2 vPos = hUnit->GetTroopOffset();
	D3DXVECTOR2 vTarget = hUnit->GetParentTroop()->GetPosV2();
	D3DXVECTOR2 vTemp;

	vTarget += vPos;
	vTemp = vTarget - D3DXVECTOR2( hUnit->GetDummyPos().x, hUnit->GetDummyPos().z );

	float fDist = D3DXVec2LengthSq( &vTemp );

	int nLength = (*m_pVecParam)[0]->GetVariableInt();
	if( (int)fDist <= nLength * nLength ) return;

	hUnit->CmdMove( (int)vTarget.x, (int)vTarget.y, -1.f, ANI_TYPE_RUN, 0 );

	CFcAISearchSlot *pSearchSlot = m_pParent->GetSearchSlot();
	GameObjHandle *pSlotHandle = pSearchSlot->GetHandle( 0 );
	if( pSlotHandle ) {
		CFcCorrelationTargeting *pCorrelation = (CFcCorrelationTargeting *)(*pSlotHandle)->GetCorrelation( CFcCorrelation::CT_TARGETING_COUNTER );
		pCorrelation->InfluenceTargeting( &hUnit, false );
	}
	pSearchSlot->SetHandle( 0, NULL );
}

*/

/*
 *	BattleHeroTargeting
 */

CFcAIHardCodingFunctionBattleHeroTargeting::CFcAIHardCodingFunctionBattleHeroTargeting()
{
	m_pSearch = NULL;
	m_nRandomMaxRange = 0;
}

CFcAIHardCodingFunctionBattleHeroTargeting::~CFcAIHardCodingFunctionBattleHeroTargeting()
{
	SAFE_DELETE( m_pSearch );
}

void CFcAIHardCodingFunctionBattleHeroTargeting::SetNullFuncParam( CFcParamVariable *pParam )
{
	m_pSearch->SetNullFuncParam( pParam );
}

void CFcAIHardCodingFunctionBattleHeroTargeting::Initialize()
{
	m_nState = 0;

	m_pSearch = new CFcAISearchGameObject( m_pParent );
	m_pSearch->SetProcessTick( 80 );
	m_pSearch->SetType( AI_SEARCH_ALL_TARGET_TROOP_OFFSET );

	m_pSearch->AddParamAlloc( CFcParamVariable( (int)-1 ) );

	m_pSearch->AddFilter( AI_SEARCH_FILTER_HERO );

	m_pSearch->AddCondition( AI_SEARCH_CON_IS_INVALID_SLOT );
	m_pSearch->AddParamAlloc( CFcParamVariable( (int)1 ) );

	m_pParent->CheckParentType( NULL );
}

void CFcAIHardCodingFunctionBattleHeroTargeting::ExecuteFunction()
{
	GameObjHandle hUnit = m_pParent->GetUnitObjectHandle();
	CFcAISearchSlot *pSearchSlot = m_pParent->GetSearchSlot();

	int nTroopState = hUnit->GetParentTroop()->GetState();
	int nTroopStateTable = CFcTroopObject::s_nTroopStateTable[ nTroopState ];
	if( !( nTroopStateTable & UNIT_ATTACKABLE ) || pSearchSlot->GetHandle(0) || hUnit->IsDie(true) ) {
		pSearchSlot->SetHandle( 1, NULL );
		return;
	}

	if( D3DXVec2LengthSq( (D3DXVECTOR2*)&(hUnit->GetParentTroop()->GetPosV2() - hUnit->GetDummyPosV2()) ) > 
		hUnit->GetParentTroop()->GetRadius() * hUnit->GetParentTroop()->GetRadius() ) {
			pSearchSlot->SetHandle( 1, NULL );

			D3DXVECTOR2 vTarget = hUnit->GetParentTroop()->GetPosV2() + hUnit->GetTroopOffset();

			hUnit->CmdMove( (int)vTarget.x, (int)vTarget.y, -1.f, ANI_ATTR_BATTLE | ANI_TYPE_RUN, -1 );

			return;
		}


	if( m_pSearch->Process( CFcAIObject::s_dwLocalTick ) == true ) {
		bool bFlag = false;

		int nProcessCount = m_pSearch->GetProcessCount();
		if( nProcessCount != 0 ) {
			for( int j=0; j<nProcessCount; j++ ) {
				m_hTarget = *(GameObjHandle*)m_pSearch->GetSearchProcess( j );
				if( m_hTarget == hUnit ) continue;

				if( pSearchSlot->GetHandle(1) ) continue;
				if( m_hTarget->IsDie(true) ) continue;
				if( m_hTarget->GetTeam() == hUnit->GetTeam() ) continue;
				if( m_hTarget->GetTeam() == -1 ) continue;
				if( D3DXVec2LengthSq( (D3DXVECTOR2*)&(hUnit->GetParentTroop()->GetPosV2() - m_hTarget->GetDummyPosV2()) ) > hUnit->GetParentTroop()->GetRadius() * hUnit->GetParentTroop()->GetRadius() ) continue;

				pSearchSlot->SetHandle( 1, &m_hTarget );
				hUnit->CmdStop( ANI_ATTR_BATTLE | ANI_TYPE_STAND, -1 );

				m_nState = 1;
				bFlag = true;
				break;
			}
		}
		if( bFlag == false ) m_hTarget.Identity();
	}
	if( !m_hTarget ) return;

	GameObjHandle *phTarget = pSearchSlot->GetHandle(1);
	if( phTarget && CFcBaseObject::IsValid( *phTarget ) ) {
		m_hTarget = *phTarget;
		if( m_hTarget->IsDie(true) ) {
			pSearchSlot->SetHandle( 1, NULL );
			m_nState = 0;
		}
		else {
			int nAniAttr = hUnit->GetCurAniAttr();
			int nAniType = hUnit->GetCurAniType() & 0x00FF;
			int nAniIndex = hUnit->GetCurAniIndex();

			// CheckTroopOffset
			float fTroopOffset = D3DXVec2LengthSq( (D3DXVECTOR2*)&(hUnit->GetParentTroop()->GetPosV2() - hUnit->GetDummyPosV2()) );
			float fRadius = hUnit->GetParentTroop()->GetRadius();
			if( nAniType == ANI_TYPE_STAND && fTroopOffset > fRadius * fRadius ) {
				D3DXVECTOR2 vTarget = hUnit->GetParentTroop()->GetPosV2() + hUnit->GetTroopOffset();

				hUnit->CmdStop( ANI_ATTR_BATTLE | ANI_TYPE_STAND, -1 );
				hUnit->CmdMove( (int)vTarget.x, (int)vTarget.y, -1.f, ANI_ATTR_BATTLE | ANI_TYPE_RUN, -1 );

				pSearchSlot->SetHandle( 1, NULL );
				m_nState = 0;
			}
			//////////////////////

			D3DXVECTOR3 vTemp = hUnit->GetPos() - m_hTarget->GetPos();
			vTemp.y = 0.f;
			float fCurLength = D3DXVec3Length( &vTemp ) - m_hTarget->GetUnitRadius();

			fTroopOffset = D3DXVec2LengthSq( (D3DXVECTOR2*)&(hUnit->GetParentTroop()->GetPosV2() - m_hTarget->GetDummyPosV2()) );
			if( fTroopOffset > fRadius * fRadius ) {
				if( nAniType == ANI_TYPE_STAND ) {
					D3DXVECTOR2 vTarget = hUnit->GetParentTroop()->GetPosV2() + hUnit->GetTroopOffset();

					hUnit->CmdStop( ANI_ATTR_BATTLE | ANI_TYPE_STAND, -1 );
					hUnit->CmdMove( (int)vTarget.x, (int)vTarget.y, -1.f, ANI_ATTR_BATTLE | ANI_TYPE_RUN, -1 );

					pSearchSlot->SetHandle( 1, NULL );
					m_nState = 0;
				}
			}
			else {
				m_nState = 1;
				float fTemp = (float)(*m_pVecParam)[0]->GetVariableInt();

				// Targetin Range Calc GlobalParam 쪽이랑 가치 수정해야함..
				int nUnitCount = 0;
				int nOptimumTargetingRange = 0;
				for( int i=0; i<m_hTarget->GetTroop()->GetMeleeEngageTroopCount(); i++ ) {
					nUnitCount += m_hTarget->GetTroop()->GetMeleeEngageTroop(i)->GetUnitCount();
				}
				nOptimumTargetingRange = ( 400 + ( nUnitCount * 4 ) ) + (*m_pVecParam)[1]->GetVariableInt();
				////////////////////

				if( ( nAniType == ANI_TYPE_STAND || ( nAniType == ANI_TYPE_MOVE && ( nAniIndex % 4 ) != 1 ) ) && fCurLength <= (*m_pVecParam)[1]->GetVariableInt() ) {
					int nMoveAmount = ::Random( hUnit->GetAniTypeCount( ANI_ATTR_BATTLE, ANI_TYPE_MOVE ) / 4 ) * 4;
					hUnit->ChangeAnimation( ANI_TYPE_MOVE, nMoveAmount + 1, ANI_ATTR_BATTLE, false );
				}
				else if( nAniType != ANI_TYPE_RUN && ( fCurLength > fTemp ) && ( fCurLength > (float)nOptimumTargetingRange ) ) {
					/*
					D3DXVECTOR3 vVec, vPos;
					vPos = m_hTarget->GetDummyPos();
					vPos.y = 0.f;
					vVec = m_hTarget->GetPos() - hUnit->GetPos();
					vVec.y = 0.f;

					D3DXVec3Normalize( &vVec, &vVec );

					m_nRandomMaxRange = RandomNumberInRange( (*m_pVecParam)[1]->GetVariableInt(), nOptimumTargetingRange - 20 );
					vVec *= (float)m_nRandomMaxRange;
					vPos -= vVec;
					hUnit->CmdMove( (int)vPos.x, (int)vPos.z, -1.0, ANI_ATTR_BATTLE | ANI_TYPE_RUN, -1 );
					*/
					m_nRandomMaxRange = RandomNumberInRange( (*m_pVecParam)[1]->GetVariableInt(), nOptimumTargetingRange - 20 );
					hUnit->CmdMove( m_hTarget, (float)m_nRandomMaxRange, -1.0, ANI_ATTR_BATTLE | ANI_TYPE_RUN, -1 );
				}
				else if( nAniType == ANI_TYPE_STAND && ( fCurLength < fTemp ) && ( fCurLength > (float)nOptimumTargetingRange ) ) {
					/*
					D3DXVECTOR3 vVec, vPos;
					vPos = m_hTarget->GetDummyPos();
					vPos.y = 0.f;
					vVec = m_hTarget->GetPos() - hUnit->GetPos();
					vVec.y = 0.f;

					D3DXVec3Normalize( &vVec, &vVec );

					m_nRandomMaxRange = RandomNumberInRange( (*m_pVecParam)[1]->GetVariableInt(), nOptimumTargetingRange - 20 );
					vVec *= (float)m_nRandomMaxRange;
					vPos -= vVec;
					hUnit->CmdMove( (int)vPos.x, (int)vPos.z, -1.0, ANI_ATTR_BATTLE | ANI_TYPE_WALK, -1 );
					*/
					m_nRandomMaxRange = RandomNumberInRange( (*m_pVecParam)[1]->GetVariableInt(), nOptimumTargetingRange - 20 );
					hUnit->CmdMove( m_hTarget, (float)m_nRandomMaxRange, -1.0, ANI_ATTR_BATTLE | ANI_TYPE_WALK, -1 );
				}

				if( ( nAniType == ANI_TYPE_WALK || nAniType == ANI_TYPE_RUN ) && fCurLength < (float)m_nRandomMaxRange ) {
					hUnit->CmdStop( hUnit->GetCurAniAttr() | ANI_TYPE_STAND, -1 );
				}

				if( nAniType != ANI_TYPE_STAND && nAniType != ANI_TYPE_MOVE && nAniType != ANI_TYPE_WALK && fCurLength < (float)m_nRandomMaxRange ) {
					hUnit->CmdStop( ANI_ATTR_BATTLE | ANI_TYPE_STAND, -1 );
				}

				if( nAniType != ANI_TYPE_MOVE && nAniType != ANI_TYPE_WALK && nAniType != ANI_TYPE_RUN && nAniType != ANI_TYPE_STAND ) {
					pSearchSlot->SetLookAt( 1, false );
				}
				else pSearchSlot->SetLookAt( 1, true );
			}
		}
	}
}

/*
 *	BattleMeleeAttack
 */


CFcAIHardCodingFunctionBattleMeleeAttack::CFcAIHardCodingFunctionBattleMeleeAttack()
{
}

CFcAIHardCodingFunctionBattleMeleeAttack::~CFcAIHardCodingFunctionBattleMeleeAttack()
{
}

void CFcAIHardCodingFunctionBattleMeleeAttack::SetNullFuncParam( CFcParamVariable *pParam )
{
}

void CFcAIHardCodingFunctionBattleMeleeAttack::Initialize()
{
	m_pParent->CheckParentType( NULL );
}

void CFcAIHardCodingFunctionBattleMeleeAttack::ExecuteFunction()
{
	GameObjHandle hUnit = m_pParent->GetUnitObjectHandle();
	CFcAISearchSlot *pSearchSlot = m_pParent->GetSearchSlot();

	int nTroopState = hUnit->GetParentTroop()->GetState();
	int nTroopStateTable = CFcTroopObject::s_nTroopStateTable[ nTroopState ];

	if( !( nTroopStateTable & UNIT_ATTACKABLE ) ) return;

	GameObjHandle *pSlotHandle = pSearchSlot->GetHandle( (*m_pVecParam)[0]->GetVariableInt() );
	if( pSlotHandle == NULL ) return;
	if( !CFcBaseObject::IsValid( (*pSlotHandle) ) ) return;
	if( (*m_pVecParam)[1]->GetVariableInt() != -1 && (*m_pVecParam)[1]->GetVariableInt() != (*pSlotHandle)->GetClassID() ) return;
	if( (*pSlotHandle)->GetParentTroop()->IsUntouchable() ) return;

	if( !IsDelay(0) && Random( 0, 2400 ) < (int)( (*m_pVecParam)[7]->GetVariableInt() * hUnit->GetRageDisposition() ) ) {
		int nAniType = hUnit->GetCurAniType() & 0x00FF;
		int nTargetAniType = (*pSlotHandle)->GetCurAniType() & 0x00FF;
		D3DXVECTOR3 vTemp = hUnit->GetPos() - (*pSlotHandle)->GetPos();
		vTemp.y = 0.f;
		float fCurLength = D3DXVec3Length( &vTemp ) - (*pSlotHandle)->GetUnitRadius();

		if( nAniType == ANI_TYPE_STAND && nTargetAniType != ANI_TYPE_DOWN && nTargetAniType != ANI_TYPE_RISE && fCurLength < (*m_pVecParam)[2]->GetVariableInt() ) {
			int nAniIndex = RandomNumberInRange( (*m_pVecParam)[5]->GetVariableInt(), (*m_pVecParam)[6]->GetVariableInt() );
			hUnit->ChangeAnimation( (*m_pVecParam)[4]->GetVariableInt(), nAniIndex, (*m_pVecParam)[3]->GetVariableInt() );
			SetDelay( 0, (*m_pVecParam)[8]->GetVariableInt() );
		}
	}
}


/*
 *	BattleMeleeAttackCheckLink
 */


CFcAIHardCodingFunctionBattleMeleeAttackCheckLink::CFcAIHardCodingFunctionBattleMeleeAttackCheckLink()
{
}

CFcAIHardCodingFunctionBattleMeleeAttackCheckLink::~CFcAIHardCodingFunctionBattleMeleeAttackCheckLink()
{
}

void CFcAIHardCodingFunctionBattleMeleeAttackCheckLink::SetNullFuncParam( CFcParamVariable *pParam )
{
}

void CFcAIHardCodingFunctionBattleMeleeAttackCheckLink::Initialize()
{
	m_pParent->CheckParentType( NULL );
}

void CFcAIHardCodingFunctionBattleMeleeAttackCheckLink::ExecuteFunction()
{
	GameObjHandle hUnit = m_pParent->GetUnitObjectHandle();
	CFcAISearchSlot *pSearchSlot = m_pParent->GetSearchSlot();

	int nTroopState = hUnit->GetParentTroop()->GetState();
	int nTroopStateTable = CFcTroopObject::s_nTroopStateTable[ nTroopState ];

	if( !( nTroopStateTable & UNIT_ATTACKABLE ) ) return;

	GameObjHandle *pSlotHandle = pSearchSlot->GetHandle( (*m_pVecParam)[0]->GetVariableInt() );
	if( pSlotHandle == NULL ) return;
	if( !CFcBaseObject::IsValid( (*pSlotHandle) ) ) return;
	if( (*m_pVecParam)[1]->GetVariableInt() != -1 && !CFcBaseObject::IsValid( (*pSlotHandle)->GetParentLinkHandle() ) ) return;
	if( (*m_pVecParam)[1]->GetVariableInt() != -1 && (*m_pVecParam)[1]->GetVariableInt() != (*pSlotHandle)->GetParentLinkHandle()->GetClassID() ) return;
	if( (*pSlotHandle)->GetParentTroop()->IsUntouchable() ) return;

	if( !IsDelay(0) && Random( 0, 2400 ) < (int)( (*m_pVecParam)[7]->GetVariableInt() * hUnit->GetRageDisposition() ) ) {
		int nAniType = hUnit->GetCurAniType() & 0x00FF;
		int nTargetAniType = (*pSlotHandle)->GetCurAniType() & 0x00FF;
		D3DXVECTOR3 vTemp = hUnit->GetPos() - (*pSlotHandle)->GetPos();
		vTemp.y = 0.f;
		float fCurLength = D3DXVec3Length( &vTemp ) - (*pSlotHandle)->GetUnitRadius();

		if( nAniType == ANI_TYPE_STAND && nTargetAniType != ANI_TYPE_DOWN && nTargetAniType != ANI_TYPE_RISE && fCurLength < (*m_pVecParam)[2]->GetVariableInt() ) {
			int nAniIndex = RandomNumberInRange( (*m_pVecParam)[5]->GetVariableInt(), (*m_pVecParam)[6]->GetVariableInt() );
			hUnit->ChangeAnimation( (*m_pVecParam)[4]->GetVariableInt(), nAniIndex, (*m_pVecParam)[3]->GetVariableInt() );
			SetDelay( 0, (*m_pVecParam)[8]->GetVariableInt() );
		}
	}
}


/*
 *	BattleMeleeAttackFirstBattle
 */


CFcAIHardCodingFunctionBattleMeleeAttackFirstBattle::CFcAIHardCodingFunctionBattleMeleeAttackFirstBattle()
{
	m_bFirstFlag = false;
}

CFcAIHardCodingFunctionBattleMeleeAttackFirstBattle::~CFcAIHardCodingFunctionBattleMeleeAttackFirstBattle()
{
}

void CFcAIHardCodingFunctionBattleMeleeAttackFirstBattle::SetNullFuncParam( CFcParamVariable *pParam )
{
}

void CFcAIHardCodingFunctionBattleMeleeAttackFirstBattle::Initialize()
{
	m_pParent->CheckParentType( NULL );
}

void CFcAIHardCodingFunctionBattleMeleeAttackFirstBattle::ExecuteFunction()
{
	GameObjHandle hUnit = m_pParent->GetUnitObjectHandle();
	CFcAISearchSlot *pSearchSlot = m_pParent->GetSearchSlot();

	int nTroopState = hUnit->GetParentTroop()->GetState();
	int nTroopStateTable = CFcTroopObject::s_nTroopStateTable[ nTroopState ];

	if( !( nTroopStateTable & UNIT_ATTACKABLE ) && !(nTroopState == TROOPSTATE_MELEE_READY) ) {
		m_bFirstFlag = false;
		return;
	}

	GameObjHandle *pSlotHandle = pSearchSlot->GetHandle( (*m_pVecParam)[0]->GetVariableInt() );
	if( pSlotHandle == NULL ) {
		m_bFirstFlag = false;
		return;
	}
	if( !CFcBaseObject::IsValid( (*pSlotHandle) ) ) {
		m_bFirstFlag = false;
		return;
	}
	if( (*pSlotHandle)->GetParentTroop()->IsUntouchable() ) {
		m_bFirstFlag = false;
		return;
	}

	if( (*m_pVecParam)[1]->GetVariableInt() != -1 && (*m_pVecParam)[1]->GetVariableInt() != (*pSlotHandle)->GetClassID() ) return;

	if( m_bFirstFlag == false && Random( 0, 2400 ) < (int)( (*m_pVecParam)[7]->GetVariableInt() * hUnit->GetRageDisposition() ) ) {
		int nAniType = hUnit->GetCurAniType() & 0x00FF;
		int nTargetAniType = (*pSlotHandle)->GetCurAniType() & 0x00FF;
		D3DXVECTOR3 vTemp = hUnit->GetPos() - (*pSlotHandle)->GetPos();
		vTemp.y = 0.f;
		float fCurLength = D3DXVec3Length( &vTemp ) - (*pSlotHandle)->GetUnitRadius();

		if( nAniType == ANI_TYPE_STAND && nTargetAniType != ANI_TYPE_DOWN && nTargetAniType != ANI_TYPE_RISE && fCurLength < (*m_pVecParam)[2]->GetVariableInt() ) {
			int nAniIndex = RandomNumberInRange( (*m_pVecParam)[5]->GetVariableInt(), (*m_pVecParam)[6]->GetVariableInt() );
			hUnit->ChangeAnimation( (*m_pVecParam)[4]->GetVariableInt(), nAniIndex, (*m_pVecParam)[3]->GetVariableInt() );
			m_bFirstFlag = true;
		}
	}
}

CFcAIHardCodingFunctionBattleRangeType1::CFcAIHardCodingFunctionBattleRangeType1()
{
	m_pSearch = NULL;
}
CFcAIHardCodingFunctionBattleRangeType1::~CFcAIHardCodingFunctionBattleRangeType1()
{
	SAFE_DELETE( m_pSearch );
}

void CFcAIHardCodingFunctionBattleRangeType1::Initialize()
{
	m_nState = 0;

	m_pSearch = new CFcAISearchGameObject( m_pParent );
	m_pSearch->SetProcessTick( 40 );
	m_pSearch->SetType( AI_SEARCH_ALL_TARGET_TROOP_OFFSET );

	m_pSearch->AddParamAlloc( CFcParamVariable( (int)-1 ) );

	m_pSearch->AddFilter( AI_SEARCH_FILTER_RANDOM );

	m_pSearch->AddCondition( AI_SEARCH_CON_IS_INVALID_SLOT );
	m_pSearch->AddParamAlloc( CFcParamVariable( (int)0 ) );

	m_pParent->CheckParentType( NULL );
}

void CFcAIHardCodingFunctionBattleRangeType1::ExecuteFunction()
{
	GameObjHandle hUnit = m_pParent->GetUnitObjectHandle();
	CFcAISearchSlot *pSearchSlot = m_pParent->GetSearchSlot();

	int nTroopState = hUnit->GetParentTroop()->GetState();
	int nTroopStateTable = CFcTroopObject::s_nTroopStateTable[ nTroopState ];
	if( !( nTroopStateTable & UNIT_RANGE ) ) {
		GameObjHandle *pSlotHandle = pSearchSlot->GetHandle( 0 );
		pSearchSlot->SetHandle( 0, NULL );
		return;
	}

	// Element 1
	if( m_pSearch->Process( CFcAIObject::s_dwLocalTick ) == true ) {
		bool bFlag = false;

		int nProcessCount = m_pSearch->GetProcessCount();
		if( nProcessCount != 0 ) {
			for( int j=0; j<nProcessCount; j++ ) {
				m_hTarget = *(GameObjHandle*)m_pSearch->GetSearchProcess( j );
				if( m_hTarget == hUnit ) continue;

				if( pSearchSlot->GetHandle(0) ) continue;
				if( m_hTarget->IsDie(true) ) continue;
				if( m_hTarget->GetTeam() == hUnit->GetTeam() ) continue;
				if( m_hTarget->GetTeam() == -1 ) continue;

				pSearchSlot->SetHandle( 0, &m_hTarget );

				bFlag = true;
				break;
			}
		}
		if( bFlag == false ) m_hTarget.Identity();
	}

	//	if( bFlag == false ) return;

	int nAniAttr = hUnit->GetCurAniAttr();
	int nAniType = hUnit->GetCurAniType() & 0x00FF;
	int nAniIndex = hUnit->GetCurAniIndex();

	D3DXVECTOR2 vTroopPos = hUnit->GetTroopOffset() + hUnit->GetParentTroop()->GetPosV2();

	float fTroopOffset = D3DXVec2LengthSq( (D3DXVECTOR2*)&( vTroopPos - hUnit->GetDummyPosV2()) );

	if( hUnit->IsDie(true) ) {
		pSearchSlot->SetHandle( 0, NULL );
		return;
	}

	if( fTroopOffset < 200 * 200 ) 
	{
		if( nAniType != ANI_TYPE_STAND )
			hUnit->CmdStop( ANI_ATTR_NONE | ANI_TYPE_STAND, -1 );

		if( nAniAttr == ANI_ATTR_NONE && nAniType == ANI_TYPE_STAND ) {
			hUnit->ChangeAnimation( ANI_TYPE_CUSTOM_1, 0, ANI_ATTR_NONE );
			pSearchSlot->SetHandle( 0, NULL );
		}
		m_nState = 1;
	}
	else {
		D3DXVECTOR2 vTarget = hUnit->GetParentTroop()->GetPosV2() + hUnit->GetTroopOffset();

		hUnit->CmdMove( (int)vTarget.x, (int)vTarget.y, -1.f, ANI_TYPE_RUN, -1 );

		pSearchSlot->SetHandle( 0, NULL );
		m_nState = 0;
	}

	GameObjHandle *phTarget = pSearchSlot->GetHandle(0);
	if( phTarget && CFcBaseObject::IsValid( *phTarget ) ) {
		m_hTarget = *phTarget;
		if( m_hTarget->IsDie(true) ) {
			pSearchSlot->SetHandle( 0, NULL );
		}
		else {
			if( !m_hTarget->GetParentTroop()->IsUntouchable() && !IsDelay(0) && m_nState == 1 && nAniType == ANI_TYPE_STAND && Random( 0, 2400 ) < (*m_pVecParam)[0]->GetVariableInt() ) {
				D3DXVECTOR3 vVec = m_hTarget->GetPos() - hUnit->GetPos();
				D3DXVec3Normalize( &vVec, &vVec );
				float fAngle = acos( D3DXVec3Dot( &hUnit->GetCrossVector()->m_ZVector, &vVec ) ) * 180.f / 3.1415926f;
				if( fAngle < 45.f ) {
					hUnit->ChangeAnimation( ANI_TYPE_ATTACK, -1, ANI_ATTR_BATTLE );
					SetDelay( 0, (*m_pVecParam)[1]->GetVariableInt() );
				}
			}

			if( nAniType != ANI_TYPE_MOVE && nAniType != ANI_TYPE_WALK && nAniType != ANI_TYPE_RUN && nAniType != ANI_TYPE_STAND ) {
				pSearchSlot->SetLookAt( 0, false );
			}
			else pSearchSlot->SetLookAt( 0, true );

			if( m_nState == 0 ) pSearchSlot->SetLookAt( 0, false );
		}
	}
}

void CFcAIHardCodingFunctionBattleRangeType1::SetNullFuncParam( CFcParamVariable *pParam )
{
	m_pSearch->SetNullFuncParam( pParam );
}


/*
 *	BattleMeleeAttackCombo
 */


CFcAIHardCodingFunctionBattleMeleeAttackCombo::CFcAIHardCodingFunctionBattleMeleeAttackCombo()
{
}

CFcAIHardCodingFunctionBattleMeleeAttackCombo::~CFcAIHardCodingFunctionBattleMeleeAttackCombo()
{                                                                                                                 
}

void CFcAIHardCodingFunctionBattleMeleeAttackCombo::SetNullFuncParam( CFcParamVariable *pParam )
{
}

void CFcAIHardCodingFunctionBattleMeleeAttackCombo::Initialize()
{
	m_pParent->CheckParentType( NULL );
}

void CFcAIHardCodingFunctionBattleMeleeAttackCombo::ExecuteFunction()
{
	GameObjHandle hUnit = m_pParent->GetUnitObjectHandle();
	CFcAISearchSlot *pSearchSlot = m_pParent->GetSearchSlot();

	int nTroopState = hUnit->GetParentTroop()->GetState();
	int nTroopStateTable = CFcTroopObject::s_nTroopStateTable[ nTroopState ];

	if( !( nTroopStateTable & UNIT_ATTACKABLE ) ) return;

	GameObjHandle *pSlotHandle = pSearchSlot->GetHandle( (*m_pVecParam)[0]->GetVariableInt() );
	if( pSlotHandle == NULL ) return;
	if( !CFcBaseObject::IsValid( (*pSlotHandle) ) ) return;
	if( (*m_pVecParam)[1]->GetVariableInt() != -1 && (*m_pVecParam)[1]->GetVariableInt() != (*pSlotHandle)->GetClassID() ) return;
	if( (*pSlotHandle)->GetParentTroop()->IsUntouchable() ) return;

	if( !IsDelay(0) && Random( 0, 2400 ) < (int)( (*m_pVecParam)[5]->GetVariableInt() * hUnit->GetRageDisposition() ) ) {
		int nAniType = hUnit->GetCurAniType() & 0x00FF;
		int nTargetAniType = (*pSlotHandle)->GetCurAniType() & 0x00FF;
		D3DXVECTOR3 vTemp = hUnit->GetPos() - (*pSlotHandle)->GetPos();
		vTemp.y = 0.f;
		float fCurLength = D3DXVec3Length( &vTemp ) - (*pSlotHandle)->GetUnitRadius();

		if( nAniType == ANI_TYPE_STAND && nTargetAniType != ANI_TYPE_DOWN && nTargetAniType != ANI_TYPE_RISE && fCurLength < (*m_pVecParam)[2]->GetVariableInt() ) {
			int nAIIndex = RandomNumberInRange( (*m_pVecParam)[3]->GetVariableInt(), (*m_pVecParam)[4]->GetVariableInt() );
			hUnit->SetAIControlIndex( nAIIndex );
			SetDelay( 0, (*m_pVecParam)[6]->GetVariableInt() );
		}
	}
}

/*
*	CFcAIHardCodingFunctionCounterAttack
 */
CFcAIHardCodingFunctionCounterAttack::CFcAIHardCodingFunctionCounterAttack()
{
}

CFcAIHardCodingFunctionCounterAttack::~CFcAIHardCodingFunctionCounterAttack()
{
}

void CFcAIHardCodingFunctionCounterAttack::Initialize()
{
	m_pParent->CheckParentType( NULL );
}

void CFcAIHardCodingFunctionCounterAttack::ExecuteCallbackFunction( int nIndex )
{
	GameObjHandle hUnit = m_pParent->GetUnitObjectHandle();
	CFcAISearchSlot *pSearchSlot = m_pParent->GetSearchSlot();

	int nTroopState = hUnit->GetParentTroop()->GetState();
	int nTroopStateTable = CFcTroopObject::s_nTroopStateTable[ nTroopState ];

	if( !( nTroopStateTable & UNIT_ATTACKABLE ) ) return;

	GameObjHandle hTarget;
	GameObjHandle *pSlotHandle = pSearchSlot->GetHandle( (*m_pVecParam)[0]->GetVariableInt() );
	if( pSlotHandle == NULL ) return;
	if( !CFcBaseObject::IsValid( (*pSlotHandle) ) ) return;
	hTarget = *pSlotHandle;
	if( hTarget->IsDie(true) ) return;
	if( hTarget->GetParentTroop()->IsUntouchable() ) return;

	if( ( hTarget->GetCurAniType() & 0x00FF ) == ANI_TYPE_DOWN ) return;
	if( ( hUnit->GetCurAniType() & 0x00FF ) != ANI_TYPE_HIT ) return;
	int nRange = (*m_pVecParam)[1]->GetVariableInt();
	if( D3DXVec2LengthSq( &( hTarget->GetPosV2() - hUnit->GetPosV2() )  ) >= nRange * nRange ) return;
	if( !IsDelay(0) && ::Random( 100 ) < (*m_pVecParam)[6]->GetVariableInt() ) {
		hUnit->CmdLookUp( hTarget );
		hUnit->SetVelocity( &D3DXVECTOR3( 0.5f, 0.5f, 0.5f ) );
		int nAniIndex = RandomNumberInRange( (*m_pVecParam)[4]->GetVariableInt(), (*m_pVecParam)[5]->GetVariableInt() );
		hUnit->ChangeAnimation( (*m_pVecParam)[3]->GetVariableInt(), nAniIndex, (*m_pVecParam)[2]->GetVariableInt() );
		SetDelay( 0, (*m_pVecParam)[7]->GetVariableInt() );
	}
}

CFcAIHardCodingFunctionRageHit::CFcAIHardCodingFunctionRageHit()
{
	/*
	m_bDiscrease = false;
	m_nPrevDecreaseTick = -1;
	*/
}

CFcAIHardCodingFunctionRageHit::~CFcAIHardCodingFunctionRageHit()
{
}

void CFcAIHardCodingFunctionRageHit::Initialize()
{
	m_pParent->CheckParentType( NULL );
}

void CFcAIHardCodingFunctionRageHit::ExecuteFunction()
{
	GameObjHandle hUnit = m_pParent->GetUnitObjectHandle();
	if( !CFcBaseObject::IsValid( hUnit ) ) return;

	int nTroopState = hUnit->GetParentTroop()->GetState();
	int nTroopStateTable = CFcTroopObject::s_nTroopStateTable[ nTroopState ];

	if( !( nTroopStateTable & UNIT_ATTACKABLE ) && nTroopState != TROOPSTATE_MELEE_READY ) {
		hUnit->SetRageDisposition( 1.f );
		return;
	}

	/*
	if( m_bDiscrease == true ) {
		if( GetProcessTick() - m_nPrevDecreaseTick > (*m_pVecParam)[4]->GetVariableInt() ) {
			if( m_nPrevDecreaseTick == -1 ) m_nPrevDecreaseTick = GetProcessTick();
			else m_nPrevDecreaseTick += (*m_pVecParam)[4]->GetVariableInt();

			float fValue = hUnit->GetRageDisposition() - (*m_pVecParam)[3]->GetVariableFloat();
			if( fValue < (*m_pVecParam)[0]->GetVariableFloat() ) {
				fValue = (*m_pVecParam)[0]->GetVariableFloat();
				m_bDiscrease = false;
			}
			hUnit->SetRageDisposition( fValue );
		}
	}
	*/
}

void CFcAIHardCodingFunctionRageHit::ExecuteCallbackFunction( int nIndex )
{
	GameObjHandle hUnit = m_pParent->GetUnitObjectHandle();

	if( !CFcBaseObject::IsValid( hUnit ) ) return;

	switch( nIndex ) {
		case AI_CALLBACK_HIT:
			{
				float fValue = hUnit->GetRageDisposition() + (*m_pVecParam)[2]->GetVariableFloat();

				if( fValue > (*m_pVecParam)[1]->GetVariableFloat() ) {
					fValue = (*m_pVecParam)[1]->GetVariableFloat();
					/*
					m_bDiscrease = true;
					m_nPrevDecreaseTick = GetProcessTick();
					*/
				}
				hUnit->SetRageDisposition( fValue );
			}
			break;
		case AI_CALLBACK_ATTACK:
			{
				float fValue;

				if( hUnit->GetRageDisposition() >= (*m_pVecParam)[1]->GetVariableFloat() )
					fValue = (*m_pVecParam)[1]->GetVariableFloat() / 2.f;
				else fValue = hUnit->GetRageDisposition() - (*m_pVecParam)[3]->GetVariableFloat();

				if( fValue < (*m_pVecParam)[0]->GetVariableFloat() ) {
					fValue = (*m_pVecParam)[0]->GetVariableFloat();
//					m_bDiscrease = false;
				}
				hUnit->SetRageDisposition( fValue );
			}
			break;
	}
}


/*
*	CFcAIHardCodingFunctionRageTime
 */
CFcAIHardCodingFunctionRageTime::CFcAIHardCodingFunctionRageTime()
{
	/*
	m_bDiscrease = false;
	m_nPrevDecreaseTick = -1;
	*/
	m_nPrevIncreaseTick = -1;
}

CFcAIHardCodingFunctionRageTime::~CFcAIHardCodingFunctionRageTime()
{
}

void CFcAIHardCodingFunctionRageTime::Initialize()
{
	m_pParent->CheckParentType( NULL );
}

void CFcAIHardCodingFunctionRageTime::ExecuteFunction()
{
	GameObjHandle hUnit = m_pParent->GetUnitObjectHandle();
	if( !CFcBaseObject::IsValid( hUnit ) ) return;

	int nTroopState = hUnit->GetParentTroop()->GetState();
	int nTroopStateTable = CFcTroopObject::s_nTroopStateTable[ nTroopState ];

	if( !( nTroopStateTable & UNIT_ATTACKABLE ) && nTroopState != TROOPSTATE_MELEE_READY ) {
		hUnit->SetRageDisposition( 1.f );
		return;
	}

	/*
	if( m_bDiscrease == true ) {
		if( GetProcessTick() - m_nPrevDecreaseTick > (*m_pVecParam)[5]->GetVariableInt() ) {
			if( m_nPrevDecreaseTick == -1 ) m_nPrevDecreaseTick = GetProcessTick();
			else m_nPrevDecreaseTick += (*m_pVecParam)[5]->GetVariableInt();

			float fValue = hUnit->GetRageDisposition() - (*m_pVecParam)[3]->GetVariableFloat();
			if( fValue < (*m_pVecParam)[0]->GetVariableFloat() ) {
				fValue = (*m_pVecParam)[0]->GetVariableFloat();
				m_bDiscrease = false;
				m_nPrevIncreaseTick = GetProcessTick();
			}
			hUnit->SetRageDisposition( fValue );
		}
	}
	else {
	*/
	if( GetProcessTick() - m_nPrevIncreaseTick > (*m_pVecParam)[4]->GetVariableInt() ) {
		if( m_nPrevIncreaseTick == -1 ) m_nPrevIncreaseTick = GetProcessTick();
		else m_nPrevIncreaseTick += (*m_pVecParam)[4]->GetVariableInt();

		float fValue = hUnit->GetRageDisposition() + (*m_pVecParam)[2]->GetVariableFloat();
		if( fValue > (*m_pVecParam)[1]->GetVariableFloat() ) {
			fValue = (*m_pVecParam)[1]->GetVariableFloat();

//			m_bDiscrease = true;
//			m_nPrevDecreaseTick = GetProcessTick();
		}
		hUnit->SetRageDisposition( fValue );
	}
//	}
}

void CFcAIHardCodingFunctionRageTime::ExecuteCallbackFunction( int nIndex )
{
	GameObjHandle hUnit = m_pParent->GetUnitObjectHandle();

	if( !CFcBaseObject::IsValid( hUnit ) ) return;

	float fValue;

	if( hUnit->GetRageDisposition() >= (*m_pVecParam)[1]->GetVariableFloat() )
		fValue = (*m_pVecParam)[1]->GetVariableFloat() / 2.f;
	else fValue = hUnit->GetRageDisposition() - (*m_pVecParam)[3]->GetVariableFloat();

	if( fValue < (*m_pVecParam)[0]->GetVariableFloat() ) {
		fValue = (*m_pVecParam)[0]->GetVariableFloat();
		/*
		m_bDiscrease = false;
		m_nPrevIncreaseTick = GetProcessTick();
		*/
	}
	hUnit->SetRageDisposition( fValue );
}

/*
*	CFcAIHardCodingFunctionRageTroopHP
 */

CFcAIHardCodingFunctionRageTroopHP::CFcAIHardCodingFunctionRageTroopHP()
{
	/*
	m_bDiscrease = false;
	m_nPrevDecreaseTick = -1;
	*/
	m_nPrevTroopHPPercent = 100;
}

CFcAIHardCodingFunctionRageTroopHP::~CFcAIHardCodingFunctionRageTroopHP()
{
}

void CFcAIHardCodingFunctionRageTroopHP::Initialize()
{
	m_pParent->CheckParentType( NULL );

}

void CFcAIHardCodingFunctionRageTroopHP::ExecuteFunction()
{
	GameObjHandle hUnit = m_pParent->GetUnitObjectHandle();
	if( !CFcBaseObject::IsValid( hUnit ) ) return;

	TroopObjHandle hTroop = hUnit->GetTroop();
	int nTroopState = hTroop->GetState();
	int nTroopStateTable = CFcTroopObject::s_nTroopStateTable[ nTroopState ];

	if( !( nTroopStateTable & UNIT_ATTACKABLE ) && nTroopState != TROOPSTATE_MELEE_READY ) {
		hUnit->SetRageDisposition( 1.f );
		return;
	}

	/*
	if( m_bDiscrease == true ) {
		if( GetProcessTick() - m_nPrevDecreaseTick > (*m_pVecParam)[4]->GetVariableInt() ) {
			if( m_nPrevDecreaseTick == -1 ) m_nPrevDecreaseTick = GetProcessTick();
			else m_nPrevDecreaseTick += (*m_pVecParam)[4]->GetVariableInt();

			float fValue = hUnit->GetRageDisposition() - (*m_pVecParam)[3]->GetVariableFloat();
			if( fValue < (*m_pVecParam)[0]->GetVariableFloat() ) {
				fValue = (*m_pVecParam)[0]->GetVariableFloat();
				m_bDiscrease = false;
			}
			hUnit->SetRageDisposition( fValue );
		}
	}
	*/


	int nPercent = (int)( ( 100 / (float)hTroop->GetMaxHP() ) * (float)hTroop->GetHP() );
	if( m_nPrevTroopHPPercent - nPercent >= (*m_pVecParam)[4]->GetVariableInt() ) {
		m_nPrevTroopHPPercent -= (*m_pVecParam)[4]->GetVariableInt();

		float fValue = hUnit->GetRageDisposition() + (*m_pVecParam)[2]->GetVariableFloat();
		if( fValue > (*m_pVecParam)[1]->GetVariableFloat() ) {
			fValue = (*m_pVecParam)[1]->GetVariableFloat();
			/*
			m_bDiscrease = true;
			m_nPrevDecreaseTick = GetProcessTick();
			*/
		}
		hUnit->SetRageDisposition( fValue );
	}
}

void CFcAIHardCodingFunctionRageTroopHP::ExecuteCallbackFunction( int nIndex )
{
	GameObjHandle hUnit = m_pParent->GetUnitObjectHandle();

	if( !CFcBaseObject::IsValid( hUnit ) ) return;

	float fValue;
	if( hUnit->GetRageDisposition() >= (*m_pVecParam)[1]->GetVariableFloat() )
		fValue = (*m_pVecParam)[1]->GetVariableFloat() / 2.f;
	else fValue = hUnit->GetRageDisposition() - (*m_pVecParam)[3]->GetVariableFloat();

	if( fValue < (*m_pVecParam)[0]->GetVariableFloat() ) {
		fValue = (*m_pVecParam)[0]->GetVariableFloat();
	}
	hUnit->SetRageDisposition( fValue );
}

