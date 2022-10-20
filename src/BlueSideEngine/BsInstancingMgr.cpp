#include "stdafx.h"
#include "BsImplMaterial.h"
#include "BsShadowMgr.h"
#include "BsInstancingMgr.h"



CBsInstancingMgr::CBsInstancingMgr()
{
	m_bEnable = true;
	m_nInstanceCount = 0;	
	m_nTableCount = 0;
	m_nBufferSize = 5;

#ifdef _XBOX
	const char *szInstancingMaterialName[INSTANCING_MATERIAL_COUNT] = 
	{"Diffuse_instancing.fx", "Ambient_instancing.fx", "Specular_instancing.fx", 
	"Reflect_instancing.fx", "Reflect_morph_instancing.fx", "Shadow_instancing.fx", 
	"Reflect_Specular_instancing.fx"};
#else
	const char *szInstancingMaterialName[INSTANCING_MATERIAL_COUNT] = 
	{"Diffuse_instancing_pc.fx", "Ambient_instancing_pc.fx", "Specular_instancing_pc.fx",
	"Reflect_instancing_pc.fx", "Reflect_morph_instancing_pc.fx", "Shadow_instancing_pc.fx",
	"Reflect_Specular_instancing_pc.fx"};
#endif
	const char *szOriginalMaterialName[INSTANCING_MATERIAL_COUNT] = 
	{"Diffuse.fx", "Ambient.fx", "Specular.fx", 
	"Reflect.fx", "Reflect_morph.fx", "Shadow.fx",
	"Reflect_Specular.fx"};

	m_nCurrentUsingBufferIndex = INSTANCING_NONE;
	m_pInstancingMatrix = NULL;

#ifndef _XBOX
	D3DCAPS9 Caps;
	g_BsKernel.GetD3DDevice()->GetDeviceCaps( &Caps );
	if( Caps.VertexShaderVersion >= D3DVS_VERSION(3,0) ) {
		m_bEnable = true;
	}
	else {
		m_bEnable = false;
	}
	m_bEnable = false;
#endif

	if( m_bEnable ) {
		for( int i = 0; i < INSTANCING_VERTEXBUFFER_COUNT; i++) {
			g_BsKernel.CreateVertexBuffer( MAX_INSTANCING_COUNT * sizeof(D3DXMATRIX), D3DUSAGE_WRITEONLY, NULL, D3DPOOL_MANAGED, &m_DeviceDataSet[ i ].m_pVB);
		}
		m_pInstancingMatrix = new D3DXMATRIX[ MAX_INSTANCING_COUNT ];
	}

	for( int i = 0; i < INSTANCING_MATERIAL_COUNT; i++) {
		m_nInstancingMaterial[i] = -1;
		m_nOriginMaterial[i] = -1;
		if( m_bEnable ) {	
			char fullName[_MAX_PATH];
			strcpy(fullName, g_BsKernel.GetShaderDirectory());
			strcat(fullName, szInstancingMaterialName[i]);
			int nMaterialIndex = g_BsKernel.LoadMaterial(fullName);
			BsAssert( nMaterialIndex != -1  && "Instancing Shader Compile Error!!" );
			m_nInstancingMaterial[i] = nMaterialIndex;
			m_pMaterial[i] = g_BsKernel.GetMaterialPtr( nMaterialIndex );
#ifdef _XBOX
			m_hVertsPerInstance[i] = m_pMaterial[i]->GetParameterByName("VERTS_PER_INSTANCE");
#endif
			strcpy(fullName, g_BsKernel.GetShaderDirectory());
			strcat(fullName, szOriginalMaterialName[i]);
			nMaterialIndex = g_BsKernel.LoadMaterial(fullName);
			BsAssert( nMaterialIndex != -1  && "Shader Compile Error!!" );
			m_pOriginalMaterial[i] = g_BsKernel.GetMaterialPtr( nMaterialIndex );
			m_nOriginMaterial[i] = nMaterialIndex;
		}
	}
}

CBsInstancingMgr::~CBsInstancingMgr()
{
	if( m_pInstancingMatrix ) {
		delete [] m_pInstancingMatrix;
		m_pInstancingMatrix = NULL;
	}
	for( int i = 0; i < INSTANCING_MATERIAL_COUNT; i++) {	
		if( m_nInstancingMaterial[i] != -1){
			g_BsKernel.ReleaseMaterial( m_nInstancingMaterial[i] );
			m_nInstancingMaterial[i] = -1;
		}
		if( m_nOriginMaterial[i] != -1){
			g_BsKernel.ReleaseMaterial( m_nOriginMaterial[i] );
			m_nOriginMaterial[i] = -1;
		}
	}
}

