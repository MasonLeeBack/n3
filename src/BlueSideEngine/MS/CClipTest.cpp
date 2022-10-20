///////////////////////////////////////////////////////////////////////////////
// CClipTest.cpp
// Collection: CClipTest
//-----------------------------------------------------------------------------

//=============================================================================
// includes
//=============================================================================
#include "stdafx.h"
#ifdef	_XBOX
#include "CClipTest.h"
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
XMVECTOR	CClipTest::m_vPlane[ CCLIP_TEST_PLANE_MAX ];

//=============================================================================
// methods
//=============================================================================
//-----------------------------------------------------------------------------
// Name: CClipTest()
// Desc: 
//-----------------------------------------------------------------------------
CClipTest::CClipTest()
{
}


//-----------------------------------------------------------------------------
// Name: ~CClipTest()
// Desc: 
//-----------------------------------------------------------------------------
CClipTest::~CClipTest()
{
}


//-----------------------------------------------------------------------------
// Name: CClipTest::ComputeClipPlane
// Arguments:
// Returns:
//-----------------------------------------------------------------------------
HRESULT	CClipTest::ComputeClipPlane( FLOAT		fPers,
									 FLOAT		fAspect,
									 FLOAT		fNear,
									 FLOAT		fFar0,
									 FLOAT		fFar1,
									 FLOAT		fFar2,
									 XMMATRIX	mView,
									 XMVECTOR*	vPlane,
									 XMVECTOR*	vEye )
{
	// Clip Plane

	// fFovY2           = m_fPers/2
    // tan( fFovY2 )    = (y/2)/z
    // tan( fFovX2 )    = (x/2)/z
    // tan( fFovX2 )/tan( fFovY2 ) = x/y = fAspect
    // fFovX2           = atan( fAspect*tan( fFovY2 ) )
    FLOAT       fFovX2;
    FLOAT       fFovY2;
    XMMATRIX	mR;
	XMVECTOR	vPlaneTmp[ CCLIP_TEST_PLANE_MAX ];
	XMVECTOR	vPlaneTmp2;

	fFovY2  = fPers/2.0f;
	fFovX2  = atanf( fAspect*tanf( fFovY2 ) );

	// Left clip plane of view frustum
    mR	= XMMatrixRotationY( -fFovX2 );
	vPlaneTmp[ CCLIP_TEST_PLANE_LEFT ]		= XMPlaneTransform( XMVectorSet( 1.0f, 0.0f, 0.0f, 0.0f ), mR );

	// Right clip plane of view frustum
	mR	= XMMatrixRotationY( fFovX2 );
	vPlaneTmp[ CCLIP_TEST_PLANE_RIGHT ]		= XMPlaneTransform( XMVectorSet( -1.0f, 0.0f, 0.0f, 0.0f ), mR );

    // Bottom clip plane of view frustum
	mR	= XMMatrixRotationX( fFovY2 );
	vPlaneTmp[ CCLIP_TEST_PLANE_BOTTOM ]	= XMPlaneTransform( XMVectorSet( 0.0f, 1.0f, 0.0f, 0.0f ), mR );

	// Top clip plane of view frustum
	mR	= XMMatrixRotationX( -fFovY2 );
	vPlaneTmp[ CCLIP_TEST_PLANE_TOP ]		= XMPlaneTransform( XMVectorSet( 0.0f, -1.0f, 0.0f, 0.0f ), mR );

	// Near clip plane of view frustum
	vPlaneTmp[ CCLIP_TEST_PLANE_NEAR ]		= XMVectorSet( 0.0f, 0.0f, 1.0f, 0.0f );

	// Far clip plane of view frustum
	vPlaneTmp[ CCLIP_TEST_PLANE_FAR0 ]		= XMVectorSet( 0.0f, 0.0f, -1.0f, 0.0f );
	vPlaneTmp[ CCLIP_TEST_PLANE_FAR1 ]		= XMVectorSet( 0.0f, 0.0f, -1.0f, 0.0f );
	vPlaneTmp[ CCLIP_TEST_PLANE_FAR2 ]		= XMVectorSet( 0.0f, 0.0f, -1.0f, 0.0f );

	// View -> World
	XMVECTOR	vD;
	XMMATRIX	mInvView;
	mInvView	= XMMatrixInverse( &vD, mView );

	// Left Right Bottom Top
	XMVECTOR	vEw, vEv;
	vEv			= XMVectorZero();
	*vEye		= vEw	= XMVector3TransformCoord( vEv, mInvView );
	for( UINT i = 0; i < 4; i++ ){
		vPlaneTmp2		= XMPlaneTransform( vPlaneTmp[ i ], mInvView );
		vPlaneTmp2.w	= -vPlaneTmp2.x*vEw.x -vPlaneTmp2.y*vEw.y -vPlaneTmp2.z*vEw.z;
		vPlane[ i ]		= XMPlaneNormalize( vPlaneTmp2 );
	}

	// Near
	vEv								= XMVectorSet( 0.0f, 0.0f, fNear, 1.0f );
	vEw								= XMVector3TransformCoord( vEv, mInvView );
	vPlaneTmp2						= XMPlaneTransform( vPlaneTmp[ CCLIP_TEST_PLANE_NEAR ], mInvView );
	vPlaneTmp2.w					= -vPlaneTmp2.x*vEw.x -vPlaneTmp2.y*vEw.y -vPlaneTmp2.z*vEw.z;
	vPlane[ CCLIP_TEST_PLANE_NEAR ]	= XMPlaneNormalize( vPlaneTmp2 );

	// Far0
	vEv								= XMVectorSet( 0.0f, 0.0f, fFar0, 1.0f );
	vEw								= XMVector3TransformCoord( vEv, mInvView );
	vPlaneTmp2						= XMPlaneTransform( vPlaneTmp[ CCLIP_TEST_PLANE_FAR0 ], mInvView );
	vPlaneTmp2.w					= -vPlaneTmp2.x*vEw.x -vPlaneTmp2.y*vEw.y -vPlaneTmp2.z*vEw.z;
	vPlane[ CCLIP_TEST_PLANE_FAR0 ]	= XMPlaneNormalize( vPlaneTmp2 );

	// Far1
	vEv								= XMVectorSet( 0.0f, 0.0f, fFar1, 1.0f );
	vEw								= XMVector3TransformCoord( vEv, mInvView );
	vPlaneTmp2						= XMPlaneTransform( vPlaneTmp[ CCLIP_TEST_PLANE_FAR1 ], mInvView );
	vPlaneTmp2.w					= -vPlaneTmp2.x*vEw.x -vPlaneTmp2.y*vEw.y -vPlaneTmp2.z*vEw.z;
	vPlane[ CCLIP_TEST_PLANE_FAR1 ]	= XMPlaneNormalize( vPlaneTmp2 );

	// Far2
	vEv								= XMVectorSet( 0.0f, 0.0f, fFar2, 1.0f );
	vEw								= XMVector3TransformCoord( vEv, mInvView );
	vPlaneTmp2						= XMPlaneTransform( vPlaneTmp[ CCLIP_TEST_PLANE_FAR2 ], mInvView );
	vPlaneTmp2.w					= -vPlaneTmp2.x*vEw.x -vPlaneTmp2.y*vEw.y -vPlaneTmp2.z*vEw.z;
	vPlane[ CCLIP_TEST_PLANE_FAR2 ]	= XMPlaneNormalize( vPlaneTmp2 );

	return	S_OK;
}


