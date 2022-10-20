///////////////////////////////////////////////////////////////////////////////
// xlMultiRenderTexture.cpp
// Colloction: xlMultiRenderTexture
//-----------------------------------------------------------------------------
//=============================================================================
// includes
//=============================================================================
#include "stdafx.h"
#ifdef	_XBOX
#include "xlMultiRenderTexture.h"
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
// Name: MultiRenderTexture::MultiRenderTexture
// Arguments:
// Returns:
//-----------------------------------------------------------------------------
MultiRenderTexture::MultiRenderTexture()
{
	m_pd3dDevice					= NULL;
	m_dwNbRenderTexture				= 0;

	for( DWORD i = 0; i < RENDERMULTITEXTURE_NB_TEXTURE; i++ ){
		m_dwRenderTextureIndex[ i ]	= 0;
		m_pRenderTexture[ i ]		= NULL;
	}

	m_pDepthStencilSurface			= NULL;

	for( DWORD i = 0; i < RENDERMULTITEXTURE_NB_TEXTURE; i++ ){
		m_pCurrentRenderSurface[ i ]= NULL;
	}

	m_pCurrentDepthStencilSurface	= NULL;
	m_bUseDepthStencil				= FALSE;
}

//-----------------------------------------------------------------------------
// Name: MultiRenderTexture::~MultiRenderTexture
// Arguments:
// Returns:
//-----------------------------------------------------------------------------
MultiRenderTexture::~MultiRenderTexture()
{
}

//-----------------------------------------------------------------------------
// Name: MultiRenderTexture::Create
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
MultiRenderTexture* MultiRenderTexture::Create( LPDIRECT3DDEVICE		pd3dDevice )
{
	// create
	MultiRenderTexture*	pMultiRenderTexture;
	if( NULL == ( pMultiRenderTexture = new MultiRenderTexture ) )
		return	NULL;

    if( FAILED( pMultiRenderTexture->Construct( pd3dDevice ) ) ){
		pMultiRenderTexture->Release();
		return  NULL;
    }

	return	pMultiRenderTexture;
}


//-----------------------------------------------------------------------------
// Name: MultiRenderTexture::Construct
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
HRESULT MultiRenderTexture::Construct( LPDIRECT3DDEVICE		pd3dDevice )
{
	// store
	m_pd3dDevice			= pd3dDevice;

	return  S_OK;
}


//-----------------------------------------------------------------------------
// Name: MultiRenderTexture::Release
//	Delete render texture.
// Arguments:
// Returns:
//-----------------------------------------------------------------------------
void	MultiRenderTexture::Release()
{
	for( DWORD i = 0; i < RENDERMULTITEXTURE_NB_TEXTURE; i++ ){
		if( m_pRenderTexture[ i ] ){
			m_pRenderTexture[ i ]->Release();
			delete( m_pRenderTexture[ i ] );
			m_pRenderTexture[ i ]	= NULL;
		}
	}

	SAFE_RELEASE( m_pDepthStencilSurface );
}


//-----------------------------------------------------------------------------
// Name: MultiRenderTexture::CreateRenderTexture
// Arguments:
// Returns:
//	RenderTexture*
//-----------------------------------------------------------------------------
RenderTexture*	MultiRenderTexture::CreateRenderTexture( DWORD					dwRenderTextureIndex,
														 UINT					uWidth,
														 UINT					uHeight,
														 BOOL					bMipmap,
														 DWORD					dwUsage,
														 D3DFORMAT				d3dFormat,
														 D3DMULTISAMPLE_TYPE	d3dMultiSampleType )
{
	if( NULL == ( m_pRenderTexture[ dwRenderTextureIndex ]	= RenderTexture::Create( m_pd3dDevice,
																					 uWidth,
																					 uHeight,
																					 bMipmap,
																					 dwUsage,
																					 d3dFormat,
																					 D3DFMT_UNKNOWN,
																					 d3dMultiSampleType ) ) ){
		return	NULL;
    }
	
	m_dwRenderTextureIndex[ m_dwNbRenderTexture++ ]	= dwRenderTextureIndex;

	return	m_pRenderTexture[ dwRenderTextureIndex ];
}


//-----------------------------------------------------------------------------
// Name: MultiRenderTexture::BeginScene
//	Note system the begining of render texture.
// Arguments:
// Returns:
//-----------------------------------------------------------------------------
HRESULT MultiRenderTexture::BeginScene()
{
	// 1by1
	for( DWORD i = 0; i < m_dwNbRenderTexture; i++ ){
		DWORD	dwIndex	= m_dwRenderTextureIndex[ i ];
		m_pRenderTexture[ dwIndex ]->BeginScene( dwIndex );
	}

    return  S_OK;
}


//-----------------------------------------------------------------------------
// Name: MultiRenderTexture::EndScene
//	Note system the end of render texture.
// Arguments:
// Returns:
//-----------------------------------------------------------------------------
HRESULT MultiRenderTexture::EndScene()
{
	// 1by1
	for( INT i = m_dwNbRenderTexture - 1; i >= 0; i-- ){
		DWORD	dwIndex	= m_dwRenderTextureIndex[ i ];
		m_pRenderTexture[ dwIndex ]->EndScene();
	}

	return  S_OK;
}

//-----------------------------------------------------------------------------
// Name: GetTexture()
// Desc: 
//-----------------------------------------------------------------------------
RenderTexture*	MultiRenderTexture::GetRenderTexture( DWORD dwRenderTextureIndex )
{
	return	m_pRenderTexture[ dwRenderTextureIndex ];
}

//=============================================================================
// namespace
//=============================================================================
} // end of namespace
#endif