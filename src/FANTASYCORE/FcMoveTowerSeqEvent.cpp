#include "StdAfx.h"
#include "FcMoveTowerSeqEvent.h"
#include "FcEventSequencer.h"
#include "FcMoveTowerObject.h"
#include "FcGlobal.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG


CFcMoveTowerSeqGetOffEvent::CFcMoveTowerSeqGetOffEvent( CFcEventSequencerMng *pMng )
: CFcEventSequencerElement( pMng )
{
	m_bResetEvent = false;
	m_nFlag = 0;
	m_nCount = 0;
}

CFcMoveTowerSeqGetOffEvent::~CFcMoveTowerSeqGetOffEvent()
{
}

void CFcMoveTowerSeqGetOffEvent::Run()
{
	CFcMoveTowerObject *pObject = (CFcMoveTowerObject *)m_pMng->GetVoid();

	switch( m_nFlag ) {
		case 0:
			m_nTick = GetProcessTick();
			m_nFlag = 1;
			break;
		case 1:
			if( ( GetProcessTick() - m_nTick ) % FRAME_PER_SEC == 0 ) {
				DWORD dwCount = pObject->GetLinkObjCount();
				if( dwCount == 0 ) {
					m_nFlag = -1;
					break;
				}

				GameObjHandle Handle = pObject->GetLinkObjHandle(0);
				Handle->RideOut();
			}
			break;
	}
}

bool CFcMoveTowerSeqGetOffEvent::IsDestroy()
{
	if( m_nFlag == -1 ) return true;
	return false;
}
