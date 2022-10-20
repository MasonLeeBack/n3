///////////////////////////////////////////////////////////////////////////////
// FcGetCaptionUTF16.cpp
// Collection: CFcGetCaptionUTF16
//-----------------------------------------------------------------------------
//=============================================================================
// includes
//=============================================================================
#include "stdafx.h"
#ifdef	_XBOX
#include "FcGetCaptionUTF16.h"
//=============================================================================
// namespace
//=============================================================================
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
LPCWSTR	CFcGetCaptionUTF16::m_pBuffer;
LPCWSTR	CFcGetCaptionUTF16::m_pBufferEnd;
BOOL	CFcGetCaptionUTF16::m_bBufferOverRun;
LPCWSTR	CFcGetCaptionUTF16::m_pTarget;
INT		CFcGetCaptionUTF16::m_iTargetIndex;
INT		CFcGetCaptionUTF16::m_iTargetLength;

//=============================================================================
// methods
//=============================================================================
//-----------------------------------------------------------------------------
// Name: CFcGetCaptionUTF16::CFcGetCaptionUTF16
// Arguments:
// Returns:
//-----------------------------------------------------------------------------  
CFcGetCaptionUTF16::CFcGetCaptionUTF16()
{
}


//-----------------------------------------------------------------------------
// Name: CFcGetCaptionUTF16::CFcGetCaptionUTF16
// Arguments:
// Returns:
//-----------------------------------------------------------------------------  
CFcGetCaptionUTF16::~CFcGetCaptionUTF16()
{
}


//-----------------------------------------------------------------------------
// Name: CFcGetCaptionUTF16::SetBufferPointer
//	Set buffer pointer entrance.
// Arguments:
//	pBuffer
// Returns:
//	HRESULT
//-----------------------------------------------------------------------------
HRESULT	CFcGetCaptionUTF16::SetBufferPointer( VOID* pBuffer, INT iSizeBuffer )
{
	if( NULL == pBuffer )
		return	E_FAIL;

	m_pBuffer	= ( LPCWSTR )pBuffer;
	m_pBufferEnd	= ( LPCWSTR )( ( DWORD )pBuffer + ( DWORD )iSizeBuffer );

	return   S_OK;
}


//-----------------------------------------------------------------------------
// Name: CFcGetCaptionUTF16::GetNbCaptions
//	Returns number of captions.
// Arguments:
// Returns:
//  INT
//-----------------------------------------------------------------------------
INT	CFcGetCaptionUTF16::GetNbCaptions()
{
	return	0;
}


//-----------------------------------------------------------------------------
// Name: CFcGetCaptionUTF16::GetCaption
//	Returns number of captions.
// Arguments:
//	iIndex		caption index
//	pCaption	uinicode caption if success / NULL if fail
// Returns:
//  INT			length of caption
//-----------------------------------------------------------------------------
INT	CFcGetCaptionUTF16::GetCaption( INT iIndex, LPWSTR pCaption )
{
	m_pTarget		= NULL;
	m_iTargetIndex	= iIndex;
	m_bBufferOverRun	= FALSE;

	GetCaptionR( m_pBuffer + 1 );	// +1 for BOM (byte order marker)

	if( pCaption ){
		wcsncpy( pCaption, m_pTarget, m_iTargetLength );
		pCaption[ m_iTargetLength ]	= 0x0000;	// null terminater
	}

	return	m_iTargetLength;
}


//-----------------------------------------------------------------------------
// Name: CFcGetCaptionUTF16::GetCaptionR
//	Returns number of captions.
// Arguments:
//	pCurrent	current LPCWSTR
// Returns:
//  VOID
//-----------------------------------------------------------------------------
VOID	CFcGetCaptionUTF16::GetCaptionR( LPCWSTR pCurrent )
{
	// Read
	INT		iIndex;
	INT		iTargetLength;
	LPCWSTR	pTarget;

	// index
	pCurrent	+= SkipBlank( pCurrent );
	pCurrent	+= ReadIndex( pCurrent, &iIndex );
	pCurrent	+= SkipBlank( pCurrent );

	// caption
	pCurrent	+= SkipDoubleQuotation( pCurrent );
	iTargetLength	= ReadCaption( pCurrent, &pTarget );
	pCurrent	+= ( iTargetLength + 1 );
	pCurrent	+= SkipDoubleQuotation( pCurrent );

	// Found
	if( m_iTargetIndex == iIndex ){
		m_pTarget		= pTarget;
		m_iTargetLength	= iTargetLength;
		return;
	}

	// Buffer over run check
	if( m_bBufferOverRun ){
		m_pTarget		= NULL;
		m_iTargetLength	= 0;
		return;
	}

	// Next
	GetCaptionR( pCurrent );
}


