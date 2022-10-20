#include "stdafx.h"
#include "FcAITriggerAction.h"
#include "FcGameObject.h"
#include "FcTroopObject.h"
#include "FcAIObject.h"
#include "FcParamVariable.h"
#include "FcAIElement.h"
#include "FcAIGlobalVariableMng.h"
#include "BsKernel.h"
#include "FcAISearchSlot.h"
#include "FcCorrelation.h"
#include "FcHeroObject.h"
#include "ASData.h"
#include "FcArcherUnitObject.h"
#include "DebugUtil.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG

// Action class
CFcAIActionDelay::CFcAIActionDelay()
{
}

int CFcAIActionDelay::Command()
{
	m_pParent->SetDelayTick( GetParamInt(0) );
	return TRUE;
}


CFcAIActionSetAni::CFcAIActionSetAni( BOOL bLoop )
{
	m_bLoop = bLoop;
}

int CFcAIActionSetAni::Command()
{
	GameObjHandle Me = m_pParent->GetUnitHandle();

//	/*
	int nFindAni = Me->FindExactAni( Me->GetUnitInfoData()->GetAniIndex( GetParamInt(1), GetParamInt(2), GetParamInt(0) ) );
	if( nFindAni != -1 ) {
		Me->ChangeAnimationByIndex( nFindAni, GetParamBool(3), ( m_bLoop == TRUE ) ? GetParamInt(4) : 1 );
	}
//	*/
//	Me->ChangeAnimation( GetParamInt(1), GetParamInt(2), GetParamInt(0), GetParamBool(3), ( m_bLoop == TRUE ) ? GetParamInt(4) : 1 );

	return TRUE;
}

//	AI_ACTION_FOLLOW_TARGET
CFcAIActionFollowTarget::CFcAIActionFollowTarget()
{

}

int CFcAIActionFollowTarget::Command()
{
	GameObjHandle Me = m_pParent->GetUnitHandle();
	GameObjHandle Target = m_pParent->GetTargetHandle();
	if( !CFcBaseObject::IsValid( Target ) )
	{
		return TRUE;
	}
	D3DXVECTOR3 vVec, vPos;
	// 이상한게 잇으면 꼭 빼라~
	vVec = Target->GetDummyPos() - Me->GetDummyPos();
	vVec.y = 0.f;
	if( D3DXVec3Length( &vVec ) < (float)GetParamInt(3) ) {
		Me->CmdStop( Me->GetCurAniAttr() | ANI_TYPE_STAND, -1 );
		return TRUE;
	}
	/////////////

//	vVec = Target->GetPos() - Me->GetPos();
	/*
	vPos = Target->GetDummyPos();
	vPos.y = 0.f;
	vVec.y = 0.f;

	D3DXVec3Normalize( &vVec, &vVec );

	vVec *= (float)GetParamInt(3);
	vPos -= vVec;

	Me->CmdMove( (int)vPos.x, (int)vPos.z, GetParamFloat(4), GetParamInt(0) | GetParamInt(1), GetParamInt(2) );
	*/
	Me->CmdMove( Target, (float)GetParamInt(3), GetParamFloat(4), GetParamInt(0) | GetParamInt(1), GetParamInt(2) );

	return TRUE;
}

// AI_ACTION_OPPOSITION_TARGET
CFcAIActionOppositionTarget::CFcAIActionOppositionTarget( BOOL bTarget )
{
	m_bTarget = bTarget;
}

int CFcAIActionOppositionTarget::Command()
{
	GameObjHandle Me = m_pParent->GetUnitHandle();
	GameObjHandle Target = m_pParent->GetTargetHandle();
	if( !CFcBaseObject::IsValid( Target ) )
	{
		BsAssert(0);
		return TRUE;
	}

	float fLength;
	D3DXVECTOR3 vVec;

	vVec = Target->GetDummyPos() - Me->GetDummyPos();
	vVec.y = 0.f;

	fLength = D3DXVec3Length( &vVec );
	D3DXVec3Normalize( &vVec, &vVec );


	if( m_bTarget == TRUE ) {
		Me->GetCrossVector()->m_PosVector -= vVec * ( GetParamInt(0) - fLength );
		Me->CmdStop();
	}
	else {
		Target->GetCrossVector()->m_PosVector += vVec * ( GetParamInt(0) - fLength );
		Target->CmdStop();
	}

	return TRUE;
}

