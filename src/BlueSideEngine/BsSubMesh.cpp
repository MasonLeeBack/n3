#include "stdafx.h"
#include "BStreamExt.h"
#include "BsSubMesh.h"
#include "BsMesh.h"
#include "BsMaterial.h"
#include "BsImplMaterial.h"
#include "BsKernel.h"
#include "BsAni.h"
#include "BsBone.h"
#include "BsStringPool.h"

int CBsSubMesh::g_nLODLevel_=0; 

CBsSubMesh::CBsSubMesh()
{
	m_szParentSubMeshName=NULL;
	m_szSubMeshName=NULL;
	m_pParentBone = NULL;
	m_nParentBoneIndex = -1;
	m_nMaterialIndex=-1;
	m_bIsShow = true;

	m_nUVAnimationV=0;
	m_nTextureAniNum=0;
	m_TextureAni=NULL;

	m_nInstancingVertexDecl = -1;

	for (int i=0; i<m_pStreamLOD_max; i++) m_pStreamBuf[i]=NULL;
	m_nStreamLODAllocated=0; // AddLODStream() 에서 증가 됩니다
}

CBsSubMesh::~CBsSubMesh()
{
	Clear_();
}

void CBsSubMesh::Clear_()
{
	g_BsKernel.FnaPoolClear(m_szParentSubMeshName);
	m_szParentSubMeshName=NULL;	// 여러번 호출 될 경우를 고려 합니다 = assert()

	g_BsKernel.FnaPoolClear(m_szSubMeshName);
	m_szSubMeshName=NULL;

	m_nUVAnimationV=0;
	m_nTextureAniNum=0;
	if (m_TextureAni) {
		delete [] m_TextureAni;
		m_TextureAni=NULL;
	}

	for (int i=0; i<m_nStreamLODAllocated; i++) {
		delete m_pStreamBuf[i];	
		m_pStreamBuf[i]=NULL;
	}

	SAFE_RELEASE_VD(m_nInstancingVertexDecl);

	m_nStreamLODAllocated=0;
}

void CBsSubMesh::LoadMaterialHeader(BStream *pStream, BM_MATERIAL_HEADER *pHeader)
{
	pStream->Read(pHeader->szMaterialName, 256);
	pStream->Read(&(pHeader->fDiffuse), sizeof(float)*13+sizeof(int)*BM_MATERIAL_HEADER::eStage7, ENDIAN_FOUR_BYTE);
	pStream->Read(pHeader->szTextureName, 255*BM_MATERIAL_HEADER::eStage7+BM_MATERIAL_HEADER::eStage7);
}

