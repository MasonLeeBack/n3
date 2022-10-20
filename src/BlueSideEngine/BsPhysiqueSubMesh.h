#pragma once

#include "BsSubMesh.h"

class CBsBone;
class BStream;

//------------------------------------------------------------------------------------------------

class CBsPhysiqueSubMesh : public CBsSubMesh
{
protected:
	static BSMATRIX  *s_pBoneMatrices; // 공융으로 사용 합니다
	static int			s_nRefCount;

//	int					m_nBoneCount; // 갯수 입니다
//	char			  **m_ppBoneName; // 파일에서 읽어서 설정 합니다 - m_FnaPool 을 사용 합니다 : m_nBoneCount 
//	int				   *m_pBoneIndex; // m_ppBoneName 으로 부터 설정 합니다						: m_nBoneCount

	std::vector<CBsStream*>	   m_SplitStream;
	bool				m_bSplit;

public:
						CBsPhysiqueSubMesh(bool bUseSoftwarePhysique=false);
	virtual			   ~CBsPhysiqueSubMesh();

	virtual void		Clear_(); // CBsSubMesh::Clear_() 는 호출 되지 않으므로 주의 합니다
	virtual void		LoadSubMesh(BStream *pStream, BM_SUBMESH_HEADER *pHeader); // 메모리 자동 소거 됩니다

	virtual int			GetBoneCount(int nLOD=0) { return m_pStreamBuf[nLOD]->GetBoneCount(); } // CBsSubMesh 로 부터 연결 됩니다
	virtual char	   *GetBoneName(int nIndex, int nLOD=0) { return m_pStreamBuf[nLOD]->GetBoneName(nIndex); } 

	virtual void		LinkBone(CBsAni *pAni);
	virtual void		LinkBone( const std::vector< std::string > & szBoneList ); // mruete: changed to pass by const reference

	void				TransformBoneMatrices(C3DDevice* pDevice, D3DXMATRIX* pMatObject, D3DXMATRIX *pBoneMatrix);
	static	BSMATRIX*	GetSharedMatrices()	{	return s_pBoneMatrices;	}

	virtual void		PrepareRender(C3DDevice* pDevice, CBsMaterial* pMaterial, D3DXMATRIX* pObjMaterix, D3DXMATRIX *pBoneMatrix );
	virtual void		Render(C3DDevice* pDevice, CBsMaterial* pMaterial, int* pnStreamIndexList);
	virtual void		RenderShadow(C3DDevice* pDevice, CBsMaterial* pMaterial, int nLevelIndex /*=0*/);
	virtual void		RenderShadowVolume(C3DDevice* pDevice, CBsMaterial* pMaterial, int nLevelIndex /*=0*/);
	virtual void		RenderAlpha(C3DDevice* pDevice, CBsMaterial* pMaterial, int* pnStreamIndexList);
	virtual void		RenderAlphaAlign(C3DDevice* pDevice, CBsMaterial* pMaterial, int* pnStreamIndexList);
	virtual void		RenderPointsprite(C3DDevice* pDevice, CBsMaterial* pMaterial, int* pnStreamIndexList);
	virtual void		EndRender(C3DDevice* pDevice, CBsMaterial* pMaterial);
};

//------------------------------------------------------------------------------------------------

