///////////////////////////////////////////////////////////////////////////////
// xlMultiRenderTexture.h
// Collection: xlMultiRenderTexture
//-----------------------------------------------------------------------------
#ifndef __inc_xlMultiRenderTexture_h__
#define __inc_xlMultiRenderTexture_h__

//=============================================================================
// includes
//=============================================================================
#include "xlRenderTexture.h"

//=============================================================================
// namespace
//=============================================================================
namespace xl {
//=============================================================================
// macros
//=============================================================================
#define	RENDERMULTITEXTURE_NB_TEXTURE	4

//=============================================================================
// structs
//=============================================================================
//=============================================================================
// classes
//=============================================================================
class MultiRenderTexture{
	protected:
		LPDIRECT3DDEVICE			m_pd3dDevice;
		DWORD						m_dwNbRenderTexture;
		DWORD						m_dwRenderTextureIndex[ RENDERMULTITEXTURE_NB_TEXTURE ];
		RenderTexture*				m_pRenderTexture[ RENDERMULTITEXTURE_NB_TEXTURE ];

		LPDIRECT3DSURFACE			m_pDepthStencilSurface;
		LPDIRECT3DSURFACE			m_pCurrentRenderSurface[ RENDERMULTITEXTURE_NB_TEXTURE ];
		LPDIRECT3DSURFACE			m_pCurrentDepthStencilSurface;
		BOOL						m_bUseDepthStencil;

		HRESULT						Construct( LPDIRECT3DDEVICE		pd3dDevice );

	public:
		MultiRenderTexture();
		~MultiRenderTexture();

		static MultiRenderTexture*	Create( LPDIRECT3DDEVICE	pd3dDevice );
		void						Release();

		HRESULT						BeginScene();
		HRESULT						EndScene();

		RenderTexture*				CreateRenderTexture( DWORD					dwRenderTextureIndex,
														 UINT					uWidth,
														 UINT					uHeight,
														 BOOL					bMipmap,
														 DWORD					dwUsage,
														 D3DFORMAT				d3dFormat,
														 D3DMULTISAMPLE_TYPE	d3dMultiSampleType );
		HRESULT						CreateDepthStencilSurface( D3DFORMAT		d3dDepthStencilformat );

		RenderTexture*				GetRenderTexture( DWORD dwRenderTextureIndex );
};

//=============================================================================
// namespace
//=============================================================================
} // end of namespace
#endif	// __inc_xlMultiRenderTexture_h__