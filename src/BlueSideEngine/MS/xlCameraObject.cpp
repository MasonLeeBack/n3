///////////////////////////////////////////////////////////////////////////////
// xlCameraObject.cpp
// Collection: xlCamera
//-----------------------------------------------------------------------------

//=============================================================================
// includes
//=============================================================================
#include "stdafx.h"
#ifdef	_XBOX
#include "xlCameraObject.h"
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
static FLOAT	s_fPers		= 42.0f;
static FLOAT	s_fAspect	= 16.0f/9.0f;
static FLOAT	s_fNear		= 0.1f;
static FLOAT	s_fFar		= 1000.0f;
static XMMATRIX	s_mView		= _XMMATRIX( 1.0f, 0.0f, 0.0f, 0.0f,
				                         0.0f, 1.0f, 0.0f, 0.0f,
				                         0.0f, 0.0f, 1.0f, 0.0f,
				                         0.0f, 0.0f, 0.0f, 1.0f );

//=============================================================================
// methods
//=============================================================================

//-----------------------------------------------------------------------------
// Name: CameraObject()
// Desc: 
//-----------------------------------------------------------------------------
CameraObject::CameraObject()
{
	m_pFrustum		= NULL;
	m_pPlane[ 0 ]	= NULL;
	m_pPlane[ 1 ]	= NULL;
}


//-----------------------------------------------------------------------------
// Name: ~CameraObject()
// Desc: 
//-----------------------------------------------------------------------------
CameraObject::~CameraObject()
{
}


//-----------------------------------------------------------------------------
// Name: Create()
// Desc: 
//-----------------------------------------------------------------------------
CameraObject*	CameraObject::Create( LPDIRECT3DDEVICE pd3dDevice, float fCameraSize )
{
    CameraObject*  pCameraObject;
    if( NULL == ( pCameraObject = new CameraObject ) )
  		return	NULL;

    if( FAILED( pCameraObject->Construct( pd3dDevice, fCameraSize ) ) ){
	    pCameraObject->Release();
	    return	NULL;
    }

    return pCameraObject;
}


//-----------------------------------------------------------------------------
// Name: CameraObject::Construct
// Arguments:
// Returns:
//	HRESULT
//-----------------------------------------------------------------------------
HRESULT CameraObject::Construct( LPDIRECT3DDEVICE pd3dDevice, float fCameraSize )
{
	// store
	m_pd3dDevice	= pd3dDevice;
    m_fCameraSize   = fCameraSize;

    // frustum
 	if( NULL == ( m_pFrustum = Frustum::Create( pd3dDevice, 1.0f, 1.0f, 1.0f ) ) )
		return	E_FAIL;

	// near plane
	if( NULL == ( m_pPlane[ 0 ] = Rectangle::Create( pd3dDevice, fCameraSize/2.0f, fCameraSize/2.0f, 1, 1 ) ) )
		return	E_FAIL;

    // lines
	if( NULL == ( m_pLine = Line::Create( pd3dDevice, 8, NULL, D3DPT_LINELIST ) ) )
		return	E_FAIL;
 
    // modeling transform
	// Only the frustom is static.
	XMMATRIX	mR, mT;
	mT	= XMMatrixTranslation( 0.0f, 0.0f, fCameraSize );
	mR	= XMMatrixRotationX( -D3DX_PI/2.0f );
	m_mM[ 0 ]	= mR*mT;
		
    // setup
	Update( s_fPers, s_fAspect, s_fNear, s_fFar, s_mView );

    return S_OK;
}


//-----------------------------------------------------------------------------
// Name: CameraObject::Release
// Arguments:
// Returns:
//-----------------------------------------------------------------------------
void	CameraObject::Release()
{
	SAFE_RELEASE( m_pFrustum );
	SAFE_RELEASE( m_pPlane[ 0 ] );
	SAFE_RELEASE( m_pLine );
}


//-----------------------------------------------------------------------------
// Name: CameraObject::Update
// Arguments:
// Returns:
//	HRESULT
//-----------------------------------------------------------------------------
HRESULT	CameraObject::Update(  FLOAT		fPers,
							   FLOAT		fAspect,
							   FLOAT		fNear,
							   FLOAT		fFar,
							   XMMATRIX&	mView )
{
	// mInvV
	XMVECTOR	vD;
	m_mInvV	= XMMatrixInverse( &vD, mView );

	// GEOMETRY
	// frustum
	// Ž®‚ÌˆÓ–¡‚ÍGetClipPlane‚ðŽQÆB
	float       fFovX2;
	float       fFovY2;
	fFovY2  = fPers/2.0f;
	fFovX2  = atanf( fAspect*tanf( fFovY2 ) );

	float       fSizeX;
	float       fSizeZ;
	float       fSizeY;
	fSizeY  = m_fCameraSize;
	fSizeX  = 2.0f*fSizeY*tanf( fFovX2 );
	fSizeZ  = 2.0f*fSizeY*tanf( fFovY2 );
	m_pFrustum->SetSize( fSizeX, fSizeZ, fSizeY );

	// near plane
	XMMATRIX	mR, mT;
	mT	= XMMatrixTranslation( 0.0f, 0.0f, fNear );
	mR	= XMMatrixRotationX( -D3DX_PI/2.0f );
	m_mM[ 1 ]	= mR*mT;

	// lines
	{
		float   fZ  = fFar;
		float   fX  = fZ*tanf( fFovX2 );
		float   fY  = fZ*tanf( fFovY2 );
		D3DXVECTOR3 pVertices[ 8 ];
		pVertices[ 0 ]  = D3DXVECTOR3( 0.0f,   0.0f,  0.0f );
		pVertices[ 1 ]  = D3DXVECTOR3(  -fX,     fY,    fZ );
		pVertices[ 2 ]  = D3DXVECTOR3( 0.0f,   0.0f,  0.0f );
		pVertices[ 3 ]  = D3DXVECTOR3(   fX,     fY,    fZ );
		pVertices[ 4 ]  = D3DXVECTOR3( 0.0f,   0.0f,  0.0f );
		pVertices[ 5 ]  = D3DXVECTOR3(  -fX,    -fY,    fZ );
		pVertices[ 6 ]  = D3DXVECTOR3( 0.0f,   0.0f,  0.0f );
		pVertices[ 7 ]  = D3DXVECTOR3(   fX,    -fY,    fZ );
   		if( FAILED( m_pLine->SetVertices( 8, pVertices, D3DPT_LINELIST ) ) )
			return	E_FAIL;
	}

    return	S_OK;
}


//-----------------------------------------------------------------------------
// Name: CameraObject::Render
// Arguments:
// Returns:
//	HRESULT
//-----------------------------------------------------------------------------
HRESULT CameraObject::Render( const XMMATRIX&	mVP,
							  const D3DXCOLOR d3dColor )
{
	// frustum
	XMMATRIX	mM;
	mM	= m_mM[ 0 ]*m_mInvV*mVP;
	m_pFrustum->RenderLine( mM, d3dColor );

	// near plane
	mM	= m_mM[ 1 ]*m_mInvV*mVP;
	m_pPlane[ 0 ]->RenderLine( mM, d3dColor );

	// lines
	mM	= m_mInvV*mVP;
	m_pLine->Render( mM, d3dColor );

    return  S_OK;
}

//=============================================================================
// namespace
//=============================================================================
} // end of namespace
#endif