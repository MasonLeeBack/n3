#pragma once

#define STRING_MAX		64
#define LAYER_MAX		6
#define LAYER_ATTR_MAX	5

#ifndef __TCHAR_DEFINED
typedef char            _TCHAR;
typedef char            TCHAR;
#define __TCHAR_DEFINED
#endif

class BStream;
class MapFilePropZoneManager;

#ifdef _USAGE_TOOL_

class CBSProp;
class CBSPropManager;


// class CBSAreaManager;
template<class TPack> class CBSAreaManager;
class CBSAreaPack;
class CBSTroopPack;
class CMapAttrNode;
class CBSLineManager;
class CBSPathManager;

struct COLORVERTEX;

//typedef COLORVERTEX		BSVECTOR3;

#else

struct COLORVERTEX
{
	D3DXVECTOR3 Vertex;
	DWORD dwColor;
};

class CFcPropManager;
#include "CrossVector.h"
#include "Box3.h"

//typedef D3DXVECTOR3		BSVECTOR3;

#endif

/*==================================================================================

 * Prop 관련

 0) 4 Bytes : Prop 갯수

 1) 4 Bytes : 고유 ID
 2) 4 Bytes : Object ID
 3) 4 Bytes : SkinIndex
 4) 4 Bytes : Prop X Pos
 5) 4 Bytes : Prop Y Pos
 6) 4 Bytes : Prop Z Pos

 7) 4 Bytes : Rotate, X Scale,Y Scale,Z Scale ( 이 순서로 각각 1 Bytes 씩 저장됨 )

 8) 2 Bytes : Attribute X
 9) 2 Bytes : Attribute Y

 * 주의

 2) 번 항목은 Map Tool 에서 사용.

 7) 번 항목.

 Rotate 의 경우 1024 값을 4로 나눈 값을 사용. 255단계로 나눔.
 복원시 
 
 ( (BYTE)(Data>>24) ) * 4

 Scale 의 경우 0.01 단계로, 배율 0.5 ~ 1.5 값까지 적용됨.
 복원시

 0.5f +  float((BYTE)(Data>>16)) * 0.01f;

 8) 번 항목.

 임시 값으로 현재는 Attribute 좌표값이 적용되어 있음.
 ( ex : mapsize * 2 )


  ==================================================================================*/




#define FC_MAP_VER	101

#define MAPID_MAININFO		0x00000001
#define MAPID_HEIGHT		0x00000002

#define MAPID_BRIGHT		0x00000004
#define MAPID_COLOR			0x00000008

#define MAPID_LIGHT			0x00000010
#define MAPID_PROP			0x00000020

#define MAPID_ATTRIBUTE		0x00000040
#define MAPID_AREA			0x00000080

#define	MAPID_TROOP			0x00000100
#define MAPID_MAPSET		0x00000200

#define MAPID_WATER			0x00000400
#define MAPID_PATH			0x00000800

#define MAPID_NAVMESH		0x00001000
#define MAPID_NAVEVENT		0x00002000

#define MAPID_COLLISION		0x00004000
#define MAPID_SKINLIST		0x00008000

#define MAPID_ATTREXMAP		0x00010000
#define MAPID_BILLBOARD		0x00020000
#define MAPID_BILLBOARDEX	0x00040000
#define MAPID_PROPZONEDATA	0x00080000

#define MAPID_ATTRIBUTEEX	0x00100000		// 대각선 속성들.

// Light 가 비어있음.



#define MAPID_BILLBOARDPROP 0x20000000
#define MAPID_RESINFO		0x40000000
#define MAPID_BLENDMAP		0x80000000


#define STR_MAX		64
#define CAPTION_MAX	32

#define MAP_ATTR_SIZE 100

struct  stSkinPack
{
	int		m_nIndex;
	char*	m_szSkinname; // [STR_MAX];

	stSkinPack() : m_nIndex(-1),m_szSkinname(NULL) {}
};

