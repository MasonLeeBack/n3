#include "stdafx.h"
#include "FcGauge.h"
#include "BSKernel.h"
#include "FcInterfaceManager.h"
#include "FcWorld.h"
#include "FcHeroObject.h"
#include "FcTroopObject.h"
#include "TextTable.h"

#include "BsUiUVManager.h"
#include ".\\data\\Menu\\BsUiUVID.h"

#include "FcCameraObject.h"
#include "FcGlobal.h"

#include "BsOffScreenMgr.h"
#include "LocalLanguage.h"
#include "FcSoundManager.h"

#include "FcUtil.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG

#define _DEFAULT_HP_GAUGE_LEN	500
#define _ADD_HP_GAUGE_LEN		50
#define _GAUGE_LENGTH			600
#define _GAUGE_SCALE_X			0.018f //0.015f
#define _GAUGE_SCALE_Y			0.022f //0.015f
#define _GAUGE_TEX_UV_HALF		0.5f

#define _HP_GAUGE_FRAME			0.5f
#define _ORB_ATTACK_FRAME		2.f
#define _ORB_SPARK_FRAME		2.f

#define _KILL_COUNT_POS_X		1054
#define _KILL_COUNT_POS_Y		626
#define _KILL_TICK_MAX		5

#define _MAX_COMBO_POS_X		996
#define _MAX_COMBO_POS_Y		510

#define _COMBO_MAIN_POS_X		928
#define _COMBO_MAIN_POS_Y		462
#define _COMBO_TICK_MAX			10
#define _COMBO_LINE_POS_Y		496


#define _BOSS_GAUGE_MAX_LENGTH	450
#define _BOSS_MAX_HP			2000
#define _TROOP_MAX_HP			10000

#define _PLAYER_NAME_POS_X		-36 + 50
#define _PLAYER_NAME_POS_Y		566
#define _IMG_NAME_ASPHARR		"Name_Aspharr"
#define _IMG_NAME_INPHYY		"Name_Inphyy"
#define _IMG_NAME_MYIFEE		"Name_Myifee"
#define _IMG_NAME_TYURRU		"Name_Tyurru"
#define _IMG_NAME_KLARRANN		"Name_Klarrann"
#define _IMG_NAME_DWINGVATT		"Name_Dwingvatt"
#define _IMG_NAME_VIGKVAGK		"Name_Vigkvagk"

static int g_nGaugeX = 190;
static int g_nGaugeY = 596;

static int orbAtX = 116 + 50;
static int orbAtY = 587;

static int orbSpX = 136 + 50;
static int orbSpY = 581;

static float g_fScaleX = 0.019f;
static float g_fScaleY = 0.019f;




//#define _FX_KILL_COUNT
#define _FX_KILL_RTT_WIDTH		550
#define _FX_KILL_RTT_HEIGHT		350

//#define _FX_COMBO_COUNT
#define _FX_COMBO_RTT_WIDTH		550
#define _FX_COMBO_RTT_HEIGHT	350

#define MAX_COMBO_KAKEGOE		36

const SOUND_COMBO_INFO g_ComboSoundInfo[MAX_COMBO_KAKEGOE] = 
{
	//KILL COUNT
	{500,CHAR_ID_INPHYY,SCT_KILL,"I_U50_01","I_U50_02","I_U50_03","I_U50_04"},
	{1000,CHAR_ID_INPHYY,SCT_KILL,"I_U100_01","I_U100_02","I_U200_01","I_U200_02"},
	{2000,CHAR_ID_INPHYY,SCT_KILL,"I_U300_01","I_U300_02","I_U400_01","I_U400_02"},
	{3000,CHAR_ID_INPHYY,SCT_KILL,"I_U500_01","I_U500_02","I_U600_01","I_U600_02"},
	{4000,CHAR_ID_INPHYY,SCT_KILL,"I_U700_01","I_U700_02","I_U800_01","I_U800_02"},
	{5000,CHAR_ID_INPHYY,SCT_KILL,"I_U900_01","I_U1000_01",NULL,NULL},

	{500,CHAR_ID_ASPHARR,SCT_KILL,"A_U50_01","A_U50_02","A_U50_03","A_U50_04"},
	{1000,CHAR_ID_ASPHARR,SCT_KILL,"A_U100_01","A_U100_02","A_U200_01","A_U200_02"},
	{2000,CHAR_ID_ASPHARR,SCT_KILL,"A_U300_01","A_U300_02","A_U400_01","A_U400_02"},
	{3000,CHAR_ID_ASPHARR,SCT_KILL,"A_U500_01","A_U500_02","A_U600_01","A_U600_02"},
	{4000,CHAR_ID_ASPHARR,SCT_KILL,"A_U700_01","A_U700_02","A_U800_01","A_U800_02"},
	{5000,CHAR_ID_ASPHARR,SCT_KILL,"A_U900_01","A_U900_02","A_U1000_01","A_U1000_02"},

	{500,CHAR_ID_MYIFEE,SCT_KILL,"M_U50_01","M_U50_02","M_U50_03","M_U100_01"},
	{1000,CHAR_ID_MYIFEE,SCT_KILL,"M_U100_02","M_U100_03","M_U200_01","M_U200_02"},
	{2000,CHAR_ID_MYIFEE,SCT_KILL,"M_U300_01","M_U300_02","M_U400_01","M_U400_02"},
	{3000,CHAR_ID_MYIFEE,SCT_KILL,"M_U500_01","M_U500_02","M_U600_01","M_U600_02"},
	{4000,CHAR_ID_MYIFEE,SCT_KILL,"M_U700_01","M_U700_02","M_U800_01","M_U800_02"},
	{5000,CHAR_ID_MYIFEE,SCT_KILL,"M_U900_01","M_U900_02","M_U1000_01","M_U1000_02"},

	{500,CHAR_ID_TYURRU,SCT_KILL,"T_U50_01","T_U50_02","T_U50_03",NULL},
	{1000,CHAR_ID_TYURRU,SCT_KILL,"T_U100_01","T_U100_02","T_U200_01","T_U200_02"},
	{2000,CHAR_ID_TYURRU,SCT_KILL,"R_U300_01","T_U400_01","T_U400_02",NULL},
	{3000,CHAR_ID_TYURRU,SCT_KILL,"R_U500_01","T_U600_01","T_U600_02",NULL},
	{4000,CHAR_ID_TYURRU,SCT_KILL,"T_U700_01","T_U700_02","T_U800_01",NULL},
	{5000,CHAR_ID_TYURRU,SCT_KILL,"T_U1000_01","T_U1000_02","T_U1000_03",NULL},

	{500,CHAR_ID_KLARRANN,SCT_KILL,"K_U50_01","K_U50_02","K_U50_03","K_U100_01"},
	{1000,CHAR_ID_KLARRANN,SCT_KILL,"K_U100_02","K_U200_01","K_U200_02","K_U300_01"},
	{2000,CHAR_ID_KLARRANN,SCT_KILL,"K_U300_02","K_U400_01","K_U400_02","K_U500_01"},
	{3000,CHAR_ID_KLARRANN,SCT_KILL,"K_U500_02","K_U600_01","K_U600_02","K_U700_01"},
	{4000,CHAR_ID_KLARRANN,SCT_KILL,"K_U700_02","K_U800_01","K_U800_02","K_U900_01"},
	{5000,CHAR_ID_KLARRANN,SCT_KILL,"K_U900_02","K_U1000_01","K_U1000_02","K_U1000_03"},

	{500,CHAR_ID_DWINGVATT,SCT_KILL,"D_U50_01","D_U50_02","D_U50_03","D_U100_01"},
	{1000,CHAR_ID_DWINGVATT,SCT_KILL,"D_U100_02","D_U200_01","D_U200_02","D_U300_01"},
	{2000,CHAR_ID_DWINGVATT,SCT_KILL,"D_U300_02","D_U400_01","D_U400_02","D_U500_01"},
	{3000,CHAR_ID_DWINGVATT,SCT_KILL,"D_U500_02","D_U600_01","D_U600_02","D_U700_01"},
	{4000,CHAR_ID_DWINGVATT,SCT_KILL,"D_U700_02","D_U800_01","D_U800_02","D_U900_01"},
	{5000,CHAR_ID_DWINGVATT,SCT_KILL,"D_U900_02","D_U1000_01","D_U1000_02","D_U1000_03"},
};


CFcGauge::CFcGauge()
{
	m_bShow = true;

	m_HPGaugeLen = _DEFAULT_HP_GAUGE_LEN;

	m_nKillCount = 0;
	m_nPrevKillCount = 0;
	m_nKillTick = 0;

	m_nComboCount = 0;
	m_nPrevComboCount = 0;
	m_nComboTick = 0;
	m_nMaxComboCount = 0;
	m_nPrevMaxComboCount = 0;
	m_nMaxComboTick = 0;

	m_nComboShock = 0;
	m_nComboShockUVId = -1;
	m_nComboShockTexID = -1;
	
	m_nKillEffectTick = 0;
	m_nKillEffectUnit = 0;
    
	m_nKillShock = 0;
	m_nKillEffectTexID = -1;
	m_nKillwEffectTexID = -1;
	m_nKillEffectObjID = -1;

	m_nPlayerNameTexId = -1;
		
	m_nGaugeTexId = -1;
	m_nDamageGaugeTexId = -1;
	m_nGaugeBaseTexId = -1;
	m_nOrbAttackTexId = -1;
	m_nOrbAttackFullTexId = -1;
	m_nOrbSparkTexId = -1;
	m_nOrbSpartFullTexId = -1;
	m_nOrbAttackLightTexId = -1;
	m_nOrbSpartLightTexId = -1;
	m_nLevelUpTexId = -1;

    m_nTexKill = -1;
	m_nTexKills = -1;
	m_nTexCombo = -1;
	m_nTexMaxCombo = -1;

	HeroObjHandle Handle = g_FcWorld.GetHeroHandle();
	m_nCurLevel = Handle->GetLevel() + 1;
	m_nLevelEffectTick = 0;
	m_nDamageHp = Handle->GetHP();
	m_fHpGaugeFrame = 0.f;
	m_fOrbAttackFrame = 0.f;
	m_fOrbSparkFrame = 0.f;

	m_pFcMenuKillObj = NULL;
	m_hKillRTTHandle = NULL;

	m_pFcMenuComboObj = NULL;
	m_hComboRTTHandle = NULL;

	memset(m_nMaxComboUnitTick, 0, sizeof(int)* _MAX_COUNT_UNIT);
	memset(m_nComboUnitTick, 0, sizeof(int)* _MAX_COUNT_UNIT);
	memset(m_nKillUnitTick, 0, sizeof(int)* _MAX_COUNT_UNIT);
	
	LoadKillComboTexture();
	LoadMeshGauge();
	//LoadPlayerNameTexture();
}

CFcGauge::~CFcGauge()
{
#ifdef _FX_KILL_COUNT
	ReleaseKillObjRTT();
#endif

#ifdef _FX_COMBO_COUNT
	ReleaseComboObjRTT();
#endif

	ReleaseData();
}

void CFcGauge::ReleaseData()
{
	ReleaseMeshGauge();
	SAFE_RELEASE_TEXTURE(m_nPlayerNameTexId);
	SAFE_RELEASE_TEXTURE(m_nTexKill);
	SAFE_RELEASE_TEXTURE(m_nTexKills);
	SAFE_RELEASE_TEXTURE(m_nTexCombo);
	SAFE_RELEASE_TEXTURE(m_nTexMaxCombo);
	SAFE_RELEASE_TEXTURE(m_nLevelUpTexId);

	SAFE_RELEASE_TEXTURE(m_nComboShockTexID);
	SAFE_RELEASE_TEXTURE(m_nKillEffectTexID);
	SAFE_RELEASE_TEXTURE(m_nKillwEffectTexID);
}


void CFcGauge::Process()
{
	if(m_bShow == false){
		return;
	}
	ProcessUnitTick();
		
	ProcessEnemyBossGauge();

#ifdef _FX_COMBO_COUNT
	ProcessLimitComboFXEffect();
#else
	ProcessComboCount();
#endif //_FX_COMBO_COUNT
	
#ifdef _FX_KILL_COUNT
	ProcessLimitKillFXEffect();
#else
	ProcessLimitKillEffect();
#endif //_FX_KILL_COUNT
}