void CBsSubMesh::LoadSubMesh(BStream *pStream, BM_SUBMESH_HEADER *pHeader)
{
	CBsSubMesh::Clear_(); // CBsPhysiqueSubMesh* 로 호출되는 경우에 문제가 발생 하지 않도록 CBsSubMesh:: 를 지정 합니다	(가상 함수 이므로 주의 합니다)

#ifndef _USAGE_TOOL_
	BM_MATERIAL_HEADER mtr_header; // 매트리얼 툴이 사용 되지 않을때 입니다 = local
#endif

	m_szParentSubMeshName=g_BsKernel.FnaPoolReadAdd(pStream);
	m_szSubMeshName		 =g_BsKernel.FnaPoolReadAdd(pStream);

	int nMaterialCount;

	pStream->Read(&nMaterialCount, sizeof(int), ENDIAN_FOUR_BYTE);

	if (nMaterialCount) { // 언제나 1 입니다
		LoadMaterialHeader(pStream, &mtr_header); // 이전의 메쉬에 있는 Material은 무시한다. by jeremy
		// 툴에서는 사용 될 것입니다
	}
	else {
		pHeader->nTexChannelCount=1; // 헤더 값을 강제로 바꾸어 줍니다
	}
	m_nMaterialIndex=-1;

	//-----------------------

	const int strLenBooking = 7; // "booking"

	if(_strnicmp(m_szSubMeshName,"booking",strLenBooking)==0) { // 박스본 처럼 하지 않은 이유 (포지션 정보를 추가로 얻기 위해서)
		m_bIsShow=false;
		int nDummyNo=atoi(m_szSubMeshName+strLenBooking);
		CBsMesh::s_nDetectBookingIndex=nDummyNo;		
		BsAssert( 0 <= nDummyNo && nDummyNo <= 99 && "Invalid Static Mesh Booking" );
	}
	else {
		CBsMesh::s_nDetectBookingIndex=-1;
	}

	//-----------------------

	const int strLenCollision = 9; // "collision"

	if(_strnicmp(m_szSubMeshName,"collision",strLenCollision)==0) { // 박스본 처럼 하지 않은 이유 (포지션 정보를 추가로 얻기 위해서)
		m_bIsShow=false;
		int nDummyNo=atoi(m_szSubMeshName+strLenCollision);
		CBsMesh::s_nDetectCollisionIndex=nDummyNo;		
		BsAssert( 0 <= nDummyNo && nDummyNo <= 99 && "Invalid Static Mesh Collision" );
	}
	else {
		CBsMesh::s_nDetectCollisionIndex=-1;
	}

	const int strLenLinkDummy = 4;	// String Length of "link"
	if(_strnicmp(m_szSubMeshName,"link",strLenLinkDummy)==0) { // 박스본 처럼 하지 않은 이유 (포지션 정보를 추가로 얻기 위해서)
		m_bIsShow=false;
// [PREFIX:beginmodify] 2006/2/15 junyash PS#5178 reports CBsSubMesh::LoadSubMesh() using uninitialized memory and cut-paste bug. thanks!
		//int nDummyNo=atoi(m_szSubMeshName+strLenCollision);
		int nDummyNo=atoi(m_szSubMeshName+strLenLinkDummy);
// [PREFIX:endmodify] junyash
		CBsMesh::s_nDetectLinkDummyIndex=nDummyNo;		
		BsAssert( 0 <= nDummyNo && nDummyNo <= 99 && "Invalid Static Mesh LinkDummy" );
	}
	else {
		CBsMesh::s_nDetectLinkDummyIndex=-1;
	}


	//-----------------------

	m_nUVAnimationV = nMaterialCount && mtr_header.szMaterialName[255];	// 이 값이 1 일때 추가 데이타를 읽을 것입니다 = 0,1

	// UV 에니메이션을 설정 합니다	  

	if (m_nUVAnimationV) {
		for (int i=0; i<BM_MATERIAL_HEADER::eStage7; i++) {
			if (mtr_header.nTextureType[i]) {
				if (m_nTextureAniNum==0) {
					pStream->Read(&m_nTextureAniNum, sizeof(int), ENDIAN_FOUR_BYTE);  // 텍스쳐 마다 갯수가 다릅니다

					m_TextureAni = new TEXTURE_ANI [m_nTextureAniNum];
					pStream->Read(m_TextureAni, sizeof(TEXTURE_ANI)*m_nTextureAniNum, ENDIAN_FOUR_BYTE);

					// 디버깅
					/*
					char buf[1024];
					for (int i=0; i<m_nTextureAniNum; i++) {
					sprintf(buf, "%3d <P> %5.2f %5.2f %5.2f   <R> %5.2f %5.2f %5.2f %5.2f   <S> %5.2f %5.2f %5.2f\n",
					i, 
					m_TextureAni[i].Position.x, m_TextureAni[i].Position.y, m_TextureAni[i].Position.z, 
					m_TextureAni[i].Rotation.x, m_TextureAni[i].Rotation.y, m_TextureAni[i].Rotation.z, m_TextureAni[i].Rotation.w,
					m_TextureAni[i].Scale.x,    m_TextureAni[i].Scale.y,    m_TextureAni[i].Scale.z);
					DebugString(buf);
					}
					*/
				}
				else { // 나머지 데이타를 읽는 임시 코드 입니다
					int n;
					pStream->Read(&n, sizeof(int), ENDIAN_FOUR_BYTE);  // 텍스쳐 마다 갯수가 다릅니다

					TEXTURE_ANI ani;
					for (int j=0; j<n; j++) {
						pStream->Read(&ani, sizeof(TEXTURE_ANI), ENDIAN_FOUR_BYTE);
					}
				}
			}
			else {
				break;	  
			}
		}
	}


	if (m_pStreamBuf[0]==NULL) {
		m_pStreamBuf[0] = new CBsStream; // 언제나 사용 되므로 메모리 영역을 미리 지정해 놓습니다
	}
	m_pStreamBuf[0]->LoadStream(pStream, pHeader); // 메모리 소거는 내부에서 자동 입니다
	m_nStreamLODAllocated=1; // 1 개가 할당 되었습니다 (이 변수에서 지정된 갯수로 소거 됩니다)

	// 로딩 시간에 영향을 미치므로 Expoter 등에서 미리 계산하도록 추후에 수정합니다..
	/*
	D3DXVECTOR3 *pVertexBuffer;
	int nVertexCount;
	m_pStreamBuf[0]->GetStreamBuffer(&pVertexBuffer, nVertexCount);	
	float fX1,fX2, fY1,fY2, fZ1,fZ2;
	fX1=fX2=pVertexBuffer[0].x;
	fY1=fY2=pVertexBuffer[0].y;
	fZ1=fZ2=pVertexBuffer[0].z;
	for (int i=0; i<nVertexCount; i++) {
		if (pVertexBuffer[i].x < fX1) fX1=pVertexBuffer[i].x;
		if (pVertexBuffer[i].x > fX2) fX2=pVertexBuffer[i].x;

		if (pVertexBuffer[i].y < fY1) fY1=pVertexBuffer[i].y;
		if (pVertexBuffer[i].y > fY2) fY2=pVertexBuffer[i].y;

		if (pVertexBuffer[i].z < fZ1) fZ1=pVertexBuffer[i].z;
		if (pVertexBuffer[i].z > fZ2) fZ2=pVertexBuffer[i].z;
	}
	delete [] pVertexBuffer;
	m_vecSubMeshSize.x=fX2-fX1;
	m_vecSubMeshSize.y=fY2-fY1;
	m_vecSubMeshSize.z=fZ2-fZ1;
	*/

	// LOD 관련 추가 코드 

	// m_nStreamLODAllocated 는 AddLODStream() 에서 증가 됩니다

	//-----------------------

	if (CBsMesh::s_nDetectBookingIndex  !=-1) m_pStreamBuf[0]->GetDummyPosition(CBsMesh::s_vecBooking  );
	if (CBsMesh::s_nDetectCollisionIndex!=-1) m_pStreamBuf[0]->GetDummyPosition(CBsMesh::s_vecCollision);
	if (CBsMesh::s_nDetectLinkDummyIndex!=-1) CBsMesh::s_matLinkDummy = pHeader->matLinkDummy;

	//-----------------------
}

