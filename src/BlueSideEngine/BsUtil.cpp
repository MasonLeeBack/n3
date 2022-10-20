#include "StdAfx.h"

#include "CrossVector.h"
#include "BsUtil.h"
#include "bstreamext.h"
#include "BsKernel.h"
#include <io.h>


bool g_bAssertScreenCapture = false;
char _assertStr[512] = "";

CBsMemChecker g_BsMemChecker;
CBsResChecker g_BsResChecker;


void ConvertMatrix( NxMat34 &NxMat, D3DXMATRIX DxMat, float fScale )
{
	DxMat._41 *= fScale;
	DxMat._42 *= fScale;
	DxMat._43 *= fScale;
	NxMat.setColumnMajor44( (NxF32*)&DxMat );
}

void ConvertMatrix( D3DXMATRIX &DxMat, NxMat34 &NxMat, float fScale )
{
	NxMat.getColumnMajor44( (NxF32*)DxMat );
	DxMat._41 *= fScale;
	DxMat._42 *= fScale;
	DxMat._43 *= fScale;
}


int ExtractLine( const char * pStream, char *cLineBuffer, int maxCount)
{
	BsAssert(pStream);
	BsAssert(cLineBuffer);

	int cCount = 0;

	while(cCount < maxCount)
	{
		char c = *pStream++;
		if(c == NULL)
		{ // eos
			break;
		}
		else if(c == 0x0d || c == 0x0a || c == '\n' )
		{ // eol
			if(c == 0x0d && *pStream == 0x0a)
				cCount += 2;
			else
				cCount++;
			break;
		}
		else
		{ // copy char
			*cLineBuffer++ = c;
			cCount++;
		}
	}

	*cLineBuffer = NULL;
	return cCount;
}


int ExtractTabLine( const char * pStream, char *cLineBuffer, int maxCount)
{
	BsAssert(pStream);
	BsAssert(cLineBuffer);

	int cCount = 0;

	while(cCount < maxCount)
	{
		char c = *pStream++;
		if(c == NULL)
		{ // eos
			break;
		}
		else if(c == '\t' || c == 0x0d || c == 0x0a || c == '\n' )
		{ // eol
			if(c == 0x0d && *pStream == 0x0a)
				cCount += 2;
			else
				cCount++;
			break;
		}
		else
		{ // copy char
			*cLineBuffer++ = c;
			cCount++;
		}
	}

	*cLineBuffer = NULL;
	return cCount;
}


void SplitDirAndFileName(char* src, char* dir, size_t dir_size, char* fn, size_t fn_size)
{
	BsAssert( dir && dir_size > 0 );
	BsAssert( fn && fn_size > 0 );

	dir[0] = 0;
	fn[0] = 0;

	char * pSlash = strrchr( src, '\\' );

	if ( pSlash )
	{
		++ pSlash;
#ifdef _XBOX
		strncpy_s( dir, dir_size, src, pSlash - src ); //aleksger - removed extraneous #defines as there are string safe defines
#else
		strcpy_s( dir, dir_size, src);
		dir[pSlash - src] = '\0';
#endif
		strcpy_s( fn, fn_size, pSlash );
	}
}


double GetPerformanceTime()
{
	LARGE_INTEGER liTime;
	QueryPerformanceCounter(&liTime);
	const double TickForXenon = 50000.0;
	double dwTime = (double)(( liTime.QuadPart ) / TickForXenon);
	return dwTime;
}

bool IsFileExist(LPCSTR szFilePath)
{
	if (!_access((szFilePath), 00))
		return true;
	return false;
}

