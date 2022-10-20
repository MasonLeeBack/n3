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


class CBsRealMovie : public CBsFileText // ���� Ȯ���� BSRTM
{
public:
	// enum { eFps60=60, }; // MAX ���� 60 ������ ���� �Դϴ�

#ifndef _XBOX

	std::map<int,PTR_FUNC>	m_BRMCommander;
	typedef std::map<int,PTR_FUNC>	mapCmder;
	mapCmder&	GetCommander()	{	return m_BRMCommander;	}
	void	DrawCamLines(void);
	void	DrawCamPivot(void);
#endif

	//----------------------------------------
	// ���Ͽ� �����Ǵ� ����Ÿ �Դϴ�
	//----------------------------------------

	struct SHeader {
		int  m_nOffset; // �� ���̸�ŭ ��� ������ �н��ϴ� (���� ���� �뵵 �Դϴ�)	= sizeof(SHeader)
		int  m_nVersion; // ��ġ ����
		int  m_nObjectMaxID; // ���� �����ǰ� �ִ� ������Ʈ ID

		void Set() {
			m_nOffset=sizeof(SHeader);
			m_nVersion=1;
			m_nObjectMaxID=0;
		}
	}
	m_Header;

	char m_szPathName[128]; // �۾� �о� �Դϴ�

	//-----------------------

	stMapInfo	m_BGInfo;		// ��׶��� ����.(Map ��Ÿ ���)

	// SKIN, BA 100 �� ������ ����Ÿ�� ��Ī �մϴ� (SKIN, BA �� ���Ϸ� ���� ���� �մϴ�)

	std::map< std::string , CRMObject* >	m_mapObject;
	typedef std::map< std::string , CRMObject* >::iterator	iteratorRMObject;
	typedef std::pair< std::string , CRMObject* >	pairStrObj;
	CRMObject*	m_pSelectedRMObject;

	// ���� ������ ���� �մϴ� (WAV ���� ���� �����մϴ�)
	std::map< std::string , CRMWave* >		m_mapWave;
	typedef std::map< std::string , CRMWave* >::iterator	iteratorRMWave;
	typedef std::pair< std::string , CRMWave* >	pairStrWav;

		  
	// ī�޶� ���� �մϴ�
	std::map< std::string , CRMCamera* >	m_mapCamera;
	typedef std::map< std::string , CRMCamera* >::iterator	iteratorRMCamera;
	typedef std::pair< std::string , CRMCamera* >	pairStrCam;
	vecCamera	m_vecCamera;		// mapCamera�� ���� �� ���� �ִ´�. ��Ʈ��

	CRMCamera*	m_pDefCam;
	CRMCamera*	m_pStaticCam;
	CRMCamera*	m_pSelectedCam;

	// ����Ʈ ��ƼŬ ����Ÿ�� ���� �մϴ� 
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



	// �Ǳ� ����� ����� �ٷ� �޸� ���� �ǹǷ� ū �޸� ����� ���� �մϴ�
	//
	// 1. ���� ���� ��ũ (�κ�����)	- ����, �罽
	// 2. ����
	// 3. ĳ����, ��, ��ű� ��ü
	// 4. 1P, 2P, Į���
	// 5. ����Ʈ (��, � ���� õ�� ���� ����)

	//----------------------------------------
	// ���� ����� ���� ����Ÿ �Դϴ�
	//----------------------------------------

	// static CBsFileBALM st_BALM; // ����ũ�� ��Ī ���� ���� (���� �Դϴ�)

	int m_nBlend; // ���� Ƚ���� �����ϰ� �ִ� �ӽ� ���� �Դϴ�
	int m_bFadeOut; // ���̵� �ƿ� �� ���¸� �����ǵ��� �մϴ�

	int  m_nVersion;


	int			m_nEventCnt;
	stEvent		m_EventArray[EVENT_COL_MAX];

	int	m_nbUseRMCam;		// �Ǳ� ����� ī�޶� ��� ����. �⺻�� 1.
	int			m_nTerminateFlag;
	float		m_fTerminateTime;
	int			m_dwTerminateTick;

	PTR_FUNC_PHYSICS_SET_FUNC	m_pfnCreatePhysics;	// Physics ��Ų �ε�� �Լ� ������.
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


	// Area ����
	vecAreaSet		m_vecAreaSetPool;

	void	ClearAreaPool(void);
	void	ResizeAreaPool(int nNewSize);
	int		GetAreaPoolSize(void);
	stAreaSet*	GetAreaSetFromPool(int nIndex);
	stAreaSet*	FindArea(char* szAreaName);

