#include "StdAfx.h"
#include "BsKernel.h"
#include "BsHFWorld.h"
#include "BStreamExt.h"
#include "Parser.h"
#include "BsFileIO.h"

#include "FcUtil.h"
#include ".\\data\\SignalType.h"
#include "FcWorld.h"
#include "FcBaseObject.h"
#include "FcAniObject.h"
#include "FcGameObject.h"
#include "FcUnitObject.h"
#include "FcHeroObject.h"
#include "FcHorseObject.h"
#include "Aspharr.h"
#include "Inphy.h"
#include "Klarrann.h"
#include "Myifee.h"
#include "FcCameraObject.h"
#include "InputPad.h"
#include "FcAIObject.h"
#include "FcTroopAIObject.h"
#include "FcTroopObject.h"

#include "FcTroopManager.h"
#include "FcProp.h"
#include "FcPropManager.h"
#include "BsClipTest.h"
#include "FcState.h"
#include "FcDefines.h"
#include "FcTriggerCommander.h"
#include "FcMinimap.h"
#include "FcGlobal.h"
#include "FcFXCommon.h"
#include "FcInterfaceManager.h"

#include "navigationmesh.h"
#include "navigationcell.h"
#include "CollisionMesh.h"

#include "FcRealtimeMovie.h"


#include "FcAbilityManager.h"
#include "FcFxBase.h"
#include "FcFxManager.h"
#include "FcFxSimplePlay.h"

#include "DebugUtil.h"
#include "FcSOXLoader.h"

#include "FcPhysicsLoader.h"

#include "BsBillboardMgr.h"
#include "FcSoundManager.h"
#include "FcProjectile.h"

#include "./Data/FXList.h"


#include <fstream>

//#define ENABLE_FULL_STAGE_CLEANOUT

#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG

extern HANDLE g_hPackedRes1;
extern HANDLE g_hPackedTex1;
extern HANDLE g_hPackedRes3;
extern HANDLE g_hPackedTex3;

extern HANDLE g_hPackedTex2;
extern HANDLE g_hPackedCharTex;


//#define FC_GLOBAL_VAR_DEBUG

float g_BillboardAnimInfo[26][4] = {	// None, Battle, Back, Horse 4가지 종류로 구분
	0.0f,				0.f,			 	0.f,				0.f, 					// ANI_TYPE_DEFAULT			0
		0.0f,				SPRITE_RANGE*3.f, 	0.f,				0.f,					// ANI_TYPE_STAND
		SPRITE_RANGE*1.f,	SPRITE_RANGE*4.f, 	SPRITE_RANGE*1.f,	SPRITE_RANGE*1.f,		// ANI_TYPE_WALK
		SPRITE_RANGE*1.f,	SPRITE_RANGE*5.f, 	SPRITE_RANGE*1.f,	SPRITE_RANGE*1.f,		// ANI_TYPE_RUN	
		SPRITE_RANGE*3.f,	SPRITE_RANGE*3.f,	SPRITE_RANGE*3.f,	SPRITE_RANGE*3.f,					// ANI_TYPE_STOP
		SPRITE_RANGE*3.f,	SPRITE_RANGE*3.f,	SPRITE_RANGE*3.f,	SPRITE_RANGE*3.f,					// ANI_TYPE_RIDE_HORSE
		SPRITE_RANGE*3.f,	SPRITE_RANGE*3.f,	SPRITE_RANGE*3.f,	SPRITE_RANGE*3.f,					// ANI_TYPE_HIT
		SPRITE_RANGE*3.f,	SPRITE_RANGE*3.f,	SPRITE_RANGE*3.f,	SPRITE_RANGE*3.f,					// ANI_TYPE_FALL
		SPRITE_RANGE*3.f,	SPRITE_RANGE*3.f,	SPRITE_RANGE*3.f,	SPRITE_RANGE*3.f,					// ANI_TYPE_JUMP_HIT		
		SPRITE_RANGE*2.f,	SPRITE_RANGE*2.f, 	SPRITE_RANGE*2.f,	SPRITE_RANGE*2.f,		// ANI_TYPE_DOWN_HIT		
		SPRITE_RANGE*6.f,	SPRITE_RANGE*6.f, 	SPRITE_RANGE*6.f,	SPRITE_RANGE*6.f,		// ANI_TYPE_DOWN			10
		SPRITE_RANGE*2.f,	SPRITE_RANGE*2.f, 	SPRITE_RANGE*2.f,	SPRITE_RANGE*2.f,		// ANI_TYPE_DIE				
		SPRITE_RANGE*1.f,	SPRITE_RANGE*1.f, 	SPRITE_RANGE*1.f,	SPRITE_RANGE*1.f,		// ANI_TYPE_ATTACK			
		SPRITE_RANGE*1.f,	SPRITE_RANGE*1.f, 	SPRITE_RANGE*1.f,	SPRITE_RANGE*1.f,		// ANI_TYPE_JUMP_ATTACK		
		SPRITE_RANGE*1.f,	SPRITE_RANGE*1.f, 	SPRITE_RANGE*1.f,	SPRITE_RANGE*1.f,		// ANI_TYPE_RUN_ATTACK		
		SPRITE_RANGE*1.f,	SPRITE_RANGE*1.f, 	SPRITE_RANGE*1.f,	SPRITE_RANGE*1.f,		// ANI_TYPE_SPECIAL_ATTACK	
		SPRITE_RANGE*3.f,	SPRITE_RANGE*3.f,	SPRITE_RANGE*3.f,	SPRITE_RANGE*3.f,					// ANI_TYPE_JUMP_UP			
		SPRITE_RANGE*3.f,	SPRITE_RANGE*3.f,	SPRITE_RANGE*3.f,	SPRITE_RANGE*3.f,					// ANI_TYPE_JUMP_DOWN		
		SPRITE_RANGE*3.f,	SPRITE_RANGE*3.f,	SPRITE_RANGE*3.f,	SPRITE_RANGE*3.f,					// ANI_TYPE_VICTORY			
		SPRITE_RANGE*1.f,	SPRITE_RANGE*4.f, 	SPRITE_RANGE*1.f,	SPRITE_RANGE*1.f,		// ANI_TYPE_MOVE			
		SPRITE_RANGE*3.f,	SPRITE_RANGE*3.f,	SPRITE_RANGE*3.f,	SPRITE_RANGE*3.f,					// ANI_TYPE_DEFENSE	
		SPRITE_RANGE*3.f,	SPRITE_RANGE*3.f,	SPRITE_RANGE*3.f,	SPRITE_RANGE*3.f,					// ANI_TYPE_CUSTOM_0
		SPRITE_RANGE*3.f,	SPRITE_RANGE*3.f,	SPRITE_RANGE*3.f,	SPRITE_RANGE*3.f,					// ANI_TYPE_CUSTOM_1
		SPRITE_RANGE*7.f,	SPRITE_RANGE*7.f, 	SPRITE_RANGE*7.f,	SPRITE_RANGE*7.f,		// ANI_TYPE_RISE	
		SPRITE_RANGE*3.f,	SPRITE_RANGE*3.f,	SPRITE_RANGE*3.f,	SPRITE_RANGE*3.f,					// ANI_TYPE_GROUND_HIT
		SPRITE_RANGE*3.f,	SPRITE_RANGE*3.f,	SPRITE_RANGE*3.f,	SPRITE_RANGE*3.f
};

const int g_nMatchingIndex[7] = { 1001, 1002,  1007, 1003, 1005, 1004, 1006};

enum UNIT_INDEX
{
	UNIT_INDEX_ORC,
	UNIT_INDEX_SOLDIER,
	UNIT_INDEX_GOBLIN,
	UNIT_INDEX_MALE_KNIGHT
};


#define USE_MENU_GUARDIAN

#define GUARDIAN_TROOP_NAME_1	"Guardian0"
#define GUARDIAN_TROOP_NAME_2	"Guardian1"

extern LARGE_INTEGER	g_liStagePauseTime;

CFcWorld::CFcWorld()
{
	int i;

	m_pMapInfo=NULL;
	m_pObjectImpl=NULL;
	m_pTroopManager=NULL;
	m_pPropManager=NULL;
	m_pTriggerCommander = NULL;	
	m_pAbilityManager = NULL;

	m_nLightIndex = -1;

	m_bLoad = false;	

	m_pNavigationMesh = NULL;
	m_pNavCollisionMesh = NULL;
	m_nMissionFinish = MISSION_FINISH_TYPE_NON;

	m_nNumTimeMark = 0;
	for( i=0; i<MAX_TIME_MARK; i++ )
	{
		m_TimeMark[i] = INVALID_TIME_MARK;
	}
	m_nCamIndex[0] = m_nCamIndex[1] = -1;
    m_nFxSimplePlay = -1;

	m_nCurHitSignal = 0;
	for( i = 0; i < MAX_HIT_SIGNAL_COUNT; i++ )
	{
		m_HitSignal[ i ].m_nID = SIGNAL_TYPE_HIT_TEMP;
		m_HitSignal[ i ].m_NumParam = HIT2_SIGNAL_LENGTH;
		m_HitSignal[ i ].m_pParam = new int[ HIT2_SIGNAL_LENGTH ];
		memset( m_HitSignal[ i ].m_pParam, 0, sizeof( int ) * HIT2_SIGNAL_LENGTH );
		m_HitSignal[ i ].m_HitLength = 1;
		m_HitSignal[ i ].m_HitAttackPoint = 100;
		m_HitSignal[ i ].m_HitParticle = 0;
		m_HitSignal[ i ].m_HitGravityFrame = 0;
	}

	m_nNumNavEventInfo = 0;
	m_pNavEventInfo = NULL;

	m_iDynamicFogID = -1;
	m_iDarkDynamicFogID = -1;
	m_nStageLevel = 0;

	m_bBackupForRM = false;
	m_bBackupAIForRM = false;
	m_nRMStartTick = 0;
	m_nBlizzardEffectIdx = -1;
	m_nBlizzardEffectTexIdx = -1;
	m_nVigBlurEffectIdx = -1;

	m_nTrueOrbTimer = 0;

	m_bChangeLightStart = false;
	m_bChangeFogStart = false;
	m_bChangeSceneStart = false;

	m_bIsDemoSkip = false;
	m_nDemoType = -1;

	m_bIsHeroEventBlockArea = false;

	m_pCurStageResultInfo = NULL;
	m_nBGMID = -1;
}

CFcWorld::~CFcWorld()
{
	int i;

	Clear();

	for( i = 0; i < MAX_HIT_SIGNAL_COUNT; i++ )
	{
		delete [] m_HitSignal[ i ].m_pParam;
		m_HitSignal[ i ].m_pParam = NULL;
	}
}

void CFcWorld::Clear()
{
	
	m_nStageLevel = 0;
	m_nReducePhysicsDamage = 0;
	g_BsKernel.GetInstance().GetBillboardMgr()->ReleaseDeviceData();
	SAFE_DELETE( m_pMapInfo );
	SAFE_DELETE( m_pObjectImpl );
	SAFE_DELETE(m_pTroopManager);
	SAFE_DELETE( m_pPropManager );

	SAFE_DELETE( m_pTriggerCommander );
	SAFE_DELETE( m_pAbilityManager );
	SAFE_DELETE( m_pNavigationMesh );
	SAFE_DELETE( m_pNavCollisionMesh );

	CInputPad::GetInstance().ClearAttachObject();
	m_HeroHandle.Identity();

	ASData::DeleteAllASData();
	CAniInfoData::DeleteAllAniInfoData();
	if( m_nLightIndex != -1 )
	{
		g_BsKernel.DeleteObject( m_nLightIndex );
		m_nLightIndex = -1;
	}

	ReleaseParticleList();

	CFcBaseObject::ReleaseObjects();
	CFcAIObject::ReleaseObjects();
	CFcTroopAIObject::ReleaseObjects();

	m_nNumNavEventInfo = 0;
	SAFE_DELETEA( m_pNavEventInfo );

	m_bLoad = false;

	m_nMissionFinish = MISSION_FINISH_TYPE_NON;

	g_BsKernel.DeleteWorld();

	int i, nSize;
	
	nSize  = m_vecLightSet.size();
	for(i=0; i<nSize; i++ )
	{
		delete m_vecLightSet[i].pInfo;
	}
	m_vecLightSet.clear();

	m_nNumTimeMark = 0;
	for( i=0; i<MAX_TIME_MARK; i++ )
	{
		m_TimeMark[i] = INVALID_TIME_MARK;
	}
	m_ShowExtraParam.Clear();

	// This is somewhat redundant because CFantasyCore::ReleaseStage already called CFcFxManager::Reset.

	/*if (m_nFxSimplePlay != -1) {
		g_pFcFXManager->SendMessage(m_nFxSimplePlay, FX_DELETE_OBJECT);
		m_nFxSimplePlay = -1;
	}*/
	SAFE_DELETE_FX(m_nFxSimplePlay,FX_TYPE_SIMPLE_PLAY);

	SAFE_DELETE_FX(m_iDynamicFogID,FX_TYPE_DYNAMICFOG);
	m_iDynamicFogID = -1;

	SAFE_DELETE_FX(m_iDarkDynamicFogID,FX_TYPE_DYNAMICFOG);
	m_iDarkDynamicFogID = -1;

	nSize = m_vecInitGameObj.size();

	for( i = 0; i < nSize; i++) {

		int nPhySize = m_vecInitGameObj[i].vecPhysicsInfo.size();
		for(int j = 0; j < nPhySize; j++) {
			CFcPhysicsLoader::ClearPhysicsData( m_vecInitGameObj[i].vecPhysicsInfo[j].pData );
		}
		m_vecInitGameObj[i].vecPhysicsInfo.clear();

		if( m_vecInitGameObj[i].pCollisionMesh ) {
			CFcPhysicsLoader::ClearCollisionMesh(  m_vecInitGameObj[i].pCollisionMesh );
		}
		if( m_vecInitGameObj[i].pRagdollSetting ) {
			CFcPhysicsLoader::ClearRagdollSetting( m_vecInitGameObj[i].pRagdollSetting );
		}
	}
	m_vecInitGameObj.clear();
	
	CameraObjHandle CamHandle;

	CamHandle.Identity();
	for( i = 0; i < 2; i++) {
		if( m_nCamIndex[i] != -1 ) {
			g_BsKernel.DeleteObject( m_nCamIndex[i] );
			m_nCamIndex[i] = -1;
		}		
		CFcBaseObject::SetCameraObjectHandle( i, CamHandle );
	}

	// Delete the water objects

	for ( std::vector<int>::iterator it = m_vecWaterList.begin(); it != m_vecWaterList.end(); ++it )
	{
		g_BsKernel.DeleteObject( *it );
	}

	m_vecWaterList.clear();

	// Clear out any data that was added to the water object

	g_BsKernel.ResetWaterObject();	
	g_BsKernel.ResetDecalObject();

	m_vecMissionObjective.clear();
	m_vecResultCalcTroops.clear();

	g_FCGameData.cFinishRealtimeMovieFileName[0] = NULL;
	int size = m_vecFxObj.size();
	if( size > 0 )
	{
		for( int i = 0 ; i < size ; i++ )
		{
			int tmp = m_vecFxObj[i];
			g_BsKernel.ReleaseFXTemplate( tmp );
		}
		m_vecFxObj.clear();
	}

	DisableBlizzardEffect();

	memset( m_nUseParticleTable, 0, sizeof( int ) * MAX_PARTICLE_LOAD_COUNT );
	memset( m_nUseFXTable, 0, sizeof( int ) * MAX_FX_LOAD_COUNT );

	nSize = ( int )m_LoadedSkinList.size();
	for( i = 0; i < nSize; i++ )
	{
		g_BsKernel.ReleaseSkin( m_LoadedSkinList[ i ] );
	}
	m_LoadedSkinList.clear(); 

	nSize = ( int )m_LoadedAniList.size();
	for( i = 0; i < nSize; i++ ) {
		SAFE_RELEASE_ANI(m_LoadedAniList[i]);
	}

	g_pFcFXManager->ReleasePreLoadedByChar();

	m_LoadedAniList.clear();
	m_vecBigSizeScanner.clear();
	SAFE_DELETE_FX(m_nFXIndex, FX_TYPE_LENSFLARE);
	//g_pFcFXManager->SendMessage(m_nFXIndex, FX_DELETE_OBJECT );
	m_vecTroopEvnetArea.clear();

	CFcProjectile::ReleaseSignal();

#if defined(ENABLE_FULL_STAGE_CLEANOUT)
	g_pSoundManager->DeleteLoadedMissionWaveBank();
#endif

	m_bBackupForRM = false;
	m_vecBackupTroopsForRM.clear();
	m_bBackupAIForRM = false;
	m_vecBackupTroopsDisableAIForRM.clear();

	m_nDemoType = -1;
	m_nTrueOrbTimer = 0;

	m_bChangeLightStart = false;
	m_bChangeFogStart = false;
	m_bChangeSceneStart = false;

	m_bUseTrueOrb = false;
	m_vecDamagedTrueOrbTroops.clear();
	m_nBGMID = -1;

	g_FCGameData.bCheatRank = false;
	g_FCGameData.SpecialSelStageIdforTrigger = STAGE_ID_NONE;
}

void CFcWorld::MainHeroFxLoading( int p_nClassId, bool p_bPlayer /*= true*/ )
{
	switch( p_nClassId )
	{
	case CFcGameObject::Class_ID_Hero_Aspharr:
		{
			g_pFcFXManager->PreLoadByChar(0, p_bPlayer);
		}
		break;
	case CFcGameObject::Class_ID_Hero_Inphy:
		{
			g_pFcFXManager->PreLoadByChar(1, p_bPlayer);
		}
		break;
	case CFcGameObject::Class_ID_Hero_Klarrann:
		{
			g_pFcFXManager->PreLoadByChar(6, p_bPlayer);
		}
		break;
	case CFcGameObject::Class_ID_Hero_VigkVagk:
		{
			g_pFcFXManager->PreLoadByChar(5, p_bPlayer);
		}
		break;
	case CFcGameObject::Class_ID_Hero_Myifee:
		{
			g_pFcFXManager->PreLoadByChar(2, p_bPlayer);
		}
		break;
	case CFcGameObject::Class_ID_Hero_Dwingvatt:
		{
			g_pFcFXManager->PreLoadByChar(4, p_bPlayer);
		}
		break;
	case CFcGameObject::Class_ID_Hero_Tyurru:
		{
			g_pFcFXManager->PreLoadByChar(3, p_bPlayer);
		}
		break;
	}
	
}

