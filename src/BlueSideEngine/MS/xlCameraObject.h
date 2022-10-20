///////////////////////////////////////////////////////////////////////////////
// xlCameraObject.h
// Collection: xlCamera
//-----------------------------------------------------------------------------
#ifndef __inc_xlCameraObject_h__
#define __inc_xlCameraObject_h__

//=============================================================================
// includes
//=============================================================================
#include "xlCommon.h"
#include "xlCamera.h"
#include "xlFrustum.h"
#include "xlRectangle.h"
#include "xlLine.h"

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
class CameraObject {
	protected:
        LPDIRECT3DDEVICE		m_pd3dDevice;
		Camera*					m_pCamera;
        float                   m_fCameraSize;

        XMMATRIX				m_mInvV;
        float					m_fPers;
        float					m_fAspect;
        float					m_fNear;
        float				    m_fFar;

		// geometry
        Frustum*				m_pFrustum;
		Rectangle*				m_pPlane[ 2 ];
        Line*                   m_pLine;
		XMMATRIX				m_mM[ 3 ];

        HRESULT					Construct( LPDIRECT3DDEVICE pd3dDevice, float fCameraSize );

    public:
		CameraObject();
		~CameraObject();

		static CameraObject*	Create( LPDIRECT3DDEVICE pd3dDevice, float fCameraSize );
		void					Release();

		HRESULT					Update( FLOAT		fPers,
										FLOAT		fAspect,
										FLOAT		fNear,
										FLOAT		fFar,
										XMMATRIX&	mView );

		HRESULT					Render( const XMMATRIX&	mVP,
										const D3DXCOLOR d3dColor );
};

//=============================================================================
// namespace
//=============================================================================
} // end of namespace
#endif	// __inc_CameraObject_h__
