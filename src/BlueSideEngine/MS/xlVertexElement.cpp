///////////////////////////////////////////////////////////////////////////////
// xlVertexElement.cpp
// Collection: xlPrimitive
//-----------------------------------------------------------------------------
//=============================================================================
// includes
//=============================================================================
#include "stdafx.h"
#ifdef	_XBOX
#include "xlVertexElement.h"
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
// Name: VertexElement::VertexElement
// Arguments:
// Returns:
//-----------------------------------------------------------------------------  
VertexElement::VertexElement()
{
}


//-----------------------------------------------------------------------------
// Name: VertexElement::VertexElement
// Arguments:
// Returns:
//-----------------------------------------------------------------------------  
VertexElement::~VertexElement()
{
}


//-----------------------------------------------------------------------------
// Name: VertexElement::GetElement
// Arguments:
//  bUsage
//  bUsageIndex
// Returns:
//  D3DVERTEXELEMENT*
//-----------------------------------------------------------------------------
D3DVERTEXELEMENT*   VertexElement::GetElement( BYTE bUsage, BYTE bUsageIndex, const D3DVERTEXELEMENT decl[] )
{
	for( DWORD i = 0; D3DDECLTYPE_UNUSED != decl[ i ].Type; i++ ){
		if( bUsage == decl[ i ].Usage &&
			bUsageIndex == decl[ i ].UsageIndex )

			return	( D3DVERTEXELEMENT* )&decl[ i ];
	}

	return   NULL;
}


//-----------------------------------------------------------------------------
// Name: VertexElement::GetOffset
// Arguments:
//  bUsage
//  bUsageIndex
// Returns:
//  WORD
//-----------------------------------------------------------------------------
WORD	VertexElement::GetOffset( BYTE bUsage, BYTE bUsageIndex, const D3DVERTEXELEMENT decl[] )
{
	D3DVERTEXELEMENT*   declTmp;
	declTmp  = GetElement( bUsage, bUsageIndex, decl );
	BsAssert( NULL != declTmp );	// mruete: prefix bug 496: added assert

	return  declTmp->Offset;
}


//-----------------------------------------------------------------------------
// Name: VertexElement::GetDeclTypeSize
// Arguments:
//  type
// Returns:
//  BYTE
//-----------------------------------------------------------------------------
BYTE    VertexElement::GetDeclTypeSize( D3DDECLTYPE type )
{
	BYTE	bSize;
	switch( D3DDECLTYPE_VERTEXFORMAT_MASK & type ){
		case	GPUVERTEXFORMAT_8_8_8_8	:
			bSize	= 4;
			break;
		case	GPUVERTEXFORMAT_2_10_10_10	:
			bSize	= 4;
			break;
		case	GPUVERTEXFORMAT_10_11_11	:
			bSize	= 4;
			break;
		case	GPUVERTEXFORMAT_11_11_10	:
			bSize	= 4;
			break;
		case	GPUVERTEXFORMAT_16_16		:
			bSize	= 4;
			break;
		case	GPUVERTEXFORMAT_16_16_16_16	:
			bSize	= 8;
			break;
		case	GPUVERTEXFORMAT_16_16_FLOAT	:
			bSize	= 4;
			break;
		case	GPUVERTEXFORMAT_16_16_16_16_FLOAT	:
			bSize	= 8;
			break;
		case	GPUVERTEXFORMAT_32	:
			bSize	= 4;
			break;
		case	GPUVERTEXFORMAT_32_32	:
			bSize	= 8;
			break;
		case	GPUVERTEXFORMAT_32_32_32_32	:
			bSize	= 16;
			break;
		case	GPUVERTEXFORMAT_32_FLOAT	:
			bSize	= 4;
			break;
		case	GPUVERTEXFORMAT_32_32_FLOAT	:
			bSize	= 8;
			break;
		case	GPUVERTEXFORMAT_32_32_32_32_FLOAT	:
			bSize	= 16;
			break;
		case	GPUVERTEXFORMAT_32_32_32_FLOAT	:
			bSize	= 12;
			break;
	}

	return	bSize;
}


//-----------------------------------------------------------------------------
// Name: VertexElement::GetSize
// Arguments:
//  decl
// Returns:
//  WORD
//-----------------------------------------------------------------------------
WORD    VertexElement::GetSize( const D3DVERTEXELEMENT decl[] )
{
	WORD    wSize  = 0;

	for( DWORD i = 0; D3DDECLTYPE_UNUSED != decl[ i ].Type; i++ ){
		wSize  += GetDeclTypeSize( ( D3DDECLTYPE )decl[ i ].Type );
	}

	return   wSize;
}


//=============================================================================
// namespace
//=============================================================================
} // end of namespace
#endif