#pragma pack( push )
#pragma pack( 4 )

#define PROP_PARAM_EXT_MAX	8
// 구조만.. 
struct PROP_INFO_EXT 
{
	char  szPropName[CAPTION_MAX];
	char  szExtBuffer[STR_MAX];

	DWORD	dwParamExt[PROP_PARAM_EXT_MAX];
};

union _PropPos
{
	float	fPos[3];
	DWORD	dwPos[3];
};

struct PROP_INFO
{
	int nUId;
	int nObjIndex;
	int nSkinIndex;

	_PropPos	Pos;
	DWORD dwParam1;

	char  szPropName[CAPTION_MAX];
	char  szExtBuffer[STR_MAX];
	DWORD	dwParamExt[PROP_PARAM_EXT_MAX];
};

struct SIMPLE_BILLBOARD
{
	int nBillboardIndex;
	D3DXVECTOR3	vecPos;
};



struct AREA_INFO
{
	int		nUId;
	int		nType;
	float	fSX;
	float	fSZ;
	float	fEX;
	float	fEZ;

	float	GetWidth(void)	{	return fEX - fSX;	}
	float	GetHeight(void)	{	return fEZ - fSZ;	}
};

struct AREA_ATTR 
{
	char  szAreaName[CAPTION_MAX];

	AREA_ATTR()
	{
		memset( szAreaName, 0 , CAPTION_MAX );
	}
};

#define TROOP_ATTR_STRSIZE	CAPTION_MAX

struct TROOP_ATTR
{
	int		nTroopType;
	int		nUnitType[5];

	int		nUnitRow; // Max;
	int		nUnitCol; // Min;

	int		nTeam;
	int		nGroup;

	// 추가
	int		nUId;

	int		nTroopHp;
	int		nMoveSpeed;
	int		nUnitAI;

	// 추가
	int		nEnable;
	int		nLeaderUnitType;
	int		nDirection;			// 부대 방향

	// 추가
	int		nLeaderHorseType;
	int		nUnitHorseType[3];

	//int		nReserve0;	// 여분 0
	//int		nReserve1;	// 여분 1

	int		nLeaderPos;		// 리더 위치
	int		nDensity;		// 밀도
	int		nSpecialType;	// special unit type
	int		nSUnitCount;	// Special unit count
	int		nTroopForm;		// 부대 형태

	int		nLeaderLevel;	// 리더 레벨
	int		nTroopLevel;	// 부대 레벨
	
	int		nGuardPickout;	// 호위부대 차출 : 1- 가능 0-불가.

	// 추가
	char	szCaption[CAPTION_MAX];

	// 
	struct 
	{
		char		szUnitAI[5][CAPTION_MAX];
		char		szLeaderUnitAI[CAPTION_MAX];
	} StrBlock;
	

	/*
	void	Clear(void)
	{
		nTroopType = -1;
		for(int i = 0 ; i < 5 ; ++i )
            nUnitType[i] = -1;
	}
	*/

	TROOP_ATTR()
	{
		memset( szCaption , 0 , CAPTION_MAX );
		memset( StrBlock.szUnitAI, 0 , CAPTION_MAX * 5 );
		memset( StrBlock.szLeaderUnitAI, 0 , CAPTION_MAX );
		nDirection = 0;
		nLeaderPos = -1;
		nDensity = -1;
        nSpecialType = -1;
		nSUnitCount = -1;
		nTroopForm = -1;

		nLeaderLevel = 0;
		nTroopLevel = 0;
		nGuardPickout = 1;
	}

	void	SetDefExtBuffer(void)
	{
		// nLeaderPos = -1;	nDensity = -1	nSpecialType = -1;
		// nSUnitCount = -1;	nTroopForm = -1;
		nLeaderLevel = 0;
		nTroopLevel = 0;
		nGuardPickout = 1;
	}
};

