#include "stdafx.h"
#include "FcInterfaceManager.h"
#include "BSKernel.h"
#include "TextTable.h"
#include "InputPad.h"
#include "FcAbility.h"

#include "BSuiClassIDManager.h"
#include "BsUiFontAttrManager.h"
#include "BsUiUVManager.h"
#include "BsUiFont.h"
#include "FcGlobal.h"
#include "FcLetterBox.h"
#include "FcSpeech.h"
#include "FcMinimap.h"
#include "FcMiniMapRTT.h"
#include "FcGauge.h"
#include "FcWorld.h"
#include "FcHeroObject.h"

#include "FcCameraObject.h"

#include "LocalLanguage.h"

#include "FcRealtimeMovie.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG

#ifndef KERNEL_RELEASE_TEXTURE
#define KERNEL_RELEASE_TEXTURE(t) {if(t != -1) { g_BsKernel.ReleaseTexture(t); t = -1; } }
#endif

#ifndef KERNEL_RELEASE_SKIN
#define KERNEL_RELEASE_SKIN(s) {if(s != -1) { g_BsKernel.ReleaseSkin(s); s = -1; } }
#endif

#ifndef KERNEL_RELEASE_ANI
#define KERNEL_RELEASE_ANI(a) {if(a != -1) { g_BsKernel.ReleaseAni(a); a = -1; } }
#endif

#ifndef KERNEL_RELEASE_MESH
#define KERNEL_RELEASE_MESH(m) {if(m != -1) { g_BsKernel.ReleaseAni(m); m = -1; } }
#endif

FcInterfaceManager::FcInterfaceManager()
{	
	m_pPortrait		 = NULL;

	m_MenuHandle = -1;

	m_MinimapHandle = -1;

	m_SpeechHandle = -1;
	m_ReportHandle = -1;
	m_ItemReportHandle = -1;
	m_MissionGoalHandle	 = -1;
	m_SpecialText = -1;
	m_SpecialGauge = -1;
	m_AbilityHandle = -1;
	m_LetterBoxHandle = -1;
	m_GaugeHandle = -1;

    m_FadeEffectHandle = -1;
	m_MsgShowHandle = -1;

	m_bShowInterface = true;
	m_bForceShowInterface = true;
	m_bLetterBox = false;

	m_nTexMMS = -1;
	m_nTexGHPD = -1;
	m_nTexGHPB = -1;
	m_nTexGOML = -1;
	m_nTexGTOML = -1;
	m_nTexGHP = -1;
	m_nTexGO = -1;
	m_nTexGOM = -1;
	m_nTexGTO = -1;
	m_nTexGTOM = -1;
	m_nTexG = -1;
	m_nTexGT = -1;
	m_nSkin = -1;
	m_nAni = -1;
	m_nMeshGO = -1;
	m_nMeshGTO = -1;
	m_nMeshHP = -1;
}

FcInterfaceManager::~FcInterfaceManager()
{	
	Finalize();
}

