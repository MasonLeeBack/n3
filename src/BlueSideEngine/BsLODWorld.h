#pragma once
#include "BsHFWorld.h"
#include "BsTerrainZone.h"

class CBsLODWorld : public CBsHFWorld
{
public:
	CBsLODWorld(int nXSizePerZone=40, int nYSizePerZone=40);
	virtual ~CBsLODWorld();

	void Create(int nSizeX, int nSizeZ, short int *pHeightp, const char **ppFileList, const char* pszLightMapFileName, DWORD *pMulColor, DWORD *pAddColor);
	void Clear();
	void Reload();

	float GetRcpWorldXSize()	{	return	m_fRcpWorldXSize;	}
	float GetRcpWorldZSize()	{	return	m_fRcpWorldZSize;	}

	int	  GetWorldLightMap()	{	return m_nLightMapTexture;	}
	void  ClearWorldLightMap(void)	{	SAFE_RELEASE_TEXTURE(m_nLightMapTexture);	}
	void  LoadWorldLightMap(const char* szLightMapFileName)
	{
		SAFE_RELEASE_TEXTURE(m_nLightMapTexture);
		m_nLightMapTexture = g_BsKernel.LoadTexture(szLightMapFileName);
	}

	virtual void Render(C3DDevice *pDevice);

	void	ForceLODLevel(int nLevel) { m_nForceLODLevel = nLevel; }	// 음수면 거리에따라 자동으로 세팅하고(디폴트) 양수면 LOD Level을 그값으로 고정시킴

#ifdef _USAGE_TOOL_
	D3DXHANDLE			m_hAttributeMap;
	int					m_nAttributeTextureIndex;
	void				SetAttributeTexture(int nTextureIndex)	{	m_nAttributeTextureIndex = nTextureIndex;	}
#endif	// For Map Tool
protected:
	CBsTerrainZone*		m_pTerrainZones;	// 모든 Zone을 가짐

	int		m_nXVertexCountPerZone;
	int		m_nZVertexCountPerZone;

	std::vector<CBsTerrainZone*> m_DrawZoneList;

	// Zone의 LOD Level에 따른 모든 경우의 Index buffer를 미리 만들어 둡니다.
	DETAIL_LEVEL		m_DetailLevel[TOTAL_LEVELS];

	int					m_nTerrainTechnique;
	D3DXHANDLE			m_hLightMap;

	int					m_nLightMapTexture;
	float				m_fRcpWorldXSize;
	float				m_fRcpWorldZSize;
public :
	int							m_nForceLODLevel;

	void WriteZoneData(IDirect3DVertexBuffer9* pVB, int nZoneIndex);
	void GenerateDetailLevel();
	void TiledZoneVisibleTest();
	void ComputeLevelOfZone(int ZoneIndex);
	void RefreshZone(int nZoneIndex);

	void	SetTerrainTechnique(int nTechIndex)	{	m_nTerrainTechnique = nTechIndex;	}
	int		GetTerrainTechnique()	{	return m_nTerrainTechnique;	}
};

inline bool CompareTile(CBsTerrainZone* lhs,CBsTerrainZone* rhs)
{
	if(lhs->fCameraDistance<rhs->fCameraDistance)
		return true;
	return false;
}