#pragma once

#include "bstreamext.h"
#include "BsAniObject.h"
#include "BsFileIO.h"
#include "BsKernel.h"

#include "BsRealMovieFile.h"

#include "BsRealMovieMsg.h"

#include <MAP>
#include <STRING>

#include "BsRealMovieConst.h"

#include "BsRealMovieWave.h"
#include "BsRealMovieCam.h"
#include "BsRealMovieEffect.h"

#include "BsRealMovieAct.h"
#include "BsRealMovieLip.h"
#include "BsRealMovieFace.h"
#include "BsRealMovieBone.h"



#pragma pack( push )
#pragma pack( 4 )


class CRMObject;

struct stObjIDPack 
{
	int		nPartsSkinIndex;
	int		nPartsObjIndex;		// Obj Pool에 연결된 ID,
	char	m_szPartsName[eFileNameLength];
	char	m_szSubPath[eFileNameLength];
	int		bUse;	// 1 - Use, 0 - Hide
};




typedef std::map< std::string, CRMAct* >	mapStrAct;
typedef std::pair< std::string , CRMAct* >	pairStrAct;
typedef std::map< std::string, CRMAct* >::iterator	itStrAct;

typedef std::map< std::string, CRMLip* >	mapStrLip;
typedef std::pair< std::string , CRMLip* >	pairStrLip;
typedef std::map< std::string, CRMLip* >::iterator	itStrLip;

typedef std::map< std::string, CRMFace* >	mapStrFace;
typedef std::pair< std::string , CRMFace* >	pairStrFace;
typedef std::map< std::string, CRMFace* >::iterator	itStrFace;

typedef std::map< std::string, CRMBone* >	mapStrBone;
typedef std::pair< std::string , CRMBone* >	pairStrBone;
typedef std::map< std::string, CRMBone* >::iterator	itStrBone;

typedef std::vector<std::string>	vecString;

class CRMObject
{
public :

	enum 
	{ 
		ePartsMax = 8,
		eLinkedObjMax = 10,
		eObjectSize = 16,		// File Name 들, CrossVector 빼고
		eObjectOffset = eFileNameLength*3 + 48,
	};

	CRMObject();
	~CRMObject();

	void	RemoveObjWithId(void);
	void	RemoveSkinNObj(void);
	void	RemoveObject(void);
	void	UpdateObject(void);

	void	ClearNameBuffer(void);	// empty object 를 만들기 위함. 툴에서 사용.
	int		CreateNewObject(void);
	int		LoadParts(int nIdx);	// Set 도 같이 함.
	
	// LoadParts 에서 불림.
	void	RefreshAni(void);


	void	LinkParts(int nIdx);
	VOID*	CreatePartsContainer(char* szPhysicsTxtName);

	void	CreateCollisionContainer();
	void	ClearCollisionContainer();

	BOOL	SetAniObject(void);

	int		ReLoadObject(void);
	int		LoadPartObjects(void);
	int		LoadLipResource(void);

	int		PlayAniObject( float fTime );

	void	PlayAct( float fTime, int& rnBlend );
	void	PlayActEx( int nTick, int& rnBlend );

	void	RenderAct( void );
	void	PlayFace( int nTime, int& rnBlend );
	void	PlayLip( int nTime, int& rnBlend );

	void	ResetAniObject(void);
	void	ClearAniFlags(void)
	{
		m_nOldAniIndex = 0;
		m_fOldFrame = 0.f;

		// RefreshAni();
	}

	void	Render(void);

	void	SetName(char *szName)	{	strcpy( m_szName, szName );	}
	void	SetSkinFileName(char *szFile)	{	strcpy( m_szSKFileName, szFile );	}
	void	SetBaFileName(char *szFile)		{	strcpy( m_szBAFileName, szFile );	}
	//void	SetLinkedObjName(char *szName)	{	strcpy( m_szLinkedObjName, szName );	}

