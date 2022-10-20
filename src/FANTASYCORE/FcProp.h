#pragma		once

#include "SmartPtr.h"
#include "Box3.h"
#include "CrossVector.h"
#include "BsPhysicsTrigger.h"


class CFcBreakableProp;
class CBSMapCore;
class NavigationMesh;
class NavigationCell;
class CBsObject;
class CCrossVector;
struct NAVEVENTINFO;
struct AABB;
struct PROPTYPEDATA;

#define GET_ROTATE(RXYZ)		((BYTE)(RXYZ>>24))
#define GET_XSCALE(XYZ)			((BYTE)(XYZ>>16))
#define GET_YSCALE(XYZ)			((BYTE)(XYZ>>8))
#define GET_ZSCALE(XYZ)			((BYTE)(XYZ))

struct FxString
{
	char _strBuffer[256];
	//void SetString( IN char* p_Str ) { strcpy( _strBuffer. p_Str); }
	FxString()
	{
		memset(this, 0, sizeof(FxString));
	}
};
typedef std::vector<FxString*> VectorFxStr;


class CFcBreakableObject;
typedef CSmartPtr< CFcBreakableObject > BreakableObjHandle;

class CFcDynamicObject;
typedef CSmartPtr< CFcDynamicObject > DynamicObjHandle;

class CFcGameObject;
typedef CSmartPtr<CFcGameObject> GameObjHandle;

#define PROP_ATTR_CRUSH					0x01
#define PROP_ATTR_COLLISION				0x02
#define PROP_ATTR_ALPHA_BLOCKING		0x04
#define PROP_ATTR_DYNAMIC				0x08
#define PROP_ATTR_CLIMB					0x10
#define PROP_ATTR_NO_PROCESS			0x20		// 속도 땜시 추가

#ifndef _DEFINE_PROP_TYPE
#define _DEFINE_PROP_TYPE
enum PROP_TYPE
{
	PROP_TYPE_NORMAL,
	PROP_TYPE_TOWER,		// 망루
	PROP_TYPE_BRIDGE,
	PROP_TYPE_GATE,
	PROP_TYPE_ENABLE_ATTACK_WALL,
	PROP_TYPE_TRAP,
	PROP_TYPE_BILLBOARD,
	PROP_TYPE_PICKUP,
	PROP_TYPE_CRUMBLE,		// 무너지면서 길을 막는 녀석
	PROP_TYPE_FALLDOWN,		// 무너지는 울타리
	PROP_TYPE_ANIMATION,
	PROP_TYPE_ITEM,
	PROP_TYPE_WALL,
};
#endif	//_DEFINE_PROP_TYPE


enum PROP_BREAK_TYPE
{
	PROP_BREAK_TYPE_NON,
	PROP_BREAK_TYPE_NORMAL,				// break by normal attack
	PROP_BREAK_TYPE_SPECIAL_ATTACK,		// break by special attack
	PROP_BREAK_TYPE_EVENT,				// break by event
	PROP_BREAK_TYPE_DAMGE_EXCEPT_SPECIAL_ATTACK,
};

enum PROP_BREAK_PARTS_TYPE
{
	PROP_BREAK_PARTS_TYPE_NON = -1,
	PROP_BREAK_PARTS_TYPE_LEFT_BASE,
	PROP_BREAK_PARTS_TYPE_ALL_DISAPPEAR,
	PROP_BREAK_PARTS_TYPE_LEFT_ALL
};

class PropHeightMap
{
public:
	PropHeightMap();
	~PropHeightMap();

	bool Initialize( int nXSize, int nYSize, D3DXVECTOR2 *pPos );
	void SetHeight( int nX, int nY, float fHeight );
	bool GetHeight( float fX, float fY, const D3DXMATRIX* pIMat, float* pHeight );

	bool Save( char* pFileName );
	bool Load( char* pFileName );

	

protected:
	D3DXVECTOR2 m_StartPos;
	int m_nXSize, m_nYSize;
	float* m_pMap;
};


class PropCollisionMeshInfo
{
public:
	PropCollisionMeshInfo();
	virtual ~PropCollisionMeshInfo();

	virtual void Initialize( int nMeshIndex, int nObjectIndex, float fMaxScaleSq );
	virtual bool GetHeight( float fX, float fY, float* pHeight, bool bCheckFace = true );
private:

