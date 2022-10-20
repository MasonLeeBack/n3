#include "stdafx.h"
#include "BStreamExt.h"
#include "BsKernel.h"
#include "BsStream.h"
#include "Box3.h"
#include "BsCommon.h"
#include "BsMesh.h"
#include "BsPhysiqueSubMesh.h"
#include "BsAni.h"
#include "BsShadowVolumeMgr.h"
#include "BsInstancingMgr.h"

#ifdef _XBOX
#define _OPT_SHINJICH_BSSTREAM	1
#endif

struct FaceTable
{
	BYTE nCount;
	BYTE nList[6];	
};

struct PartTable
{
	int BoneList[255];
	int nBoneCount;
	int StripList[255];
	int nStripCount;
};

struct SortTableFn
{
	const bool operator() (const PartTable &lhs, const PartTable &rhs)
	{
		return (lhs.nBoneCount > rhs.nBoneCount);
	}
};

CBsStream::CBsStream()
{
	m_bAllocated = false;

	m_PrimitiveType = D3DPT_TRIANGLELIST ; 
	m_nVertexCount = 0;	
	m_nPrimitiveCount = 0;
	m_nMaxBlendWeight = 0;

	m_pIB = NULL;	
	m_pInstancingIB = NULL;
	m_nInstancingCount = 0;

	//m_bUse32 = false;
	//m_bUseVB = false;

	m_nBoneCount = 0;
	m_pBoneIndex = NULL;
	m_ppBoneName = NULL;


	m_pShadowVolumeVB = NULL;
	m_pShadowVolumeIB = NULL;	
	m_nShadowVolumePrimCount = 0;
}

CBsStream::~CBsStream()
{
	Clear();
}

void CBsStream::Clear()
{
	if (!m_bAllocated) {
		return;
	}

	if(m_ppBoneName){
		for(int i=0;i<m_nBoneCount;i++) g_BsKernel.FnaPoolClear(m_ppBoneName[i]);
		delete [] m_ppBoneName; 
		m_ppBoneName=NULL;
	}

	if(m_pBoneIndex){
		delete [] m_pBoneIndex;
		m_pBoneIndex=NULL;
	}

	m_nBoneCount = 0;
	m_nMaxBlendWeight = 0;

	for(unsigned int i=0;i<m_StreamMaps.size();++i) {
		if(m_StreamMaps[i].m_pVB) {
			m_StreamMaps[i].m_pVB->Release(); // NULL 지정은 하지 않습니다
		}
	}
	m_StreamMaps.clear();

	SAFE_RELEASE(m_pIB);
	SAFE_RELEASE(m_pInstancingIB);
	SAFE_RELEASE(m_pShadowVolumeVB);
	SAFE_RELEASE(m_pShadowVolumeIB);

	m_bAllocated = false;
}

int CBsStream::GetCountByUsage(BS_DECLUSAGE usage)    
{
	int nCount=0;
	for(unsigned int i=0;i<m_StreamMaps.size();++i) {
		if(m_StreamMaps[i].m_DeclUsage==usage) nCount++;
	}
	return nCount;
}

void CBsStream::ReadByDeclType(BStream *pStream, void* pOut, BS_DECLTYPE type, unsigned long size)
{
	switch(type) {
		case BS_TYPE_FLOAT1:
		case BS_TYPE_FLOAT2:
		case BS_TYPE_FLOAT3:
		case BS_TYPE_FLOAT4:
		case BS_TYPE_D3DCOLOR:
			pStream->Read(pOut, size, ENDIAN_FOUR_BYTE);
			break;
		case BS_TYPE_UBYTE4:
			pStream->Read(pOut, size);
			break;
		case BS_TYPE_SHORT2:
		case BS_TYPE_SHORT4:
			pStream->Read(pOut, size, ENDIAN_TWO_BYTE);
			break;
		case BS_TYPE_UBYTE4N:
			pStream->Read(pOut, size);
			break;
		case BS_TYPE_SHORT2N:
		case BS_TYPE_SHORT4N:
		case BS_TYPE_USHORT2N:
		case BS_TYPE_USHORT4N:
			pStream->Read(pOut, size, ENDIAN_TWO_BYTE);
			break;
		case BS_TYPE_UDEC3:
		case BS_TYPE_DEC3N:
		case BS_TYPE_FLOAT16_2:
		case BS_TYPE_FLOAT16_4:
		case BS_TYPE_UNUSED:
			BsAssert( 0 && "이거 작업 임시 보류!!" );
			break;
	}
}

void CBsStream::ReadAndCompressByDeclType(BStream* pStream, void* pOut, BS_DECLUSAGE usage, BS_DECLTYPE type, int nVertexCount)
{
	if(type == BS_TYPE_HEND3N) {
		D3DXVECTOR3* pElement = new D3DXVECTOR3[nVertexCount];
		pStream->Read(pElement, sizeof(D3DXVECTOR3)*nVertexCount, ENDIAN_FOUR_BYTE);
		DWORD* pdwOut = (DWORD*)pOut;
		D3DXVECTOR3* pTemp = pElement;
		for(int i=0;i<nVertexCount;++i) {
			*(pdwOut++) = MakePacked_11_11_10(int(pTemp->x*1023.f), int(pTemp->y*1023.f), int(pTemp->z*511.f));
			pTemp++;
		}
		delete[] pElement;
	}
	else if(type == BS_TYPE_FLOAT16_2) {
		D3DXVECTOR2* pElement = new D3DXVECTOR2[nVertexCount];
		pStream->Read(pElement, sizeof(D3DXVECTOR2)*nVertexCount, ENDIAN_FOUR_BYTE);
		D3DXFloat32To16Array((D3DXFLOAT16*)pOut, (float*)pElement, nVertexCount*2);
		delete [] pElement;	// mruete: prefix 460: fixed delete
	}
}

void CBsStream::BuildBasisVector() 
{
	int nFaceOffset, nOffset[3];
	WORD* pFaceBuf;
	char* pPosBuf;
	D3DXVECTOR3* pNorBuf;
	char* pTexReadBuf;
	D3DXVECTOR3* pTangentBuf;
	D3DXVECTOR3* pBinormalBuf;

	D3DXVECTOR2 *pV0TexUV, *pV1TexUV, *pV2TexUV;
	D3DXVECTOR3 *pV0Pos, *pV1Pos, *pV2Pos;
	D3DXVECTOR3 *pV0Tan, *pV1Tan, *pV2Tan;
	D3DXVECTOR3	*pV0Bi, *pV1Bi, *pV2Bi;
	D3DXVECTOR3 Edge1, Edge2, Cross, Normal;

	int nPosIdx = FindStreamByUsage(BS_USAGE_POSITION, 0);
	int nNorIndex = FindStreamByUsage(BS_USAGE_NORMAL, 0);
	int nTexIndex = FindStreamByUsage(BS_USAGE_TEXCOORD, 0);
	int nTanIndex = FindStreamByUsage(BS_USAGE_TANGENT, 0);
	int nBinorIndex = FindStreamByUsage(BS_USAGE_BINORMAL, 0);

	int nPosStride = m_StreamMaps[nPosIdx].m_nStride;
	int nNorStride = m_StreamMaps[nNorIndex].m_nStride;
	int nTexStride = m_StreamMaps[nTexIndex].m_nStride;
	int nTanStride = m_StreamMaps[nTanIndex].m_nStride;
	int nBinorStride = m_StreamMaps[nBinorIndex].m_nStride;

	m_pIB->Lock(0, 0, (void**)&pFaceBuf, 0);
	m_StreamMaps[nPosIdx].m_pVB->Lock(0, m_nVertexCount*m_StreamMaps[nPosIdx].m_nStride, (void**)&pPosBuf, 0);
	m_StreamMaps[nNorIndex].m_pVB->Lock(0, m_nVertexCount*m_StreamMaps[nNorIndex].m_nStride, (void**)&pNorBuf, 0);
	m_StreamMaps[nTexIndex].m_pVB->Lock(0, m_nVertexCount*m_StreamMaps[nTexIndex].m_nStride, (void**)&pTexReadBuf, 0);
	D3DXVECTOR2* pTexUnpackBuf = new D3DXVECTOR2[m_nVertexCount];
	if(m_StreamMaps[nTexIndex].m_DeclType==D3DDECLTYPE_FLOAT16_2) {
		D3DXFloat16To32Array((FLOAT*)pTexUnpackBuf, (D3DXFLOAT16*)pTexReadBuf, m_nVertexCount*2);
	}
	else if(m_StreamMaps[nTexIndex].m_DeclType == D3DDECLTYPE_FLOAT2) {
		memcpy(pTexUnpackBuf, pTexReadBuf, sizeof(D3DXVECTOR2)*m_nVertexCount);
	}
	else {
	}

	pTangentBuf = new D3DXVECTOR3[ m_nVertexCount ];
	pBinormalBuf = new D3DXVECTOR3[ m_nVertexCount ];

	//////////////////////////////////////////////////////////////////////////

	for(int i=0;i<m_nVertexCount;++i) {
		*(pTangentBuf+i) = D3DXVECTOR3(0.f, 0.f, 0.f);
		*(pBinormalBuf+i) = D3DXVECTOR3(0.f, 0.f, 0.f);
	}

	nFaceOffset = 0;
	for(int i=0;i<m_nPrimitiveCount;++i) {
		if(m_PrimitiveType == D3DPT_TRIANGLELIST ) {
			nFaceOffset = i*3;	
			nOffset[0] = pFaceBuf[nFaceOffset];
			nOffset[1] = pFaceBuf[nFaceOffset+1];
			nOffset[2] = pFaceBuf[nFaceOffset+2];
		} else if(m_PrimitiveType == D3DPT_TRIANGLESTRIP  ) {
			if( pFaceBuf[i+2] == 0xFFFF ) {
				i+=3;
			}
			nOffset[0] = pFaceBuf[i];			
			nOffset[1] = pFaceBuf[i+1];
			nOffset[2] = pFaceBuf[i+2];			
		} else {
			BsAssert( 0 && "Invalid Primitive Type!!" );
		}


		pV0TexUV=(D3DXVECTOR2*)(pTexUnpackBuf+nOffset[0]);
		pV0Pos = (D3DXVECTOR3*)(pPosBuf+nOffset[0]*nPosStride);
		pV0Tan = pTangentBuf+nOffset[0];
		pV0Bi  = pBinormalBuf + nOffset[0];

		pV1TexUV=(D3DXVECTOR2*)(pTexUnpackBuf+nOffset[1]);
		pV1Pos = (D3DXVECTOR3*)(pPosBuf+nOffset[1]*nPosStride);
		pV1Tan = pTangentBuf+nOffset[1];
		pV1Bi  = pBinormalBuf + nOffset[1];

		pV2TexUV=(D3DXVECTOR2*)(pTexUnpackBuf+nOffset[2]);
		pV2Pos = (D3DXVECTOR3*)(pPosBuf+nOffset[2]*nPosStride);
		pV2Tan = pTangentBuf+nOffset[2];
		pV2Bi  = pBinormalBuf + nOffset[2];
		///////////////////////////////////////
		Edge1 = D3DXVECTOR3(pV1Pos->x - pV0Pos->x, pV1TexUV->x - pV0TexUV->x, pV1TexUV->y - pV0TexUV->y );
		Edge2 = D3DXVECTOR3(pV2Pos->x - pV0Pos->x, pV2TexUV->x - pV0TexUV->x, pV2TexUV->y - pV0TexUV->y );

		D3DXVec3Cross(&Cross, &Edge1, &Edge2);
		if(fabs(Cross.x)>NEAR_ZERO) {
			pV0Tan->x += -Cross.y/Cross.x;
			pV0Bi->x  += -Cross.z/Cross.x;
			pV1Tan->x += -Cross.y/Cross.x;
			pV1Bi->x  += -Cross.z/Cross.x;
			pV2Tan->x += -Cross.y/Cross.x;
			pV2Bi->x  += -Cross.z/Cross.x;
		}

		///////////////////////////////////////
		Edge1 = D3DXVECTOR3(pV1Pos->y - pV0Pos->y, pV1TexUV->x - pV0TexUV->x, pV1TexUV->y - pV0TexUV->y );
		Edge2 = D3DXVECTOR3(pV2Pos->y - pV0Pos->y, pV2TexUV->x - pV0TexUV->x, pV2TexUV->y - pV0TexUV->y );

		D3DXVec3Cross(&Cross, &Edge1, &Edge2);
		if(fabs(Cross.x)>NEAR_ZERO) {
			pV0Tan->y += -Cross.y/Cross.x;
			pV0Bi->y  += -Cross.z/Cross.x;
			pV1Tan->y += -Cross.y/Cross.x;
			pV1Bi->y  += -Cross.z/Cross.x;
			pV2Tan->y += -Cross.y/Cross.x;
			pV2Bi->y  += -Cross.z/Cross.x;
		}

		///////////////////////////////////////
		Edge1 = D3DXVECTOR3(pV1Pos->z - pV0Pos->z, pV1TexUV->x - pV0TexUV->x, pV1TexUV->y - pV0TexUV->y );
		Edge2 = D3DXVECTOR3(pV2Pos->z - pV0Pos->z, pV2TexUV->x - pV0TexUV->x, pV2TexUV->y - pV0TexUV->y );

		D3DXVec3Cross(&Cross, &Edge1, &Edge2);
		if(fabs(Cross.x)>NEAR_ZERO) {
			pV0Tan->z += -Cross.y/Cross.x;
			pV0Bi->z  += -Cross.z/Cross.x;
			pV1Tan->z += -Cross.y/Cross.x;
			pV1Bi->z  += -Cross.z/Cross.x;
			pV2Tan->z += -Cross.y/Cross.x;
			pV2Bi->z  += -Cross.z/Cross.x;
		}
	}

	D3DXVECTOR3 vecSxT;
	for(int i=0;i<m_nVertexCount;++i) {
		D3DXVec3Normalize(pTangentBuf+i, pTangentBuf+i);
		D3DXVec3Normalize(pBinormalBuf+i, pBinormalBuf+i);
		//		D3DXVec3Cross( &vecSxT, pTangentBuf+i, pBinormalBuf+i);

		//		if(D3DXVec3Dot(&vecSxT, pNorBuf+i)<0.f) {		// Texture가 뒤집혀진건지 판단!!
		//			vecSxT *= -1.f;
		//		}
		//		D3DXVec3Normalize(pNorBuf+i, &vecSxT);
	}
	delete[] pTexUnpackBuf;
	m_pIB->Unlock();
	m_StreamMaps[nPosIdx].m_pVB->Unlock();
	m_StreamMaps[nNorIndex].m_pVB->Unlock();
	m_StreamMaps[nTexIndex].m_pVB->Unlock();

	D3DXVECTOR3 *pTangentSrcBuf;
	D3DXVECTOR3 *pBinormalSrcBuf;

	m_StreamMaps[nTanIndex].m_pVB->Lock(0, m_nVertexCount*m_StreamMaps[nTanIndex].m_nStride, (void**)&pTangentSrcBuf, 0);
	m_StreamMaps[nBinorIndex].m_pVB->Lock(0, m_nVertexCount*m_StreamMaps[nBinorIndex].m_nStride, (void**)&pBinormalSrcBuf, 0);

	memcpy( pTangentSrcBuf, pTangentBuf, m_nVertexCount*m_StreamMaps[nTanIndex].m_nStride);
	memcpy( pBinormalSrcBuf, pBinormalBuf, m_nVertexCount*m_StreamMaps[nBinorIndex].m_nStride);

	m_StreamMaps[nTanIndex].m_pVB->Unlock();
	m_StreamMaps[nBinorIndex].m_pVB->Unlock();

	delete [] pTangentBuf;
	delete [] pBinormalBuf;

}

