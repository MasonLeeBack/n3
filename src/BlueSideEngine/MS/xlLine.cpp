///////////////////////////////////////////////////////////////////////////////
// xlLine.cpp
// Collection: xlPrimitive
//-----------------------------------------------------------------------------

//=============================================================================
// includes
//=============================================================================
#include "stdafx.h"
#ifdef	_XBOX
#include "xlLine.h"
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
static const	D3DVERTEXELEMENT9	g_xlLineVertexElements[] = {
	{ 0,	0,	D3DDECLTYPE_FLOAT3,		D3DDECLMETHOD_DEFAULT,	D3DDECLUSAGE_POSITION,	0 },
	D3DDECL_END()
};

	
//=============================================================================
// methods
//=============================================================================

//-----------------------------------------------------------------------------
// Name: Line::Line
// Arguments:
// Returns:
//-----------------------------------------------------------------------------  
Line::Line() : PrimitiveBase()
{
    m_d3dPtimitiveType  = D3DPT_LINESTRIP;
}


//-----------------------------------------------------------------------------
// Name: Line::~Line
// Arguments:
// Returns:
//-----------------------------------------------------------------------------  
Line::~Line()
{
}


//-----------------------------------------------------------------------------
// Name: Line::Create
// Arguments:
//  pd3dDevice      device
//  dwNbVertices    number of vertices
//  pVertices       verices
//  dwFlag          strip or list
// Returns:
//	Line*
//-----------------------------------------------------------------------------
Line*	Line::Create( LPDIRECT3DDEVICE    pd3dDevice,
                      DWORD               dwNbVertices,
                      D3DXVECTOR3*        pVertices,
                      D3DPRIMITIVETYPE    d3dPtimitiveType )
{
    Line*  pLine;
    if( NULL == ( pLine = new Line ) )
  		return	NULL;

    if( FAILED( pLine->Construct( pd3dDevice, dwNbVertices, pVertices, d3dPtimitiveType ) ) ){
	    pLine->Release();
	    return	NULL;
    }

    return pLine;
}

//-----------------------------------------------------------------------------
// Name: Line::Construct
// Arguments:
//  pd3dDevice      device
//  dwNbVertices    number of vertices
//  pVertices       verices
//  dwFlag          strip or list
// Returns:
//	Line*
//-----------------------------------------------------------------------------
HRESULT Line::Construct( LPDIRECT3DDEVICE pd3dDevice,
                         DWORD            dwNbVertices,
                         D3DXVECTOR3*     pVertices,
                         D3DPRIMITIVETYPE d3dPtimitiveType )
{
    if( D3DPT_LINELIST != d3dPtimitiveType &&
        D3DPT_LINESTRIP != d3dPtimitiveType )
        return  E_FAIL;

    m_d3dPtimitiveType  = d3dPtimitiveType;

    // primitive base
    if( FAILED( PrimitiveBase::Construct( pd3dDevice ) ) )
        return  E_FAIL;

    // vertex buffer
    if( NULL == CreateVertexBuffer( dwNbVertices, sizeof( LINE_VERTEX ), 0L, pVertices ) )
        return	E_FAIL;
    
    // index buffer
    {
    }

	// FXLite
	{
		VOID*	pCode	= NULL;
		DWORD	dwSize	= 0;
		if( FAILED( File::Load( XL_FX_FILE, &pCode, &dwSize ) ) )
			return	E_FAIL;

		if( NULL == CreateEffectFXL( g_xlLineVertexElements, pCode, dwSize ) ){
// [PREFIX:beginmodify] 2006/2/15 junyash PS#5186 reports memory leak 'pCode'
			File::Unload( pCode );
// [PREFIX:endmodify] junyash
			return	E_FAIL;
		}

		File::Unload( pCode );
	}

	m_hTqLine		= m_pEffectFXL->GetTechniqueHandle( "Line" );
	m_hPassLine		= m_pEffectFXL->GetPassHandleFromIndex( m_hTqLine, 0 );
	m_hWVP			= m_pEffectFXL->GetParameterHandle( "g_mWVP" );
	m_hColor		= m_pEffectFXL->GetParameterHandle( "g_vColor" );

	return S_OK;
}


