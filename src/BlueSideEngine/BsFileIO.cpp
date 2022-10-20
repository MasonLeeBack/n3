// BsFileIO.cpp: implementation of the CBsDirManager class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BsFileIO.h"
#include "zlib\zlibdecode.h"
#include "BsUtil.h"
#include "BsKernel.h"

#ifdef _USAGE_TOOL_
	HANDLE g_hRenderThread = 0;
    DWORD  g_dwRenderThreadId = 0;
#else 
	extern DWORD  g_dwRenderThreadId;
	extern HANDLE g_hRenderThread;
#endif //_USAGE_TOOL_


CBsDirManager::CBsDirManager()
{
	m_szRootDir[0][0]=0;
	m_szRootDir[1][0]=0;
	m_szCurDir[0][0]=0;
	m_szCurDir[1][0]=0;
}

CBsDirManager::~CBsDirManager()
{
}

void CBsDirManager::Initialize(const char *pResourceDir)
{
	if(pResourceDir)
	{
		SetRootDir(pResourceDir);
		m_szCurDir[0][0]=0;
		m_szCurDir[1][0]=0;
	}
}

char g_szLoaderFullName[2][_MAX_PATH];
char g_szLoaderCurDir[2][_MAX_PATH];
char *CBsDirManager::GetFullName(const char *pFileName)
{
	int nThreadNum = 0;
	int nIndex = -1;
	if(GetCurrentThreadId() == g_dwRenderThreadId) {
		nIndex = 0;
	}
	else {
		nIndex = 1;
	}
	if((pFileName[1]!=':')&&((pFileName[0]!='\\')||(pFileName[1]!='\\'))){
		strcpy(g_szLoaderFullName[nIndex], m_szRootDir[nIndex]);
		strcat(g_szLoaderFullName[nIndex], m_szCurDir[nIndex]);
		if(pFileName[0]=='\\'){
			strcat(g_szLoaderFullName[nIndex], pFileName+1);
		}
		else{
			strcat(g_szLoaderFullName[nIndex], pFileName);
		}
	}
	else
	{
		strcpy(g_szLoaderFullName[nIndex], pFileName);
	}

	if( g_hRenderThread == NULL )
		strcpy( g_szLoaderFullName[0], g_szLoaderFullName[1] );

	return g_szLoaderFullName[nIndex];
}

char *CBsDirManager::GetCurrentDirectory()
{
	int nIndex = -1;
	if(GetCurrentThreadId() == g_dwRenderThreadId) {
		nIndex = 0;
	}
	else {
		nIndex = 1;
	}
	strcpy(g_szLoaderCurDir[nIndex], m_szRootDir[nIndex]);
	strcat(g_szLoaderCurDir[nIndex], m_szCurDir[nIndex]);

	if( g_hRenderThread == NULL )
		strcpy( g_szLoaderCurDir[0], g_szLoaderCurDir[1] );

	return g_szLoaderCurDir[nIndex];
}

void CBsDirManager::SetCurrentDirectory(const char *pPathName)
{
	int nIndex = -1;
	if(GetCurrentThreadId() == g_dwRenderThreadId) {
		nIndex = 0;
	}
	else {
		nIndex = 1;
	}

	const char *pFindPtr;

	pFindPtr=strstr(pPathName, m_szRootDir[nIndex]);
	if(!pFindPtr){
		strcpy(m_szRootDir[nIndex], pPathName);
		m_szCurDir[nIndex][0]=0;
		if(m_szRootDir[nIndex][strlen(m_szRootDir[nIndex])-1]!='\\'){
			strcat(m_szRootDir[nIndex], "\\");
		}
	}
	else{
		if(strcmp(pPathName, m_szRootDir[nIndex])==0){
			m_szCurDir[nIndex][0]=0;
		}
		else{
			SetCurDir(pPathName+strlen(m_szRootDir[nIndex]));
		}
	}

	if( g_hRenderThread == NULL )
		strcpy( m_szRootDir[0], m_szRootDir[1] );
}


void CBsDirManager::SetRootDir(const char *pRootDir)
{
	int nIndex = -1;
	if(GetCurrentThreadId() == g_dwRenderThreadId) {
		nIndex = 0;
	}
	else {
		nIndex = 1;
	}

	int nLength;

	nLength=strlen(pRootDir);
	if(nLength<=0){
		return;
	}
	strcpy(m_szRootDir[nIndex], pRootDir);
	if(m_szRootDir[nIndex][nLength-1]!='\\'){
		strcat(m_szRootDir[nIndex], "\\");
	}

	if( g_hRenderThread == NULL )
		strcpy( m_szRootDir[0], m_szRootDir[1] );
}


void CBsDirManager::SetCurDir(const char *pCurDir)
{
	int nIndex = -1;
	if(GetCurrentThreadId() == g_dwRenderThreadId) {
		nIndex = 0;
	}
	else {
		nIndex = 1;
	}

	int nLength;

	nLength=strlen(pCurDir);
	if(nLength<=0){
		return;
	}
	strcpy(m_szCurDir[nIndex], pCurDir);
	if(m_szCurDir[nIndex][nLength-1]!='\\'){
		strcat(m_szCurDir[nIndex], "\\");
	}

	if( g_hRenderThread == NULL )
		strcpy( m_szCurDir[0], m_szCurDir[1] );
}