int GetFileTime( LPCSTR szFilePath ) 
{	
	HANDLE hFile = CBsFileIO::BsCreateFile(szFilePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if( hFile == INVALID_HANDLE_VALUE ) return -1;	
	FILETIME fTime;
	GetFileTime( hFile, NULL, NULL, &fTime);
	CBsFileIO::BsCloseFileHandle( hFile );
	//FILETIME LocalFileTime;
	//FileTimeToLocalFileTime( &fTime, &LocalFileTime );
	SYSTEMTIME LocalSysTime;
	FileTimeToSystemTime( &fTime, &LocalSysTime );

	int nFileTime = 0;
	nFileTime = 0;
	nFileTime += LocalSysTime.wDay * (3600 * 24);
	nFileTime += LocalSysTime.wHour * 3600;
	nFileTime += LocalSysTime.wMinute * 60;
	nFileTime += LocalSysTime.wSecond;

	return nFileTime;
}
///////////////////////////////////////////////////////////////////////////////////////
// DebugString

#ifndef _LTCG
void _DebugStringV( const char* format, va_list pArg )
{
	char buf[1024];
	_vsnprintf( buf, 1024, format, pArg );
	printf(buf);
}

void _DebugString( const char* format, ... )
{
	char buf[1024];

	va_list	arg;
	va_start( arg, format );
	_vsnprintf( buf, 1024, format, arg );
	va_end( arg );
	printf(buf);
	//	g_BsKernel.AddConsoleString( buf );
}

void _DebugString( const char* str )
{
	printf(str);
}
#else
void _DebugStringV( const char* format, va_list pArg ){};
void _DebugString( const char* format, ... ){};
void _DebugString( const char* str ){};
#endif //#ifndef _LTCG

#ifdef ON_SCREEN_ASSERT

void _OnScreenAssert() 
{
	if( _assertStr[0] == NULL )
		return;

	//	throw("Error!!");				// Release모드에서 커널디버거 쓸때...
	const int LINE_LENGTH = 40;
	char errorLine[LINE_LENGTH + 1];
	errorLine[LINE_LENGTH] = 0;

	const char* errorStr = _assertStr;
	int length = strlen(errorStr);

	int y = 150;
	while( length > 0 )
	{
		DebugString( "Assert: %s\n", _assertStr );

		memcpy( errorLine, errorStr, LINE_LENGTH );
//		strncpy( errorLine, errorStr, LINE_LENGTH );
		
		errorStr += LINE_LENGTH;
		length -= LINE_LENGTH;

		g_BsKernel.PrintString( 60, y, errorLine );
		y += 20;
	}

	g_BsKernel.PrintString( 60, y+40, "Waiting for 5 sec." );
	g_BsKernel.PrintString( 60, y+60, "Please capture this screen." );

	g_BsKernel.GetDevice()->ClearTarget( D3DCOLOR_COLORVALUE(0.0f, 0.0f, 0.0f, 1.0f) );
	g_BsKernel.RenderText();

	bool bSaved = g_bAssertScreenCapture;
	g_bAssertScreenCapture=true;
	g_BsKernel.AssertCaptureScreen();
	g_bAssertScreenCapture=bSaved;
	g_BsKernel.GetDevice()->ShowFrame();

	Sleep(10000);

	_assertStr[0] = NULL;
}
#endif


///////////////////////////////////////////////////////////////////////////////////////
// Check Available Physical Memory Difference
AvailMemoryDifference::AvailMemoryDifference(int nIndex/*=0*/, char* pszScopeName/*=NULL*/)
{
#ifdef USE_DEBUG_TOOLS	
	memset(m_szScopeName, 0, SCOPENAME_STRINGSIZE );
	if(pszScopeName)
		strcpy(m_szScopeName, pszScopeName );

	DebugString("-%s-Scope Start\n", m_szScopeName);

	QueryPerformanceCounter( &m_Start );
	m_nIndex=nIndex;
	MEMORYSTATUS memInfo;
	GlobalMemoryStatus(&memInfo);
	m_dwStartAvailMemory=memInfo.dwAvailPhys;
#endif
}

unsigned long AvailMemoryDifference::GetAvailMemDifference()
{
#ifdef USE_DEBUG_TOOLS
	MEMORYSTATUS memInfo;
	GlobalMemoryStatus(&memInfo);
	unsigned long dwEndAvailMemory=memInfo.dwAvailPhys;
	unsigned long dwDifference = m_dwStartAvailMemory-dwEndAvailMemory;
	DebugString("-%s-Scope Current Using : Memory Difference of Index %d : %d byte\n",
		m_szScopeName, m_nIndex, dwDifference);
	return dwDifference;
#else
	return 0;
#endif
}

unsigned long AvailMemoryDifference::GetTotalAvailMemory()
{
#ifdef USE_DEBUG_TOOLS
	MEMORYSTATUS memInfo;
	GlobalMemoryStatus(&memInfo);
	DebugString("Now Total Available Physics Memory : %d byte\n", memInfo.dwAvailPhys);
	return memInfo.dwAvailPhys;
#else
	return 0;
#endif
}


AvailMemoryDifference::~AvailMemoryDifference()
{
#ifdef USE_DEBUG_TOOLS	
	LARGE_INTEGER TicksPerSecond;
	QueryPerformanceFrequency( &TicksPerSecond );
	LARGE_INTEGER Current;
	QueryPerformanceCounter( &Current );
	float fTicks = float(Current.QuadPart - m_Start.QuadPart) / float(TicksPerSecond.QuadPart);
	DebugString("-%s-Scope Elapsed Time : %5.5f second\n", m_szScopeName, fTicks);

	MEMORYSTATUS memInfo;
	GlobalMemoryStatus(&memInfo);
	unsigned long dwEndAvailMemory=memInfo.dwAvailPhys;
	DebugString("-%s-Scope finished : Difference of Index %d : %d byte\n\t\t\t\t\t\t\t\t\t\t\tTotal Memory In use : %dK\n",
		m_szScopeName, m_nIndex, m_dwStartAvailMemory-dwEndAvailMemory, (memInfo.dwTotalPhys-dwEndAvailMemory)/1024);

#endif
}






CBsMemChecker::CBsMemChecker()
{
#ifdef ENABLE_MEM_CHECKER
	m_hFile=CBsFileIO::BsCreateFile("d:\\Mem.txt",GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
#endif
}

CBsMemChecker::~CBsMemChecker()
{
#ifdef ENABLE_MEM_CHECKER
//	End();
	CBsFileIO::BsCloseFileHandle( m_hFile );
#ifdef _USAGE_TOOL_
	// Check 
	CBsFileIO::DestroyPackedFileHandlerInstance();
#endif

#endif
}

void CBsMemChecker::Start( char* pName )
{
#ifdef ENABLE_MEM_CHECKER
	MemUsageInfo Info;
	strcpy( Info.cStr, pName );
	MEMORYSTATUS memInfo;
	GlobalMemoryStatus(&memInfo);
	Info.dwStartAvailMemory = memInfo.dwAvailPhys;
	m_vecInfo.push_back( Info );

	char cStr[MAX_PATH] = "";
	char cStr2[MAX_PATH] = "";
	int nCnt = m_vecInfo.size() - 1;
	for( int i=0; i<nCnt; i++ )
		strcat( cStr, "\t" );

	sprintf( cStr2, "%s CurMem: %d {\r\n", pName, Info.dwStartAvailMemory/1024 );
	strcat( cStr, cStr2 );

	DWORD wValue;
	int nSize = strlen( cStr ) + 1;
	CBsFileIO::BsWriteFile( m_hFile, cStr, nSize, &wValue, NULL);
#endif
}

void CBsMemChecker::End()
{
#ifdef ENABLE_MEM_CHECKER
	if( m_vecInfo.size() <= 0 )
	{
		return;
	}
	MemUsageInfo* pInfo = &( m_vecInfo[m_vecInfo.size()-1] );	

	MEMORYSTATUS memInfo;
	GlobalMemoryStatus(&memInfo);

	char cStr[MAX_PATH] = "";
	char cStr2[MAX_PATH] = "";
	int nCnt = m_vecInfo.size() - 1;
	for( int i=0; i<nCnt; i++ )
		strcat( cStr, "\t" );

	unsigned long Size;
	if( pInfo->dwStartAvailMemory > memInfo.dwAvailPhys )
	{
		Size = pInfo->dwStartAvailMemory - memInfo.dwAvailPhys;
	}
	else
	{
		Size = memInfo.dwAvailPhys - pInfo->dwStartAvailMemory;
	}

	sprintf( cStr2, "} %s CurMem:%d DiffMem:%d\r\n", 
		pInfo->cStr,
		memInfo.dwAvailPhys/1024, 
		Size/1024 );

	strcat( cStr, cStr2 );

	DWORD wValue;
	int nSize = strlen( cStr ) + 1;
	CBsFileIO::BsWriteFile( m_hFile, cStr, nSize, &wValue, NULL);

	m_vecInfo.pop_back();
#endif
}



// CBsResChecker class
CBsResChecker::CBsResChecker()
{
#ifdef ENABLE_RES_CHECKER
	for( int i=0; i<RES_GROUP_NUM; i++ )
		m_hOthFile[i] = m_hTexFile[i] = INVALID_HANDLE_VALUE;

	// First init
	char cStr[MAX_PATH];
	sprintf( cStr, "%s", "d:\\Res_Init.txt" );
	m_hOthFile[RES_GROUP_FIRST_INIT]=CBsFileIO::BsCreateFile( cStr,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL );

	sprintf( cStr, "%s", "d:\\Tex_Init.txt" );
	m_hTexFile[RES_GROUP_FIRST_INIT]=CBsFileIO::BsCreateFile( cStr,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL );

	// Title
	sprintf( cStr, "%s", "d:\\Res_Title.txt" );
	m_hOthFile[RES_GROUP_TITLE]=CBsFileIO::BsCreateFile( cStr,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL );

	sprintf( cStr, "%s", "d:\\Tex_Title.txt" );
	m_hTexFile[RES_GROUP_TITLE]=CBsFileIO::BsCreateFile( cStr,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL );

	m_bEnableFolder = false;
	m_bSkipNextFile = false;
#endif
}

CBsResChecker::~CBsResChecker()
{
#ifdef ENABLE_RES_CHECKER
	//	End();
	CloseFile();
	m_vecUseFolder.clear();
	m_vecAlreadyWroteFolder.clear();

#ifdef _USAGE_TOOL_
	// Check 
	CBsFileIO::DestroyPackedFileHandlerInstance();
#endif

#endif
}

// call it into level
void CBsResChecker::Create( char* pFileName )
{
#ifdef ENABLE_RES_CHECKER
	char cStr[MAX_PATH];
	char* cTempStr;

	CloseFile();

	char cMissionName[32];
	strcpy( cMissionName, pFileName );
	cTempStr = strstr( cMissionName, "." );
	if( cTempStr )
		cTempStr[0] = NULL;

	sprintf( cStr, "d:\\%s_Res1_Res.txt", cMissionName );
	m_hOthFile[RES_GROUP_RES1]=CBsFileIO::BsCreateFile( cStr,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL );
	sprintf( cStr, "d:\\%s_Res1_Tex.txt", cMissionName );
	m_hTexFile[RES_GROUP_RES1]=CBsFileIO::BsCreateFile( cStr,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL );

	sprintf( cStr, "d:\\%s_Res2_Res.txt", cMissionName );
	m_hOthFile[RES_GROUP_RES2]=CBsFileIO::BsCreateFile( cStr,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL );
	sprintf( cStr, "d:\\%s_Res2_Tex.txt", cMissionName );
	m_hTexFile[RES_GROUP_RES2]=CBsFileIO::BsCreateFile( cStr,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL );

	sprintf( cStr, "d:\\%s_Char_Res.txt", cMissionName );
	m_hOthFile[RES_GROUP_CHAR]=CBsFileIO::BsCreateFile( cStr,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL );
	sprintf( cStr, "d:\\%s_Char_Tex.txt", cMissionName );
	m_hTexFile[RES_GROUP_CHAR]=CBsFileIO::BsCreateFile( cStr,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL );

	sprintf( cStr, "d:\\%s_Res3_Res.txt", cMissionName );
	m_hOthFile[RES_GROUP_RES3]=CBsFileIO::BsCreateFile( cStr,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL );
	sprintf( cStr, "d:\\%s_Res3_Tex.txt", cMissionName );
	m_hTexFile[RES_GROUP_RES3]=CBsFileIO::BsCreateFile( cStr,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL );

	// After game
	sprintf( cStr, "d:\\%s_AfterGame_Res.txt", cMissionName );
	m_hOthFile[RES_GROUP_AFTER_GAME]=CBsFileIO::BsCreateFile( cStr,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL );
	sprintf( cStr, "d:\\%s_AfterGame_Tex.txt", cMissionName );
	m_hTexFile[RES_GROUP_AFTER_GAME]=CBsFileIO::BsCreateFile( cStr,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL );

	for( int i=0; i<RES_GROUP_NUM; i++ )
	{
		m_ResCheckerListData[i].m_vecRes.clear();
		m_ResCheckerListData[i].m_vecTex.clear();
	}
#endif
}

void CBsResChecker::PopGroup()
{
	m_vecGroupID.pop_back();
	int nSize = m_vecGroupID.size();
	if( nSize != 0 )
	{
		m_CurID = m_vecGroupID[ nSize-1 ];
	}
	else
	{
		m_CurID = RES_GROUP_NON;
	}
}


void CBsResChecker::AddResList( char* pName )
{
#ifdef ENABLE_RES_CHECKER

	if(m_bSkipNextFile)
	{
		m_bSkipNextFile = false;
		return;
	}

	char szUpper[MAX_PATH];
	strcpy(szUpper, pName);
	_strupr(szUpper);

	if( m_bEnableFolder )
	{
		if( AddResFolderList( szUpper ) == true )
			return;
	}

	if( m_CurID == RES_GROUP_NON )
	{
		DebugString( "Cannot packed file : %s\n", szUpper );
		return;
	}

	if( m_hTexFile[m_CurID] == INVALID_HANDLE_VALUE )
		return;
 
	DWORD wValue;
	char* tempStr = szUpper + 3;

	// Test if valid file exists in build...
	if(CBsFileIO::BsGetFileSize( szUpper, &wValue) != S_OK)
		return;

	char szFolder[MAX_PATH];
	char szFile[MAX_PATH];
	char szStr[MAX_PATH];

	SplitDirAndFileName( szUpper, szFolder, MAX_PATH, szFile, MAX_PATH );

	int nCnt = m_vecIgnoreFolder.size();
	for( int i=0; i<nCnt; i++ )
	{
		if( strcmp( szFolder, m_vecIgnoreFolder[i].c_str() ) == 0 )
			return;
	}

	sprintf( szStr, "%s\r\n", tempStr );
	if(strstr(szStr, ".DDS") || strstr(szStr, ".36T") )
	{	// convert file extension to compressed
		char *pExt = strstr(szStr, ".");
		memset(pExt, 0, MAX_PATH - (pExt - szStr));
		strncat(pExt,".36T\r\n", 6);

		if( IsRegisteredFile( szFile, true ) == false && IsIgnoreFile( szFile ) == false )
		{
			m_ResCheckerListData[m_CurID].m_vecTex.push_back( szFile );
			CBsFileIO::BsWriteFile( m_hTexFile[m_CurID], szStr, strlen(szStr), &wValue, NULL);
		}
	}
	else
	{
		if( IsRegisteredFile( szFile, false ) == false && IsIgnoreFile( szFile ) == false )
		{
			m_ResCheckerListData[m_CurID].m_vecRes.push_back( szFile );
			CBsFileIO::BsWriteFile( m_hOthFile[m_CurID], szStr, strlen(szStr), &wValue, NULL);
		}
	}
#endif
}

void CBsResChecker::EnableWriteOnlyFolder( bool bEnable )
{
	if( m_bEnableFolder != bEnable )
	{
		if(!bEnable)
		{	// Flush folder contents to texture and object files

			std::vector<std::string> m_vecAddList;

			int nCnt = m_vecAlreadyWroteFolder.size();
			for( int i=0; i<nCnt; i++ )
			{		
				WIN32_FIND_DATA w32FD;
				HANDLE hSearch;
				char szSrcPath[MAX_PATH];

				strncpy_s(szSrcPath, MAX_PATH, m_vecAlreadyWroteFolder[i].c_str(), _TRUNCATE);
				strncat(szSrcPath, "*", 3);

				hSearch = FindFirstFile(szSrcPath, &w32FD);

				if ( hSearch == INVALID_HANDLE_VALUE) continue;

				do
				{
					if (w32FD.dwFileAttributes != FILE_ATTRIBUTE_DIRECTORY)
					{
						DWORD wValue;
						char szFullPath[MAX_PATH];
						sprintf(szFullPath, "%s%s", m_vecAlreadyWroteFolder[i].c_str(), w32FD.cFileName);
						_strupr(szFullPath);
						char* tempStr = szFullPath + 3;
						if(strstr(tempStr, "TXT") || strstr(tempStr, "SCC"))
							continue;

						sprintf( szFullPath, "%s\r\n", tempStr );

						if(strstr(szFullPath, ".DDS"))
						{	//special handling for billboards
							if(strstr(szFullPath,"BILL_"))
							{
								if(!strstr(szFullPath, "BILL_ (0)"))
									continue;

								// Change the name of the file to use
								// the folder name so it is unique...
								char *pFolderName=(char *)strrchr(szFullPath, '\\');
								*pFolderName= 0;
								pFolderName=(char *)strrchr(szFullPath, '\\');
								char szFolderName[MAX_PATH];
								strcpy(szFolderName, pFolderName);
								strcat(szFullPath, szFolderName);
								strcat(szFullPath, ".");
							}
							char *pExt = strstr(szFullPath, ".");
							memset(pExt, 0, MAX_PATH - (pExt - szFullPath));
							strncat(pExt,".36T\r\n", 6);

							bool bExist = false;
							char szFolder[MAX_PATH];
							char szFile[MAX_PATH];
							SplitDirAndFileName( szFullPath, szFolder, MAX_PATH, szFile, MAX_PATH );
							int nCnt = m_vecAddList.size();
							for( int i=0; i<nCnt; i++ )
							{
								if( strcmp( m_vecAddList[i].c_str(), szFile ) == 0 )
								{
									bExist = true;
									break;
								}
							}

							if( bExist == false && IsIgnoreFile( szFile ) == false )
							{
								m_vecAddList.push_back( szFile );
								CBsFileIO::BsWriteFile( m_hTexFile[m_CurID], szFullPath, strlen(szFullPath), &wValue, NULL);
							}
						}
						else if(!strstr(szFullPath, ".36T"))
						{
							bool bExist = false;
							char szFolder[MAX_PATH];
							char szFile[MAX_PATH];
							SplitDirAndFileName( szFullPath, szFolder, MAX_PATH, szFile, MAX_PATH );
							int nCnt = m_vecAddList.size();
							for( int i=0; i<nCnt; i++ )
							{
								if( strcmp( m_vecAddList[i].c_str(), szFile ) == 0 )
								{
									bExist = true;
									break;
								}
							}
							if( bExist == false && IsIgnoreFile( szFile ) == false )
							{
								m_vecAddList.push_back( szFile );
								CBsFileIO::BsWriteFile( m_hOthFile[m_CurID], szFullPath, strlen(szFullPath), &wValue, NULL);
							}
						}
					}
				} while (FindNextFile(hSearch, &w32FD));
			}
		}

		m_vecUseFolder.clear();
		m_vecAlreadyWroteFolder.clear();
		m_bEnableFolder = bEnable;
	}
}

void CBsResChecker::AddUseFolder( char* pFolder )
{
#ifdef ENABLE_RES_CHECKER
	char cTempFolder[MAX_PATH];
	strcpy( cTempFolder, pFolder );
	_strupr( cTempFolder );
	std::string Str( cTempFolder );
	m_vecUseFolder.push_back( Str );
#endif
}

bool CBsResChecker::AddResFolderList( char* pName )
{
	char szName[MAX_PATH];
	bool bUserFolder = false;
	int nCnt = m_vecUseFolder.size();
	
	strcpy_s(szName, MAX_PATH, pName);
	_strupr( szName );

	for( int i=0; i<nCnt; i++ )
	{
		char szFolder[MAX_PATH];

		strcpy_s(szFolder, MAX_PATH, m_vecUseFolder[i].c_str());
		_strupr( szFolder );
		
		if( strstr( szName, szFolder) != NULL )
		{
			bUserFolder = true;
			break;
		}
	}
	if( bUserFolder )
	{
		char cFolder[MAX_PATH];
		char cFile[MAX_PATH];
		SplitDirAndFileName( szName, cFolder, MAX_PATH, cFile, MAX_PATH );

		if( IsRegisteredFolder( cFolder )== false )
		{
			m_vecAlreadyWroteFolder.push_back( cFolder );

			// Billboard
			if( strstr( cFolder, "_BILL" ) || strstr( cFolder, "_bill" ) )
			{
				int nLength = strlen( cFolder );
				cFolder[nLength-7] = NULL;
				strcat( cFolder, "A_BILL\\" );
				if( IsRegisteredFolder( cFolder ) == false )
					m_vecAlreadyWroteFolder.push_back( cFolder );
/*
				cFolder[nLength-7] = NULL;
				strcat( cFolder, "B_BILL\\" );
				if( IsRegisteredFolder( cFolder ) == false )
					m_vecAlreadyWroteFolder.push_back( cFolder );

				cFolder[nLength-7] = NULL;
				strcat( cFolder, "C_BILL\\" );
				if( IsRegisteredFolder( cFolder ) == false )
					m_vecAlreadyWroteFolder.push_back( cFolder );
*/
				cFolder[nLength-7] = NULL;
				strcat( cFolder, "D_BILL\\" );
				if( IsRegisteredFolder( cFolder ) == false )
					m_vecAlreadyWroteFolder.push_back( cFolder );
			}
		}

		return true;
	}
	return false;
}

void CBsResChecker::AddIgnoreFolderList( char* pFolder )
{
	char szUpper[MAX_PATH];
	strcpy(szUpper,pFolder);
	_strupr(szUpper);
	m_vecIgnoreFolder.push_back( szUpper );
}

void CBsResChecker::ClearIgnoreFolderList()
{
	m_vecIgnoreFolder.clear();
}

void CBsResChecker::AddIgnoreFileList( char* pFile )
{
	char szUpper[MAX_PATH];
	strcpy(szUpper,pFile);
	_strupr(szUpper);
	m_vecIgnoreFile.push_back( szUpper );
}

void CBsResChecker::ClearIgnoreFileList()
{
	m_vecIgnoreFile.clear();
}



void CBsResChecker::CloseFile()
{
	for( int i=0; i<RES_GROUP_NUM; i++ )
	{
		if( m_hOthFile[i] != INVALID_HANDLE_VALUE )
		{
			CBsFileIO::BsCloseFileHandle( m_hOthFile[i] );
			m_hOthFile[i] = INVALID_HANDLE_VALUE;
		}
		if( m_hTexFile[i] != INVALID_HANDLE_VALUE )
		{
			CBsFileIO::BsCloseFileHandle( m_hTexFile[i] );
			m_hTexFile[i] = INVALID_HANDLE_VALUE;
		}
	}
}

bool CBsResChecker::IsRegisteredFile( char* pStr, bool bTex )
{
	int nCnt;

	if( bTex )
		nCnt = m_ResCheckerListData[m_CurID].m_vecTex.size();
	else
		nCnt = m_ResCheckerListData[m_CurID].m_vecRes.size();

	char *pFindPtr=(char *)strrchr(pStr, '\\');
	pFindPtr = pFindPtr ? (pFindPtr+1) : pStr;

	for( int i=0; i<nCnt; i++ )
	{
		if( bTex )
		{
			if( strcmp( m_ResCheckerListData[m_CurID].m_vecTex[i].c_str(), pFindPtr ) == 0 )
				return true;
		}
		else
		{
			if( strcmp( m_ResCheckerListData[m_CurID].m_vecRes[i].c_str(), pFindPtr ) == 0 )
				return true;
		}
	}
	return false;
}

bool CBsResChecker::IsIgnoreFile( char* pFile )
{
	int nCnt = m_vecIgnoreFile.size();
	for( int i=0; i<nCnt; i++ )
	{
		if( strstr( pFile, m_vecIgnoreFile[i].c_str() ) )
		{
			return true;
		}
	}
	return false;
}

bool CBsResChecker::IsRegisteredFolder( char* pStr )
{
	int nCnt = m_vecAlreadyWroteFolder.size();
	for( int i=0; i<nCnt; i++ )
	{
		if( strcmp( m_vecAlreadyWroteFolder[i].c_str(), pStr ) == 0 )
		{
			return true;
		}
	}
	return false;
}