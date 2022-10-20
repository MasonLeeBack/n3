#include "stdafx.h"
#include "BsKernel.h"
#include "BsObject.h"
#include "BsMesh.h"
#include "BsMaterial.h"
#include "BsImplMaterial.h"
#include "BsParticle.h"
#include "BsShadowMgr.h"
#include "BsShadowVolumeMgr.h"
#include "BsInstancingMgr.h"
#include "BsSkin.h"
#include "BsAniObject.h"

#define _OPT_SHINJICH_BSOBJECT_CPP	0

int CBsObject::m_sCurrentProcessBuffer	= 0;
int CBsObject::m_sCurrentRenderBuffer	= 0;
CBsShadowMgr* CBsObject::s_pShadowMgr	= NULL;
CBsShadowVolumeMgr* CBsObject::s_pShadowVolumeMgr	= NULL;
CBsInstancingMgr* CBsObject::s_pInstancingMgr		= NULL;
int			CBsObject::s_nTraceIndex = -1;

CBsCriticalSection	CBsObject::s_csProtectLinkedObject;


CBsObject::CBsObject()
	: m_nSkinIndex(-1)
{
	SetObjectType(BS_STATIC_OBJECT);
	SetKernelPoolIndex(-1);

	m_pMesh = NULL;
	D3DXMatrixIdentity( m_matObject );
	D3DXMatrixIdentity( m_matObject + 1 );	
	m_pImplMaterials = NULL;

	m_bShow = false;
	m_bShowBoundingBox = false;
	m_dwBoundingBoxColor = 0xffffffff;
	m_bShowGrid	= false;
	SetRootObject(true);
	EnableObjectCull(true);
	m_bEnableBillBoard = false;
	m_bEnableInstancing = true;

	EnableLOD(true);

#ifdef _USAGE_TOOL_
	m_pCallBack = NULL;
	m_pCallBackObject = NULL;
#endif

	SetShadowCastType(BS_SHADOW_NONE);
	SetClipType(BS_CLIP_NONE);

	EnableObjectAlphaBlend(false);
	EnableRestoreAlphaBlend(false);
	SetAlphaWeight(1.f);
	
	m_fDistanceFromCamera[0] = 0.f;
	m_fDistanceFromCamera[1] = 0.f;

	m_vecCenter = D3DXVECTOR3(0,0,0);

	//m_nParentIndex = -1;	
}

CBsObject::~CBsObject()
{
	if(m_pImplMaterials) {
		delete[] m_pImplMaterials;
		m_pImplMaterials = NULL;
	}

	// Release any references that we are holding to a linked skin.
	SAFE_RELEASE_SKIN(m_nSkinIndex);

	/*
	if( m_nParentIndex != -1 && g_BsKernel.GetEngineObjectPtr( m_nParentIndex ) != NULL ) {		 
#ifdef _XBOX
#ifdef PROFILE		// Enable in Only Release_i mode
		//_DEBUGBREAK;	// 여기서 멈추면 엔진팀에 문의하세요.
		DebugString("Parent : %d, Child : %d,  Unlink 안하고 Delete 되는 경우 \n", m_nParentIndex, m_nKernelPoolIndex);
		CBsObject *pAniObj = g_BsKernel.GetEngineObjectPtr( m_nParentIndex );
		KERNEL_COMMAND cmd;
		cmd.nBuffer[1] = m_nKernelPoolIndex;
        pAniObj->ReqUnlinkObject(&cmd);
#endif
#endif
	}
	*/
}

void CBsObject::ReInit()
{
	if(!m_pMesh)
		return;
	int nSubMeshCount = m_pMesh->GetSubMeshCount();
	for(int i=0;i<nSubMeshCount;++i) {
		for(int j=0;j<m_pImplMaterials[i].m_nStreamMapperCount;++j) {
            SAFE_RELEASE_VD(m_pImplMaterials[i].m_pStreamMappers[j].m_nVertexDeclIndex);
		}
		m_pMesh->AnalyzeStreamMap( i, &m_pImplMaterials[i]);
	}
}

void CBsObject::LinkObject(int nLinkDummyIndex, int nObjectID)
{
	DebugString("CBsObject(%d) : Request Link Object(%d, %d)\n", GetKernelPoolIndex(), nLinkDummyIndex, nObjectID);

	KERNEL_COMMAND cmd;
	cmd.nCommand = OBJECT_COMMAND_LINKOBJECT;
	cmd.nBuffer[ 0 ] = GetKernelPoolIndex();
	cmd.nBuffer[ 1 ] = nLinkDummyIndex;
	cmd.nBuffer[ 2 ] = nObjectID;
	g_BsKernel.GetPreKernelCommand().push_back(cmd);
}

const D3DXMATRIX* CBsObject::UnlinkObject(int nObjectID, D3DXVECTOR3 *unlinkVelocity)
{
	DebugString("CBsObject(%d) : Request Unlink Object(%d)\n", GetKernelPoolIndex(), nObjectID);

	// 래그돌인경우 Kinematic 으로 세팅되었던 Bone을 다시 Simulation 으로 바꿔줌
	CBsObject* pObject = g_BsKernel.GetEngineObjectPtr( nObjectID );
	if( pObject->IsRagdoll()) {				
		pObject->ProcessRagdollUnlink( unlinkVelocity );
	}
	KERNEL_COMMAND cmd;
	cmd.nCommand = OBJECT_COMMAND_UNLINKOBJECT;
	cmd.nBuffer[ 0 ] = GetKernelPoolIndex();
	cmd.nBuffer[ 1 ] = nObjectID;
	g_BsKernel.GetPreKernelCommand().push_back(cmd);

	return pObject->GetObjectMatrixByProcess();
}

