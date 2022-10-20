///////////////////////////////////////////////////////////////////////////////
// xlRectangle.cpp
// Collection: xlPrimitive
//-----------------------------------------------------------------------------

//=============================================================================
// includes
//=============================================================================
#include "stdafx.h"
#ifdef	_XBOX
#include "xlRectangle.h"
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
static const	D3DVERTEXELEMENT9	g_xlRectangleVertexElements[] = {
	{ 0,	0,	D3DDECLTYPE_FLOAT3,		D3DDECLMETHOD_DEFAULT,	D3DDECLUSAGE_POSITION,	0 },
	{ 0,	12,	D3DDECLTYPE_FLOAT3,		D3DDECLMETHOD_DEFAULT,	D3DDECLUSAGE_NORMAL,	0 },
	{ 0,	24,	D3DDECLTYPE_FLOAT2,		D3DDECLMETHOD_DEFAULT,	D3DDECLUSAGE_TEXCOORD,	0 },
	D3DDECL_END()
};
	
//=============================================================================
// methods
//=============================================================================

//-----------------------------------------------------------------------------
// Name: Rectangle::Rectangle
// Arguments:
// Returns:
//-----------------------------------------------------------------------------  
Rectangle::Rectangle() : PrimitiveBase()
{
}


//-----------------------------------------------------------------------------
// Name: Rectangle::~Rectangle
// Arguments:
// Returns:
//-----------------------------------------------------------------------------  
Rectangle::~Rectangle()
{
}


//-----------------------------------------------------------------------------
// Name: Rectangle::Create
// Arguments:
//  pd3dDevice  device
//  fSizeX      size of x
//  fSizeZ      size of z
//  dwRepeatU   texture repeat u
//  dwRepeatV   texture repeat v
// Returns:
//	Rectangle*
//-----------------------------------------------------------------------------
Rectangle*	Rectangle::Create( LPDIRECT3DDEVICE pd3dDevice, float fSizeX, float fSizeZ, DWORD dwRepeatU, DWORD dwRepeatV )
{
    Rectangle*  pRectangle;
    if( NULL == ( pRectangle = new Rectangle ) )
  		return	NULL;

    if( FAILED( pRectangle->Construct( pd3dDevice, fSizeX, fSizeZ, dwRepeatU, dwRepeatV ) ) ){
	    pRectangle->Release();
	    return	NULL;
    }

    return pRectangle;
}

//-----------------------------------------------------------------------------
// Name: Rectangle::Construct
// Arguments:
//  pd3dDevice  device
//  fSizeX      size of x
//  fSizeZ      size of z
//  dwRepeatU   texture repeat u
//  dwRepeatV   texture repeat v
// Returns:
//	HRESULT
//-----------------------------------------------------------------------------
HRESULT Rectangle::Construct( LPDIRECT3DDEVICE pd3dDevice, float fSizeX, float fSizeZ, DWORD dwRepeatU, DWORD dwRepeatV )
{
    if( FAILED( PrimitiveBase::Construct( pd3dDevice ) ) )
        return  E_FAIL;

    // vertex buffer
    {
    	DWORD               dwNbVertices;
        RECTANGLE_VERTEX	*pVertices;

        dwNbVertices = 4;
        if( NULL == ( pVertices = new RECTANGLE_VERTEX [ dwNbVertices ] ) )
            return  E_FAIL;

	    pVertices[ 0 ].vP	= D3DXVECTOR3( -fSizeX/2.0f, 0.0f, fSizeZ/2.0f );
	    pVertices[ 0 ].vN	= D3DXVECTOR3(  0.0f, 1.0f, 0.0f );
	    pVertices[ 0 ].vT0	= D3DXVECTOR2(  0.0f, 0.0f );

	    pVertices[ 1 ].vP	= D3DXVECTOR3(  fSizeX/2.0f, 0.0f, fSizeZ/2.0f );
	    pVertices[ 1 ].vN	= D3DXVECTOR3(  0.0f, 1.0f, 0.0f );
	    pVertices[ 1 ].vT0	= D3DXVECTOR2(  1.0f * ( float )dwRepeatU, 0.0f );

	    pVertices[ 2 ].vP	= D3DXVECTOR3( -fSizeX/2.0f, 0.0f, -fSizeZ/2.0f );
	    pVertices[ 2 ].vN	= D3DXVECTOR3(  0.0f, 1.0f, 0.0f );
	    pVertices[ 2 ].vT0	= D3DXVECTOR2(  0.0f, 1.0f * ( float )dwRepeatV );

	    pVertices[ 3 ].vP	= D3DXVECTOR3(  fSizeX/2.0f, 0.0f, -fSizeZ/2.0f );
	    pVertices[ 3 ].vN	= D3DXVECTOR3(  0.0f, 1.0f, 0.0f );
	    pVertices[ 3 ].vT0	= D3DXVECTOR2(  1.0f * ( float )dwRepeatU, 1.0f * ( float )dwRepeatV );

        if( NULL == CreateVertexBuffer( dwNbVertices, sizeof( RECTANGLE_VERTEX ), 0L, pVertices ) )
            return	E_FAIL;

        SAFE_DELETE_ARRAY( pVertices );
    }
    
    // index buffer for line
    {
        DWORD       dwNbIndices = 5;
        WORD	    pIndices[] = { 0, 1, 3, 2, 0, };

        if( NULL == CreateLineIndexBuffer( dwNbIndices, sizeof( WORD ), D3DFMT_INDEX16, pIndices ) )
            return	E_FAIL;
    }

	// shader
	{
		VOID*	pCode	= NULL;
		DWORD	dwSize	= 0;
		if( FAILED( File::Load( XL_FX_FILE, &pCode, &dwSize ) ) )
			return	E_FAIL;

		if( NULL == CreateEffectFXL( g_xlRectangleVertexElements, pCode, dwSize ) ){
// [PREFIX:beginmodify] 2006/2/15 junyash PS#5187 reports memory leak 'pCode'
			File::Unload( pCode );
// [PREFIX:endmodify] junyash
			return	E_FAIL;
		}

		File::Unload( pCode );
	}

	m_hTqTexture	= m_pEffectFXL->GetTechniqueHandle( "Texture" );
	m_hTqColor		= m_pEffectFXL->GetTechniqueHandle( "Color" );
	m_hPassTexture	= m_pEffectFXL->GetPassHandleFromIndex( m_hTqTexture, 0 );
	m_hPassColor	= m_pEffectFXL->GetPassHandleFromIndex( m_hTqColor, 0 );
	m_hWVP			= m_pEffectFXL->GetParameterHandle( "g_mWVP" );
	m_hColor		= m_pEffectFXL->GetParameterHandle( "g_vColor" );
    m_hColorSampler = m_pEffectFXL->GetParameterHandle( "g_tColorSampler" );

	return S_OK;
}