struct TROOP_INFO
{
	AREA_INFO	m_areaInfo;
	TROOP_ATTR	m_troopAttr;
};

struct AREA_PACK 
{
	AREA_INFO	m_areaInfo;
	AREA_ATTR	m_areaAttr;
};

#ifdef _USAGE_TOOL_
#pragma pack( push )
#pragma pack( 4 )
#endif

struct SET_INFO
{
	int				nVersion;			// 4

	CCrossVector	crossLight;				// 12 * 4 : 48
	
	D3DCOLORVALUE	clrLightAmbient;	// 16
	D3DCOLORVALUE	clrLightDiffuse;	// 16
	D3DCOLORVALUE	clrLightSpecular;	// 16
	D3DXVECTOR4		vecFogColor;
	float			fNear;
	float			fFar;
	float			fBlur;
	float			fGlow;

	// 추가
	float			fProjectionZMax;
	float			fSightFar[2];	// Near,Normal

	// 추가
	struct stScene
	{
        float			fSceneIntensity;
        float			fBlurIntensity;
        float			fGlowIntensity;
		float			fHighlightThreshold;

		stScene()
		{
			fSceneIntensity = 1.f;
			fBlurIntensity = 0.f;
			fGlowIntensity = 0.f;
			fHighlightThreshold = 0.9f;
		}

	} SceneData;

	void	Clear(void)
	{
		memset( this, 0, sizeof(SET_INFO) );
		crossLight.Reset();
		SceneData.fSceneIntensity = 1.f;
		SceneData.fBlurIntensity = 0.f;
		SceneData.fGlowIntensity = 0.f;
		SceneData.fHighlightThreshold = 0.9f;
	}

	float	GetSceneIntensity(void)	{	return SceneData.fSceneIntensity;	}
	float	GetBlurIntensity(void)	{	return SceneData.fBlurIntensity;	}
	float	GetGlowIntensity(void)	{	return SceneData.fGlowIntensity;	}
	float	GetThreshold(void)		{	return SceneData.fHighlightThreshold;	}

	void	SetSceneIntensity(float fVal)	{	SceneData.fSceneIntensity = fVal;	}
	void	SetBlurIntensity(float fVal)	{	SceneData.fBlurIntensity = fVal;	}
	void	SetGlowIntensity(float fVal)	{	SceneData.fGlowIntensity = fVal;	}
	void	SetThreshold(float fVal)		{	SceneData.fHighlightThreshold = fVal;	}

};

#ifdef _USAGE_TOOL_
#pragma pack( pop )
#endif

struct FILENAME_MOLD 
{
	int		nLen;
	char*	m_pszFileName;
};

// 맵툴용
struct RESOURCE_INFO
{
	int		nTroopID;
	int		nAreaID;
	int		nPathID;
};


struct PATH_INFO
{
	int		nID;
	int		nParam;
	int		nReserved[4];
	char	szCaption[CAPTION_MAX];
};

struct PATH_DATA
{
	int		nVecCnt;

#ifdef _USAGE_TOOL_
	std::vector<COLORVERTEX>	vecPosPool;
#else
	std::vector<D3DXVECTOR3>	vecPosPool;
#endif

};

struct PATH_PACK
{
	PATH_INFO	pathInfo;
	PATH_DATA	pathData;
};


#pragma pack( pop )

typedef std::vector<int>	intVector;

class CBSMapCore
{
protected :

#ifdef _USAGE_TOOL_
	CBSPropManager	*m_pPropManager;
	CCrossVector*	m_pLightCross;

	CBSAreaManager<CBSAreaPack> *m_pAreaManager;
	CBSAreaManager<CBSTroopPack> *m_pTroopManager;
	SET_INFO*		m_pDefaultSetInfo;
	CMapAttrNode*				m_pAttrManager;
	CBSLineManager*				m_pLineManager;
	CBSPathManager*				m_pPathManager;