void FcInterfaceManager::Initialize()
{
	BsAssert(m_pPortrait == NULL && "NULL");
	m_pPortrait = new CFcPortrait;
	m_pPortrait->Load();

	BsAssert(g_TextTable == NULL && "NULL");
	g_TextTable = new FCTextTable;
	g_TextTable->SetDefaultTextTable(0); //현재 디폴트 텍스트 테이블

	BsAssert(g_pFont == NULL && "NULL");
	g_pFont = new BsUiFont();

	g_ClassIdMgr.Load();
	g_FontAttrMgr.Load();
	g_UVMgr.Load();


	////////////////////////////////////////////////////////////////
	//Menu는 고정
	CFcInterfaceObj *pMenuObj = new CFcMenuManager;
	BsAssert(m_MenuHandle == -1 && "Duplication definition");	
	m_MenuHandle = _INTERFACE_OBJ_MENU;
	m_InterfaceList[m_MenuHandle] = pMenuObj;

	// Pre-load common interface resources as they are
	// used throughout the game...
	g_BsKernel.chdir("interface");
	m_nMeshGO = g_BsKernel.LoadMesh(-1, _GAUGE_ORB_MESH);
	m_nMeshGTO = g_BsKernel.LoadMesh(-1, _GAUGE_TRUEORB_MESH);
	m_nMeshHP = g_BsKernel.LoadMesh(-1, _GAUGE_HP_MESH);
	m_nTexMMS = g_BsKernel.LoadTexture(_FC_MINIMAP_SIGHT);
	m_nTexGHPD = g_BsKernel.LoadTexture(_GAUGE_HP_DAMAGE);
	m_nTexGHPB = g_BsKernel.LoadTexture(_GAUGE_HP_BASE);
	m_nTexGOML = g_BsKernel.LoadTexture(_GAUGE_ORB_MAX_LIGHT);
	m_nTexGTOML = g_BsKernel.LoadTexture(_GAUGE_TRUEORB_MAX_LIGHT);
	m_nTexGHP = g_BsKernel.LoadVolumeTexture(_GAUGE_HP);
	m_nTexGO = g_BsKernel.LoadVolumeTexture(_GAUGE_ORB);
	m_nTexGOM = g_BsKernel.LoadVolumeTexture(_GAUGE_ORB_MAX);
	m_nTexGTO = g_BsKernel.LoadVolumeTexture(_GAUGE_TRUEORB);
	m_nTexGTOM = g_BsKernel.LoadVolumeTexture(_GAUGE_TRUEORB_MAX);
	m_nTexG = g_BsKernel.LoadTexture(_GUARDIAN_DECAL_TEXTURE);
	m_nTexGT = g_BsKernel.LoadTexture(_GUARDIAN_TARGET_DECAL_TEXTURE);
	g_BsKernel.chdir("..");
	m_nSkin = g_BsKernel.LoadSkin(-1, _ENEMY_TARGET_SKIN);;
	m_nAni = g_BsKernel.LoadAni(-1, _ENEMY_TARGET_ANI);
}


void FcInterfaceManager::Finalize()
{
	if(m_pPortrait)
	{
		delete m_pPortrait;
		m_pPortrait = NULL;
	}
	
	if(g_TextTable)
	{
		delete g_TextTable;
		g_TextTable = NULL;
	}

	if(g_pFont)
	{
		delete g_pFont;
		g_pFont = NULL;
	}

	for(unsigned int i = 0;i < MAX_INTERFACE_OBJ;i++){
		if( m_InterfaceList[i] )
		{
			m_InterfaceList[i]->ReleaseData();
			delete m_InterfaceList[i];
			m_InterfaceList[i] = NULL;
		}
	}

	m_MenuHandle = -1;
	m_MinimapHandle = -1;
	m_SpeechHandle = -1;
	m_ReportHandle = -1;
	m_ItemReportHandle = -1;
	m_MissionGoalHandle = -1;
	m_SpecialText = -1;
	m_SpecialGauge = -1;
	m_AbilityHandle = -1;
	m_LetterBoxHandle = -1;
	m_GaugeHandle = -1;
    m_FadeEffectHandle = -1;
	m_MsgShowHandle = -1;

	g_UVMgr.Release();
	BsUiSystem::DestroyInstance();

	KERNEL_RELEASE_TEXTURE(m_nTexMMS);
	KERNEL_RELEASE_TEXTURE(m_nTexGHPD);
	KERNEL_RELEASE_TEXTURE(m_nTexGHPB);
	KERNEL_RELEASE_TEXTURE(m_nTexGOML);
	KERNEL_RELEASE_TEXTURE(m_nTexGTOML);
	KERNEL_RELEASE_TEXTURE(m_nTexGHP);
	KERNEL_RELEASE_TEXTURE(m_nTexGO);
	KERNEL_RELEASE_TEXTURE(m_nTexGOM);
	KERNEL_RELEASE_TEXTURE(m_nTexGTO);
	KERNEL_RELEASE_TEXTURE(m_nTexGTOM);
	KERNEL_RELEASE_TEXTURE(m_nTexG);
	KERNEL_RELEASE_TEXTURE(m_nTexGT);
	KERNEL_RELEASE_TEXTURE(m_nTexMMS);
	KERNEL_RELEASE_SKIN(m_nSkin);
	KERNEL_RELEASE_ANI(m_nAni);
	KERNEL_RELEASE_MESH(m_nMeshGO);
	KERNEL_RELEASE_MESH(m_nMeshGTO);
	KERNEL_RELEASE_MESH(m_nMeshHP);
}

