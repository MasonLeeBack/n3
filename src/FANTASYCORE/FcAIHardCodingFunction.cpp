#include "StdAfx.h"
#include "FcAIHardCodingFunction.h"
#include "FcAIHardCodingFunctionAdditional.h"
#include "FCCommon.h"
#include "BsCommon.h"
#include "FcAIObject.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG

std::vector<CFcAIHardCodingFunction *> CFcAIHardCodingFunction::s_pVecFunction;
DWORD CFcAIHardCodingFunction::s_dwDefineOffset = 0;

#define REGISTER_HARDCODING_CLASS( function_name, class_name, ParamCount )	\
	{								\
		CFcAIHardCodingFunction *pFunction = new class_name;	\
		pFunction->SetFunctionName( function_name );			\
		pFunction->m_dwParamCount = ParamCount;					\
		CFcAIHardCodingFunction::s_pVecFunction.push_back( pFunction );	\
		++CFcAIHardCodingFunction::s_dwDefineOffset;	\
	}


CFcAIHardCodingFunction::CFcAIHardCodingFunction()
{
	m_szFunctionName = NULL;
	m_dwPrevTick = 0;
	m_nLevel = -1;
}

CFcAIHardCodingFunction::~CFcAIHardCodingFunction()
{
	SAFE_DELETEA( m_szFunctionName );
}


void CFcAIHardCodingFunction::CreateObject()
{
	REGISTER_HARDCODING_CLASS( "BattleMelee_Type1", CFcAIHardCodingFunctionBattleMeleeType1, 6 );
	REGISTER_HARDCODING_CLASS( "BattleMeleeTroop_Type1", CFcAIHardCodingFunctionBattleMeleeTroopType1, 5 );
	REGISTER_HARDCODING_CLASS( "BattleIdle", CFcAIHardCodingFunctionBattleIdle, 2 );
	REGISTER_HARDCODING_CLASS( "BattleHeroTargeting", CFcAIHardCodingFunctionBattleHeroTargeting, 2 );
	REGISTER_HARDCODING_CLASS( "Correlation_LastHit", CFcAIHardCodingFunctionCorrelationLastHit, 1 );
	REGISTER_HARDCODING_CLASS( "TroopCommand", CFcAIHardCodingFunctionTroopCommand, 1 );
	REGISTER_HARDCODING_CLASS( "BattleMeleeAttack", CFcAIHardCodingFunctionBattleMeleeAttack, 9 );
	REGISTER_HARDCODING_CLASS( "BattleMeleeAttackCheckLink", CFcAIHardCodingFunctionBattleMeleeAttackCheckLink, 9 );
	REGISTER_HARDCODING_CLASS( "BattleMeleeAttackFirstBattle", CFcAIHardCodingFunctionBattleMeleeAttackFirstBattle, 8 );
	REGISTER_HARDCODING_CLASS( "BattleMeleeAttackCombo", CFcAIHardCodingFunctionBattleMeleeAttackCombo, 7 );
	REGISTER_HARDCODING_CLASS( "BattleRange_Type1", CFcAIHardCodingFunctionBattleRangeType1, 2 );
	REGISTER_HARDCODING_CLASS( "CounterAttack", CFcAIHardCodingFunctionCounterAttack, 8 );

	// 스크립트 없고 하드코딩만 있는 함수덜~

	// 한대 터질 때마다 일정량의 분노수치 증가하는 함수 ( 공격시, 최대값이 되면 줄어든다 )
	// [ 최소값(float), 최대값(float), 증가치(float), 감소치(float) ]
	REGISTER_HARDCODING_CLASS( "RageHit", CFcAIHardCodingFunctionRageHit, 4 ); 

	// 일정 시간이 지날때마다 분노수치 증가하는 함수 ( 공격시, 최대값이 되면 줄어든다 )
	// [ 최소값(float), 최대값(float), 증가치(float), 감소치(float), 증가시간(int) ]
	REGISTER_HARDCODING_CLASS( "RageTime", CFcAIHardCodingFunctionRageTime, 5 ); 

	// 부대의 HP 가 일정량 줄때마다 증가 ( 공격시 줄어든다 )
	// [ 최소값(float), 최대값(float), 증가치(float), 감소치(float), 부대HP의 백분율(int 0~100) ]
	REGISTER_HARDCODING_CLASS( "RageTroopHP", CFcAIHardCodingFunctionRageTroopHP, 5 ); 
}