void CBsObject::RequestObjectAlphaBlend(bool bIsEnable)
{
	KERNEL_COMMAND cmd;
	cmd.nCommand = OBJECT_COMMAND_ENABLEALPHABLEND;
	cmd.nBuffer[ 0 ] = GetKernelPoolIndex();
	cmd.nBuffer[ 1 ] = bIsEnable? 1 : 0;

	g_BsKernel.GetPreKernelCommand().push_back(cmd);
}

void CBsObject::RequestRestoreAlphaBlend()
{
	KERNEL_COMMAND cmd;
	cmd.nCommand = OBJECT_COMMAND_RESTOREALPHABLEND;
	cmd.nBuffer[ 0 ] = GetKernelPoolIndex();

	g_BsKernel.GetPreKernelCommand().push_back(cmd);
}

void CBsObject::RequestObjectAlphaWeight(float fAlpha)
{
	KERNEL_COMMAND cmd;
	cmd.nCommand = OBJECT_COMMAND_SET_ALPHAWEIGHT;
	cmd.nBuffer[ 0 ] = GetKernelPoolIndex();
	cmd.fBuffer = fAlpha;

	g_BsKernel.GetPreKernelCommand().push_back(cmd);
}

// Render Thread에서 사용해야합니다. Thread보호 안합니다.
void CBsObject::ObjectAlphaBlend(bool bIsEnable)
{
	EnableObjectAlphaBlend(bIsEnable);
	UINT uiCount = m_LinkedObjectList.size();
	for(UINT ui=0 ; ui<uiCount ; ++ui) {
		CBsObject* pObject = g_BsKernel.GetEngineObjectPtr( m_LinkedObjectList[ui].nLinkObjectHandle );
		pObject->ObjectAlphaBlend(bIsEnable);
	}
}

// Render Thread에서 사용해야합니다. Thread보호 안합니다.
void CBsObject::SetObjectAlphaWeight(float fAlpha)
{
	SetAlphaWeight(fAlpha);

	UINT uiCount = m_LinkedObjectList.size();
	for(UINT ui=0 ; ui<uiCount ; ++ui) {
		CBsObject* pObject = g_BsKernel.GetEngineObjectPtr( m_LinkedObjectList[ui].nLinkObjectHandle );
		pObject->SetObjectAlphaWeight(fAlpha);
	}
}

// Render Thread에서 사용해야합니다. Thread보호 안합니다.
void CBsObject::RestoreAlphaBlend()
{
	EnableRestoreAlphaBlend(true);
	UINT uiCount = m_LinkedObjectList.size();
	for(UINT ui=0 ; ui<uiCount ; ++ui) {
		CBsObject* pObject = g_BsKernel.GetEngineObjectPtr( m_LinkedObjectList[ui].nLinkObjectHandle );
		pObject->RestoreAlphaBlend();
	}
}

void CBsObject::ReqLinkObject(KERNEL_COMMAND* pCmd)
{
	THREAD_AUTOLOCK(&s_csProtectLinkedObject);
	DebugString("CBsObject(%d) : Link Object(%d, %d)\n", GetKernelPoolIndex(), pCmd->nBuffer[1], pCmd->nBuffer[2]);
	BsAssert( pCmd->nBuffer[0]== GetKernelPoolIndex() && "Error : Invalid Link Request!!");

	LINKEDOBJECT temp;
	temp.nLinkDummyIndex = pCmd->nBuffer[1];
	temp.nLinkObjectHandle = pCmd->nBuffer[2];

	CBsObject* pObject = g_BsKernel.GetEngineObjectPtr(pCmd->nBuffer[2]);
	pObject->SetRootObject(false);
	pObject->ResetRenderMatrix();

	//pObject->SetParentIndex( m_nKernelPoolIndex );

	m_LinkedObjectList.push_back(temp);
}

void CBsObject::ReqUnlinkObject(KERNEL_COMMAND* pCmd)
{
	THREAD_AUTOLOCK(&s_csProtectLinkedObject);
	DebugString("CBsObject(%d) : UnLink Object(%d)\n", GetKernelPoolIndex(), pCmd->nBuffer[1]);
	int nObjectID = pCmd->nBuffer[1];
	UINT uiCount = m_LinkedObjectList.size();
	for(UINT ui=0 ; ui<uiCount ; ++ui) {
		if ( m_LinkedObjectList[ui].nLinkObjectHandle == nObjectID) {
			CBsObject* pObject = g_BsKernel.GetEngineObjectPtr( nObjectID );
			//pObject->SetParentIndex( -1 );
			m_LinkedObjectList.erase(m_LinkedObjectList.begin()+ui);
			pObject->SetRootObject(true);
			return;
		}
	}
}

int CBsObject::AttachMesh(CBsMesh* pMesh)  
{
	m_pMesh = pMesh;
	int nSubMeshCount = m_pMesh->GetSubMeshCount();
	m_pImplMaterials = new CBsImplMaterial[nSubMeshCount];
	int nLODCount = pMesh->GetLODCount();

	for (int i=0; i<nSubMeshCount; i++) {
		m_pImplMaterials[i].SetupLOD(nLODCount);
		m_pImplMaterials[i].SetUVAnimation(pMesh->GetSubMeshPt(i)->GetUVAnimationV());
	}

	return nSubMeshCount;
}

int CBsObject::AttachMaterial(int nSubMeshIndex, int nMaterialIndex)
{
	//	material과 mesh의 Stream Map Matching작업 필요 by jeremy
	CBsMaterial* pMaterial = g_BsKernel.GetMaterialPtr(nMaterialIndex);
	CBsSubMesh* pSubMesh = m_pMesh->GetSubMeshPt(nSubMeshIndex);

	int nSamplerCount = m_pImplMaterials[nSubMeshIndex].LinkMaterial(nMaterialIndex, pSubMesh);
	m_pMesh->AnalyzeStreamMap( nSubMeshIndex, &(m_pImplMaterials[nSubMeshIndex]));
	m_pImplMaterials[nSubMeshIndex].SetUVAnimation(m_pMesh->GetSubMeshPt(nSubMeshIndex)->GetUVAnimationV());
	return nSamplerCount;
}