int CFcWorld::LoadWorld(const char *pFileName, int nStageLevel)
{
	char cStr[256];
	sprintf( cStr, "LoadWorld %s", pFileName );
	g_BsMemChecker.Start( cStr );

	char *pFullName;
	float fWidth, fHeight;

	char cMissionName[32];
	strcpy( cMissionName, g_FCGameData.cMapFileName );
	char* cTempStr = strstr( cMissionName, "." );
	if( cTempStr )
		cTempStr[0] = NULL;

	char cPakStr[256];
	g_BsMemChecker.Start( "LoadWorld clear" );
	StartTimeMark( "LoadWorld Start\n" );
	NextTimeMark( "LoadWorld Start!\n" );
	Clear();
	m_nGuardianKillCount = 0;
	m_nGuardianOrb = 0;
	g_BsMemChecker.End();

	m_pCurStageResultInfo = NULL;

	int nHeroID = g_MissionToSoxID[g_FCGameData.nPlayerType];
	HeroRecordInfo* pHeroRecordInfo = g_FCGameData.tempFixedSave.GetHeroRecordInfo(nHeroID);
	if(pHeroRecordInfo)
	{
		StageResultInfo* pStageResultInfo = pHeroRecordInfo->GetStageResultInfo(g_FCGameData.SelStageId);
		if(pStageResultInfo)
		{
			m_pCurStageResultInfo = pStageResultInfo;
		}
	}
	if( m_pCurStageResultInfo == NULL )
		m_pCurStageResultInfo = &(g_FCGameData.stageInfo);

//_CrtDumpMemoryLeaks();

	NextTimeMark( "LoadWorld Clear\n" );

	g_BsMemChecker.Start( "TroopManager Init" );
	m_pTroopManager = new CFcTroopManager();
	m_nStageLevel = nStageLevel;
	if( m_pTroopManager->Initialize() == false )
	{
		BsAssert( 0 && "Troop Init" );
		return 0;
	}
	g_BsMemChecker.End();

	g_BsMemChecker.Start( "Load Map Info" );
	NextTimeMark( "LoadWorld TroopManager\n" );
	
	// Map 데이터 로드
    CBsKernel::GetInstance().chdir("map");
	pFullName=g_BsKernel.GetFullName(pFileName);
	VOID *pData;
	DWORD dwFileSize;

	g_BsResChecker.AddResList( pFullName );
	if(FAILED(CBsFileIO::LoadFile( pFullName, &pData, &dwFileSize)))
		BsAssert( 0 );

	BMemoryStream Stream(pData, dwFileSize);
	m_pMapInfo=new CBSMapCore();
	m_pMapInfo->SetMapFileInfo( pFullName );
	m_pMapInfo->LoadAndProcessFile(&Stream);
	CBsFileIO::FreeBuffer(pData);

	NextTimeMark( "LoadWorld LoadMapInfo\n" );

	CBsKernel::GetInstance().chdir("..");
	g_BsMemChecker.End();

	g_BsMemChecker.Start( "Init Propmanager" );
	fWidth=m_pMapInfo->GetXSize()*DISTANCE_PER_CELL;
	fHeight=m_pMapInfo->GetYSize()*DISTANCE_PER_CELL;
//	int nPropZoneSize = m_pMapInfo->GetPropZoneManager()->m_nZoneSize;

	// Prop Manager 세팅 
	m_pPropManager = new CFcPropManager();
	if( m_pPropManager->Initialize( fWidth, fHeight, PROP_ZONE_SIZE ) == false )
	{
		BsAssert( 0 && "Prop Init" );
	}
	g_BsMemChecker.End();
	NextTimeMark( "LoadWorld PropManager\n" );

	VectorFxStr vecTmpFxStringBuffer;
	// Prop 데이터 로드

	// 맵에서 사용하는 프랍만 로딩한다.
	CBsKernel::GetInstance().chdir("prop");
	g_BsMemChecker.Start( "Load prop data" );
	m_pPropManager->LoadTypeDat( "SkinList.dat", m_pMapInfo, &vecTmpFxStringBuffer);		// \\data\\prop
	g_BsMemChecker.End();
	g_BsMemChecker.Start( "Load prop attribute" );
	m_pPropManager->LoadBSAttr( "PropAttr.bsattr" );
	CBsKernel::GetInstance().chdir("..");
	g_BsMemChecker.Start( "LoadWorld clear" );

	g_BsMemChecker.Start( "Create height field" );
	NextTimeMark( "LoadWorld LoadPropData\n" );

	// HeightField 만들기
	CBsKernel::GetInstance().chdir("map");
	g_BsKernel.CreateHeightField(m_pMapInfo->GetXSize(), m_pMapInfo->GetYSize(), m_pMapInfo->GetHeightBuffer(), (const char **)m_pMapInfo->GetLayerStr(),
		m_pMapInfo->GetLightMapFileName(),m_pMapInfo->GetMulColorMap(), m_pMapInfo->GetAddColorMap());
	g_BsKernel.chdir("..");
	
	NextTimeMark( "LoadWorld CreateHeightField\n" );
	g_BsMemChecker.End();
	g_BsMemChecker.Start( "Init unit scanner" );

	m_pObjectImpl=new ObjectImpl;
//	m_pObjectImpl->ObjectScanner.Initialize(fWidth, fHeight, fWidth/4.0f, fHeight/4.0f, 300.0f);
	m_pObjectImpl->ObjectScanner.Initialize(fWidth, fHeight, fWidth/100.f, fHeight/100.f, 300.0f);

	NextTimeMark( "LoadWorld InitScanner\n" );
	g_BsMemChecker.End();
	g_BsMemChecker.Start( "Init troop AI script" );

	if( m_pMapInfo->GetAIFileName() != NULL )
	{
//	BsAssert( m_pMapInfo->GetAIFileName()[0] != NULL && "TroopAI is not Set In Map file"  );
		if( CFcTroopAIObject::InitLoadScript( m_pMapInfo->GetAIFileName() ) == false )
		{
			BsAssert( 0 && "Cannot load TROOP AI Script" );
//		return 0;
		}
	}
	g_BsMemChecker.End();
	g_BsMemChecker.Start( "Create navigation info" );
	NextTimeMark( "LoadWorld AIScript\n" );
	CreateNavigationMesh();			// Troop 생성전에 있어야 한다
	NextTimeMark( "LoadWorld CreateNavigationMesh\n" );
	g_BsMemChecker.End();
	g_BsMemChecker.Start( "Init camera" );

	char cDefaultCameraName[64], cUserCameraName[64];
	strcpy( cDefaultCameraName, "default.cam" );
	if( m_pMapInfo->GetCameraFileName() )
		strcpy( cUserCameraName, m_pMapInfo->GetCameraFileName() );
	else
		strcpy( cUserCameraName, "user.cam" );

	g_FCGameData.nPlayerCount = CalcPlayerCount();

	CreateCamera( g_FCGameData.nPlayerCount, cDefaultCameraName, cUserCameraName );
	NextTimeMark( "LoadWorld InitCam\n" );
	g_BsMemChecker.End();
	g_BsMemChecker.Start( "Init sky" );
	CreateSky();

	NextTimeMark( "LoadWorld InitSky\n" );
	g_BsMemChecker.End();

	g_BsMemChecker.Start( "Init billboard props" );
	m_pPropManager->InitBillboardProps();	// 캐릭터 빌보드를 위해서 빌보드매니저 초기화를 여기에서..
	g_BsMemChecker.End();

	// InitStage에서 초기화 한것 여기서 dismount, PopGroup 해야 한다.;;;;;
	g_BsResChecker.PopGroup();
	if( g_hPackedRes1 )
	{
		CBsFileIO::BsDismountPackFile( g_hPackedRes1 );
		g_hPackedRes1 = NULL;
	}
/*
	if( g_hPackedTex1 )
	{
		CBsFileIO::BsDismountPackFile( g_hPackedTex1 );
		g_hPackedTex1 = NULL;
	}
*/
	g_BsMemChecker.Start( "Create objects" );
	sprintf( cPakStr, "d:\\data\\PackedResources\\%s_Char_Res.pak", cMissionName );
	HANDLE hCharRes = CBsFileIO::BsMountCompressedPackFile( cPakStr );
	sprintf( cPakStr, "d:\\data\\PackedResources\\%s_Char_Tex.pak", cMissionName );
	g_hPackedCharTex = CBsFileIO::BsMountCompressedPackFile( cPakStr, true);

	sprintf( cPakStr, "d:\\data\\PackedResources\\%s_Res2_Res.pak", cMissionName );
	HANDLE hRes2 = CBsFileIO::BsMountCompressedPackFile( cPakStr );
	sprintf( cPakStr, "d:\\data\\PackedResources\\%s_Res2_Tex.pak", cMissionName );
	g_hPackedTex2 = CBsFileIO::BsMountCompressedPackFile( cPakStr, true);

	g_BsResChecker.SetGroup( RES_GROUP_RES2 );
	g_BsResChecker.EnableWriteOnlyFolder( true );		// It must be use before AddUseFolder()
	g_BsResChecker.AddUseFolder( "data\\char" );
	g_BsResChecker.AddUseFolder( "data\\UnitBillboard" );

	g_BsResChecker.AddIgnoreFileList( "Cubemap.36t" );
	g_BsResChecker.AddIgnoreFileList( "Cubemap1.36t" );

	CreateObjects();			// Hero도 이 안에서 등록, 어떤 Hero를 Player로 할 것인지 알려주는 파라미터 필요
	g_BsMemChecker.End();

	// Add guardian res
#ifdef ENABLE_RES_CHECKER
	switch( g_FcWorld.GetHeroHandle()->GetHeroClassID() )
	{
	case Class_ID_Hero_Aspharr:
	case Class_ID_Hero_Inphy:
	case Class_ID_Hero_Tyurru:
		g_BsResChecker.AddResFolderList( "D:\\data\\char\\C_LS_SL1\\" );
		g_BsResChecker.AddResFolderList( "D:\\data\\char\\C_LS_SL1\\Parts\\" );
		g_BsResChecker.AddResFolderList( "D:\\data\\char\\C_LS_SL4\\" );
		g_BsResChecker.AddResFolderList( "D:\\data\\char\\C_LS_SL4\\Parts\\" );
		// Billboard타입 0으로 고정 나중에 형섭씨가 타입 바꿀 예정!!
		g_BsResChecker.AddResFolderList( "D:\\data\\UnitBillboard\\C_LS_SL1\\C_LS_SL1_A_BILL\\" );
		g_BsResChecker.AddResFolderList( "D:\\data\\UnitBillboard\\C_LS_SL4\\C_LS_SL4_A_BILL\\" );

		g_BsResChecker.AddResList( "D:\\data\\as\\C_LS_SL1_OH.as" );
		g_BsResChecker.AddResList( "D:\\data\\as\\C_LS_SL1_OH.info" );
		g_BsResChecker.AddResList( "D:\\data\\as\\C_LS_SL1_SP.as" );
		g_BsResChecker.AddResList( "D:\\data\\as\\C_LS_SL1_SP.info" );
		g_BsResChecker.AddResList( "D:\\data\\as\\C_LS_SL1_TH.as" );
		g_BsResChecker.AddResList( "D:\\data\\as\\C_LS_SL1_TH.info" );
		g_BsResChecker.AddResList( "D:\\data\\as\\C_LS_SL4_AR.as" );
		g_BsResChecker.AddResList( "D:\\data\\as\\C_LS_SL4_AR.info" );

		g_BsResChecker.AddResList( "D:\\data\\AI\\unit\\C_LS_SL1_OH_DEFAULT.aib" );
		g_BsResChecker.AddResList( "D:\\data\\AI\\unit\\C_LS_SL1_SP_DEFAULT.aib" );
		g_BsResChecker.AddResList( "D:\\data\\AI\\unit\\C_LS_SL1_TH_DEFAULT.aib" );
		g_BsResChecker.AddResList( "D:\\data\\AI\\unit\\C_LS_SL4_AR_DEFAULT.aib" );

		break;

	case Class_ID_Hero_Myifee:
		g_BsResChecker.AddResFolderList( "D:\\data\\char\\C_LS_WR1\\" );
		g_BsResChecker.AddResFolderList( "D:\\data\\char\\C_LS_WR1\\Parts\\" );
		g_BsResChecker.AddResFolderList( "D:\\data\\char\\C_LS_SL1\\" );
		g_BsResChecker.AddResFolderList( "D:\\data\\char\\C_LS_SL1\\Parts\\" );
		g_BsResChecker.AddResFolderList( "D:\\data\\char\\C_LS_SL4\\" );
		g_BsResChecker.AddResFolderList( "D:\\data\\char\\C_LS_SL4\\Parts\\" );

		// Billboard타입 0으로 고정 나중에 형섭씨가 타입 바꿀 예정!!
		g_BsResChecker.AddResFolderList( "D:\\data\\UnitBillboard\\C_LS_WR1\\C_LS_WR1_A_BILL\\" );	// B,C,D 자동 등록됨
		g_BsResChecker.AddResFolderList( "D:\\data\\UnitBillboard\\C_LS_SL1\\C_LS_SL1_A_BILL\\" );
		g_BsResChecker.AddResFolderList( "D:\\data\\UnitBillboard\\C_LS_SL4\\C_LS_SL4_A_BILL\\" );

		g_BsResChecker.AddResList( "D:\\data\\as\\C_LS_WR1_OH.as" );
		g_BsResChecker.AddResList( "D:\\data\\as\\C_LS_WR1_OH.info" );
		g_BsResChecker.AddResList( "D:\\data\\as\\C_LS_WR1_TH.as" );
		g_BsResChecker.AddResList( "D:\\data\\as\\C_LS_WR1_TH.info" );
		g_BsResChecker.AddResList( "D:\\data\\as\\C_LS_SL1_SP.as" );
		g_BsResChecker.AddResList( "D:\\data\\as\\C_LS_SL1_SP.info" );
		g_BsResChecker.AddResList( "D:\\data\\as\\C_LS_SL4_AR.as" );
		g_BsResChecker.AddResList( "D:\\data\\as\\C_LS_SL4_AR.info" );

		g_BsResChecker.AddResList( "D:\\data\\AI\\unit\\C_LS_WR1_OH_DEFAULT.aib" );
		g_BsResChecker.AddResList( "D:\\data\\AI\\unit\\C_LS_WR1_TH_DEFAULT.aib" );
		g_BsResChecker.AddResList( "D:\\data\\AI\\unit\\C_LS_SL1_SP_DEFAULT.aib" );
		g_BsResChecker.AddResList( "D:\\data\\AI\\unit\\C_LS_SL4_AR_DEFAULT.aib" );
		break;

	case Class_ID_Hero_Klarrann:
	case Class_ID_Hero_VigkVagk:
	case Class_ID_Hero_Dwingvatt:
		break;
	}

	// 맵에서 등록할 수 없고 실기무비에서만 나오는 녀석들은 여기서 강제로 등록시킵니다.
	if( _strcmpi( g_FCGameData.cMapFileName, "ev_as.bsmap" ) == 0 )
	{
		g_BsResChecker.AddResFolderList( "D:\\data\\char\\C_DN_GB1\\" );
		g_BsResChecker.AddResFolderList( "D:\\data\\char\\EV_MS1\\" );
	}
	else if( _strcmpi( g_FCGameData.cMapFileName, "ev_in.bsmap" ) == 0 )
	{
		g_BsResChecker.AddResFolderList( "D:\\data\\char\\C_DN_GB1\\" );
		g_BsResChecker.AddResFolderList( "D:\\data\\char\\EV_MS1\\" );
	}
	else if( _strcmpi( g_FCGameData.cMapFileName, "vf_in.bsmap" ) == 0 )
	{
		g_BsResChecker.AddResFolderList( "D:\\data\\char\\C_LN_LS1\\" );
		g_BsResChecker.AddResFolderList( "D:\\data\\char\\C_LN_LS1\\PH_PARTS\\" );
	}
	else if( _strcmpi( g_FCGameData.cMapFileName, "vg_dw.bsmap" ) == 0 )
	{
		g_BsResChecker.AddResFolderList( "D:\\data\\char\\C_DN_GB2\\" );
	}
	else if( _strcmpi( g_FCGameData.cMapFileName, "vz_dw2.bsmap" ) == 0 )
	{
		g_BsResChecker.AddResFolderList( "D:\\data\\char\\C_DN_GB1\\" );
	}
	else if( _strcmpi( g_FCGameData.cMapFileName, "wy_as.bsmap" ) == 0 ||
			strcmpi( g_FCGameData.cMapFileName, "wy_in.bsmap" ) == 0 ||
			strcmpi( g_FCGameData.cMapFileName, "wy_kl.bsmap" ) == 0 ||
			strcmpi( g_FCGameData.cMapFileName, "wy_ty.bsmap" ) == 0 )
	{
		g_BsResChecker.AddResFolderList( "D:\\data\\char\\C_DN_GK3\\" );
		g_BsResChecker.AddResFolderList( "D:\\data\\char\\C_DN_GK3\\PH_PARTS\\" );
		g_BsResChecker.AddResFolderList( "D:\\data\\char\\C_DN_GK4\\" );
		g_BsResChecker.AddResFolderList( "D:\\data\\char\\C_DN_GK4\\PH_PARTS\\" );
	}
	else if( _strcmpi( g_FCGameData.cMapFileName, "awed.bsmap" ) == 0 )
	{
		g_BsResChecker.AddResFolderList( "D:\\data\\char\\C_LN_LS1\\" );
		g_BsResChecker.AddResFolderList( "D:\\data\\char\\C_LN_LS1\\PH_PARTS\\" );
	}
	else if( _strcmpi( g_FCGameData.cMapFileName, "ced01.bsmap" ) == 0 )
	{
		g_BsResChecker.AddResFolderList( "D:\\data\\char\\C_DN_GB1\\" );
	}
	else if( _strcmpi( g_FCGameData.cMapFileName, "yw_in.bsmap" ) == 0 )
	{
		g_BsResChecker.AddResList( "D:\\data\\sky\\S_VZ_MY0_00.SKIN" );
	}

#endif

	g_BsResChecker.PopGroup();

	g_BsResChecker.SetGroup( RES_GROUP_CHAR );
	g_BsResChecker.EnableWriteOnlyFolder( false );
	g_BsResChecker.PopGroup();
	g_BsResChecker.ClearIgnoreFileList();

	if( hCharRes )
		CBsFileIO::BsDismountPackFile( hCharRes );
/*
	if( hCharTex )
		CBsFileIO::BsDismountPackFile( hCharTex );
*/
	if( hRes2 )
		CBsFileIO::BsDismountPackFile( hRes2 );

/*
	if( g_hPackedTex2 )
	{
		CBsFileIO::BsDismountPackFile( g_hPackedTex2 );
		g_hPackedTex2 = NULL;
	}
*/

	sprintf( cPakStr, "d:\\data\\PackedResources\\%s_Res3_Res.pak", cMissionName );
	g_hPackedRes3 = CBsFileIO::BsMountCompressedPackFile( cPakStr );
	sprintf( cPakStr, "d:\\data\\PackedResources\\%s_Res3_Tex.pak", cMissionName );
	g_hPackedTex3 = CBsFileIO::BsMountCompressedPackFile( cPakStr, true);

	g_BsMemChecker.Start( "LoadWorld clear" );
	g_BsResChecker.SetGroup( RES_GROUP_RES3 );

	NextTimeMark( "LoadWorld CreateObject\n" );
	g_BsMemChecker.Start( "Load particle" );
	LoadParticleList();
	
//#ifdef _XBOX
	// RealMovie FX

	

	if( _strcmpi( g_FCGameData.cMapFileName, "vf_in.bsmap" ) == 0 )
	{
		FxString* tmp2 = new FxString;
		strcpy( tmp2->_strBuffer, "event_YUABE02_N.bfx" );
        vecTmpFxStringBuffer.push_back(tmp2);
		tmp2 = new FxString;
		strcpy( tmp2->_strBuffer, "event_YUABE01_N.bfx" );
		vecTmpFxStringBuffer.push_back(tmp2);
	}
	else if( _strcmpi( g_FCGameData.cMapFileName, "He_My.bsmap" ) == 0 )
	{
		FxString* tmp2 = new FxString;
		strcpy( tmp2->_strBuffer, "event_VGM02_N.bfx" );
		vecTmpFxStringBuffer.push_back(tmp2);
		tmp2 = new FxString;
		strcpy( tmp2->_strBuffer, "event_VGM03_N.bfx" );
		vecTmpFxStringBuffer.push_back(tmp2);
		tmp2 = new FxString;
		strcpy( tmp2->_strBuffer, "event_VGM04_N.bfx" );
		vecTmpFxStringBuffer.push_back(tmp2);
	}
	else if( _strcmpi( g_FCGameData.cMapFileName, "CED01.bsmap" ) == 0 )
	{
		FxString* tmp2 = new FxString;
		strcpy( tmp2->_strBuffer, "event_OrbCross01_N.bfx" );
		vecTmpFxStringBuffer.push_back(tmp2);
	}
	else if( _strcmpi( g_FCGameData.cMapFileName, "ph_kl.bsmap" ) == 0 )
	{
		FxString* tmp2 = new FxString;
		strcpy( tmp2->_strBuffer, "event_darkhall01_N.bfx" );
		vecTmpFxStringBuffer.push_back(tmp2);
		tmp2 = new FxString;
		strcpy( tmp2->_strBuffer, "event_sun01_N.bfx" );
		vecTmpFxStringBuffer.push_back(tmp2);
	}
	else if( _strcmpi( g_FCGameData.cMapFileName, "ph_my.bsmap" ) == 0 )
	{
		FxString* tmp2 = new FxString;
		strcpy( tmp2->_strBuffer, "event_sun01_N.bfx" );
		vecTmpFxStringBuffer.push_back(tmp2);
	}
	else if( _strcmpi( g_FCGameData.cMapFileName, "ph_ty.bsmap" ) == 0 )
	{
		FxString* tmp2 = new FxString;
		strcpy( tmp2->_strBuffer, "event_sun01_N.bfx" );
		vecTmpFxStringBuffer.push_back(tmp2);
	}
	else if( _strcmpi( g_FCGameData.cMapFileName, "yw_as.bsmap" ) == 0 )
	{
		FxString* tmp2 = new FxString;
		strcpy( tmp2->_strBuffer, "event_YUABE03bigLight_N.bfx" );
		vecTmpFxStringBuffer.push_back(tmp2);
	}
	else if( _strcmpi( g_FCGameData.cMapFileName, "yw_in.bsmap" ) == 0 )
	{
		FxString* tmp2 = new FxString;
		strcpy( tmp2->_strBuffer, "event_YUIE01_N.bfx" );
		vecTmpFxStringBuffer.push_back(tmp2);
	}

	CreateFxToPlay(&vecTmpFxStringBuffer);		// CreateProps 전에 와야 한다.
//#endif //_XBOX

	int size = vecTmpFxStringBuffer.size();
	if( size > 0 )
	{
		for( int i = 0 ; i < size ; i++ )
		{
			FxString* tmp = vecTmpFxStringBuffer[i];
			SAFE_DELETE(tmp);
		}
		vecTmpFxStringBuffer.clear();
	}
	g_BsMemChecker.End();

	NextTimeMark( "LoadWorld CreateFX\n" );

	g_BsMemChecker.Start( "Create prop objects" );
	CreateProps();
	NextTimeMark( "LoadWorld CreateProp\n" );
	g_BsMemChecker.End();

	g_BsMemChecker.Start( "Create HeightField Actor" );
	CreateHeightFieldActor( );
	g_BsMemChecker.End();

	g_BsMemChecker.Start( "Create river" );
	CBsKernel::GetInstance().chdir("map");
	CreateWater( pFileName );
	CBsKernel::GetInstance().chdir("..");
	g_BsMemChecker.End();

	NextTimeMark( "LoadWorld CreateWater\n" );

	g_BsMemChecker.Start( "Create light" );
	CreateLight();

	NextTimeMark( "LoadWorld CreateLight\n" );

	CBsKernel::GetInstance().chdir("set");
	LoadLightSet( "LightSet.txt" );
	CBsKernel::GetInstance().chdir("..");

	g_BsMemChecker.End();

	NextTimeMark( "LoadWorld LoadLightSet\n" );

	g_BsMemChecker.Start( "Load trigger" );

	CBsKernel::GetInstance().chdir("map");
	if( LoadTrigger( pFileName ) == false )
	{
		BsAssert( 0 && "Trigger load fail" );
	}
	CBsKernel::GetInstance().chdir("..");
	g_BsMemChecker.End();

	NextTimeMark( "LoadWorld LoadTrigger\n" );

	g_BsMemChecker.Start( "Create abilitymanager" );

	m_pAbilityManager = new CFcAbilityManager();
	if(m_pAbilityManager)
		m_pAbilityManager->Initialize();

	NextTimeMark( "LoadWorld AbilityManger\n" );
	g_BsMemChecker.End();

	g_BsMemChecker.Start( "Load text" );
	char* pUserTextFileName = (char*)m_pMapInfo->GetTextFileName();
	if( pUserTextFileName == NULL )
		g_InterfaceManager.SetUserTextTable( "UserTextTable0000_KOR.txt" );
	else
		g_InterfaceManager.SetUserTextTable( m_pMapInfo->GetTextFileName() );
	g_BsMemChecker.End();

	NextTimeMark( "LoadWorld InitTextTable\n" );

	g_BsMemChecker.Start( "Create dynamic fog" );
	//------------------------------Dynamic Fog
	CreateDynamicFog();
	g_BsMemChecker.End();
	
//-----------------------------------------
//	CameraObjHandle CamHandle = CFcBaseObject::GetCameraObjectHandle( 0 );
//	CamHandle->PlayEventSeq( 20 );
	m_pTroopManager->PostInitialize();

	g_BsMemChecker.Start( "Load sound data" );
	g_pSoundManager->LoadMissionWaveBank( pFileName );
	NextTimeMark( "LoadWorld SoundData\n" );

	ResetProcessTick();
	QueryPerformanceCounter( &m_liStageStartTime );
	memset(&g_liStagePauseTime,0,sizeof(LARGE_INTEGER));

//	m_liStagePauseTime = 0;
//	m_liStartPauseTime = 0;
	g_BsMemChecker.End();


// 동영상 오브 전용
//CFcWorld::GetInstance().GetHeroHandle()->AddTrueOrbSpark( 10000 );

/*
	CCrossVector HeroCross = *CFcWorld::GetInstance().GetHeroHandle()->GetCrossVector();
	HeroCross.SetPosition( HeroCross.GetPosition() + D3DXVECTOR3(0,300,0));
	int nShadowSkin = LoadMeshData(-1, "shadowtest.skin");
	BaseObjHandle Shadow = CFcBaseObject::CreateObject< CFcBaseObject >(&HeroCross);
	Shadow->Initialize( nShadowSkin , -1, CFcBaseObject::NORMAL);

	CCrossVector Cross  = *CFcWorld::GetInstance().GetHeroHandle()->GetCrossVector();
	Cross.SetPosition( Cross.GetPosition() + D3DXVECTOR3(0,100,0));
	nLightIndex = g_BsKernel.CreateLightObject(::POINT_LIGHT);
	g_BsKernel.UpdateObject(nLightIndex, Cross);
	D3DCOLORVALUE color;
	color.a = 3.0f;
	color.r = 0.2f;
	color.g = 0.3f;
	color.b = 1.0f;
	g_BsKernel.SendMessage(nLightIndex, BS_SET_POINTLIGHT_COLOR, (DWORD)&color);
*/

	g_BsResChecker.ClearIgnoreFolderList();
	g_BsMemChecker.End();
	m_bLoad = true;

	return 1;
}

bool CFcWorld::LoadParticleList()
{
	int nIndex;
	char szFileName[ _MAX_PATH ];
	Parser parser;
	TokenList toklist;

	parser.Create();
	parser.EnableComments( true );

	bool result = parser.ProcessSource( g_BsKernel.GetCurrentDirectory(), "ParticleList.txt", &toklist );
	if(result==false) {
		BsAssert( 0 && "Unable to read Particle List script" );
		return false;
	}
	result = parser.ProcessHeaders(&toklist);
	if(result==false) {
		BsAssert( 0 && "Error processing script header" );
		return false;
	}

	parser.ProcessMacros(&toklist);
	TokenList::iterator itr = toklist.begin();

	g_BsKernel.chdir("fx");

	while( 1 )
	{
		if(itr == toklist.end() )
			break;

		BsAssert( itr->IsInteger() );
		nIndex=itr->GetInteger();

		itr++;												
		BsAssert( itr->IsString() );
		strcpy( szFileName, itr->GetString() );

		itr++;
		BsAssert( itr->IsInteger() );

		if( ( m_nUseParticleTable[ nIndex ] ) || ( itr->GetInteger() == FCFX_USE_COMMON ) )
		{
			int nParticleGroupIndex = g_BsKernel.LoadParticleGroup( nIndex, szFileName );
			if ( nParticleGroupIndex != -1 )
				m_vecParticleGroupList.push_back( nParticleGroupIndex );
		}

		itr++;
	}

	g_BsKernel.chdir("..");

	return true;
}

void CFcWorld::ReleaseParticleList()
{
	for ( std::vector<int>::iterator it = m_vecParticleGroupList.begin(); it != m_vecParticleGroupList.end(); ++it )
	{
		SAFE_RELEASE_PARTICLE_GROUP( *it );
	}

	m_vecParticleGroupList.clear();
}

void CFcWorld::CreateHeightFieldActor() 
{
	int nXSize = m_pMapInfo->GetXSize(); 
	int nYSize = m_pMapInfo->GetYSize(); 

	if( m_pPropManager->GetDynamicProps().empty() ) {
		g_BsKernel.CreateHeightFieldActor( nXSize, nYSize, m_pMapInfo->GetHeightBuffer());
	}
	else {
		short *pHeightBuffer = new short[ (nXSize+1) * (nYSize+1) ];
		memcpy( pHeightBuffer, m_pMapInfo->GetHeightBuffer(), sizeof(short) * (nXSize+1) * (nYSize+1) );

		std::vector<CFcProp*> vecProps;
		for( int j = 1; j < nYSize; j++)
		for( int i = 1; i < nXSize; i++)
		{
			vecProps.clear();
			GetActiveProps( D3DXVECTOR3(i * 200.f, 0, j * 200.f), 400.f, vecProps );
			if( !vecProps.empty() ) continue;

			int nHeight = 0;
			if( (m_pMapInfo->GetAttr( i * 2 - 1, j * 2 - 1 ) & 2 ) ) nHeight++;
			if( 	(m_pMapInfo->GetAttr( i * 2 , j * 2 - 1 ) & 2 ) ) nHeight++;
			if( 	(m_pMapInfo->GetAttr( i * 2 - 1, j * 2  ) & 2 ) ) nHeight++;
			if( 	(m_pMapInfo->GetAttr( i * 2 , j * 2  ) & 2 ) ) nHeight++;

			pHeightBuffer[ i + j * (nXSize + 1)  ] += ( nHeight * nHeight ) * 5;	// (0,1,4,9,16)  * 5
		}

		g_BsKernel.CreateHeightFieldActor( nXSize, nYSize, pHeightBuffer);

		delete [] pHeightBuffer;
	}
}

void CFcWorld::CreateFxToPlay( IN VectorFxStr* p_VecFxStr )
{
	AvailMemoryDifference test( 0, "test" );


    if(m_nFxSimplePlay != -1)
        return;
    m_nFxSimplePlay = g_pFcFXManager->Create(FX_TYPE_SIMPLE_PLAY);

    char szFxFilePath[MAX_PATH];
    strcpy(szFxFilePath,"FxList.txt");
    g_pFcFXManager->SendMessage( m_nFxSimplePlay, FX_INIT_OBJECT, ( DWORD )szFxFilePath, ( DWORD )m_nUseFXTable );


	// Fx파일 로드.
	g_BsKernel.chdir("fx");
	int size = p_VecFxStr->size();
	for( int i = 0 ; i < size ; i++)
	{
		FxString tmpFxStr = *(*p_VecFxStr)[i];
		char szFxFilePath[256];
		
		sprintf(szFxFilePath,"%s%s",g_BsKernel.GetCurrentDirectory(), tmpFxStr._strBuffer );
		int nFxObj = g_BsKernel.LoadFXTemplate(-1, szFxFilePath);
		m_vecFxObj.push_back(nFxObj);
	}
	g_BsKernel.chdir("..");
	test.GetAvailMemDifference();
}

void CFcWorld::PlaySimpleFx(int nFxIndex,CCrossVector *pCross, int nLoop )
{
    g_pFcFXManager->SendMessage(m_nFxSimplePlay,FX_PLAY_OBJECT,nFxIndex,(DWORD)pCross, nLoop);
}

void CFcWorld::PlaySimpleLinkedFx( int nFxIndex, GameObjHandle Handle, int nBoneIndex)
{
	FX_BY_PARENT_PLAY_FOR PlayInfo;
	PlayInfo.nFxID = nFxIndex;
	PlayInfo.hParent = Handle;
//	PlayInfo.pCross = pCross;
	PlayInfo.nBoneIdx = nBoneIndex;

	g_pFcFXManager->SendMessage( m_nFxSimplePlay, FX_LINKED_PLAY_OBJECT, (DWORD)&PlayInfo );
}

void CFcWorld::StopSimpleLinkedFx( GameObjHandle Handle, int nBoneIndex )
{
	FX_BY_PARENT_PLAY_FOR PlayInfo;
	PlayInfo.hParent = Handle;
	//	PlayInfo.pCross = pCross;
	PlayInfo.nBoneIdx = nBoneIndex;

	g_pFcFXManager->SendMessage( m_nFxSimplePlay, FX_LINKED_STOP_OBJECT, (DWORD)&PlayInfo );
}

void CFcWorld::PlaySimpleFxByParent( int nFxIndex, GameObjHandle Handle, CCrossVector *pCross )
{
	FX_BY_PARENT_PLAY_FOR PlayInfo;
	PlayInfo.nFxID = nFxIndex;
	PlayInfo.hParent = Handle;
	PlayInfo.pCross = pCross;
	PlayInfo.nBoneIdx = -1;

	g_pFcFXManager->SendMessage( m_nFxSimplePlay, FX_PLAY_OBJECT_PARENT, (DWORD)&PlayInfo );
}