	// Light ����.
	vecLightSet		m_vecLightSetPool;

	void	ClearLightPool(void);
	void	ResizeLightPool(int nNewSize);
	int		GetLightPoolSize(void);
	stLightPack*	GetLightSetFromPool(int nIndex);
	stLightPack*	FindLightSet(char* szLightName);

	// Point Light ����
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

	// Package ����.
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
	CBsRealMovie(); // ��� �ִ� ������ ����� ������ �ɰ��Դϴ�


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
	void Load_text(BStream *pStream) { } // ���� (��� ���� ������ ���� �մϴ�)

	// void Load_t(int md12); //?? �׽�Ʈ�� ������ ���� ���� �Դϴ�

    /*

		��� ���� �Դϴ� <1>
		|
		CBsRealMovie *pMovie = new CBsRealMovie(); // ū �޸� �� 1 ���� �����ȴٰ� ���� �ֽ��ϴ�

		{ 
			pMovie->Load(szFileName); // Ŭ���� �� �ʿ䰡 ���� �޸𸮿� ����Ÿ�� �ε� �մϴ�
			pMovie->AllocateResource(); // -Clear() ȣ����- , ����Ÿ�� ����ϴ�
			
			pMovie->Play(fTime, pMatrix); // �÷��� �մϴ�
		}
		|
		�ݺ� ���� �մϴ�

		delete pMovie; // -Clear() ȣ����- . pMovie �� Ȯ���ϰ� �ִ� �Ƹ��� sizeof(CBsRealMovie) ��ŭ�� ����Ÿ�� ���� ��ŵ�ϴ�. ���� ���� ������ Ŀ�ΰ� �浹 ���ɼ��� �ֽ��ϴ�

		pMovie->Clear(); // �ʿ��Ҷ� �޸𸮿��� ���� �մϴ� = AllocateResource() �Լ������� �ڵ� ȣ�� �մϴ�

		----------------------------

		��� ���� �Դϴ� <2>
		|
		Ŀ�ο� �ִ� ���� ���븸 ����մϴ�

		void *m_pRealMovie;	// CBsRealMovie * �Դϴ� (���� ���� ���� �Դϴ�)

		void RealMovieLoad(char *szFileName); // �ε� �մϴ� (�޸� �Ҵ�)
		void RealMoviePlay(float fTime); // �÷��� �մϴ�
		void RealMovieClear(); // ����� �ݵ�� Ŭ���� �մϴ� (�޸� �Ұ�) m_pRealMovie=NULL (���α׷� ����ÿ��� �ڵ� Ŭ���� �˴ϴ�)

	*/

	void SetDefaultLighting();

	void ChdirStart();
	void ChdirEnd();
	void Chdir(char* szPath);


	//int  Is_nObjID(int nObjID); // ���⼭�� �з����Ͱ� nObjID �ӿ� ���� �մϴ�
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

	void AllocateResource(); // Load() ������ ����Ÿ�� ��������, �ش� ���ҽ��� ��� �Ҵ� �մϴ�


	int		PreProcess(float fTime,DWORD dwTick,msgRMTNotify* pRMTNotify);
	void	Process(float fTime,int nTick);
	int		PostProcess(float fTime);

	void	Update(float fTime);
	void	ProcessBone();

	void	ResetAni(void);

	// void Play     (float fTime, D3DXMATRIX *pMatrix, CCrossVector *pCameraCross, CCrossVector *pLightCross); // �ش� �ð��� ���� ��ο� �մϴ�

#ifndef _XBOX
	void	SortCameraWithTime(void);
#endif
	CRMCamera*	GetCameraByTime(float fTime);

	// CRMEffect*	ProcessEffect(CRMEffect* pEffect,float fTime);
	CRMEffect*	ProcessEffect(CRMEffect* pEffect,int nFrame);

	void	ProcessSelectedEffect(CRMEffect* pSelectEffect,int nFrame);
	void	ProcessPackage(CRMPackage* pPack,float fTime,DWORD dwTick);

	void PlayEff_ ( int nFrame ); // ����Ʈ ��ƾ 
	void PlayEff_ForProcess( int nFrame );
	void PlayWave_( int nFrame ); // ���̺� ��ƾ 
	//                   |
	//                   �ð� (3.2f �� 3.2 �ʸ� ���մϴ�)

	void Clear();
   virtual ~CBsRealMovie();

    void	DeleteObject(char* szObjName);	// ���� ȣ�� ���� �ʽ��ϴ�
	int		DeleteWave(char* szWavName);
	int		DeleteCamera(char* szCamName);
	int		DeleteCameraPath(char* szCamName,int nIndex);
	int		DeleteEffect(char* szEffName);

