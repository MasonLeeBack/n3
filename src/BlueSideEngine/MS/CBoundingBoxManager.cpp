///////////////////////////////////////////////////////////////////////////////
// CBoundingBoxManager.cpp
// Collection: CBoundingBoxManager
//-----------------------------------------------------------------------------

//=============================================================================
// includes
//=============================================================================
#include "stdafx.h"
#ifdef	_XBOX
#include "CBoundingBoxManager.h"
//=============================================================================
// namespace
//=============================================================================
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
// Name: CBoundingBoxManager()
// Desc: 
//-----------------------------------------------------------------------------
CBoundingBoxManager::CBoundingBoxManager()
{
	m_pd3dDevice		= NULL;
	m_uNbCBoundingBox	= 0;

	for( UINT i = 0; i < CBOUNDINGBOX_NB_MAX; i++ ){
		m_pCBoundingBox[ i ]	= NULL;
	}
}


//-----------------------------------------------------------------------------
// Name: ~CBoundingBoxManager()
// Desc: 
//-----------------------------------------------------------------------------
CBoundingBoxManager::~CBoundingBoxManager()
{
}


//-----------------------------------------------------------------------------
// Name: CBoundingBoxManager::Create
// Arguments:
// Returns:
//	CBoundingBoxManager*
//-----------------------------------------------------------------------------
CBoundingBoxManager*	CBoundingBoxManager::Create( LPDIRECT3DDEVICE	pd3dDevice )
{
	CBoundingBoxManager*  pCBoundingBoxManager;
	if( NULL == ( pCBoundingBoxManager = new CBoundingBoxManager ) )
  		return	NULL;

	if( FAILED( pCBoundingBoxManager->Construct( pd3dDevice ) ) ){
		pCBoundingBoxManager->Release();
		return	NULL;
	}

	return pCBoundingBoxManager;
}


//-----------------------------------------------------------------------------
// Name: CBoundingBoxManager::Construct
// Arguments:
// Returns:
//	HRESULT
//-----------------------------------------------------------------------------
HRESULT CBoundingBoxManager::Construct( LPDIRECT3DDEVICE	pd3dDevice )
{
	m_pd3dDevice	= pd3dDevice;

	return S_OK;
}


//-----------------------------------------------------------------------------
// Name: CBoundingBoxManager::Release
// Arguments:
// Returns:
//-----------------------------------------------------------------------------
void	CBoundingBoxManager::Release()
{
	for( UINT i = 0; i < m_uNbCBoundingBox; i++ ){
		if( m_pCBoundingBox[ i ] ){
			SAFE_RELEASE( m_pCBoundingBox[ i ] );
			m_pCBoundingBox[ i ]	= NULL;
		}
	}

	m_uNbCBoundingBox	= 0;
}

//-----------------------------------------------------------------------------
// Name: CBoundingBoxManager::Release
// Arguments:
// Returns:
//-----------------------------------------------------------------------------
CBoundingBox*	CBoundingBoxManager::CreateBoundingBox(	const XMFLOAT3&		vCenter,
														const FLOAT			fRotateY,
														const FLOAT			fWidth,
														const FLOAT			fHeight,
														const FLOAT			fDepth )
{
// [PREFIX:beginmodify] 2006/2/15 junyash PS#5180 reports Buffer Overrun writing to m_pCBoundingBox in CBoundingBoxManager::CreateBoundingBox()
	//if( CBOUNDINGBOX_NB_MAX < m_uNbCBoundingBox )
	if( CBOUNDINGBOX_NB_MAX <= m_uNbCBoundingBox )
// [PREFIX:endmodify] junyash
		return	NULL;

	if( NULL == ( m_pCBoundingBox[ m_uNbCBoundingBox ]	= CBoundingBox::Create( m_pd3dDevice,
																				vCenter,
																				fRotateY,
																				fWidth,
																				fHeight,
																				fDepth ) ) ){
		return	NULL;
	}

	return	m_pCBoundingBox[ m_uNbCBoundingBox++ ];
}


//-----------------------------------------------------------------------------
// Name: CBoundingBoxManager::Render
// Arguments:
// Returns:
//	HRESULT
//-----------------------------------------------------------------------------
HRESULT	CBoundingBoxManager::Render( const XMMATRIX&	mVP,
								    const D3DXCOLOR	d3dxColorIn,
								    const D3DXCOLOR	d3dxColorOut )
{
	for( UINT i = 0; i < m_uNbCBoundingBox; i++ ){
		m_pCBoundingBox[ i ]->RenderClip( mVP, d3dxColorIn, d3dxColorOut );
	}

	return	S_OK;
}


//=============================================================================
// namespace
//=============================================================================
#endif