void CFcWorld::PlaySimpleParticleByParent( int nID, GameObjHandle hParent, bool bLoop, bool bIterate, float fScale, D3DXVECTOR3* pOffset/*=NULL*/)
{
    BsAssert(m_nFxSimplePlay != -1 && "Fx wasn't loaded");

    PARTICLE_BY_PARENT_PLAY_FOR PlayInfo;
    PlayInfo.nParticleID = nID;
    PlayInfo.hParent     = hParent;
    PlayInfo.bLoop       = bLoop;
    PlayInfo.bIterate    = bIterate;
    PlayInfo.fScale      = fScale;
    PlayInfo.vecOffset   = pOffset;

    g_pFcFXManager->SendMessage(m_nFxSimplePlay,FX_PLAY_SIMPLE_PARENT_PARTICLE,(DWORD)&PlayInfo);
}

void CFcWorld::SetAbility( int nID, GameObjHandle hParent, DWORD Param1, DWORD Param2, DWORD Param3, DWORD Param4 )
{
	m_pAbilityManager->Start( (FC_ABILITY)nID, hParent, Param1, Param2, Param3, Param4 );
	if( hParent == GetHeroHandle() )
	{
/*
		CameraObjHandle CamHandle = CFcBaseObject::GetCameraObjectHandle( 0 );
		int nSeqID = CAM_ABILITY_SEQ_START + nID;
		CamHandle->PlayEventSeq( nSeqID );
*/
	}
	switch( nID ) {
		case FC_ABILITY_WATER:			m_UseTrueOrbHandle = hParent;	m_nTrueOrbTimer = 400;	break;
		case FC_ABILITY_FIRE:			m_UseTrueOrbHandle = hParent;	m_nTrueOrbTimer = 400;	break;
		case FC_ABILITY_EARTH:			m_UseTrueOrbHandle = hParent;	m_nTrueOrbTimer = 400;	break;
		case FC_ABILITY_SOUL:			m_UseTrueOrbHandle = hParent;	m_nTrueOrbTimer = 400;	break;
		case FC_ABILITY_WIND:			m_UseTrueOrbHandle = hParent;	m_nTrueOrbTimer = 400;	break;
		case FC_ABILITY_LIGHT_1:		m_UseTrueOrbHandle = hParent;	m_nTrueOrbTimer = 400;	break;
		case FC_ABILITY_LIGHT_2:		m_UseTrueOrbHandle = hParent;	m_nTrueOrbTimer = 400;	break;
	}
}


bool CFcWorld::LoadLightSet( char* pFileName )
{
	VOID *pData;
	DWORD dwFileSize;

	if(FAILED(CBsFileIO::LoadFile( g_BsKernel.GetFullName(pFileName), &pData, &dwFileSize)))
	{
		BsAssert( 0 && "Lightset file can't load" );
		return false;
	}

	int nCnt;

	const char *  pBuff = (const char*)pData;
	int cCount = (int)dwFileSize;
	char szBuff[MAX_PATH];
	int length;

	length = ExtractTabLine( pBuff, szBuff, cCount < MAX_PATH ? cCount : MAX_PATH );
	cCount -= length;
	pBuff += length;
	nCnt = atoi(szBuff);

	m_vecLightSet.reserve( nCnt );

	for( int i=0; i<nCnt; i++ )
	{
		LIGHTSETDATA Data;

		length = ExtractTabLine( pBuff, szBuff, cCount < MAX_PATH ? cCount : MAX_PATH );
		cCount -= length;
		pBuff += length;
		Data.nID = atoi(szBuff);

		length = ExtractTabLine( pBuff, szBuff, cCount < MAX_PATH ? cCount : MAX_PATH );
		cCount -= length;
		pBuff += length;

		BFileStream bfStream( g_BsKernel.GetFullName(szBuff) );
		if( !bfStream.Valid() )
		{
			BsAssert( 0 );
			return false;
		}
		Data.pInfo = new SET_INFO;
		bfStream.Read( ( Data.pInfo), sizeof(SET_INFO), 4 );

		m_vecLightSet.push_back( Data );
	}
	CBsFileIO::FreeBuffer(pData);

	return true;
}

bool CFcWorld::ChangeFogSet( int nLightSetID, int nTick )
{
	int nCnt = m_vecLightSet.size();
	SET_INFO* pInfo = NULL;
	for( int i = 0; i < nCnt; i++ )
	{
		if( nLightSetID == m_vecLightSet[i].nID )
		{
			pInfo = m_vecLightSet[i].pInfo;
			break;
		}
	}
	BsAssert( pInfo != NULL );
	CameraObjHandle CamHandle = CFcBaseObject::GetCameraObjectHandle( 0 );
	CamHandle->ChangeFog( &(pInfo->vecFogColor), pInfo->fNear, pInfo->fFar, nTick );
	return true;
}


void CFcWorld::CreateDynamicFog()
{
	if( m_iDynamicFogID == -1 )
		m_iDynamicFogID = g_pFcFXManager->Create(FX_TYPE_DYNAMICFOG);
/*
	if( m_iDarkDynamicFogID == -1 )
		m_iDarkDynamicFogID = g_pFcFXManager->Create(FX_TYPE_DYNAMICFOG);
*/
}

void CFcWorld::ChangeLight(const D3DXVECTOR4 &vDiffuse, const D3DXVECTOR4 &vSpecular, const D3DXVECTOR4 &vAmbient, int nSec) 
{
	m_bChangeLightStart = true;

	m_vCurAmbient = *(D3DXVECTOR4*)g_BsKernel.SendMessage( g_FcWorld.GetLightIndex(), BS_GET_LIGHT_AMBIENT, 0);
	m_vCurDiffuse = *(D3DXVECTOR4*)g_BsKernel.SendMessage( g_FcWorld.GetLightIndex(), BS_GET_LIGHT_DIFFUSE, 0);
	m_vCurSpecular = *(D3DXVECTOR4*)g_BsKernel.SendMessage( g_FcWorld.GetLightIndex(), BS_GET_LIGHT_SPECULAR, 0);

	m_vIncDiffuse = (vDiffuse - m_vCurDiffuse)/float(nSec*FRAME_PER_SEC);
	m_vIncSpecular = (vSpecular - m_vCurSpecular)/float(nSec*FRAME_PER_SEC);
	m_vIncAmbient = (vAmbient - m_vCurAmbient)/float(nSec*FRAME_PER_SEC);

	m_nChangeLightSec = nSec;

	m_nChangeLightOldTick = GetProcessTick();
}


void CFcWorld::ChangeFog(const D3DXVECTOR4 &vColor, int nNear, int nFar, int nTick, int nSec) 
{
	m_bChangeFogStart = true;

	CameraObjHandle CamHandle = CFcBaseObject::GetCameraObjectHandle( 0 );
	//CamHandle->ChangeFog( &col, (float)nNear, (float)nFar, nTick );
	m_vCurFogColor = *CamHandle->GetFogColor();
	m_vIncFogColor = (vColor- m_vCurFogColor)/float(nSec*FRAME_PER_SEC);	
	m_nChangeFogNear = nNear;
	m_nChangeFogFar = nFar;
	m_nChangeFogTick = nTick;
	m_nChangeFogSec = nSec;
	m_nChangeFogOldTick = GetProcessTick();
}

void CFcWorld::ChangeScene(float fScene, float fGlow, float fBlur, int nSec)
{
	m_bChangeSceneStart = true;

	m_fCurScene  = g_BsKernel.GetSceneIntensity();
	m_fCurBlur = g_BsKernel.GetBlurIntensity();
	m_fCurGlow = g_BsKernel.GetGlowIntensity();

	m_fIncScene = (fScene - m_fCurScene)/float(nSec*FRAME_PER_SEC);
	m_fIncBlur = (fScene - m_fCurBlur)/float(nSec*FRAME_PER_SEC);
	m_fIncGlow = (fScene - m_fCurGlow)/float(nSec*FRAME_PER_SEC);

	m_nChangeSceneSec = nSec;
	m_nChangeSceneOldTick = GetProcessTick();
}

bool CFcWorld::SetEnemyAttr( AREA_INFO* pInfo, bool bSet )
{
	int nXSize = (int)( ( pInfo->fEX - pInfo->fSX ) / MAP_ATTR_SIZE ) + 1;
	int nYSize = (int)( ( pInfo->fEZ - pInfo->fSZ ) / MAP_ATTR_SIZE ) + 1;

	int nSX = (int)(pInfo->fSX / MAP_ATTR_SIZE);
	int nSY = (int)(pInfo->fSZ / MAP_ATTR_SIZE);
	int nCurX, nCurY;

	for( int i=0; i<nYSize; i++ )
	{
		nCurY = nSY + i;
		for( int j=0; j<nXSize; j++ )
		{
			nCurX = nSX + j;

			BYTE* pBuf = m_pMapInfo->GetAttrExBuf();
			BYTE Attr = *(pBuf + nCurX + nCurY * m_pMapInfo->GetAttrXSize() );
			if( bSet )
				Attr = Attr | 0x08;
			else
				Attr = Attr & 0xF7;

			*(pBuf + nCurX + nCurY * m_pMapInfo->GetAttrXSize() ) = Attr;
		}
	}
	return true;
}


void CFcWorld::SetEnableDynamicProp( bool bEnable, float fSX, float fSY, float fEX, float fEY )
{
	int nCnt = m_pPropManager->GetActivePropCount();
	for( int i=0; i<nCnt; i++ )
	{
		CFcProp* pProp = m_pPropManager->GetActiveProp( i );
		if( !pProp->IsDynamicProp() )
			continue;

		D3DXVECTOR2 Pos = pProp->GetPosV2();
		if( fSX < Pos.x && fEX > Pos.x && fSY < Pos.y && fEY > Pos.y )
		{
			CFcDynamicProp* pDynamic = (CFcDynamicProp*)pProp;
			pDynamic->SetEnable( bEnable );
		}
	}
}


void CFcWorld::CreateCamera( int nCameraCount, char* pDefaultCameraFileName, char* pUserCameraFileName )
{
	BsAssert( nCameraCount == 1 || nCameraCount == 2 );

	CameraObjHandle CamHandle;
	CCrossVector Cross;

	CamHandle = CFcBaseObject::CreateObject< CFcCameraObject >( &Cross );
	if( nCameraCount == 1 )
	{
//		m_nCamIndex[0]=CamHandle->Initialize( 0, false, 0, pDefaultCameraFileName, pUserCameraFileName );
// [beginmodify] 2006/1/16 junyash PS#1259,3249,3251, TCR#033PER multi controller
		#if 0
		m_nCamIndex[0]=CamHandle->Initialize( 0, false, 0, pDefaultCameraFileName, pUserCameraFileName );
		#else
		m_nCamIndex[0]=CamHandle->Initialize( 0, false, g_FCGameData.nEnablePadID, pDefaultCameraFileName, pUserCameraFileName );
		#endif
// [endmodify] junyash
	}
	else
	{
//		m_nCamIndex[0]=CamHandle->Initialize( 0, true, 0, pDefaultCameraFileName, pUserCameraFileName );
// [beginmodify] 2006/1/16 junyash PS#1259,3249,3251, TCR#033PER multi controller
		#if 0
		m_nCamIndex[0]=CamHandle->Initialize( 0, true, 0, pDefaultCameraFileName, pUserCameraFileName );
		#else
		m_nCamIndex[0]=CamHandle->Initialize( 0, true, g_FCGameData.nEnablePadID, pDefaultCameraFileName, pUserCameraFileName );
		#endif
// [endmodify] junyash
	}
	CFcBaseObject::SetCameraObjectHandle( 0, CamHandle );
	CamHandle->SetProjectionMatrix( 10.f, 20000.f );
	
	if( nCameraCount != 1 )
	{
		CamHandle = CFcBaseObject::CreateObject< CFcCameraObject >( &Cross );
		m_nCamIndex[1] = CamHandle->Initialize( 1, true, 1, pDefaultCameraFileName, pUserCameraFileName );
		CFcBaseObject::SetCameraObjectHandle( 1, CamHandle );
		CamHandle->SetProjectionMatrix( 10.f, 20000.f );
	}
}

void CFcWorld::CreateSky()
{
	int nSkySkin;
	BaseObjHandle SkyHandle;
	CCrossVector Cross;

	CBsKernel::GetInstance().chdir("sky");
	nSkySkin = LoadMeshData( -1, m_pMapInfo->GetSkyboxName() );
	CBsKernel::GetInstance().chdir("..");

	BsAssert( nSkySkin >= 0 && "Cannot load Skybox" );

	Cross.SetPosition(0.0f, 0.0f, 0.0f);
	SkyHandle=CFcBaseObject::CreateObject<CFcBaseObject>(&Cross);
	SkyHandle->Initialize(nSkySkin, -1, CFcBaseObject::SKYBOX);
}

HeroObjHandle CFcWorld::CreateHero(int nUnitIndex, float fX, float fY)
{
	CameraObjHandle CamHandle;
	CCrossVector Cross;
	TroopObjHandle hTroop;

	if(m_HeroHandle)
	{
		hTroop = m_HeroHandle->GetParentTroop();
		Cross = *m_HeroHandle->GetCrossVector();		// 콘솔에서 플레이어 바꿀 때 필요
		m_HeroHandle->Delete();
		m_HeroHandle.Identity();
	}
	else 
	{
		float fLandHeight = g_BsKernel.GetLandHeight(fX, fY);
		Cross.SetPosition(fX, fLandHeight, fY);
	}

	
	int nClassID = CUnitSOXLoader::GetInstance().GetUnitType( nUnitIndex );
	m_HeroHandle = CFcGameObject::CreateGameObject( (CFcGameObject::GameObj_ClassID)nClassID, &Cross );

	MainHeroFxLoading(nClassID);

	int nInputPort;

	nInputPort = g_FCGameData.nEnablePadID;
	if( nInputPort == -1 )
	{
		nInputPort = 0;
	}

// [beginmodify] 2006/1/16 junyash PS#1259,3249,3251, TCR#033PER multi controller
	#if 0
	// PlayerIndex(CFcHeroObject::m_nPlayerIndex) access Hero array. not equal PAD number
	m_HeroHandle->SetPlayerIndex( nInputPort );
	#else
	m_HeroHandle->SetPlayerIndex( 0 );
	#endif
// [endmodify] junyash
	CInputPad::GetInstance().AttachObject( nInputPort, m_HeroHandle );
	CamHandle=CFcBaseObject::GetCameraObjectHandle(0);
	CamHandle->AttachObject(m_HeroHandle);
	CamHandle->SetTargetTroop( m_HeroHandle->GetTroop() );

	m_HeroHandle->SetInScanner( true );
	m_pTroopManager->AddHero( m_HeroHandle );		// 부대와의 연관관계 때문에 필요

	return m_HeroHandle;
}



void CFcWorld::CreateObjects()
{
	bool bGuardian = false;

	LoadUnitData();

	int nCurPlayerCount = 0;
	TROOP_INFO *pTroopInfo;
	int nTroopCount = m_pMapInfo->GetTroopCount();

	bool bUseGuardian = false;

	int nClassID = -1;
	for( int i=0; i<nTroopCount; i++ )
	{
		pTroopInfo=m_pMapInfo->GetTroopInfo( i );
		if( pTroopInfo->m_troopAttr.nTroopType == TROOPTYPE_PLAYER_1 )
		{
			nClassID = CUnitSOXLoader::GetInstance().GetUnitType( pTroopInfo->m_troopAttr.nLeaderUnitType );
			break;
		}
	}
	switch( nClassID ) {
		case Class_ID_Hero_Klarrann:
		case Class_ID_Hero_VigkVagk:
		case Class_ID_Hero_Dwingvatt:
			bUseGuardian = false;
			break;

		case Class_ID_Hero_Aspharr:
		case Class_ID_Hero_Inphy:
		case Class_ID_Hero_Myifee:
		case Class_ID_Hero_Tyurru:
			bUseGuardian = true;
			break;
	}

	for(int iTroop = 0; iTroop < nTroopCount; iTroop++)
	{
		pTroopInfo=m_pMapInfo->GetTroopInfo( iTroop );

		bGuardian = false;
#ifdef USE_MENU_GUARDIAN
		// 호위부대인지 체크
		if( _strcmpi( pTroopInfo->m_troopAttr.szCaption, GUARDIAN_TROOP_NAME_1 ) == 0 && bUseGuardian )
		{
			if( g_FCGameData.GuardianInfo[0].nID >= 0 )
			{
				pTroopInfo->m_troopAttr.nLeaderUnitType = -1;
				for( int i=0; i<5; i++ )
					pTroopInfo->m_troopAttr.nUnitType[i] = -1;

				pTroopInfo->m_troopAttr.nLeaderUnitType = -1;
				pTroopInfo->m_troopAttr.nUnitType[0] = g_FCGameData.GuardianInfo[0].nID;
				bGuardian = true;
			}
		}
		else if( _strcmpi( pTroopInfo->m_troopAttr.szCaption, GUARDIAN_TROOP_NAME_2 ) == 0 && bUseGuardian )
		{
			if( g_FCGameData.GuardianInfo[1].nID >= 0 )
			{
				pTroopInfo->m_troopAttr.nLeaderUnitType = -1;
				for( int i=0; i<5; i++ )
				{
					pTroopInfo->m_troopAttr.nUnitType[i] = -1;
				}

				pTroopInfo->m_troopAttr.nLeaderUnitType = -1;
				pTroopInfo->m_troopAttr.nUnitType[0] = g_FCGameData.GuardianInfo[1].nID;
				bGuardian = true;
			}
		}
#endif

		switch( pTroopInfo->m_troopAttr.nTroopType )
		{
		case TROOPTYPE_PLAYER_1:
			{
				m_pTroopManager->AddPlayerTroop( this, pTroopInfo );
				nCurPlayerCount++;
				break;
			}
		case TROOPTYPE_PLAYER_2:
			{
				BsAssert(0);		// Not Use
				break;
			}
		default:
			{
				TroopObjHandle hTroop = m_pTroopManager->AddTroop( this, pTroopInfo );
				
				/* =====================================================================
						영웅 케릭터가 player가 아닌 상태로 나올때 프리로딩 한다.
							TODO : 테스트가 완료되면 주석을 지우겠습니다.
				========================================================================*/
				// 영웅 캐릭터가 리더 캐릭터로 나올 경우.
				if( pTroopInfo->m_troopAttr.nLeaderUnitType > -1 && pTroopInfo->m_troopAttr.nLeaderUnitType < 7 ){
					MainHeroFxLoading(g_nMatchingIndex[pTroopInfo->m_troopAttr.nLeaderUnitType], false);				
				}
				// 영웅 캐릭터가 일반 유닛으로 정의되어 나오는 경우.
				for( int i = 0 ; i < 5 ; ++i ){
					int nUnit = pTroopInfo->m_troopAttr.nUnitType[i];
					if ( nUnit > -1 && nUnit < 7 ) {
						MainHeroFxLoading(g_nMatchingIndex[nUnit], false);
					}
				}
				/////////////////////////////////////////////////////////////////////////

				
				if( bGuardian )
				{
					m_pTroopManager->SetGuardianTroop( hTroop );
				}
			}
			break;
		}
	}
	m_pTroopManager->SetFirstGuardian();
}

int CFcWorld::LoadMeshData( int nSlot, const char *pFileName )
{
	int nSkinIndex;

	nSkinIndex = g_BsKernel.LoadSkin( nSlot, pFileName );
	if ( nSkinIndex != -1 )
	{
		if( std::find( m_LoadedSkinList.begin(), m_LoadedSkinList.end(), nSkinIndex ) == m_LoadedSkinList.end() )
		{
			m_LoadedSkinList.push_back( nSkinIndex );
		}
		else
		{
			// Since we are keeping track of the skin in the loaded list we don't want to increase the reference
			// count each time it is loaded.

			g_BsKernel.ReleaseSkin( nSkinIndex );
		}
	}

	return nSkinIndex;
}

int CFcWorld::LoadAniData( int nSlot, const char *pFileName )
{
	int nAniIndex;

	nAniIndex = g_BsKernel.LoadAni( nSlot, pFileName );
	if( std::find( m_LoadedAniList.begin(), m_LoadedAniList.end(), nAniIndex ) == m_LoadedAniList.end() )
	{
		m_LoadedAniList.push_back( nAniIndex );
	}
	else
	{
		// Each time we call load ani the reference count is increased, since we only keep track of each
		// animation once we release the reference count here.

		g_BsKernel.ReleaseAni( nAniIndex );
	}

	return nAniIndex;
}

void CFcWorld::CreateProps()
{
AvailMemoryDifference* pCheck1 = new AvailMemoryDifference(0, "CreateProps_ZoneData");
	// Zone별 박스 먼저 저장
	MapFilePropZoneManager* pMapZoneManager = m_pMapInfo->GetPropZoneManager();
	if( pMapZoneManager == NULL )
		return;

	m_pPropManager->InitZoneData( pMapZoneManager->m_nXSize, pMapZoneManager->m_nYSize, pMapZoneManager->m_nZoneSize );

	int nCnt = pMapZoneManager->GetCount();
	for( int i=0; i<nCnt; i++ )
	{
		Box3* pBox = NULL;
		if( pMapZoneManager->m_pZoneData[i].bProp == true )
			pBox = &(pMapZoneManager->m_pZoneData[i].Box);

		m_pPropManager->SetZoneData( i, pBox );
	}

delete pCheck1;

	std::vector<PROP_INFO> m_vecProps = m_pMapInfo->GetPropInfoList();

pCheck1 = new AvailMemoryDifference(0, "CreateProps_CreateProps");

	m_pPropManager->CreateProps( m_vecProps );
	m_pPropManager->SetEventNavInProps( m_pNavigationMesh, m_pNavEventInfo, m_nNumNavEventInfo);

	// ActiveProp중에 프랍위에 있는 부대들 생성
	nCnt = m_pPropManager->GetActivePropCount();
	for( int i=0; i<nCnt; i++ )
	{
		CFcProp* pProp = m_pPropManager->GetActiveProp( i );
		if( pProp->IsAttackProp() && pProp->GetName() )
		{
			m_pTroopManager->AddTroopOnProp( this, pProp );
		}
		// 발석차에 부서지는 성벽! 더미 부대 필요
		else if( pProp->IsWallProp() && pProp->GetName() )
		{
			m_pTroopManager->AddTroopWallDummy( this, pProp );
		}
	}

delete pCheck1;

pCheck1 = new AvailMemoryDifference(0, "CreateProps_BillBoard");

	int nBillBoardCount = m_pMapInfo->GetBillboardCount();

	const char* szImageName = m_pMapInfo->GetBillboardDDSName();
	// 빌보드 텍스쳐가 없으면 빌보드 무시함
	if( szImageName == NULL ) {
		nBillBoardCount = 0;
	}
	std::vector<SIMPLE_BILLBOARD>& BillboardList = m_pMapInfo->GetBillboardInfoList();

	for(int i=0;i<nBillBoardCount;++i) {
		g_BsKernel.GetInstance().GetBillboardMgr()->PutPosToPool( BillboardList[i].vecPos, D3DXVECTOR2( 110.f, 90.f ), BillboardList[i].nBillboardIndex );
	}

	// 빌보드매니저에 등록된 빌보드를 Zone과 Set에 맞게 재구성.
	// [로딩중 빌보드매니저의 PutPosToPool함수가 시간을 많이 먹는 관계로]
	// [PutPosToPool의 루틴중 일부가 OptiZone으로 옮길 예정]
	AvailMemoryDifference check(0, "OptiZone()");
	g_BsKernel.GetInstance().GetBillboardMgr()->OptiZone();

	BillboardList.clear();
delete pCheck1;
}

void CFcWorld::CreateLight()
{
//	CCrossVector Cross;
	SET_INFO *pInfo;
	pInfo = ( SET_INFO * ) m_pMapInfo->GetSetInfo();

	m_nLightIndex = CBsKernel::GetInstance().CreateLightObject( DIRECTIONAL_LIGHT );
	CBsKernel::GetInstance().UpdateObject( m_nLightIndex, pInfo->crossLight );

	int nFXIndex = g_pFcFXManager->Create(FX_TYPE_LENSFLARE);
	g_pFcFXManager->SendMessage(nFXIndex, FX_SET_LIGHTDIRECTION, (DWORD)&(pInfo->crossLight.m_ZVector));
	m_nFXIndex = nFXIndex;	// 임시 Realmovie 테스트.

	g_BsKernel.SendMessage( m_nLightIndex, BS_SET_LIGHT_AMBIENT, (DWORD)&pInfo->clrLightAmbient );
	g_BsKernel.SendMessage( m_nLightIndex, BS_SET_LIGHT_DIFFUSE, (DWORD)&pInfo->clrLightDiffuse );
	g_BsKernel.SendMessage( m_nLightIndex, BS_SET_LIGHT_SPECULAR, (DWORD)&pInfo->clrLightSpecular );

	CameraObjHandle CamHandle;

	CamHandle = CFcBaseObject::GetCameraObjectHandle( 0 );

	float fAppliedFarZ = BsMin(100000.f, pInfo->fProjectionZMax );
	CamHandle->SetProjectionMatrix( 10.f, fAppliedFarZ );
	CamHandle->SetFogColor( &pInfo->vecFogColor );
	CamHandle->SetFogFactor( pInfo->fNear, pInfo->fFar );

	g_BsKernel.SetBlurIntensity( pInfo->fBlur );
	g_BsKernel.SetSceneIntensity( 1.0f - pInfo->fBlur );
	g_BsKernel.SetGlowIntensity( pInfo->fGlow );

	g_BsKernel.SetSceneIntensity( pInfo->GetSceneIntensity() );
	g_BsKernel.SetBlurIntensity( pInfo->GetBlurIntensity() );
	g_BsKernel.SetGlowIntensity( pInfo->GetGlowIntensity() );
	g_BsKernel.SetHighlightThreshold( pInfo->GetThreshold() );
}

