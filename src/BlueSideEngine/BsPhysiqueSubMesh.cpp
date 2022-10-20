#include "stdafx.h"
#include "BStreamExt.h"
#include "BsPhysiqueSubMesh.h"
#include "BsMaterial.h"
#include "BsMesh.h"
#include "BsKernel.h"
#include "BsAni.h"
#include "BsStringPool.h"

BSMATRIX *CBsPhysiqueSubMesh::s_pBoneMatrices = NULL;
int			CBsPhysiqueSubMesh::s_nRefCount = 0;

CBsPhysiqueSubMesh::CBsPhysiqueSubMesh(bool bUseSoftwarePhysique/*=false*/)
{
//	m_nBoneCount=0;
//	m_ppBoneName=NULL;
//	m_pBoneIndex=NULL;

	m_bSplit = false;

	if (!s_pBoneMatrices) s_pBoneMatrices = new BSMATRIX[MAX_BONEMATRIX_ARRAY*10];

	s_nRefCount++;
}

CBsPhysiqueSubMesh::~CBsPhysiqueSubMesh()
{
	Clear_(); // CBsSubMesh::Clear_() 는 호출 되지 않았습니다

	s_nRefCount--; // 생성시에만 증가하는 값이므로 소멸자에서만 실행되도록 구성 합니다
	if(s_nRefCount==0) {
		delete[] s_pBoneMatrices;
		s_pBoneMatrices = NULL;
	}	
}

void CBsPhysiqueSubMesh::Clear_() // CBsSubMesh::Clear_() 는 호출 되지 않았습니다
{
//	if(m_ppBoneName){
//		for(int i=0;i<m_nBoneCount;i++) g_BsKernel.FnaPoolClear(m_ppBoneName[i]);
//		delete [] m_ppBoneName; 
//		m_ppBoneName=NULL;
//	}

//	if(m_pBoneIndex){
//		delete [] m_pBoneIndex;
//		m_pBoneIndex=NULL;
//	}

	SAFE_DELETE_PVEC( m_SplitStream );

	m_bSplit = false;
}

void CBsPhysiqueSubMesh::LoadSubMesh(BStream *pStream, BM_SUBMESH_HEADER *pHeader)
{
	Clear_(); // 피직용 루틴만 호출 될 것입니다

	CBsSubMesh::LoadSubMesh(pStream, pHeader); // 원래 루틴을 먼저 불러 줍니다 = CBsSubMesh::Clear_() 하고 로드 할것입니다

	int nBoneCount = 0;
	pStream->Read(&nBoneCount, sizeof(int), ENDIAN_FOUR_BYTE); // 갯수 입니다

//	m_ppBoneName = new char* [m_nBoneCount];
//	m_pBoneIndex = new int [m_nBoneCount];

//	for(int i=0;i<m_nBoneCount;i++){
//		m_ppBoneName[i]=g_BsKernel.FnaPoolReadAdd(pStream); 
//		m_pBoneIndex[i]=-1;
//		if (stricmp(m_ppBoneName[i],"MeshCopy")==0) {
//			CBsMesh::s_nDetectMeshCopy=1; 
//		}
//	}

	m_pStreamBuf[0]->PreparePhysique(nBoneCount, pStream);

	const int cMaxBoneCount = 50;
	if( nBoneCount > cMaxBoneCount ) {
		m_pStreamBuf[0]->SplitStream(m_SplitStream, cMaxBoneCount);
		m_bSplit = true;
		BsAssert( 0 && "Max Bone Count 초과!!");
	}
}

void CBsPhysiqueSubMesh::LinkBone(CBsAni *pAni)
{
//	for (int i=0;i<m_nBoneCount;i++){
//		m_pBoneIndex[i]=pAni->FindBoneIndex(m_ppBoneName[i]);
//	}
	for(int i=0 ; i<m_nStreamLODAllocated ; ++i ) {
		m_pStreamBuf[i]->LinkBone(pAni);
	}
}

void CBsPhysiqueSubMesh::LinkBone( const std::vector< std::string > & szBoneList ) //mruete - pass by reference for performance
{
/*	int i, j, nCount; 
	nCount = szBoneList.size();
	for( i = 0; i < m_nBoneCount; i++ )
	{
		for( j = 0; j < nCount; j++ )
		{
			if( stricmp( szBoneList[ j ].c_str(), m_ppBoneName[i] ) == 0 )
			{
				m_pBoneIndex[ i ] = j;
				break;
			}
		}
	}
*/
	for(int i=0 ; i<m_nStreamLODAllocated ; ++i ) {
		m_pStreamBuf[i]->LinkBone(szBoneList);
	}
}

