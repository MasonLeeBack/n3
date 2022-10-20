///////////////////////////////////////////////////////////////////////////////
// xlFrustum.cpp
// Collection: xlFrustum : Frustum
//-----------------------------------------------------------------------------

//=============================================================================
// includes
//=============================================================================
#include "stdafx.h"
#ifdef	_XBOX
#include "xlFrustum.h"
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
static const	D3DVERTEXELEMENT9	g_xlFlustumVertexElements[] = {
	{ 0,	0,	D3DDECLTYPE_FLOAT3,		D3DDECLMETHOD_DEFAULT,	D3DDECLUSAGE_POSITION,	0 },
	D3DDECL_END()
};

//=============================================================================
// methods
//=============================================================================

//-----------------------------------------------------------------------------
// Name: Frustum::Frustum
// Arguments:
// Returns:
//-----------------------------------------------------------------------------
Frustum::Frustum() : PrimitiveBase()
{
    m_fSizeX    = 0.0f;
    m_fSizeZ    = 0.0f;
    m_fSizeY    = 0.0f;
}


//-----------------------------------------------------------------------------
// Name: Frustum::~Frustum
// Arguments:
// Returns:
//-----------------------------------------------------------------------------
Frustum::~Frustum()
{
}


//-----------------------------------------------------------------------------
// Name: Frustum::Create
// Arguments:
//	pd3dDevice
//	fSize
//	dwRepeat
// Returns:
//	Square*
//-----------------------------------------------------------------------------
Frustum*	Frustum::Create( LPDIRECT3DDEVICE pd3dDevice, float fSizeX, float fSizeZ, float fSizeY )
{
    Frustum*  pFrustum;
    if( NULL == ( pFrustum = new Frustum ) )
  		return	NULL;

    if( FAILED( pFrustum->Construct( pd3dDevice, fSizeX, fSizeZ, fSizeY ) ) ){
	    pFrustum->Release();
	    return	NULL;
    }

    return pFrustum;
}


//-----------------------------------------------------------------------------
// Name: Frustum::Construct
// Arguments:
// Returns:
//	HRESULT
//-----------------------------------------------------------------------------
HRESULT Frustum::Construct( LPDIRECT3DDEVICE pd3dDevice, float fSizeX, float fSizeZ, float fSizeY )
{
    // PrimitiveBase
    if( FAILED( PrimitiveBase::Construct( pd3dDevice ) ) )
        return  E_FAIL;

    // vertex buffer
    if( NULL == CreateVertexBuffer( 5, sizeof( FRUSTUM_VERTEX ), 0L, NULL ) )
        return	E_FAIL;
    SetSize( fSizeX, fSizeZ, fSizeY );

    // index buffer for polygon
    {
        DWORD       dwNbIndices = 4 + 3*4;
        WORD	    pIndices[]  = { 1, 0, 3, 2,
                                    0, 1, 4,
                                    2, 0, 4,
                                    3, 2, 4,
                                    1, 3, 4, };

        if( NULL == CreateIndexBuffer( dwNbIndices, sizeof( WORD ), D3DFMT_INDEX16, pIndices ) )
            return	E_FAIL;
    }

    // index buffer for line
    {
        DWORD       dwNbIndices = 5 + 3*2;
        WORD	    pIndices[] = { 0, 1, 3, 2, 0,
                                   0, 4, 1,
                                   2, 4, 3 };

        if( NULL == CreateLineIndexBuffer( dwNbIndices, sizeof( WORD ), D3DFMT_INDEX16, pIndices ) )
            return	E_FAIL;
    }

	// Shader
	{
		VOID*	pCode	= NULL;
		DWORD	dwSize	= 0;
		if( FAILED( File::Load( XL_FX_FILE, &pCode, &dwSize ) ) )
			return	E_FAIL;

		if( NULL == CreateEffectFXL( g_xlFlustumVertexElements, pCode, dwSize ) ){
// [PREFIX:beginmodify] 2006/2/15 junyash PS#5183 reports memory leak 'pCode'
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
// Name: Release()
// Desc: 
//-----------------------------------------------------------------------------
void	Frustum::Release()
{
    PrimitiveBase::Release();
}


//-----------------------------------------------------------------------------
// Name: RenderLine()
// Desc: 
//-----------------------------------------------------------------------------
HRESULT	Frustum::RenderLine( const XMMATRIX& mWVP, D3DXCOLOR d3dxColor )
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

		m_pd3dDevice->DrawIndexedPrimitive( D3DPT_LINESTRIP, 0, 0, m_dwNbVertices, 5, 2 );
		m_pd3dDevice->DrawIndexedPrimitive( D3DPT_LINESTRIP, 0, 0, m_dwNbVertices, 8, 2 );
	}
	m_pEffectFXL->EndPass();
	m_pEffectFXL->EndTechnique();

	// ==== state ====

    return  S_OK;
}


//-----------------------------------------------------------------------------
// Name: SetSize()
// Desc: 
//-----------------------------------------------------------------------------
HRESULT	Frustum::SetSize( float  fSizeX, float  fSizeZ, float  fSizeY )
{
    // store
    m_fSizeX    = fSizeX;
    m_fSizeZ    = fSizeZ;
    m_fSizeY    = fSizeY;

    // vertex buffer
    {
        DWORD					dwNbVertices;
        LPDIRECT3DVERTEXBUFFER  pVertexBuffer;
        FRUSTUM_VERTEX          *pVertices;

        if( 0 == ( dwNbVertices = GetNbVertices() ) )
            return  E_FAIL;
        if( NULL == ( pVertexBuffer = GetVertexBuffer() ) )
            return  E_FAIL;
        if( FAILED( pVertexBuffer->Lock( 0, 0, ( void** )&pVertices, 0 ) ) )
            return  E_FAIL;

        float   fX = fSizeX/2.0f;
        float   fZ = fSizeZ/2.0f;
	    pVertices[ 0 ].vP	= D3DXVECTOR3( -fX,    0.0f,    fZ );
        pVertices[ 1 ].vP	= D3DXVECTOR3(  fX,    0.0f,    fZ );
        pVertices[ 2 ].vP	= D3DXVECTOR3( -fX,    0.0f,   -fZ );
	    pVertices[ 3 ].vP	= D3DXVECTOR3(  fX,    0.0f,   -fZ );
        pVertices[ 4 ].vP	= D3DXVECTOR3(  0.0f,  fSizeY,  0.0f );

        if( FAILED( pVertexBuffer->Unlock() ) )
            return  E_FAIL;
    }

    return  S_OK;
}


//-----------------------------------------------------------------------------
// Name: GetSize()
// Desc: 
//-----------------------------------------------------------------------------
HRESULT	Frustum::GetSize( float* pSizeX, float* pSizeZ, float* pSizeY )
{
    *pSizeX = m_fSizeX;
    *pSizeZ = m_fSizeZ;
    *pSizeY = m_fSizeY;
    
    return S_OK;
}


//=============================================================================
// namespace
//=============================================================================
} // end of namespace
#endif