void FcInterfaceManager::InitStage()
{	
	m_bForceShowInterface = true;

#ifdef _XBOX
	CreateObject( new CFcUiMiniMap, m_MinimapHandle);
#else
	CreateObject( new CFcMinimap, m_MinimapHandle);
#endif

	CreateObject( new CFcGauge,m_GaugeHandle);

	CreateObject( new FcAbilInterface(m_pPortrait), m_AbilityHandle);
		
	CreateObject(new CFcSpeech(m_pPortrait), m_SpeechHandle);
	SetSpeechOn(g_FCGameData.ConfigInfo.bCaption);

	CreateObject(new CFcReport, m_ReportHandle);

	CreateObject(new CFcItemReport, m_ItemReportHandle);

	CreateObject(new CFcMissionGoal, m_MissionGoalHandle);

	CreateObject(new CFcSpecialMissionText, m_SpecialText);

	CreateObject(new CFcSpecialMissionGauge, m_SpecialGauge);

	CreateObject(new CFcFadeEffect, m_FadeEffectHandle);

	CreateObject(new CFcLetterBox,m_LetterBoxHandle);
	SetLetterOn(g_FCGameData.ConfigInfo.bCaption);

	CreateObject(new CFcMsgShow, m_MsgShowHandle);
}

void FcInterfaceManager::FinishStage()
{
	m_pPortrait->ReleaseTexture();

	DeleteObject( m_MinimapHandle );
	DeleteObject( m_AbilityHandle );
	DeleteObject( m_GaugeHandle );
	DeleteObject( m_LetterBoxHandle );
	DeleteObject( m_SpeechHandle );
	DeleteObject( m_ReportHandle );
	DeleteObject( m_ItemReportHandle );
	DeleteObject( m_MissionGoalHandle );
	DeleteObject( m_SpecialText );
	DeleteObject( m_SpecialGauge );
	DeleteObject( m_FadeEffectHandle );
	DeleteObject( m_MsgShowHandle );

}


void FcInterfaceManager::Clear()
{
	if( m_LetterBoxHandle != -1 )
		((CFcLetterBox *)m_InterfaceList[m_LetterBoxHandle])->Clear();
}

void FcInterfaceManager::Process()
{
	OnInputKey();

	for(int i = 0;i < MAX_INTERFACE_OBJ;i++)
	{
		if( m_InterfaceList[i] )
		{
			if(IsPause() == false || i == _INTERFACE_OBJ_MENU){
				m_InterfaceList[i]->Process();
			}
		}
	}

	g_pFont->PreProcessAniSpecialFont(GetProcessTick());

	g_BsKernel.SetUIViewerIndex(_UI_FULL_VIEWER);

	for(int i=0; i<MAX_INTERFACE_OBJ; i++)
	{
		if( m_InterfaceList[i] )
		{
			if(g_FCGameData.bMenuShow || i == _INTERFACE_OBJ_MENU){
				m_InterfaceList[i]->Update();
			}
		}
	}
	
	if(g_FCGameData.bShowSafeArea)
	{
		//up
		g_BsKernel.DrawUIBox(_Ui_Mode_Box,
			64, 36 - 1, 1215, 36,
			0.f, D3DXCOLOR(1.f, 0.f, 0.f, 1));

		//down
		g_BsKernel.DrawUIBox(_Ui_Mode_Box,
			64, 683, 1215, 683 + 1,
			0.f, D3DXCOLOR(1.f, 0.f, 0.f, 1));

		//left
		g_BsKernel.DrawUIBox(_Ui_Mode_Box,
			64 -1, 36, 64, 683,
			0.f, D3DXCOLOR(1.f, 0.f, 0.f, 1));

		//right
		g_BsKernel.DrawUIBox(_Ui_Mode_Box,
			1215, 36, 1215 + 1, 683,
			0.f, D3DXCOLOR(1.f, 0.f, 0.f, 1));
	}
}

