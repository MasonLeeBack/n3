#pragma once

#include "Singleton.h"
#ifndef _LTCG
#ifndef DO_PERFORMENCE_CHECK
	//#define DO_PERFORMENCE_CHECK
#endif
#endif //_LTCG



#define PERF_CHECK_FRAME 40

extern int g_nPerfCounter;

int RegisterPerfID(const char* lpFnName);
void StartPerfCheck(int nID);
void EndPerfCheck(int nID);
void ProfilePrint(void);
void OffsetPrefOffset();

class CPerfCheckInstance__Helper
{
public:
	CPerfCheckInstance__Helper(int nID)
	{
		m_nID = nID;
		StartPerfCheck(nID);
	}

	~CPerfCheckInstance__Helper()
	{
		EndPerfCheck(m_nID);
	}

private:
	int m_nID;
};


#ifdef DO_PERFORMENCE_CHECK

#define PROFILE_FUNC(fnname)									\
	static int __perfid__ = RegisterPerfID( #fnname );				\
	CPerfCheckInstance__Helper	perf_chekc_helper(__perfid__);

#define PROFILE_START(profilename)									\
	static int __perfid__##profilename = RegisterPerfID( #profilename );				\
	StartPerfCheck(__perfid__##profilename);

#define PROFILE_END(profilename)									\
	EndPerfCheck(__perfid__##profilename);
#else
#define PROFILE_FUNC(fnname) {}
#define PROFILE_START(profilename) {}
#define PROFILE_END(profilename) {}

#endif //DO_PERFORMENCE_CHECK



#define PROFILE_PRINT()		ProfilePrint()


#ifdef DO_PERFORMENCE_CHECK

#define PROFILE_TIME_TEST( func )	\
{								\
	static double fIncreaseCount = 0;	\
	static bool bOutput = false;	\
	static int nCallCount = 0;		\
	LARGE_INTEGER liFrequency;		\
	LARGE_INTEGER liStartTime;	\
	LARGE_INTEGER liCurTime;	\
	QueryPerformanceFrequency(&liFrequency);	\
	QueryPerformanceCounter(&liStartTime);	\
	func;		\
	nCallCount++;	\
	QueryPerformanceCounter(&liCurTime);	\
	if( g_nPerfCounter % PERF_CHECK_FRAME == 0 ) {	\
		if( bOutput == false ) {	\
			char szStr[256];	\
			sprintf( szStr, "Call Count : %d / %s Process Time Per Second (ms): %.2f\n", nCallCount, #func, fIncreaseCount );	\
			DebugString( szStr );	\
			fIncreaseCount = 0.f;	\
			nCallCount = 0;			\
			bOutput = true;		\
		}	\
	}	\
	else {	\
		double dwTime = (double)(( liCurTime.QuadPart - liStartTime.QuadPart ) / ( liFrequency.QuadPart / 100000.f ));	\
		fIncreaseCount += dwTime / 100.f;	\
		bOutput = false; \
	}	\
}

#define PROFILE_TIME_TEST_RETURN_INT( func )	\
{								\
	static float fIncreaseCount = 0;	\
	static bool bOutput = false;	\
	static int nCallCount = 0;		\
	LARGE_INTEGER liFrequency;		\
	LARGE_INTEGER liStartTime;	\
	LARGE_INTEGER liCurTime;	\
	QueryPerformanceFrequency(&liFrequency);	\
	QueryPerformanceCounter(&liStartTime);	\
	int Result = func;		\
	nCallCount++;			\
	QueryPerformanceCounter(&liCurTime);	\
	if( g_nPerfCounter % PERF_CHECK_FRAME == 0 ) {	\
		if( bOutput == false ) {	\
			char szStr[256];	\
			sprintf( szStr, "Call Count : %d / %s Process Time Per Second (ms): %.2f\n", nCallCount, #func, fIncreaseCount );	\
			DebugString( szStr );	\
			fIncreaseCount = 0.f;	\
			nCallCount = 0;			\
			bOutput = true;		\
		}	\
	}	\
	else {	\
		__int64 dwTime = (__int64 )(( liCurTime.QuadPart - liStartTime.QuadPart ) / ( liFrequency.QuadPart / 100000.f ));	\
		fIncreaseCount += dwTime / 100.f;	\
		bOutput = false; \
	}	\
	return Result;	\
}

#define PROFILE_TIME_TEST_RETURN_BOOL( func )	\
{								\
	static float fIncreaseCount = 0;	\
	static bool bOutput = false;	\
	static int nCallCount = 0;		\
	LARGE_INTEGER liFrequency;		\
	LARGE_INTEGER liStartTime;	\
	LARGE_INTEGER liCurTime;	\
	QueryPerformanceFrequency(&liFrequency);	\
	QueryPerformanceCounter(&liStartTime);	\
	bool Result = func;		\
	nCallCount++;			\
	QueryPerformanceCounter(&liCurTime);	\
	if( g_nPerfCounter % PERF_CHECK_FRAME == 0 ) {	\
	if( bOutput == false ) {	\
	char szStr[256];	\
	sprintf( szStr, "Call Count : %d / %s Process Time Per Second (ms): %.2f\n", nCallCount, #func, fIncreaseCount );	\
	DebugString( szStr );	\
	fIncreaseCount = 0.f;	\
	nCallCount = 0;			\
	bOutput = true;		\
	}	\
	}	\
	else {	\
	__int64 dwTime = (__int64 )(( liCurTime.QuadPart - liStartTime.QuadPart ) / ( liFrequency.QuadPart / 100000.f ));	\
	fIncreaseCount += dwTime / 100.f;	\
	bOutput = false; \
	}	\
	return Result;	\
}


#define PROFILE_TIME_TEST_BLOCK_START( string )	\
{								\
	static double fIncreaseCount = 0;	\
	static bool bOutput = false;	\
	static int nCallCount = 0;		\
	static char *szFunctionString;	\
	szFunctionString = string;		\
	LARGE_INTEGER liFrequency;		\
	LARGE_INTEGER liStartTime;	\
	LARGE_INTEGER liCurTime;	\
	QueryPerformanceFrequency(&liFrequency);	\
	QueryPerformanceCounter(&liStartTime);


#define PROFILE_TIME_TEST_BLOCK_END()	\
	nCallCount++;	\
	QueryPerformanceCounter(&liCurTime);	\
	if( g_nPerfCounter % PERF_CHECK_FRAME == 0 ) {	\
		if( bOutput == false ) {	\
			char szStr[256];	\
			sprintf( szStr, "Call Count : %d / %s Process Time Per Second (ms): %.2f\n", nCallCount, szFunctionString, fIncreaseCount );	\
			DebugString( szStr );	\
			fIncreaseCount = 0.f;	\
			nCallCount = 0;			\
			bOutput = true;		\
		}	\
	}	\
	else {	\
		double dwTime = (double)(( liCurTime.QuadPart - liStartTime.QuadPart ) / ( liFrequency.QuadPart / 100000.f ));	\
		fIncreaseCount += dwTime / 100.f;	\
		bOutput = false; \
	}	\
}

#define PROFILE_TICK_TEST_BLOCK_START( string )	\
{								\
	static char *szFunctionString;	\
	szFunctionString = string;		\
	LARGE_INTEGER liFrequency;		\
	LARGE_INTEGER liStartTime;	\
	LARGE_INTEGER liCurTime;	\
	QueryPerformanceFrequency(&liFrequency);	\
	QueryPerformanceCounter(&liStartTime);


#define PROFILE_TICK_TEST_BLOCK_END()	\
	QueryPerformanceCounter(&liCurTime);	\
	double dwTime = (double)(( liCurTime.QuadPart - liStartTime.QuadPart ) / ( liFrequency.QuadPart / 100000.f ));	\
	char szProfileTestStr[256];	\
	sprintf( szProfileTestStr, "Profiling Code Block / %s (ms): %.2f\n", szFunctionString, dwTime / 100.f );	\
	DebugString( szProfileTestStr );	\
}

#else

#define PROFILE_TIME_TEST( func )	\
{								\
	func;		\
}

#define PROFILE_TIME_TEST_BLOCK_START( string ) {}
#define PROFILE_TIME_TEST_BLOCK_END() {}
#define PROFILE_TICK_TEST_BLOCK_START( string ) {}
#define PROFILE_TICK_TEST_BLOCK_END() {}

#endif



class CProfileMemChecker : public CSingleton< CProfileMemChecker > {
public:
	CProfileMemChecker();
	virtual ~CProfileMemChecker();

protected:
	struct InfoStruct {
		std::string szString;
		DWORD dwUseMem;
		int nCallCount;
	};
	struct TempStruct {
		std::string szString;
		DWORD dwCurrentMem;
	};
	std::vector<InfoStruct> m_vecResult;
	std::vector<TempStruct> m_vecTemp;

	InfoStruct *FindResult( char *szStr );

public:
	void StartBlock( char *szString );
	void EndBlock( bool bPrintResult = true );
	void CancleBlock();
	void OutputResult( char *szString = NULL );
};
extern CProfileMemChecker g_MemChecker;;
