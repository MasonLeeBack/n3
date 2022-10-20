#pragma once
#include <vector>
#include "Singleton.h"
#include "BsKernel.h"
using namespace std;

#define ZONE_SIZE							2000
#define MAX_RENDER_BILLBOARD				15000
#define CHECK_RANGE							5000.f
#define MAX_NUM_OBJECT_BILLBOARD			2000
#define MAX_NUM_OBJECT						100

const float SPRITE_RANGE					((1.f+(1.f/63.f))/8.f);

#ifdef _XBOX
#define NUM_VERTEX_PER_BILLBOARD			4
#else
#define NUM_VERTEX_PER_BILLBOARD			6
#endif

#define BILLBOARD_CHANGE_LIMIT				400.f

struct BILLBOARD_VERTEX
{
	D3DXVECTOR3			WPos;
	D3DXVECTOR2			Tex;
	D3DXVECTOR2			LPos;
};

struct BILLBOARD_DATA
{
	D3DXVECTOR3				vecWorldPos;
	D3DXVECTOR2				vecOffset;
	int						nBufferIndex;
};

struct ZONE_DATA
{
public:
	ZONE_DATA() { bZoneEnable = false; pRenderVertices = NULL;	fZoneMaxHeight=-FLT_MAX;	fZoneMinHeight=FLT_MAX; }
	~ZONE_DATA() 
	{ 
		arrayBillboardData.clear();
		if (pRenderVertices) {
			delete [] pRenderVertices;
			pRenderVertices = NULL;
		}
		bZoneEnable = false;
	}

	bool					bZoneEnable;
	std::vector<BILLBOARD_DATA>		arrayBillboardData;
	BILLBOARD_VERTEX*		pRenderVertices;

	Box3					ZoneBoundingBox;

	float					fZoneMaxHeight;
	float					fZoneMinHeight;
};

struct SPRITE_DATA
{
	int						nIndex;
	D3DXVECTOR2				vecTexCoord[2];
};

class CBsBillboardMgr : public CSingleton<CBsBillboardMgr> {
public:
	CBsBillboardMgr();
	virtual ~CBsBillboardMgr();

	HRESULT			InitBillBoard(int iMapXSize, int iMapZSize, const char* pTexName=NULL, int nDataNum=0, SPRITE_DATA* pData=NULL);
	HRESULT			SpriteTextureLoad(LPCSTR szFileName);
	void			ReleaseDeviceData(void);

	void			PutPosToPool(D3DXVECTOR3 vecPos, D3DXVECTOR2 vOffset, int nBillboardIndex);
	void			OptiZone(void);
	void			RefreshZone(D3DXVECTOR3 vecPos);
	void			RefreshZone(int nXZoneIndex, int nZZoneIndex);
	void			ClearZone(int nXZoneIndex, int nZZoneIndex);

	virtual void	Render(C3DDevice* pDevice);

protected:
	int										m_nMaterialIndex;
	int										m_nBillboardVertexDeclIndex;

	D3DXHANDLE								m_hTexture;		// Billboard Texture Handle
	D3DXHANDLE								m_hInvV;
	D3DXHANDLE								m_hVP;
	D3DXHANDLE								m_hCommon;
	D3DXHANDLE								m_hFogFactor;
	D3DXHANDLE								m_hFogColor;
	D3DXHANDLE								m_hLightAmbient;
	D3DXHANDLE								m_hRCPXSize;
	D3DXHANDLE								m_hRCPZSize;
	D3DXHANDLE								m_hLightTexture;

private:
	ZONE_DATA*								m_pZoneList;
	std::vector<int>						m_arrayZoneIndices;
	int										m_nXZoneCount, m_nZZoneCount;

	int										m_nTextureIndex;
	int										m_nNumRenderBillboard;
	LPDIRECT3DVERTEXBUFFER9					m_pVB;
	SPRITE_DATA*							m_pArraySprite;
	int										m_nSpriteNum;

	D3DXHANDLE								m_hObjTexture;

	int										m_nRenderBillboard;

	bool									m_bRenderReady;

	void									CreateBillboardZone(int iMapXSize, int iMapZSize);
	void									PreRender(void);
	bool									IsVisibleTest(int iZoneIndex);
	D3DXVECTOR2*							FindTexCoord(int nBufferIndex);
};

