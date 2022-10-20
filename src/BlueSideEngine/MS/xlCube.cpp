///////////////////////////////////////////////////////////////////////////////
// xlCube.cpp
// Collection: xlCube : Cube
//-----------------------------------------------------------------------------

//=============================================================================
// includes
//=============================================================================
#include "stdafx.h"
#ifdef	_XBOX
#include "xlCube.h"
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
static const	D3DVERTEXELEMENT9	g_xlCubeVertexElements[] = {
	{ 0,	0,	D3DDECLTYPE_FLOAT3,		D3DDECLMETHOD_DEFAULT,	D3DDECLUSAGE_POSITION,	0 },
	{ 0,	12,	D3DDECLTYPE_FLOAT3,		D3DDECLMETHOD_DEFAULT,	D3DDECLUSAGE_NORMAL,	0 },
	{ 0,	24,	D3DDECLTYPE_FLOAT2,		D3DDECLMETHOD_DEFAULT,	D3DDECLUSAGE_TEXCOORD,	0 },
	D3DDECL_END()
};
	
//=============================================================================
// methods
//=============================================================================
//-----------------------------------------------------------------------------
// Name: Cube::Cube
// Arguments:
// Returns:
//-----------------------------------------------------------------------------
Cube::Cube() : PrimitiveBase()
{
    m_fSize = 0.0f;
}


//-----------------------------------------------------------------------------
// Name: Square::~Square
// Arguments:
// Returns:
//-----------------------------------------------------------------------------
Cube::~Cube()
{
}


//-----------------------------------------------------------------------------
// Name: Cube::Create
// Arguments:
//	pd3dDevice
//	fSize
//	dwRepeat
// Returns:
//	Square*
//-----------------------------------------------------------------------------
Cube*	Cube::Create( LPDIRECT3DDEVICE pd3dDevice, float fSize, DWORD dwRepeat )
{
    Cube*  pCube;
    if( NULL == ( pCube = new Cube ) )
  		return	NULL;

    if( FAILED( pCube->Construct( pd3dDevice, fSize, dwRepeat ) ) ){
	    pCube->Release();
	    return	NULL;
    }

    return pCube;
}


//-----------------------------------------------------------------------------
// Name: Cube::Construct
// Arguments:
//	pd3dDevice
//	fSize
//	dwRepeat
// Returns:
//	HRESULT
//-----------------------------------------------------------------------------
#define CUBE_SQRT3   1.7320508f