	void	SetSkinSubPath(char* szPath)
	{
		if( szPath == NULL || szPath[0] == NULL )
			memset( m_szSkinSubPath , 0, eFileNameLength);
		else
			strcpy( m_szSkinSubPath, szPath );
	}

	void	SetBAFileSubPath(char* szPath)
	{
		if( szPath == NULL || szPath[0] == NULL )
			memset( m_szBAFileSubPath , 0, eFileNameLength);
		else
			strcpy( m_szBAFileSubPath, szPath );
	}

	void	SetPhysicsSkinName(int nIdx,char *szFile)
	{	
		strcpy( m_szLinkedObjName[nIdx], szFile );
	}

	void	ClearPhysicsSkinName(int nIdx)
	{
		memset( m_szLinkedObjName[nIdx], 0, eFileNameLength );
	}

	void	SetBoneName(int nIdx,char *szName)
	{
		strcpy( m_szBoneName[nIdx], szName );
	}

	void	ClearBoneName(int nIdx )
	{
		memset( m_szBoneName[nIdx], 0, eFileNameLength );
	}

	void	SetPosition(float fx,float fy,float fz)
	{
		m_Cross.SetPosition( fx, fy, fz);
	}

	void	SetPartsSubPath(int nIdx,char* szPath)
	{
		if( szPath == NULL || szPath[0] == NULL )
			memset( m_ObjPack[nIdx].m_szSubPath , 0, eFileNameLength);
		else
			strcpy( m_ObjPack[nIdx].m_szSubPath, szPath );
	}

	int 	SetPartsSkinName(int nIdx,char *szName)
	{
		if( szName == NULL || szName[0] == NULL )
		{
			DeleteParts( nIdx );
			memset( m_ObjPack[nIdx].m_szPartsName , 0, eFileNameLength);
			return 0;
		}
		else
		{
			strcpy( m_ObjPack[nIdx].m_szPartsName, szName );
			return 1;
		}

		return 0;
	}

	void	SetUseParts(int nIdx,int nUse)
	{
		if( m_ObjPack[nIdx].nPartsObjIndex != -1 )
            m_ObjPack[nIdx].bUse = nUse;
		else
			m_ObjPack[nIdx].bUse = 0;
	}

	void	SetObjType(int nType)		{	SubData.m_nObjType = nType;			}


	char*	GetName(void)				{	return m_szName;			}
	char*	GetSkinFileName(void)			{	return m_szSKFileName;			}
	char*	GetBaFileName(void)				{	return m_szBAFileName;			}

	char*	GetSkinSubPath(void)			{	return m_szSkinSubPath;			}
	char*	GetBAFileSubPath(void)			{	return m_szBAFileSubPath;		}

	char*	GetLinkedObjName(int nIdx)	{	return m_szLinkedObjName[nIdx];		}
	char*	GetBoneName(int nIdx)		{	return m_szBoneName[nIdx];			}

	char*	GetPartsSkinName(int nIdx)	{	return m_ObjPack[nIdx].m_szPartsName;	}
	char*	GetPartsSubPath(int nIdx)	{	return m_ObjPack[nIdx].m_szSubPath;		}
	int		GetUseParts(int nIdx)		{	return m_ObjPack[nIdx].bUse;			}
	int		GetObjType(void)			{	return SubData.m_nObjType;				}


	int		GetAniCount(void)				{	return m_nAniCount;				}
	//	    해당 애니의 프레임 길이를 리턴 합니다 (60으로 나누면 초가 됩니다)
	int		GetAniLength(int nAniIndex);
	float	GetAniTime(int nAniIndex);	// 60으로 나뉜값을 줌.

	CCrossVector&	GetObjCross()			{	return m_Cross;					}
	D3DXVECTOR3		GetPosition()			{	return m_Cross.m_PosVector;		}
	D3DXVECTOR3*	GetPosPtr()				{	return &m_Cross.m_PosVector;		}
	void	SetPosition(D3DXVECTOR3& rPos)	{	m_Cross.m_PosVector = rPos;		}
	void	SetRot(int nRot);
	void	SetAbsoluteRot(int nRot);
	int		GetRotValue(void)				{	return SubData.m_nRot;			}


