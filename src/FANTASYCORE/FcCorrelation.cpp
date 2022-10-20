#include "StdAFx.h"
#include "FcCorrelation.h"
#include "FcGameObject.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG


CFcCorrelationMng::CFcCorrelationMng( GameObjHandle hParent, int Type )
{
	m_hParent = hParent;

	CFcCorrelation *pObject = NULL;
	if( Type & CFcCorrelation::CT_TARGETING_COUNTER ) {
		pObject = new CFcCorrelationTargeting( this );
		m_pVecCorrelation.push_back( pObject );
	}
	if( Type & CFcCorrelation::CT_HIT ) {
		pObject = new CFcCorrelationHit( this );
		m_pVecCorrelation.push_back( pObject );
	}
}

CFcCorrelationMng::~CFcCorrelationMng()
{
	SAFE_DELETE_PVEC( m_pVecCorrelation );
}

CFcCorrelation *CFcCorrelationMng::FindCorrelation( CFcCorrelation::CORRELATION_TYPE Type )
{
	for( DWORD i=0; i<m_pVecCorrelation.size(); i++ ) {
		if( m_pVecCorrelation[i]->GetType() == Type ) return m_pVecCorrelation[i];
	}

	return NULL;
}

CFcCorrelation *CFcCorrelationMng::GetCorrelation( CFcCorrelation::CORRELATION_TYPE Type )
{
	return FindCorrelation( Type );
}

void CFcCorrelationMng::Influence( CFcCorrelation::CORRELATION_TYPE Type, void *pValue )
{
	CFcCorrelation *pObject = FindCorrelation( Type );
	if( pObject ) pObject->Influence( pValue );
}

void CFcCorrelationMng::Reset( CFcCorrelation::CORRELATION_TYPE Type )
{
	if( Type == CFcCorrelation::CT_NONE ) {
		for( DWORD i=0; i<m_pVecCorrelation.size(); i++ ) {
			m_pVecCorrelation[i]->Reset();
		}
	}
	else {
		CFcCorrelation *pObject = FindCorrelation( Type );
		if( pObject ) pObject->Reset();
	}
}

void *CFcCorrelationMng::GetResult( CFcCorrelation::CORRELATION_TYPE Type )
{
	CFcCorrelation *pObject = FindCorrelation( Type );
	if( pObject ) return pObject->GetResult();
	return NULL;
}


CFcCorrelationTargeting::CFcCorrelationTargeting( CFcCorrelationMng *pMng )
: CFcCorrelation( pMng )
{
	m_Type = CT_TARGETING_COUNTER;
}

CFcCorrelationTargeting::~CFcCorrelationTargeting()
{
	m_VecHandle.clear();
	m_VecHandle.swap( m_VecHandle );
}

void CFcCorrelationTargeting::Influence( void *pValue )
{
	ParamStruct *pStruct = (ParamStruct *)pValue;

	InfluenceTargeting( pStruct->pHandle, pStruct->bAddRemoveFlag );
}

void CFcCorrelationTargeting::Reset()
{
	m_VecHandle.clear();
}

void CFcCorrelationTargeting::InfluenceTargeting( GameObjHandle *pHandle, bool bAddRemoveFlag )
{
	if( bAddRemoveFlag == true ) {
		if( std::find( m_VecHandle.begin(), m_VecHandle.end(), *pHandle ) == m_VecHandle.end() ) 
			m_VecHandle.push_back( *pHandle );
	}
	else {
		std::vector<GameObjHandle>::iterator itr;
		if( ( itr = std::find( m_VecHandle.begin(), m_VecHandle.end(), *pHandle ) ) != m_VecHandle.end() ) 
			m_VecHandle.erase( itr );
	}
}

void *CFcCorrelationTargeting::GetResult()
{
	m_nTotalCount = m_VecHandle.size();
	return (void*)&m_nTotalCount;
}


CFcCorrelationHit::CFcCorrelationHit( CFcCorrelationMng *pMng )
: CFcCorrelation( pMng )
{
	m_Type = CT_HIT;
	m_nLastHit = -1;
}

CFcCorrelationHit::~CFcCorrelationHit()
{
	SAFE_DELETE_PVEC( m_pVecHit );
}

static bool CorrelationHitCountSortFunc( CFcCorrelationHit::HitStruct *a, CFcCorrelationHit::HitStruct *b )
{
	if( a->nCount > b->nCount ) return true;
	return false;
}


void CFcCorrelationHit::Influence( void *pValue )
{
	ParamStruct *pStruct = (ParamStruct *)pValue;

	InfluenceHit( pStruct->pHandle,	pStruct->nDamage );
}

void CFcCorrelationHit::Reset()
{
	SAFE_DELETE_PVEC( m_pVecHit );
}

void CFcCorrelationHit::InfluenceHit( GameObjHandle *pHandle, int nDamage )
{
	for( DWORD i=0; i<m_pVecHit.size(); i++ ) {
		HitStruct *pStruct = m_pVecHit[i];
		if( pStruct->Handle == *pHandle ) {
			pStruct->nDamage += nDamage;
			++pStruct->nCount;
			m_nLastHit = i;
			return;
		}
	}
	HitStruct *pStruct = new HitStruct;
	pStruct->Handle = *pHandle;
	pStruct->nDamage = nDamage;
	pStruct->nCount = 1;
	m_pVecHit.push_back( pStruct );

	m_nLastHit = m_pVecHit.size() - 1; 
//	std::sort( m_pVecHit.begin(), m_pVecHit.end(), CorrelationHitCountSortFunc );
}

void *CFcCorrelationHit::GetResult()
{
	if( m_pVecHit.size() == 0 ) return NULL;
	else {
		if( m_nLastHit != -1 )
			return (void*)&m_pVecHit[m_nLastHit]->Handle;
	}
	return NULL;
}

CFcCorrelationHit::HitStruct *CFcCorrelationHit::FindResultFromHandle( GameObjHandle &Handle )
{
	for( DWORD i=0; i<m_pVecHit.size(); i++ ) {
		if( m_pVecHit[i]->Handle == Handle ) return m_pVecHit[i];
	}
	return NULL;
}

/*
CFcCorrelation::CFcCorrelation( GameObjHandle hParent )
{
	m_hParent = hParent;
}

CFcCorrelation::~CFcCorrelation()
{
	m_VecTargetingHandle.clear();
	m_VecTargetingHandle.swap( m_VecTargetingHandle );
}

GameObjHandle CFcCorrelation::GetSuitableHandle()
{
	return m_hResult;
}

void CFcCorrelation::InfluenceHit( HIT_PARAM *pParam )
{
}

void CFcCorrelation::InfluenceTargeting( GameObjHandle *pHandle, bool bAddRemoveFlag )
{
	if( bAddRemoveFlag == true ) {
		if( std::find( m_VecTargetingHandle.begin(), m_VecTargetingHandle.end(), *pHandle ) == m_VecTargetingHandle.end() ) 
			m_VecTargetingHandle.push_back( *pHandle );
	}
	else {
		std::vector<GameObjHandle>::iterator itr;
		if( ( itr = std::find( m_VecTargetingHandle.begin(), m_VecTargetingHandle.end(), *pHandle ) ) != m_VecTargetingHandle.end() ) 
			m_VecTargetingHandle.erase( itr );
	}
}

int CFcCorrelation::GetTargetingCount()
{
	return (int)m_VecTargetingHandle.size();
}
void CFcCorrelation::InfluenceEmptyTargeting()
{
	m_VecTargetingHandle.clear();
}
*/