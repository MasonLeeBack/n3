#include "stdafx.h"
#include "FcFXBase.h"
#include "FcGlobal.h"



#include "BsKernel.h"
#include "FcWorld.h"
#include "FcHeroObject.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG

DWORD CFcFXBase::s_dwFxTimer = 0;

CFcFXBase::CFcFXBase()
{
	m_state = READY;
	m_DeviceDataState= FXDS_READY;
	m_nPriority = 0;
#ifndef _LTCG
	SetFxRtti(-1);
#endif //_LTCG
}


CFcFXBase::~CFcFXBase()
{
}

int CFcFXBase::ProcessMessage(int nCode,DWORD dwParam1, DWORD dwParam2,DWORD dwParam3)
{
	switch(nCode){
	case FX_DELETE_OBJECT:
#ifndef _LTCG
		if( dwParam1 )
		{
			BsAssert( GetFxRtti() == (int)dwParam1 );
		}
#endif //_LTCG
		m_state=END;
		m_DeviceDataState=FXDS_RELEASEREADY;
		return 1;

	case FX_GET_STATE:
		*((DWORD*)dwParam1) = m_state;
		return 1;
#ifndef _LTCG
	case FX_GET_RTTI:
		*((DWORD*)dwParam1) = GetFxRtti();
		return 1;
#endif //_LTCG

	}
	return 0;
}

void CFcFXBase::PreRender()
{
	switch(m_DeviceDataState)
	{
//		case FXDS_READY:
		case FXDS_INITREADY:
			InitDeviceData();
			m_DeviceDataState = FXDS_INITED;
			break;
		case FXDS_INITED: 
			break;

		case FXDS_RELEASEREADY: 
			ReleaseDeviceData();
			m_DeviceDataState = FXDS_RELEASED;
		
		case FXDS_RELEASED:
			break;

	}
	return;
}


bool	CFcFXBase::IsFinished()
{ 
	if(	m_state == END && m_DeviceDataState == FXDS_RELEASED  )
		return true;
	return false;
}

