#include "StdAfx.h"
#include "FcAISearchGameObject.h"
#include "FcWorld.h"
#include "FcParamVariable.h"
#include "FcUtil.h"
#include "FcAIGlobalVariableMng.h"
#include "FcAIFunc.h"
#include "FcAIElement.h"
#include "FcTroopObject.h"
#include "FcAISearchSlot.h"
#include "FcAITriggerBase.h"
#include "FcCorrelation.h"
#include "PerfCheck.h"
#include "FcGlobal.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG

CFcAISearchGameObject::CFcAISearchGameObject( CFcAIObject *pParent )
: CFcAISearch( pParent )
{
	m_SearcherType = GAMEOBJECT;
	m_vLastSearchPos = D3DXVECTOR2( 0.f, 0.f );
}

CFcAISearchGameObject::~CFcAISearchGameObject()
{
}

bool CFcAISearchGameObject::CheckCondition()
{
	for( DWORD i=0; i<m_VecCon.size(); i++ ) {
		DWORD dwOffset = m_VecCon[i].dwParamOffset;
		switch( m_VecCon[i].Con ) {
			case AI_SEARCH_CON_IS_VALID_SLOT:
				{
					GameObjHandle *pHandle = m_pParent->GetSearchSlot()->GetHandle( m_pVecParam[dwOffset]->GetVariableInt() );
					if( !pHandle || !CFcBaseObject::IsValid( *pHandle ) ) return false;
				}
				break;
			case AI_SEARCH_CON_IS_INVALID_SLOT:
				{
					GameObjHandle *pHandle = m_pParent->GetSearchSlot()->GetHandle( m_pVecParam[dwOffset]->GetVariableInt() );
					if( pHandle ) return false;
					if( pHandle && CFcBaseObject::IsValid( *pHandle ) ) return false;
				}
				break;
			case AI_SEARCH_CON_GLOBAL_VARIABLE_INT:
				{
					CFcAIGlobalVariableMng *pMng = m_pParent->GetGlobalVariableMng();

					const char *szString = m_pVecParam[dwOffset]->GetVariableString();
					int nValueIndex = pMng->GetVariableIndex( szString );
					if( nValueIndex == -1 ) return false;
					CFcParamVariable *pGlobalParam = pMng->GetVariable( nValueIndex );
					BsAssert( NULL != pGlobalParam );	// mruete: prefix bug 555: added assert

					// 랜덤일때 비율 마춰주어야한다.
					// CFcAIObject 에서 TickInterval 에 따라 랜덤의 비율을 조정해주어야 한다.
					CFcParamVariable *pCurParam = NULL;
					if( pGlobalParam->GetType() == CFcParamVariable::RANDOM ) {
						static CFcParamVariable RandomValue( CFcParamVariable::RANDOM );
						RandomValue.SetVariable( pGlobalParam->GetVariableRandom()[0] / m_pParent->GetTickInterval(), 
							pGlobalParam->GetVariableRandom()[1] / m_pParent->GetTickInterval() );

						pCurParam = &RandomValue;
					}
					else pCurParam = pGlobalParam;

					bool bResult = true;
					int nOp = m_pVecParam[dwOffset+2]->GetVariableInt();
					switch( nOp ) {
						case AI_OP_EQUAL:					bResult = ( *pCurParam == *m_pVecParam[dwOffset+1] );	break;
						case AI_OP_NOT_EQUAL:				bResult = ( *pCurParam != *m_pVecParam[dwOffset+1] );	break;
						case AI_OP_GREATER:					bResult = ( *pCurParam > *m_pVecParam[dwOffset+1] );	break;
						case AI_OP_GREATER_THAN_OR_EQUAL:	bResult = ( *pCurParam >= *m_pVecParam[dwOffset+1] );	break;
						case AI_OP_LESS_THAN:				bResult = ( *pCurParam < *m_pVecParam[dwOffset+1] );	break;
						case AI_OP_LESS_THAN_OR_EQUAL:		bResult = ( *pCurParam <= *m_pVecParam[dwOffset+1] );	break;
						default:
							BsAssert(0);
							break;
					}
					if( !bResult ) return false;
				}
				break;
		}
	}
	return true;
}