HRESULT Cube::Construct( LPDIRECT3DDEVICE pd3dDevice, float fSize, DWORD dwRepeat )
{
    if( FAILED( PrimitiveBase::Construct( pd3dDevice ) ) )
        return  E_FAIL;

    // store
    m_fSize = fSize;

    // vertex buffer
    {
    	DWORD       dwNbVertices;
        CUBE_VERTEX *pVertices;

        dwNbVertices = 8;
        if( NULL == ( pVertices = new CUBE_VERTEX [ dwNbVertices ] ) )
            return  E_FAIL;

        float   fP = fSize/2.0f;
        float   fN = 1.0f/CUBE_SQRT3;

	    pVertices[ 0 ].vP	= D3DXVECTOR3( -fP,  fP,  fP );
	    pVertices[ 0 ].vN	= D3DXVECTOR3( -fN,  fN, -fN );
	    pVertices[ 0 ].vT0	= D3DXVECTOR2(  0.0f,0.0f );
        pVertices[ 1 ].vP	= D3DXVECTOR3(  fP,  fP,  fP );
        pVertices[ 1 ].vN	= D3DXVECTOR3(  fN,  fN,  fN );
	    pVertices[ 1 ].vT0	= D3DXVECTOR2(  1.0f*( float )dwRepeat, 0.0f );
        pVertices[ 2 ].vP	= D3DXVECTOR3( -fP,  fP, -fP );
	    pVertices[ 2 ].vN	= D3DXVECTOR3( -fN,  fN, -fN );
	    pVertices[ 2 ].vT0	= D3DXVECTOR2(  0.0f,1.0f*( float )dwRepeat );
	    pVertices[ 3 ].vP	= D3DXVECTOR3(  fP,  fP, -fP );
	    pVertices[ 3 ].vN	= D3DXVECTOR3(  fN,  fN, -fN );
	    pVertices[ 3 ].vT0	= D3DXVECTOR2(  1.0f*( float )dwRepeat, 1.0f*( float )dwRepeat );

        pVertices[ 4 ].vP	= D3DXVECTOR3( -fP, -fP, -fP );
	    pVertices[ 4 ].vN	= D3DXVECTOR3( -fN, -fN, -fN );
        pVertices[ 4 ].vT0	= D3DXVECTOR2(  0.0f,0.0f );
        pVertices[ 5 ].vP	= D3DXVECTOR3(  fP, -fP, -fP );
	    pVertices[ 5 ].vN	= D3DXVECTOR3(  fN, -fN, -fN );
	    pVertices[ 5 ].vT0	= D3DXVECTOR2(  1.0f*( float )dwRepeat, 0.0f );
        pVertices[ 6 ].vP	= D3DXVECTOR3( -fP, -fP,  fP );
	    pVertices[ 6 ].vN	= D3DXVECTOR3( -fN, -fN, -fN );      
        pVertices[ 6 ].vT0	= D3DXVECTOR2(  0.0f,1.0f*( float )dwRepeat );
        pVertices[ 7 ].vP	= D3DXVECTOR3(  fP, -fP,  fP );
        pVertices[ 7 ].vN	= D3DXVECTOR3(  fN, -fN,  fN );
	    pVertices[ 7 ].vT0	= D3DXVECTOR2(  1.0f*( float )dwRepeat, 1.0f*( float )dwRepeat );

        if( NULL == CreateVertexBuffer( dwNbVertices, sizeof( CUBE_VERTEX ), 0L, pVertices ) )
            return	E_FAIL;

        SAFE_DELETE_ARRAY( pVertices );
    }
    
    // index
    {
        DWORD       dwNbIndices;
        WORD	    pIndices[] = { 0, 1, 2, 2, 1, 3,
                                   2, 3, 4, 4, 3, 5,
                                   4, 5, 6, 6, 5, 7,
                                   6, 7, 0, 0, 7, 1,
                                   0, 2, 6, 6, 2, 4,
                                   3, 1, 5, 5, 1, 7, };

        dwNbIndices = 6*6;
        if( NULL == CreateIndexBuffer( dwNbIndices, sizeof( WORD ), D3DFMT_INDEX16, pIndices ) )
            return	E_FAIL;
    } 

	// index line
    {
        DWORD       dwNbIndices;
        WORD	    pIndices[] = { 0, 1, 2, 3, 0, 2, 1, 3,
								   4, 5, 6, 7, 4, 6, 5, 7,
								   2, 4, 3, 5, 0, 6, 1, 7 };
		dwNbIndices = 8*3;
		if( NULL == CreateLineIndexBuffer( dwNbIndices, sizeof( WORD ), D3DFMT_INDEX16, pIndices ) )
			return	E_FAIL;
    }

	// shader
	{
		VOID*	pCode	= NULL;
		DWORD	dwSize	= 0;
		if( FAILED( File::Load( XL_FX_FILE, &pCode, &dwSize ) ) )
			return	E_FAIL;

		if( NULL == CreateEffectFXL( g_xlCubeVertexElements, pCode, dwSize ) ){
// [PREFIX:beginmodify] 2006/2/15 junyash PS#5182 reports memory leak 'pCode'
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
// Name: Cube::Release
// Arguments:
// Returns:
//	void
//-----------------------------------------------------------------------------
void	Cube::Release()
{
    PrimitiveBase::Release();
}

//-----------------------------------------------------------------------------
// Name: Cube::Render
// Arguments:
// Returns:
//	HRESULT
//-----------------------------------------------------------------------------
HRESULT	Cube::Render( const XMMATRIX& mWVP )
{
	// ==== state ====
	m_pd3dDevice->SetRenderState( D3DRS_ZENABLE, TRUE );
	m_pd3dDevice->SetVertexDeclaration( GetVertexDeclaration() );
	m_pd3dDevice->SetStreamSource( 0, GetVertexBuffer(), 0, GetVertexStride() );
	m_pd3dDevice->SetIndices( GetIndexBuffer() );

	if( m_hColorSampler )
		m_pEffectFXL->SetSampler( m_hColorSampler, m_pTexture[ 0 ] );
	m_pEffectFXL->SetMatrixF4x4( m_hWVP, ( FLOAT* )&mWVP );

	// ==== render ====
	m_pEffectFXL->BeginTechnique( m_hTqTexture, FXL_RESTORE_DEFAULT_STATE );
	m_pEffectFXL->BeginPass( m_hPassTexture );
	{
		m_pEffectFXL->Commit();
		m_pd3dDevice->DrawIndexedPrimitive( D3DPT_TRIANGLELIST, 0, 0, m_dwNbVertices, 0, 12 );
	}
	m_pEffectFXL->EndPass();
	m_pEffectFXL->EndTechnique();

	// ==== state ====

	return S_OK;
}

//-----------------------------------------------------------------------------
// Name: Cube::Render
// Arguments:
// Returns:
//	HRESULT
//-----------------------------------------------------------------------------
HRESULT	Cube::Render( const XMMATRIX& mWVP, D3DXCOLOR d3dxColor )
{
	// ==== state ====
	m_pd3dDevice->SetRenderState( D3DRS_ZENABLE, TRUE );
	m_pd3dDevice->SetVertexDeclaration( GetVertexDeclaration() ); 
	m_pd3dDevice->SetStreamSource( 0, GetVertexBuffer(), 0, GetVertexStride() );
	m_pd3dDevice->SetIndices( GetIndexBuffer() );

	m_pEffectFXL->SetMatrixF4x4( m_hWVP, ( FLOAT* )&mWVP );
	m_pEffectFXL->SetVectorF( m_hColor, ( FLOAT* )&d3dxColor );

	// ==== render ====
	m_pEffectFXL->BeginTechnique( m_hTqColor, FXL_RESTORE_DEFAULT_STATE );
	m_pEffectFXL->BeginPass( m_hPassColor );
	{
		m_pEffectFXL->Commit();
		m_pd3dDevice->DrawIndexedPrimitive( D3DPT_TRIANGLELIST, 0, 0, m_dwNbVertices, 0, 12 );
	}
	m_pEffectFXL->EndPass();
	m_pEffectFXL->EndTechnique();

	// ==== state ====

	return S_OK;
}

//-----------------------------------------------------------------------------
// Name: Cube::RenderLine
// Arguments:
// Returns:
//	HRESULT
//-----------------------------------------------------------------------------
HRESULT	Cube::RenderLine( const XMMATRIX& mWVP, D3DXCOLOR d3dxColor )
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
		m_pd3dDevice->DrawIndexedPrimitive( D3DPT_LINELIST, 0, 0, 0, 0, GetNbLineIndices() );
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