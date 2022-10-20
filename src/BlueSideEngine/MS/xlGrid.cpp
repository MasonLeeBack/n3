///////////////////////////////////////////////////////////////////////////////
// xlGrid.cpp
// Collection: xlGrid : Grid
//	$Header: /xeProject_031009/xeLibrary/utility/xlRenderTexture.h 2     03/11/07 14:27 Koheio $
//-----------------------------------------------------------------------------

//=============================================================================
// includes
//=============================================================================
#include "stdafx.h"
#ifdef	_XBOX
#include "xlGrid.h"
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
//=============================================================================
// methods
//=============================================================================

//-----------------------------------------------------------------------------
// Name: Grid()
// Desc: 
//-----------------------------------------------------------------------------
Grid::Grid() : PrimitiveBase()
{
	m_fSizeX			= 0.0f;
	m_fSizeZ			= 0.0f;
	m_dwNbCellX			= 0;
	m_dwNbCellZ			= 0;
	m_d3dIndexFormat	= D3DFMT_UNKNOWN;
}

//-----------------------------------------------------------------------------
// Name: ~Grid()
// Desc: 
//-----------------------------------------------------------------------------
Grid::~Grid()
{
}

//-----------------------------------------------------------------------------
// Name: Create()
// Desc: 
//-----------------------------------------------------------------------------
Grid*	Grid::Create( LPDIRECT3DDEVICE	pd3dDevice,
                      float				fSizeX,
                      float				fSizeZ,
                      DWORD				dwNbCellX,
                      DWORD				dwNbCellZ,
                      DWORD				dwRepeatU,
                      DWORD				dwRepeatV )
{
    Grid*  pGrid;
    if( NULL == ( pGrid = new Grid ) )
  		return	NULL;

    if( FAILED( pGrid->Construct( pd3dDevice, fSizeX, fSizeZ, dwNbCellX, dwNbCellZ, dwRepeatU, dwRepeatV ) ) ){
	    pGrid->Release();
	    return	NULL;
    }

    return pGrid;
}