void CFcGauge::Update()
{
	if(m_bShow == false){
		return;
	}

	if(g_FCGameData.bShowOrbs == true){	
		DrawOrbs();
	}

	//--------------------------------
	DrawObjectGaugeforTroopList();
		
	//main gauge--------------------------------
	DrawMeshGaugeBase();
	DrawMeshTrueOrbGauge();
	DrawMeshOrbGauge();
	DrawMeshHpGauge();
	DrawPlayerName();
	
	//boss gauge-------------------------------------
	DrawEnemyBossGauge();

	//combo----------------------------------------
	if(g_InterfaceManager.IsSpeechPlay() == false){
		DrawMaxComboCount();
	}

	DrawComboCount();

	//kill----------------------------------------
	if(g_InterfaceManager.IsSpeechPlay() == false){
		DrawKillCount();	
	}

	//combo effect----------------------------------------
#ifdef _FX_COMBO_COUNT
	DrawLimitComboFXEffect();
#else
	DrawLimitComboEffect();
#endif //_FX_COMBO_COUNT

	//kill effect----------------------------------------
#ifdef _FX_KILL_COUNT
	DrawLimitKillFXEffect();
#else
	DrawLimitKillEffect();
#endif //_FX_KILL_COUNT

	//level up----------------------------------------
	DrawLevelUpEffect();

}


void CFcGauge::LoadMeshGauge()
{
	g_BsKernel.chdir("Interface");

	m_GaugeMeshIndex = g_BsKernel.LoadMesh(-1, "Gauge_Hp.BM");
	m_OrbGaugeMeshIndex = g_BsKernel.LoadMesh(-1, "Gauge_Orb.BM");
	m_TrueOrbGaugeMeshIndex = g_BsKernel.LoadMesh(-1, "Gauge_TrueOrb.BM");
	
	m_nDamageGaugeTexId		= g_BsKernel.LoadTexture("Gauge_Hp_Damage.dds");
	m_nGaugeBaseTexId		= g_BsKernel.LoadTexture("Gauge_Hp_base.dds");

	m_nOrbAttackLightTexId	= g_BsKernel.LoadTexture("Gauge_Orb_Max_Light.dds");
	m_nOrbSpartLightTexId	= g_BsKernel.LoadTexture("Gauge_TrueOrb_Max_Light.dds");
	
	m_nGaugeTexId			= g_BsKernel.LoadVolumeTexture("Gauge_Hp.dds");
	m_nOrbAttackTexId		= g_BsKernel.LoadVolumeTexture("Gauge_Orb.dds");
	m_nOrbAttackFullTexId	= g_BsKernel.LoadVolumeTexture("Gauge_Orb_Max.dds");
	m_nOrbSparkTexId		= g_BsKernel.LoadVolumeTexture("Gauge_TrueOrb.dds");
	m_nOrbSpartFullTexId	= g_BsKernel.LoadVolumeTexture("Gauge_TrueOrb_Max.dds");

	g_BsKernel.chdir("..");
}

void CFcGauge::ReleaseMeshGauge()
{
	//mesh
	if(m_GaugeMeshIndex != -1)
	{
		g_BsKernel.ReleaseMesh(m_GaugeMeshIndex);
		m_GaugeMeshIndex = -1;
	}

	if(m_OrbGaugeMeshIndex != -1)
	{
		g_BsKernel.ReleaseMesh(m_OrbGaugeMeshIndex);
		m_OrbGaugeMeshIndex = -1;
	}

	if(m_TrueOrbGaugeMeshIndex != -1)
	{
		g_BsKernel.ReleaseMesh(m_TrueOrbGaugeMeshIndex);
		m_TrueOrbGaugeMeshIndex = -1;
	}

	//texture
	SAFE_RELEASE_TEXTURE(m_nDamageGaugeTexId);
	SAFE_RELEASE_TEXTURE(m_nGaugeBaseTexId);

	SAFE_RELEASE_TEXTURE(m_nGaugeTexId);	
	SAFE_RELEASE_TEXTURE(m_nOrbAttackTexId);
	SAFE_RELEASE_TEXTURE(m_nOrbAttackFullTexId);
	SAFE_RELEASE_TEXTURE(m_nOrbSparkTexId);	
	SAFE_RELEASE_TEXTURE(m_nOrbSpartFullTexId);

	SAFE_RELEASE_TEXTURE(m_nOrbAttackLightTexId);
	SAFE_RELEASE_TEXTURE(m_nOrbSpartLightTexId);
}


static int g_nbasegaugeX = 119;
static int g_nbasegaugeY = 530;
void CFcGauge::DrawMeshGaugeBase()
{
	UVImage* pImg = g_UVMgr.GetUVImage(_UV_PlayerGauge_bg);

	BsAssert(pImg != NULL);
	g_BsKernel.DrawUIBox_s(_Ui_Mode_Image,
		g_nbasegaugeX, g_nbasegaugeY,
		(pImg->u2 - pImg->u1) * 2, (pImg->v2 - pImg->v1) * 2,
		0.f, D3DXCOLOR(1, 1, 1, 1),
		0.f, pImg->nTexId,
		pImg->u1, pImg->v1, pImg->u2, pImg->v2);
	
}

void CFcGauge::DrawMeshTrueOrbGauge()
{
	BsAssert(m_TrueOrbGaugeMeshIndex != -1);
	HeroObjHandle Handle = g_FcWorld.GetHeroHandle();
	BsAssert(Handle);

	float fOrbSpark = Handle->GetTrueOrbSpark() / (float)Handle->GetMaxTrueOrbSpark() ;
	fOrbSpark = (fOrbSpark / 2.f) + _GAUGE_TEX_UV_HALF;

	int nTexId = -1;
	float fFrame = 0.f;
	if(fOrbSpark == 1.f)
	{
		m_fOrbSparkFrame += _ORB_ATTACK_FRAME;
		if(m_fOrbSparkFrame > 16.f){
			m_fOrbSparkFrame = 0.f;
		}

		nTexId = m_nOrbSpartFullTexId;
		fFrame = m_fOrbSparkFrame/16.f;
	}
	else
	{
		if(Handle->IsOrbSparkOn() == false)
		{
			m_fOrbSparkFrame += _ORB_ATTACK_FRAME;
			if(m_fOrbSparkFrame > 32.f){
				m_fOrbSparkFrame = 0.f;
			}
		}
		else
		{
			m_fOrbSparkFrame -= _ORB_ATTACK_FRAME;
			if(m_fOrbSparkFrame < 0.f){
				m_fOrbSparkFrame = 32.f;
			}
		}

		nTexId = m_nOrbSparkTexId;
		fFrame = m_fOrbSparkFrame/32.f;
	}

	if(fOrbSpark == 1.f)
	{
		if(g_nProcessTick%15 == 0)
		{
			SIZE size = g_BsKernel.GetTextureSize(m_nOrbSpartLightTexId);
			g_BsKernel.DrawUIBox_s(_Ui_Mode_Image,
				orbSpX, orbSpY, size.cx, size.cy,
				0.f, D3DXCOLOR(1, 1, 1, 1),
				0.f, m_nOrbSpartLightTexId,
				0, 0, size.cx, size.cy);
		}
	}
	
	//True orb gauge
	g_BsKernel.DrawUIMesh(_Ui_Mode_Mesh_Gauge,
		m_TrueOrbGaugeMeshIndex, g_nGaugeX, g_nGaugeY,
		D3DXCOLOR(1, 1, 1, 1),
		m_nGaugeBaseTexId,
		g_fScaleX, g_fScaleY, 0.f);

	if(fOrbSpark > 0.5f)
	{
		g_BsKernel.DrawUIMesh(_Ui_Mode_Mesh_Gauge_Volumn,
			m_TrueOrbGaugeMeshIndex, g_nGaugeX, g_nGaugeY-2,
			D3DXCOLOR(1, 1, 1, 1),
			nTexId,
			g_fScaleX, g_fScaleY, fOrbSpark, fFrame);
	}
}


void CFcGauge::DrawMeshOrbGauge()
{
	BsAssert(m_OrbGaugeMeshIndex != -1);
	HeroObjHandle Handle = g_FcWorld.GetHeroHandle();
	BsAssert(Handle);

	float fOrbAttack = 0.f;
	if( Handle->GetOrbSparkFrame() > 0.f ) {
		fOrbAttack = Handle->GetOrbSparkFrame() / (float)Handle->CalcMaxOrbSpark();
	}
	else {
		fOrbAttack = Handle->GetOrbSpark() / (float)Handle->CalcMaxOrbSpark();
	}
	fOrbAttack = (fOrbAttack / 2.f) + _GAUGE_TEX_UV_HALF;
		
	int nTexId = -1;
	float fFrame = 0.f;
	if(fOrbAttack == 1.f)
	{
		m_fOrbAttackFrame += _ORB_ATTACK_FRAME;
		if(m_fOrbAttackFrame > 16.f){
			m_fOrbAttackFrame = 0.f;
		}

		nTexId = m_nOrbAttackFullTexId;
		fFrame = m_fOrbAttackFrame/16.f;
	}
	else
	{
		if(Handle->IsOrbSparkOn() == false)
		{
			m_fOrbAttackFrame += _ORB_ATTACK_FRAME;
			if(m_fOrbAttackFrame > 32.f){
				m_fOrbAttackFrame = 0.f;
			}
		}
		else
		{
			m_fOrbAttackFrame -= _ORB_ATTACK_FRAME;
			if(m_fOrbAttackFrame < 0.f){
				m_fOrbAttackFrame = 32.f;
			}
		}

		nTexId = m_nOrbAttackTexId;
		fFrame = m_fOrbAttackFrame/32.f;
	}

	if(fOrbAttack == 1.f)
	{	
		if(g_nProcessTick%15 == 0)
		{
			SIZE size = g_BsKernel.GetTextureSize(m_nOrbAttackLightTexId);
			g_BsKernel.DrawUIBox_s(_Ui_Mode_Image,
				orbAtX, orbAtY, size.cx, size.cy,
				0.f, D3DXCOLOR(1, 1, 1, 1),
				0.f, m_nOrbAttackLightTexId,
				0, 0, size.cx, size.cy);
		}
	}

	//orb gauge
	g_BsKernel.DrawUIMesh(_Ui_Mode_Mesh_Gauge,
		m_OrbGaugeMeshIndex, g_nGaugeX, g_nGaugeY,
		D3DXCOLOR(1, 1, 1, 1),
		m_nGaugeBaseTexId,
		g_fScaleX, g_fScaleY, 0.f);

	if(fOrbAttack > 0.5f)
	{
		g_BsKernel.DrawUIMesh(_Ui_Mode_Mesh_Gauge_Volumn,
			m_OrbGaugeMeshIndex, g_nGaugeX, g_nGaugeY-2,
			D3DXCOLOR(1, 1, 1, 1),
			nTexId,
			g_fScaleX, g_fScaleY, fOrbAttack, fFrame);
	}
}


static int g_nHp = 450;
static int g_nMaxHp = 450;
static int g_nDamageHp = g_nHp;
static float g_fAddBase = 0.005f;

