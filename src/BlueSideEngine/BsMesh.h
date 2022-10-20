#pragma once

#include "Box3.h"
#include "BsSubMesh.h"
#include "BsStringPool.h"

class C3DDevice;
class CBsMaterial;
class CBsSubMesh;
class CBsAniObject;

//------------------------------------------------------------------------------------------------

class CBsMesh
{			 
protected:
	char		*m_szMeshFileName; // m_FnaPool 을 사용 합니다
	int			 m_nSubMeshCount;
	CBsSubMesh **m_ppSubMesh;

	int          m_nLOD;			// 현재 Mesh가 가지고 있는 Level값(1 .. )
	int          m_nLODLevel;		// 현재 그려지고 있는 LOD 레벨 입니다  0..m_nLOD-1  (0 일때 가장 폴리곤 수가 많습니다)

	int			 m_nLODRatio;		// Mesh의 Radius에 기반한 오차계산용 Factor;

	int          m_nUVAnimation; // 0,1

	AABB		 m_BoundingBox;
	float		 m_fRadius;
	int          m_nBoundingBoxPos; // 파일에서의 위치 입니다

	int							m_nBookingCount;		// Booking 갯수입니다.
	std::vector<D3DXVECTOR3>	m_vecBookingList;
	int							m_nCollisionCount;		// Collision 갯수입니다.
	std::vector<D3DXVECTOR3>	m_vecCollisionList;
	int							m_nLinkDummyCount;		// Link Dummy 갯수입니다.
	std::vector<D3DXMATRIX>		m_matLinkDummyList;

	std::vector< std::string > m_szBoneList;

	int			m_nRefCount;

public:
	static int			s_nDetectBookingIndex; // 임시 변수 입니다
	static D3DXVECTOR3	s_vecBooking;
	static int			s_nDetectCollisionIndex; // 임시 변수 입니다
	static D3DXVECTOR3	s_vecCollision;
	static int			s_nDetectLinkDummyIndex;
	static D3DXMATRIX	s_matLinkDummy;

	static int          s_nDetectMeshCopy; // 임시 변수 입니다

	static int          s_bLowLOD; // 디버깅용 변수 입니다 (LOD 를 가장 Low 메쉬로 만들어 줍니다)

public:
				 CBsMesh();
	virtual     ~CBsMesh();

	int			Release();
	void		AddRef() { m_nRefCount++; }

	void		 DecLODLevel() { if (m_nLODLevel>       0) m_nLODLevel--; }
	void		 IncLODLevel() { if (m_nLODLevel<m_nLOD-1) m_nLODLevel++; }

	void		 SetMeshFileName(const char *pFileName);
	const char  *GetMeshFileName();
	const char  *GetMeshFileFullName() { return m_szMeshFileName; }
	int		 	 GetSubMeshCount() { return m_nSubMeshCount; }

	float		 GetMeshRadius()	{	return m_fRadius;	}

	AABB*		 GetBoundingBox() {	return &m_BoundingBox; }
	void		 SetBoundingBox(); // 스트림의 좌표의 최소 최대치를 구해서 바운딩 박스로 만듭니다
	void		 WriteBoundingBox(char *szFileName); // 파일에 저장 합니다

	int	 		 GetLODCount() { return m_nLOD; }
	int	 		 GetUVAnimation_() { return m_nUVAnimation; } // 0,1

	CBsSubMesh*  GetSubMeshPt(int nSubMesh) { return m_ppSubMesh[nSubMesh]; }
	int          GetPrimitiveCountLOD(); // 메쉬에서 사용된 삼각형 갯수를 현재 LOD 기준으로 넘겨 줍니다

	int	 		 GetPrimitiveType(int nSubMeshIndex) {	return m_ppSubMesh[nSubMeshIndex]->GetPrimitiveType(); } // CBsStream 의 변수를 사용 합니다
	int          GetPrimitiveCount(int nSubMeshIndex) { return m_ppSubMesh[nSubMeshIndex]->GetPrimitiveCount(); }
	const char	*GetSubMeshName(int nSubMeshIndex) { return m_ppSubMesh[nSubMeshIndex]->GetName(); }
	int          GetSubMeshBoneCount(int nSubMeshIndex) { return m_ppSubMesh[nSubMeshIndex]->GetBoneCount(); }
	const char  *GetSubMeshBoneName(int nSubMeshIndex, int nIndex) { return m_ppSubMesh[nSubMeshIndex]->GetBoneName(nIndex); }