int CBsDirManager::ChangeDir(const char *pDir)
{
	int nIndex = -1;
	if(GetCurrentThreadId() == g_dwRenderThreadId) {
		nIndex = 0;
	}
	else {
		nIndex = 1;
	}

	char *pPtr;
	int nLength;

	nLength=strlen(pDir);
	if(nLength==0){
		return -1;
	}

	if(strcmp(pDir, "..")==0){
		if(m_szCurDir[nIndex][0]==0){
			return -1;
		}
		m_szCurDir[nIndex][strlen(m_szCurDir[nIndex])-1]=0;
		pPtr=strrchr(m_szCurDir[nIndex], '\\');
		if(pPtr){
			pPtr[1]=0;
		}
		else{
			m_szCurDir[nIndex][0]=0;

			if( g_hRenderThread == NULL )
			{
				strcpy( m_szRootDir[0], m_szRootDir[1] );
				strcpy( m_szCurDir[0], m_szCurDir[1] );
			}
			return -1;
		}
	}
	else{
		strcat(m_szCurDir[nIndex], pDir);
		if(pDir[nLength-1]!='\\'){
			strcat(m_szCurDir[nIndex], "\\");
		}
	}

	if( g_hRenderThread == NULL )
	{
		strcpy( m_szRootDir[0], m_szRootDir[1] );
		strcpy( m_szCurDir[0], m_szCurDir[1] );
	}

	return 1;
}

#define _BS_HANDLE_ID (__int64)0x00000b5f17e00000
#define _MAX_FILE_HANDLES 64
#define _BS_HANDLE_MASK 0x1

class BsFileHandle
{
public:

	BsFileHandle()
	{
	}

	~BsFileHandle()
	{
	}

	__int64 m_HandleID;
	BYTE	*m_pFileData;
	BYTE	*m_pFilePtr;
	BYTE	*m_pEOF;
	DWORD	m_dwPackedFileIdx;
	DWORD	m_dwFlags;
	DWORD	dwPAD;
}; 


typedef struct
{
	DWORD dwRefCount;
	BYTE *pFileData;
	BOOL bLoadPhysical;
	DWORD dwPAD;
} PACKFILEHANDLE;


class BsPackedFileHandler
{
protected:

	BsPackedFileHandler()
	{
		memset(m_Handles, 0, sizeof(m_Handles));
		m_dwHandleCount = 0;
	}

	~BsPackedFileHandler()
	{
		DWORD dwCount = 0;

		for( int i = 0; i < _MAX_FILE_HANDLES && dwCount < m_dwHandleCount; i++ )
		{
			if(m_Handles[i].dwRefCount != 0)
			{
				BsAssert(0 && "Outstanding file references to packed file");
			}
			dwCount++;
		}
	}


public:

	static BsPackedFileHandler & GetInstance()
	{
		if( m_sObj == NULL )
			m_sObj = new BsPackedFileHandler;
		return(*m_sObj);
	}

#ifdef _USAGE_TOOL_
	static bool CreateInstance()
	{
		// assert(!m_sObj);m_sObj = new BsPackedFileHandler;assert(m_sObj);
		return true;
	}

	static void DestroyInstance()
	{
		assert(m_sObj);
		delete m_sObj;
		m_sObj = NULL;
	}
#endif

	bool ReleaseFileHandle( HANDLE hFile )
	{
		BsFileHandle *pHandle = (BsFileHandle*)hFile;
		InterlockedDecrement(reinterpret_cast<volatile long*>(&m_Handles[pHandle->m_dwPackedFileIdx].dwRefCount));
		delete pHandle;
		return true;
	}


	HANDLE AddPackFile( BYTE * pFileData, BOOL bLoadPhysical )
	{
#ifndef _LTCG
		if(GetCurrentThreadId()==g_dwRenderThreadId)
			BsAssert(0 && "Error: mounting packed file in Render Thread");
#endif

		HANDLE hFile = NULL;

		for( int i = 0; i < _MAX_FILE_HANDLES; i++ )
		{
			if(m_Handles[i].pFileData == NULL)
			{
				m_Handles[i].pFileData = pFileData;
				m_Handles[i].bLoadPhysical = bLoadPhysical;
				InterlockedIncrement(reinterpret_cast<volatile long*>(&m_Handles[i].dwRefCount));
				m_dwHandleCount++;
				hFile = pFileData;
				break;
			}
		}
		return hFile;
	}


	void RemovePackFile( HANDLE hFile )
	{
#ifndef _LTCG
		if(GetCurrentThreadId()==g_dwRenderThreadId)
			BsAssert(0 && "Error: mounting packed file in Render Thread");
#endif
		DWORD dwCount = 0;

		// by yooty
//		for( int i = 0; i < _MAX_FILE_HANDLES && dwCount < m_dwHandleCount; i++ )
		for( int i = 0; i < _MAX_FILE_HANDLES; i++ )
		{
			if(m_Handles[i].pFileData == hFile)
			{
				// dwRefCount should now be one, otherwise there are outstanding
				// references to files within the pack file.
				BsAssert( m_Handles[i].dwRefCount == 1 );
				
#ifndef _XBOX
				CBsFileIO::FreeBuffer(m_Handles[i].pFileData);
#else
				if(!m_Handles[i].bLoadPhysical)
					CBsFileIO::FreeBuffer(m_Handles[i].pFileData);
				else
					XPhysicalFree(m_Handles[i].pFileData);
#endif
				m_Handles[i].dwRefCount = 0;
				m_Handles[i].pFileData = NULL;
				m_dwHandleCount--;
				break;
			}
			dwCount++;
		}
	}

