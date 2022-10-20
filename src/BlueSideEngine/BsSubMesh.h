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

struct TEXTURE_ANI // �ؽ��� �ִ� ����Ÿ �Դϴ�
{
	int nTime;
	D3DXVECTOR3 Position;
	D3DXQUATERNION Rotation;
	D3DXVECTOR3 Scale;
};

class CBsSubMesh
{
protected:
	char		   *m_szParentSubMeshName; // m_FnaPool �� ��� �մϴ�
	//              |
	//				m_ppBoneName : CBsPhysiqueSubMesh

	char		   *m_szSubMeshName; 

	int				m_nMaterialIndex; // �ε��� = -1 
	bool			m_bIsShow;

	CBsBone		   *m_pParentBone; // m_szParentSubMeshName �� �ش� �ϴ� ���� ã���ϴ�
	int				m_nParentBoneIndex;
	//				|
	//				m_ppBoneList : CBsPhysiqueSubMesh
	//				m_pBoneIndex

	enum { m_pStreamLOD_max=7, }; // ��� 7 �ܰ��� LOD ���� ���� �մϴ�
	CBsStream	   *m_pStreamBuf[m_pStreamLOD_max];

	int				m_nStreamLODAllocated; // ��ü �Ҵ�� ���� �Դϴ�

	int             m_nUVAnimationV; // 0,1
	int             m_nTextureAniNum; // �ִϸ��̼� ���� �Դϴ�
	TEXTURE_ANI    *m_TextureAni; // �޸�

	//D3DXVECTOR3     m_vecSubMeshSize; // ��ο�޽��� ������� �ʱ� ���� �뵵�� �ٿ�� �ڽ��� ũ�⸦ ���� �մϴ�

	int				m_nInstancingVertexDecl;		// instancing �� declaration -> Ȯ���ʿ�

	struct {
		int m_nBoneLink; // 0, 1..4

		/*
		int m_nPositionIndex; // m_pStreamBuf.FindStreamByUsage(BS_USAGE_POSITION, 0);
		int nBinorIndex; // FindStreamByUsage(BS_USAGE_BINORMAL, 0);

		enum { eStreamNumberMax=10, }; // <2004 12 21 ���� ���� ȣȯ>
		int					nStreamNumber; 
		int					nStreamType [eStreamNumberMax];
		int					nStreamUsage[eStreamNumberMax];
		*/

		// �ʿ��� ����Ÿ ���°� �Ƹ��� �ؼ�, //!!
		// ����Ÿ ���¸� ������ �ֽø�
		// �ͽ���Ʈ�� �ε� �ܰ迡�� �ʿ��� ����Ÿ ������ ������� �մϴ�
	}
	m_flag;

public:
	static  int     g_nLODLevel_; // ���� �׸� LOD �ܰ� �Դϴ� (������Ʈ�� ���� �Լ� ȣ�� ���� �̸� ���� �Ǿ� ���ϴ�)
	// 0 �϶� �ְ� ���� �Դϴ� (0 .. CBsMesh::m_nLOD-1)

	CBsSubMesh();
	virtual		   ~CBsSubMesh();

	//				�Ҹ���, �ε� �Լ����� ������ ȣ�� �ǰ� �ֽ��ϴ� = virtual void Clear() �� �ʿ��ϴٸ� �߰��Ͽ��� �մϴ�
	//				|
	virtual void	Clear_(); // CBsPhysiqueSubMesh* �� ȣ��Ǵ� ��쿡 ������ �߻� ���� �ʵ��� ���� �մϴ�
	void			LoadMaterialHeader(BStream *pStream, BM_MATERIAL_HEADER *pHeader);
	virtual void	LoadSubMesh(BStream *pStream, BM_SUBMESH_HEADER *pHeader); // �޸� �ڵ� �Ұ� �˴ϴ�

	bool IsShow() { return m_bIsShow; }
	void Show(bool bShow) { m_bIsShow=bShow; }
	
	int				GetVertexCount( int nLodLevel ) { return m_pStreamBuf[ nLodLevel ]->GetVertexCount(); }	

	const	char*	GetName() const { return m_szSubMeshName; }
	int				GetPrimitiveType()	{	return m_pStreamBuf[0]->GetPrimitiveType();	}
	int             GetPrimitiveCount() { return m_pStreamBuf[0]->GetPrimitiveCount(); }
	int             GetPrimitiveCountLOD(int nLODLevel); // �ش� LOD �� ������Ƽ�� ������ �Ѱ� �ݴϴ�, 0 �϶� �ְ� ���� �Դϴ� (0 .. CBsMesh::m_nLOD-1)
	int				GetLODCount()		{	return m_nStreamLODAllocated;	}

#ifdef _USAGE_TOOL_
	BM_MATERIAL_HEADER mtr_header; // ��Ʈ���� �������� ��� �մϴ�
#endif

	int	 	  	    GetUVAnimationV() { return m_nUVAnimationV; }  // v=value
	float			GetUVAnimationFrame(float fCurrentFrameUVAnimation) { return fCurrentFrameUVAnimation * (m_TextureAni[m_nTextureAniNum-1].nTime - m_TextureAni[0].nTime); }	// �ش� ����Ÿ�� �ƽ����� ������ ������ ���� �Դϴ� 

	virtual int     GetBoneCount(int nLOD=0) { return 0; } // CBsPhysiqueSubMesh �� ���� �˴ϴ�
	virtual char   *GetBoneName(int nIndex,int nLOD=0) { return NULL; } 

	virtual void    AddBoneName_BA(const char *szName) { } // BM�� BA�� ���� ���� �߰� �մϴ�

	int				GetBoneLinkCount(int nLOD=0)		{	return m_pStreamBuf[nLOD]->GetBoneLinkCount();	}
	virtual void	LinkBone(CBsAni *pAni);
	virtual void	LinkBone( const std::vector< std::string > & szBoneList );	// mruete: changed to pass by const reference

	virtual bool	AnalyzeStreamMap(CBsImplMaterial* pImplMaterial);

	void			SetBoundingBox(AABB &aabb) { m_pStreamBuf[0]->SetBoundingBox(aabb);	}

	int				GetPositionVector(D3DXVECTOR3    &Position, float fFrame); // �ؽ��� �ִϸ��̼�
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

	CBsStream*		GetStreamForLOD(int nLODLevel = 0) { return m_pStreamBuf[nLODLevel]; } // AddLODStream() �� ����� ����Ÿ�� �Ѱ� �ݴϴ�
	void		    AddLODStream(CBsSubMesh *pLODSubMesh);

	//											                  ���� ���� �̸� �Դϴ�
	//											                  |
	static int      MakeOneBm(char **ppFileName, int nFile, char *pStoreName); // nFile ���� bm �� �� ���� bm �� ����� �ݴϴ� (LOD)
	//				   	             |
	//                              nFile ���� bm ���� �̸��� ��� �մϴ� = ������ bm �� lod�� 1 �� ��� ���� �մϴ�

};

//------------------------------------------------------------------------------------------------


