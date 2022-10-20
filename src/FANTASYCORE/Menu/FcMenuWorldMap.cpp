#include "stdafx.h"
#include "FcMenuWorldMap.h"
#include "BsKernel.h"
#include "InputPad.h"

#include "FcInterfaceManager.h"

#include "FcGlobal.h"
#include "LocalLanguage.h"
#include "bstreamExt.h"
#include "FcSoxLoader.h"

#include "BsUiFont.h"
#include "BsUiUVManager.h"
#include ".\\data\\Menu\\BsUiUVID.h"
#include "FantasyCore.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG


CLocalTextTableLoader g_FCBriefingSOX;

#define _MAP_SIDE_SPACE				30.f
#define _CHANGE_TICK				40
#define _STAGE_BRIEFINE_MAX_COUNT	10
#define _WORLD_MAP_NAME				"mn_worldmap_whole.dds"
#define _WORLD_MAP_POS_ICON_CLEAR	"mn_worldmap_areapoint_clear.dds"
#define _WORLD_MAP_POS_ICON_GO		"mn_worldmap_areapoint_go.dds"
#define _WORLD_MAP_POS_ICON_DIS		"mn_worldmap_areapoint_dis.dds"
#define _BRIEFING_TEXT_TICK			40 * BRIEFING_VIEW_TEXT_COUNT

//grammer code는 1부터!!!!
enum FcWorldMapCMD
{
	FcWorld_POINT				= 1,
	FcWorld_POINT_POS,	
	FcWorld_NAME_POS,
	FcWorld_NAME_IMG,
	FcWorld_AREA_IMG,
	FcWorld_MAP_NAME,
	FcWorld_GUARDIAN_TYPE,
	FcWorld_GUARDIAN_INFO,
	
	FcWorld_STAGE,		
	FcWorld_STAGE_MOVE,	


	FcWorld_END_BRACE			= 999,
	FcWorld_VERSION				= 1000,
};

/*
FcWorld_POINT_INFO			= 1,
FcWorld_STAGE_INFO,
FcWorld_NEW_POINT_INFO,
 */
FcMenuWorldMapInfo::FcMenuWorldMapInfo()
{
	m_nStartPointId = -1;
	m_nStartPathId = -1;
}

void FcMenuWorldMapInfo::Release()
{
	m_nStartPointId = -1;
	m_nStartPathId = -1;

	DWORD dwCount = m_PointList.size();
	for(DWORD i=0; i<dwCount; i++)
	{	
		if(m_PointList[i].szImage != NULL){
// [beginmodify] 2006/2/3 junyash PS#4609 detect new[] and delete mis-match
			//delete m_PointList[i].szImage;
			delete[] m_PointList[i].szImage; // delate char[] is stable but delete[] to make sure.
// [endmodify] junyash
		}

		if(m_PointList[i].szImageNM != NULL){
// [beginmodify] 2006/2/3 junyash PS#4609 detect new[] and delete mis-match
			//delete m_PointList[i].szImageNM;
			delete[] m_PointList[i].szImageNM; // delate char[] is stable but delete[] to make sure.
// [endmodify] junyash
		}

		if(m_PointList[i].szStage != NULL){
// [beginmodify] 2006/2/3 junyash PS#4609 detect new[] and delete mis-match
			//delete m_PointList[i].szStage;
			delete[] m_PointList[i].szStage; // delate char[] is stable but delete[] to make sure.
// [endmodify] junyash
		}
	}
	m_PointList.clear();

	m_PathList.clear();
}

void FcMenuWorldMapInfo::Load()
{
	Release();

	BFileStream *pStream;
	g_BsKernel.chdir( "Briefing" );
	g_BsKernel.chdir( g_LocalLanguage.GetLanguageDir() );
	char szBriefingFile[MAX_PATH];
	sprintf(szBriefingFile, "Briefing_%d_%s.sox", g_FCGameData.nPlayerType, g_LocalLanguage.GetLanguageDir());
	pStream = new BFileStream( g_BsKernel.GetFullName( szBriefingFile ) );
	g_FCBriefingSOX.Load( pStream );
	delete pStream;
	g_BsKernel.chdir( ".." );
	g_BsKernel.chdir( ".." );

	char szMissionName[MAX_PATH];
	sprintf(szMissionName, "Briefing_%d.txt", g_FCGameData.nPlayerType);
	
	if(g_FCGameData.nPlayerType == MISSION_TYPE_INPHYY)
	{
		if(g_FCGameData.SelStageId == STAGE_ID_ANOTHER_WORLD ||
			g_FCGameData.tempFixedSave.bOpenBonusStage == true)
		{
			sprintf(szMissionName, "Briefing_%d_Bonus.txt", g_FCGameData.nPlayerType);
		}
	}

	char szFullName[MAX_PATH];
	g_BsKernel.chdir("Briefing");
	sprintf(szFullName, "%s%s", g_BsKernel.GetCurrentDirectory(), szMissionName);
	g_BsKernel.chdir("..");

	Load(szFullName);

	//현재 
	DWORD dwCount = m_PointList.size();
	for(DWORD i=0; i<dwCount; i++)
	{
		if(g_FCGameData.SelStageId != -1 &&
			m_PointList[i].nId != g_FCGameData.SelStageId){
			continue;
		}

		m_nStartPointId = i;
		break;
	}

	dwCount = m_PathList.size();
	for(DWORD i=0; i<dwCount; i++)
	{
		if(g_FCGameData.SelStageId != -1 &&
			m_PathList[i].nStartId != g_FCGameData.SelStageId){
			continue;
		}

		m_nStartPathId = i;
		break;
	}
}