	BYTE * FileIsPhysicalLoaded(const char * szFileName)
	{
		BsAssert(szFileName);
		DWORD dwCount = 0;

		char *pName =(char *)strrchr(szFileName, '\\');
		pName = pName ? (pName+1) : (char*)szFileName;

		for( int i = 0; i < _MAX_FILE_HANDLES && dwCount < m_dwHandleCount; i++ )
		{
			if(m_Handles[i].pFileData != NULL && m_Handles[i].bLoadPhysical)
			{
				PACKEDFILEHEADER *pHeader = (PACKEDFILEHEADER*)m_Handles[i].pFileData;
				PACKEDFILEENTRY *pPFEntry = &pHeader->Entries[0];
				DWORD dwFileCount = pHeader->dwPackedFileCount;

				for(DWORD j = 0; j < dwFileCount; j++)
				{
					if(_stricmp( pPFEntry->szFileName, pName ) == 0)
					{
						return m_Handles[i].pFileData + pPFEntry->dwOffset;
					}
					pPFEntry++;
				}
			}
			dwCount++;
		}
		return NULL;
	}

	HANDLE IsPackedFileHandle( HANDLE hFile )
	{
		if(hFile != INVALID_HANDLE_VALUE && ((int)hFile & _BS_HANDLE_MASK))
		{
			BsFileHandle *hTmp = (BsFileHandle*)((int)hFile & ~_BS_HANDLE_MASK);
			if(hTmp->m_HandleID == _BS_HANDLE_ID)
				return hTmp;
		}
		return NULL;
	}
	

	HANDLE CreateFile( LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD dwFlagsAndAttributes )
	{
		DWORD dwCount = 0;

		char *pName =(char *)strrchr(lpFileName, '\\');
		pName = pName ? (pName+1) : (char*)lpFileName;

		// change by yooty
//		for( int i = 0; i < _MAX_FILE_HANDLES && dwCount < m_dwHandleCount; i++ )
		for( int i = 0; i < _MAX_FILE_HANDLES; i++ )
		{
			if(m_Handles[i].pFileData != NULL)
			{
				PACKEDFILEHEADER *pHeader = (PACKEDFILEHEADER*)m_Handles[i].pFileData;
				PACKEDFILEENTRY *pPFEntry = &pHeader->Entries[0];
				DWORD dwFileCount = pHeader->dwPackedFileCount;

				for(DWORD j = 0; j < dwFileCount; j++)
				{
					if(_stricmp( pPFEntry->szFileName, pName ) == 0)
					{
#ifndef _LTCG
						if(dwDesiredAccess & GENERIC_WRITE)
							BsAssert(0 && "Can't write to packed file");
#endif					
						BsFileHandle *pFile = new BsFileHandle;
						BsAssert(pFile);

						pFile->m_HandleID = _BS_HANDLE_ID;
						pFile->m_dwPackedFileIdx = i;
						pFile->m_pFileData = m_Handles[i].pFileData + pPFEntry->dwOffset;
						pFile->m_pEOF = pFile->m_pFileData + pPFEntry->dwSize;
						pFile->m_pFilePtr = pFile->m_pFileData;
						pFile->m_dwFlags = dwFlagsAndAttributes;
						InterlockedIncrement(reinterpret_cast<volatile long*>(&m_Handles[i].dwRefCount));
						// Make the address returned odd to help identify this
						// handle as a belonging to a packed file...
						return( (HANDLE)((int)pFile | _BS_HANDLE_MASK) );
					}
					pPFEntry++;
				}
				dwCount++;
			}
		}
		return NULL;
	}


	BOOL ReadFile( HANDLE hFile, LPVOID lpBuffer, DWORD dwNumberOfBytesToRead,
					LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped)
	{	
		BsFileHandle *pHandle = (BsFileHandle*)hFile;
		*lpNumberOfBytesRead = 0;

		if(hFile == NULL || lpBuffer == NULL || dwNumberOfBytesToRead == 0 || lpNumberOfBytesRead == NULL || 
			((pHandle->m_dwFlags & FILE_FLAG_OVERLAPPED) && lpOverlapped == NULL ))
		{
			SetLastError(E_INVALIDARG);
			return false;
		}

		if(lpOverlapped)
		{
			pHandle->m_pFilePtr = pHandle->m_pFileData + lpOverlapped->Offset;
		}

		if(pHandle->m_pFilePtr < pHandle->m_pFileData)
		{
			SetLastError(E_INVALIDARG);
			return false;
		}

		DWORD dwBytesRemaining = 0;
		
		if(pHandle->m_pFilePtr < pHandle->m_pEOF)
		{
			dwBytesRemaining = pHandle->m_pEOF - pHandle->m_pFilePtr;
		}
		
		DWORD dwBytesCopied = dwNumberOfBytesToRead > dwBytesRemaining ? dwBytesRemaining : dwNumberOfBytesToRead;

		if(dwBytesCopied>0)
		{
#ifdef _XBOX
			XMemCpy(lpBuffer, pHandle->m_pFilePtr, dwBytesCopied);
#else
			memcpy(lpBuffer, pHandle->m_pFilePtr, dwBytesCopied);
#endif
		}
		else
			SetLastError(ERROR_HANDLE_EOF);

		pHandle->m_pFilePtr += dwBytesCopied;
		*lpNumberOfBytesRead = dwBytesCopied;
		if(lpOverlapped && lpOverlapped->hEvent)
			SetEvent(lpOverlapped->hEvent);
		
		SetLastError(ERROR_SUCCESS);
		return 1;
	}

