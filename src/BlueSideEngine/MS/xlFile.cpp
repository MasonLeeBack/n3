///////////////////////////////////////////////////////////////////////////////
// xlFile.cpp
// Collection: File
//-----------------------------------------------------------------------------
//=============================================================================
// includes
//=============================================================================
#include "stdafx.h"
#ifdef	_XBOX
#include "xlFile.h"
#ifdef	XL_BLUE_SIDE_ENGINE
#include "BsFileIO.h"
#endif
//=============================================================================
// namespace
//=============================================================================
namespace xl {
//=============================================================================
// macros
//=============================================================================
//=============================================================================
// structs
//=============================================================================
//=============================================================================
// classes
//=============================================================================
//=============================================================================
// variables
//=============================================================================
//=============================================================================
// methods
//=============================================================================

//-----------------------------------------------------------------------------
// Name: File()
// Desc: 
//-----------------------------------------------------------------------------
File::File()
{
}


//-----------------------------------------------------------------------------
// Name: ~File()
// Desc: 
//-----------------------------------------------------------------------------
File::~File()
{
}


//-----------------------------------------------------------------------------
// Name: File::Load
// Arguments:
// Returns:
//	HRESULT
//-----------------------------------------------------------------------------
HRESULT	File::Load( const CHAR* strFileName, VOID** ppFileData, DWORD* pFileSize )
{
	// mruete: cleaning up this function to make it a bit safer
	*ppFileData	= NULL;
	*pFileSize = 0;

#ifdef	XL_BLUE_SIDE_ENGINE
	HRESULT	hr;
	hr	= CBsFileIO::LoadFile( strFileName, ppFileData, pFileSize );

	return	hr;
#else
	CHAR	strMessage[ 1024 ];


	// Open the file for reading
	HANDLE hFile = CreateFile( strFileName, GENERIC_READ, 0, NULL, 
							   OPEN_EXISTING, 0, NULL );

	if( INVALID_HANDLE_VALUE == hFile ){
		sprintf( strMessage, "Unable to open file %s\n", strFileName );		
		return	E_FAIL;
	}

	DWORD dwFileSize = GetFileSize( hFile, NULL );
	VOID * pFileData = malloc( *pFileSize ); 

	if( NULL == pFileData ){
		CloseHandle( hFile );

		sprintf( strMessage, "Unable to open allocate memory for file %s\n", strFileName );		
		return	E_OUTOFMEMORY;
	}

	DWORD dwBytesRead;
	BOOL bRead = ReadFile( hFile, *ppFileData, *pFileSize, &dwBytesRead, NULL );

	// Finished reading file
	CloseHandle( hFile ); 

	// mruete: prefix bug 495: added check for success
	if( !bRead || dwBytesRead != *pFileSize ){
		free( pFileData );
		sprintf( strMessage, "Unable to read file %s\n", strFileName );		
		return	E_FAIL;
	}

	*ppFileData = pFileData;
	*pFileSize = dwFileSize;

	return  S_OK;
#endif
}


//-----------------------------------------------------------------------------
// Name: File::Unload
// Arguments:
// Returns:
//	HRESULT
//-----------------------------------------------------------------------------
VOID	File::Unload( VOID* pFileData )
{
#ifdef	XL_BLUE_SIDE_ENGINE
	CBsFileIO::FreeBuffer( pFileData );
#else
	assert( pFileData != NULL );
	free( pFileData );
#endif
}


//=============================================================================
// namespace
//=============================================================================
} // end of namespace
#endif