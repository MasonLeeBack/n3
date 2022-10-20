#ifndef __FC_MINIMAP_H__
#define __FC_MINIMAP_H__
#include "FCinterfaceObj.h"
class CFcWorld;
class C3DDevice;
	
class CFcMinimapObjectBase;
class CFCMinimapBackGround;
class CFCMinimapUnit;
class CFcMinimapObj;
class CFcTroopObject;

struct UVImage;

//--------------------------------------------------------------------------------------------
class CFCMinimapObject
{
public:
	CFCMinimapObject(){};
	virtual ~CFCMinimapObject(){};
	virtual void Draw(){};
	virtual void Initialize(){};
};


//--------------------------------------------------------------------------------------------
class CFcMinimap :  public CFcInterfaceObj
{
public:
	CFcMinimap();
	~CFcMinimap();
	
	void Process()						{}
	void Update();
	void Zoom(float fScale);
	void MoveX(float fX);
	void MoveY(float fY);
	void ShowToggle();
	void ZoomProcess();
	void KeyInput(int nPlayerID, int KeyType,int PressTick);	
	void ZoomChange(float fZoom){m_fNewZoom = fZoom;}
    void PointOn(bool bShow,const char *szName,CFcTroopObject *pTroop,float fPosX,float fPosY,DWORD nTick);

protected:
	bool				  m_bShowMinimap;
	float				  m_fMoveSpeed;
	int					  m_nCurAttackIconIndex;

	std::vector<CFCMinimapObject *>m_ObjList;
public:
	static float m_fStartX;
	static float m_fStartY;
	static float m_fEndX;
	static float m_fEndY;
	static float m_fScaleX;
	static float m_fScaleY;

	static float m_fSaveStartX;
	static float m_fSaveStartY;
	static float m_fSaveEndX;
	static float m_fSaveEndY;

	float m_fZoom;
	float m_fNewZoom;

	DWORD	m_hMapRTTHandle;
	int		m_nMapTextureId;

	

};

//--------------------------------------------------------------------------------------------
class CFCMinimapBackGround : public CFCMinimapObject
{
public:
	CFCMinimapBackGround();
	~CFCMinimapBackGround();
	void Initialize();
	void Draw();
	//CFcDrawInfo *GetMapDrawInfo(){return &m_Map;}
protected:
    void GetFileName(char *szBuf,char *szFileName);
	void DrawMap();

protected:
	int m_nMapTexId;
};
//--------------------------------------------------------------------------------------------


struct MinimapPointInfo
{
    int   nMarkIndex;
    bool  bUpDown;
    int   nTimeTick;
    float fPointX;
    float fPointY;
    CFcTroopObject *pTroop;
	char szName[64];

    MinimapPointInfo()
    {
        nMarkIndex = 0;
        bUpDown = false;
        nTimeTick = 0;
        fPointX = 0.f;
        fPointY = 0.f;
        pTroop = NULL;
		szName[0] = 0;

    };
};

#define MAX_MARK_FRAME	4
class CFCMinimapUnit : public CFCMinimapObject
{
public:
	CFCMinimapUnit();
    ~CFCMinimapUnit();
	
	void Initialize(CFCMinimapBackGround *pBG);

	void Draw();
	void RenderProcess();

	void DrawTroop();
	void DrawPlayerWave(D3DXVECTOR2 pos);
	void DrawPlayerArrow(D3DXVECTOR2 pos, float fAngle);
	void DrawUnit(float fPosRatioX, float fPosRatioY, UVImage* pImg, float fAngle = 0.f);


	void DrawBorder();
	void DrawMark();
    void PointOn(bool bShow,const char *szName,CFcTroopObject *pTroop,float fPosX,float fPosY,DWORD nTick);
	
protected:
	CFCMinimapBackGround *m_pBG;
	int m_BattleMarkIndex;
    std::vector<MinimapPointInfo *>m_PointInfoList;
	
	float m_fZoomX;
	float m_fZoomY;
	int m_prtPosX;
	int m_prtPosY;
	int m_prtScaleX;
	int m_prtScaleY;
};

//typedef CSmartPtr<CFcMinimap> MinimapObjHandle;
#endif