void CFcAIHardCodingFunction::ReleaseObject( bool bExitGame )
{
	if( bExitGame == true ) {
		SAFE_DELETE_PVEC( s_pVecFunction );
	}
	else {
		for( DWORD i=s_dwDefineOffset; i<s_pVecFunction.size(); i++ ) {
			SAFE_DELETE( s_pVecFunction[i] );
			s_pVecFunction.erase( s_pVecFunction.begin() + i );
			i--;
		}
	}
}

int CFcAIHardCodingFunction::FindEmptySlot()
{
	for( DWORD i=s_dwDefineOffset; i<s_pVecFunction.size(); i++ ) {
		if( s_pVecFunction[i] == NULL ) return i;
	}
	return -1;
}

int CFcAIHardCodingFunction::FindFunction( const char *szFunctionName, int nLevel )
{
	for( DWORD i=0; i<s_dwDefineOffset; i++ ) {
		if( strcmp( szFunctionName, s_pVecFunction[i]->m_szFunctionName ) == NULL ) {
			CFcAIHardCodingFunction *pTemp = s_pVecFunction[i]->Clone();
			pTemp->m_dwParamCount = s_pVecFunction[i]->m_dwParamCount;
			pTemp->m_nLevel = nLevel;
			int nSlot = FindEmptySlot();
			if( nSlot == -1 ) {
				s_pVecFunction.push_back( pTemp );
				return (int)s_pVecFunction.size() - 1;
			}
			else {
				s_pVecFunction[nSlot] = pTemp;
				return nSlot;
			}
		}
	}
	return -1;
}

void CFcAIHardCodingFunction::SetFunctionName( char *szFunctionName )
{
	SAFE_DELETEA( m_szFunctionName );
	const size_t szFunctionName_len = strlen( szFunctionName ) + 1;
	m_szFunctionName = new char[ szFunctionName_len ];
	strcpy_s( m_szFunctionName, szFunctionName_len, szFunctionName ); //aleksger - safe string
}

void CFcAIHardCodingFunction::SetParent( CFcAIObject *pParent )
{
	m_pParent = pParent;
}

void CFcAIHardCodingFunction::SetParamPtr( std::vector<CFcParamVariable *> *pParam )
{
	m_pVecParam = pParam;
}

CFcAIHardCodingFunction *CFcAIHardCodingFunction::GetFunction( int nIndex )
{
	return s_pVecFunction[(DWORD)nIndex];
}

void CFcAIHardCodingFunction::ReleaseFunction( int nIndex )
{
	delete s_pVecFunction[(DWORD)nIndex];
	s_pVecFunction[(DWORD)nIndex] = NULL;
}

DWORD CFcAIHardCodingFunction::GetParamCount()
{
	return m_dwParamCount;
}

int CFcAIHardCodingFunction::Random( int nMin, int nMax )
{
	int nRatioMin = nMin / m_pParent->GetTickInterval();
	int nRadioMax = nMax / m_pParent->GetTickInterval();

 	return nRatioMin + ::Random( nRadioMax - nRatioMin );
}


void CFcAIHardCodingFunction::SetDelay( DWORD dwSlot, int nDelay )
{
	if( dwSlot >= m_nVecDelay.size() ) {
		DWORD dwCount = ( dwSlot - m_nVecDelay.size() ) + 1;
		for( DWORD i=0; i<dwCount; i++ )
			m_nVecDelay.push_back( (int)0 );
	}
	m_nVecDelay[dwSlot] = nDelay;
}

bool CFcAIHardCodingFunction::IsDelay( DWORD dwSlot )
{
	if( dwSlot >= m_nVecDelay.size() ) return false;

	if( m_nVecDelay[dwSlot] > 0 ) return true;

	return false;
}

void CFcAIHardCodingFunction::Process()
{
	for( DWORD i=0; i<m_nVecDelay.size(); i++ ) {
		if( m_nVecDelay[i] > 0 ) {
			m_nVecDelay[i] -= ( CFcAIObject::s_dwLocalTick - m_dwPrevTick );
			if( m_nVecDelay[i] < 0 ) m_nVecDelay[i] = 0;
		}
	}
	m_dwPrevTick = CFcAIObject::s_dwLocalTick;
}