void CBsObject::SetFadeInOut()
{
	const float fLimitRange = 500.f;
	if(GetClipType() == BS_CLIP_NONE)
		return;
	float fLimitDistance = g_BsKernel.GetActiveCamera()->GetClippingDistance(GetClipType());
	if(GetDistanceFromCam() > fLimitDistance) {
		EnableObjectAlphaBlend(true);
		SetObjectAlphaWeight(0.f);
		EnableRestoreAlphaBlend(true);
	}
	else if( GetDistanceFromCam() > fLimitDistance - fLimitRange ) {
		float fAlpha = (fLimitDistance - GetDistanceFromCam() ) / fLimitRange;
		EnableObjectAlphaBlend(true);
		SetObjectAlphaWeight(fAlpha);
		EnableRestoreAlphaBlend(true);
	}
}

void CBsObject::Render(C3DDevice *pDevice)
{
	BOOL  bIsTransparentObject = FALSE;

	if((m_pImplMaterials)&&(m_pMesh)){
		if(!IsEnableObjectAlphaBlend()) {
			int nCurLOD;

			if(IsEnableLOD())
				nCurLOD = m_pMesh->CalculateLODLevel( GetDistanceFromCam() ); // CBsSubMesh::g_nLODLevel 값이 설정됩니다
			else
				nCurLOD = m_pMesh->ForceLODLevel(0);
			int nPointLightNum = g_BsKernel.ApplyPointLight( m_vecCenter, m_pMesh->GetMeshRadius() );

			for(int i=0;i<m_pMesh->GetSubMeshCount();++i) {
				s_nTraceIndex = i;
				if(!m_pMesh->IsShow(i)){
					continue;
				}				
				CBsImplMaterial *pImplMaterial = &m_pImplMaterials[i];

				if(pImplMaterial->m_bAlphaBlendEnable) {
					bIsTransparentObject = TRUE;
					continue;
				}
				
				int nTechIndex = m_pMesh->GetBoneLinkCount(i, nCurLOD);
				if( nTechIndex == 0 && m_bEnableInstancing && nPointLightNum == 0 && m_ShadowCastType != BS_SHADOW_VOLUME )  {	// ShadowVolume 은 VS 정밀도 문제로 instancing 안씀
					if( s_pInstancingMgr->AddInstancingObject( GetSkinIndex(), m_pMesh->GetSubMeshPt( i ), nCurLOD, GetObjectMatrix(), pImplMaterial, GetRenderMatrix(0) ) ) {
						continue;
					}
				}

				pImplMaterial->HoldMaterial(pDevice, this, nTechIndex, nCurLOD); // Material Lock!!
				pImplMaterial->SetCustomParameter();

				if(pImplMaterial->m_bTwoSideEnable) {
					pDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
				}
				if(!pImplMaterial->m_bDepthCehckEnable ) {
					pDevice->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);
					pDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
				}

				if(pImplMaterial->m_bOnlyDepthWrite ) 
				{
					pDevice->SetRenderState( D3DRS_ALPHATESTENABLE, TRUE);
					pDevice->SetRenderState( D3DRS_ALPHAREF, 0xff);
				}

			
				m_pMesh->PrepareRender( i, pDevice, pImplMaterial->m_pMaterial, GetObjectMatrix(), GetRenderMatrix(0) );
				m_pMesh->Render(i, pDevice, pImplMaterial->m_pMaterial, pImplMaterial->GetStreamMapper(nCurLOD)); // 해당 서브메쉬를 렌더 합니다
				m_pMesh->EndRender(i, pDevice, pImplMaterial->m_pMaterial);	

				if(!pImplMaterial->m_bDepthCehckEnable) {
					pDevice->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
					pDevice->SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE);
				}
				if(pImplMaterial->m_bOnlyDepthWrite ) 
				{
					pDevice->SetRenderState( D3DRS_ALPHATESTENABLE, TRUE);
					pDevice->SetRenderState( D3DRS_ALPHAREF, 0x7f);
				}
				if(pImplMaterial->m_bTwoSideEnable) {
					pDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
				}

				pImplMaterial->FreeMaterial();								// Material Unlock!!
			}
		}
		else {
			bIsTransparentObject = TRUE;
		}
	}
#ifdef _USAGE_TOOL_
	if( m_pCallBack ) {
		m_pCallBack( m_pCallBackObject, pDevice, GetObjectMatrix() );
	}
	if(bIsTransparentObject || m_bShowBoundingBox || m_bShowGrid) {
		g_BsKernel.AddTranparentObject(this);
	}
#else
	if(bIsTransparentObject || m_bShowBoundingBox) {
		g_BsKernel.AddTranparentObject(this);
	}
#endif

	UINT uiCount = m_LinkedObjectList.size();
	CBsObject* pObject;
	for(UINT i=0;i<uiCount;i++){
		pObject=g_BsKernel.GetEngineObjectPtr(m_LinkedObjectList[i].nLinkObjectHandle);
		// Linked Object도 UpdateObject()호출의 영향을 받도록 바꿉니다.!!
		if(pObject->IsShow()) {
			pObject->PreRender(pDevice);
			pObject->Render(pDevice);
			pObject->PostRender(pDevice);
		}
	}	
}