	void		 Clear();
	int			 LoadMesh(BStream *pStream);
	int			 Reload();

	bool IsShow(int nSubMeshIndex) { return m_ppSubMesh[nSubMeshIndex]->IsShow(); }
	void Show(int nSubMeshIndex, bool bIsShow)	{	m_ppSubMesh[nSubMeshIndex]->Show(bIsShow);	}

	void		 LinkBone(CBsAni *pAni);
	void		 LinkBone();
	int			 GetBoneLinkCount(int nSubMeshIndex) { return m_ppSubMesh[nSubMeshIndex]->GetBoneLinkCount(); }
	int			 GetBoneLinkCount(int nSubMeshIndex, int nLODLevel)	{	return m_ppSubMesh[nSubMeshIndex]->GetBoneLinkCount(nLODLevel);	}

	int			 GetBookingCount()		{ return m_nBookingCount; }
	D3DXVECTOR3* GetBookingPosition(int nIndex);
	int			 GetCollisionCount()	{ return m_nCollisionCount; }
	D3DXVECTOR3* GetCollisionPosition(int nIndex);
	int			 GetLinkDummyCount()	{	return m_nLinkDummyCount;	}
	const D3DXMATRIX*	 GetLinkDummyMatrix(int nIndex);

	int			 CalculateLODLevel(float fDistanceFromCamera);		//Return Value : 최종으로 Setting된 LOD단계!!
	int			ForceLODLevel(int nLevel);

	bool		 AnalyzeStreamMap(int nSubMeshIndex, CBsImplMaterial* pImplMaterial)
	{
		return m_ppSubMesh[nSubMeshIndex]->AnalyzeStreamMap( pImplMaterial );
	}

    void		 PrepareRender		(int nSubmeshIndex, C3DDevice* pDevice, CBsMaterial* pMaterial, D3DXMATRIX* pObjMatrix, D3DXMATRIX *pBoneMatrix)	{ m_ppSubMesh[nSubmeshIndex]->PrepareRender(pDevice, pMaterial, pObjMatrix, pBoneMatrix);}
	void		 Render				(int nSubmeshIndex, C3DDevice* pDevice, CBsMaterial* pMaterial, int* pnStreamIndexList)	{ m_ppSubMesh[nSubmeshIndex]->Render(pDevice, pMaterial, pnStreamIndexList);}
	void		 RenderAlpha		(int nSubmeshIndex, C3DDevice* pDevice, CBsMaterial* pMaterial, int* pnStreamIndexList)	{ m_ppSubMesh[nSubmeshIndex]->RenderAlpha(pDevice, pMaterial, pnStreamIndexList);}
	void		 RenderAlphaAlign	(int nSubmeshIndex, C3DDevice* pDevice, CBsMaterial* pMaterial, int* pnStreamIndexList)	{ m_ppSubMesh[nSubmeshIndex]->RenderAlphaAlign(pDevice, pMaterial, pnStreamIndexList);}
	void		 RenderPointSprite	(int nSubmeshIndex, C3DDevice* pDevice, CBsMaterial* pMaterial, int* pnStreamIndexList)	{ m_ppSubMesh[nSubmeshIndex]->RenderPointsprite(pDevice, pMaterial, pnStreamIndexList);}
	void		 EndRender			(int nSubmeshIndex, C3DDevice* pDevice, CBsMaterial* pMaterial)							{ m_ppSubMesh[nSubmeshIndex]->EndRender(pDevice, pMaterial);}

//	void		 PrepareRenderShadow(int nSubmeshIndex, C3DDevice* pDevice, CBsMaterial* pMaterial, D3DXMATRIX* pObjMatrix)	{ m_ppSubMesh[nSubmeshIndex]->PrepareRenderShadow(pDevice, pMaterial, pObjMatrix);}
	void		 RenderShadow		(int nSubmeshIndex, C3DDevice* pDevice, CBsMaterial* pMaterial, int nLevelIndex = 0)	{ m_ppSubMesh[nSubmeshIndex]->RenderShadow(pDevice, pMaterial, 4);	}
	void		 RenderShadowVolume		(int nSubmeshIndex, C3DDevice* pDevice, CBsMaterial* pMaterial, int nLevelIndex = 0)	{ m_ppSubMesh[nSubmeshIndex]->RenderShadowVolume(pDevice, pMaterial, 4);	}
//	void		 EndRenderShadow	(int nSubmeshIndex, C3DDevice* pDevice, CBsMaterial* pMaterial)							{ m_ppSubMesh[nSubmeshIndex]->EndRenderShadow(pDevice, pMaterial);}