void CBsSubMesh::LinkBone(CBsAni *pAni)
{
	BsAssert(pAni);
	m_pParentBone=pAni->FindBone(m_szParentSubMeshName);
	m_nParentBoneIndex=pAni->FindBoneIndex(m_szParentSubMeshName);
}

void CBsSubMesh::LinkBone(const std::vector< std::string > & szBoneList) // mruete: changed to pass by const reference
{
	int i, nCount;

	nCount = szBoneList.size();
	for( i = 0; i < nCount; i++ )
	{
		if( _stricmp( szBoneList[ i ].c_str(), m_szParentSubMeshName ) == 0 )
		{
			m_nParentBoneIndex = i;
			return;
		}
	}
}

bool CBsSubMesh::AnalyzeStreamMap(CBsImplMaterial* pImplMaterial)
{
	// mruete: prefix bug 469: renamed outer i to iStream, and inner i to iElement
	CBsMaterial* pMaterial = pImplMaterial->m_pMaterial;
	for(int iStream=0;iStream<pImplMaterial->m_nStreamMapperCount;++iStream) {
		CBsStream* pStream = m_pStreamBuf[iStream];
		STREAM_MAPPER* pMapper = &(pImplMaterial->m_pStreamMappers[iStream]);
		{
			int nTechiqueIndex = pMaterial->QueryTechnique(pStream->GetBoneLinkCount());
			std::vector<VERTEX_ELEMENT>& VertexElements = pMaterial->GetVertexElements(nTechiqueIndex);
			int nSize = VertexElements.size();

			D3DVERTEXELEMENT9 *pVertexElements = new D3DVERTEXELEMENT9[nSize+5]; // 실재 사이즈 + 5

			for(int iElement=0;iElement<nSize;++iElement) {
				pMapper->m_pnStreamIndex[iElement] = pStream->FindStreamByUsage(VertexElements[iElement].Usage, VertexElements[iElement].UsageIndex);
				if(pMapper->m_pnStreamIndex[iElement]==-1) {
					// 모든 LOD Stream에 적용!!
					for(int j=0;j<m_nStreamLODAllocated;++j) {
						if(!m_pStreamBuf[j]->GenerateStream(VertexElements[iElement].Usage, VertexElements[iElement].UsageIndex )) {
							pMapper->m_pnStreamIndex[iElement] = -1; // 설정 불가능 입니다
						}
						else {
							pMapper->m_pnStreamIndex[iElement] = m_pStreamBuf[0]->FindStreamByUsage(VertexElements[iElement].Usage, VertexElements[iElement].UsageIndex);
						}
					}
				}
				pVertexElements[iElement].Stream = iElement;
				pVertexElements[iElement].Offset = 0;
				pVertexElements[iElement].Type   = pStream->GetDeclType(pMapper->m_pnStreamIndex[iElement]);
				pVertexElements[iElement].Method = D3DDECLMETHOD_DEFAULT;
				pVertexElements[iElement].Usage  = VertexElements[iElement].Usage;
				pVertexElements[iElement].UsageIndex = VertexElements[iElement].UsageIndex;
			}

			pVertexElements[nSize].Stream = 0xff;
			pVertexElements[nSize].Offset = 0;
			pVertexElements[nSize].Type   = D3DDECLTYPE_UNUSED;
			pVertexElements[nSize].Method = 0;
			pVertexElements[nSize].Usage  = 0;
			pVertexElements[nSize].UsageIndex = 0;

			pMapper->m_nVertexDeclIndex = g_BsKernel.LoadVertexDeclaration(pVertexElements);

			if( nTechiqueIndex == 0 && m_nInstancingVertexDecl == -1 ) {
				int j;
				for( j = nSize; j < nSize + 4; j++) {
					pVertexElements[j].Stream = nSize;
					pVertexElements[j].Offset = (j - nSize) * 16;
					pVertexElements[j].Type   = D3DDECLTYPE_FLOAT4;
					pVertexElements[j].Method = D3DDECLMETHOD_DEFAULT;
					pVertexElements[j].Usage  = D3DDECLUSAGE_TEXCOORD;
					pVertexElements[j].UsageIndex = (j - nSize)+1;
				}
				pVertexElements[j].Stream = 0xff;
				pVertexElements[j].Offset = 0;
				pVertexElements[j].Type   = D3DDECLTYPE_UNUSED;
				pVertexElements[j].Method = 0;
				pVertexElements[j].Usage  = 0;
				pVertexElements[j].UsageIndex = 0;
				m_nInstancingVertexDecl = g_BsKernel.LoadVertexDeclaration(pVertexElements);
			}

			delete [] pVertexElements;	// mruete: prefix bug 470: changed to delete []
		}
	}

	return true;
}

