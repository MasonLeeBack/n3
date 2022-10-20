///////////////////////////////////////////////////////////////////////////////
// CBoundingBox.h
// Collection: xlPrimitive 
//-----------------------------------------------------------------------------
#ifndef __inc_CBoundingBox_h__
#define __inc_CBoundingBox_h__

//=============================================================================
// includes
//=============================================================================
#include "xl.h"
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
class CBoundingBox {
	protected:
		XMFLOAT3				m_vCenter;
		FLOAT					m_fRotateY;
		FLOAT					m_fRadius;
		XMMATRIX				m_mW;
		xl::Hexahedron*			m_pBox;
		BOOL					m_bClip;

		HRESULT					Construct( LPDIRECT3DDEVICE	pd3dDevice,
										   const XMFLOAT3&	vCenter,
										   const FLOAT		fRotateY,
										   const FLOAT		fX,
										   const FLOAT		fY,
										   const FLOAT		fZ );
// [PREFIX:beginmodify] 2006/2/15 junyash PS#5179 reports CBoundingBox::Create() momory leaking 'pCBoundingBox'. modify constructor/destructor to stable.
	//public:
	private:
// [PREFIX:endmodify] junyash
		CBoundingBox();
		~CBoundingBox();

// [PREFIX:beginmodify] 2006/2/15 junyash PS#5179 reports CBoundingBox::Create() momory leaking 'pCBoundingBox'. modify constructor/destructor to stable.
	public:
// [PREFIX:endmodify] junyash
		static CBoundingBox*	Create( LPDIRECT3DDEVICE	pd3dDevice,
										const XMFLOAT3&		vCenter,
										const FLOAT			fRotateY,
										const FLOAT			fX,
										const FLOAT			fY,
										const FLOAT			fZ );
		void					Release();

		HRESULT					Render( const XMMATRIX& mVP,
										const D3DXCOLOR d3dxColor );
		HRESULT					RenderClip( const XMMATRIX& mVP,
											const D3DXCOLOR d3dxColorIn,
											const D3DXCOLOR d3dxColorOut );

		HRESULT					GetResource( XMFLOAT3&	vCenter,
											 FLOAT&		fRadius );
		BOOL					GetClip()				{ return	m_bClip; }
		void					SetClip( BOOL bClip )	{ m_bClip	= bClip; }
};

//=============================================================================
// namespace
//=============================================================================
#endif	// __inc_CBoundingBox_h__