void CFcWorld::CreateNavigationMesh()
{
	BsAssert( m_pNavigationMesh == NULL );
	m_pNavigationMesh = new NavigationMesh;

	bool bReadyNavFile = false;
	
	char cNavFile[MAX_PATH];
	strcpy( cNavFile, m_pMapInfo->GetMapFileName() );
	char* pTempStr = strstr( cNavFile, "." );
	if ( pTempStr )	pTempStr[1] = NULL;	// mruete: prefix bug 644: added check
	strcat_s( cNavFile, _countof(cNavFile), "nav" ); //aleksger - safe string

	BMemoryStream ReadStream;
	VOID *pData = NULL;
	DWORD dwFileSize;

	g_BsResChecker.AddResList( cNavFile );

	if(!FAILED(CBsFileIO::LoadFile(cNavFile, &pData, &dwFileSize)))
	{
		SYSTEMTIME Dummy;
		ReadStream.Initialize(pData, dwFileSize);
		ReadStream.Read( &Dummy, sizeof(SYSTEMTIME), 2 );
/*
		if( memcmp( &MapTime, m_pMapInfo->GetMapSysTime(), sizeof(SYSTEMTIME) ) != 0 )
		{
			DebugString( "Old NAV file! %s\n", cNavFile );
		}
*/
	}
	else
	{
		DebugString( "Cannot found NAV file! %s\n", cNavFile );
		BsAssert( 0 &&"Cannot found NAV file!" );
	}

	m_pNavigationMesh->Load( &ReadStream );
	SAFE_DELETEA( m_pNavEventInfo );
	ReadStream.Read( &m_nNumNavEventInfo, sizeof(int), 4 );
	m_pNavEventInfo = new NAVEVENTINFO[m_nNumNavEventInfo];
	for( int i=0; i<m_nNumNavEventInfo; i++ )
		ReadStream.Read( &m_pNavEventInfo[i], sizeof(NAVEVENTINFO), 4 );
	
	CBsFileIO::FreeBuffer(pData);

	// 유닛 피해가기 용 
	BsAssert( m_pNavCollisionMesh == NULL );
	m_pNavCollisionMesh = new NavCollisionMesh;

	int nGroupCnt = m_pMapInfo->GetColEventCount();

//		Collision Mesh 생성 잠시 막아둔다.
/*
	m_pNavCollisionMesh->Initialize( nGroupCnt );
	int nStartIndex = -1;
	for( int i=0; i<nGroupCnt; i++ )
	{
		nStartIndex = -1;
		int nCnt = m_pMapInfo->GetColEventIndexBufferCount( i );
		int* pIndexBuf = m_pMapInfo->GetColEventIndexBuffer( i );
		for( int j=0; j<nCnt; j+=3 )
		{
			D3DXVECTOR3 Pos = m_pMapInfo->GetColVector( pIndexBuf[j] );
			D3DXVECTOR3 Pos2 = m_pMapInfo->GetColVector( pIndexBuf[j + 1] );
			D3DXVECTOR3 Pos3 = m_pMapInfo->GetColVector( pIndexBuf[j + 2] );

			m_pNavCollisionMesh->AddTri( i, &Pos, &Pos2, &Pos3 );
		}
	}
	m_pNavCollisionMesh->GenerateExternalLine();
*/

	m_pMapInfo->ClearNav();
	m_pMapInfo->ClearCol();
	m_pMapInfo->ClearNavEventIndexBuffer();
}

void CFcWorld::CreateWater(const char *pMapFileName)
{
	BsAssert( m_vecWaterList.empty() );

	char szMapFileName[255];
	strcpy_s( szMapFileName, 255, pMapFileName);

	szMapFileName[5] = '\0';

	int nWaterCount = 0;

	// 소스코드 상으로 물개수를 넣어버린다..
	if( _stricmp(szMapFileName, "he_my") == 0) nWaterCount = 1;
	if( _stricmp(szMapFileName, "ph_as") == 0) nWaterCount = 1;
	if( _stricmp(szMapFileName, "ph_dw") == 0) nWaterCount = 1;
	if( _stricmp(szMapFileName, "ph_in") == 0) nWaterCount = 1;
	if( _stricmp(szMapFileName, "ph_my") == 0) nWaterCount = 1;
	if( _stricmp(szMapFileName, "ph_vi") == 0) nWaterCount = 1;
	if( _stricmp(szMapFileName, "ph_ty") == 0) nWaterCount = 1;
	if( _stricmp(szMapFileName, "vf_as") == 0 ) nWaterCount = 1;
	if( _stricmp(szMapFileName, "vf_in") == 0 && pMapFileName[5] != '2' /* vf_in2 는 제외*/ ) nWaterCount = 1;
	if( _stricmp(szMapFileName, "wy_as") == 0) nWaterCount = 1;
	if( _stricmp(szMapFileName, "wy_in") == 0) nWaterCount = 1;
	if( _stricmp(szMapFileName, "wy_kl") == 0) nWaterCount = 1;
	if( _stricmp(szMapFileName, "wy_ty") == 0) nWaterCount = 1;
	if( _stricmp(szMapFileName, "yw_in") == 0) nWaterCount = 4;
	if( _stricmp(szMapFileName, "yw_as") == 0) nWaterCount = 4;

	for( int nWater = 0; nWater < nWaterCount; nWater++) {

		char *pWaterFileName = ReplaceEXT( pMapFileName, ComposeFormatString("_w%d.skin", nWater+1) );
		int nWaterSkin = LoadMeshData( -1,  pWaterFileName);

		CCrossVector Cross;
		BaseObjHandle Water = CFcBaseObject::CreateObject< CFcBaseObject >( &Cross );
		
		int nWaterIndex = Water->Initialize( nWaterSkin , -1, CFcBaseObject::WATER);
		if ( nWaterIndex != -1 )
			m_vecWaterList.push_back(nWaterIndex);
	}
}

void CFcWorld::Process()
{
	if( m_bLoad == false )
	{
		return;
	}
	/*
	CCrossVector Cross  = *CFcWorld::GetInstance().GetHeroHandle()->GetCrossVector();
	D3DXVECTOR4 info;
	info.x = Cross.GetPosition().x;
	info.y = Cross.GetPosition().y + 100.f;
	info.z = Cross.GetPosition().z;
	info.w = 800.f;
	g_BsKernel.SendMessage(nLightIndex, BS_SET_POINTLIGHT_INFO, (DWORD)&info);
	*/

	// 실기무비, 컷신 스킵 처리
	if( m_nDemoType != -1 )
	{
		if( m_nDemoType == 0 )		// 실기 무비 끝났는지 체크
		{
			if( g_pFcRealMovie->IsPlay() == false && g_pFcRealMovie->IsSetDirectMsg() == false )
				m_nDemoType = -1;
		}

		// 실기 무비 버튼 체크
		if( m_bIsDemoSkip == false && CInputPad::GetInstance().GetKeyPressTick( g_FCGameData.nEnablePadID, PAD_INPUT_A ) >= 1 )
			m_bIsDemoSkip = true;

		if( m_nDemoType == 1 )
		{
			if( g_pFcRealMovie->IsPlay() == false && g_pFcRealMovie->IsSetDirectMsg() == false )
			{
				m_nDemoType = -1;
				m_bIsDemoSkip = false;
			}
		}
	}

	// 실기무비 후 부대 상태 돌려줌
	if( m_bBackupForRM )
	{
		if( !g_pFcRealMovie->IsPlay() && !g_pFcRealMovie->IsSetDirectMsg() )
			//			(m_nRMStartTick + 3 < ::GetProcessTick()) )
		{
			m_bBackupForRM = false;
			m_nRMStartTick = 0;
			int nCnt = m_vecBackupTroopsForRM.size();
			for( int i=0; i<nCnt; i++ )
			{
				TroopObjHandle hTroop = m_vecBackupTroopsForRM[i];
				hTroop->CmdEnable( true, false );
			}
		}
	}

	if( m_bBackupAIForRM )
	{
		if( !g_pFcRealMovie->IsPlay() && !g_pFcRealMovie->IsSetDirectMsg() )
			//			(m_nRMStartTick + 3 < ::GetProcessTick()) )
		{
			m_bBackupAIForRM = false;
			//			m_nRMStartTick = 0;
			int nCnt = m_vecBackupTroopsDisableAIForRM.size();
			for( int i=0; i<nCnt; i++ )
			{
				TroopObjHandle hTroop = m_vecBackupTroopsDisableAIForRM[i];
				if( m_pTroopManager->GetGuardianTroop( 0 ) == hTroop )
				{
					m_pTroopManager->SetGuardianEnable( 0, true );
				}
				else if( m_pTroopManager->GetGuardianTroop( 1 ) == hTroop )
				{
					m_pTroopManager->SetGuardianEnable( 1, true );
				}
				else
				{
					CFcTroopAIObject::SetEnableAI( hTroop, true );
				}
			}
		}
	}

	m_pPropManager->Process();
	m_pTriggerCommander->Process();
	m_pTroopManager->Process();
	m_pAbilityManager->Process();

	// True Orb 시전시간 채크, AI 등에서 참조하기 위해 타이머 필요!! by Siva
	if( m_nTrueOrbTimer > 0 ) --m_nTrueOrbTimer;

	// 화면 인터페이스에 특별히 보여줄 것 처리(게이지, 시간)
	ProcessExtraInterfaceParam();

	if(m_bChangeLightStart) 
	{
		m_vCurDiffuse += m_vIncDiffuse;
		m_vCurSpecular += m_vIncSpecular;
		m_vCurAmbient += m_vIncAmbient;

		g_BsKernel.SendMessage( g_FcWorld.GetLightIndex(), BS_SET_LIGHT_AMBIENT, (DWORD)&m_vCurAmbient );
		g_BsKernel.SendMessage( g_FcWorld.GetLightIndex(), BS_SET_LIGHT_DIFFUSE, (DWORD)&m_vCurDiffuse );
		g_BsKernel.SendMessage( g_FcWorld.GetLightIndex(), BS_SET_LIGHT_SPECULAR, (DWORD)&m_vCurSpecular );

		if( GetProcessTick()  - m_nChangeLightOldTick > m_nChangeLightSec*FRAME_PER_SEC ) 
			m_bChangeLightStart = false;
	}

	if(m_bChangeFogStart) 
	{
		m_vCurFogColor += m_vIncFogColor;

		CameraObjHandle CamHandle = CFcBaseObject::GetCameraObjectHandle( 0 );	
		CamHandle->ChangeFog(&m_vCurFogColor, (float)m_nChangeFogNear, (float)m_nChangeFogFar, m_nChangeFogTick);

		if( GetProcessTick()  - m_nChangeFogOldTick > m_nChangeFogSec*FRAME_PER_SEC ) 
			m_bChangeFogStart = false;
	}

	if(m_bChangeSceneStart) 
	{
		m_fCurScene  += m_fIncScene; 
		m_fCurBlur += m_fIncBlur;
		m_fCurGlow += m_fIncGlow;

		g_BsKernel.SetSceneIntensity( m_fCurScene );
		g_BsKernel.SetBlurIntensity( m_fCurBlur );
		g_BsKernel.SetGlowIntensity( m_fCurGlow );
		
		if( GetProcessTick()  - m_nChangeSceneOldTick > m_nChangeSceneSec*FRAME_PER_SEC ) 
			m_bChangeSceneStart = false;
	}

	if( g_FCGameData.bShowAttr ) {

//		D3DXVECTOR3 heroPos = g_FcWorld.GetHeroHandle(0)->GetCrossVector()->GetPosition();
		CameraObjHandle CamHandle = CFcBaseObject::GetCameraObjectHandle( 0 );
		CCrossVector* pCross = CamHandle->GetCrossVector();

		for( int m = -30; m < 30; m++) {
			int nZ = (int)(pCross->m_PosVector.z*0.01f + m);
			if( nZ < 0 || nZ >= g_FcWorld.GetAttrYSize() )continue;
			for( int n = -30; n < 30; n++) {
				int nX = (int)(pCross->m_PosVector.x*0.01f + n);
				if( nX < 0 || nX >= g_FcWorld.GetAttrXSize() )continue;
				static int nAndBit = 2;
				static int nAndExBit = 0;

				BYTE cAttr = CFcWorld::GetInstance().GetAttr( nX, nZ);
				BYTE cAttrEx = CFcWorld::GetInstance().GetAttrEx( nX, nZ);
				int R, G, B;
				int A = 255;

				int nTextureID = g_BsKernel.GetBlankTexture();
				float fRotAngle = 0.f;

				bool bAttr = false;
				bool bAttrEx = false;
				if( ( cAttr & 0x04 ) >> 2 == 1 )		// 다리 속성
				{
					R = 255;	G = 0;		B = 0;	bAttr = true;
				}
				else if( cAttr & 2 )					// 건물 속성
				{
					R = 0;	G = 255;		B = 0;	bAttr = true;
				}
				else if( cAttr & 1 )					// 나무 속성
				{
					R = 255;	G = 255;		B = 0;	bAttr = true;
				}
				else if( ( cAttrEx & 0x08 ) >> 3 == 1 )	// 아군만 못가는 속성
				{
					R = 128;	G = 128;		B = 128;	bAttrEx = true;
				}
				else if( ( cAttr & 0x10 ) >> 4 == 1 )	// 히어로만 가는 속성
				{
					R = 0;	G = 0;		B = 255;		bAttr = true;
				}
				else if( (cAttrEx & 0x07) != 0 ) {

					if( (cAttrEx & 0x07) == 1) fRotAngle = 0.f;
					if( (cAttrEx & 0x07) == 2) fRotAngle = 90.f;
					if( (cAttrEx & 0x07) == 3) fRotAngle = 180.f;
					if( (cAttrEx & 0x07) == 4) fRotAngle = 270.f;

					static int nDiagonalTextureID = g_BsKernel.LoadTexture( "DebugRes\\Diagonal.dds" );	
					nTextureID = nDiagonalTextureID;
					R = 0;	G = 255;		B = 0;	bAttrEx = true; 
				}
				else if( cAttr != 0 || cAttrEx != 0 ) {	// 뭔가 다른 속성이 있는 경우.
                    A = 32, R = 255;		G = 0;		B = 255;		bAttr = true;
				}

				if( bAttr )	{
					g_BsKernel.AddDecal( nX*100.f + 50.f, nZ*100.f + 50.f, nTextureID, 50.f, 0, fRotAngle, D3DCOLOR_ARGB(A,R,G,B) );
				}
				if( bAttrEx ) {
					g_BsKernel.AddDecal( nX*100.f + 50.f, nZ*100.f + 50.f, nTextureID, 50.f, 0, fRotAngle, D3DCOLOR_ARGB(A,R,G,B) );
				}
			}		
		}
	}

}

void CFcWorld::Update()
{
//	CheckUnitUpdateObject();
	/*
	if( m_bLoad ) {		
		D3DXMATRIX mat;
		CCrossVector cross = *CFcWorld::GetInstance().GetHeroHandle()->GetCrossVector();
		cross.SetPosition( cross.GetPosition() + D3DXVECTOR3(0,90.f,0));
		mat = *cross;			
	}
	*/


	if( m_pAbilityManager )
		m_pAbilityManager->Update();

	if( m_pPropManager )
	{
		m_pPropManager->UpdateObjects();
	}

	if( m_pTroopManager )
	{
		m_pTroopManager->UpdateObjects();
	}
}

#include "BSTriggerVariable.h"
void CFcWorld::DebugRender()
{
	if( g_FCGameData.State != GAME_STATE_LOOP_STAGE )
		return;


	// Trigger Debug Render
	if( g_FCGameData.bShowVariable == true ) {
		char szVal[256];
		for( int i=0; i<m_pTriggerCommander->GetVariableCount(); i++ ) {
			std::string szStr;
			CBSVariable *pVariable = m_pTriggerCommander->GetVariable(i);

			szStr += pVariable->GetParamData(0)->GetString();
			switch( pVariable->GetParamData(1)->GetInteger() ) {
				case 0:	
					szStr += " (Integer) : ";	
					sprintf( szVal, "%d", pVariable->GetParamData(2)->GetInteger() );
					break;
				case 1:	
					szStr += " (Bool) : ";	
					sprintf( szVal, "%d", pVariable->GetParamData(2)->GetInteger() );
					break;
				case 2: 
					szStr += " (Random) : ";	
					sprintf( szVal, "%d", pVariable->GetParamData(2)->GetInteger() );
					break;
				case 3: 
					szStr += " (String) : ";	
					sprintf( szVal, "%s", pVariable->GetParamData(2)->GetString() );
					break;
			}
			szStr += szVal;
			g_BsKernel.PrintString( 20, 50 + ( i * 20 ), szStr.c_str(), 0xFFFFFFFF );
		}
	}

	if( g_FCGameData.bShowHeroPos == true ) {
		char cStr[256];
		D3DXVECTOR2 Pos = GetHeroHandle()->GetPosV2();
		sprintf( cStr, "Hero %d, %d", (int)(Pos.x/100.f), (int)(Pos.y/100.f) );
		g_BsKernel.PrintString( 30, 30, cStr, 0xFFFFFFFF );
	}


#ifndef _XBOX
	if( g_FCGameData.bShowCollisionMesh == true ) {
		if( m_pNavCollisionMesh )
			m_pNavCollisionMesh->Render();
	}

	if( g_FCGameData.bShowNavigationMesh == false )
	{
		return;
	}

	C3DDevice* pDevice = g_BsKernel.GetDevice();
	pDevice->SaveState();
	pDevice->BeginScene();


	const DWORD LINE_FVF = D3DFVF_XYZ | D3DFVF_DIFFUSE;

	struct LINE_VERTEX
	{
		D3DXVECTOR3 v;
		DWORD       color;
	};

	LINE_VERTEX pVertices[4];

	D3DXMATRIX matView, matProj;
	pDevice->GetTransform( D3DTS_VIEW,		&matView );
	pDevice->GetTransform( D3DTS_PROJECTION,	&matProj );

	D3DXMATRIX mat;
	D3DXMatrixIdentity( &mat );

//	CameraObjHandle CamHandle = CFcBaseObject::GetCameraObjectHandle( 0 );
//	CCrossVector* pCross = CamHandle->GetCrossVector();
//	const D3DXMATRIX* pMatVP = g_BsKernel.GetViewProjectionMatrix( CamHandle->GetEngineIndex() );
//	matView = (*pCross);


	pDevice->SetTransform( D3DTS_WORLD, &mat);
	pDevice->SetTransform( D3DTS_VIEW, &matView );
	pDevice->SetTransform( D3DTS_PROJECTION, &matProj);

	pDevice->SetPixelShader(NULL);
	pDevice->SetVertexShader(NULL);
	pDevice->SetFVF(LINE_FVF);
	pDevice->SetRenderState(D3DRS_ALPHATESTENABLE, false);

//	D3DXMATRIX mat;
//	D3DXMatrixMultiply( &mat, &matView, &matProj );

//	D3DXVec3TransformCoord( &PhysicsInfo.Position, &PhysicsInfo.PivotPosition, m_Cross );
//	D3DXMatrixTranspose(&mat, &mat );
	
	if( m_pNavigationMesh )
	{
		int nCnt = m_pNavigationMesh->TotalCells();
		for( int i=0; i<nCnt; i++ )
//		for( int i=0; i<1; i++ )
		{
			NavigationCell* pCell = m_pNavigationMesh->Cell( i );
			for( int j=0; j<3; j++ )
			{
				D3DXVECTOR3 TempPos = pCell->Vertex(j);
				pVertices[j].v.x = TempPos.x;		pVertices[j].v.y = 100.f;		pVertices[j].v.z = TempPos.z;
				pVertices[j].color = 0xffffffff;

//				D3DXVec3TransformCoord( &pVertices[j].v, &pVertices[j].v, &mat );
			}
			pVertices[3].v.x = pVertices[0].v.x;		pVertices[3].v.y = 100.f;		pVertices[3].v.z = pVertices[0].v.z;
			pVertices[3].color = 0xffffffff;
			
			pDevice->SetFVF(LINE_FVF);
			pDevice->DrawPrimitiveUP(D3DPT_LINESTRIP,3,pVertices,sizeof(LINE_VERTEX));
		}
	}
	m_pPropManager->DebugCollsionPropRender();
	
	m_pTroopManager->DebugPathRender();

	pDevice->EndScene();
	pDevice->RestoreState();
#endif
}

#include "PerfCheck.h"
void CFcWorld::SetFog( D3DXVECTOR4* vecFogColor, float fNear, float fFar  )
{
	CameraObjHandle CamHandle;
	CamHandle = CFcBaseObject::GetCameraObjectHandle( 0 );
	CamHandle->SetFogColor( vecFogColor );
	CamHandle->SetFogFactor( fNear, fFar );
}

struct ObjectScannerCheckLive
{
	ObjectScannerCheckLive() {}

	bool operator() ( GameObjHandle Handle ) {
		return Handle->CheckHit() == true;
	}
};

struct ObjectScannerCheckLiveTeam
{
	ObjectScannerCheckLiveTeam( int nTeam ) { m_nTeam = nTeam; }

	bool operator() ( GameObjHandle Handle ) {
		if( Handle->CheckHit() == false ) return false;
		return Handle->GetTeam() == m_nTeam;
	}
	int m_nTeam;
};

struct ObjectScannerCheckLiveEnemyTeam
{
	ObjectScannerCheckLiveEnemyTeam( int nTeam ) { m_nTeam = nTeam; }

	bool operator() ( GameObjHandle Handle ) {
		if( Handle->CheckHit() == false ) return false;
		return Handle->GetTeam() != m_nTeam;
	}
	int m_nTeam;
};

struct ObjectScannerCheckDie
{
	ObjectScannerCheckDie() {}

	bool operator() ( GameObjHandle Handle ) {
		return Handle->CheckHit() == false;
	}
};



void CFcWorld::GetObjectListInRangeByTeam( D3DXVECTOR3 *pPosition, float fRadius, std::vector<GameObjHandle> &Objects, int nTeam )
{
	GetObjectListInRange( pPosition,fRadius,Objects );

	for(int i = 0; i < (int)Objects.size(); i++ ) 
	{
		if( Objects[i]->GetTeam() != nTeam ){		
			Objects.erase(Objects.begin() + i );
			i--;
		}
	}

}

void CFcWorld::GetObjectListInRange(D3DXVECTOR3 *pPosition, float fRadius, std::vector<GameObjHandle> &Objects)
{
	PROFILE_TIME_TEST( m_pObjectImpl->ObjectScanner.FindFor(pPosition->x, pPosition->z, fRadius, ObjectScannerCheckLive(), Objects) );

	D3DXVECTOR2 vVec;
	DWORD dwSize = m_vecBigSizeScanner.size();
	float fTemp;
	for( DWORD i=0; i<dwSize; i++ ) {
		vVec.x = pPosition->x - m_vecBigSizeScanner[i]->GetCrossVector()->m_PosVector.x;
		vVec.y = pPosition->z - m_vecBigSizeScanner[i]->GetCrossVector()->m_PosVector.z;
		fTemp = fRadius + m_vecBigSizeScanner[i]->GetUnitRadius();
		if( D3DXVec2LengthSq( &vVec ) < fTemp * fTemp ) {
			Objects.push_back( m_vecBigSizeScanner[i] );
		}
	}

	/*
	PROFILE_TIME_TEST( m_pObjectImpl->ObjectScanner.Find(pPosition->x, pPosition->z, fRadius, Objects) );

	D3DXVECTOR2 vVec;
	DWORD dwSize = m_vecBigSizeScanner.size();
	float fTemp;
	for( DWORD i=0; i<dwSize; i++ ) {
		vVec.x = pPosition->x - m_vecBigSizeScanner[i]->GetCrossVector()->m_PosVector.x;
		vVec.y = pPosition->z - m_vecBigSizeScanner[i]->GetCrossVector()->m_PosVector.z;
		fTemp = fRadius + m_vecBigSizeScanner[i]->GetUnitRadius();
		if( D3DXVec2LengthSq( &vVec ) < fTemp * fTemp ) {
		Objects.push_back( m_vecBigSizeScanner[i] );
		}
	}
	*/
}

void CFcWorld::GetObjectListInRangeByDie( D3DXVECTOR3 *pPosition, float fRadius, std::vector<GameObjHandle> &Objects )
{
	PROFILE_TIME_TEST( m_pObjectImpl->ObjectScanner.FindFor(pPosition->x, pPosition->z, fRadius, ObjectScannerCheckDie(), Objects) );
}

void CFcWorld::GetEnemyObjectListInRange(int nMyGroup, D3DXVECTOR3 *pPosition, float fRadius, std::vector<GameObjHandle> &Objects, bool p_bIncludeDead /*= true*/ )
{
	PROFILE_TIME_TEST( m_pObjectImpl->ObjectScanner.FindFor( pPosition->x, pPosition->z, fRadius, ObjectScannerCheckLiveEnemyTeam(nMyGroup), Objects ) );

	D3DXVECTOR2 vVec;
	DWORD dwSize = m_vecBigSizeScanner.size();
	float fTemp;
	for( DWORD i=0; i<dwSize; i++ ) {
		if( m_vecBigSizeScanner[i]->GetTeam() == nMyGroup ) continue;

		vVec.x = pPosition->x - m_vecBigSizeScanner[i]->GetCrossVector()->m_PosVector.x;
		vVec.y = pPosition->z - m_vecBigSizeScanner[i]->GetCrossVector()->m_PosVector.z;
		fTemp = fRadius + m_vecBigSizeScanner[i]->GetUnitRadius();
		if( D3DXVec2LengthSq( &vVec ) < fTemp * fTemp ) {
			if( p_bIncludeDead ){
                Objects.push_back( m_vecBigSizeScanner[i] );
			}
			else{
                if(!m_vecBigSizeScanner[i]->IsDie()) Objects.push_back( m_vecBigSizeScanner[i] );
				else continue;
			}
		}
	}


	/*
	GetObjectListInRange( pPosition, fRadius, Objects );
	//	m_pObjectImpl->ObjectScanner.Find(pPosition->x, pPosition->z, fRadius, Objects);

	std::vector<GameObjHandle>::iterator it = Objects.begin();
	while( it != Objects.end() )
	{
		GameObjHandle Unit = *it;
		if( Unit->GetTeam() == nMyGroup )		//  아군이면 삭제
			it = Objects.erase( it );
		else
			it++;
	}
	*/
}