bool FcMenuWorldMapInfo::Load(char* szFileName)
{
	tpGrammer grammer;

	grammer.Add(FcWorld_VERSION,			"MENU_WORLD_INFO_VERSION(%s)");
	grammer.Add(FcWorld_END_BRACE,			"}");

	grammer.Add(FcWorld_POINT,				"POINT(%d) {");
	grammer.Add(FcWorld_POINT_POS,			"POS(%d,%d)");
	grammer.Add(FcWorld_NAME_POS,			"NAME_POS(%d,%d)");
	grammer.Add(FcWorld_NAME_IMG,			"NAME_IMAGE(%s)");
	grammer.Add(FcWorld_AREA_IMG,			"AREA_IMAGE(%s)");
	grammer.Add(FcWorld_MAP_NAME,			"STAGE_NAME(%s)");

	//guardian info
	grammer.Add(FcWorld_GUARDIAN_TYPE,		"GUARDIAN_TYPE(%d,%d)");
	grammer.Add(FcWorld_GUARDIAN_INFO,		"GUARDIAN_INFO(%d,%d,%d,%d)");		//모델 종류, 가로, 세로, 간격
	


	grammer.Add(FcWorld_STAGE,				"STAGE(%d) {");
	grammer.Add(FcWorld_STAGE_MOVE,			"MOVE_POS(%d,%d,%d)");
	

	char fullname[MAX_PATH];
	char dir[MAX_PATH];
	char fn[MAX_PATH];

	strcpy(fullname, szFileName);
	SplitDirAndFileName( fullname, dir, _countof(dir), fn, _countof(fn)); //aleksger - safe string
	grammer.Create(szFileName, dir);

	int ret = 0;
	do {
		ret = grammer.Get();

		switch(ret)
		{
		case FcWorld_POINT:
			{
				WorldMapPointInfo info;

				info.nId = atoi(grammer.GetParam(0));
				LoadPoint(&grammer, &info);
				
				m_PointList.push_back(info);

				break;
			}
		case FcWorld_STAGE:
			{	
				WorldMapPassInfo info;

				info.nStartId = atoi(grammer.GetParam(0));
				LoadStage(&grammer, &info);

				m_PathList.push_back(info);

				break;
			}
		case -1:
			{
				BsAssert(0);
				CBsConsole::GetInstance().AddFormatString("%s File Not Found", szFileName);
				return false;
			}
			break;
		}
	} while (ret != 0);

	return true;
}


bool FcMenuWorldMapInfo::LoadPoint(tpGrammer* pGrammer, WorldMapPointInfo* pInfo)
{
	int nGuardianInfo = 0;
	int ret = 0;
	do {
		ret = pGrammer->Get();

		switch(ret)
		{
		case FcWorld_POINT_POS:
			{
				//point 좌표
				pInfo->pos.x = (float)(atoi(pGrammer->GetParam(0)));
				pInfo->pos.y = (float)(atoi(pGrammer->GetParam(1)));
				break;
			}
		case FcWorld_NAME_POS:
			{
				//이름 좌표
				pInfo->posNM.x = (float)(atoi(pGrammer->GetParam(0)));
				pInfo->posNM.y = (float)(atoi(pGrammer->GetParam(1)));
				break;
			}
		case FcWorld_NAME_IMG:
			{
				// 이름 이미지 - 확장자 붙이지 말
				int len = (int)strlen((char*)pGrammer->GetParam(0));
				pInfo->szImageNM = new char[len+1];
				strcpy_s(pInfo->szImageNM, len+1, (char*)pGrammer->GetParam(0)); //aleksger - safe string
				break;
			}
		case FcWorld_AREA_IMG:
			{
				//확대된 지역 이미지
				int len = (int)strlen((char*)pGrammer->GetParam(0));
				pInfo->szImage = new char[len+1];
				strcpy_s(pInfo->szImage, len+1, (char*)pGrammer->GetParam(0));//aleksger - safe string
				break;
			}
		case FcWorld_MAP_NAME:
			{
				//stage map name
				int len = (int)strlen((char*)pGrammer->GetParam(0));
				pInfo->szStage = new char[len+1];
				strcpy_s(pInfo->szStage, len+1, (char*)pGrammer->GetParam(0));//aleksger - safe string
				break;
			}
		case FcWorld_GUARDIAN_TYPE:
			{
				pInfo->nGuadian_L = atoi(pGrammer->GetParam(0));
				pInfo->nGuadian_R = atoi(pGrammer->GetParam(1));
				break;
			}
		case FcWorld_GUARDIAN_INFO:
			{	
				pInfo->GuadianType[nGuardianInfo].nID = atoi(pGrammer->GetParam(0));
				pInfo->GuadianType[nGuardianInfo].nWidth = atoi(pGrammer->GetParam(1));
				pInfo->GuadianType[nGuardianInfo].nHeight = atoi(pGrammer->GetParam(2));
				pInfo->GuadianType[nGuardianInfo].nSpace = atoi(pGrammer->GetParam(3));
				nGuardianInfo++;
				break;
			}
		case BsUiCMD_END_BRACE:
			{
				return true;
			}
		default:
			{
				BsAssert(0);
				return false;
			}
			break;
		}
	} while (ret != 0);

	return false;
}

bool FcMenuWorldMapInfo::LoadStage(tpGrammer* pGrammer, WorldMapPassInfo* pInfo)
{
	int ret = 0;
	do {
		ret = pGrammer->Get();

		switch(ret)
		{
		case FcWorld_STAGE_MOVE:
			{
				pInfo->nEndId[0] = atoi(pGrammer->GetParam(0));
				pInfo->nEndId[1] = atoi(pGrammer->GetParam(1));
				pInfo->nEndId[2] = atoi(pGrammer->GetParam(2));
				break;
			}
		case BsUiCMD_END_BRACE:
			{
				return true;
			}
		default:
			{
				BsAssert(0);
				return false;
			}
			break;
		}
	} while (ret != 0);

	return false;
}



//-------------------------------------------------------------------
//-------------------------------------------------------------------