void CBsPhysiqueSubMesh::TransformBoneMatrices(C3DDevice* pDevice, D3DXMATRIX* pMatObject, D3DXMATRIX *pBoneMatrix )
{
	BSMATRIX matWV;
	const BSMATRIX *pMatView = (BSMATRIX*)g_BsKernel.GetParamViewMatrix();
	BsMatrixMultiply(&matWV, (BSMATRIX*)pMatObject, pMatView);

/*	for(int i=0;i<m_nBoneCount;i++) { // 모든 본을 Looping Mat를 Constant로 입력
		if( m_pBoneIndex[ i ] != -1 )
		{
			BsMatrixMultiply( &s_pBoneMatrices[ i ], 
				( BSMATRIX * )pObj->GetRenderMatrix( m_pBoneIndex[ i ] ), &matWV );
		}
		else {
			s_pBoneMatrices[i] = matWV;
		}
	}
*/
	CBsStream *pt = m_pStreamBuf[g_nLODLevel_];
	pt->TransformBoneMatrices(matWV, pBoneMatrix);
}

void CBsPhysiqueSubMesh::PrepareRender(C3DDevice* pDevice, CBsMaterial* pMaterial, D3DXMATRIX* pMatObject, D3DXMATRIX *pBoneMatrix )
{
	if(pMaterial->GetMatrixArrayHandle()) {	// Set Bone Matrix!!
		TransformBoneMatrices(pDevice, pMatObject, pBoneMatrix);		
	}
	*(pMaterial->GetWorldMatrix()) = *pMatObject;

	pMaterial->SetParameters(); // Parameter Setting!!!!
}

void CBsPhysiqueSubMesh::Render(C3DDevice* pDevice, CBsMaterial* pMaterial, int* pnStreamIndexList)
{
	int i, j;

	if( m_bSplit ) {

		int nSplitCount = m_SplitStream.size();

		pMaterial->BeginPass(0);

		int nMaxBoneCount = 0;
		for( i = 0; i < nSplitCount; i++)
		{
			if(m_SplitStream[i]->GetBoneCount() > nMaxBoneCount)
				nMaxBoneCount = m_SplitStream[i]->GetBoneCount();
		}
		BsAssert(nMaxBoneCount);
		BSMATRIX *pBoneMatrixArray = (BSMATRIX*)_aligned_malloc(sizeof(BSMATRIX) * nMaxBoneCount, 16);
		BsAssert(pBoneMatrixArray);

		if(pBoneMatrixArray)
		{
			for( i = 0; i < nSplitCount; i++)
			{
				CBsStream *pt = m_SplitStream[i];
				int nBoneCount = pt->GetBoneCount();

				for(j = 0; j < nBoneCount; j++)
					pBoneMatrixArray[j] = s_pBoneMatrices[ pt->GetBoneIndex( j ) ];

				pMaterial->SetBoneTransformMatrices(pBoneMatrixArray, nBoneCount);

				pMaterial->CommitChanges();

				pt->SetStreamSource(pDevice, pMaterial->GetStreamCount(pMaterial->QueryActiveTechnique()), pnStreamIndexList);
				pt->DrawMesh(pDevice);
			}
			_aligned_free(pBoneMatrixArray);
		}

		pMaterial->EndPass();

	}
	else {
        pMaterial->SetBoneTransformMatrices(s_pBoneMatrices, m_pStreamBuf[g_nLODLevel_]->GetBoneCount());        
		CBsSubMesh::Render(pDevice, pMaterial, pnStreamIndexList);
	}
}


void CBsPhysiqueSubMesh::RenderShadow(C3DDevice* pDevice, CBsMaterial* pMaterial, int nLevelIndex /*=0*/)
{
	int i, j;

	if( m_bSplit ) {

		int nSplitCount = m_SplitStream.size();

		pMaterial->BeginPass(0);

		int nMaxBoneCount = 0;
		for( i = 0; i < nSplitCount; i++)
		{
			if(m_SplitStream[i]->GetBoneCount() > nMaxBoneCount)
				nMaxBoneCount = m_SplitStream[i]->GetBoneCount();
		}
		BsAssert(nMaxBoneCount);
		BSMATRIX *pBoneMatrixArray = (BSMATRIX*)_aligned_malloc(sizeof(BSMATRIX) * nMaxBoneCount, 16);
		BsAssert(pBoneMatrixArray);

		if(pBoneMatrixArray)
		{
			for( i = 0; i < nSplitCount; i++) {

				CBsStream *pt = m_SplitStream[i];
				int nBoneCount = pt->GetBoneCount();

				for( j = 0; j < nBoneCount; j++)
					pBoneMatrixArray[j] = s_pBoneMatrices[ pt->GetBoneIndex( j ) ];

				pMaterial->SetBoneTransformMatrices(pBoneMatrixArray, nBoneCount);

				pMaterial->CommitChanges();

				int nBoneLinkCount = pt->GetBoneLinkCount() + 1;
				int nStreamIndex[3];
				UINT uiUsage[3] = { BS_USAGE_POSITION, BS_USAGE_BLENDINDICES, BS_USAGE_BLENDWEIGHT};	
				for ( j = 0; j < nBoneLinkCount; j++) {
					nStreamIndex[j] = pt->FindStreamByUsage(uiUsage[j], 0);
				}
				pt->SetStreamSource(pDevice, nBoneLinkCount, nStreamIndex);
				pt->DrawMesh(pDevice);
			}
			_aligned_free(pBoneMatrixArray);
		}

		pMaterial->EndPass();
	}
	else {
		pMaterial->SetBoneTransformMatrices(s_pBoneMatrices, m_pStreamBuf[g_nLODLevel_]->GetBoneCount());
		CBsSubMesh::RenderShadow(pDevice, pMaterial, nLevelIndex);
	}	
}

