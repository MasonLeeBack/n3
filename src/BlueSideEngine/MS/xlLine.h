///////////////////////////////////////////////////////////////////////////////
// xlLine.h
// Collection: xlPrimitive 
//-----------------------------------------------------------------------------
#ifndef __inc_xlLine_h__
#define __inc_xlLine_h__

//=============================================================================
// includes
//=============================================================================
#include "xlPrimitiveBase.h"

//=============================================================================
// macros
//=============================================================================
//=============================================================================
// namespace
//=============================================================================
namespace xl {   
//=============================================================================
// structs
//=============================================================================
struct LINE_VERTEX
{
	D3DXVECTOR3 vP;
};

//=============================================================================
// classes
//=============================================================================
class Line : public PrimitiveBase {
	protected:
		D3DPRIMITIVETYPE    m_d3dPtimitiveType;

		FXLHANDLE			m_hTqLine;
		FXLHANDLE			m_hPassLine;
		FXLHANDLE			m_hWVP;
		FXLHANDLE			m_hColor;

		HRESULT             Construct( LPDIRECT3DDEVICE	pd3dDevice,
									   DWORD			dwNbVertices,
									   D3DXVECTOR3*		pVertices,
									   D3DPRIMITIVETYPE	d3dPtimitiveType );
    public:
		Line();
		~Line();

		static Line*        Create( LPDIRECT3DDEVICE    pd3dDevice,
									DWORD				dwNbVertices,
									D3DXVECTOR3*        pVertices,
									D3DPRIMITIVETYPE    d3dPtimitiveType );       
        void			    Release();

		HRESULT			    Render( const XMMATRIX& mWVP, D3DXCOLOR d3dxColor );
		HRESULT			    Render( const XMMATRIX& mWVP, D3DXCOLOR d3dxColor, DWORD dwNbPrimitive );

		HRESULT             SetVertices( DWORD				 dwNbVertices,
										 D3DXVECTOR3*        pVertices,
										 D3DPRIMITIVETYPE    d3dPtimitiveType );
};

//=============================================================================
// namespace
//=============================================================================
} // end of namespace
#endif	// __inc_xlLine_h__
