#include "stdafx.h"
#include "FcAITriggerCon.h"
#include "FcGameObject.h"
#include "FcAIObject.h"
#include "FcParamVariable.h"
#include "FcAISearch.h"
#include "FcAIGlobalVariableMng.h"
#include "FcWorld.h"
#include "data/AnimType.h"
#include "FcAISearchSlot.h"
#include "FcAbilityManager.h"
#include "FcEventSequencer.h"
#include "FcHeroObject.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG

CFcAIConTargetTeamCheck::CFcAIConTargetTeamCheck( BOOL bSame )
{
	m_bSame = bSame;
}

bool CFcAIConTargetTeamCheck::CheckCondition()
{
	GameObjHandle Me = m_pParent->GetUnitHandle();
	GameObjHandle Target = m_pParent->GetTargetHandle();

	if( !CFcBaseObject::IsValid( Target ) )
		return false;

	if( Target->GetTeam() == -1 )
		return ( m_bSame == TRUE ) ? true : false;

	if( Me->GetTeam() == Target->GetTeam() ) {
		return ( m_bSame == TRUE ) ? true : false;
	}
	else {
		return ( m_bSame == FALSE ) ? true : false;
	}

	return true;
}

CFcAIConTargetLiveCheck::CFcAIConTargetLiveCheck( BOOL bSame )
{
	m_bSame = bSame;
}

bool CFcAIConTargetLiveCheck::CheckCondition()
{
	GameObjHandle Target = m_pParent->GetTargetHandle();
	if( !CFcBaseObject::IsValid( Target ) ) return false;

	if( (Target->IsDie(true)||Target->GetHP()<=0)||Target->IsRagDollMode() ) return ( m_bSame == TRUE ) ? false : true;
	else return ( m_bSame == FALSE ) ? false : true;
	/*
	if( !Target->IsDie() ) return ( m_bSame == TRUE ) ? true : false;
	else  return ( m_bSame == FALSE ) ? true : false;
	*/
}


CFcAIConSelfLiveCheck::CFcAIConSelfLiveCheck( BOOL bSame )
{
	m_bSame = bSame;
}

bool CFcAIConSelfLiveCheck::CheckCondition()
{
	GameObjHandle Me = m_pParent->GetUnitHandle();
	if( !CFcBaseObject::IsValid( Me ) ) return false;

	/*
	if( !Me->IsDie() ) return ( m_bSame == TRUE ) ? true : false;
	else  return ( m_bSame == FALSE ) ? true : false;
	*/
	if( (Me->IsDie(true)||Me->GetHP()<=0) ) return ( m_bSame == TRUE ) ? false : true;
	else return ( m_bSame == FALSE ) ? false : true;

}

CFcAIConUnitState::CFcAIConUnitState( BOOL bTarget, int nStateIndex )
{
	m_bTarget = bTarget;
	m_nStateIndex = nStateIndex;
}

bool CFcAIConUnitState::CheckCondition()
{
	GameObjHandle Handle;
	if( m_bTarget ) Handle = m_pParent->GetTargetHandle();
	else Handle = m_pParent->GetUnitHandle();

	if( !CFcBaseObject::IsValid( Handle ) )
		return false;

	int nOp = GetParamInt(1);
	int nValue[2];


	switch( m_nStateIndex ) {
		case 0:	// HP
			nValue[0] = int( ( 100 / (float)Handle->GetMaxHP() ) * Handle->GetHP() );
			break;
		case 1:
			BsAssert2( false, "This code path uses uninitialized memory" );
			break;
		default:
			nValue[0] = 0;
			break;
	}
	nValue[1] = GetParamInt(0);

	return Compare( nValue[0], nValue[1], nOp );
}


// AI_CON_TARGET_RANGE
CFcAIConTargetRange::CFcAIConTargetRange()
{

}

bool CFcAIConTargetRange::CheckCondition()
{
	int nRange = GetParamInt(0);
	int nOp = GetParamInt(1);

	GameObjHandle Me = m_pParent->GetUnitHandle();
	GameObjHandle Target = m_pParent->GetTargetHandle();
	if( !CFcBaseObject::IsValid( Target ) )
		return false;

	D3DXVECTOR3 Vec = Me->GetPos() - Target->GetPos();
	Vec.y = 0.f;
	float fDist = (float)D3DXVec3Length( &Vec ) - Target->GetUnitRadius();

	return Compare( fDist, (float)nRange, nOp );
}


