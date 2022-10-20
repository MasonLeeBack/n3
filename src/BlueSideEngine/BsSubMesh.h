#pragma once

#include "BsStream.h"

class C3DDevice;
class CBsMaterial;
class CBsImplMaterial;
class CBsAni;
class CBsBone;
class CBsObject;
struct STREAM_MAPPER;

//------------------------------------------------------------------------------------------------

struct TEXTURE_ANI // 텍스쳐 애니 데이타 입니다
{
	int nTime;
	D3DXVECTOR3 Position;
	D3DXQUATERNION Rotation;
	D3DXVECTOR3 Scale;
};

class CBsSubMesh
{
protected:
	char		   *m_szParentSubMeshName; // m_FnaPool 을 사용 합니다
	//              |
	//				m_ppBoneName : CBsPhysiqueSubMesh

	char		   *m_szSubMeshName; 

	int				m_nMaterialIndex; // 인덱스 = -1 
	bool			m_bIsShow;

	CBsBone		   *m_pParentBone; // m_szParentSubMeshName 에 해당 하는 본을 찾습니다
	int				m_nParentBoneIndex;
	//				|
	//				m_ppBoneList : CBsPhysiqueSubMesh
	//				m_pBoneIndex

	enum { m_pStreamLOD_max=7, }; // 모두 7 단계의 LOD 까지 가능 합니다
	CBsStream	   *m_pStreamBuf[m_pStreamLOD_max];

	int				m_nStreamLODAllocated; // 전체 할당된 갯수 입니다

	int             m_nUVAnimationV; // 0,1
	int             m_nTextureAniNum; // 애니메이션 갯수 입니다
	TEXTURE_ANI    *m_TextureAni; // 메모리

	//D3DXVECTOR3     m_vecSubMeshSize; // 드로우메쉬를 사용하지 않기 위한 용도로 바운딩 박스의 크기를 저장 합니다

	int				m_nInstancingVertexDecl;		// instancing 용 declaration -> 확인필요

	struct {
		int m_nBoneLink; // 0, 1..4

		/*
		int m_nPositionIndex; // m_pStreamBuf.FindStreamByUsage(BS_USAGE_POSITION, 0);
		int nBinorIndex; // FindStreamByUsage(BS_USAGE_BINORMAL, 0);

		enum { eStreamNumberMax=10, }; // <2004 12 21 이후 버전 호환>
		int					nStreamNumber; 
		int					nStreamType [eStreamNumberMax];
		int					nStreamUsage[eStreamNumberMax];
		*/

		// 필요한 데이타 형태가 아리송 해서, //!!
		// 데이타 형태를 정의해 주시면
		// 익스포트나 로드 단계에서 필요한 데이타 정보를 만들려고 합니다
	}
	m_flag;

public:
	static  int     g_nLODLevel_; // 현재 그릴 LOD 단계 입니다 (오브젝트서 렌더 함수 호출 전에 미리 설정 되어 집니다)
	// 0 일때 최고 레벨 입니다 (0 .. CBsMesh::m_nLOD-1)

	CBsSubMesh();
	virtual		   ~CBsSubMesh();

	//				소멸자, 로드 함수에서 적절히 호출 되고 있습니다 = virtual void Clear() 가 필요하다면 추가하여야 합니다
	//				|
	virtual void	Clear_(); // CBsPhysiqueSubMesh* 로 호출되는 경우에 문제가 발생 하지 않도록 주의 합니다
	void			LoadMaterialHeader(BStream *pStream, BM_MATERIAL_HEADER *pHeader);
	virtual void	LoadSubMesh(BStream *pStream, BM_SUBMESH_HEADER *pHeader); // 메모리 자동 소거 됩니다

	bool IsShow() { return m_bIsShow; }
	void Show(bool bShow) { m_bIsShow=bShow; }
	
	int				GetVertexCount( int nLodLevel ) { return m_pStreamBuf[ nLodLevel ]->GetVertexCount(); }	

	const	char*	GetName() const { return m_szSubMeshName; }
	int				GetPrimitiveType()	{	return m_pStreamBuf[0]->GetPrimitiveType();	}
	int             GetPrimitiveCount() { return m_pStreamBuf[0]->GetPrimitiveCount(); }
	int             GetPrimitiveCountLOD(int nLODLevel); // 해당 LOD 의 프리미티브 갯수를 넘겨 줍니다, 0 일때 최고 레벨 입니다 (0 .. CBsMesh::m_nLOD-1)
	int				GetLODCount()		{	return m_nStreamLODAllocated;	}

#ifdef _USAGE_TOOL_
	BM_MATERIAL_HEADER mtr_header; // 매트리얼 툴에서만 사용 합니다
#endif

