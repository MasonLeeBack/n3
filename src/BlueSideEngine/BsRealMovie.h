#pragma once

#include "bstreamext.h"
#include "BsAniObject.h"
#include "BsFileIO.h"
#include "BsKernel.h"

#include "BsRealMovieFile.h"

#include "BsRealMoviePackage.h"
#include "BsRealMovieUtil.h"
#include "BsRealMovieMsg.h"

#include <MAP>
#include <STRING>
#include <SET>

//------------------------------------------------------------------------------------------------

class CRMAct;
class CRMLip;
class CRMFace;
class CRMObject;
class CRMWave;
class CRMCamera;
class CRMEffect;

#ifndef _XBOX
#define msg_func	static
#endif


class CBsRealMovie;
struct msgRMTNotify;

typedef float	(*PTR_FUNC_GETHEIGHT)(float fX, float fZ,short *pHeight);
typedef DWORD (*PTR_FUNC)(CBsRealMovie* pObj,DWORD dwParam1,DWORD dwParam2);
typedef VOID*	(*PTR_FUNC_PHYSICS_SET_FUNC)(char* szPhysicDataName);
typedef VOID	(*PTR_FUNC_PHYSICS_CLEAR_FUNC)(VOID* pPhysicsInfo);
typedef VOID*	(*PTR_FUNC_COLLISION_SET_FUNC)(char* szSkinFileName);
typedef VOID	(*PTR_FUNC_COLLISION_CLEAR_FUNC)(VOID* pCollisionInfo);

typedef std::pair<int,PTR_FUNC>	pairIntFunc;
typedef std::vector<std::string>	vecString;
typedef std::vector<CRMCamera*>		vecCamera;
typedef std::vector<stObjPack>	vecSelObjPack;
typedef std::vector<stBonePack>	vecBonePack;
typedef std::vector<stAreaSet>	vecAreaSet;

typedef std::vector<stLightPack>	vecLightSet;
typedef std::vector<stPointLightPack>	vecPointLightSet;
typedef std::vector<CRMPackage>		vecPackageSet;
typedef std::set<std::string>		setString;

#define RM_FILE_OLD_VER	100
#define	RM_FILE_VER		102

#define RMID_SETINFO		0x00000001
#define RMID_OBJECT			0x00000002

#define RMID_WAVE			0x00000004
#define RMID_CAMERA			0x00000008

#define RMID_EFFECT			0x00000010
#define RMID_OBJ_EX			0x00000020
#define RMID_OBJ_EX2		0x00000040
#define RMID_BONE_EX		0x00000080

#define RMID_EFFECT_EX		0x00000100
#define RMID_AREA			0x00000200
#define RMID_LIGHT			0x00000400
#define RMID_POINT_LIGHT	0x00000800

#define RMID_WAVE_EX		0x00001000
#define RMID_PACKAGE		0x00002000
#define RMID_CAMERA_EX		0x00004000


class CBsRealMovie : public CBsFileText // 파일 확장자 BSRTM
{
public:
	// enum { eFps60=60, }; // MAX 에서 60 프레임 기준 입니다

#ifndef _XBOX

	std::map<int,PTR_FUNC>	m_BRMCommander;
	typedef std::map<int,PTR_FUNC>	mapCmder;
	mapCmder&	GetCommander()	{	return m_BRMCommander;	}
	void	DrawCamLines(void);
	void	DrawCamPivot(void);
#endif

	//----------------------------------------
	// 파일에 대응되는 데이타 입니다
	//----------------------------------------

	struct SHeader {
		int  m_nOffset; // 이 길이만큼 헤더 파일을 읽습니다 (버전 관리 용도 입니다)	= sizeof(SHeader)
		int  m_nVersion; // 위치 주의
		int  m_nObjectMaxID; // 현재 유지되고 있는 오브젝트 ID

		void Set() {
			m_nOffset=sizeof(SHeader);
			m_nVersion=1;
			m_nObjectMaxID=0;
		}
	}
	m_Header;

	char m_szPathName[128]; // 작업 패쓰 입니다

	//-----------------------

	stMapInfo	m_BGInfo;		// 백그라운드 정보.(Map 기타 등등)

	// SKIN, BA 100 개 정도의 데이타를 매칭 합니다 (SKIN, BA 는 파일로 따로 존재 합니다)