// AI_CON_ENEMY_SEARCH_COUNT
std::vector<GameObjHandle> CFcAIConEnemySearchCount::m_VecList;
CFcAIConEnemySearchCount::CFcAIConEnemySearchCount()
{
}

bool CFcAIConEnemySearchCount::CheckCondition()
{
	GameObjHandle Me = m_pParent->GetUnitHandle();

//	CFcAISearch *pSearch = m_pParent->GetSearch();
//	std::vector<GameObjHandle> *pVecList = pSearch->GetHandleList();

	m_VecList.clear();
	CFcWorld::GetInstance().GetObjectListInRange( (D3DXVECTOR3*)&Me->GetDummyPos(), (float)GetParamFloat(0), m_VecList );

	int nCount = 0;
	for( DWORD i=0; i<m_VecList.size(); i++ ) {
		if( m_VecList[i] == Me ) continue;
		if( m_VecList[i]->GetTeam() != Me->GetTeam() ) nCount++;
	}

	int nValue = GetParamInt(1);
	int nOp = GetParamInt(2);

	return Compare( nCount, nValue, nOp );
}

// AI_CON_GLOBAL_VARIABLE
CFcAIConGlobalVariable::CFcAIConGlobalVariable( int nValueType )
{
	m_nValueType = nValueType;
	m_nValueIndex = -1;
}

bool CFcAIConGlobalVariable::CheckCondition()
{
	CFcAIGlobalVariableMng *pMng = m_pParent->GetParent()->GetGlobalVariableMng();
	if( m_nValueIndex == -1 ) {
		const char *szString = GetParamString(0);
		m_nValueIndex = pMng->GetVariableIndex( szString );
	}
	if( m_nValueIndex == -1 ) return false;
	CFcParamVariable *pGlobalParam = pMng->GetVariable( m_nValueIndex );

	// 랜덤일때 비율 마춰주어야한다.
	// CFcAIObject 에서 TickInterval 에 따라 랜덤의 비율을 조정해주어야 한다.
	CFcParamVariable *pCurParam = NULL;
	if( pGlobalParam->GetType() == CFcParamVariable::RANDOM && pGlobalParam->GetVariableRandom()[1] == 2400 ) {
		static CFcParamVariable RandomValue( CFcParamVariable::RANDOM );
		RandomValue.SetVariable( pGlobalParam->GetVariableRandom()[0] / m_pParent->GetParent()->GetTickInterval(), 
								pGlobalParam->GetVariableRandom()[1] / m_pParent->GetParent()->GetTickInterval() );

		pCurParam = &RandomValue;
		if( strcmp( GetParamString(0), "난수" ) == 0 ) {
			CFcParamVariable * pParam1 = GetParam(1);
			if( strcmp( m_pParent->GetParent()->GetFileName() , "BattleMeleeAttack.ai_func" ) == NULL ) {
				int asdf = 0;
			}
		}
	}
	else pCurParam = pGlobalParam;

	// mruete: prefix bug 562: added intermediate variable and assert.
	CFcParamVariable * pParam1 = GetParam(1);
	BsAssert( NULL != pParam1 );

	int nOp = GetParamInt(2);
	switch( nOp ) {
		case AI_OP_EQUAL:					return ( *pCurParam == *pParam1 );
		case AI_OP_NOT_EQUAL:				return ( *pCurParam != *pParam1 );
		case AI_OP_GREATER:					return ( *pCurParam > *pParam1 );
		case AI_OP_GREATER_THAN_OR_EQUAL:	return ( *pCurParam >= *pParam1 );
		case AI_OP_LESS_THAN:				return ( *pCurParam < *pParam1 );
		case AI_OP_LESS_THAN_OR_EQUAL:		return ( *pCurParam <= *pParam1 );
		default:
			BsAssert(0);
			break;
	}
	return false;
}

// AI_CON_CAN_ANI
CFcAIConCanAni::CFcAIConCanAni( BOOL bValid )
{
	m_bValid = bValid;
}

