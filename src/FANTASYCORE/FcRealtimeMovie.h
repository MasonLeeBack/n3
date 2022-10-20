#pragma once

#include "CDefinition.h"

class CBsRealMovie;
class CRMObject;
struct msgRMTNotify;
class CRMCamera;
class CRMEffect;
struct rmEffWavSet;

typedef std :: vector<rmEffWavSet> vecEffWavList;

class FcRealtimeMovie;

typedef VOID	(*PTR_RM_FUNC)(FcRealtimeMovie* pFcRM);

namespace RM
{
	struct FXData
	{
		FXData() : nObjIndex(-1), nIndex(-1), pEffect( NULL ) {}
		int nObjIndex;
		int nIndex;
		VOID*	pEffect;
		D3DXMATRIX Mat;
	};

} // namespace RM

typedef std::vector< RM::FXData>	vecFXData;

typedef	std::map< std::string , int >		mapStrInt;
typedef std::pair< std::string , int >		pairStrInt;
typedef std::map< std::string , int >::iterator		mapStrIntIterator;

struct RM3DSndPack
{
	int nHandle;
	int nSoundIndex;
	CRMObject* pObjPtr;
};

#define SNDPOOL_SIZE	20


class FcRealtimeMovie
{
public:
	FcRealtimeMovie();
	~FcRealtimeMovie();

	void	SetFilename(char* szName)
	{
		strcpy( m_szFileName, szName );
	}

	char*	GetFilename(void)
	{
		return m_szFileName;
	}

	static VOID	EmptyRenderProcess(FcRealtimeMovie* pFcRM);
	static VOID	PreAllocateRenderProcess(FcRealtimeMovie* pFcRM);	// Load�� �ϴ� ����/
	static VOID	AllocateRenderProcess(FcRealtimeMovie* pFcRM);		// ���������� ���ҽ� ����.
	static VOID	ReleaseRMDataRenderProcess(FcRealtimeMovie* pFcRM);		// RenderProcess���� Clear�� ���� ����.
	static VOID	ReleasePostProcess(FcRealtimeMovie* pFcRM);

	static VOID StartSignalWorkProcess(FcRealtimeMovie* pFcRM);	// ���� ��ȣ ����.

	VOID	PreLoadRealMovie(char* pFileName);
	void	PlayDirect(char* pFileName);	// PreLoadRealMovie, StartRealMovie �������� ���� ���� ����.

	void	LoadRealMovie(char* pFileName);
	void	StartRealMovie();
	void Process();
	void Update();
	void	Clear();
	void ResetAni();
	bool IsPlay()	{ return m_bPlay; }
	BOOL	IsLoadComplete()	{	return m_bLoaded;	}
	BOOL	IsSetDirectMsg()	{ return m_bDirectMsg; }

	void SetPause( BOOL bPause );

	void	LoadWaveResource(CBsRealMovie& rRealMovie);

	static VOID*	LoadPhysicsData( char *szFileName );
	static VOID		ReleasePhysicsData( VOID *pPhysicsInfo );
	static VOID*	LoadCollisionData( char *szSkinFileName );
	static VOID		ReleaseCollisionData( VOID *pCollisionInfo );

	struct 
	{
		int		nPreviousSeqIndex;
		int		nBaseTick;
		BOOL	bUsePreviousCam;

	} UseCamInfo;

	LARGE_INTEGER m_liSaveTime;
	int					m_nSavedCamSeqIndex;

	msgRMTNotify* m_pKey; //[96];

	PTR_RM_FUNC			m_pfnRenderProcess;
	PTR_RM_FUNC			m_pfnWorkProcess;
	PTR_RM_FUNC			m_pfnPostRenderProcess;
	

	int			m_nTempObjCnt;
	int			m_nTempIndex;

	VOID	ReleaseFXPool(VOID);
	VOID	ReleaseFXHandle(VOID);
	

	//PTR_FUNC	m_pfnRealtimeMovieRenderProcess;
	//VOID	(FcRealtimeMovie::*m_pfnRealtimeMovieRenderProcess)(VOID);

	VOID	SetConsoleFlag(VOID)	{	m_bConsoleMsg = TRUE;	}
	VOID	ClearConsoleFlag(VOID)	{	m_bConsoleMsg = FALSE;	}
	BOOL	GetConsoleFlag(VOID)	{	return m_bConsoleMsg;	}

	VOID	EnableCmdFX(float fSX,float fSZ,float fWidth,float fHeight,int nID,int nTime);
	VOID	DisableCmdFX(int nID);

	VOID	TerminateRealMove(VOID);

	VOID	RMCmdSetTroop( msgRMTNotify* pKey );

	VOID	SetPostPauseFlag(BOOL bSet)			{	m_bPostPause = bSet;	}
	BOOL	GetPostPauseFlag(VOID)				{	return m_bPostPause;	}

	VOID	DeleteContents(VOID);
	

protected:

	static BOOL	m_bPause;	// XBOX ���̵� ��½�.
	bool m_bPlay;
		
	int m_nTick;

	float	m_fOldNear;
	float	m_fOldFar;

	// BC Cam ��.
	float	m_fStartTime;
	DWORD	m_dwStartTick;
	CRMCamera*	m_pRMBCCam;

	// Light ���
	D3DXVECTOR4	m_vecGameLightDir;

	vecEffWavList	m_vecSndEventList;

	// Process Tick ����.
	int		m_nOldTick;
	char	m_szFileName[32];

	// State ����
	BOOL	m_bLoaded;

	// Direct Message ����
	BOOL	m_bDirectMsg;	// ���� ����� äũ��.
	BOOL	m_bConsoleMsg;	// �ܼ� ����� ���.

	
	// FX ����
	vecFXData			m_vecFXPool;
	mapStrInt			m_mapFXHandlePool;

	// 3D Sound ����
	RM3DSndPack*		m_pSndHandlePool[SNDPOOL_SIZE];

	// �Ǳ� ���� ���� �� Pause �޴� ����
	BOOL				m_bPostPause;

	HANDLE				m_hResPack;
	HANDLE				m_hTexPack;


	void	ClearSndPool(void);
	void	InitSndPool(void);
	int		FindEmptySndSlotIndex(void);
	int 	SetSndSlot(int nIndex,int nHandle,int nSndIndex,CRMObject* pObjPtr);
	void	RMSndProcess(void);		// ���� ó�� �κ�.
	void	DeleteSndSlot(int nIndex);

	int		PlayRMSound( int nTick, int nCol, char* pCurName,D3DXVECTOR3* pPos,CRMObject* pObjPtr);
	
	VOID	PreLoadFXForRM(CBsRealMovie* pRealMovie);

	BOOL	CreateFXForRM( CRMEffect* pEffect );
	VOID	ProcessFX(VOID);

	PUBLIC_GETSET_PROP(bool, IsDelete, m_bIsDelete);
};



extern FcRealtimeMovie* g_pFcRealMovie;
// extern VOID	(FcRealtimeMovie::m_pfnRealtimeMovieRenderProcess)(VOID);