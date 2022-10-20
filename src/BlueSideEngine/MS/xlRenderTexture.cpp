///////////////////////////////////////////////////////////////////////////////
// xlRenderTexture.cpp
// Colloction: xlRenderTexture
//-----------------------------------------------------------------------------
//=============================================================================
// includes
//=============================================================================
#include "stdafx.h"
#ifdef	_XBOX
#include "xlRenderTexture.h"
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
// Name: RenderTexture::RenderTexture
// Arguments:
// Returns:
//-----------------------------------------------------------------------------
RenderTexture::RenderTexture()
{
    m_pd3dDevice            = NULL;
    m_uWidth				= 0;
    m_uHeight				= 0;
    m_pTexture              = NULL;
    m_pTextureSurface       = NULL;
    m_pDepthStencilSurface  = NULL;

    m_pCurrentRenderSurface       = NULL;
    m_pCurrentDepthStencilSurface = NULL;
	m_bMipmap				= FALSE;
	m_bUseDepthStencil			= FALSE;
	m_dwRenderTargetIndex	= 0;
}

//-----------------------------------------------------------------------------
// Name: RenderTexture::~RenderTexture
// Arguments:
// Returns:
//-----------------------------------------------------------------------------
RenderTexture::~RenderTexture()
{
}

//-----------------------------------------------------------------------------
// Name: RenderTexture::Create
//	Create render texture.
// Arguments:
//	pd3dDevice	device.
//	uWidth		width of the texture.
//	uHeight	height of the texture.
//	d3dFormat		d3dFormat.
//	d3dDepthStencilformat		z buffer d3dFormat.
// Returns:
//	true		success.
//	false		false.
//-----------------------------------------------------------------------------
RenderTexture* RenderTexture::Create( LPDIRECT3DDEVICE		pd3dDevice,
									  UINT					uWidth,
									  UINT					uHeight,
									  BOOL					bMipmap,
									  DWORD					dwUsage,
									  D3DFORMAT				d3dFormat,
									  D3DFORMAT				d3dDepthStencilformat,
									  D3DMULTISAMPLE_TYPE	d3dMultiSampleType )
{
	// create
	RenderTexture*	pRenderTexture;
	if( NULL == ( pRenderTexture = new RenderTexture ) )
		return	NULL;

    if( FAILED( pRenderTexture->Construct( pd3dDevice,
                                           uWidth,
                                           uHeight,
										   bMipmap,
										   dwUsage,
                                           d3dFormat,
                                           d3dDepthStencilformat,
										   d3dMultiSampleType ) ) ){
	    pRenderTexture->Release();	// delete RenderTexture instance in Release().
	    return  NULL;
    }

    return  pRenderTexture;
}


//-----------------------------------------------------------------------------
// Name: RenderTexture::Construct
//	Create render texture.
// Arguments:
//	pd3dDevice	device.
//	uWidth		width of the texture.
//	uHeight	height of the texture.
//	d3dFormat		d3dFormat.
//	d3dDepthStencilformat		z buffer d3dFormat.
// Returns:
//	true		success.
//	false		false.
//-----------------------------------------------------------------------------
HRESULT RenderTexture::Construct( LPDIRECT3DDEVICE		pd3dDevice,
						          UINT					uWidth,
								  UINT					uHeight,
								  BOOL					bMipmap,
								  DWORD					dwUsage,
								  D3DFORMAT				d3dFormat,
								  D3DFORMAT				d3dDepthStencilformat,
								  D3DMULTISAMPLE_TYPE	d3dMultiSampleType )
{
	// store
	m_pd3dDevice	= pd3dDevice;
	m_uWidth		= uWidth;
	m_uHeight		= uHeight;
	m_bMipmap		= bMipmap;
	m_bUseDepthStencil	= ( D3DFMT_UNKNOWN == d3dDepthStencilformat )?	FALSE : TRUE;

	// texture
	if( FAILED( pd3dDevice->CreateTexture( uWidth,
										   uHeight,
										   1,
										   dwUsage,
										   d3dFormat,
										   0,
										   &m_pTexture,
										   NULL ) ) )
        return  E_FAIL;

	// EDRAM
	if( FAILED( pd3dDevice->CreateRenderTarget( uWidth,
												uHeight,
												d3dFormat,
												d3dMultiSampleType,
												0,
												FALSE,
												&m_pTextureSurface,
												NULL ) ) )
		return	E_FAIL;

	// Depth stencil
	if( m_bUseDepthStencil ){
		if( FAILED( pd3dDevice->CreateDepthStencilSurface( uWidth,
														   uHeight,
														   d3dDepthStencilformat,
														   d3dMultiSampleType,
														   0,
														   TRUE,
														   &m_pDepthStencilSurface,
														   NULL ) ) )
			return  E_FAIL;
	}
	
	return  S_OK;
}


