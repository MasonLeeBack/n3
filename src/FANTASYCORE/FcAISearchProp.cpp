#include "StdAfx.h"
#include "FcAISearchProp.h"
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
#include "FcPropManager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG

CFcAISearchProp::CFcAISearchProp( CFcAIObject *pParent )
: CFcAISearch( pParent )
{
	m_SearcherType = PROP;
}

CFcAISearchProp::~CFcAISearchProp()
{
}

bool CFcAISearchProp::CheckCondition()
{
	for( DWORD i=0; i<m_VecCon.size(); i++ ) {
		DWORD dwOffset = m_VecCon[i].dwParamOffset;
		switch( m_VecCon[i].Con ) {
			case AI_SEARCH_CON_IS_VALID_SLOT:
				{
					void *pPtr = m_pParent->GetSearchSlot()->GetPtr( m_pVecParam[dwOffset]->GetVariableInt() );
					if( !pPtr ) return false;
				}
				break;
			case AI_SEARCH_CON_IS_INVALID_SLOT:
				{
					void *pPtr = m_pParent->GetSearchSlot()->GetPtr( m_pVecParam[dwOffset]->GetVariableInt() );
					if( pPtr ) return false;
				}
				break;
			case AI_SEARCH_CON_GLOBAL_VARIABLE_INT:
				{
					CFcAIGlobalVariableMng *pMng = m_pParent->GetGlobalVariableMng();

					const char *szString = m_pVecParam[dwOffset]->GetVariableString();
					int nValueIndex = pMng->GetVariableIndex( szString );
					if( nValueIndex == -1 ) return false;
					CFcParamVariable *pGlobalParam = pMng->GetVariable( nValueIndex );
					BsAssert( NULL != pGlobalParam );	// mruete: prefix bug 557: added assert

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

bool CFcAISearchProp::Process( int nTick )
{
	nTick += m_nStandTick;
	if( nTick < m_nPrevSearchTick ) return false;
	if( nTick - m_nPrevSearchTick < m_nProcessTick ) return false;

	m_nPrevSearchTick = m_nPrevSearchTick + m_nProcessTick;
	m_pVecPropList.clear();

	if( CheckCondition() == false ) return false;
	switch( m_Type ) {
		case AI_SEARCH_NONE:
			break;
		case AI_SEARCH_ALL:
			{
				D3DXVECTOR3 *pPos = (D3DXVECTOR3*)&m_pParent->GetUnitObjectHandle()->GetPos();
				g_FcWorld.GetPropManager()->GetObjectListInRange( pPos, (float)m_pVecParam[0]->GetVariableInt(), m_pVecPropList );
			}
			break;
		case AI_SEARCH_ALL_TROOP_OFFSET:
			{
				float fRange = 0.f;
				if( m_pVecParam[0]->GetVariableInt() == -1 ) {
					TroopObjHandle hTroop = m_pParent->GetUnitObjectHandle()->GetParentTroop();
					fRange = hTroop->GetRadius();
				}
				else fRange = (float)m_pVecParam[0]->GetVariableInt();

				D3DXVECTOR2 vPos = m_pParent->GetUnitObjectHandle()->GetTroopOffset();
				D3DXVECTOR2 vTarget = m_pParent->GetUnitObjectHandle()->GetParentTroop()->GetPosV2();
				vTarget += vPos;

				D3DXVECTOR3 Pos( vTarget.x, 0.f, vTarget.y);

				g_FcWorld.GetPropManager()->GetObjectListInRange( &Pos, fRange, m_pVecPropList );
			}
			break;
		case AI_SEARCH_GLOBAL_VARIABLE:
			{
				DWORD dwIndex = m_pParent->GetGlobalVariableMng()->GetVariableIndex( m_pVecParam[0]->GetVariableString() );
				CFcProp *pObject = (CFcProp *)m_pParent->GetGlobalVariableMng()->GetVariable( dwIndex )->GetVariablePtr();

				if( pObject ) {
					m_pVecPropList.push_back( pObject );
				}
			}
			break;
		case AI_SEARCH_SLOT:
			{
				void *pPtr = m_pParent->GetSearchSlot()->GetPtr( m_pVecParam[0]->GetVariableInt() );

				if( pPtr ) {
					m_pVecPropList.push_back( (CFcProp*)pPtr );
				}
			}
			break;

		case AI_SEARCH_ALL_TARGET_TROOP_OFFSET:
			{
				// 임시로 Archer 일 경우엔 예전방식으로 얻어와서 pushback 한다.
				TroopObjHandle hTroop = m_pParent->GetUnitObjectHandle()->GetParentTroop();
				if( hTroop->GetState() == TROOPSTATE_RANGE_ATTACK ) {
					TroopObjHandle Troop = m_pParent->GetUnitObjectHandle()->GetParentTroop()->GetTargetTroop();

					if( Troop ) {
						D3DXVECTOR3 vPos = Troop->GetPos();
						float fRange = 0.f;
						if( m_pVecParam[0]->GetVariableInt() == -1 ) {
							fRange = Troop->GetRadius();
						}
						else fRange = (float)m_pVecParam[0]->GetVariableInt();
						g_FcWorld.GetPropManager()->GetObjectListInRange( &vPos, fRange, m_pVecPropList );
					}
				}
				else {
					for( int i=0; i<m_pParent->GetUnitObjectHandle()->GetParentTroop()->GetMeleeEngageTroopCount(); i++ ) {
						TroopObjHandle Troop = m_pParent->GetUnitObjectHandle()->GetParentTroop()->GetMeleeEngageTroop(i);

						if( Troop ) {
							float fRange = 0.f;
							if( m_pVecParam[0]->GetVariableInt() == -1 ) {
								fRange = Troop->GetRadius();
							}
							else fRange = (float)m_pVecParam[0]->GetVariableInt();

							D3DXVECTOR3 vPos = Troop->GetPos();
							g_FcWorld.GetPropManager()->GetObjectListInRange( &vPos, fRange, m_pVecPropList );
						}
					}
				}

			}
			break;
	}
	PROFILE_TIME_TEST( ProcessFilter() );

	return true;
}

static D3DXVECTOR2 g_sSearchFilter_Offset;
struct SearchFilter_MinLength_Sort {
	inline bool operator() (CFcProp *A, CFcProp *B) {
		return D3DXVec2LengthSq( (D3DXVECTOR2*)&( g_sSearchFilter_Offset - A->GetPosV2() ) ) <
			D3DXVec2LengthSq( (D3DXVECTOR2*)&( g_sSearchFilter_Offset - B->GetPosV2() ) );
	}
};

struct SearchFilter_MaxLength_Sort {
	inline bool operator() (CFcProp *A, CFcProp *B) {
		return D3DXVec2LengthSq( (D3DXVECTOR2*)&( g_sSearchFilter_Offset - A->GetPosV2() ) ) >
			D3DXVec2LengthSq( (D3DXVECTOR2*)&( g_sSearchFilter_Offset - B->GetPosV2() ) );
	}
};

void CFcAISearchProp::ProcessFilter()
{
	for( DWORD i=0; i<m_VecFilter.size(); i++ ) {
		DWORD dwOffset = m_VecFilter[i].dwParamOffset;
		switch( m_VecFilter[i].Filter ) {
			case AI_SEARCH_FILTER_NONE:
				break;
			case AI_SEARCH_FILTER_RANDOM:
				if( m_pVecPropList.size() > 1 ) {
					CFcProp *pProp = m_pVecPropList[ Random( m_pVecPropList.size() ) ];
					m_pVecPropList.clear();
					m_pVecPropList.push_back( pProp );
				}
				break;
			case AI_SEARCH_FILTER_NEAREST:
				g_sSearchFilter_Offset = m_pParent->GetUnitObjectHandle()->GetPosV2();

#ifdef _XBOX
				std::sort( m_pVecPropList.begin(), m_pVecPropList.end(), SearchFilter_MinLength_Sort() );
#else
				std::sort( &*m_pVecPropList.begin(), &*m_pVecPropList.end(), SearchFilter_MinLength_Sort() );
#endif
				break;
			case AI_SEARCH_FILTER_FARTHEST:
				g_sSearchFilter_Offset = m_pParent->GetUnitObjectHandle()->GetPosV2();

#ifdef _XBOX
				std::sort( m_pVecPropList.begin(), m_pVecPropList.end(), SearchFilter_MaxLength_Sort() );
#else
				std::sort( &*m_pVecPropList.begin(), &*m_pVecPropList.end(), SearchFilter_MaxLength_Sort() );
#endif
				break;
		}
	}
}

int CFcAISearchProp::GetProcessCount()
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
			return m_pVecPropList.size();
		default:
			BsAssert(0);
			return 0;
	}
}

void *CFcAISearchProp::GetSearchProcess( int nIndex )
{
	switch( m_Type ) {
		case AI_SEARCH_ALL:
		case AI_SEARCH_GLOBAL_VARIABLE:
		case AI_SEARCH_SLOT:
		case AI_SEARCH_ALL_TROOP_OFFSET:
		case AI_SEARCH_ALL_TARGET_TROOP_OFFSET:
			return (void*)m_pVecPropList[nIndex];
			break;
		case AI_SEARCH_NONE:
			return NULL;
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
	return NULL;
}