int CBsSubMesh::GetPositionVector(D3DXVECTOR3 &Position, float fFrame)
{
	int i;
	float fWeight;
	int nKeyCount, nRangeStart, nRangeEnd;
	TEXTURE_ANI *pPositionAni;

	nKeyCount=m_nTextureAniNum;
	if(nKeyCount>1){
		pPositionAni=m_TextureAni;
		nRangeStart=0;
		nRangeEnd=nKeyCount-1;
		while(1){
			i=(nRangeStart+nRangeEnd)/2;
			if(pPositionAni[i].nTime==fFrame){
				Position=pPositionAni[i].Position;
				break;
			}
			else if(pPositionAni[i].nTime<fFrame){
				if(pPositionAni[i+1].nTime>=fFrame){
					i++;
				}
				else{
					nRangeStart=i+1;
					continue;
				}
			}
			else{
				if(pPositionAni[i-1].nTime>fFrame){
					nRangeEnd=i-1;
					continue;
				}
			}
			fWeight=((float)(fFrame-pPositionAni[i-1].nTime))/(pPositionAni[i].nTime-pPositionAni[i-1].nTime);
			D3DXVec3Lerp(&Position, &(pPositionAni[i-1].Position), &(pPositionAni[i].Position), fWeight);
			break;
		}
	}
	else if(nKeyCount==1){
		Position=m_TextureAni[0].Position;
	}
	else{
		return 0;
	}

	return 1;
}