//-----------------------------------------------------------------------------
// Name: Rectangle::Release()
// Arguments:
// Returns:
//-----------------------------------------------------------------------------
void	Rectangle::Release()
{
    PrimitiveBase::Release();
}


//-----------------------------------------------------------------------------
// Name: Rectangle::Render()
// Arguments:
// Returns:
//	HRESULT
//-----------------------------------------------------------------------------
HRESULT	Rectangle::Render( const XMMATRIX& mWVP )
{
	// ==== state ====
	m_pd3dDevice->SetRenderState( D3DRS_ZENABLE, TRUE );
	m_pd3dDevice->SetVertexDeclaration( GetVertexDeclaration() ); 
	m_pd3dDevice->SetStreamSource( 0, GetVertexBuffer(), 0, GetVertexStride() );

	if( m_hColorSampler )
		m_pEffectFXL->SetSampler( m_hColorSampler, m_pTexture[ 0 ] );
	m_pEffectFXL->SetMatrixF4x4( m_hWVP, ( FLOAT* )&mWVP );

	// ==== render ====
	m_pEffectFXL->BeginTechnique( m_hTqTexture, FXL_RESTORE_DEFAULT_STATE );
	m_pEffectFXL->BeginPass( m_hPassTexture );
	{
		m_pEffectFXL->Commit();
		m_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, 2 );
	}
	m_pEffectFXL->EndPass();
	m_pEffectFXL->EndTechnique();

	// ==== state ====

	return S_OK;
}


//-----------------------------------------------------------------------------
// Name: Rectangle::Render( D3DXCOLOR d3dColor )
// Arguments:
//  d3dColor    color
// Returns:
//	HRESULT
//-----------------------------------------------------------------------------
HRESULT	Rectangle::Render( const XMMATRIX& mWVP, D3DXCOLOR d3dxColor )
{
	// ==== state ====
	m_pd3dDevice->SetRenderState( D3DRS_ZENABLE, TRUE );
	m_pd3dDevice->SetVertexDeclaration( GetVertexDeclaration() ); 
	m_pd3dDevice->SetStreamSource( 0, GetVertexBuffer(), 0, GetVertexStride() );

	m_pEffectFXL->SetMatrixF4x4( m_hWVP, ( FLOAT* )&mWVP );
	m_pEffectFXL->SetVectorF( m_hColor, ( FLOAT* )&d3dxColor );

	// ==== render ====
	m_pEffectFXL->BeginTechnique( m_hTqColor, FXL_RESTORE_DEFAULT_STATE );
	m_pEffectFXL->BeginPass( m_hPassColor );
	{
		m_pEffectFXL->Commit();
		m_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, 2 );
	}
	m_pEffectFXL->EndPass();
	m_pEffectFXL->EndTechnique();

	// ==== state ====

	return S_OK;
}


//-----------------------------------------------------------------------------
// Name: Rectangle::RenderLine( D3DXCOLOR d3dColor )
// Arguments:
//  d3dColor    color
// Returns:
//	HRESULT
//-----------------------------------------------------------------------------
HRESULT	Rectangle::RenderLine( const XMMATRIX& mWVP, D3DXCOLOR d3dxColor )
{
	// ==== state ====
	m_pd3dDevice->SetRenderState( D3DRS_ZENABLE, TRUE );
	m_pd3dDevice->SetVertexDeclaration( GetVertexDeclaration() );
	m_pd3dDevice->SetStreamSource( 0, GetVertexBuffer(), 0, GetVertexStride() );
	m_pd3dDevice->SetIndices( GetLineIndexBuffer() );

	m_pEffectFXL->SetMatrixF4x4( m_hWVP, ( FLOAT* )&mWVP );
	m_pEffectFXL->SetVectorF( m_hColor, ( FLOAT* )&d3dxColor );

	// ==== render ====
	m_pEffectFXL->BeginTechnique( m_hTqColor, FXL_RESTORE_DEFAULT_STATE );
	m_pEffectFXL->BeginPass( m_hPassColor );
	{
		m_pEffectFXL->Commit();
		m_pd3dDevice->DrawIndexedPrimitive( D3DPT_LINESTRIP, 0, 0, m_dwNbVertices, 0, 4 );
	}
	m_pEffectFXL->EndPass();
	m_pEffectFXL->EndTechnique();

	// ==== state ====

	return S_OK;
}


//=============================================================================
// namespace
//=============================================================================
} // end of namespace
#endif