//-----------------------------------------------------------------------------
// Name: CFcGetCaptionUTF16::ReadIndex
// Arguments:
//	pCurrent	current LPCWSTR
//	pIndex		Index
// Returns:
//  INT			read LPCWSTR
//-----------------------------------------------------------------------------
INT	CFcGetCaptionUTF16::ReadIndex( LPCWSTR pCurrent, LPINT pIndex )
{
	// The number of index's figure
	INT	iNbRead =	FindBlank( pCurrent );

	// Buffer over run check
	if( m_bBufferOverRun )
		return	0;

	// convert
	CHAR	pIndexChar[ CFCGETCAPTIONUTF16_MAX_NB_FIGURE ];
	WideCharToMultiByte( CP_ACP,
						 0,
						 pCurrent,
						 iNbRead,
						 pIndexChar,
						 CFCGETCAPTIONUTF16_MAX_NB_FIGURE,
						 NULL,
						 NULL );

	pIndexChar[ iNbRead ]	= '\0';

	if( pIndex )
		*pIndex	= atoi( pIndexChar );

	return	iNbRead;
}


//-----------------------------------------------------------------------------
// Name: CFcGetCaptionUTF16::ReadCaption
// Arguments:
//	pCurrent		current LPCWSTR
//	ppTarget		target caption
// Returns:
//  INT				read LPCWSTR
//-----------------------------------------------------------------------------
INT	CFcGetCaptionUTF16::ReadCaption( LPCWSTR pCurrent, LPCWSTR* ppTarget )
{
	// The number of caption"s character
	INT	iNbRead =	FindDoubleQuotation( pCurrent );

	// Buffer over run check
	if( m_bBufferOverRun )
		return	0;

	// get
	if( ppTarget )
		*ppTarget	= pCurrent;

	return	iNbRead;
}


//-----------------------------------------------------------------------------
// Name: CFcGetCaptionUTF16::FindBlank
//	Read Space, Tab, Return
// Arguments:		
//	pCurrent		current LPCWSTR
// Returns:
//  INT				read LPCWSTR
//-----------------------------------------------------------------------------
INT	CFcGetCaptionUTF16::FindBlank( LPCWSTR pCurrent )
{
	INT	iNbRead = 0;
	for( ;; ){
		// Buffer over run check
		if( ( DWORD )pCurrent + iNbRead >= ( DWORD )m_pBufferEnd ){
			m_bBufferOverRun	= TRUE;
			return	0;
		}

		if( ( 0x0020 == pCurrent[ iNbRead ] ) ||		// space
			( 0x0009 == pCurrent[ iNbRead ] ) ||		// tab
			( 0x000d == pCurrent[ iNbRead ] &&			// return
			  0x000a == pCurrent[ iNbRead + 1 ] )		// return		  
		  )
			break;

		iNbRead++;
	}

	return	iNbRead;
}


//-----------------------------------------------------------------------------
// Name: CFcGetCaptionUTF16::FindDoubleQuotation
//	Double Quotation
// Arguments:		
//	pCurrent		current LPCWSTR
// Returns:
//  INT				read LPCWSTR
//-----------------------------------------------------------------------------
INT	CFcGetCaptionUTF16::FindDoubleQuotation( LPCWSTR pCurrent )
{
	INT	iNbRead = 0;
	for( ;; ){
		// Buffer over run check
		if( ( DWORD )pCurrent + iNbRead >= ( DWORD )m_pBufferEnd ){
			m_bBufferOverRun	= TRUE;
			return	0;
		}

		if( ( 0x0022 == pCurrent[ iNbRead ] )			// double quotation
		  )
			break;

		iNbRead++;
	}

	return	iNbRead;
}


//-----------------------------------------------------------------------------
// Name: CFcGetCaptionUTF16::SkipBlank
//	Read Space, Tab, Return
// Arguments:
//	pCurrent		current LPCWSTR
// Returns:
//  INT				read LPCWSTR
//-----------------------------------------------------------------------------
INT	CFcGetCaptionUTF16::SkipBlank( LPCWSTR pCurrent )
{
	INT	iNbRead = 0;
	for( ;; ){
		// Buffer over run check
		if( ( DWORD )pCurrent + iNbRead >= ( DWORD )m_pBufferEnd ){
			m_bBufferOverRun	= TRUE;
			return	0;
		}

		if( ( 0x0020 != pCurrent[ iNbRead ] ) &&	// space
			( 0x0009 != pCurrent[ iNbRead ] ) &&	// tab
			( 0x000d != pCurrent[ iNbRead ] ) &&	// return
			( 0x000a != pCurrent[ iNbRead ] )		// return
		  )
			break;

		iNbRead++;
	}

	return	iNbRead;
}

//-----------------------------------------------------------------------------
// Name: CFcGetCaptionUTF16::SkipDoubleQuotation
//	Double Quotation
// Arguments:		
//	pCurrent		current LPCWSTR
// Returns:
//  INT				read LPCWSTR
//-----------------------------------------------------------------------------
INT	CFcGetCaptionUTF16::SkipDoubleQuotation( LPCWSTR pCurrent )
{
	INT	iNbRead = 0;
	for( ;; ){
		// Buffer over run check
		if( ( DWORD )pCurrent + iNbRead >= ( DWORD )m_pBufferEnd ){
			m_bBufferOverRun	= TRUE;
			return	0;
		}

		if( ( 0x0022 != pCurrent[ iNbRead ] )			// double quotation
		  )
			break;

		iNbRead++;
	}

	return	iNbRead;
}


//=============================================================================
// namespace
//=============================================================================
#endif