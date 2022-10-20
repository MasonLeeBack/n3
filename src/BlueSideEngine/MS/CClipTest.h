///////////////////////////////////////////////////////////////////////////////
// CClipTest.h
// Collection: CClipTest 
//-----------------------------------------------------------------------------
#ifndef __inc_CClipTest_h__
#define __inc_CClipTest_h__

//=============================================================================
// includes
//=============================================================================
#include "xlCommon.h"
#ifdef	_XBOX
//=============================================================================
// namespace
//=============================================================================
//=============================================================================
// macros
//=============================================================================
enum{
	CCLIP_TEST_PLANE_LEFT	= 0,
	CCLIP_TEST_PLANE_RIGHT,
	CCLIP_TEST_PLANE_BOTTOM,
	CCLIP_TEST_PLANE_TOP,
	CCLIP_TEST_PLANE_NEAR,
	CCLIP_TEST_PLANE_FAR0,
	CCLIP_TEST_PLANE_FAR1,
	CCLIP_TEST_PLANE_FAR2,
	CCLIP_TEST_PLANE_MAX,
};

//=============================================================================
// structs
//=============================================================================
//=============================================================================
// classes
//=============================================================================
class CClipTest {
	protected:
		static XMVECTOR			m_vPlane[ CCLIP_TEST_PLANE_MAX ];

	public:
		CClipTest();
		~CClipTest();

		static HRESULT			ComputeClipPlane( FLOAT		fPers,
												  FLOAT		fAspect,
												  FLOAT		fNear,
												  FLOAT		fFar0,
												  FLOAT		fFar1,
												  FLOAT		fFar2,
												  XMMATRIX	mView,
												  XMVECTOR*	vPlane,
												  XMVECTOR*	vE );

		static HRESULT			SetClipPlane( const XMVECTOR*	pPlane );
		static BOOL				Test( const XMVECTOR	vCenter,
									  const FLOAT		fRadiusXRadius );
		static BOOL				TestFar1( const XMVECTOR	vCenter,
										  const FLOAT		fRadiusXRadius );
		static BOOL				TestFar2( const XMVECTOR	vCenter,
										  const FLOAT		fRadiusXRadius );
};

//=============================================================================
// namespace
//=============================================================================
#endif
#endif	// __inc_CClipTest_h__