///////////////////////////////////////////////////////////////////////////////
// xlSprite.cpp
// Collection: xlSprite : Sprite
//-----------------------------------------------------------------------------

//=============================================================================
// includes
//=============================================================================
#include "stdafx.h"
#ifdef	_XBOX
#include "xlSprite.h"
#include "xlFile.h"
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
static const	D3DVERTEXELEMENT9	g_xlSpriteVertexElements[] = {
	{ 0,	0,	D3DDECLTYPE_FLOAT4,		D3DDECLMETHOD_DEFAULT,	D3DDECLUSAGE_POSITION,	0 },
	{ 0,	16,	D3DDECLTYPE_FLOAT2,		D3DDECLMETHOD_DEFAULT,	D3DDECLUSAGE_TEXCOORD,	0 },
	D3DDECL_END()
};


//=============================================================================
// methods
//=============================================================================
//-----------------------------------------------------------------------------
// Name: Sprite()
// Desc: 
//-----------------------------------------------------------------------------
Sprite::Sprite() : PrimitiveBase()
{
}


//-----------------------------------------------------------------------------
// Name: ~Sprite()
// Desc: 
//-----------------------------------------------------------------------------
Sprite::~Sprite()
{
}


//-----------------------------------------------------------------------------
// Name: Sprite::Create
// Arguments:
// Returns:
//	Sprite*
//-----------------------------------------------------------------------------
Sprite*	Sprite::Create( LPDIRECT3DDEVICE	pd3dDevice,
					    UINT				uX,
						UINT				uY,
						UINT				uSizeX,
						UINT				uSizeY,
						float				fNearZ,
						DWORD				dwRepeatU,
						DWORD				dwRepeatV )
{
    Sprite*  pSprite;
    if( NULL == ( pSprite = new Sprite ) )
  		return	NULL;

    if( FAILED( pSprite->Construct( pd3dDevice, uX, uY, uSizeX, uSizeY, fNearZ, dwRepeatU, dwRepeatV ) ) ){
	    pSprite->Release();
	    return	NULL;
    }

    return pSprite;
}


//-----------------------------------------------------------------------------
// Name: Sprite::Construct
// Arguments:
// Returns:
//	HRESULT
//-----------------------------------------------------------------------------
HRESULT Sprite::Construct( LPDIRECT3DDEVICE pd3dDevice,
						   UINT				uX,
						   UINT				uY,
						   UINT				uSizeX,
						   UINT				uSizeY,
						   float			fNearZ,
						   DWORD			dwRepeatU,
						   DWORD			dwRepeatV )
{
    if( FAILED( PrimitiveBase::Construct( pd3dDevice ) ) )
        return  E_FAIL;

    // this
    m_uX = uX;
    m_uY = uY;
    m_uSizeX = uSizeX;
    m_uSizeY = uSizeY;
    m_fNearZ = fNearZ;
    m_dwRepeatU = dwRepeatU;
    m_dwRepeatV = dwRepeatV;

    // vertex buffer
    if( NULL == CreateVertexBuffer( 4, sizeof( SPRITE_VERTEX ), 0L, NULL ) )
        return	E_FAIL;

    SetVertexBuffer();

    // index buffer
    {
    } 

	// FXLite
	{
		VOID*	pCode	= NULL;
		DWORD	dwSize	= 0;
		if( FAILED( File::Load( XL_FX_FILE, &pCode, &dwSize ) ) )
			return	E_FAIL;

		if( NULL == CreateEffectFXL( g_xlSpriteVertexElements, pCode, dwSize ) ){
// [PREFIX:beginmodify] 2006/2/15 junyash PS#5189 reports memory leak 'pCode'
			File::Unload( pCode );
// [PREFIX:endmodify] junyash
			return	E_FAIL;
		}

		File::Unload( pCode );
	}

	m_hTqSprite		= m_pEffectFXL->GetTechniqueHandle( "Sprite" );
	m_hPassSprite	= m_pEffectFXL->GetPassHandleFromIndex( m_hTqSprite, 0 );
	m_hSpriteSampler	= m_pEffectFXL->GetParameterHandle( "g_tSpriteSampler" );

	return S_OK;
}