//-----------------------------------------------------------------------------
// Name: Grid::Construct
// Arguments:
// Returns:
//	HRESULT
//-----------------------------------------------------------------------------
HRESULT Grid::Construct( LPDIRECT3DDEVICE	pd3dDevice,
                         float				fSizeX,
                         float				fSizeZ,
                         DWORD				dwNbCellX,
                         DWORD				dwNbCellZ,
                         DWORD				dwRepeatU,
                         DWORD				dwRepeatV )
{
    if( FAILED( PrimitiveBase::Construct( pd3dDevice ) ) )
        return  E_FAIL;

    m_fSizeX = fSizeX;
    m_fSizeZ = fSizeZ;
	m_dwNbCellX = dwNbCellX;
	m_dwNbCellZ = dwNbCellZ;

	// vertex buffer
    {
    	DWORD       dwNbVertices;
        GRID_VERTEX	*pVertices, *pV;
    	float	    fSx, fSz;
    	float	    fDx, fDz;
        float       fDTu, fDTv;

        dwNbVertices = ( dwNbCellX + 1 )*( dwNbCellZ + 1 );
        if( NULL == ( pV = pVertices = new GRID_VERTEX [ dwNbVertices ] ) )
            return  E_FAIL;

    	fSx = -fSizeX/2.0f;
    	fDx =  fSizeX/( float )dwNbCellX;
    	fSz =  fSizeZ/2.0f;
    	fDz = -fSizeZ/( float )dwNbCellZ;
    	fDTu    =  1.0f*( float )( dwRepeatU )/( float )( dwNbCellX );
    	fDTv    =  1.0f*( float )( dwRepeatV )/( float )( dwNbCellZ );

	    for( DWORD i = 0; i < dwNbCellX + 1; i++ ){
		    for( DWORD j = 0; j < dwNbCellZ + 1; j++ ){
			    pV->vP   = D3DXVECTOR3( fSx + fDx*i, 0.0f, fSz + fDz*j );
			    pV->vN   = D3DXVECTOR3( 0.0f, 1.0f, 0.0f );
			    pV->vT0  = D3DXVECTOR2( fDTu*i, fDTv*j );

			    pV++;
		    }
	    }

        if( NULL == CreateVertexBuffer( dwNbVertices, sizeof( GRID_VERTEX ), 0L, pVertices ) )
// [PREFIX:beginmodify] 2006/2/15 junyash PS#5184 reports memory leak 'pVerteices'
		{
			SAFE_DELETE_ARRAY( pVertices );
			return	E_FAIL;
		}
// [PREFIX:endmodify] junyash

        SAFE_DELETE_ARRAY( pVertices );
    }

	// index buffer
    {
    	DWORD       dwNbIndices;
      	dwNbIndices = 2*( dwNbCellZ + 1 )*dwNbCellX;

		if( dwNbIndices < 65536 ){

			// 16bit index
			m_d3dIndexFormat	= D3DFMT_INDEX16;

			WORD	    *pIndices, *pI;
			if( NULL == ( pI = pIndices = new WORD [ dwNbIndices ] ) )
				return  E_FAIL;

			for( DWORD i = 0; i < dwNbCellX; i++ ){
				for( DWORD j = 0; j < dwNbCellZ + 1; j++ ){
					*pI++ = ( WORD )( ( dwNbCellZ + 1 )*  i       + j );
					*pI++ = ( WORD )( ( dwNbCellZ + 1 )*( i + 1 ) + j );
				}
			}

			if( NULL == CreateIndexBuffer( dwNbIndices, sizeof( WORD ), D3DFMT_INDEX16, pIndices ) )
// [PREFIX:beginmodify] 2006/2/15 junyash PS#5184 reports memory leak 'pIndices'
			{
				SAFE_DELETE_ARRAY( pIndices );
				return	E_FAIL;
			}
// [PREFIX:endmodify] junyash

			SAFE_DELETE_ARRAY( pIndices );
		}
		else{

			// 32bit index
			m_d3dIndexFormat	= D3DFMT_INDEX32;

			DWORD	    *pIndices, *pI;
			if( NULL == ( pI = pIndices = new DWORD [ dwNbIndices ] ) )
				return  E_FAIL;

			for( DWORD i = 0; i < dwNbCellX; i++ ){
				for( DWORD j = 0; j < dwNbCellZ + 1; j++ ){
					*pI++ = ( DWORD )( ( dwNbCellZ + 1 )*  i       + j );
					*pI++ = ( DWORD )( ( dwNbCellZ + 1 )*( i + 1 ) + j );
				}
			}

			if( NULL == CreateIndexBuffer( dwNbIndices, sizeof( DWORD ), D3DFMT_INDEX32, pIndices ) )
// [PREFIX:beginmodify] 2006/2/15 junyash PS#5184 reports memory leak 'pIndices'
			{
				SAFE_DELETE_ARRAY( pIndices );
				return	E_FAIL;
			}
// [PREFIX:endmodify] junyash

			SAFE_DELETE_ARRAY( pIndices );
		}
    } 


	// index rectangle grid
    {
    	DWORD       dwNbLineIndices;
      	dwNbLineIndices = 2*( dwNbCellZ + 1 )*( dwNbCellX + 1 );

		if( dwNbLineIndices < 65536 ){

			// 16bit index
			m_d3dIndexFormat	= D3DFMT_INDEX16;

			WORD	    *pLineIndices, *pLI;
			if( NULL == ( pLI = pLineIndices = new WORD [ dwNbLineIndices ] ) )
				return  E_FAIL;

			for( DWORD i = 0; i < dwNbCellX + 1; i++ ){
				for( DWORD j = 0; j < dwNbCellZ + 1; j++ ){
					*pLI++ = ( WORD )( ( dwNbCellZ + 1 )*  i       + j );
				}
			}

			for( DWORD j = 0; j < dwNbCellZ + 1; j++ ){
				for( DWORD i = 0; i < dwNbCellX + 1; i++ ){
					*pLI++ = ( WORD )( ( dwNbCellZ + 1 )*  i       + j );
				}
			}

			if( NULL == CreateLineIndexBuffer( dwNbLineIndices, sizeof( WORD ), D3DFMT_INDEX16, pLineIndices ) )
// [PREFIX:beginmodify] 2006/2/15 junyash PS#5184 reports memory leak 'pLineIndices'
			{
				SAFE_DELETE_ARRAY( pLineIndices );
				return	E_FAIL;
			}
// [PREFIX:endmodify] junyash

			SAFE_DELETE_ARRAY( pLineIndices );
		}
		else{

			// 32bit index
			m_d3dIndexFormat	= D3DFMT_INDEX32;

			DWORD	    *pLineIndices, *pLI;
			if( NULL == ( pLI = pLineIndices = new DWORD [ dwNbLineIndices ] ) )
				return  E_FAIL;

			for( DWORD i = 0; i < dwNbCellX + 1; i++ ){
				for( DWORD j = 0; j < dwNbCellZ + 1; j++ ){
					*pLI++ = ( DWORD )( ( dwNbCellZ + 1 )*  i       + j );
				}
			}

			for( DWORD j = 0; j < dwNbCellZ + 1; j++ ){
				for( DWORD i = 0; i < dwNbCellX + 1; i++ ){
					*pLI++ = ( DWORD )( ( dwNbCellZ + 1 )*  i       + j );
				}
			}

			if( NULL == CreateLineIndexBuffer( dwNbLineIndices, sizeof( DWORD ), D3DFMT_INDEX32, pLineIndices ) )
// [PREFIX:beginmodify] 2006/2/15 junyash PS#5184 reports memory leak 'pLineIndices'
			{
				SAFE_DELETE_ARRAY( pLineIndices );
				return	E_FAIL;
			}
// [PREFIX:endmodify] junyash

			SAFE_DELETE_ARRAY( pLineIndices );
		}
	}

	return S_OK;
}

