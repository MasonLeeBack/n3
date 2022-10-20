///////////////////////////////////////////////////////////////////////////////
// xlRenderTexture.h
// Collection: xlRenderTexture
//-----------------------------------------------------------------------------
#ifndef __inc_xlRenderTexture_h__
#define __inc_xlRenderTexture_h__

//=============================================================================
// includes
//=============================================================================
#include "xlCommon.h"
#include "xlSprite.h"
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
class RenderTexture{
	protected:
		LPDIRECT3DDEVICE		m_pd3dDevice;
		UINT					m_uWidth;
		UINT					m_uHeight;
		LPDIRECT3DTEXTURE		m_pTexture;
		LPDIRECT3DSURFACE		m_pTextureSurface;
		LPDIRECT3DSURFACE		m_pDepthStencilSurface;
		LPDIRECT3DSURFACE		m_pCurrentRenderSurface;
		LPDIRECT3DSURFACE		m_pCurrentDepthStencilSurface;
		BOOL					m_bMipmap;
		BOOL					m_bUseDepthStencil;
		DWORD					m_dwRenderTargetIndex;

		HRESULT					Construct( LPDIRECT3DDEVICE		pd3dDevice,
											UINT				uWidth,
											UINT				uHeight,
											BOOL				bMipmap,
											DWORD				dwUsage,
											D3DFORMAT			d3dFormat,
											D3DFORMAT			d3dDepthStencilformat,
											D3DMULTISAMPLE_TYPE	d3dMultiSampleType );
		public:
		RenderTexture();
		~RenderTexture();

		static RenderTexture*	Create( LPDIRECT3DDEVICE	pd3dDevice,
										UINT				uWidth,
										UINT				uHeight,
									    BOOL				bMipmap,
										DWORD				dwUsage,
										D3DFORMAT			d3dFormat,
										D3DFORMAT			d3dDepthStencilformat,
										D3DMULTISAMPLE_TYPE	d3dMultiSampleType );
		void					Release();

		HRESULT					BeginScene( DWORD dwRenderTargetIndex );
		HRESULT					EndScene();

		LPDIRECT3DTEXTURE		GetTexture();
};

//=============================================================================
// namespace
//=============================================================================
} // end of namespace
#endif	// __inc_xlRenderTexture_h__