	RESOURCE_INFO	m_ResInfo;		// ID 부여를 위한 정보들.
#endif

public:
	CBSMapCore();
	~CBSMapCore();

	//
	int		GetXSize()				{	return m_nXSize;		}
	int		GetYSize()				{	return m_nYSize;		}

#ifdef _USAGE_TOOL_

	void	SetXSize(int cx)		{	m_nXSize = cx;			}
	void	SetYSize(int cy)		{	m_nYSize = cy;			}

	void	SetPropMan(CBSPropManager* pMan)	{	m_pPropManager = pMan;	}
	void	SetLightCross(CCrossVector*	pCross)	{	m_pLightCross = pCross;	}
	void	SetAreaMan(CBSAreaManager<CBSAreaPack>* pMan)	{	m_pAreaManager = pMan;	}
	void	SetTroopMan(CBSAreaManager<CBSTroopPack>* pMan)	{	m_pTroopManager = pMan;	}
	void	SetDefSetInfo(SET_INFO* pInfo)					{	m_pDefaultSetInfo = pInfo;}
	void	SetAttrMap(CMapAttrNode* pAttrNode)				{	m_pAttrManager = pAttrNode;	}
	void	SetLineManager(CBSLineManager* pLineMan)		{	m_pLineManager = pLineMan;	}
	void	SetPathManager(CBSPathManager* pPathMan)		{	m_pPathManager = pPathMan;	}
	void	SetAttrByteMap(BYTE* pBuffer,int nXSize,int nYSize)
	{
		m_pAttrByteMap = pBuffer;
		m_nABXSize = nXSize;
		m_nABYSize = nYSize;
	}

	RESOURCE_INFO&	GetResInfo(void)						{	return m_ResInfo;	}

	bool	SetNavList(int nVecCnt,int nIdxCnt,D3DXVECTOR3* pSrc,int* pIdxSrc);
	bool	SetNavEventIndexBuffer(int nIndex,int* pIdxBuffer,int nCount);

	bool	SetColList(int nVecCnt,int nIdxCnt,D3DXVECTOR3* pSrc,int* pIdxSrc);
	bool	SetColEventIndexBuffer(int nIndex,int* pIdxBuffer,int nCount);

	bool	SetColBuffer(int nVecCnt,D3DXVECTOR3* pSrc);
	BOOL	HasAttrExBuffer(VOID)	{	return m_bHasExAttrMap;	}

#endif
	bool	CreateNavEventIndexBuffer(int nCount);
	bool	ClearNavEventIndexBuffer(void);

	bool	CreateColEventIndexBuffer(int nCount);
	bool	ClearColEventIndexBuffer(void);

	SET_INFO	*GetSetInfo() { return &m_LightInfo; }

	void	SetSkyboxName(char *szFilename);
	const char*	GetSkyboxName(void);
	void	ClearSkyboxName(void);

	void	SetCubeMapName(char *szFilename);
	const char* GetCubeMapName(void);
	void	ClearCubeMapName(void);

	void	SetAIFileName(char *szFilename);
	const char* GetAIFileName(void);
	void	ClearAIFileName(void);

	void	SetTextFileName(char *szFilename);
	const char* GetTextFileName(void);
	void	ClearTextFileName(void);

	void	SetCameraFileName(char *szFilename);
	const char* GetCameraFileName(void);
	void	ClearCameraFileName(void);

	void	SetLightMapFileName(char *szFilename);
	const char* GetLightMapFileName(void);
	void	ClearLightMapFileName(void);

	void	SetBillboardDDSName(char* szFilename);
	const char* GetBillboardDDSName(void);
	void	ClearBillboardDDSName(void);

	void	CheckBillboardDDS(void);

	//
	short*	GetHeightBuffer(void)	{	return m_pHeightData;	}
	short*	GetHeightBuffer(int x,int y)	{	return &m_pHeightData[ x + y * m_nHeightXSize];	}
	short	GetHeightValue(int x,int y)	{	return m_pHeightData[ x + y * m_nHeightXSize];	}

