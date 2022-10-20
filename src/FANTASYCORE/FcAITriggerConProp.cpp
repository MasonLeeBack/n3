#include "StdAfx.h"
#include "FcAITriggerConProp.h"
#include "FcProp.h"
#include "FcAIElement.h"
#include "FcParamVariable.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG



CFcAIConTargetPropRange::CFcAIConTargetPropRange()
{
}

bool CFcAIConTargetPropRange::CheckCondition()
{

	int nRange = GetParamInt(0);
	int nOp = GetParamInt(1);

	GameObjHandle Me = m_pParent->GetUnitHandle();
	CFcProp *pProp = (CFcProp *)m_pParent->GetTargetPointer();
	if( !pProp ) return false;

	D3DXVECTOR3 Vec = Me->GetDummyPos() - pProp->GetPos();
	Vec.y = 0.f;
	float fDist = (float)D3DXVec3Length( &Vec );

	return Compare( fDist, (float)nRange, nOp );
}


CFcAIConTargetPropValid::CFcAIConTargetPropValid( BOOL bValid )
{
	m_bValid = bValid;
}

bool CFcAIConTargetPropValid::CheckCondition()
{
	CFcProp *pProp = (CFcProp *)m_pParent->GetTargetPointer();
	if( !pProp ) return ( m_bValid == TRUE ) ? false : true ;
	return ( m_bValid == TRUE ) ? true : false;
}

CFcAIConTargetPropType::CFcAIConTargetPropType()
{
}

bool CFcAIConTargetPropType::CheckCondition()
{
	CFcProp *pProp = (CFcProp *)m_pParent->GetTargetPointer();
	if( !pProp ) return false;

	return Compare( pProp->GetType(), GetParamInt(0), GetParamInt(1) );
}
