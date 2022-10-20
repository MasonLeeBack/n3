#pragma once

#ifdef _XBOX

#include <map>
#include <string>

#define NUM_MIN_USERS		0
#define NUM_MAX_USERS		4
#define NUM_SIGNIN_PANES	1
#define NUM_USERS			4

class CFcAchievement;

enum SETTING
{
	Setting_YAxis_Inversion = 0,
// [beginmodify] 2006/2/13 junyash PS#4786 does not allow the vibration from the gamer profile.
	Setting_Controller_Vibration,
// [endmodify] junyash
	NUM_SETTINGS
};

const DWORD SettingIDs[ NUM_SETTINGS ] =
{
	XPROFILE_GAMER_YAXIS_INVERSION,
// [beginmodify] 2006/2/13 junyash PS#4786 does not allow the vibration from the gamer profile.
	XPROFILE_OPTION_CONTROLLER_VIBRATION,
// [endmodify] junyash
};

struct USER_DATA
{
	XUID            xuid;                     // user id
	DWORD           dwUserIndex;             // user index (0..3)
	DWORD           dwLastPresenceEnumerate;

	DWORD			dwPresenceID;
	DWORD			dwDescID;
};


union UMapCode
{
	char szCode[2];
	unsigned short nCode;
};

struct MapCode
{
	DWORD dwMapCode;
	UMapCode unit;
	unsigned short nDummy;
};

struct MsgContextPack
{
	int nMsg1;
	int nMsg2;

	MsgContextPack() : nMsg1(0), nMsg2(0) {};
	MsgContextPack(int nId1,int nId2) : nMsg1( nId1 ) , nMsg2( nId2 ) {};
};

#define LIVE_CONTEXT_MENU 0
#define LIVE_CONTEXT_MAP  1

#define LIVE_CONST_TITLE	"title"
#define LIVE_CONST_CHAR		"character"
#define LIVE_CONST_ENDING	"ending"
#define LIVE_CONST_LIBRARY	"library"
#define LIVE_CONST_OPTION	"option"
#define LIVE_CONST_TUTORIAL "tutorial"
#define LIVE_CONST_SAVE		"save"
#define LIVE_CONST_LOADING	"loading"
#define LIVE_CONST_PAUSED	"paused"




class CFcLiveManager;

typedef VOID	(*PTR_LM_FUNC)(CFcLiveManager* pFcLM);

typedef std::map< std::string , MsgContextPack >	mapStrPack;

class CFcLiveManager
{
public :

	CFcLiveManager();
	virtual ~CFcLiveManager();

#ifdef _TNT_
#ifdef _TICKET_TRACKER_
	VOID	InitializeXNet(VOID);	// XNet, WSA setting non default parameters
#endif //_TICKET_TRACKER_
#endif //_TNT_

	HRESULT	Initialize(VOID);	// Live Start Up , Sign In and Set Notify Listener
	VOID	CheckSignedInInfo(VOID);
	VOID	Process(VOID);


	VOID	ProcessDeviceUI(VOID);
	HRESULT UpdateAndRetrieveNotify(VOID);		// 통지 받기. 
	VOID	BringUpFriendsUI(VOID);
	VOID	ShowSigninUI(VOID);
	VOID	ShowAchievementUI(VOID);



	VOID UpdatePresence();		// 온라인 상태 업데이트.
	VOID EnumeratePresence( DWORD dwUserIndex );

	VOID UpdateAchievement();

	VOID	SetUIActive(BOOL bFlag)		{	m_bUIActive = bFlag;	}
	BOOL	IsUIActive(VOID)			{	return m_bUIActive;		}
	DWORD	GetUserCount(VOID)			{	return m_dwUserCount;	}
	DWORD	GetPresenceCount(VOID)		{	return m_dwPresenceCount;}
	DWORD	GetUserIndex(DWORD dwCnt )	{	return m_UserData[dwCnt].dwUserIndex;	}

	BOOL	HasSignedInUser(VOID);
	BOOL	IsUserSignedIn(VOID) const;
	
	CFcAchievement* GetAchievement() { return m_pAchievement; }

	WCHAR*	GetFriendsRichPresence(DWORD nIdx)
	{
		return m_Friends[nIdx].wszRichPresence;
	}

	USER_DATA               m_UserData[NUM_USERS];  // user specific data
	DWORD                   m_dwUserCount;          // number of users signed in
	DWORD					m_dwSignedInUserMask;// old bitfields for signed-in users 
	BOOL                    m_bFriendsUIActive[NUM_USERS]; // friends UI active
	BOOL                    m_bUIActive;            // is any UI active
	HANDLE                  m_hNotification;        // UI notification handle

	DWORD					m_dwStartTick;	// 테스트용.
	XONLINE_FRIEND          m_Friends[NUM_USERS];
	DWORD                   m_dwPresenceCount;
	DWORD					m_dwWorkerThreadID;
	HANDLE					m_hWorkerThread;

	CFcAchievement*			m_pAchievement;

	void	SetRichPresenceMainID(DWORD dwPCode = 0 );			// g_FCGameData.nPlayerType 이용할것.
	void	SetRichPresenceDescID(DWORD dwDCode = 0 );
	void	SetRichPresenceInfo( int nCode,const char* szMapName );

	void	MakePresenceMap(void);
	void	ClearPresenceMap(void);

	// RichPresence data
	mapStrPack							m_mapPresence;


	// Settings data
	int									m_nYAxis_Inversion;
	bool								m_bController_Vibration;
	DWORD                               m_dwSettingSizeMax;
	XOVERLAPPED							m_Overlapped;
	DWORD								m_dwOverlappedReadStatus;

	XUSER_PROFILE_SETTING               m_Settings[ NUM_SETTINGS ];
	XUSER_PROFILE_SETTING               m_WriteableSettings[ NUM_SETTINGS ];
	XUSER_READ_PROFILE_SETTING_RESULT*  m_pSettingResults;
	

	BOOL	ProfileInitialize();
	void	ReleaseProfileBuffer();
	void	ReadProfileSettings();

	PTR_LM_FUNC		m_pfnProfileProcess;

	static VOID		EmptyProcess(CFcLiveManager* pFcLM);
	static VOID		ProfileOverlappedIoPorcess(CFcLiveManager* pFcLM);

	void	SaveContext();
	void	RestoreContext();
protected:
	char	szCurrContext[64];
	char	szSaveContext[64];
	int		nCurrCode;
	int		nSaveCode;
};
// extern CFcLiveManager	g_FcLiveManager;
#else

class CFcLiveManager
{
public :

	CFcLiveManager(){}
	virtual ~CFcLiveManager(){}

	HRESULT	Initialize(VOID)	{	return S_OK;	}	// Live Start Up , Sign In and Set Notify Listener
	VOID	Process(VOID)		{}

	BOOL	HasSignedInUser(VOID) { return FALSE; }
	BOOL	IsUserSignedIn(VOID) { return FALSE; }
};


#endif