bool CFcAISearchGameObject::Process( int nTick )
{
	nTick += m_nStandTick;
	if( nTick < m_nPrevSearchTick ) return false;
	if( nTick - m_nPrevSearchTick < m_nProcessTick ) return false;

	m_nPrevSearchTick = m_nPrevSearchTick + m_nProcessTick;

	PROFILE_TIME_TEST_BLOCK_START( "Process AI" );

	if( CheckCondition() == false ) return false;

	m_VecHandleList.clear();
	switch( m_Type ) {
		case AI_SEARCH_NONE:
			break;
		case AI_SEARCH_ALL:
			{
				D3DXVECTOR3 *pPos = (D3DXVECTOR3*)&m_pParent->GetUnitObjectHandle()->GetPos();
				CFcWorld::GetInstance().GetObjectListInRange( pPos, (float)m_pVecParam[0]->GetVariableInt(), m_VecHandleList );
			}
			break;
		case AI_SEARCH_ALL_TROOP_OFFSET:
			{
				if( m_pVecParam[0]->GetVariableInt() == -1 ) {
					TroopObjHandle hTroop = m_pParent->GetUnitObjectHandle()->GetParentTroop();

					GameObjHandle Handle;
					for( int i=0; i<hTroop->GetUnitCount(); i++ ) {
						Handle = hTroop->GetUnit(i);
						if( CFcBaseObject::IsValid( Handle ) ) m_VecHandleList.push_back( Handle );
					}
				}
				else {
					D3DXVECTOR2 vPos = m_pParent->GetUnitObjectHandle()->GetTroopOffset();
					D3DXVECTOR2 vTarget = m_pParent->GetUnitObjectHandle()->GetParentTroop()->GetPosV2();
					vTarget += vPos;

					D3DXVECTOR3 Pos( vTarget.x, 0.f, vTarget.y);

					CFcWorld::GetInstance().GetObjectListInRange( &Pos, (float)m_pVecParam[0]->GetVariableInt(), m_VecHandleList );
				}
			}
			break;
		case AI_SEARCH_GLOBAL_VARIABLE:
			{
				DWORD dwIndex = m_pParent->GetGlobalVariableMng()->GetVariableIndex( m_pVecParam[0]->GetVariableString() );
				CFcGameObject *pObject = (CFcGameObject *)m_pParent->GetGlobalVariableMng()->GetVariable( dwIndex )->GetVariablePtr();

				if( pObject && CFcBaseObject::IsValid( pObject->GetHandle() ) ) {
					m_VecHandleList.push_back( pObject->GetHandle() );
				}
			}
			break;
		case AI_SEARCH_SLOT:
			{
				/*
				int nTemp = m_pVecParam[0]->GetVariableInt();
				if( nTemp != 0 && nTemp != 1 ) {
					BsAssert(0);
				}
				*/
				GameObjHandle *pHandle = m_pParent->GetSearchSlot()->GetHandle( m_pVecParam[0]->GetVariableInt() );

				if( pHandle && CFcBaseObject::IsValid( *pHandle ) ) {
					m_VecHandleList.push_back( *pHandle );
				}
			}
			break;
		case AI_SEARCH_ALL_TARGET_TROOP_OFFSET:
			{
				// 임시로 Archer 일 경우엔 예전방식으로 얻어와서 pushback 한다.
				TroopObjHandle hTroop = m_pParent->GetUnitObjectHandle()->GetParentTroop();
				int nTroopStateTable = CFcTroopObject::s_nTroopStateTable[ hTroop->GetState() ];
				if( nTroopStateTable & UNIT_RANGE ) {
					TroopObjHandle Troop = m_pParent->GetUnitObjectHandle()->GetParentTroop()->GetTargetTroop();

					if( Troop ) {
						D3DXVECTOR3 vPos = Troop->GetPos();
						if( m_pVecParam[0]->GetVariableInt() == -1 ) {
							GameObjHandle Handle;
							for( int i=0; i<Troop->GetUnitCount(); i++ ) {
								Handle = Troop->GetUnit(i);
								if( CFcBaseObject::IsValid( Handle ) && !Handle->IsDie(true) ) m_VecHandleList.push_back( Handle );
							}
						}
						else {
							CFcWorld::GetInstance().GetObjectListInRange( &vPos, (float)m_pVecParam[0]->GetVariableInt(), m_VecHandleList );
						}
					}
				}
				else {
					for( int i=0; i<m_pParent->GetUnitObjectHandle()->GetParentTroop()->GetMeleeEngageTroopCount(); i++ ) {
						TroopObjHandle Troop = m_pParent->GetUnitObjectHandle()->GetParentTroop()->GetMeleeEngageTroop(i);

						if( Troop ) {
							if( m_pVecParam[0]->GetVariableInt() == -1 ) {
								GameObjHandle Handle;
								for( int i=0; i<Troop->GetUnitCount(); i++ ) {
									Handle = Troop->GetUnit(i);
									if( CFcBaseObject::IsValid( Handle ) && !Handle->IsDie(true) ) m_VecHandleList.push_back( Handle );
								}
							}
							else {
								D3DXVECTOR3 vPos = Troop->GetPos();
								CFcWorld::GetInstance().GetObjectListInRange( &vPos, (float)m_pVecParam[0]->GetVariableInt(), m_VecHandleList );
							}
						}
					}
				}

			}
			break;
		case AI_SEARCH_CORRELATION:
			{
				DWORD i=0;
				CFcCorrelation *pCorrelation = m_pParent->GetUnitObjectHandle()->GetCorrelation( (CFcCorrelation::CORRELATION_TYPE)m_pVecParam[0]->GetVariableInt() );
				switch( m_pVecParam[0]->GetVariableInt() ) {
					case CT_NONE:
						break;
					case CT_TARGETING_COUNTER:
						{
							std::vector<GameObjHandle> *pList = ((CFcCorrelationTargeting*)pCorrelation)->GetList();
							m_VecHandleList = *pList;
						}
						break;
					case CT_HIT:
						{
							GameObjHandle *pResult = (GameObjHandle *)((CFcCorrelationHit*)pCorrelation)->GetResult();
							if( pResult && CFcBaseObject::IsValid( *pResult ) )
								m_VecHandleList.push_back( *pResult );
						}
						break;
				}
			}
			break;
		case AI_SEARCH_ALL_TROOP:
			{
				std::vector<TroopObjHandle> vecList;
				TroopObjHandle hTroop = m_pParent->GetUnitObjectHandle()->GetParentTroop();
				float fRadius = (float)m_pVecParam[0]->GetVariableInt();
				if( (int)fRadius == -1 ) {
					fRadius = hTroop->GetAttackRadius();
				}

				g_FcWorld.GetTroopmanager()->GetListInRange( &hTroop->GetPosV2(), fRadius, vecList );

				GameObjHandle Handle;
				TroopObjHandle Troop;
				for( DWORD i=0; i<vecList.size(); i++ ) {
					Troop = vecList[i];
					if( Troop->GetState() == TROOPSTATE_ELIMINATED ) continue;
					if( Troop->GetTeam() == hTroop->GetTeam() ) {
						if( m_pVecParam[1]->GetVariableBool() == true ) continue;
					}
					else {
						if( m_pVecParam[1]->GetVariableBool() == false ) continue;
					}

					for( int i=0; i<Troop->GetUnitCount(); i++ ) {
						Handle = Troop->GetUnit(i);
						if( CFcBaseObject::IsValid( Handle ) && !Handle->IsDie(true) ) m_VecHandleList.push_back( Handle );
					}
				}
			}
			break;
		case AI_SEARCH_LINK_PARENT_TROOP:
			{
				TroopObjHandle hTroop = m_pParent->GetUnitObjectHandle()->GetParentTroop()->GetParentFollowTroop();
				if( hTroop ) {
					GameObjHandle Handle;
					for( int i=0; i<hTroop->GetUnitCount(); i++ ) {
						Handle = hTroop->GetUnit(i);
						if( CFcBaseObject::IsValid( Handle ) && !Handle->IsDie(true) ) m_VecHandleList.push_back( Handle ); 
					}
				}
			}
			break;
	}
	PROFILE_TIME_TEST_BLOCK_END();
	PROFILE_TIME_TEST( ProcessFilter() );

	return true;
}