void CFcGauge::DrawMeshHpGauge()
{
	BsAssert(m_GaugeMeshIndex != -1);

	m_fHpGaugeFrame += _HP_GAUGE_FRAME;
	if(m_fHpGaugeFrame > 32.f){
		m_fHpGaugeFrame = 0.f;
	}

	HeroObjHandle Handle = g_FcWorld.GetHeroHandle();
	int nHp = Handle->GetHP();
	int nMaxHp = Handle->GetMaxHP();
	int nDamageHpCount = ((m_nDamageHp-1) / _GAUGE_LENGTH) + 1;
	int nHpCount = (nHp / _GAUGE_LENGTH) + 1;
	int nMaxHpCount = (nMaxHp / _GAUGE_LENGTH) + 1;
	int nDamageHp = m_nDamageHp;

/*
	int nHp = g_nHp;
	int nMaxHp = g_nMaxHp;
	int nDamageHp = g_nDamageHp;
	int nDamageHpCount = ((g_nDamageHp-1) / _GAUGE_LENGTH) + 1;
	int nHpCount = (nHp / _GAUGE_LENGTH) + 1;
	int nMaxHpCount = (nMaxHp / _GAUGE_LENGTH) + 1;
*/
	for(int i=0; i<nDamageHpCount; i++)
	{
		float fBase(1.f), fDamageGauge(1.f);
		D3DXCOLOR color = D3DXCOLOR(1, 1, 1, 1);
		if(i == nDamageHpCount-1)
		{
			if(nDamageHpCount == nMaxHpCount){
				fBase = ((float)(nMaxHp % _GAUGE_LENGTH) / _GAUGE_LENGTH / 2.f) + _GAUGE_TEX_UV_HALF + g_fAddBase;
			}

			fDamageGauge = ((float)(nDamageHp % _GAUGE_LENGTH) / _GAUGE_LENGTH / 2.f) + _GAUGE_TEX_UV_HALF;
		}

		int nGap = i * 8;

		//hp gauge
		if(fBase > 1.f){
			fBase = 1.f;
		}

		g_BsKernel.DrawUIMesh(_Ui_Mode_Mesh_Gauge,
			m_GaugeMeshIndex, g_nGaugeX, g_nGaugeY - nGap,
			D3DXCOLOR(1, 1, 1, 1),
			m_nGaugeBaseTexId,
			g_fScaleX, g_fScaleY, fBase);

		//demage Gauge
		if(fDamageGauge > 0.5f){
			g_BsKernel.DrawUIMesh(_Ui_Mode_Mesh_Gauge,
				m_GaugeMeshIndex, g_nGaugeX, g_nGaugeY - (2 + nGap),
				color,
				m_nDamageGaugeTexId,
				g_fScaleX, g_fScaleY, fDamageGauge);
		}
		
		if(i <= nHpCount-1)
		{
			float fGauge(1.f);
			if(i == nHpCount-1){
				fGauge = ((float)(nHp % _GAUGE_LENGTH) / _GAUGE_LENGTH / 2.f) + _GAUGE_TEX_UV_HALF;
			}

			if(fGauge > 0.5f)
			{
				g_BsKernel.DrawUIMesh(_Ui_Mode_Mesh_Gauge_Volumn,
					m_GaugeMeshIndex, g_nGaugeX, g_nGaugeY - (2 + nGap),
					color,
					m_nGaugeTexId,
					g_fScaleX, g_fScaleY, fGauge, m_fHpGaugeFrame/32.f);
			}
		}
	}

	if(m_nDamageHp > nHp){
		m_nDamageHp -=2;
	}

	if(m_nDamageHp < nHp){
		m_nDamageHp = nHp;
	}
/*
	if(g_nDamageHp > nHp){
		g_nDamageHp -=2;
	}

	if(g_nDamageHp < nHp){
		g_nDamageHp = nHp;
	}
*/
}

#define _KILL_EFFECT_TICK			65
void CFcGauge::ProcessLimitKillEffect()
{
	if(m_nKillEffectTexID != -1)
	{
		if(m_nKillEffectTick > 0){
			m_nKillEffectTick--;
		}
		else{
			SAFE_RELEASE_TEXTURE(m_nKillEffectTexID);
			SAFE_RELEASE_TEXTURE(m_nKillwEffectTexID);
		}
		return;
	}

	HeroObjHandle Handle = g_FcWorld.GetHeroHandle();
	BsAssert(Handle != NULL);
	
	int nKillCount = Handle->GetKillCount();
	if(m_nKillEffectUnit < nKillCount)
	{
		for(int i=0; i<_KILL_EFFECT_COUNT; i++)
		{
			if(m_nKillEffectUnit < g_KillEffectUnit[i] && nKillCount >= g_KillEffectUnit[i])
			{
				m_nKillEffectUnit = g_KillEffectUnit[i];

#ifdef _TEST_KILL_COUNT_FX_FILENAME
				PlayComboSound( g_KillEffectUnit[i]*50, g_FcWorld.GetHeroHandle()->GetUnitSoxIndex(), SCT_KILL);
				char cKillText[256];
				sprintf(cKillText, "%s\\gm_kill_%d_%s.dds",
					g_LocalLanguage.GetLanguageDir(),
					(m_nKillEffectUnit*50),
					g_LocalLanguage.GetLanguageStr());

				char cKillwText[256];
				sprintf(cKillwText, "%s\\gm_kill_%d_w_%s.dds",
					g_LocalLanguage.GetLanguageDir(),
					m_nKillEffectUnit*50,
					g_LocalLanguage.GetLanguageStr());
#else
				PlayComboSound(g_KillEffectUnit[i], g_FcWorld.GetHeroHandle()->GetUnitSoxIndex(), SCT_KILL);
				char cKillText[256];
				sprintf(cKillText, "%s\\gm_kill_%d_%s.dds",
					g_LocalLanguage.GetLanguageDir(),
					m_nKillEffectUnit,
					g_LocalLanguage.GetLanguageStr());

				char cKillwText[256];
				sprintf(cKillwText, "%s\\gm_kill_%d_w_%s.dds",
					g_LocalLanguage.GetLanguageDir(),
					m_nKillEffectUnit,
					g_LocalLanguage.GetLanguageStr());
#endif

				g_BsKernel.chdir("interface");
				g_BsKernel.chdir("Local");
				m_nKillEffectTexID = g_BsKernel.LoadTexture(cKillText);
				BsAssert(m_nKillEffectTexID != -1);
				m_nKillwEffectTexID = g_BsKernel.LoadTexture(cKillwText);
				BsAssert(m_nKillwEffectTexID != -1);
				g_BsKernel.chdir("..");
				g_BsKernel.chdir("..");

				m_nKillEffectTick = _KILL_EFFECT_TICK;

				break;
			}
		}
		
		if(m_nKillEffectTexID == -1){
			return;
		}
	}
}

static int g_rate = 100;
#define _KILL_EFFECT_TEX_SIZE_X		1024
#define _KILL_EFFECT_TEX_SIZE_Y		128

#define _KILL_EFFECT_TICK_STEP_1	_KILL_EFFECT_TICK - 6
#define _KILL_EFFECT_TICK_STEP_2	_KILL_EFFECT_TICK_STEP_1 - 6
#define _KILL_EFFECT_TICK_STEP_3	_KILL_EFFECT_TICK_STEP_2 - 6
#define _KILL_EFFECT_TICK_STEP_4	_KILL_EFFECT_TICK - 55
void CFcGauge::DrawLimitKillEffect()
{
	if(m_nKillEffectTick == 0){
		return;
	}
		
	if (m_nKillEffectTexID == -1) {
		return;
	}

	if(m_nKillEffectTick > _KILL_EFFECT_TICK_STEP_1)
	{
		int nStep1 = _KILL_EFFECT_TICK_STEP_1;
		int nRate = ((m_nKillEffectTick + 1) - nStep1) / 2;
		float fsizeX = _KILL_EFFECT_TEX_SIZE_X * nRate * 1.f;
		float fsizeY = _KILL_EFFECT_TEX_SIZE_Y * nRate * 1.f;
		
		float fx = (_SCREEN_WIDTH/2) - (fsizeX/2);
		float fy = (_SCREEN_HEIGHT/2) - (fsizeY/2);

		g_BsKernel.DrawUIBox_s(_Ui_Mode_Image,
			(int)fx, (int)fy, (int)fsizeX, (int)fsizeY,
			0.f, D3DXCOLOR(1.f, 1.f, 1.f, 1.f),
			0.f, m_nKillwEffectTexID,
			0, 0, _KILL_EFFECT_TEX_SIZE_X, _KILL_EFFECT_TEX_SIZE_Y);
	}
	else if(m_nKillEffectTick > _KILL_EFFECT_TICK_STEP_2)
	{
		float fsizeX = _KILL_EFFECT_TEX_SIZE_X;
		float fsizeY = _KILL_EFFECT_TEX_SIZE_Y;

		float fx = (_SCREEN_WIDTH/2) - (fsizeX/2);
		float fy = (_SCREEN_HEIGHT/2) - (fsizeY/2);

		for(int i=0; i<3; i++)
		{
			g_BsKernel.DrawUIBox_s(_Ui_Mode_Image,
				(int)fx, (int)fy, (int)fsizeX, (int)fsizeY,
				0.f, D3DXCOLOR(1.f, 1.f, 1.f, 1.f),
				0.f, m_nKillEffectTexID,
				0, 0, _KILL_EFFECT_TEX_SIZE_X, _KILL_EFFECT_TEX_SIZE_Y);
		}

		int nStep2 = _KILL_EFFECT_TICK_STEP_2;
		int nCount = ((m_nKillEffectTick + 1) - nStep2) / 2;
		for(int i=0; i<nCount; i++)
		{
			g_BsKernel.DrawUIBox_s(_Ui_Mode_Image,
				(int)fx, (int)fy, (int)fsizeX, (int)fsizeY,
				0.f, D3DXCOLOR(1.f, 1.f, 1.f, 1.f),
				0.f, m_nKillwEffectTexID,
				0, 0, _KILL_EFFECT_TEX_SIZE_X, _KILL_EFFECT_TEX_SIZE_Y);
		}
	}
	else if(m_nKillEffectTick > _KILL_EFFECT_TICK_STEP_3)
	{
		float fsizeX = _KILL_EFFECT_TEX_SIZE_X;
		float fsizeY = _KILL_EFFECT_TEX_SIZE_Y;

		float fx = (_SCREEN_WIDTH/2) - (fsizeX/2);
		float fy = (_SCREEN_HEIGHT/2) - (fsizeY/2);

		int nStep3 = _KILL_EFFECT_TICK_STEP_3;
		int nCount = ((m_nKillEffectTick + 1) - nStep3) / 2;
		for(int i=0; i<nCount; i++)
		{
			g_BsKernel.DrawUIBox_s(_Ui_Mode_Image,
				(int)fx, (int)fy, (int)fsizeX, (int)fsizeY,
				0.f, D3DXCOLOR(1.f, 1.f, 1.f, 1.f),
				0.f, m_nKillEffectTexID,
				0, 0, _KILL_EFFECT_TEX_SIZE_X, _KILL_EFFECT_TEX_SIZE_Y);
		}
	}
	else if(m_nKillEffectTick > _KILL_EFFECT_TICK_STEP_4)
	{
		float fsizeX = _KILL_EFFECT_TEX_SIZE_X;
		float fsizeY = _KILL_EFFECT_TEX_SIZE_Y;

		float fx = (_SCREEN_WIDTH/2) - (fsizeX/2);
		float fy = (_SCREEN_HEIGHT/2) - (fsizeY/2);

		g_BsKernel.DrawUIBox_s(_Ui_Mode_Image,
			(int)fx, (int)fy, (int)fsizeX, (int)fsizeY,
			0.f, D3DXCOLOR(1.f, 1.f, 1.f, 1.f),
			0.f, m_nKillEffectTexID,
			0, 0, _KILL_EFFECT_TEX_SIZE_X, _KILL_EFFECT_TEX_SIZE_Y);
	}
	else
	{
		float fsizeX = _KILL_EFFECT_TEX_SIZE_X;
		float fsizeY = _KILL_EFFECT_TEX_SIZE_Y;

		float fx = (_SCREEN_WIDTH/2) - (fsizeX/2);
		float fy = (_SCREEN_HEIGHT/2) - (fsizeY/2);
				
		int nStep3 = _KILL_EFFECT_TICK_STEP_4;
		float fValue = (float)(nStep3 - m_nKillEffectTick);
		float fAlpha = 1.f - (fValue / nStep3);
		g_BsKernel.DrawUIBox_s(_Ui_Mode_Image,
			(int)fx, (int)fy, (int)fsizeX, (int)fsizeY,
			0.f, D3DXCOLOR(1.f, 1.f, 1.f, fAlpha),
			0.f, m_nKillEffectTexID,
			0, 0, _KILL_EFFECT_TEX_SIZE_X, _KILL_EFFECT_TEX_SIZE_Y);
	}
}


TroopObjHandle g_DebugTestGuageTroop;

#define _TROOP_GAUGE_MAX_LEN	25000000.f//5000.f * 5000.f 제곱으로 연산
#define _TROOP_GAUGE_MIN_LEN	10000.f//100.f * 100.f
void CFcGauge::DrawObjectGaugeforTroopList()
{
	if( g_InterfaceManager.GetInstance().CanICheckUnitGauge() == false )
		return;

	CameraObjHandle CamHandle = CFcBaseObject::GetCameraObjectHandle(0);
	if( CamHandle == NULL ){
		return;
	}

	CFcTroopManager* pTroopManager = g_FcWorld.GetTroopmanager();
	for( int i=0; i<pTroopManager->GetTroopCount(); i++ )
	{
		TroopObjHandle hTroop = pTroopManager->GetTroop( i );
		if( !hTroop->IsEnable() ){
			continue;
		}

		D3DXVECTOR3 Distance = hTroop->GetPos() - CamHandle->GetPos();
		float fLength = D3DXVec3LengthSq(&Distance);
		if(fLength < _TROOP_GAUGE_MIN_LEN){
			continue;
		}

		float fMaxlen = (float)g_FCGameData.nShowTroopGaugeForFar;
		fMaxlen *= fMaxlen;
		float fFar = fMaxlen * 3.f / 4.f;
		if(fLength > fFar){
			continue;
		}
		
		bool bEnemy = true;
		if(hTroop->GetTeam() == 0){
			bEnemy = false;
		}
		
g_DebugTestGuageTroop = hTroop;

		if(hTroop->GetLeader() != NULL)
		{
			int nUnitSoxId = hTroop->GetLeader()->GetUnitSoxIndex();
			//Trigger에서 그리라 하는 gauge, 아래 조건 무시...
			if(g_InterfaceManager.GetInstance().IsUnitGauge(nUnitSoxId))
			{
				DrawObjectGaugeforTroop(hTroop);
				continue;
			}
		}

		// our force troop 
		if(bEnemy == false && g_FCGameData.bShowFriendlyGauge == false){
			continue;
		}
		// enemy troop
		else if(bEnemy == true && g_FCGameData.bShowEnemyGauge == false){
			continue;
		}

		DrawObjectGaugeforTroop(hTroop);
	}
}

