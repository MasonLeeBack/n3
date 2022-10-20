///////////////////////////////////////////////////////////////////////////////
// xlCamera.cpp
// Colloction: xlCamera
//-----------------------------------------------------------------------------

//=============================================================================
// includes
//=============================================================================
#include "stdafx.h"
#ifdef	_XBOX
#include "xlCamera.h"
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
// Name: Camera::Camera
// Arguments:
// Returns:
//-----------------------------------------------------------------------------
Camera::Camera()
{
	m_bHalt	= false;
}


//-----------------------------------------------------------------------------
// Name: Camera::~Camera
// Arguments:
// Returns:
//-----------------------------------------------------------------------------
Camera::~Camera()
{
}


//-----------------------------------------------------------------------------
// Name: Camera::Create
//	static public
//	Create. 作成
// Arguments:
//	None.
// Returns:
//	[Camera*]	Pointer.
//-----------------------------------------------------------------------------
Camera* Camera::Create( LPDIRECT3DDEVICE pd3dDevice )
{
    Camera*  pCamera;
    if( NULL == ( pCamera = new Camera ) )
  		return	NULL;

    if( FAILED( pCamera->Construct( pd3dDevice ) ) )
        return  NULL;

    return pCamera;
}


//-----------------------------------------------------------------------------
// Name: Camera::Construct
// Arguments:
//	None.
// Returns:
//	HRESULT	
//-----------------------------------------------------------------------------
HRESULT    Camera::Construct( LPDIRECT3DDEVICE pd3dDevice )
{
    // this
    m_pd3dDevice = pd3dDevice;

    // init
    Default();

    return  S_OK;
}


//-----------------------------------------------------------------------------
// Name: Camera::Release
//	public
//	Release. 解放
// Arguments:
//	None.
// Returns:
//-----------------------------------------------------------------------------
void    Camera::Release()
{
}


//-----------------------------------------------------------------------------
// Name: Camera::Default
//	Init Camera status.
// Arguments:
//	None.
// Returns:
//
//-----------------------------------------------------------------------------
void	Camera::Default()
{
    m_bHalt = false;
	
	m_pd3dDevice->GetViewport( &m_d3dViewport );
    float   fAspect = ( float )m_d3dViewport.Width / ( float )m_d3dViewport.Height;

    // 通常イニシャライズされていないMinZは０であるので、ここはあえて使わないほうが良い。
    /*
    float   fNear   = m_d3dViewport.MinZ;
    float   fFar    = m_d3dViewport.MaxZ;
    */
    float   fNear   = 0.1f;
    float   fFar    = 50.0f;

    // 身長1.8mの人がｚ?イナスの方向から原?を向いている状態。

    // defaut setup
	Update( 0.0f,  1.8f, -5.0f,
		    0.0f,  0.0f,  0.0f,
		    0.0f,  1.0f,  0.0f,
		    D3DX_PI/4, fAspect, fNear, fFar );
}


//-----------------------------------------------------------------------------
// Name: Camera::Update
//	Update Camera status.
// Arguments:
//	fFromX		from x
//	fFromY		from y
//	fFromZ		from z
//	fAtX		at x
//	fAtY		at y
//	fAtZ		at z
//	fUpX		up x
//	fUpY		up y
//	fUpZ		up z
//	fPers		perspective
//	fAspect		aspect ratio
//	fNear		near
//	fFar		far
// Returns:
//  HRESULT
//-----------------------------------------------------------------------------
HRESULT	Camera::Update( float	fFromX,	float fFromY,	float fFromZ,
					    float	fAtX,	float fAtY,		float fAtZ,
					    float	fUpX,	float fUpY,		float fUpZ,
					    float	fPers,	float fAspect,	float fNear, float fFar )
{
    // halt
    if( m_bHalt )
        return  E_FAIL;

	m_vFrom     = XMVectorSet( fFromX, fFromY, fFromZ, 0.0f );
	m_vAt       = XMVectorSet( fAtX, fAtY, fAtZ, 0.0f );
	m_vUp       = XMVectorSet( fUpX, fUpY, fUpZ, 0.0f );
    m_fPers     = fPers;
    m_fAspect   = fAspect;
    m_fNear     = fNear;
    m_fFar      = fFar;

	m_mView			= XMMatrixLookAtLH( m_vFrom, m_vAt, m_vUp );
	XMVECTOR	vDeterminant;
	m_mViewInverse	= XMMatrixInverse( &vDeterminant, m_mView );
	m_mProj			= XMMatrixPerspectiveFovLH( m_fPers, m_fAspect, m_fNear, m_fFar );

    return  S_OK;
}


