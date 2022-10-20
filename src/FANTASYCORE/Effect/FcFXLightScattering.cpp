#include "stdafx.h"
#include "BsKernel.h"
#include "BsMaterial.h"
#include "FcFxLightScattering.h"
#include "FcWorld.h"
#include "FcHeroObject.h"
#include "FcUtil.h"
#include "IntBox3Frustum.h"

#ifdef _XBOX
#define _OPT_SHINJICH_FCFXLIGHTSCATTERING	1
#endif

//////////////////////////////////////////////////////////////////////////
// 포리아용 Light Ray 테이블 ( 파일로 따로 저장하기 귀찮아서 이렇게 합니다.. (수정시 맥스스크립트(소스세이프에 있음)로 뽑습니다.)
//////////////////////////////////////////////////////////////////////////
BSVECTOR vPholyaLightRay[] = {
	BSVECTOR(1.23208e+006f, 499914.0f, 6176.65f),
		BSVECTOR(1.06009e+006f, 84503.3f, 53335.5f),
		BSVECTOR(1.20153e+006f, 472322.0f, -265178.0f),
		BSVECTOR(1.04406e+006f, 84503.3f, -181361.0f),
		BSVECTOR(1.03905e+006f, 591990.0f, -597742.0f),
		BSVECTOR(939581.0f, 84503.3f, -485294.0f),
		BSVECTOR(789481.0f, 505616.0f, -925265.0f),
		BSVECTOR(721581.0f, 87723.8f, -770394.0f),
		BSVECTOR(175060.0f, 654121.0f, -1.20887e+006f),
		BSVECTOR(183084.0f, -1859.12f, -902165.0f),
		BSVECTOR(-284669.0f, 578904.0f, -1.19004e+006f),
		BSVECTOR(-191938.0f, 87723.8f, -1.03668e+006f),
		BSVECTOR(-1.05567e+006f, 278588.0f, -626304.0f),
		BSVECTOR(-808322.0f, 37271.9f, -536214.0f),
		BSVECTOR(-1.09515e+006f, 513585.0f, -580967.0f),
		BSVECTOR(-908757.0f, 87723.8f, -542898.0f),
		BSVECTOR(-1.23713e+006f, 259903.0f, -119097.0f),
		BSVECTOR(-1.13248e+006f, -14792.0f, -160657.0f),
		BSVECTOR(-916411.0f, 229319.0f, -686362.0f),
		BSVECTOR(-824058.0f, -8154.44f, -676689.0f),
		BSVECTOR(-1.21538e+006f, 243287.0f, 284923.0f),
		BSVECTOR(-960158.0f, 37271.9f, 174688.0f),
		BSVECTOR(-1.22024e+006f, 488724.0f, 247432.0f),
		BSVECTOR(-1.13344e+006f, -14792.0f, 169995.0f),
		BSVECTOR(-1.06242e+006f, 617098.0f, 642441.0f),
		BSVECTOR(-861459.0f, 37271.9f, 463364.0f),
		BSVECTOR(-800286.0f, 305272.0f, 934493.0f),
		BSVECTOR(-791119.0f, -14792.0f, 835261.0f),
		BSVECTOR(-285089.0f, 619941.0f, 1.19827e+006f),
		BSVECTOR(-322717.0f, -14792.0f, 1.10583e+006f),
		BSVECTOR(295555.0f, 334694.0f, 1.20339e+006f),
		BSVECTOR(183773.0f, 37271.9f, 963937.0f),
		BSVECTOR(353919.0f, 296330.0f, 1.18582e+006f),
		BSVECTOR(252350.0f, 87723.8f, 1.0392e+006f),
		BSVECTOR(1.05042e+006f, 470909.0f, 645179.0f),
		BSVECTOR(802788.0f, 37271.9f, 557665.0f),
		BSVECTOR(1.07899e+006f, 433273.0f, 592650.0f),
		BSVECTOR(957027.0f, -44021.8f, 595156.0f),
		BSVECTOR(1.09346e+006f, 1.04348e+006f, 360806.0f),
		BSVECTOR(954012.0f, 335058.0f, 368806.0f),
		BSVECTOR(1.1302e+006f, 1.04348e+006f, 212700.0f),
		BSVECTOR(911219.0f, 447807.0f, 217682.0f),
		BSVECTOR(-1.09908e+006f, 824045.0f, -334716.0f),
		BSVECTOR(-958851.0f, 335058.0f, -344600.0f),
		BSVECTOR(-1.13521e+006f, 1.05287e+006f, -183421.0f),
		BSVECTOR(-914735.0f, 447807.0f, -193857.0f),
		BSVECTOR(-435929.0f, 1.04348e+006f, 1.07794e+006f),
		BSVECTOR(-398372.0f, 447807.0f, 862146.0f),
		BSVECTOR(-574105.0f, 1.04348e+006f, 1.01319e+006f),
		BSVECTOR(-554924.0f, 245205.0f, 874837.0f),
		BSVECTOR(391771.0f, 1.05287e+006f, -1.06774e+006f),
		BSVECTOR(359275.0f, 447807.0f, -849415.0f),


};