	D3DXVECTOR3* m_pVertexBuf;
	int m_nVertexCount;
	WORD* m_pFaceBuf;
	int m_nFaceCount;
	D3DXMATRIX* m_Mat;
	float m_fBoundingDistSq;
	bool* m_pCheckFace;
};

struct CollisionTriangleInfo
{
	D3DXVECTOR3 p[3];
	float fMinX, fMaxX, fCenterX;
	float fMinZ, fMaxZ, fCenterZ;
};

class PropCollisionMeshInfoExt : public PropCollisionMeshInfo
{		
	int				m_nStartTriIndex;
	int				m_nTriCount;

public:

	PropCollisionMeshInfoExt();
	virtual ~PropCollisionMeshInfoExt();
	
	virtual void Initialize( int nMeshIndex, int nObjectIndex, float fMaxScaleSq );	
};

struct PROPTYPEDATA;

#define PROP_PARAM_MAX	8

struct NAMED_PROP_DATA
{
	char cName[64];
	DWORD dwParam[PROP_PARAM_MAX];
	char cParamStr[64];
};

//#define _SHOW_COLLISION_PROP
class CFcProp
{
public:
	CFcProp();
	virtual ~CFcProp();

	void Initialize( int nTypeDataIndex, PROPTYPEDATA* pData, char* pName, D3DXVECTOR3* pPos, DWORD dwRotAndScale, DWORD* pParam = NULL, char* pParamString = NULL );
	virtual void Process();
	virtual void UpdateObject();

	float GetX()	{ return m_vecPos.x; }
	float GetZ()	{ return m_vecPos.z; }
	float GetOldX() { return m_vecOldPos.x; }
	float GetOldZ() { return m_vecOldPos.z; }
	D3DXVECTOR3 GetPos()	{	return m_vecPos;	}
	D3DXVECTOR3 *GetPosPtr()	{	return &m_vecPos;   }

	D3DXVECTOR2 GetPosV2()	{	return D3DXVECTOR2( m_vecPos.x, m_vecPos.z ); }
	void SetPos(D3DXVECTOR3* Pos);
//	D3DXVECTOR3	GetCenterPos();		// 임시
	AABB* GetBoundingBox();
	void GetBox3( Box3 & B );
	void CalculateRadius();
	float GetRadius() { return m_fRadius; }
	const D3DXMATRIX& GetMat();
	virtual bool GetHeight( float fX, float fY, float* pHeight );
	int	 GetEngineIndex()	{	return m_nObjectIndex;	}	
	void Delete() { m_bDelete = true; }	
	bool IsDelete() {return m_bDelete;}

	PROP_TYPE GetType() { return m_Type; }

	void DebugCollisionPropRender();

	char* GetName()				{ if( m_pNamedData == NULL ) return NULL;	return m_pNamedData->cName; }
	int GetParam( int nIndex )	{ if( m_pNamedData == NULL ) return -1;	BsAssert( nIndex >= 0 && nIndex < 8 ); return m_pNamedData->dwParam[nIndex]; }
	char* GetParamStr()			{ if( m_pNamedData == NULL ) return NULL;	return m_pNamedData->cParamStr; };

	bool IsNoProcessProp()		{return !m_bProcess; }
	bool IsCrushProp()			{return ( ( m_Attr & PROP_ATTR_CRUSH ) != 0 );}
	bool IsDynamicProp()		{return ( ( m_Attr & PROP_ATTR_DYNAMIC ) != 0 );}
	bool IsBridgeProp()			{return ( m_Type == PROP_TYPE_BRIDGE );}
	bool IsGateProp()				{return ( m_Type == PROP_TYPE_GATE );}
	bool IsCollisionProp()			{return ( ( m_Attr & PROP_ATTR_COLLISION ) != 0 );}
	bool IsClimbProp()				{return ( ( m_Attr & PROP_ATTR_CLIMB ) != 0 );}
	bool IsAlphaBlocking()		{return ( ( m_Attr & PROP_ATTR_ALPHA_BLOCKING ) != 0 );}
	bool IsAttackProp()			{return ( m_Type == PROP_TYPE_ENABLE_ATTACK_WALL );}
	bool IsPickUpProp()			{return ( m_Type == PROP_TYPE_PICKUP );}
	bool IsBreakAndBlockWayProp() {return ( m_Type == PROP_TYPE_CRUMBLE );}
	bool IsItemProp()				{return ( m_Type == PROP_TYPE_ITEM );}
	bool IsWallProp()				{ return ( m_Type == PROP_TYPE_WALL );}

