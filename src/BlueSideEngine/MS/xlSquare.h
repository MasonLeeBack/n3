///////////////////////////////////////////////////////////////////////////////
// xlSquare.h
// Collection: xlPrimitive
//-----------------------------------------------------------------------------
#ifndef __inc_xlSquare_h__
#define __inc_xlSquare_h__

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
struct SQUARE_VERTEX
{
	D3DXVECTOR3 vP;
	D3DXVECTOR3 vN;
	D3DXVECTOR2 vT0;
};

//=============================================================================
// classes
//=============================================================================
class Square : public PrimitiveBase {
	protected:
		float               m_fSize;

		FXLHANDLE			m_hTqTexture;
		FXLHANDLE			m_hTqColor;
		FXLHANDLE			m_hPassTexture;
		FXLHANDLE			m_hPassColor;
		FXLHANDLE			m_hWVP;
		FXLHANDLE			m_hColor;
		FXLHANDLE			m_hColorSampler;

		HRESULT             Construct( LPDIRECT3DDEVICE pd3dDevice, float fSize, DWORD dwRepeat );

	public:
		Square();
		~Square();

		static Square*		Create( LPDIRECT3DDEVICE pd3dDevice, float fSize, DWORD dwRepeat );
		void		    	Release();

		HRESULT				Render( const XMMATRIX& mWVP );
		HRESULT				Render( const XMMATRIX& mWVP, D3DXCOLOR d3dxColor );

		float               GetSize()           { return m_fSize; }
};

//=============================================================================
// namespace
//=============================================================================
} // end of namespace
#endif	// __inc_xlSquare_h__