void CFcWorld::GetEnemyObjectListInRangeDir(int nMyGroup, D3DXVECTOR3 *pPosition, D3DXVECTOR3 *pDir, float fDotValue, float fRadius, std::vector<GameObjHandle> &Objects, float fMinimumDist/*=0.0f*/)
{
	GetObjectListInRange( pPosition, fRadius, Objects );
	//	m_pObjectImpl->ObjectScanner.Find(pPosition->x, pPosition->z, fRadius, Objects);

	std::vector<GameObjHandle>::iterator it = Objects.begin();
	while( it != Objects.end() )
	{
		GameObjHandle Unit = *it;
		if( Unit->GetTeam() == nMyGroup || Unit->GetHP() <= 0)		//  아군이면 삭제
			it = Objects.erase( it );
		else 
		{
			D3DXVECTOR3 ePos = Unit->GetPos();
			D3DXVECTOR3 eDir = ePos - *pPosition;
			eDir.y = 0.0f;
			float fDistFromHero = D3DXVec3Length(&eDir);
			D3DXVec3Normalize(&eDir, &eDir);
			float fdot = D3DXVec3Dot(&eDir, pDir);

			if( fMinimumDist >= fDistFromHero)
			{
				if( fdot <= 1 && fdot > 0.5f )
				{
					it++;
				}
				else
				{
					it = Objects.erase( it );
				}
			}
			else if( fdot <= 1 && fdot > fDotValue )
			{
				it++;
			}
			else
			{
				it = Objects.erase( it );
			}
		}

	}
}
D3DXVECTOR3 CFcWorld::GetEnemyObjectInRangeDirNear( int nMyGroup, D3DXVECTOR3 *pPosition, D3DXVECTOR3 *pDir, float fDotValue, float fRadius, int nCount /*=0*/, float fMinimumDist /*=0.0f*/ )
{
	std::vector<GameObjHandle> Objects;
	GetEnemyObjectListInRangeDir(nMyGroup, pPosition, pDir, fDotValue, fRadius, Objects, fMinimumDist);
	D3DXVECTOR3 vRet(0,0,0);
	float fDist1 = fRadius;

	std::vector<GameObjHandle>::iterator it = Objects.begin();
	while( it != Objects.end() )
	{
		GameObjHandle Unit = *it;
		D3DXVECTOR3 vDist;
		vDist = *pPosition - Unit->GetPos();
		float fDist = D3DXVec3Length(&vDist);
		if( nCount == 0 )
		{
			if( fDist1 > fDist )
			{
				vRet = Unit->GetPos();
				fDist1 = fDist;
			}
		}
		else
		{
			if( (int)Objects.size() > nCount )
			{
				GameObjHandle Unit2 = Objects[nCount];
				vRet = Unit2->GetPos();
				break;
			}
		}

		it++;
	}

	return vRet;
}

void CFcWorld::SetAlphaBlocking( bool bEnable )
{
	m_pPropManager->SetAlphaBlocking( bEnable );
}

void CFcWorld::AddUnitToScanner( GameObjHandle Unit )
{
	switch( Unit->GetClassID() ) {
		case CFcGameObject::Class_ID_Catapult:
			AddBigSizeScanner( Unit );
			break;
	}
	m_pObjectImpl->ObjectScanner.AddObject( Unit );
	/*
	if( m_pObjectImpl->ObjectScanner.AddObject( Unit ) == false ) {
		if( CFcBaseObject::IsValid( Unit ) ) {
			DebugString( "Add UnitSOXID faild: %d, Pos : Cur( %.2f, %.2f ), %d %d %d %x\n", Unit->GetUnitSoxIndex(), Unit->GetPos().x, Unit->GetPos().z, Unit.GetID(), Unit.GetIndex(), GetProcessTick(), Unit.GetPointer() );
		}
		else {
			DebugString( "AddUnitToScanner Faild %d %d %d %x\n", Unit.GetID(), Unit.GetIndex(), GetProcessTick(), Unit.GetPointer() );
		}
	}
	*/
}

void CFcWorld::RemoveUnitToScanner( GameObjHandle Unit )
{
	switch( Unit->GetClassID() ) {
		case CFcGameObject::Class_ID_Catapult:
			RemoveBigSizeScanner( Unit );
			break;
	}
	m_pObjectImpl->ObjectScanner.RemoveObject( Unit );
	/*
	if( m_pObjectImpl->ObjectScanner.RemoveObject( Unit ) == false ) {
		if( CFcBaseObject::IsValid( Unit ) ) {
			DebugString( "Remove UnitSOXID faild: %d, Pos : Cur( %.2f, %.2f ), %d %d %d %x\n", Unit->GetUnitSoxIndex(), Unit->GetPos().x, Unit->GetPos().z, Unit.GetID(), Unit.GetIndex(), GetProcessTick(), Unit.GetPointer() );

			DebugString( "Temp : Prev( %.2f, %.2f) Debug( %.2f, %.2f )\n", Unit->GetPrevPos().x, Unit->GetPrevPos().z, Unit->GetLastMoveToScannerPos().x, Unit->GetLastMoveToScannerPos().z );
		}
		else {
			DebugString( "RemoveUnitToScanner Faild %d %d %d %x\n", Unit.GetID(), Unit.GetIndex(), GetProcessTick(), Unit.GetPointer() );
		}
	}
	*/
}

void CFcWorld::MoveUnitToScanner(GameObjHandle Unit, D3DXVECTOR3 *pPrevPos)
{
	/*
	if( D3DXVec3LengthSq( &Unit->GetLastMoveToScannerPos() ) > 0.f ) {
		D3DXVECTOR3 vTemp = *pPrevPos - Unit->GetLastMoveToScannerPos();
		vTemp.y = 0.f;
		if( D3DXVec3LengthSq( &vTemp ) > 0.f ) {
			_DEBUGBREAK;
		}
	}
	*/
	m_pObjectImpl->ObjectScanner.MoveObject( pPrevPos->x, pPrevPos->z, Unit );
	/*
	if( m_pObjectImpl->ObjectScanner.MoveObject(pPrevPos->x, pPrevPos->z, Unit) == false ) {
		if( CFcBaseObject::IsValid( Unit ) ) {
			DebugString( "Move UnitSOXID : %d, Pos : Cur( %.2f, %.2f ), Prev( %.2f, %.2f ) %d %d %d %x\n", Unit->GetUnitSoxIndex(), Unit->GetPos().x, Unit->GetPos().z, pPrevPos->x, pPrevPos->z, Unit.GetID(), Unit.GetIndex(), GetProcessTick(), Unit.GetPointer() );
		}
		else {
			DebugString( "MoveUnitToScanner Faild %d %d %d %x\n", Unit.GetID(), Unit.GetIndex(), GetProcessTick(), Unit.GetPointer() );
		}
	}
	else {
		Unit->SetLastMoveToScannerPos( &Unit->GetCrossVector()->m_PosVector );
	}
	*/
}


void CFcWorld::GetTroopListInRange( D3DXVECTOR2 *pPosition, float fRadius, std::vector<TroopObjHandle> &Objects )
{
	m_pTroopManager->GetListInRange( pPosition, fRadius, Objects );
}

void CFcWorld::GetEnemyTroopListInRange( D3DXVECTOR2 *pPosition, int nGroup, float fRadius, std::vector<TroopObjHandle> &Objects )
{
	m_pTroopManager->GetEnemyListInRange( pPosition, nGroup, fRadius, Objects );
}

TroopObjHandle CFcWorld::GetNearMeleeEnemyInRange( D3DXVECTOR2 *pPosition, int nTeam, float fRadius )
{
	return m_pTroopManager->GetNearMeleeEnemyInRange( pPosition, nTeam, fRadius );
}


AREA_INFO* CFcWorld::GetAreaInfo( char* pName )
{
	int nCnt = m_pMapInfo->GetAreaCount();
	for( int i=0; i<nCnt; i++ )
	{
		AREA_ATTR* pAttr = m_pMapInfo->GetAreaAttr( i );
		if( strcmp( pName, pAttr->szAreaName ) == 0 )
		{
			return m_pMapInfo->GetAreaInfo( i );
		}
	}
	DebugString( "Cannot found Area ID : %s\n", pName );
	BsAssert( 0 && "Cannot found Area ID" );
	return NULL;
}

TroopObjHandle CFcWorld::GetTroopObject( char* pName )
{
	return m_pTroopManager->GetTroopByName( pName );
}

TroopObjHandle CFcWorld::GetTroopObject( int nIndex )
{
	return m_pTroopManager->GetTroop( nIndex );
}


void CFcWorld::SetTroopAI( TroopObjHandle hTroop, int nAIID, bool bGuardianSkip )
{
	if( bGuardianSkip )
	{
		if( !m_pTroopManager->IsGuardian( hTroop ) )
			CFcTroopAIObject::SetAI( hTroop, nAIID );
	}
	else
	{
		CFcTroopAIObject::SetAI( hTroop, nAIID );
	}
}

void CFcWorld::SetPathToTroopAI( char* pPathName, int nAIPathID )
{
	PATH_PACK* pPath = NULL;

	int nCnt = GetPathCount();
	for( int i=0; i<nCnt; i++ )
	{
		pPath = GetPathInfo( i );
		if( strcmp( pPath->pathInfo.szCaption, pPathName ) == 0 )
		{
			break;
		}
	}
	BsAssert( pPath );

	CFcTroopAIObject::SetAIPath( pPath->pathInfo.nID, nAIPathID );
}

void CFcWorld::EnableTroopAI( TroopObjHandle hTroop, bool bEnable )
{
	CFcTroopAIObject::SetEnableAI( hTroop, bEnable );
}

PROPTYPEDATA* CFcWorld::GetPropTypeData_( int index )
{
	return m_pPropManager->GetPropTypeData_( index );
}


CFcProp* CFcWorld::GetProp( char* pName )
{
	BsAssert( pName[0] != NULL );

	CFcProp* pProp = m_pPropManager->GetProp( pName );
	if( pProp == NULL )
	{
		DebugString( "Cannot found Prop Name %s\n", pName );
//		BsAssert( 0 ); 
	}
	return pProp;
}





void CFcWorld::GetMapSize(float &fWidth,float &fHeight)
{
	fWidth=m_pMapInfo->GetXSize()*DISTANCE_PER_CELL;
	fHeight=m_pMapInfo->GetYSize()*DISTANCE_PER_CELL;
}

int CFcWorld::GetTroopCount()
{
	return m_pTroopManager->GetTroopCount();
}


int CFcWorld::GetPathCount()
{
	return m_pMapInfo->GetPathCount();
}

PATH_PACK* CFcWorld::GetPathInfo(int nIndex)
{
	return m_pMapInfo->GetPathInfo( nIndex );
}


void CFcWorld::LinkNavCells( std::vector<NavigationCell*>& navCells, int nNavEventInfoIndex )
{
	if ( !m_pNavigationMesh )
		return;

	int nStart = m_pNavEventInfo[nNavEventInfoIndex].nStartIndex;
	int nNum = m_pNavEventInfo[nNavEventInfoIndex].nNumNavIndex;
	BsAssert( nNum == navCells.size() );

	m_pNavigationMesh->SetUseCell( navCells, nStart, nNum );

	/*
	int nStartIndex = -1;
	int nNumIndex = 0;

	// O(n) link to cells not in navCells
	int numCells = navCells.size();
	for ( int i=0; i<numCells; i++ ) {

		NavigationCell* cell = new NavigationCell;
		cell = navCells[i];

		int nIndex = m_pNavigationMesh->AddCell( cell );
		if( nStartIndex == -1 )
		{
			nStartIndex = nIndex;
		}
		++nNumIndex;

		if (cell->Link(0))
			cell->Link(0)->RequestLink(cell->Vertex(0), cell->Vertex(1), cell);

		if (cell->Link(1))
			cell->Link(1)->RequestLink(cell->Vertex(1), cell->Vertex(2), cell);

		if (cell->Link(2))
			cell->Link(2)->RequestLink(cell->Vertex(2), cell->Vertex(0), cell);
	}

	BsAssert( m_vecNavEventInfo[nNavEventInfoIndex].nStartIndex == -1 );
	m_vecNavEventInfo[nNavEventInfoIndex].nStartIndex = nStartIndex;
	m_vecNavEventInfo[nNavEventInfoIndex].nNumNavIndex = nNumIndex;

	// O(n^2) link navCells (몇개 안된다, 보통 2개)
	m_pNavigationMesh->LinkCells( navCells );
//	m_pNavigationMesh->LinkCells();

//	navCells.clear();

*/

}


void CFcWorld::UnlinkNavCells( int nIndex )
{
	if( !m_pNavigationMesh )
		return;

	m_pNavigationMesh->SetUnuseCell( m_pNavEventInfo[nIndex].nStartIndex, m_pNavEventInfo[nIndex].nNumNavIndex );

/*
	int nCnt = m_vecNavEventInfo.size();
	for( int i=0; i<nCnt; i++ )
	{
		if( m_vecNavEventInfo[i].nStartIndex > m_vecNavEventInfo[nIndex].nStartIndex )
		{
			m_vecNavEventInfo[i].nStartIndex -= m_vecNavEventInfo[nIndex].nNumNavIndex;
		}
	}
	m_vecNavEventInfo[nIndex].nStartIndex = -1;
	m_vecNavEventInfo[nIndex].nNumNavIndex = 0;
*/
}


void CFcWorld::GetActiveProps( D3DXVECTOR3& Pos, float rRadius, std::vector<CFcProp*>& vecProps )
{
	m_pPropManager->GetActiveProps( Pos, rRadius, vecProps );
}

void CFcWorld::SetTriggerVar( int nVarID, int nValue )
{
	if(m_pTriggerCommander->GetVariableType( nVarID ) == BS_TG_VALTYPE_GLOBAL)
	{
		int nGlobalIndex = m_pTriggerCommander->GetVariableValInteger( nVarID );
		BsAssert(nGlobalIndex < MAX_TRG_VAR_NUM && "Trigger Global Variable Index Excess!");
		g_FCGameData.nTrgGlobalVar[ nGlobalIndex ] = nValue;
#ifdef FC_GLOBAL_VAR_DEBUG
		DebugString( "*************************************************\n" );
		DebugString( "Var index %d Global index %d SET %d\n",nVarID, nGlobalIndex,nValue );
		DebugString( "*************************************************\n" );
#endif
	}
	else
	{
		m_pTriggerCommander->SetVariableVal( nVarID, nValue );
	}
}	


void CFcWorld::SetTriggerEnable(int iTriggerIndex,bool bEnable)
{
	m_pTriggerCommander->EnableTrigger(iTriggerIndex,bEnable);
}


void CFcWorld::StopTrigger()
{
	m_pTriggerCommander->SetStopTrigger( true );
}


int CFcWorld::GetTriggerVarInt( int nVarID )
{
	if( m_pTriggerCommander->GetVariableType( nVarID ) == BS_TG_VALTYPE_GLOBAL )
	{
		int nGlobalIndex = m_pTriggerCommander->GetVariableValInteger( nVarID );
		BsAssert(nGlobalIndex < MAX_TRG_VAR_NUM && "Trigger Global Variable Index Excess!");
#ifdef FC_GLOBAL_VAR_DEBUG
		DebugString( "*************************************************\n" );
		DebugString( "Var index %d global Index %d GET %d\n", nVarID, nGlobalIndex, g_FCGameData.nTrgGlobalVar[ nGlobalIndex ] );
		DebugString( "*************************************************\n" );
#endif
		//char szErr[128];
		if( g_FCGameData.nTrgGlobalVar[ nGlobalIndex ] == -1 )
		{
			DebugString("It didn't initialize global var(Var index %d)\n",nVarID );
		}

		return g_FCGameData.nTrgGlobalVar[ nGlobalIndex ];
	}
	return m_pTriggerCommander->GetVariableValInteger( nVarID );
}


void CFcWorld::SetEnableTroopInArea( float fSX, float fSY, float fEX, float fEY )
{
	m_pTroopManager->SetEnableTroopInArea( fSX, fSY, fEX, fEY );
}

void CFcWorld::SetDisableEnemyTroopInArea( float fSX, float fSY, float fEX, float fEY )
{
	m_pTroopManager->SetDisableEnemyTroopInArea( fSX, fSY, fEX, fEY );
}

void CFcWorld::SetEnableEnemyTroopInArea( float fSX, float fSY, float fEX, float fEY )
{
	m_pTroopManager->SetEnableEnemyTroopInArea( fSX, fSY, fEX, fEY );
}

void CFcWorld::SetDisableFriendTroopInArea( float fSX, float fSY, float fEX, float fEY )
{
	m_pTroopManager->SetDisableFriendTroopInArea( fSX, fSY, fEX, fEY );
}

void CFcWorld::SetEnableFriendTroopInArea( float fSX, float fSY, float fEX, float fEY )
{
	m_pTroopManager->SetEnableFriendTroopInArea( fSX, fSY, fEX, fEY );
}

void CFcWorld::SetTimeMark( int nID )
{
	BsAssert( nID >=0 && nID < 15 );
//	BsAssert( m_TimeMark[nID] == INVALID_TIME_MARK );
	DebugString( "SetTimeMark %d, %d\n", nID, m_TimeMark[nID] );

	m_TimeMark[nID] = GetProcessTick() / FRAME_PER_SEC;
}

int CFcWorld::GetTimeMark( int nID )
{
	BsAssert( nID >=0 && nID < 15 );
	if( m_TimeMark[nID] == INVALID_TIME_MARK )
	{
		return -1;
	}
	return m_TimeMark[nID];
}







void CFcWorld::CmdGuardianAttack()
{
	m_pTroopManager->CmdGuardianAttack();
}

void CFcWorld::CmdGuardianDefense()
{
	m_pTroopManager->CmdGuardianDefense();
}

bool CFcWorld::CmdRegisterGuardian( int nIndex )
{
	return m_pTroopManager->CmdRegisterGuardian( nIndex );
}


int CFcWorld::GetPlayerCamIndex( int nPlayer )
{
	return m_nCamIndex[nPlayer];
}

void CFcWorld::SetBuildingAttrAtCrossAttr( PROPTYPEDATA* pData, DWORD dwRotAndScale, D3DXMATRIX* pMat, bool bSet )
{
	int nRot   = GET_ROTATE( dwRotAndScale );
	nRot *= 4;

	float fXRate = (float)(0.5f + ( 0.01f * (float)GET_XSCALE( dwRotAndScale ) ));
	float fZRate = (float)(0.5f + ( 0.01f * (float)GET_ZSCALE( dwRotAndScale ) ));

	int nAttrX = ((int)( pMat->_41 * 0.01f )) - ( pData->nAttrLeft);
	int nAttrY = ((int)( pMat->_43 * 0.01f )) + ( pData->nAttrTop);

	m_pPropManager->SetBuildingAttrAtCrossAttr( pData->nAttrXSize, pData->nAttrYSize, pData->pAttr,
		nAttrX, nAttrY, nRot, fXRate, fZRate, pMat,
		GetAttrXSize(), GetAttrYSize(), m_pMapInfo->GetAttrBuf(), m_pMapInfo->GetAttrExBuf(), bSet, pData->nType );
}

void CFcWorld::SetGuardianTroop( TroopObjHandle hTroop )
{
	m_pTroopManager->SetGuardianTroop( hTroop );
	CFcTroopAIObject::SetAI( hTroop, -1 );
}

static	float	FuncBlueLightOrbSpark(float fX)
{
	float	fRValue;
	float	fX1 = 1.0f - fX;

	fRValue = fX1 * sinf(pow(fX1,3.0f) * 3.141592f * .6f) + 0.025f;

	return fRValue;
}

static	float	FuncBlueLightOrbSpark2(float fX)
{
	float	fRValue = 0.0f;

	fRValue = cosf(fX * 3.141592f * 0.5f) + 0.50f;

	return fRValue;
}

void CFcWorld::GiveRangeDamageLight( GameObjHandle Attacker, D3DXVECTOR3 *pPosition ,float fMaxRadius, float fHeight, float fVXFactor, int nFuncType, HIT_TYPE nType /*= HT_NORMAL_ATTACK*/,int nOrbGenType /*= GENERATE_ORB_DISABLE*/,int nOrbGenPer/* = 0*/)
{

	int i, nSize;
	std::vector< GameObjHandle > Result;
	HIT_PARAM HitParam;

	BsAssert(fMaxRadius >0.0f);

	GetObjectListInRange( pPosition, fMaxRadius, Result );
	nSize = ( int )Result.size();
	
	DebugString("\n");

	for( i = 0; i < nSize; i++ )
	{
		if(Attacker == Result[i])
			continue;

		if (Result[i]->GetTeam() == 0)
			continue;

		D3DXVECTOR3	Pos = Result[i]->GetPos();
		D3DXVECTOR3	Err;

		Err = Pos - (*pPosition);

		float	fX1 = D3DXVec3Length(&Err) / fMaxRadius;
		float	fX2 = fX1 * (1.0f + fVXFactor);

		if (fX2 < 0.0f)
			fX2 = 0.0f;

		float	fY;

		switch (nFuncType)
		{
		case 0:
			fY = FuncBlueLightOrbSpark((fX1 + fX2) * .5f);
			break;

		case 1:
			fY = FuncBlueLightOrbSpark2((fX1 + fX2) * .5f);
			break;

		default:
			fY = FuncBlueLightOrbSpark((fX1 + fX2) * .5f);
			break;
		}
		
		
		// fx2, fY가 발사체가 정점에 이르렀을 때의 위치.

		fY *= fHeight;
		fX1 *= fMaxRadius;
		fX2 *= fMaxRadius;

		fY = fY + g_BsKernel.GetLandHeight(Result[i]->GetPos().x, Result[i]->GetPos().z) - Result[i]->GetPos().y;

		float	fG = -Result[i]->GetGravity();
		float	fVY,fT, fVX;

		if (fY > 0.0f)
		{
  			fVY = sqrt(2.0f * fG * fY);
			fT = sqrt(2.0f * fY / fG);	// 정점에 도달하는 시간을 계산
		}
		else
		{
			fVY = 0.0f;
			fT = sqrt(2.0f * -fY / fG);
		}

		if (fT > 0.0f)
			fVX = (fX2 - fX1) * .5f / fT; // 수평성분이 정점에 도착하는 동안 수직성분은 fX1에서 fX1과 fX2의 중점에 가 있어야 한다.
		else
			fVX = 0.0f;
 
		if (fVX < - 300.0f)
			fVX = -300.0f;

		if (fVX > 300.0f)
			fVX = 300.0f;

		HitParam.nAttackPower	= nFuncType * 1900/*Result[i]->GetMaxHP()*/ + 100;
		HitParam.nHitRemainFrame = 1;
		HitParam.ObjectHandle	= Attacker;
		HitParam.Position		= *pPosition;
		HitParam.pHitSignal		= m_HitSignal + m_nCurHitSignal;

		HitParam.pHitSignal->m_HitAniType = ANI_TYPE_DOWN;
		HitParam.pHitSignal->m_HitAniIndex = 0;
		HitParam.nNotGenerateOrb = nOrbGenType;
		HitParam.nGenerateOrbPercent = nOrbGenPer;
		HitParam.nNotLookAtHitUnit = 1;
		HitParam.nHitType = nType;
		

		// 힘, 위치에 따라 다르게 해줘야 한다.

//		DebugString("%f	%f	%f\n",fVX,fVY,Result[i]->GetPos().y);

		HitParam.fVelocityY = fVY;
		HitParam.fVelocityZ = -fVX;

		if( Result[i]->IsHittable( HitParam.ObjectHandle ) )
		{
			Result[i]->CmdHit( &HitParam );
			if( g_FcWorld.IsUseTrueOrb() )	// 함수 이름 좀 이상하다
				g_FcWorld.AddTroopDamagedTrueOrb( Result[ i ]->GetTroop() );
		}
	}
	m_nCurHitSignal++;
	if( m_nCurHitSignal >= MAX_HIT_SIGNAL_COUNT )
	{
		m_nCurHitSignal = 0;
	}

}


void CFcWorld::GiveRangeDamage( _FC_RANGE_DAMAGE &rd ,bool p_bAniDown /*=true*/, bool p_bIncludeDead /*=true*/)
{
	int i, nSize;
	std::vector< GameObjHandle > Result;
	HIT_PARAM HitParam;

	if( rd.nTeam == -1 )
	{
		GetObjectListInRange( rd.pPosition, rd.fRadius, Result );
	}
	else
	{
		GetEnemyObjectListInRange( rd.nTeam, rd.pPosition, rd.fRadius, Result, p_bIncludeDead );
	}
	nSize = ( int )Result.size();
	HitParam.nAttackPower = rd.nPower;
	HitParam.nHitRemainFrame = rd.nHitRemainFrame;
	HitParam.nAdjutantHitRemainRatio = rd.nAdjutantHitRemainRatio;
	HitParam.ObjectHandle = rd.Attacker;
	HitParam.Position = *rd.pPosition;
	HitParam.pHitSignal = m_HitSignal + m_nCurHitSignal;
	HitParam.nNotGenerateOrb = rd.nNotGenOrb;

	HitParam.nGenerateOrbPercent = rd.nOrbGenPer;
	HitParam.nNotLookAtHitUnit = rd.nNotLookAtHitUnit;
	HitParam.nHitType = rd.nType;
	HitParam.nGuardBreak = rd.nGuardBreak;


	if( p_bAniDown )
		HitParam.pHitSignal->m_HitAniType = ANI_TYPE_DOWN;
	else
		HitParam.pHitSignal->m_HitAniType = ANI_TYPE_HIT;
	HitParam.pHitSignal->m_HitAniIndex = 0;
	HitParam.pHitSignal->m_Hit2SoundAction = rd.SoundAttackType;
	HitParam.pHitSignal->m_Hit2WeaponMaterial = rd.WeaponType;
	// 힘, 위치에 따라 다르게 해줘야 한다.
	HitParam.fVelocityY = ( float )rd.nPushY;
	HitParam.fVelocityZ = ( float )rd.nPushZ;


	D3DXVECTOR3 ObjDir;
	for( i = 0; i < nSize; i++ )
	{	
		//D3DXVec3Normalize(&ObjDir, &(*pPosition - Result[ i ]->GetCrossVector()->GetPosition()) );
		//Result[ i ]->GetCrossVector()->m_ZVector = ObjDir;
		//Result[ i ]->GetCrossVector()->UpdateVectors();
		if(rd.Attacker == Result[i])
			continue;

		if (Result[i]->GetTeam() == 0 && rd.p_bTeamDamage)
			continue;

		// 왜 이중으로 체크해야 제대로 되는지 알아보자. by express8
		if( !p_bIncludeDead && Result[i]->GetHP() <= 0 ) 
			continue;

		if (Result[i]->IsJump())
			continue;

		if( Result[i]->GetCrossVector()->m_PosVector.y > rd.fYMax ||
			Result[i]->GetCrossVector()->m_PosVector.y < rd.fYMin )
			continue;

		if( Result[i]->IsHittable( HitParam.ObjectHandle ) )
		{
			if( rd.nType == HT_BY_TRAP )
			{
				int nRate = Result[ i ]->GetTroop()->GetTrapDamageAdd();
				HitParam.nAttackPower = HitParam.nAttackPower * (100 + nRate ) / 100;

				

				Result[ i ]->CmdHit( &HitParam, rd.pHitDirection );
				HitParam.nAttackPower = rd.nPower;
			}
			else
			{
				Result[ i ]->CmdHit( &HitParam, rd.pHitDirection );
				
				if( g_FcWorld.IsUseTrueOrb() )	// 함수 이름 좀 이상하다
					g_FcWorld.AddTroopDamagedTrueOrb( Result[ i ]->GetTroop() );
			}
		}
	}
	if( rd.Attacker == m_HeroHandle )
	{
		GivePhysicsRange( rd.Attacker, rd.pPosition, rd.fRadius, 10.0f, rd.nPower, PROP_BREAK_TYPE_NORMAL, rd.bDisableDynamicProp );
	}
	m_nCurHitSignal++;
	if( m_nCurHitSignal >= MAX_HIT_SIGNAL_COUNT )
	{
		m_nCurHitSignal = 0;
	}

}


