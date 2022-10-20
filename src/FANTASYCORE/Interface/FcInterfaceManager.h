#ifndef __FC_INTERFACE_MANAGER_H__
#define __FC_INTERFACE_MANAGER_H__

#include "FcInterfaceDataDefine.h"
#include "Singleton.h"
#include "FcMenuManager.h"
#include "FcInterfaceMisc.h"
#include "BsUiImageCtrl.h"
#include "data/Text/DefaultTextTableDefine.h"

#define _FC_MINIMAP_SIGHT		"minimap_sight_128_cam.dds"
#define _GUARDIAN_DECAL_TEXTURE	"Guardian.dds"
#define _GUARDIAN_TARGET_DECAL_TEXTURE	"GuardianTarget.dds"

#define _ENEMY_TARGET_SKIN		"char\\EnemyTargetArrow\\mn_Enemy_Target_arrow.SKIN"
#define _ENEMY_TARGET_ANI		"char\\EnemyTargetArrow\\mn_Enemy_Target_arrow.BA"

#define _GAUGE_HP_MESH			"Gauge_Hp.BM"
#define _GAUGE_ORB_MESH			"Gauge_Orb.BM"
#define _GAUGE_TRUEORB_MESH		"Gauge_TrueOrb.BM"
#define _GAUGE_HP_DAMAGE		"Gauge_Hp_Damage.dds"
#define _GAUGE_HP_BASE			"Gauge_Hp_base.dds"
#define _GAUGE_ORB_MAX_LIGHT	"Gauge_Orb_Max_Light.dds"
#define _GAUGE_TRUEORB_MAX_LIGHT	"Gauge_TrueOrb_Max_Light.dds"
#define _GAUGE_HP				"Gauge_Hp.dds"
#define _GAUGE_ORB				"Gauge_Orb.dds"
#define _GAUGE_ORB_MAX			"Gauge_Orb_Max.dds"
#define _GAUGE_TRUEORB			"Gauge_TrueOrb.dds"
#define _GAUGE_TRUEORB_MAX		"Gauge_TrueOrb_Max.dds"


#define MAX_INTERFACE_OBJ		64
#define _INTERFACE_OBJ_MENU		63
enum _INTERFACE_OBJECT_TYPE
{	
	_IOT_MENU,
	_IOT_MINIMAP,
	_IOT_SPEECH,
	_IOT_REPORT,
	_IOT_ITEMREPORT,
	_IOT_MISSIONGOAL,
	_IOT_SPECIALTEXT,
	_IOT_SPECIALGAUGE,
	_IOT_ABILITY,
	_IOT_LETTERBOX,
	_IOT_GAUGE,
	_IOT_FADEEFFECT,
	_IOT_MSGSHOW,
};

class CFcMinimap;
class CFcGauge;
class CFcPortrait;
class CFcSpeech;
class CFcReport;
class FCTextTable;
class FcAbilInterface;
class CFcMenuManager;
class CFcInterfaceObj;
class CFcTroopObject;


class FcInterfaceManager : public CSingleton<FcInterfaceManager>
{
public:
	FcInterfaceManager();
	~FcInterfaceManager();

	void Initialize();
	void Finalize();
	
	void InitStage();
	void FinishStage();

	void Clear();
	
	void Process();

	bool SetUserTextTable( const char* pFileName );
	CFcMenuManager*		GetMenuMgr()				{ return (CFcMenuManager*)m_InterfaceList[m_MenuHandle]; }

	bool IsShowHandle(int nMenuType);
	void SetShowHandle(int nMenuType, bool bShow);
	int GetMenuObjHandle(int nMenuType);

//Show interface--------------------------------------------------------------------------------------	
	void ShowInterface(bool bShow);
	bool IsInterfaceShow()				{ return m_bShowInterface; }