	std::map< std::string , CRMObject* >	m_mapObject;
	typedef std::map< std::string , CRMObject* >::iterator	iteratorRMObject;
	typedef std::pair< std::string , CRMObject* >	pairStrObj;
	CRMObject*	m_pSelectedRMObject;

	// 사운드 파일을 관리 합니다 (WAV 파일 따로 존재합니다)
	std::map< std::string , CRMWave* >		m_mapWave;
	typedef std::map< std::string , CRMWave* >::iterator	iteratorRMWave;
	typedef std::pair< std::string , CRMWave* >	pairStrWav;

		  
	// 카메라를 관리 합니다
	std::map< std::string , CRMCamera* >	m_mapCamera;
	typedef std::map< std::string , CRMCamera* >::iterator	iteratorRMCamera;
	typedef std::pair< std::string , CRMCamera* >	pairStrCam;
	vecCamera	m_vecCamera;		// mapCamera에 넣을 때 같이 넣는다. 소트용

	CRMCamera*	m_pDefCam;
	CRMCamera*	m_pStaticCam;
	CRMCamera*	m_pSelectedCam;

	// 이펙트 파티클 데이타를 수록 합니다 
	std::map< std::string , CRMEffect* >	m_mapEffect;
	typedef std::map< std::string , CRMEffect* >::iterator	iteratorRMEffect;
	typedef std::pair< std::string , CRMEffect* >	pairStrEff;
	CRMEffect* m_pSelectedRMEffect;

	std::map< DWORD , std::vector< stEvent > >		m_mapEvent;
	typedef std::map< DWORD , std::vector< stEvent > >	mapDwordVector;

#ifdef _USAGE_TOOL_
	CBsErrorMsgManager*	m_pErrMsgManager;
	VOID	PushErrorMsg(char* szMsg)
	{
		if( m_pErrMsgManager )
		{
			m_pErrMsgManager->AddErrMsg( szMsg );
		}
	}

	VOID	CheckingCurrentRealMovie(vecString& rResult,setString* psetTroop=NULL);
	VOID	CheckingCurrentRealMovieEx(vecString& rResult,setString* psetTroop=NULL);

#endif



	// 실기 무비는 사용후 바로 메모리 해제 되므로 큰 메모리 블럭들로 구성 합니다
	//
	// 1. 물리 엔진 링크 (로보덱스)	- 망토, 사슬
	// 2. 지형
	// 3. 캐릭터, 옷, 장신구 교체
	// 4. 1P, 2P, 칼모양
	// 5. 이펙트 (비, 운석 같은 천재 지변 포함)

	//----------------------------------------
	// 사용시 만들어 지는 데이타 입니다
	//----------------------------------------

	// static CBsFileBALM st_BALM; // 립싱크용 매칭 정보 파일 (공용 입니다)

	int m_nBlend; // 블렌드 횟수를 저장하고 있는 임시 변수 입니다
	int m_bFadeOut; // 페이드 아웃 된 상태를 유지되도록 합니다

	int  m_nVersion;


	int			m_nEventCnt;
	stEvent		m_EventArray[EVENT_COL_MAX];

	int	m_nbUseRMCam;		// 실기 무비용 카메라 사용 유무. 기본값 1.
	int			m_nTerminateFlag;
	float		m_fTerminateTime;
	int			m_dwTerminateTick;

	PTR_FUNC_PHYSICS_SET_FUNC	m_pfnCreatePhysics;	// Physics 스킨 로드용 함수 포인터.
	PTR_FUNC_PHYSICS_CLEAR_FUNC	m_pfnClearPhysics;
	PTR_FUNC_COLLISION_SET_FUNC	m_pfnCreateCollision;
	PTR_FUNC_COLLISION_CLEAR_FUNC		m_pfnClearCollision;

	vecBonePack		m_vecBonePackPool;

	// Player Weapon Skin Index
	int			m_nPlayerWeaponSkinIndex;

