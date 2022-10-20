///////////////////////////////////////////////////////////////////////////////
// CBoundingBoxManager.h
// Collection: xlPrimitive 
//-----------------------------------------------------------------------------
#ifndef __inc_CBoundingBoxManager_h__
#define __inc_CBoundingBoxManager_h__

//=============================================================================
// includes
//=============================================================================
#include "CBoundingBox.h"

//=============================================================================
// namespace
//=============================================================================
//=============================================================================
// macros
//=============================================================================
#define	CBOUNDINGBOX_NB_MAX		5000

//=============================================================================
// structs
//=============================================================================
//=============================================================================
// classes
//=============================================================================
class CBoundingBoxManager {
	protected:
		LPDIRECT3DDEVICE			m_pd3dDevice;
		UINT						m_uNbCBoundingBox;
		CBoundingBox*				m_pCBoundingBox[ CBOUNDINGBOX_NB_MAX ];

		HRESULT						Construct( LPDIRECT3DDEVICE	pd3dDevice );

	public:
		CBoundingBoxManager();
		~CBoundingBoxManager();

		static CBoundingBoxManager*	Create( LPDIRECT3DDEVICE	pd3dDevice );

		CBoundingBox*				CreateBoundingBox(	const XMFLOAT3&		vCenter,
														const FLOAT			fRotateY,
														const FLOAT			fWidth,
														const FLOAT			fHeight,
														const FLOAT			fDepth );
		void						Release();

		HRESULT						Update();
		HRESULT						Render( const XMMATRIX&	mVP,
											const D3DXCOLOR	d3dxColorIn,
											const D3DXCOLOR	d3dxColorOut );

		UINT						GetNbBoundingBox()			{ return	m_uNbCBoundingBox; }
		CBoundingBox*				GetBoundingBox( UINT uN )	{ return	m_pCBoundingBox[ uN ]; }
};

//=============================================================================
// namespace
//=============================================================================
#endif	// __inc_CBoundingBoxManager_h__