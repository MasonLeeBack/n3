///////////////////////////////////////////////////////////////////////////////
// xlGrid.h
// Collection: xlPrimitiveBase
//-----------------------------------------------------------------------------
#ifndef __inc_xlGrid_h__
#define __inc_xlGrid_h__

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
struct GRID_VERTEX
{
	D3DXVECTOR3 vP;
	D3DXVECTOR3 vN;
	D3DXVECTOR2 vT0;
};

#define GRID_VERTEX_FVF ( D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1 )

//=============================================================================
// classes
//=============================================================================
class Grid : public PrimitiveBase {
	protected:
        float                   m_fSizeX;
        float                   m_fSizeZ;
		DWORD					m_dwNbCellX;
		DWORD					m_dwNbCellZ;
		D3DFORMAT				m_d3dIndexFormat;

        HRESULT                 Construct( LPDIRECT3DDEVICE pd3dDevice,
                                           float fSizeX,
                                           float fSizeZ,
                                           DWORD dwNbCellX,
                                           DWORD dwNbCellZ,
                                           DWORD dwRepeatX,
                                           DWORD dwRepeatZ );

	public:
		Grid();
		~Grid();

		static Grid*    		Create( LPDIRECT3DDEVICE pd3dDevice,
                                        float fSizeX,
                                        float fSizeZ,
                                        DWORD dwNbCellX,
                                        DWORD dwNbCellZ,
                                        DWORD dwRepeatX,
                                        DWORD dwRepeatZ );
		void					Release();

		HRESULT                 GetSize( float* pSizeX, float* pSizeZ )			{ *pSizeX = m_fSizeX; *pSizeZ = m_fSizeZ; return S_OK; }
        HRESULT                 GetNbCell( DWORD* pNbCellX, DWORD* pNbCellZ )	{ *pNbCellX = m_dwNbCellX; *pNbCellZ = m_dwNbCellZ; return S_OK; }
		D3DFORMAT               GetIndexFormat()								{ return m_d3dIndexFormat; }

        HRESULT                 RecomputeNormals();
};

//=============================================================================
// namespace
//=============================================================================
} // end of namespace
#endif	// __inc_xlGrid_h__

