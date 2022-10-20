#pragma once

#include "BsMaterial.h"
#include "BsSubMesh.h"

class CBsObject;
class C3DDevice;

struct STREAM_MAPPER
{
	STREAM_MAPPER()	{
		m_nVertexDeclIndex = -1;
		m_nStreamIndexCount = 0;
		m_pnStreamIndex = NULL;
	}
	~STREAM_MAPPER() {
		SAFE_RELEASE_VD(m_nVertexDeclIndex);
		SAFE_DELETEA(m_pnStreamIndex);
		m_nStreamIndexCount = 0;
	}
	int		m_nVertexDeclIndex;
	int		m_nStreamIndexCount;
	int*	m_pnStreamIndex;
};

class CBsImplMaterial
{
public:
	CBsImplMaterial();
	virtual ~CBsImplMaterial();

	void Clear();

public:
	int								m_nStreamMapperCount;			// LOD �����̴�.
	STREAM_MAPPER*					m_pStreamMappers;				// LOD ������ŭ �����ȴ�.

	int*			GetStreamMapper(int nLODLevel)		{	return m_pStreamMappers[nLODLevel].m_pnStreamIndex;	}
	int				GetVertexDeclIndex(int nLODLevel)	{	return m_pStreamMappers[nLODLevel].m_nVertexDeclIndex;	}

	CBsMaterial*					m_pMaterial;			// AnalyzeStreamMap
	int								m_nMaterialIndex;
	int								m_nSamplerCount;
	int*							m_pnTextureIndex; // m_nSamplerCount ��ŭ ���� �մϴ�

	bool							m_bAlphaAlignEnable; // Alpha Blend �ΰ������� valid!!!!!!
	bool							m_bTwoSideEnable; // Alpha Blend �ƴѰ������� valid!!!!!!!!
	bool							m_bAlphaBlendEnable;
	bool							m_bDepthCehckEnable;
	bool							m_bOnlyDepthWrite;
	bool							m_bPointSprite;
	
	float							m_fAlpha;
	D3DBLENDOP						m_BlendOp;
	D3DBLEND						m_SrcBlend;
	D3DBLEND						m_DestBlend;

	int								m_nUVAnimationPos; // -1 �� �ƴҶ��� <Ŀ���� �з����� UV ������>�� �����ϴ� �ε��� ���� ������ �ֽ��ϴ� 0..N-1
	int								m_nUVAnimationSca;
	int								m_nUVAnimationRot;
	float							m_fCurrentFrameUVAnimation; // �ؽ��� �ִϸ��̼� (0.0f .. 1.0f)	= �������� ��� �մϴ�

	// Customized(User Defined) Parameter Value�� ���Ŀ� �۾� by jeremy!!
	std::vector<CustomParam_Info*>	m_EditableParameters;

public:
	void	SetAlphaState(C3DDevice* pDevice);
	void	RestoreAlphaState(C3DDevice* pDevice);

	int		AddEditableParameter(PARAMETER_TYPE type);
	void	SetEditableParameter(int nIndex, void* pBuf);
	void	GetEditableParameter(int nIndex, void* pBuf);
	int		SearchEditableParameter(PARAMETER_TYPE type);

	void	SetUVAnimation(int v); // m_nUVAnimationPos ��ǥ�� ���� �˴ϴ� = v(0,1) --> r(-2,-1)
	int	    GetUVAnimation() { return m_nUVAnimationPos; } // UV �ִϸ��̼� �Դϴ� (��ǥ ���Դϴ�) -1, 0..N-1
	void	SetCurrentFrameUVAnimation(float fFrame) { m_fCurrentFrameUVAnimation=fFrame; }
	void	SetUVAnimationRender(CBsSubMesh *pSubMesh);

	void	SetupLOD(int nLODCount)	{
		SAFE_DELETEA(m_pStreamMappers);
		m_nStreamMapperCount = nLODCount;
		m_pStreamMappers = new STREAM_MAPPER[m_nStreamMapperCount];
	}
	void	AttachSubMesh(CBsSubMesh* pSubMesh);						// SubMesh�� ���� ó���� �Ѵ�(LOD ��Ÿ ���)
	int		LinkMaterial(int nMaterial, CBsSubMesh* pSubMesh);			// return value�� Material���� ���̴� Sampler Count!!
	bool	UnlinkMaterial();

	void	SetCustomParameter();
	void	GetCustomParameterValue(PARAMETER_TYPE type, void* pBuf);
	//*****************************************************************
	void	HoldMaterial(C3DDevice* pDevice, CBsObject* pObject, int nTechIndex, int nLODLevel);
	void	FreeMaterial();
	//*****************************************************************
	const char *GetMaterialName() {	return m_pMaterial->GetMaterialName(); }
	// int         GetMaterialSemanticsName(char *pName) {	return m_pMaterial->GetMaterialSemanticsName(pName); } // char [N_MAX][32]    (���� ����)
	int         GetMaterialCustomSemanticsName(char *pName, const size_t pName_len) { return m_pMaterial->GetMaterialCustomSemanticsName(pName, pName_len); } // char [N_MAX][32]
	int         GetMaterialSemanticsInfo(std::vector<SEM_INFO>* pInfo) {	return m_pMaterial->GetMaterialSemanticsInfo(pInfo); } // std::vector<SEM_INFO>*
	int         GetMaterialCustomSemanticsInfo(std::vector<SEM_INFO>* pInfo) { return m_pMaterial->GetMaterialCustomSemanticsInfo(pInfo); } // std::vector<SEM_INFO>* 

protected:
	int		SearchCustomParameterIndex(PARAMETER_TYPE type);
	VariableType GetVariableTypeByParam(PARAMETER_TYPE type);
};