	VOID SetBonePackPool(int nObjIndex,DWORD dwTextPtr,D3DXVECTOR3& rVec)
	{
		if( !m_vecBonePackPool.size() )
		{
			vecBonePack::iterator it = m_vecBonePackPool.begin();
			vecBonePack::iterator itEnd = m_vecBonePackPool.end();

			for( ; it != itEnd ; ++it )
			{
				if( it->nObjIndex == nObjIndex && it->dwBoneNamePtr == dwTextPtr )
				{
					it->vecRot = rVec;
					//return (*it);
				}
			}
		}

		stBonePack newPack;
		newPack.nObjIndex = nObjIndex;
		newPack.dwBoneNamePtr = dwTextPtr;
		m_vecBonePackPool.push_back( newPack );
		( m_vecBonePackPool.end()-1 )->vecRot = rVec;
		//return (*(m_vecBonePackPool.end()-1));
	}


	// Area 관련
	vecAreaSet		m_vecAreaSetPool;

	void	ClearAreaPool(void);
	void	ResizeAreaPool(int nNewSize);
	int		GetAreaPoolSize(void);
	stAreaSet*	GetAreaSetFromPool(int nIndex);
	stAreaSet*	FindArea(char* szAreaName);

	// Light 관련.
	vecLightSet		m_vecLightSetPool;

	void	ClearLightPool(void);
	void	ResizeLightPool(int nNewSize);
	int		GetLightPoolSize(void);
	stLightPack*	GetLightSetFromPool(int nIndex);
	stLightPack*	FindLightSet(char* szLightName);

	// Point Light 관련
	vecPointLightSet	m_vecPointLightSetPool;

	stPointLightPack*	CreateEmptyPointLight(void);
	BOOL	CreateNewPointLight(char* pszLightName, const size_t pszLightName_len); //aleksger - safe string

	stPointLightPack*	FindPointLight(char* szLightName);
	BOOL	DeletePointLight(char* szLightName);

	void	ClearAllPointLights(void)	{	m_vecPointLightSetPool.clear();	}
	int		GetPointLightCount(void)	{	return (int)m_vecPointLightSetPool.size();	}
	char*	GetPointLightName(int nIdx)	
	{	
		return m_vecLightSetPool[nIdx].NameBlock.szLightName;	
	}

	// Package 관련.
	vecPackageSet		m_vecPackagePool;

	void	ClearAllPackages(void)	{	m_vecPackagePool.clear();	}
	int		GetPackageCount(void)	{	return (int)m_vecPackagePool.size();	}

	CRMPackage*	FindPackage(char* szPackName);
	CRMPackage*	CreateEmptyPackage(void);
	BOOL		CreateNewPackage(char* szPackName, const size_t szPackName_len); //aleksger - safe string

	BOOL	DeletePackage(char* szPackName);
	const char*	GetPackageName(int nIdx)
	{
		return m_vecPackagePool[nIdx].GetName();
	}

	void	ConformAllItmesTime(void);
	void	ConformItmesTimeToPackTime(char* szPackName);
	void	ConformItmesTimeToPackTimeCore(CRMPackage* pPack);
	void	ConformItmesTimeToPackTimeCore(CRMPackage* pPack,int nIndex);
	void	ResetItemsTime(CRMPackage* pPack,int nIndex);


	stMapInfo&	GetBGInfo(void)		{	return m_BGInfo;	}

	static bool m_bPause;
	static LARGE_INTEGER m_liSaveTime;

public:
	CBsRealMovie(); // 비어 있는 파일인 경우의 내용이 될것입니다


	int	GetCamMapCount(void)					{	return (int)m_mapCamera.size();		}
	CRMCamera*	GetFirstCam(void)			{	return m_mapCamera.begin()->second;			}
	CRMCamera*	GetDefCamera(void)			{	return m_pDefCam;					}


	int GetCamCount(void)					{ return (int)m_vecCamera.size(); }
	CRMCamera*	GetCamera( int nIndex )		{ return m_vecCamera[nIndex]; }

	int	GetObjectCount(void)				{	return (int)m_mapObject.size();		}
	int	GetWaveCount(void)					{	return (int)m_mapWave.size();		}
	int	GetEffectCount(void)				{	return (int)m_mapEffect.size();		}

	void UpdateFcCamera();
	



#ifndef _XBOX

#ifdef _USAGE_TOOL_
	int	ChangeTimeOfAllData(void);
#endif


