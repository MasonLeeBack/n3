#include "stdafx.h"
#include "FrameChecker.h"

CFrameChecker::CFrameChecker()
{
	m_bStart=false;
	m_dwStartTime=0;
	m_nTotlaFrame=0;
	m_fFps=0.0f;
}

CFrameChecker::~CFrameChecker()
{
}

void CFrameChecker::Start()
{
	m_bStart=true;
	m_nTotlaFrame=0;
	m_dwStartTime=GetTickCount();
}

void CFrameChecker::End()
{
	m_bStart=false;
}

float CFrameChecker::GetFPS()
{
	return m_fFps;
}

const char *CFrameChecker::GetFPSString()
{
	static char szFPS[255];

	sprintf(szFPS, "FPS : %.2f\n", m_fFps);

	return szFPS;
}

void CFrameChecker::AddFrame(int nCount)
{
	DWORD dwCurTime;

	dwCurTime=GetTickCount();
	m_nTotlaFrame+=nCount;
	if(dwCurTime-m_dwStartTime>1000){
		m_fFps=m_nTotlaFrame*1000/(float)(dwCurTime-m_dwStartTime);
		m_dwStartTime=dwCurTime;
		m_nTotlaFrame=0;
	}
}