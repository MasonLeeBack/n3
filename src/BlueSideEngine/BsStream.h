#pragma once

#include "3DDevice.h"

struct AABB;
class BStream;
class CBsAni;
class CBsObject;

//------------------------------------------------------------------------------------------------

struct BS_VERTEX_ELEMENT
{
	D3DDECLTYPE				m_DeclType;
	D3DDECLUSAGE			m_DeclUsage;
	UINT					m_nUsageIndex;			
	int						m_nStride;
	LPDIRECT3DVERTEXBUFFER9	m_pVB;
};

struct BS_VERTEX_CHECK {
	BS_DECLTYPE				m_DeclType;
	BS_DECLUSAGE			m_DeclUsage;
	UINT					m_nUsageIndex;			

	void set(BS_DECLTYPE typ, BS_DECLUSAGE usg, UINT idx) { m_DeclType=typ, m_DeclUsage=usg, m_nUsageIndex=idx; }
};

//------------------------------------------------------------------------------------------------

class CBsStream	// LOD 별로 구분 되어 지는 데이타 입니다
{
protected:
	bool							m_bAllocated;

	D3DPRIMITIVETYPE				m_PrimitiveType;
	int								m_nVertexCount;

	int								m_nPrimitiveCount;
	int								m_nMaxBlendWeight; // 블렌드 갯수 입니다 = 사용 하지 않을때는 0 입니다 (1-4)

	std::vector<BS_VERTEX_ELEMENT>	m_StreamMaps; 	
	LPDIRECT3DINDEXBUFFER9			m_pIB;
	LPDIRECT3DINDEXBUFFER9			m_pInstancingIB; // Instancing용 인덱스 버퍼.	
	int								m_nInstancingCount;

	LPDIRECT3DVERTEXBUFFER9	m_pShadowVolumeVB;
	LPDIRECT3DINDEXBUFFER9		m_pShadowVolumeIB;
	int								m_nShadowVolumePrimCount;

	//bool							m_bUse32;			// Index32사용유무
	//bool							m_bUseVB; // false

	int								m_nBoneCount;		// SplitStream  Render 시에  필요
	int*							m_pBoneIndex;
	char**							m_ppBoneName; // 파일에서 읽어서 설정 합니다 - m_FnaPool 을 사용 합니다 : m_nBoneCount 

protected:
	int			GetCountByUsage(BS_DECLUSAGE usage); // 해당 usage를 사용하는 스트림의 갯수를 리턴 합니다
	void		ReadByDeclType(BStream *pStream, void* pOut, BS_DECLTYPE type, unsigned long size); // 엔디언스를 고려하여 읽습니다
	void		ReadAndCompressByDeclType(BStream* pStream, void* pOut, BS_DECLUSAGE usage, BS_DECLTYPE type, int nVertexCount);
	void		BuildBasisVector();	// 바이노말 + 탄젠트 를 생성 합니다

	void		AllocationStream(BS_DECLTYPE type, BS_DECLUSAGE usage, UINT uiUsageIndex, int nStride);

public:
	CBsStream();
	virtual    ~CBsStream();

	void		Clear();  
	int			LoadStream(BStream *pStream, BM_SUBMESH_HEADER *pHeader);
	void		PreparePhysique(int nBoneCount, BStream* pStream);

	void		CreateStream(int nVertexCount, int nFaceCount);
	void		CreateInstancingBuffer(	 int nBufferSize, WORD *pIndicesDest );
	void		SetBoundingBox(AABB &aabb);

	bool		GenerateStream(UINT usage, UINT uiIndex=0);
	bool		AddStream(BS_DECLTYPE type, BS_DECLUSAGE usage, int nIndex, int nStride);
	//						        |
	//						        BS_USAGE_TANGENT || BS_USAGE_BINORMAL --> BuildBasisVector()

	void		GetStream( UINT uiUsage, LPDIRECT3DVERTEXBUFFER9& pVB, LPDIRECT3DINDEXBUFFER9& pIB );
	void		GetStreamMap( UINT uiUsage, BS_VERTEX_ELEMENT*& pVE);