static D3DXVECTOR2 g_sSearchFilter_Offset;
struct SearchFilter_MinLength_Sort {
	inline bool operator() (GameObjHandle &A, GameObjHandle &B) {
		return D3DXVec2LengthSq( (D3DXVECTOR2*)&( g_sSearchFilter_Offset - A->GetPosV2() ) ) <
			D3DXVec2LengthSq( (D3DXVECTOR2*)&( g_sSearchFilter_Offset - B->GetPosV2() ) );
	}
};

struct SearchFilter_MaxLength_Sort {
	inline bool operator() (GameObjHandle &A, GameObjHandle &B) {
		return D3DXVec2LengthSq( (D3DXVECTOR2*)&( g_sSearchFilter_Offset - A->GetPosV2() ) ) >
			D3DXVec2LengthSq( (D3DXVECTOR2*)&( g_sSearchFilter_Offset - B->GetPosV2() ) );
	}
};

struct SearchFilter_Troop_Offset_MinLength_Sort {
	inline bool operator() (GameObjHandle &A, GameObjHandle &B) {
		return D3DXVec2LengthSq( (D3DXVECTOR2*)&( g_sSearchFilter_Offset - ( A->GetTroopOffset() + A->GetParentTroop()->GetPosV2() ) ) ) <
			D3DXVec2LengthSq( (D3DXVECTOR2*)&( g_sSearchFilter_Offset - ( B->GetTroopOffset() + B->GetParentTroop()->GetPosV2() ) ) );
	}
};