void FcInterfaceManager::OnInputKey()
{
	if(IsShowLetterBox() == true){
		return;
	}
	if( g_pFcRealMovie && g_pFcRealMovie->IsPlay() == true ){
		return;
	}

	for(int inp = 0;inp < PAD_INPUT_RSTICK+1;inp++)
	{
		int nPressTick = CInputPad::GetInstance().GetKeyPressTick(g_FCGameData.nEnablePadID, inp);
		for(int j = 0;j < MAX_INTERFACE_OBJ;j++)
		{
			if( m_InterfaceList[j] ){	
				m_InterfaceList[j]->KeyInput(0, inp, nPressTick);
			}
		}
	}
}


bool FcInterfaceManager::SetUserTextTable( const char* pFileName )
{
	return g_TextTable->SetUserTextTable( (char*)pFileName );	//현재 디폴트 텍스트 테이블
}

//Show interface--------------------------------------------------------------------------------------	
void FcInterfaceManager::ForceShowInterface(bool bShow)
{
	m_bForceShowInterface = bShow;
	SetShowGameInterface(bShow);
}

void FcInterfaceManager::ShowInterface(bool bShow)
{
	if(g_pFcRealMovie && g_pFcRealMovie->IsPlay() == true){
		bShow = false;
	}
	else if(m_bForceShowInterface == false){
		return;
	}

	m_bShowInterface = bShow;
	SetShowGameInterface(bShow);
}

void FcInterfaceManager::SetShowGameInterface(bool bShow)
{
	SetShowHandle(_IOT_SPEECH, bShow);
	SetShowHandle(_IOT_MINIMAP, bShow);
	SetShowHandle(_IOT_GAUGE, bShow);
	SetShowHandle(_IOT_ABILITY, bShow);
	SetShowHandle(_IOT_REPORT, bShow);
	SetShowHandle(_IOT_ITEMREPORT, bShow);
	SetShowHandle(_IOT_MISSIONGOAL, bShow);
	SetShowHandle(_IOT_SPECIALTEXT, bShow);
	SetShowHandle(_IOT_SPECIALGAUGE, bShow);
}


bool FcInterfaceManager::IsShowHandle(int nMenuType)
{
	int nHandle = GetMenuObjHandle(nMenuType);
	if(nHandle != -1){
		return (m_InterfaceList[nHandle])->IsShow();
	}

	return false;
}

void FcInterfaceManager::SetShowHandle(int nMenuType, bool bShow)
{
	int nHandle = GetMenuObjHandle(nMenuType);
	if(nHandle != -1){
		m_InterfaceList[nHandle]->Show(bShow);
	}
}

int FcInterfaceManager::GetMenuObjHandle(int nMenuType)
{
	switch(nMenuType)
	{
	case _IOT_MENU:				return m_MenuHandle;
	case _IOT_MINIMAP:			return m_MinimapHandle;
	case _IOT_SPEECH:			return m_SpeechHandle;
	case _IOT_REPORT:			return m_ReportHandle;
	case _IOT_ITEMREPORT:		return m_ItemReportHandle;
	case _IOT_MISSIONGOAL:		return m_MissionGoalHandle;
	case _IOT_SPECIALTEXT:		return m_SpecialText;
	case _IOT_SPECIALGAUGE:		return m_SpecialGauge;
	case _IOT_ABILITY:			return m_AbilityHandle;
	case _IOT_LETTERBOX:		return m_LetterBoxHandle;
	case _IOT_GAUGE:			return m_GaugeHandle;
	case _IOT_FADEEFFECT:		return m_FadeEffectHandle;
	case _IOT_MSGSHOW:			return m_MsgShowHandle;
	}

	return -1;
}

