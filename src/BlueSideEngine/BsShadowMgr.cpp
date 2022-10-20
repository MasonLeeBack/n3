#include "stdafx.h"
#include "BsKernel.h"
#include "CrossVector.h"
#include "BsLODWorld.h"
#include "BsShadowMgr.h"
#include "Box3.h"
#include "IntLin3Box3.h"
#include "BsInstancingMgr.h"

int CBsShadowMgr::s_ShadowBufferWidth = 1600;
int CBsShadowMgr::s_ShadowBufferHeight = 1600;

CBsShadowMgr::CBsShadowMgr()
{
	for(int i=0;i<BS_MAX_BONELINK_TECHNIQUE;++i) {
		m_nVertexDeclIndices[i] = -1;	
	}
	m_nInstancingVertexDeclIndex = -1;

	m_nShadowMaterialIndex = -1;

	m_vecLightDir = D3DXVECTOR3(1.f, 0.f, 0.f);
}


CBsShadowMgr::~CBsShadowMgr()
{
	SAFE_RELEASE(m_pSMColorSurface);
	SAFE_RELEASE(m_pSMColorTexture);
	SAFE_RELEASE(m_pSMZSurface);

	SAFE_RELEASE_MATERIAL(m_nShadowMaterialIndex);

	for(int i=0 ; i<BS_MAX_BONELINK_TECHNIQUE ; ++i) {
		SAFE_RELEASE_VD(m_nVertexDeclIndices[i]);		
	}
	SAFE_RELEASE_VD(m_nInstancingVertexDeclIndex);
}

