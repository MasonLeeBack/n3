#ifndef _DEBUG_UTIL
#define _DEBUG_UTIL


#ifdef WRITE_LIVE_LOG_FILE
	#define SetLogString _SetLogString
#else 
	#if _MSC_VER > 1200
		#define SetLogString __noop
	#else
		#define SetLogString 0
	#endif
#endif


void _SetLogString( const char* format, ... );


// float Exceptions
void EnableFloatExceptions();

// Timer
void StartTimeMark( const char* label );
int EndTimeMark();
int NextTimeMark( const char* label );

// Memory
void PrintMemoryStatus( char* s = NULL );
int GetCurrentMemoryUsage(unsigned int& totalMem, int& availMem);

// 이 클래스는 size용량 만큼만 저장한다. (그 이상 넘으면 앞에것은 무시)
class MemoryLogFile {
public:
	MemoryLogFile(int size);
	~MemoryLogFile();

	void LogString(const char* format, ...);
	void LogString(const char* format, va_list arg);
	bool WriteLogFile(char* fileName);

private:
	int m_size;

	unsigned char *m_logBuf[2];
	bool m_fullLogBuf;
	int m_bufIndex;
	int m_curBuf;
};

extern MemoryLogFile* g_logFile;

#endif