WorldMapIconInfo g_FcWorldPosIcon[MAX_STAGE] = 
{
	{ STAGE_ID_VARRGANDD,		170,   4, 308, 108, 239,  56,  391, 468 }, 	//바르간다
	{ STAGE_ID_OUT_VARRGANDD,	  0,   0,   0,   0,   0,   0,    0,   0 },	//바르간다 외곽
	{ STAGE_ID_VARRFARRINN,		  4,  94, 150, 208,  77, 151,  153, 426 },	//발파린
	{ STAGE_ID_OUT_VARRFARRINN,	352, 116, 440, 196, 396, 156,  722, 238 },  //발파린 외곽
	{ STAGE_ID_VARRVAZZAR,		274, 118, 348, 192, 311, 155,  698, 470 },	//발바잘 진입로
	{ STAGE_ID_OUT_VARRVAZZAR,	  4, 342, 112, 424,  58, 383,  835, 455 },	//발바잘 외곽
	{ STAGE_ID_WYANDEEK,		312,   4, 434, 112, 373,  58,  720, 148 },	//완디크
	{ STAGE_ID_WYANDEEK_VILLAGE,  0,   0,   0,   0,   0,   0,    0,   0 },	//완디크 근방의 고블린 마을	 
	{ STAGE_ID_YWA_UE_UAR,		256, 212, 412, 320, 334, 266, 1063, 273 },	//야페왈 숲
	{ STAGE_ID_EAURVARRIA,		154, 110, 270, 208, 212, 159,  532, 218 },	//아바리아
	{ STAGE_ID_FELPPE,			 76,   4, 166,  86, 121,  45,  339, 449 },	//헬페마을
	{ STAGE_ID_PHOLYA,			  4, 212, 252, 338, 128, 275,  785, 343 },	//포리아 평원
	{ STAGE_ID_BEFORE_PHOLYA_1,	352, 116, 440, 196, 722, 238,  722, 238 }, 	//포리아 평원 가기전1
	{ STAGE_ID_BEFORE_PHOLYA_2,	352, 116, 440, 196, 396, 156,  685, 334 }, 	//포리아 평원 가기전2
	{ STAGE_ID_ANOTHER_WORLD,	256, 326, 378, 416, 317, 371, 1129, 364 },	//마계
	{ STAGE_ID_VARRVAZZAR_2,	116, 342, 240, 424, 178, 383,  761, 522 },	//발바잘

	//world 맵상에 나타나지 안는다. dummy
	{ STAGE_ID_TUTORIAL,          0,   0,   0,   0,   0,   0,    0,   0 },	//tutorial

	{ STAGE_ID_ENDING_MYIFEE,     0,   0,   0,   0,   0,   0,    0,   0 },	//미피 엔딩맵
	{ STAGE_ID_ENDING_DWINGVATT,  0,   0,   0,   0,   0,   0,    0,   0 },	//딩그바트 엔딩맵
	{ STAGE_ID_ENDING_VIGKVAGK,   0,   0,   0,   0,   0,   0,    0,   0 },	//비그바그 엔딩맵
	{ STAGE_ID_ENDING_MA_INPHYY,  0,   0,   0,   0,   0,   0,    0,   0 },	//인피 마계 엔딩맵
};

FcMenuWorldMap::FcMenuWorldMap()
{	
	m_nWorldMapTexId = -1;
	m_MapPos = D3DXVECTOR2(0.f, 0.f);
	m_MapSize = D3DXVECTOR2((float)_SCREEN_WIDTH, (float)_SCREEN_HEIGHT);

	m_nStartPointId = g_MenuHandle->GetMenuWorldMapInfo()->GetStartPointId();
	m_nStartPathId = g_MenuHandle->GetMenuWorldMapInfo()->GetStartPathId();
	m_pPointList = g_MenuHandle->GetMenuWorldMapInfo()->GetPointList();
	m_pPathList = g_MenuHandle->GetMenuWorldMapInfo()->GetPathList();
	
	GetEnableStageList();

	//cursor
	m_TargetPointId = -1;
	m_nAreaNameTexID = -1;

	m_CursorInfo.nId = 0;
	m_CursorInfo.color = D3DXCOLOR(0.f, 1.f, 0.f, 1.f);
	if(m_EnableStageList.size() == 0)
	{
		m_TargetPointId = -1;
		m_CursorInfo.pos = (*m_pPointList)[m_nStartPointId].pos;
	}
	else
	{
		BsAssert(m_EnableStageList.size() == 1);
		DWORD dwCount = m_pPointList->size();
		for(DWORD i=0; i<dwCount; i++)
		{
			WorldMapPointInfo* pPoint = &((*m_pPointList)[i]);
			if(pPoint->nId == m_EnableStageList[0])
			{
				m_TargetPointId = i;
				break;
			}
		}

		BsAssert(m_TargetPointId != -1);
		m_CursorInfo.pos = (*m_pPointList)[m_TargetPointId].pos;

		char cAreaName[256];
		sprintf(cAreaName, "%s\\%s_%s.dds",
			g_LocalLanguage.GetLanguageDir(),
			(*m_pPointList)[m_TargetPointId].szImageNM,
			g_LocalLanguage.GetLanguageStr());

		g_BsKernel.chdir("interface");
		g_BsKernel.chdir("Local");
		if( m_nAreaNameTexID != -1 )
		{
			SAFE_RELEASE_TEXTURE( m_nAreaNameTexID );
		}
		m_nAreaNameTexID = g_BsKernel.LoadTexture(cAreaName);
		g_BsKernel.chdir("..");
		g_BsKernel.chdir("..");

		ASSERT(m_nAreaNameTexID != -1);
	}

	m_CsrPos = m_CursorInfo.pos;

	//arrow
	m_ArrowInfo.nId = -1;
	m_ArrowInfo.nUVId = 1;
	m_ArrowInfo.pos1 = (*m_pPointList)[m_nStartPointId].pos;
	m_ArrowInfo.pos2 = m_CursorInfo.pos;
		
	//cur lock
	m_bCursorLocked = false;
	m_nCursorLockCounter = -1;
	m_fCsrSpeedX = 0.f;
	m_fCsrSpeedY = 0.f;

	m_bBriefing = false;
	m_bWorldToBriefing = false;
	m_nChangeTick = 0;

	m_bPrev = false;
	m_bNext = false;
	m_bEnableKey = true;

	m_nStageId = -1;
	m_szMapFileName = NULL;
	m_nBriefingTextCount = 0;
	m_nBriefingTime = 0;

	m_nStageTexId = -1;

	Create(0, 0);

	m_bEnable = true;
	m_bShowOn = true;
	
	m_bEnableGuadianMenu = false;
}


void FcMenuWorldMap::GetEnableStageList()
{
	DWORD dwCount = m_pPathList->size();
	for(DWORD i=0; i<dwCount; i++)
	{
		WorldMapPassInfo* pPathInfo = &((*m_pPathList)[i]);
		if(IsClearStagePoint(pPathInfo->nStartId) == false){
			continue;
		}

		for(int j=0; j<3; j++)
		{
			if(pPathInfo->nEndId[j] == -1){
				continue;
			}
			
			if(IsClearStagePoint(pPathInfo->nEndId[j]) == false)
			{
				//예외처리(아스파 분기맵 처리)
				if(g_FCGameData.nPlayerType == MISSION_TYPE_ASPHARR && pPathInfo->nStartId == STAGE_ID_EAURVARRIA)
				{
					if(g_FCGameData.SpeaialSelStageId == pPathInfo->nEndId[j]){
						m_EnableStageList.push_back(pPathInfo->nEndId[j]);
					}
				}
				else{
					m_EnableStageList.push_back(pPathInfo->nEndId[j]);
				}
			}
		}
	}
}

