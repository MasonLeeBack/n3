#include "StdAfx.h"
#include "FcAIGlobalParam.h"
#include "FcParamVariable.h"
#include "FcAIElement.h"
#include "FcAIObject.h"
#include "FcGameObject.h"
#include "FcSOXLoader.h"
#include "FcAIGlobalVariableMng.h"
#include "FcTroopObject.h"
#include "FcGlobal.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG


CFcAIGlobalParam g_AIGlobalParam;
struct CFcAIGlobalParam::GlobalParamStruct CFcAIGlobalParam::s_GlobalParamList[] = {
	{ CFcParamVariable::INT, "_GetUnitAttackAmount", false },		// ANI_ATTR_BATTLE, ANI_TYPE_ATTACK Count
	{ CFcParamVariable::INT, "_GetUnitRunAttackAmount", false },	// ANI_ATTR_BATTLE, ANI_TYPE_RUN_ATTACK Count
	{ CFcParamVariable::INT, "_GetUnitBattleStandAmount", false },	// ANI_ATTR_BATTLE, ANI_TYPE_STAND Count
	{ CFcParamVariable::INT, "_GetUnitVictoryAmount", false },		// ANI_ATTR_NONE, ANI_TYPE_VICTORY Count
	{ CFcParamVariable::INT, "_GetUnitBattleRunAmount", false },	// ANI_ATTR_BATTLE, ANI_TYPE_RUN Count
	{ CFcParamVariable::INT, "_GetUnitHorseAttackAmount", false },	// ANI_ATTR_HORSE, ANI_TYPE_ATTACK Count
	{ CFcParamVariable::INT, "_GetUnitStandAmount", false },		// ANI_ATTR_STAND, ANI_TYPE_STAND Count
	{ CFcParamVariable::INT, "_GetUnitRunAmount", false },			// ANI_ATTR_NONE, ANI_TYPE_RUN Count
	{ CFcParamVariable::INT, "_GetUnitWalkAmount", false },		// ANI_ATTR_NONE, ANI_TYPE_WALK Count
	{ CFcParamVariable::INT, "_GetUnitBattleWalkAmount", false },	// ANI_ATTR_BATTLE, ANI_TYPE_WALK Count
	{ CFcParamVariable::INT, "_GetUnitNoneAttackAmount", false },	// ANI_ATTR_NONE, ANI_TYPE_ATTACK Count
	{ CFcParamVariable::INT, "_GetTroopRange", false },			// Troop Range
	{ CFcParamVariable::INT, "_GetUnitMoveAmount", false },		// MoveSpeed
	{ CFcParamVariable::INT, "_GetOptimumTargetingRange", true },	// HeroTargeting 에서 현제 타겟에 붙어있는 적들에 비례한 최적의 Range
	{ CFcParamVariable::INT, "_GetTroopRangeAttackRadius", true },		// 부대의 Range Attack 거리
	{ CFcParamVariable::INT, "_GetProcessTick", true },	// ProcessTick
	{ CFcParamVariable::INT, "_GetCurrentRage", true },	// 분노 수치
	{ CFcParamVariable::INT, "_GetUnitRadius", false },	// 유닛 사이즈
	{ CFcParamVariable::UNKNOWN, NULL },
};

CFcAIGlobalParam::CFcAIGlobalParam()
{
	DWORD i;
	for( i=0; ; i++ ) {
		if( s_GlobalParamList[i].szStr == NULL ) break;
	}
	m_dwCount = i;
}

CFcAIGlobalParam::~CFcAIGlobalParam()
{
	m_TempValue.Clear();
}

bool CFcAIGlobalParam::IsValidGlobalParam( const char *szStr )
{
	for( DWORD i=0; i<m_dwCount; i++ ) {
		if( strcmp( szStr, s_GlobalParamList[i].szStr ) == NULL ) return true;
	}
	return false;
}

DWORD CFcAIGlobalParam::GetGlobalParamCount()
{
	return m_dwCount;
}

const char *CFcAIGlobalParam::GetGlobalParamString( DWORD dwIndex )
{
	return s_GlobalParamList[dwIndex].szStr;
}

int CFcAIGlobalParam::GetIndex( const char *szStr )
{
	for( DWORD i=0; i<m_dwCount; i++ ) {
		if( strcmp( szStr, s_GlobalParamList[i].szStr ) == NULL ) {
			return (int)i;
		}
	}
	return -1;
}