/*
typedef enum _D3DDECLTYPE {
D3DDECLTYPE_FLOAT1 = 0,		   ! 04				웨이트
D3DDECLTYPE_FLOAT2 = 1,		   ! 08	텍스쳐 	    웨이트
D3DDECLTYPE_FLOAT3 = 2,        ! 12	포지션 노말	웨이트
D3DDECLTYPE_FLOAT4 = 3,		   ! 16				웨이트
D3DDECLTYPE_D3DCOLOR = 4,
D3DDECLTYPE_UBYTE4 = 5,
D3DDECLTYPE_SHORT2 = 6,		   ! 04	본
D3DDECLTYPE_SHORT4 = 7,		   ! 08	본
D3DDECLTYPE_UBYTE4N = 8,
D3DDECLTYPE_SHORT2N = 9,
D3DDECLTYPE_SHORT4N = 10,
D3DDECLTYPE_USHORT2N = 11,
D3DDECLTYPE_USHORT4N = 12,
D3DDECLTYPE_UDEC3 = 13,
D3DDECLTYPE_DEC3N = 14,
D3DDECLTYPE_FLOAT16_2 = 15,
D3DDECLTYPE_FLOAT16_4 = 16,
D3DDECLTYPE_UNUSED = 17
} D3DDECLTYPE;

typedef enum _D3DDECLUSAGE {
D3DDECLUSAGE_POSITION = 0,	   ! f3 포지션
D3DDECLUSAGE_BLENDWEIGHT = 1,  ! f?	웨이트
D3DDECLUSAGE_BLENDINDICES = 2, ! s? 본
D3DDECLUSAGE_NORMAL = 3,	   ! f3 노말
D3DDECLUSAGE_PSIZE = 4,
D3DDECLUSAGE_TEXCOORD = 5,	   ! f2 텍스쳐
D3DDECLUSAGE_TANGENT = 6,      ! f3	확장
D3DDECLUSAGE_BINORMAL = 7,	   ! f3	확장
D3DDECLUSAGE_TESSFACTOR = 8,
D3DDECLUSAGE_POSITIONT = 9,
D3DDECLUSAGE_COLOR = 10,
D3DDECLUSAGE_FOG = 11,
D3DDECLUSAGE_DEPTH = 12,
D3DDECLUSAGE_SAMPLE = 13
} D3DDECLUSAGE;
*/

#define SET_STREAM_COPY(TYP,USG,IDX,SIZ)														\
	vertex.m_DeclType = TYP;													                \
	vertex.m_DeclUsage = USG;													                \
	vertex.m_nUsageIndex = IDX;																	\
	vertex.m_nStride = SIZ;																	    \
	g_BsKernel.CreateVertexBuffer(m_nVertexCount*SIZ, 0, 0, D3DPOOL_MANAGED, &vertex.m_pVB);	\
	m_StreamMaps.push_back(vertex);