bool CFcAIConCanAni::CheckCondition()
{
	GameObjHandle Me = m_pParent->GetUnitHandle();
	int nAniAttr = GetParamInt(0);
	int nAniType = GetParamInt(1);
	int nAniCount = GetParamInt(2);

	int nCurType = Me->GetCurAniType();
	int nCurAniAttr = nCurType & 0xFF00;
	int nCurAniType = nCurType & 0x00FF;

	if( nAniAttr != -1 ) {
		if( nCurAniType != nAniAttr ) return !m_bValid;
	}

	if( nAniType != -1 ) {
		switch( nAniType ) {
			case ANI_TYPE_STAND:
				if( nCurAniType != ANI_TYPE_STAND ) return !m_bValid;
				break;
			case ANI_TYPE_ATTACK:
				if( nCurAniType != ANI_TYPE_STAND ) return !m_bValid;
				break;
			case ANI_TYPE_MOVE:
				if( nCurAniType != ANI_TYPE_STAND ) return !m_bValid;
				break;
			case ANI_TYPE_RUN_ATTACK:
				if( nCurAniType != ANI_TYPE_RUN ) return !m_bValid;
				break;
		}
	}
	return ( m_bValid == TRUE );
}

CFcAIConCheckSlot::CFcAIConCheckSlot( BOOL bValid )
{
	m_bValid = bValid;
}

bool CFcAIConCheckSlot::CheckCondition()
{
	switch( m_pParent->GetParent()->GetSearchSlot()->GetSlotType( GetParamInt(0) ) ) {
		case 0:
			if( m_pParent->GetParent()->GetSearchSlot()->GetHandle( GetParamInt(0) ) )
				return ( m_bValid == TRUE );
			break;
		case 1:
			if( m_pParent->GetParent()->GetSearchSlot()->GetPtr( GetParamInt(0) ) )
				return ( m_bValid == TRUE );
			break;
	}

	return ( m_bValid == FALSE );
}


CFcAIConAniType::CFcAIConAniType( BOOL bTarget )
{
	m_bTarget = bTarget;
}

bool CFcAIConAniType::CheckCondition()
{
	GameObjHandle Handle;
	if( m_bTarget == TRUE ) Handle = m_pParent->GetTargetHandle();
	else Handle = m_pParent->GetUnitHandle();

	if( !CFcBaseObject::IsValid( Handle ) ) return false;
	if( GetParamInt(0) != -1 ) {
		if( Compare( Handle->GetCurAniType() & 0xFF00, GetParamInt(0), GetParamInt(3) ) == false )
			return false;
	}
	if( GetParamInt(1) != -1 ) {
		if( Compare( Handle->GetCurAniType() & 0x00FF, GetParamInt(1), GetParamInt(3) ) == false )
			return false;
	}
	if( GetParamInt(2) != -1 ) {
		if( Compare( Handle->GetCurAniIndex(), GetParamInt(2), GetParamInt(3) ) == false ) 
			return false;
//		Handle->GetCurAni
	}

	return true;
}

CFcAIConDebugString::CFcAIConDebugString()
{
}

bool CFcAIConDebugString::CheckCondition()
{
	CBsKernel::GetInstance().AddConsoleString( (const char*)GetParamString(0) );
	return true;
}

CFcAIConFunction::CFcAIConFunction()
{
}

bool CFcAIConFunction::CheckCondition()
{
	// mruete: prefix bug 563: added intermediate variables and assert.
	CFcParamVariable * pParam0 = GetParam(0);
	CFcParamVariable * pParam1 = GetParam(1);
	BsAssert( NULL != pParam0 && NULL != pParam1 );

	int nOp = GetParamInt(2);
	switch( nOp ) {
		case AI_OP_EQUAL:					return ( *pParam0 == *pParam1 );
		case AI_OP_NOT_EQUAL:				return ( *pParam0 != *pParam1 );
		case AI_OP_GREATER:					return ( *pParam0 > *pParam1 );
		case AI_OP_GREATER_THAN_OR_EQUAL:	return ( *pParam0 >= *pParam1 );
		case AI_OP_LESS_THAN:				return ( *pParam0 < *pParam1 );
		case AI_OP_LESS_THAN_OR_EQUAL:		return ( *pParam0 <= *pParam1 );
		default:
			BsAssert(0);
			break;
	}
	return false;
}

CFcAIConInfluenceTargetingCount::CFcAIConInfluenceTargetingCount()
{
}