void CFcGauge::DrawObjectGaugeforTroop(TroopObjHandle hTroop)
{	
	for( int i=0; i<hTroop->GetUnitCount(); i++ )
	{
		GameObjHandle hUnit = hTroop->GetUnit(i);
		if(hUnit == NULL){
			continue;
		}

		if( CFcBaseObject::IsValid( hUnit ) == false ) {
			hUnit.Identity();
		}

		if(hUnit->IsEnable() == false){
			continue;
		}
		
		if(hUnit->IsDie() == true){
			continue;
		}

		CameraObjHandle CamHandle = CFcBaseObject::GetCameraObjectHandle(0);
		D3DXVECTOR3 Distance = hUnit->GetPos() - CamHandle->GetPos();
		float fLength = D3DXVec3LengthSq(&Distance);
		if(fLength < _TROOP_GAUGE_MIN_LEN){
			continue;
		}

		float fMaxlen = (float)g_FCGameData.nShowTroopGaugeForFar;
		fMaxlen *= fMaxlen;
		float fFar = fMaxlen * 3.f / 4.f;
		if(fLength > fFar){
			continue;
		}

		float fRate = 1.f - (fLength / fMaxlen);
		DrawObjectGauge(hUnit, fRate);
	}
}


#define _TROOP_GAUGE_WIDTH	30
#define _TROOP_GAUGE_HEIGHT	6
void CFcGauge::DrawObjectGauge(GameObjHandle hUnit, float fRate)
{	
	if(hUnit->GetHP() == 0){
		return;
	}

	int nCamIndex = g_FcWorld.GetPlayerCamIndex(0);
	D3DXVECTOR3 Pos = hUnit->GetPos();
	Pos.y += hUnit->GetUnitHeight() + 20.f;

	D3DXVECTOR3 OutPut;
	g_BsKernel.GetProjectionPos(nCamIndex, &Pos, &OutPut);

	if(OutPut.x > 1.f || OutPut.x < -1.f){
		return;
	}
	
	if(OutPut.y > 1.f || OutPut.y < -1.f){
		return;
	}
	
	if(OutPut.z < 0.f){
		return;
	}
	
	float fScreenWidth = float(g_BsKernel.GetDevice()->GetBackBufferWidth());
	float fScreenHeight = float(g_BsKernel.GetDevice()->GetBackBufferHeight());

	D3DXVECTOR3 OutPutVec;
	OutPutVec.x = (1.f+OutPut.x) *0.5f * fScreenWidth;
	OutPutVec.y = -(-1.f+OutPut.y) *0.5f * fScreenHeight;
	OutPutVec.z = OutPut.z;

	
	int nWidth = (int)(_TROOP_GAUGE_WIDTH * fRate);
	int nHeight = (int)(_TROOP_GAUGE_HEIGHT * fRate);
	POINT pos = { (int)OutPutVec.x - nWidth/2, (int)OutPutVec.y - nHeight/2};

	float fRateHP = (float)(hUnit->GetHP()) / hUnit->GetMaxHP();
	int nGuage = (int)(fRateHP*nWidth);

	float fAlpha = 1.f * fRate;
	fAlpha = (fAlpha > 1.f) ? 1.f : fAlpha;
	D3DXCOLOR color = D3DXCOLOR(0,1.f,0,fAlpha);
	bool bEnemy = (hUnit->GetTeam() == 0) ? false : true;
	color = (bEnemy == true) ? D3DXCOLOR(1.f,0,0,fAlpha) : color;
	
	//max hp
	g_BsKernel.DrawUIBox_s(_Ui_Mode_Box_ZTEST,
		pos.x-1, pos.y-1,
		nWidth+2, nHeight+2,
		OutPutVec.z,
		D3DXCOLOR(0,0,0,fAlpha));

	//hp
	g_BsKernel.DrawUIBox_s(_Ui_Mode_Box_ZTEST,
		pos.x, pos.y,
		nGuage, nHeight,
		OutPutVec.z,
		color);

}


static int LevelX = 104 + 50;
static int LevelY = 530;
static int LevelCountX = 76 + 50;
static int LevelCountY = 526;

static int PlayerNamePosX = 250;
static int PlayerNamePosY = 560;
static int PlayerNameID = 6;


void CFcGauge::DrawPlayerName()
{
	//level
	g_pFont->DrawUIText(0, PlayerNamePosY, PlayerNamePosX, -1, "@(reset)@(scale=0.8,0.8)@(space=-4)@(align=2)");

	HeroObjHandle Handle = g_FcWorld.GetHeroHandle();
	int nUnitID = Handle->GetUnitSoxIndex();

	int nTextId = -1;
	switch(nUnitID)
	{
	case CHAR_ID_ASPHARR:	nTextId = _TEX_Aspharr; break;
	case CHAR_ID_INPHYY:	nTextId = _TEX_Inphyy; break;
	case CHAR_ID_TYURRU:	nTextId = _TEX_Tyurru; break;
	case CHAR_ID_KLARRANN:	nTextId = _TEX_Klarrann; break;
	case CHAR_ID_MYIFEE:	nTextId = _TEX_Myifee; break;
	case CHAR_ID_VIGKVAGK:	nTextId = _TEX_Vigkvagk; break;
	case CHAR_ID_DWINGVATT:	nTextId = _TEX_Dwingvatt; break;
	default: BsAssert(0);
	}

	char cName[256];
	g_TextTable->GetText(nTextId, cName, _countof(cName));
	g_pFont->DrawUIText(0, PlayerNamePosY, PlayerNamePosX, -1, cName);

	//level
	char szLevel[256];
	g_TextTable->GetText(_TEX_SUM_LEVEL, szLevel, _countof(szLevel));

	char szFullLevel[256];
	sprintf(szFullLevel, "@(reset)@(align=2)@(scale=0.8,0.8)@(space=-2)%s %d", szLevel, m_nCurLevel);
	g_pFont->DrawUIText(0, LevelY, LevelX, -1, szFullLevel);


	//---------------------------------------------------------------
	//갈 수 없는 지역에 갔을때 머리 위에 갈 수 없다는 Icon이 뜬다.
	if(g_FcWorld.IsHeroEventBlockArea() == true)
	{
		CameraObjHandle CamHandle = CFcBaseObject::GetCameraObjectHandle( 0 );
		D3DXVECTOR3 CamPos = CamHandle->GetPos();
		D3DXVECTOR3 Pos = Handle->GetPos();

		D3DXVECTOR3 Dir = Pos - CamPos;
		float fDist = D3DXVec3Length( &Dir );
		if( fDist < 6000.f )
		{
			Pos.y += Handle->GetUnitHeight() + 20.f;
			g_BsKernel.SendMessage( g_FcWorld.GetPlayerCamIndex(0), BS_REFRESH_CAMERA );

			char cWarning[256];
			sprintf(cWarning, "@(reset)@(Nopass)");

			int nTempWidth(0), nHeight(0), nLine(0), nWordCount(0);
			g_pFont->GetTextLengthInfo(nTempWidth, nHeight, nLine, nWordCount, 0, 0, -1, -1, cWarning);
			Pos = CamHandle->GetCrossVector()->m_XVector * (float)(-nTempWidth/2) + Pos;
			g_InterfaceManager.DrawDebugText( 0, &Pos, cWarning );
		}
	}
}

#define _SCALE_UP_100			100
#define _SCALE_UP_500			500
void CFcGauge::DrawKillCount()
{	
	HeroObjHandle Handle = g_FcWorld.GetHeroHandle();
	if(Handle)
	{
		int nKillCount = Handle->GetKillCount();
		if(m_nKillCount != nKillCount)
		{
			m_nPrevKillCount = m_nKillCount;
			m_nKillCount = nKillCount;
			m_nKillTick = _KILL_TICK_MAX;
		}
	}

	int nPer(0), nTempY(0), nSpace(0);
	float fScale(0.f);
	if(m_nKillCount < _SCALE_UP_100)
	{
		nPer = 6; nTempY = 4;
		
		nSpace = -8;
		fScale = 0.6f;
	}
	else if(m_nKillCount < _SCALE_UP_500)
	{
		nPer = 8; nTempY = 0;
		
		nSpace = -10;
		fScale = 0.8f;
	}
	else
	{	
		nPer = 10; nTempY = -4;

		nSpace = -12;
		fScale = 1.f;
	}
		
	//kill count -----------------------------------
	DrawSpecialCountText(0, _KILL_COUNT_POS_Y, _KILL_COUNT_POS_X - 10, -1,
		2, nSpace, fScale, fScale, 1.f,
		m_nKillCount, m_nPrevKillCount,
		"@(kill_%c)", m_nKillUnitTick);
	m_nPrevKillCount = m_nKillCount;



	//kill Text image -----------------------------------
	if(m_nKillCount <= 1)
	{
		g_BsKernel.DrawUIBox_s(_Ui_Mode_Image,
			_KILL_COUNT_POS_X, _KILL_COUNT_POS_Y - 4 + nTempY,
			(int)(256.f * nPer * 0.1f), (int)(64.f * nPer * 0.1f),
			0.f, D3DXCOLOR(1, 1, 1, 1),
			0.f, m_nTexKill,
			0, 0, 256, 64);
	}
	else{
		g_BsKernel.DrawUIBox_s(_Ui_Mode_Image,
			_KILL_COUNT_POS_X, _KILL_COUNT_POS_Y - 4 + nTempY,
			(int)(256.f * nPer * 0.1f), (int)(64.f * nPer * 0.1f),
			0.f, D3DXCOLOR(1, 1, 1, 1),
			0.f, m_nTexKills,
			0, 0, 256, 64);
	}

	g_pFont->DrawUIText(0, 0, 0, 0, "@(reset)");
}

void CFcGauge::PlayComboSound(int nCount,int nPlayerType,SOUND_COMBO_TYPE nType )
{
	std::vector<char *>vtSoundCues;
	for(int i = 0;i < MAX_COMBO_KAKEGOE;i++)
	{
		if( g_ComboSoundInfo[i].nCount != nCount)
			continue;
		if( g_ComboSoundInfo[i].nPlayerType != nPlayerType )
			continue;
		if( g_ComboSoundInfo[i].nType != nType )
			continue;

		if(g_ComboSoundInfo[i].szSoundName0){ vtSoundCues.push_back(g_ComboSoundInfo[i].szSoundName0); }
		if(g_ComboSoundInfo[i].szSoundName1){ vtSoundCues.push_back(g_ComboSoundInfo[i].szSoundName1); }
		if(g_ComboSoundInfo[i].szSoundName2){ vtSoundCues.push_back(g_ComboSoundInfo[i].szSoundName2); }
		if(g_ComboSoundInfo[i].szSoundName3){ vtSoundCues.push_back(g_ComboSoundInfo[i].szSoundName3); }		

		if( g_pSoundManager && vtSoundCues.size() ){
			int nIndex = Random(vtSoundCues.size());
			if(nType == SCT_COMBO){			
				//g_pSoundManager->PlaySystemSound( SB_COMMON,"SYS_KILL" );
			}
			else
			{
				g_pSoundManager->PlaySystemSound( SB_COMMON,"SYS_KILL" );
			}			
			DebugString( "*************Kakegoe %s**********************\n",vtSoundCues[nIndex] );
			g_pSoundManager->PlaySystemSound( SB_ALL_KAKEGOE, vtSoundCues[nIndex] );
			break;
		}

	}
}


