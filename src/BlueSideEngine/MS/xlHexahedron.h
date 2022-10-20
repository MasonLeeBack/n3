///////////////////////////////////////////////////////////////////////////////
// xlHexahedron.h
// Collection: xlPrimitiveBase
//-----------------------------------------------------------------------------
#ifndef __inc_xlHexahedron_h__
#define __inc_xlHexahedron_h__

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
struct HEXAHEDRON_VERTEX
{
	D3DXVECTOR3 vP;
	D3DXVECTOR3 vN;
    D3DXVECTOR2 vT0;
};

//=============================================================================
// classes
//=============================================================================
class Hexahedron : public PrimitiveBase {
	protected:
		FLOAT           m_fX;
		FLOAT           m_fY;
		FLOAT           m_fZ;

		FXLHANDLE		m_hTqTexture;
		FXLHANDLE		m_hTqColor;
		FXLHANDLE		m_hPassTexture;
		FXLHANDLE		m_hPassColor;
		FXLHANDLE		m_hWVP;
		FXLHANDLE		m_hColor;
		FXLHANDLE		m_hColorSampler;

		HRESULT         Construct( LPDIRECT3DDEVICE	pd3dDevice,
								   FLOAT			fX,
								   FLOAT			fY,
								   FLOAT			fZ,
								   DWORD			dwRepeat );

	public:
		Hexahedron();
		~Hexahedron();

		static Hexahedron*    Create( LPDIRECT3DDEVICE	pd3dDevice,
									  FLOAT				fX,
									  FLOAT				fY,
									  FLOAT				fZ,
									  DWORD				dwRepeat );
		void			Release();

		HRESULT			Render( const XMMATRIX& mWVP );
		HRESULT			Render( const XMMATRIX& mWVP, D3DXCOLOR d3dxColor );
		HRESULT			RenderLine( const XMMATRIX& mWVP, D3DXCOLOR d3dxColor );

		HRESULT			GetSize( FLOAT* pWidth, FLOAT* pHeight, FLOAT* pDepth );
};

//=============================================================================
// namespace
//=============================================================================
} // end of namespace
#endif	// __inc_Hexahedron_h__