#include "stdafx.h"
#include "FcMinimapObject.h"
#include "3DDevice.h"
#include "BsKernel.h"
#include "BsObject.h"

struct COLORTEXTUREVERTEX
{
	D3DXVECTOR3 Vertex;
	DWORD		dwColor;
	float		fTextureU;
	float		fTextureV;
};


#define	D3DFVF_COLORTEXTUREVERTEX		(D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1)


CFcMinimapObject::CFcMinimapObject()
{
	m_nEngineIndex = -1;
	m_nBackgroundTexIndex = -1;
	m_pD3DDevice = NULL;
	m_fZoom = 1.f;
}

CFcMinimapObject::~CFcMinimapObject()
{
}

void CFcMinimapObject::Process()
{

}

void CFcMinimapObject::Initialize( char* pMinimapFileName )
{
	m_pD3DDevice = g_BsKernel.GetD3DDevice();
	m_nEngineIndex = g_BsKernel.CreateStaticObject(-1,true);
	m_nBackgroundTexIndex = g_BsKernel.LoadVolumeTexture( pMinimapFileName );
	g_BsKernel.SendMessage( m_nEngineIndex, BS_SET_CUSTOM_RENDERER, (DWORD)CFcMinimapObject::CustomRender, (DWORD)this );
}





void CFcMinimapObject::CustomRender( void *pThis, C3DDevice *pDev, D3DXMATRIX *matObject )
{
/*
	D3DXMATRIX		Mat;
	D3DXMATRIX		MatProj;
	D3DXMATRIX		MatView;
	D3DXMATRIX		MatWorld;

	D3DXMatrixIdentity(&Mat);

	LPDIRECT3DDEVICE9 pD3DDevice = pDev->GetD3DDevice();


	pD3DDevice->GetTransform(D3DTS_VIEW, &MatView);
	pD3DDevice->GetTransform(D3DTS_WORLD, &MatWorld);
	pD3DDevice->GetTransform(D3DTS_PROJECTION, &MatProj);


	pD3DDevice->SetTransform(D3DTS_VIEW, &Mat);
//	pD3DDevice->SetTransform(STR_TRANSFORM_WORLD, &Mat);
	pD3DDevice->SetTransform(D3DTS_WORLD, &Mat);

//	D3DXMatrixOrthoOffCenterLH(&Mat, 0, (float)(m_nWidth - 1), (float)(m_nHeight - 1), 0, -1.0f, 1.0f);
	D3DXMatrixOrthoOffCenterLH(&Mat, 0, (float)(640 - 1), (float)(480 - 1), 0, -1.0f, 1.0f);

	pD3DDevice->SetTransform(D3DTS_PROJECTION, &Mat);
//	pD3DDevice->SetVertexShader(D3DFVF_COLORTEXTUREVERTEX);

	DWORD dwColor = 0xffffffff;
	D3DXVECTOR3 Pos( 0.f, 0.f, 0.f );
//	DrawUVMesh( Pos, dwColor, m_fZoom, m_fZoom, 0, 0, 512, 512, 512, 512 );
	DrawUVMesh( pD3DDevice, Pos, dwColor, 1.f, 1.f, 0, 0, 512, 512, 512, 512 );


	pD3DDevice->SetTransform(D3DTS_VIEW, &MatView);
	pD3DDevice->SetTransform(D3DTS_WORLD, &MatWorld);
	pD3DDevice->SetTransform(D3DTS_PROJECTION, &MatProj);


//	pD3DDevice->RestoreState();
//	pDev->SetRenderTarget(pDev->GetBackBuffer(), pDev->GetDepthBuffer());
//	pDev->EndScene();

*/


/*
	D3DXMATRIX matWorld;
	D3DXMATRIX matWVP;
	D3DXMATRIX matView;
	D3DXMATRIX matProj;


	UINT iPass, cPasses;

	CFakeLiquid *pObj = (CFakeLiquid *)pThis;
	if (pObj->m_bInitialized == false)
		return;

	LPDIRECT3DDEVICE9 pD3DDevice = pDev->GetD3DDevice();
	pD3DDevice->GetTransform(D3DTS_PROJECTION,&matProj);
	pD3DDevice->GetTransform(D3DTS_VIEW,&matView);

	pObj->m_pEffect->Begin(&cPasses, 0);

	pDev->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE  );
	pDev->SetRenderState( D3DRS_ALPHATESTENABLE, FALSE );

	pDev->SetRenderState( D3DRS_LIGHTING, TRUE  );

	static	D3DXVECTOR4	vCount(0.0f,1.0f,0.0f, 0.0f);

	vCount.x += .1f;

	for (iPass = 0; iPass < cPasses; iPass++)
	{
		pObj->m_pEffect->BeginPass(iPass);


		D3DXMatrixIdentity(&matWorld);

		pObj->m_pEffect->SetMatrix( "worldViewProj", D3DXMatrixMultiply(&matWVP, &matWorld, &(matView * matProj)) );
		pObj->m_pEffect->SetMatrix( "world", &matWorld );
		pObj->m_pEffect->SetMatrix( "worldInverseTranspose", &matWorld );
		pObj->m_pEffect->SetMatrix( "viewInverse", g_BsKernel.GetParamInvViewMatrix());

		pObj->m_pEffect->SetVector("lightDir", CBsKernel::GetInstance().GetLightDirection());
		pObj->m_pEffect->SetVector("lightDiffuse", CBsKernel::GetInstance().GetLightDiffuse());
		pObj->m_pEffect->SetVector("lightAmbient", CBsKernel::GetInstance().GetLightAmbient());

		pObj->m_pEffect->SetVector("counter", &vCount); 


		int nBackBuffer = g_BsKernel.GetImageProcess()->GetBackBufferTexture();

		pObj->m_pEffect->SetTexture( "BackBuffer", (LPDIRECT3DBASETEXTURE9)(g_BsKernel.GetTexturePtr(nBackBuffer)));
		pObj->m_pEffect->SetTexture( "Swizzle", (LPDIRECT3DBASETEXTURE9)(g_BsKernel.GetTexturePtr(pObj->m_nSwizzleTex)));

		pObj->m_pEffect->CommitChanges();


		pD3DDevice->SetStreamSource( 0 , pObj->m_pVB , 0 , sizeof(CUSTOMVERTEX));
		pD3DDevice->SetIndices( pObj->m_pIB ); // , 0 );
		pD3DDevice->SetVertexDeclaration(pObj->m_pVertexDecl);

		pD3DDevice->DrawIndexedPrimitive(
			D3DPT_TRIANGLELIST,
			0,
			0,
			pObj->m_iVertex,
			0,
			pObj->m_iTriangle);

		pObj->m_pEffect->EndPass() ;
	}
	pObj->m_pEffect->End();

	//	pDev->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
	pDev->SetRenderState( D3DRS_ALPHATESTENABLE, TRUE );
#endif //_XBOX
*/
}




