//-----------------------------------------------------------------------------
// Name: RenderTexture::Release
//	Delete render texture.
// Arguments:
// Returns:
//-----------------------------------------------------------------------------
void	RenderTexture::Release()
{
	SAFE_RELEASE( m_pTexture );
	SAFE_RELEASE( m_pTextureSurface );
	SAFE_RELEASE( m_pDepthStencilSurface );
// [PREFIX:beginmodify] 2006/2/15 junyash PS#5188 reports RenderTexture::Create() momory leaking 'pRenderTexture'.
	delete this;
// [PREFIX:endmodify] junyash
}


//-----------------------------------------------------------------------------
// Name: RenderTexture::BeginScene
//	Note system the begining of render texture.
// Arguments:
// Returns:
//-----------------------------------------------------------------------------
HRESULT RenderTexture::BeginScene( DWORD dwRenderTargetIndex )
{
	// RenderTargetIndex
	m_dwRenderTargetIndex	= dwRenderTargetIndex;

	// Store the current backbuffer and zbuffer
	m_pd3dDevice->GetRenderTarget( dwRenderTargetIndex, &m_pCurrentRenderSurface );
	if( m_bUseDepthStencil )
		m_pd3dDevice->GetDepthStencilSurface( &m_pCurrentDepthStencilSurface );

	// render target
	m_pd3dDevice->SetRenderTarget( dwRenderTargetIndex, m_pTextureSurface );
	if( m_bUseDepthStencil )
		m_pd3dDevice->SetDepthStencilSurface( m_pDepthStencilSurface );
//	g_BsKernel.GetDevice()->SetDepthStencilSurface( g_BsKernel.GetDevice()->GetDepthStencilRTTSurface() );

	// Render the scene
	m_pd3dDevice->BeginScene();

    return  S_OK;
}


//-----------------------------------------------------------------------------
// Name: RenderTexture::EndScene
//	Note system the end of render texture.
// Arguments:
// Returns:
//-----------------------------------------------------------------------------
HRESULT RenderTexture::EndScene()
{
	m_pd3dDevice->EndScene();

	// Resolve
	DWORD	dwFlag	= D3DRESOLVE_ALLFRAGMENTS;
	switch( m_dwRenderTargetIndex ){
		case	0 :
			dwFlag	|= D3DRESOLVE_RENDERTARGET0;
			break;
		case	1 :
			dwFlag	|= D3DRESOLVE_RENDERTARGET1;
			break;
		case	2 :
			dwFlag	|= D3DRESOLVE_RENDERTARGET2;
			break;
		case	3 :
			dwFlag	|= D3DRESOLVE_RENDERTARGET3;
			break;
	}

	m_pd3dDevice->Resolve( dwFlag,
						   NULL,
						   m_pTexture,
						   NULL,
						   0,		// I dont know why but not 1.
						   0,
						   NULL,
						   1.0f,
						   0,
						   NULL );

	// Change the rendertarget back to the main backbuffer
	m_pd3dDevice->SetRenderTarget( m_dwRenderTargetIndex, m_pCurrentRenderSurface );	
	if( m_bUseDepthStencil )
		m_pd3dDevice->SetDepthStencilSurface( m_pCurrentDepthStencilSurface );
	//g_BsKernel.GetDevice()->SetDepthStencilSurface( g_BsKernel.GetDevice()->GetDepthStencilSurface() );

	SAFE_RELEASE( m_pCurrentRenderSurface );
	if( m_bUseDepthStencil ){
		SAFE_RELEASE( m_pCurrentDepthStencilSurface );
	}

	return  S_OK;
}

//-----------------------------------------------------------------------------
// Name: GetTexture()
// Desc: 
//-----------------------------------------------------------------------------
LPDIRECT3DTEXTURE	RenderTexture::GetTexture()
{
	return	m_pTexture;
}

//=============================================================================
// namespace
//=============================================================================
} // end of namespace
#endif