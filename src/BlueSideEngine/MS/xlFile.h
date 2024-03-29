///////////////////////////////////////////////////////////////////////////////
// xlFile.h
// Collection: xlFile
//-----------------------------------------------------------------------------
#ifndef __inc_xlFile_h__
#define __inc_xlFile_h__

//=============================================================================
// includes
//=============================================================================
#include "xlCommon.h"
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
class File {
	protected:
	public:
		File();
		~File();

		static HRESULT		Load( const CHAR* strFileName, VOID** ppFileData, DWORD* pFileSize );
		static HRESULT		Save( const CHAR* strFileName, VOID*   pFileData, DWORD dwFileSize );

		static VOID			Unload( VOID* pFileData );
};

//=============================================================================
// namespace
//=============================================================================
} // end of namespace
#endif	// __inc_xlFile_h__