void CFcGauge::ProcessComboCount()
{
	HeroObjHandle Handle = g_FcWorld.GetHeroHandle();
	BsAssert(Handle);
	
	int nComboCount = Handle->GetComboCount();
	if(nComboCount == 0)
	{
		if(m_nComboTick > 0){
			m_nComboTick--;
		}
		else
		{
			if(m_nComboShock > 0){
				m_nComboShock--;
			}

			m_nComboCount = 0;
			m_nPrevComboCount = 0;
			if(m_nComboShock == 0){
				m_nComboShockUVId = -1;
			}
			return;
		}
	}
	else if(nComboCount == 1){
		return;
	}
	else if(m_nComboTick < _COMBO_TICK_MAX){
		m_nComboTick++;
	}
	
	m_nPrevComboCount = m_nComboCount;
	m_nComboCount = nComboCount;

	if(m_nComboShock > 0){
		m_nComboShock--;
	}
	else if(m_nComboShock == 0)
	{
		SAFE_RELEASE_TEXTURE(m_nComboShockTexID);

		for(int i=0; i<_COMBO_EFFECT_COUNT; i++)
		{
			if(m_nComboShockUVId < i && m_nComboCount > g_ComboEffectUnit[i])
			{	
				m_nComboShockUVId = i;
				m_nComboShock = 40;
				
				char cTempText[256];
#ifdef _TEST_COMBO_COUNT_FX_FILENAME
				int ntemp = g_ComboEffectUnit[i] * 50;
				sprintf(cTempText, "%s\\gm_com_%d_%s.dds",
					g_LocalLanguage.GetLanguageDir(),
					ntemp,
					g_LocalLanguage.GetLanguageStr());
#else
				sprintf(cTempText, "%s\\gm_com_%d_%s.dds",
					g_LocalLanguage.GetLanguageDir(),
					g_ComboEffectUnit[i],
					g_LocalLanguage.GetLanguageStr());
#endif
				g_BsKernel.chdir("interface");
				g_BsKernel.chdir("Local");
				m_nComboShockTexID = g_BsKernel.LoadTexture(cTempText);
				BsAssert(m_nComboShockTexID != -1);
				g_BsKernel.chdir("..");
				g_BsKernel.chdir("..");
				
				break;
			}
		}
	}
	
}

void CFcGauge::DrawComboCount()
{
	if(m_nComboCount <= 1){
		return;
	}

	//combo line ------------------------------------
	//숫자 단위 계산(기본 1단위)
	char szSrc[32];
	sprintf(szSrc, "%d", m_nComboCount);
	int nAddUnit = strlen(szSrc);

	//(nAddUnit * '숫자중에 젤루 width가 큰 5') + 'combo특수문자간의 거리(10)' 
	// + 'combo count앞의 line길이(10)' + 'combo특수문자의 시작위치'
	int nLineStartPosX = _COMBO_MAIN_POS_X - ((nAddUnit * 38) + 10 + 10);
	int nAddLineWidth = _SCREEN_WIDTH - nLineStartPosX;
	int nComboLinePosX = _SCREEN_WIDTH - (int)(nAddLineWidth * ((float)m_nComboTick / _COMBO_TICK_MAX));
/*
	UVImage* pTmpImg = NULL;
	pTmpImg = g_UVMgr.GetUVImage(_UV_gm_combo_left);
	g_BsKernel.DrawUIBox(_Ui_Mode_Image, 
		nComboLinePosX, _COMBO_LINE_POS_Y, nComboLinePosX+6, _COMBO_LINE_POS_Y+6, 
		0.f ,D3DXCOLOR(1.f, 1.f, 1.f, 1.f),
		0.f, pTmpImg->nTexId,
		pTmpImg->u1, pTmpImg->v1, pTmpImg->u2, pTmpImg->v2);

	pTmpImg = g_UVMgr.GetUVImage(_UV_gm_combo_center);
	g_BsKernel.DrawUIBox(_Ui_Mode_Image, 
		nComboLinePosX+6, _COMBO_LINE_POS_Y, _SCREEN_WIDTH, _COMBO_LINE_POS_Y+6, 
		0.f ,D3DXCOLOR(1.f, 1.f, 1.f, 1.f),
		0.f, pTmpImg->nTexId,
		pTmpImg->u1, pTmpImg->v1, pTmpImg->u2, pTmpImg->v2);*/

	//combo count -----------------------------------
	// 10 : 문자 combo로 부터의 간격
	int nAddComboCountWidth = _SCREEN_WIDTH - _COMBO_MAIN_POS_X - 20;
	int nComboCountPosX = _SCREEN_WIDTH - (int)(nAddComboCountWidth * ((float)m_nComboTick / _COMBO_TICK_MAX)) - 30;
	int nComboCountPosY = _COMBO_MAIN_POS_Y;

#ifdef _FX_COMBO_COUNT
	DrawSpecialCountText(0, nComboCountPosY, nComboCountPosX, -1,
		2, -4, 1.f, 1.f, 1.f,
		m_nComboCount, m_nPrevComboCount,
		"@(combo_%c)", m_nComboUnitTick);
#else
	if(m_nComboShock == 0)
	{
		DrawSpecialCountText(0, nComboCountPosY, nComboCountPosX, -1,
			2, -4, 1.f, 1.f, 1.f,
			m_nComboCount, m_nPrevComboCount,
			"@(combo_%c)", m_nComboUnitTick);
	}
#endif
	m_nPrevComboCount = m_nComboCount;

	//combo Text image -----------------------------------
	int nAddComboWidth = _SCREEN_WIDTH - _COMBO_MAIN_POS_X;
	int nComboPosX = _SCREEN_WIDTH - (int)(nAddComboWidth * ((float)m_nComboTick / _COMBO_TICK_MAX));
	int nComboPosY = _COMBO_MAIN_POS_Y;

#ifdef _FX_COMBO_COUNT
	g_BsKernel.DrawUIBox_s(_Ui_Mode_Image,
		nComboPosX, nComboPosY,
		256, 64,
		0.f, D3DXCOLOR(1, 1, 1, 1),
		0.f, m_nTexCombo,
		0, 0, 256, 64);
#else
	if(m_nComboShock == 0)
	{
		g_BsKernel.DrawUIBox_s(_Ui_Mode_Image,
			nComboPosX, nComboPosY,
			256, 64,
			0.f, D3DXCOLOR(1, 1, 1, 1),
			0.f, m_nTexCombo,
			0, 0, 256, 64);
	}
#endif
}


#define _COMBO_EFFECT_TEX_SIZE_X		512
#define _COMBO_EFFECT_TEX_SIZE_Y		64
void CFcGauge::DrawLimitComboEffect()
{
	if(m_nComboShock <= 0){
		return;
	}
	
	int sizeX = _COMBO_EFFECT_TEX_SIZE_X;
	int sizeY = _COMBO_EFFECT_TEX_SIZE_Y;
	int nCenterX = _SCREEN_WIDTH - 80 - (sizeX/2);
	int nCenterY = _COMBO_MAIN_POS_Y;

	if(m_nComboShock > 30)
	{
		float fScale = (40 - m_nComboShock -1) * 0.2f;
		sizeX = (int)(sizeX * fScale);
		sizeY = (int)(sizeY * fScale);

		int sizeX1 = (int)(sizeX * fScale * 2);
		int sizeY1 = (int)(sizeY * fScale * 2);
		int x1 = nCenterX - sizeX1 / 2;
		int y1 = nCenterY - sizeY1 / 2;
		int x2 = nCenterX + sizeX1 / 2;
		int y2 = nCenterY + sizeY1 / 2;

		g_BsKernel.DrawUIBox(_Ui_Mode_Image, 
			x1, y1, x2, y2,
			0.f ,D3DXCOLOR(1.f, 1.f, 1.f, 0.5f),
			0.f, m_nComboShockTexID,
			0, 0, _COMBO_EFFECT_TEX_SIZE_X, _COMBO_EFFECT_TEX_SIZE_Y);
	}
	else if(m_nComboShock > 15){
		;
	}
	else
	{
		int nAddValue = 15 - m_nComboShock;
		sizeX += nAddValue * 2;
		sizeY -= nAddValue * 4;
	}

	int x1 = nCenterX - sizeX / 2;
	int y1 = nCenterY - sizeY / 2;
	int x2 = nCenterX + sizeX / 2;
	int y2 = nCenterY + sizeY / 2;

	g_BsKernel.DrawUIBox(_Ui_Mode_Image, 
		x1, y1, x2, y2,
		0.f ,D3DXCOLOR(1.f, 1.f, 1.f, 1.f),
		0.f, m_nComboShockTexID,
		0, 0, _COMBO_EFFECT_TEX_SIZE_X, _COMBO_EFFECT_TEX_SIZE_Y);
}

void CFcGauge::DrawMaxComboCount()
{	
	if(m_nMaxComboCount < m_nComboCount)
	{
		m_nPrevMaxComboCount = m_nMaxComboCount;
		m_nMaxComboCount = m_nComboCount;
	}

	float fAlpha(0.f);
	if(m_nComboCount == 0){
		fAlpha = 0.5f;
	}
	else{
		fAlpha = 1.f;
	}

	//Max combo count ------------------------------------------
	DrawSpecialCountText(0, _MAX_COMBO_POS_Y, _MAX_COMBO_POS_X - 10, -1,
		2, -4, 1.f, 1.f, fAlpha,
		m_nMaxComboCount, m_nPrevMaxComboCount,
		"@(combo_%c)", m_nMaxComboUnitTick);
	m_nPrevMaxComboCount = m_nMaxComboCount;
	
	//Max combo Text Image ------------------------------------------
	g_BsKernel.DrawUIBox_s(_Ui_Mode_Image,
		_MAX_COMBO_POS_X, _MAX_COMBO_POS_Y,
		256, 64,
		0.f, D3DXCOLOR(1, 1, 1, fAlpha),
		0.f, m_nTexMaxCombo,
		0, 0, 256, 64);
}


static int g_nUnitTickGap = 2;
static float g_fUnitTickGap = 0.5f;
int CFcGauge::DrawSpecialCountText(int x1, int y1, int x2, int y2,
								   int align, int nSpace,
								   float fScaleX, float fScaleY,
								   float fAlpha,
								   int nSrcCount, int nPrevCount,
								   char* szSpecial, int* pUnitTick)
{
	//default font attribute
	char cDefaultAttr[256];
	sprintf(cDefaultAttr, "@(reset)@(align=%d)@(space=%d)@(color=255,255,255,%d)",
		align, nSpace, (int)(fAlpha * 255));
	g_pFont->DrawUIText(x1, y1, x2, y2, cDefaultAttr);

	//숫자 단위 계산(기본 1단위)
	char cSrcUnit[256], cPrevUnit[256];
	sprintf(cSrcUnit, "%9d", nSrcCount);
	sprintf(cPrevUnit, "%9d", nPrevCount);
	
	int nAddUnit(0);
	for(int i=0; i<_MAX_COUNT_UNIT; i++)
	{
		if(cSrcUnit[i] == ' '){
			continue;
		}

		nAddUnit++;

		if(pUnitTick == NULL){
			continue;
		}
		
		if(pUnitTick[i] == 0 && cSrcUnit[i] != cPrevUnit[i]){
			pUnitTick[i] = g_nUnitTickGap;
		}
	}

	char cAttr[256];
	strcpy(cAttr, "@(scale=%1.1f,%1.1f)");

	int nlen = ((strlen(cAttr) - 2) + (strlen(szSpecial) - 1)) * nAddUnit;
	char* szSpecialCount = new char[nlen+1];
	memset(szSpecialCount, 0, nlen);

	for(int i=0; i<_MAX_COUNT_UNIT; i++)
	{
		if(cSrcUnit[i] == ' '){
			continue;
		}

		int nUnitTick = 0;
		if(pUnitTick != NULL){
			nUnitTick = pUnitTick[i];
		}

		char ctempAttr[32];
		sprintf(ctempAttr, cAttr, fScaleX, fScaleY + (nUnitTick * g_fUnitTickGap));
		strcat_s(szSpecialCount, nlen+1, ctempAttr);

		char szTemp[32];
		sprintf(szTemp, szSpecial, cSrcUnit[i]);
		strcat_s(szSpecialCount, nlen+1, szTemp);
	}
	
	int nWidth(0), nHeight(0), nLine(0);
	int nWordCount(0);
	g_pFont->GetTextLengthInfo(nWidth, nHeight, nLine, nWordCount, 0, 0, -1, -1, szSpecialCount);
	g_pFont->DrawUIText(x1, y1, x2, y2, szSpecialCount);

	delete [] szSpecialCount;

	return nWidth;
}

