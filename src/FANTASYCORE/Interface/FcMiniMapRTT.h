#pragma once

#include "FCinterfaceObj.h"

#include "3DDevice.h"
#include "BsOffScreenMgr.h"
#include "BsGenerateTexture.h"

struct UVImage;
class CBsUiSourceMapTexture;
class CFcTroopObject;

class CFcUiMiniMap : public CFcInterfaceObj
{
public:
	CFcUiMiniMap();
	~CFcUiMiniMap();
	
	void Process() {}
	void Update();

	void PointOn(bool bShow, const char *szName, CFcTroopObject *pTroop, DWORD nTick);
	void AreaOn(bool bShow, const char *szName, float fStartX, float fStartY, float fEndX, float fEndY, DWORD nTick);
	
	D3DXVECTOR2		SetViewToMap(float fViewX, float fViewY);
	D3DVIEWPORT9*	GetViewport()		{ return m_pViewport; }
	float			GetMapSightRadian()	{ return m_fMapSightRadian; }

protected:
	void ReleaseData();

	void	LoadViewPort();
	void	LoadOffset();
	void	LoadMiniMap();
	void	GetFileName(char *szBuf,char *szFileName);
	void	LoadRtt();
	void	GetMinimapUVPos(int &nU1, int &nV1, int &nU2, int &nV2);

protected:
	D3DVIEWPORT9*	m_pViewport;
	float			m_fMapWidth;
	float			m_fMapHeight;
	float			m_fMapRate;
	
	float			m_fMapSightRadian;

	SIZE			m_MiniMapSize;
	float			m_fMiniMapRate;
	float			m_fMiniMapRateX;
	float			m_fMiniMapRateY;
	float			m_fMiniMapAddPosX;
	float			m_fMiniMapAddPosY;

	int				m_nMapU1;
	int				m_nMapV1;
	int				m_nMapU2;
	int				m_nMapV2;

	int				m_nMapTexID;
	
	DWORD			m_hSourceRTTHandle;
	int				m_nSourceTextureId;

	DWORD			m_hAlphaRTTHandle;
	int				m_nAlphaTextureId;
};


struct PointInfo
{
	int   nCount;		//증가
	int   nTimeTick;
	CFcTroopObject *pTroop;
	char szName[64];

	PointInfo()
	{
		nCount = 0;
		nTimeTick = 0;
		pTroop = NULL;
		szName[0] = 0;
	};
};

struct AreaInfo
{
	int   nCount;		//증가
	int   nTimeTick;
	float fsX, fsY;
	float feX, feY;
	char szName[64];

	AreaInfo()
	{
		nCount = 0;
		nTimeTick = 0;
		fsX = fsY = 0.f;
		feX = feY = 0.f;
		szName[0] = 0;
	};
};

struct	_UNDER_ATTACT_SIGN
{
	_UNDER_ATTACT_SIGN(){
		init();
	};

	void init()
	{
		bCheck = false;
		nCallTick = -1;
		pos = D3DXVECTOR2(0.f,0.f);
	};

	bool		bCheck;
	int			nCallTick;
	D3DXVECTOR2 pos;
};

class CBsUiSourceMapTexture : public CBsGenerateTexture
{
public:
	CBsUiSourceMapTexture(CFcUiMiniMap* pMiniMap);
	~CBsUiSourceMapTexture() {}

	virtual void Create(int nTextureWidth, int nTextureHeight, float fStartX = 0.f, float fStartY = 0.f, float fWidth = 1.f, float fHeight = 1.f, bool bAlpha = false);
	void Render(C3DDevice *pDevice);
	void InitRenderRTT();
	void PointOn(bool bShow, const char *szName, CFcTroopObject *pTroop, DWORD nTick);
	void AreaOn(bool bShow, const char *szName, float fStartX, float fStartY, float fEndX, float fEndY, DWORD nTick);

protected:
	void Release();
	
	void DrawTroopList();
	void DrawTroop(int* pTroopList, int nTroopCount, bool bEnemy, bool bAir);
	void DrawPlayer();
	void DrawArrowSet();
	void DrawPlayerWave(float fViewX, float fViewY);
	void DrawPlayerArrow(float fViewX, float fViewY, float fAngle);
	void DrawUnit(float fPosRatioX, float fPosRatioY, int nUVId);
	void DrawPoint();
	void DrawCamDir();
	void DrawUnderAttack();
	void DrawSignUnderAttack(_UNDER_ATTACT_SIGN* info);
	void DrawSignUnderAttackCircle(_UNDER_ATTACT_SIGN* info);
	void DrawArea();
	void DrawCircleSign(D3DXVECTOR2 pos, int nTick);

protected:
	CFcUiMiniMap*				m_pMiniMap;

	int							m_nSightTexID;
	std::vector<PointInfo *>	m_PointInfoList;
	std::vector<AreaInfo *>		m_AreaInfoList;
	_UNDER_ATTACT_SIGN			m_underAttackSign[50];
};


class CBsUiAlphaMapTexture : public CBsGenerateTexture
{
public:
	CBsUiAlphaMapTexture(CFcUiMiniMap* pMiniMap);
	~CBsUiAlphaMapTexture() {}

	virtual void Create(int nTextureWidth, int nTextureHeight, float fStartX = 0.f, float fStartY = 0.f, float fWidth = 1.f, float fHeight = 1.f, bool bAlpha = false);
	void Render(C3DDevice *pDevice);
	void InitRenderRTT();

protected:
	void Release();

protected:
	CFcUiMiniMap*		m_pMiniMap;
	int					m_nSightTexID;
};
