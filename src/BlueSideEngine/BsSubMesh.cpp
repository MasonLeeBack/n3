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
	m_nStreamLODAllocated=0; // AddLODStream() ���� ���� �˴ϴ�
}

CBsSubMesh::~CBsSubMesh()
{
	Clear_();
}

void CBsSubMesh::Clear_()
{
	g_BsKernel.FnaPoolClear(m_szParentSubMeshName);
	m_szParentSubMeshName=NULL;	// ������ ȣ�� �� ��츦 ��� �մϴ� = assert()

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
	CBsSubMesh::Clear_(); // CBsPhysiqueSubMesh* �� ȣ��Ǵ� ��쿡 ������ �߻� ���� �ʵ��� CBsSubMesh:: �� ���� �մϴ�	(���� �Լ� �̹Ƿ� ���� �մϴ�)

#ifndef _USAGE_TOOL_
	BM_MATERIAL_HEADER mtr_header; // ��Ʈ���� ���� ��� ���� ������ �Դϴ� = local
#endif

	m_szParentSubMeshName=g_BsKernel.FnaPoolReadAdd(pStream);
	m_szSubMeshName		 =g_BsKernel.FnaPoolReadAdd(pStream);

	int nMaterialCount;

	pStream->Read(&nMaterialCount, sizeof(int), ENDIAN_FOUR_BYTE);

	if (nMaterialCount) { // ������ 1 �Դϴ�
		LoadMaterialHeader(pStream, &mtr_header); // ������ �޽��� �ִ� Material�� �����Ѵ�. by jeremy
		// �������� ��� �� ���Դϴ�
	}
	else {
		pHeader->nTexChannelCount=1; // ��� ���� ������ �ٲپ� �ݴϴ�
	}
	m_nMaterialIndex=-1;

	//-----------------------

	const int strLenBooking = 7; // "booking"

	if(_strnicmp(m_szSubMeshName,"booking",strLenBooking)==0) { // �ڽ��� ó�� ���� ���� ���� (������ ������ �߰��� ��� ���ؼ�)
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

	if(_strnicmp(m_szSubMeshName,"collision",strLenCollision)==0) { // �ڽ��� ó�� ���� ���� ���� (������ ������ �߰��� ��� ���ؼ�)
		m_bIsShow=false;
		int nDummyNo=atoi(m_szSubMeshName+strLenCollision);
		CBsMesh::s_nDetectCollisionIndex=nDummyNo;		
		BsAssert( 0 <= nDummyNo && nDummyNo <= 99 && "Invalid Static Mesh Collision" );
	}
	else {
		CBsMesh::s_nDetectCollisionIndex=-1;
	}

	const int strLenLinkDummy = 4;	// String Length of "link"
	if(_strnicmp(m_szSubMeshName,"link",strLenLinkDummy)==0) { // �ڽ��� ó�� ���� ���� ���� (������ ������ �߰��� ��� ���ؼ�)
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

	m_nUVAnimationV = nMaterialCount && mtr_header.szMaterialName[255];	// �� ���� 1 �϶� �߰� ����Ÿ�� ���� ���Դϴ� = 0,1

	// UV ���ϸ��̼��� ���� �մϴ�	  

	if (m_nUVAnimationV) {
		for (int i=0; i<BM_MATERIAL_HEADER::eStage7; i++) {
			if (mtr_header.nTextureType[i]) {
				if (m_nTextureAniNum==0) {
					pStream->Read(&m_nTextureAniNum, sizeof(int), ENDIAN_FOUR_BYTE);  // �ؽ��� ���� ������ �ٸ��ϴ�

					m_TextureAni = new TEXTURE_ANI [m_nTextureAniNum];
					pStream->Read(m_TextureAni, sizeof(TEXTURE_ANI)*m_nTextureAniNum, ENDIAN_FOUR_BYTE);

					// �����
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
				else { // ������ ����Ÿ�� �д� �ӽ� �ڵ� �Դϴ�
					int n;
					pStream->Read(&n, sizeof(int), ENDIAN_FOUR_BYTE);  // �ؽ��� ���� ������ �ٸ��ϴ�

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
		m_pStreamBuf[0] = new CBsStream; // ������ ��� �ǹǷ� �޸� ������ �̸� ������ �����ϴ�
	}
	m_pStreamBuf[0]->LoadStream(pStream, pHeader); // �޸� �ҰŴ� ���ο��� �ڵ� �Դϴ�
	m_nStreamLODAllocated=1; // 1 ���� �Ҵ� �Ǿ����ϴ� (�� �������� ������ ������ �Ұ� �˴ϴ�)

	// �ε� �ð��� ������ ��ġ�Ƿ� Expoter ��� �̸� ����ϵ��� ���Ŀ� �����մϴ�..
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

	// LOD ���� �߰� �ڵ� 

	// m_nStreamLODAllocated �� AddLODStream() ���� ���� �˴ϴ�

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

			D3DVERTEXELEMENT9 *pVertexElements = new D3DVERTEXELEMENT9[nSize+5]; // ���� ������ + 5

			for(int iElement=0;iElement<nSize;++iElement) {
				pMapper->m_pnStreamIndex[iElement] = pStream->FindStreamByUsage(VertexElements[iElement].Usage, VertexElements[iElement].UsageIndex);
				if(pMapper->m_pnStreamIndex[iElement]==-1) {
					// ��� LOD Stream�� ����!!
					for(int j=0;j<m_nStreamLODAllocated;++j) {
						if(!m_pStreamBuf[j]->GenerateStream(VertexElements[iElement].Usage, VertexElements[iElement].UsageIndex )) {
							pMapper->m_pnStreamIndex[iElement] = -1; // ���� �Ұ��� �Դϴ�
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

	// �ӽ÷�.............���� ����!! by jeremy
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
	if( pnStreamIndexList != NULL ) {	// shadow �ΰ�� skip
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
	pMaterial->EndMaterial(); // SetReady()�� ���� End() ȣ��!!
}

void CBsSubMesh::AddLODStream(CBsSubMesh *pLODSubMesh)
{
	CBsStream *pt = new CBsStream; // �޸𸮸� ����� �ش� ����Ÿ�� �����մϴ�
	pt->Assign(pLODSubMesh->GetStreamForLOD()); 

	m_pStreamBuf[m_nStreamLODAllocated] = pt;
	m_nStreamLODAllocated++;
}

//------------------------------------------------------------------------------------------------

#ifdef _USAGE_TOOL_

int CBsSubMesh::MakeOneBm(char **ppFileName, int nFile, char *pStoreName) // static �Լ� �Դϴ�
{	
	DeleteFile(pStoreName); // ������ �����ϴ� ������ �ִٸ� ���� �մϴ�

	BFileStream Stream(pStoreName, BFileStream::create);

	if (!Stream.Valid()){
		CBsConsole::GetInstance().AddFormatString("%s File Create Failed", pStoreName);
		return 0;
	}

	//

	CBsMesh	*mesh[16];
	int nSubMeshCount, nLOD=0, i,j;

	if (nFile>16) return 0; // ������ �����ϴ�

	//--------------------

	for (i=0; i<nFile; i++) {
		mesh[i]=new CBsMesh;

		{ 
			BFileStream StreamMesh(ppFileName[i]);
			mesh[i]->LoadMesh(&StreamMesh);	// ��ü�� ���� �ʿ�� ������ BM �� ����� ���� ������ Ȯ���� �Ǵ� ȿ���� �ֽ��ϴ�
		}  
		// �� �������� StreamMesh �� �Ұ� �˴ϴ�

		if (i==0) {
			nSubMeshCount = mesh[0]->GetSubMeshCount();
		}

		if (mesh[i]->GetLODCount()!=1 || mesh[i]->GetSubMeshCount() != nSubMeshCount) { // BM LOD 1�� ¥�� ���� Ȯ�� �մϴ�
#ifndef _XBOX
			char buf[256];
			sprintf(buf, "BM �� LOD 1�� ¥���� �ƴϰų�, ����޽� ������ �ٸ��ϴ� %s", ppFileName[i]);
			MessageBox(NULL, buf, "���� �޼��� �ڽ�", MB_OK);
#endif

			for (j=0; j<i; j++) delete mesh[j]; // �޸�

			return 0; // (�޸� ���� ��ŵ = ����)
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
				sprintf(buf, "�޽� ���� ũ�Ⱑ ������ ����ϴ� %s %d", ppFileName[i],nLength);
				MessageBox(NULL, buf, "���� �޼��� �ڽ�", MB_OK);
#endif

				for (j=0; j<nFile; j++) delete mesh[j]; // �޸�
				delete [] pMemory; // �޸� ���� �մϴ�

				return 0; // (�޸� ���� ��ŵ = ����)
			}

			StreamMesh.Read(pMemory,nLength); // �о� �ɴϴ�

			// �� �������� StreamMesh �� �Ұ� (���� close) �˴ϴ� (����)
		} 

		if (mesh[i]->GetSubMeshCount() != nSubMeshCount) { // ����޽� ���� Ȯ�� �մϴ�
			// ���� �޽� ������ �ٸ��� ���⼭ ó�� �մϴ� (���� �޽� ������ ���߱�)
		}

		Stream.Write(pMemory,nLength); // ���� �մϴ�
	} 

	Stream.Seek(BM_FILE_HEADER::eLODOffset, BStream::fromBegin); // ���Ͽ��� nLOD ���� ���� �մϴ�
	Stream.Write(&nLOD,4,ENDIAN_FOUR_BYTE);

	// �޽� ���� ������ ������ �����ϴ�
	//
	// �޽� ���
	// for () {
	//     ����޽� ���
	//     m_ppSubMesh[i]->LoadSubMesh(pStream, &headerSubMesh); // ���ؽ� ��Ʈ���� ���� ���Դϴ�
	// }
	// m_BoundingBox

	//--------------------

	for (j=0; j<nFile; j++) delete mesh[j]; // �޸�
	delete [] pMemory; // �޸� ���� �մϴ�

	return 1;
}

#endif

int CBsSubMesh::GetPrimitiveCountLOD(int nLODLevel)
{ 
	if (nLODLevel>=0 && nLODLevel<m_pStreamLOD_max && m_pStreamBuf[nLODLevel]) return m_pStreamBuf[nLODLevel]->GetPrimitiveCount(); 
	else return 0;
} 

//------------------------------------------------------------------------------------------------