#define _ORB_COUNT_POS_X	336
#define _ORB_COUNT_POS_Y	114
void CFcGauge::DrawOrbs()
{
	HeroObjHandle Handle = g_FcWorld.GetHeroHandle();
	int nOrbExp = Handle->GetEXP();

	//kill count -----------------------------------
	int nWidth = DrawSpecialCountText(_ORB_COUNT_POS_X, _ORB_COUNT_POS_Y, _SCREEN_WIDTH, -1,
		0, -3, 0.8f, 0.8f, 1.f,
		nOrbExp, nOrbExp,
		"@(orb_%c)", NULL);

	//kill Text image -----------------------------------
	g_pFont->DrawUIText(_ORB_COUNT_POS_X + nWidth + 10, _ORB_COUNT_POS_Y,
		_SCREEN_WIDTH, -1, "@(reset)@(scale=1.2,1.2)@(orb_orbs)");
	
/*
	char cOrbCount[256];
	sprintf(cOrbCount, "@(reset)@(align=0)Orbs  %d", nOrbExp);

	g_pFont->DrawUIText(346, 116, _SCREEN_WIDTH, _SCREEN_HEIGHT, cOrbCount);
	*/
}


#define _LEVEL_UP_SIZE_X		512
#define _LEVEL_UP_SIZE_Y		128
void CFcGauge::DrawLevelUpEffect()
{
	HeroObjHandle Handle = g_FcWorld.GetHeroHandle();
	int nCurLevel = Handle->GetLevel() + 1;
	if(m_nCurLevel < nCurLevel)
	{
		m_nCurLevel = nCurLevel;
		m_nLevelEffectTick = 40;
		m_HPGaugeLen += _ADD_HP_GAUGE_LEN;
	
		//level에 대응되는 Hero의 skill이 있다면 Report를 찍는다.
		int nUnitSoxID = Handle->GetUnitSoxIndex();
		if(g_NewSkillforLevel[nUnitSoxID][m_nCurLevel - 1]){
			g_InterfaceManager.SetReport(_TEX_REPORT_ADD_SKILL);
		}
	}

	if(m_nLevelEffectTick == 0){
		return;
	}
	
	int sizeX = _LEVEL_UP_SIZE_X;
	int sizeY = _LEVEL_UP_SIZE_Y;
	int nCenterX = _SCREEN_WIDTH / 2;
	int nCenterY = _SCREEN_HEIGHT / 2;

	if(m_nLevelEffectTick > 30)
	{
		float fScale = (40 - m_nLevelEffectTick -1) * 0.2f;
		sizeX = (int)(sizeX * fScale);
		sizeY = (int)(sizeY * fScale);

		int sizeX1 = (int)(sizeX * fScale * 2);
		int sizeY1 = (int)(sizeY * fScale * 2);
		int x1 = nCenterX - sizeX1 / 2;
		int y1 = nCenterY - sizeY1 / 2;
		int x2 = nCenterX + sizeX1 / 2;
		int y2 = nCenterY + sizeY1 / 2;

		g_BsKernel.DrawUIBox(_Ui_Mode_Image, 
			x1, y1, x2, y2,
			0.f ,D3DXCOLOR(1.f, 1.f, 1.f, 0.5f),
			0.f, m_nLevelUpTexId,
			0, 0, _LEVEL_UP_SIZE_X, _LEVEL_UP_SIZE_Y);
	}
	else if(m_nComboShock > 15){
		;
	}
	else
	{
		int nAddValue = 15 - m_nLevelEffectTick;
		sizeX += nAddValue * 2;
		sizeY -= nAddValue * 4;
	}

	int x1 = nCenterX - sizeX / 2;
	int y1 = nCenterY - sizeY / 2;
	int x2 = nCenterX + sizeX / 2;
	int y2 = nCenterY + sizeY / 2;

	g_BsKernel.DrawUIBox(_Ui_Mode_Image, 
		x1, y1, x2, y2,
		0.f ,D3DXCOLOR(1.f, 1.f, 1.f, 1.f),
		0.f, m_nLevelUpTexId,
		0, 0, _LEVEL_UP_SIZE_X, _LEVEL_UP_SIZE_Y);

	m_nLevelEffectTick--;
}


void CFcGauge::SetGauge(TroopObjHandle Handle, int nNameTextId, GAUGETYPE Type)
{
	DWORD dwCount = m_BossGaugeList.size();
	if(dwCount > 0)
	{
		for(DWORD i=0; i<dwCount; i++)
		{
			if(m_BossGaugeList[i].Handle != Handle){
				continue;
			}

			return;
		}
	}
	GaugeInfo BossInfo;
	BossInfo.Type = Type;
	BossInfo.Handle = Handle;
	BossInfo.nNameTextId = nNameTextId;
	m_BossGaugeList.push_back(BossInfo);
	if( Handle->GetLeader() )
	{
		Handle->GetLeader()->SetCatchable( false );
		Handle->GetLeader()->SetBoss( true );
	}
	else
	{
		int nCnt = Handle->GetUnitCount();
		for( int i=0; i<nCnt; i++ )
		{
			GameObjHandle hUnit = Handle->GetUnit( i );
			if( hUnit == NULL )
				continue;

			hUnit->SetCatchable( false );
		}
	}
}

void CFcGauge::RemoveGauge(TroopObjHandle Handle)
{
	DWORD dwCount = m_BossGaugeList.size();
	if(dwCount == 0){
		return;
	}
	for(DWORD i=0; i<dwCount; i++)
	{
		if(m_BossGaugeList[i].Handle != Handle){
			continue;
		}
		m_BossGaugeList.erase(m_BossGaugeList.begin() + i);
		return;
	}
}



void CFcGauge::OnHitMsg( GameObjHandle &Handle )
{
	DWORD dwCount = m_BossGaugeList.size();
	if(dwCount == 0){
		return;
	}

	for(DWORD i=0; i<dwCount; i++)
	{
		if(m_BossGaugeList[i].Handle != Handle->GetTroop()){
			continue;
		}
		GaugeInfo BossInfo = m_BossGaugeList[i];
		m_BossGaugeList.erase(m_BossGaugeList.begin() + i);
		m_BossGaugeList.insert(m_BossGaugeList.begin(), BossInfo);
		return;
	}
}

struct TFn_CompareID
{
	bool operator() ( GaugeInfo& A, GaugeInfo& B)
	{
		return A.fFar <= B.fFar;
	}
};

void CFcGauge::ProcessEnemyBossGauge()
{
	std::vector<GaugeInfo>::iterator itr;
	for (itr=m_BossGaugeList.begin(); itr!=m_BossGaugeList.end();)
	{
		GaugeInfo* pBossInfo = &(*itr);

		switch(pBossInfo->Type)
		{
		case GAUGETYPE_BOSS:
			{
				bool bTroopEnable = pBossInfo->Handle->IsEnable();
				GameObjHandle hLeader = pBossInfo->Handle->GetLeader();
				if( hLeader == NULL )
					hLeader = pBossInfo->Handle->GetFirstLiveUnit();

				if(hLeader && bTroopEnable && hLeader->IsEnable() && hLeader->IsDie() == false)
				{
					itr++;
					continue;
				}
				break;
			}
		case GAUGETYPE_TROOP:
		case GAUGETYPE_TROOP_EXCEPT_LEADER:
			{
				bool bEnable = pBossInfo->Handle->IsEnable();
				bool bEliminated = pBossInfo->Handle->IsEliminated();

				if(bEnable == true && bEliminated == false)
				{
					itr++;
					continue;
				}
				break;
			}
		default: BsAssert(0);
		}
        
		itr = m_BossGaugeList.erase(itr);
	}

	DWORD dwCount = m_BossGaugeList.size();
	if(dwCount == 0){
		return;
	}

	HeroObjHandle hHero = g_FcWorld.GetHeroHandle();
	D3DXVECTOR3 vHeroPos = hHero->GetPos();

	for(DWORD i=0; i<dwCount; i++)
	{
		GaugeInfo* pBossInfo = &(m_BossGaugeList[i]);
		if( !pBossInfo->Handle ){
			continue;
		}

		GameObjHandle BossHandle = pBossInfo->Handle->GetLeader();
		if( CFcBaseObject::IsValid( BossHandle ) == NULL )
		{
			if(pBossInfo->Handle->GetUnitCount() == 0){
				continue;
			}

			if(CFcBaseObject::IsValid(pBossInfo->Handle->GetUnit( 0 )) == NULL){
				continue;
			}

			BossHandle = pBossInfo->Handle->GetUnit( 0 );
		}

		D3DXVECTOR3 vBossPos = BossHandle->GetPos();

		D3DXVECTOR3 Out;
		D3DXVec3Subtract(&Out, &vHeroPos, &vBossPos);
		pBossInfo->fFar = D3DXVec3Length(&Out);
	}

	std::sort(m_BossGaugeList.begin(),m_BossGaugeList.end(), TFn_CompareID() );
}

void CFcGauge::DrawEnemyBossGauge()
{
	DWORD dwCount = m_BossGaugeList.size();
	if(dwCount == 0){
		return;
	}

	HeroObjHandle hHero = g_FcWorld.GetHeroHandle();

	int nCount = 0;

	for(DWORD i=0; i<dwCount; i++)
	{
		GaugeInfo* pBossInfo = &(m_BossGaugeList[i]);
		if( !pBossInfo->Handle ){
			continue;
		}

		int nHP(0), nMaxHP(1);
		bool bEnemy = true;
		switch(pBossInfo->Type)
		{
		case GAUGETYPE_BOSS:
			{
				nHP = pBossInfo->Handle->GetBossHP();
				nMaxHP = pBossInfo->Handle->GetBossMaxHP();
				if(hHero->GetTeam() == pBossInfo->Handle->GetTeam()){
					bEnemy = false;
				}
				break;
			}
		case GAUGETYPE_TROOP:
			{
				nHP = pBossInfo->Handle->GetHP();
				nMaxHP = pBossInfo->Handle->GetMaxHP();
				if(hHero->GetTeam() == pBossInfo->Handle->GetTeam()){
					bEnemy = false;
				}
				break;
			}
		case GAUGETYPE_TROOP_EXCEPT_LEADER:
			{
				nHP = pBossInfo->Handle->GetHP();
				nMaxHP = pBossInfo->Handle->GetMaxHP();

				GameObjHandle hLeader = pBossInfo->Handle->GetLeader();
				if( hLeader )
				{
					nHP -= hLeader->GetHP();
					nMaxHP -= hLeader->GetMaxHP();
				}

				if(hHero->GetTeam() == pBossInfo->Handle->GetTeam()){
					bEnemy = false;
				}
			}
			break;
		default: BsAssert(0);
		}

		if(nHP == 0){
			continue;
		}

		char cPlayerName[256];
		g_TextTable->GetText(pBossInfo->nNameTextId, cPlayerName, _countof(cPlayerName));

		if( pBossInfo->Type == GAUGETYPE_BOSS )
		{
			GameObjHandle hUnit = pBossInfo->Handle->GetFirstLiveUnit();
			if( hUnit )
			{
				CameraObjHandle CamHandle = CFcBaseObject::GetCameraObjectHandle( 0 );
				D3DXVECTOR3 CamPos = CamHandle->GetPos();
				D3DXVECTOR3 Pos = hUnit->GetPos();

				D3DXVECTOR3 Dir = Pos - CamPos;
				float fDist = D3DXVec3Length( &Dir );
				if( fDist < 6000.f )
				{
					Pos.y += hUnit->GetUnitHeight() + 20.f;
					g_BsKernel.SendMessage( g_FcWorld.GetPlayerCamIndex(0), BS_REFRESH_CAMERA );

					char cName[256];
					sprintf(cName, "@(reset)@(scale=0.7,0.7)%s", cPlayerName );

					int nTempWidth, nHeight, nLine;
					int nWordCount(0);
					g_pFont->GetTextLengthInfo(nTempWidth, nHeight, nLine, nWordCount, 0, 0, -1, -1, cName);

//					Pos.x -= (float)(nTempWidth/2);
					Pos = CamHandle->GetCrossVector()->m_XVector * (float)(-nTempWidth/2) + Pos;
  					g_InterfaceManager.DrawDebugText( 0, &Pos, cName );
				}
			}
		}

		if(bEnemy == false)
		{
			DrawFriendlyCauge(nHP, nMaxHP, cPlayerName);
			continue;
		}
		
		if( nCount == 0)
		{
			DrawFirstEnemyBossGauge(nHP, nMaxHP, cPlayerName);
		}
		else
		{
			DrawSubEnemyBossGauge(nCount, nHP, nMaxHP, cPlayerName);
		}

		nCount++;
	}
}



#define _Boss_Gauge_LeftX			84
#define _Boss_Gauge_MiddleX			92
#define _Boss_Gauge_RightX			486
#define _Boss_Gauge_Y				52

#define _Boss_Gauge_BarX			89
#define _Boss_Gauge_BarY			57
#define _Boss_Gauge_Bar_LENGTH		400