CFcParamVariable *CFcAIGlobalParam::GetGlobalParamValue( CFcAIElement *pElement, CFcParamVariable *pParam )
{
	switch( pParam->GetType() ) {
		case CFcParamVariable::CUSTOM:	
			{
				void *pPtr = (void *)pParam->GetDescription();
				if( pPtr == NULL ) {
					int nIndex = GetIndex( pParam->GetVariableString() );
					pParam->SetDescription( "    " );

					pPtr = (void *)pParam->GetDescription();
					*(int*)pPtr = nIndex;
				}
				GetValue( pElement, *(int*)pPtr );
			}
			break;
		case CFcParamVariable::STRING:	
			{
				if( pParam->GetVariableString()[0] != '@' ) return pParam;
				void *pPtr = (void *)pParam->GetDescription();
				CFcAIGlobalVariableMng *pMng = pElement->GetParent()->GetGlobalVariableMng();
				if( pPtr == NULL ) {
					int nIndex = pMng->GetVariableIndex( pParam->GetVariableString() );
					pParam->SetDescription( "    " );

					pPtr = (void *)pParam->GetDescription();
					*(int*)pPtr = nIndex;
				}
				CFcParamVariable *pTempVariable = pMng->GetVariable( *(int*)pPtr );
				BsAssert( NULL != pTempVariable );	// mruete: prefix bug 547: added assert
				m_TempValue.Clear();
				switch( pTempVariable->GetType() ) {
					case CFcParamVariable::RANDOM:
						m_TempValue = *pTempVariable;
						m_TempValue.SetVariable( pTempVariable->GetGenRandom() );
						break;
					default:
						m_TempValue = *pTempVariable;
						break;
				}
			}
			break;
		default:	return pParam;
	}

	return &m_TempValue;
}

void CFcAIGlobalParam::GetValue( CFcAIElement *pElement, int nIndex )
{
	GameObjHandle hUnitHandle = pElement->GetUnitHandle();
	GameObjHandle hTargetHandle = pElement->GetTargetHandle();
	m_TempValue.Clear();
	switch( nIndex ) {
		case 0:
			m_TempValue.SetVariable( hUnitHandle->GetAniTypeCount( ANI_ATTR_BATTLE, ANI_TYPE_ATTACK ) );
			break;
		case 1:
			m_TempValue.SetVariable( hUnitHandle->GetAniTypeCount( ANI_ATTR_BATTLE, ANI_TYPE_RUN_ATTACK ) );
			break;
		case 2:
			m_TempValue.SetVariable( hUnitHandle->GetAniTypeCount( ANI_ATTR_BATTLE, ANI_TYPE_STAND ) );
			break;
		case 3:
			m_TempValue.SetVariable( hUnitHandle->GetAniTypeCount( ANI_ATTR_NONE, ANI_TYPE_VICTORY ) );
			break;
		case 4:
			m_TempValue.SetVariable( hUnitHandle->GetAniTypeCount( ANI_ATTR_BATTLE, ANI_TYPE_RUN ) );
			break;
		case 5:
			m_TempValue.SetVariable( hUnitHandle->GetAniTypeCount( ANI_ATTR_HORSE, ANI_TYPE_ATTACK ) );
			break;
		case 6:
			m_TempValue.SetVariable( hUnitHandle->GetAniTypeCount( ANI_ATTR_NONE, ANI_TYPE_STAND ) );
			break;
		case 7:
			m_TempValue.SetVariable( hUnitHandle->GetAniTypeCount( ANI_ATTR_NONE, ANI_TYPE_RUN ) );
			break;
		case 8:
			m_TempValue.SetVariable( hUnitHandle->GetAniTypeCount( ANI_ATTR_NONE, ANI_TYPE_WALK ) );
			break;
		case 9:
			m_TempValue.SetVariable( hUnitHandle->GetAniTypeCount( ANI_ATTR_BATTLE, ANI_TYPE_WALK ) );
			break;
		case 10:
			m_TempValue.SetVariable( hUnitHandle->GetAniTypeCount( ANI_ATTR_NONE, ANI_TYPE_ATTACK ) );
			break;
		case 11:
			m_TempValue.SetVariable( (int)hUnitHandle->GetParentTroop()->GetRadius() );
			break;
		case 12:
			m_TempValue.SetVariable( hUnitHandle->GetAniTypeCount( ANI_ATTR_BATTLE, ANI_TYPE_MOVE ) );
			break;
		case 13:
			{
				if( !CFcBaseObject::IsValid( hTargetHandle ) ) {
					m_TempValue.SetVariable( 0 );
					break;
				}
				int nUnitCount = 0;
				int nResultRange = 0;
				for( int i=0; i<hTargetHandle->GetTroop()->GetMeleeEngageTroopCount(); i++ ) {
					nUnitCount += hTargetHandle->GetTroop()->GetMeleeEngageTroop(i)->GetUnitCount();
				}
				nResultRange = 400 + ( nUnitCount * 4 );
				m_TempValue.SetVariable( nResultRange );
			}
			break;
		case 14:
			{
				CFcTroopObject *pTroop = hUnitHandle->GetTroop().GetPointer();
				if( pTroop == NULL ) {
					m_TempValue.SetVariable( (int)0 );
					break;
				}
//				int nResult = 0;
				int nResult = (int)((CFcRangeTroop*)pTroop)->GetAttackRadius();
				/*
				switch( pTroop->GetType() ) {
					case TROOPTYPE_RANGEMELEE:
						nResult = (int)((CFcRangeTroop*)pTroop)->GetAttackRadius();
						break;
					case TROOPTYPE_FLYING:
						nResult = (int)((CFcRangeTroop*)pTroop)->GetAttackRadius();
						break;
					default:
						break;
				}
				*/
				m_TempValue.SetVariable( nResult );
			}
			break;
		case 15:
			m_TempValue.SetVariable( GetProcessTick() );
			break;
		case 16:
			m_TempValue.SetVariable( hUnitHandle->GetRageDisposition() );
			break;
		case 17:
			m_TempValue.SetVariable( (int)hUnitHandle->GetUnitRadius() );
			break;

	}
}
