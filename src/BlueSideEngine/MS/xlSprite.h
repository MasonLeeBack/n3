///////////////////////////////////////////////////////////////////////////////
// xlSprite.h
// Collection: xlPrimitive 
//-----------------------------------------------------------------------------
#ifndef __inc_xlSprite_h__
#define __inc_xlSprite_h__

//=============================================================================
// includes
//=============================================================================
#include "xlPrimitiveBase.h"
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
struct SPRITE_VERTEX
{
	D3DXVECTOR4 vPT;
	D3DXVECTOR2 vT0;
};

//=============================================================================
// classes
//=============================================================================
class Sprite : public PrimitiveBase {
	protected:
		UINT            m_uX;
		UINT            m_uY;
		UINT            m_uSizeX;
		UINT            m_uSizeY;
		DWORD           m_dwRepeatU;
		DWORD           m_dwRepeatV;
		float           m_fNearZ;

		FXLHANDLE		m_hTqSprite;
		FXLHANDLE		m_hPassSprite;
		FXLHANDLE		m_hSpriteSampler;

		HRESULT         Construct( LPDIRECT3DDEVICE pd3dDevice,
									UINT				uX,
									UINT				uY,
									UINT				uSizeX,
									UINT				uSizeY,
									float			fNearZ,
									DWORD			dwRepeatU,
									DWORD			dwRepeatV );
		HRESULT         SetVertexBuffer();

	public:
		Sprite();
		~Sprite();

		static Sprite*  Create( LPDIRECT3DDEVICE	pd3dDevice,
								UINT				uX,
								UINT				uY,
								UINT				uSizeX,
								UINT				uSizeY,
								float				fNearZ,
								DWORD				dwRepeatU,
								DWORD				dwRepeatV );
		void			Release();

		HRESULT			Render();

		HRESULT         GetTranslation( UINT* uX, UINT* uY )        { *uX = m_uX; *uY = m_uY; }
		HRESULT         GetSize( UINT* uSizeX, UINT* uSizeY )       { *uSizeX = m_uSizeX; *uSizeY = m_uSizeY; }
};

//=============================================================================
// namespace
//=============================================================================
} // end of namespace
#endif	// __inc_xlSprite_h__