	int		GetHeightXSize(void)	{	return m_nHeightXSize;	}
	int		GetHeightYSize(void)	{	return m_nHeightYSize;	}
	int		GetHeightBufSize(void)	{	return m_nHeightXSize * m_nHeightYSize;	}

	int		GetPropCount() { return (int)m_PropList.size(); }
	PROP_INFO *GetPropInfo(int nIndex) { return &(m_PropList[nIndex]); }
	std::vector<PROP_INFO>& GetPropInfoList()	{ return m_PropList; }

	int		GetBillboardCount()	{	return (int)m_BillboardList.size();	}
	SIMPLE_BILLBOARD*	GetBillboardInfo(int nIndex)	{	return &(m_BillboardList[nIndex]);	}
	std::vector<SIMPLE_BILLBOARD>& GetBillboardInfoList()	{	return m_BillboardList;	}

	int		GetTroopCount() { return (int)m_TroopList.size(); }
	TROOP_INFO *GetTroopInfo(int nIndex) { return &(m_TroopList[nIndex]); }

	int		GetAreaCount() { return (int)m_AreaList.size(); }

	AREA_INFO *GetAreaInfo(int nIndex) { return &(m_AreaList[nIndex].m_areaInfo); }
	AREA_PACK *GetAreaPack(int nIndex) { return &(m_AreaList[nIndex]);	}
	AREA_ATTR *GetAreaAttr(int nIndex) { return &(m_AreaList[nIndex].m_areaAttr); }

	int		GetPathCount() { return (int)m_PathList.size(); }
	PATH_PACK *GetPathInfo(int nIndex) { return &(m_PathList[nIndex]); }

	// Nav
	int		GetNavListCount()				{	return (int)m_NavList.size();	}
	D3DXVECTOR3& GetNavVector(int nIndex)	{	return m_NavList[nIndex];		}

	int		GetNavIndexListCount()			{	return (int)m_NavIndexList.size();	}
	int		GetNavVectorIndex(int nIndex)	{	return m_NavIndexList[nIndex];		}

	int*	GetNavIndexBuffer(void)			{	return &m_NavIndexList[0];		}
	D3DXVECTOR3*	GetNavBuffer(void)		{	return &m_NavList[0];			}

	int		GetNavEventCount(void)			{	return m_nNavEventCount;		}
	int		GetNavEventIndexBufferCount(int nIndex)
	{
		if( nIndex < m_nNavEventCount )
            return (int)m_pNavEventIndexList[nIndex].size();
		return 0;		// 에러. 없는 버퍼인덱스
	};

	int*	GetNavEventIndexBuffer(int nIndex)
	{	
		if( nIndex < m_nNavEventCount )
            return &m_pNavEventIndexList[nIndex][0];
		return NULL;
	};

	// Col
	int		GetColListCount()				{	return (int)m_ColList.size();	}
	D3DXVECTOR3& GetColVector(int nIndex)	{	return m_ColList[nIndex];		}

	// GetColIndexListCount, GetColVectorIndex 안쓴다. 갯수가 항상 0
	int		GetColIndexListCount()			{	return (int)m_ColIndexList.size();	}
	int		GetColVectorIndex(int nIndex)	{	return m_ColIndexList[nIndex];		}

	int*	GetColIndexBuffer(void)			{	return &m_ColIndexList[0];			}
	D3DXVECTOR3*	GetColBuffer(void)		{	return &m_ColList[0];				}

	int		GetColEventCount(void)			{	return m_nColEventCount;			}
	int		GetColEventIndexBufferCount(int nIndex)
	{
		if( nIndex < m_nColEventCount )
			return (int)m_pColEventIndexList[nIndex].size();
		return 0;
	}

	int*	GetColEventIndexBuffer(int nIndex)
	{
		if( nIndex < m_nColEventCount )
			return &m_pColEventIndexList[nIndex][0];
		return NULL;
	}