FcMenuWorldMap::~FcMenuWorldMap()
{
	Release();
}

void FcMenuWorldMap::Create(int x, int y)
{	
	g_BsKernel.chdir("interface");
	g_BsKernel.chdir("WorldMap");
	m_nWorldMapTexId = g_BsKernel.LoadTexture(_WORLD_MAP_NAME);
	m_nClearIconTexId = g_BsKernel.LoadTexture(_WORLD_MAP_POS_ICON_CLEAR);
	m_nGoIconTexId = g_BsKernel.LoadTexture(_WORLD_MAP_POS_ICON_GO);
	m_nDisIconTexId = g_BsKernel.LoadTexture(_WORLD_MAP_POS_ICON_DIS);
	g_BsKernel.chdir("..");
	g_BsKernel.chdir("..");
}


void FcMenuWorldMap::Release()
{
	SAFE_RELEASE_TEXTURE(m_nWorldMapTexId);
	SAFE_RELEASE_TEXTURE(m_nClearIconTexId);
	SAFE_RELEASE_TEXTURE(m_nGoIconTexId);
	SAFE_RELEASE_TEXTURE(m_nDisIconTexId);
	SAFE_RELEASE_TEXTURE(m_nStageTexId);
	SAFE_RELEASE_TEXTURE(m_nAreaNameTexID);
}


void FcMenuWorldMap::Process()
{
	if(g_FC.GetMainState() != GAME_STATE_LOOP_TITLE){
		return;
	}

	if(m_bEnable == false){
		return;
	}

	if(m_bBriefing == false &&m_nChangeTick == 0){

		if(m_nStageTexId != -1)
		{
			g_BsKernel.ReleaseTexture(m_nStageTexId);
			m_nStageTexId = -1;
		}

		ProcessWorldMap();
	}
	
	if(m_bBriefing == true){
		LoadBriefingImage();
		if(m_nChangeTick == _CHANGE_TICK){
			ProcessBriefing();
		}
	}
}

void FcMenuWorldMap::ProcessWorldMap()
{
	if(m_nCursorLockCounter >= 0){
		m_nCursorLockCounter++;
	}

	MoveArrow();

	if(LockArrow() == false)
	{	
		m_CsrPos.x += m_fCsrSpeedX;
		if(m_CsrPos.x < _MAP_SIDE_SPACE){
			m_CsrPos.x = _MAP_SIDE_SPACE;
		}
		else if(m_CsrPos.x > m_MapSize.x - _MAP_SIDE_SPACE){
			m_CsrPos.x = m_MapSize.x - _MAP_SIDE_SPACE;
		}

		m_CsrPos.y += m_fCsrSpeedY;
		if(m_CsrPos.y < _MAP_SIDE_SPACE){
			m_CsrPos.y = _MAP_SIDE_SPACE;
		}
		else if(m_CsrPos.y > m_MapSize.y - _MAP_SIDE_SPACE){
			m_CsrPos.y = m_MapSize.y - _MAP_SIDE_SPACE;
		}

		m_CursorInfo.pos = m_CsrPos;
	}

	if(g_MenuHandle->GetCurMenuType() == _FC_MENU_SELECT_STAGE){
		OnInputKeyforWorldMap();
	}
}

#define _TEXT_LINE_GAP			40
void FcMenuWorldMap::OnInputKeyforWorldMap()
{
	if(m_bEnableKey == false){
		return;
	}

	int nA_PressTick = CInputPad::GetInstance().GetKeyPressTick( g_FCGameData.nEnablePadID, PAD_INPUT_A );
	int nB_PressTick = CInputPad::GetInstance().GetKeyPressTick( g_FCGameData.nEnablePadID, PAD_INPUT_B );
	if(nA_PressTick == 1 && m_TargetPointId != -1)
	{	
		int nStageId = (*m_pPointList)[m_TargetPointId].nId;
		if(IsClearStagePoint(nStageId) || 
			IsNextStagePoint(nStageId) || 
			(m_nStartPointId == m_TargetPointId))
		{
			if(strlen((*m_pPointList)[m_TargetPointId].szStage) > 0 )
			{
				m_bBriefing = true;
				m_nStageId = nStageId;
				UpdateEnableGuadianMenu();
				m_szMapFileName = (*m_pPointList)[m_TargetPointId].szStage;
				m_nBriefingTime = g_BsUiSystem.GetMenuTick();
				
				for(int i=0; i<BRIEFING_VIEW_TEXT_COUNT; i++)
				{
					m_BriefingTick[i].nTick = _TEXT_LINE_GAP * (i + 1);
					m_BriefingTick[i].nTextID = i - (BRIEFING_VIEW_TEXT_COUNT /2);
				}
			}
		}
	}
	else if(nB_PressTick == 1){
		m_bPrev = true;
		m_nStageId = -1;
		m_szMapFileName = NULL;
	}
}


void FcMenuWorldMap::UpdateEnableGuadianMenu()
{
	int nSelStageID = g_MenuHandle->GetMenuWorldMap()->GetStageId();
	switch(g_FCGameData.nPlayerType)
	{
	case MISSION_TYPE_INPHYY:
		{
			switch(m_nStageId)
			{
			case STAGE_ID_VARRFARRINN:
			case STAGE_ID_ANOTHER_WORLD: m_bEnableGuadianMenu = false; break;
			default:					 m_bEnableGuadianMenu = true; break;
			}
			break;
		}
	case MISSION_TYPE_ASPHARR:
		{
			switch(m_nStageId)
			{
			case STAGE_ID_VARRFARRINN: m_bEnableGuadianMenu = false; break;
			default:				   m_bEnableGuadianMenu = true; break;
			}
			break;
		}
	case MISSION_TYPE_MYIFEE:
		{
			switch(m_nStageId)
			{
			case STAGE_ID_FELPPE:	m_bEnableGuadianMenu = false; break;
			default:				m_bEnableGuadianMenu = true; break;
			}
			break;
		}
	case MISSION_TYPE_TYURRU:
	case MISSION_TYPE_KLARRANN:
	case MISSION_TYPE_DWINGVATT:
	case MISSION_TYPE_VIGKVAGK:
		{
			m_bEnableGuadianMenu = false;
			break;
		}
	default: BsAssert(0);
	}
}