	DWORD SetFilePointer( HANDLE hFile, LONG lDistanceToMove,
						  PLONG lpDistanceToMoveHigh, DWORD dwMoveMethod )
	{	// Note: only supports low lDistanceToMove since we don't have larger files
		if(hFile == NULL || lDistanceToMove == NULL)
		{
			SetLastError(E_INVALIDARG);
			return INVALID_SET_FILE_POINTER;
		}

		BsFileHandle *pHandle = (BsFileHandle*)hFile;
		BYTE *pbOldPtr = pHandle->m_pFilePtr; 
		switch(dwMoveMethod)
		{
		case FILE_BEGIN:
			pHandle->m_pFilePtr = pHandle->m_pFileData + lDistanceToMove;
			break;
		case FILE_CURRENT:
			pHandle->m_pFilePtr = pHandle->m_pFilePtr + lDistanceToMove;
			break;
		case FILE_END:
			pHandle->m_pFilePtr = pHandle->m_pEOF + lDistanceToMove;
			break;
		default:
			SetLastError(E_INVALIDARG);
		}

		if(pHandle->m_pFilePtr < pHandle->m_pFileData)
		{
			pHandle->m_pFilePtr = pbOldPtr;
			SetLastError(ERROR_NEGATIVE_SEEK);
			return INVALID_SET_FILE_POINTER;
		}
		return pHandle->m_pFilePtr - pHandle->m_pFileData;
	}


private:

	DWORD	m_dwHandleCount;
	static BsPackedFileHandler *m_sObj;
	PACKFILEHANDLE	m_Handles[_MAX_FILE_HANDLES];
};


BsPackedFileHandler*  BsPackedFileHandler::m_sObj = 0;


HANDLE CBsFileIO::BsMountPackFile( const LPCSTR lpFilename)
{
#ifndef _PACKED_RESOURCES
	return NULL;
#endif

	DWORD dwFileSize;
	BYTE *pFileData = NULL;
	HANDLE hPackFile; 

	if(!FAILED(LoadFile(lpFilename, (void**)&pFileData, &dwFileSize)))
	{
		if((hPackFile = BsPackedFileHandler::GetInstance().AddPackFile(pFileData, false)) != NULL)
		{
			return hPackFile;
		}
		else
		{
			if(pFileData)
				FreeBuffer(pFileData);
		}
	}
	return NULL;
}


HANDLE CBsFileIO::BsMountCompressedPackFile( const LPCSTR lpFilename, BOOL bLoadPhysical)
{
#ifndef _PACKED_RESOURCES
	return NULL;
#endif

	DWORD dwFileSize;
	BYTE *pFileData = NULL;
	HANDLE hPackFile; 

	if(!FAILED(LoadFileDecompress(lpFilename, (char**)&pFileData, &dwFileSize, bLoadPhysical)))
	{
		if((hPackFile = BsPackedFileHandler::GetInstance().AddPackFile(pFileData, bLoadPhysical)) != NULL)
		{
			return hPackFile;
		}
		else
		{
#ifndef _XBOX
			if(pFileData)
				FreeBuffer(pFileData);
#else
			if(pFileData)
			{
				if(!bLoadPhysical)
					FreeBuffer(pFileData);
				else
					XPhysicalFree(pFileData);
			}
#endif
		}
	}
	return NULL;
}


void CBsFileIO::BsDismountPackFile(HANDLE hFile)
{
#ifndef _PACKED_RESOURCES
	return;
#endif

	if(hFile)
		BsPackedFileHandler::GetInstance().RemovePackFile(hFile);
}

BYTE* CBsFileIO::BsFileIsPhysicalLoaded(const char * szFileName)
{
#ifndef _PACKED_RESOURCES
	return NULL;
#endif

	return BsPackedFileHandler::GetInstance().FileIsPhysicalLoaded(szFileName);
}

const DWORD BSXALLOCID = 42;
BYTE * CBsFileIO::m_pReadbuffer = NULL;
static CBsCriticalSection s_csBufferManager;
static CBsCriticalSection s_csDVDListLock;
STDHANDLELIST CBsFileIO::m_dvdFileHandles;
void ( *CBsFileIO::m_pCallBackFunc )( void );

void CBsFileIO::Init()
{
	BsAssert(m_pReadbuffer==0);
#ifdef _XBOX
	m_pReadbuffer = (BYTE*)XPhysicalAlloc( STREAMING_BUFFER_SIZE, MAXULONG_PTR, 0, PAGE_READWRITE );
#else
	m_pReadbuffer = new BYTE[STREAMING_BUFFER_SIZE];
#endif //_XBOX
}


void CBsFileIO::Finalize()
{
#ifdef _XBOX
	if(m_pReadbuffer)
		XPhysicalFree(m_pReadbuffer);
#else
	if(m_pReadbuffer)
		delete [] m_pReadbuffer;
#endif //_XBOX

	m_pReadbuffer = NULL;
}