CFcFXLightScattering::CFcFXLightScattering()
{
	m_DeviceDataState = FXDS_INITREADY;	
	m_bPholyaEnable = false;

	m_pVB = NULL;
	m_nTextureIndex[0] = -1;
	m_nTextureIndex[1] = -1;
	m_nTextureIndex[2] = -1;
	m_nTextureIndex[3] = -1;
	m_nMaterialIndex = -1;
	m_nVertexDeclIndex = -1;
	m_bInitialize = false;
#ifndef _LTCG
	SetFxRtti(FX_TYPE_LIGHTSCATTERING);
#endif //_LTCG
}

CFcFXLightScattering::~CFcFXLightScattering()
{
	SAFE_RELEASE_TEXTURE( m_nTextureIndex[0]);
	SAFE_RELEASE_TEXTURE( m_nTextureIndex[1] );
	SAFE_RELEASE_TEXTURE( m_nTextureIndex[2] );
	SAFE_RELEASE_TEXTURE( m_nTextureIndex[3] );
}

void CFcFXLightScattering::Initialize()
{
	CFcFXBase::Initialize();

	m_nTextureIndex[0] = g_BsKernel.LoadTexture("prop\\P_FR_SS00.DDS");
	m_nTextureIndex[1] = g_BsKernel.LoadTexture("prop\\P_FR_SS01.DDS");

	m_nTextureIndex[2] = g_BsKernel.LoadTexture("prop\\pl_sun_m01.dds");
	m_nTextureIndex[3] = g_BsKernel.LoadTexture("prop\\pl_sun_m02.dds");
}