	void        GetStreamBuffer( D3DXVECTOR3 **ppVertexBuffer, WORD **ppFaceBuffer, int &nVertexCount, int &nFaceCount);	// 내용은 메쉬 헤더 파일 참조 바랍니다
	void        GetStreamBuffer( D3DXVECTOR3 **ppVertexBuffer, int &nVertexCount );	// 내용은 메쉬 헤더 파일 참조 바랍니다
	LPDIRECT3DINDEXBUFFER9	GetInstancingIB(void) { return m_pInstancingIB; }
	void        GetStreamBufferExt(D3DXVECTOR3 **ppPositionBuffer, D3DXVECTOR3 **ppNormalBuffer, int **ppBoneBuffer, int &nVertexCount);

	int					FindStreamByUsage(UINT uiUsage, UINT uiIndex=0); // 해당 usage와 usage 인덱스를 사용하는 스트림의 인덱스를 리턴 합니다
	D3DDECLTYPE			GetDeclType(int nIndex) { return m_StreamMaps[nIndex].m_DeclType; }
	D3DPRIMITIVETYPE	GetPrimitiveType() { return m_PrimitiveType; }
	int					GetPrimitiveCount() { return m_nPrimitiveCount; }
	int					GetBoneLinkCount() { return m_nMaxBlendWeight; } // 블렌드 갯수 입니다 = 사용 하지 않을때는 0 입니다 (1-4)
	int					GetVertexCount()	{ return m_nVertexCount; }
	void                GetDummyPosition(D3DXVECTOR3 &vecPosition); // booking, collision 에서 사용 합니다 

	void				SetStreamSource(C3DDevice* pDevice, int nStreamCount, int* pnStreamIndexList);
	void				SetStreamSourceForShadowVolume(C3DDevice *pDevice);
	//					|
	//					nStreamCount 만큼 Stream 설정 하고, 인덱스 버퍼도 설정 합니다

	void				DrawMesh(C3DDevice *pDevice) { pDevice->DrawIndexedMeshVB( m_PrimitiveType, m_nVertexCount, m_nPrimitiveCount, 0, 0 ); }
	void				DrawMeshForShadowVolume( C3DDevice *pDevice );
	void				DrawMeshPointSprite(C3DDevice *pDevice) { pDevice->DrawPrimitiveVB( D3DPT_POINTLIST, 0, m_nVertexCount ); }
	void				DrawMeshForInstancing(C3DDevice *pDevice, int nStreamCount, int* pnStreamIndexList, LPDIRECT3DVERTEXBUFFER9 pVertexBuffer, int nStartIndex, int nInstancingCount );

	void                Assign(CBsStream *pLODStream); // new 로 생성된 객체의 내용을 채워 줍니다 (메모리 중복 되지 않도록 주의 합니다)

	void                ChangeBlendValue(int v1, int v2);
	void				LinkBone(CBsAni* pAni);
	void				LinkBone( const std::vector< std::string > & szBoneList ); // mruete: prefix bug 463: changed to pass by const reference

	void				TransformBoneMatrices(BSMATRIX matWorldView, D3DXMATRIX *pBoneMatrix );

	static D3DPRIMITIVETYPE	ConvertPrimitiveType(BS_PRIMITIVE_TYPE type );
	static D3DDECLTYPE		ConvertDeclType     (BS_DECLTYPE	   type );
	static D3DDECLUSAGE		ConvertDeclUsage    (BS_DECLUSAGE	   usage);

	void	SplitStream( std::vector<CBsStream*> &SplitStreamList , int nMaxBoneCount);
	void	SplitStreamStrip( std::vector<CBsStream*> &SplitStreamList, int nMaxBoneCount);
	int		GetBoneCount() { return m_nBoneCount;}
	int		GetBoneIndex(int nIndex) { return m_pBoneIndex[nIndex];}
	char*	GetBoneName(int nBoneIndex) { return m_ppBoneName[nBoneIndex]; } 

	bool GenerateShadowVolume();		
};

//------------------------------------------------------------------------------------------------