int CBsStream::LoadStream(BStream *pStream, BM_SUBMESH_HEADER *pHeader)  
{
	Clear(); // 언제나 메모리를 파일에서 읽은 버텍스 디클레이션을 참조하여 재확보 합니다

	m_bAllocated = true;

	m_PrimitiveType = ConvertPrimitiveType(pHeader->nPrimitiveType);	

	m_nVertexCount = pHeader->nVertexCount;
	if(m_nVertexCount>=MAX_UNSIGNED_SHORT) {
		BsAssert( 0 && "vertex count limit, decrese vertex count in MAX");
	}
	m_nPrimitiveCount = pHeader->nFaceCount;

	//-------------
	// 버텍스 버퍼
	//-------------

	BS_VERTEX_CHECK check;
	int index=0, *bufp, size; 

	for (;;) {
		pStream->Read(&check, sizeof(BS_VERTEX_CHECK), ENDIAN_FOUR_BYTE);
		if (999 == check.m_nUsageIndex) break;

		int nStride = -1;
#ifdef _XBOX
		switch(check.m_DeclUsage) {
case BS_USAGE_NORMAL:
	check.m_DeclType = BS_TYPE_HEND3N;
	nStride = sizeof(DWORD);
	break;
case BS_USAGE_TEXCOORD:	
	check.m_DeclType = BS_TYPE_FLOAT16_2;
	nStride = sizeof(D3DXFLOAT16)*2;
	break;
		}
#else
		switch(check.m_DeclUsage) {
case BS_USAGE_NORMAL:
	nStride = sizeof(float)*3;
	break;
case BS_USAGE_TEXCOORD:
	nStride = sizeof(float)*2;
	break;
		}
#endif
		// 에러 보정을 위한 임시 코드 입니다
		/*
		if (check.m_DeclUsage==BS_USAGE_BLENDINDICES) {	
		if (check.m_nUsageIndex/10 > 2) { // m_nMaxBlendWeight>2 인 경우를 강제 보정 합니다
		char str[512];
		sprintf(str, "%02d  wt=%d\n", index,check.m_nUsageIndex/10);
		DebugString(str);

		char bufp_[1024*16];
		int size_ = sizeof(short int) * (check.m_DeclType-BS_TYPE_SHORT2+1) * 2;
		ReadByDeclType(pStream, bufp_, check.m_DeclType, size_);

		continue; // 스킵 합니다
		}
		}
		*/
		/*
		else if (check.m_DeclUsage==BS_USAGE_POSITION || check.m_DeclUsage==BS_USAGE_NORMAL || check.m_DeclUsage==BS_USAGE_TEXCOORD || check.m_DeclUsage==BS_USAGE_BLENDWEIGHT) {	

		}
		else {
		char str[512];
		sprintf(str, "%02d  pass\n", index);
		DebugString(str);

		continue; // 스킵 합니다
		}
		*/


		switch (check.m_DeclUsage) {
			case BS_USAGE_POSITION:
				AllocationStream( check.m_DeclType, BS_USAGE_POSITION, check.m_nUsageIndex, sizeof(float) * 3);
				break;
			case BS_USAGE_NORMAL:
				AllocationStream(check.m_DeclType, BS_USAGE_NORMAL, check.m_nUsageIndex, nStride);
				break;
			case BS_USAGE_TEXCOORD:
				AllocationStream(check.m_DeclType, BS_USAGE_TEXCOORD, check.m_nUsageIndex, nStride);
				break;
			case BS_USAGE_BLENDINDICES:
				AllocationStream(check.m_DeclType, BS_USAGE_BLENDINDICES, check.m_nUsageIndex%10, sizeof(short int) * (check.m_DeclType-BS_TYPE_SHORT2+1) * 2);
				m_nMaxBlendWeight=check.m_nUsageIndex/10;
				break; // BS_TYPE_SHORT2 값이 차례되어 있다고 가정 됩니다, m_nBlend 가 여러번 호출 될 가능성이 있습니다
			case BS_USAGE_BLENDWEIGHT:
				AllocationStream(check.m_DeclType, BS_USAGE_BLENDWEIGHT,  check.m_nUsageIndex,    sizeof(float    ) * (check.m_DeclType-BS_TYPE_FLOAT1+1)    );
				break; // BS_TYPE_FLOAT1 값이 차례되어 있다고 가정 됩니다
			default:
				BsAssert(0 && "지원하지 않는 Usage 입니다");
				break;
		}    
		if(m_nMaxBlendWeight>4) {
			BsAssert(0 && "4 Link까지만 지원합니다.");
		}

		size=m_StreamMaps[index].m_nStride*m_nVertexCount;
		m_StreamMaps[index].m_pVB->Lock(0, size, (void**)&bufp, 0);

		switch(check.m_DeclUsage) {
			case BS_USAGE_NORMAL:
			case BS_USAGE_TEXCOORD:
#ifdef _XBOX
				ReadAndCompressByDeclType(pStream, bufp, check.m_DeclUsage, check.m_DeclType, m_nVertexCount);
				break;
#endif
			default:
				ReadByDeclType(pStream, bufp, check.m_DeclType, size);
				break;
		}
		// 디버깅 용도
		/*
		if (check.m_DeclUsage==BS_USAGE_BLENDWEIGHT) {
		float *pt=(float *)bufp;
		char buf[256];
		int num = size / (int)sizeof(float), wb=0,w1=0,w2=0;
		const int link2=2;
		for (int i=0; i<num; i+=link2) {
		if (m_nMaxBlendWeight==link2) {
		if (i<16*link2) {
		sprintf(buf, "%04d, %f, %f,\n", i/link2, pt[i],pt[i+1]); 
		DebugString(buf); 
		}

		//const float NearZero=0.0000001f; // 51 2
		const float NearZero=0.000001f; // 51 2

		if (fabs(pt[i]+pt[i+1]-1) < NearZero) {
		if      (fabs(pt[i  ]-1) < NearZero) w1++;
		else if (fabs(pt[i+1]-1) < NearZero) w1++;
		else  							     w2++;
		}
		else {
		wb++;
		}
		}
		}
		sprintf(buf, "num_w=%04d  wb=%d  w1=%d  w2=%d  sub=%d\n", num/link2, wb,w1,w2, wb+w1+w2); 
		DebugString(buf); 
		}
		*/
		m_StreamMaps[index].m_pVB->Unlock();
		index++; // 위에서 언제나 스트림이 확장 된다고 가정 합니다
	}

	//-------------
	// 인덱스 버퍼
	//-------------

	int nIndexBufferSize = 0;
	if(m_PrimitiveType==D3DPT_TRIANGLELIST){
		nIndexBufferSize = m_nPrimitiveCount*sizeof(WORD)*3;
		WORD *pSrcFaceBuf;
		WORD *pFaceBuf = new WORD[m_nPrimitiveCount*3];
		pStream->Read(pFaceBuf, nIndexBufferSize, ENDIAN_TWO_BYTE);
		if( m_nMaxBlendWeight == 0) {
			CreateInstancingBuffer( g_BsKernel.GetInstancingMgr()->GetBufferSize(), pFaceBuf );
		}
		g_BsKernel.CreateIndexBuffer( nIndexBufferSize, 0, D3DFMT_INDEX16, D3DPOOL_MANAGED, &m_pIB);
		m_pIB->Lock(0, 0, (void **)&pSrcFaceBuf, 0); // <2> 면읽기
		memcpy(pSrcFaceBuf, pFaceBuf, nIndexBufferSize);
		m_pIB->Unlock();
		delete [] pFaceBuf;
	}
	else if (m_PrimitiveType==D3DPT_TRIANGLESTRIP) {
		nIndexBufferSize = m_nPrimitiveCount*sizeof(WORD);

		WORD *pSrcFaceBuf;
		WORD *pFaceBuf = new WORD[m_nPrimitiveCount];

		pStream->Read(pFaceBuf, nIndexBufferSize, ENDIAN_TWO_BYTE);
#ifdef _XBOX
		/// Make Reset Index Strip
		int i, j;
		for( i = 0, j = 0; i < m_nPrimitiveCount; i++) {
			pFaceBuf[j++] = pFaceBuf[i];
			if( i+3 < m_nPrimitiveCount && pFaceBuf[i] == pFaceBuf[i+1] &&  pFaceBuf[i+2] == pFaceBuf[i+3] ) {	
				if( pFaceBuf[i+3] == pFaceBuf[i+4] ) { 					
					i+=3;
				}
				else {
					i+=2;
				}				
				pFaceBuf[j++] = 0xFFFF;
				if( (i & 1) == 0 ) {
					pFaceBuf[j++] = pFaceBuf[i+1];
				}
			}			
		}
		m_nPrimitiveCount = j;
#endif
		m_nPrimitiveCount-=2;
		if( m_nMaxBlendWeight == 0) {
			CreateInstancingBuffer( g_BsKernel.GetInstancingMgr()->GetBufferSize(), pFaceBuf );
		}

		g_BsKernel.CreateIndexBuffer( nIndexBufferSize, 0, D3DFMT_INDEX16, D3DPOOL_MANAGED, &m_pIB);
		m_pIB->Lock(0, 0, (void **)&pSrcFaceBuf, 0); // <2> 면읽기
		memcpy( pSrcFaceBuf, pFaceBuf, nIndexBufferSize);
		m_pIB->Unlock();
		delete [] pFaceBuf;
	}
	else {
		BsAssert(0 && "지원하지 않는 Primitive 입니다");
	}	

	//-------------
	// 확장
	//-------------

	if (pHeader->nMeshFlag & BM_SUBMESH_HEADER::eMAKE_TANGENT) { // 범프가 있는 서브메쉬만 해당 됩니다
		AllocationStream(BS_TYPE_FLOAT3, BS_USAGE_TANGENT,  0, sizeof(float)*3); // Tangent
		AllocationStream(BS_TYPE_FLOAT3, BS_USAGE_BINORMAL, 0, sizeof(float)*3); // Binormal
		BuildBasisVector();
	}

	return 1;
}

void CBsStream::PreparePhysique(int nBoneCount, BStream* pStream)
{
	m_nBoneCount = nBoneCount;
	m_ppBoneName = new char* [m_nBoneCount];
	m_pBoneIndex = new int [m_nBoneCount];

	for(int i=0;i<m_nBoneCount;i++){
		m_ppBoneName[i]=g_BsKernel.FnaPoolReadAdd(pStream); 
		m_pBoneIndex[i]=-1;
		if (_stricmp(m_ppBoneName[i],"MeshCopy")==0) {
			CBsMesh::s_nDetectMeshCopy=1; 
		}
	}
}

// CreateStream으로 생성되는 Stream은 현재 TriangleList만 지원합니다.
void CBsStream::CreateStream(int nVertexCount, int nFaceCount)
{
	Clear();
	m_bAllocated = false;
	m_PrimitiveType = D3DPT_TRIANGLELIST;

	m_nVertexCount = nVertexCount;
	m_nPrimitiveCount = nFaceCount;
}

void CBsStream::SetBoundingBox(AABB &aabb)
{
	int nPosIdx = FindStreamByUsage(BS_USAGE_POSITION, 0);
	int nPosStride = m_StreamMaps[nPosIdx].m_nStride;
	char* pPosBuf;
	D3DXVECTOR3 *pV0Pos;
	m_StreamMaps[nPosIdx].m_pVB->Lock(0, m_nVertexCount*m_StreamMaps[nPosIdx].m_nStride, (void**)&pPosBuf, 0);

	for(int i=0;i<m_nVertexCount;++i) {
		pV0Pos = (D3DXVECTOR3*)(pPosBuf + i*nPosStride);

		if (aabb.Vmin.x > pV0Pos->x) aabb.Vmin.x = pV0Pos->x;
		if (aabb.Vmin.y > pV0Pos->y) aabb.Vmin.y = pV0Pos->y;
		if (aabb.Vmin.z > pV0Pos->z) aabb.Vmin.z = pV0Pos->z;

		if (aabb.Vmax.x < pV0Pos->x) aabb.Vmax.x = pV0Pos->x;
		if (aabb.Vmax.y < pV0Pos->y) aabb.Vmax.y = pV0Pos->y;
		if (aabb.Vmax.z < pV0Pos->z) aabb.Vmax.z = pV0Pos->z;
	}

	m_StreamMaps[nPosIdx].m_pVB->Unlock();
}

bool CBsStream::GenerateStream(UINT usage, UINT uiIndex/*=0*/)
{
	if (usage==BS_USAGE_TANGENT || usage==BS_USAGE_BINORMAL) {
		AllocationStream(BS_TYPE_FLOAT3, BS_USAGE_TANGENT,  0, sizeof(float)*3); // Tangent
		AllocationStream(BS_TYPE_FLOAT3, BS_USAGE_BINORMAL, 0, sizeof(float)*3); // Binormal
		BuildBasisVector();
	} 
	else {
		return false;
	}

	return true;
}

bool CBsStream::AddStream(BS_DECLTYPE type, BS_DECLUSAGE usage, int nIndex, int nStride)
{
	AllocationStream(type, usage, nIndex, nStride);
	return true;
}

void CBsStream::GetStreamBuffer(D3DXVECTOR3 **ppVertexBuffer, WORD **ppFaceBuffer, int &nVertexCount, int &nFaceCount)
{
	//	assert(m_PrimitiveType == D3DPT_TRIANGLELIST && "스트립은 지원하지 않습니다");
	if( m_PrimitiveType == D3DPT_TRIANGLESTRIP )
	{
		char buf[256];
		sprintf(buf, "GetStreamBuffer를 스트립으로 사용하면 느리다\n" ); 
		DebugString(buf);
	}
	//	     |
	//       D3DPT_TRIANGLELIST          = 4,
	//       D3DPT_TRIANGLESTRIP         = 5,

	nVertexCount=m_nVertexCount;

	WORD* pFaceBuf;
	char* pPosBuf;

	int nPosIdx = FindStreamByUsage(BS_USAGE_POSITION, 0);
	int nPosStride = m_StreamMaps[nPosIdx].m_nStride;

	m_pIB->Lock(0, 0, (void**)&pFaceBuf, 0);
	m_StreamMaps[nPosIdx].m_pVB->Lock(0, m_nVertexCount*m_StreamMaps[nPosIdx].m_nStride, (void**)&pPosBuf, 0);

	BsAssert( (sizeof(D3DXVECTOR3)==nPosStride) && "메모리 사이즈가 다릅니다");

	*ppVertexBuffer=new D3DXVECTOR3 [m_nVertexCount];
	*ppFaceBuffer=new WORD [m_nPrimitiveCount*3];

	memcpy(*ppVertexBuffer, pPosBuf, m_nVertexCount*sizeof(D3DXVECTOR3));

	int nIndex = 0;

	if( m_PrimitiveType == D3DPT_TRIANGLESTRIP )
	{		
		int w = 0;
		bool bSwap = false;	// 삼각형 뒤집어야 하는 경우 true

		int nPreValue1 = pFaceBuf[0];
		int nPreValue2 = pFaceBuf[1];
		nIndex = 2;
		for( int i=0; i<m_nPrimitiveCount; i++ )
		{
			int nValue = pFaceBuf[nIndex];
			if( nPreValue2 == nValue || nPreValue1 == nPreValue2 )
			{
				++nIndex;
				nPreValue1 = pFaceBuf[nIndex-2];
				nPreValue2 = pFaceBuf[nIndex-1];
				bSwap = !bSwap;
				continue;
			}
			if( nValue == 0xFFFF ) // reset index 확인 필요 by realgaia
			{
				nIndex += 3;
				i += 2;
				nPreValue1 = pFaceBuf[nIndex-2];
				nPreValue2 = pFaceBuf[nIndex-1];				
				bSwap = false;
				if( nPreValue1 == nPreValue2 ) {
					++i;
					++nIndex;
					nPreValue1 = pFaceBuf[nIndex-2];
					nPreValue2 = pFaceBuf[nIndex-1];
					bSwap = true;
				}				
				continue;
			}

			if( bSwap )
			{
				(*ppFaceBuffer)[w*3] = nPreValue1;
				(*ppFaceBuffer)[w*3+1] = nValue;
				(*ppFaceBuffer)[w*3+2] = nPreValue2;
			}
			else
			{
				(*ppFaceBuffer)[w*3] = nValue;
				(*ppFaceBuffer)[w*3+1] = nPreValue1;
				(*ppFaceBuffer)[w*3+2] = nPreValue2;
			}

			nPreValue1 = nPreValue2;
			nPreValue2 = nValue;

			++w;
			++nIndex;
			bSwap = !bSwap;
		}
		nFaceCount = w;
	}
	else
	{
		memcpy(*ppFaceBuffer, pFaceBuf, m_nPrimitiveCount*sizeof(WORD)*3);
		nFaceCount = m_nPrimitiveCount;
	}

	m_pIB->Unlock();
	m_StreamMaps[nPosIdx].m_pVB->Unlock();
}