int CBsSubMesh::GetRotationVector(D3DXQUATERNION &Rotation, float fFrame)
{
	int i;
	float fWeight;
	int nKeyCount, nCurrentTime, nRangeStart, nRangeEnd;
	TEXTURE_ANI *pRotationAni;

	nKeyCount=m_nTextureAniNum;
	if(nKeyCount>1){
		pRotationAni=m_TextureAni;
		nRangeStart=0;
		nRangeEnd=nKeyCount-1;
		while(1){
			i=(nRangeStart+nRangeEnd)/2;
			nCurrentTime=pRotationAni[i].nTime;
			if(nCurrentTime==fFrame){
				Rotation=pRotationAni[i].Rotation;
				break;
			}
			else if(nCurrentTime<fFrame){
				if(pRotationAni[i+1].nTime>=fFrame){
					i++;
					nCurrentTime=pRotationAni[i].nTime;
				}
				else{
					nRangeStart=i+1;
					continue;
				}
			}
			else{
				if(pRotationAni[i-1].nTime>fFrame){
					nRangeEnd=i-1;
					continue;
				}
			}
			fWeight=((float)(fFrame-pRotationAni[i-1].nTime))/(nCurrentTime-pRotationAni[i-1].nTime);
			D3DXQuaternionSlerp(&Rotation, &(pRotationAni[i-1].Rotation), &(pRotationAni[i].Rotation), fWeight);
			break;
		}
	}
	else if(nKeyCount==1){
		Rotation=m_TextureAni[0].Rotation;
	}
	else{
		return 0;
	}

	return 1;
}

int CBsSubMesh::GetScaleVector(D3DXVECTOR3 &Scale, float fFrame)
{
	int i;
	float fWeight;
	int nKeyCount, nRangeStart, nRangeEnd;
	TEXTURE_ANI *pPositionAni;

	nKeyCount=m_nTextureAniNum;
	if(nKeyCount>1){
		pPositionAni=m_TextureAni;
		nRangeStart=0;
		nRangeEnd=nKeyCount-1;
		while(1){
			i=(nRangeStart+nRangeEnd)/2;
			if(pPositionAni[i].nTime==fFrame){
				Scale=pPositionAni[i].Scale;
				break;
			}
			else if(pPositionAni[i].nTime<fFrame){
				if(pPositionAni[i+1].nTime>=fFrame){
					i++;
				}
				else{
					nRangeStart=i+1;
					continue;
				}
			}
			else{
				if(pPositionAni[i-1].nTime>fFrame){
					nRangeEnd=i-1;
					continue;
				}
			}
			fWeight=((float)(fFrame-pPositionAni[i-1].nTime))/(pPositionAni[i].nTime-pPositionAni[i-1].nTime);
			D3DXVec3Lerp(&Scale, &(pPositionAni[i-1].Scale), &(pPositionAni[i].Scale), fWeight);
			break;
		}
	}
	else if(nKeyCount==1){
		Scale=m_TextureAni[0].Scale;
	}
	else{
		return 0;
	}

	return 1;
}