void CFcMinimapObject::DrawUVMesh(LPDIRECT3DDEVICE9 pD3dDevice, D3DXVECTOR3 center, DWORD dwColor, float	fScaleX, float fScaleY, int nU1, int nV1, int nU2, int nV2, int mapSizeX, int mapSizeY)
{
	float				u1,v1,u2,v2;

	u1 = ((float)(nU1)) / ((float)(mapSizeX));
	v1 = ((float)(nV2)) / ((float)(mapSizeY));
	u2 = ((float)(nU2)) / ((float)(mapSizeX));
	v2 = ((float)(nV1)) / ((float)(mapSizeY));

	DrawUVMesh(pD3dDevice, center, dwColor, (int)((float)(nU2 - nU1 + 1) * fScaleX), (int)((float)(nV2 - nV1 + 1) * fScaleY), u1,v1,u2,v2);

}


void CFcMinimapObject::DrawUVMesh(LPDIRECT3DDEVICE9 pD3dDevice, D3DXVECTOR3 center, DWORD dwColor, float fRot, float	fScaleX, float fScaleY, int nU1, int nV1, int nU2, int nV2, int mapSizeX, int mapSizeY)
{
	float				u1,v1,u2,v2;

	u1 = ((float)(nU1)) / ((float)(mapSizeX));
	v1 = ((float)(nV2)) / ((float)(mapSizeY));
	u2 = ((float)(nU2)) / ((float)(mapSizeX));
	v2 = ((float)(nV1)) / ((float)(mapSizeY));

	DrawUVMesh(pD3dDevice, center, dwColor, (int)((float)(nU2 - nU1 + 1) * fScaleX), (int)((float)(nV2 - nV1 + 1) * fScaleY), fRot, u1,v1,u2,v2);
}