void CBsPhysiqueSubMesh::RenderShadowVolume(C3DDevice* pDevice, CBsMaterial* pMaterial, int nLevelIndex /*=0*/)
{
	pMaterial->SetBoneTransformMatrices(s_pBoneMatrices, m_pStreamBuf[g_nLODLevel_]->GetBoneCount());
	CBsSubMesh::RenderShadowVolume(pDevice, pMaterial, nLevelIndex);
}

void CBsPhysiqueSubMesh::RenderAlpha(C3DDevice* pDevice, CBsMaterial* pMaterial, int* pnStreamIndexList)
{
	int i, j;

	if( m_bSplit ) {
		int nSplitCount = m_SplitStream.size();

		pMaterial->BeginPass(0);

		int nMaxBoneCount = 0;
		for( i = 0; i < nSplitCount; i++)
		{
			if(m_SplitStream[i]->GetBoneCount() > nMaxBoneCount)
				nMaxBoneCount = m_SplitStream[i]->GetBoneCount();
		}
		BsAssert(nMaxBoneCount);
		BSMATRIX *pBoneMatrixArray = (BSMATRIX*)_aligned_malloc(sizeof(BSMATRIX) * nMaxBoneCount, 16);
		BsAssert(pBoneMatrixArray);

		if(pBoneMatrixArray)
		{
			for( i = 0; i < nSplitCount; i++) {

				CBsStream *pt = m_SplitStream[i];
				int nBoneCount = pt->GetBoneCount();

				for(j = 0; j < nBoneCount; j++)
					pBoneMatrixArray[j] = s_pBoneMatrices[ pt->GetBoneIndex( j ) ];

				pMaterial->SetBoneTransformMatrices(pBoneMatrixArray, nBoneCount);

				pMaterial->CommitChanges();

				pt->SetStreamSource(pDevice, pMaterial->GetStreamCount(pMaterial->QueryActiveTechnique()), pnStreamIndexList);
				pt->DrawMesh(pDevice);
			}
			_aligned_free(pBoneMatrixArray);
		}

		pMaterial->EndPass();
	}
	else {
		pMaterial->SetBoneTransformMatrices(s_pBoneMatrices, m_pStreamBuf[g_nLODLevel_]->GetBoneCount());
		CBsSubMesh::RenderAlpha(pDevice, pMaterial, pnStreamIndexList);
	}	
}

void CBsPhysiqueSubMesh::RenderAlphaAlign(C3DDevice* pDevice, CBsMaterial* pMaterial, int* pnStreamIndexList)
{
	if( m_bSplit ) {
		BsAssert( 0 ); // 왠만해선 안들어온다고 가정...
	}
	else {
		pMaterial->SetBoneTransformMatrices(s_pBoneMatrices, m_pStreamBuf[g_nLODLevel_]->GetBoneCount());
		CBsSubMesh::RenderAlphaAlign(pDevice, pMaterial, pnStreamIndexList);
	}
}

void CBsPhysiqueSubMesh::RenderPointsprite(C3DDevice* pDevice, CBsMaterial* pMaterial, int* pnStreamIndexList)
{
	if( m_bSplit ) {
		BsAssert( 0 ); // 왠만해선 안들어온다고 가정...
	}
	else {
		pMaterial->SetBoneTransformMatrices(s_pBoneMatrices, m_pStreamBuf[g_nLODLevel_]->GetBoneCount());
		CBsSubMesh::RenderPointsprite(pDevice, pMaterial, pnStreamIndexList);
	}
}

void CBsPhysiqueSubMesh::EndRender(C3DDevice* pDevice, CBsMaterial* pMaterial)
{
	pMaterial->EndMaterial(); // SetReady()에 대해 End() 호출!!
}