void CBsStream::GetStreamBuffer( D3DXVECTOR3 **ppVertexBuffer, int &nVertexCount )
{
	nVertexCount=m_nVertexCount; 
	char* pPosBuf;

	int nPosIdx = FindStreamByUsage(BS_USAGE_POSITION, 0);
	int nPosStride = m_StreamMaps[nPosIdx].m_nStride;

	m_StreamMaps[nPosIdx].m_pVB->Lock(0, m_nVertexCount*m_StreamMaps[nPosIdx].m_nStride, (void**)&pPosBuf, 0);

	BsAssert( (sizeof(D3DXVECTOR3)==nPosStride) && "메모리 사이즈가 다릅니다");

	*ppVertexBuffer=new D3DXVECTOR3 [m_nVertexCount];
	memcpy(*ppVertexBuffer, pPosBuf, m_nVertexCount*sizeof(D3DXVECTOR3));

	m_StreamMaps[nPosIdx].m_pVB->Unlock();
}


void CBsStream::GetStream( UINT uiUsage, LPDIRECT3DVERTEXBUFFER9& pVB, LPDIRECT3DINDEXBUFFER9& pIB )
{
	int nIndex = FindStreamByUsage(uiUsage);
	pVB = m_StreamMaps[nIndex].m_pVB;
	pIB = m_pIB;
}

void CBsStream::GetStreamMap( UINT uiUsage, BS_VERTEX_ELEMENT*& pVE)
{
	int nIndex = FindStreamByUsage(uiUsage);
	pVE = &(m_StreamMaps[nIndex]);
}

void CBsStream::GetStreamBufferExt(D3DXVECTOR3 **ppPositionBuffer, D3DXVECTOR3 **ppNormalBuffer, int **ppBoneBuffer, int &nVertexCount)
{
	//assert(m_PrimitiveType == D3DPT_TRIANGLELIST && "스트립은 지원하지 않습니다");

	nVertexCount=m_nVertexCount;

	char* pPosBuf;
	D3DXVECTOR3* pNorBuf;
	int* pBonBuf;

	int nPosIdx = FindStreamByUsage(BS_USAGE_POSITION, 0); 
	int nNorIndex = FindStreamByUsage(BS_USAGE_NORMAL, 0);
	int nBlendIndex = FindStreamByUsage(D3DDECLUSAGE_BLENDINDICES, 0);

	BsAssert(nPosIdx!=-1 && "해당 데이타 영역이 없습니다");
	BsAssert(nNorIndex!=-1 && "해당 데이타 영역이 없습니다");
	// assert(nBlendIndex!=-1 && "해당 데이타 영역이 없습니다");	
	
	if(ppPositionBuffer)
	{
		int nPosStride = m_StreamMaps[nPosIdx].m_nStride;
		BsAssert( (sizeof(D3DXVECTOR3)==nPosStride) && "메모리 사이즈가 다릅니다");
		m_StreamMaps[nPosIdx].m_pVB->Lock(0, m_nVertexCount*nPosStride, (void**)&pPosBuf, 0);
		*ppPositionBuffer=new D3DXVECTOR3 [m_nVertexCount];
		memcpy(*ppPositionBuffer, pPosBuf, m_nVertexCount*sizeof(D3DXVECTOR3));
		m_StreamMaps[nPosIdx].m_pVB->Unlock();
	}
	
	if(ppNormalBuffer)
	{
		int nNorStride = m_StreamMaps[nNorIndex].m_nStride;
		BsAssert( (sizeof(D3DXVECTOR3)==nNorStride) && "메모리 사이즈가 다릅니다");
		m_StreamMaps[nNorIndex].m_pVB->Lock(0, m_nVertexCount*nNorStride, (void**)&pNorBuf, 0);
		*ppNormalBuffer=new D3DXVECTOR3 [m_nVertexCount];
		memcpy(*ppNormalBuffer, pNorBuf, m_nVertexCount*sizeof(D3DXVECTOR3));
		m_StreamMaps[nNorIndex].m_pVB->Unlock();
	}

	if (nBlendIndex==-1) {
		if( ppBoneBuffer ) {
			*ppBoneBuffer=NULL;
		}
	}
	else {	
		if(ppBoneBuffer)
		{
			int nBlendStride = m_StreamMaps[nBlendIndex].m_nStride;
			m_StreamMaps[nBlendIndex].m_pVB->Lock(0, m_nVertexCount*nBlendStride, (void**)&pBonBuf, 0);
			BsAssert( (sizeof(int)==nBlendStride) && "메모리 사이즈가 다릅니다");

			*ppBoneBuffer=new int [m_nVertexCount];
			memcpy(*ppBoneBuffer, pBonBuf, m_nVertexCount*sizeof(int));

			m_StreamMaps[nBlendIndex].m_pVB->Unlock();
		}
		
	}
}

int CBsStream::FindStreamByUsage(UINT uiUsage, UINT uiIndex/*=0*/)
{
	for (unsigned int i=0;i<m_StreamMaps.size();++i) {
		if(m_StreamMaps[i].m_DeclUsage==uiUsage && m_StreamMaps[i].m_nUsageIndex==uiIndex) return i;
	}
	return -1;
}

void CBsStream::GetDummyPosition(D3DXVECTOR3 &vecPosition)
{
	int nPosIdx = FindStreamByUsage(BS_USAGE_POSITION, 0);
	D3DXVECTOR3 *pPosBuf;
	m_StreamMaps[nPosIdx].m_pVB->Lock(0, sizeof(D3DXVECTOR3), (void**)&pPosBuf, 0);
	vecPosition = *pPosBuf;
	m_StreamMaps[nPosIdx].m_pVB->Unlock();
}

void CBsStream::SetStreamSource(C3DDevice* pDevice, int nStreamCount, int* pnStreamIndexList)
{
	for(int i=0;i<nStreamCount;++i) {
		if(pnStreamIndexList[i]!=-1) {
			BS_VERTEX_ELEMENT* pInfo = &m_StreamMaps[pnStreamIndexList[i]];
			pDevice->SetStreamSource(i, pInfo->m_pVB, pInfo->m_nStride);
		}
		else {
			pDevice->SetStreamSource(i, NULL, 0);
		}
	}
	pDevice->SetIndices(m_pIB);	
}

void CBsStream::SetStreamSourceForShadowVolume(C3DDevice* pDevice)
{
	if( m_nShadowVolumePrimCount <= 0) return;


	if( m_nMaxBlendWeight == 0) {
#ifdef _XBOX 
		pDevice->SetStreamSource(0, m_pShadowVolumeVB, sizeof(SHADOWVERTEX_STREAM));
#else
		pDevice->SetStreamSource(0, m_pShadowVolumeVB, sizeof(SHADOWVERTEX));
#endif
	}
	else {
#ifdef _XBOX 
		pDevice->SetStreamSource(0, m_pShadowVolumeVB, sizeof(SHADOWVERTEX_BLEND_STREAM));
#else
		pDevice->SetStreamSource(0, m_pShadowVolumeVB, sizeof(SHADOWVERTEX_BLEND));
#endif
	}

	pDevice->SetIndices( m_pShadowVolumeIB );
}

void CBsStream::DrawMeshForShadowVolume( C3DDevice *pDevice )
{
	if( m_nShadowVolumePrimCount <= 0) return;

	pDevice->DrawIndexedMeshVB( D3DPT_TRIANGLELIST, 0, m_nShadowVolumePrimCount, 0, 0 );	
}

void CBsStream::DrawMeshForInstancing(C3DDevice *pDevice, int nStreamCount, int* pnStreamIndexList, LPDIRECT3DVERTEXBUFFER9 pVertexBuffer, int nStartIndex, int nInstancingCount )
{
	int i;
#ifdef _XBOX
	pDevice->SetIndices(m_pInstancingIB);

#else
	pDevice->SetIndices(m_pIB);
#endif

	if( pnStreamIndexList == NULL)	{ // for shadow 
		int nStreamIndex = FindStreamByUsage(BS_USAGE_POSITION, 0);
		BS_VERTEX_ELEMENT* pInfo = &m_StreamMaps[nStreamIndex ];
		pDevice->SetStreamSource(0, pInfo->m_pVB, pInfo->m_nStride);
#ifndef _XBOX 
		pDevice->GetD3DDevice()->SetStreamSourceFreq( 0, D3DSTREAMSOURCE_INDEXEDDATA | nInstancingCount );
#endif
	}
	else {
		for( i = 0; i < nStreamCount;++i) {
			if(pnStreamIndexList[i]!=-1) {
				BS_VERTEX_ELEMENT* pInfo = &m_StreamMaps[pnStreamIndexList[i]];
				pDevice->SetStreamSource(i, pInfo->m_pVB, pInfo->m_nStride);
			}
			else {
				pDevice->SetStreamSource(i, NULL, 0);
			}
#ifndef _XBOX 
			pDevice->GetD3DDevice()->SetStreamSourceFreq( i, D3DSTREAMSOURCE_INDEXEDDATA | nInstancingCount );
#endif
		}
	}

	int nInstancingStreamIndex = nStreamCount;
#ifdef _XBOX

	for( i = 0; i < nInstancingCount; i += m_nInstancingCount ) {
		pDevice->SetStreamSource( nInstancingStreamIndex,  pVertexBuffer, sizeof(D3DXMATRIX) * ( nStartIndex + i ) , sizeof(D3DXMATRIX) );

		int nDrawCount = BsMin( m_nInstancingCount, nInstancingCount - i);

		if( m_PrimitiveType == D3DPT_TRIANGLESTRIP ) {
			int nDrawPrimCount = (m_nPrimitiveCount+3) * nDrawCount - 3;			
			pDevice->DrawIndexedMeshVB( m_PrimitiveType, 0, nDrawPrimCount, 0, 0 );
		}
		else if( m_PrimitiveType == D3DPT_TRIANGLELIST ) {
			int nDrawPrimCount = m_nPrimitiveCount * nDrawCount;
			pDevice->DrawIndexedMeshVB( m_PrimitiveType, 0, nDrawPrimCount , 0, 0 );
		}
	}
	pDevice->SetStreamSource( nInstancingStreamIndex,  NULL, 0);
#else
	pDevice->SetStreamSource( nInstancingStreamIndex,  pVertexBuffer, sizeof(D3DXMATRIX) * nStartIndex, sizeof(D3DXMATRIX) );	
	pDevice->GetD3DDevice()->SetStreamSourceFreq( nInstancingStreamIndex, D3DSTREAMSOURCE_INSTANCEDATA | 1ul );
	pDevice->DrawIndexedMeshVB( m_PrimitiveType, m_nVertexCount, m_nPrimitiveCount , 0, 0 );	
	for( i = 0; i <= nInstancingStreamIndex; i++) {
		pDevice->GetD3DDevice()->SetStreamSourceFreq( i, 1 );
	}		
#endif
}