	virtual bool AddForce( D3DXVECTOR3 *pForce, D3DXVECTOR3 *pPos, int nBreakPower, PROP_BREAK_TYPE Type = PROP_BREAK_TYPE_NON) { return true; }
	virtual bool IsBreak() { return false; }
	virtual bool Break( int nDamage = -1, PROP_BREAK_TYPE nType = PROP_BREAK_TYPE_NORMAL ) { return false; }
	virtual int GetCrushParticleIndex() { return 0; }

	void SetMoveUpdate(bool bUpdate) { m_bMoveUpdate = bUpdate;}
	bool IsMoveUpdate() { return m_bMoveUpdate;}

	int GetBookingNum();
	D3DXVECTOR3* GetBookingPos( int nIndex );


	CCrossVector		m_Cross;

	int					m_nFxTemplate[2];
	int					GetFxTemplate( int p_nIndex)					{ return m_nFxTemplate[p_nIndex];			}
	void				SetFxTemplate( int p_nIndex, int p_nValue )		{ m_nFxTemplate[ p_nIndex ] = p_nValue;	}

	int					m_nFxIndex[2];
	int					GetFxIndex( int p_nIndex)						{ return m_nFxIndex[p_nIndex];			}
	void				SetFxIndex( int p_nIndex, int p_nValue )		{ m_nFxIndex[ p_nIndex ] = p_nValue;	}

	static void			InitCrossVec( D3DXVECTOR3* pPos, DWORD dwRotAndScale, CCrossVector* pCross );
	int					GetTypeDataIndex() { return m_nTypeDataIndex; }
	int					GetSoundMaterial() { return m_nPropSoundMaterial; }

protected:
	
	void SetInitData( int nObjectIndex, int nTypeDataIndex, int Type, int nAttr, char* pName, D3DXVECTOR3 vecPos, DWORD* pParam = NULL, char* pParamString = NULL );
	PropCollisionMeshInfo* GetCollsionMeshInfo()	{ return m_pCollisionMeshInfo; }
	virtual PropHeightMap* GetHeightMapInfo();

	int MakeAttr( PROPTYPEDATA* pData );

protected:
	bool		m_bProcess;
	int m_nObjectIndex;
	int m_nTypeDataIndex;
	PROP_TYPE		m_Type;
	int				m_Attr;

	NAMED_PROP_DATA* m_pNamedData;

	D3DXVECTOR3 m_vecPos;
	D3DXVECTOR3 m_vecOldPos;
	float	m_fRadius;
	bool	m_bMoveUpdate;
	bool	m_bDelete;
	int		m_nPropSoundMaterial;

	PropCollisionMeshInfo* m_pCollisionMeshInfo;
#ifdef _SHOW_COLLISION_PROP
	int		m_nCollisionIndex;
#endif
};

class CFcAnimationProp : public CFcProp
{
public:
	CFcAnimationProp();
	virtual ~CFcAnimationProp();

	void Initialize( int nTypeDataIndex, PROPTYPEDATA* pData, char* pName, D3DXVECTOR3* pPos, DWORD dwRotAndScale, DWORD* pParam = NULL, char* pParamString = NULL );
	virtual void Process();

protected:
	int m_nAniLength;
	float m_fCurFrame;
};

class CFcDynamicProp : public CFcProp
{
public:
	CFcDynamicProp();
	virtual ~CFcDynamicProp();

	void Initialize( int nTypeDataIndex, PROPTYPEDATA* pData, char* pName, D3DXVECTOR3* pPos, DWORD dwRotAndScale );	
	virtual void Process();
	virtual void UpdateObject();

	virtual bool AddForce( D3DXVECTOR3 *pForce, D3DXVECTOR3 *pPos, int nBreakPower, PROP_BREAK_TYPE Type = PROP_BREAK_TYPE_NON );
	void SetEnable(bool bEnable);

	void SetTeam( int nTeam )		{ m_nTeam = nTeam; }

protected:	
	IBsPhysicsActor	*m_pActor;
	int m_nTeam;
	int m_nSoundHandle;
	bool m_bMove;
	bool m_bEnable;
	int m_nAlphaTick;
	int m_nAddForceRemain;
};