#define _Boss_Gauge_NameX			90
#define _Boss_Gauge_NameY			72

void CFcGauge::DrawFirstEnemyBossGauge(int nHP, int nMaxHP, char* strName)
{
	//------------------------------------------------
	UVImage* pLeft = g_UVMgr.GetUVImage(_UV_Boss_front);
	g_BsKernel.DrawUIBox_s(_Ui_Mode_Image,
		_Boss_Gauge_LeftX, _Boss_Gauge_Y,
		pLeft->u2 - pLeft->u1, pLeft->v2 - pLeft->v1,
		0.f, D3DXCOLOR(1.f, 1.f, 1.f, 1),
		0.f, pLeft->nTexId,
		pLeft->u1, pLeft->v1, pLeft->u2, pLeft->v2);

	//------------------------------------------------
	UVImage* pMiddle = g_UVMgr.GetUVImage(_UV_Boss_middle);
	g_BsKernel.DrawUIBox_s(_Ui_Mode_Image,
		_Boss_Gauge_MiddleX, _Boss_Gauge_Y,
		_Boss_Gauge_Bar_LENGTH - 6, pMiddle->v2 - pMiddle->v1,
		0.f, D3DXCOLOR(1.f, 1.f, 1.f, 1),
		0.f, pMiddle->nTexId,
		pMiddle->u1, pMiddle->v1, pMiddle->u2, pMiddle->v2);

	//------------------------------------------------
	UVImage* pRight = g_UVMgr.GetUVImage(_UV_Boss_back);
	g_BsKernel.DrawUIBox_s(_Ui_Mode_Image,
		_Boss_Gauge_RightX, _Boss_Gauge_Y,
		pRight->u2 - pRight->u1, pRight->v2 - pRight->v1,
		0.f, D3DXCOLOR(1.f, 1.f, 1.f, 1),
		0.f, pRight->nTexId,
		pRight->u1, pRight->v1, pRight->u2, pRight->v2);

	//------------------------------------------------
	UVImage* pBar = g_UVMgr.GetUVImage(_UV_Boss_Bar);
	int nValue = (int)(nHP * _Boss_Gauge_Bar_LENGTH / nMaxHP);
	g_BsKernel.DrawUIBox_s(_Ui_Mode_Image,
		_Boss_Gauge_BarX, _Boss_Gauge_BarY,
		nValue, pBar->v2 - pBar->v1,
		0.f, D3DXCOLOR(1.f, 1.f, 1.f, 1),
		0.f, pBar->nTexId,
		pBar->u1, pBar->v1, pBar->u2, pBar->v2);

	//------------------------------------------------
	char cName[256];
	sprintf(cName, "@(reset)@(scale=0.7,0.7)%s", strName);
	g_pFont->DrawUIText(_Boss_Gauge_NameX, _Boss_Gauge_NameY, _SCREEN_WIDTH, -1, cName);
}


#define _Sub_Boss_Gauge_Bar_LEFTX	84 
#define _Sub_Boss_Gauge_Bar_MIDDLEX	92
#define _Sub_Boss_Gauge_Bar_RIGHTX	284
#define _Sub_Boss_Gauge_Bar_Y		52

#define _Sub_Boss_Gauge_Bar_BAR_X	88
#define _Sub_Boss_Gauge_Bar_BAR_Y	56
#define _Sub_Boss_Gauge_Bar_BAR_LEN	200

#define _Sub_Boss_Gauge_Bar_TEXT_X	295
#define _Sub_Boss_Gauge_Bar_TEXT_Y	45
void CFcGauge::DrawSubEnemyBossGauge(int nCount, int nHP, int nMaxHP, char* strName)
{	
	//y gap
	int nAddY = _Sub_Boss_Gauge_Bar_BAR_Y + (20 * (nCount-1));

	//------------------------------------------------
	UVImage* pLeft = g_UVMgr.GetUVImage(_UV_mission_front);
	g_BsKernel.DrawUIBox_s(_Ui_Mode_Image,
		_Sub_Boss_Gauge_Bar_LEFTX, _Sub_Boss_Gauge_Bar_Y + nAddY,
		pLeft->u2 - pLeft->u1, pLeft->v2 - pLeft->v1,
		0.f, D3DXCOLOR(1.f, 1.f, 1.f, 1),
		0.f, pLeft->nTexId,
		pLeft->u1, pLeft->v1, pLeft->u2, pLeft->v2);

	//------------------------------------------------
	UVImage* pMiddle = g_UVMgr.GetUVImage(_UV_mission_middle);
	g_BsKernel.DrawUIBox_s(_Ui_Mode_Image,
		_Sub_Boss_Gauge_Bar_MIDDLEX, _Sub_Boss_Gauge_Bar_Y + nAddY,
		_Sub_Boss_Gauge_Bar_BAR_LEN - 8, pMiddle->v2 - pMiddle->v1,
		0.f, D3DXCOLOR(1.f, 1.f, 1.f, 1),
		0.f, pMiddle->nTexId,
		pMiddle->u1, pMiddle->v1, pMiddle->u2, pMiddle->v2);

	//------------------------------------------------
	UVImage* pRight = g_UVMgr.GetUVImage(_UV_mission_back);
	g_BsKernel.DrawUIBox_s(_Ui_Mode_Image,
		_Sub_Boss_Gauge_Bar_RIGHTX, _Sub_Boss_Gauge_Bar_Y + nAddY,
		pRight->u2 - pRight->u1, pRight->v2 - pRight->v1,
		0.f, D3DXCOLOR(1.f, 1.f, 1.f, 1),
		0.f, pRight->nTexId,
		pRight->u1, pRight->v1, pRight->u2, pRight->v2);

	//------------------------------------------------
	UVImage* pBar = g_UVMgr.GetUVImage(_UV_Boss_Bar);
	int nValue = (int)(nHP * _Sub_Boss_Gauge_Bar_BAR_LEN / nMaxHP);
	g_BsKernel.DrawUIBox_s(_Ui_Mode_Image,
		_Sub_Boss_Gauge_Bar_BAR_X, _Sub_Boss_Gauge_Bar_BAR_Y + nAddY,
		nValue, (pBar->v2 - pBar->v1) /2,
		0.f, D3DXCOLOR(1.f, 1.f, 1.f, 1),
		0.f, pBar->nTexId,
		pBar->u1, pBar->v1, pBar->u2, pBar->v2);
	

	//------------------------------------------------
	char cName[256];
	sprintf(cName, "@(reset)@(scale=0.65,0.65)%s", strName);
	g_pFont->DrawUIText(_Sub_Boss_Gauge_Bar_TEXT_X, _Sub_Boss_Gauge_Bar_TEXT_Y + nAddY, _SCREEN_WIDTH, -1, cName);

}


void CFcGauge::CreateKillObjRTT()
{
	CFcMenu3DObjManager* pRTTexture = new CFcMenu3DObjManager();
	BsAssert(g_BsKernel.GetRTTManager() != NULL);
	m_hKillRTTHandle = g_BsKernel.GetRTTManager()->Create((CBsGenerateTexture*)pRTTexture,
		_FX_KILL_RTT_WIDTH, _FX_KILL_RTT_HEIGHT, 0.f, 0.f, 1.f, 1.f, true);

	m_pFcMenuKillObj = pRTTexture;

	CCrossVector* pCameraVector = m_pFcMenuKillObj->GetCameraCrossVector();
	pCameraVector->SetPosition(D3DXVECTOR3(0.f, 500.f, 0.f));
	m_pFcMenuKillObj->SetCameraTargetPos(D3DXVECTOR3(0.f, 0.f, 1.f));
}

void CFcGauge::ReleaseKillObjRTT()
{
	if(m_hKillRTTHandle != NULL)
	{
		BsAssert(g_BsKernel.GetRTTManager() != NULL);
		g_BsKernel.GetRTTManager()->Release(m_hKillRTTHandle);
		m_hKillRTTHandle = NULL;
		m_pFcMenuKillObj = NULL;
	}
}


void CFcGauge::ProcessLimitKillFXEffect()
{
	if(m_pFcMenuKillObj != NULL)
	{
		m_pFcMenuKillObj->Process();
		
		if(m_pFcMenuKillObj->GetObjectCount() != 0)
		{
			m_nKillShock--;
			return;
		}
		else
		{
			ReleaseKillObjRTT();
			m_nKillEffectObjID = -1;
			m_nKillShock = 0;
		}
	}

	HeroObjHandle Handle = g_FcWorld.GetHeroHandle();
	BsAssert(Handle != NULL);

	int nKillCount = Handle->GetKillCount();
	if(m_nKillEffectUnit < nKillCount)
	{
		for(int i=0; i<_KILL_EFFECT_COUNT; i++)
		{
			if(m_nKillEffectUnit < g_KillEffectUnit[i] && nKillCount >= g_KillEffectUnit[i])
			{	
				m_nKillEffectUnit = g_KillEffectUnit[i];

				if(m_pFcMenuKillObj == NULL){
					CreateKillObjRTT();
				}

				char cFxFileName[256];
#ifndef _TEST_KILL_COUNT_FX_FILENAME
				sprintf(cFxFileName, "%dKills.bfx", m_nKillEffectUnit);
#else
				strcpy(cFxFileName, "500Kills.bfx");
#endif //_TEST_KILL_COUNT_FX_FILENAME

				int nFxTemplateIndex = m_pFcMenuKillObj->LoadFXTemplate(cFxFileName);
				if(nFxTemplateIndex == -1)
				{
					BsAssert(nFxTemplateIndex != -1);
					return;
				}

				D3DXVECTOR3 pos = D3DXVECTOR3(0, 0, 0);
				m_nKillEffectObjID = m_pFcMenuKillObj->AddFXObject(nFxTemplateIndex, pos, 1.f, false);
				m_nKillShock = 40;

#ifdef _TEST_KILL_COUNT_FX_FILENAME
				PlayComboSound(g_KillEffectUnit[i]*50, g_FcWorld.GetHeroHandle()->GetUnitSoxIndex(), SCT_KILL);
#else
				PlayComboSound(g_KillEffectUnit[i], g_FcWorld.GetHeroHandle()->GetUnitSoxIndex(), SCT_KILL);
#endif

				break;
			}
		}

		if(m_nKillEffectObjID == -1){
			return;
		}
	}
}

void CFcGauge::DrawLimitKillFXEffect()
{
	if(m_pFcMenuKillObj == NULL){
		return;
	}

	if(m_nKillEffectObjID == -1){
		return;
	}

	int nRttId = g_BsKernel.GetRTTManager()->GetRTTextureID(m_hKillRTTHandle);
	if(nRttId == -1){
		return;
	}
	
	int nSizeX = (int)(_FX_KILL_RTT_WIDTH * 1.5f);
	int nSizeY = (int)(_FX_KILL_RTT_HEIGHT * 1.5f);
	int nX = (_SCREEN_WIDTH - nSizeX)/2;
	int nY = (_SCREEN_HEIGHT- nSizeY)/2;
	
	if(m_nKillShock > 30)
	{
		float fScale = (40 - m_nComboShock -1) * 0.2f;
		float fsizeX = nSizeX * fScale;
		float fsizeY = nSizeY * fScale;

		int sizeX1 = (int)(fsizeX * fScale * 2);
		int sizeY1 = (int)(fsizeY * fScale * 2);
		int x1 = nX - sizeX1 / 2;
		int y1 = nY - sizeY1 / 2;
		int x2 = nX + sizeX1 / 2;
		int y2 = nY + sizeY1 / 2;

		g_BsKernel.DrawUIBox(_Ui_Mode_Image, 
			x1, y1, x2, y2,
			0.f ,D3DXCOLOR(1.f, 1.f, 1.f, 0.5f),
			0.f, nRttId,
			0, 0, _FX_KILL_RTT_WIDTH, _FX_KILL_RTT_HEIGHT);
	}	

	g_BsKernel.DrawUIBox_s(_Ui_Mode_Image,
		nX, nY, nSizeX, nSizeY,
		0.f, D3DXCOLOR(1.f,1.f,1.f,1.f),
		0.f, nRttId,
		0, 0, _FX_KILL_RTT_WIDTH, _FX_KILL_RTT_HEIGHT);
}