//	AI_ACTION_STOP
CFcAIActionStop::CFcAIActionStop()
{

}

int CFcAIActionStop::Command()
{
	GameObjHandle Me = m_pParent->GetUnitHandle();
	Me->CmdStop( GetParamInt(0) | GetParamInt(1), GetParamInt(2) );
	return TRUE;
}


//	AI_ACTION_LOOK_TARGET
CFcAIActionLookTarget::CFcAIActionLookTarget()
{

}

int CFcAIActionLookTarget::Command()
{
	GameObjHandle Me = m_pParent->GetUnitHandle();
	GameObjHandle Target = m_pParent->GetTargetHandle();
	if( !CFcBaseObject::IsValid( Me ) ) return TRUE;
	if( !CFcBaseObject::IsValid( Target ) ) return TRUE;
	Me->CmdLookUp( Target );

	return TRUE;
}

CFcAIActionChangeGlobalVariable::CFcAIActionChangeGlobalVariable( int nValueType )
{
	m_nValueType = nValueType;
	m_nValueIndex = -1;
}

//int g_nPrevPattern = 0;
int CFcAIActionChangeGlobalVariable::Command()
{
	CFcAIGlobalVariableMng *pMng = m_pParent->GetParent()->GetGlobalVariableMng();
	if( m_nValueIndex == -1 ) {
		const char *szString = GetParamString(0);
		m_nValueIndex = pMng->GetVariableIndex( szString );
	}
	CFcParamVariable *pGlobalParam = pMng->GetVariable( m_nValueIndex );
	BsAssert( NULL != pGlobalParam );	// mruete: prefix bug 558: added assert.

	/*
	if( strcmp( pGlobalParam->GetDescription(), "Pattern" ) == NULL ) {
		if( GetParamInt(1) == 4 ) {
			if( g_nPrevPattern == 0 ) {
				int asdf = 0;
			}
			g_nPrevPattern = 4;
		}
		else if( GetParamInt(1) == 0 ) {
			if( g_nPrevPattern == 4 ) {
				int asdf = 0;
			}
			g_nPrevPattern = 0;
		}
	}
	*/
	/*
	if( strcmp( pGlobalParam->GetDescription(), "Pattern" ) == NULL ) {
		char szStr[64];
		sprintf( szStr, "Pattern : %d\n", GetParamInt(1) );
		DebugString( szStr );
	}
	if( strcmp( pGlobalParam->GetDescription(), "PickupWeaponUseCount" ) == NULL ) {
		char szStr[64];
		sprintf( szStr, "PickupWeaponUseCount : %d\n", GetParamInt(1) );
		DebugString( szStr );
	}
	*/

	switch( pGlobalParam->GetType() ) {
		case CFcParamVariable::INT:		pGlobalParam->SetVariable( GetParamInt(1) );		break;
		case CFcParamVariable::FLOAT:	pGlobalParam->SetVariable( GetParamFloat(1) );	break;
		case CFcParamVariable::STRING:	pGlobalParam->SetVariable( GetParamString(1) );	break;
		case CFcParamVariable::VECTOR:	pGlobalParam->SetVariable( GetParamVector(1) );	break;
		case CFcParamVariable::BOOLEAN:	pGlobalParam->SetVariable( GetParamBool(1) );		break;
		case CFcParamVariable::RANDOM:	pGlobalParam->SetVariable( GetParamInt(1), GetParamInt(2) );	break;
		case CFcParamVariable::PTR:		pGlobalParam->SetVariable( (void*)NULL );	break;

	}

	return TRUE;
}

CFcAIActionGlobalVariableCounter::CFcAIActionGlobalVariableCounter( int nValueType )
{
	m_nValueType = nValueType;
	m_nValueIndex = -1;
}