void CFcWorld::GiveRangeDamageCustom( _FC_RANGE_DAMAGE &rd , bool p_bAniDown, float p_fPercent1, float p_fPercent2 )
{
	int i, nSize;
	std::vector< GameObjHandle > Result;
	HIT_PARAM HitParam;

	if( rd.nTeam == -1 )
	{
		GetObjectListInRange( rd.pPosition, rd.fRadius, Result );
	}
	else
	{
		GetEnemyObjectListInRange( rd.nTeam, rd.pPosition, rd.fRadius, Result );
	}
	nSize = ( int )Result.size();
	HitParam.nAttackPower = rd.nPower;
	HitParam.nHitRemainFrame = 1;
	HitParam.nAdjutantHitRemainRatio = rd.nAdjutantHitRemainRatio;
	HitParam.ObjectHandle = rd.Attacker;
	HitParam.Position = *rd.pPosition;
	HitParam.pHitSignal = m_HitSignal + m_nCurHitSignal;
	HitParam.nNotGenerateOrb = rd.nNotGenOrb;

	HitParam.nGenerateOrbPercent = rd.nOrbGenPer;
	HitParam.nNotLookAtHitUnit = rd.nNotLookAtHitUnit;
	HitParam.nHitType = rd.nType;
	HitParam.nHitRemainFrame = rd.nHitRemainFrame;

	if( p_bAniDown )
		HitParam.pHitSignal->m_HitAniType = ANI_TYPE_DOWN;
	else
		HitParam.pHitSignal->m_HitAniType = ANI_TYPE_HIT;
	HitParam.pHitSignal->m_HitAniIndex = 0;
	HitParam.pHitSignal->m_Hit2SoundAction = rd.SoundAttackType;
	HitParam.pHitSignal->m_Hit2WeaponMaterial = rd.WeaponType;
	// 힘, 위치에 따라 다르게 해줘야 한다.
	HitParam.fVelocityY = ( float )rd.nPushY;
	HitParam.fVelocityZ = ( float )rd.nPushZ;


	D3DXVECTOR3 ObjDir;
	for( i = 0; i < nSize; i++ )
	{	
		//D3DXVec3Normalize(&ObjDir, &(*pPosition - Result[ i ]->GetCrossVector()->GetPosition()) );
		//Result[ i ]->GetCrossVector()->m_ZVector = ObjDir;
		//Result[ i ]->GetCrossVector()->UpdateVectors();
		if(rd.Attacker == Result[i])
			continue;

		if (Result[i]->GetTeam() == 0 && rd.p_bTeamDamage)
			continue;

		if (Result[i]->IsJump())
			continue;

		if( Result[i]->GetCrossVector()->m_PosVector.y > rd.fYMax ||
			Result[i]->GetCrossVector()->m_PosVector.y < rd.fYMin )
			continue;

		if( Result[i]->IsHittable( HitParam.ObjectHandle ) )
		{
			if( rd.nType == HT_BY_TRAP )
			{
				int nRate = Result[ i ]->GetTroop()->GetTrapDamageAdd();
				HitParam.nAttackPower = HitParam.nAttackPower * (100 + nRate ) / 100;

				D3DXVECTOR3 vDist = Result[i]->GetPos() - *rd.pPosition;
				float fDist = D3DXVec3Length(&vDist);
				D3DXVec3Normalize(&vDist, &vDist);
				rd.pHitDirection = &vDist;
				if( fDist > rd.fRadius * p_fPercent1 )
				{
					HitParam.fVelocityY = ( float )rd.nPushY * p_fPercent2;
					HitParam.fVelocityZ = ( float )rd.nPushZ * p_fPercent2;
				}

				Result[ i ]->CmdHit( &HitParam, rd.pHitDirection );
				HitParam.nAttackPower = rd.nPower;
			}
			else
			{
				D3DXVECTOR3 vDist = Result[i]->GetPos() - *rd.pPosition;
				float fDist = D3DXVec3Length(&vDist);
				D3DXVec3Normalize(&vDist, &vDist);
				rd.pHitDirection = &vDist;
				if( fDist > rd.fRadius * p_fPercent1 )
				{
					HitParam.fVelocityY = ( float )rd.nPushY * p_fPercent2;
					HitParam.fVelocityZ = ( float )rd.nPushZ * p_fPercent2;
				}

				Result[ i ]->CmdHit( &HitParam, rd.pHitDirection );
			}
		}
	}
	if( rd.Attacker == m_HeroHandle )
	{
		GivePhysicsRange( rd.Attacker, rd.pPosition, rd.fRadius, 10.0f, rd.nPower, PROP_BREAK_TYPE_NORMAL, rd.bDisableDynamicProp);
	}
	m_nCurHitSignal++;
	if( m_nCurHitSignal >= MAX_HIT_SIGNAL_COUNT )
	{
		m_nCurHitSignal = 0;
	}

}


void CFcWorld::GiveRangeDamage( GameObjHandle Attacker, D3DXVECTOR3 *pPosition, float fRadius, int nPower, 
	int nPushY, int nPushZ, int nTeam/* = -1*/ , int nNotGenOrb/*= 0*/, int nNotLookAtHitUnit/*= 0*/,
	bool p_bTeamDamage /*= false*/, D3DXVECTOR3 *pHitDirection/* = NULL*/ ,HIT_TYPE nType /*= HT_NORMAL_ATTACK*/,
	float fYMax /*= 1000000.f*/, float fYMin /*= -1000000.f*/,
	SOUND_WEAPON_TYPE WeaponType /*=WT_NONE*/,SOUND_ATTACK_TYPE SoundAttackType/*=ATT_SKIP */, bool p_bAniDown /*=true*/)
{
	int i, nSize;
	std::vector< GameObjHandle > Result;
	HIT_PARAM HitParam;

	if( nTeam == -1 )
	{
		GetObjectListInRange( pPosition, fRadius, Result );
	}
	else
	{
		GetEnemyObjectListInRange( nTeam, pPosition, fRadius, Result );
	}
	nSize = ( int )Result.size();
	HitParam.nAttackPower = nPower;
	HitParam.nHitRemainFrame = 1;
	HitParam.ObjectHandle = Attacker;
	HitParam.Position = *pPosition;
	HitParam.pHitSignal = m_HitSignal + m_nCurHitSignal;
	HitParam.nNotGenerateOrb = nNotGenOrb;
	HitParam.nNotLookAtHitUnit = nNotLookAtHitUnit;
	HitParam.nHitType = nType;

	if( p_bAniDown )
        HitParam.pHitSignal->m_HitAniType = ANI_TYPE_DOWN;
	else
		HitParam.pHitSignal->m_HitAniType = ANI_TYPE_HIT;
	HitParam.pHitSignal->m_HitAniIndex = 0;
	HitParam.pHitSignal->m_Hit2SoundAction = SoundAttackType;
	HitParam.pHitSignal->m_Hit2WeaponMaterial = WeaponType;
	// 힘, 위치에 따라 다르게 해줘야 한다.
	HitParam.fVelocityY = ( float )nPushY;
	HitParam.fVelocityZ = ( float )nPushZ;

	D3DXVECTOR3 ObjDir;
	for( i = 0; i < nSize; i++ )
	{	
		//D3DXVec3Normalize(&ObjDir, &(*pPosition - Result[ i ]->GetCrossVector()->GetPosition()) );
		//Result[ i ]->GetCrossVector()->m_ZVector = ObjDir;
		//Result[ i ]->GetCrossVector()->UpdateVectors();
		if(Attacker == Result[i])
			continue;

		if (Result[i]->GetTeam() == 0 && p_bTeamDamage)
			continue;

		if (Result[i]->IsJump())
			continue;

		if( Result[i]->GetCrossVector()->m_PosVector.y > fYMax ||
			Result[i]->GetCrossVector()->m_PosVector.y < fYMin )
			continue;

		if( Result[i]->IsHittable( HitParam.ObjectHandle ) )
		{
			if( nType == HT_BY_TRAP )
			{
				int nRate = Result[ i ]->GetTroop()->GetTrapDamageAdd();
				HitParam.nAttackPower = HitParam.nAttackPower * (100 + nRate ) / 100;
				Result[ i ]->CmdHit( &HitParam, pHitDirection );
				HitParam.nAttackPower = nPower;
			}
			else
			{
				Result[ i ]->CmdHit( &HitParam, pHitDirection );
			}
		}
	}
	if( Attacker == m_HeroHandle )
	{
		GivePhysicsRange( Attacker, pPosition, fRadius, 10.0f, nPower, PROP_BREAK_TYPE_NORMAL );
	}
	m_nCurHitSignal++;
	if( m_nCurHitSignal >= MAX_HIT_SIGNAL_COUNT )
	{
		m_nCurHitSignal = 0;
	}
}



void CFcWorld::GivePhysicsRange( GameObjHandle Attacker, D3DXVECTOR3* pPos, float fRadius, float fPower, int nDamage, PROP_BREAK_TYPE nType, bool bDisableDynamicProp )
{
	float fLength;
	D3DXVECTOR3 Direction, Force;
	std::vector< CFcProp * > Result;

	CFcWorld::GetInstance().GetActiveProps( *pPos, fRadius * 3, Result );
	int nCount = ( int ) Result.size();
	for( int i = 0; i < nCount; i++ )
	{
		if( Result[ i ]->IsBreak() )
		{
			continue;
		}

		Direction = Result[ i ]->GetPos() - *pPos;
		fLength = D3DXVec3Length( &Direction );
		if( fLength - Result[ i ]->GetRadius() > fRadius )
		{
			continue;
		}
		D3DXVec3Normalize( &Direction, &Direction );
		if( Result[ i ]->IsDynamicProp() )
		{
			if( bDisableDynamicProp ) {
				continue;
			}
			Force = D3DXVECTOR3( 0.0f, 0.0f, 5.0f );
		}
		else
		{
			Force = D3DXVECTOR3( 0.0f, 5.0f, 5.0f );			
			D3DXVec3TransformNormal( &Force, &Force, ( D3DXMATRIX * )Attacker->GetCrossVector() );
		}
		Result[ i ]->AddForce( &Force, &Attacker->GetCrossVector()->GetPosition(), nDamage, nType );

//		파티클 위치와 파티클 정해줘야 한다.
/*
		Cross.m_PosVector = ( Result[ i ]->GetPos() - fRadius * Direction );
		Cross.m_PosVector.y += pSignal->m_PhysicsParticleY;
		g_BsKernel.CreateParticleObject( pSignal->m_PhysicsParticle, false, false, Cross );
*/
	}
}

void CFcWorld::GiveDamage( GameObjHandle Attacker, GameObjHandle Hitter, int nPower, int nAniType, int nAniTypeIndex, D3DXVECTOR3 *pVelocity , int nNotGenOrb/*= 0*/, int nNotLookAtHitUnit/*= 0*/, D3DXVECTOR3 *pPosition /*= NULL*/,HIT_TYPE nType /*= HT_NORMAL_ATTACK*/,int nOrbGenPer/* = 0*/)
{
	if( !CFcBaseObject::IsValid( Attacker ) ) return;
	if( !CFcBaseObject::IsValid( Hitter ) || !Hitter->CheckHit() ) return;
	if( !Hitter->IsHittable( Attacker ) ) return;
	HIT_PARAM HitParam;

	HitParam.nAttackPower = nPower;
	HitParam.nHitRemainFrame = 1;
	HitParam.ObjectHandle = Attacker;
	if(!pPosition )
		HitParam.Position = Attacker->GetPos();
	else
		HitParam.Position = *pPosition;
	HitParam.pHitSignal = m_HitSignal + m_nCurHitSignal;

	HitParam.pHitSignal->m_HitAniType = nAniType;
	HitParam.pHitSignal->m_HitAniIndex = nAniTypeIndex;
	HitParam.fVelocityY = pVelocity->y;
	HitParam.fVelocityZ = pVelocity->z;
	HitParam.nNotGenerateOrb = nNotGenOrb;
	HitParam.nNotLookAtHitUnit = nNotLookAtHitUnit;
	HitParam.nHitType = nType;
	HitParam.nGenerateOrbPercent = nOrbGenPer;

	Hitter->CmdHit( &HitParam );

	if( g_FcWorld.IsUseTrueOrb() )	// 함수 이름 좀 이상하다
		g_FcWorld.AddTroopDamagedTrueOrb( Hitter->GetTroop() );

	m_nCurHitSignal++;
	if( m_nCurHitSignal >= MAX_HIT_SIGNAL_COUNT )
	{
		m_nCurHitSignal = 0;
	}
}


void CFcWorld::SetPlayerEnable( bool bEnable )
{
	if( g_FcWorld.GetHeroHandle() ) {
		g_FcWorld.GetHeroHandle()->SetEnableAI( !bEnable );
		TroopObjHandle hTroop = g_FcWorld.GetHeroHandle()->GetTroop();
		CFcPlayerTroop* pTroop = (CFcPlayerTroop*)hTroop.m_pInstance;
		pTroop->SetControl( bEnable );
	}
}


#define ACTIVE_PROP_CHECK_RADIUS		8000.f

// 캡 느리다
// mruete: prefix bug 648: renamed bProp -> bPropOut
float CFcWorld::GetLandHeight( float fX, float fY, bool* pPropOut )
{
	float fPropHeight;
	bool bPropHeight = m_pPropManager->GetAABBTree().GetHeightEx( fX, fY, &fPropHeight);
	float fLandHeight = g_BsKernel.GetLandHeight( fX, fY );

	bool bPropOut = false;
	float fResultHeight = fLandHeight;

	if( bPropHeight )	{
		if( fPropHeight > fLandHeight ) {
			bPropOut = true;
			fResultHeight = fPropHeight;
		}
	}

	if( pPropOut ) {
		*pPropOut = bPropOut;
	}

	return fResultHeight;
}

float CFcWorld::GetLandHeight2( float fX, float fY, float p_fCharHeight, bool* pPropOut )
{
	float fPropHeight;
	bool bPropHeight = m_pPropManager->GetAABBTree().GetHeightEx( fX, fY, &fPropHeight);
	float fLandHeight = g_BsKernel.GetLandHeight( fX, fY );

	bool bPropOut = false;
	float fResultHeight = fLandHeight;

	if( bPropHeight )	{
		if( fPropHeight > fLandHeight && fPropHeight-p_fCharHeight < 800.0f ) {
			bPropOut = true;
			fResultHeight = fPropHeight;
		}
	}

	if( pPropOut ) {
		*pPropOut = bPropOut;
	}

	return fResultHeight;
}

float CFcWorld::GetLandHeightEx( float fX, float fY, D3DXVECTOR3* N /*= NULL*/ )
{
	float fPropHeight;
	bool bPropHeight = m_pPropManager->GetAABBTree().GetHeightEx( fX, fY, &fPropHeight);
	float fLandHeight = g_BsKernel.GetLandHeight( fX, fY, N );

	bool bPropOut = false;
	float fResultHeight = fLandHeight;

	if( bPropHeight )	{
		if( fPropHeight > fLandHeight ) {
			bPropOut = true;
			fResultHeight = fPropHeight;
		}
	}

	return fResultHeight;
}

float CFcWorld::GetLandHeightEx2( float fX, float fY, float p_fCharHeight, bool bRender, D3DXVECTOR3* N /*= NULL*/ )
{
	float fPropHeight;
	bool bPropHeight = m_pPropManager->GetAABBTree().GetHeightEx( fX, fY, &fPropHeight, bRender);
	float fLandHeight = g_BsKernel.GetLandHeight( fX, fY, N );
	
	float fResultHeight = fLandHeight;

	if( bPropHeight )	{
		if( fPropHeight > fLandHeight && fPropHeight-p_fCharHeight < 800.0f ) {
			fResultHeight = fPropHeight;
		}
	}

	return fResultHeight;
}

void CFcWorld::AddPosCheckCollisionProp( D3DXVECTOR3 Pos, float fHeight, float &fAddX, float &fAddZ)
{
	m_pPropManager->GetAABBTree().AddPosCheck( Pos, fHeight, fAddX, fAddZ);
}

void CFcWorld::AddPosCheckDynamicProp( D3DXVECTOR3 Pos, float &fAddX, float &fAddZ )
{
	std::vector<CFcProp*> &DynamicPropList = m_pPropManager->GetDynamicProps();

	D3DXVECTOR2 DestPos;
	DestPos.x = Pos.x + fAddX;	
	DestPos.y = Pos.z + fAddZ;

	int i, nSize;	
	nSize = DynamicPropList.size();
	for( i = 0; i < nSize; i++) {
		CFcProp *pProp = DynamicPropList[i];
		D3DXVECTOR2 PropPos(pProp->m_Cross.m_PosVector.x, pProp->m_Cross.m_PosVector.z);

		const float fMaxRadius = 300.f;

		if( fabs( PropPos.x - DestPos.x ) > fMaxRadius || fabs( PropPos.y - DestPos.y ) > fMaxRadius ) continue;

		float fRadius = pProp->GetRadius() + 50.f;			
		float fLengthSq = D3DXVec2LengthSq( &(DestPos - PropPos));			
		if( fLengthSq < (fRadius*fRadius) - 1.f ) { 
			float fLength = sqrtf( fLengthSq );
			D3DXVECTOR2 vDir = (DestPos - PropPos) * fRadius / fLength;
			DestPos.x = PropPos.x + vDir.x;
			DestPos.y = PropPos.y + vDir.y;
		}
	}

	fAddX = (DestPos.x) - Pos.x;
	fAddZ = (DestPos.y) - Pos.z;
}

int CFcWorld::GetLandAttrIndex( float fX, float fY )
{
	float fWidth,fHeight;
	GetMapSize( fWidth, fHeight );
	return m_pMapInfo->GetLayerAttr( ( int )( (fX / fWidth) * 512.f ), ( int )( (fY / fHeight) * 512.f ) );
}

int CFcWorld::GetStagePlayTime()
{
	int nSecond;
	LARGE_INTEGER liCurTime, liFrequency;

	QueryPerformanceCounter( &liCurTime );
	QueryPerformanceFrequency( &liFrequency );
	nSecond = (int)( ( liCurTime.QuadPart - m_liStageStartTime.QuadPart - g_liStagePauseTime.QuadPart) / liFrequency.QuadPart );

	return nSecond;

	//return GetProcessTick() / 40;
}

bool CFcWorld::LoadTrigger( const char* pFileName )
{
	BsAssert( m_pTriggerCommander == NULL );

	char cName[MAX_PATH];
	strcpy( cName, pFileName );
	RemoveEXT( cName );
	//	strcat( cName, ".txt" );
	strcat( cName, ".bstrg" );
	char* pFullName = g_BsKernel.GetFullName( cName );

	m_pTriggerCommander = new CFcTriggerCommander;
	m_pTriggerCommander->Load( pFullName );
	return true;
}


D3DXVECTOR2 CFcWorld::GetCameraDir( int nPlayer )
{
	CameraObjHandle CamHandle;
	CCrossVector* Cross;

	CamHandle = CFcBaseObject::GetCameraObjectHandle( nPlayer );
	BsAssert( CamHandle );
	Cross = CamHandle->GetCrossVector();

	D3DXVECTOR3 ZVec = Cross->m_ZVector;
	D3DXVECTOR2 Dir( ZVec.x, ZVec.z );
	D3DXVec2Normalize( &Dir, &Dir );		
	return D3DXVECTOR2( ZVec.x, ZVec.z );

}

void CFcWorld::CheckUnitUpdateObject()
{
	if( m_pTroopManager == NULL )
	{
		return;
	}

	CameraObjHandle CamHandle;
	CCrossVector* Cross;

	// 모든 Unit UpdateObject disable
	int nCnt = m_pTroopManager->GetTroopCount();
	for( int i=0; i<nCnt; i++ )
	{
		TroopObjHandle hTroop = m_pTroopManager->GetTroop( i );
		int nUnitCnt = hTroop->GetUnitCount();

		if( hTroop->GetLeader() )
		{
			hTroop->GetLeader()->SetUpdateObject( false );
		}

		for( int j=0; j<nUnitCnt; j++ )
		{
			UnitObjHandle hUnit = hTroop->GetUnit( j );
			hUnit->SetUpdateObject( false );
		}
	}

	for( int k=0; k<2; k++ )
	{
		if( m_nCamIndex[k] == -1 )
		{
			continue;
		}
		CamHandle = CFcBaseObject::GetCameraObjectHandle( k );
		BsAssert( CamHandle );
		Cross = CamHandle->GetCrossVector();
		D3DXVECTOR2 Dir( Cross->m_ZVector.x, Cross->m_ZVector.z );
		D3DXVec2Normalize( &Dir, &Dir );
		float fRadius = m_pMapInfo->GetSetInfo()->fFar/ 2.f;
		Dir = Dir * fRadius;
		D3DXVECTOR3 Pos( Dir.x + Cross->m_PosVector.x, Cross->m_PosVector.y, Dir.y + Cross->m_PosVector.z );
		std::vector<GameObjHandle> Objects;
		GetObjectListInRange( &Pos, fRadius + 1000.f, Objects);
		int nCnt = Objects.size();
		for( int i=0; i<nCnt; i++ )
		{
			GameObjHandle hUnit = Objects[i];
			hUnit->SetUpdateObject( true );
		}
	}
}


int CFcWorld::CalcPlayerCount()
{
	int nCurPlayerCount = 0;
	bool bIsPlayer[2] = { false, false };
	TROOP_INFO *pTroopInfo;
	int nTroopCount = m_pMapInfo->GetTroopCount();
	for(int i = 0; i < nTroopCount; i++)
	{
		pTroopInfo=m_pMapInfo->GetTroopInfo( i );

		switch( pTroopInfo->m_troopAttr.nTroopType ) {
			case TROOPTYPE_PLAYER_1:	
				nCurPlayerCount++;
				bIsPlayer[ pTroopInfo->m_troopAttr.nTroopType ] = true;
				break;
			case TROOPTYPE_PLAYER_2:
				nCurPlayerCount++;
				bIsPlayer[ pTroopInfo->m_troopAttr.nTroopType ] = true;
				break;
		}
	}

	if( nCurPlayerCount == 0 ) {
		BsAssert( 0 && "Player Troop Is not setting" );
		return 0;
	}
	else if( nCurPlayerCount == 1 && bIsPlayer[0] == false ) {
		BsAssert( 0 && "Player Troop is invalid setting" );
		return 0;
	}
	else if( nCurPlayerCount == 2 && ( bIsPlayer[0] == false || bIsPlayer[1] == false ) ) {
		BsAssert( 0 && "Player Troop is invalid setting" );
		return 0;
	}

	return nCurPlayerCount;
}