void CBsSubMesh::PrepareRender(C3DDevice* pDevice, CBsMaterial* pMaterial, D3DXMATRIX* pMatObject, D3DXMATRIX *pBoneMatrix )
{
	//std::vector<EffectParam_Info>& parameters = pMaterial->GetParameters();
	//*******************************************************************************************
	if(m_nParentBoneIndex != -1 && pBoneMatrix != NULL ){
		D3DXMatrixMultiply( pMaterial->GetWorldMatrix(), &pBoneMatrix[m_nParentBoneIndex], pMatObject);		
	}
	else{
		*(pMaterial->GetWorldMatrix()) = *pMatObject;
	}
	//*******************************************************************************************	
	pMaterial->SetParameters();	// Parameter Setting!!!!
}

void CBsSubMesh::Render(C3DDevice* pDevice, CBsMaterial* pMaterial, int* pnStreamIndexList)
{
	pMaterial->BeginPass(0);
	pMaterial->CommitChanges();

	CBsStream *pt = m_pStreamBuf[g_nLODLevel_]; 

	pt->SetStreamSource(pDevice, pMaterial->GetStreamCount(pMaterial->QueryActiveTechnique()), pnStreamIndexList);
	pt->DrawMesh(pDevice);

	pMaterial->EndPass();
}

void CBsSubMesh::RenderShadow(C3DDevice* pDevice, CBsMaterial* pMaterial, int nLevelIndex /*=0*/)
{
	pMaterial->BeginPass(0);
	pMaterial->CommitChanges();

	CBsStream *pt = m_pStreamBuf[g_nLODLevel_]; 

	// 임시로.............추후 교정!! by jeremy
	int nBoneLinkCount = pt->GetBoneLinkCount();
	int nStreamIndex[3];
	switch(nBoneLinkCount) {
		case 0:
			{
				nStreamIndex[0]=pt->FindStreamByUsage(BS_USAGE_POSITION, 0);
				pt->SetStreamSource(pDevice, 1, nStreamIndex);
			}
			break;
		case 1:
			{
				nStreamIndex[0]=pt->FindStreamByUsage(BS_USAGE_POSITION, 0);
				nStreamIndex[1]=pt->FindStreamByUsage(BS_USAGE_BLENDINDICES, 0);
				pt->SetStreamSource(pDevice, 2, nStreamIndex);
			}
			break;
		case 2:
		case 3:
		case 4:
			{
				nStreamIndex[0]=pt->FindStreamByUsage(BS_USAGE_POSITION, 0);
				nStreamIndex[1]=pt->FindStreamByUsage(BS_USAGE_BLENDINDICES, 0);
				nStreamIndex[2]=pt->FindStreamByUsage(BS_USAGE_BLENDWEIGHT, 0);
				pt->SetStreamSource(pDevice, 3, nStreamIndex);
			}
			break;
	}
	pt->DrawMesh(pDevice);

	pMaterial->EndPass();
}

void CBsSubMesh::RenderShadowVolume(C3DDevice* pDevice, CBsMaterial* pMaterial, int nLevelIndex /*=0*/)
{
	pMaterial->BeginPass(0);
	pMaterial->CommitChanges();

	CBsStream *pt = m_pStreamBuf[g_nLODLevel_];

	pt->SetStreamSourceForShadowVolume( pDevice );
	pt->DrawMeshForShadowVolume(pDevice);

	pMaterial->EndPass();
}

void CBsSubMesh::RenderAlpha(C3DDevice* pDevice, CBsMaterial* pMaterial, int* pnStreamIndexList)
{
	CBsStream *pt = m_pStreamBuf[g_nLODLevel_]; 

	pMaterial->BeginPass(0);
	pMaterial->CommitChanges();

	pt->SetStreamSource(pDevice, pMaterial->GetStreamCount(pMaterial->QueryActiveTechnique()), pnStreamIndexList);
	pt->DrawMesh(pDevice);

	pMaterial->EndPass();
}