void CBsStream::CreateInstancingBuffer( int in_nBufferSize, WORD *pIndicesDest )	// mruete: prefix bug 462: renamed paramter
{
#ifdef _XBOX

	int nDrawVertexCount;	

	if( m_PrimitiveType == D3DPT_TRIANGLESTRIP ) {
		nDrawVertexCount = (m_nPrimitiveCount+2)*3;
	}
	else {
		nDrawVertexCount = m_nPrimitiveCount * 3;
	}

	int nMaxInstancingCount = ( 65532 / nDrawVertexCount );
	int nInstancingCount = BsMax(1, BsMin( BsMax(3, in_nBufferSize), nMaxInstancingCount ));

	BsAssert( nInstancingCount > 0);
	SAFE_RELEASE( m_pInstancingIB );
	//////////////////////////////////////////////////////////////////////////

	if( m_PrimitiveType == D3DPT_TRIANGLESTRIP ) {		

		int nBufferSize = ((m_nPrimitiveCount+3) * nInstancingCount - 1)*sizeof(WORD);	

		WORD *pSrcIndices;
		WORD *pIndices = new WORD[ nBufferSize ];
		for (int i = 0; i < nInstancingCount; i++)
		{
			WORD *pStartIndices = &pIndices[(m_nPrimitiveCount+3) * i];
			int nSize = (m_nPrimitiveCount+2);			
			int nAddSize = (i*m_nVertexCount);

			for(int j = 0; j < nSize; j++ ) {
				if( pIndicesDest[j] != 0xFFFF ) {			// reset Index 는 제외
					pStartIndices[j] = pIndicesDest[j] + nAddSize;
				}
				else {
					pStartIndices[j] = 0xFFFF;
				}
			}
			if( i != nInstancingCount - 1) {
				pStartIndices[nSize] = 0xFFFF;	// Reset Index
			}
		}

		g_BsKernel.CreateIndexBuffer(nBufferSize, D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_MANAGED, &m_pInstancingIB);
		m_pInstancingIB->Lock(0, nBufferSize, (void**)&pSrcIndices, 0);		
		memcpy(pSrcIndices, pIndices, nBufferSize);
		m_pInstancingIB->Unlock();
		delete [] pIndices;

	}
	else {		// D3DPT_TRIANGLELIST
		int nBufferSize = (m_nPrimitiveCount * 3 * nInstancingCount) * sizeof(WORD) ;

		WORD *pSrcIndices;
		WORD *pIndices = new WORD[ nBufferSize ];
		for (int i = 0; i < nInstancingCount; i++) 
		{
			WORD *pStartIndices = &pIndices[(m_nPrimitiveCount * 3 * i)];
			int nSize = (m_nPrimitiveCount*3);			
			int nAddSize = (i*m_nVertexCount);
			for( int j = 0; j < nSize; j++) {
				pStartIndices[j] = pIndicesDest[j] + nAddSize;
			}
		}

		g_BsKernel.CreateIndexBuffer( nBufferSize, D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_MANAGED, &m_pInstancingIB);
		m_pInstancingIB->Lock(0, nBufferSize, (void**)&pSrcIndices, 0);		
		memcpy(pSrcIndices, pIndices,  nBufferSize);
		m_pInstancingIB->Unlock();
		delete [] pIndices;
	}

	m_nInstancingCount = nInstancingCount;

#endif
}

D3DPRIMITIVETYPE CBsStream::ConvertPrimitiveType(BS_PRIMITIVE_TYPE type)
{
	switch (type) {
case BS_PT_POINTLIST:		return D3DPT_POINTLIST;
case BS_PT_LINELIST:		return D3DPT_LINELIST;
case BS_PT_LINESTRIP:		return D3DPT_LINESTRIP;
case BS_PT_TRIANGLELIST:	return D3DPT_TRIANGLELIST;
case BS_PT_TRIANGLESTRIP:	return D3DPT_TRIANGLESTRIP;
case BS_PT_TRIANGLEFAN:		return D3DPT_TRIANGLEFAN;
	//case BS_PT_QUADLIST: return // 지논일때 추가 예정
	}

	return (D3DPRIMITIVETYPE)0;
}

D3DDECLTYPE CBsStream::ConvertDeclType(BS_DECLTYPE type)
{
	switch(type) {
case BS_TYPE_FLOAT1:	return D3DDECLTYPE_FLOAT1;
case BS_TYPE_FLOAT2:	return D3DDECLTYPE_FLOAT2;
case BS_TYPE_FLOAT3:	return D3DDECLTYPE_FLOAT3;
case BS_TYPE_FLOAT4:	return D3DDECLTYPE_FLOAT4;
case BS_TYPE_D3DCOLOR:	return D3DDECLTYPE_D3DCOLOR;
case BS_TYPE_UBYTE4:	return D3DDECLTYPE_UBYTE4;
case BS_TYPE_SHORT2:	return D3DDECLTYPE_SHORT2;
case BS_TYPE_SHORT4:	return D3DDECLTYPE_SHORT4;
case BS_TYPE_UBYTE4N:	return D3DDECLTYPE_UBYTE4N;
case BS_TYPE_SHORT2N:	return D3DDECLTYPE_SHORT2N;
case BS_TYPE_SHORT4N:	return D3DDECLTYPE_SHORT4N;
case BS_TYPE_USHORT2N:	return D3DDECLTYPE_USHORT2N;
case BS_TYPE_USHORT4N:	return D3DDECLTYPE_USHORT4N;
case BS_TYPE_UDEC3:		return D3DDECLTYPE_UDEC3;
case BS_TYPE_DEC3N:		return D3DDECLTYPE_DEC3N;
case BS_TYPE_FLOAT16_2:	return D3DDECLTYPE_FLOAT16_2;
case BS_TYPE_FLOAT16_4:	return D3DDECLTYPE_FLOAT16_4;
#ifdef _XBOX
case BS_TYPE_HEND3N:	return D3DDECLTYPE_HEND3N;
#endif
case BS_TYPE_UNUSED:	return D3DDECLTYPE_UNUSED;
	}

	return D3DDECLTYPE_UNUSED;	// 지논일때 추가 예정
}

D3DDECLUSAGE CBsStream::ConvertDeclUsage(BS_DECLUSAGE usage)
{
	switch(usage) {
case BS_USAGE_POSITION:		return D3DDECLUSAGE_POSITION;
case BS_USAGE_BLENDWEIGHT:	return D3DDECLUSAGE_BLENDWEIGHT;
case BS_USAGE_BLENDINDICES:	return D3DDECLUSAGE_BLENDINDICES;
case BS_USAGE_NORMAL:		return D3DDECLUSAGE_NORMAL;
case BS_USAGE_PSIZE:		return D3DDECLUSAGE_PSIZE;
case BS_USAGE_TEXCOORD:		return D3DDECLUSAGE_TEXCOORD;
case BS_USAGE_TANGENT:		return D3DDECLUSAGE_TANGENT;
case BS_USAGE_BINORMAL:		return D3DDECLUSAGE_BINORMAL;
#ifndef _XBOX
case BS_USAGE_TESSFACTOR:	return D3DDECLUSAGE_TESSFACTOR;
case BS_USAGE_POSITIONT:	return D3DDECLUSAGE_POSITIONT;
#endif
case BS_USAGE_COLOR:		return D3DDECLUSAGE_COLOR;
case BS_USAGE_FOG:			return D3DDECLUSAGE_FOG;
case BS_USAGE_DEPTH:		return D3DDECLUSAGE_DEPTH;
case BS_USAGE_SAMPLE:		return D3DDECLUSAGE_SAMPLE;
	}

	return D3DDECLUSAGE_SAMPLE;  // 지논일때 추가 예정
}

void CBsStream::Assign(CBsStream *pLODStream)
{
	BsAssert(m_bAllocated==false && "Data already exist");
	BsAssert(pLODStream->m_bAllocated && "Not allcation");

	m_bAllocated = true;

	m_PrimitiveType = pLODStream->m_PrimitiveType; 

	m_nVertexCount = pLODStream->m_nVertexCount; 
	m_nPrimitiveCount = pLODStream->m_nPrimitiveCount; 

	m_nMaxBlendWeight = pLODStream->m_nMaxBlendWeight; 

	// m_bUseVB (디폴트 false)

	//-------------
	// 버텍스 버퍼
	//-------------

	BS_VERTEX_ELEMENT vertex; 
	int *dp, *bufp, size; 

	for (int i=0; i<(int)pLODStream->m_StreamMaps.size(); i++)  {
		BS_VERTEX_ELEMENT *pt = &pLODStream->m_StreamMaps[i];
		SET_STREAM_COPY(pt->m_DeclType, pt->m_DeclUsage, pt->m_nUsageIndex, pt->m_nStride)

			size=pLODStream->m_StreamMaps[i].m_nStride*m_nVertexCount;
		pLODStream->m_StreamMaps[i].m_pVB->Lock(0, size, (void**)&bufp, 0);
		m_StreamMaps[i].m_pVB->Lock(0, size, (void**)&dp, 0);

		memcpy(dp, bufp, size);

		pLODStream->m_StreamMaps[i].m_pVB->Unlock();
		m_StreamMaps[i].m_pVB->Unlock();
	}

	//-------------
	// 인덱스 버퍼
	//-------------

	if ( m_PrimitiveType == D3DPT_TRIANGLELIST ) {
		g_BsKernel.CreateIndexBuffer(m_nPrimitiveCount*sizeof(WORD)*3, 0, D3DFMT_INDEX16, D3DPOOL_MANAGED, &m_pIB);
		WORD *pFaceBuf, *sp;
		m_pIB->Lock(0, m_nPrimitiveCount*sizeof(WORD)*3, (void **)&pFaceBuf, 0); // <2> 면읽기
		pLODStream->m_pIB->Lock(0, m_nPrimitiveCount*sizeof(WORD)*3, (void **)&sp, 0);	
		memcpy(pFaceBuf, sp, m_nPrimitiveCount*sizeof(WORD)*3);
		if( m_nMaxBlendWeight == 0) {
			CreateInstancingBuffer( g_BsKernel.GetInstancingMgr()->GetBufferSize(), pFaceBuf );
		}
		m_pIB->Unlock();
		pLODStream->m_pIB->Unlock();
	}
	else if ( m_PrimitiveType == D3DPT_TRIANGLESTRIP ) {
		m_nPrimitiveCount += 2;
		g_BsKernel.CreateIndexBuffer(m_nPrimitiveCount*sizeof(WORD), 0, D3DFMT_INDEX16, D3DPOOL_MANAGED, &m_pIB);
		WORD *pFaceBuf, *sp;
		m_pIB->Lock(0, m_nPrimitiveCount*sizeof(WORD), (void **)&pFaceBuf, 0); // <2> 면읽기
		pLODStream->m_pIB->Lock(0, m_nPrimitiveCount*sizeof(WORD), (void **)&sp, 0);	
		memcpy(pFaceBuf, sp, m_nPrimitiveCount*sizeof(WORD));
		if( m_nMaxBlendWeight == 0) {
			CreateInstancingBuffer( g_BsKernel.GetInstancingMgr()->GetBufferSize(), pFaceBuf );
		}
		m_pIB->Unlock();
		pLODStream->m_pIB->Unlock();
		m_nPrimitiveCount -= 2;
	}

	// Physique Stream 처리!!
	m_nBoneCount = pLODStream->GetBoneCount();
	if(m_nBoneCount>0) {
		m_ppBoneName = new char* [m_nBoneCount];
		m_pBoneIndex = new int [m_nBoneCount];

		for(int i=0;i<m_nBoneCount;i++){
			m_ppBoneName[i]=g_BsKernel.FnaPoolAdd(pLODStream->GetBoneName(i));
			m_pBoneIndex[i]=pLODStream->GetBoneIndex(i);
			//			if (stricmp(m_ppBoneName[i],"MeshCopy")==0) {
			//				CBsMesh::s_nDetectMeshCopy=1; 
			//			}
		}

	}
}

