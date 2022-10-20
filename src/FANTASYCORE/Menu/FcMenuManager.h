#pragma once
#include "FCinterfaceObj.h"

#include "FcGameObject.h"
#include "BSuiSystem.h"
#include "FcMenuDef.h"

#include "FcMenu3DObject.h"
#include "FcMenuWorldMap.h"
#include "FcMenuLibraryManager.h"

class CFcMenuForm;


enum{
	_MENU_WAVE_START,
	_MENU_WAVE_RUN,
	_MENU_WAVE_END,
};

enum SYS_MESSAGE{
	_SYS_MSG_REMOVE_PAD,	
	_SYS_MSG_REMOVE_STORAGE,
	_SYS_MSG_CANCEL_DEVICESELECTOR,	// 디바이스 선택을 취소 했을 시
};


struct FcMessageToken {
	FCMenu_Message	message;
	DWORD			Param1;
	DWORD			Param2;
	DWORD			Param3;
	DWORD			Param4;
	DWORD			Param5;
	DWORD			Param6;
};

typedef std::deque <FcMessageToken* >		FcMSGTOKENDEQUE;
typedef FcMSGTOKENDEQUE::iterator		FcMSGTOKENDEQUEITOR;

typedef std::vector<CFcMenuForm* >		FcMenuLIST;
typedef FcMenuLIST::iterator			FcMenuLISTITOR;

typedef std::vector<int>				FcLoadMenuLIST;
typedef FcLoadMenuLIST::iterator		FcLoadMenuLISTITOR;


class CFcMenuManager:public CFcInterfaceObj//public CFcLayerObject
{
public:
	CFcMenuManager();
	~CFcMenuManager();

	void PostMessage(FCMenu_Message message, DWORD param1=0, DWORD param2=0, DWORD param3=0, DWORD param4=0, DWORD param5=0, DWORD param6=0);
	void Release();

	void RemoveAllMenu();

	_FC_MENU_TYPE	GetCurMenuType();
	DWORD			GetCurMenuHandle();
	
	CFcMenu3DObjManager*	Get3DObjectMgr()		{ return m_pFcMenu3DObj; }
	FcMenuWorldMapInfo*		GetMenuWorldMapInfo()	{ return m_pMenuWorldMapInfo; }
	FcMenuWorldMap*			GetMenuWorldMap()		{ return m_pMenuWorldMap; }

	void SetGamePauseMenu(int nMenuType)			{ m_nGamePauseMenuType = nMenuType; }
	int	 GetGamePauseMenu()							{ return m_nGamePauseMenuType; }
	
	void LoadGameMenuData();
	bool LoadMenuScript(_FC_MENU_TYPE nType);

	void CheckedSystemMessageBox(SYS_MESSAGE nMsg, DWORD param1=0, DWORD param2=0, DWORD param3=0, DWORD param4=0);

	CFcMenuLibraryManager* GetLibManager()			{ return m_pLibManager; }
	bool IsSameMenuMessage(FCMenu_Message message);
	void ReleaseLoadMenu()							{ m_LoadMenuList.clear(); }

	void EnableKey(bool bEnable=true) { m_bEnableKey = bEnable; }

	CFcMenuForm*	GetMessageBoxMenu(int nMsgAsk);
	bool IsSameMessageBox(int nMsgAsk, bool bExceptionClose);

	int GetWaveState() const { return m_WaveState; }


protected:
	virtual void Process();
	virtual void UpdateProcess();
	
	void CreateMovieRTT();
	void Create3DObjRTT(int nTextureWidth, int nTextureHeight,
		float fStartX, float fStartY,
		float fWidth, float fHeight, bool bLight);

	void ReleaseMovieRTT();
	void Release3DObjRTT();
	
	void OnInputKey();
	void TranslateMessage(int Input);
	void Update();
	DWORD DispatchMessage(FcMessageToken* pMsg);

	bool OpenMenu(_FC_MENU_TYPE nType, DWORD param1=0, DWORD param2=0, DWORD param3=0, DWORD param4=0, DWORD param5=0, DWORD param6=0);
	bool CloseMenu(_FC_MENU_TYPE nType, _FC_MENU_TYPE* nPreType=NULL, CFcMenuForm* pMenu=NULL, bool bReleaseMovieRTT = true);

	bool RemoveMenu(_FC_MENU_TYPE nType, CFcMenuForm* pMenu=NULL);
	
	bool IsSameMenu(_FC_MENU_TYPE nType, CFcMenuForm* pMenu=NULL);

	CFcMenuForm*	GetMenu(_FC_MENU_TYPE nType);

	bool IsSameLoadMenu(_FC_MENU_TYPE nType);
			
	void UpdateSpecialEffect();
	bool UpdateBGMovie();
	void UpdateGameInterface();

	void UpdateWave(FcMessageToken* pMsg);
	bool CheckMenuWaveState();

	bool UpdateSound(FCMenu_Message message);

	void CreatePreCharLoadingThread();
	void ReleasePreCharLoading();
	void WaitForPreCharLoading();

	static DWORD WINAPI PreCharLoadingThreadProc( LPVOID lpParameter );
	void SetSystemMessageLossOfController();
	void SetSystemMessageCancelledDeviceSelector();
	void SetSystemMessageRemoveStorage();

	void SetSystemMessage(FcMessageToken* pMsg);
	void SetGuardianforFirstMission();

	
protected:

	HANDLE				m_hPreCharLoadingThread;

	static std::vector<int> m_PreLoadSkinList;
	static std::vector<int> m_PreLoadAniList;
	static std::vector<int> m_PreLoadTextureList;
	static std::vector<int> m_PreLoadFXList;

	FcMenuLIST			m_MenuList;
	FcMSGTOKENDEQUE		m_msgQue;						// 전체 메시지 큐

	FcLoadMenuLIST		m_LoadMenuList;
		
	DWORD				m_hMovieRTTHandle;
	int					m_nMovieTextureId;

	CFcMenu3DObjManager*	m_pFcMenu3DObj;
	DWORD					m_h3DObjRTTHandle;

	FcMenuWorldMap*			m_pMenuWorldMap;
	FcMenuWorldMapInfo*		m_pMenuWorldMapInfo;

	int					m_nGamePauseMenuType;		//start나 back 모두 pause 상태를 만들지만 호출하는 메뉴는 틀리다.

	bool				m_bSetToggleOpenMovie;

	bool				m_bEnableKey;
	POINT				m_WavePos;
	int					m_AddWaveCount;
	int					m_nMAXWaveCount;
	int					m_nWaveSize;
	int					m_nWaveSpeed;
	
	int					m_WaveState;

	CFcMenuLibraryManager*	m_pLibManager;
};

typedef CSmartPtr<CFcMenuManager> MenuObjHandle;