struct PHYSICS_DATA_CONTAINER;
class IBsPhysicsActor;
class CFcBreakableProp : public CFcProp//, public CBsPhysicsTriggerBase
{
public:
	CFcBreakableProp();
	virtual ~CFcBreakableProp();

	void Initialize( int nTypeDataIndex, 
					PROPTYPEDATA* pData, 
					PROPTYPEDATA* pPhysicsPropData, 
					char* pName, D3DXVECTOR3* pPos, DWORD dwRotAndScale, DWORD* pParam = NULL, char* pParamString = NULL );	
	virtual void Process();
	virtual void UpdateObject();

	bool IsDestroyed()		{ return m_bDestroyed; }	
	virtual void Destroy();
	virtual bool AddForce( D3DXVECTOR3 *pForce, D3DXVECTOR3 *pPos, int nBreakPower, PROP_BREAK_TYPE Type = PROP_BREAK_TYPE_NON );
	bool IsBreak();
	virtual bool Break( int nDamage = -1, PROP_BREAK_TYPE nType = PROP_BREAK_TYPE_NORMAL );
	int GetCrushParticleIndex() { return m_nCrushParticleIndex; }

	int GetMaxHP()		{ return m_nMaxHP; }
	int GetHP()			{ return m_nHP; }
	void SetHP( int nHP )	{ m_nHP = nHP; }
	void SetMaxHP( int nMaxHP )	{ m_nMaxHP = nMaxHP; BsAssert( nMaxHP > 0 ); }
	void AddHP( int nHP );	

	void SetEnableCrush( bool bEnable ) { m_bCrushEnable = bEnable; }

	void HideCrushedPieces();

	void DestroyCheck();

protected:
//	BreakableObjHandle m_hObject;
	PHYSICS_DATA_CONTAINER	*m_pPhysicsData;
	
	int	m_nBreakSkinIndex;
	bool m_bDestroyed;
	bool m_bDeleteObject;		// 부서진 녀석 지우라는 플래그
	bool m_bAnimation;
	bool m_bFastDelete;
	int m_nDeleteAlphaFrame;
	int	m_nBreakableShadowType;
	int	m_nAdditionalLeftBaseIndex;	// 본단위로 알파 처리하는게 불가능해서 어쩔수 없이 오브젝트 하나더 생성해야된다.	

	int m_nAniLength;
	float m_fCurFrame;

	DWORD m_dwRotAndScale;

	int		m_nSleepFrame;

	D3DXVECTOR3 m_CrushVel;
	D3DXVECTOR3 m_ReturnVel;
		
	AABB		m_BoundingBox;

	int			m_nHP;
	int			m_nMaxHP;
	int			m_nCrushParticleIndex;


	PROP_BREAK_TYPE m_BreakType;	
	D3DXVECTOR3 m_BreakForce;

	PROP_BREAK_PARTS_TYPE m_BreakPartsType;

	int m_nAddForceRemain;
	PROP_BREAK_TYPE m_nLastAddForceType;

	bool m_bCrushEnable;	
};



enum GATE_STATE
{
	GATE_STATE_CLOSED,
	GATE_STATE_OPEN,
	GATE_STATE_OPENED,
	GATE_STATE_CLOSE,
	GATE_STATE_NUM
};

enum TRAP_STATE
{
	TRAP_STATE_HIDED,
	TRAP_STATE_ATTACK,
	TRAP_STATE_ATTACKED,
	TRAP_STATE_HIDE,
	TRAP_STATE_NUM
};



class CFcPropNavInfo
{
public:
	CFcPropNavInfo();
	virtual ~CFcPropNavInfo();

	void SetEventNav( int nIndex, NavigationMesh* pNavigationMesh, NAVEVENTINFO& vecNavEventInfo, bool bLink );

	void SetLink();
	void SetUnlink();

protected:
	std::vector<NavigationCell*> m_NavCells;
	bool m_bLink;
	int m_nNavEventIfnoIndex;
};


class CFcGateProp : public CFcProp
{
public:
	CFcGateProp();
	virtual ~CFcGateProp();

	void Initialize( int nTypeDataIndex, PROPTYPEDATA* pData, char* pName, D3DXVECTOR3* pPos, DWORD dwRotAndScale );	
	virtual void Process();
	virtual void UpdateObject();