void CBsStream::ChangeBlendValue(int v1, int v2) // 본정보가 서로 다른 BM 을 강제로 일치 시킬때 사용 됩니다
{
	int nBlenIndex = FindStreamByUsage(BS_USAGE_BLENDINDICES, 0);	
	int nBlenStride = m_StreamMaps[nBlenIndex].m_nStride;

	short int *pBlen;
	m_StreamMaps[nBlenIndex].m_pVB->Lock(0, m_nVertexCount*nBlenStride, (void**)&pBlen, 0);

	int nSize=(int)(m_nVertexCount*nBlenStride/sizeof(short int));
	for (int i=0; i<nSize; i++) {
		if (pBlen[i]==v1) { // 해당하는 것은 모두 바꿉니다
			pBlen[i]=v2;
		}
	}

	m_StreamMaps[nBlenIndex].m_pVB->Unlock();
}

void CBsStream::LinkBone(CBsAni* pAni)
{
	for (int i=0;i<m_nBoneCount;i++){
		m_pBoneIndex[i]=pAni->FindBoneIndex(m_ppBoneName[i]);
	}
}

void CBsStream::LinkBone(const std::vector< std::string > & szBoneList ) // mruete: prefix bug 463: changed to pass by const reference
{
	int i, j, nCount; 

	nCount = szBoneList.size();
	for( i = 0; i < m_nBoneCount; i++ ) {
		for( j = 0; j < nCount; j++ ) {
			if( _stricmp( szBoneList[ j ].c_str(), m_ppBoneName[i] ) == 0 ) {
				m_pBoneIndex[ i ] = j;
				break;
			}
		}
	}
}

#if _OPT_SHINJICH_BSSTREAM
void CBsStream::TransformBoneMatrices(BSMATRIX matWorldView, D3DXMATRIX *pBoneMatrix )
{
	BSMATRIX * __restrict pSharedMatrices = CBsPhysiqueSubMesh::GetSharedMatrices();

	if( pBoneMatrix == NULL )
	{
		for(int i=0;i<m_nBoneCount;i++) { // 모든 본을 Looping Mat를 Constant로 입력
			CBsPhysiqueSubMesh::GetSharedMatrices()[i] = matWorldView;
		}
	}
	else
	{
		// insert prefetch by shinjich
		int prefetch;
		for( prefetch = 0; prefetch < 128*8; prefetch += 128 )
		{
			__dcbt( prefetch, pBoneMatrix );
		}

		for(int i=0;i<m_nBoneCount;i++) { // 모든 본을 Looping Mat를 Constant로 입력
			int nBoneIndex = m_pBoneIndex[ i ];
			if( (nBoneIndex + 1) ) 
			{
				BsMatrixMultiply( &pSharedMatrices[i], ( BSMATRIX * )&pBoneMatrix[ nBoneIndex ], &matWorldView );
			}
			else {
				CBsPhysiqueSubMesh::GetSharedMatrices()[i] = matWorldView;
			}
		}
	}
}
#else
void CBsStream::TransformBoneMatrices(BSMATRIX matWorldView, D3DXMATRIX *pBoneMatrix )
{
	BSMATRIX *pSharedMatrices = CBsPhysiqueSubMesh::GetSharedMatrices();
	for(int i=0;i<m_nBoneCount;i++) { // 모든 본을 Looping Mat를 Constant로 입력
		int nBoneIndex = m_pBoneIndex[ i ];
		if( ( pBoneMatrix != NULL ) && ( nBoneIndex != -1 ) ) 
		{
			BsMatrixMultiply( &pSharedMatrices[i], ( BSMATRIX * )&pBoneMatrix[ nBoneIndex ], &matWorldView );
		}
		else {
			CBsPhysiqueSubMesh::GetSharedMatrices()[i] = matWorldView;
		}
	}
}
#endif

void CBsStream::SplitStream( std::vector<CBsStream*> &SplitStreamList, int nMaxBoneCount)
{
	if( m_PrimitiveType == D3DPT_TRIANGLESTRIP ) {
		SplitStreamStrip(SplitStreamList, nMaxBoneCount);
		return;
	}

	int i, j, m, n;
	int *pBoneBuf, *pBoneIndex = new int [m_nVertexCount];
	WORD *pFaceBuf;
	WORD *pIndexBuffer = new WORD [m_nPrimitiveCount * 3];

	int nBlendIndex = FindStreamByUsage(D3DDECLUSAGE_BLENDINDICES, 0);
	m_StreamMaps[nBlendIndex].m_pVB->Lock(0, m_nVertexCount*sizeof(int), (void**)&pBoneBuf, 0);	
	memcpy(pBoneIndex, pBoneBuf, m_nVertexCount*sizeof(int));
	m_StreamMaps[nBlendIndex].m_pVB->Unlock();

	int nReadSize = m_nPrimitiveCount * sizeof(WORD) * 3;
	m_pIB->Lock(0, nReadSize, (void **)&pFaceBuf, 0);
	memcpy(pIndexBuffer, pFaceBuf, nReadSize);
	m_pIB->Unlock();

	const int cMaxSplitCount = 8;

	int faceCount[cMaxSplitCount] = {0,};
	WORD *faceBuffer[cMaxSplitCount];
	int SaveBoneList[cMaxSplitCount][255] = {0,};

	for( i = 0; i < cMaxSplitCount; i++) {
		faceBuffer[i] = new WORD[m_nPrimitiveCount];
	}

	char *pUseTable = new char[m_nPrimitiveCount];
	memset(pUseTable, -1, m_nPrimitiveCount);

	int nn = sizeof(FaceTable);
	FaceTable *pTable = new FaceTable[m_nPrimitiveCount];
	memset(pTable, 0, sizeof(FaceTable) * m_nPrimitiveCount );

	DWORD *pVertexUseTable = new DWORD[m_nVertexCount];
	memset(pVertexUseTable, 0, sizeof(DWORD)*m_nVertexCount);

	for( i = 0; i < m_nPrimitiveCount; i++) {

		DWORD nBone1 = pBoneIndex[ pIndexBuffer[i * 3 + 0]] ;
		DWORD nBone2 = pBoneIndex[ pIndexBuffer[i * 3 + 1]] ;
		DWORD nBone3 = pBoneIndex[ pIndexBuffer[i * 3 + 2]] ;

		WORD insertList[6] = {	HIWORD(nBone1), LOWORD(nBone1), 
			HIWORD(nBone2), LOWORD(nBone2), 
			HIWORD(nBone3), LOWORD(nBone3), };

		if( insertList[1] == 0 ) insertList[1] = 0xffff;
		if( insertList[3] == 0 ) insertList[3] = 0xffff;
		if( insertList[5] == 0 ) insertList[5] = 0xffff;

		FaceTable &Table = pTable[i];
		for( j = 0; j < 6; j++){			
			for(m = 0;m < Table.nCount; m++) {
				if(Table.nList[m] == insertList[j] ) break;
			}
			if(m == Table.nCount && insertList[j] != 0xffff) {
				Table.nList[Table.nCount] = (BYTE)insertList[j];
				Table.nCount++;
			}
		}
	}

	std::vector<int*> SplitBoneList;

	for( i = 0,j = 0; ; j++) {
		int nBoneList[255];
		int nBoneCount = 0;
		for( ; i < m_nPrimitiveCount; i++) {
			if( pUseTable[i] != -1) continue;

			int bNotUse[6] = {0,};
			int addCount = pTable[i].nCount;
			for( m = 0; m < nBoneCount; m++ ) {
				for( n = 0; n < pTable[i].nCount; n++){
					if( nBoneList[m] == pTable[i].nList[n] ) {
						bNotUse[n] = 1;
						addCount--;
					}
				}
			}

			if( addCount + nBoneCount > nMaxBoneCount ) {

				std::sort(nBoneList, nBoneList+nBoneCount);

				int *pBoneList = new int [ nBoneCount + 1];
				memcpy(&pBoneList[1], nBoneList, sizeof(int) * nBoneCount);
				pBoneList[0] = nBoneCount;
				SplitBoneList.push_back( pBoneList);


				for(m = 0; m < nBoneCount; m++) {
					SaveBoneList[j][ nBoneList[m] ] = m;
				}

				for( m = i; m < m_nPrimitiveCount; m++) {
					if(pUseTable[m] != -1) continue;

					int next_m = 0;
					n = 0;
					for( ; next_m < nBoneCount; next_m++ ) {
						if( nBoneList[next_m] == pTable[m].nList[n] ) {
							n++;
							if( n == pTable[m].nCount ) {
								pUseTable[m] = j;

								pVertexUseTable[ pIndexBuffer[m * 3 + 0] ] |= 1<<j;
								pVertexUseTable[ pIndexBuffer[m * 3 + 1] ] |= 1<<j;
								pVertexUseTable[ pIndexBuffer[m * 3 + 2] ] |= 1<<j;

								faceBuffer[j][ faceCount[j]++ ] = m;								

								break;
							}
							next_m = 0;
						}
					}
				}
				nBoneCount = 0;
				break;
			}
			pUseTable[i] = j;
			pVertexUseTable[ pIndexBuffer[i * 3 + 0] ] |= 1<<j;
			pVertexUseTable[ pIndexBuffer[i * 3 + 1] ] |= 1<<j;
			pVertexUseTable[ pIndexBuffer[i * 3 + 2] ] |= 1<<j;

			faceBuffer[j][ faceCount[j]++ ] = i;

			for(m = 0; m < pTable[i].nCount; m++) {
				if(bNotUse[m]) continue;
				nBoneList[nBoneCount++] = pTable[i].nList[m];
			}
		}

		if(i == m_nPrimitiveCount) {
			std::sort(nBoneList, nBoneList+nBoneCount);

			int *pBoneList = new int [ nBoneCount + 1];
			memcpy(&pBoneList[1], nBoneList, sizeof(int) * nBoneCount);
			pBoneList[0] = nBoneCount;
			SplitBoneList.push_back( pBoneList);

			for(m = 0; m < nBoneCount; m++) {
				SaveBoneList[j][ nBoneList[m] ] = m;
			}
			break;
		}
	}

	int nSplitNum = j+1;
	int nStreamSize = m_StreamMaps.size();

	SAFE_DELETE_PVEC(SplitStreamList);
	for(i = 0; i < nSplitNum; i++) {

		CBsStream *pStream = new CBsStream;
		pStream->m_bAllocated = true;
		pStream->m_PrimitiveType = m_PrimitiveType;
		pStream->m_nMaxBlendWeight = m_nMaxBlendWeight;
		//pStream->m_bUseVB = m_bUseVB;
		pStream->m_StreamMaps.resize(nStreamSize);
		pStream->m_nMaxBlendWeight = m_nMaxBlendWeight;
		pStream->m_nBoneCount = SplitBoneList[i][0];
		pStream->m_pBoneIndex = new int[ pStream->m_nBoneCount ];
		memcpy(pStream->m_pBoneIndex,  &SplitBoneList[i][1], sizeof(int) * pStream->m_nBoneCount);

		delete [] SplitBoneList[i];

		SplitStreamList.push_back( pStream );
	}

	void **pReadBuf = new void*[nStreamSize];

	for( i = 0; i < nStreamSize; i++ ) {
		m_StreamMaps[i].m_pVB->Lock(0, m_nVertexCount*m_StreamMaps[i].m_nStride, &pReadBuf[i], 0);	

		for( j = 0; j < nSplitNum; j++) {
			SplitStreamList[j]->m_StreamMaps[i] = m_StreamMaps[i];
			SplitStreamList[j]->m_StreamMaps[i].m_pVB = NULL;
		}
	}

	for( j = 0; j < nSplitNum; j++) {

		CBsStream *pStream = SplitStreamList[j];

		DWORD nBit = 1<<j;

		for( m = 0; m < nStreamSize; m++)	 {

			int nStride = m_StreamMaps[m].m_nStride;
			unsigned char *pBuffer = new unsigned char[ m_nVertexCount * nStride ]; // 임시로 크게 잡음

			int nCount = 0;
			for( i = 0; i < m_nVertexCount; i++ ) {
				if( pVertexUseTable[ i ] & nBit ) {
					memcpy(pBuffer + nCount * nStride, (char*)pReadBuf[m] + i * nStride, nStride );
					nCount++;
				}
			}

			// BlendIndices 인경우 재정렬 해준다.
			if(m_StreamMaps[m].m_DeclUsage == D3DDECLUSAGE_BLENDINDICES && m_StreamMaps[m].m_nUsageIndex == 0)  {
				int *pTmpBoneIndex = (int*)pBuffer;
				for(i = 0; i < nCount; i++) {
					WORD high = HIWORD(pTmpBoneIndex[i]);
					WORD low = LOWORD(pTmpBoneIndex[i]);

					if(low != 0) low = SaveBoneList[j][low];
					high = SaveBoneList[j][high];

					pTmpBoneIndex[i] = MAKELONG( low , high);
				}
			}

			void *pLockBuf;			

			LPDIRECT3DVERTEXBUFFER9 &pVB = pStream->m_StreamMaps[m].m_pVB;

			g_BsKernel.CreateVertexBuffer(nCount*nStride, 0, 0, D3DPOOL_MANAGED, &pVB);

			pVB->Lock(0, nCount * nStride, &pLockBuf, 0);
			memcpy(pLockBuf, pBuffer, nCount * nStride);
			pVB->Unlock();
			delete [] pBuffer;
		}

		// index buffer
		WORD *pVertIndex = new WORD[m_nVertexCount];
		memset(pVertIndex, 0xff, m_nVertexCount * sizeof(WORD));
		int nCount = 0;
		for( i = 0; i < m_nVertexCount; i++ ) {
			if( pVertexUseTable[ i ] & nBit ) {
				pVertIndex[i] = nCount;
				nCount++;
			}
		}
		pStream->m_nVertexCount = nCount;
		pStream->m_nPrimitiveCount = faceCount[j];

		int nIndexLockSize = faceCount[j]*sizeof(WORD)*3;

		WORD *pFaceBuffer = new WORD [ nIndexLockSize ];
		for( i = 0; i < faceCount[j] ; i++) {
			pFaceBuffer[i * 3 + 0] = pVertIndex [ pIndexBuffer[ faceBuffer[j][i] * 3 + 0 ] ];
			pFaceBuffer[i * 3 + 1] = pVertIndex [ pIndexBuffer[ faceBuffer[j][i] * 3 + 1 ] ];
			pFaceBuffer[i * 3 + 2] = pVertIndex [ pIndexBuffer[ faceBuffer[j][i] * 3 + 2 ] ];
		}

		g_BsKernel.CreateIndexBuffer( nIndexLockSize, 0, D3DFMT_INDEX16, D3DPOOL_MANAGED, &pStream->m_pIB);
		pStream->m_pIB->Lock(0, nIndexLockSize, (void **)&pFaceBuf, 0);
		memcpy(pFaceBuf, pFaceBuffer, nIndexLockSize);
		pStream->m_pIB->Unlock();

		delete [] pFaceBuffer;
		delete [] pVertIndex;

	}

	for( i = 0; i < nStreamSize; i++ ) {
		m_StreamMaps[i].m_pVB->Unlock();
	}

	for(i = 0; i < cMaxSplitCount; i++) {
		delete  [] faceBuffer[i];
	}
	delete [] pIndexBuffer;
	delete [] pBoneIndex;
	delete [] pReadBuf;
	delete [] pUseTable;
	delete [] pTable;
	delete [] pVertexUseTable;
}

