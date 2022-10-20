#pragma once

#include "3DDevice.h"
#include "CrossVector.h"
#include "FcGameObject.h"
#include "BsGenerateTexture.h"


struct UnitDataInfo;
class ASData;
class CAniInfoData;

enum{
	_TYPE_MENU_NONE_OBJ = -1,

	_TYPE_MENU_3DOBJ,
	_TYPE_MENU_FXOBJ,
};

struct CharStandInfo
{	
	float fcx, fcy, fcz;
	float fctx, fcty, fctz;

	int Angle;
	int aniID;
	float fScale;
};

#define _MENU_MAIN_CHAR_COUNT	7

class CFcMenuObject
{
public:
	CFcMenuObject();
	virtual ~CFcMenuObject();

	virtual void Release()	{}

	virtual void Process()	{}
	virtual void Update()	{}
/*
	virtual bool CreateObject(int nObjIndex,
		int nAniIndex,
		D3DXVECTOR3 vPos,
		float fScale,
		bool bFxLoop) { return false; }
*/
	int	 GetType()						{ return m_nType; }
	int  GetEngineIndex()				{ return m_nEngineIndex; }
	void SetHandle(DWORD hHandle)		{ m_hHandle = hHandle; }
	DWORD GetHandle()					{ return m_hHandle; }

	int GetObjIndex()					{ return m_nObjIndex; }
	CCrossVector* GetCrossVector()		{ return &m_Cross; }

	void SetShowOn(bool bShow)			{ m_bShowOn = bShow; }
	bool GetShowOn()					{ return m_bShowOn; }

protected:
	int				m_nType;
	DWORD			m_hHandle;
	int				m_nObjIndex;
	int				m_nEngineIndex;
	int				m_nSkinIndex;
	int				m_nLoadAniIndex;

	CCrossVector	m_Cross;

	bool			m_bShowOn;
};


class CFcMenu3DObject : public CFcMenuObject
{
public:
	CFcMenu3DObject();
	virtual ~CFcMenu3DObject();
	
	void Process();
	void Update();

	bool CreateObject(int nObjIndex,
		int nAniIndex,
		D3DXVECTOR3 vPos,
		float fScale,
		bool bFxLoop,
		int nWeaponID = 0,
		int nWeaponCount = 1);

	bool CreateObject(char* szSkinName,
		char* szAniName,
		char* szUnitInfoName,
		int nAniIndex,
		D3DXVECTOR3 vPos,
		float fScale,
		int nWeaponID = 0,
		int nWeaponCount = 1);

	void SetScale(float fScale)	{ D3DXMatrixScaling( &m_ScaleMat, fScale, fScale, fScale); }

	void SetAniIndex(int nIndex);
	void SetNextAniIndex(int nIndex);
	int	 GetAniIndex()			{ return m_nAniIndex; }
	int	 GetAniLength()			{ return m_nCurAniLength; }
	void ChangeWeapon(int nSkinIndex);
	
protected:
	void CreateParts(int nWeaponID, int nWeaponCount);
	void CreateCollisionMesh(const char *pFileName);

protected:
	UnitDataInfo*	m_pUnitSOX;
	CAniInfoData	*m_pUnitInfoData;
	
	D3DXMATRIX		m_ScaleMat;
	int				m_nRotate;

	int				m_nCurAniLength;
	float			m_fFrame;
	float			m_fPrevFrame;
	int				m_nAniIndex;
	int				m_nNextAniIndex;
	int				m_nWeaponCount;

	D3DXVECTOR3		m_Pos;

	std::vector< ARMOR_OBJECT >		m_ArmorList;
	std::vector< WEAPON_OBJECT >	m_WeaponList;
	int								m_nCurrentWeaponIndex;		// m_WeaponList 상의 인덱스이다..
	std::vector< PHYSICS_DATA_CONTAINER* > m_PhysicsInfoList;
	PHYSICS_COLLISION_CONTAINER*	m_pCollisionMesh;
	std::vector<int>				m_PartsSkinIndexList;
	std::vector<int> m_NoSimulateList;
};

class CFcMenuFxObject : public CFcMenuObject
{
public:
	CFcMenuFxObject();
	
	void Update();
	
	bool CreateObject(int nObjIndex,
		int nAniIndex,
		D3DXVECTOR3 vPos,
		float fScale,
		bool bFxLoop);
};


typedef std::vector<CFcMenuObject*>		FcMenuObjList;
class CFcMenu3DObjManager : public CBsGenerateTexture
{
public:
	CFcMenu3DObjManager();
	~CFcMenu3DObjManager();

	virtual void Create(int nTextureWidth, int nTextureHeight,
		float fStartX = 0.f, float fStartY = 0.f,
		float fWidth = 1.f, float fHeight = 1.f, bool bAlpha = false);
	void CreateLight();

	virtual void InitRenderRTT();
	virtual void Render(C3DDevice *pDevice);

	void Release();

	void Process();
			
	int	LoadFXTemplate(char* szFxFileName);

	DWORD AddObject(int nObjIndex,
					int nAniIndex,
					D3DXVECTOR3 vPos,
					float fScale,
					int nWeaponID,
					int nWeaponCount);
	DWORD AddObject(char* szSkinName,
					char* szAniName,
					char* szUnitInfoName,
					int nAniIndex,
					D3DXVECTOR3 vPos,
					float fScale,
					int nWeaponID,
					int nWeaponCount);
	bool  ChangeWeapon(DWORD hHandle, int nSkinID);

	DWORD AddFXObject(int nObjIndex, D3DXVECTOR3 vPos, float fScale, bool bLoop);

	bool RemoveObject(DWORD hHandle);
	void RemoveAllObject();
	
	void SetShowonObject(DWORD hHandle, bool bShow);
	bool GetShowonObject(DWORD hHandle);

	void SetObjectScale(DWORD hHandle, float fScale);

	CCrossVector* GetCameraCrossVector()			{ return &m_CameraCross; }
	CCrossVector* GetLightCrossVector()				{ return &m_LightCross; }

	CCrossVector* GetObjCrossVector(DWORD hHandle);
		
	D3DXVECTOR3	GetCameraTargetPos()					{ return m_vCamTargetPos; }
	void		SetCameraTargetPos(D3DXVECTOR3 pos)		{ m_vCamTargetPos = pos; }

	float		GetMenuCameraFov()						{ return m_fCamFov;		}
	void		SetMenuCameraFov(float fFov)			{ m_fCamFov = fFov;		}

	D3DXVECTOR3	GetLightTargetPos()						{ return m_vLightTargetPos; }
	void		SetLightTargetPos(D3DXVECTOR3 pos)		{ m_vLightTargetPos = pos; }

	void SetAniIndex(int nObjIndex, int nAniIndex);

	//char select에 ani가 없는 관계로 정상적인 next ani는 안된다.
	//next가 들어가면 ani가 멈춘다.
	void SetNextAniIndex(int hHandle, int nAniIndex);

	int GetObjectCount()								{ return (int)m_ObjList.size(); }

protected:

	void CreateCamera();

	CFcMenuObject* GetObject(DWORD hHandle);
	
protected:
	int				m_nLightIndex;

	D3DXMATRIX		m_ScaleMat;

	CCrossVector	m_CameraCross;
	CCrossVector	m_LightCross;
	FcMenuObjList	m_ObjList;
	
	D3DXVECTOR3		m_vCamTargetPos;
	float			m_fCamFov;
	D3DXVECTOR3		m_vLightTargetPos;

	std::vector<int>	m_FxTemplateList;	
};

extern CharStandInfo g_CharStandInfo[_MENU_MAIN_CHAR_COUNT];