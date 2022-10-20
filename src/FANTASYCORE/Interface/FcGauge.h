#ifndef __FC_GAUGE_H__
#define __FC_GAUGE_H__
#include "FcInterfaceObj.h"
#include "FCInterfaceDataDefine.h"
#include "FcTroopManager.h"
#include "FcMenu3DObject.h"

enum GAUGETYPE
{
	GAUGETYPE_NON = -1,
	GAUGETYPE_BOSS,
	GAUGETYPE_TROOP,
	GAUGETYPE_TROOP_EXCEPT_LEADER,
};

struct GaugeInfo{
	GaugeInfo()
	{
		Type = GAUGETYPE_NON;
		nNameTextId = -1;
		fFar = 0.f;
	};

	GAUGETYPE		Type;

	TroopObjHandle  Handle;
	int				nNameTextId;
	float			fFar;
};

#define _MAX_COUNT_UNIT			9
enum SOUND_COMBO_TYPE
{
	SCT_COMBO,
	SCT_KILL,
};

//enum CFcGameObject::GameObj_ClassIDGameObj_ClassID;

struct SOUND_COMBO_INFO
{
	int nCount;
	int nPlayerType;
	SOUND_COMBO_TYPE nType;
    char *szSoundName0;
	char *szSoundName1;
	char *szSoundName2;
	char *szSoundName3;
};


class CFcGauge : public CFcInterfaceObj
{
public:
	CFcGauge();
	~CFcGauge();
	
	void Process();
	void Update();
	
	void SetGauge(TroopObjHandle Handle, int nNameTextId, GAUGETYPE Type);
	void RemoveGauge(TroopObjHandle Handle);

	void OnHitMsg( GameObjHandle &Handle );

	int GetMaxComboCount()	{ return m_nMaxComboCount; }

protected:
	void ReleaseData();
	void ProcessUnitTick();

	void DrawObjectGaugeforTroopList();
	void DrawObjectGaugeforTroop(TroopObjHandle hTroop);
	void DrawObjectGauge(GameObjHandle hUnit, float fRate);

	//2D limit kill effect
	void ProcessLimitKillEffect();
	void DrawLimitKillEffect();

	//2D limit combo effect
	void ProcessComboCount();
	void DrawComboCount();
	void DrawLimitComboEffect();
	
	//2D level up
	void DrawLevelUpEffect();
	
	void DrawPlayerName();
	void ProcessEnemyBossGauge();
	void DrawEnemyBossGauge();
	void DrawFirstEnemyBossGauge(int nHP, int nMaxHP, char* strName);
	void DrawSubEnemyBossGauge(int nCount, int nHP, int nMaxHP, char* strName);
	void DrawFriendlyCauge(int nHP, int nMaxHP, char* strName);

	void DrawKillCount();
	void DrawMaxComboCount();
	void DrawOrbs();

	void LoadPlayerNameTexture();

	int DrawSpecialCountText(int x1, int y1, int x2, int y2,
		int align, int nSpace,
		float fScaleX, float fScaleY,
		float fAlpha,
		int nSrcCount, int nPrevCount,
		char* szSpecial, int* pUnitTick);
	
	//-------------------------------------
	//fx kill effect
	void CreateKillObjRTT();
	void ReleaseKillObjRTT();
	
	void ProcessLimitKillFXEffect();
	void DrawLimitKillFXEffect();

	//-------------------------------------
	//fx combo effect
	void CreateComboObjRTT();
	void ReleaseComboObjRTT();

	void ProcessLimitComboFXEffect();
	void DrawLimitComboFXEffect();	

	//-------------------------------------
	//mesh
	void LoadMeshGauge();
	void ReleaseMeshGauge();

	void DrawMeshGaugeBase();
	void DrawMeshHpGauge();
	void DrawMeshOrbGauge();
	void DrawMeshTrueOrbGauge();

	//-------------------------------------
	void LoadKillComboTexture();
	void PlayComboSound(int nCount,int nHeroClass,SOUND_COMBO_TYPE nType );
	
protected:
	int			m_HPGaugeLen;

	int			m_nKillCount;
	int			m_nPrevKillCount;
	int			m_nKillTick;

	int			m_nComboCount;
	int			m_nPrevComboCount;
	int			m_nComboTick;
	int			m_nMaxComboCount;
	int			m_nPrevMaxComboCount;
	int			m_nMaxComboTick;
	int			m_nMaxComboUnitTick[_MAX_COUNT_UNIT];

	int			m_nComboShock;
	int			m_nComboShockUVId;
	int			m_nComboShockTexID;
	int			m_nComboUnitTick[_MAX_COUNT_UNIT];

	int			m_nKillEffectTexID;
	int			m_nKillwEffectTexID;
	int			m_nKillEffectObjID;
	int			m_nKillEffectTick;
	int			m_nKillEffectUnit;
	int			m_nKillUnitTick[_MAX_COUNT_UNIT];

	std::vector<GaugeInfo> m_BossGaugeList;
	
	int			m_nCurLevel;
	int			m_nLevelEffectTick;

	CFcMenu3DObjManager*	m_pFcMenuKillObj;
	DWORD					m_hKillRTTHandle;
	int						m_nKillShock;

	CFcMenu3DObjManager*	m_pFcMenuComboObj;
	DWORD					m_hComboRTTHandle;

	int			m_GaugeMeshIndex;
	int			m_OrbGaugeMeshIndex;
	int			m_TrueOrbGaugeMeshIndex;

	int			m_nGaugeTexId;
	int			m_nDamageGaugeTexId;
	int			m_nGaugeBaseTexId;
	int			m_nOrbAttackTexId;
	int			m_nOrbAttackFullTexId;
	int			m_nOrbSparkTexId;
	int			m_nOrbSpartFullTexId;

	int			m_nOrbAttackLightTexId;
	int 		m_nOrbSpartLightTexId;

	int			m_nDamageHp;
	float		m_fHpGaugeFrame;
	float		m_fOrbAttackFrame;
	float		m_fOrbSparkFrame;

	int			m_nPlayerNameTexId;

	int			m_nLevelUpTexId;

	int			m_nTexKill;
	int			m_nTexKills;
	int			m_nTexCombo;
	int			m_nTexMaxCombo;
};
#endif