void FcMenuWorldMap::ProcessBriefing()
{
	if(m_bEnableKey == false){
		return;
	}
	
	if(m_bEnableGuadianMenu)
	{
		int nPressTick = CInputPad::GetInstance().GetKeyPressTick( g_FCGameData.nEnablePadID, PAD_INPUT_A );
		if(nPressTick == 1){
			m_bNext = true;
			return;
		}
	}
	else
	{
		int nPressTick = CInputPad::GetInstance().GetKeyPressTick( g_FCGameData.nEnablePadID, PAD_INPUT_START );
		if(nPressTick == 1){
			m_bNext = true;
			return;
		}
	}

	int nPressTick = CInputPad::GetInstance().GetKeyPressTick( g_FCGameData.nEnablePadID, PAD_INPUT_B );
	if(nPressTick == 1)
	{
		m_bBriefing = false;
		m_bEnableGuadianMenu = false;
		m_nChangeTick = 0;
		m_nBriefingTextCount = 0;
		return;
	}
	
}


void FcMenuWorldMap::Update()
{
	if(m_bShowOn == false){
		return;
	}

	if(m_bBriefing == true && m_nChangeTick < _CHANGE_TICK){
		m_nChangeTick++;
	}
	
	if(m_bBriefing == false || m_nChangeTick != _CHANGE_TICK)
	{
		//world map
		UpdateWorldMap();

		// arrow
		UpdateArrow();

		// point
		UpdateStagePoint();
	
		// Area Name
		if(m_bBriefing == false){
			UpdateAreaName();
		}

		//cursor
		UpdateCursor();
	}

	if(m_bBriefing == true)
	{
		UpdateBriefing();
	}
}

void FcMenuWorldMap::UpdateWorldMap()
{	
	float fAlpha = 1.f - (float)m_nChangeTick / _CHANGE_TICK;
	SIZE size = g_BsKernel.GetTextureSize(m_nWorldMapTexId);
	g_BsKernel.DrawUIBox(_Ui_Mode_Image,
		(int)m_MapPos.x, (int)m_MapPos.y,
		(int)(m_MapPos.x + m_MapSize.x), (int)(m_MapPos.y + m_MapSize.y),
		0.f, D3DXCOLOR(1,1,1,fAlpha),
		0.f, m_nWorldMapTexId,
		0, 0, 1024, 512);
}

void FcMenuWorldMap::UpdateStagePoint()
{	
	DWORD dwCount = m_pPointList->size();
	for(DWORD i=0; i<dwCount; i++)
	{
		WorldMapPointInfo* pPoint = &((*m_pPointList)[i]);
		int nStageId = pPoint->nId;

		BsAssert(g_FcWorldPosIcon[nStageId].nId == nStageId);

		int u1(0), v1(0), u2(0), v2(0);
		int sizex(0), sizey(0);
		int x(0), y(0);
		//예외 처리...같은 아이디인데...브리핑에서 맵상에 위치가 서로 틀리다.. ㅡ.ㅡ;;;;
		if(g_FCGameData.nPlayerType == MISSION_TYPE_ASPHARR && nStageId == STAGE_ID_VARRVAZZAR)
		{
			u1 = g_FcWorldPosIcon[STAGE_ID_VARRVAZZAR_2].u1;
			v1 = g_FcWorldPosIcon[STAGE_ID_VARRVAZZAR_2].v1;
			u2 = g_FcWorldPosIcon[STAGE_ID_VARRVAZZAR_2].u2;
			v2 = g_FcWorldPosIcon[STAGE_ID_VARRVAZZAR_2].v2;

			sizex = u2 - u1;
			sizey = v2 - v1;

			x = g_FcWorldPosIcon[STAGE_ID_VARRVAZZAR_2].posx - (sizex/2);
			y = g_FcWorldPosIcon[STAGE_ID_VARRVAZZAR_2].posy - (sizey/2);
		}
		else
		{
			u1 = g_FcWorldPosIcon[nStageId].u1;
			v1 = g_FcWorldPosIcon[nStageId].v1;
			u2 = g_FcWorldPosIcon[nStageId].u2;
			v2 = g_FcWorldPosIcon[nStageId].v2;

			sizex = u2 - u1;
			sizey = v2 - v1;

			x = g_FcWorldPosIcon[nStageId].posx - (sizex/2);
			y = g_FcWorldPosIcon[nStageId].posy - (sizey/2);
		}

		int nTexID = -1;
		if(IsClearStagePoint(nStageId)){
			nTexID = m_nClearIconTexId;
		}else if(IsNextStagePoint(nStageId)){
			nTexID = m_nGoIconTexId;
		}else{
			nTexID = m_nDisIconTexId;
		}	

		BsAssert(nTexID != -1);
		g_BsKernel.DrawUIBox_s(_Ui_Mode_Image,
			x, y, sizex, sizey,
			0.f, D3DXCOLOR(1.f, 1.f, 1.f, 1.f),
			0.f, nTexID,
			u1, v1, u2, v2);

		//point icon
		BsUiUVId nUVId = (BsUiUVId)GetPointUVID(nStageId);
		UVImage* pImg = g_UVMgr.GetUVImage(nUVId);
		BsAssert(pImg != NULL);
		
		int nPointSizeX = (pImg->u2 - pImg->u1) / 2;
		int nPointSizeY = (pImg->v2 - pImg->v1) / 2;

		int x1 = (int)(m_MapPos.x + pPoint->pos.x - nPointSizeX);
		int y1 = (int)(m_MapPos.y + pPoint->pos.y - nPointSizeY);
		int x2 = (int)(m_MapPos.x + pPoint->pos.x + nPointSizeX);
		int y2 = (int)(m_MapPos.y + pPoint->pos.y + nPointSizeY);

		g_BsKernel.DrawUIBox(_Ui_Mode_Image,
			x1, y1, x2, y2,
			0.f, D3DXCOLOR(1.f, 1.f, 1.f, 1.f),
			0.f, pImg->nTexId,
			pImg->u1, pImg->v1, pImg->u2, pImg->v2);
	}
}