struct OBJ_BILLBOARD_VERTEX {
	D3DXVECTOR4			WPos; // w == animindex
	D3DXVECTOR2			Tex;
	D3DXVECTOR2			LPos;
	D3DCOLOR			Color;
};

struct BillboardAnimOffset
{
	float					fOffset;
	int						nAnimType;
	int						nAniAttr;
};

struct OBJECT_BILLBOARD_DATA
{
public:
	OBJECT_BILLBOARD_DATA()
	{
		pnSkinIndices = NULL;
		nSkinCount = 0;
		nTextureIndex = -1;
		pVB = NULL;
		nRenderBillboardCount[0] = 0; 
		nRenderBillboardCount[1] = 0; 
		pVertices[0] = NULL;
		pVertices[1] = NULL;
		pMaterialAmb = pMaterialDiff = NULL;
		pAnimationOffset = NULL;
	}
	~OBJECT_BILLBOARD_DATA()
	{ 
		if (pnSkinIndices) { delete [] pnSkinIndices; pnSkinIndices = NULL; }
		if (pMaterialAmb) { delete [] pMaterialAmb; pMaterialAmb = NULL; }
		if (pMaterialDiff) { delete [] pMaterialDiff; pMaterialDiff = NULL; }
		SAFE_RELEASE_TEXTURE(nTextureIndex);
		if (pVertices[0]) { delete [] pVertices[0];  pVertices[0] = NULL; }
		if (pVertices[1]) { delete [] pVertices[1];  pVertices[1] = NULL; }
		if (pAnimationOffset) { delete [] pAnimationOffset; pAnimationOffset = NULL; }
		SAFE_RELEASE(pVB);
	}
	int*					pnSkinIndices;
	int						nSkinCount;
	D3DXVECTOR4*			pMaterialAmb;
	D3DXVECTOR4*			pMaterialDiff;
	int						nTextureIndex;
	OBJ_BILLBOARD_VERTEX*	pVertices[DOUBLE_BUFFERING];
	int						nRenderBillboardCount[DOUBLE_BUFFERING];
	LPDIRECT3DVERTEXBUFFER9	pVB;
	BillboardAnimOffset*	pAnimationOffset;
	int						nBillboardSize;
};

typedef vector<OBJECT_BILLBOARD_DATA*>		pArrayObjBillboardPool;

class CBsObjectBillboardMgr : public CBsBillboardMgr
{
public:
	CBsObjectBillboardMgr();
	virtual ~CBsObjectBillboardMgr();

	void				Clear(void);
	int					FindBillboard( int *pSkinIndex, int nNumVari );
	int					RegisterAniBillboardType( const char* szFolderName, int* pnSkinIndices, int nSkinCount, int nAniCount, int nBillboardSize);
	void				SetBillboardAniOffset(int nBillboardIndex, int nCount, float fAniOffset, int nAniType, int nAniAttr);
	HRESULT				InitBillBoard(int iMapXSize, int iMapZSize, const char* pTexName, int nDataNum, SPRITE_DATA* pData);
	void				ReleaseDeviceData(void);
	void				AddObjectBillboard(int nBillBoardIndex, D3DXVECTOR3 vWorldPos, int iSkinIndex, int iOffsetInd, float fAlpha, float fAnim);								
	virtual void		Render(C3DDevice* pDevice);

	void				SetCurrentBillboardRange(float fRange)
	{
		m_fCurrentRange = fRange;
		m_fLimitMinimum = m_fCurrentRange - BILLBOARD_CHANGE_LIMIT;
		m_fLimitMaximum = m_fCurrentRange + BILLBOARD_CHANGE_LIMIT;
	}
	float				GetCurrentBillboardRange()	{	return m_fCurrentRange; }
	float				GetLimitMinimumDistance()	{	return m_fLimitMinimum;	}
	float				GetLimitMaximumDistance()	{	return m_fLimitMaximum;	}

private:
	OBJECT_BILLBOARD_DATA*	m_arrayObjectBillboardPool[MAX_NUM_OBJECT];

	int										m_nVertexDeclIndex;

	D3DXHANDLE								m_hObjectTexture;
	D3DXHANDLE								m_hLightColor;

	int										m_nCurrentPool;
	float									m_fCurrentRange;
	float									m_fLimitMinimum;
	float									m_fLimitMaximum;
};