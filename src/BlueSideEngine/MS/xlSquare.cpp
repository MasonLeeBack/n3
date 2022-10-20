///////////////////////////////////////////////////////////////////////////////
// xlSquare.cpp
// Collection: xlPrimitive
//-----------------------------------------------------------------------------

//=============================================================================
// includes
//=============================================================================
#include "stdafx.h"
#ifdef	_XBOX
#include "xlSquare.h"
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
static const	D3DVERTEXELEMENT9	g_xlSqareVertexElements[] = {
	{ 0,	0,	D3DDECLTYPE_FLOAT3,		D3DDECLMETHOD_DEFAULT,	D3DDECLUSAGE_POSITION,	0 },
	{ 0,	12,	D3DDECLTYPE_FLOAT3,		D3DDECLMETHOD_DEFAULT,	D3DDECLUSAGE_NORMAL,	0 },
	{ 0,	24,	D3DDECLTYPE_FLOAT2,		D3DDECLMETHOD_DEFAULT,	D3DDECLUSAGE_TEXCOORD,	0 },
	D3DDECL_END()
};

//=============================================================================
// methods
//=============================================================================
//-----------------------------------------------------------------------------
// Name: Square::Square
// Arguments:
// Returns:
//-----------------------------------------------------------------------------
Square::Square() : PrimitiveBase()
{
    m_fSize = 0.0f;
}


//-----------------------------------------------------------------------------
// Name: Square::~Square
// Arguments:
// Returns:
//-----------------------------------------------------------------------------
Square::~Square()
{
}


//-----------------------------------------------------------------------------
// Name: Square::Create
// Arguments:
//	pd3dDevice
//	fSize
//	dwRepeat
// Returns:
//	Square*
//-----------------------------------------------------------------------------
Square*	Square::Create( LPDIRECT3DDEVICE pd3dDevice, float fSize, DWORD dwRepeat )
{
    Square*  pSquare;
    if( NULL == ( pSquare = new Square ) )
  		return	NULL;

    if( FAILED( pSquare->Construct( pd3dDevice, fSize, dwRepeat ) ) ){
	    pSquare->Release();
	    return	NULL;
    }

    return pSquare;
}


//-----------------------------------------------------------------------------
// Name: Square::Construct
// Arguments:
//	pd3dDevice
//	fSize
//	dwRepeat
// Returns:
//	HRESULT
//-----------------------------------------------------------------------------
HRESULT Square::Construct( LPDIRECT3DDEVICE pd3dDevice, float fSize, DWORD dwRepeat )
{
    if( FAILED( PrimitiveBase::Construct( pd3dDevice ) ) )
        return  E_FAIL;

    // store
    m_fSize = fSize;

    // vertex buffer
    {
    	DWORD           dwNbVertices;
        SQUARE_VERTEX	*pVertices;

        dwNbVertices = 4;
        if( NULL == ( pVertices = new SQUARE_VERTEX [ dwNbVertices ] ) )
            return  E_FAIL;

	    pVertices[ 0 ].vP		= D3DXVECTOR3( -fSize/2.0f, 0.0f, fSize/2.0f );
	    pVertices[ 0 ].vN		= D3DXVECTOR3(  0.0f, 1.0f, 0.0f );
	    pVertices[ 0 ].vT0      = D3DXVECTOR2(  0.0f, 0.0f );

        pVertices[ 1 ].vP		= D3DXVECTOR3( fSize/2.0f, 0.0f, fSize/2.0f );
	    pVertices[ 1 ].vN		= D3DXVECTOR3( 0.0f, 1.0f, 0.0f );
	    pVertices[ 1 ].vT0      = D3DXVECTOR2( 1.0f * ( float )dwRepeat, 0.0f );

        pVertices[ 2 ].vP		= D3DXVECTOR3( -fSize/2.0f, 0.0f, -fSize/2.0f );
	    pVertices[ 2 ].vN		= D3DXVECTOR3(  0.0f, 1.0f, 0.0f );
	    pVertices[ 2 ].vT0	    = D3DXVECTOR2(  0.0f, 1.0f * ( float )dwRepeat );

        pVertices[ 3 ].vP		= D3DXVECTOR3( fSize/2.0f, 0.0f, -fSize/2.0f );
	    pVertices[ 3 ].vN		= D3DXVECTOR3( 0.0f, 1.0f, 0.0f );
	    pVertices[ 3 ].vT0  	= D3DXVECTOR2( 1.0f * ( float )dwRepeat, 1.0f * ( float )dwRepeat );

        if( NULL == CreateVertexBuffer( dwNbVertices, sizeof( SQUARE_VERTEX ), 0L, pVertices ) )
            return	E_FAIL;

        SAFE_DELETE_ARRAY( pVertices );
    }
    
    // index
    {
    } 

	{
		VOID*	pCode	= NULL;
		DWORD	dwSize	= 0;
		if( FAILED( File::Load( XL_FX_FILE, &pCode, &dwSize ) ) )
			return	E_FAIL;

		if( NULL == CreateEffectFXL( g_xlSqareVertexElements, pCode, dwSize ) ){
// [PREFIX:beginmodify] 2006/2/15 junyash PS#5190 reports memory leak 'pCode'
			File::Unload( pCode );
// [PREFIX:endmodify] junyash
			return	E_FAIL;
		}

		File::Unload( pCode );

		// Retrieve technique desc
		/*
		FXLEFFECT_DESC desc;
		m_pEffectFXL->GetEffectDesc( &desc );
		FXLTECHNIQUE_DESC techdesc;
		m_pEffectFXL->GetTechniqueDesc( m_pEffectFXL->GetTechniqueHandleFromIndex( 0 ), &techdesc );
		*/
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
// Name: Square::Release
// Arguments:
// Returns:
//	void
//-----------------------------------------------------------------------------
void	Square::Release()
{
	PrimitiveBase::Release();
}


//-----------------------------------------------------------------------------
// Name: Square::Render
// Arguments:
// Returns:
//	HRESULT
//-----------------------------------------------------------------------------
HRESULT	Square::Render( const XMMATRIX& mWVP )
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

	return	S_OK;
}


//-----------------------------------------------------------------------------
// Name: Square::Render
// Arguments:
// Returns:
//	HRESULT
//-----------------------------------------------------------------------------
HRESULT	Square::Render( const XMMATRIX& mWVP, D3DXCOLOR d3dxColor )
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

	return	S_OK;
}


//=============================================================================
// namespace
//=============================================================================
} // end of namespace
#endif