int FcMenuWorldMap::GetPointUVID(int nStageID)
{
	if(m_nStartPathId != -1)
	{
		WorldMapPassInfo* pCurPathInfo = &((*m_pPathList)[m_nStartPathId]);
		if(nStageID == pCurPathInfo->nStartId){
			return _UV_MN_WorldMap_Start;
		}
	}

	if(IsClearStagePoint(nStageID)){
		return _UV_MN_WorldMap_nml_0;
	}
	else if(IsNextStagePoint(nStageID))
	{	
		int nAddFrame = (g_nProcessTick % 40) / 10;
		return _UV_MN_WorldMap_Target_0 + nAddFrame;
	}

	return _UV_MN_WorldMap_Target_None;
}

bool FcMenuWorldMap::IsClearStagePoint(int nStageID)
{	
	int nHeroSoxId = g_MissionToSoxID[g_FCGameData.nPlayerType];
	BsAssert(nHeroSoxId == g_FCGameData.tempUserSave.nHeroID);

	std::vector<int>* pClearStageList = &(g_FCGameData.tempUserSave.ClearStageList);
	DWORD dwCount = pClearStageList->size();
	for(DWORD i=0; i<dwCount; i++)
	{
		if((*pClearStageList)[i] == nStageID){
			return true;
		}
	}

	return false;
}


bool FcMenuWorldMap::IsNextStagePoint(int nStageID)
{
	DWORD dwCount = m_EnableStageList.size();
	for(DWORD i=0; i<dwCount; i++)
	{
		if(nStageID == m_EnableStageList[i]){
			return true;
		}
	}

	return false;
}

void FcMenuWorldMap::UpdateAreaName()
{
	if(m_TargetPointId == -1){
		return;
	}

	assert(m_nAreaNameTexID != -1);

	int nX(0), nY(0);
	nX = (int)((*m_pPointList)[m_TargetPointId].posNM.x);
	nY = (int)((*m_pPointList)[m_TargetPointId].posNM.y);

	if(nX == 0){
		nX = (int)((*m_pPointList)[m_TargetPointId].pos.x) - 80;
	}

	if(nY == 0){
		nY = (int)((*m_pPointList)[m_TargetPointId].pos.y) + 16;
	}

	SIZE size = g_BsKernel.GetTextureSize(m_nAreaNameTexID);
	g_BsKernel.DrawUIBox_s(_Ui_Mode_Image,
		nX, nY, size.cx * 2/3, size.cy * 2/3,
		0.f,  D3DXCOLOR(1, 1, 1, 1),
		0.f, m_nAreaNameTexID,
		0, 0, size.cx, size.cy);
}


void FcMenuWorldMap::UpdateArrow()
{
	if(m_TargetPointId == -1){
		return;
	}

	int nTargetStageId = (*m_pPointList)[m_TargetPointId].nId;

	m_ArrowInfo.pos2 = (*m_pPointList)[m_TargetPointId].pos;

	D3DXVECTOR2 vSubOut;
	D3DXVec2Subtract(&vSubOut, &m_ArrowInfo.pos1, &m_ArrowInfo.pos2);

	D3DXVECTOR2 vNorOut;
	D3DXVec2Normalize(&vNorOut, &vSubOut);

	D3DXVECTOR2 vInvOut = D3DXVECTOR2(vNorOut.y, -vNorOut.x);

	D3DXVECTOR2 vPos1 = m_MapPos + m_ArrowInfo.pos1 + ( vInvOut * 15 );
	D3DXVECTOR2 vPos2 = m_MapPos + m_ArrowInfo.pos1 + ( vInvOut * -15 );
	D3DXVECTOR2 vPos3 = m_MapPos + m_ArrowInfo.pos2 + ( vInvOut * 15 );
	D3DXVECTOR2 vPos4 = m_MapPos + m_ArrowInfo.pos2 + ( vInvOut * -15 );

	int nUVID = _UV_MN_WorldMap_Direction_None;
	if(IsClearStagePoint(nTargetStageId) || IsNextStagePoint(nTargetStageId)){
		nUVID = _UV_MN_WorldMap_Direction;
	}
	
	UVImage* pImg = g_UVMgr.GetUVImage(nUVID);
	g_BsKernel.DrawUIBox_Pos4(_Ui_Mode_Image,
		(int)vPos1.x, (int)vPos1.y,
		(int)vPos2.x, (int)vPos2.y,
		(int)vPos3.x, (int)vPos3.y,
		(int)vPos4.x, (int)vPos4.y,
		0.f,  D3DXCOLOR(1, 1, 1, 1),
		pImg->nTexId,
		pImg->u1, pImg->v1, pImg->u2, pImg->v2);
}

void FcMenuWorldMap::UpdateArrowList()
{
	DWORD dwCount = m_ArrowList.size();
	for(DWORD i=0; i<dwCount; i++)
	{
		WorldMapArrowInfo* pArrowInfo = &(m_ArrowList[i]);
		
		D3DXVECTOR2 vSubOut;
		D3DXVec2Subtract(&vSubOut, &pArrowInfo->pos1, &pArrowInfo->pos2);

		D3DXVECTOR2 vNorOut;
		D3DXVec2Normalize(&vNorOut, &vSubOut);

		D3DXVECTOR2 vInvOut = D3DXVECTOR2(vNorOut.y, -vNorOut.x);

		D3DXVECTOR2 vPos1 = m_MapPos + pArrowInfo->pos1 + ( vInvOut * 20 );
		D3DXVECTOR2 vPos2 = m_MapPos + pArrowInfo->pos1 + ( vInvOut * -20 );
		D3DXVECTOR2 vPos3 = m_MapPos + pArrowInfo->pos2 + ( vInvOut * 20 );
		D3DXVECTOR2 vPos4 = m_MapPos + pArrowInfo->pos2 + ( vInvOut * -20 );

		g_BsKernel.DrawUIBox_Pos4(_Ui_Mode_Box, 
			(int)vPos1.x, (int)vPos1.y,
			(int)vPos2.x, (int)vPos2.y,
			(int)vPos3.x, (int)vPos3.y,
			(int)vPos4.x, (int)vPos4.y,
			0.f,  D3DXCOLOR(1, 1, 1, 1));


		TestPoint((int)vPos1.x, (int)vPos1.y, D3DXCOLOR(1, 0, 0, 1));
		TestPoint((int)vPos2.x, (int)vPos2.y, D3DXCOLOR(0, 1, 0, 1));
		TestPoint((int)vPos3.x, (int)vPos3.y, D3DXCOLOR(0, 0, 1, 1));
		TestPoint((int)vPos4.x, (int)vPos4.y, D3DXCOLOR(1, 1, 1, 1));
	}
}