void CBsObject::RenderAlpha(C3DDevice *pDevice) //$$ LOD 관련 확인 필요 합니다
{
	if(m_bShowBoundingBox) {
		DrawBoundingBox(pDevice);
	}
	if (m_bShowGrid) {
		DrawGrid(pDevice);
	}

	if((m_pImplMaterials)&&(m_pMesh)) {
		int nCurLOD;
		if(IsEnableLOD())
			nCurLOD = m_pMesh->CalculateLODLevel( GetDistanceFromCam() );	// CBsSubMesh::g_nLODLevel 값이 설정됩니다
		else
			nCurLOD = m_pMesh->ForceLODLevel(0);

		for(int i=0;i<m_pMesh->GetSubMeshCount();++i) {
			s_nTraceIndex = i;
			if(!m_pMesh->IsShow(i)){
				continue;
			}

			CBsImplMaterial *pImplMaterial = &m_pImplMaterials[i];

			if(!pImplMaterial->m_bAlphaBlendEnable) {
				if(!IsEnableObjectAlphaBlend())
                    continue;
			}

			float fSavedAlpha = pImplMaterial->m_fAlpha;
			pImplMaterial->m_fAlpha *= GetAlphaWeight();

			if(!pImplMaterial->m_bDepthCehckEnable) {
				pDevice->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);
				pDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
			}
			
			if(pImplMaterial->m_bTwoSideEnable) {
				pDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
			}

			pImplMaterial->SetAlphaState(pDevice);

			int nTechIndex = m_pMesh->GetBoneLinkCount(i, nCurLOD);

			pImplMaterial->HoldMaterial(pDevice, this, nTechIndex, nCurLOD);		// Material Lock!!
			pImplMaterial->SetCustomParameter();
			
			m_pMesh->PrepareRender(i, pDevice, pImplMaterial->m_pMaterial, GetObjectMatrix(), GetRenderMatrix(0) );
			if(pImplMaterial->m_bPointSprite) {
				// Render() call forPoint Sprite
				pDevice->SetRenderState(D3DRS_POINTSPRITEENABLE, TRUE);
				m_pMesh->RenderPointSprite(i, pDevice, pImplMaterial->m_pMaterial, pImplMaterial->GetStreamMapper(nCurLOD));
				pDevice->SetRenderState(D3DRS_POINTSPRITEENABLE, FALSE);
			}
			else {
				if(pImplMaterial->m_bAlphaAlignEnable)
					m_pMesh->RenderAlphaAlign(i, pDevice, pImplMaterial->m_pMaterial, pImplMaterial->GetStreamMapper(nCurLOD));
				else
					m_pMesh->RenderAlpha(i, pDevice, pImplMaterial->m_pMaterial, pImplMaterial->GetStreamMapper(nCurLOD));
			}
			m_pMesh->EndRender(i, pDevice, pImplMaterial->m_pMaterial);

			pImplMaterial->FreeMaterial();						// Material Unlock!!

			pImplMaterial->RestoreAlphaState(pDevice);
			pImplMaterial->m_fAlpha = fSavedAlpha;			

			if(!pImplMaterial->m_bDepthCehckEnable) {
				pDevice->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
				pDevice->SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE);
			}

			if(pImplMaterial->m_bTwoSideEnable) {
				pDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
			}

		}		// End for()

		if(IsEnableRestoreAlphaBlend()) {
			EnableObjectAlphaBlend(false);
			SetAlphaWeight(1.f);
			EnableRestoreAlphaBlend(false);
		}
	}
}

void CBsObject::RenderShadow(C3DDevice* pDevice) //$$ LOD 관련 확인 필요 합니다
{
	CBsMaterial* pShadowMaterial = s_pShadowMgr->GetMaterial();
	BsAssert( NULL != pShadowMaterial );	// mruete: prefix bug 346: added assert
	// 문제의 소지가 있다. 추후 Submesh도 Material과 Physique 여부에 따라 사전에 정리할 필요가 있다. by jeremy
	int nCurLOD = m_pMesh->ForceLODLevel(4);
	for(int i=0;i<m_pMesh->GetSubMeshCount();++i) {
		s_nTraceIndex = i;
		if(!m_pMesh->IsShow(i)){
			continue;
		}
		int nTechIndex = m_pMesh->GetBoneLinkCount(i, nCurLOD);
		if( nTechIndex == 0 && m_bEnableInstancing )  {					
			if( s_pInstancingMgr->AddInstancingObject( GetSkinIndex(), m_pMesh->GetSubMeshPt( i ), nCurLOD, GetObjectMatrix(), NULL, GetRenderMatrix(0) ) ) {
				continue;
			}
		}
		s_pShadowMgr->SetVertexDeclaration(nTechIndex);
		pShadowMaterial->Hold(this, nTechIndex);
		pShadowMaterial->BeginMaterial(nTechIndex, D3DXFX_DONOTSAVESTATE);
		m_pMesh->PrepareRender(i, pDevice, pShadowMaterial, GetObjectMatrix(), GetRenderMatrix(0) );
		m_pMesh->RenderShadow(i, pDevice, pShadowMaterial);
		m_pMesh->EndRender(i, pDevice, pShadowMaterial);
		pShadowMaterial->Free();
	}

	// 래그돌에 링크된것도 쉐도우 그려줍니다.
	UINT uiCount = m_LinkedObjectList.size();
	CBsObject* pObject;
	for(UINT i=0;i<uiCount;i++){
		pObject=g_BsKernel.GetEngineObjectPtr(m_LinkedObjectList[i].nLinkObjectHandle);
		// Linked Object도 UpdateObject()호출의 영향을 받도록 바꿉니다.!!
		if(pObject->IsShow() && pObject->GetMeshPt()) {
			pObject->PreRenderShadow(pDevice);
			pObject->RenderShadow(pDevice);
			pObject->PostRender(pDevice);
		}
	}	
}

