#include "stdafx.h"

#include "FcAbilityBase.h"
#include "FcCommon.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG


CFcAbilityBase::CFcAbilityBase( GameObjHandle hParent )
{
	m_hParent = hParent;
	m_bFinished = false;
	SetRtti( -1 );
	m_nOrbGenType = GENERATE_ORB_DISABLE;
	m_nOrbGenPer = 100;
}

void CFcAbilityBase::SetOrbGen( int nGenType, int nGetPer )
{
	m_nOrbGenType = nGenType;
	m_nOrbGenPer = nGetPer;
}

CFcAbilityBase::~CFcAbilityBase()
{

}

float TimeValueTable::GetVaule(DWORD nTime)
{
	if(!m_nTable)
		return 0.f;

	if( nTime < m_pTable[0].nFrame)
		return m_pTable[0].fFactor;

	int ii= 0;
	for(; ii < m_nTable-1 ;++ii)
	{
		if(nTime <  m_pTable[ii+1].nFrame )
		{	
			break;
		}
	}

	if(ii >=  m_nTable-1 )
		return m_pTable[m_nTable-1].fFactor;

	int nElapse = nTime - m_pTable[ii].nFrame;
	int nTotal = m_pTable[ii+1].nFrame - m_pTable[ii].nFrame;

	float fA = m_pTable[ii].fFactor;
	float fB = m_pTable[ii+1].fFactor;

	float fResult = fA+ (fB-fA)*float(nElapse)/float(nTotal);
	return fResult;
}