void FcMenuWorldMap::TestPoint(int x, int y, D3DXCOLOR color)
{
	int x1 = (int)x-5;
	int y1 = (int)y-5;
	int x2 = (int)x+5;
	int y2 = (int)y+5;

	g_BsKernel.DrawUIBox(_Ui_Mode_Box,
		x1, y1, x2, y2,
		0.f,
		color);
}

void FcMenuWorldMap::UpdateCursor()
{	
	UVImage* pImg = g_UVMgr.GetUVImage(_UV_MN_WorldMap_Cursor_1);
	BsAssert(pImg != NULL);

	int x1 = (int)(m_MapPos.x + m_CursorInfo.pos.x - 10);
	int y1 = (int)(m_MapPos.y + m_CursorInfo.pos.y - (pImg->v2 - pImg->v1 + 10));
	int x2 = (int)(m_MapPos.x + m_CursorInfo.pos.x + (pImg->u2 - pImg->u1 - 10));
	int y2 = (int)(m_MapPos.y + m_CursorInfo.pos.y + 10);

	if(m_TargetPointId != -1)
	{
		int nUVId = _UV_MN_WorldMap_Cursor_0;
		float fAlpha = 0.5f;

		//갈수 있는 곳...
		int nStageID = (*m_pPointList)[m_TargetPointId].nId;
		if(IsClearStagePoint(nStageID) || IsNextStagePoint(nStageID))
		{
			nUVId = _UV_MN_WorldMap_Cursor_1;
			fAlpha = 1.f;
		}
				
		UVImage* pImg = g_UVMgr.GetUVImage(nUVId);
		BsAssert(pImg != NULL);

		g_BsKernel.DrawUIBox(_Ui_Mode_Image,
			x1, y1, x2, y2,
			0.f, D3DXCOLOR(1,1,1,fAlpha),
			0.f, pImg->nTexId,
			pImg->u1, pImg->v1, pImg->u2, pImg->v2);
	}
	else
	{
		//target이 안잡힌 경우
		UVImage* pCurImg = g_UVMgr.GetUVImage(_UV_MN_WorldMap_Cursor_0);
		BsAssert(pCurImg != NULL);

		g_BsKernel.DrawUIBox(_Ui_Mode_Image,
			x1, y1, x2, y2,
			0.f, D3DXCOLOR(1,1,1,1),
			0.f, pCurImg->nTexId,
			pCurImg->u1, pCurImg->v1, pCurImg->u2, pCurImg->v2);


		UVImage* pLightImg = g_UVMgr.GetUVImage(_UV_MN_WorldMap_Cursor_1);
		BsAssert(pLightImg != NULL);
		
		int nTick = g_nProcessTick % 40;
		float fAlpha = 1.f;
		if(nTick < 10){
			fAlpha = nTick * 0.1f;
		}
		else if(nTick >= 30){
			fAlpha = (40 - nTick) * 0.1f;
		}

		int lx1 = x1 - (int)(fAlpha * 2);
		int ly1 = y1 - (int)(fAlpha * 2);
		int lx2 = x2 + (int)(fAlpha * 2);
		int ly2 = y2 + (int)(fAlpha * 2);

		g_BsKernel.DrawUIBox(_Ui_Mode_Image,
			lx1, ly1, lx2, ly2,
			0.f, D3DXCOLOR(1,1,1,fAlpha),
			0.f, pLightImg->nTexId,
			pLightImg->u1, pLightImg->v1, pLightImg->u2, pLightImg->v2);
	}
}

void FcMenuWorldMap::MoveArrow()
{
	D3DXVECTOR2 pos = D3DXVECTOR2(0.f, 0.f);
	bool bLStick = false;

	if(m_bEnableKey == true)
	{
		if(CInputPad::GetInstance().GetKeyPressTick(g_FCGameData.nEnablePadID, PAD_INPUT_LSTICK)){
// [beginmodify] 2006/1/16 junyash PS#1259,3249,3251, TCR#033PER multi controller
			#if 0
			KEY_EVENT_PARAM* pParam = CInputPad::GetInstance().GetKeyParam(-1, 0);
			#else
			KEY_EVENT_PARAM* pParam = CInputPad::GetInstance().GetKeyParam(g_FCGameData.nEnablePadID, 0);
			#endif
// [endmodify] junyash
			pos = D3DXVECTOR2((float)pParam->nPosX*0.0003f, -(float)pParam->nPosY*0.0003f);

			bLStick = true;
		}
		else{
			for(int Input = PAD_INPUT_UP; Input <= PAD_INPUT_RIGHT; ++Input)
			{
				int nPressTick = CInputPad::GetInstance().GetKeyPressTick( g_FCGameData.nEnablePadID, Input );
				if(nPressTick > 0)
				{
					switch(Input)
					{
					case PAD_INPUT_UP:		pos.y -= 20.f; break;
					case PAD_INPUT_DOWN:	pos.y += 20.f; break;
					case PAD_INPUT_LEFT:	pos.x -= 20.f; break;
					case PAD_INPUT_RIGHT:	pos.x += 20.f; break;
					}
				}
			}
		}
	}

	if (m_bCursorLocked == true && m_nCursorLockCounter > 30)
	{
		m_bCursorLocked = false;
		m_nCursorLockCounter = -1;
	}

	if (m_bCursorLocked == false)
	{
		if(bLStick == true)
		{
			m_fCsrSpeedX += (pos.x - m_fCsrSpeedX) * .3f;
			m_fCsrSpeedY += (pos.y - m_fCsrSpeedY) * .3f;
		}
		else
		{
			if(pos.x != 0.f || pos.y != 0.f) {
				m_fCsrSpeedX += pos.x * .2f;
				m_fCsrSpeedY += pos.y * .2f;
				
				if(m_fCsrSpeedX >= 3.6f){
					m_fCsrSpeedX = 3.6f;
				}
				else if(m_fCsrSpeedX <= -3.6f){
					m_fCsrSpeedX = -3.6f;
				}

				if(m_fCsrSpeedY >= 3.6f){
					m_fCsrSpeedY = 3.6f;
				}
				else if(m_fCsrSpeedY <= -3.6f){
					m_fCsrSpeedY = -3.6f;
				}
			}
			else{
				m_fCsrSpeedX = 0.f;
				m_fCsrSpeedY = 0.f;
			}
		}
	}
}


