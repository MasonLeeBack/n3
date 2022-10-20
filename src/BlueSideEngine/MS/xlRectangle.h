///////////////////////////////////////////////////////////////////////////////
// xlRectangle.h
// Collection: xlPrimitive 
//-----------------------------------------------------------------------------
#ifndef __inc_xlRectangle_h__
#define __inc_xlRectangle_h__

//=============================================================================
// includes
//=============================================================================
#include "xlPrimitiveBase.h"
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
struct RECTANGLE_VERTEX
{
	D3DXVECTOR3 vP;
	D3DXVECTOR3 vN;
    D3DXVECTOR2 vT0;
};

//=============================================================================
// classes
//=============================================================================
class Rectangle : public PrimitiveBase {
	protected:
		FXLHANDLE				m_hTqTexture;
		FXLHANDLE				m_hTqColor;
		FXLHANDLE				m_hPassTexture;
		FXLHANDLE				m_hPassColor;
		FXLHANDLE				m_hWVP;
		FXLHANDLE				m_hColor;
		FXLHANDLE				m_hColorSampler;

        HRESULT                 Construct( LPDIRECT3DDEVICE pd3dDevice,
                                           float            fSizeX,
                                           float            fSizeZ,
                                           DWORD            dwRepeatU,
                                           DWORD            dwRepeatV );

    public:
		Rectangle();
		~Rectangle();

		static Rectangle*    	Create( LPDIRECT3DDEVICE pd3dDevice,
                                        float            fSizeX,
                                        float            fSizeZ,
                                        DWORD            dwRepeatU,
                                        DWORD            dwRepeatV );
		void					Release();

		HRESULT					Render( const XMMATRIX& mWVP );
		HRESULT					Render( const XMMATRIX& mWVP, D3DXCOLOR d3dxColor );
		HRESULT					RenderLine( const XMMATRIX& mWVP, D3DXCOLOR d3dxColor );
};

//=============================================================================
// namespace
//=============================================================================
} // end of namespace
#endif	// __inc_xlRectangle_h__