HRESULT CBsFileIO::LoadFile( const CHAR* strFileName, VOID** ppFileData, DWORD* pdwFileSize )
{
	BsAssert( ppFileData );
	if( pdwFileSize )
		*pdwFileSize = 0L;
	*ppFileData = NULL;


	DWORD dwFileFlags = FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN;;

	// Open the file for reading
	HANDLE hFile = BsCreateFile( strFileName, GENERIC_READ, 0, NULL, 
		OPEN_EXISTING, dwFileFlags, NULL );

	if( INVALID_HANDLE_VALUE == hFile )
	{
		DebugString( "Unable to open file %s\n", strFileName );
		return E_FAIL;
	}

	DWORD dwFileSize = BsGetFileSize( hFile, NULL );

	// Do a non-buffered read if the file-size is sector aligned
	if( dwFileSize % _DVD_SECTOR_SIZE == 0 )
	{
		BsCloseFileHandle(hFile);
		dwFileFlags |= FILE_FLAG_NO_BUFFERING;
		hFile = BsCreateFile( strFileName, GENERIC_READ, 0, NULL, 
			OPEN_EXISTING, dwFileFlags, NULL );

		if( INVALID_HANDLE_VALUE == hFile )
		{
			DebugString( "Unable to open file %s\n", strFileName );
			return E_FAIL;
		}
	}

	VOID* pFileData = malloc( dwFileSize );

	if( NULL == pFileData )
	{
		BsCloseFileHandle( hFile );
		DebugString( "Unable to open allocate memory for file %s\n", strFileName );
		return E_OUTOFMEMORY;
	}

	DWORD dwBytesRead;
	BOOL bRead = BsReadFile( hFile, pFileData, dwFileSize, &dwBytesRead, NULL );

	// Finished reading file
	BsCloseFileHandle( hFile ); 

	// mruete: prefix bug 890: adding check for ReadFile() success
	if( !bRead || dwBytesRead != dwFileSize )
	{
		DebugString( "Unable to read file %s\n", strFileName );
		free(pFileData);
		return E_FAIL;
	}

	if( pdwFileSize )
		*pdwFileSize = dwFileSize;
	*ppFileData = pFileData;

	return S_OK;
}

HRESULT CBsFileIO::LoadSectorImmediate( HANDLE hIPFile, VOID* pReadBufferIn, OVERLAPPED* pOvr )
{
	DWORD dwErr;
	BOOL bOK;
	DWORD dwBytesRead;

	// Read in a single sector - Read whole sector to get non-buffered overlapped I/O...
	bOK = CBsFileIO::BsReadFile( hIPFile, pReadBufferIn, _DVD_SECTOR_SIZE, &dwBytesRead, pOvr );

	dwErr = GetLastError();

	if(( bOK && dwBytesRead != _DVD_SECTOR_SIZE ) || (!bOK && (dwErr != ERROR_IO_PENDING )))
	{
		BsAssert( 0  && "LoadSectorImmediate - file size or read failure\n");
		return E_FAIL;
	}

	// Block until the I/O is complete
	if(dwErr == ERROR_IO_PENDING)
	{
		if(!BsGetOverlappedResult(hIPFile, pOvr, &dwBytesRead, true) || (dwBytesRead != _DVD_SECTOR_SIZE))
		{
			BsAssert( 0  && "LoadSectorImmediate - file read failure\n");
			return E_FAIL;
		}
	}

	return S_OK;
}

HRESULT CBsFileIO::LoadFileNoBuffering( const CHAR* strFileName, VOID** pReadBufferInOut, DWORD* pdwFileSizeInOut )
{
	BsAssert( pReadBufferInOut );
	BsAssert( pReadBufferInOut );

	if(pdwFileSizeInOut && !(*pReadBufferInOut))
		*pdwFileSizeInOut = 0;

	DWORD dwFlags = FILE_FLAG_NO_BUFFERING | FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN;

	// Open the file for reading
	HANDLE hFile = BsCreateFile( strFileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, dwFlags, NULL );

	if( INVALID_HANDLE_VALUE == hFile )
	{
		DebugString( "Unable to open file %s\n", strFileName );
		return E_FAIL;
	}

	DWORD dwFileSize = BsGetFileSize( hFile, NULL );
	VOID* pFileData = NULL;
	
	if(*pReadBufferInOut)
	{
// [PREFIX:beginmodify] 2006/2/25 junyash PS#5797 dereferencing NULL pointer
		if(pdwFileSizeInOut && *pdwFileSizeInOut != dwFileSize)
		{
			BsCloseFileHandle(hFile);
			return E_FAIL;
		}
// [PREFIX:endmodify] junyash
		pFileData = *pReadBufferInOut;
	}
	else
	{
		pFileData = malloc( dwFileSize );
	}

	if( NULL == pFileData )
	{
		BsCloseFileHandle( hFile );
		DebugString( "Unable to open allocate memory for file %s\n", strFileName );
		return E_OUTOFMEMORY;
	}

	DWORD dwBytesRead;
	BsReadFile( hFile, pFileData, dwFileSize, &dwBytesRead, NULL );

	// Finished reading file
	BsCloseFileHandle( hFile ); 

	if( dwBytesRead != dwFileSize )
	{
		DebugString( "Unable to read file %s\n", strFileName );
		
		// Only free pFileData if allocated here
		if(*pReadBufferInOut == NULL)
			free(pFileData);
		return E_FAIL;
	}

	if( pdwFileSizeInOut )
		*pdwFileSizeInOut = dwFileSize;

	if(*pReadBufferInOut == NULL)
		*pReadBufferInOut = pFileData;

	return S_OK;
}


HRESULT CBsFileIO::BsGetFileSize( const CHAR* strFileName, DWORD *dwFileSize )
{
	BsAssert(strFileName);
	BsAssert(dwFileSize);

	// Open the file for reading
	HANDLE hFile = BsCreateFile( strFileName, GENERIC_READ, 0, NULL, 
		OPEN_EXISTING, 0, NULL );

	if( INVALID_HANDLE_VALUE == hFile )
	{
		DebugString( "Unable to open file %s\n", strFileName );
		return E_FAIL;
	}

	*dwFileSize = BsGetFileSize( hFile, NULL );
	BsCloseFileHandle(hFile);
	return S_OK;
}