	void		 StoreBoneList();
	int			 GetStoredBoneIndex( const char *pBoneName );
	int			 GetStoredBoneCount() { return ( int )m_szBoneList.size(); }

	//--------------------

	D3DXVECTOR3 *m_ppVertexBuffer; // 버텍스 리스트 
	int          m_nVertexCount; // 버텍스 갯수 (육면체의 경우에 4*6=24개 입니다, VertexBuffer가 24개 메모리 할당 됩니다)
	WORD        *m_ppFaceBuffer; // 면 리스트
	int          m_nFaceCount; // 면 갯수 (육면체의 경우에 2*6=12개 입니다, FaceBuffer가 12*3개 (주의) 메모리 할당 됩니다)
	//           |
	//           모델 데이타 BM 에서 MeshCopy 가 검색되면 위의 데이타를 아래의 함수를 사용해서 자동으로 만들고, 직접 메모리 관리 합니다
	//           페이스 관련 데이타는 WORD 3 개씩 사용 됨에 주의 해 주세요

	void		 GetStream( UINT uiUsage, LPDIRECT3DVERTEXBUFFER9& pVB, LPDIRECT3DINDEXBUFFER9& pIB );
	//			 |
	//			 해당 Usage 의 데이타 포인트를 넘겨 줍니다

	void		 GetStreamBuffer( D3DXVECTOR3 **ppVertexBuffer, WORD **ppFaceBuffer, int &nVertexCount, int &nFaceCount, int nSubMeshIndex = 0);
	void		 GetStreamBuffer( D3DXVECTOR3 **ppVertexBuffer, int &nVertexCount );
	//			 |
	//			 함수 안에서 메모리 할당을 하므로, 사용하시고 나서 직접 메모리 프리 해주어야 합니다 (주의)
	//			 서브메쉬가 여러개 일때도 처음 서브메쉬의 내용만 얻어 옵니다 (주의)
	//           D3DPT_TRIANGLELIST 만 지원 합니다
	//           WORD 3개 이므로 nFaceCount * 3 입니다 (주의)

	void		 GetStreamBufferAll( D3DXVECTOR3 **ppVertexBuffer, WORD **ppFaceBuffer, int &nVertexCount, int &nFaceCount );
	void		 GetStreamBufferAll( D3DXVECTOR3 **ppVertexBuffer, int &nVertexCount );
	//			 |	
	//           위의 함수에서 서브 메쉬 전부 얻어 와서 합치는 부분 만 다릅니다

	void         GetStreamBufferExt(D3DXVECTOR3 **ppPositionBuffer, D3DXVECTOR3 **ppNormalBuffer, int **ppBoneBuffer, int &nVertexCount);
	//			 |
	//			 함수 안에서 메모리 할당을 하므로, 사용하시고 나서 직접 메모리 프리 해주어야 합니다 (주의) = pp 는 사용후 모두 프리해 주어야 합니다
	//			 서브메쉬가 여러개 일때도 처음 서브메쉬의 내용만 얻어 옵니다 (주의)
	//			 D3DPT_TRIANGLELIST 만 지원 합니다
	//
	//			 피직이 있다면  
	//             피직 1W 만 지원 합니다 (맥스에서 익스포트 할때)
	//             ppBoneBuffer : 1W 사용시에는 int 로 그냥 사용해도 됩니다 (원래는 short 2 입니다)
	//			 피직이 없다면
	//             ppBoneBuffer=NULL
};

//------------------------------------------------------------------------------------------------


