///////////////////////////////////////////////////////////////////////////////
// CBoundingBox.cpp
// Collection: CBoundingBox
//-----------------------------------------------------------------------------

//=============================================================================
// includes
//=============================================================================
#include "stdafx.h"
#ifdef	_XBOX
#include "CBoundingBox.h"
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
// Name: CBoundingBox()
// Desc: 
//-----------------------------------------------------------------------------
CBoundingBox::CBoundingBox()
{
	m_vCenter	= XMFLOAT3( 0.0f, 0.0f, 0.0f );
	m_fRotateY	= 0.0f;
	m_mW		= _XMMATRIX( 1.0f, 0.0f, 0.0f, 0.0f,
							 0.0f, 1.0f, 0.0f, 0.0f,
							 0.0f, 0.0f, 1.0f, 0.0f,
							 0.0f, 0.0f, 0.0f, 1.0f );
	m_pBox		= NULL;
	m_bClip		= FALSE;
}


//-----------------------------------------------------------------------------
// Name: ~CBoundingBox()
// Desc: 
//-----------------------------------------------------------------------------
CBoundingBox::~CBoundingBox()
{
}


//-----------------------------------------------------------------------------
// Name: CBoundingBox::Create
// Arguments:
// Returns:
//	CBoundingBox*
//-----------------------------------------------------------------------------
CBoundingBox*	CBoundingBox::Create( LPDIRECT3DDEVICE	pd3dDevice,
									  const	XMFLOAT3&	vCenter,
									  const	FLOAT		fRotateY,
									  const FLOAT		fX,
									  const FLOAT		fY,
									  const FLOAT		fZ )
{
	CBoundingBox*  pCBoundingBox;
	if( NULL == ( pCBoundingBox = new CBoundingBox ) )
  		return	NULL;

	if( FAILED( pCBoundingBox->Construct( pd3dDevice, vCenter, fRotateY, fX, fY, fZ ) ) ){
		pCBoundingBox->Release();	// delete CBoundingBox instance in Release().
		return	NULL;
	}

	return pCBoundingBox;
}


//-----------------------------------------------------------------------------
// Name: CBoundingBox::Construct
// Arguments:
// Returns:
//	HRESULT
//-----------------------------------------------------------------------------
HRESULT CBoundingBox::Construct( LPDIRECT3DDEVICE	pd3dDevice,
								 const	XMFLOAT3&	vCenter,
								 const	FLOAT		fRotateY,
								 const	FLOAT		fX,
								 const	FLOAT		fY,
								 const	FLOAT		fZ )
{
    // this
	m_vCenter	= vCenter;
	m_fRotateY	= fRotateY;
	m_fRadius	= sqrtf( fX/2.0f*fX/2.0f + fY/2.0f*fY/2.0f + fZ/2.0f*fZ/2.0f );
	XMMATRIX	mT, mR;
	mT		= XMMatrixTranslation( vCenter.x, vCenter.y, vCenter.z );
	mR		= XMMatrixRotationY( fRotateY );
	m_mW	= mR*mT;

	// Box
	if( NULL == ( m_pBox = xl::Hexahedron::Create( pd3dDevice, fX, fY, fZ, 1 ) ) )
		return	E_FAIL;
	
	return S_OK;
}


//-----------------------------------------------------------------------------
// Name: CBoundingBox::Release
// Arguments:
// Returns:
//-----------------------------------------------------------------------------
void	CBoundingBox::Release()
{
// [PREFIX:beginmodify] 2006/2/15 junyash PS#5179 reports CBoundingBox::Create() momory leaking 'pCBoundingBox'.
	#if 0
	if( m_pBox ){
		m_pBox->Release();		// These code
		SAFE_RELEASE( m_pBox );	// call m_pBox->Relase() twice.
		m_pBox	= NULL;
	}
	#else
	SAFE_RELEASE( m_pBox );
	delete this;				// I think CBoundingBoxManager.cpp needs delete this in Release()
	#endif
// [PREFIX:endmodify] junyash
}


//-----------------------------------------------------------------------------
// Name: CBoundingBox::GetResource
// Arguments:
// Returns:
//-----------------------------------------------------------------------------
HRESULT	CBoundingBox::GetResource( XMFLOAT3&	vCenter,
								   FLOAT&		fRadius )
{
	vCenter	= m_vCenter;
	fRadius	= m_fRadius;

	return	S_OK;
}


//-----------------------------------------------------------------------------
// Name: CBoundingBox::Render
// Arguments:
// Returns:
//	HRESULT
//-----------------------------------------------------------------------------
HRESULT	CBoundingBox::Render( const XMMATRIX& mVP,
							  const D3DXCOLOR d3dxColor )
{
	XMMATRIX	mWVP	= m_mW*mVP;
	m_pBox->RenderLine( mWVP, d3dxColor );

	return	S_OK;
}


//-----------------------------------------------------------------------------
// Name: CBoundingBox::RenderClip
// Arguments:
// Returns:
//	HRESULT
//-----------------------------------------------------------------------------
HRESULT	CBoundingBox::RenderClip( const XMMATRIX& mVP,
								  const D3DXCOLOR d3dxColorIn,
								  const D3DXCOLOR d3dxColorOut )
{
	XMMATRIX	mWVP	= m_mW*mVP;

	if( m_bClip )
		m_pBox->RenderLine( mWVP, d3dxColorIn );
	else
		m_pBox->RenderLine( mWVP, d3dxColorOut );

	return	S_OK;
}


//=============================================================================
// namespace
//=============================================================================
#endif