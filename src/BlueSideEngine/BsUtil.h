#pragma once
#include <vector>
using namespace std;
class NxMat34;

// It must delete for commercial Version
//#define _RETAIL_VERSION	
#ifndef _RETAIL_VERSION
#define _USE_CONSOLE_COMMAND_	// Always enable Debug command..
#endif // _RETAIL_VERSION

#ifndef _LTCG
#define USE_DEBUG_TOOLS	// 이것끄면 DebugOutputString 안 나온다.
#endif // _LTCG

#ifdef USE_DEBUG_TOOLS
#define WRITE_LIVE_LOG_FILE
#define USE_DEBUG_MENU
#define USE_DEBUG_MENU_IN_GAME
#define USE_ASSERT_IN_RELEASE
#define SHOW_OUTPUT_DEBUG_STRING
#define ON_SCREEN_ASSERT
#define _LOAD_MAP_CHECK_
//#define ENABLE_MEM_CHECKER

#ifndef _RESLISTGEN
  // build target not RESLISTGEN
  #ifndef _USAGE_TOOL_
  //	#define ENABLE_RES_CHECKER
  #endif
  //#define MAKE_ALL_MISSION_RESLIST		// Run all levels and make resource list automatically
  //#define _USE_CONSOLE_COMMAND_
#else // ifndef _RESLISTGEN
  // build target RESLISTGEN
  #define ENABLE_RES_CHECKER
  #define MAKE_ALL_MISSION_RESLIST		// Run all levels and make resource list automatically
#endif // ifndef _RESLISTGEN


#define DebugString _DebugString
#else 
#if _MSC_VER > 1200
#define DebugString __noop
#else
#define DebugString 0
#endif
#endif



void ConvertMatrix( NxMat34 &NxMat, D3DXMATRIX DxMat, float fScale = 0.01f);
void ConvertMatrix( D3DXMATRIX &DxMat, NxMat34 &NxMat, float fScale = 100.f);
void SplitDirAndFileName(char* src, char* dir, size_t dir_size, char* fn, size_t fn_size);

class BMemoryStream;
int ExtractLine( const char * pStream, char *cLineBuffer, int maxCount);
int ExtractTabLine( const char * pStream, char *cLineBuffer, int maxCount);
double GetPerformanceTime();
bool IsFileExist(LPCSTR szFilePath);
int GetFileTime( LPCSTR szFilePath );
void _DebugString( const char* format, ... );


// Make Packed Normalized 3D value 
#define MakePacked_10_11_11(x,y,z) ((x) + ((y)<<10) + ((z)<<21))
#define MakePacked_11_11_10(x,y,z) ((x) + ((y)<<11) + ((z)<<22))

#ifdef USE_ASSERT_IN_RELEASE
	#ifdef _DEBUG 
		#define BsAssert(exp)			assert(exp)
		#define BsAssert2(exp, str)	assert(exp)
	#else
		#ifdef _D3D8PERF
			#define BsAssert(exp)		((exp) ? (void)(0):_DebugString("Assert!\n\t%s, Line:%d, File:%s\n",#exp, __LINE__, __FILE__))
			#define BsAssert2(exp, str)	((exp) ? (void)(0):_DebugString("Assert!\n\t%s, Line:%d, File:%s\n",#exp, __LINE__, __FILE__))
		#else
			#define ON_SCREEN_ASSERT
			#ifdef PROFILE		// release i
				#define BsAssert(exp)		if(exp) (void)(0); else __debugbreak(); /* if(exp) (void)(0); else { sprintf( _assertStr, "%s Line:%d File%s", #exp, __LINE__, __FILE__); _OnScreenAssert(); } */
				#define BsAssert2(exp, str)	if(exp) (void)(0); else __debugbreak(); /* if(exp) (void)(0); else { sprintf( _assertStr, "%s Line:%d File%s %s", #exp, __LINE__, __FILE__, str); _OnScreenAssert(); } */
			#else
				#define BsAssert(exp)		if(exp) (void)(0); else __debugbreak();
				#define BsAssert2(exp, str)	if(exp) (void)(0); else __debugbreak();
			#endif

		#endif
	#endif
#else
	#define BsAssert(exp)		((void)0)
	#define BsAssert2(exp, str)	((void)0)
#endif


#ifdef ON_SCREEN_ASSERT
extern bool g_bAssertScreenCapture;
extern char _assertStr[512];
extern void _OnScreenAssert();
#endif