int CFcAIActionGlobalVariableCounter::Command()
{
	CFcAIGlobalVariableMng *pMng = m_pParent->GetParent()->GetGlobalVariableMng();
	if( m_nValueIndex == -1 ) {
		const char *szString = GetParamString(0);
		m_nValueIndex = pMng->GetVariableIndex( szString );
	}
	CFcParamVariable *pGlobalParam = pMng->GetVariable( m_nValueIndex );
	BsAssert( NULL != pGlobalParam );	// mruete: prefix bug 559: added assert

	switch( pGlobalParam->GetType() ) {
		case CFcParamVariable::INT:		pGlobalParam->SetVariable( pGlobalParam->GetVariableInt() + GetParamInt(1) );	break;
			/* 안만들었음..쓸일없어보임..
		case CFcParamVariable::FLOAT:	pGlobalParam->SetVariable( GetParamFloat(1) );	break;
		case CFcParamVariable::STRING:	pGlobalParam->SetVariable( GetParamString(1) );	break;
		case CFcParamVariable::VECTOR:	pGlobalParam->SetVariable( GetParamVector(1) );	break;
		case CFcParamVariable::BOOLEAN:	pGlobalParam->SetVariable( GetParamBool(1) );		break;
		case CFcParamVariable::RANDOM:	pGlobalParam->SetVariable( GetParamInt(1), GetParamInt(2) );	break;
		case CFcParamVariable::PTR:		pGlobalParam->SetVariable( (void*)NULL );	break;
			*/

	}

	return TRUE;
}


CFcAIActionSetTargetGlobalValue::CFcAIActionSetTargetGlobalValue()
{
}

int CFcAIActionSetTargetGlobalValue::Command()
{
	DWORD dwIndex = m_pParent->GetParent()->GetGlobalVariableMng()->GetVariableIndex( GetParamString(0) );
	CFcParamVariable *pVariable = m_pParent->GetParent()->GetGlobalVariableMng()->GetVariable( dwIndex );
	BsAssert( NULL != pVariable );	// mruete: prefix bug 560: added assert

	pVariable->SetVariable( (void*)m_pParent->GetTargetHandle().GetPointer() );
	return TRUE;
}

CFcAIActionSetTargetSlot::CFcAIActionSetTargetSlot()
{
}

int CFcAIActionSetTargetSlot::Command()
{
	GameObjHandle Handle = m_pParent->GetTargetHandle();
	if( CFcBaseObject::IsValid( Handle ) ) {
		m_pParent->GetParent()->GetSearchSlot()->SetHandle( GetParamInt(0), &Handle );
	}

	return TRUE;
}

CFcAIActionEmptySlot::CFcAIActionEmptySlot()
{
}

int CFcAIActionEmptySlot::Command()
{
	switch( m_pParent->GetParent()->GetSearchSlot()->GetSlotType( GetParamInt(0) ) ) {
		case 0:
			m_pParent->GetParent()->GetSearchSlot()->SetHandle( GetParamInt(0), NULL );
			break;
		case 1:
			m_pParent->GetParent()->GetSearchSlot()->SetPtr( GetParamInt(0), NULL );
			break;
	}
	
	return TRUE;
}

CFcAIActionDebugString::CFcAIActionDebugString()
{
}

int CFcAIActionDebugString::Command()
{
	CBsKernel::GetInstance().AddConsoleString( (const char*)GetParamString(0) );
	return TRUE;
}


CFcAIActionLookSlot::CFcAIActionLookSlot()
{
}

int CFcAIActionLookSlot::Command()
{
	m_pParent->GetParent()->GetSearchSlot()->SetLookAt( GetParamInt(0), GetParamBool(1) );
	return TRUE;
}


CFcAIActionInfluenceTargeting::CFcAIActionInfluenceTargeting( BOOL bEmptyAll )
{
	m_bEmptyAll = bEmptyAll;

}

