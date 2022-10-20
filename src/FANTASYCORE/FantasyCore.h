#pragma once

#include "resource.h"
#include "FcGlobal.h"

class CSkipper;
class CInputPad;
class Orb;
class CFcAchievement;
class CFcLiveManager;

class CFantasyCore
{
public:
	CFantasyCore();
	~CFantasyCore();

	int InitGameFirstTime(bool bWindow, bool bIsFrameSkip);
	void InitFirstTimeImg();
	void ReleaseGame();

	bool InitTitle();		// 현재는 미션로딩하는 메뉴
	bool InitStage();

	bool IsMissionEnd();
	int  InitPauseMenu();
	bool IsPauseMenu();
	void CheckActiveXUI();

	bool InitSpecialSelStageMenu();
	bool IsSpecialSelStageMenu();
	
	bool InitGameResultSign();
	bool IsGameResultSign();

	bool InitGameResult();
	bool IsGameResult();

	int MainGameLoop();
	bool LoadTerrainParticleList();
	bool LoadSOX();
	bool LoadLocalSOX();
	void ReLoadLocal();

	void ProcessMainState();
	void CustomRender();

	void SetPause( bool bPause );
	void SetPauseForXUI( bool bPause );
	bool IsPause();
	
	void SetMainState( GAME_STATE State, DWORD dwParam1 = 0, DWORD dwParam2 = 0, DWORD dwParam3 = 0 );
	GAME_STATE GetMainState()		{ return g_FCGameData.State; }

	void ReleaseStage();
	void FreeCam();

	bool IsFrameSkip()				{ return m_bIsFrameSkip; }
	void SetFrameSkip( bool bSkip )	{ m_bIsFrameSkip = bSkip; }

	CSkipper* GetSkipper()	{	return m_pSkip;	}

	void StopFrameSkip();
	void StartFrameSkip();
	
	int InputProcess();

	void MainProcess();
	void MainRenderProcess();

	// Thread 관련 함수
	void CreateRenderThread();
	void DestroyRenderThread();
	
	void ProcessStartBack();
	void SetPausMenuforXboxGuide();

	void SaveGlobalGameState();

	void	CreateLiveManager();	// Not XBox Live, but not substitute.
	CFcLiveManager*	GetLiveManager()	{ return m_pLiveManager; }	
	void	SetLiveContext( int nCode, const char* szContext );
	void	SaveLiveContext();
	void	RestoreLiveContext();
	void	ProcessLive();

	bool	IsEndingCharStage();

#ifdef _XBOX
	static void DirtyDiskHandlerCallBackFunc( void );
#endif

#ifdef _TNT_
	// Buffer for XeCR signal messages
	wchar_t g_signalMessage[256];
	static void __cdecl HandleSignal(LPCSTR message, PVOID data);
	void HandleDebugMenu();
	void HandleClearMenus();
	void HandleClosestEnemy();
	void HandleLoadMap(LPCSTR message);
	void HandleChangeHero(LPCSTR message);
	void HandleSetInvincible();
	void HandleGetLoc();
	void HandleWarpToTriggerPoint(LPCSTR message);
#endif	//_TNT_ end

protected:
	BOOL m_bUseThread;
	Orb* m_pOrb;
//	std::vector<CLevelTableLoader>	m_pLevelSOX;
	
	CSkipper*			m_pSkip;
	bool m_bFreeCam;

//	CLevelTableLoader*	m_pLevelSOX;

	bool				m_bIsFrameSkip;
	bool				m_bTestFakeLiquid;
	bool				m_bReleaseTitleData;
	HANDLE				m_hTexInit;
	int					m_nPhysicsEnable;
	int					m_nCubemap;
	int					m_nCubemap1;
	int					m_nLogoTexID;
	int					m_nLevelUpTexID;
	int					m_nTexImgID;
	int					m_nComboEffectTexID[_COMBO_EFFECT_COUNT];
	int					m_nKillEffectTexID[_KILL_EFFECT_COUNT];
	int					m_nKillwEffectTexID[_KILL_EFFECT_COUNT];

	CFcLiveManager* m_pLiveManager;		// RichPresence part
};


// [beginmodify] 2005/10/26 junya
class CTimeRender
{
	BOOL m_bUseThread;
	HANDLE	m_hEndRequest;
	HANDLE	m_hEndAcknowledge;
	
	int		m_nLogoTexID;
	int		m_nTexImgID;
	int		m_nBGImgTexID;
	int		m_nRandValue;

	double m_fStartLoadingTime;

public:
	CTimeRender( BOOL bUseThread, LONG lPeriod = 10, BOOL bManualReset = FALSE );
	~CTimeRender();

	static HANDLE	ms_hRenderProgressThread;
	static DWORD	ms_dwRenderProgressThreadID;

private:
	static DWORD WINAPI RenderProgressThreadProc( LPVOID lpParameter );
	static void RenderLoading(CTimeRender* pSelf, DWORD dwStartTime);
};


extern CFantasyCore	g_FC;
extern HANDLE g_hPackedResTitle;
extern HANDLE g_hPackedTexTitle;
extern HANDLE g_hPackedRes1;
extern HANDLE g_hPackedTex1;
extern HANDLE g_hPackedRes3;
extern HANDLE g_hPackedTex3;