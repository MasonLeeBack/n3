#include "stdafx.h"
#include "BsCommon.h"
#include "BsMesh.h"
#include "BsImageProcess.h"
#include "FcWaveObject.h"
#include "CrossVector.h"
#include "FcBaseObject.h"
#include "FcGameObject.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG



#define		_BEFORE_CAPTURE

// 임시 스태틱

int CFcWaveObject::m_nVertexCount = -1;
int CFcWaveObject::m_nFaceCount = -1;
LPDIRECT3DVERTEXBUFFER9 CFcWaveObject::m_pVB = NULL;
LPDIRECT3DVERTEXBUFFER9 CFcWaveObject::m_pNormalVB = NULL;
LPDIRECT3DINDEXBUFFER9 CFcWaveObject::m_pIB = NULL;


CFcWaveObject::CFcWaveObject(CCrossVector *pCross)
	: CFcGameObject( pCross )
{
	m_pEffect = NULL;
//	m_nVertexCount = -1;
//	m_nFaceCount = -1;
//	m_pVB = NULL;
//	m_pIB = NULL;
	m_nSwizzleTex = -1;
	m_nSplashPtc = -1;
	m_bInit = false;
	m_nVertexDeclIdx = -1;
}
    
CFcWaveObject::~CFcWaveObject()
{
	if(m_pEffect) {
		m_pEffect->Release();
		m_pEffect = NULL;
	}

	SAFE_RELEASE_VD(m_nVertexDeclIdx);

	SAFE_RELEASE_PARTICLE_GROUP( m_nSplashPtc );
}

void CFcWaveObject::Process()
{
	CFcAniObject::Process();
//	CFcWaveObject pObj = (CFcWaveObject*)pThis;

	

	// 점들마다 위치 조절 해야 함
}

void CFcWaveObject::PostProcess()
{
	float fLandHeight = g_BsKernel.GetLandHeight( m_Cross.m_PosVector.x, m_Cross.m_PosVector.z );
	m_Cross.m_PosVector.y=fLandHeight;
}