	CRMObject*	CheckHitObject(D3DXVECTOR3 &vPickRayDir,D3DXVECTOR3 &vPickRayOrig);
	CRMObject*	CheckHitObjectEx(int cx,int cy,int posx,int posy,D3DXMATRIXA16* pmatViewProj);
	CRMCamera*	CheckHitCameraEx(int cx,int cy,int posx,int posy,
		D3DXMATRIXA16* pmatViewProj,D3DXVECTOR3& rPos,int& rnPathIndex);
	CRMEffect*	CheckHitEffectEx(int cx,int cy,int posx,int posy,D3DXMATRIXA16* pmatViewProj);


	void	OnInitCommander(void);
#endif
	void	RemoveAllBuffer(void);

	void SaveOld     (BStream *pStream); 

	void Save(BStream *pStream);
	void SaveObjects(BStream *pStream);
	void SaveObjectsEx(BStream *pStream);
	void SaveObjectsEx2(BStream *pStream);
	void SaveCameras(BStream *pStream);
	void SaveCamerasEx(BStream *pStream);
	void SaveWaves(BStream *pStream);
	void SaveWavesEx(BStream *pStream);
	void SaveEffects(BStream *pStream);
	void SaveBoneEx(BStream *pStream);
	void SaveEffectsEx(BStream *pStream);
	void SaveArea(BStream *pStream);
	void SaveLight(BStream *pStream);
	void SavePointLight(BStream *pStream);
	void SavePackage(BStream* pStream);
		

	void Load(BStream *pStream);
	void LoadObjects(BStream *pStream);
	void LoadObjectsEx(BStream *pStream);
	void LoadObjectsEx2(BStream *pStream);
	void LoadCameras(BStream *pStream,int nVer);
	void LoadCamerasEx(BStream *pStream,int nVer);
	void LoadWaves(BStream *pStream);
	void LoadWavesEx(BStream *pStream);
	void LoadEffects(BStream *pStream);
	void LoadBoneEx(BStream *pStream);
	void LoadEffectsEx(BStream *pStream);
	void LoadArea(BStream* pStream);
	void LoadLight(BStream* pStream);
	void LoadPointLight(BStream *pStream);
	void LoadPackage(BStream* pStream);

	void	LinkObjToEffects(void);
	void	LinkEffectToPackage(void);
	void	CheckObjFolder(void);
	void	CheckCamFolder(void);
	void	CheckWavFolder(void);
	void	LinkEffectToWaveRes(void);
	void	ClearEffectOwnTable(void);


	void LoadOld     (BStream *pStream);
	void Load_text(BStream *pStream) { } // 더미 (상속 관계 때문에 존재 합니다)

	// void Load_t(int md12); //?? 테스트가 끝나면 삭제 예정 입니다

    /*

		사용 패턴 입니다 <1>
		|
		CBsRealMovie *pMovie = new CBsRealMovie(); // 큰 메모리 블럭 1 개가 생성된다고 볼수 있습니다

		{ 
			pMovie->Load(szFileName); // 클리어 할 필요가 없는 메모리에 데이타를 로드 합니다
			pMovie->AllocateResource(); // -Clear() 호출함- , 데이타를 만듭니다
			
			pMovie->Play(fTime, pMatrix); // 플레이 합니다
		}
		|
		반복 가능 합니다

		delete pMovie; // -Clear() 호출함- . pMovie 가 확보하고 있는 아마도 sizeof(CBsRealMovie) 만큼의 데이타도 프리 시킵니다. 프리 시점 때문에 커널과 충돌 가능성이 있습니다

		pMovie->Clear(); // 필요할때 메모리에서 삭제 합니다 = AllocateResource() 함수에서는 자동 호출 합니다

		----------------------------

		사용 패턴 입니다 <2>
		|
		커널에 있는 다음 내용만 사용합니다

		void *m_pRealMovie;	// CBsRealMovie * 입니다 (변수 선언 내용 입니다)

		void RealMovieLoad(char *szFileName); // 로드 합니다 (메모리 할당)
		void RealMoviePlay(float fTime); // 플레이 합니다
		void RealMovieClear(); // 사용후 반드시 클리어 합니다 (메모리 소거) m_pRealMovie=NULL (프로그램 종료시에는 자동 클리어 됩니다)

	*/

	void SetDefaultLighting();

	void ChdirStart();
	void ChdirEnd();
	void Chdir(char* szPath);