	void	ClearNav()						{ m_NavList.clear();	m_NavIndexList.clear(); }
	void	ClearCol()						{ m_ColList.clear();	m_ColIndexList.clear();	}

#ifndef _USAGE_TOOL_
	// Attribute
	int		GetAttrXSize()					{	return m_nAttrXSize;	}
	int		GetAttrYSize()					{	return m_nAttrYSize;	}
	BYTE	GetAttr(int x,int y)			{	return *(m_pAttrMap + x + y * m_nAttrXSize); }
	BYTE*	GetAttrBuf()					{	return m_pAttrMap; }

	BYTE	GetAttrEx(int x,int y)			{	return *(m_pAttrMapEx + x + y * m_nAttrXSize);	}
	BYTE*	GetAttrExBuf()					{	return m_pAttrMapEx;	}


	BYTE*	GetLayerAttrBuf()			{	return m_pAttrByteMap;	}
	int		GetLayerAttrXSize()			{	return m_nABXSize;		}
	int		GetLayerAttrYSize()			{	return m_nABYSize;		}
	BYTE	GetLayerAttr(int x,int y);

#endif

	void	InitHeightMap(int cx,int cy);
	void	ResetHeightMap(void);
	void	ReleaseHeightMap(void);

	void	InitColorMap(int cx,int cy);
	void	ReleaseColorMap(void);

	void	InitBrightMap(int cx,int cy);
	void	ReleaseBrightMap(void);

	void	InitAddColorMap(int cx,int cy);
	void	ReleaseAddColorMap(void);

	DWORD*	GetMulColorMap(void)	{	return m_pdwMulColor;	}
	DWORD*	GetAddColorMap(void)	{	return m_pdwAddColor;	}

	void	SetMulColor(int x,int y,DWORD dwColor)	{	m_pdwMulColor[ x + y * (m_nXSize+1)] = dwColor;	}
	void	SetAddColor(int x,int y,DWORD dwColor)	{	m_pdwAddColor[ x + y * (m_nXSize+1)] = dwColor;	}

	int		GetColorMapSize(void)		{	return m_nHeightXSize*m_nHeightYSize;	}

	//
	TCHAR*	GetLayerStr(int nIndex)	{	return m_szLayer[nIndex];	}
	TCHAR**	GetLayerStr()			{	return m_szLayer;	}
	TCHAR*	GetBlendStr(void)		{	return m_szLayer[LAYER_MAX-1];	}
	int		GetLayerStrLen(int nIndex)	{	return (int)strlen( m_szLayer[nIndex] );	}

	void	SetLayerAttrVal(int nIdx,int nVal)	{	m_nLayerAttr[nIdx] = nVal;	}
	int		GetLayerAttrVal(int nIdx)	{	return m_nLayerAttr[nIdx];	}
	int*	GetLayerAttrValBuf(void)	{	return m_nLayerAttr;		}

	void	InitLayerInfo(void);

#ifdef _USAGE_TOOL_

	void	SetLightDiffuse(DWORD dwColor)		{	m_dwLight[0] = dwColor;		}
	void	SetLightSpecular(DWORD dwColor)		{	m_dwLight[1] = dwColor;	}
	void	SetLightAmbient(DWORD dwColor)		{	m_dwLight[2] = dwColor;		}

#endif

	DWORD	GetLightDiffuse(void)				{	return m_dwLight[0];		}
	DWORD	GetLightSpecular(void)				{	return m_dwLight[1];		}
	DWORD	GetLightAmbient(void)				{	return m_dwLight[2];		}

	//
#ifdef _USAGE_TOOL_
	bool	SaveTerrain(CFile *fp);		// 맵의 Terrain 부분만 외부 파일로 저장.

