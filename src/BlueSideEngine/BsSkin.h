#pragma once

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

class BStream;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

struct SBsSampler_Info {
	char                               *m_pSamplerFile;		// m_FnaPool �� ��� �մϴ�
	int									m_nSamplerIndex;
};

struct SBsSubMesh_Info {
public:
	char							   *m_szFxFileName;	// m_FnaPool �� ��� �մϴ�
	int									m_nFxIndex;		// material

	BOOL								m_bUseAlphaBlend;
	float								m_fAlpha;
	bool								m_bUseTwoSideRender;
	bool								m_bUseAlphaAlignRender;

	int									m_nSamplerCount;
	SBsSampler_Info**					m_ppSamplers; // ���� �̸��� �ε���
	std::vector<CustomParam_Info*>		m_CustomParameters;	
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

class CBsSkin {
public:
	CBsSkin();
	~CBsSkin();

protected:
	char							   *m_szSkinFileName;	// m_FnaPool �� ��� �մϴ�

protected:
	char						       *m_szMeshFileName;	// m_FnaPool �� ��� �մϴ�
	int									m_nMeshIndex;

	SBsSubMesh_Info**					m_ppSubMesh;		// submesh�� ����
	int									m_nSubMeshCount;

	int									m_nRefCount;

public:
	void				AddRef();
	int					Release();

	int					GetRefCount() { return m_nRefCount; }

	int					LoadSkin(BStream* pStream);

	D3DXVECTOR4			GetSkinMaterialAVParam(PARAMETER_TYPE type);
	const char*			GetSkinFileName();
	void				SetSkinFileName(char* pFilename);
	int					GetMeshIndex() { return m_nMeshIndex; }
	SBsSubMesh_Info*	GetSubMeshInfo(int nIndex) { return m_ppSubMesh[nIndex]; }
	int					GetSubMeshCount() { return m_nSubMeshCount; }

	void GetTextureList( std::vector< std::string > *pNameList );

#ifdef _LOAD_MAP_CHECK_
	int m_nLoadSize;
	int m_nUseCount;
#endif
};