void CBsSubMesh::RenderAlphaAlign(C3DDevice* pDevice, CBsMaterial* pMaterial, int* pnStreamIndexList)
{
	pMaterial->BeginPass(0);
	pMaterial->CommitChanges();

	CBsStream *pt = m_pStreamBuf[g_nLODLevel_]; 

	pt->SetStreamSource(pDevice, pMaterial->GetStreamCount(pMaterial->QueryActiveTechnique()), pnStreamIndexList);

	pDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE);
	pDevice->SetRenderState( D3DRS_ZWRITEENABLE, true);
	pDevice->SetRenderState( D3DRS_ALPHATESTENABLE, true);
	pDevice->SetRenderState( D3DRS_ALPHAREF, 0xff);
	pt->DrawMesh(pDevice);

	pDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_CW);
	pDevice->SetRenderState( D3DRS_ZWRITEENABLE, false);
#ifdef INV_Z_TRANSFORM
	pDevice->SetRenderState( D3DRS_ZFUNC, D3DCMP_GREATER);
#else
	pDevice->SetRenderState( D3DRS_ZFUNC, D3DCMP_LESS);
#endif
	pDevice->SetRenderState( D3DRS_ALPHATESTENABLE, false);
	pt->DrawMesh(pDevice);

	pDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
	pt->DrawMesh(pDevice);

	// restore
	pDevice->SetRenderState( D3DRS_ZWRITEENABLE, true);
#ifdef INV_Z_TRANSFORM
	pDevice->SetRenderState( D3DRS_ZFUNC, D3DCMP_GREATEREQUAL);
#else
	pDevice->SetRenderState( D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
#endif
	pDevice->SetRenderState( D3DRS_ALPHAREF, 0x7f);
	pMaterial->EndPass();
}

void CBsSubMesh::RenderInstancing(C3DDevice* pDevice, CBsMaterial* pMaterial, int* pnStreamIndexList, int nStreamCount, LPDIRECT3DVERTEXBUFFER9 pVertexBuffer, int nStartIndex, int nInstancingCount)
{
	if( pnStreamIndexList != NULL ) {	// shadow 인경우 skip
		pDevice->SetVertexDeclaration(g_BsKernel.GetBsVertexDeclaration( m_nInstancingVertexDecl )->GetVertexDeclaration());
	}

	pMaterial->BeginPass(0);
	pMaterial->CommitChanges();

	CBsStream *pt = m_pStreamBuf[g_nLODLevel_];

	pt->DrawMeshForInstancing( pDevice , nStreamCount, pnStreamIndexList, pVertexBuffer, nStartIndex, nInstancingCount);

	pMaterial->EndPass();
}

void CBsSubMesh::RenderPointsprite(C3DDevice* pDevice, CBsMaterial* pMaterial, int* pnStreamIndexList)
{
	CBsStream *pt = m_pStreamBuf[g_nLODLevel_]; 

	pMaterial->BeginPass(0);
	pMaterial->CommitChanges();

	pt->SetStreamSource(pDevice, pMaterial->GetStreamCount(pMaterial->QueryActiveTechnique()), pnStreamIndexList);
	pt->DrawMeshPointSprite(pDevice);

	pMaterial->EndPass();
}

void CBsSubMesh::EndRender(C3DDevice* pDevice, CBsMaterial* pMaterial)
{
	pMaterial->EndMaterial(); // SetReady()에 대해 End() 호출!!
}

void CBsSubMesh::AddLODStream(CBsSubMesh *pLODSubMesh)
{
	CBsStream *pt = new CBsStream; // 메모리를 만들고 해당 데이타를 복사합니다
	pt->Assign(pLODSubMesh->GetStreamForLOD()); 

	m_pStreamBuf[m_nStreamLODAllocated] = pt;
	m_nStreamLODAllocated++;
}

//------------------------------------------------------------------------------------------------

#ifdef _USAGE_TOOL_