void CBsStream::SplitStreamStrip( std::vector<CBsStream*> &SplitStreamList, int nMaxBoneCount)
{
	int i, j, m;
	int *pBoneBuf, *pBoneIndex = new int [m_nVertexCount];
	WORD *pFaceBuf;
	int nPrimCount = m_nPrimitiveCount + 2;
	WORD *pIndexBuffer = new WORD [nPrimCount];

	// blend indices 버퍼 읽음 (영향받는 본을 알기위함.)
	int nBlendIndex = FindStreamByUsage(D3DDECLUSAGE_BLENDINDICES, 0);
	m_StreamMaps[nBlendIndex].m_pVB->Lock(0, m_nVertexCount*sizeof(int), (void**)&pBoneBuf, 0);	
	memcpy(pBoneIndex, pBoneBuf, m_nVertexCount*sizeof(int));
	m_StreamMaps[nBlendIndex].m_pVB->Unlock();

	// strip index 버퍼 읽음
	int nReadSize = nPrimCount * sizeof(WORD);
	m_pIB->Lock(0, nReadSize, (void **)&pFaceBuf, 0);
	memcpy(pIndexBuffer, pFaceBuf, nReadSize);
	m_pIB->Unlock();

	struct StripPart
	{
		int nStartIndex;
		int nStripSize;
		int BoneList[255];
		int nBoneCount;
	};

	int nStripPartCount = 0;
	for( i = 0; i < nPrimCount-3; i++) {
		if( pIndexBuffer[i] == pIndexBuffer[i+1] &&  pIndexBuffer[i+2] == pIndexBuffer[i+3] ){
			nStripPartCount++;
		}
	}
	nStripPartCount++;

	// 합쳐져 있는 스트립을 쪼갠다.
	StripPart *pStripList = new StripPart[ nStripPartCount ];

	nStripPartCount = 0;	
	int nBeforeStartIndex = 0;
	pStripList[0].nBoneCount = 0;

	for( i = 0; i < nPrimCount;  ) {

		WORD high = HIWORD( pBoneIndex[ pIndexBuffer[i]]);
		WORD low = LOWORD( pBoneIndex[ pIndexBuffer[i]]);

		int &nBoneCount = pStripList[ nStripPartCount ].nBoneCount;
		for( j = 0 ; j < nBoneCount; j++) {
			if( pStripList[ nStripPartCount ].BoneList[ j ] == high) break;
		}
		if( j == nBoneCount) {
			pStripList[ nStripPartCount ].BoneList[ nBoneCount++ ] = high;
		}

		if( low != 0) {
			for( j = 0 ; j < nBoneCount; j++) {
				if( pStripList[ nStripPartCount ].BoneList[ j ] == low) break;
			}
			if( j == nBoneCount) {
				pStripList[ nStripPartCount ].BoneList[ nBoneCount++ ] = low;
			}
		}

		if( i+3 < nPrimCount && pIndexBuffer[i] == pIndexBuffer[i+1] &&  pIndexBuffer[i+2] == pIndexBuffer[i+3]){
			pStripList[ nStripPartCount ].nStartIndex = nBeforeStartIndex;
			pStripList[ nStripPartCount ].nStripSize = i - nBeforeStartIndex + 1;

			nBeforeStartIndex = i + 3;
			if(  i+4 < nPrimCount && pIndexBuffer[i+3] == pIndexBuffer[i+4] ) {
				nBeforeStartIndex ++;
			}

			nStripPartCount++;
			pStripList[ nStripPartCount ].nBoneCount = 0;
			i = nBeforeStartIndex;
		}
		else {
			i++;
		}
	}
	pStripList[ nStripPartCount ].nStartIndex = nBeforeStartIndex;
	pStripList[ nStripPartCount ].nStripSize = nPrimCount - nBeforeStartIndex;
	nStripPartCount++;

	// 본리스트들을 소팅한다
	for( i = 0; i < nStripPartCount; i++) {
		std::sort( pStripList[i].BoneList, pStripList[i].BoneList+pStripList[i].nBoneCount);
	}

	// 나눠진 스트립을 본제한에 맞춰서 합치기
	char *pUseTable = new char [nStripPartCount];
	memset(pUseTable, -1, nStripPartCount);

	const int cMaxSplitCount = 8;
	int BoneList[255]={0,};
	int nBoneCount = 0;
	std::vector<int*> SplitBoneList;
	int SaveBoneList[cMaxSplitCount][255] = {0,};

	DWORD *pVertexUseTable = new DWORD[m_nVertexCount];
	memset(pVertexUseTable, 0, sizeof(DWORD)*m_nVertexCount);

	int faceCount[cMaxSplitCount] = {0,};
	WORD *faceBuffer[cMaxSplitCount];
	for( i = 0; i < cMaxSplitCount; i++) {
		faceBuffer[i] = new WORD[nStripPartCount];
	}

	int nSplit;
	for( nSplit = 0, i = 0;  ; nSplit++) {

		for( ; i < nStripPartCount; i++) {

			if( pUseTable[i] != -1) continue;

			int TempBoneList[255];

			int *pEnd = std::set_union(pStripList[i].BoneList, pStripList[i].BoneList+pStripList[i].nBoneCount, BoneList, BoneList+nBoneCount, TempBoneList);          
			int nCount = pEnd - TempBoneList;

			if(nCount <= nMaxBoneCount) {
				pUseTable[i] = nSplit;
				for(m = 0; m < pStripList[i].nStripSize; m++) {
					pVertexUseTable[ pIndexBuffer[ pStripList[i].nStartIndex + m ] ] |= 1 << nSplit;
				}				
				faceBuffer[nSplit][ faceCount[nSplit]++ ] = i;

				memcpy(BoneList, TempBoneList, sizeof(int) * nCount);
				nBoneCount = nCount;
			}
			else {
				for( j = i+1; j < nStripPartCount; j++) {
					int TempBoneList[255];
					int *pEnd = std::set_union(pStripList[j].BoneList, pStripList[j].BoneList+pStripList[j].nBoneCount, BoneList, BoneList+nBoneCount, TempBoneList);
					int nCount = pEnd - TempBoneList;
					if( nCount <= nMaxBoneCount ) {
						pUseTable[j] = nSplit;
						for(m = 0; m < pStripList[j].nStripSize; m++) {
							pVertexUseTable[ pIndexBuffer[ pStripList[j].nStartIndex + m ] ] |= (1 << nSplit);
						}	
						faceBuffer[nSplit][ faceCount[nSplit]++ ] = j;

						memcpy(BoneList, TempBoneList, sizeof(int) * nCount);
						nBoneCount = nCount;
					}
				}

				//
				std::sort(BoneList, BoneList + nBoneCount);
				int *pBoneList = new int [ nBoneCount + 1];
				memcpy(&pBoneList[1], BoneList, sizeof(int) * nBoneCount);
				pBoneList[0] = nBoneCount;
				SplitBoneList.push_back( pBoneList);
				for(j = 0; j < nBoneCount; j++) {
					SaveBoneList[nSplit][ BoneList[j] ] = j;
				}
				nBoneCount = 0;
				break;
			}
		}

		if( i == nStripPartCount ) {
			std::sort(BoneList, BoneList + nBoneCount);
			int *pBoneList = new int [ nBoneCount + 1];
			memcpy(&pBoneList[1], BoneList, sizeof(int) * nBoneCount);
			pBoneList[0] = nBoneCount;
			SplitBoneList.push_back( pBoneList);
			for(j = 0; j < nBoneCount; j++) {
				SaveBoneList[nSplit][ BoneList[j] ] = j;
			}
			break;
		}
	}

	nSplit++;
	int nStreamSize = m_StreamMaps.size();

	SAFE_DELETE_PVEC(SplitStreamList);
	for(i = 0; i < nSplit; i++) {

		CBsStream *pStream = new CBsStream;
		pStream->m_bAllocated = true;
		pStream->m_PrimitiveType = m_PrimitiveType;
		pStream->m_nMaxBlendWeight = m_nMaxBlendWeight;
		//pStream->m_bUseVB = m_bUseVB;
		pStream->m_StreamMaps.resize(nStreamSize);
		pStream->m_nMaxBlendWeight = m_nMaxBlendWeight;
		pStream->m_nBoneCount = SplitBoneList[i][0];
		pStream->m_pBoneIndex = new int[ pStream->m_nBoneCount ];
		memcpy(pStream->m_pBoneIndex,  &SplitBoneList[i][1], sizeof(int) * pStream->m_nBoneCount);
		delete [] SplitBoneList[i];

		SplitStreamList.push_back( pStream );
	}

	void **pReadBuf = new void*[nStreamSize];

	for( i = 0; i < nStreamSize; i++ ) {
		m_StreamMaps[i].m_pVB->Lock(0, m_nVertexCount*m_StreamMaps[i].m_nStride, &pReadBuf[i], 0);	

		for( j = 0; j < nSplit; j++) {
			SplitStreamList[j]->m_StreamMaps[i] = m_StreamMaps[i];
			SplitStreamList[j]->m_StreamMaps[i].m_pVB = NULL;
		}
	}

	for( j = 0; j < nSplit; j++) {

		CBsStream *pStream = SplitStreamList[j];

		DWORD nBit = 1<<j;

		for( m = 0; m < nStreamSize; m++)	 {

			int nStride = m_StreamMaps[m].m_nStride;
			unsigned char *pBuffer = new unsigned char[ m_nVertexCount * nStride ]; // 임시로 크게 잡음

			int nCount = 0;
			for( i = 0; i < m_nVertexCount; i++ ) {
				if( pVertexUseTable[ i ] & nBit ) {
					memcpy(pBuffer + nCount * nStride, (char*)pReadBuf[m] + i * nStride, nStride );
					nCount++;
				}
			}
			// BlendIndices 인경우 재정렬 해준다.
			if(m_StreamMaps[m].m_DeclUsage == D3DDECLUSAGE_BLENDINDICES && m_StreamMaps[m].m_nUsageIndex == 0)  {
				int *pTmpBoneIndex = (int*)pBuffer;
				for(i = 0; i < nCount; i++) {
					WORD high = HIWORD(pTmpBoneIndex[i]);
					WORD low = LOWORD(pTmpBoneIndex[i]);
					if(low != 0) low = SaveBoneList[j][low];
					high = SaveBoneList[j][high];
					pTmpBoneIndex[i] = MAKELONG( low , high);
				}
			}

			void *pLockBuf;			

			LPDIRECT3DVERTEXBUFFER9 &pVB = pStream->m_StreamMaps[m].m_pVB;

			g_BsKernel.CreateVertexBuffer(nCount*nStride, 0, 0, D3DPOOL_MANAGED, &pVB);

			pVB->Lock(0, nCount * nStride, &pLockBuf, 0);
			memcpy(pLockBuf, pBuffer, nCount * nStride);
			pVB->Unlock();
			delete [] pBuffer;
		}

		// index buffer			
		WORD *pVertIndex = new WORD[m_nVertexCount];
		memset(pVertIndex, 0xff, m_nVertexCount * sizeof(WORD));
		int nCount = 0;
		for( i = 0; i < m_nVertexCount; i++ ) {
			if( pVertexUseTable[ i ] & nBit ) {
				pVertIndex[i] = nCount;
				nCount++;
			}
		}
		pStream->m_nVertexCount = nCount;

		WORD *pFaceBuffer = new WORD [ nPrimCount ];
		int nPrimIndex = 0;
		int nLastIndex = -1;
		for( i = 0; i < faceCount[j]; i++) {
			int nStripIndex = faceBuffer[j][i];

			if(nLastIndex != -1) {
				pFaceBuffer[nPrimIndex++] = nLastIndex;
				pFaceBuffer[nPrimIndex++] = pIndexBuffer[pStripList[nStripIndex].nStartIndex];
				if ((nPrimIndex & 1) != (pStripList[nStripIndex].nStartIndex & 1)) { // CCW, CW 와 Index가 짝수,홀수 인지 여부에 따라서 같은버텍스의 추가 여부를 결정					
					pFaceBuffer[nPrimIndex++] = pIndexBuffer[pStripList[nStripIndex].nStartIndex];
				}				
			}
			memcpy(pFaceBuffer + nPrimIndex,  &pIndexBuffer[pStripList[nStripIndex].nStartIndex],  pStripList[nStripIndex].nStripSize * sizeof(WORD));
			nPrimIndex += pStripList[nStripIndex].nStripSize;
			nLastIndex = pIndexBuffer[pStripList[nStripIndex].nStartIndex + pStripList[nStripIndex].nStripSize -1];
		}
		pStream->m_nPrimitiveCount = nPrimIndex-2;

		for( i = 0; i < nPrimIndex; i++) {
			pFaceBuffer[i] = pVertIndex[ pFaceBuffer[i] ];
		}

		int nIndexLockSize = nPrimIndex*sizeof(WORD);

		g_BsKernel.CreateIndexBuffer( nIndexLockSize, 0, D3DFMT_INDEX16, D3DPOOL_MANAGED, &pStream->m_pIB);
		pStream->m_pIB->Lock(0, nIndexLockSize, (void **)&pFaceBuf, 0);
		memcpy(pFaceBuf, pFaceBuffer, nIndexLockSize);
		pStream->m_pIB->Unlock();

		delete [] pFaceBuffer;
		delete [] pVertIndex;
	}

	for( i = 0; i < nStreamSize; i++ ) {
		m_StreamMaps[i].m_pVB->Unlock();
	}

	for(i = 0; i < cMaxSplitCount; i++) {
		delete  [] faceBuffer[i];
	}

	delete [] pUseTable;
	delete [] pVertexUseTable;
	delete [] pIndexBuffer;
	delete [] pStripList;
	delete [] pReadBuf;
}