void CBsObject::RenderShadowVolume(C3DDevice* pDevice) //$$ LOD 관련 확인 필요 합니다
{	
	CBsMaterial* pShadowMaterial = s_pShadowVolumeMgr->GetShadowVolumeMaterial();
	BsAssert( NULL != pShadowMaterial );	// mruete: prefix bug 347: added assert

	int nCurLOD = m_pMesh->ForceLODLevel(4);	

	for(int i=0;i<m_pMesh->GetSubMeshCount();++i) {
		s_nTraceIndex = i;
		if(!m_pMesh->IsShow(i)){
			continue;
		}
		int nTechIndex = m_pMesh->GetBoneLinkCount(i, nCurLOD);

		s_pShadowVolumeMgr->SetVertexDeclarationVolume(nTechIndex);

		s_pShadowVolumeMgr->SetShadowVolumeSetting( pDevice );				
		pShadowMaterial->Hold(this, nTechIndex);
		pShadowMaterial->BeginMaterial(nTechIndex, D3DXFX_DONOTSAVESTATE);
		m_pMesh->PrepareRender(i, pDevice, pShadowMaterial, GetObjectMatrix(), GetRenderMatrix(0) );
		m_pMesh->RenderShadowVolume(i, pDevice, pShadowMaterial);
		m_pMesh->EndRender(i, pDevice, pShadowMaterial);
		pShadowMaterial->Free();
		s_pShadowVolumeMgr->RestoreShadowVolumeSetting( pDevice );				
	}
}

void CBsObject::DrawBoundingBox(C3DDevice* pDevice)
{
	Box3 BB;

	if(GetBox3(BB)) {
		BB.compute_vertices();

		static short s_LineIndices[] =
		{
			0, 1,  1, 3,  3, 2,  2, 0,
			4, 5,  5, 7,  7, 6,  6, 4,
			0, 4,  1, 5,  3, 7,  2, 6,
		};
		pDevice->SetTexture( 0, NULL );
		pDevice->SetVertexShader( NULL );
		pDevice->SetPixelShader( NULL );
		
#ifdef _XBOX		
		pDevice->SetDefaultShader( g_BsKernel.GetParamViewProjectionMatrix(), &D3DXVECTOR4(1,1,1,1));
#else
		pDevice->SetFVF(D3DFVF_XYZ);
		pDevice->SetRenderState(D3DRS_LIGHTING,FALSE);
		pDevice->SetRenderState(D3DRS_TEXTUREFACTOR, m_dwBoundingBoxColor);
		pDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
		pDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TFACTOR);
#endif		
		pDevice->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE );

		pDevice->DrawIndexedPrimitiveUP( D3DPT_LINELIST, /*MinIndex*/0, /*NumVertices*/8, /*PrimitiveCount*/12,
			s_LineIndices, D3DFMT_INDEX16, BB.V, sizeof(BSVECTOR) );

		pDevice->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE );

#ifndef _XBOX
		pDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
		pDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
#endif
	}
}

void CBsObject::DrawGrid(C3DDevice* pDevice)
{
#ifndef _XBOX
	Box3 BB2;
	if(GetBox3_grid(BB2)) {  
		BB2.compute_vertices();

		Grid7x7 GG;
		GG.compute_vertices(&BB2.V[0], &BB2.V[1], &BB2.V[4], &BB2.V[5], &BB2.V[0], &BB2.V[4], &BB2.V[1], &BB2.V[5]);

		static short s_LineIndices[28]={-1,};
		if (s_LineIndices[0]==-1) {
			for (int i=0; i<7; i++) {
				s_LineIndices[   i*2  ]=   i;
				s_LineIndices[   i*2+1]= 7+i;
				s_LineIndices[14+i*2  ]=14+i;
				s_LineIndices[14+i*2+1]=21+i;
			}
		}
		LPDIRECT3DDEVICE9 pD3DDevice = pDevice->GetD3DDevice();
		pD3DDevice->SetTexture( 0, NULL );
		pD3DDevice->SetVertexShader( NULL );
		pD3DDevice->SetPixelShader( NULL );
		pD3DDevice->SetFVF(D3DFVF_XYZ);
		pD3DDevice->SetRenderState(D3DRS_LIGHTING,FALSE);
		pD3DDevice->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE );

		pD3DDevice->DrawIndexedPrimitiveUP( D3DPT_LINELIST, /*MinIndex*/0, /*NumVertices*/28, /*PrimitiveCount*/14,
			s_LineIndices, D3DFMT_INDEX16, GG.V, sizeof(D3DXVECTOR3) );

		pD3DDevice->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE );
	}
#endif
}

AABB*	CBsObject::GetBoundingBox()
{
	if(m_pMesh) {
		return m_pMesh->GetBoundingBox();
	}
	else {
		return NULL;
	}
}

bool CBsObject::GetBox3( Box3 & B )
{
	if( m_pMesh ) {	
		AABB* pBB = m_pMesh->GetBoundingBox();

		// 우선은 GetBox3()에서 쓰는 matrix는 process 꺼 쓴다.. 그럴일이 많을거 같아서..
		D3DXMATRIX *pMatrix = GetObjectMatrixByProcess();
		D3DXVECTOR3	tmpVec = *(D3DXVECTOR3*)&(pBB->GetExtent());
		B.E[0] = tmpVec.x;
		B.E[1] = tmpVec.y;
		B.E[2] = tmpVec.z; // 익스텐드 벡터

		B.C = pBB->GetCenter();
		D3DXVec3TransformCoord( (D3DXVECTOR3*)(&B.C), (D3DXVECTOR3*)&B.C, pMatrix );
		m_vecCenter = *(D3DXVECTOR3*)&B.C;

#if _OPT_SHINJICH_BSOBJECT_CPP
		B.A[0] = *((BSVECTOR *)&pMatrix->_11);
		B.A[1] = *((BSVECTOR *)&pMatrix->_21);
		B.A[2] = *((BSVECTOR *)&pMatrix->_31);
#else
		memcpy(&B.A[0], &(pMatrix->_11), sizeof(BSVECTOR));
		memcpy(&B.A[1], &(pMatrix->_21), sizeof(BSVECTOR));
		memcpy(&B.A[2], &(pMatrix->_31), sizeof(BSVECTOR));
#endif
		return true;
	}
	return false;
}