HRESULT CBsFileIO::LoadFileDecompress(const char *pFilename, char **ppOutputBuffer, DWORD* pdwFileSize, BOOL bLoadPhysical)
{
	BsAssert(ppOutputBuffer);

	*ppOutputBuffer = NULL;
	if(pdwFileSize)
		*pdwFileSize = 0;

	DWORD dwIPFileSize;
	DWORD dwUncompressedSize;
	BYTE *pData;
	HRESULT hr;

	hr = BsGetFileSize(pFilename, &dwIPFileSize);
	if( hr != S_OK)
		return E_FAIL;
	BsAssert((dwIPFileSize / _DVD_SECTOR_SIZE)!= 0);


	if(dwIPFileSize <= DECOMPRESS_IMMEDIATE_THRESHOLD )
	{
		THREAD_AUTOLOCK(&s_csBufferManager);
		if(LoadFileNoBuffering( pFilename, (VOID**)&m_pReadbuffer, &dwIPFileSize ) != S_OK)
		{
			BsAssert( 0  && "LoadFileDecompress - failed to open compressed file\n");
			return E_FAIL;
		}

		dwUncompressedSize = *(DWORD*)(&m_pReadbuffer[0]);
//		dwUncompressedSize = (DWORD)&m_pReadbuffer[0];
#ifdef _XBOX
		if(bLoadPhysical)
			pData = (BYTE*)XPhysicalAlloc( dwUncompressedSize, MAXULONG_PTR, GPU_TEXTURE_ALIGNMENT, PAGE_READWRITE );
		else
#endif
		pData = (BYTE*)malloc(dwUncompressedSize);
		if(pData == NULL)
		{
			BsAssert( 0  && "LoadFileDecompress - error allocating memory\n");
			return E_OUTOFMEMORY;
		}

		ZLIBDecode dc;
		BYTE * pRead = m_pReadbuffer + sizeof(DWORD);
		DWORD dwIPSize = dwIPFileSize - sizeof(DWORD);
		hr = dc.DecodeImmediate(pRead, pData, dwIPSize, dwUncompressedSize);
		if(hr != S_OK)
			BsAssert( 0  && "LoadFileDecompress - error uncompressing file\n");
	}
	else
	{
		THREAD_AUTOLOCK(&s_csBufferManager);
		DWORD dwFlags = FILE_FLAG_OVERLAPPED | FILE_FLAG_NO_BUFFERING |
			FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN;

		HANDLE ipFile = BsCreateFile( pFilename, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, dwFlags, NULL );
		if (ipFile == INVALID_HANDLE_VALUE)
		{
			BsAssert( 0  && "LoadFileNoBuffering - failed to open compressed file\n");
			return E_FAIL;
		}

		OVERLAPPED ovrR;
		memset(&ovrR, 0, sizeof(OVERLAPPED));

		// Read in uncompressed file size and file header - Read whole sector to
		// get non-buffered overlapped I/O...
		if(LoadSectorImmediate( ipFile, m_pReadbuffer, &ovrR ) != S_OK)
		{
			BsAssert( 0  && "LoadFileDecompress - file size or read failure\n");
			BsCloseFileHandle( ipFile );
			return E_FAIL;
		}

		ovrR.Offset += _DVD_SECTOR_SIZE;
		dwUncompressedSize = *(DWORD*)(&m_pReadbuffer[0]);
#ifdef _XBOX
		if(bLoadPhysical)
			pData = (BYTE*)XPhysicalAlloc( dwUncompressedSize, MAXULONG_PTR, GPU_TEXTURE_ALIGNMENT, PAGE_READWRITE );
		else
#endif
		pData = (BYTE*)malloc(dwUncompressedSize);
		if(pData == NULL)
		{
			BsAssert( 0  && "LoadFileDecompress - error allocating memory\n");
			BsCloseFileHandle( ipFile );
			return E_OUTOFMEMORY;
		}

		ZLIBDecode dc;
		dc.InitMmDecompress(m_pReadbuffer, pData, STREAMING_BUFFER_SIZE, dwUncompressedSize, ipFile, &ovrR, dwIPFileSize-_DVD_SECTOR_SIZE, true);
		hr = dc.DecompressStreamToMemory();
		if(hr != S_OK)
			BsAssert( 0  && "LoadFileDecompress - error uncompressing file\n");

		BsCloseFileHandle( ipFile );
	}

	if( hr == S_OK)
	{
		*ppOutputBuffer = (CHAR*)pData;
		if(pdwFileSize)
			*pdwFileSize = dwUncompressedSize;
	}
	return S_OK;
}