//-----------------------------------------------------------------------------
// Name: Sprite::Release
// Arguments:
// Returns:
//-----------------------------------------------------------------------------
void	Sprite::Release()
{
    PrimitiveBase::Release();
}


//-----------------------------------------------------------------------------
// Name: Sprite::SetVertexBuffer
// Arguments:
// Returns:
//	HRESULT
//-----------------------------------------------------------------------------
HRESULT Sprite::SetVertexBuffer()
{
    SPRITE_VERTEX	*pVertices;
    if( NULL == ( pVertices = new SPRITE_VERTEX [ m_dwNbVertices ] ) )
        return  E_FAIL;

    float   fX  = ( float )m_uX;
    float   fY  = ( float )m_uY;
    float   fHSizeX  = ( float )m_uSizeX/2.0f;
    float   fHSizeY  = ( float )m_uSizeY/2.0f;
    float   fRepeatU = ( float )m_dwRepeatU;
    float   fRepeatV = ( float )m_dwRepeatV;

	pVertices[ 0 ].vPT	= D3DXVECTOR4( fX - fHSizeX, fY - fHSizeY, m_fNearZ, 1.0f );
	pVertices[ 0 ].vT0  = D3DXVECTOR2( 0.0f, 0.0f );

    pVertices[ 1 ].vPT	= D3DXVECTOR4( fX + fHSizeX, fY - fHSizeY, m_fNearZ, 1.0f );
	pVertices[ 1 ].vT0	= D3DXVECTOR2( fRepeatU, 0.0f );

    pVertices[ 2 ].vPT	= D3DXVECTOR4( fX - fHSizeX, fY + fHSizeY, m_fNearZ, 1.0f );
	pVertices[ 2 ].vT0	= D3DXVECTOR2( 0.0f, fRepeatV );

    pVertices[ 3 ].vPT	= D3DXVECTOR4( fX + fHSizeX, fY + fHSizeY, m_fNearZ, 1.0f );
	pVertices[ 3 ].vT0	= D3DXVECTOR2( fRepeatU, fRepeatV );

    if( FAILED( PrimitiveBase::SetVertexBuffer( pVertices ) ) )
        return	E_FAIL;

    SAFE_DELETE_ARRAY( pVertices );

    return  S_OK;
}


//-----------------------------------------------------------------------------
// Name: Sprite::Render
// Arguments:
// Returns:
//	HRESULT
//-----------------------------------------------------------------------------
HRESULT	Sprite::Render()
{
	// ==== state ====
	m_pd3dDevice->SetRenderState( D3DRS_ZENABLE, TRUE );
	m_pd3dDevice->SetRenderState( D3DRS_VIEWPORTENABLE ,FALSE );
	m_pd3dDevice->SetVertexDeclaration( GetVertexDeclaration() ); 
	m_pd3dDevice->SetStreamSource( 0, GetVertexBuffer(), 0, GetVertexStride() );
	if( m_hSpriteSampler )
		m_pEffectFXL->SetSampler( m_hSpriteSampler, m_pTexture[ 0 ] );

	// ==== render ====
	m_pEffectFXL->BeginTechnique( m_hTqSprite, FXL_RESTORE_DEFAULT_STATE );
	m_pEffectFXL->BeginPass( m_hPassSprite );
	{
		m_pEffectFXL->Commit();
		m_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, 2 );
	}
	m_pEffectFXL->EndPass();
	m_pEffectFXL->EndTechnique();

	// ==== state ====
	m_pd3dDevice->SetRenderState( D3DRS_VIEWPORTENABLE ,TRUE );

	return	S_OK;
}


//=============================================================================
// namespace
//=============================================================================
} // end of namespace
#endif