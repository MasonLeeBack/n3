#include "StdAfx.h"
#include "PerfCheck.h"
#include "BsUtil.h"

#include <string>
#include <vector>
#include <algorithm>

using namespace std;

int g_nPerfCounter = 0;

typedef struct tagPERFDATA
{
	double	fMinCounter;
	double	fMaxCounter;
	double	fTotalCounter;

	LARGE_INTEGER	prevCounter;
	DWORD	nCountHits;

	// offset 용
	double fFpsTotalCounter;
	DWORD dwFpsCountHits;
	double fAveFpsTotalCounter;
	DWORD dwAveFpsCountHits;

	int nPrevPrefCounter;

	string  strName;	

} PERFDATA, *LPPERFDATA;

static bool PerfCmp(const LPPERFDATA lhs, const LPPERFDATA rhs)
{
	return lhs->fMaxCounter > rhs->fMaxCounter;
}

class CPerfCheck
{
public:
	CPerfCheck()
	{
		m_vecPerfDataPtr.reserve(64);
	}

	~CPerfCheck()
	{
		OutputPerfInfo();

		vector<LPPERFDATA>::iterator it = m_vecPerfDataPtr.begin();
		vector<LPPERFDATA>::iterator itend = m_vecPerfDataPtr.end();

		for (; it != itend; it++)
			delete (*it);
	}

	void OutputPerfInfo()
	{

		char buf[256];

		vector<LPPERFDATA> vecPerfDataPtr;

		vecPerfDataPtr.assign(m_vecPerfDataPtr.begin(), m_vecPerfDataPtr.end());

		vector<LPPERFDATA>::iterator it = vecPerfDataPtr.begin();
		vector<LPPERFDATA>::iterator itend = vecPerfDataPtr.end();

		sort(it, itend, PerfCmp);

		DebugString("\n\n-----  Profiling Result Start -----\n\n");
		DebugString("      [ Func Name ]      Call Count          MAX(ms)          MIN(ms)          AVG(ms)\n\n");

		LARGE_INTEGER lFreq;		
		QueryPerformanceFrequency(&lFreq);

		double fFreq = double(lFreq.QuadPart) / 1000.;

		for (; it != itend; it++)
		{
			LPPERFDATA lpPerf = *it;
			
			// 기본적인 프로파일링 결과
			sprintf(buf, "%-25s    %6d,          %6.4lf,          %6.4lf,          %6.4lf \n", lpPerf->strName.c_str(), lpPerf->nCountHits,
				lpPerf->fMaxCounter / fFreq,
				lpPerf->fMinCounter / fFreq,
				lpPerf->fTotalCounter / fFreq / double(lpPerf->nCountHits)
			);

			DebugString(buf);

			// 초당 계산 결과
			sprintf(buf, "   Frame Per Result -> Call : ( %6d ),      Profiling : ( %6.4lf )\n\n", lpPerf->dwAveFpsCountHits,
				lpPerf->fAveFpsTotalCounter / fFreq / double(lpPerf->dwAveFpsCountHits) );

			DebugString(buf);
		}


		DebugString("\n-----  Profiling Result End -----\n\n");
		
	}

	int RegisterPerfID(const char* lpFnName)
	{
		LPPERFDATA pPerfData = new PERFDATA;
		
		pPerfData->fMinCounter = 100000000.;
		pPerfData->fMaxCounter = 0.0;
		pPerfData->fTotalCounter = 0.0;
		pPerfData->prevCounter.QuadPart = 0;

		pPerfData->nCountHits = 0;
		pPerfData->strName = lpFnName;

		pPerfData->fFpsTotalCounter = 0.0f;
		pPerfData->dwFpsCountHits = 0;
		pPerfData->fAveFpsTotalCounter = 0.f;
		pPerfData->dwAveFpsCountHits = 0;
		pPerfData->nPrevPrefCounter = 0;

		m_vecPerfDataPtr.push_back(pPerfData);
		
		return m_vecPerfDataPtr.size() - 1;
	}


	void StartPerfCheck(int nID)
	{		
		QueryPerformanceCounter(&m_vecPerfDataPtr[nID]->prevCounter);
	}

