#pragma once
#include "BsWorld.h"

#define MAX_VISIBLE_ZONE_COUNT 200
#define DISTANCE_PER_CELL 200.0f

#define INT_DISTANCE_PER_CELL 200

struct HFVERTEX
{
	D3DXVECTOR3 vecPos;
	D3DXVECTOR3 vecNor;
	DWORD		dwMulColor;
	DWORD		dwAddColor;
	D3DXVECTOR2 vecUV;
};

class CBsHFWorld : public CBsWorld
{
public:
	CBsHFWorld(int nXSizePerZone=40, int nYSizePerZone=40);
	virtual ~CBsHFWorld();

protected:
	int m_nWorldXSize;
	int m_nWorldZSize;

	short int *m_pHeight;
	DWORD*		m_pdwMul;
	DWORD*		m_pdwAdd;

	LPDIRECT3DVERTEXBUFFER9 m_pVB[MAX_VISIBLE_ZONE_COUNT];
	LPDIRECT3DINDEXBUFFER9 m_pIB;

	int m_nZoneIndex[MAX_VISIBLE_ZONE_COUNT];
	bool m_bUse[MAX_VISIBLE_ZONE_COUNT];
	int m_nUseCount[MAX_VISIBLE_ZONE_COUNT];

	int m_nZoneCheckOffSet;				// Zone Visible Check Range OffSet from CameraZone

	int m_nZoneHoriCount;
	int m_nZoneVertCount;
	int m_nXSizePerZone;
	int m_nZSizePerZone;

	float *m_pMaxHeight;
	float *m_pMinHeight;

	int m_nVertexCountPerZone;
	int m_nFaceCountPerZone;

	int m_nIndexCountPerZone;

	int			m_nHFMaterialIndex;

	D3DXHANDLE m_hLightDir;
	D3DXHANDLE m_hLightDiffuse;
	D3DXHANDLE m_hLightSpecular;
	D3DXHANDLE m_hLightAmbient;
	D3DXHANDLE m_hFogFactor;
	D3DXHANDLE m_hFogColor;

	D3DXHANDLE m_hLayer0Texture;
	D3DXHANDLE m_hLayer1Texture;
	D3DXHANDLE m_hLayer2Texture;
	D3DXHANDLE m_hLayer3Texture;
	D3DXHANDLE m_hLayer4Texture;
	D3DXHANDLE m_hBlendTexture;

	D3DXHANDLE m_hShadowTexture;

	D3DXHANDLE m_hWVP;
	D3DXHANDLE m_hWV;
	D3DXHANDLE m_hWVIT;
	D3DXHANDLE m_hShadowMatrix;

	int			m_nTerrainVertexDeclIndex;

	int m_nTextureIndex[6];

	int m_nBlendTexture;

	void CheckVisibleZone();			// Rendering 하기 전에 Visible Zone인지 검사!!

public:
	virtual void Clear();
	virtual void Create(int nSizeX, int nSizeZ, short int *pHeightp, const char **ppFileList,const char* pszLightMapFileName, DWORD *pMulColor, DWORD *pAddColor);
	virtual	void Reload();

	virtual void Process();
	virtual void Render(C3DDevice *pDevice);
	void CheckVisibleZone(D3DXMATRIX *pCamera);

	virtual bool IsVisibleTestFromFrustum(int nIndex);
	void CreateVisibleZone(int nIndex);

	int	GetWorldXSize()	{	return m_nWorldXSize;	}
	int GetWorldZSize()	{	return m_nWorldZSize;	}

	int SearchEmptyZone();
	D3DXVECTOR3 GetNormalVector(int nCellX, int nCellY);
	float GetLandHeight(float fX, float fZ, D3DXVECTOR3 * N=NULL);
	float GetFlatHeight(int nX, int nZ) { return (float)m_pHeight[ nX + (m_nWorldXSize+1) * nZ ]; } 
	short* GetHeightBufferPtr() { return m_pHeight;}

	float GetHeight(float fX, float fZ);
	void GetSlope(float fX, float fZ, float& fVX, float& fVZ);

	int ChangeLayer(int nLayerIndex, int nTextureIndex)
	{
		int nRet = m_nTextureIndex[nLayerIndex];
		m_nTextureIndex[nLayerIndex] = nTextureIndex;
		return nRet; 
	}

#ifdef _USAGE_TOOL_
	// Tool에서만 사용합니다.
	void	SetHeightBufferPtr(short* pHeight)
	{
		m_pHeight = pHeight;
	}

	void	SetMulColorBufferPtr(DWORD* pdwMul)
	{
		m_pdwMul = pdwMul;
	}

	void	SetAddColorBufferPtr(DWORD* pdwAdd)
	{
		m_pdwAdd = pdwAdd;
	}

#endif
};