bool CBsObject::GetBox3_grid( Box3 & B )
{
	if( m_pMesh ) {
		D3DXMATRIX *pMatrix;
		D3DXVECTOR3	Vmin, Vmax;
		// AABB BB = *m_pMesh->GetBoundingBox();
		// BB.Vmax.x *= 0.45f;
		AABB BB;
		BB.Vmax.x=20*3;
		BB.Vmin.z=BB.Vmin.x=-BB.Vmax.x;
		BB.Vmax.z		   = BB.Vmax.x;
		BB.Vmin.y=BB.Vmax.y= 0;

#if _OPT_SHINJICH_BSOBJECT_CPP
		// ...should be more check
		XMVECTOR tmpVec = BB.GetExtent();
		*((XMVECTOR*) B.E) = tmpVec;
#else
		D3DXVECTOR3	tmpVec = *(D3DXVECTOR3*)&BB.GetExtent();
		B.E[0] = tmpVec.x;
		B.E[1] = tmpVec.y;
		B.E[2] = tmpVec.z; // 익스텐드 벡터
#endif

		// GetBox3_grid 에서 쓰는 매트릭스는 Process 꺼 쓴다..
		pMatrix = GetObjectMatrixByProcess();
		B.C = BB.GetCenter();
		D3DXVec3TransformCoord( (D3DXVECTOR3*)&B.C, (D3DXVECTOR3*)&B.C, pMatrix );

		B.A[0] = *((BSVECTOR *)&pMatrix->_11);
		B.A[1] = *((BSVECTOR *)&pMatrix->_21);
		B.A[2] = *((BSVECTOR *)&pMatrix->_31);

		return true;
	}
	return false;
}

float CBsObject::GetMeshRadius()
{
	if(m_pMesh)
		return m_pMesh->GetMeshRadius();
	return 0.f;
}

bool CBsObject::InitRender(float fDistanceFromCamera)
{
	SetDistanceFromCam( fDistanceFromCamera );

	//****************************************************************************************************
	D3DXMATRIX matLink;
	{
		THREAD_AUTOLOCK(&s_csProtectLinkedObject);
		UINT uiCount = m_LinkedObjectList.size();
		for(UINT i=0;i<uiCount;i++){
			CBsObject* pObject = g_BsKernel.GetEngineObjectPtr(m_LinkedObjectList[i].nLinkObjectHandle);
			if(pObject->IsShow()) {
				// Object Matrix를 가지고 Link하는 경우입니다.
				D3DXMatrixMultiply( &matLink, m_pMesh->GetLinkDummyMatrix( m_LinkedObjectList[ i ].nLinkDummyIndex ), 
					GetObjectMatrixByProcess() );
				pObject->SetObjectMatrix( &matLink );
				pObject->InitRender( GetDistanceFromCamByProcess() );
			}
		}
	}
	//****************************************************************************************************
	return true;	// TODO : BsObject 는 false return 한다.. 편법.. ^^;
}

void CBsObject::Show( bool bIsShow )
{
	m_bShow = bIsShow;
}

void CBsObject::SetSkinIndex( int nIndex )
{
	if ( nIndex == m_nSkinIndex )
		return;

	if ( nIndex != -1 )
		g_BsKernel.AddSkinRef(nIndex);

	if ( m_nSkinIndex != -1 )
		g_BsKernel.ReleaseSkin(m_nSkinIndex);

	m_nSkinIndex = nIndex;
}

void CBsObject::ProcessLinkedPhysicsObject( DWORD dwParam1, DWORD dwParam2 )
{
	if(!m_pMesh)
		return;
	if(m_pMesh->GetLinkDummyCount()) {
		int i, nSize;
		D3DXMATRIX matLink;
		{
			THREAD_AUTOLOCK(&s_csProtectLinkedObject);
			// Mesh에 Link Dummy가 있는 경우만!!
			nSize = m_LinkedObjectList.size();
			for(i=0;i<nSize;++i) {
	 			CBsObject* pObject = g_BsKernel.GetEngineObjectPtr(m_LinkedObjectList[i].nLinkObjectHandle);
				if(pObject->IsRagdoll()) {
					D3DXMatrixMultiply(&matLink, m_pMesh->GetLinkDummyMatrix(m_LinkedObjectList[i].nLinkDummyIndex), GetObjectMatrixByProcess());
					pObject->SetRagdollMatrix(false, &matLink);
				}
			}
		}

		// Link Command 가 불려진직후에도 Ragdoll Matrix 처리를 해주어야 된다.
		std::vector<KERNEL_COMMAND>& kernelCommand = g_BsKernel.GetPreKernelCommand();

		nSize = kernelCommand.size();
		for( i = 0; i < nSize; i++) {
			KERNEL_COMMAND *pCmd = &kernelCommand[i];

			if( pCmd->nCommand == OBJECT_COMMAND_LINKOBJECT &&
				pCmd->nBuffer[0] == GetKernelPoolIndex() ) {		

				int nLinkObjectIndex = pCmd->nBuffer[2];
				int nLinkDummyIndex = pCmd->nBuffer[1];

				CBsObject* pObject = (CBsObject*)g_BsKernel.GetEngineObjectPtr( nLinkObjectIndex );	

				if( pObject->IsRagdoll( ) ) {				
					D3DXMatrixMultiply(&matLink, m_pMesh->GetLinkDummyMatrix( nLinkDummyIndex ), GetObjectMatrixByProcess());
					pObject->SetRagdollMatrix(false, &matLink);
				}					
			}
		}
	}
}