//-----------------------------------------------------------------------------
// Name: Release()
// Desc: 
//-----------------------------------------------------------------------------
void	Grid::Release()
{
    PrimitiveBase::Release();
}

//-----------------------------------------------------------------------------
// Name: RecomputeNormals()
// Desc: 
//-----------------------------------------------------------------------------
HRESULT Grid::RecomputeNormals()
{
    GRID_VERTEX	*pVertices;
    if( FAILED( m_pVertexBuffer->Lock( 0, 0, ( void ** )&pVertices, 0 ) ) )
	    return	E_FAIL;

    for( DWORD i = 0; i < m_dwNbCellX; i++ ){
        for( DWORD j = 0; j < m_dwNbCellZ; j++ ){
            D3DXVECTOR3 vP0, vP1, vP2;

            vP0 = pVertices[ ( m_dwNbCellZ + 1 )*  i       + j ].vP;
            vP1 = pVertices[ ( m_dwNbCellZ + 1 )*( i + 1 ) + j ].vP;
            vP2 = pVertices[ ( m_dwNbCellZ + 1 )*  i       + j + 1 ].vP;

            D3DXVECTOR3 vE0, vE1, vN;
            vE0 = vP1 - vP0;
            vE1 = vP2 - vP0;
            D3DXVec3Cross( &vN, &vE1, &vE0 );
            D3DXVec3Normalize( &vN, &vN );
            pVertices[ ( m_dwNbCellZ + 1 )*  i       + j ].vN = vN;
        }

        pVertices[ ( m_dwNbCellZ + 1 )* i + m_dwNbCellZ ].vN = pVertices[ ( m_dwNbCellZ + 1 )* i + 0 ].vN;
    }

    for( DWORD j = 0; j < m_dwNbCellZ; j++ ){
        pVertices[ ( m_dwNbCellZ + 1 )* m_dwNbCellX + j ].vN = pVertices[ ( m_dwNbCellZ + 1 )* 0 + j ].vN;
    }

    pVertices[ ( m_dwNbCellZ + 1 )* m_dwNbCellX + m_dwNbCellZ ].vN = pVertices[ ( m_dwNbCellZ + 1 )* 0 + 0 ].vN;

    m_pVertexBuffer->Unlock();

    return  S_OK;
}

//=============================================================================
// namespace
//=============================================================================
} // end of namespace
#endif