	int			GetActCount()				{	return (int)m_mapAct.size();	}
	int			GetLipCount()				{	return (int)m_mapLip.size();	}
	int			GetFaceCount()				{	return (int)m_mapFace.size();	}
	int			GetBoneCount()				{	return (int)m_mapBone.size();	}

	int			GetActNameList( vecString& rvecList );
	int			GetLipNameList( vecString& rvecList );
	int			GetFaceNameList( vecString& rvecList );
	int			GetBoneNameList( vecString& rvecList );
	int			GetOriginalBoneNameList( vecString& rvecList );

	mapStrAct&	GetActMap()					{	return m_mapAct;	}
	mapStrLip&	GetLipMap()					{	return m_mapLip;	}
	mapStrFace&	GetFaceMap()				{	return m_mapFace;	}
	mapStrBone&	GetBoneMap()				{	return m_mapBone;	}
	
	CRMAct*		InsertAct(char* szActName);
	CRMLip*		InsertLip(char* szLipName);
	CRMFace*	InsertFace(char* szFaceName);
	CRMBone*	InsertBone(char* szBoneName);

	CRMAct*		GetAct(char* szActName);
	CRMLip*		GetLip(char* szLipName);
	CRMFace*	GetFace(char* szFaceName);
	CRMBone*	GetBone(char* szBoneName);

	int			ChangeActName(char* szOldName,char* szNewName);
	int			ChangeLipName(char* szOldName,char* szNewName);
	int			ChangeFaceName(char* szOldName,char* szNewName);
	int			ChangeBoneName(char* szOldName,char* szNewName);

	bool		DeleteAct(char* szActName);
	bool		DeleteLip(char* szLipName);
	bool		DeleteFace(char* szFaceName);
	bool		DeleteBone(char* szBoneName);

	BOOL		DeleteParts( int nIdx );

	void		Save(BStream *pStream);
	void		Load(BStream *pStream);

	void		SaveEx(BStream *pStream);
	void		LoadEx(BStream *pStream);

	void		SaveEx2(BStream *pStream);
	void		LoadEx2(BStream *pStream);

	void		SaveBoneEx(BStream *pStream);
	void		LoadBoneEx(BStream *pStream);

	void		SetAlphaMode(int nMode)		{	m_nAlphaMode = nMode;	}
	int			GetAlphaMode(void)			{	return m_nAlphaMode;	}
	void		ToggleAlphaMode(void);
	void		ObjectAlphaMode(float fAlphaValue,DWORD dwOp,DWORD dwSrc,DWORD dwDest);
	void		DisableObjectAlphaMode(void);

	VOID		SetObjShowFlag(BOOL bSet )
	{
		if( m_nObjectIndex )
            m_bHideObj = !bSet;
		else
			m_bHideObj = TRUE;
	}

	BOOL		PtInfRect(D3DXVECTOR4& rfRc);

	CRMBone*	FindBone(char* szBoneName);

#ifdef _USAGE_TOOL_
	int	ChangeTimeOfAllData(void);
#endif

	BOOL	ChangeDataFolder(VOID);

	void	DropWeapon(void);
	// D3DXMATRIX* GetAniObjMatrix(void);
	D3DXVECTOR3*	GetAniObjPos(void);

	// 게임에서만 사용.
	VOID	SetSkinIndexForWeapon(int nSIdx);
	int		GetSkinIndexForWeapon();


	void	LinkObject(CRMObject *pLinkedObject,char* szBoneName);
	void	LinkFXObject(int nFXIndex,char* szBoneName);
	void	UnlinkFXObject(int nFXIndex);

	void AddLipTime( LARGE_INTEGER liAddTime );


protected :

	void	RemoveActList(void);
	void	RemoveLipList(void);
	void	RemoveFaceList(void);
	void	RemoveBoneList(void);