int CFcAIActionInfluenceTargeting::Command()
{
	GameObjHandle Target = m_pParent->GetTargetHandle();
	if( !CFcBaseObject::IsValid( Target ) ) {
		return FALSE;
	}
	CFcCorrelationTargeting *pCorrelation = (CFcCorrelationTargeting *)Target->GetCorrelation( CFcCorrelation::CT_TARGETING_COUNTER );

	if( pCorrelation == NULL ) return FALSE;

	if( m_bEmptyAll == TRUE ) {
		pCorrelation->Reset();
		return TRUE;
	}
	else {
		pCorrelation->InfluenceTargeting( &m_pParent->GetUnitHandle(), GetParamBool(0) );
	}
	/*
	char szStr[64];
	if( GetParamBool(0) == true ) {
		sprintf( szStr, "Count : %d, Add : %d\n", *(int*)pCorrelation->GetResult(), m_pParent->GetUnitHandle() );
	}
	else {
		sprintf( szStr, "Count : %d, Remove : %d\n", *(int*)pCorrelation->GetResult(), m_pParent->GetUnitHandle() );
	}
	DebugString( szStr );
	*/
	return TRUE;
}

CFcAIActionInfluenceTargetingSlot::CFcAIActionInfluenceTargetingSlot()
{
}

int CFcAIActionInfluenceTargetingSlot::Command()
{
	GameObjHandle *pTarget = m_pParent->GetParent()->GetSearchSlot()->GetHandle( GetParamInt(0) );
	if( pTarget == NULL || !CFcBaseObject::IsValid( *pTarget ) ) return FALSE;

	CFcCorrelationTargeting *pCorrelation = (CFcCorrelationTargeting *)(*pTarget)->GetCorrelation( CFcCorrelation::CT_TARGETING_COUNTER );

	if( pCorrelation == NULL ) return FALSE;

	pCorrelation->InfluenceTargeting( &m_pParent->GetUnitHandle(), GetParamBool(1) );
	return TRUE;
}

CFcAIActionCmd::CFcAIActionCmd( int nCmdType )
{
	m_nCmdType = nCmdType;
}

int CFcAIActionCmd::Command()
{
	switch( m_nCmdType ) {
		case 0:	// ATTACK
			m_pParent->GetUnitHandle()->CmdAttack( m_pParent->GetTargetHandle() );
			break;
	}
	return TRUE;
}


CFcAIActionCustomMove::CFcAIActionCustomMove()
{
}

int CFcAIActionCustomMove::Command()
{
	GameObjHandle Me = m_pParent->GetUnitHandle();
	GameObjHandle Target = m_pParent->GetTargetHandle();
	if( !CFcBaseObject::IsValid( Target ) ) return FALSE;

	D3DXMATRIX matRotate;
	D3DXVECTOR3 vVec = Target->GetPos() - Me->GetPos();
	D3DXVECTOR3 vResult;
	vVec.y = 0.f;
	D3DXVec3Normalize( &vVec, &vVec );

	D3DXMatrixIdentity( &matRotate );
	D3DXMatrixRotationY( &matRotate, GetParamInt(4) * 3.1415926f / 180.f );
	D3DXVec3TransformNormal( &vVec, &vVec, &matRotate );

	vResult = Target->GetPos() + ( vVec * (float)GetParamInt(5) );
	Me->CmdMove( (int)vResult.x, (int)vResult.z, GetParamFloat(3), GetParamInt(0) | GetParamInt(1), GetParamInt(2) );
	return TRUE;
}

CFcAIActionExit::CFcAIActionExit()
{
}

int CFcAIActionExit::Command()
{
	return 0x000000fa;
}

CFcAIActionPushButton::CFcAIActionPushButton()
{
}

int CFcAIActionPushButton::Command()
{
	GameObjHandle Me = m_pParent->GetUnitHandle();
	if( Me->GetClassID() != CFcGameObject::Class_ID_Hero ) return FALSE;

	((CFcHeroObject*)Me.m_pInstance)->AIInput( GetParamInt(0) );
	return TRUE;
}

CFcAIActionReleaseButton::CFcAIActionReleaseButton()
{
}

int CFcAIActionReleaseButton::Command()
{
	GameObjHandle Me = m_pParent->GetUnitHandle();
	if( Me->GetClassID() != CFcGameObject::Class_ID_Hero ) return FALSE;

	((CFcHeroObject*)Me.m_pInstance)->AIInput( 0x80000000 + GetParamInt(0) );
	return TRUE;
}