bool CFcWorld::LoadUnitData()
{
	char buf[256];
	sprintf( buf, "LoadUnitData 1\n" );
	NextTimeMark( buf );

	std::vector<UNITLOADINFO> vecUnitIndex;
	std::vector<AILOADINFO> vecAIList;

	int nTroopCount = m_pMapInfo->GetTroopCount();
	char szStr[MAX_PATH];
	for(int i = 0; i < nTroopCount; i++)
	{
		TROOP_INFO *pTroopInfo=m_pMapInfo->GetTroopInfo( i );

		int nType = pTroopInfo->m_troopAttr.nLeaderUnitType;
		if( nType != -1 )
		{
			int nSkinLevel = GetSkinLevel( nType, pTroopInfo->m_troopAttr.nLeaderLevel );
			CheckAndAddUnitPreLoad( nType, nSkinLevel, pTroopInfo->m_troopAttr.nLeaderLevel, pTroopInfo->m_troopAttr.nGroup != 0, vecUnitIndex );
			CheckAndAddAIPreLoad( nType, pTroopInfo->m_troopAttr.StrBlock.szLeaderUnitAI, pTroopInfo->m_troopAttr.nLeaderLevel, pTroopInfo->m_troopAttr.nGroup != 0, vecAIList );
		}

		for(int j = 0; j < 5; j++)
		{
			int nType = pTroopInfo->m_troopAttr.nUnitType[ j ];
			if( nType != -1 )
			{
				int nSkinLevel = GetSkinLevel( nType, pTroopInfo->m_troopAttr.nTroopLevel );
				CheckAndAddUnitPreLoad( nType, nSkinLevel, pTroopInfo->m_troopAttr.nTroopLevel, pTroopInfo->m_troopAttr.nGroup != 0, vecUnitIndex );
				CheckAndAddAIPreLoad( nType, pTroopInfo->m_troopAttr.StrBlock.szUnitAI[j], pTroopInfo->m_troopAttr.nTroopLevel, pTroopInfo->m_troopAttr.nGroup != 0, vecAIList );
			}
		}
/* 말 없음
		for(int j = 0; j < 3; j++)
		{
			int nType = pTroopInfo->m_troopAttr.nUnitHorseType[ j ];
			if( nType != -1 )
			{
				bool bExist = IsUnitIndex( nType, vecUnitIndex );
				if( bExist == false ) {
					vecUnitIndex.push_back( nType );

					for( int k=0; k<5; k++ ) {
						int nUnitIndex = pTroopInfo->m_troopAttr.nUnitType[k];
						sprintf( szStr, "%s_horse.ai", CUnitSOXLoader::GetInstance().GetUnitData( nUnitIndex )->cUnitName );
						vecAIName.push_back( szStr );
					}
				}
			}
		}
*/
	}

	//yooty
	for(int i=0; i<MAX_GUARDIAN_SLOT_NUM; i++)
	{
		int nUnitIndex = g_FCGameData.GuardianInfo[i].nID;
		if(nUnitIndex == -1){
			continue;
		}

		// TODO : 호위부대 레벨 여기서 세팅해야 한다.
		int nSkinLevel = GetSkinLevel( nUnitIndex, g_FCGameData.tempUserSave.nLevel );
		/*
		bool bExist = IsUnitIndex( nUnitIndex, nSkinLevel, vecUnitIndex );
		if( bExist == false ) {

			UNITLOADINFO LoadInfo;
			LoadInfo.nUnitIndex = nUnitIndex;
			LoadInfo.nSkinLevel = nSkinLevel;
			LoadInfo.bEnemy = false;
			vecUnitIndex.push_back( LoadInfo );
		}
		*/
		CheckAndAddUnitPreLoad( nUnitIndex, nSkinLevel, 0, false, vecUnitIndex );

		sprintf( szStr, "%s_Default.ai", CUnitSOXLoader::GetInstance().GetUnitData( nUnitIndex )->cUnitName );
		CheckAndAddAIPreLoad( nUnitIndex, szStr, 0, false, vecAIList );
	}

	// 추가 유닛 데이타 체크
	int nCnt = vecUnitIndex.size();
	for( int i=0; i<nCnt; i++ ) {
		switch( CUnitSOXLoader::GetInstance().GetUnitType( vecUnitIndex[i].nUnitIndex ) ) {
			case CFcGameObject::Class_ID_Catapult:	
				{
					int nSkinLevel = GetSkinLevel( 34, 0 );		// 레벨 0으로 세팅
					CheckAndAddUnitPreLoad( 34, nSkinLevel, vecUnitIndex[i].nLevel, vecUnitIndex[i].bEnemy, vecUnitIndex );

					sprintf( szStr, "%s_Default.ai", CUnitSOXLoader::GetInstance().GetUnitData( 34 )->cUnitName );
					CheckAndAddAIPreLoad( 34, szStr, vecUnitIndex[i].nLevel, vecUnitIndex[i].bEnemy, vecAIList );
				}
				break;
			case CFcGameObject::Class_ID_MoveTower:
				{
					int nSkinLevel = GetSkinLevel( 28, 0 );		// 레벨 0으로 세팅
					CheckAndAddUnitPreLoad( 34, nSkinLevel, vecUnitIndex[i].nLevel, vecUnitIndex[i].bEnemy, vecUnitIndex );

					sprintf( szStr, "%s_Default.ai", CUnitSOXLoader::GetInstance().GetUnitData( 28 )->cUnitName );
					CheckAndAddAIPreLoad( 28, szStr, vecUnitIndex[i].nLevel, vecUnitIndex[i].bEnemy, vecAIList );
				}
				break;
		}
	}

	sprintf( buf, "LoadUnitData 2\n" );
	NextTimeMark( buf );

	nCnt = vecUnitIndex.size();
	for( int i=0; i<nCnt; i++ )
	{
		INITGAMEOBJECTDATA Data;
		Data.nUnitIndex = vecUnitIndex[i].nUnitIndex;
		Data.pUnitSOX=CUnitSOXLoader::GetInstance().GetUnitData(Data.nUnitIndex);

		if( (CFcGameObject::GameObj_ClassID)Data.pUnitSOX->nUnitType >= Class_ID_Hero ) {
			MainHeroFxLoading( Data.pUnitSOX->nUnitType, false );
		}

		char szTemp[256];
		sprintf( szTemp, "Unit %s", Data.pUnitSOX->cUnitName );

		g_pSoundManager->LoadCharWaveBank( Data.pUnitSOX->cWBKakegoe );
		g_pSoundManager->LoadCharWaveBank( Data.pUnitSOX->cWBCrowd );

		g_BsMemChecker.Start( szTemp );

		int nSkinLevel = vecUnitIndex[i].nSkinLevel;
		char szFileName[_MAX_PATH];
		for( int j=0; j<Data.pUnitSOX->cSkinVariationNum; j++ )
		{
			if( nSkinLevel == -1 )
			{
				sprintf(szFileName, "%s%s%d.skin", Data.pUnitSOX->cSkinDir, Data.pUnitSOX->cSkinFileName, j+1);
				Data.nSkinIndex[j] = LoadMeshData(-1, szFileName);
				BsAssert( Data.nSkinIndex[j] >= 0 );
			}
			else
			{
				BsAssert( nSkinLevel >=0 && nSkinLevel < 4 );
				sprintf(szFileName, "%s%s%d_A.skin", Data.pUnitSOX->cSkinDir, Data.pUnitSOX->cSkinFileName, j+1);
				int nIndex = LoadMeshData(-1, szFileName);
				if( nSkinLevel == 0 )
					Data.nSkinIndex[j] = nIndex;

				sprintf(szFileName, "%s%s%d_D.skin", Data.pUnitSOX->cSkinDir, Data.pUnitSOX->cSkinFileName, j+1);
				nIndex = LoadMeshData(-1, szFileName);
				if( nSkinLevel == 3 )
					Data.nSkinIndex[j] = nIndex;

				BsAssert( Data.nSkinIndex[j] >= 0 );
			}
			Data.nSkinLevel = nSkinLevel;
		}
		Data.nAniIndex = LoadAniData(-1, Data.pUnitSOX->cAniName);
		BsAssert( Data.nAniIndex >= 0 );

		ASData *pASData;
		int *pUseList;

		sprintf(szFileName, "as\\%s", Data.pUnitSOX->cASFileName );
		pASData = ASData::LoadASData( szFileName, &Data.nASDataIndex );

		int nAniCnt = g_BsKernel.GetAniCount( Data.nAniIndex );
//		BsAssert( nAniCnt == pASData->GetAniNum() );
		if( nAniCnt != pASData->GetAniNum() )
		{
			DebugString( "Not match Ani & AS file! %s\n", Data.pUnitSOX->cAniName );
		}

		pUseList = pASData->GetUseParticleList();
		for( int j = 0; j < pASData->GetUseParticleCount(); j++ )
		{
			BsAssert( pUseList[ j ] < MAX_PARTICLE_LOAD_COUNT);
			m_nUseParticleTable[ pUseList[ j ] ] = 1;
		}
		pUseList = pASData->GetUseFXList();

		for( int j = 0; j < pASData->GetUseFXCount(); j++ )
		{
			BsAssert( pUseList[ j ] < MAX_FX_LOAD_COUNT);
			m_nUseFXTable[ pUseList[ j ] ] = 1;
		}

		sprintf(szFileName, "as\\%s", Data.pUnitSOX->cUnitInfoFileName );
		CAniInfoData* pAniInfoData = CAniInfoData::LoadAniInfoData( szFileName, &Data.nAniInfoDataIndex );

		//-Unit billboard texture를 로딩하는 부분과 animation offset을 셋팅하는 부분-----
		char szName[256];
//		strcpy( szName, Data.pUnitSOX->cBillboardName );

		int nBillboardIndex = -1;
		if( nSkinLevel == -1 )
		{
			nBillboardIndex = 0;
		}
		else
		{
			nBillboardIndex = nSkinLevel;
		}

		if ( Data.pUnitSOX->cBillboardName[0] != NULL && _strcmpi( Data.pUnitSOX->cBillboardName[nBillboardIndex], "noname" ) != 0 ) {

			BsAssert( nBillboardIndex >=0 && nBillboardIndex < 4 );
			sprintf( szName, "%s%s", g_BsKernel.GetCurrentDirectory(), Data.pUnitSOX->cBillboardName[nBillboardIndex] );
			
			Data.nBillboardIndex = g_BsKernel.GetInstance().RegisterAniBillboardType(szName, Data.nSkinIndex,
				Data.pUnitSOX->cSkinVariationNum, pAniInfoData->GetAniNum(), Data.pUnitSOX->nBillboardSize); 
			if (Data.nBillboardIndex != -1) {
				for(int j = 0; j < pAniInfoData->GetAniNum(); j++ ) {
					ASAniData* pData = pAniInfoData->GetAniData( j );
					int nIndex = pData->GetAniType();
					int nAniAttr=0;
					switch( pData->GetAniAttr() ) {
						case ANI_ATTR_NONE:		nAniAttr = 0;	break;	// None
						case ANI_ATTR_BATTLE:	nAniAttr = 1;	break;	// Battle
						case ANI_ATTR_BACK:		nAniAttr = 2;	break;	// Back
						case ANI_ATTR_HORSE:	nAniAttr = 3;	break;	// Horse
					}
					g_BsKernel.GetInstance().SetBillboardAniOffset( Data.nBillboardIndex, j, g_BillboardAnimInfo[nIndex][nAniAttr], nIndex, nAniAttr );
				}
			}
			else {
#ifdef _XBOX
				BsAssert(0 && "Unit texture load failed.");
#endif
			}
		}

		//////////////////////////////////////////////////////////////////////////////////

		// Parts
		ASPartsData *pPartsData;
		int nPartsGroupCount = pAniInfoData->GetPartsGroupCount();
		for(int j = 0; j < nPartsGroupCount; j++ )
		{
			pPartsData = pAniInfoData->GetPartsGroupInfo( j );

			

			switch( pPartsData->m_nType - 1 )
			{
			case PARTS_TYPE_WEAPON:
			case PARTS_TYPE_PROJECTILE:
			case PARTS_TYPE_ARMOR:
				{
					if( !(pPartsData->m_nSimulation == PHYSICS_CLOTH || 
						pPartsData->m_nSimulation == PHYSICS_COLLISION || 
						pPartsData->m_nSimulation == PHYSICS_RAGDOLL) )
					{
						int nVariation = pPartsData->GetPartsCount();
						for( int k=0; k<nVariation; k++ )
						{
							g_BsKernel.SetInstancingBufferSize(100);
							pPartsData->m_pSkinIndex[k] = LoadMeshData( -1, pPartsData->GetPartsSkinName( k ) );
							g_BsKernel.SetInstancingBufferSize(5);
							BsAssert( pPartsData->m_pSkinIndex[k] >= 0 );

							/*
							pPartsData->m_pSkinIndex[k] = LoadMeshData( -1, pPartsData->GetPartsSkinName( k ) );
							char cPartsName[MAX_PATH];
							strcpy( cPartsName, pPartsData->GetPartsSkinName( k ) );
							char* pStr = strstr( cPartsName, "." );
							if( pStr )
							{
								pStr[0] = NULL;
								strcat( pStr, ".bm" );
							}

							pPartsData->m_pSkinIndex[k] = LoadMeshData( -1, cPartsName );

							// 여기서 Instancing으로 등록해야 한다.

							BsAssert( pPartsData->m_pSkinIndex[k] >= 0 );
							*/
						}
					}
				}
				break;
			default:
				continue;
			}
#ifdef _XBOX	// by Siva
			if( pPartsData->m_nSimulation == PHYSICS_CLOTH )
			{            
				int nSimulParts = pPartsData->GetPartsCount();

				if( nSkinLevel == -1 )
				{
					for( int k = 0; k <nSimulParts ; k++ )
					{
						char szPhysicsFileName[255];
						strcpy( szPhysicsFileName, pPartsData->GetPartsSkinName( k ) );

						sprintf(szFileName, "%s%s%d.skin", Data.pUnitSOX->cSkinDir, Data.pUnitSOX->cSkinFileName, j+1);
						pPartsData->m_pSkinIndex[k] = LoadMeshData( -1, pPartsData->GetPartsSkinName( k ) );
						RemoveEXT( szPhysicsFileName );
						strcat( szPhysicsFileName, ".txt" );
						BsAssert( pPartsData->m_pSkinIndex[k] >= 0 );
						PHYSICS_INIT_INFO PhysicsInfo;
						PhysicsInfo.pData = CFcPhysicsLoader::LoadPhysicsData( szPhysicsFileName );
						PhysicsInfo.nSkinIndex = pPartsData->m_pSkinIndex[k];
						Data.vecPhysicsInfo.push_back( PhysicsInfo );
					}
				}
				else
				{
					delete [] pPartsData->m_pSkinIndex;
					pPartsData->m_pSkinIndex = new int [ nSimulParts * 4 ];

					for( int k = 0; k <nSimulParts ; k++ )
					{
						char szPhysicsFileName[255];
						strcpy( szPhysicsFileName, pPartsData->GetPartsSkinName( k ) );
						
						int nUseLevel[ 2 ] = { 0, 3 };
						for( int nLevel = 0; nLevel < 2; nLevel ++ )
						{						
							char cLevel = 'A' + nUseLevel[nLevel];
							char szFileName[255];						

							RemoveEXT( szPhysicsFileName );

							sprintf(szFileName, "%s_%c.skin", szPhysicsFileName, cLevel );
							int nSkinIndex = LoadMeshData( -1, szFileName );							
							BsAssert( nSkinIndex >= 0 );
							PHYSICS_INIT_INFO PhysicsInfo;

							RemoveEXT( szPhysicsFileName );
							strcat( szPhysicsFileName, ".txt" );
							PhysicsInfo.pData = CFcPhysicsLoader::LoadPhysicsData( szPhysicsFileName );
							PhysicsInfo.nSkinIndex = nSkinIndex;
							Data.vecPhysicsInfo.push_back( PhysicsInfo );
							pPartsData->m_pSkinIndex[ k + nSimulParts * nUseLevel[nLevel]  ] = nSkinIndex;							
						}
					}
				}
			}
			else if( pPartsData->m_nSimulation == PHYSICS_COLLISION)
			{
				Data.pCollisionMesh = CFcPhysicsLoader::LoadCollisionMesh( pPartsData->GetPartsSkinName( 0 ) );
			}
			else if( pPartsData->m_nSimulation == PHYSICS_RAGDOLL)
			{
				Data.pRagdollSetting = CFcPhysicsLoader::LoadRagdollSetting( pPartsData->GetPartsSkinName( 0 ) );
			}
			else if( pPartsData->m_nSimulation == PHYSICS_BREAKABLE ) 
			{
				char szPhysicsFileName[255];
				strcpy( szPhysicsFileName, pPartsData->GetPartsSkinName( 0 ) );
				RemoveEXT( szPhysicsFileName );
				strcat( szPhysicsFileName, ".txt" );

				pPartsData->m_pSkinIndex[0] = LoadMeshData( -1, pPartsData->GetPartsSkinName( 0 ) );

				PHYSICS_INIT_INFO PhysicsInfo;
				PhysicsInfo.pData = CFcPhysicsLoader::LoadPhysicsData( szPhysicsFileName );
				PhysicsInfo.nSkinIndex = pPartsData->m_pSkinIndex[0];
				Data.vecPhysicsInfo.push_back( PhysicsInfo );
			}
#endif //_XBOX
		}
		m_vecInitGameObj.push_back( Data );
		g_BsMemChecker.End();
	}

	sprintf( buf, "LoadUnitData 3 Count:%d\n", nCnt );
	NextTimeMark( buf );

//--------------------------------------------------------------------------------

	g_BsMemChecker.Start( "Load unit AI" );

	// AI Load
	PROFILE_TICK_TEST_BLOCK_START( "AI PreLoad" );
	AIObjHandle AIHandle;
//	GameObjHandle hDummyHandle[MAX_NPC_LEVEL];
	GameObjHandle hDummyHandle;
	CCrossVector Cross;
	/*
	for( int i=0; i<MAX_NPC_LEVEL; i++ ) {
		hDummyHandle[i] = CFcGameObject::CreateGameObject( CFcGameObject::Class_ID_Unknown, &Cross );
		hDummyHandle[i]->SetLevel(i);
	}
	*/
	hDummyHandle = CFcGameObject::CreateGameObject( CFcGameObject::Class_ID_Unknown, &Cross );

	for( DWORD i=0; i<vecAIList.size(); i++ ) {
		hDummyHandle->SetLevel( vecAIList[i].nLevel );
		hDummyHandle->SetTeam( ( vecAIList[i].bEnemy == true ) ? 1 : 0 );

		AIHandle = CFcAIObject::CreateObject<CFcAIObject>();
		if( AIHandle->Initialize( vecAIList[i].szAIName.c_str(), hDummyHandle, true ) == false ) {
			delete AIHandle;
		}

//		int nMaxLevel = CUnitSOXLoader::GetInstance().GetMaxLevel( vecAIList[i].nUnitIndex );
//		for( int j=0; j<nMaxLevel; j++ ) {
//		}
	}
	delete hDummyHandle;

	// 실기 무비 캐릭터 예외 처리!!
	
	if( _strcmpi( g_FCGameData.cMapFileName, "ev_as.bsmap" ) == 0 )
	{
		LoadMeshData(-1, "D:\\data\\char\\C_DN_GB1\\C_DN_GB1.SKIN");
		LoadMeshData(-1, "D:\\data\\char\\EV_MS1\\EV_MS1.SKIN");
	}
	else if( _strcmpi( g_FCGameData.cMapFileName, "ev_in.bsmap" ) == 0 )
	{
		LoadMeshData(-1, "D:\\data\\char\\C_DN_GB1\\C_DN_GB1.SKIN");
		LoadMeshData(-1, "D:\\data\\char\\EV_MS1\\EV_MS1.SKIN");
	}
	else if( _strcmpi( g_FCGameData.cMapFileName, "vf_in.bsmap" ) == 0 )
	{
		LoadMeshData(-1, "D:\\data\\char\\C_LN_LS1\\C_LN_LS1.SKIN");
		LoadMeshData(-1, "D:\\data\\char\\C_LN_LS1\\PH_PARTS\\C_LN_LS1_HEAD.SKIN");
		CFcPhysicsLoader::LoadPhysicsData( "char\\C_LN_LS1\\PH_PARTS\\C_LN_LS1_HEAD.TXT" );
		LoadMeshData(-1, "D:\\data\\char\\C_LN_LS1\\PH_PARTS\\C_LN_LS1_NECK.SKIN");
		CFcPhysicsLoader::LoadPhysicsData( "char\\C_LN_LS1\\PH_PARTS\\C_LN_LS1_NECK.TXT" );
		LoadMeshData(-1, "D:\\data\\char\\C_LN_LS1\\PH_PARTS\\C_LN_LS1_SKIRT.SKIN");
		CFcPhysicsLoader::LoadPhysicsData( "char\\C_LN_LS1\\PH_PARTS\\C_LN_LS1_SKIRT.TXT" );
		CFcPhysicsLoader::LoadCollisionMesh( "char\\C_LN_LS1\\PH_PARTS\\C_LN_SL1_collision.txt" );
		LoadMeshData(-1, "D:\\data\\char\\C_LN_LS1\\PH_PARTS\\C_LN_SL1_SILKKK.SKIN");
		CFcPhysicsLoader::LoadPhysicsData( "char\\C_LN_LS1\\PH_PARTS\\C_LN_SL1_SILKKK.TXT" );
		LoadMeshData(-1, "D:\\data\\char\\C_LN_LS1\\PH_PARTS\\C_LN_SL1_WAVE.SKIN");
		CFcPhysicsLoader::LoadPhysicsData( "char\\C_LN_LS1\\PH_PARTS\\C_LN_SL1_WAVE.TXT" );
	}
	else if( _strcmpi( g_FCGameData.cMapFileName, "vg_dw.bsmap" ) == 0 )
	{
		LoadMeshData(-1, "D:\\data\\char\\C_DN_GB2\\C_DN_GB21.SKIN");
	}
	else if( _strcmpi( g_FCGameData.cMapFileName, "vz_dw2.bsmap" ) == 0 )
	{
		LoadMeshData(-1, "D:\\data\\char\\C_DN_GB1\\C_DN_GB1.SKIN");
	}
	else if( _strcmpi( g_FCGameData.cMapFileName, "wy_as.bsmap" ) == 0 ||
			_strcmpi( g_FCGameData.cMapFileName, "wy_in.bsmap" ) == 0 ||
			_strcmpi( g_FCGameData.cMapFileName, "wy_kl.bsmap" ) == 0 ||
			_strcmpi( g_FCGameData.cMapFileName, "wy_ty.bsmap" ) == 0 )
	{
		LoadMeshData(-1, "D:\\data\\char\\C_DN_GK3\\C_DN_GK31.SKIN");
		LoadMeshData(-1, "D:\\data\\char\\C_DN_GK3\\PH_PARTS\\C_DN_GK3_CLOTH.SKIN");
		CFcPhysicsLoader::LoadPhysicsData( "char\\C_DN_GK3\\PH_PARTS\\C_DN_GK3_CLOTH.txt" );

		LoadMeshData(-1, "D:\\data\\char\\C_DN_GK4\\C_DN_GK41.SKIN");
		CFcPhysicsLoader::LoadCollisionMesh( "char\\C_DN_GK4\\PH_PARTS\\c_dn_gk4_collision.txt" );
		LoadMeshData(-1, "D:\\data\\char\\C_DN_GK4\\PH_PARTS\\C_DN_GK4_SKIRT.SKIN");
		CFcPhysicsLoader::LoadPhysicsData( "char\\C_DN_GK4\\PH_PARTS\\c_dn_gk4_skirt.txt" );
	}
	else if( _strcmpi( g_FCGameData.cMapFileName, "awed.bsmap" ) == 0 )
	{
		LoadMeshData(-1, "D:\\data\\char\\C_LN_LS1\\C_LN_LS1.SKIN");
		LoadMeshData(-1, "D:\\data\\char\\C_LN_LS1\\PH_PARTS\\C_LN_LS1_HEAD.SKIN");
		CFcPhysicsLoader::LoadPhysicsData( "char\\C_LN_LS1\\PH_PARTS\\C_LN_LS1_HEAD.TXT" );
		LoadMeshData(-1, "D:\\data\\char\\C_LN_LS1\\PH_PARTS\\C_LN_LS1_NECK.SKIN");
		CFcPhysicsLoader::LoadPhysicsData( "char\\C_LN_LS1\\PH_PARTS\\C_LN_LS1_NECK.TXT" );
		LoadMeshData(-1, "D:\\data\\char\\C_LN_LS1\\PH_PARTS\\C_LN_LS1_SKIRT.SKIN");
		CFcPhysicsLoader::LoadPhysicsData( "char\\C_LN_LS1\\PH_PARTS\\C_LN_LS1_SKIRT.TXT" );
		CFcPhysicsLoader::LoadCollisionMesh( "char\\C_LN_LS1\\PH_PARTS\\C_LN_SL1_collision.txt" );
		LoadMeshData(-1, "D:\\data\\char\\C_LN_LS1\\PH_PARTS\\C_LN_SL1_SILKKK.SKIN");
		CFcPhysicsLoader::LoadPhysicsData( "char\\C_LN_LS1\\PH_PARTS\\C_LN_SL1_SILKKK.TXT" );
		LoadMeshData(-1, "D:\\data\\char\\C_LN_LS1\\PH_PARTS\\C_LN_SL1_WAVE.SKIN");
		CFcPhysicsLoader::LoadPhysicsData( "char\\C_LN_LS1\\PH_PARTS\\C_LN_SL1_WAVE.TXT" );
	}
	else if( _strcmpi( g_FCGameData.cMapFileName, "ced01.bsmap" ) == 0 )
	{
		LoadMeshData(-1, "D:\\data\\char\\C_DN_GB1\\C_DN_GB1.SKIN");
	}
	else if( _strcmpi( g_FCGameData.cMapFileName, "yw_in.bsmap" ) == 0 )
	{
		LoadMeshData(-1, "D:\\data\\sky\\S_VZ_MY0_00.SKIN");
	}

	/*
	for( int i=0; i<MAX_NPC_LEVEL; i++ ) {
		delete hDummyHandle[i];
	}
	*/
	PROFILE_TICK_TEST_BLOCK_END();

	g_BsMemChecker.End();

	return true;
}

/*
bool CFcWorld::IsUnitIndex( int nType, int nSkinLevel, bool bEnemy, std::vector<UNITLOADINFO>& vecUnitIndex )
{
	int nCnt = vecUnitIndex.size();
	for( int j=0; j<nCnt; j++ )
	{
		if( vecUnitIndex[j].nUnitIndex == nType && vecUnitIndex[j].nSkinLevel == nSkinLevel && vecUnitIndex[j].bEnemy == bEnemy )
		{
			return true;
		}
	}
	return false;
}
*/
void CFcWorld::CheckAndAddUnitPreLoad( int nType, int nSkinLevel, int nLevel, bool bEnemy, std::vector<UNITLOADINFO>& vecUnitIndex )
{
	int nCnt = vecUnitIndex.size();
	for( int j=0; j<nCnt; j++ ) {
		if( vecUnitIndex[j].nUnitIndex == nType && 
			vecUnitIndex[j].nSkinLevel == nSkinLevel && 
			vecUnitIndex[j].bEnemy == bEnemy &&
			vecUnitIndex[j].nLevel == CLevelTableLoader::GetInstance().GetMatchingLevel( nType, nLevel ) ) return;
	}

	UNITLOADINFO LoadInfo;
	LoadInfo.nUnitIndex = nType;
	LoadInfo.nSkinLevel = nSkinLevel;
	LoadInfo.bEnemy = bEnemy;
	LoadInfo.nLevel = CLevelTableLoader::GetInstance().GetMatchingLevel( nType, nLevel );

	vecUnitIndex.push_back( LoadInfo );

	int nSubType = CUnitSOXLoader::GetInstance().GetUnitData( nType )->nSubJobSOXIndex;
	if( nSubType != -1 )
	{
		LoadInfo.nUnitIndex = nSubType;
		LoadInfo.nSkinLevel = nSkinLevel;
		LoadInfo.bEnemy = bEnemy;
		LoadInfo.nLevel = CLevelTableLoader::GetInstance().GetMatchingLevel( nSubType, nLevel );

		vecUnitIndex.push_back( LoadInfo );
	}
}

void CFcWorld::CheckAndAddAIPreLoad( int nType, char *szAIName, int nLevel, bool bEnemy, std::vector<AILOADINFO>& vecAIList )
{
	char szStr[256];
	if( strlen( szAIName ) == 0 || atoi( szAIName ) == -1 || szAIName[0] == -1 )
		sprintf( szStr, "%s_Default.ai", CUnitSOXLoader::GetInstance().GetUnitData( nType )->cUnitName );
	else sprintf( szStr, szAIName );

	DWORD dwSize = vecAIList.size();
	for( DWORD i=0; i<dwSize; i++ ) {
		if( vecAIList[i].nUnitIndex == nType && 
			vecAIList[i].bEnemy == bEnemy && 
			vecAIList[i].nLevel == CLevelTableLoader::GetInstance().GetMatchingLevel( nType, nLevel ) ) {
			if( strcmp( vecAIList[i].szAIName.c_str(), szStr ) == NULL ) return;
			break;
		}

	}

	AILOADINFO Info;
	Info.nUnitIndex = nType;
	Info.szAIName = szStr;
	Info.bEnemy = bEnemy;
	Info.nLevel = CLevelTableLoader::GetInstance().GetMatchingLevel( nType, nLevel );
	vecAIList.push_back( Info );

	int nSubType = CUnitSOXLoader::GetInstance().GetUnitData( nType )->nSubJobSOXIndex;
	if( nSubType != -1 )
	{
		char szStr[256];
		if( strlen( szAIName ) == 0 || atoi( szAIName ) == -1 || szAIName[0] == -1 )
			sprintf( szStr, "%s_Default.ai", CUnitSOXLoader::GetInstance().GetUnitData( nSubType )->cUnitName );
		else sprintf( szStr, szAIName );

		DWORD dwSize = vecAIList.size();
		for( DWORD i=0; i<dwSize; i++ ) {
			if( vecAIList[i].nUnitIndex == nSubType && 
				vecAIList[i].bEnemy == bEnemy && 
				vecAIList[i].nLevel == CLevelTableLoader::GetInstance().GetMatchingLevel( nSubType, nLevel ) ) {
					if( strcmp( vecAIList[i].szAIName.c_str(), szStr ) == NULL ) return;
					break;
				}
		}

		AILOADINFO Info;
		Info.nUnitIndex = nSubType;
		Info.szAIName = szStr;
		Info.bEnemy = bEnemy;
		Info.nLevel = CLevelTableLoader::GetInstance().GetMatchingLevel( nSubType, nLevel );
		vecAIList.push_back( Info );
	}
}