//--------------------------------------------------------------------------------------
// Name: LoadFilePhysicalMemory()
// Desc: Helper function to load a file into physical memory
//--------------------------------------------------------------------------------------
HRESULT CBsFileIO::LoadFilePhysicalMemory( const CHAR* strFileName,
										   VOID** ppFileData,
										   DWORD* pdwFileSize,
										   DWORD* pdwFlags,
										   DWORD dwAlignment,
										   DWORD dwProtection,
										   bool bNoBuffering )
{
    assert( ppFileData );
    if( pdwFileSize )
        *pdwFileSize = 0L;
	*ppFileData = NULL;

	DWORD dwFileFlags = FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN;;
	
	if( bNoBuffering )
		dwFileFlags |= FILE_FLAG_NO_BUFFERING;

	// Open the file for reading
    HANDLE hFile = BsCreateFile( strFileName, GENERIC_READ, 0, NULL, 
                               OPEN_EXISTING, dwFileFlags, NULL );
    
    if( INVALID_HANDLE_VALUE == hFile )
    {
        DebugString( "Unable to open file %s\n", strFileName );
        return E_FAIL;
    }

    DWORD dwFileSize = GetFileSize( hFile, NULL );

	// Do a non-buffered read if the file-size is sector aligned
	if( bNoBuffering == false && dwFileSize % _DVD_SECTOR_SIZE == 0 )
	{
		BsCloseFileHandle(hFile);
		dwFileFlags |= FILE_FLAG_NO_BUFFERING;
		hFile = BsCreateFile( strFileName, GENERIC_READ, 0, NULL, 
			OPEN_EXISTING, dwFileFlags, NULL );

		if( INVALID_HANDLE_VALUE == hFile )
		{
			DebugString( "Unable to open file %s\n", strFileName );
			return E_FAIL;
		}
	}

	DWORD dwMemFlags = 0;
#ifdef _XBOX
	dwMemFlags = MAKE_XALLOC_ATTRIBUTES( 0, 0, 0, 0, BSXALLOCID, dwAlignment, dwProtection, 0, XALLOC_MEMTYPE_PHYSICAL );
	VOID* pFileData = XMemAlloc( dwFileSize, dwMemFlags );
#else
	VOID* pFileData = malloc( dwFileSize );
#endif
    if( NULL == pFileData )
    {
        BsCloseFileHandle( hFile );
        DebugString( "Unable to open allocate memory for file %s\n", strFileName );
        return E_OUTOFMEMORY;
    }

    DWORD dwBytesRead;
    BsReadFile( hFile, pFileData, dwFileSize, &dwBytesRead, NULL );
    
    // Finished reading file
    BsCloseFileHandle( hFile ); 

    if( dwBytesRead != dwFileSize )
    {
        DebugString( "Unable to read file %s\n", strFileName );
        return E_FAIL;
    }

	if( pdwFlags )
		*pdwFlags = dwMemFlags;
    if( pdwFileSize )
        *pdwFileSize = dwFileSize;
    *ppFileData = pFileData;

    return S_OK;
}



//--------------------------------------------------------------------------------------
// Name: SaveFile()
// Desc: Helper function to save a file
//--------------------------------------------------------------------------------------
HRESULT CBsFileIO::SaveFile( const CHAR* strFileName, VOID* pFileData, DWORD dwFileSize )
{
    // Open the file for reading
    HANDLE hFile = BsCreateFile( strFileName, GENERIC_WRITE, 0, NULL, 
                               CREATE_ALWAYS, 0, NULL );
    if( INVALID_HANDLE_VALUE == hFile )
    {
        DebugString( "Unable to open file %s\n", strFileName );
        return E_FAIL;
    }

    DWORD dwBytesWritten;
    BsWriteFile( hFile, pFileData, dwFileSize, &dwBytesWritten, NULL );
    
    // Finished reading file
    BsCloseFileHandle( hFile ); 

    if( dwBytesWritten != dwFileSize )
    {
        DebugString( "Unable to write file %s\n", strFileName );
        return E_FAIL;
    }

    return S_OK;
}


#ifdef _LTCG
void CBsFileIO::DoDirtyDiskHandler( HANDLE hFile, LPCSTR lpFileName )
{	
	// Search to see if this is a DVD file
	if(hFile != INVALID_HANDLE_VALUE)
	{
		THREAD_AUTOLOCK(&s_csDVDListLock);
		int nCount = m_dvdFileHandles.size();
		int i = 0;
		for( ; i < nCount; i++)
		{
			if( m_dvdFileHandles[i] == hFile )
				break;
		}
		if( nCount == i)
		{	// File handle is not for DVD file so just return;
			return;
		}
	}
	// Set call back in application using
	// CBsFileIO::SetDirtyDiskHandlerCallBackFunc
	// Ogu

#ifdef _RETAIL_VERSION
	if(m_pCallBackFunc)
		m_pCallBackFunc();
#else // ifdef _RETAIL_VERSION
	static LPCWSTR g_pwstrButtons[1] = { L"OK" };
	WCHAR strWideChar[1024] = L"NONAME";
	MESSAGEBOX_RESULT Result;
	XOVERLAPPED Overlapped;
	int size = (lpFileName)?(MultiByteToWideChar( CP_ACP, 0, lpFileName, -1, strWideChar, 1024 )):(0);
	DWORD dwResult = XShowMessageBoxUI( 0, L"BsCreateFile", strWideChar, ARRAYSIZE( g_pwstrButtons ), g_pwstrButtons, 0, XMB_ERRORICON, &Result, &Overlapped );
#endif // ifdef _RETAIL_VERSION
}
#endif

void CBsFileIO::SetDirtyDiskHandlerCallBackFunc( void (*pCallBackFunc )( void ) )
{
	m_pCallBackFunc	= pCallBackFunc;
}

// These functions are wrappers for the Windows file routines
// ALL in-game file reading should go through these functions
// to aid in error detection/handling

// TODO - make all these inline!!!!!
HANDLE CBsFileIO::BsCreateFile( LPCSTR lpFileName,
								DWORD dwDesiredAccess,
								DWORD dwShareMode,
								LPSECURITY_ATTRIBUTES lpSecurityAttributes,
								DWORD dwCreationDisposition,
								DWORD dwFlagsAndAttributes,
								HANDLE hTemplateFile)
{
	HANDLE hFile = BsPackedFileHandler::GetInstance().CreateFile(lpFileName, dwDesiredAccess, dwFlagsAndAttributes);
	
	if(!hFile)
	{
		hFile = ::CreateFile( lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes,
					  dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);

		//  No writing to the DVD in the LTCG/shipping build so ignore...
		if((dwDesiredAccess & GENERIC_WRITE) == 0)
		{
			if(strstr(lpFileName, "D:\\") || strstr(lpFileName, "d:\\") || strstr(lpFileName, "game:\\"))
			{
				if(hFile == INVALID_HANDLE_VALUE)
				{
					DoDirtyDiskHandler(hFile,lpFileName);
				}
				else
				{
					THREAD_AUTOLOCK(&s_csDVDListLock);
					m_dvdFileHandles.push_back(hFile);
				}
			}
		}
	}

	return hFile;
}