//-----------------------------------------------------------------------------
// Name: Camera::Update
//	Update Camera status.
// Arguments:
//	fFromX		from x
//	fFromY		from y
//	fFromZ		from z
//	fAtX		at x
//	fAtY		at y
//	fAtZ		at z
//	fUpX		up x
//	fUpY		up y
//	fUpZ		up z
//	fLeft		left boundary
//	fRight		right boundary
//	fBottom		bottom boundary
//	fTop		top boundary
//	fNear		near
//	fFar		far
// Returns:
//  HRESULT
//-----------------------------------------------------------------------------
HRESULT Camera::Update( float fFromX,	float fFromY,	float fFromZ,
					    float fAtX,	    float fAtY,		float fAtZ,
					    float fUpX,	    float fUpY,		float fUpZ,
					    float fLeft,	float fRight,   float fBottom,  float fTop,
                        float fNear,    float fFar )
{
    // halt
    if( m_bHalt )
        return  E_FAIL;

	m_vFrom     = XMVectorSet( fFromX, fFromY, fFromZ, 0.0f );
	m_vAt       = XMVectorSet( fAtX, fAtY, fAtZ, 0.0f );
	m_vUp       = XMVectorSet( fUpX, fUpY, fUpZ, 0.0f );
    m_fLeft     = fLeft;
    m_fRight    = fRight;
    m_fBottom   = fBottom;
    m_fTop      = fTop;
    m_fNear     = fNear;
    m_fFar      = fFar;

	m_mView			= XMMatrixLookAtLH( m_vFrom, m_vAt, m_vUp );
	XMVECTOR	vDeterminant;
	m_mViewInverse	= XMMatrixInverse( &vDeterminant, m_mView );
	m_mProj			= XMMatrixOrthographicOffCenterLH( m_fLeft, m_fRight, m_fBottom, m_fTop, m_fNear, m_fFar );

    return  S_OK;
}


//-----------------------------------------------------------------------------
// Name: Camera::Update
//	Update Camera status.
// Arguments:
//	fdFromX		    differential from x  -128~127
//	fdFromY		    differential from y  -128~127
//	fdAtX		    differential at x    -128~127 
//	fdAtY		    differential at y    -128~127
//	bPress		    change translation xz to y
//  fTransScale     scale for translation 
//  fRotScale       scale for rotation
//	fElapsedTime	time
// Returns:
//	HRESULT
//-----------------------------------------------------------------------------
HRESULT	Camera::Update( float   fdFromX, float fdFromY,
                        float   fdAtX,   float fdAtY,
                        bool    bPress,
                        float   fTransScale, float fRotScale,
                        float   fElapsedTime )
{
    // halt
    if( m_bHalt )
        return  E_FAIL;

	// update view position
	fdFromX *= fdFromX * fdFromX; // fdFromX cubed
	fdFromY *= fdFromY * fdFromY; // fdFromY cubed
	
	XMVECTOR	vOffset;
	if( bPress )
		vOffset = XMVectorSet( 0.0f, fdFromY, 0.0f, 0.0f );
	else{
		vOffset = XMVectorSet( fdFromX, 0.0f, fdFromY, 0.0f );
		vOffset	= XMVector3TransformNormal( vOffset, m_mViewInverse );
		m_vUp = XMVector3Normalize( m_vUp );
		vOffset -= XMVector3Dot( vOffset, m_vUp ) * m_vUp;
	}

	vOffset	= XMVector3Normalize( vOffset );
	vOffset *= fTransScale * fElapsedTime;
	m_vFrom += vOffset;
	m_vAt	+= vOffset;

	// update view angle
	XMVECTOR	vAtOffset;
	vAtOffset	= XMVectorZero();
	fdAtX	*= fdAtX * fdAtX; // fdAtX cubed
	fdAtY	*= fdAtY * fdAtY; // fdAtY cubed
	vAtOffset.x += fRotScale * fdAtX * fElapsedTime;
	XMVECTOR	vE = m_vAt - m_vFrom;
	vE		= XMVector3Normalize( vE );
	FLOAT	fThreshold = 0.99f;
	FLOAT	fEdotU = XMVector3Dot( vE, m_vUp ).x;
	if ( ( fEdotU < -fThreshold && fdAtY < 0.0f )						// near -vUp, but positive movement
		|| ( fEdotU > fThreshold && fdAtY > 0.0f )						// near vUp, but negative movement
		|| ( fEdotU > -fThreshold && fEdotU < fThreshold ) )			// ordinary case
		vAtOffset.y -= fRotScale * fdAtY * fElapsedTime;				// screen-space Y displacement means up-down view turn
	vAtOffset	= XMVector3TransformNormal( vAtOffset, m_mViewInverse );
	m_vAt += vAtOffset;

    // matrix
	m_mView			= XMMatrixLookAtLH( m_vFrom, m_vAt, m_vUp );
	XMVECTOR	vDeterminant;
	m_mViewInverse	= XMMatrixInverse( &vDeterminant, m_mView );

    return  S_OK;
}


