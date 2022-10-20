#include "stdafx.h"
#include "BsKernel.h"
#include "BsWaterObject.h"
#include "BsMesh.h"
#include "BsMaterial.h"
#include "BsImplMaterial.h"

BOOL CBsWaterObject::s_bCaptureRefractMap = FALSE;

CBsWaterObject::CBsWaterObject()
{
	SetObjectType(BS_STATIC_OBJECT);
	m_bObjectAlphaBlend = true;		// Skin 파일에 Alpha Enable 세팅 하지 않아도 되도록..
	m_bEnableInstancing = false;
	m_pWaterCheckBuffer = NULL;
}

CBsWaterObject::~CBsWaterObject()
{
}

int CBsWaterObject::AttachMesh(CBsMesh* pMesh)  
{
	int nRet = CBsObject::AttachMesh( pMesh );

    int nVertexCount;
	D3DXVECTOR3 *pVB;
	m_pMesh->GetStreamBuffer( &pVB, nVertexCount );

	AABB *pBox = m_pMesh->GetBoundingBox();
	D3DXVECTOR3 vMin(pBox->Vmin.x, 0, pBox->Vmin.z) ;
	D3DXVECTOR3 vExt(pBox->Vmax.x - pBox->Vmin.x, 0, pBox->Vmax.z - pBox->Vmin.z) ;

	int nGridSize = 100;

	nGridSize = BsMin( nGridSize, (int)( BsMin(vExt.x, vExt.z) / 300.f ) );

	vExt.x = nGridSize / vExt.x;
	vExt.z = nGridSize / vExt.z;

	if( m_pWaterCheckBuffer ){
		delete [] m_pWaterCheckBuffer;
	}
	m_pWaterCheckBuffer = new BYTE[ nGridSize * nGridSize + 1 ];	
	memset( m_pWaterCheckBuffer, 0, nGridSize * nGridSize + 1);
	m_pWaterCheckBuffer[ 0 ] = nGridSize;

	for( int i = 0; i < nVertexCount; i++)
	{
		int nX = (int)(( pVB[i].x - vMin.x ) * vExt.x );
		int nY = (int)(( pVB[i].z - vMin.z ) * vExt.z );

		nX = BsMax(BsMin(nX, nGridSize-1), 0);
		nY = BsMax(BsMin(nY, nGridSize-1), 0);

        m_pWaterCheckBuffer[ nX + nY * nGridSize + 1 ] = 1;
	}

	delete [ ] pVB;
	
	return nRet;
}

void CBsWaterObject::Render(C3DDevice *pDevice)
{
	CBsObject::Render( pDevice );
}

void CBsWaterObject::RenderAlpha(C3DDevice *pDevice)
{
	if(!s_bCaptureRefractMap) {
		g_BsKernel.GetImageProcess()->ScreenCaptureBeforeAlphaBlendPass(pDevice);
		s_bCaptureRefractMap = TRUE;
	}
	CBsObject::RenderAlpha( pDevice );
}