bool CBsInstancingMgr::AddInstancingObject( int nSkinIndex, CBsSubMesh *pSubMesh, int nLodLevel, D3DXMATRIX *pMatrix, CBsImplMaterial *pImplMaterial , D3DXMATRIX *pBoneMatrix )
{
	if( !m_bEnable ) return false;
	if( m_nTableCount >= MAX_INSTANCING_TABLE_COUNT ) return false;
	if( m_nInstanceCount >= MAX_INSTANCING_COUNT ) return false;

	int i;

	int nMaterialIndex = -1;	
	if( pImplMaterial == NULL ) {		// Shadow 인 경우 NULL
		nMaterialIndex = SHADOW_MATERIAL_INDEX;	// shadow material index
	}
	else {
		for( i = 0; i < INSTANCING_MATERIAL_COUNT; i++) {
			if( m_nOriginMaterial[i] == pImplMaterial->m_nMaterialIndex ) {
				nMaterialIndex = i;
				break;
			}
		}
		if( nMaterialIndex == -1) return false;
	}

	// 뒤부터 찾는게 빠르다
	for( i = m_nTableCount-1; i >= 0; --i ) {
		InstancingSet *pInstance = &m_InstancingTable[ i ];
		if( pInstance->pSubMesh == pSubMesh && pInstance->nSkinIndex == nSkinIndex && pInstance->nLodLevel == nLodLevel ) { 
			if( pImplMaterial && nMaterialIndex == MORPHING_MATERIAL_INDEX ) { // 모핑의 경우 _44 에 morph_weight 를 넣어준다.				
				pMatrix->_44 = *(float*)pImplMaterial->m_EditableParameters[0]->m_pValue;				
			}
			pInstance->matObjectList.push_back( pMatrix );						
			break;
		}
	}

	// 못찾은 경우
	if( i == -1 ) {
		InstancingSet *pInstance = &m_InstancingTable[ m_nTableCount ];
		pInstance->nSkinIndex = nSkinIndex;
		pInstance->pSubMesh = pSubMesh;
		pInstance->nLodLevel = nLodLevel;
		if( pImplMaterial && nMaterialIndex == MORPHING_MATERIAL_INDEX ) { // 모핑의 경우 _44 에 morph_weight 를 넣어준다.			
			pMatrix->_44 = *(float*)pImplMaterial->m_EditableParameters[0]->m_pValue;
		}
		pInstance->matObjectList.push_back( pMatrix );
		pInstance->pImplMaterial = pImplMaterial;
		pInstance->nMaterialIndex = nMaterialIndex;		
        m_nTableCount++;
	}
	m_nInstanceCount++;
	return true;
}

