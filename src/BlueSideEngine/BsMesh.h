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
	char		*m_szMeshFileName; // m_FnaPool �� ��� �մϴ�
	int			 m_nSubMeshCount;
	CBsSubMesh **m_ppSubMesh;

	int          m_nLOD;			// ���� Mesh�� ������ �ִ� Level��(1 .. )
	int          m_nLODLevel;		// ���� �׷����� �ִ� LOD ���� �Դϴ�  0..m_nLOD-1  (0 �϶� ���� ������ ���� �����ϴ�)

	int			 m_nLODRatio;		// Mesh�� Radius�� ����� �������� Factor;

	int          m_nUVAnimation; // 0,1

	AABB		 m_BoundingBox;
	float		 m_fRadius;
	int          m_nBoundingBoxPos; // ���Ͽ����� ��ġ �Դϴ�

	int							m_nBookingCount;		// Booking �����Դϴ�.
	std::vector<D3DXVECTOR3>	m_vecBookingList;
	int							m_nCollisionCount;		// Collision �����Դϴ�.
	std::vector<D3DXVECTOR3>	m_vecCollisionList;
	int							m_nLinkDummyCount;		// Link Dummy �����Դϴ�.
	std::vector<D3DXMATRIX>		m_matLinkDummyList;

	std::vector< std::string > m_szBoneList;

	int			m_nRefCount;

public:
	static int			s_nDetectBookingIndex; // �ӽ� ���� �Դϴ�
	static D3DXVECTOR3	s_vecBooking;
	static int			s_nDetectCollisionIndex; // �ӽ� ���� �Դϴ�
	static D3DXVECTOR3	s_vecCollision;
	static int			s_nDetectLinkDummyIndex;
	static D3DXMATRIX	s_matLinkDummy;

	static int          s_nDetectMeshCopy; // �ӽ� ���� �Դϴ�

	static int          s_bLowLOD; // ������ ���� �Դϴ� (LOD �� ���� Low �޽��� ����� �ݴϴ�)

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
	void		 SetBoundingBox(); // ��Ʈ���� ��ǥ�� �ּ� �ִ�ġ�� ���ؼ� �ٿ�� �ڽ��� ����ϴ�
	void		 WriteBoundingBox(char *szFileName); // ���Ͽ� ���� �մϴ�

	int	 		 GetLODCount() { return m_nLOD; }
	int	 		 GetUVAnimation_() { return m_nUVAnimation; } // 0,1

	CBsSubMesh*  GetSubMeshPt(int nSubMesh) { return m_ppSubMesh[nSubMesh]; }
	int          GetPrimitiveCountLOD(); // �޽����� ���� �ﰢ�� ������ ���� LOD �������� �Ѱ� �ݴϴ�

	int	 		 GetPrimitiveType(int nSubMeshIndex) {	return m_ppSubMesh[nSubMeshIndex]->GetPrimitiveType(); } // CBsStream �� ������ ��� �մϴ�
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

	int			 CalculateLODLevel(float fDistanceFromCamera);		//Return Value : �������� Setting�� LOD�ܰ�!!
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

	D3DXVECTOR3 *m_ppVertexBuffer; // ���ؽ� ����Ʈ 
	int          m_nVertexCount; // ���ؽ� ���� (����ü�� ��쿡 4*6=24�� �Դϴ�, VertexBuffer�� 24�� �޸� �Ҵ� �˴ϴ�)
	WORD        *m_ppFaceBuffer; // �� ����Ʈ
	int          m_nFaceCount; // �� ���� (����ü�� ��쿡 2*6=12�� �Դϴ�, FaceBuffer�� 12*3�� (����) �޸� �Ҵ� �˴ϴ�)
	//           |
	//           �� ����Ÿ BM ���� MeshCopy �� �˻��Ǹ� ���� ����Ÿ�� �Ʒ��� �Լ��� ����ؼ� �ڵ����� �����, ���� �޸� ���� �մϴ�
	//           ���̽� ���� ����Ÿ�� WORD 3 ���� ��� �ʿ� ���� �� �ּ���

	void		 GetStream( UINT uiUsage, LPDIRECT3DVERTEXBUFFER9& pVB, LPDIRECT3DINDEXBUFFER9& pIB );
	//			 |
	//			 �ش� Usage �� ����Ÿ ����Ʈ�� �Ѱ� �ݴϴ�

	void		 GetStreamBuffer( D3DXVECTOR3 **ppVertexBuffer, WORD **ppFaceBuffer, int &nVertexCount, int &nFaceCount, int nSubMeshIndex = 0);
	void		 GetStreamBuffer( D3DXVECTOR3 **ppVertexBuffer, int &nVertexCount );
	//			 |
	//			 �Լ� �ȿ��� �޸� �Ҵ��� �ϹǷ�, ����Ͻð� ���� ���� �޸� ���� ���־�� �մϴ� (����)
	//			 ����޽��� ������ �϶��� ó�� ����޽��� ���븸 ��� �ɴϴ� (����)
	//           D3DPT_TRIANGLELIST �� ���� �մϴ�
	//           WORD 3�� �̹Ƿ� nFaceCount * 3 �Դϴ� (����)

	void		 GetStreamBufferAll( D3DXVECTOR3 **ppVertexBuffer, WORD **ppFaceBuffer, int &nVertexCount, int &nFaceCount );
	void		 GetStreamBufferAll( D3DXVECTOR3 **ppVertexBuffer, int &nVertexCount );
	//			 |	
	//           ���� �Լ����� ���� �޽� ���� ��� �ͼ� ��ġ�� �κ� �� �ٸ��ϴ�

	void         GetStreamBufferExt(D3DXVECTOR3 **ppPositionBuffer, D3DXVECTOR3 **ppNormalBuffer, int **ppBoneBuffer, int &nVertexCount);
	//			 |
	//			 �Լ� �ȿ��� �޸� �Ҵ��� �ϹǷ�, ����Ͻð� ���� ���� �޸� ���� ���־�� �մϴ� (����) = pp �� ����� ��� ������ �־�� �մϴ�
	//			 ����޽��� ������ �϶��� ó�� ����޽��� ���븸 ��� �ɴϴ� (����)
	//			 D3DPT_TRIANGLELIST �� ���� �մϴ�
	//
	//			 ������ �ִٸ�  
	//             ���� 1W �� ���� �մϴ� (�ƽ����� �ͽ���Ʈ �Ҷ�)
	//             ppBoneBuffer : 1W ���ÿ��� int �� �׳� ����ص� �˴ϴ� (������ short 2 �Դϴ�)
	//			 ������ ���ٸ�
	//             ppBoneBuffer=NULL
};

//------------------------------------------------------------------------------------------------