	void SetEventNav( int nIndex, NavigationMesh* pNavigationMesh, NAVEVENTINFO& vecNavEventInfo, bool bLink );

	bool IsOpened();
	bool IsClosed();			
	bool CmdOpen();
	bool CmdClose();

	void PlayGateSound();

protected:
	GATE_STATE m_State;
	int m_nLength[GATE_STATE_NUM];
	int m_nCurAni;
	float m_fCurFrame;
	bool m_bSoundPlay;
	CFcPropNavInfo m_NavInfo;
	DWORD m_dwRotAndScale;
};


class CFcBridgeProp : public CFcBreakableProp
{
public:
	CFcBridgeProp();
	virtual ~CFcBridgeProp();

	void Initialize( int nTypeDataIndex, PROPTYPEDATA* pData, PROPTYPEDATA* pPhysicsPropData, char* pName, D3DXVECTOR3* pPos, DWORD dwRotAndScale );
	virtual void Process();

	void SetEventNav( int nIndex, NavigationMesh* pNavigationMesh, NAVEVENTINFO& vecNavEventInfo, bool bLink );
	virtual void Destroy();
	
protected:
	D3DXVECTOR3 m_Pos;
	CFcPropNavInfo m_NavInfo;
};



class CFcTrap : public CFcProp
{
public:
	CFcTrap();
	virtual ~CFcTrap();

	void Initialize( int nTypeDataIndex, PROPTYPEDATA* pData, char* pName, D3DXVECTOR3* pPos, 
					DWORD dwRotAndScale, DWORD* pParam = NULL );

	virtual void Process();
	virtual void UpdateObject();

protected:
	bool m_bUse;	
	TRAP_STATE m_State;
	int m_nLength[GATE_STATE_NUM];
	int m_nCurAni;
	float m_fCurFrame;
	int m_nTeam; // Param[0] 
	bool m_bRepeat; // Param[1] true - 무한반복
	int m_nOldTick; // 반복용
	int m_nStartDelayFrame;

	std::vector<GameObjHandle> m_vecObjs;
};


class CFcClimbProp : public CFcProp
{
public:
	CFcClimbProp();
	~CFcClimbProp();

	void Initialize( int nTypeDataIndex, PROPTYPEDATA* pData, char* pName, D3DXVECTOR3* pPos, DWORD dwRotAndScale, DWORD* pParam = NULL );
	virtual bool GetHeight( float fX, float fY, float* pHeight );

protected:
	D3DXMATRIX m_IMat;
};


class CFcBreakAndBlockWayProp : public CFcBreakableProp
{
public:
	CFcBreakAndBlockWayProp();
	~CFcBreakAndBlockWayProp();

	void Initialize( int nTypeDataIndex, PROPTYPEDATA* pData, PROPTYPEDATA* pPhysicsPropData, char* pName, D3DXVECTOR3* pPos, DWORD dwRotAndScale );
	virtual void Process();

	void SetEventNav( int nIndex, NavigationMesh* pNavigationMesh, NAVEVENTINFO& vecNavEventInfo, bool bLink );
	virtual void Destroy();

protected:
	int m_nDeleyTick;

protected:
	D3DXVECTOR3 m_Pos;
	CFcPropNavInfo m_NavInfo;
};


class CFcItemProp : public CFcProp
{
public:
	CFcItemProp();
	~CFcItemProp();

	void Initialize( int nTypeDataIndex, PROPTYPEDATA* pData, char* pName, D3DXVECTOR3* pPos, DWORD* pParam = NULL );
	virtual void Process();
	virtual void UpdateObject();
	bool Break( int nDamage = -1, PROP_BREAK_TYPE nType = PROP_BREAK_TYPE_NORMAL );
	bool AddForce( D3DXVECTOR3 *pForce, D3DXVECTOR3 *pPos, int nBreakPower, PROP_BREAK_TYPE Type = PROP_BREAK_TYPE_NON );

	bool IsBreak()	{ return m_bBreak; }
	void SetItemIndex(int nItem) { m_nItemIndex = nItem;	}

	void SetEnable();
	void SetDisable();

protected:
	int m_nItemIndex;		// +이면 아이템 인덱스 -이면 아이템 드랍테이블 인덱스
	bool m_bEnable;
	bool m_bBreak;
	int m_nDelayTick;
};