void CBsInstancingMgr::RenderInstancingObjectList( C3DDevice *pDevice )
{
	if( m_nInstanceCount == 0 ) return;

	int i, j;
	int nIndex = 0;	

	for(i = 0; i < m_nTableCount; i++) {
		std::vector< D3DXMATRIX* > &matObjectList = m_InstancingTable[ i ].matObjectList;
		int nCount = matObjectList.size();
		for(j = 0; j < nCount; j++) { 
			m_pInstancingMatrix[nIndex + j] = *matObjectList[j];
		}
		nIndex += nCount;
	}

	D3DXMATRIX *pMatrixPtr;

	
	DWORD dwLockFlag = D3DLOCK_NOOVERWRITE;

	int &nStartBufferIndex = m_DeviceDataSet[ m_nCurrentUsingBufferIndex ].m_nStartBufferIndex;

	if( nStartBufferIndex + m_nInstanceCount >  MAX_INSTANCING_COUNT ) {
		nStartBufferIndex = 0;
		dwLockFlag = 0;		
	}

	m_DeviceDataSet[ m_nCurrentUsingBufferIndex ].m_pVB->Lock( nStartBufferIndex * sizeof(D3DXMATRIX), m_nInstanceCount * sizeof(D3DXMATRIX), (void **)&pMatrixPtr, dwLockFlag);
	memcpy(pMatrixPtr, m_pInstancingMatrix, sizeof(D3DXMATRIX) * m_nInstanceCount);
	m_DeviceDataSet[ m_nCurrentUsingBufferIndex ].m_pVB->Unlock();


	CBsShadowMgr *pShadowMgr = g_BsKernel.GetShadowMgr();

	nIndex = nStartBufferIndex;
	for(i = 0; i < m_nTableCount; i++) {

		CBsSubMesh *pSubMesh = m_InstancingTable[ i ].pSubMesh;
		 int nCurLOD = m_InstancingTable[ i ].nLodLevel;
		 CBsSubMesh::g_nLODLevel_ = nCurLOD;

		std::vector< D3DXMATRIX* > &matObjectList = m_InstancingTable[ i ].matObjectList;

		CBsImplMaterial *pImplMaterial = m_InstancingTable[i].pImplMaterial;
		
		int nInstanceCount = matObjectList.size();

		if( pImplMaterial == NULL ) { // Shadow 인 경우
			if( nInstanceCount == 1) {
				int nMaterialIndex = m_InstancingTable[ i ].nMaterialIndex;
				CBsMaterial *pMaterial = m_pOriginalMaterial[nMaterialIndex];
				pShadowMgr->SetVertexDeclaration(0);
				pMaterial->Hold(NULL, 0);
				pMaterial->BeginMaterial(0, D3DXFX_DONOTSAVESTATE);
				pSubMesh->PrepareRender( pDevice, pMaterial, matObjectList[0], NULL );
				pSubMesh->RenderShadow( pDevice, pMaterial);
				pSubMesh->EndRender( pDevice, pMaterial);				
				pMaterial->Free();
			}
			else  {
				int nMaterialIndex = m_InstancingTable[ i ].nMaterialIndex;
				CBsMaterial *pMaterial = m_pMaterial[nMaterialIndex];
				pShadowMgr->SetVertexDeclarationInstancing();
				pMaterial->Hold(NULL, 0);
				pMaterial->BeginMaterial(0, D3DXFX_DONOTSAVESTATE);
				D3DXMATRIX matIdent;
				D3DXMatrixIdentity(&matIdent);
				pSubMesh->PrepareRender( pDevice, pMaterial, &matIdent, NULL );
#ifdef _XBOX
				pMaterial->SetInt( m_hVertsPerInstance[nMaterialIndex], pSubMesh->GetVertexCount( nCurLOD ));
#endif
				pSubMesh->RenderInstancing(pDevice, pMaterial, NULL, 1, m_DeviceDataSet[ m_nCurrentUsingBufferIndex ].m_pVB,  nIndex , nInstanceCount );
				pSubMesh->EndRender( pDevice, pMaterial);				
				pMaterial->Free();
			}
		}
		else if( nInstanceCount == 1 ) {		// 1개인 경우 일반 렌더링

			pImplMaterial->HoldMaterial(pDevice, (NULL), 0, nCurLOD); // Material Lock!!
			pImplMaterial->SetCustomParameter();

			if(pImplMaterial->m_bTwoSideEnable) {
				pDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
			}
			if(!pImplMaterial->m_bDepthCehckEnable ) {
				pDevice->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);
				pDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
			}
			
			matObjectList[0]->_44 = 1.f;
			pSubMesh->PrepareRender( pDevice, pImplMaterial->m_pMaterial, matObjectList[0], NULL );
			pSubMesh->Render( pDevice, pImplMaterial->m_pMaterial, pImplMaterial->GetStreamMapper(nCurLOD)); // 해당 서브메쉬를 렌더 합니다
			pSubMesh->EndRender( pDevice, pImplMaterial->m_pMaterial);	

			if(!pImplMaterial->m_bDepthCehckEnable) {
				pDevice->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
				pDevice->SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE);
			}
			if(pImplMaterial->m_bTwoSideEnable) {
				pDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
			}
			pImplMaterial->FreeMaterial();								// Material Unlock!!
		}
		else {		// 2개이상은 인스턴싱
			
			int nMaterialIndex = m_InstancingTable[ i ].nMaterialIndex;
			CBsMaterial *pMaterial = m_pMaterial[nMaterialIndex];

			pMaterial->Hold( NULL, 0);
			pMaterial->BeginMaterial(0 , D3DXFX_DONOTSAVESTATE);
			pMaterial->SetSamplers( pImplMaterial->m_pnTextureIndex );

			CBsMaterial *pMaterialBackup = pImplMaterial->m_pMaterial;
			pImplMaterial->m_pMaterial = pMaterial;
			pImplMaterial->SetCustomParameter();
			pImplMaterial->m_pMaterial = pMaterialBackup;

			if(pImplMaterial->m_bTwoSideEnable) {
				pDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
			}
			if(!pImplMaterial->m_bDepthCehckEnable ) {
				pDevice->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);
				pDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);				
			}
			D3DXMATRIX matIdent;
			D3DXMatrixIdentity(&matIdent);
			pSubMesh->PrepareRender( pDevice, pMaterial, &matIdent, NULL );
#ifdef _XBOX
			pMaterial->SetInt( m_hVertsPerInstance[nMaterialIndex], pSubMesh->GetVertexCount( nCurLOD ));
#endif
			int nStreamCount = pImplMaterial->m_pMaterial->GetStreamCount( 0 );
			pSubMesh->RenderInstancing(pDevice, pMaterial, pImplMaterial->GetStreamMapper(nCurLOD), nStreamCount, m_DeviceDataSet[ m_nCurrentUsingBufferIndex ].m_pVB, nIndex, nInstanceCount );

			pSubMesh->EndRender( pDevice, pMaterial);

			if(!pImplMaterial->m_bDepthCehckEnable) {
				pDevice->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
				pDevice->SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE);
			}
			if(pImplMaterial->m_bTwoSideEnable) {
				pDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
			}
			pMaterial->Free();
		}
		nIndex += nInstanceCount;
	}
		
	for( i = 0; i < m_nTableCount; i++) {
		m_InstancingTable[ i ].pSubMesh = NULL;
		m_InstancingTable[ i ].nLodLevel = -1;
		m_InstancingTable[ i ].matObjectList.clear();	
	}

	nStartBufferIndex += m_nInstanceCount;

	m_nTableCount = 0;
	m_nInstanceCount = 0;
}