	bool	SaveFile(CFile *fp);
	int		SaveMapInfo(CFile *fp);
	int		SaveResInfo(CFile *fp);
	int		SaveHeight(CFile *fp);
	int		SaveBrightMap(CFile *fp);
	int		SaveColorMap(CFile *fp);
	int		SaveProp(CFile *fp);
	int		SaveBillboard(CFile *fp);
	int		SaveBillboardEx(CFile *fp);
	int		SavePropZoneData(CFile *fp );
	int		SavePropExtensionData(CFile *fp);
	int		SaveUsedSkinList(CFile *fp);
	int		SaveAttrByteMap(CFile *fp);

	int		SaveLightInfo(CFile *fp);

	int		SaveArea(CFile *fp);
	int		SaveAreaExtensionData(CFile *fp);
	int		SaveTroop(CFile *fp);
	int		SaveMapSet(CFile *fp);

	

	int		SaveAttributeMap(CFile *fp);
	int		SaveAttributeMapEx(CFile *fp);

	int		SaveWaterLine(CFile *fp);
	int		SaveWaterLineEx(CFile *fp);
	int		SavePath(CFile *fp);
	int		SaveNavList(CFile *fp);
	int		SaveNavEventList(CFile *fp);

	int		SaveColList(CFile *fp);

	int		SaveNavFile( char* pMapFileName );
	

	bool	LoadTerrain(BStream *fp);
	
	BOOL	ChangeAttrDataIntoGameAttr(VOID);

#endif // _USAGE_TOOL_
	
	BOOL	CreateTempBuffer(int xsize,int ysize);
	VOID	DeleteTempBuffer(VOID);
	BOOL	CreateDiagonalAttrMap(BYTE* pAttrMapBuf,BYTE* pAttrMapExBuf,int xsize,int ysize);
	//
	bool	LoadAndProcessFile(BStream *fp);
	int		LoadMapInfo(BStream *fp);
	int		LoadResInfo(BStream *fp);
	int		LoadHeight(BStream *fp);
	int		LoadBrightMap(BStream *fp);
	int		LoadColorMap(BStream *fp);
	int		LoadProp(BStream *fp);
	int		LoadBillboard(BStream *fp);
	int		LoadPropZoneData(BStream *fp );
	int		LoadPropOldVer(BStream *fp);
	int		LoadPropExtensionData(BStream *fp);
	int		LoadUsedSkinList(BStream *fp);
	int		LoadAttrByteMap(BStream *fp);

	int		LoadLightInfo(BStream *fp);

	int		LoadArea(BStream *fp);
	int		LoadAreaExtensionData(BStream *fp);
	int		LoadTroop(BStream *fp);
	int		LoadMapSet(BStream *fp);
	int		LoadAttributeMap(BStream *fp);
	int		LoadAttributeMapEx(BStream *fp);


	int		LoadWaterLine(BStream *fp);
	int		LoadWaterLineEx(BStream *fp);
	int		LoadPath(BStream *fp);
	int		LoadNavList(BStream *fp);
	int		LoadNavEventList(BStream *fp);

	int		LoadColList(BStream *fp);

	void	DeleteContents(void);

	void	SetMapFileInfo( char* pFileName );
	char*	GetMapFileName()	{ return m_cMapFileName; }

	void	SetWaterCount(int nVal)	{	m_nWaterCount = nVal;	}
	int		GetWaterCount(void)	{	return m_nWaterCount;	}

	MapFilePropZoneManager* GetPropZoneManager() { return m_pZoneManager; }

#ifdef _USAGE_TOOL_
	int		GetBillboardSkinGap(VOID)	{	return m_nBillboardIndexGap;	}
#endif

protected :

	BOOL	CreateAttrExBuffer(int xsize,int ysize)
	{
		DeleteAttrExBuffer();
		m_pAttrMapEx = new BYTE[ xsize*ysize ]; // 무조건 생성
		memset( m_pAttrMapEx, 0 , xsize*ysize );
		return TRUE;
	}