void CBsStream::AllocationStream(BS_DECLTYPE type, BS_DECLUSAGE usage, UINT uiUsageIndex, int nStride)
{
	BS_VERTEX_ELEMENT element;
	element.m_DeclType = ConvertDeclType(type);
	element.m_DeclUsage= ConvertDeclUsage(usage);
	element.m_nUsageIndex = uiUsageIndex;
	element.m_nStride = nStride;
	g_BsKernel.CreateVertexBuffer(m_nVertexCount*element.m_nStride, 0, 0, D3DPOOL_MANAGED, &element.m_pVB);
	m_StreamMaps.push_back(element);
}

bool CBsStream::GenerateShadowVolume()
{
	if( m_nShadowVolumePrimCount != 0 ) return false;

	//double a = GetPerformanceTime();
	// Lock index and vertex buffers.
	D3DXVECTOR3* pMeshVertices;
	WORD* pMeshIndices;

	int nVertexCount = 0;
	int nPrimCount  = 0;

	GetStreamBuffer(&pMeshVertices, &pMeshIndices, nVertexCount, nPrimCount);

	bool bResult = false;

	if( m_nMaxBlendWeight == 0) {
		bResult = CBsShadowVolumeMgr::GenerateShadowMesh( pMeshVertices, pMeshIndices, nVertexCount, nPrimCount, &m_pShadowVolumeVB, &m_pShadowVolumeIB, &m_nShadowVolumePrimCount);
	}
	else if( m_nMaxBlendWeight >= 1) {
		SHADOWVERTEX_BLEND *pShadowVertex = new SHADOWVERTEX_BLEND[ nVertexCount ];
		int nBlendIndices = FindStreamByUsage(BS_USAGE_BLENDINDICES, 0);
		int nBlenStride = m_StreamMaps[nBlendIndices].m_nStride;		
		int *pBlen;
		m_StreamMaps[nBlendIndices].m_pVB->Lock(0, nVertexCount * nBlenStride, (void**)&pBlen, 0);
		for( int i = 0; i < nVertexCount; i++ ){
			pShadowVertex[i].Position = pMeshVertices[i];
			pShadowVertex[i].Normal = D3DXVECTOR3(0,0,0);
			pShadowVertex[i].nBlendIndices = pBlen[i];
		}
		m_StreamMaps[nBlendIndices].m_pVB->Unlock();
		bResult = CBsShadowVolumeMgr::GenerateShadowMeshForBlend( pShadowVertex, pMeshIndices, nVertexCount, nPrimCount, &m_pShadowVolumeVB, &m_pShadowVolumeIB, &m_nShadowVolumePrimCount);
		delete [] pShadowVertex;		
	}

	delete [] pMeshVertices;
	delete [] pMeshIndices;

	if( !bResult ) {
		m_nShadowVolumePrimCount = -1;
		//DebugString("ShadowVolume Fail\n");
		return false;
	}
	//DebugString("ShadowVolume Success\n");
	//double b = GetPerformanceTime();
	//static double c = 0.f;
	//c += (b-a);
	//DebugString( "ShadowVolumeTime = %lf \n", c);
	return true;
}