CFcAIActionAddDefenseProb::CFcAIActionAddDefenseProb()
{
}

int CFcAIActionAddDefenseProb::Command()
{
	GameObjHandle Me = m_pParent->GetUnitHandle();
	Me->SetDefenseProbAdd( GetParamInt(0) );

	return TRUE;
}

CFcAIActionDestroyElement::CFcAIActionDestroyElement()
{
}

int CFcAIActionDestroyElement::Command()
{
	int nIndex = GetParamInt(0);
	return (( nIndex << 16 ) | (int)0x000000ff);
}

CFcAIActionLinkObject::CFcAIActionLinkObject()
{
}

int CFcAIActionLinkObject::Command()
{
	GameObjHandle Me = m_pParent->GetUnitHandle();
	GameObjHandle Target = m_pParent->GetTargetHandle();
//	if( !Target || !Me ) return FALSE;
	if( !CFcBaseObject::IsValid( Target ) || !CFcBaseObject::IsValid( Me ) ) return FALSE;

	Me->RideOn( Target );

	return TRUE;
}

CFcAIActionUnlinkObject::CFcAIActionUnlinkObject()
{
}

int CFcAIActionUnlinkObject::Command()
{
	GameObjHandle Me = m_pParent->GetUnitHandle();
//	if( !Me ) return FALSE;
	if( !CFcBaseObject::IsValid( Me ) ) return FALSE;

	Me->RideOut();

	return TRUE;
}

CFcAIActionSetArcherHitProb::CFcAIActionSetArcherHitProb()
{
}

int CFcAIActionSetArcherHitProb::Command()
{
	GameObjHandle Me = m_pParent->GetUnitHandle();
	if( !CFcBaseObject::IsValid( Me ) ) return FALSE;
	if( Me->GetClassID() != CFcGameObject::Class_ID_Archer ) return FALSE;

	((CFcArcherUnitObject*)Me.GetPointer())->SetHitProb( GetParamInt(0) );

	return TRUE;
}

CFcAIActionSetControlIndex::CFcAIActionSetControlIndex()
{
}

int CFcAIActionSetControlIndex::Command()
{
	GameObjHandle Me = m_pParent->GetUnitHandle();
	if( !CFcBaseObject::IsValid( Me ) ) return FALSE;

	Me->SetAIControlIndex( GetParamInt(0) );

	return TRUE;
}

CFcAIActionInfluenceDamageEmpty::CFcAIActionInfluenceDamageEmpty()
{
}

int CFcAIActionInfluenceDamageEmpty::Command()
{
	GameObjHandle Me = m_pParent->GetUnitHandle();
	GameObjHandle Handle = m_pParent->GetTargetHandle();

	if( !CFcBaseObject::IsValid( Me ) ) return false;
	if( !CFcBaseObject::IsValid( Handle ) ) return false;

	CFcCorrelationHit *pCorrelation = (CFcCorrelationHit *)Me->GetCorrelation( CFcCorrelation::CT_HIT );
	if( pCorrelation == NULL ) return false;

	CFcCorrelationHit::HitStruct *pHitStruct = pCorrelation->FindResultFromHandle( Handle );
	if( pHitStruct == NULL ) return false;

	pHitStruct->nDamage = 0;

	return TRUE;
}

CFcAIActionInfluenceHitCountEmpty::CFcAIActionInfluenceHitCountEmpty()
{
}

int CFcAIActionInfluenceHitCountEmpty::Command()
{
	GameObjHandle Me = m_pParent->GetUnitHandle();
	GameObjHandle Handle = m_pParent->GetTargetHandle();

	if( !CFcBaseObject::IsValid( Me ) ) return false;
	if( !CFcBaseObject::IsValid( Handle ) ) return false;

	CFcCorrelationHit *pCorrelation = (CFcCorrelationHit *)Me->GetCorrelation( CFcCorrelation::CT_HIT );
	if( pCorrelation == NULL ) return false;

	CFcCorrelationHit::HitStruct *pHitStruct = pCorrelation->FindResultFromHandle( Handle );
	if( pHitStruct == NULL ) return false;

	pHitStruct->nCount = 0;

	return TRUE;
}