//speech--------------------------------------------------------------------------------------
void FcInterfaceManager::SetSpeech(int PortraitID,int nTextTableID)
{
	if(m_SpeechHandle == -1)
	{
		BsAssert(0);
		return;
	}

	((CFcSpeech *)m_InterfaceList[m_SpeechHandle])->SetSpeech(PortraitID, nTextTableID);
}

void FcInterfaceManager::SetSpeechOn(bool bOn)
{
	if(m_SpeechHandle == -1)
	{
		BsAssert(0);
		return;
	}

	((CFcSpeech *)m_InterfaceList[m_SpeechHandle])->SetSpeechOn(bOn);
}

bool FcInterfaceManager::GetSpeechOn()
{
	if(m_SpeechHandle == -1)
	{
		BsAssert(0);
		return false;
	}

	return ((CFcSpeech *)m_InterfaceList[m_SpeechHandle])->GetSpeechOn();
}

bool FcInterfaceManager::IsSpeechPlay()
{
	if(m_SpeechHandle == -1)
	{
		BsAssert(0);
		return false;
	}

	return ((CFcSpeech *)m_InterfaceList[m_SpeechHandle])->IsPlay();
}

//letterbox--------------------------------------------------------------------------------------
void FcInterfaceManager::ShowLetterBox(bool bShow)
{
	if(m_LetterBoxHandle == -1)
	{
		BsAssert(0);
		return;
	}

	if(g_pFcRealMovie)
	{
		if(g_pFcRealMovie->IsPlay() == true){
			bShow = false;
		}
	}

	((CFcLetterBox *)m_InterfaceList[m_LetterBoxHandle])->ShowLetterBox(bShow);
}

void FcInterfaceManager::SetLetterOn(bool bOn)
{
	if(m_LetterBoxHandle == -1)
	{
		BsAssert(0);
		return;
	}

	((CFcLetterBox *)m_InterfaceList[m_LetterBoxHandle])->SetLetterOn(bOn);
}

bool FcInterfaceManager::IsShowTextInLetterBox()
{
	if(m_LetterBoxHandle == -1)
	{
		BsAssert(0);
		return false;
	}

	return ((CFcLetterBox *)m_InterfaceList[m_LetterBoxHandle])->IsShowText();
}

void FcInterfaceManager::HideTextInLetterBox()
{
	if(m_LetterBoxHandle == -1)
	{
		BsAssert(0);
		return;
	}
	((CFcLetterBox *)m_InterfaceList[m_LetterBoxHandle])->HideText();
}


float FcInterfaceManager::GetLetterBoxAlpha()
{
	if(m_LetterBoxHandle == -1)
	{
		BsAssert(0);
		return 0.f;
	}

	return ((CFcLetterBox *)m_InterfaceList[m_LetterBoxHandle])->GetAlpha();
}

bool FcInterfaceManager::IsShowLetterBox()
{
	if(m_LetterBoxHandle == -1){
		return false;
	}

	CFcLetterBox* pLetterBox = (CFcLetterBox*)m_InterfaceList[m_LetterBoxHandle];
	if(pLetterBox->IsShow())
	{
		return pLetterBox->IsShowLetterBox();
	}

	return false;
}

void FcInterfaceManager::ShowTextInLetterBox(int nTextTableID)
{
	if(m_LetterBoxHandle == -1)
	{
		BsAssert(0);
		return;
	}

	((CFcLetterBox *)m_InterfaceList[m_LetterBoxHandle])->ShowText(nTextTableID);
}

//Minimap--------------------------------------------------------------------------------------
void FcInterfaceManager::MinimapShowToggle()
{
	if(m_MinimapHandle == -1)
	{
		BsAssert(0);
		return;
	}

#ifndef _XBox
	((CFcMinimap *)m_InterfaceList[m_MinimapHandle])->ShowToggle();
#endif
}