void CFcMinimapObject::DrawUVMesh(LPDIRECT3DDEVICE9 pD3dDevice, D3DXVECTOR3 center, DWORD dwColor, int width, int height, int nU1, int nV1, int nU2, int nV2, int mapSizeX, int mapSizeY)
{

	float				u1,v1,u2,v2;

	u1 = ((float)(nU1)) / ((float)(mapSizeX));
	v1 = ((float)(nV2)) / ((float)(mapSizeY));
	u2 = ((float)(nU2)) / ((float)(mapSizeX));
	v2 = ((float)(nV1)) / ((float)(mapSizeY));

	DrawUVMesh(pD3dDevice, center, dwColor, width, height, u1,v1, u2, v2);
}

void CFcMinimapObject::DrawUVMesh(LPDIRECT3DDEVICE9 pD3dDevice, D3DXVECTOR3 center, DWORD dwColor, int width, int height, float fRot, float fU1, float fV1, float fU2, float fV2)
{
	int					nScan;
	D3DVECTOR			pos[4];
	D3DVECTOR			Crd[4];
	COLORTEXTUREVERTEX	Vertex[6];
	D3DVECTOR			lt,rb;

	float				fWidth, fHeight;

	fWidth = (float)width;

	if (fWidth < 0)
		fWidth *= -1;

	fHeight = (float)height;

	if (fHeight < 0)
		fHeight *= -1;

	lt.x = - fWidth / 2.0f;
	rb.x = fWidth / 2.0f;

	lt.y = - fHeight / 2.0f;
	rb.y = fHeight / 2.0f;

	Crd[0].x = rb.x;
	Crd[0].y = lt.y;

	Crd[1].x = lt.x;
	Crd[1].y = lt.y;

	Crd[2].x = lt.x;
	Crd[2].y = rb.y;

	Crd[3].x = rb.x;
	Crd[3].y = rb.y;


	for(nScan = 0;nScan < 4;nScan++)
	{
		pos[nScan].x = (float)cos(fRot) * Crd[nScan].x + (float)sin(fRot) * Crd[nScan].y + center.x;
		pos[nScan].y = -(float)sin(fRot) * Crd[nScan].x + (float)cos(fRot) * Crd[nScan].y + center.y;
		pos[nScan].z = center.z;
	}

	Vertex[0].Vertex=pos[3];
	Vertex[1].Vertex=pos[1];
	Vertex[2].Vertex=pos[2];
	Vertex[3].Vertex=pos[0];
	Vertex[4].Vertex=pos[1];
	Vertex[5].Vertex=pos[3];

	Vertex[0].dwColor= dwColor;
	Vertex[1].dwColor= dwColor;
	Vertex[2].dwColor= dwColor;
	Vertex[3].dwColor= dwColor;
	Vertex[4].dwColor= dwColor;
	Vertex[5].dwColor= dwColor;

	Vertex[0].fTextureU = fU2;
	Vertex[0].fTextureV = fV1;
	Vertex[1].fTextureU = fU1;
	Vertex[1].fTextureV = fV2;
	Vertex[2].fTextureU = fU1;
	Vertex[2].fTextureV = fV1;
	Vertex[3].fTextureU = fU2;
	Vertex[3].fTextureV = fV2;
	Vertex[4].fTextureU = fU1;
	Vertex[4].fTextureV = fV2;
	Vertex[5].fTextureU = fU2;
	Vertex[5].fTextureV = fV1;

//	m_pKernel->DrawCustomMesh(2, Vertex, sizeof(COLORTEXTUREVERTEX));
	pD3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLELIST, 2, Vertex, sizeof(COLORTEXTUREVERTEX));
}