	int	 	  	    GetUVAnimationV() { return m_nUVAnimationV; }  // v=value
	float			GetUVAnimationFrame(float fCurrentFrameUVAnimation) { return fCurrentFrameUVAnimation * (m_TextureAni[m_nTextureAniNum-1].nTime - m_TextureAni[0].nTime); }	// 해당 데이타의 맥스에서 지정한 프레임 범위 입니다 

	virtual int     GetBoneCount(int nLOD=0) { return 0; } // CBsPhysiqueSubMesh 와 연결 됩니다
	virtual char   *GetBoneName(int nIndex,int nLOD=0) { return NULL; } 

	virtual void    AddBoneName_BA(const char *szName) { } // BM에 BA의 본을 강제 추가 합니다

	int				GetBoneLinkCount(int nLOD=0)		{	return m_pStreamBuf[nLOD]->GetBoneLinkCount();	}
	virtual void	LinkBone(CBsAni *pAni);
	virtual void	LinkBone( const std::vector< std::string > & szBoneList );	// mruete: changed to pass by const reference

	virtual bool	AnalyzeStreamMap(CBsImplMaterial* pImplMaterial);

	void			SetBoundingBox(AABB &aabb) { m_pStreamBuf[0]->SetBoundingBox(aabb);	}

	int				GetPositionVector(D3DXVECTOR3    &Position, float fFrame); // 텍스쳐 애니메이션
	int				GetRotationVector(D3DXQUATERNION &Rotation, float fFrame);	
	int				GetScaleVector   (D3DXVECTOR3    &Scale,    float fFrame);	

	int				GetParentBoneIndex() {return m_nParentBoneIndex;}
	
	virtual void	PrepareRender(C3DDevice* pDevice, CBsMaterial* pMaterial, D3DXMATRIX* pObjMatrix, D3DXMATRIX *pBoneMatrix );
	virtual void	Render(C3DDevice* pDevice, CBsMaterial* pMaterial, int* pnStreamIndexList);
	virtual void	RenderAlpha(C3DDevice* pDevice, CBsMaterial* pMaterial, int* pnStreamIndexList);
	virtual void	RenderAlphaAlign(C3DDevice* pDevice, CBsMaterial* pMaterial, int* pnStreamIndexList);
	virtual void	RenderInstancing(C3DDevice* pDevice, CBsMaterial* pMaterial, int* pnStreamIndexList, int nStreamCount,  LPDIRECT3DVERTEXBUFFER9 pVertexBuffer, int nStartIndex, int nInstancingCount);
	virtual void	RenderPointsprite(C3DDevice* pDevice, CBsMaterial* pMaterial, int* pnStreamIndexList);
	virtual void	EndRender(C3DDevice* pDevice, CBsMaterial* pMaterial                        );

	//	virtual void	PrepareRenderShadow(C3DDevice* pDevice, CBsMaterial* pMaterial, D3DXMATRIX* pObjMatrix);
	virtual void	RenderShadow(C3DDevice* pDevice, CBsMaterial* pMaterial, int nLevelIndex = 0);
	virtual void RenderShadowVolume(C3DDevice* pDevice, CBsMaterial* pMaterial, int nLevelIndex = 0);
	//	virtual void	EndRenderShadow(C3DDevice* pDevice, CBsMaterial* pMaterial                        );

	CBsStream*		GetStreamForLOD(int nLODLevel = 0) { return m_pStreamBuf[nLODLevel]; } // AddLODStream() 에 사용할 데이타를 넘겨 줍니다
	void		    AddLODStream(CBsSubMesh *pLODSubMesh);

	//											                  저장 파일 이름 입니다
	//											                  |
	static int      MakeOneBm(char **ppFileName, int nFile, char *pStoreName); // nFile 개의 bm 를 한 개의 bm 로 만들어 줍니다 (LOD)
	//				   	             |
	//                              nFile 개의 bm 파일 이름을 사용 합니다 = 각각의 bm 는 lod가 1 개 라고 가정 합니다

};

//------------------------------------------------------------------------------------------------


