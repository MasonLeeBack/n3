#include "stdafx.h"
#include "BsKernel.h"
#include "MS/BsClipTest.h"
#ifdef _XBOX
#include "MS/CClipTest.h"
#include "MS/CBoundingBoxManager.h"

class CBsClipTest: public IBsClipTest
{
protected:
	bool					m_bReady;
	bool					m_bClipUpdate;

	xl::CameraObject*		m_pCameraObject;
	CBoundingBoxManager*	m_pCBoundingBoxManager;

public:
	bool IsReady()
	{
		return	m_bReady;
	}

	CBsClipTest()
	{
		m_bReady		= false;
		m_bClipUpdate	= false;
	}	

	HRESULT Initialize()
	{
		// camera object
		if( NULL == ( m_pCameraObject = xl::CameraObject::Create( g_BsKernel.GetD3DDevice(), 100.0f ) ) )
			return	E_FAIL;

		// bounding box
		if( NULL == ( m_pCBoundingBoxManager = CBoundingBoxManager::Create( g_BsKernel.GetD3DDevice() ) ) )
			return	E_FAIL;

		// ready
		m_bReady = true;

		return S_OK;
	}

	void Destroy()
	{
		if( m_pCameraObject ){
			SAFE_RELEASE( m_pCameraObject );
			m_pCameraObject = NULL;
		}

		if( m_pCBoundingBoxManager ){
			SAFE_RELEASE( m_pCBoundingBoxManager );
			m_pCBoundingBoxManager	= NULL;
		}
	}

	void Update()
	{
		if( !m_bReady )
			return;

		if( m_bClipUpdate ){
/*
			int			iCameraHandle;
			CBsCamera*	pCBsCamera;
			float		fPers;
			float		fAspect;	
			float		fNear;
			float		fFar;
			XMMATRIX	mV;

			// camera
			iCameraHandle	= g_BsKernel.GetCameraHandle( 0 );
			pCBsCamera		= g_BsKernel.GetCamera( iCameraHandle );
			fPers			= pCBsCamera->GetFOVByProcess()*1.0f;
			fAspect			= pCBsCamera->GetAspect();
			fNear			= pCBsCamera->GetNearZ();
			fFar			= pCBsCamera->GetFarZ();
			mV				= _XMMATRIX( ( FLOAT* )pCBsCamera->GetViewMatrix() );

			// clip test
			CClipTest::ComputeClipPlane( fPers,
										 fAspect,
										 fNear,
										 fFar,
										 fFar,
										 fFar,
										 mV );
*/
			for( UINT i = 0; i < m_pCBoundingBoxManager->GetNbBoundingBox(); i++ ){
				CBoundingBox*	pCBoundingBox;
				pCBoundingBox	= m_pCBoundingBoxManager->GetBoundingBox( i );

		//		XMFLOAT3	vCenter;
		//		FLOAT		fRadius;
		//		pCBoundingBox->GetResource( vCenter, fRadius );
		//		pCBoundingBox->SetClip( CClipTest::Test( vCenter, fRadius ) );
			}
		}
	}

	void Render()
	{
		if( !m_bReady )
			return;

		int			iCameraHandle;
		CBsCamera*	pCBsCamera;
		float		fPers;
		float		fAspect;	
		float		fNear;
		float		fFar;
		XMMATRIX	mV, mVP;

		iCameraHandle	= g_BsKernel.GetCameraHandle( 0 );
		pCBsCamera		= g_BsKernel.GetCamera( iCameraHandle );
		fPers			= XMConvertToDegrees( pCBsCamera->GetFOVByProcess()*2.0f );
		fAspect			= pCBsCamera->GetAspect();
		fNear			= pCBsCamera->GetNearZ();
		fFar			= pCBsCamera->GetFarZ();
		mV				= _XMMATRIX( ( FLOAT* )pCBsCamera->GetViewMatrix() );
		mVP				= _XMMATRIX( ( FLOAT* )pCBsCamera->GetViewProjMatrix() );

		// camera Object
		if( m_bClipUpdate ){
			m_pCameraObject->Update( fPers,
									fAspect,
									fNear,
									fFar,
									mV );
		}
		m_pCameraObject->Render( mVP, 0xffffffff );
		m_pCBoundingBoxManager->Render( mVP, 0xffff0000, 0xffffffff );
	}

	void AddBoundingBox( const FLOAT		fCx,
						 const FLOAT		fCy,
						 const FLOAT		fCz,
						 const FLOAT		fRotateY,
						 const FLOAT		fX,
						 const FLOAT		fY,
						 const FLOAT		fZ )
	{
		if( !m_bReady )
			return;

		// create
		CBoundingBox*	pCBoundingBox;
		if( NULL == ( pCBoundingBox	= m_pCBoundingBoxManager->CreateBoundingBox( XMFLOAT3( fCx, fCy, fCz ), fRotateY, fX, fY, fZ ) ) )
			return;
	}

	void SetClipUpdate( bool	bUpdate )
	{
		if( !m_bReady )
			return;

		m_bClipUpdate	= bUpdate;
	}

	bool GetClipUpdate()
	{
		return	m_bClipUpdate;
	}
};

static CBsClipTest	s_ClipTest;
#else

class CBsNullClipTest : public IBsClipTest
{
public:
	HRESULT Initialize()		{ return	S_FALSE; }
	void	Destroy()			{}
	void	Update()			{}
	void	Render()			{}
	bool	IsReady()			{ return	false; }
	void	AddBoundingBox( const FLOAT		fCx,
							const FLOAT		fCy,
							const FLOAT		fCz,
							const FLOAT		fRotateY,
							const FLOAT		fX,
							const FLOAT		fY,
							const FLOAT		fZ )	{}
	void SetClipUpdate( bool bUpdate )	{}
	bool GetClipUpdate()				{ return	false; }

};

static CBsNullClipTest s_NullClipTest;
#endif

IBsClipTest& IBsClipTest::GetInstance()
{
#ifdef _XBOX
	return	s_ClipTest;
#else
	return	s_NullClipTest;
#endif
}