	void ForceShowInterface(bool bShow);
	bool IsInterfaceForceShow()			{ return m_bForceShowInterface; }
	
//speech--------------------------------------------------------------------------------------
	void SetSpeech(int PortraitID,int nTextTableID);
	void SetSpeechOn(bool bOn);
	bool GetSpeechOn();
	bool IsSpeechPlay();

//letterbox--------------------------------------------------------------------------------------
	void ShowLetterBox(bool bShow);
	void SetLetterOn(bool bOn);
	float GetLetterBoxAlpha();
	bool IsShowLetterBox();
	void ShowTextInLetterBox(int nTextTableID);
	bool IsShowTextInLetterBox();
	void HideTextInLetterBox();

//Minimap--------------------------------------------------------------------------------------
	void MinimapShowToggle();
	void MinimapShow(bool bShow);
	void MinimapZoomChange(float fZoom);
	void MinimapPointOn(bool bShow, const char *szName, CFcTroopObject *pTroop = NULL, DWORD nTick = 0);
	void MinimapAreaOn(bool bShow, const char *szName, float fStartX = 0.f, float fStartY = 0.f,
		float fEndX = 0.f, float fEndY = 0.f, DWORD nTick = 0);

//portrait----------------------------------------------------------------------------------------------
	void DrawPortrait(int ID,int iSX,int iSY,int iEX,int iEY);
	CFcPortrait *GetPortrait()		{ return m_pPortrait; }
	//void GetImageInfo(int nPortraitID, ImageCtrlInfo* pInfo);

//report----------------------------------------------------------------------------------------------
	void SetReport(int nTextTableID);
	void SetItemReport(char* szText);

//mission----------------------------------------------------------------------------------------------
	void SetMissionGoal(int nTextTableID);
	bool IsPlayMissionGoal();
	void SetSpecialMissionGauge(int nValue, int nMaxValue, char* pCaption = NULL);	// gauge등
	void SetSpecialMissionText(char* szText);										// timer등 사용
	void ClearSpecialMissionGauge();
	void ClearSpecialMissionText();

//gauge----------------------------------------------------------------------------------------------
	void SetEnemyBossGauge(TroopObjHandle Handle, int nNameTextId);		// boss gauge에 사용
	void RemoveEnemyBossGauge(TroopObjHandle Handle);					// boss gauge에 사용
	void SetTroopGauge(TroopObjHandle Handle, int nNameTextId, bool bIncludeLeader = true);
	void RemoveTroopGauge(TroopObjHandle Handle);

	int GetMaxComboCount();
	void SetUnitGauge(int nUnitId, bool bOn);
	bool IsUnitGauge(int nUnitId);
	bool CanICheckUnitGauge();
	bool IsValidUnitGauge(int nUnitId, int &nIndex);
	void OnHitMsg( GameObjHandle &Handle );

//fade----------------------------------------------------------------------------------------------
	void SetFadeEffect(bool bFadeIn,int nSec,D3DXVECTOR3 *pColor = NULL);
	bool IsFadeInStop();
	bool IsFadeOutStop();
	bool IsFade();

//message----------------------------------------------------------------------------------------------
	bool IsMsgShowXYEnd();
	void SetMsgShowXY(int nX,int nY,int nSec,int nTextTableID);
	void DrawDebugText(int nPlayer,D3DXVECTOR3 *Pos,char *szText);

protected:
	void CreateObject(CFcInterfaceObj *pObject,int &nHandle);
	int  GetEmptyIndex();
	void DeleteObject(int &nHandle);

	void OnInputKey();
	void SetShowGameInterface(bool bShow);
	
protected:
	int				m_MenuHandle;

	int				m_MinimapHandle;

	int				m_SpeechHandle;
	int				m_ReportHandle;
	int				m_ItemReportHandle;
	int				m_MissionGoalHandle;
	int				m_SpecialText;
	int				m_SpecialGauge;
	int				m_AbilityHandle;
	int				m_LetterBoxHandle;
	int				m_GaugeHandle;
    int             m_FadeEffectHandle;
	int				m_MsgShowHandle;
	
	CFcPortrait		 *m_pPortrait;
	//FcCommonInterface m_CommonInterface;

	CFcInterfaceObj *m_InterfaceList[MAX_INTERFACE_OBJ];
	
	//int				m_PlayerCount;
	bool			m_bShowInterface;
	bool			m_bForceShowInterface;

	bool			m_bLetterBox;

	// Pre-load menu items - these are used in throughout the game so
	// pre-load them at start-up
	int				m_nTexMMS;
	int				m_nTexGHPD;
	int				m_nTexGHPB;
	int				m_nTexGOML;
	int				m_nTexGTOML;
	int				m_nTexGHP;
	int				m_nTexGO;
	int				m_nTexGOM;
	int				m_nTexGTO;
	int				m_nTexGTOM;
	int				m_nTexG;
	int				m_nTexGT;
	int				m_nSkin;
	int				m_nAni;
	int				m_nMeshHP;
	int				m_nMeshGO;
	int				m_nMeshGTO;

	std::vector<int>	m_UnitGaugeList;
};


extern FcInterfaceManager g_InterfaceManager;
#define g_MenuHandle g_InterfaceManager.GetMenuMgr()




#endif