//-----------------------------------------------------------------------------
// Name: CClipTest::Test
// Arguments:
// Returns:
//	BOOL
//-----------------------------------------------------------------------------
BOOL	CClipTest::Test( const XMVECTOR		vCenter,
						 const FLOAT		fRadiusXRadius )
{
	XMVECTOR	vC;
	vC		= vCenter;
	vC.w	= 1.0f;

	for( UINT i = 0; i < 6; i++ ){
		XMVECTOR	vD;
		vD		= XMVector4Dot( m_vPlane[ i ], vC );
		if( vD.x*vD.x > fRadiusXRadius ){
			if( vD.x < 0.0f )
				return	FALSE;
		}
	}

	return	TRUE;
}


//-----------------------------------------------------------------------------
// Name: CClipTest::TestFar1
// Arguments:
// Returns:
//	BOOL
//-----------------------------------------------------------------------------
BOOL	CClipTest::TestFar1( const XMVECTOR		vCenter,
							 const FLOAT		fRadiusXRadius )
{
	XMVECTOR	vC;
	XMVECTOR	vD;
	vC		= vCenter;
	vC.w	= 1.0f;

	for( UINT i = 0; i < 5; i++ ){
		vD		= XMVector4Dot( m_vPlane[ i ], vC );
		if( vD.x*vD.x > fRadiusXRadius ){
			if( vD.x < 0.0f )
				return	FALSE;
		}
	}

	vD		= XMVector4Dot( m_vPlane[ CCLIP_TEST_PLANE_FAR1 ], vC );
	if( vD.x*vD.x > fRadiusXRadius ){
		if( vD.x < 0.0f )
			return	FALSE;
	}

	return	TRUE;
}

//-----------------------------------------------------------------------------
// Name: CClipTest::TestFar2
// Arguments:
// Returns:
//	BOOL
//-----------------------------------------------------------------------------
BOOL	CClipTest::TestFar2( const XMVECTOR		vCenter,
							 const FLOAT		fRadiusXRadius )
{
	XMVECTOR	vC;
	XMVECTOR	vD;
	vC		= vCenter;
	vC.w	= 1.0f;

	for( UINT i = 0; i < 5; i++ ){
		vD		= XMVector4Dot( m_vPlane[ i ], vC );
		if( vD.x*vD.x > fRadiusXRadius ){
			if( vD.x < 0.0f )
				return	FALSE;
		}
	}

	vD		= XMVector4Dot( m_vPlane[ CCLIP_TEST_PLANE_FAR2 ], vC );
	if( vD.x*vD.x > fRadiusXRadius ){
		if( vD.x < 0.0f )
			return	FALSE;
	}

	return	TRUE;
}


//-----------------------------------------------------------------------------
// Name: CClipTest::SetClipPlane
// Arguments:
// Returns:
//	BOOL
//-----------------------------------------------------------------------------
HRESULT	CClipTest::SetClipPlane( const XMVECTOR*	pPlane )
{
	for( UINT i = 0; i < CCLIP_TEST_PLANE_MAX; i++ ){
		m_vPlane[ i ]	= pPlane[ i ];
	}

	return	S_OK;
}


//=============================================================================
// namespace
//=============================================================================
#endif