int CFcWorld::GetSkinLevel( int nType, int nLevel )
{
	int nCurLevel = CLevelTableLoader::GetInstance().GetMatchingLevel( nType, nLevel );
	UnitDataInfo *pUnitInfo=CUnitSOXLoader::GetInstance().GetUnitData(nType);
	int nSkinLevel = -1;
	if( pUnitInfo->bUseLevel )
	{
		for( int i=0; i<4; i++ )		// Level skin은 4개
		{
			if( pUnitInfo->nLevelStartOffset[i] > nCurLevel )
			{
				nSkinLevel = i;
				break;
			}
		}
		if( nSkinLevel == -1 )
			nSkinLevel = 0;
	}

	// nSkinLevel (0, 3)2개만 사용!
	if( nSkinLevel >= 0 )
	{
		if( nSkinLevel == 0 || nSkinLevel == 1 )
			return 0;
		else
			return 3;
	}

	return -1;
}

/*
int CFcWorld::GetSkinLevel( int nType, int nLevel )
{
	int nCurLevel = CLevelTableLoader::GetInstance().GetMatchingLevel( nType, nLevel );
	UnitDataInfo *pUnitInfo=CUnitSOXLoader::GetInstance().GetUnitData(nType);
	int nSkinLevel = -1;
	if( pUnitInfo->bUseLevel )
	{
		for( int i=0; i<3; i++ )		// Level skin은 4개
		{
			if( pUnitInfo->nLevelStartOffset[i] >= nCurLevel && 
				pUnitInfo->nLevelStartOffset[i+1] < nCurLevel )
			{
				nSkinLevel = i;
			}
		}
		if( nSkinLevel == -1 )
			nSkinLevel = 3;
	}
	return nSkinLevel;
}
*/

void CFcWorld::ProcessExtraInterfaceParam()
{
	if( m_ShowExtraParam.m_nCurShowVarID != -1 )
	{
		SHOW_VAR_INFO* pInfo = NULL;
		int nCnt = m_ShowExtraParam.m_vecShowVar.size();
		for( int i=0; i<nCnt; i++ )
		{
			pInfo = &(m_ShowExtraParam.m_vecShowVar[i]);
			if( m_ShowExtraParam.m_nCurShowVarID == pInfo->m_nVarTextID )
				break;
		}
		if( pInfo )
		{
			char szTempStr[STR_SPEECH_MAX];
			int nVariableValue = g_FcWorld.GetTriggerVarInt( pInfo->m_nVarTextID );
			sprintf( szTempStr, "%s\n%d", pInfo->m_cVarTextStr, nVariableValue );
			g_InterfaceManager.GetInstance().SetSpecialMissionText( szTempStr ); 
		}
	}
	if( m_ShowExtraParam.m_nCurShowGaugeID != -1 )
	{
		bool bExist = false;
		int nCnt = m_ShowExtraParam.m_vecShowGaugeVarID.size();
		for( int i=0; i<nCnt; i++ )
		{
			if( m_ShowExtraParam.m_nCurShowGaugeID == m_ShowExtraParam.m_vecShowGaugeVarID[i] )
			{
				bExist = true;
				break;
			}
		}
		if( bExist )
		{
			int nVariableValue = g_FcWorld.GetTriggerVarInt( m_ShowExtraParam.m_nCurShowGaugeID );
			g_InterfaceManager.SetSpecialMissionGauge( nVariableValue, 100 );
		}
	}
}


INITGAMEOBJECTDATA* CFcWorld::GetInitGameObjectData( int nUnitIndex, int nLevel )
{
	int nSkinLevel = GetSkinLevel( nUnitIndex, nLevel );
	int nCnt = m_vecInitGameObj.size();
	for( int i=0; i<nCnt; i++ )
	{
		INITGAMEOBJECTDATA* pData = &(m_vecInitGameObj[i]);
		if( pData->nUnitIndex == nUnitIndex )
		{
			if( pData->nSkinLevel == nSkinLevel )
			return pData;
		}
	}

	DebugString( "스킨 레벨 안 맞는다! UnitIndex:%d, Level:%d\n", nUnitIndex, nLevel );
	// Debug메뉴로 들어왔을 때 처리
//	if( g_bIsStartTitleMenu == FALSE )
//	{
		nCnt = m_vecInitGameObj.size();
		for( int i=0; i<nCnt; i++ )
		{
			INITGAMEOBJECTDATA* pData = &(m_vecInitGameObj[i]);
			if( pData->nUnitIndex == nUnitIndex )
			{
				return pData;
			}
		}
//	}
	BsAssert(0);
	return NULL;
}


void CFcWorld::AddObjective( int nID, int nTextID, bool bClear)
{
	int nCnt = m_vecMissionObjective.size();
	for( int i=0; i<nCnt; i++ )
	{
		MISSION_OBJECTIVE* pData = &(m_vecMissionObjective[i]);
		if( pData->nID == nID )
		{
			DebugString( "Err. Exist same objective ID!\n" );
			BsAssert( 0 && "Err. Exist same objective ID!\n" );
			return;
		}
	}
	MISSION_OBJECTIVE Data;
	Data.nID = nID;
	Data.nTextID = nTextID;
	Data.nClear = (int)bClear;
	m_vecMissionObjective.push_back( Data );
	g_InterfaceManager.SetMissionGoal( nTextID );
}

void CFcWorld::ClearObjective( int nID )
{
	int nCnt = m_vecMissionObjective.size();
	for( int i=0; i<nCnt; i++ )
	{
		MISSION_OBJECTIVE* pData = &(m_vecMissionObjective[i]);
		if( pData->nID != nID ){
			continue;
		}

		MISSION_OBJECTIVE Data;
		Data.nID = pData->nID;
		Data.nTextID = pData->nTextID;
		Data.nClear = 1;

		m_vecMissionObjective.erase( m_vecMissionObjective.begin() + i );
		m_vecMissionObjective.push_back( Data );

		return;
	}

	DebugString( "Err. Cannot found objective ID! ClearID %d\n", nID );
	BsAssert( 0 && "Err. Cannot found objective ID!\n" );
}

void CFcWorld::RemoveObjective( int nID )
{
	int nCnt = m_vecMissionObjective.size();
	for( int i=0; i<nCnt; i++ )
	{
		MISSION_OBJECTIVE* pData = &(m_vecMissionObjective[i]);
		if( pData->nID == nID )
		{
			m_vecMissionObjective.erase( m_vecMissionObjective.begin() + i );
			return;
		}
	}

	DebugString( "Err. Cannot found objective ID RemoveID %d!\n", nID );
	BsAssert( 0 && "Err. Cannot found objective ID 2!\n" );
}


void CFcWorld::ShowVarText( char* pStr, int nVarID )
{
	BsAssert( strlen(pStr) < 64 );

	SHOW_VAR_INFO info;
	strcpy( info.m_cVarTextStr, pStr );
	info.m_nVarTextID = nVarID;

	int nCnt = m_ShowExtraParam.m_vecShowVar.size();
	for( int i=0; i<nCnt; i++ )
	{
		SHOW_VAR_INFO* pInfo = &(m_ShowExtraParam.m_vecShowVar[i]);
		if( pInfo->m_nVarTextID == nVarID )
		{
			m_ShowExtraParam.m_nCurShowVarID = nVarID;
			return;
		}
	}
	m_ShowExtraParam.m_vecShowVar.push_back( info );
	m_ShowExtraParam.m_nCurShowVarID = nVarID;
}

void CFcWorld::HideVarText( int nVarID )
{
	if( m_ShowExtraParam.m_nCurShowVarID == nVarID )
		m_ShowExtraParam.m_nCurShowVarID = -1;

	int nCnt = m_ShowExtraParam.m_vecShowVar.size();
	for( int i=0; i<nCnt; i++ )
	{
		SHOW_VAR_INFO* pInfo = &(m_ShowExtraParam.m_vecShowVar[i]);
		if( pInfo->m_nVarTextID == nVarID )
		{
			m_ShowExtraParam.m_vecShowVar.erase( m_ShowExtraParam.m_vecShowVar.begin() + i );
			return;
		}
	}
}

void CFcWorld::ShowVarGauge( int nVarID )
{
	int nCnt = m_ShowExtraParam.m_vecShowGaugeVarID.size();
	for( int i=0; i<nCnt; i++ )
	{
		if( m_ShowExtraParam.m_vecShowGaugeVarID[i] == nVarID )
		{
			m_ShowExtraParam.m_nCurShowVarID = nVarID;
			return;
		}
	}
	m_ShowExtraParam.m_nCurShowVarID = nVarID;
	m_ShowExtraParam.m_vecShowGaugeVarID.push_back( nVarID );
}

void CFcWorld::HideVarGauge( int nVarID )
{
	if( m_ShowExtraParam.m_nCurShowVarID == nVarID )
		m_ShowExtraParam.m_nCurShowVarID = -1;

	int nCnt = m_ShowExtraParam.m_vecShowGaugeVarID.size();
	for( int i=0; i<nCnt; i++ )
	{
		if( m_ShowExtraParam.m_vecShowGaugeVarID[i] == nVarID )
		{
			m_ShowExtraParam.m_vecShowGaugeVarID.erase( m_ShowExtraParam.m_vecShowGaugeVarID.begin() + i );
			return;
		}
	}
}

void CFcWorld::AddResultCalcTroop( TroopObjHandle hTroop )
{
	int nCnt = m_vecResultCalcTroops.size();
	for( int i=0; i<nCnt; i++ )
	{
		if( hTroop == m_vecResultCalcTroops[i] )
		{
			return;
		}
	}
	m_vecResultCalcTroops.push_back( hTroop );
}

float CFcWorld::GetGuardianLiveRatePercent()
{
	return m_pTroopManager->GetGuardianLiveRatePercent();
}

float CFcWorld::GetToopsLiveRatePercent()
{
	float fMaxHP = 0.f;
	float fHP = 0.f;
	int nCnt = m_vecResultCalcTroops.size();
	for( int i=0; i<nCnt; i++ )
	{
		TroopObjHandle hTroop = m_vecResultCalcTroops[i];
		fMaxHP += hTroop->GetMaxHP();
		fHP += hTroop->GetHP();
	}

	if( fMaxHP == 0.f )
	{
		return 0.f;
	}
	return ( fHP / fMaxHP * 100.f );
}
float CFcWorld::GetFriendlyTroopsLiveRatePercent()
{
	return m_pTroopManager->GetFriendlyTroopsLiveRatePercent();
}

void CFcWorld::GetItemList( std::vector<CFcProp*>& vecItems )
{
	vecItems.clear();
	m_pPropManager->GetItemList( vecItems );
}

// RealMovie
void	CFcWorld::SaveLight(void)
{
	m_crossOldLight = m_pMapInfo->GetSetInfo()->crossLight;
}

void	CFcWorld::RestoreLight(void)
{
	m_pMapInfo->GetSetInfo()->crossLight = m_crossOldLight;
    UpdateLightForRealMovie( &m_crossOldLight );
}

void	CFcWorld::UpdateLightForRealMovie(CCrossVector* pCross)
{
	if( m_nLightIndex != -1 )
	{
        CBsKernel::GetInstance().UpdateObject( m_nLightIndex, *pCross );
        g_pFcFXManager->SendMessage(m_nFXIndex, FX_SET_LIGHTDIRECTION, (DWORD)&(pCross->m_ZVector));
	}
}

void CFcWorld::SetMarkPoint( D3DXVECTOR2* pPos )
{
	CameraObjHandle CamHandle = CFcBaseObject::GetCameraObjectHandle( 0 );
	CamHandle->SetMarkPoint( pPos );
}

void CFcWorld::AddDarkDynamicFog(int index, int iDynamicFogID) 
{	 
	 m_vecDarkDynFog.push_back(std::make_pair(index, iDynamicFogID));
}
int CFcWorld::FindDarkDynamicFog(int index) 
{
	std::vector<std::pair<int,int> >::iterator iter = m_vecDarkDynFog.begin();
	for(; iter!=m_vecDarkDynFog.end(); ++iter)
		if( (*iter).first == index)
			return (*iter).second;
	return -1;
}
void CFcWorld::EraseDarkDyanmicFog(int index) 
{
	std::vector<std::pair<int,int> >::iterator iter = m_vecDarkDynFog.begin();
	for(; iter!=m_vecDarkDynFog.end(); ++iter)
		if( (*iter).first == index)
		{
			m_vecDarkDynFog.erase(iter);
			break;
		}
			
}
void CFcWorld::StartRealmovie( float fSX, float fEX, float fSY, float fEY )
{
	BsAssert( m_bBackupForRM == false );
	m_bBackupForRM = true;
	m_nRMStartTick = ::GetProcessTick();
	m_vecBackupTroopsForRM.clear();

	int nCnt = m_pTroopManager->GetTroopCount();
	for( int i=0; i<nCnt; i++ )
	{
		TroopObjHandle hTroop = m_pTroopManager->GetTroop( i );
		if( hTroop->IsEnable() == false )
			continue;

		if( hTroop->IsEliminated() )
			continue;

		D3DXVECTOR3 Pos = hTroop->GetPos();
		if( fSX < Pos.x && fEX > Pos.x && fSY < Pos.z && fEY > Pos.z )
		{
			hTroop->CmdEnable( false, false, false );
			m_vecBackupTroopsForRM.push_back( hTroop );
		}
	}
}

void CFcWorld::StartTroopAIDisableForRealmovie( float fSX, float fEX, float fSY, float fEY )
{
	BsAssert( m_bBackupAIForRM == false );
	m_bBackupAIForRM = true;
	m_nRMStartTick = ::GetProcessTick();
	m_vecBackupTroopsDisableAIForRM.clear();

	int nCnt = m_pTroopManager->GetTroopCount();
	for( int i=0; i<nCnt; i++ )
	{
		TroopObjHandle hTroop = m_pTroopManager->GetTroop( i );
		if( hTroop->IsEnable() == false )
			continue;

		if( hTroop->IsEliminated() )
			continue;

		D3DXVECTOR3 Pos = hTroop->GetPos();
		if( fSX < Pos.x && fEX > Pos.x && fSY < Pos.z && fEY > Pos.z )
		{
			if( m_pTroopManager->GetGuardianTroop( 0 ) == hTroop )
			{
				m_pTroopManager->SetGuardianEnable( 0, false );
				m_vecBackupTroopsDisableAIForRM.push_back( hTroop );
			}
			else if( m_pTroopManager->GetGuardianTroop( 1 ) == hTroop )
			{
				m_pTroopManager->SetGuardianEnable( 1, false );
				m_vecBackupTroopsDisableAIForRM.push_back( hTroop );
			}
			else
			{
				if( CFcTroopAIObject::IsEnableAI( hTroop ) == true )
				{
					CFcTroopAIObject::SetEnableAI( hTroop, false );
					hTroop->CmdStop();
					m_vecBackupTroopsDisableAIForRM.push_back( hTroop );
				}
			}
		}
	}
}

void CFcWorld::EnableBlizzardEffect(int nAlpha, int nIntervalTick) 
{
	m_nBlizzardEffectIdx = g_pFcFXManager->Create(FX_TYPE_2DFSEFFECT);
	g_pFcFXManager->SendMessage(m_nBlizzardEffectIdx, FX_INIT_OBJECT);
	g_pFcFXManager->SendMessage(m_nBlizzardEffectIdx, FX_PLAY_OBJECT, 1);

	// 알파값은 float인데 float입력이 안되니까 100을 1.f로 생각해서 한다고 생각
	float fAlpha = nAlpha/100.f;
	

	g_BsKernel.chdir("Fx");
	m_nBlizzardEffectTexIdx = g_BsKernel.LoadTexture("SnowStorm.dds");
	g_BsKernel.chdir("..");


	g_pFcFXManager->SendMessage(m_nBlizzardEffectIdx, FX_SET_FSALPHA, DWORD(&fAlpha)); 
//	g_pFcFXManager->SendMessage(m_nBlizzardEffectIdx, FX_SET_FSINTERVAL, nIntervalTick); 
	g_pFcFXManager->SendMessage(m_nBlizzardEffectIdx, FX_SET_TEXTURE, m_nBlizzardEffectTexIdx); 
}

void CFcWorld::DisableBlizzardEffect() 
{	
	if(m_nBlizzardEffectIdx != -1)
	{
		g_pFcFXManager->SendMessage(m_nBlizzardEffectIdx, FX_STOP_OBJECT);
		SAFE_DELETE_FX(m_nBlizzardEffectIdx,FX_TYPE_2DFSEFFECT);
		/*g_pFcFXManager->SendMessage(m_nBlizzardEffectIdx, FX_DELETE_OBJECT);
		m_nBlizzardEffectIdx = -1;*/
	}

	SAFE_RELEASE_TEXTURE(m_nBlizzardEffectTexIdx);
}

void CFcWorld::EnableVigBlurEffect()
{
	m_nVigBlurEffectIdx = g_pFcFXManager->Create(FX_TYPE_MOTIONBLUR );
	g_pFcFXManager->SendMessage(m_nVigBlurEffectIdx, FX_INIT_OBJECT);
	g_pFcFXManager->SendMessage(m_nVigBlurEffectIdx, FX_PLAY_OBJECT, 1);	
}
void CFcWorld::DisableVigBlurEffect()
{
	if(m_nVigBlurEffectIdx != -1)
	{
		g_pFcFXManager->SendMessage(m_nVigBlurEffectIdx, FX_STOP_OBJECT);
		SAFE_DELETE_FX(m_nVigBlurEffectIdx, FX_TYPE_MOTIONBLUR );	
	}
}

bool CFcWorld::IsProcessTrueOrb( int nTeam )
{
	if( m_nTrueOrbTimer > 0 ) {
		if( CFcBaseObject::IsValid( m_UseTrueOrbHandle ) && m_UseTrueOrbHandle->GetTeam() == nTeam ) return false;
		return true;
	}
	return false;
//	return ( m_nTrueOrbTimer > 0 );
}

void CFcWorld::AddBigSizeScanner( GameObjHandle &Handle )
{
	m_vecBigSizeScanner.push_back( Handle );
}

void CFcWorld::RemoveBigSizeScanner( GameObjHandle &Handle )
{
	std::vector<GameObjHandle>::iterator it = m_vecBigSizeScanner.begin();
	while( it != m_vecBigSizeScanner.end() ) {
		if( *it == Handle ) {
			m_vecBigSizeScanner.erase( it );
			break;
		}
		it++;
	}
}

void CFcWorld::AddTroopEventArea( AREA_INFO* pArea )
{
	m_vecTroopEvnetArea.push_back( *pArea );
}

void CFcWorld::SetTroopEventArea( TroopObjHandle hTroop, TroopObjHandle hTargetTroop, float fDist )
{
	D3DXVECTOR2 Center = hTroop->GetPosV2();
	
	int nIndex = -1;
	float fBestDist = 0.f;
	int nCnt = m_vecTroopEvnetArea.size();
	for( int i=0; i<nCnt; i++ )
	{
		AREA_INFO* pArea = &(m_vecTroopEvnetArea[i]);

		D3DXVECTOR2 Pos;
		Pos.x = pArea->fSX + (pArea->fEX - pArea->fSX)/2.f;  
		Pos.y = pArea->fSZ + (pArea->fEZ - pArea->fSZ)/2.f;  

		D3DXVECTOR2 Dir = Center - Pos;
		float fCurDist = D3DXVec2Length( &Dir );

		if( nIndex == -1 )
		{
			if( fCurDist > fDist )
			{
				fBestDist = fCurDist;
				nIndex = i;
			}
		}
		else
		{
			if( fCurDist > fDist && fBestDist > fCurDist )
			{
				fBestDist = fCurDist;
				nIndex = i;
			}
		}
	}

	if( nIndex >= 0 )
	{
		AREA_INFO* pArea = &(m_vecTroopEvnetArea[nIndex]);

		D3DXVECTOR2 Pos;
		Pos.x = pArea->fSX + (pArea->fEX - pArea->fSX)/2.f;  
		Pos.y = pArea->fSZ + (pArea->fEZ - pArea->fSZ)/2.f; 

		hTargetTroop->CmdPlace( Pos.x, Pos.y, 0 );
		hTargetTroop->CmdEnable( true );
	}
	else
	{
		DebugString( "Cannot found area in SetTroopEventArea\n" );
		BsAssert( 0 && "Cannot found area in SetTroopEventArea\n" );
	}
}


bool CFcWorld::CanIPlayBGM()
{
	bool bPlay = g_pSoundManager->IsBGMPlay();

	// 이벤트 신일 때 처리해야 한다.
	if( bPlay == false && g_pFcRealMovie->IsPlay() == false )
		return true;

	return false;
}

void CFcWorld::PlayLastBGM()
{
	if( m_nBGMID < 0 )
		return;

	g_pSoundManager->PlayBGM( m_nBGMID, 0 );
}


void CFcWorld::SetGuardianTroopDefenseAdd( int nRate )
{
	m_pTroopManager->SetGuardianTroopDefenseAdd( nRate );
}


void CFcWorld::SetGuardianTroopAttackAdd( int nRate )
{
	m_pTroopManager->SetGuardianTroopAttackAdd( nRate );
}

void CFcWorld::SetGuardianTroopMoveSpeedAdd( int nRate )
{
	m_pTroopManager->SetGuardianTroopMoveSpeedAdd( nRate );
}

void CFcWorld::SetGuardianTroopMaxHPAdd( int nRate )
{
	m_pTroopManager->SetGuardianTroopMaxHPAdd( nRate );
}

void CFcWorld::SetGuardianTrapDamageAdd( int nRate )
{
	m_pTroopManager->SetGuardianTroopTrapDamageAdd( nRate );
}

void CFcWorld::AddGuardianHP( int nRate ,bool bPercent )
{
	m_pTroopManager->AddGuardianTroopHP( nRate ,bPercent );
}

void CFcWorld::SetEnemyRangeProbbtAdd(int nPer)
{
	int nTroopCount = m_pTroopManager->GetTroopCount();
	for(int i = 0;i < nTroopCount;i++ )
	{
		int nUnitCount = m_pTroopManager->GetTroop(i)->GetUnitCount();
		for( int s = 0;s < nUnitCount;s++ )
		{
			GameObjHandle Handle = m_pTroopManager->GetTroop(i)->GetUnit(s);
			if( CFcBaseObject::IsValid( Handle ) == false)
				continue;

			if( Handle->GetTeam() == GetHeroHandle()->GetTeam() )
				continue;


			if( Handle->GetClassID() == Class_ID_Archer ){			
				Handle->SetHitProbAdd( nPer );
			}
		}
	}
	
}


int CFcWorld::GetCurMapLevel()
{
	BsAssert( m_pCurStageResultInfo );
	return m_pCurStageResultInfo->nEnemyLevelUp;
}

void CFcWorld::InitTrueOrbDamagedTroops()
{
	m_bUseTrueOrb = true;
	m_vecDamagedTrueOrbTroops.clear();
}

void CFcWorld::FinishTrueOrbDamagedTroops()
{
	int nCnt = m_vecDamagedTrueOrbTroops.size();
	for( int i=0; i<nCnt; i++ )
	{
		m_vecDamagedTrueOrbTroops[i]->SetFakeDisable( false );
	}

	m_bUseTrueOrb = false;
	m_vecDamagedTrueOrbTroops.clear();
}

void CFcWorld::AddTroopDamagedTrueOrb( TroopObjHandle hTroop )
{
	if( hTroop == NULL )
		return;

	int nCnt = m_vecDamagedTrueOrbTroops.size();
	for( int i=0; i<nCnt; i++ )
	{
		if( m_vecDamagedTrueOrbTroops[i] == hTroop )
			return;
	}

	m_vecDamagedTrueOrbTroops.push_back( hTroop );
	hTroop->SetFakeDisable( true );
}

void CFcWorld::ResetUnitAILOD()
{
	int nCnt = m_pTroopManager->GetTroopCount();
	for( int i=0; i<nCnt; i++ )
	{
		TroopObjHandle hTroop = m_pTroopManager->GetTroop( i );
		if( hTroop->IsEnable() == false )
			continue;

		if( hTroop->IsEliminated() )
			continue;

		int nUnitCnt = hTroop->GetUnitCount();
		for( int j=0; j<nUnitCnt; j++ )
		{
			GameObjHandle hUnit = hTroop->GetUnit( j );
			if( hUnit == NULL )
				continue;

			if( hUnit->IsEnable() == false )
				continue;

			if( hUnit->IsDie() )
				continue;

			if( hUnit->GetAIHandle() ) {
				hUnit->GetAIHandle()->SetForceProcessTick(1);
			}
		}
	}
}

//_TNT_
#ifdef _TNT_
//JKB: hook added for automation: warps during random stress to nearest enemy
bool CFcWorld::WarpHeroToEnemy(int heroIndex = 0, float range = 100000.0f)
{
	try
	{
		HeroObjHandle pHero = m_HeroHandle;
		CFcTroopManager *pTroopMgr = m_pTroopManager;
		D3DXVECTOR2 ePos;

		// Get hero position and find neareset enemy:
		if (pHero && pTroopMgr)
		{
			D3DXVECTOR2 heroPos = pHero->GetPosV2();
			TroopObjHandle closestEnemy = pTroopMgr->GetNearMeleeEnemyInRange(&heroPos,pHero->GetTeam(),range);
			if (closestEnemy)
				ePos = closestEnemy->GetPosV2();
			else return false;

			// Now warp hero to that enemy location:
			pHero->HeroWarp(ePos.x,ePos.y);
		}
		else return false;
	}
	catch (char* str)
	{
		OutputDebugString(str);
		return false;
	}

	return true;
}
#endif //_TNT_