//-----------------------------------------------------------------------------
// Name: Line::Release()
// Arguments:
// Returns:
//-----------------------------------------------------------------------------
void	Line::Release()
{
    PrimitiveBase::Release();
}


//-----------------------------------------------------------------------------
// Name: Line::SetVertices
// Arguments:
//  dwNbVertices        number of vertices
//  pVertices           vertices
//  d3dPtimitiveType    primitive type
// Returns:
//	HRESULT
//-----------------------------------------------------------------------------
HRESULT Line::SetVertices( DWORD               dwNbVertices,
                           D3DXVECTOR3*        pVertices,
                           D3DPRIMITIVETYPE    d3dPtimitiveType )
{
    if( dwNbVertices > m_dwNbVertices ){
        ReleaseVertexBuffer();

        if( FAILED( Construct( m_pd3dDevice, dwNbVertices, pVertices, d3dPtimitiveType ) ) ){
	            return	E_FAIL;
        }
    }
    else{
        if( pVertices ){
	        void	*pP;
	        if( FAILED( m_pVertexBuffer->Lock( 0, 0, &pP, 0 ) ) )
		        return	E_FAIL;

	        memcpy( pP, pVertices, dwNbVertices * sizeof( LINE_VERTEX ) );
	        m_pVertexBuffer->Unlock();
        }
    }

    return  S_OK;
}


//-----------------------------------------------------------------------------
// Name: Line::Render( D3DXCOLOR d3dColor )
// Arguments:
//  d3dColor    color
// Returns:
//	HRESULT
//-----------------------------------------------------------------------------
HRESULT	Line::Render( const XMMATRIX& mWVP, D3DXCOLOR d3dxColor )
{
	// ==== state ====
	m_pd3dDevice->SetRenderState( D3DRS_ZENABLE, TRUE );

	// ==== const ====
	m_pEffectFXL->SetMatrixF4x4( m_hWVP, ( FLOAT* )&mWVP );
	m_pEffectFXL->SetVectorF( m_hColor, ( FLOAT* )&d3dxColor );

	// ==== render ====
	m_pEffectFXL->BeginTechnique( m_hTqLine, FXL_RESTORE_DEFAULT_STATE );
	m_pEffectFXL->BeginPass( m_hPassLine );
	{
		m_pEffectFXL->Commit();
		m_pd3dDevice->SetVertexDeclaration( GetVertexDeclaration() ); 
		m_pd3dDevice->SetStreamSource( 0, GetVertexBuffer(), 0, GetVertexStride() );

		// primitive
		if( D3DPT_LINELIST == m_d3dPtimitiveType )
			m_pd3dDevice->DrawPrimitive( D3DPT_LINELIST, 0, m_dwNbVertices/2 );
		else
			m_pd3dDevice->DrawPrimitive( D3DPT_LINESTRIP, 0, m_dwNbVertices - 1 );
	}
	m_pEffectFXL->EndPass();
	m_pEffectFXL->EndTechnique();

	// ==== state ====

	return S_OK;
}

//-----------------------------------------------------------------------------
// Name: Line::Render( D3DXCOLOR d3dColor )
// Arguments:
//  d3dColor    color
// Returns:
//	HRESULT
//-----------------------------------------------------------------------------
HRESULT	Line::Render( const XMMATRIX& mWVP, D3DXCOLOR d3dxColor, DWORD dwNbPrimitive )
{
	// ==== state ====
	m_pd3dDevice->SetRenderState( D3DRS_ZENABLE, TRUE );

	// ==== const ====
	m_pEffectFXL->SetMatrixF4x4( m_hWVP, ( FLOAT* )&mWVP );
	m_pEffectFXL->SetVectorF( m_hColor, ( FLOAT* )&d3dxColor );

	// ==== render ====
	m_pEffectFXL->BeginTechnique( m_hTqLine, FXL_RESTORE_DEFAULT_STATE );
	m_pEffectFXL->BeginPass( m_hPassLine );
	{
		m_pEffectFXL->Commit();
		m_pd3dDevice->SetVertexDeclaration( GetVertexDeclaration() ); 
		m_pd3dDevice->SetStreamSource( 0, GetVertexBuffer(), 0, GetVertexStride() );

		// primitive
		m_pd3dDevice->DrawPrimitive( m_d3dPtimitiveType, 0, dwNbPrimitive );
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