	CRMAct*		FindAct(char* szActName);
	CRMLip*		FindLip(char* szLipName);
	CRMFace*	FindFace(char* szFaceName);
	
	
	CRMAct*		FindActWithTime( float fTime );
	CBsAniObject*	GetAniObj(void)	{	return m_pAniObject;	}

	BOOL	CheckSamePartsContainer(PHYSICS_DATA_CONTAINER *pNewContainer);

	// Data
public :

	char	m_szName[eFileNameLength];
	char	m_szSKFileName[eFileNameLength];
	char	m_szBAFileName[eFileNameLength];

	char	m_szSkinSubPath[eFileNameLength];
	char	m_szBAFileSubPath[eFileNameLength];

	CCrossVector  m_Cross;		// [eObjectMax]; // D3DXVECTOR3 4개	

	struct 
	{
		int		m_nRot;
		int		m_nType;
		DWORD	m_dwParam[2];

		// 추가
		int		m_nObjType;		// 0-none, 1-Player1, 2-Player2, 3-Player1-WP, 4-Player2-WP
		// int		m_nWeaponType;	// weapon: 0-none, 1-Player1, 

		int		m_nDataType;	// 0-RM Data, 1-Game Char : 폴더 채크를 위함

	} SubData;

	// 추가됨.
	char	m_szLinkedObjName[eLinkedObjMax][eFileNameLength];	// 연결할 오브젝트명.
	char	m_szBoneName[eLinkedObjMax][eFileNameLength];
	// int		m_nUsePhysics



	// 추가
	stObjIDPack	m_ObjPack[ePartsMax];
	// char	m_szLinkedObjName[eFileNameLength];
	

		

	// 립싱크 애니가 존재할 경우에 0(무) 1..15(음소 43개에 매핑될 애니 15개) 16..19(단순 표정 4개) 20(더미)로 구성합니다
	// 마지막 애니 프레임은 사용하지 않게 되므로 더미가 반드시 필요 합니다 (위의 포맷으로 고정하는 편이 작업이 수월 할 것입니다)

	// 음소 매핑은 (*.ba) *.balm 를 사용합니다
	// default43.balm 을 사용 합니다

	int           m_nSkinIndex;
	int           m_nAniIndex;
	int           m_nObjectIndex; // CreateStaticObject() or CreateAniObject() 의 결과물 입니다
	CBsAniObject *m_pAniObject;	// 애니(0) 립싱크(1) = 보장되지 않는 참조이므로 실시간으로 만들어서 사용 해야 함에 주의 합니다
	int           m_nAniCount; // 립싱크 애니가 포함되어 있다면 2 이상의 값일 것입니다 (애니가 없는 경우에는 이 값이 0 입니다, 애니는 없지만 립싱크가 있는 경우에는 이값이 2 임에 주의 합니다)

	int			m_nHairSkinIndex;
	int			m_nHairObjIndex;

	// 하위 오브젝트들..
	std::map< std::string, CRMAct* >	m_mapAct;
	std::map< std::string, CRMLip* >	m_mapLip;
	std::map< std::string, CRMFace* >	m_mapFace;
	std::map< std::string, CRMBone* >	m_mapBone;

	// Temp
	std::vector< CRMFace* >	m_vecTempFacePool;

	// Physics
	std::vector< PHYSICS_DATA_CONTAINER* > m_PhysicsInfoList;
	PHYSICS_COLLISION_CONTAINER	*m_pCollisionInfo;

	int			m_nOldAniIndex;
	float		m_fOldFrame;
	int			m_nAlphaMode;
	BOOL		m_bHideObj;

	// Render 용
	int			m_nRenderAniIndex;
	float		m_fRenderFrame;

	//
	BOOL		m_bUseObjectAlphaMode;

	// Sound Position 용
	D3DXVECTOR3	m_vecSndPos;

	// Bone Index 
	int			m_nBoneIndex;
};




#pragma pack( pop )