struct SearchFilter_Troop_Offset_MaxLength_Sort {
	inline bool operator() (GameObjHandle &A, GameObjHandle &B) {
		return D3DXVec2LengthSq( (D3DXVECTOR2*)&( g_sSearchFilter_Offset - ( A->GetTroopOffset() + A->GetParentTroop()->GetPosV2() ) ) )>
			D3DXVec2LengthSq( (D3DXVECTOR2*)&( g_sSearchFilter_Offset - ( B->GetTroopOffset() + B->GetParentTroop()->GetPosV2() ) ) );
	}
};

/*
inline bool SearchFilter_MinLength_Sort( GameObjHandle &A, GameObjHandle &B )
{
if( D3DXVec2LengthSq( (D3DXVECTOR2*)&( g_sSearchFilter_Offset - A->GetPosV2() ) ) <
D3DXVec2LengthSq( (D3DXVECTOR2*)&( g_sSearchFilter_Offset - B->GetPosV2() ) ) ) return true;

return false;
}

inline bool SearchFilter_MaxLength_Sort( GameObjHandle &A, GameObjHandle &B )
{
if( D3DXVec2LengthSq( (D3DXVECTOR2*)&( g_sSearchFilter_Offset - A->GetPosV2() ) ) >
D3DXVec2LengthSq( (D3DXVECTOR2*)&( g_sSearchFilter_Offset - B->GetPosV2() ) ) ) return true;

return false;
}

inline bool SearchFilter_Troop_Offset_MinLength_Sort( GameObjHandle &A, GameObjHandle &B )
{
if( D3DXVec2LengthSq( (D3DXVECTOR2*)&( g_sSearchFilter_Offset - ( A->GetTroopOffset() + A->GetParentTroop()->GetPosV2() ) ) ) <
D3DXVec2LengthSq( (D3DXVECTOR2*)&( g_sSearchFilter_Offset - ( B->GetTroopOffset() + B->GetParentTroop()->GetPosV2() ) ) ) ) return true;

return false;
}

inline bool SearchFilter_Troop_Offset_MaxLength_Sort( GameObjHandle &A, GameObjHandle &B )
{
if( D3DXVec2LengthSq( (D3DXVECTOR2*)&( g_sSearchFilter_Offset - ( A->GetTroopOffset() + A->GetParentTroop()->GetPosV2() ) ) ) >
D3DXVec2LengthSq( (D3DXVECTOR2*)&( g_sSearchFilter_Offset - ( B->GetTroopOffset() + B->GetParentTroop()->GetPosV2() ) ) ) ) return true;

return false;
}
*/