	VOID	DeleteAttrExBuffer(VOID)
	{
		if( m_pAttrMapEx )
		{
			delete [] m_pAttrMapEx;
			m_pAttrMapEx = NULL;
		}
	}

protected:
	// Height
	short*			m_pHeightData;

	int				m_nHeightXSize;
	int				m_nHeightYSize;

	// Map Size
	int				m_nXSize;
	int				m_nYSize;

	// Layer
	TCHAR	*m_szLayer[LAYER_MAX];
	int		m_nLayerAttr[LAYER_ATTR_MAX];

	// Vertex Color
	DWORD	*m_pdwMulColor;
	DWORD	*m_pdwAddColor;

	DWORD	m_dwLight[3];	// Diffuse , Specular , Ambient

	std::vector<PROP_INFO> m_PropList;
	std::vector<SIMPLE_BILLBOARD> m_BillboardList;

	std::vector<TROOP_INFO> m_TroopList;
	// std::vector<AREA_INFO> m_AreaList;
	std::vector<AREA_PACK>	m_AreaList;
	std::vector<PATH_PACK>	m_PathList;

	std::vector<int>		m_SkinIndexList;

	// 길찾기 용.
	std::vector<D3DXVECTOR3>	m_NavList;
	std::vector<int>			m_NavIndexList;
	
	int							m_nNavEventCount;
	intVector*					m_pNavEventIndexList;

	// Collision Mesh List
	std::vector<D3DXVECTOR3>	m_ColList;
	std::vector<int>			m_ColIndexList;

	int							m_nColEventCount;
	intVector*					m_pColEventIndexList;

	// Water Vertex
	std::vector<D3DXVECTOR3> m_WaterVertex;

	// Water 관련
	int				m_nWaterCount;
	float*			m_pWaterHeight;


	// Prop Zone 관련
	MapFilePropZoneManager* m_pZoneManager;


	SET_INFO		m_LightInfo;

	FILENAME_MOLD	m_SkyboxInfo;
	FILENAME_MOLD	m_CubeMapInfo;
	FILENAME_MOLD	m_AIFileInfo;
	FILENAME_MOLD	m_TextFileInfo;
	FILENAME_MOLD	m_CameraFileInfo;
	FILENAME_MOLD	m_LightMapFileInfo;
	FILENAME_MOLD	m_BillboardDDSInfo;

	// Layer Attribute Byte Map 관련
	int				m_nABXSize;
	int				m_nABYSize;
	BYTE*			m_pAttrByteMap;		// Tool 에서는 링크만, 게임에서는 데이터 로드시 생성.

	// Extended Attribute
	BYTE*			m_pTempAttrBuffer;

	int				m_nBillboardIndexGap;	// 스킨 Index -> BillBoard Index 차이값

	// 게임에서 쓰이는 데이터 부분
#ifndef _USAGE_TOOL_

	// 속성 맵.
	int		m_nAttrXSize;
	int		m_nAttrYSize;
	BYTE*	m_pAttrMap;
#endif

	BYTE*	m_pAttrMapEx;	// 확장 속성들.
	BOOL	m_bHasExAttrMap;

	char m_cMapFileName[MAX_PATH];
	SYSTEMTIME m_MapFileSysTime;

private:

};



#define PROP_ZONE_SIZE		2000
struct MAPFILEPROPZONEDATA
{
	bool bProp;
	std::vector<int> vecObjIndex;
	Box3 Box;
};


class MapFilePropZoneManager
{
public:
	MapFilePropZoneManager();
	~MapFilePropZoneManager();

	void Initialize( int nMapXSize, int nMapYSize );
	void AddProp( int nEngineIndex );
	void CaculateBoundingBox();
	int GetCount() { return m_nXSize * m_nYSize; }		
/*
	bool Save( CFile *fp );
	bool Load( BStream *fp );
*/

	int m_nZoneSize;
	int m_nXSize, m_nYSize;
	MAPFILEPROPZONEDATA* m_pZoneData;
};