	//int  Is_nObjID(int nObjID); // 여기서만 패러미터가 nObjID 임에 주의 합니다
	CRMObject*	GetObject(char* szObjName);
	CRMEffect*	GetEffect(char* szEffName);
	CRMWave*	GetWave(char* szWavName);
	CRMCamera*	GetCamera(char* szCamName);
	CRMLip*		GetLip(char* szFullName);

	CRMCamera* GetPlayCam()	{ return m_pStaticCam; }
	void ClearPlayCam()	{ m_pStaticCam = NULL; }
	void	ClearObjFlags(void);
	void	ResetAllObjectAni(void);
    
	int  GetFileSize(char *szFileName);

	void AllocateResource(); // Load() 이후의 데이타를 바탕으로, 해당 리소스를 모두 할당 합니다


	int		PreProcess(float fTime,DWORD dwTick,msgRMTNotify* pRMTNotify);
	void	Process(float fTime,int nTick);
	int		PostProcess(float fTime);

	void	Update(float fTime);
	void	ProcessBone();

	void	ResetAni(void);

	// void Play     (float fTime, D3DXMATRIX *pMatrix, CCrossVector *pCameraCross, CCrossVector *pLightCross); // 해당 시간의 무비를 드로우 합니다

#ifndef _XBOX
	void	SortCameraWithTime(void);
#endif
	CRMCamera*	GetCameraByTime(float fTime);

	// CRMEffect*	ProcessEffect(CRMEffect* pEffect,float fTime);
	CRMEffect*	ProcessEffect(CRMEffect* pEffect,int nFrame);

	void	ProcessSelectedEffect(CRMEffect* pSelectEffect,int nFrame);
	void	ProcessPackage(CRMPackage* pPack,float fTime,DWORD dwTick);

	void PlayEff_ ( int nFrame ); // 이펙트 루틴 
	void PlayEff_ForProcess( int nFrame );
	void PlayWave_( int nFrame ); // 웨이브 루틴 
	//                   |
	//                   시간 (3.2f 는 3.2 초를 뜻합니다)

	void Clear();
   virtual ~CBsRealMovie();

    void	DeleteObject(char* szObjName);	// 따로 호출 하지 않습니다
	int		DeleteWave(char* szWavName);
	int		DeleteCamera(char* szCamName);
	int		DeleteCameraPath(char* szCamName,int nIndex);
	int		DeleteEffect(char* szEffName);

	void  DrawUITextDebug_LS(int nLTF_id43, int nBALM_id16, int y); // 립싱크용 디버깅 정보 입니다

	//---------------------------

	//    m_nObjectIndex[] = 오브젝트 인덱스
	//    m_nAniIndex[]    = 애니메이션 인덱스 (립싱크인 경우에는 0 으로 고정 - 주의) = 툴에서는 index+1 로 보여줍니다 (매트리얼 툴과의 매치 문제로)
	//    m_fTime[]        = 타임
	//    |
	CRMObject*	InsertObject(char* szName); // nObject01 = nTreeIndex 
	CRMObject*	InsertEmptyObject(char* szName);	// Load 용.
	int		InsertAct(char* szObjName,char* szActName);
	int		InsertLip(char* szObjName,char* szLipName);
	int		InsertFace(char* szObjName,char* szFaceName);
	int		InsertBone(char* szObjName,char* szBoneName);
	
	CRMEffect*	InsertEff   (char* szEffName); 
	CRMWave*	InsertWave  (char* szWaveName); // nObject01 을 지원하지 않을때는 -1 을 사용 합니다 (Wave 에만 해당)
	CRMCamera*	InsertCamera(char* szCamName); // 카메라는 1 개 입니다 

	int		ChangeObjName(char* szOldName,char *szNewName);
	int		ChangeActName(char* szObjName,char* szOldName,char *szNewName);
	int		ChangeLipName(char* szObjName,char* szOldName,char *szNewName);
	int		ChangeFaceName(char* szObjName,char* szOldName,char *szNewName);
	int		ChangeBoneName(char* szObjName,char* szOldName,char *szNewName);

	int		ChangeWaveName(char* szOldName,char* szNewName);
	int		ChangeCameraName(char* szOldName,char* szNewName);
	int		ChangeEffectName(char* szOldName,char* szNewName);

