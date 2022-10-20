///////////////////////////////////////////////////////////////////////////////
// xlCamera.h
// Collection: xlCamera
//-----------------------------------------------------------------------------
#ifndef __inc_xlCamera_h__
#define __inc_xlCamera_h__

//=============================================================================
// includes
//=============================================================================
#include "xlCommon.h"

//=============================================================================
// macros
//=============================================================================
enum {
    XL_CAMERA_CLIPPLANE_LEFT = 0,
    XL_CAMERA_CLIPPLANE_RIGHT,
    XL_CAMERA_CLIPPLANE_BOTTOM,
    XL_CAMERA_CLIPPLANE_UPPER,
    XL_CAMERA_CLIPPLANE_NEAR,
    XL_CAMERA_CLIPPLANE_FAR,
    XL_CAMERA_CLIPPLANE_MAX,
    XL_CAMERA_CLIPPLANE_FORCE_DWORD =	0x7fffffff, /* force 32-bit size enum */
};

//=============================================================================
// namespace
//=============================================================================
namespace xl {
//=============================================================================
// structs
//=============================================================================
//=============================================================================
// classes
//=============================================================================
class Camera {
	private :
        LPDIRECT3DDEVICE    m_pd3dDevice;
        D3DVIEWPORT         m_d3dViewport;
        XMMATRIX			m_mView, m_mViewInverse;
        XMMATRIX			m_mProj;
        XMVECTOR			m_vFrom;
        XMVECTOR			m_vAt;
        XMVECTOR			m_vUp;

        float               m_fPers;
        float               m_fAspect;

        float               m_fLeft;
        float               m_fRight;
        float               m_fBottom;
        float               m_fTop;

        float               m_fNear;
        float               m_fFar;
        bool                m_bHalt;

		// mouse
        bool				m_bTrans;
        bool				m_bRot;
        float				m_fLastMouseX;
        float				m_fLastMouseY;
        XMVECTOR			m_vWorldPos;
        XMVECTOR			m_vWorldVec;

        HRESULT             Construct( LPDIRECT3DDEVICE pd3dDevice );

	public :
        Camera();
        ~Camera();
        static Camera*      Create( LPDIRECT3DDEVICE pd3dDevice );
        void                Release();

        void                Default();
		HRESULT 			Update( float fFromX,	float fFromY,	float fFromZ,
								    float fAtX,	    float fAtY,		float fAtZ,
								    float fUpX,	    float fUpY,		float fUpZ,
								    float fPers,	float fAspect,	float fNear,   float fFar );        // perspective
        HRESULT 			Update( float fFromX,	float fFromY,	float fFromZ,
								    float fAtX,	    float fAtY,		float fAtZ,
								    float fUpX,	    float fUpY,		float fUpZ,
								    float fLeft,	float fRight,   float fBottom,  float fTop,
                                    float fNear,    float fFar );                                        // ortho

        HRESULT 			Update( float fdFromX,  float fdFromY,
                                    float fdAtX,    float fdAtY,
                                    bool  bPress,
                                    float fTransScale, float fRotScale,
                                    float fElapsedTime );                                               // pad

		XMMATRIX			GetViewMatrix();
		XMMATRIX			GetProjectionMatrix();
		HRESULT             GetClipPlane( XMVECTOR pPlane[] );

		inline XMVECTOR		GetFrom()							{ return	m_vFrom; }
		inline XMVECTOR		GetAt()								{ return	m_vAt; }
		inline XMVECTOR		GetUp()								{ return	m_vUp; }

		inline float		GetPers()                           { return	m_fPers; }
		inline float		GetAspect()                         { return	m_fAspect; }

		inline float		GetLeft()                           { return	m_fLeft; }
		inline float		GetRight()                          { return	m_fRight; }
		inline float		GetBottom()                         { return	m_fBottom; }
		inline float		GetTop()                            { return	m_fTop; }

		inline float		GetNear()                           { return	m_fNear; }
		inline float		GetFar()                            { return	m_fFar; }

		inline void			Halt( bool bHalt )                  { m_bHalt = bHalt; }
};

//=============================================================================
// prototypes
//=============================================================================
//=============================================================================
// variables
//=============================================================================
//=============================================================================
// inline methods
//=============================================================================
//=============================================================================
// namespace
//=============================================================================
} // end of namespace xl
#endif	// __inc_xlCamera_h__

