#include "stdafx.h"
#include "FcFXTidalWave.h"
#include "3DDevice.h"
#include "BsMaterial.h"
#include "BsKernel.h"
#include "BsImageProcess.h"
#include "BsObject.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif 



void CFcFXTidalWave::CustomRender( C3DDevice *pDev)
{
#ifdef _XBOX
	DWORD value[4];
	pDev->GetRenderState( D3DRS_ALPHABLENDENABLE, &value[0] );
	pDev->GetRenderState( D3DRS_ALPHATESTENABLE, &value[1] );
	pDev->GetRenderState(D3DRS_ZWRITEENABLE , &value[2]);
	pDev->GetRenderState( D3DRS_CULLMODE,   &value[3] );

	D3DXMATRIX matWorld;
	D3DXMATRIX matWVP;
	D3DXMATRIX matView;
	D3DXMATRIX matProj;
    D3DXMATRIX matInvView;

	matView = *g_BsKernel.GetParamViewMatrix();
	matProj = *g_BsKernel.GetParamProjectionMatrix();
	matInvView = *g_BsKernel.GetParamInvViewMatrix();
	D3DXMatrixIdentity(&matWorld);

	if( GetWaterDrop() ) {
		pDev->SetRenderState( D3DRS_CULLMODE,   D3DCULL_NONE );
		BsAssert(m_nMaterialIndex != -1);
		CBsMaterial* pBsMaterial = g_BsKernel.GetMaterialPtr(m_nMaterialIndex);
		pBsMaterial->BeginMaterial(0, 0);
		pDev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);// D3DBLEND_SRCALPHA); D3DBLEND_SRCCOLOR
		pDev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);//D3DBLEND_INVSRCALPHA); D3DBLEND_INVSRCCOLOR

		pBsMaterial->BeginPass(0);
		g_BsKernel.SetVertexDeclaration(m_nVertexDeclIdx);

		pBsMaterial->SetTexture( m_hBackBuffer, g_BsKernel.GetImageProcess()->GetBackBufferTexture());

		D3DXMatrixMultiply(&matWVP, &matWorld, &(matView * matProj));
		pBsMaterial->SetMatrix( m_hWorldViewProj, &matWVP);
		pBsMaterial->SetMatrix( m_hWorld, &matWorld );
		pBsMaterial->SetVector(m_hlightAmb, &m_vCount );
		pBsMaterial->SetTexture( m_hSwizzle, (LPDIRECT3DBASETEXTURE9)(g_BsKernel.GetTexturePtr(m_nSwizzleTex)));
		pBsMaterial->CommitChanges();

		WORD *pBuffer = NULL;
		BYTE *pBuffer2 = NULL;
		HRESULT hr = pDev->BeginIndexedVertices( D3DPT_TRIANGLELIST, 0, m_iVertex, m_iTriangle * 3,D3DFMT_INDEX16, sizeof(CUSTOMVERTEX),  (void**)&pBuffer, (void**)&pBuffer2 );
		if( SUCCEEDED( hr ) )
		{
			memcpy( pBuffer, m_pI2, m_iTriangle * sizeof(WORD) * 3 );
			memcpy( pBuffer2, m_pV2, sizeof(CUSTOMVERTEX)*m_iVertex );
			pDev->EndIndexedVertices();
		}
		pBsMaterial->EndPass();
		pBsMaterial->EndMaterial();
	}
	else {
		CBsMaterial* pBsMaterial = g_BsKernel.GetMaterialPtr(m_nMaterialIndex);
		pBsMaterial->BeginMaterial(0, 0);
		pDev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);// D3DBLEND_SRCALPHA); D3DBLEND_SRCCOLOR
		pDev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);//D3DBLEND_INVSRCALPHA); D3DBLEND_INVSRCCOLOR

		pBsMaterial->BeginPass(0);

		g_BsKernel.SetVertexDeclaration(m_nVertexDeclIdx);
		D3DXMatrixMultiply(&matWVP, &matWorld, &(matView * matProj));
		pBsMaterial->SetMatrix( m_hWorldViewProj, &matWVP);
		pBsMaterial->SetMatrix( m_hWorld, &matWorld );
		pBsMaterial->SetMatrix( m_hworldInverseTranspose, &matWorld );
		pBsMaterial->SetVector(m_hcounter, &m_vCount); 

		pBsMaterial->SetTexture( m_hBackBuffer, g_BsKernel.GetImageProcess()->GetBackBufferTexture());

		if( m_nSwizzleTex != -1 ) pBsMaterial->SetTexture( m_hSwizzle, (LPDIRECT3DBASETEXTURE9)(g_BsKernel.GetTexturePtr(m_nSwizzleTex)));
		pBsMaterial->CommitChanges();
		
		WORD *pBuffer = NULL;
		BYTE *pBuffer2 = NULL;
		HRESULT hr = pDev->BeginIndexedVertices( D3DPT_TRIANGLELIST, 0, m_iVertex, m_iTriangle*3,D3DFMT_INDEX16, sizeof(CUSTOMVERTEX),  (void**)&pBuffer, (void**)&pBuffer2 );
		if( SUCCEEDED( hr ) ) {
			memcpy( pBuffer, m_pI2, m_iTriangle * sizeof(WORD) * 3 );
			memcpy( pBuffer2, m_pV2, sizeof(CUSTOMVERTEX)*m_iVertex );
			pDev->EndIndexedVertices();
		}
		pBsMaterial->EndPass();
		pBsMaterial->EndMaterial();
	}
	
	pDev->SetRenderState( D3DRS_ALPHABLENDENABLE, value[0] );
	pDev->SetRenderState( D3DRS_ALPHATESTENABLE, value[1] );
	pDev->SetRenderState(D3DRS_ZWRITEENABLE , value[2]);
	pDev->SetRenderState( D3DRS_CULLMODE,   value[3] );
#endif
}