DWORD CBsFileIO::BsGetFileSize( HANDLE hFile, LPDWORD lpFileSizeHigh)
{
	HANDLE hPack = BsPackedFileHandler::GetInstance().IsPackedFileHandle(hFile);
	if(hPack)
	{
		if(lpFileSizeHigh)
			*lpFileSizeHigh = 0;
		BsFileHandle* pFile = (BsFileHandle*)hPack;
		return pFile->m_pEOF - pFile->m_pFileData;
	}
	else
		return ::GetFileSize(hFile, lpFileSizeHigh);
}


BOOL CBsFileIO::BsReadFile( HANDLE hFile,
							LPVOID lpBuffer,
							DWORD dwNumberOfBytesToRead,
							LPDWORD lpNumberOfBytesRead,
							LPOVERLAPPED lpOverlapped)
{
	HANDLE hPack = BsPackedFileHandler::GetInstance().IsPackedFileHandle(hFile);
	if(hPack)
		return BsPackedFileHandler::GetInstance().ReadFile(hPack, lpBuffer, dwNumberOfBytesToRead, lpNumberOfBytesRead, lpOverlapped);

	BOOL bOK = ::ReadFile( hFile, lpBuffer, dwNumberOfBytesToRead, lpNumberOfBytesRead, lpOverlapped);
	
	if(( bOK && dwNumberOfBytesToRead != *lpNumberOfBytesRead ) || (!bOK && (GetLastError() != ERROR_IO_PENDING )))
		DoDirtyDiskHandler(hFile,NULL);

	return bOK;
}

BOOL CBsFileIO::BsGetOverlappedResult( HANDLE hFile,
									   LPOVERLAPPED lpOverlapped,
									   LPDWORD lpNumberOfBytesTransferred,
									   BOOL bWait)
{
#ifdef _DEBUG
	HANDLE hPack = BsPackedFileHandler::GetInstance().IsPackedFileHandle(hFile);
	if(hPack)
		BsAssert(0 && "Can't get overlapped result for packed file");
#endif

	BOOL bOK = ::GetOverlappedResult( hFile, lpOverlapped, lpNumberOfBytesTransferred, bWait);

	if(!bOK)
	{
		DWORD dwErr = GetLastError();
		#if 0
		if(dwErr != ERROR_IO_PENDING )
		#else
		// Phantagram's super programmers found! GetOverlappedResult() return ERROR_IO_INCOMPLETE
		// see XDK help about GetOverlappedResult() bWait section.
		if(/*dwErr != ERROR_IO_PENDING &&*/ dwErr != ERROR_IO_INCOMPLETE)
		#endif
			DoDirtyDiskHandler(hFile,NULL);
	}

	return bOK;
}


BOOL CBsFileIO::BsWriteFile( HANDLE hFile,
						     LPCVOID lpBuffer,
							 DWORD nNumberOfBytesToWrite,
							 LPDWORD lpNumberOfBytesWritten,
							 LPOVERLAPPED lpOverlapped )
{
#ifdef _LTCG
	HANDLE hPack = BsPackedFileHandler::GetInstance().IsPackedFileHandle(hFile);
	if(hPack)
		BsAssert(0 && "Can't write to packed file");
#endif
	BOOL bSuccess = ::WriteFile( hFile, lpBuffer, nNumberOfBytesToWrite, lpNumberOfBytesWritten, lpOverlapped );
	return bSuccess;
}

DWORD CBsFileIO::BsSetFilePointer( HANDLE hFile,
								   LONG lDistanceToMove,
								   PLONG lpDistanceToMoveHigh,
								   DWORD dwMoveMethod )
{
	HANDLE hPack = BsPackedFileHandler::GetInstance().IsPackedFileHandle(hFile);
	if(hPack)
		return BsPackedFileHandler::GetInstance().SetFilePointer(hPack, lDistanceToMove, lpDistanceToMoveHigh, dwMoveMethod);

	return ::SetFilePointer( hFile, lDistanceToMove, lpDistanceToMoveHigh, dwMoveMethod );
}


BOOL CBsFileIO::BsCloseFileHandle( HANDLE hFile)
{
	HANDLE hPack = BsPackedFileHandler::GetInstance().IsPackedFileHandle(hFile);
	if(hPack)
		return(BsPackedFileHandler::GetInstance().ReleaseFileHandle(hPack));
	else
	{
		THREAD_AUTOLOCK(&s_csDVDListLock);
		STDHANDLELIST::iterator itr = m_dvdFileHandles.begin();
		while( itr != m_dvdFileHandles.end())
		{
			HANDLE hTmp = (*itr);

			if(hTmp == hFile)
			{
				itr = m_dvdFileHandles.erase(itr);
			}
			else
				++itr;
		}

	}
	return ::CloseHandle(hFile);
}

#ifdef _USAGE_TOOL_
bool CBsFileIO::CreatePackedFileHandlerInstance(void)
{
	return BsPackedFileHandler::CreateInstance();
}

void CBsFileIO::DestroyPackedFileHandlerInstance(void)
{
	BsPackedFileHandler::DestroyInstance();
}
#endif