// Check Available Physical Memory Difference
#define SCOPENAME_STRINGSIZE	128
class AvailMemoryDifference {
public:
	AvailMemoryDifference(int nIndex=0, char* pScopeName=NULL);
	~AvailMemoryDifference();
	unsigned long GetAvailMemDifference();
	static unsigned long GetTotalAvailMemory();
protected:
	int				m_nIndex;
	char			m_szScopeName[SCOPENAME_STRINGSIZE];
	unsigned long	m_dwStartAvailMemory;
	LARGE_INTEGER	m_Start;
};


struct MemUsageInfo
{
	char cStr[256];
	unsigned long	dwStartAvailMemory;
};

class CBsMemChecker
{
public:
	CBsMemChecker();
	~CBsMemChecker();

	void Start( char* pName );
	void End();

protected:
	std::vector<MemUsageInfo> m_vecInfo;
	HANDLE m_hFile;
};


enum RES_GROUP_ID
{
	RES_GROUP_NON = -1,
	RES_GROUP_FIRST_INIT,
	RES_GROUP_TITLE,
	RES_GROUP_RES1,				// prop, prop info, map info
	RES_GROUP_RES2,				// weapon, ai, char info
	RES_GROUP_CHAR,				// char group use folder list
	RES_GROUP_RES3,			// particle, fx, weapon;;, prop physics info
	RES_GROUP_AFTER_GAME,
	RES_GROUP_NUM
};

struct ResCheckerListData
{
	std::vector<std::string> m_vecRes;
	std::vector<std::string> m_vecTex;
};

class CBsResChecker
{
public:
	CBsResChecker();
	~CBsResChecker();

	void Create( char* pFileName );
	void SetGroup( RES_GROUP_ID GroupID )	{ m_vecGroupID.push_back( GroupID ); m_CurID = GroupID; }
	void PopGroup();
	RES_GROUP_ID GetGroupID()				{ return m_CurID; }
	void AddResList( char* pName );

	void EnableWriteOnlyFolder( bool bEnable );
	void AddUseFolder( char* pFolder );			// Write only folder name if it is below pFolder
	bool AddResFolderList( char* pName );

	void AddIgnoreFolderList( char* pFolder );
	void ClearIgnoreFolderList();

	void AddIgnoreFileList( char* pFile );
	void ClearIgnoreFileList();

	void CloseFile();
	void SkipNextFile()	{ m_bSkipNextFile = true; }

protected:
	bool IsRegisteredFile( char* pStr, bool bTex );
	bool IsIgnoreFile( char* pFile );
	bool IsRegisteredFolder( char* pStr );

protected:
	std::vector<std::string> m_vecInfo;
	HANDLE m_hOthFile[RES_GROUP_NUM];
	HANDLE m_hTexFile[RES_GROUP_NUM];

	RES_GROUP_ID m_CurID;	

	bool m_bEnableFolder;
	bool m_bSkipNextFile;

	std::vector<std::string> m_vecUseFolder;
	std::vector<std::string> m_vecAlreadyWroteFolder;

	std::vector<std::string> m_vecIgnoreFolder;
	std::vector<std::string> m_vecIgnoreFile;

	std::vector<RES_GROUP_ID> m_vecGroupID;

	ResCheckerListData m_ResCheckerListData[RES_GROUP_NUM];
};

class CBsCriticalSection
{
public:
	CBsCriticalSection()
	{
		InitializeCriticalSection( &cs );
	}
	virtual ~CBsCriticalSection()
	{
		DeleteCriticalSection( &cs );
	}
	inline void enter()
	{
		EnterCriticalSection( &cs );
	}
	inline void leave()
	{
		LeaveCriticalSection( &cs );
	}
private:
	CRITICAL_SECTION cs;
};

class CBsAutoLock
{
public:
	CBsAutoLock( CBsCriticalSection* pCS )
	{
		pCS->enter();
		m_pCS = pCS;
	}

	~CBsAutoLock()
	{
		m_pCS->leave();		
	}
protected:
	CBsCriticalSection* m_pCS;
};

#ifdef _USAGE_TOOL_
#define THREAD_AUTOLOCK( SyncObject )
#else
#define THREAD_AUTOLOCK( SyncObject ) CBsAutoLock __auto_lock( SyncObject )
#endif


extern CBsMemChecker g_BsMemChecker;
extern CBsResChecker g_BsResChecker;