int CBsSubMesh::MakeOneBm(char **ppFileName, int nFile, char *pStoreName) // static 함수 입니다
{	
	DeleteFile(pStoreName); // 기존에 존재하는 파일이 있다면 삭제 합니다

	BFileStream Stream(pStoreName, BFileStream::create);

	if (!Stream.Valid()){
		CBsConsole::GetInstance().AddFormatString("%s File Create Failed", pStoreName);
		return 0;
	}

	//

	CBsMesh	*mesh[16];
	int nSubMeshCount, nLOD=0, i,j;

	if (nFile>16) return 0; // 파일이 많습니다

	//--------------------

	for (i=0; i<nFile; i++) {
		mesh[i]=new CBsMesh;

		{ 
			BFileStream StreamMesh(ppFileName[i]);
			mesh[i]->LoadMesh(&StreamMesh);	// 전체를 읽을 필요는 없지만 BM 이 제대로 읽혀 지는지 확인이 되는 효과는 있습니다
		}  
		// 이 지점에서 StreamMesh 가 소거 됩니다

		if (i==0) {
			nSubMeshCount = mesh[0]->GetSubMeshCount();
		}

		if (mesh[i]->GetLODCount()!=1 || mesh[i]->GetSubMeshCount() != nSubMeshCount) { // BM LOD 1개 짜리 인지 확인 합니다
#ifndef _XBOX
			char buf[256];
			sprintf(buf, "BM 이 LOD 1개 짜리가 아니거나, 서브메쉬 갯수가 다릅니다 %s", ppFileName[i]);
			MessageBox(NULL, buf, "엔진 메세지 박스", MB_OK);
#endif

			for (j=0; j<i; j++) delete mesh[j]; // 메모리

			return 0; // (메모리 관리 스킵 = 주의)
		}

		nLOD++;
	}

	//--------------------

	const int nSize = 1024 * 1024 * 16;
	int nLength;
	char *pMemory = new char [nSize];

	for (i=0; i<nFile; i++) {
		{
			BFileStream StreamMesh(ppFileName[i]);
			nLength=StreamMesh.Length();

			if (nLength>nSize) {
#ifndef _XBOX
				char buf[256];
				sprintf(buf, "메쉬 파일 크기가 범위를 벗어납니다 %s %d", ppFileName[i],nLength);
				MessageBox(NULL, buf, "엔진 메세지 박스", MB_OK);
#endif

				for (j=0; j<nFile; j++) delete mesh[j]; // 메모리
				delete [] pMemory; // 메모리 삭제 합니다

				return 0; // (메모리 관리 스킵 = 주의)
			}

			StreamMesh.Read(pMemory,nLength); // 읽어 옵니다

			// 이 지점에서 StreamMesh 가 소거 (파일 close) 됩니다 (주의)
		} 

		if (mesh[i]->GetSubMeshCount() != nSubMeshCount) { // 서브메쉬 갯수 확인 합니다
			// 서브 메쉬 갯수가 다를때 여기서 처리 합니다 (서브 메쉬 강제로 맞추기)
		}

		Stream.Write(pMemory,nLength); // 저장 합니다
	} 

	Stream.Seek(BM_FILE_HEADER::eLODOffset, BStream::fromBegin); // 파일에서 nLOD 값을 갱신 합니다
	Stream.Write(&nLOD,4,ENDIAN_FOUR_BYTE);

	// 메쉬 파일 내용은 다음과 같습니다
	//
	// 메쉬 허더
	// for () {
	//     서브메쉬 헤더
	//     m_ppSubMesh[i]->LoadSubMesh(pStream, &headerSubMesh); // 버텍스 스트림을 읽을 것입니다
	// }
	// m_BoundingBox

	//--------------------

	for (j=0; j<nFile; j++) delete mesh[j]; // 메모리
	delete [] pMemory; // 메모리 삭제 합니다

	return 1;
}

#endif

int CBsSubMesh::GetPrimitiveCountLOD(int nLODLevel)
{ 
	if (nLODLevel>=0 && nLODLevel<m_pStreamLOD_max && m_pStreamBuf[nLODLevel]) return m_pStreamBuf[nLODLevel]->GetPrimitiveCount(); 
	else return 0;
} 

//------------------------------------------------------------------------------------------------


