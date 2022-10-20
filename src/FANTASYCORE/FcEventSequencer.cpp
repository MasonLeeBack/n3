#include "StdAfx.h"
#include "FcEventSequencer.h"
#include "BsCommon.h"
#include "FcGlobal.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG



CFcEventSequencerMng::CFcEventSequencerMng( void *pVoid )
{
	m_pVoid = pVoid;
	m_nResetTime = GetProcessTick();

}

CFcEventSequencerMng::~CFcEventSequencerMng()
{
	Reset();
}

void CFcEventSequencerMng::Reset()
{
	m_nResetTime = GetProcessTick();
	for( DWORD i=0; i<m_pVecList.size(); i++ ) {
		if( m_pVecList[i]->IsResetEvent() ) {
			delete m_pVecList[i];
			m_pVecList.erase( m_pVecList.begin() + i );
			--i;
		}
	}
//	SAFE_DELETE_PVEC( m_pVecList );
}

void *CFcEventSequencerMng::GetVoid()
{
	return m_pVoid;
}

DWORD CFcEventSequencerMng::GetEventCount()
{
	return m_pVecList.size();
}

void CFcEventSequencerMng::Process()
{
	int nCurTime = GetProcessTick() - m_nResetTime;
	for( DWORD i=0; i<m_pVecList.size(); i++ ) {
		if( nCurTime == m_pVecList[i]->GetTime() || m_pVecList[i]->IsRun() ) {
			m_pVecList[i]->Run();
			if( m_pVecList[i]->IsDestroy() ) {
				delete m_pVecList[i];
				m_pVecList.erase( m_pVecList.begin() + i );
				i--;
				continue;
			}
			return;
		}
	}
}

void CFcEventSequencerMng::AddEvent( CFcEventSequencerElement *pEvent )
{
	m_pVecList.push_back( pEvent );
}

CFcEventSequencerElement::CFcEventSequencerElement( CFcEventSequencerMng *pMng )
{
	m_pMng = pMng;
	m_nTime = -1;
	m_bResetEvent = true;
}

CFcEventSequencerElement::~CFcEventSequencerElement()
{
}

int CFcEventSequencerElement::GetTime()
{
	return m_nTime;
}

bool CFcEventSequencerElement::IsRun()
{
	return true;
}


bool CFcEventSequencerElement::IsDestroy()
{
	return true;
}


void CFcEventSequencerElement::Run()
{
}

bool CFcEventSequencerElement::IsResetEvent()
{
	return m_bResetEvent;
}