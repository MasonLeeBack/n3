#include "StdAfx.h"
#include "FcAITriggerConTroop.h"
#include "FcGameObject.h"
#include "FcTroopObject.h"
#include "FcParamVariable.h"
#include "FcWorld.h"
#include "FcHeroObject.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG

int GetProcessTick();

CFcAIConTroopState::CFcAIConTroopState()
{
}

bool CFcAIConTroopState::CheckCondition()
{
	GameObjHandle Me = m_pParent->GetUnitHandle();
	TroopObjHandle Troop = Me->GetParentTroop();
	return Compare( Troop->GetState(), GetParamInt(0), GetParamInt(1) );
}

CFcAIConTroopOffsetRange::CFcAIConTroopOffsetRange( BOOL bTarget )
{
	m_bTarget = bTarget;
}

bool CFcAIConTroopOffsetRange::CheckCondition()
{
	GameObjHandle Handle;
	GameObjHandle Me = m_pParent->GetUnitHandle();

	if( m_bTarget == TRUE )
		Handle = m_pParent->GetTargetHandle();
	else Handle = m_pParent->GetUnitHandle();
	if( !CFcBaseObject::IsValid( Handle ) ) return false;

	D3DXVECTOR2 vPos = Me->GetTroopOffset();
	D3DXVECTOR2 vTarget = Me->GetParentTroop()->GetPosV2();

	vTarget += vPos;
	vTarget -= D3DXVECTOR2(Handle->GetDummyPos().x, Handle->GetDummyPos().z);

	float fDist = D3DXVec2Length( &vTarget );

	int nLength = GetParamInt(0);
	return Compare( (int)fDist, nLength, GetParamInt(1) );
}

CFcAICoTroopStateTable::CFcAICoTroopStateTable()
{
}

bool CFcAICoTroopStateTable::CheckCondition()
{
	GameObjHandle Me = m_pParent->GetUnitHandle();
	TroopObjHandle Troop = Me->GetParentTroop();

	int nCurState = CFcTroopObject::s_nTroopStateTable[ Troop->GetState() ];
	switch( GetParamInt(1) ) {
		case AI_OP_EQUAL:
			if( nCurState & GetParamInt(0) ) return true;
			return false;
		case AI_OP_NOT_EQUAL:
			if( nCurState & GetParamInt(0) ) return false;
			return true;
		case AI_OP_GREATER:
		case AI_OP_GREATER_THAN_OR_EQUAL:
		case AI_OP_LESS_THAN:
		case AI_OP_LESS_THAN_OR_EQUAL:
			return Compare( nCurState, GetParamInt(0), GetParamInt(1) );
	}
	return false;
}

CFcAIConTroopInRange::CFcAIConTroopInRange( BOOL bTarget )
{
	m_bTarget = bTarget;
}

bool CFcAIConTroopInRange::CheckCondition()
{
	GameObjHandle Handle;
	if( m_bTarget == TRUE ) Handle = m_pParent->GetTargetHandle();
	else Handle = m_pParent->GetUnitHandle();

	GameObjHandle Me = m_pParent->GetUnitHandle();
	if( !CFcBaseObject::IsValid( Handle ) ) return !GetParamBool(0);

	if( Me->GetParentTroop()->GetType() == TROOPTYPE_RANGEMELEE && CFcTroopObject::s_nTroopStateTable[ Me->GetParentTroop()->GetState() ] & UNIT_RANGE ) {
		D3DXVECTOR2 vPos = Handle->GetParentTroop()->GetPosV2() - Me->GetParentTroop()->GetPosV2();

		float fRadius = Me->GetParentTroop()->GetAttackRadius();
		if( D3DXVec2LengthSq( &vPos ) < fRadius * fRadius ) return GetParamBool(0);
		return !GetParamBool(0);
	}
	else {
		D3DXVECTOR2 vCenter = Me->GetParentTroop()->GetPosV2();
		D3DXVECTOR2 vPos = Handle->GetDummyPosV2();

		float fRadius = Me->GetParentTroop()->GetRadius();
		float fLength = D3DXVec2LengthSq( (D3DXVECTOR2*)&( vCenter - vPos ) );
		if( fLength < fRadius * fRadius ) return GetParamBool(0);
		return !GetParamBool(0);
	}
	return false;
}