void CFcGauge::LoadPlayerNameTexture()
{
	char cName[256];
	int nUnitId = g_FcWorld.GetHeroHandle(0)->GetUnitSoxIndex();
	switch(nUnitId)
	{
	case 0:	strcpy(cName, _IMG_NAME_ASPHARR); break;
	case 1:	strcpy(cName, _IMG_NAME_INPHYY); break;
	case 2:	strcpy(cName, _IMG_NAME_TYURRU); break;
	case 3:	strcpy(cName, _IMG_NAME_KLARRANN); break;
	case 4:	strcpy(cName, _IMG_NAME_MYIFEE); break;
	case 5:	strcpy(cName, _IMG_NAME_VIGKVAGK); break;
	case 6:	strcpy(cName, _IMG_NAME_DWINGVATT); break;
	default: BsAssert(0);
	}

	char cImgTexture[256];
	sprintf(cImgTexture, "%s\\%s_%s.dds",
		g_LocalLanguage.GetLanguageDir(), cName, g_LocalLanguage.GetLanguageStr());

	g_BsKernel.chdir("interface");
	g_BsKernel.chdir("Local");
	m_nPlayerNameTexId = g_BsKernel.LoadTexture(cImgTexture);
	BsAssert(m_nPlayerNameTexId != -1);
	g_BsKernel.chdir("..");
	g_BsKernel.chdir("..");
}

void CFcGauge::LoadKillComboTexture()
{
	char cTempText[256];

	g_BsKernel.chdir("interface");
	g_BsKernel.chdir("Local");

	sprintf(cTempText, "%s\\gm_kill_%s.dds",
		g_LocalLanguage.GetLanguageDir(),
		g_LocalLanguage.GetLanguageStr());

	m_nTexKill = g_BsKernel.LoadTexture(cTempText);
	BsAssert(m_nTexKill != -1);

	sprintf(cTempText, "%s\\gm_kills_%s.dds",
		g_LocalLanguage.GetLanguageDir(),
		g_LocalLanguage.GetLanguageStr());

	m_nTexKills = g_BsKernel.LoadTexture(cTempText);
	BsAssert(m_nTexKills != -1);

	sprintf(cTempText, "%s\\gm_combo_%s.dds",
		g_LocalLanguage.GetLanguageDir(),
		g_LocalLanguage.GetLanguageStr());

	m_nTexCombo = g_BsKernel.LoadTexture(cTempText);
	BsAssert(m_nTexKills != -1);

	sprintf(cTempText, "%s\\gm_m_combo_%s.dds",
		g_LocalLanguage.GetLanguageDir(),
		g_LocalLanguage.GetLanguageStr());

	m_nTexMaxCombo = g_BsKernel.LoadTexture(cTempText);
	BsAssert(m_nTexKills != -1);
	
	sprintf(cTempText, "%s\\gm_LvUp_%s.dds",
		g_LocalLanguage.GetLanguageDir(),
		g_LocalLanguage.GetLanguageStr());

	m_nLevelUpTexId = g_BsKernel.LoadTexture(cTempText);
	BsAssert(m_nLevelUpTexId != -1);
		
	g_BsKernel.chdir("..");
	g_BsKernel.chdir("..");
}


void CFcGauge::CreateComboObjRTT()
{
	CFcMenu3DObjManager* pRTTexture = new CFcMenu3DObjManager();
	BsAssert(g_BsKernel.GetRTTManager() != NULL);
	m_hComboRTTHandle = g_BsKernel.GetRTTManager()->Create((CBsGenerateTexture*)pRTTexture,
		_FX_KILL_RTT_WIDTH, _FX_KILL_RTT_HEIGHT, 0.f, 0.f, 1.f, 1.f, true);

	m_pFcMenuComboObj = pRTTexture;

	CCrossVector* pCameraVector = m_pFcMenuComboObj->GetCameraCrossVector();
	pCameraVector->SetPosition(D3DXVECTOR3(0.f, 500.f, 0.f));
	m_pFcMenuComboObj->SetCameraTargetPos(D3DXVECTOR3(0.f, 0.f, 1.f));
}

void CFcGauge::ReleaseComboObjRTT()
{
	if(m_hComboRTTHandle != NULL)
	{
		BsAssert(g_BsKernel.GetRTTManager() != NULL);
		g_BsKernel.GetRTTManager()->Release(m_hComboRTTHandle);
		m_hComboRTTHandle = NULL;
		m_pFcMenuComboObj = NULL;
	}
}

void CFcGauge::ProcessLimitComboFXEffect()
{
	if(m_pFcMenuComboObj != NULL)
	{
		m_pFcMenuComboObj->Process();
		
		if(m_pFcMenuComboObj->GetObjectCount() == 0)
		{	
			ReleaseComboObjRTT();
			m_nComboShock = 0;
		}
	}
	
	HeroObjHandle Handle = g_FcWorld.GetHeroHandle();
	BsAssert(Handle);

	int nComboCount = Handle->GetComboCount();
	if(nComboCount == 0)
	{
		if(m_nComboTick > 0){
			m_nComboTick--;
		}
		else
		{
			m_nComboCount = 0;
			if(m_nComboShock == 0){
				m_nComboShockUVId = -1;
			}
			return;
		}
	}
	else if(nComboCount == 1){
		return;
	}
	else if(m_nComboTick < _COMBO_TICK_MAX){
		m_nComboTick++;
	}

	m_nComboCount = nComboCount;

	if(m_pFcMenuComboObj != NULL)
	{
		if(m_nComboShock > 0){
			m_nComboShock--;
		}
		return;
	}
	
	if(m_nComboShock == 0)
	{
		for(int i=0; i<_COMBO_EFFECT_COUNT; i++)
		{
			if(m_nComboShockUVId < i && m_nComboCount > g_ComboEffectUnit[i])
			{	
				m_nComboShockUVId = i;
				m_nComboShock = 40;

				if(m_pFcMenuComboObj == NULL){
					CreateComboObjRTT();
				}

				char cFxFileName[256];
#ifndef _TEST_COMBO_COUNT_FX_FILENAME
				sprintf(cFxFileName, "%dCombo.bfx", g_ComboEffectUnit[i]);
#else
				strcpy(cFxFileName, "500Combo.bfx");
#endif //_TEST_KILL_COUNT_FX_FILENAME

				int nFxTemplateIndex = m_pFcMenuComboObj->LoadFXTemplate(cFxFileName);
				if(nFxTemplateIndex == -1)
				{
					BsAssert(nFxTemplateIndex != -1);
					return;
				}

				D3DXVECTOR3 pos = D3DXVECTOR3(0, 0, 0);
				m_pFcMenuComboObj->AddFXObject(nFxTemplateIndex, pos, 1.f, false);
				
				PlayComboSound(g_ComboEffectUnit[i], g_FcWorld.GetHeroHandle()->GetUnitSoxIndex(), SCT_COMBO);

				break;
			}
		}
	}
}

void CFcGauge::DrawLimitComboFXEffect()
{
	if(m_nComboShockUVId == -1){
		return;
	}

	int nRttId = g_BsKernel.GetRTTManager()->GetRTTextureID(m_hComboRTTHandle);
	if(nRttId == -1){
		return;
	}

	int nSizeX = (int)(_FX_COMBO_RTT_WIDTH * 1.5f);
	int nSizeY = (int)(_FX_COMBO_RTT_HEIGHT * 1.5f);
	int nX = (_SCREEN_WIDTH - nSizeX)/2;
	int nY = (_SCREEN_HEIGHT- nSizeY)/2;

	if(m_nComboShock > 30)
	{
		float fScale = (40 - m_nComboShock -1) * 0.2f;
		float fsizeX = nSizeX * fScale;
		float fsizeY = nSizeY * fScale;

		int sizeX1 = (int)(fsizeX * fScale * 2);
		int sizeY1 = (int)(fsizeY * fScale * 2);
		int x1 = nX - sizeX1 / 2;
		int y1 = nY - sizeY1 / 2;
		int x2 = nX + sizeX1 / 2;
		int y2 = nY + sizeY1 / 2;

		g_BsKernel.DrawUIBox(_Ui_Mode_Image, 
			x1, y1, x2, y2,
			0.f ,D3DXCOLOR(1.f, 1.f, 1.f, 0.5f),
			0.f, nRttId,
			0, 0, _FX_COMBO_RTT_WIDTH, _FX_COMBO_RTT_HEIGHT);
	}	

	g_BsKernel.DrawUIBox_s(_Ui_Mode_Image,
		nX, nY, nSizeX, nSizeY,
		0.f, D3DXCOLOR(1.f,1.f,1.f,1.f),
		0.f, nRttId,
		0, 0, _FX_COMBO_RTT_WIDTH, _FX_COMBO_RTT_HEIGHT);
}


#define _MISSION_GAUGE_LEFTX	86 
#define _MISSION_GAUGE_MIDDLEX	94
#define _MISSION_GAUGE_RIGHTX	266
#define _MISSION_GAUGE_Y		448

#define _MISSION_GAUGE_BAR_X	90
#define _MISSION_GAUGE_BAR_Y	452
#define _MISSION_GAUGE_BAR_LEN	180

#define _MISSION_GAUGE_TEXT_X	90
#define _MISSION_GAUGE_TEXT_Y	460
void CFcGauge::DrawFriendlyCauge(int nHP, int nMaxHP, char* strName)
{
	//------------------------------------------------
	//left
	UVImage* pLeft = g_UVMgr.GetUVImage(_UV_mission_front);

	g_BsKernel.DrawUIBox_s(_Ui_Mode_Image,
		_MISSION_GAUGE_LEFTX, _MISSION_GAUGE_Y,
		pLeft->u2 - pLeft->u1, pLeft->v2 - pLeft->v1,
		0.f, D3DXCOLOR(1.f, 1.f, 1.f, 1),
		0.f, pLeft->nTexId,
		pLeft->u1, pLeft->v1, pLeft->u2, pLeft->v2);

	//------------------------------------------------
	//middle
	UVImage* pMiddle = g_UVMgr.GetUVImage(_UV_mission_middle);

	g_BsKernel.DrawUIBox_s(_Ui_Mode_Image,
		_MISSION_GAUGE_MIDDLEX, _MISSION_GAUGE_Y,
		_MISSION_GAUGE_BAR_LEN - 8, pMiddle->v2 - pMiddle->v1,
		0.f, D3DXCOLOR(1.f, 1.f, 1.f, 1),
		0.f, pMiddle->nTexId,
		pMiddle->u1, pMiddle->v1, pMiddle->u2, pMiddle->v2);

	//------------------------------------------------
	//right
	UVImage* pRight = g_UVMgr.GetUVImage(_UV_mission_back);

	g_BsKernel.DrawUIBox_s(_Ui_Mode_Image,
		_MISSION_GAUGE_RIGHTX, _MISSION_GAUGE_Y,
		pRight->u2 - pRight->u1, pRight->v2 - pRight->v1,
		0.f, D3DXCOLOR(1.f, 1.f, 1.f, 1),
		0.f, pRight->nTexId,
		pRight->u1, pRight->v1, pRight->u2, pRight->v2);

	//------------------------------------------------
	//bar
	int nValue = (int)(nHP * _MISSION_GAUGE_BAR_LEN / nMaxHP);
	UVImage* pBar = g_UVMgr.GetUVImage(_UV_mission_bar);

	g_BsKernel.DrawUIBox_s(_Ui_Mode_Image,
		_MISSION_GAUGE_BAR_X, _MISSION_GAUGE_BAR_Y,
		nValue, pBar->v2 - pBar->v1,
		0.f, D3DXCOLOR(1.f, 1.f, 1.f, 1),
		0.f, pBar->nTexId,
		pBar->u1, pBar->v1, pBar->u2, pBar->v2);

	if(strName != NULL)
	{
		g_pFont->DrawUIText(0, 0, -1, -1, "@(reset)@(scale=0.5,0.5)");
		g_pFont->DrawUIText(_MISSION_GAUGE_TEXT_X, _MISSION_GAUGE_TEXT_Y,
			-1, -1, strName);
	}
}


void CFcGauge::ProcessUnitTick()
{
	bool bMaxComboUnitTick = false;
	bool bComboUnitTick = false;
	bool bKillUnitTick = false;
	for(int i=0; i<_MAX_COUNT_UNIT; i++)
	{
		if(m_nMaxComboUnitTick[i] > 0)
		{
			m_nMaxComboUnitTick[i]--;
			bMaxComboUnitTick = true;
		}

		if(m_nComboUnitTick[i] > 0)
		{
			m_nComboUnitTick[i]--;
			bComboUnitTick = true;
		}

		if(m_nKillUnitTick[i] > 0)
		{
			m_nKillUnitTick[i]--;
			bKillUnitTick = true;
		}
	}

	if(bMaxComboUnitTick == false){
		m_nPrevMaxComboCount = m_nMaxComboCount;
	}

	if(bComboUnitTick == false){
		m_nPrevComboCount = m_nComboCount;
	}

	if(m_nPrevKillCount	 == false){
		m_nPrevComboCount = m_nKillCount;
	}
}