void FcInterfaceManager::MinimapShow(bool bShow)
{
	if(m_MinimapHandle == -1)
	{
		BsAssert(0);
		return;
	}

#ifdef _XBOX
	((CFcUiMiniMap *)m_InterfaceList[m_MinimapHandle])->Show(bShow);
#else
	((CFcMinimap *)m_InterfaceList[m_MinimapHandle])->Show(bShow);
#endif
}

void FcInterfaceManager::MinimapZoomChange(float fZoom)
{
	;
}

void FcInterfaceManager::MinimapPointOn(bool bShow, const char *szName,	CFcTroopObject *pTroop, DWORD nTick)
{
	if( m_MinimapHandle == -1 )
	{
		BsAssert(0);
		return;
	}

#ifdef _XBOX
	((CFcUiMiniMap *)m_InterfaceList[m_MinimapHandle])->PointOn( bShow, szName, pTroop, nTick);
#else
	((CFcMinimap *)m_InterfaceList[m_MinimapHandle])->PointOn( bShow, szName, pTroop, 0.f, 0.f, nTick);
#endif
}

void FcInterfaceManager::MinimapAreaOn(bool bShow, const char *szName, float fStartX, float fStartY,
										float fEndX, float fEndY, DWORD nTick)
{
	if( m_MinimapHandle == -1 )
	{
		BsAssert(0);
		return;
	}

#ifdef _XBOX
	((CFcUiMiniMap *)m_InterfaceList[m_MinimapHandle])->AreaOn(bShow, szName, fStartX, fStartY, fEndX, fEndY, nTick);
#endif
}

//portrait----------------------------------------------------------------------------------------------
void FcInterfaceManager::DrawPortrait(int ID,int iSX,int iSY,int iEX,int iEY)
{
	if(m_pPortrait){
		m_pPortrait->DrawPortrait(ID,iSX,iSY,iEX,iEY, 1.f);
	}
}

//report----------------------------------------------------------------------------------------------
void FcInterfaceManager::SetReport(int nTextTableID)
{
	if(m_ReportHandle == -1)
	{
		BsAssert(0);
		return;
	}

	((CFcReport*)m_InterfaceList[m_ReportHandle])->SetReport(nTextTableID);
}

void FcInterfaceManager::SetItemReport(char* szText)
{
	if(m_ItemReportHandle == -1)
	{
		BsAssert(0);
		return;
	}

	((CFcItemReport*)m_InterfaceList[m_ItemReportHandle])->SetItemReport(szText);
}


//mission----------------------------------------------------------------------------------------------
void FcInterfaceManager::SetMissionGoal(int nTextTableID)
{
	if(m_MissionGoalHandle == -1)
	{	
		BsAssert(0);
		return;
	}

	if( nTextTableID < 0 )
	{
		BsAssert( 0 );
		DebugString( "Mission Objective 텍스트ID가 잘못 되었습니다\n" );
		return;
	}

	char szStr[STR_SPEECH_MAX];
	bool result = g_TextTable->GetText(nTextTableID,szStr, _countof(szStr));

	((CFcMissionGoal*)m_InterfaceList[m_MissionGoalHandle])->SetMissionGoal(szStr);
}

bool FcInterfaceManager::IsPlayMissionGoal()
{
	return ((CFcMissionGoal*)m_InterfaceList[m_MissionGoalHandle])->IsPlay();
}

void FcInterfaceManager::SetSpecialMissionGauge(int nValue, int nMaxValue, char* pCaption)
{
	return;

	if(m_SpecialGauge == -1)
	{
		BsAssert(0);
		return;
	}

	((CFcSpecialMissionGauge*)m_InterfaceList[m_SpecialGauge])->SetValue(nValue, nMaxValue, pCaption);
}

void FcInterfaceManager::SetSpecialMissionText(char* szText)
{
	if(m_SpecialText == -1)
	{
		BsAssert(0);
		return;
	}

	((CFcSpecialMissionText*)m_InterfaceList[m_SpecialText])->SetText(szText);
}

