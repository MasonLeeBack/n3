// BsFileIO.h: Interfaces for the file loading classs.
//
//////////////////////////////////////////////////////////////////////

#pragma once

class CBsDirManager  
{
public:
	CBsDirManager();
	virtual ~CBsDirManager();

protected:
	char m_szRootDir[2][_MAX_PATH];
	char m_szCurDir[2][_MAX_PATH];

public:
	void Initialize(const char *pResourceName);
	char *GetFullName(const char *pFileName);
	char *GetCurrentDirectory();
	void SetCurrentDirectory(const char *pPathName);
	void SetRootDir(const char *pRootDir);
	void SetCurDir(const char *pCurDir);
	int ChangeDir(const char *pDir);
};


#ifndef XALLOC_PHYSICAL_ALIGNMENT_DEFAULT
#define XALLOC_PHYSICAL_ALIGNMENT_DEFAULT 0x0
#endif

#ifndef XALLOC_MEMPROTECT_READWRITE
#define XALLOC_MEMPROTECT_READWRITE 2
#endif

#ifndef _HD_SECTOR_SIZE
#define _HD_SECTOR_SIZE	512
#endif
#ifndef _DVD_SECTOR_SIZE
#define _DVD_SECTOR_SIZE 2048
#endif

#define STREAMING_BUFFER_SIZE (1024 * 128)
#define DECOMPRESS_IMMEDIATE_THRESHOLD (STREAMING_BUFFER_SIZE >> 2)

struct PACKEDFILEENTRY
{
	char szFileName[MAX_PATH];
	DWORD dwOffset;
	DWORD dwSize;
};

struct PACKEDFILEHEADER
{
	DWORD dwPackedFileCount;
	PACKEDFILEENTRY Entries[1];
};

// D3DTEXTURE_ALIGNMENT is only defined in Xbox and
// win32 for Xbox.....
#ifdef D3DTEXTURE_ALIGNMENT
struct XBOX360TEXTUREFILEHEADER
{
	DWORD dwTextureSize;
	DWORD dwCompressedSize;
	D3DBaseTexture d3dTexHeader[1];
};
#endif


typedef std::vector<HANDLE>		STDHANDLELIST;

class CBsFileIO
{
public:
	static void	Init();
	static void Finalize();
	static HRESULT LoadFile( const CHAR* strFileName, VOID** ppFileData, DWORD* pdwFileSize = NULL );
	static HRESULT LoadSectorImmediate( HANDLE hIPFile, VOID* pReadBufferIn, OVERLAPPED* pOvr );
	static HRESULT LoadFileNoBuffering( const CHAR* strFileName, VOID** pReadBufferInOut, DWORD* pdwFileSizeInOut = NULL );
	static HRESULT LoadFileDecompress(const char *pFilename, char **ppOutputBuffer, DWORD* pdwFileSize = NULL, BOOL bLoadPhysical = false);
	static HRESULT LoadFilePhysicalMemory( const CHAR* strFileName,
										   VOID** ppFileData,
										   DWORD* pdwFileSize = NULL,
										   DWORD* pdwFlags = NULL,
										   DWORD dwAlignment = XALLOC_PHYSICAL_ALIGNMENT_DEFAULT,
										   DWORD dwProtection = XALLOC_MEMPROTECT_READWRITE,
										   bool bNoBuffering = false );
	static HRESULT BsGetFileSize( const CHAR* strFileName, DWORD *dwFileSize );
	static HRESULT SaveFile( const CHAR* strFileName, VOID* pFileData, DWORD dwFileSize );
	static void FreeBuffer( VOID *pFileData ) { free(pFileData); };
#ifdef _XBOX	
	static void FreePhysicalMemory( VOID *pFileData, DWORD dwMemFlags ) { XMemFree(pFileData, dwMemFlags); };
#else
	static void FreePhysicalMemory( VOID *pFileData, DWORD dwMemFlags ) {};
#endif

#ifdef _LTCG
	static void DoDirtyDiskHandler( HANDLE hFile, LPCSTR lpFileName );
#else
	static void DoDirtyDiskHandler( HANDLE hFile, LPCSTR lpFileName ) {}
#endif

	// These functions are wrappers for the Windows file routines
	// ALL in-game file reading should go through these functions
	// to aid in error detection/handling
	static	HANDLE BsCreateFile( LPCSTR lpFileName,
								DWORD dwDesiredAccess,
								DWORD dwShareMode,
								LPSECURITY_ATTRIBUTES lpSecurityAttributes,
								DWORD dwCreationDisposition,
								DWORD dwFlagsAndAttributes,
								HANDLE hTemplateFile);

	static DWORD BsGetFileSize( HANDLE hFile, LPDWORD lpFileSizeHigh);

	static BOOL BsReadFile( HANDLE hFile,
						LPVOID lpBuffer,
						DWORD dwNumberOfBytesToRead,
						LPDWORD lpNumberOfBytesRead,
						LPOVERLAPPED lpOverlapped);

	static BOOL BsGetOverlappedResult( HANDLE hFile,
									LPOVERLAPPED lpOverlapped,
									LPDWORD lpNumberOfBytesTransferred,
									BOOL bWait);
	static BOOL BsWriteFile( HANDLE hFile,
							 LPCVOID lpBuffer,
							 DWORD nNumberOfBytesToWrite,
							 LPDWORD lpNumberOfBytesWritten,
							 LPOVERLAPPED lpOverlapped );

	static DWORD BsSetFilePointer( HANDLE hFile,
								 LONG lDistanceToMove,
								 PLONG lpDistanceToMoveHigh,
								 DWORD dwMoveMethod );


	static BOOL BsCloseFileHandle( HANDLE hFile );

	static HANDLE BsMountPackFile( const LPCSTR lpFilename);
	static HANDLE BsMountCompressedPackFile( const LPCSTR lpFilename, BOOL bLoadPhysical = false);
	static void BsDismountPackFile(HANDLE hFile);
	static BYTE * BsFileIsPhysicalLoaded(const char * szFileName);

#ifdef _USAGE_TOOL_
	static bool CreatePackedFileHandlerInstance(void);
	static void DestroyPackedFileHandlerInstance(void);
#endif

	static void SetDirtyDiskHandlerCallBackFunc( void (*pCallBackFunc )( void ) );

protected:
	static BYTE *m_pReadbuffer;
	static STDHANDLELIST m_dvdFileHandles;

	static void ( *m_pCallBackFunc )( void );
};