void CFcMinimapObject::DrawUVMesh(LPDIRECT3DDEVICE9 pD3dDevice, D3DXVECTOR3 center, DWORD dwColor, int width, int height, float fU1, float fV1, float fU2, float fV2)
{
	D3DVECTOR			lt,rb;
	COLORTEXTUREVERTEX	Vertex[6];

	lt.x = center.x - width / 2.0f;
	rb.x = center.x + width / 2.0f - 1;

	lt.y = center.y - height / 2.0f;
	rb.y = center.y + height / 2.0f - 1;

	Vertex[0].Vertex=D3DXVECTOR3(rb.x, rb.y, center.z);
	Vertex[1].Vertex=D3DXVECTOR3(lt.x, lt.y, center.z);
	Vertex[2].Vertex=D3DXVECTOR3(lt.x, rb.y, center.z);
	Vertex[3].Vertex=D3DXVECTOR3(rb.x, lt.y, center.z);
	Vertex[4].Vertex=D3DXVECTOR3(lt.x, lt.y, center.z);
	Vertex[5].Vertex=D3DXVECTOR3(rb.x, rb.y, center.z);

	Vertex[0].dwColor= dwColor;
	Vertex[1].dwColor= dwColor;
	Vertex[2].dwColor= dwColor;
	Vertex[3].dwColor= dwColor;
	Vertex[4].dwColor= dwColor;
	Vertex[5].dwColor= dwColor;

	Vertex[0].fTextureU = fU2;
	Vertex[0].fTextureV = fV1;
	Vertex[1].fTextureU = fU1;
	Vertex[1].fTextureV = fV2;
	Vertex[2].fTextureU = fU1;
	Vertex[2].fTextureV = fV1;
	Vertex[3].fTextureU = fU2;
	Vertex[3].fTextureV = fV2;
	Vertex[4].fTextureU = fU1;
	Vertex[4].fTextureV = fV2;
	Vertex[5].fTextureU = fU2;
	Vertex[5].fTextureV = fV1;

//	m_pKernel->DrawCustomMesh(2, Vertex, sizeof(COLORTEXTUREVERTEX));
	pD3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLELIST, 2, Vertex, sizeof(COLORTEXTUREVERTEX));
}

void CFcMinimapObject::DrawUVMesh(LPDIRECT3DDEVICE9 pD3dDevice, D3DXVECTOR3* pLT, D3DXVECTOR3 *pRB, DWORD dwColorLT, DWORD dwColorRT, DWORD dwColorLB, DWORD dwColorRB, float fU1, float fV1, float fU2, float fV2)
{
	COLORTEXTUREVERTEX	Vertex[6];

	Vertex[0].Vertex=D3DXVECTOR3(pRB->x, pRB->y, pLT->z);
	Vertex[1].Vertex=D3DXVECTOR3(pLT->x, pLT->y, pLT->z);
	Vertex[2].Vertex=D3DXVECTOR3(pLT->x, pRB->y, pLT->z);
	Vertex[3].Vertex=D3DXVECTOR3(pRB->x, pLT->y, pLT->z);
	Vertex[4].Vertex=D3DXVECTOR3(pLT->x, pLT->y, pLT->z);
	Vertex[5].Vertex=D3DXVECTOR3(pRB->x, pRB->y, pLT->z);

	Vertex[0].dwColor= dwColorRB;
	Vertex[1].dwColor= dwColorLT;
	Vertex[2].dwColor= dwColorLB;
	Vertex[3].dwColor= dwColorRT;
	Vertex[4].dwColor= dwColorLT;
	Vertex[5].dwColor= dwColorRB;

	Vertex[0].fTextureU = fU2;
	Vertex[0].fTextureV = fV1;
	Vertex[1].fTextureU = fU1;
	Vertex[1].fTextureV = fV2;
	Vertex[2].fTextureU = fU1;
	Vertex[2].fTextureV = fV1;
	Vertex[3].fTextureU = fU2;
	Vertex[3].fTextureV = fV2;
	Vertex[4].fTextureU = fU1;
	Vertex[4].fTextureV = fV2;
	Vertex[5].fTextureU = fU2;
	Vertex[5].fTextureV = fV1;

//	m_pKernel->DrawCustomMesh(2, Vertex, sizeof(COLORTEXTUREVERTEX));
	pD3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLELIST, 2, Vertex, sizeof(COLORTEXTUREVERTEX));
}
