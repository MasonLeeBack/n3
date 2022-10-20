#pragma once

#include "tpGrammer.h"
#include "FcGlobal.h"

struct WorldMapPointInfo
{
	WorldMapPointInfo()
	{
		nId = -1;
		pos = D3DXVECTOR2(0.f, 0.f);
		posNM = D3DXVECTOR2(0.f, 0.f);
		szImageNM = NULL;
		szImage = NULL;
		szStage = NULL;
		nGuadian_L = -1;
		nGuadian_R = -1;
	};

	int				nId;				// GAME_STAGE_ID
	D3DXVECTOR2 	pos;				// point pos
	D3DXVECTOR2 	posNM;				// area name pos
	char*			szImageNM;			// area name image
	char*			szImage;			// area image
	char*			szStage;			// map name
	int				nGuadian_L;			// Guadian left
	int				nGuadian_R;			// Guadian Right
	GuardianTroop	GuadianType[4];		// Guadian type, one hand, two hand, spear, arrow¼ø...
};


struct WorldMapPassInfo
{
	WorldMapPassInfo()
	{
		nStartId = -1;
		nEndId[0] = nEndId[1] = nEndId[2] = -1;
	};

	int		nStartId;
	int		nEndId[3];
};

typedef std::vector<WorldMapPointInfo>		WorldMapPointLIST;
typedef std::vector<WorldMapPassInfo>		WorldMapPassList;


class FcMenuWorldMapInfo
{
public:
	FcMenuWorldMapInfo();
	~FcMenuWorldMapInfo()					{ Release(); }
	
	void Release();
	void Load();

	WorldMapPointLIST*	GetPointList()		{ return &m_PointList; }
	WorldMapPassList*	GetPathList()		{ return &m_PathList; }

	int					GetStartPointId()	{ return m_nStartPointId; }
	int					GetStartPathId()	{ return m_nStartPathId; }


protected:
	bool Load(char* szFileName);

	bool LoadPoint(tpGrammer* pGrammer, WorldMapPointInfo* pInfo);
	bool LoadStage(tpGrammer* pGrammer, WorldMapPassInfo* pInfo);

protected:
	WorldMapPointLIST		m_PointList;
	WorldMapPassList		m_PathList;
	
	int				m_nStartPointId;
	int				m_nStartPathId;
};

//-------------------------------------------------------------------
//-------------------------------------------------------------------
struct WorldMapArrowInfo
{
	int	nId;
	D3DXVECTOR2 pos1;
	D3DXVECTOR2 pos2;

	int nUVId;
	D3DXCOLOR color;
};

typedef std::vector<WorldMapArrowInfo>		WorldMapArrowLIST;

struct SpriteInfo
{
	int	nId;
	int nTexID;
	D3DXVECTOR2 pos;
	D3DXCOLOR color;
};

struct WorldMapIconInfo
{
	int nId;
	int u1, v1, u2, v2;
	int cx, cy;
	int posx, posy;
};

#define BRIEFING_VIEW_TEXT_COUNT	5

struct BriefingTextInfo
{
	BriefingTextInfo()
	{
		nTick = 0;
		nTextID = -1;
	};

	int nTick;
	int nTextID;
};

class FcMenuWorldMap
{
public:
	FcMenuWorldMap();
	~FcMenuWorldMap();

	//bool Load(char* szFileName);
	void Create(int x, int y);
	void Release();

	void Process();
	void Update();

	void SetExitToPrev(bool bFlag)				{ m_bPrev = bFlag; }
	void SetExitToNext(bool bFlag)				{ m_bNext = bFlag; }
	bool IsExitToPrev()							{ return m_bPrev; }
	bool IsExitToNext()							{ return m_bNext; }
	void SetEnableKey(bool bFlag)				{ m_bEnableKey = bFlag; }

	int	 GetStageId()							{ return m_nStageId; }
	bool IsEnableGuadianMenu()					{ return m_bEnableGuadianMenu; }
	char* GetMapFileName()						{ return m_szMapFileName; }

	bool IsBriefing()							{ return m_bBriefing; }

	bool IsEnable()								{ return m_bEnable; }
	bool IsShowOn()								{ return m_bShowOn; }
	void SetEnable(bool bEnable)				{ m_bEnable = bEnable; }
	void SetShowOn(bool bShowOn)				{ m_bShowOn = bShowOn; }

	int GetTargetPointId()						{ return m_TargetPointId; }

protected:
	void GetEnableStageList();

	void ProcessWorldMap();
	void OnInputKeyforWorldMap();
	void ProcessBriefing();

	void UpdateWorldMap();
	void UpdateStagePoint();
	void UpdateAreaName();
	void UpdateArrow();
	void UpdateCursor();

	void MoveArrow();
	bool LockArrow();

	//test
	void UpdateArrowList();
	void TestPoint(int x, int y, D3DXCOLOR color);

	//briefing
	void UpdateBriefing();
	void UpdateBriefingText(int x1, int y1, int x2, int y2);
	void LoadBriefingImage();
	
	int GetPointUVID(int nStageID);
	bool IsClearStagePoint(int nStageID);
	bool IsNextStagePoint(int nStageID);

	void UpdateEnableGuadianMenu();

protected:
	bool	m_bEnable;
	bool	m_bShowOn;

	int						m_nClearIconTexId;
	int						m_nGoIconTexId;
	int						m_nDisIconTexId;
	int						m_nWorldMapTexId;
	D3DXVECTOR2				m_MapPos;
	D3DXVECTOR2				m_MapSize;
	
	WorldMapArrowInfo		m_ArrowInfo;
	SpriteInfo				m_CursorInfo;

	WorldMapPointLIST*		m_pPointList;
	int						m_nStartPointId;
	WorldMapPassList*		m_pPathList;
	int						m_nStartPathId;
	
	std::vector<int>		m_EnableStageList;

	WorldMapArrowLIST		m_ArrowList;
	int						m_nAreaNameTexID;
	
	D3DXVECTOR2		m_CsrPos;
	bool			m_bCursorLocked;
	int				m_nCursorLockCounter;
	float			m_fCsrSpeedX;
	float			m_fCsrSpeedY;
	
	int				m_TargetPointId;

	bool			m_bBriefing;
	bool			m_bWorldToBriefing;
	int				m_nChangeTick;

	bool			m_bPrev;
	bool			m_bNext;
	bool			m_bEnableKey;

	int				m_nStageId;
	char*			m_szMapFileName;

	int				m_nBriefingTextCount;
	int				m_nBriefingTime;

	int				m_nStageTexId;
	bool			m_bEnableGuadianMenu;

	BriefingTextInfo	m_BriefingTick[BRIEFING_VIEW_TEXT_COUNT];
	
	
};