//-----------------------------------------------------------------------------
// Name: Camera::GetViewMatrix
//	Get matrix.
// Arguments:
// Returns:
//	XMMATRIX
//-----------------------------------------------------------------------------
XMMATRIX	Camera::GetViewMatrix()
{
	return	m_mView;
}


//-----------------------------------------------------------------------------
// Name: Camera::GetProjectionMatrix
// Arguments:
// Returns:
//	XMMATRIX
//-----------------------------------------------------------------------------
XMMATRIX	Camera::GetProjectionMatrix()
{
	return	m_mProj;
}


//-----------------------------------------------------------------------------
// Name: Camera::GetClipPlane
// Arguments:
//  pPlane
// Returns:
//	HRESULT
//-----------------------------------------------------------------------------
HRESULT Camera::GetClipPlane( XMVECTOR pPlane[] )
{
    // fFovY2           = m_fPers/2
    // tan( fFovY2 )    = (y/2)/z
    // tan( fFovX2 )    = (x/2)/z
    // tan( fFovX2 )/tan( fFovY2 ) = x/y = fAspect
    // fFovX2           = atan( fAspect*tan( fFovY2 ) )
    FLOAT       fFovX2;
    FLOAT       fFovY2;
    XMMATRIX	mR;

	fFovY2  = m_fPers/2.0f;
	fFovX2  = atanf( m_fAspect*tanf( fFovY2 ) );

	// 全てView Frustumの内側を向いている

	// Left clip plane of view frustum
    mR	= XMMatrixRotationY( -fFovX2 );
	pPlane[ XL_CAMERA_CLIPPLANE_LEFT]		= XMPlaneTransform( XMVectorSet( 1.0f, 0.0f, 0.0f, 0.0f ), mR );

	// Right clip plane of view frustum
	mR	= XMMatrixRotationY( fFovX2 );
	pPlane[ XL_CAMERA_CLIPPLANE_RIGHT ]		= XMPlaneTransform( XMVectorSet( -1.0f, 0.0f, 0.0f, 0.0f ), mR );

    // Bottom clip plane of view frustum
	mR	= XMMatrixRotationX( fFovY2 );
	pPlane[ XL_CAMERA_CLIPPLANE_BOTTOM ]	= XMPlaneTransform( XMVectorSet( 0.0f, 1.0f, 0.0f, 0.0f ), mR );

	// Upper clip plane of view frustum
	mR	= XMMatrixRotationX( -fFovY2 );
	pPlane[ XL_CAMERA_CLIPPLANE_UPPER ]		= XMPlaneTransform( XMVectorSet( 0.0f, -1.0f, 0.0f, 0.0f ), mR );

	// Near clip plane of view frustum
	pPlane[ XL_CAMERA_CLIPPLANE_NEAR ]		= XMPlaneNormalize( XMVectorSet( 0.0f, 0.0f, 1.0f, m_fNear ) );

	// Far clip plane of view frustum
	pPlane[ XL_CAMERA_CLIPPLANE_FAR ]		= XMPlaneNormalize( XMVectorSet( 0.0f, 0.0f, -1.0f, m_fFar ) );

    return  S_OK;
}


//=============================================================================
// namespace
//=============================================================================
} // end of namespace
#endif