void CFcFXLightScattering::InitDeviceData() 
{
	char fullName[_MAX_PATH];
	strcpy(fullName, g_BsKernel.GetShaderDirectory());
	strcat(fullName, "LightScattering.fx");
	m_nMaterialIndex = g_BsKernel.LoadMaterial(fullName, FALSE);

	CBsMaterial* pMaterial = g_BsKernel.GetMaterialPtr( m_nMaterialIndex );	    

	
#ifdef _XBOX
	m_hTexture = pMaterial->GetParameterByName("TextureSampler");	
	m_hWorld = pMaterial->GetParameterByName("World");		
#else
	m_hTexture = pMaterial->GetParameterByName("diffuseTexture");	
	m_hWorld = pMaterial->GetParameterByName("world");		
#endif
	
	

	D3DVERTEXELEMENT9 decl[] =
	{
		{ 0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
		{ 0, 12, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
		D3DDECL_END()
	};

	m_nVertexDeclIndex = g_BsKernel.LoadVertexDeclaration( decl );

	g_BsKernel.CreateVertexBuffer( 4 * sizeof(LIGHT_SCATTER_VERTEX), D3DUSAGE_WRITEONLY, sizeof(LIGHT_SCATTER_VERTEX), D3DPOOL_MANAGED, &m_pVB);

	LIGHT_SCATTER_VERTEX verts[4] = 
	{ 
		{D3DXVECTOR3( 0,0, -1), D3DXVECTOR2(1,0) },
		{D3DXVECTOR3( 1, 0, -1), D3DXVECTOR2(1,1) },
		{D3DXVECTOR3( 0, 0, 1), D3DXVECTOR2(0,0) },
		{D3DXVECTOR3( 1,0, 1), D3DXVECTOR2(0,1) },
	};

	LIGHT_SCATTER_VERTEX *pLockedVerts;
	m_pVB->Lock(0, 0, (void **)&pLockedVerts, 0);
	memcpy( pLockedVerts, &verts, sizeof(LIGHT_SCATTER_VERTEX) * 4);
	m_pVB->Unlock();
}

void CFcFXLightScattering::ReleaseDeviceData()
{
	SAFE_RELEASE_MATERIAL( m_nMaterialIndex );
	SAFE_RELEASE_VD( m_nVertexDeclIndex );
	SAFE_RELEASE( m_pVB );
	
}

void CFcFXLightScattering::Process()
{
	const Frustum	& frustum = ((CBsCamera*)g_BsKernel.GetEngineObjectPtr(g_BsKernel.GetCameraHandle(0)))->GetFrustumForBig();

	int i, nSize;

	nSize = m_vecLightInfo.size();
	for( i = 0; i < nSize; i++) {

		Box3 Box;

		BSVECTOR	vExt = m_vecLightInfo[i].BoundingBox.GetExtent();
		Box.E[0] = vExt.x;
		Box.E[1] = vExt.y;
		Box.E[2] = vExt.z;

		Box.C = m_vecLightInfo[i].BoundingBox.GetCenter();

		Box.A[0] = BSVECTOR(1,0,0);
		Box.A[1] = BSVECTOR(0,1,0);
		Box.A[2] = BSVECTOR(0,0,1);

		Box.compute_vertices();

		m_vecLightInfo[i].bVisible = TestIntersection(Box, frustum);
	}

}

void CFcFXLightScattering::PreRender()
{
	CFcFXBase::PreRender();
}

void CFcFXLightScattering::Render(C3DDevice *pDevice)
{
	CBsMaterial* pMaterial = g_BsKernel.GetMaterialPtr( m_nMaterialIndex );

	pMaterial->BeginMaterial(0, 0);
	pMaterial->BeginPass(0);
	g_BsKernel.SetVertexDeclaration( m_nVertexDeclIndex);

	BSVECTOR vEyePos;

	LPVOID p = (LPVOID) &g_BsKernel.GetActiveCamera()->GetObjectMatrix()->_41;
#if _OPT_SHINJICH_FCFXLIGHTSCATTERING
	// optimized memory copy - shinjich
	BsAssert( ! ((DWORD) p & 0xf) );
	vEyePos = __lvx( p, 0 );
#else
	memcpy(&vEyePos, p, sizeof(BSVECTOR));
#endif

	int i, j, nSize;
	nSize = m_vecLightInfo.size();

	for( i = 0; i < nSize; i++) {
		
		const LIGHT_RAY_INFO &rayInfo = m_vecLightInfo[i];

		if( !rayInfo.bVisible ) continue;

		for( j = 0; j < rayInfo.nNum; j++ ) {

			pMaterial->SetTexture( m_hTexture, (LPDIRECT3DBASETEXTURE9)g_BsKernel.GetTexturePtr( m_nTextureIndex[ rayInfo.nTexture[j] ] ) );

			BSMATRIX matRay;
			BsMatrixTranslation(&matRay, rayInfo.Position[j].x, rayInfo.Position[j].y, rayInfo.Position[j].z);

			BSMATRIX matScale;
			BsMatrixScaling(&matScale, rayInfo.Scale[j].x, rayInfo.Scale[j].y, rayInfo.Scale[j].z);	

			BSMATRIX matRot;
			BsMatrixIdentity(&matRot);

			BSVECTOR LightRayDir = rayInfo.Direction[j];
#if _OPT_SHINJICH_FCFXLIGHTSCATTERING
			// - shinjich
			matRot.r[0] = LightRayDir;
			matRot._14 = 0.f;
#else
			matRot._11 = LightRayDir.x;
			matRot._12 = LightRayDir.y;
			matRot._13 = LightRayDir.z;
#endif			
			BSVECTOR vEyeToRay;
#if _OPT_SHINJICH_FCFXLIGHTSCATTERING
			// - shinjich
			vEyeToRay = matRay.r[3] - vEyePos;
			//vEyeToRay.w = 0.f;
#else
			vEyeToRay.x = matRay._41 - vEyePos.x;
			vEyeToRay.y = matRay._42 - vEyePos.y;
			vEyeToRay.z = matRay._43 - vEyePos.z;
			//vEyeToRay.w = 0.f;
#endif
			BSVECTOR vEyeProj = ( BsVec3Dot(&vEyeToRay, &LightRayDir) / BsVec3Dot(&LightRayDir, &LightRayDir) ) * LightRayDir;
			BSVECTOR vRayToEyeBillBoard = vEyeToRay - vEyeProj ;
			BsVec3Normalize((BSVECTOR*)&matRot._21, &vRayToEyeBillBoard);			
			BsVec3Cross((BSVECTOR*)&matRot._31, (BSVECTOR*)&matRot._11, (BSVECTOR*)&matRot._21);		

			BSMATRIX mat;
			BsMatrixMultiply(&mat, &matScale, &matRot);
			BsMatrixMultiply(&mat, &mat, &matRay);

			pMaterial->SetMatrix( m_hWorld, (D3DXMATRIX*)&mat );
			pMaterial->CommitChanges();

			if(  fmod( rayInfo.Position[j].x, 1.f) > 0.5f ) {
				pDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
			}
			else {
				pDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ONE);
			}
			pDevice->DrawMeshVB(D3DPT_TRIANGLESTRIP, 2, m_pVB, sizeof( LIGHT_SCATTER_VERTEX ));
		}
	}

	CBsCamera* pActiveCamera = g_BsKernel.GetActiveCamera();
	float fSaveNearZ = pActiveCamera->GetNearZ();
	float fSaveFarZ = pActiveCamera->GetFarZ();

	D3DXMATRIX matProj;

	const int cSize = sizeof( vPholyaLightRay ) / ( sizeof( vPholyaLightRay[0] ) * 2 );

#if _OPT_SHINJICH_FCFXLIGHTSCATTERING
#undef _OPT_SHINJICH_FCFXLIGHTSCATTERING
#define _OPT_SHINJICH_FCFXLIGHTSCATTERING 0
#endif

#if _OPT_SHINJICH_FCFXLIGHTSCATTERING
	// - shinjich
	static const __declspec(align(16)) float vScale[4] = { 0.04f, 0.04f, 0.04f, 0.04f };
#else
	static float fScale = 0.04f;
#endif
	static float fScaleTable[cSize];
	static bool bFirst = true;
	if( bFirst ) {
		for( i = 0; i < cSize; i++) {
			fScaleTable[i] = RandomNumberInRange( 1300.f, 1800.f );
		}
        bFirst = false;
	}

	pDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ONE);

	for( i = 0; i < cSize; i++) {
#if _OPT_SHINJICH_FCFXLIGHTSCATTERING
		// - shinjich
		BSVECTOR Pos = __vmaddfp( vPholyaLightRay[i * 2 + 0], __lvx( vScale, 0 ), vEyePos );
		BSVECTOR Target = __vmaddfp( vPholyaLightRay[i * 2 + 1], __lvx( vScale, 0 ), vEyePos );
#else
		BSVECTOR Pos = vPholyaLightRay[i * 2 + 0] * fScale + vEyePos;		
		BSVECTOR Target = vPholyaLightRay[i * 2 + 1]* fScale + vEyePos;
#endif
		BSVECTOR LightRayDir = Target - Pos;
#if _OPT_SHINJICH_FCFXLIGHTSCATTERING
		// - shinjich
		BSVECTOR vLength = XMVector3Length( LightRayDir);
		LightRayDir *= __vrefp( vLength);
#else
		float fLength = BsVec3Length( &LightRayDir);
		LightRayDir *= (1.f / fLength);
#endif
#if _OPT_SHINJICH_FCFXLIGHTSCATTERING
		// - shinjich
		pMaterial->SetTexture( m_hTexture, (LPDIRECT3DBASETEXTURE9)g_BsKernel.GetTexturePtr( m_nTextureIndex[ (i & 1) + 2] ) );
#else
		pMaterial->SetTexture( m_hTexture, (LPDIRECT3DBASETEXTURE9)g_BsKernel.GetTexturePtr( m_nTextureIndex[ (i % 2) + 2] ) );
#endif
		BSMATRIX matRay;
		BsMatrixTranslation(&matRay, Pos.x, Pos.y, Pos.z);

		BSMATRIX matScale;
#if _OPT_SHINJICH_FCFXLIGHTSCATTERING
		// - shinjich
		BsMatrixScaling(&matScale, vLength.x, 1.f, fScaleTable[i] );
#else
		BsMatrixScaling(&matScale, fLength, 1.f, fScaleTable[i] );
#endif
		BSMATRIX matRot;
		BsMatrixIdentity(&matRot);
		
#if _OPT_SHINJICH_FCFXLIGHTSCATTERING
		// - shinjich
		matRot.r[0] = LightRayDir;
		matRot._14 = 0.f;
#else
		matRot._11 = LightRayDir.x;
		matRot._12 = LightRayDir.y;
		matRot._13 = LightRayDir.z;
#endif
		BSVECTOR vEyeToRay;
#if _OPT_SHINJICH_FCFXLIGHTSCATTERING
		// - shinjich
		vEyeToRay = matRay.r[0] - vEyePos;
		vEyeToRay.w = 0.f;
#else
		vEyeToRay.x = matRay._41 - vEyePos.x;
		vEyeToRay.y = matRay._42 - vEyePos.y;
		vEyeToRay.z = matRay._43 - vEyePos.z;
		//vEyeToRay.w = 0.f;
#endif
#if _OPT_SHINJICH_FCFXLIGHTSCATTERING
		// - shinjich
		BSVECTOR vEyeProj = ( XMVector3Dot(vEyeToRay, LightRayDir) * __vrefp( XMVector3Dot(LightRayDir, LightRayDir) ) ) * LightRayDir;
#else
		BSVECTOR vEyeProj = ( BsVec3Dot(&vEyeToRay, &LightRayDir) / BsVec3Dot(&LightRayDir, &LightRayDir) ) * LightRayDir;
#endif
		BSVECTOR vRayToEyeBillBoard = vEyeToRay - vEyeProj ;
		BsVec3Normalize((BSVECTOR*)&matRot._21, &vRayToEyeBillBoard);			
		BsVec3Cross((BSVECTOR*)&matRot._31, (BSVECTOR*)&matRot._11, (BSVECTOR*)&matRot._21);		

		BSMATRIX mat;
		BsMatrixMultiply(&mat, &matScale, &matRot);
		BsMatrixMultiply(&mat, &mat, &matRay);

		pMaterial->SetMatrix( m_hWorld, (D3DXMATRIX*)&mat );
		pMaterial->CommitChanges();
		pDevice->DrawMeshVB(D3DPT_TRIANGLESTRIP, 2, m_pVB, sizeof( LIGHT_SCATTER_VERTEX ));
	}

	pDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	// render
	pMaterial->EndPass() ;
	pMaterial->EndMaterial();
}

