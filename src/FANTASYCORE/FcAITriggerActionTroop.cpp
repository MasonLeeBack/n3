#include "StdAfx.h"
#include "FcAITriggerActionTroop.h"
#include "FcGameObject.h"
#include "FcAIObject.h"
#include "FcParamVariable.h"
#include "FcAIElement.h"
#include "FcAIGlobalVariableMng.h"
#include "FcTroopObject.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG

CFcAIActionTroopFollow::CFcAIActionTroopFollow()
{
}

int CFcAIActionTroopFollow::Command()
{
	GameObjHandle Me = m_pParent->GetUnitHandle();
	D3DXVECTOR2 vPos = Me->GetTroopOffset();
	D3DXVECTOR2 vTarget = Me->GetParentTroop()->GetPosV2();

	vTarget += vPos;

	Me->CmdMove( (int)vTarget.x, (int)vTarget.y, GetParamFloat(3), GetParamInt(0) | GetParamInt(1), GetParamInt(2) );
	/*
	D3DXVECTOR3 vResult;
	GameObjHandle Me = m_pParent->GetUnitHandle();

	if( Me->GetTroopMovePos( vResult ) == false ) {
		D3DXVECTOR2 vPos = Me->GetTroopOffset();
		vResult = Me->GetParentTroop()->GetPos();

		vResult.x += vPos.x;
		vResult.z += vPos.y;

		Me->SetTroopMoveQueueCount( -1 );
	}
	else {
		if( Me->IsUseTroopMoveQueue() == false )
			Me->SetTroopMoveQueueCount( 0 );
	}
	Me->CmdMove( (int)vResult.x, (int)vResult.z, GetParamFloat(3), GetParamInt(0) | GetParamInt(1), GetParamInt(2) );
	*/
	return TRUE;
}