CFcAIConTroopSubState::CFcAIConTroopSubState()
{
}

bool CFcAIConTroopSubState::CheckCondition()
{
	GameObjHandle Me = m_pParent->GetUnitHandle();
	TroopObjHandle Troop = Me->GetParentTroop();
	return Compare( Troop->GetSubState(), GetParamInt(0), GetParamInt(1) );
}

CFcAIConTroopTargetTroopRange::CFcAIConTroopTargetTroopRange()
{
}

bool CFcAIConTroopTargetTroopRange::CheckCondition()
{
	GameObjHandle Handle = m_pParent->GetTargetHandle();
	GameObjHandle Me = m_pParent->GetUnitHandle();
	if( !CFcBaseObject::IsValid( Handle ) ) return false;
	if( !CFcBaseObject::IsValid( Me ) ) return false;

	D3DXVECTOR2 vVec = Handle->GetTroop()->GetPosV2() - Me->GetPosV2();
	float fLength = D3DXVec2LengthSq( &vVec );
	float fValue = (float)GetParamInt(0);

	return Compare( fLength, fValue * fValue, GetParamInt(1) );
}

CFcAIConTroopTargetInTroop::CFcAIConTroopTargetInTroop()
{
}

bool CFcAIConTroopTargetInTroop::CheckCondition()
{
	GameObjHandle Handle = m_pParent->GetTargetHandle();
	GameObjHandle Me = m_pParent->GetUnitHandle();

	if( !CFcBaseObject::IsValid( Handle ) ) return !GetParamBool(0);
	if( !CFcBaseObject::IsValid( Me ) ) return !GetParamBool(0);

	TroopObjHandle hTroop = Me->GetTroop()->GetTargetTroop();
	if( !hTroop ) return !GetParamBool(0);

	bool bFind = false;
	for( int i=0; i<hTroop->GetUnitCount(); i++ ) {
		if( hTroop->GetUnit(i) == Handle ) {
			bFind = true;
			break;
		}
	}
	if( bFind == true ) return GetParamBool(0);
	else return !GetParamBool(0);
}

CFcAIConTroopIsUntouchable::CFcAIConTroopIsUntouchable()
{
}

bool CFcAIConTroopIsUntouchable::CheckCondition()
{
	GameObjHandle Me = m_pParent->GetUnitHandle();

	if( !CFcBaseObject::IsValid( Me ) ) return !GetParamBool(0);

	TroopObjHandle hTroop = Me->GetTroop()->GetTargetTroop();
	if( !hTroop ) return !GetParamBool(0);

	if( hTroop->IsUntouchable() ) return GetParamBool(0);
	else return !GetParamBool(0);
}

CFcAIConTroopReadyRangeOrder::CFcAIConTroopReadyRangeOrder()
{
}

bool CFcAIConTroopReadyRangeOrder::CheckCondition()
{
	GameObjHandle Me = m_pParent->GetUnitHandle();

	if( !CFcBaseObject::IsValid( Me ) ) return false;

	TroopObjHandle hTroop = Me->GetTroop();
	if( !hTroop ) return false;

	bool bUpdate = false;
	if( GetProcessTick() - hTroop->GetUnitsNearbyPlayerCheckTick() >= GetParamInt(2) ) bUpdate = true;

	int nTemp;
	std::vector<GameObjHandle> VecList;
	hTroop->GetUnitsNearbyPlayer( &nTemp, VecList, bUpdate );

	for( DWORD i=0; i<VecList.size(); i++ ) {
		if( VecList[i] == Me ) {
			return Compare( (int)i, GetParamInt(0), GetParamInt(1) );
		}
	}

	return false;
}


CFcAIConTroopTargetIsPlayer::CFcAIConTroopTargetIsPlayer()
{
}

bool CFcAIConTroopTargetIsPlayer::CheckCondition()
{
	GameObjHandle Me = m_pParent->GetUnitHandle();

	if( !CFcBaseObject::IsValid( Me ) ) return !GetParamBool(0);

	TroopObjHandle hTroop = Me->GetTroop()->GetTargetTroop();
	if( !hTroop ) return !GetParamBool(0);

	if( hTroop == g_FcWorld.GetHeroHandle()->GetTroop() ) return GetParamBool(0);

	return !GetParamBool(0);
}
