#include "StdAfx.h"
#ifdef _XBOX
	#include <xtl.h>
#else
	#include <windows.h>
	#include <float.h>
#endif

#include <stdio.h>
#include <assert.h>
#include "BsUtil.h"
#include "DebugUtil.h"


MemoryLogFile* g_logFile=NULL;

///////////////////////////////////////////////////////////////////////////////////////
// float Exception
// from Game Programming Gems 3 1.9
void EnableFloatExceptions()
{
	_fpreset();

	int i = _controlfp(0, 0);
	//i &= ~(EM_ZERODIVIDE | EM_OVERFLOW | EM_INVALID);
	i &= ~(EM_ZERODIVIDE | EM_OVERFLOW);
	_controlfp( i, MCW_EM );
}

///////////////////////////////////////////////////////////////////////////////////////
// Timer

static LARGE_INTEGER currentTime;

void StartTimeMark( const char* label )
{
#ifdef SHOW_OUTPUT_DEBUG_STRING
	DebugString( label );
//	currentTime = timeGetTime();		// 제논에서 컴파일 안되서 빼놓음

	QueryPerformanceCounter( &currentTime );

#endif
}


int EndTimeMark()
{
#ifdef SHOW_OUTPUT_DEBUG_STRING
	LARGE_INTEGER prevTime = currentTime;
//	currentTime = timeGetTime();		// 제논에서 컴파일 안되서 빼놓음

	LARGE_INTEGER TicksPerSecond;
	QueryPerformanceFrequency( &TicksPerSecond );
//	DOUBLE fTicksPerMicrosecond = (DOUBLE)TicksPerSecond.QuadPart * 0.000001;
	DOUBLE fTicksPerMillisecond = (DOUBLE)TicksPerSecond.QuadPart * 0.001;

	QueryPerformanceCounter( &currentTime );

	__int64 Ticks = (currentTime.QuadPart - prevTime.QuadPart) / (__int64)fTicksPerMillisecond;
	DebugString( ": %d ms\n", (DWORD)Ticks );
	return (int)Ticks;
#endif
	return 0;
}

int NextTimeMark( const char* label )
{
#ifdef SHOW_OUTPUT_DEBUG_STRING
	int nTick = EndTimeMark();
	DebugString( label );
	return nTick;
#endif
	return 0;
}


///////////////////////////////////////////////////////////////////////////////////////
// Memory
void PrintMemoryStatus( char *s )
{
#ifdef USE_DEBUG_TOOLS
	unsigned int total;
	int available;
	int used = GetCurrentMemoryUsage( total, available );

	if ( s == NULL )
		DebugString( "Memory : used %d KB\n", s, used );
	else
		DebugString( "%s: used %d KB\n", s, used );
#endif
}

int GetCurrentMemoryUsage(unsigned int& totalMem, int& availMem)
{
#ifdef USE_DEBUG_TOOLS
	MEMORYSTATUS	memStatus;

	GlobalMemoryStatus(&memStatus);

#ifdef _XBOX
	totalMem = 512 * 1024;
	if ( totalMem < memStatus.dwAvailPhys / 1024 )
		totalMem *= 2;
#else
	totalMem = memStatus.dwTotalPhys / 1024;
#endif

	availMem = memStatus.dwAvailPhys / 1024;

	return totalMem - availMem;
#else
	return 0;
#endif
}

///////////////////////////////////////////////////////////////////////////////////////
// Memory log file

MemoryLogFile::MemoryLogFile(int size)
{
	m_size=size;
	m_logBuf[0]=new unsigned char[m_size];
	m_logBuf[1]=new unsigned char[m_size];
	m_fullLogBuf=false;
	m_bufIndex=0;
	m_curBuf=0;
}

MemoryLogFile::~MemoryLogFile()
{
	delete[] m_logBuf[0];
	delete[] m_logBuf[1];
}


void MemoryLogFile::LogString(const char* format, ...)
{
	va_list arg;
	va_start(arg, format);
	LogString( format, arg );
	va_end(arg);
	
}

void MemoryLogFile::LogString( const char* format, va_list arg )
{
	char buf[1024];
	_vsnprintf( buf, 1024, format, arg );
	int length=strlen(buf);
	if((m_bufIndex+length)>=m_size) {
		m_fullLogBuf=true;
		int tailLength=m_size-m_bufIndex;
		BsAssert(tailLength>=0);
		memcpy(m_logBuf[m_curBuf]+m_bufIndex, buf, tailLength);

		if(m_curBuf==0)	{ m_curBuf=1; }
		else			{ m_curBuf=0; }

		memcpy(m_logBuf[m_curBuf], buf+tailLength, length-tailLength);
		m_bufIndex=length-tailLength;
	}
	else {
		memcpy(m_logBuf[m_curBuf]+m_bufIndex, buf, length);
		m_bufIndex+=length;
	}
}

bool MemoryLogFile::WriteLogFile(char* fileName)
{
	HANDLE file=CreateFile(fileName,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
	if(file==INVALID_HANDLE_VALUE) {
		BsAssert(0 && "log파일열기실패");
		return false;
	}
	DWORD wValue;
	if(m_fullLogBuf) {
		if(m_curBuf==0) {
			WriteFile(file, m_logBuf[1], m_size, &wValue, NULL);
		}
		else {
			WriteFile(file, m_logBuf[0], m_size, &wValue, NULL);
		}
	}
	WriteFile(file, m_logBuf[m_curBuf], m_bufIndex, &wValue, NULL);
	CloseHandle(file);
	return true;
}


void _SetLogString(const char* format, ...)
{
	if(g_logFile==NULL) {
		return;
	}
	va_list	arg;
	va_start( arg, format );
	g_logFile->LogString( format, arg );
	va_end( arg );
}




