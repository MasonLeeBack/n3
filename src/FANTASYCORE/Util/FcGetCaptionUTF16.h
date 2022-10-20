///////////////////////////////////////////////////////////////////////////////
// FcGetCaptionUTF16.h
// Collection: CFcGetCaptionUTF16
//-----------------------------------------------------------------------------
#ifndef __inc_CFcGetCaptionUTF16_h__
#define __inc_CFcGetCaptionUTF16_h__

//=============================================================================
// restriction
//=============================================================================
// Need one or more space behind an index.
// The number of figure of index must be less than CFCGETCAPTIONUTF16_MAX_NB_FIGURE.
// Need return behind caption.

//=============================================================================
// includes
//=============================================================================
#ifdef _XBOX
#include <xtl.h>
#endif
#include <d3d9.h>

//=============================================================================
// namespace
//=============================================================================
//=============================================================================
// macros
//=============================================================================
#define	CFCGETCAPTIONUTF16_MAX_NB_FIGURE	8

//=============================================================================
// structs
//=============================================================================
//=============================================================================
// classes
//=============================================================================
class CFcGetCaptionUTF16 {
	protected:
		static	LPCWSTR	m_pBuffer;
		static	LPCWSTR	m_pBufferEnd;
		static	BOOL	m_bBufferOverRun;
		static	LPCWSTR	m_pTarget;
		static	INT		m_iTargetIndex;
		static	INT		m_iTargetLength;

		static	INT		ReadIndex( LPCWSTR pCurrent, LPINT pIndex );
		static	INT		ReadCaption( LPCWSTR pCurrent, LPCWSTR* ppTarget );
		static	INT		FindBlank( LPCWSTR pCurrent );
		static	INT		FindDoubleQuotation( LPCWSTR pCurrent );
		static	INT		SkipBlank( LPCWSTR pCurrent );
		static	INT		SkipDoubleQuotation( LPCWSTR pCurrent );
		static	VOID	GetCaptionR( LPCWSTR pCurrent );

	public:
		CFcGetCaptionUTF16();
		~CFcGetCaptionUTF16();

		static	HRESULT	SetBufferPointer( LPVOID pBuffer, INT iSizeBuffer );
		static	INT		GetNbCaptions();
		static	INT		GetCaption( INT iIndex, LPWSTR pCaption );
};

//=============================================================================
// namespace
//=============================================================================
#endif	// __inc_CFcGetCaptionUTF16_h__