void CFcAISearchGameObject::ProcessFilter()
{
	//	return;
	for( DWORD i=0; i<m_VecFilter.size(); i++ ) {
		DWORD dwOffset = m_VecFilter[i].dwParamOffset;
		switch( m_VecFilter[i].Filter ) {
			case AI_SEARCH_FILTER_NONE:
				break;
			case AI_SEARCH_FILTER_RANDOM:
				if( m_VecHandleList.size() > 1 ) {
					GameObjHandle Handle = m_VecHandleList[ Random( m_VecHandleList.size() ) ];
					m_VecHandleList.clear();
					m_VecHandleList.push_back( Handle );
				}
				break;
			case AI_SEARCH_FILTER_HERO:
				{
					std::vector<GameObjHandle> VecTemp;

					for( DWORD j=0; j<m_VecHandleList.size(); j++ ) {
						if( m_VecHandleList[j]->GetClassID() == CFcGameObject::Class_ID_Hero || 
							m_VecHandleList[j]->IsBoss() ) {
							VecTemp.push_back( m_VecHandleList[j] );
						}
					}
					m_VecHandleList.clear();
					m_VecHandleList = VecTemp;
				}
				break;
			case AI_SEARCH_FILTER_NEAREST:
				g_sSearchFilter_Offset = m_pParent->GetUnitObjectHandle()->GetPosV2();

#ifdef _XBOX
				std::sort( m_VecHandleList.begin(), m_VecHandleList.end(), SearchFilter_MinLength_Sort() );
#else
				std::sort( m_VecHandleList.begin(), m_VecHandleList.end(), SearchFilter_MinLength_Sort() );
#endif
				break;
			case AI_SEARCH_FILTER_FARTHEST:
				g_sSearchFilter_Offset = m_pParent->GetUnitObjectHandle()->GetPosV2();

#ifdef _XBOX
				std::sort( m_VecHandleList.begin(), m_VecHandleList.end(), SearchFilter_MaxLength_Sort() );
#else
				std::sort( m_VecHandleList.begin(), m_VecHandleList.end(), SearchFilter_MaxLength_Sort() );
#endif
				break;
			case AI_SEARCH_FILTER_TROOP_OFFSET_NEAREST:
				g_sSearchFilter_Offset = m_pParent->GetUnitObjectHandle()->GetTroopOffset() + m_pParent->GetUnitObjectHandle()->GetParentTroop()->GetPosV2();

#ifdef _XBOX
				std::sort( m_VecHandleList.begin(), m_VecHandleList.end(), SearchFilter_Troop_Offset_MinLength_Sort() );
#else
				std::sort( m_VecHandleList.begin(), m_VecHandleList.end(), SearchFilter_Troop_Offset_MinLength_Sort() );
#endif
				break;
			case AI_SEARCH_FILTER_TROOP_OFFSET_FARTHEST:
				g_sSearchFilter_Offset = m_pParent->GetUnitObjectHandle()->GetTroopOffset() + m_pParent->GetUnitObjectHandle()->GetParentTroop()->GetPosV2();

#ifdef _XBOX
				std::sort( m_VecHandleList.begin(), m_VecHandleList.end(), SearchFilter_Troop_Offset_MaxLength_Sort() );
#else
				std::sort( m_VecHandleList.begin(), m_VecHandleList.end(), SearchFilter_Troop_Offset_MaxLength_Sort() );
#endif
				break;
			case AI_SEARCH_FILTER_TARGETING_COUNT:
				{
					for( DWORD j=0; j<m_VecHandleList.size(); j++ ) {
						if( !CFcBaseObject::IsValid( m_VecHandleList[j] ) || m_VecHandleList[j]->GetCorrelationMng() == NULL ) continue;
						int nCount = *(int*)m_VecHandleList[j]->GetCorrelationMng()->GetResult( CFcCorrelation::CT_TARGETING_COUNTER );
						if( CFcAIConditionBase::Compare( nCount, m_pVecParam[dwOffset]->GetVariableInt(), m_pVecParam[dwOffset+1]->GetVariableInt() ) == false ) {
							m_VecHandleList.erase( m_VecHandleList.begin() + j );
							j--;
						}
					}
				}
				break;
			case AI_SEARCH_FILTER_LAST_HIT:
				{
					GameObjHandle *pHandle = (GameObjHandle *)m_pParent->GetUnitObjectHandle()->GetCorrelationMng()->GetResult( CFcCorrelation::CT_HIT );
					if( pHandle == NULL ) {
						m_VecHandleList.clear();
					}
					for( DWORD j=0; j<m_VecHandleList.size(); j++ ) {
						if( *pHandle != m_VecHandleList[j] ) {
							m_VecHandleList.erase( m_VecHandleList.begin() + j );
							j--;
						}
					}
				}
				break;
			case AI_SEARCH_FILTER_PREFERENCE_CLASS_ID:
				{
					int nOffset[3] = { 0, 0, 0 };
					int nClassID[3] = { m_pVecParam[dwOffset]->GetVariableInt(), m_pVecParam[dwOffset+1]->GetVariableInt(), m_pVecParam[dwOffset+2]->GetVariableInt() };
					GameObjHandle Handle;

					if( nClassID[0] == -1 ) break;

					int nCount = 3;
					if( nClassID[1] == -1 ) nCount = 1;
					else if( nClassID[2] == -1 ) nCount = 2;

					for( DWORD j=0; j<m_VecHandleList.size(); j++ ) {
						for( int n=0; n<nCount; n++ ) {
							if( m_VecHandleList[j]->GetClassID() == nClassID[n] ) {
								Handle = m_VecHandleList[j];

								m_VecHandleList.erase( m_VecHandleList.begin() + j );
								m_VecHandleList.insert( m_VecHandleList.begin() + nOffset[n], Handle );

								for( int k=n; k<nCount; k++ ) ++nOffset[k];
								break;
							}
						}
					}
				}
				break;
			case AI_SEARCH_FILTER_MAXIMUM_DAMAGE:
				break;
			case AI_SEARCH_FILTER_DIR:
				{
					D3DXVECTOR3 vVec;
					switch( m_pVecParam[dwOffset]->GetVariableInt() ) {
						case 0: // Left
							vVec = m_pParent->GetUnitObjectHandle()->GetCrossVector()->m_XVector;
							break;
						case 1:	// Right
							vVec = -m_pParent->GetUnitObjectHandle()->GetCrossVector()->m_XVector;
							break;
						case 2:	// Front
							vVec = m_pParent->GetUnitObjectHandle()->GetCrossVector()->m_ZVector;
							break;
						case 3:	// Back
							vVec = -m_pParent->GetUnitObjectHandle()->GetCrossVector()->m_ZVector;
							break;
					}
					D3DXVECTOR3 vPos = m_pParent->GetUnitObjectHandle()->GetPos();
					D3DXVECTOR3 vDot;
					float fDot;
					float fRadian = cos( (float)m_pVecParam[dwOffset+1]->GetVariableInt() * 3.1415926f / 180.f );

					for( DWORD j=0; j<m_VecHandleList.size(); j++ ) {
						GameObjHandle Handle = m_VecHandleList[j];
						vDot = Handle->GetDummyPos() - vPos;
						vDot.y = 0.f;
						D3DXVec3Normalize( &vDot, &vDot );
						fDot = D3DXVec3Dot( &vVec, &vDot );
						if( fDot > fRadian ) {
							m_VecHandleList.erase( m_VecHandleList.begin() + j );
							j--;
						}

					}
				}
				break;
		}
	}
}