void CFcFXLightScattering::AddRayInfo( int dwParam1, int dwParam2, int dwParam3 )
{
	if( dwParam3 == 2 ) 
		m_bPholyaEnable = true;
	
	if( dwParam1 == 0) 
		return;

	int nRayNum = BsMin(dwParam2, 20);	// 최대개수 제한

	if( dwParam3 ) {
		nRayNum = 1;
	}

	BsAssert( nRayNum >= 0);

	LIGHT_RAY_INFO rayInfo;
	rayInfo.nNum = nRayNum;

	rayInfo.BoundingBox.Reset();

	D3DXVECTOR3 vLightDir = g_FcWorld.GetMapInfo()->GetSetInfo()->crossLight.m_ZVector;

	if( dwParam3 == 1) {
		vLightDir.y = BsMin(vLightDir.y, -2.0f);
		D3DXVec3Normalize(&vLightDir, &vLightDir);

	}

	for( int i = 0; i < nRayNum; i++) {

		if( dwParam3 == 0) {
			rayInfo.nTexture[i] = RandomNumberInRange(0, 999) / 500;
		}
		else if( dwParam3 == 1) {
			rayInfo.nTexture[i] = 2 + RandomNumberInRange(0, 999) / 500;
		}

		rayInfo.Direction[i].x = vLightDir.x + RandomNumberInRange(-0.15f, 0.15f);
		rayInfo.Direction[i].y = vLightDir.y + RandomNumberInRange(-0.15f, 0.15f);
		rayInfo.Direction[i].z = vLightDir.z + RandomNumberInRange(-0.15f, 0.15f);
		BsVec3Normalize(&rayInfo.Direction[i], &rayInfo.Direction[i]);

		const float fRayHeight = 2500.f;
		rayInfo.Scale[i].x = (fRayHeight / fabs(rayInfo.Direction[i].y)) ;
		rayInfo.Scale[i].y = 1.f;
		if( dwParam3 == 0) {
			rayInfo.Scale[i].z = RandomNumberInRange(40.f, 55.f);
		}

		float fAreaRandomRange = 100.f;
		rayInfo.Position[i].x = ((CCrossVector*)dwParam1)->GetPosition().x + RandomNumberInRange(-fAreaRandomRange, fAreaRandomRange);
		if( dwParam3 == 0) {
			fAreaRandomRange = 100.f;
			rayInfo.Position[i].y = ((CCrossVector*)dwParam1)->GetPosition().y + RandomNumberInRange(0.f, 150.f);
		}
		else if( dwParam3 == 1) {
			fAreaRandomRange = 500.f;
			rayInfo.Position[i].y = ((CCrossVector*)dwParam1)->GetPosition().y + 200.f ;
		}
		rayInfo.Position[i].z = ((CCrossVector*)dwParam1)->GetPosition().z + RandomNumberInRange(-fAreaRandomRange, fAreaRandomRange);
		rayInfo.BoundingBox.Merge( rayInfo.Position[i] );
		rayInfo.Position[i].x -= vLightDir.x * ( rayInfo.Scale[i].x);
		rayInfo.Position[i].y -= vLightDir.y * ( rayInfo.Scale[i].x);
		rayInfo.Position[i].z -= vLightDir.z * ( rayInfo.Scale[i].x);
		rayInfo.BoundingBox.Merge( rayInfo.Position[i] );
	}
// [PREFIX:beginmodify] 2006/2/16 junyash PS#5223 reports using uninitialized memory 'rayInfo.bVisible'
	rayInfo.bVisible = false;
// [PREFIX:endmodify] junyash
	m_vecLightInfo.push_back( rayInfo );
}

int CFcFXLightScattering::ProcessMessage(int nCode,DWORD dwParam1, DWORD dwParam2,DWORD dwParam3) 
{
	if ( CFcFXBase::ProcessMessage( nCode, dwParam1, dwParam2, dwParam3) )
		return 1;
	switch(nCode)
	{
	case FX_INIT_OBJECT:
		{			
			if( !m_bInitialize ) {
				Initialize();
				m_bInitialize = true;
			}
			AddRayInfo( dwParam1, dwParam2, dwParam3 );
			return 1;
		}
	case FX_PLAY_OBJECT:
		m_state = PLAY;
		return 1;

	case FX_STOP_OBJECT:
		m_state = READY;
		m_vecLightInfo.clear();
		return 1;

	}

	return  0;
}