	int		GetObjList( vecString& rvecList );
	int		GetActList( vecString& rvecList );
	int		GetLipList( vecString& rvecList );
	int		GetFaceList( vecString& rvecList );
	int		GetBoneList( vecString& rvecList );
	int		GetWaveList( vecString& rvecList );
	int		GetCamList( vecString& rvecList );
	int		GetEffectList( vecString& rvecList );

	VOID*	GetNodePtrByName( int nType,char *szCaption);


	//                                                        측정 하고 싶은 시간 입니다 = 리턴 값은 nLipIndex 에 해당하는, 실기 입력값 시간 입니다 
	//                                                        |
	float GetEvent    (int &nLipIndex, int &nObject01, float fCurrentTime); // 립싱크 애니가 구동된 시점에 통보 합니다 (1번만 통보 됩니다)
	//                       |
	//                       해당 하는 것이 없을때는 nLipIndex=-1 입니다 (m_Lip 의 인덱스)

	void  PlayedEvent (int  nLipIndex); // 해당 nLipIndex 가 처리 되었다면 이 함수를 호출해 주세여 (이후에 이 이벤트는 다시 호출 되지 않습니다)
	void  ResetEvent  (float fCurrentTime); // 해당 시간 이후의 이벤트(evt>=fCurrentTime)만 리셋됩니다 (주의)

	//
	int   GetEffEvent (float fCurrentTime);
	void	SetTerminateTime( float fTime );
	int		GetLastEvent( msgRMTNotify* pRMTNotify );
	//

	float GetWaveEvent    (int &nWaveIndex, int &nObject01, float fCurrentTime);
	void  PlayedWaveEvent (int  nWaveIndex); // 해당 nWaveIndex 가 처리 되었다면 이 함수를 호출해 주세여 (이후에 이 이벤트는 다시 호출 되지 않습니다)
	void  ResetWaveEvent  (float fCurrentTime); // 해당 시간 이후의 이벤트(evt>=fCurrentTime)만 리셋됩니다 (주의)

	//---------------------------

	char*   GetSKFilenameBuffer(char* szObjName); // nObject01 = nIndex  (SKIN)
	char*   GetBAFilenameBuffer(char* szObjName); // (BA)

	char*   GetLipFilenameBuffer(char* szObjName,char* szLipName); // filename (LTF)
	char*   GetLipWaveNameBuffer(char* szObjName,char* szLipName); // wavename (WAVE)

	void    SetCurPath(LPCTSTR szCurPath, LPCTSTR szFilename); // 데이타에 디렉토리를 기록 합니다

	DWORD	RealMovieCommand(int nMsg,DWORD dwParam1=0,DWORD dwParam2=0);

	// [in] 시간, [out] 갯수
	int		EventNotify( float fTime , DWORD dwTick );

	VOID*	GetSelectedEvent(int nIndex,int& rType)	
	{	
		rType = m_EventArray[nIndex].nType;
		return m_EventArray[nIndex].pEventObj;
	}

	VOID	ResetFlags(VOID);		// 이벤트 초기화, 카메라 초기화등을 한다.

	void	MakeEventMap(vecEffWavList* pEventPool);

	void	ApplyCamDurationToEffect(CRMCamera* pRMCam);

	CRMEffect*	GetCamEffectOnTick(DWORD dwTick);

	VOID	ToggleShowBoundingBox(BOOL bShow);

	CRMObject*	FindObjectByOBJIndex(int nObjIndex);
	CRMObject*	FindObjectByOBJName(char* szName);

	VOID	ApplyObjectCullMove(bool bSet);
	int		GetSelectedAllObj(D3DXVECTOR4& rfRect,vecSelObjPack& rvecRetPool,short* pHeightBuf,
		PTR_FUNC_GETHEIGHT pfnGetHeight);
	int		ResetSelectedAllObjYPos(vecSelObjPack& rvecRetPool,short* pHeightBuf,
		PTR_FUNC_GETHEIGHT pfnGetHeight);
#ifdef _USAGE_TOOL_
	int		ResetSelectedObjectYPos(vecSelObjPack& rvecRetPool,short* pHeightBuf,
		PTR_FUNC_GETHEIGHT pfnGetHeight);

#endif

	VOID	ResetCamsPivotLine(VOID);