bool CFcAIConInfluenceTargetingCount::CheckCondition()
{
	GameObjHandle Target = m_pParent->GetTargetHandle();
	if( !CFcBaseObject::IsValid( Target ) ) {
		return false;
	}
	CFcCorrelationTargeting *pCorrelation = (CFcCorrelationTargeting *)Target->GetCorrelation( CFcCorrelation::CT_TARGETING_COUNTER );

	if( pCorrelation == NULL ) return false;

	return Compare( *(int*)pCorrelation->GetResult(), GetParamInt(0), GetParamInt(1) );
}


CFcAIConJobIndex::CFcAIConJobIndex()
{
}

bool CFcAIConJobIndex::CheckCondition()
{
	GameObjHandle Me = m_pParent->GetUnitHandle();
	
	return Compare( (int)Me->IsSubJob(), GetParamInt(0), GetParamInt(1) );
}

CFcAIConClassID::CFcAIConClassID()
{
}
bool CFcAIConClassID::CheckCondition()
{
	GameObjHandle Target = m_pParent->GetTargetHandle();
	if( !CFcBaseObject::IsValid( Target ) ) {
		return false;
	}
	return Compare( (int)Target->GetClassID(), GetParamInt(0), GetParamInt(1) );
}

CFcAIConLinkClassID::CFcAIConLinkClassID()
{
}
bool CFcAIConLinkClassID::CheckCondition()
{
	GameObjHandle Target = m_pParent->GetTargetHandle();
	if( !CFcBaseObject::IsValid( Target ) ) {
		return false;
	}
	GameObjHandle hLink = Target->GetParentLinkHandle();
	if( !CFcBaseObject::IsValid( hLink ) ) {
		return false;
	}
	return Compare( (int)hLink->GetClassID(), GetParamInt(0), GetParamInt(1) );
}


CFcAIConWeapon::CFcAIConWeapon( BOOL bShow )
{
	m_bShow = bShow;
}

bool CFcAIConWeapon::CheckCondition()
{
	GameObjHandle Me = m_pParent->GetUnitHandle();
	if( !CFcBaseObject::IsValid( Me ) ) {
		return false;
	}

	std::vector<WEAPON_OBJECT> *pVecList = Me->GetWeaponList();
	int nWeaponIndex = GetParamInt(0) - 1;
	if( nWeaponIndex < 0 || nWeaponIndex >= (int)pVecList->size()  ) return false;
	if( (*pVecList)[nWeaponIndex].bShow ) return m_bShow ? TRUE : FALSE;
	else return m_bShow ? FALSE : TRUE;
}


CFcAIConIsValidLink::CFcAIConIsValidLink( BOOL bValid )
{
	m_bValid = bValid;
}

bool CFcAIConIsValidLink::CheckCondition()
{
	GameObjHandle Target = m_pParent->GetTargetHandle();
	if( !CFcBaseObject::IsValid( Target ) ) {
		return !m_bValid;
	}
	GameObjHandle hLink = Target->GetParentLinkHandle();
	if( !CFcBaseObject::IsValid( hLink ) ) {
		return !m_bValid;
	}
	return ( m_bValid == TRUE );
}


CFcAIConTargetViewAngle::CFcAIConTargetViewAngle()
{
}

bool CFcAIConTargetViewAngle::CheckCondition()
{
	GameObjHandle Me = m_pParent->GetUnitHandle();
	GameObjHandle Target = m_pParent->GetTargetHandle();
	if( !CFcBaseObject::IsValid( Me ) ) return false;
	if( !CFcBaseObject::IsValid( Target ) ) return false;

	D3DXVECTOR3 vVec = Target->GetPos() - Me->GetPos();
	D3DXVec3Normalize( &vVec, &vVec );
	float fAngle = acos( D3DXVec3Dot( &Me->GetCrossVector()->m_ZVector, &vVec ) ) * 180.f / 3.1415926f;

	return Compare( abs( (int)fAngle ), GetParamInt(0), GetParamInt(1) );
}



CFcAIConTargetValid::CFcAIConTargetValid( BOOL bValid )
{
	m_bValid = bValid;
}

bool CFcAIConTargetValid::CheckCondition()
{
	GameObjHandle Target = m_pParent->GetTargetHandle();

	if( !CFcBaseObject::IsValid( Target ) ) return ( m_bValid == TRUE ) ? false : true ;
	return ( m_bValid == TRUE ) ? true : false;
}