CFcAIActionGlobalVariableIntRandom::CFcAIActionGlobalVariableIntRandom( int nValueCount )
{
	m_nValueCount = nValueCount;
	m_nValueIndex = -1;
}

int CFcAIActionGlobalVariableIntRandom::Command()
{
	int nRandom = Random(100);
	std::vector<int> nVecOffset;

	int nResult = 0;
	nVecOffset.push_back( 0 );
	for( int i=0; i<m_nValueCount-1; i++ ) {
		nResult += GetParamInt( 1 + m_nValueCount + i );
		nVecOffset.push_back( nResult );
	}
	nVecOffset.push_back( 100 );

	bool bDebugFlag = false;
	for( DWORD i=0; i<nVecOffset.size()-1; i++ ) {
		if( nRandom >= nVecOffset[i] && nRandom < nVecOffset[i+1] ) {
			nResult = GetParamInt( 1 + i );
			bDebugFlag = true;
			break;
		}
	}

	if( bDebugFlag == false ) assert(0);

	CFcAIGlobalVariableMng *pMng = m_pParent->GetParent()->GetGlobalVariableMng();
	if( m_nValueIndex == -1 ) {
		const char *szString = GetParamString(0);
		m_nValueIndex = pMng->GetVariableIndex( szString );
	}
	CFcParamVariable *pGlobalParam = pMng->GetVariable( m_nValueIndex );
// [PREFIX:beginmodify] 2006/2/25 junyash PS#5813 dereferencing NULL pointer
	BsAssert(pGlobalParam);
// [PREFIX:endmodify] junyash
	switch( pGlobalParam->GetType() ) {
		case CFcParamVariable::INT:		pGlobalParam->SetVariable( nResult );	break;
	}

	return TRUE;
}


CFcAIActionLookCustom::CFcAIActionLookCustom()
{
}

int CFcAIActionLookCustom::Command()
{
	GameObjHandle Me = m_pParent->GetUnitHandle();
	GameObjHandle Target = m_pParent->GetTargetHandle();

	if( !CFcBaseObject::IsValid( Me ) ) return TRUE;
	if( !CFcBaseObject::IsValid( Target ) ) return TRUE;

	D3DXVECTOR3 TargetPos = Target->GetPos();
	D3DXVECTOR3 Pos = Me->GetCrossVector()->GetPosition();

	D3DXVECTOR3 vMoveDir;
	vMoveDir.x = TargetPos.x - Pos.x;
	vMoveDir.y = 0.f;
	vMoveDir.z = TargetPos.z - Pos.z;
	D3DXVec3Normalize(&vMoveDir, &vMoveDir);
	
	D3DXMATRIX matRotate;
	D3DXMatrixIdentity( &matRotate );
	D3DXMatrixRotationY( &matRotate, GetParamInt(0) / 180.f * 3.1415926f );
	D3DXVec3TransformNormal( &vMoveDir, &vMoveDir, &matRotate );

	*Me->GetMoveDir() = vMoveDir;
	return TRUE;
}



CFcAIActionSelfCustomMove::CFcAIActionSelfCustomMove()
{
}

int CFcAIActionSelfCustomMove::Command()
{
	GameObjHandle Me = m_pParent->GetUnitHandle();
	if( !CFcBaseObject::IsValid( Me ) ) return TRUE;

	D3DXVECTOR3 vMoveDir = Me->GetCrossVector()->m_ZVector;
	D3DXMATRIX matRotate;
	D3DXMatrixIdentity( &matRotate );
	D3DXMatrixRotationY( &matRotate, GetParamInt(4) / 180.f * 3.1415926f );
	D3DXVec3TransformNormal( &vMoveDir, &vMoveDir, &matRotate );
	vMoveDir *= (float)GetParamInt(5);

	vMoveDir += Me->GetPos();
	Me->CmdMove( (int)vMoveDir.x, (int)vMoveDir.z, GetParamFloat(3), GetParamInt(0) | GetParamInt(1), GetParamInt(2) );


	return TRUE;
}