	int 	ClearAllObjSelectedMark(vecSelObjPack& rvecRetPool);
	int		MoveObjects( vecSelObjPack& rvecRetPool, float fMovX, float fMovZ, short* pHeightBuf,
		PTR_FUNC_GETHEIGHT pfnGetHeight);


	CRMObject*	GetTempObj(int nIndex)	{	return m_ppObjTempBuf[nIndex];	}
	int			GetTempObjBufSize(void)	{	return m_nTempBufSize;			}

	VOID	AllocateSinglyTempResource(int nIndex);
	VOID	SyncResources(VOID);
	VOID	ReleaseTempBuffer(VOID);

	void AddLipTime( LARGE_INTEGER liAddTime );
	void SetPause( bool bPause, LARGE_INTEGER SaveTime );
	static LARGE_INTEGER GetLipTime();

#ifdef _USAGE_TOOL_
	VOID	SetDummyLoadFlag(BOOL bSet)		{	m_bIsDummyLoad = bSet;	}
	BOOL	GetDummyLoadFlag()				{	return m_bIsDummyLoad;	}

	int		FindEndOfMovieTime(VOID);
#endif

protected :

	CRMObject**	m_ppObjTempBuf;
	int			m_nTempBufSize;

#ifdef _USAGE_TOOL_
	BOOL		m_bIsDummyLoad;
#endif


	// 컴파일과 등록시에 채크됨.
	// fTime : 이벤트 통지 시간, 
	void	InsertEvent( DWORD dwTick, int nType, void* pEventObj );
	bool	DeleteEvent( DWORD dwTick, int nType );

	void	ReleaseEventMap(void);


	DWORD	RealMovieDefCommand(int nMsg,DWORD dwParam1=0,DWORD dwParam2=0);
	DWORD	RealMovieDelCommand(int nMsg,DWORD dwParam1=0,DWORD dwParam2=0);


#ifndef _XBOX

	msg_func	DWORD OnEmptyFunc(CBsRealMovie* pObj,DWORD dwParam1,DWORD dwParam2);

	msg_func	DWORD OnInsertNewObj(CBsRealMovie* pObj,DWORD dwParam1,DWORD dwParam2);
	msg_func	DWORD OnInsertNewAct(CBsRealMovie* pObj,DWORD dwParam1,DWORD dwParam2);
	msg_func	DWORD OnInsertNewLip(CBsRealMovie* pObj,DWORD dwParam1,DWORD dwParam2);
	msg_func	DWORD OnInsertNewFace(CBsRealMovie* pObj,DWORD dwParam1,DWORD dwParam2);
	msg_func	DWORD OnInsertNewBone(CBsRealMovie* pObj,DWORD dwParam1,DWORD dwParam2);

	msg_func	DWORD OnInsertNewWave(CBsRealMovie* pObj,DWORD dwParam1,DWORD dwParam2);
	msg_func	DWORD OnInsertNewCam(CBsRealMovie* pObj,DWORD dwParam1,DWORD dwParam2);
	msg_func	DWORD OnInsertNewEffect(CBsRealMovie* pObj,DWORD dwParam1,DWORD dwParam2);
	msg_func	DWORD OnInsertNewParticle(CBsRealMovie* pObj,DWORD dwParam1,DWORD dwParam2);


	msg_func	DWORD OnGetNodePtr(CBsRealMovie* pObj,DWORD dwParam1,DWORD dwParam2);
	msg_func	DWORD OnSetTimeByNodePtr(CBsRealMovie* pObj,DWORD dwParam1,DWORD dwParam2);
	msg_func	DWORD OnGetNodeNameByNodePtr(CBsRealMovie* pObj,DWORD dwParam1,DWORD dwParam2);

	msg_func	DWORD OnGetObjectMember(CBsRealMovie* pObj,DWORD dwParam1,DWORD dwParam2);
	msg_func	DWORD OnGetActMember(CBsRealMovie* pObj,DWORD dwParam1,DWORD dwParam2);
	msg_func	DWORD OnGetLipMember(CBsRealMovie* pObj,DWORD dwParam1,DWORD dwParam2);
	msg_func	DWORD OnGetFaceMember(CBsRealMovie* pObj,DWORD dwParam1,DWORD dwParam2);
	msg_func	DWORD OnGetBoneMember(CBsRealMovie* pObj,DWORD dwParam1,DWORD dwParam2);