void FcInterfaceManager::ClearSpecialMissionGauge()
{
	return;

	if(m_SpecialGauge == -1)
	{
		BsAssert(0);
		return;
	}

	((CFcSpecialMissionGauge*)m_InterfaceList[m_SpecialGauge])->SetValue(-1, -1);
}

void FcInterfaceManager::ClearSpecialMissionText()
{
	if(m_SpecialText == -1)
	{
		BsAssert(0);
		return;
	}

	char szTemp[2] = "";
	((CFcSpecialMissionText*)m_InterfaceList[m_SpecialText])->SetText(szTemp);
}


//gauge----------------------------------------------------------------------------------------------
void FcInterfaceManager::SetEnemyBossGauge(TroopObjHandle Handle, int nNameTextId)
{
	if(m_GaugeHandle == -1)
	{
		BsAssert(0);
		return;
	}

	((CFcGauge*)m_InterfaceList[m_GaugeHandle])->SetGauge(Handle, nNameTextId, GAUGETYPE_BOSS);
}

void FcInterfaceManager::RemoveEnemyBossGauge(TroopObjHandle Handle)
{
	if(m_GaugeHandle == -1)
	{
		BsAssert(0);
		return;
	}

	((CFcGauge*)m_InterfaceList[m_GaugeHandle])->RemoveGauge(Handle);
}



void FcInterfaceManager::SetTroopGauge(TroopObjHandle Handle, int nNameTextId, bool bIncludeLeader)
{
	if(m_GaugeHandle == -1)
	{
		BsAssert(0);
		return;
	}
	if( bIncludeLeader )
		((CFcGauge*)m_InterfaceList[m_GaugeHandle])->SetGauge(Handle, nNameTextId, GAUGETYPE_TROOP);
	else
		((CFcGauge*)m_InterfaceList[m_GaugeHandle])->SetGauge(Handle, nNameTextId, GAUGETYPE_TROOP_EXCEPT_LEADER);
}

void FcInterfaceManager::RemoveTroopGauge(TroopObjHandle Handle)
{
	RemoveEnemyBossGauge(Handle);
}



int FcInterfaceManager::GetMaxComboCount()
{
	if(m_GaugeHandle == -1)
	{
		BsAssert(0);
		return 0;
	}

	return ((CFcGauge*)m_InterfaceList[m_GaugeHandle])->GetMaxComboCount();
}

void FcInterfaceManager::SetUnitGauge(int nUnitId, bool bOn)
{
	int nIndex = -1;
	if(bOn == true)
	{
		if(IsValidUnitGauge(nUnitId, nIndex) == false)
		{
			m_UnitGaugeList.push_back(nUnitId);
		}
	}
	else
	{
		if(IsValidUnitGauge(nUnitId, nIndex) == true)
		{
			m_UnitGaugeList.erase(m_UnitGaugeList.begin() + nIndex);
		}
	}
}

bool FcInterfaceManager::IsUnitGauge(int nUnitId)
{
	int nIndex = -1;
	return IsValidUnitGauge(nUnitId, nIndex);
}

bool FcInterfaceManager::CanICheckUnitGauge()
{
	if( m_UnitGaugeList.size() > 0 )
		return true;

	if( g_FCGameData.bShowFriendlyGauge == true )
		return true;

	if( g_FCGameData.bShowEnemyGauge == true )
		return true;

	return false;
}

bool FcInterfaceManager::IsValidUnitGauge(int nUnitId, int &nIndex)
{
	for(DWORD i=0; i<m_UnitGaugeList.size(); i++)
	{
		if(m_UnitGaugeList[i] == nUnitId)
		{
			nIndex = i;
			return true;
		}
	}

	return false;
}

void FcInterfaceManager::OnHitMsg( GameObjHandle &Handle )
{
	if(m_GaugeHandle == -1)
	{
		BsAssert(0);
		return;
	}

	((CFcGauge*)m_InterfaceList[m_GaugeHandle])->OnHitMsg(Handle);
}