void CFcWaveObject::Initialize( INITGAMEOBJECTDATA* pData, int nForce, int nTeam, TroopObjHandle hTroop, float fScaleMin, float fScaleMax )
{
	CFcGameObject::Initialize( pData, nForce, nTeam, hTroop, fScaleMin, fScaleMax );
	g_BsKernel.SendMessage( m_nEngineIndex, BS_SHOW_SUBMESH, 0, 0 );
	
	
#ifdef _BEFORE_CAPTURE
	g_BsKernel.SendMessage( m_nEngineIndex, BS_SET_CUSTOM_RENDERER, (DWORD)CustomRender, (DWORD)this );
#else
	g_BsKernel.SendMessage( m_nEngineIndex, BS_SET_CUSTOM_RENDERER2, (DWORD)CustomRender, (DWORD)this );
#endif

	g_BsKernel.chdir("material");
	m_nSwizzleTex = g_BsKernel.LoadVolumeTexture("swizzle_vol.dds");
	g_BsKernel.chdir("..");

	g_BsKernel.chdir("particle");
	m_nSplashPtc = g_BsKernel.LoadParticleGroup(-1,"WaterSplash.bpf");
	g_BsKernel.chdir("..");

	// Material Setting!!
	char fullName[_MAX_PATH];
	strcpy(fullName, g_BsKernel.GetShaderDirectory());
	strcat(fullName, "WaterTest.fx");

	HRESULT hr = g_BsKernel.GetDevice()->CreateEffectFromFile(fullName, &m_pEffect);
	if(hr != S_OK) {
		BsAssert( 0 && "WaterTest shader error." );
	}

	D3DVERTEXELEMENT9 decl[] =
	{
		{ 0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
		{ 1, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0 },
		//		{ 0, 24, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
		D3DDECL_END()
	};
	m_nVertexDeclIdx = g_BsKernel.LoadVertexDeclaration(decl);


	//	BS_USAGE_POSITION = 0,	   
	//	BS_USAGE_NORMAL = 3,	   

	CBsObject* pObj = g_BsKernel.GetEngineObjectPtr( m_nEngineIndex );
	CBsMesh* pMesh = pObj->GetMeshPt();

	pMesh->GetStream( BS_USAGE_POSITION, m_pVB, m_pIB );
	pMesh->GetStream( BS_USAGE_NORMAL, m_pNormalVB, m_pIB );

	D3DVERTEXBUFFER_DESC pVBDesc;
	m_pVB->GetDesc( &pVBDesc );
	int nStride = sizeof(float) * 3;
	m_nVertexCount = pVBDesc.Size / nStride;

	D3DINDEXBUFFER_DESC pIBDesc;
	m_pIB->GetDesc( &pIBDesc );
	nStride = sizeof(short) * 3;
	m_nFaceCount = pIBDesc.Size / nStride;

	m_bInit = true;
}




void CFcWaveObject::CustomRender( void *pThis, C3DDevice *pDevice, D3DXMATRIX *matObject )
{
#ifndef _XBOX
	D3DXMATRIX matWorld;
	D3DXMATRIX matWVP;
	D3DXMATRIX matView;
	D3DXMATRIX matProj;

	UINT nPass, cPasses;

	CFcWaveObject* pObj = (CFcWaveObject *)pThis;
	if( pObj->IsInit() == false )
	{
		return;
	}

	pDevice->GetTransform(D3DTS_PROJECTION,&matProj);
	pDevice->GetTransform(D3DTS_VIEW,&matView);

	pObj->m_pEffect->Begin(&cPasses, 0);

	pDevice->SetRenderState( D3DRS_ALPHATESTENABLE, FALSE );
	pDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
	pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
	pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );

	pDevice->SetRenderState( D3DRS_LIGHTING, TRUE  );

	static	D3DXVECTOR4	vCount(0.0f,1.0f,0.0f, 0.0f);

	for (nPass = 0; nPass < cPasses; nPass++)
	{
		pObj->m_pEffect->BeginPass(nPass);

		D3DXMatrixIdentity(&matWorld);
		D3DXMATRIX matWorldViewProj;
		D3DXMATRIX matWorldView;

		CCrossVector* pCross = pObj->GetCrossVector();

		pObj->m_pEffect->SetMatrix( "worldViewProj", D3DXMatrixMultiply(&matWorldViewProj, *pCross, CBsKernel::GetInstance().GetParamViewProjectionMatrix()) );
		pObj->m_pEffect->SetMatrix( "worldView", D3DXMatrixMultiply(&matWorldView, *pCross, CBsKernel::GetInstance().GetParamViewMatrix()) );

		pObj->m_pEffect->SetFloat( "time", g_BsKernel.GetRenderTime());

#ifdef _XBOX
		pObj->m_pEffect->SetTexture("backBuffer", g_BsKernel.GetImageProcess()->GetBackBufferTexture());
#else
		pObj->m_pEffect->SetTexture("backBuffer", (LPDIRECT3DBASETEXTURE9)g_BsKernel.GetImageProcess()->GetBackBufferTexture());
#endif

		pObj->m_pEffect->CommitChanges();


		pDevice->SetStreamSource( 0, m_pVB, sizeof(float) * 3);
		pDevice->SetStreamSource( 1, m_pNormalVB, sizeof(float) * 3);
		pDevice->SetIndices( m_pIB );
//		g_BsKernel.SetVertexDeclaration(m_nVertexDeclIdx);

		pDevice->DrawIndexedMeshVB(	D3DPT_TRIANGLESTRIP, m_nVertexCount, m_nFaceCount, 0, 0);

		pObj->m_pEffect->EndPass() ;
	}
	pObj->m_pEffect->End();

	pDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
	pDevice->SetRenderState( D3DRS_ALPHATESTENABLE, TRUE );
#endif //_XBOX
 
}