bool FcMenuWorldMap::LockArrow()
{
	if (m_bCursorLocked || m_nCursorLockCounter >= 0)
		return false;

	float fLenShortest = 10.f;
	if(m_TargetPointId != -1)
	{
		WorldMapPointInfo* pPoint = &((*m_pPointList)[m_TargetPointId]);

		D3DXVECTOR2 Diff;
		D3DXVec2Subtract(&Diff, &m_CsrPos, &pPoint->pos);
		float fLen = D3DXVec2Length(&Diff);

		if (fLen > fLenShortest)
		{
			m_TargetPointId = -1;
			SAFE_RELEASE_TEXTURE(m_nAreaNameTexID);
		}
	}

	bool bLocked = false;
	int nTargetId = -1;

	DWORD dwCount = m_pPointList->size();
	for(DWORD i=0; i<dwCount; i++)
	{
		WorldMapPointInfo* pPoint = &((*m_pPointList)[i]);
		if(i == m_TargetPointId)
			continue;

		D3DXVECTOR2 Diff;
		D3DXVec2Subtract(&Diff, &m_CsrPos, &pPoint->pos);
		float fLen = D3DXVec2Length(&Diff);

		if (fLen < fLenShortest)
		{	
			fLenShortest = fLen;
			bLocked = true;
			nTargetId = i;
		}
	}

	if(bLocked == true)
	{
		m_bCursorLocked = true;
		m_nCursorLockCounter = 0;

		m_TargetPointId = nTargetId;
		m_CsrPos = (*m_pPointList)[m_TargetPointId].pos;

		m_fCsrSpeedX = 0.f;
		m_fCsrSpeedY = 0.f;

		char cAreaName[256];
		sprintf(cAreaName, "%s\\%s_%s.dds",
			g_LocalLanguage.GetLanguageDir(),
			(*m_pPointList)[m_TargetPointId].szImageNM,
			g_LocalLanguage.GetLanguageStr());

		g_BsKernel.chdir("interface");
		g_BsKernel.chdir("Local");
		if( m_nAreaNameTexID != -1 )
		{
			SAFE_RELEASE_TEXTURE( m_nAreaNameTexID );
		}
		m_nAreaNameTexID = g_BsKernel.LoadTexture(cAreaName);
		g_BsKernel.chdir("..");
		g_BsKernel.chdir("..");

		ASSERT(m_nAreaNameTexID != -1);
	}

	return bLocked;
}

void FcMenuWorldMap::LoadBriefingImage()
{
	if(m_nStageTexId != -1){
		return;
	}

	g_BsKernel.chdir("interface");
	g_BsKernel.chdir("WorldMap");
	m_nStageTexId = g_BsKernel.LoadTexture((*m_pPointList)[m_TargetPointId].szImage);
	g_BsKernel.chdir("..");
	g_BsKernel.chdir("..");
}

void FcMenuWorldMap::UpdateBriefing()
{
	float fAlpha = 1.f;

	D3DXVECTOR2 vHalf = D3DXVECTOR2(m_MapSize.x / 2.f, m_MapSize.y / 2.f);

	int x1 = (int)(m_MapPos.x + vHalf.x - (vHalf.x * fAlpha));
	int y1 = (int)(m_MapPos.y + vHalf.y - (vHalf.y * fAlpha));
	int x2 = (int)(m_MapPos.x + vHalf.x + (vHalf.x * fAlpha));
	int y2 = (int)(m_MapPos.y + vHalf.y + (vHalf.y * fAlpha));

	fAlpha = (float)m_nChangeTick / _CHANGE_TICK;

	g_BsKernel.DrawUIBox(_Ui_Mode_Image,
		x1, y1, x2, y2,
		0.f, D3DXCOLOR(1.f, 1.f, 1.f, fAlpha),
		0.f, m_nStageTexId,
		0, 0, 1024, 512);

	int nY = 280;

	UVImage* pImg = g_UVMgr.GetUVImage(_UV_SpeechMessage);
	if( pImg != NULL)
	{
		fAlpha -= 0.3f;
		fAlpha = (fAlpha < 0.f) ? 0.f : fAlpha;

		g_BsKernel.DrawUIBox(_Ui_Mode_Image,
			x1, nY, x2, nY + 270,
			0.f, D3DXCOLOR(1.f, 1.f, 1.f, fAlpha),
			0.f, pImg->nTexId,
			pImg->u1, pImg->v1, pImg->u2, pImg->v2);
	}

	UpdateBriefingText(x1, nY+15, x2, -1);
}


void FcMenuWorldMap::UpdateBriefingText(int x1, int y1, int x2, int y2)
{	
	int ntime = g_BsUiSystem.GetMenuTick();
	int nStartId = m_nStageId * _STAGE_BRIEFINE_MAX_COUNT;
		
	for(int i=0; i<BRIEFING_VIEW_TEXT_COUNT; i++)
	{
		int nTextId = m_BriefingTick[i].nTextID;
		if(nTextId >= 0)
		{
			int nTick = m_BriefingTick[i].nTick;
			float fAlpha = 1.f;
			if(nTick < 32){
				fAlpha = nTick * 0.03125f;
			}
			else if(nTick >= _BRIEFING_TEXT_TICK - 32){
				fAlpha = (_BRIEFING_TEXT_TICK - nTick) * 0.03125f;
			}
			
			char szAttr[256];
			sprintf(szAttr, "@(reset)@(scale=1.0,1.0)@(color=255,255,255,%d)@(space=-3)@(align=1)", (int)(fAlpha*255));
			g_pFont->DrawUIText(0, 0, -1, -1, szAttr);

			nTextId += nStartId;
			char* szText = g_FCBriefingSOX.GetItemData(nTextId);
			
			if(szText != NULL){
				g_pFont->DrawUIText(x1, y1+nTick, x2, nTick, szText);
			}
		}

		if(m_BriefingTick[i].nTick <= 0)
		{
			m_BriefingTick[i].nTick = BRIEFING_VIEW_TEXT_COUNT * _TEXT_LINE_GAP;
			m_BriefingTick[i].nTextID += BRIEFING_VIEW_TEXT_COUNT;

			if(m_BriefingTick[i].nTextID >= _STAGE_BRIEFINE_MAX_COUNT){
				m_BriefingTick[i].nTextID -= BRIEFING_VIEW_TEXT_COUNT + _STAGE_BRIEFINE_MAX_COUNT;
			}

		}else{
			m_BriefingTick[i].nTick--;
		}
	}
}