int CBsObject::ProcessMessage(DWORD dwCode, DWORD dwParam1/*=0*/, DWORD dwParam2/*=0*/, DWORD dwParam3/*=0*/)
{
	switch(dwCode){
		case BS_GET_SKIN_INDEX:
			return m_nSkinIndex;
		case BS_GET_OBJECT_MAT:
			return ( int )&m_matObject;
		case BS_REINIT_OBJECT:
			ReInit();
			return 1;
		case BS_GET_SUBMESH_COUNT:
			return m_pMesh->GetSubMeshCount();
		case BS_GET_LOD_COUNT:
			return m_pMesh->GetLODCount();
		case BS_DEC_LOD_LEVEL:
			m_pMesh->DecLODLevel();
			return 1;
		case BS_INC_LOD_LEVEL:
			m_pMesh->IncLODLevel();
			return 1;
		case BS_ENABLE_LOD:
			EnableLOD(dwParam1!=0);
			return 1;
		case BS_GET_MESH_POINTER:
			return (int)m_pMesh;
		case BS_GET_MESH_PRIMITIVE_COUNT_LOD:
			return m_pMesh->GetPrimitiveCountLOD();
		case BS_GET_UV_ANI: 
			BsAssert(dwParam1>=0 && int(dwParam1)<m_pMesh->GetSubMeshCount() && "인덱스 범위 벗어납니다");
			return m_pImplMaterials[dwParam1].GetUVAnimation();	// 대표 값을 리턴 합니다
		case BS_SET_UV_ANI_FRAME:
			BsAssert(dwParam1>=0 && int(dwParam1)<m_pMesh->GetSubMeshCount() && "인덱스 범위 벗어납니다");
			m_pImplMaterials[dwParam1].SetCurrentFrameUVAnimation(*((float *)dwParam2)); // 공용 값을 리턴 합니다
			return 1;
		case BS_GET_BOOKING_COUNT: // 갯수를 얻어 옵니다
			return m_pMesh->GetBookingCount();
		case BS_GET_BOOKING_POSITION: // index에 해당하는 Position을 얻어옵니다
			return (int)(m_pMesh->GetBookingPosition(dwParam1));
		case BS_GET_COLLISION_COUNT: // 갯수를 얻어 옵니다
			return m_pMesh->GetCollisionCount();
		case BS_GET_COLLISION_POSITION: // index에 해당하는 Position을 얻어옵니다
			return (int)(m_pMesh->GetCollisionPosition(dwParam1));
		case BS_GET_LINKDUMMY_COUNT:
			return m_pMesh->GetLinkDummyCount();
		case BS_GET_LINKDUMMY_MATRIX:
			return (int)(m_pMesh->GetLinkDummyMatrix(dwParam1));
		case BS_SHOW_BOUNDINGBOX:
			m_bShowBoundingBox = dwParam1 ? true : false;
			m_dwBoundingBoxColor = dwParam2 ? dwParam2 : 0xffffffff;
			return 1;
		case BS_SHOW_GRID:
			m_bShowGrid = dwParam1 ? true : false;
			return 1;
		case BS_SHOW_SUBMESH:
			m_pMesh->Show(int(dwParam1), (dwParam2!=0));
			return 1;
		case BS_SET_SUBMESH_MATERIAL:
			AttachMaterial(dwParam1, dwParam2); // 서브메쉬 인덱스, 매트리얼 인덱스
			return 1;
		case BS_GET_SUBMESH_NAME:
			return (int)m_pMesh->GetSubMeshName(dwParam1);
		case BS_GET_SUBMESH_BONE_COUNT:
			return m_pMesh->GetSubMeshBoneCount(dwParam1);
		case BS_GET_SUBMESH_BONE_NAME:
			return (int)m_pMesh->GetSubMeshBoneName(dwParam1,dwParam2);
		case BS_GET_SUBMESH_BONE_LINK:
			return (int)m_pMesh->GetBoneLinkCount(dwParam1);
		case BS_GET_SUBMESH_PRIMITIVE_COUNT:
			return m_pMesh->GetPrimitiveCount(dwParam1);
		case BS_GET_SUBMESH_PRIMITIVE_TYPE:
			return m_pMesh->GetPrimitiveType(dwParam1);
		case BS_GET_SUBMESH_MATERIAL_INDEX:
			return m_pImplMaterials[dwParam1].m_nMaterialIndex;
		case BS_GET_SUBMESH_MATERIAL_NAME:
			return (int)m_pImplMaterials[dwParam1].GetMaterialName();
		case BS_GET_SUBMESH_CUSTOM_PARAMETERS:
			return m_pImplMaterials[dwParam1].GetMaterialCustomSemanticsInfo((std::vector<SEM_INFO>*)dwParam2);
		case BS_SET_SAMPLER:
			if( (int)dwParam1 < m_pMesh->GetSubMeshCount() && (int)dwParam2 < m_pImplMaterials[dwParam1].m_nSamplerCount ) {
				m_pImplMaterials[dwParam1].m_pnTextureIndex[dwParam2]=dwParam3;
			}
			return 1;
		case BS_GET_SAMPLER_COUNT:
			return m_pImplMaterials[dwParam1].m_nSamplerCount;
		case BS_GET_SAMPLER_TEXTURE_INDEX:
			return m_pImplMaterials[dwParam1].m_pnTextureIndex[dwParam2];  
		case BS_GET_SAMPLER_TEXTURE_NAME:
			return (int)g_BsKernel.GetTextureName(m_pImplMaterials[dwParam1].m_pnTextureIndex[dwParam2]);
		case BS_GET_SAMPLER_SEMANTIC_NAME: // 해당 서브 메쉬의 해당 샘플러의 텍스쳐 타입 문자열 (fx 참조)
			return (int)(m_pImplMaterials[dwParam1].m_pMaterial->GetSamplerSemanticName(dwParam2)); // 서브메쉬 인덱스, 샘플러 인덱스
		case BS_GET_SAMPLER_TEXTURE_TYPE:
			return m_pImplMaterials[dwParam1].m_pMaterial->GetTextureType(dwParam2);
		case BS_SET_SUBMESH_ALPHABLENDENABLE:
			m_pImplMaterials[dwParam1].m_bAlphaBlendEnable = (dwParam2!=0);
			return 1;
		case BS_SET_SUBMESH_DEPTHCHECKENABLE:
			m_pImplMaterials[dwParam1].m_bDepthCehckEnable = (dwParam2!=0);
			return 1;
		case BS_SET_SUBMESH_ALPHA:
			m_pImplMaterials[dwParam1].m_fAlpha = *(float*)dwParam2;
			return 1;
		case BS_SET_SUBMESH_BLENDOP:
			m_pImplMaterials[dwParam1].m_BlendOp = D3DBLENDOP(dwParam2);
			return 1;
		case BS_SET_SUBMESH_SRCBLEND:
			m_pImplMaterials[dwParam1].m_SrcBlend = D3DBLEND(dwParam2);
			return 1;
		case BS_SET_SUBMESH_DESTBLEND:
			m_pImplMaterials[dwParam1].m_DestBlend = D3DBLEND(dwParam2);
			return 1;
		case BS_GET_SUBMESH_ALPHABLENDENABLE:
			return m_pImplMaterials[dwParam1].m_bAlphaBlendEnable;
		case BS_GET_SUBMESH_ALPHA:
			*(float*)dwParam2 = m_pImplMaterials[dwParam1].m_fAlpha;
			return 1;
		case BS_SET_SUBMESH_TWOSIDEENABLE:
			m_pImplMaterials[dwParam1].m_bTwoSideEnable = (dwParam2!=0);
			return 1;
		case BS_GET_SUBMESH_TWOSIDEENABLE:
			return m_pImplMaterials[dwParam1].m_bTwoSideEnable;
		case BS_SET_SUBMESH_ALPHA_ALIGN_ENABLE:
			m_pImplMaterials[dwParam1].m_bAlphaAlignEnable = (dwParam2!=0);
			return 1;
		case BS_GET_SUBMESH_ALPHA_ALIGN_ENABLE:
			return m_pImplMaterials[dwParam1].m_bAlphaAlignEnable;
		case BS_GET_BOUNDING_BOX:
			return (int)m_pMesh->GetBoundingBox();
		case BS_SET_BOUNDING_BOX_BM:
			m_pMesh->SetBoundingBox();
			return 1;
		case BS_WRITE_BOUNDING_BOX:
			m_pMesh->WriteBoundingBox((char *)dwParam1);
			return 1;
		case BS_ADD_EDITABLE_PARAMETER:
			return m_pImplMaterials[dwParam1].AddEditableParameter((PARAMETER_TYPE)dwParam2);
		case BS_SET_EDITABLE_PARAMETER:
			m_pImplMaterials[dwParam1].SetEditableParameter(dwParam2, (void*)dwParam3);
			return 1;
		case BS_GET_EDITABLE_PARAMETER:
			m_pImplMaterials[dwParam1].GetEditableParameter(dwParam2, (void*)dwParam3);
			return 1;
		case BS_SET_CUSTOM_RENDERER:
			{
#ifdef _USAGE_TOOL_
				m_pCallBack=(CustomRenderCallBackPtr)dwParam1;
				m_pCallBackObject=(void *)dwParam2;
#endif
			}
			return 1;
		case BS_ENABLE_OBJECT_ALPHABLEND:
			RequestObjectAlphaBlend(dwParam1 != 0);
			return 1;
		case BS_SET_OBJECT_ALPHA:
			RequestObjectAlphaWeight(*((float*)dwParam1));
			return 1;
		case BS_RESTORE_OBJECT_ALPHABLEND:
			RequestRestoreAlphaBlend();
			return 1;
		case BS_LINKOBJECT:
			LinkObject(dwParam1, dwParam2);
			return 1;
		case BS_UNLINKOBJECT:
			return (int)UnlinkObject(dwParam1, (D3DXVECTOR3*)dwParam2);
		case BS_SHADOW_CAST:
			{
				SetShadowCastType((BS_SHADOW_TYPE)dwParam1);
				if( (BS_SHADOW_TYPE)dwParam1 == BS_SHADOW_VOLUME && m_pMesh ) {
					int nLodLevel = BsMin( 4, m_pMesh->GetLODCount() - 1);
					for( int i = 0; i < m_pMesh->GetSubMeshCount(); i++) {
						m_pMesh->GetSubMeshPt(i)->GetStreamForLOD( nLodLevel )->GenerateShadowVolume();
					}
				}
			}
			return 1;
		case BS_ENABLE_OBJECT_CULL:
			EnableObjectCull( dwParam1 != 0 );
			return 1;
		case BS_ENABLE_BILLBOARD:
			EnableBillBoard( dwParam1 != 0 );
			return 1;
		case BS_SET_POINTSPRITE:
			if(dwParam3 != 0) {
				m_pImplMaterials[dwParam1].m_bOnlyDepthWrite = (dwParam2!=0);
			}
			else {
				m_pImplMaterials[dwParam1].m_bPointSprite = (dwParam2!=0);
			}
			return 1;
		case BS_SET_CLIPPING_TYPE:
			SetClipType(BS_CLIP_TYPE(dwParam1));
			return 1;
	}
	return 0;
}