#include "StdAfx.h"
#include "FcAITriggerActionProp.h"
#include "FcProp.h"
#include "FcAIElement.h"
#include "FcParamVariable.h"
#include "FcAISearchSlot.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG



CFcAIActionFollowProp::CFcAIActionFollowProp()
{
}

int CFcAIActionFollowProp::Command()
{
	GameObjHandle Me = m_pParent->GetUnitHandle();
	CFcProp *pProp = (CFcProp *)m_pParent->GetTargetPointer();
	if( !pProp ) return TRUE;

	D3DXVECTOR3 vVec, vPos;
	// ÀÌ»óÇÑ°Ô ÀÕÀ¸¸é ²À »©¶ó~
	vVec = pProp->GetPos() - Me->GetDummyPos();
	vVec.y = 0.f;
	if( D3DXVec3Length( &vVec ) < (float)GetParamInt(3) ) {
		Me->CmdStop( Me->GetCurAniAttr() | ANI_TYPE_STAND );
		return TRUE;
	}
	/////////////

	vPos = pProp->GetPos();
	vPos.y = 0.f;
	vVec.y = 0.f;

	D3DXVec3Normalize( &vVec, &vVec );

	vVec *= (float)GetParamInt(3);
	vPos -= vVec;

	Me->CmdMove( (int)vPos.x, (int)vPos.z, GetParamFloat(4), GetParamInt(0) | GetParamInt(1), GetParamInt(2) );

	return TRUE;
}

CFcAIActionSetTargetSlotProp::CFcAIActionSetTargetSlotProp()
{
}

int CFcAIActionSetTargetSlotProp::Command()
{
	CFcProp *pProp = (CFcProp *)m_pParent->GetTargetPointer();
	if( !pProp ) return TRUE;

	m_pParent->GetParent()->GetSearchSlot()->SetPtr( GetParamInt(0), pProp );

	return TRUE;
}