	void  DrawUITextDebug_LS(int nLTF_id43, int nBALM_id16, int y); // ����ũ�� ����� ���� �Դϴ�

	//---------------------------

	//    m_nObjectIndex[] = ������Ʈ �ε���
	//    m_nAniIndex[]    = �ִϸ��̼� �ε��� (����ũ�� ��쿡�� 0 ���� ���� - ����) = �������� index+1 �� �����ݴϴ� (��Ʈ���� ������ ��ġ ������)
	//    m_fTime[]        = Ÿ��
	//    |
	CRMObject*	InsertObject(char* szName); // nObject01 = nTreeIndex 
	CRMObject*	InsertEmptyObject(char* szName);	// Load ��.
	int		InsertAct(char* szObjName,char* szActName);
	int		InsertLip(char* szObjName,char* szLipName);
	int		InsertFace(char* szObjName,char* szFaceName);
	int		InsertBone(char* szObjName,char* szBoneName);
	
	CRMEffect*	InsertEff   (char* szEffName); 
	CRMWave*	InsertWave  (char* szWaveName); // nObject01 �� �������� �������� -1 �� ��� �մϴ� (Wave ���� �ش�)
	CRMCamera*	InsertCamera(char* szCamName); // ī�޶�� 1 �� �Դϴ� 

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


	//                                                        ���� �ϰ� ���� �ð� �Դϴ� = ���� ���� nLipIndex �� �ش��ϴ�, �Ǳ� �Է°� �ð� �Դϴ� 
	//                                                        |
	float GetEvent    (int &nLipIndex, int &nObject01, float fCurrentTime); // ����ũ �ִϰ� ������ ������ �뺸 �մϴ� (1���� �뺸 �˴ϴ�)
	//                       |
	//                       �ش� �ϴ� ���� �������� nLipIndex=-1 �Դϴ� (m_Lip �� �ε���)

	void  PlayedEvent (int  nLipIndex); // �ش� nLipIndex �� ó�� �Ǿ��ٸ� �� �Լ��� ȣ���� �ּ��� (���Ŀ� �� �̺�Ʈ�� �ٽ� ȣ�� ���� �ʽ��ϴ�)
	void  ResetEvent  (float fCurrentTime); // �ش� �ð� ������ �̺�Ʈ(evt>=fCurrentTime)�� ���µ˴ϴ� (����)

	//
	int   GetEffEvent (float fCurrentTime);
	void	SetTerminateTime( float fTime );
	int		GetLastEvent( msgRMTNotify* pRMTNotify );
	//

	float GetWaveEvent    (int &nWaveIndex, int &nObject01, float fCurrentTime);
	void  PlayedWaveEvent (int  nWaveIndex); // �ش� nWaveIndex �� ó�� �Ǿ��ٸ� �� �Լ��� ȣ���� �ּ��� (���Ŀ� �� �̺�Ʈ�� �ٽ� ȣ�� ���� �ʽ��ϴ�)
	void  ResetWaveEvent  (float fCurrentTime); // �ش� �ð� ������ �̺�Ʈ(evt>=fCurrentTime)�� ���µ˴ϴ� (����)

	//---------------------------

	char*   GetSKFilenameBuffer(char* szObjName); // nObject01 = nIndex  (SKIN)
	char*   GetBAFilenameBuffer(char* szObjName); // (BA)

	char*   GetLipFilenameBuffer(char* szObjName,char* szLipName); // filename (LTF)
	char*   GetLipWaveNameBuffer(char* szObjName,char* szLipName); // wavename (WAVE)

	void    SetCurPath(LPCTSTR szCurPath, LPCTSTR szFilename); // ����Ÿ�� ���丮�� ��� �մϴ�

	DWORD	RealMovieCommand(int nMsg,DWORD dwParam1=0,DWORD dwParam2=0);

	// [in] �ð�, [out] ����
	int		EventNotify( float fTime , DWORD dwTick );

	VOID*	GetSelectedEvent(int nIndex,int& rType)	
	{	
		rType = m_EventArray[nIndex].nType;
		return m_EventArray[nIndex].pEventObj;
	}

	VOID	ResetFlags(VOID);		// �̺�Ʈ �ʱ�ȭ, ī�޶� �ʱ�ȭ���� �Ѵ�.

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


	// �����ϰ� ��Ͻÿ� äũ��.
	// fTime : �̺�Ʈ ���� �ð�, 
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

