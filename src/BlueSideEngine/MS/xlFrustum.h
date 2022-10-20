///////////////////////////////////////////////////////////////////////////////
// xlFrustum.h
// Collection: xlPrimitiveBase
//-----------------------------------------------------------------------------
#ifndef __inc_xlFrustum_h__
#define __inc_xlFrustum_h__

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
struct FRUSTUM_VERTEX
{
	D3DXVECTOR3 vP;
};

//=============================================================================
// classes
//=============================================================================
class Frustum : public PrimitiveBase {
	protected:
		float			m_fSizeX;
		float			m_fSizeY;
		float			m_fSizeZ;

		FXLHANDLE		m_hTqTexture;
		FXLHANDLE		m_hTqColor;
		FXLHANDLE		m_hPassTexture;
		FXLHANDLE		m_hPassColor;
		FXLHANDLE		m_hWVP;
		FXLHANDLE		m_hColor;
		FXLHANDLE		m_hColorSampler;

		HRESULT			Construct( LPDIRECT3DDEVICE pd3dDevice, float fSizeX, float fSizeZ, float fSizeY );

    public:
		Frustum();
		~Frustum();

		static Frustum* Create( LPDIRECT3DDEVICE pd3dDevice, float fSizeX, float fSizeZ, float fSizeY );
		void			Release();

		HRESULT			RenderLine( const XMMATRIX& mWVP, D3DXCOLOR d3dxColor );

		HRESULT			SetSize( float  fSizeX, float  fSizeZ, float  fSizeY );
		HRESULT			GetSize( float* pSizeX, float* pSizeZ, float* pSizeY );
};

//=============================================================================
// namespace
//=============================================================================
} // end of namespace
#endif	// __inc_Frustum_h__