	void EndPerfCheck(int nID)
	{
		LARGE_INTEGER currCount;
		QueryPerformanceCounter(&currCount);
		
		LPPERFDATA pPerf = m_vecPerfDataPtr[nID];		
		double fDiffCount;

		fDiffCount = double(currCount.QuadPart - pPerf->prevCounter.QuadPart);

		if (pPerf->fMinCounter > fDiffCount)
			pPerf->fMinCounter = fDiffCount;
		else if (pPerf->fMaxCounter < fDiffCount)
			pPerf->fMaxCounter = fDiffCount;

		pPerf->fTotalCounter += fDiffCount;
		pPerf->nCountHits++;

		if( g_nPerfCounter != pPerf->nPrevPrefCounter ) {
			pPerf->fAveFpsTotalCounter += pPerf->fFpsTotalCounter;
			pPerf->fAveFpsTotalCounter /= 2.f;
			pPerf->dwAveFpsCountHits++;

			pPerf->fFpsTotalCounter += 0;
			pPerf->dwFpsCountHits = 0;

			pPerf->nPrevPrefCounter = g_nPerfCounter;
		}
		else {
			pPerf->fFpsTotalCounter += fDiffCount;
			pPerf->dwFpsCountHits++;
		}
	}

protected:

	vector<LPPERFDATA> m_vecPerfDataPtr;

};



static CPerfCheck	s_PerfCheck;



int RegisterPerfID(const char* lpFnName)
{
	return s_PerfCheck.RegisterPerfID(lpFnName);
}

void StartPerfCheck(int nID)
{
	s_PerfCheck.StartPerfCheck(nID);
}

void EndPerfCheck(int nID)
{
	s_PerfCheck.EndPerfCheck(nID);
}


void ProfilePrint(void)
{
	s_PerfCheck.OutputPerfInfo();
}

CProfileMemChecker::CProfileMemChecker()
{
}

CProfileMemChecker::~CProfileMemChecker()
{
	OutputResult( NULL );
}

void CProfileMemChecker::StartBlock( char *szString )
{
	MEMORYSTATUS memInfo;
	GlobalMemoryStatus(&memInfo);

	TempStruct Block;

	Block.szString = szString;
	Block.dwCurrentMem = memInfo.dwAvailPhys;
	m_vecTemp.push_back( Block );
}

void CProfileMemChecker::EndBlock( bool bPrintResult )
{
	if( m_vecTemp.size() == 0 ) return;

	MEMORYSTATUS memInfo;
	GlobalMemoryStatus(&memInfo);

	TempStruct Block = m_vecTemp[ m_vecTemp.size() - 1 ];

	DWORD dwUseMem = Block.dwCurrentMem - memInfo.dwAvailPhys;

	InfoStruct *pInfo = NULL;

	pInfo = FindResult( (char*)Block.szString.c_str() );
	if( pInfo ) {
		pInfo->dwUseMem += dwUseMem;
		pInfo->nCallCount++;
	}
	else {
		InfoStruct Info;
		Info.szString = Block.szString;
		Info.dwUseMem = dwUseMem;
		Info.nCallCount = 1;
		m_vecResult.push_back( Info );
	}
	m_vecTemp.erase( m_vecTemp.end() - 1 );

	if( bPrintResult ) {
		OutputResult( (char*)Block.szString.c_str() );
	}
}

void CProfileMemChecker::CancleBlock()
{
	m_vecTemp.erase( m_vecTemp.end() - 1 );
}

void CProfileMemChecker::OutputResult( char *szString )
{
	if( szString ) {
		InfoStruct *pInfo = FindResult( szString );
		if( pInfo )
			DebugString( "Mem Checker : [ %s ], Use Mem : %d bytes\n", pInfo->szString.c_str(), pInfo->dwUseMem );
	}
	else {
		DebugString( "\n--------------------------------- Memory Use Result --------------------------------------\n" );
		DebugString( "\nFunction Name\t\t\t\t\tCall Count\t\tTotal Allocate memory ( byte )\n" );
		for( DWORD i=0; i<m_vecResult.size(); i++ ) {
			InfoStruct *pInfo = &m_vecResult[i];
			DebugString( "[ %s ]      Call : %d           Use %d bytes\n", pInfo->szString.c_str(), pInfo->nCallCount, pInfo->dwUseMem );
		}
	}
}

CProfileMemChecker::InfoStruct *CProfileMemChecker::FindResult( char *szStr )
{
	for( DWORD i=0; i<m_vecResult.size(); i++ ) {
		if( strcmp( m_vecResult[i].szString.c_str(), szStr ) == NULL ) return &m_vecResult[i];
	}
	return NULL;
}