CFcAIActionDelayOrder::CFcAIActionDelayOrder() 
{
}


int CFcAIActionDelayOrder::Command()
{
	int nMaxDelay = GetParamInt(0);

	GameObjHandle hMe = m_pParent->GetUnitHandle();
	if(hMe->IsMove() || hMe->IsEnabledMoveDelay()==false)
		return TRUE;
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

// [PREFIX:beginmodify] 2006/2/16 junyash PS#5195 reports dividing by zero using 'farthest-nearest'
	//m_pParent->SetDelayTick( int( ((dis-nearest)/(farthest-nearest))*nMaxDelay) + RandomNumberInRange(0, 80) );
	// modify '(dis-nearest)/(farthest-nearest)' to 'if farthest-nearest == 0 then 0', because it seems '(dis-nearest) < (farthest-nearest)' and result is 0~1
	m_pParent->SetDelayTick( ( (farthest-nearest) ? int( ((dis-nearest)/(farthest-nearest))*nMaxDelay) : 0 ) + RandomNumberInRange(0, 80) );
// [PREFIX:endmodify] junyash
	hMe->EnableMoveDelay(false);
	return TRUE;
}

CFcAIActionPauseFunction::CFcAIActionPauseFunction()
{
	m_bFind = false;
}

int CFcAIActionPauseFunction::Command()
{
	if( m_bFind == false ) {
		m_pParent->GetParent()->GetFuncParamList( GetParamString(0), m_nVecFunctionList );
		m_bFind = true;
	}
	for( DWORD i=0; i<m_nVecFunctionList.size(); i++ ) {
		m_pParent->GetParent()->PauseFunction( m_nVecFunctionList[i], true );
	}
	return TRUE;
}

CFcAIActionUnpauseFunction::CFcAIActionUnpauseFunction()
{
	m_bFind = false;
}

int CFcAIActionUnpauseFunction::Command()
{
	if( m_bFind == false ) {
		m_pParent->GetParent()->GetFuncParamList( GetParamString(0), m_nVecFunctionList );
		m_bFind = true;
	}
	for( DWORD i=0; i<m_nVecFunctionList.size(); i++ ) {
		m_pParent->GetParent()->PauseFunction( m_nVecFunctionList[i], false );
	}
	return TRUE;
}


CFcAIActionWarp::CFcAIActionWarp() 
{
}


int CFcAIActionWarp::Command()
{
	int nDis = GetParamInt(0);

	GameObjHandle Me = m_pParent->GetUnitHandle();	
	if( !CFcBaseObject::IsValid( Me ) )
	{
		BsAssert(0);
		return TRUE;
	}
	GameObjHandle Target = m_pParent->GetTargetHandle();
	if( !CFcBaseObject::IsValid( Target ) )
	{
		BsAssert(0);
		return TRUE;
	}

	float fLength;
	D3DXVECTOR3 vVec;

	vVec = Target->GetDummyPos() - Me->GetDummyPos();
	vVec.y = 0.f;

	fLength = D3DXVec3Length( &vVec );
	D3DXVec3Normalize( &vVec, &vVec );


	Me->GetCrossVector()->m_PosVector += vVec * ( fLength + nDis );
	Me->CmdStop();
	
	return TRUE;
}

CFcAIActionDiminutionVelocity::CFcAIActionDiminutionVelocity()
{
}

int CFcAIActionDiminutionVelocity::Command()
{
	GameObjHandle Me = m_pParent->GetUnitHandle();	
	if( !CFcBaseObject::IsValid( Me ) ) return TRUE;

	D3DXVECTOR3 vVelocity = *Me->GetVelocity();
	vVelocity *= GetParamFloat(0);
	Me->SetVelocity( &vVelocity );

	return TRUE;
}

CFcAIActionResetMoveVector::CFcAIActionResetMoveVector()
{
}

int CFcAIActionResetMoveVector::Command()
{
	GameObjHandle Me = m_pParent->GetUnitHandle();	
	if( !CFcBaseObject::IsValid( Me ) ) return TRUE;

	Me->ResetMoveVector();
	return TRUE;
}