CFcAIConIsProcessTrueOrb::CFcAIConIsProcessTrueOrb()
{
}

bool CFcAIConIsProcessTrueOrb::CheckCondition()
{
	GameObjHandle Me = m_pParent->GetUnitHandle();
	if( !CFcBaseObject::IsValid( Me ) ) return !GetParamBool(0);

	if( g_FcWorld.IsProcessTrueOrb( Me->GetTeam() ) ) {
		return GetParamBool(0);
	}

	return !GetParamBool(0);
//	if( g_FcWorld.GetAbilityMng() && g_FcWorld.GetAbilityMng()->GetCount() > 0 ) return GetParamBool(0);
//	return !GetParamBool(0);
}

CFcAIConEventSeqCount::CFcAIConEventSeqCount( BOOL bTarget )
{
	m_bTarget = bTarget;
}

bool CFcAIConEventSeqCount::CheckCondition()
{
	GameObjHandle Handle;
	if( m_bTarget ) Handle = m_pParent->GetTargetHandle();
	else Handle = m_pParent->GetUnitHandle();

	if( !CFcBaseObject::IsValid( Handle ) ) return false;

	return Compare( (int)Handle->GetEventSeqMng()->GetEventCount(), GetParamInt(0), GetParamInt(1) );
}

CFcAIConControlIndex::CFcAIConControlIndex()
{
}

bool CFcAIConControlIndex::CheckCondition()
{
	GameObjHandle Handle = m_pParent->GetUnitHandle();
	if( !CFcBaseObject::IsValid( Handle ) ) return false;

	return Compare( Handle->GetAIControlIndex(), GetParamInt(0), GetParamInt(1) );
}

CFcAIConInfluenceDamage::CFcAIConInfluenceDamage()
{
}

bool CFcAIConInfluenceDamage::CheckCondition()
{
	GameObjHandle Me = m_pParent->GetUnitHandle();
	GameObjHandle Handle = m_pParent->GetTargetHandle();

	if( !CFcBaseObject::IsValid( Me ) ) return false;
	if( !CFcBaseObject::IsValid( Handle ) ) return false;

	CFcCorrelationHit *pCorrelation = (CFcCorrelationHit *)Me->GetCorrelation( CFcCorrelation::CT_HIT );
	if( pCorrelation == NULL ) return false;

	CFcCorrelationHit::HitStruct *pHitStruct = pCorrelation->FindResultFromHandle( Handle );
	if( pHitStruct == NULL ) return false;

	return Compare( pHitStruct->nDamage, GetParamInt(0), GetParamInt(1) );
}

CFcAIConInfluenceHitCount::CFcAIConInfluenceHitCount()
{
}

bool CFcAIConInfluenceHitCount::CheckCondition()
{
	GameObjHandle Me = m_pParent->GetUnitHandle();
	GameObjHandle Handle = m_pParent->GetTargetHandle();

	if( !CFcBaseObject::IsValid( Me ) ) return false;
	if( !CFcBaseObject::IsValid( Handle ) ) return false;

	CFcCorrelationHit *pCorrelation = (CFcCorrelationHit *)Me->GetCorrelation( CFcCorrelation::CT_HIT );
	if( pCorrelation == NULL ) return false;

	CFcCorrelationHit::HitStruct *pHitStruct = pCorrelation->FindResultFromHandle( Handle );
	if( pHitStruct == NULL ) return false;

	return Compare( pHitStruct->nCount, GetParamInt(0), GetParamInt(1) );
}

CFcAIConCheckCatch::CFcAIConCheckCatch( BOOL bValid )
{
	m_bValid = bValid;
}

bool CFcAIConCheckCatch::CheckCondition()
{
	GameObjHandle Me = m_pParent->GetUnitHandle();

	if( !CFcBaseObject::IsValid( Me ) ) return !m_bValid;

	if( Me->GetClassID() != CFcGameObject::Class_ID_Hero ) return !m_bValid;

	CFcHeroObject *pHero = (CFcHeroObject *)Me.GetPointer();
	GameObjHandle Handle = pHero->GetCatchHandle();

	if( !CFcBaseObject::IsValid( Handle ) ) return !m_bValid;
	return ( m_bValid == TRUE );
}