void CBsShadowMgr::Create()
{
	m_nShadowTexXSize = s_ShadowBufferWidth;
	m_nShadowTexYSize = s_ShadowBufferHeight;

	m_ShadowViewport.X = 0;
	m_ShadowViewport.Y = 0;
	m_ShadowViewport.Width = m_nShadowTexXSize;
	m_ShadowViewport.Height= m_nShadowTexYSize;
#ifdef INV_Z_TRANSFORM
	m_ShadowViewport.MinZ = 1.f;
	m_ShadowViewport.MaxZ = 0.f;
#else
	m_ShadowViewport.MinZ = 0.f;
	m_ShadowViewport.MaxZ = 1.f;
#endif

	LPDIRECT3DDEVICE9 pD3DDevice = g_BsKernel.GetD3DDevice();
	C3DDevice *pDevice = g_BsKernel.GetDevice();

	//  use R32F & shaders instead of depth textures
	m_pSMZTexture = NULL;
	if(FAILED(pD3DDevice->CreateTexture(m_nShadowTexXSize, m_nShadowTexYSize, 1, D3DUSAGE_RENDERTARGET, D3DFMT_R32F, D3DPOOL_DEFAULT, &m_pSMColorTexture, NULL))) {
		BsAssert(0);
	}
#ifdef _XBOX
	D3DSURFACE_PARAMETERS TileSurfaceParams;
	TileSurfaceParams.Base = 0;
	TileSurfaceParams.HierarchicalZBase = 0;
	// XDK 1640버전부터 유효!!
	#if _XDK_VER>=1640
		TileSurfaceParams.ColorExpBias = 0;
	#endif
	if(FAILED(pD3DDevice->CreateDepthStencilSurface(m_nShadowTexXSize, m_nShadowTexYSize/2, D3DFMT_D24FS8, D3DMULTISAMPLE_NONE, 0, FALSE, &m_pSMZSurface, &TileSurfaceParams))) {
		BsAssert(0);
	}
	TileSurfaceParams.Base += m_pSMZSurface->Size / GPU_EDRAM_TILE_SIZE;
#else
	if(FAILED(pD3DDevice->CreateDepthStencilSurface(m_nShadowTexXSize, m_nShadowTexYSize, D3DFMT_D24S8, D3DMULTISAMPLE_NONE, 0, FALSE, &m_pSMZSurface, NULL))) {
		BsAssert(0);
	}
#endif

	if(!m_pSMColorTexture || !m_pSMZSurface)
	{
		BsAssert(0);
	}

	// Retrieve top-level surfaces of our shadow buffer (need these for use with SetRenderTarget)
#ifdef _XBOX
	HRESULT hr = g_BsKernel.GetDevice()->GetD3DDevice()->CreateRenderTarget( m_nShadowTexXSize, m_nShadowTexXSize/2, D3DFMT_R32F, D3DMULTISAMPLE_NONE, 0, FALSE, &m_pSMColorSurface, &TileSurfaceParams );
#else
	if(FAILED(m_pSMColorTexture->GetSurfaceLevel(0, &m_pSMColorSurface))) {
		BsAssert(0);
	}
	D3DSURFACE_DESC surfacedesc;
	m_pSMColorSurface->GetDesc(&surfacedesc);
	if(!m_pSMColorSurface ) {
		BsAssert(0);
	}
#endif
	// Shadow Material Setting!!
	if(m_nShadowMaterialIndex == -1) {
		char fullName[_MAX_PATH];
		strcpy(fullName, g_BsKernel.GetShaderDirectory());
		strcat(fullName, "Shadow.fx");
		m_nShadowMaterialIndex = g_BsKernel.LoadMaterial(fullName);
	}


	D3DVERTEXELEMENT9 decl0[] =
	{
		{ 0, 0,  D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
		D3DDECL_END()
	};
	m_nVertexDeclIndices[0] = g_BsKernel.LoadVertexDeclaration(decl0);

	D3DVERTEXELEMENT9 decl1[] =
	{
		{ 0, 0,  D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
		{ 1, 0, D3DDECLTYPE_SHORT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BLENDINDICES, 0 },
		D3DDECL_END()
	};
	m_nVertexDeclIndices[1] = g_BsKernel.LoadVertexDeclaration(decl1);

	D3DVERTEXELEMENT9 decl2[] =
	{
		{ 0, 0,  D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
		{ 1, 0, D3DDECLTYPE_SHORT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BLENDINDICES, 0 },
		{ 2, 0, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BLENDWEIGHT, 0 },
		D3DDECL_END()
	};
	m_nVertexDeclIndices[2] = g_BsKernel.LoadVertexDeclaration(decl2);

	D3DVERTEXELEMENT9 decl3[] =
	{
		{ 0, 0,  D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
		{ 1, 0, D3DDECLTYPE_SHORT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BLENDINDICES, 0 },
		{ 2, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BLENDWEIGHT, 0 },
		D3DDECL_END()
	};
	m_nVertexDeclIndices[3] = g_BsKernel.LoadVertexDeclaration(decl3);

	D3DVERTEXELEMENT9 decl4[] =
	{
		{ 0, 0,  D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
		{ 1, 0, D3DDECLTYPE_SHORT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BLENDINDICES, 0 },
		{ 2, 0, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BLENDWEIGHT, 0 },
		D3DDECL_END()
	};
	m_nVertexDeclIndices[4] = g_BsKernel.LoadVertexDeclaration(decl4);

	// instancing vertex declaration

	D3DVERTEXELEMENT9 decl_instancing[] =
	{
		{ 0, 0,  D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
		{ 1, 0,  D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 1 },
		{ 1, 16,  D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 2 },
		{ 1, 32,  D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 3 },
		{ 1, 48,  D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 4 },
		D3DDECL_END()
	};
	m_nInstancingVertexDeclIndex = g_BsKernel.LoadVertexDeclaration(decl_instancing);

}

void CBsShadowMgr::Reload()
{
}

void CBsShadowMgr::Update(CBsCamera* pCamera)
{
}

HRESULT CBsShadowMgr::RenderShadowMap(C3DDevice* pDevice)
{
	CBsCamera* pActiveCamera = g_BsKernel.GetActiveCamera();
	std::vector<int>& RenderObjectList = pActiveCamera->GetShadowObjectList();

	m_ShadowCasterObjects.clear();

	ComputeLightMatrix();

	for(unsigned int i=0; i<RenderObjectList.size(); ++i) {
#ifndef _LTCG
		CBsKernel::s_nTrackingIndex = i;
#endif
		CBsObject* pObject = g_BsKernel.GetEngineObjectPtr(RenderObjectList[i]);
		BsAssert(pObject && "Invalid Object Pointer");
		if(pObject && pObject->GetShadowCastType() == BS_SHADOW_BUFFER) {
			if(pObject->GetDistanceFromCam() < SHADOW_LIMIT_FROMVIEW && IsInLightBox(pObject)) {
				m_ShadowCasterObjects.push_back(pObject);
			}
		}
	}

	D3DRECT TilingRects2X[] = 
	{
		{ 0,   0, 1600,  800 },
		{ 0, 800, 1600, 1600 }
	};

	SetShadowPass(true);

	pDevice->BeginScene();

	D3DVIEWPORT9 saved;
	pDevice->GetViewport(&saved);
#ifdef _XBOX
	D3DVECTOR4 ClearColor = { 0.f, 0.f, 0.f, 0.f };
#endif
	{
		pDevice->SetRenderTarget(0, m_pSMColorSurface);
		pDevice->SetDepthStencilSurface(m_pSMZSurface);

#ifdef _XBOX
		ClearColor.x = 1.f;
		pDevice->BeginTiling( NULL, 2, TilingRects2X, &ClearColor, 0.0f, 0L);
#else
		pDevice->ClearBuffer(0xffff0000);
#endif
		pDevice->SetViewport(&m_ShadowViewport);
		pDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
		pDevice->SetRenderState(D3DRS_ALPHATESTENABLE, false);
		pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, false);
	}

	g_BsKernel.GetInstancingMgr()->SetCurrentUsingBufferIndex( CBsInstancingMgr::INSTANCING_SHADOW );

	int nRenderCount = m_ShadowCasterObjects.size();
	for ( int i=0; i<nRenderCount; i++ ) {
		CBsObject* pObject = m_ShadowCasterObjects[i];
#ifndef _LTCG
		CBsKernel::s_nTrackingIndex = i;
#endif
		if(pObject->GetShadowCastType()==BS_SHADOW_NONE) {
			continue;
		}
		pObject->PreRenderShadow(pDevice);
		pObject->RenderShadow(pDevice);
		pObject->PostRender(pDevice);
	}
	g_BsKernel.GetInstancingMgr()->RenderInstancingObjectList( pDevice );

	g_BsKernel.GetInstancingMgr()->SetCurrentUsingBufferIndex( CBsInstancingMgr::INSTANCING_NONE );

#ifdef _XBOX
	ClearColor.x = 0.f;
	pDevice->EndTiling( D3DRESOLVE_CLEARRENDERTARGET | D3DRESOLVE_CLEARDEPTHSTENCIL, NULL, m_pSMColorTexture, &ClearColor, 0.0f, 0L, NULL );
#endif
	{
		pDevice->SetRenderTarget( 0, pDevice->GetBackBuffer());
		pDevice->SetDepthStencilSurface(pDevice->GetDepthStencilSurface());
		pDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
		pDevice->SetRenderState(D3DRS_ALPHATESTENABLE, true);	
	}
	pDevice->SetViewport(&saved);

	pDevice->EndScene();
	SetShadowPass(false);

	return S_OK;
}

void CBsShadowMgr::ComputeLightMatrix()
{
	D3DXVECTOR3 vecLightTarget, vecLightPos;

	CBsCamera* pCamera = g_BsKernel.GetActiveCamera();
	CBsLODWorld* pWorld = (CBsLODWorld*)g_BsKernel.GetWorld();

	// Light Direction이 xz평면에 너무 가까워질때 Shadow의 맵핑이 잘못 그려지는 것을 막기위해 너무 낮은각은 20도의 각으로 Shadow만 계산된다.
	m_vecLightDir.x = g_BsKernel.GetLightDirection()->x;
	m_vecLightDir.y = g_BsKernel.GetLightDirection()->y;
	m_vecLightDir.z = g_BsKernel.GetLightDirection()->z;

	{	// by jeremy
		m_vecLightDir.y = BsMin(m_vecLightDir.y, -0.7f);
	}
	D3DXVec3Normalize(&m_vecLightDir, &m_vecLightDir);

	CCrossVector cvCamera;
	cvCamera.m_PosVector = *(D3DXVECTOR3*)&(g_BsKernel.GetParamInvViewMatrix()->_41);
	cvCamera.m_ZVector = *(D3DXVECTOR3*)&(g_BsKernel.GetParamInvViewMatrix()->_31);

	cvCamera.UpdateVectors();

	D3DXVECTOR2 vec2Dir(cvCamera.m_ZVector.x, cvCamera.m_ZVector.z);
	D3DXVec2Normalize(&vec2Dir, &vec2Dir);
	float fFov_div_2 = pCamera->GetFOVByRender() * 0.5f;		// 시야벡터에서의 Angle!!
	D3DXVECTOR3 vecDownRay;
	D3DXMATRIX matRotX, matRes;
	D3DXMatrixRotationX(&matRotX, fFov_div_2);
	D3DXMatrixMultiply(&matRes, &matRotX, cvCamera);
	D3DXVec3TransformNormal(&vecDownRay, &D3DXVECTOR3(0.f, 0.f, 1.f), &matRes);
	//	vecRay = cvCamera.m_ZVector;
	D3DXVECTOR3 vecGoingStartPos=cvCamera.m_PosVector;

	const float fWorldXSize = DISTANCE_PER_CELL * pWorld->GetWorldXSize();
	const float fWorldYSize = DISTANCE_PER_CELL * pWorld->GetWorldZSize();
	for(int i=0;i<10;++i) {
		float fHeight = 0.f;
		if ( vecGoingStartPos.x <= 0 || vecGoingStartPos.x >= fWorldXSize || vecGoingStartPos.z <= 0 || vecGoingStartPos.z >= fWorldYSize ) 
			fHeight = 0.0f;
		else 
			fHeight = pWorld->GetLandHeight(vecGoingStartPos.x, vecGoingStartPos.z);
		if(vecGoingStartPos.y<fHeight) {
			// 충돌발생
			vecGoingStartPos.y = fHeight;
			break;
		}
		vecGoingStartPos+=(100.f*vecDownRay);
	}

	// Camera Z-direction
	D3DXVECTOR3 vecUpRay;
	D3DXMatrixRotationX(&matRotX, -fFov_div_2);
	D3DXMatrixMultiply(&matRes, &matRotX, cvCamera);
	D3DXVec3TransformNormal(&vecUpRay, &D3DXVECTOR3(0.f, 0.f, 1.f), &matRes);

	D3DXVECTOR3 vecGoingEndPos = cvCamera.m_PosVector;
	for(int i=0;i<25;++i) {
		float fHeight = 0.f;
		if ( vecGoingEndPos.x <= 0 || vecGoingEndPos.x >= fWorldXSize || vecGoingEndPos.z <= 0 || vecGoingEndPos.z >= fWorldYSize ) 
			fHeight = 0.0f;
		else 
			fHeight = pWorld->GetLandHeight(vecGoingEndPos.x, vecGoingEndPos.z);
		if(vecGoingEndPos.y<fHeight) {
			// 충돌발생
			vecGoingEndPos.y = fHeight;
			break;
		}
		vecGoingEndPos+=(200.f*vecUpRay);
	}

	///////////////////////////////////////////////////////
	D3DXVECTOR3 vecStartToEnd = vecGoingEndPos - vecGoingStartPos;
	float fLength = D3DXVec3Length(&vecStartToEnd);
	float fHalfLength = fLength*0.5f;

	fLength = BsMin(fLength, 4000.f);
	fHalfLength = BsMin(fHalfLength, 2000.f);

	vecGoingStartPos.x += (fHalfLength * vec2Dir.x);
	vecGoingStartPos.z += (fHalfLength * vec2Dir.y);
	vecLightTarget=vecGoingStartPos;
	vecLightPos = vecLightTarget + 6000.f * (-m_vecLightDir);

	// View Mat
	D3DXMATRIX matLightView, matLightProj;
	D3DXMatrixLookAtLH( &matLightView, &vecLightPos, &vecLightTarget,
		(D3DXVECTOR3*)&(g_BsKernel.GetParamInvViewMatrix()->_31) );
	// Proj Mat

	float fNear = 10.f;
	float fFar  = 10000.f;
	float fLeft = fLength + 200.f;
	float fUp   = fLength + 200.f;

	D3DXMatrixOrthoLH( &matLightProj, fLeft, fUp, fNear, fFar);
	// end of code
	D3DXMATRIX matLight;
	D3DXMatrixInverse(&matLight, NULL, &matLightView);

	m_LightBox.E[0] = fLeft*0.5f;
	m_LightBox.E[1] = fUp*0.5f;
	m_LightBox.E[2] = (fFar-fNear)*0.5f; // 익스텐드 벡터
	m_LightBox.A[0] = *((BSVECTOR *)&matLight._11);
	m_LightBox.A[1] = *((BSVECTOR *)&matLight._21);
	m_LightBox.A[2] = *((BSVECTOR *)&matLight._31);
	m_LightBox.C = *(BSVECTOR*)&(vecLightPos + (0.5f*(fFar + fNear)) * (*(D3DXVECTOR3*)&(m_LightBox.A[2])));

	m_LightBox.compute_vertices();


	m_LightViewProj = matLightView * matLightProj;
	m_LightViewProjNoView = *(g_BsKernel.GetParamInvViewMatrix())*matLightView*matLightProj;

	float fOffsetX = 0.5f + (0.5f / (float)(GetShadowMapXSize()));
	float fOffsetY = 0.5f + (0.5f / (float)(GetShadowMapYSize()));
	float fRange = 1.f;
	D3DXMATRIX texScaleBiasMat( 0.5f,     0.0f,     0.0f,         0.0f,
								0.0f,    -0.5f,     0.0f,         0.0f,
								0.0f,     0.0f,     fRange,       0.0f,
								fOffsetX, fOffsetY, 0.0f,         1.0f );
	D3DXMatrixMultiply(&m_LightViewProjForSampling, &m_LightViewProj, &texScaleBiasMat);
	D3DXMatrixMultiply(&m_LightViewProjNoViewForSampling, &m_LightViewProjNoView, &texScaleBiasMat);
}

bool CBsShadowMgr::IsInLightBox(CBsObject* pObject)
{
	if( pObject->GetObjectType() == CBsObject::BS_SIMULATE_OBJECT ) {
        return true;
	}
	else {
		Box3 B;
		if(pObject->GetBox3(B)) {
			B.compute_vertices();
			if(TestIntersection(B, m_LightBox)) {
				return true;
			}
		}
		return false;
	}
}

CBsMaterial* CBsShadowMgr::GetMaterial()
{
	if(m_nShadowMaterialIndex == -1) {
		return NULL;
	}
	else {
		return g_BsKernel.GetMaterialPtr(m_nShadowMaterialIndex);
	}
}

void CBsShadowMgr::SetVertexDeclaration(int nBoneLinkCount)
{
	g_BsKernel.SetVertexDeclaration(m_nVertexDeclIndices[nBoneLinkCount]);
}

void CBsShadowMgr::SetVertexDeclarationInstancing()
{
	g_BsKernel.SetVertexDeclaration(m_nInstancingVertexDeclIndex);
}