	msg_func	DWORD OnGetWaveMember(CBsRealMovie* pObj,DWORD dwParam1,DWORD dwParam2);
	msg_func	DWORD OnGetCameraMember(CBsRealMovie* pObj,DWORD dwParam1,DWORD dwParam2);
	msg_func	DWORD OnGetEffectMember(CBsRealMovie* pObj,DWORD dwParam1,DWORD dwParam2);

	msg_func	DWORD OnGetObjPtr(CBsRealMovie* pObj,DWORD dwParam1,DWORD dwParam2);
	msg_func	DWORD OnGetActPtr(CBsRealMovie* pObj,DWORD dwParam1,DWORD dwParam2);
	msg_func	DWORD OnGetLipPtr(CBsRealMovie* pObj,DWORD dwParam1,DWORD dwParam2);
	msg_func	DWORD OnGetFacePtr(CBsRealMovie* pObj,DWORD dwParam1,DWORD dwParam2);
	msg_func	DWORD OnGetBonePtr(CBsRealMovie* pObj,DWORD dwParam1,DWORD dwParam2);
	msg_func	DWORD OnGetWavePtr(CBsRealMovie* pObj,DWORD dwParam1,DWORD dwParam2);
	msg_func	DWORD OnGetCameraPtr(CBsRealMovie* pObj,DWORD dwParam1,DWORD dwParam2);
	msg_func	DWORD OnGetEffectPtr(CBsRealMovie* pObj,DWORD dwParam1,DWORD dwParam2);


	msg_func	DWORD OnChangeObjMember(CBsRealMovie* pObj,DWORD dwParam1,DWORD dwParam2);
	msg_func	DWORD OnChangeActMember(CBsRealMovie* pObj,DWORD dwParam1,DWORD dwParam2);
	msg_func	DWORD OnChangeLipMember(CBsRealMovie* pObj,DWORD dwParam1,DWORD dwParam2);
	msg_func	DWORD OnChangeFaceMember(CBsRealMovie* pObj,DWORD dwParam1,DWORD dwParam2);
	msg_func	DWORD OnChangeBoneMember(CBsRealMovie* pObj,DWORD dwParam1,DWORD dwParam2);
	msg_func	DWORD OnChangeWaveMember(CBsRealMovie* pObj,DWORD dwParam1,DWORD dwParam2);
	msg_func	DWORD OnChangeCameraMember(CBsRealMovie* pObj,DWORD dwParam1,DWORD dwParam2);
	msg_func	DWORD OnChangeEffectMember(CBsRealMovie* pObj,DWORD dwParam1,DWORD dwParam2);

	msg_func	DWORD OnDeleteObj(CBsRealMovie* pObj,DWORD dwParam1,DWORD dwParam2);
	msg_func	DWORD OnDeleteAct(CBsRealMovie* pObj,DWORD dwParam1,DWORD dwParam2);
	msg_func	DWORD OnDeleteLip(CBsRealMovie* pObj,DWORD dwParam1,DWORD dwParam2);
	msg_func	DWORD OnDeleteFace(CBsRealMovie* pObj,DWORD dwParam1,DWORD dwParam2);
	msg_func	DWORD OnDeleteBone(CBsRealMovie* pObj,DWORD dwParam1,DWORD dwParam2);
	msg_func	DWORD OnDeleteWave(CBsRealMovie* pObj,DWORD dwParam1,DWORD dwParam2);
	msg_func	DWORD OnDeleteCamera(CBsRealMovie* pObj,DWORD dwParam1,DWORD dwParam2);
	msg_func	DWORD OnDeleteCameraPath(CBsRealMovie* pObj,DWORD dwParam1,DWORD dwParam2);
	msg_func	DWORD OnDeleteEffect(CBsRealMovie* pObj,DWORD dwParam1,DWORD dwParam2);

	msg_func	DWORD OnPrePlay(CBsRealMovie* pObj,DWORD dwParam1,DWORD dwParam2);
	msg_func	DWORD OnRmCommand(CBsRealMovie* pObj,DWORD dwParam1,DWORD dwParam2);

	msg_func	DWORD OnRmEnvironmentCmd(CBsRealMovie* pObj,DWORD dwParam1,DWORD dwParam2);

#endif
};

extern float	g_fRMFps;
//------------------------------------------------------------------------------------------------