int CFcAISearchGameObject::GetProcessCount()
{
	switch( m_Type ) {
		case AI_SEARCH_NONE:
			return 1;
		case AI_SEARCH_GET:
			{
				CFcAIElement *pElement = m_pParent->GetElement( m_pVecParam[0]->GetVariableInt() );
				return pElement->GetSearch()->GetProcessCount();
			}
			break;
		case AI_SEARCH_ALL:
		case AI_SEARCH_GLOBAL_VARIABLE:
		case AI_SEARCH_SLOT:
		case AI_SEARCH_ALL_TROOP_OFFSET:
		case AI_SEARCH_ALL_TARGET_TROOP_OFFSET:
		case AI_SEARCH_CORRELATION:
		case AI_SEARCH_ALL_TROOP:
		case AI_SEARCH_LINK_PARENT_TROOP:
			return m_VecHandleList.size();
		default:
			BsAssert(0);
			return 0;
	}
}


void *CFcAISearchGameObject::GetSearchProcess( int nIndex )
{
	static GameObjHandle DummyHandle;
	switch( m_Type ) {
		case AI_SEARCH_ALL:
		case AI_SEARCH_GLOBAL_VARIABLE:
		case AI_SEARCH_SLOT:
		case AI_SEARCH_ALL_TROOP_OFFSET:
		case AI_SEARCH_ALL_TARGET_TROOP_OFFSET:
		case AI_SEARCH_CORRELATION:
		case AI_SEARCH_ALL_TROOP:
		case AI_SEARCH_LINK_PARENT_TROOP:
			return (void*)&m_VecHandleList[nIndex];
			break;
		case AI_SEARCH_NONE:
			return (void*)&DummyHandle;
			break;
		case AI_SEARCH_GET:
			{
				CFcAIElement *pElement = m_pParent->GetElement( m_pVecParam[0]->GetVariableInt() );
				return pElement->GetSearch()->GetSearchProcess( nIndex );
			}			
			break;
		default:
			BsAssert(0);
	}
	return (void*)&DummyHandle;
}