//fade----------------------------------------------------------------------------------------------
void FcInterfaceManager::SetFadeEffect(bool bFadeIn,int nSec,D3DXVECTOR3 *pColor)
{
	if(m_FadeEffectHandle == -1)
	{
		BsAssert(0);
		return;
	}

	if(bFadeIn) {
		((CFcFadeEffect *)m_InterfaceList[m_FadeEffectHandle])->SetFadeIn(nSec);
	}
	else {
		((CFcFadeEffect *)m_InterfaceList[m_FadeEffectHandle])->SetFadeOut(nSec,pColor);
	}
}

bool FcInterfaceManager::IsFadeInStop()
{
	if(m_FadeEffectHandle == -1)
	{
		BsAssert(0);
		return false;
	}

	return ((CFcFadeEffect *)m_InterfaceList[m_FadeEffectHandle])->IsFadeInStop();
}

bool FcInterfaceManager::IsFadeOutStop()
{
	if(m_FadeEffectHandle == -1)
	{
		BsAssert(0);
		return false;
	}

	return ((CFcFadeEffect *)m_InterfaceList[m_FadeEffectHandle])->IsFadeOutStop();
}

bool FcInterfaceManager::IsFade()
{
	if(m_FadeEffectHandle == -1)
	{
		BsAssert(0);
		return false;
	}

	return ((CFcFadeEffect *)m_InterfaceList[m_FadeEffectHandle])->IsFade();
}


//message----------------------------------------------------------------------------------------------
void FcInterfaceManager::SetMsgShowXY(int nX,int nY,int nSec,int nTextTableID)
{
	if(m_MsgShowHandle == -1)
	{
		BsAssert(0);
		return;
	}

	((CFcMsgShow *)m_InterfaceList[m_MsgShowHandle])->SetMsgShowXY(nX,nY,nSec,nTextTableID);
}

bool FcInterfaceManager::IsMsgShowXYEnd()
{
	if(m_MsgShowHandle == -1)
	{
		BsAssert(0);
		return false;
	}

	return ((CFcMsgShow *)m_InterfaceList[m_MsgShowHandle])->IsMsgShowXYEnd();
}


void FcInterfaceManager::DrawDebugText(int nPlayer,D3DXVECTOR3 *Pos,char *szText)
{
	D3DXVECTOR3 OutPutVec;
	int nCamIndex = g_FcWorld.GetPlayerCamIndex(nPlayer);
	g_BsKernel.GetScreenPos(nCamIndex,Pos,&OutPutVec);

	if(OutPutVec.z > 0.f && OutPutVec.z < 1.f)
	{
		g_pFont->DrawUIText((int)OutPutVec.x,(int)OutPutVec.y, -1, -1, szText);
	}
}



//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
void FcInterfaceManager::CreateObject(CFcInterfaceObj *pObject,int &nHandle)
{
	BsAssert(pObject != NULL);
	BsAssert(nHandle == -1);
	
	nHandle = GetEmptyIndex();

	BsAssert(nHandle != -1);

	pObject->SetHandle(&nHandle);
	m_InterfaceList[nHandle] = pObject;
}

int FcInterfaceManager::GetEmptyIndex()
{
	for( int i=0; i<MAX_INTERFACE_OBJ; i++ )
	{
		if( m_InterfaceList[i] == NULL )
		{
			return i;
		}
	}
	BsAssert( 0 && "InterfaceObj꽉찼음" );
	return -1;
}

void FcInterfaceManager::DeleteObject(int &nHandle)
{
	if( nHandle == -1 ){
		return;
	}

	BsAssert(nHandle < _countof(m_InterfaceList)); //aleksger: prefix bug 759: No validation is made that we are within upper bound.
	BsAssert( m_InterfaceList[nHandle] );
	m_InterfaceList[nHandle]->ReleaseData();
	delete m_InterfaceList[nHandle];
	m_InterfaceList[nHandle] = NULL;

	nHandle = -1;
}
