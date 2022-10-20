#include "StdAfx.h"
#include "BsKernel.h"
#include "FcWorld.h"
#include "FcHeroObject.h"
#include "FcConsole.h"
#include "PerfCheck.h"
#include "FcGlobal.h"
#include "FcInterfaceManager.h"
#include "FcAIObject.h"
#include "FcHeroObject.h"
#include "FantasyCore.h"
#include "FcFXManager.h"
#include "FcCameraObject.h"
#include "FcRealtimeMovie.h"
#include "FcItem.h"
#include "FcItem.h"

#include "FcAbilityManager.h"
#include "BsClipTest.h"
#include "BsImageProcess.h"

#ifdef _USE_CONSOLE_COMMAND_

char g_szConsoleString[ 1024 ];
char g_szSearchString[ 1024 ] = { 0 };
int g_nSearchCommand = -1;

char *g_szConsoleCommand[ ] =
{
	"charallopen",
	"changehero",
	"wireframe",
	"minimap",
	"frameskip",
	"showfps",
	"showarrow",
	"shownavigationmesh",
	"showcollisionmesh",
	"unitai",
	"changeani",
	"resetai",
	"showtroopstate",
	"showfriendlygauge",
	"showenemygauge",
	"showminimapallobj"
	"showgaugedistance",
	"showunitstate",
	"showphysics",
	"addforce",
	"physics on",
	"physics off",
	"gravity",
	"addorb",
	"addtrueorb",
	"missionsuccess",
	"missionfail",
	"god",
	"menushow",
	"dofenable",
	"doffocus",
	"dofrange",
	"dofnear",
	"doffar",
	"dofcheck",
	"debugrenderbaseobject",
	"debugrendertroopobject",
	"saveset",
	"loadset",
	"OrbsparkFire",
	"playmovie",
	"showvariable",
	"oneshotmode",
	"showparts", 
	"showshadow", 
	"skipmode",
	"reloadfx",
	"abillight",
	"capture",
	"billboardenable",
	"objectbillboardrange",
	"instancing",
	"pointlight",
	"setcam",
	"setfullhp",
	"quit",
	"rmplay",
	"showorbs",
	"addexp",
	"nodamage",
	"drawprop",
	"dropitem",
	"loadmemcheck",
	"cliptest",
	"reloadmaterial",
	"showheropos",
	"dofoutfar",
	"dofoutNear",
	"setdie",
	"qqq",
	"www",
	"lightar",
	"lightag",
	"lightab",
	"lightdr",
	"lightdg",
	"lightdb",
	"lightsr",
	"lightsg",
	"lightsb",
	"sceneorigin",
	"sceneblur",
	"sceneglow",
	"scenethreshold",
	"ooo",				// 실기 무비 테스트용. 실기무비에서 Fade된 경우 다시 화면을 밝히기 위해 사용.
	"sii",				// show item info
	"crashall",
	"setfps",			// FPS 조절합니다.
	"showsafearea",		// safe area를 보여줍니다.
	"showattr",
	"addbonus",
	"showaistate",
	"setheropos"

	"setrank"
	//"liballopen",
};

#ifdef _LOAD_MAP_CHECK_
extern bool	g_bMemCheckPrint;
#endif

void ProcessConsoleString(const char *pString)
{
	char *pFindPtr;

	pFindPtr = ( char * ) strstr(pString, "changehero");
	if(pFindPtr)
	{
		CFcWorld::GetInstance().CreateHero( atoi( pFindPtr+11 ), 0.f, 0.f );
		return;
	}

	pFindPtr = ( char * ) strstr(pString, "wireframe");
	if(pFindPtr)
	{
		if( atoi( pFindPtr+10 ) == 0 )	{ g_BsKernel.GetDevice()->SetRenderState( D3DRS_FILLMODE, D3DFILL_SOLID ); }
		else							{ g_BsKernel.GetDevice()->SetRenderState( D3DRS_FILLMODE, D3DFILL_WIREFRAME ); }
		return;
	}

	pFindPtr = ( char * ) strstr(pString, "minimap");
	if(pFindPtr)
	{
		g_InterfaceManager.GetInstance().MinimapShowToggle();
		return;
	}

	pFindPtr = ( char * ) strstr(pString, "frameskip");
	if(pFindPtr)
	{
		if( atoi( pFindPtr+10 ) == 0 )	{ g_FC.StopFrameSkip(); }
		else							{ g_FC.StartFrameSkip(); }
		return;
	}

	pFindPtr = ( char * ) strstr(pString, "showfps");
	if(pFindPtr)
	{
		if( atoi( pFindPtr+strlen("showfps")+1 ) == 0 ) { CBsKernel::GetInstance().ShowFPS(false, 100, 100); }
		else											{ CBsKernel::GetInstance().ShowFPS(true, 100, 100); }
		return;
	}

	pFindPtr = ( char * ) strstr(pString, "showarrow");
	if(pFindPtr)
	{
		if( atoi( pFindPtr+strlen("showarrow")+1 ) == 0 )	{ g_FCGameData.bShowDebugArrow = false; }
		else												{ g_FCGameData.bShowDebugArrow = true;	}
		return;
	}

	pFindPtr = ( char * ) strstr(pString, "shownavigationmesh");
	if(pFindPtr)
	{
		if( atoi( pFindPtr+strlen("shownavigationmesh")+1 ) == 0 )	{ g_FCGameData.bShowNavigationMesh = false; }
		else														{ g_FCGameData.bShowNavigationMesh = true;	}
		return;
	}

	pFindPtr = ( char * ) strstr(pString, "showcollisionmesh");
	if(pFindPtr)
	{
		if( atoi( pFindPtr+strlen("showcollisionmesh")+1 ) == 0 )	{ g_FCGameData.bShowCollisionMesh = false; }
		else														{ g_FCGameData.bShowCollisionMesh = true;	}
		return;
	}

	pFindPtr = ( char * ) strstr(pString, "unitai" );
	if(pFindPtr)
	{
		if( atoi( pFindPtr+strlen("unitai")+1 ) == 0 )	{ g_FCGameData.bEnableUnitAI = false; }
		else											{ g_FCGameData.bEnableUnitAI = true; }
		return;
	}

	pFindPtr = ( char * ) strstr(pString, "changeani");
	if(pFindPtr)
	{
		CFcWorld::GetInstance().GetHeroHandle()->ChangeAnimation( atoi( pFindPtr+strlen("changeani") + 1 ), 0 );
		return;
	}

	pFindPtr = ( char * ) strstr(pString, "resetai");
	if(pFindPtr)
	{
		CFcAIObject::ReinitializeObjects();
		return;
	}

	pFindPtr = ( char * ) strstr(pString, "showtroopstate");
	if(pFindPtr)	{
		if( atoi( pFindPtr+strlen("Showtroopstate")+1 ) == 0 ) { g_FCGameData.bShowTroopState = false; }
		else											{ g_FCGameData.bShowTroopState = true; }
		return;
	}

	pFindPtr = ( char * ) strstr(pString, "showfriendlygauge");
	if(pFindPtr)	{
		if( atoi( pFindPtr+strlen("showfriendlygauge")+1 ) == 0 ) { g_FCGameData.bShowFriendlyGauge = false; }
		else											{ g_FCGameData.bShowFriendlyGauge = true; }
		return;
	}

	pFindPtr = ( char * ) strstr(pString, "showenemygauge");
	if(pFindPtr)	{
		if( atoi( pFindPtr+strlen("showenemygauge")+1 ) == 0 ) { g_FCGameData.bShowEnemyGauge = false; }
		else											{ g_FCGameData.bShowEnemyGauge = true; }
		return;
	}

	pFindPtr = ( char * ) strstr(pString, "setgaugedistance");
	if(pFindPtr)
	{
		int nFar = atoi( pFindPtr+strlen("setgaugedistance")+1 );
		g_FCGameData.nShowTroopGaugeForFar = nFar;
		return;
	}

	pFindPtr = ( char * ) strstr(pString, "showunitstate");
	if(pFindPtr)	{
		if( atoi( pFindPtr+strlen("showunitstate")+1 ) == 0 ) { g_FCGameData.bShowUnitState = false; }
		else											{ g_FCGameData.bShowUnitState = true; }
		return;
	}

	pFindPtr = ( char * ) strstr(pString, "showphysics");
	if(pFindPtr)
	{
		if( atoi( pFindPtr+strlen("showphysics")+1 ) == 0 ) { g_BsKernel.GetPhysicsMgr()->SetVisible(false); }
		else											{ g_BsKernel.GetPhysicsMgr()->SetVisible(true); }
		return;
	}

	pFindPtr = ( char * ) strstr(pString, "addforce");
	if(pFindPtr)
	{
		float fForceScalar = (float)atof( pFindPtr+strlen("addforce")+1 );
		HeroObjHandle hHero = CFcWorld::GetInstance().GetHeroHandle();
		CCrossVector *cross = hHero->GetCrossVector();			
		g_BsKernel.GetPhysicsMgr()->AddForce( &(cross->m_ZVector * fForceScalar));
		return;
	}
	
	if(strstr(pString, "physics on"))
	{
		g_BsKernel.GetPhysicsMgr()->SetEnable(true);
		return;
	}

	if(strstr(pString, "physics off"))
	{
		g_BsKernel.GetPhysicsMgr()->SetEnable(false);
		return;
	}

	pFindPtr = (char *) strstr(pString, "gravity");
	if(pFindPtr)
	{
		float fGravity = (float)atof(pFindPtr + strlen("gravity") + 1);
		g_BsKernel.GetPhysicsMgr()->SetGravity(fGravity);
		return;
	}

	pFindPtr = ( char * ) strstr(pString, "addorb");
	if(pFindPtr)
	{
		CFcWorld::GetInstance().GetHeroHandle()->AddOrbSpark( atoi( pFindPtr+strlen("addorb")+1 ) );
		return;
	}

	pFindPtr = ( char * ) strstr(pString, "addtrueorb");
	if(pFindPtr)
	{
		CFcWorld::GetInstance().GetHeroHandle()->AddTrueOrbSpark( atoi( pFindPtr+strlen("addtrueorb")+1 ) );
		return;
	}

	pFindPtr = ( char * ) strstr(pString, "missionsuccess");
	if(pFindPtr)
	{
		g_pFcRealMovie->TerminateRealMove();
		CFcWorld::GetInstance().SetMissionFinishType(MISSION_FINISH_TYPE_SUCCESS);

		//예외 처리 cheat
		if(g_FCGameData.nPlayerType == MISSION_TYPE_ASPHARR &&
			g_FCGameData.SelStageId == STAGE_ID_EAURVARRIA)
		{
			g_FCGameData.nSpecialSelStageState = SPECIAL_STAGE_MENU_OFF;
			g_FCGameData.SpeaialSelStageId = STAGE_ID_WYANDEEK;
		}

		return;
	}

	pFindPtr = ( char * ) strstr(pString, "missionfail");
	if(pFindPtr)
	{
		g_pFcRealMovie->TerminateRealMove();
		CFcWorld::GetInstance().SetMissionFinishType(MISSION_FINISH_TYPE_FAIL);
		return;
	}

	pFindPtr = ( char * ) strstr(pString, "god");
	if(pFindPtr)
	{
		if( atoi( pFindPtr+strlen("god")+1 ) == 0 ) g_FCGameData.bGod = false;
		else g_FCGameData.bGod = true;
		return;
	}

	pFindPtr = ( char * ) strstr(pString, "menushow");
	if(pFindPtr)
	{
		if( atoi( pFindPtr+strlen("menushow")+1 ) == 0 ) g_FCGameData.bMenuShow = false;
		else g_FCGameData.bMenuShow = true;
		return;
	}

	pFindPtr = ( char * ) strstr(pString, "dofenable");
	if(pFindPtr)
	{
		g_BsKernel.EnableDOF( atoi(pFindPtr+strlen("dofenable")+1) );
		return;
	}

	pFindPtr = ( char * ) strstr(pString, "doffocus");
	if(pFindPtr)
	{
		g_BsKernel.SetDOFFocus( (float)atof(pFindPtr+strlen("doffocus")+1) );
		return;
	}

	pFindPtr = ( char * ) strstr(pString, "dofrange");
	if(pFindPtr)
	{
		g_BsKernel.SetDOFFocusRange( (float)atof(pFindPtr+strlen("dofrange")+1) );
		return;
	}

	pFindPtr = ( char * ) strstr(pString, "dofnear");
	if(pFindPtr)
	{
		g_BsKernel.SetDOFFocusNear( (float)atof(pFindPtr+strlen("dofnear")+1) );
		return;
	}

	pFindPtr = ( char * ) strstr(pString, "doffar");
	if(pFindPtr)
	{
		g_BsKernel.SetDOFFocusFar( (float)atof(pFindPtr+strlen("doffar")+1) );
		return;
	}

	pFindPtr = ( char * ) strstr(pString, "dofoutfar");
	if(pFindPtr)
	{
		g_BsKernel.SetDOFFocusOutFar( (float)atof(pFindPtr+strlen("dofoutfar")+1) );
		return;
	}

	pFindPtr = ( char * ) strstr(pString, "dofoutnear");
	if(pFindPtr)
	{
		g_BsKernel.SetDOFFocusOutNear( (float)atof(pFindPtr+strlen("dofoutnear")+1) );
		return;
	}

	pFindPtr = ( char * ) strstr(pString, "dofcheck");
	if(pFindPtr)
	{
		char str[256];
		float fFocus = g_BsKernel.GetDOFFocus();
		float fNear  = fFocus-g_BsKernel.GetDOFFocusNear();
		float fFar	 = fFocus+g_BsKernel.GetDOFFocusFar();
		float fOutNear = g_BsKernel.GetDOFFocusOutNear();
		float fOutFar = g_BsKernel.GetDOFFocusOutFar();
		sprintf(str,"Focus %f, NearPlane %f, FarPlane %f\n FocusOut Far %f FocusOut Near %f\n",
			fFocus, fNear, fFar, fOutNear, fOutFar);

		g_BsKernel.AddConsoleString( str );
		return;
	}

	

	pFindPtr = ( char * ) strstr(pString, "debugrenderbaseobject");
	if(pFindPtr)
	{
		if( atoi( pFindPtr+strlen("debugrenderbaseobject")+1 ) == 0 ) g_FCGameData.bDebugRenderBaseObject = false;
		else g_FCGameData.bDebugRenderBaseObject = true;
		return;
	}

	pFindPtr = ( char * ) strstr(pString, "debugrendertroopobject");
	if(pFindPtr)
	{
		if( atoi( pFindPtr+strlen("debugrendertroopobject")+1 ) == 0 ) g_FCGameData.bDebugRenderTroopObject = false;
		else g_FCGameData.bDebugRenderTroopObject = true;
		return;
	}

	pFindPtr = ( char * ) strstr(pString, "saveset");
	if(pFindPtr)
	{
		int nIndex = atoi( pFindPtr+strlen("saveset")+1 );
		g_FCGameData.SaveSet( nIndex );
		return;
	}

	pFindPtr = ( char * ) strstr(pString, "loadset");
	if(pFindPtr)
	{
		int nIndex = atoi( pFindPtr+strlen("loadset")+1 );
		g_FCGameData.LoadSet( nIndex );
		return;
	}

	pFindPtr = ( char * ) strstr(pString, "OrbsparkFire");
	if(pFindPtr)
	{
		CFcWorld::GetInstance().SetAbility( FC_ABILITY_FIRE, CFcWorld::GetInstance().GetHeroHandle() );
		return;
	}

	pFindPtr = ( char * ) strstr(pString, "playmovie");
	if(pFindPtr)
	{
		if( atoi( pFindPtr+strlen("playmovie")+1 ) == 0 ) g_FCGameData.bPlayMovie = false;
		else g_FCGameData.bPlayMovie = true;
		
		return;
	}

	pFindPtr = ( char * ) strstr(pString, "showvariable");
	if(pFindPtr)
	{
		if( atoi( pFindPtr+strlen("showvariable")+1 ) == 0 ) g_FCGameData.bShowVariable = false;
		else g_FCGameData.bShowVariable = true;

		return;
	}
	pFindPtr = ( char * ) strstr(pString, "oneshotmode");
	if(pFindPtr)
	{
		if( atoi( pFindPtr+strlen("oneshotmode")+1 ) == 0 ) g_FCGameData.bOneShotMode = false;
		else g_FCGameData.bOneShotMode = true;

		return;
	}

	pFindPtr = ( char * ) strstr(pString, "showparts");
	if(pFindPtr)
	{
		if( atoi( pFindPtr+strlen("showparts")+1 ) == 0 ) CFcBaseObject::ShowAllObjectParts( false );
		else CFcBaseObject::ShowAllObjectParts( true );

		return;
	}

	pFindPtr = ( char * ) strstr(pString, "showshadow");
	if(pFindPtr)
	{
		if( atoi( pFindPtr+strlen("showshadow")+1 ) == 0 ) g_BsKernel.ShowShadow( false );
		else g_BsKernel.ShowShadow( true );

		return;
	}

	pFindPtr = ( char * ) strstr(pString, "skipmode");
	if(pFindPtr)
	{
		g_FCGameData.nSkipMode = atoi( pFindPtr + strlen( "skipmode" ) + 1 );
		return;
	}

	pFindPtr = ( char * ) strstr(pString, "reloadfx");
	if(pFindPtr)
	{
		int nIndex = g_FcWorld.GetSimpleFXPlayIndex();
		if( nIndex == -1 ) return;
		g_pFcFXManager->SendMessage( nIndex, FX_INIT_OBJECT );
		return;
	}

	pFindPtr = ( char * ) strstr(pString, "abillight");
	if(pFindPtr)
	{
		CFcWorld::GetInstance().SetAbility(FC_ABILITY_LIGHT_1,CFcWorld::GetInstance().GetHeroHandle());
		//( atoi( pFindPtr+strlen("shotl")+1 ) );
		return;
	}
	pFindPtr = ( char * ) strstr(pString, "capture");
	if(pFindPtr)
	{
		g_FCGameData.nCapture = atoi( pFindPtr+strlen("capture")+1 );
		return;
	}

	pFindPtr = ( char * ) strstr(pString, "billboardenable");
	if(pFindPtr)
	{
		if( atoi( pFindPtr+strlen("billboardenable")+1 ) == 0 ) 
			g_BsKernel.GetInstance().EnableBillboard(false);
		else 
			g_BsKernel.GetInstance().EnableBillboard(true);
		return;
	}

	pFindPtr = ( char * ) strstr(pString, "instancing");
	if(pFindPtr)
	{
		if( atoi( pFindPtr+strlen("instancing")+1 ) == 0 ) 
			g_BsKernel.GetInstance().EnableInstancing(false);
		else 
			g_BsKernel.GetInstance().EnableInstancing(true);
		return;
	}

	pFindPtr = ( char * ) strstr(pString, "pointlight");
	if(pFindPtr)
	{		
		return;
	}

	pFindPtr = ( char * ) strstr(pString, "setcam");
	if(pFindPtr)
	{
		CameraObjHandle CamHandle = CFcBaseObject::GetCameraObjectHandle( 0 );
		if( atoi( pFindPtr+strlen("setcam")+1 ) == 0 ) 
			CamHandle->SetDevCam( true );
		else 
			CamHandle->SetDevCam( false );
		return;
	}

	pFindPtr = ( char * ) strstr(pString, "setfullhp");
	if(pFindPtr)
	{
		GameObjHandle hHero = CFcWorld::GetInstance().GetHeroHandle();
		hHero->SetHP( hHero->GetMaxHP() );
		return;
	}

	pFindPtr = ( char * ) strstr(pString, "objectbillboardrange" );
	if(pFindPtr)
	{
		g_BsKernel.GetInstance().SetBillboardRange((float)atof(pFindPtr+strlen("objectbillboardrange")+1));
		return;
	}

	pFindPtr = ( char * ) strstr(pString, "showminimapallobj" );
	if(pFindPtr)
	{
		if( atoi( pFindPtr+strlen("showminimapallobj")+1 ) == 0 ) 
			g_FCGameData.bShowMinimapAllObj = false;
		else 
			g_FCGameData.bShowMinimapAllObj = true;
		return;
	}

#ifdef _XBOX
	pFindPtr = ( char * ) strstr(pString, "quit");
	if(pFindPtr)
	{
//		g_FCGameData.bQuit = true;
		XLaunchNewImage( "", 0 );
		return;
	}
#endif //_XBOX

	pFindPtr = ( char * ) strstr(pString, "rmplay" );
	if(pFindPtr)
	{
		char cStr[256] = "";
		int nStrLen = strlen( pFindPtr+strlen("rmplay")+1 );
		if( nStrLen < 256 )
		{
			strcpy( cStr, pFindPtr+strlen("rmplay")+1 );
		}
		g_pFcRealMovie->PlayDirect( (char*)cStr );			// test
		g_pFcRealMovie->SetConsoleFlag();
		return;
	}

	pFindPtr = ( char * ) strstr(pString, "showorbs");
	if(pFindPtr)
	{	
		if( atoi( pFindPtr+strlen("showorbs")+1 ) == 0 ) 
			g_FCGameData.bShowOrbs = false;
		else 
			g_FCGameData.bShowOrbs = true;
		return;
	}

	pFindPtr = ( char * ) strstr(pString, "addexp");
	if(pFindPtr)
	{	
		int nAddExp = atoi( pFindPtr+strlen("addexp")+1 );
		GameObjHandle hHero = CFcWorld::GetInstance().GetHeroHandle();
		hHero->AddExp( nAddExp, hHero );

		return;
	}

	pFindPtr = ( char * ) strstr(pString, "nodamage");
	if(pFindPtr)
	{	
		if( atoi( pFindPtr+strlen("nodamage")+1 ) == 0 ) 
			g_FCGameData.bNoDamage = false;
		else g_FCGameData.bNoDamage = true;

		return;
	}
	pFindPtr = ( char * ) strstr(pString, "drawprop");
	if(pFindPtr)
	{
		if( atoi( pFindPtr+strlen("drawprop")+1 ) == 0 ) 
			g_FCGameData.bDrawProp = false;
		else g_FCGameData.bDrawProp = true;

		return;
	}
	pFindPtr = ( char * ) strstr(pString, "dropitem");
	if(pFindPtr)
	{
		CCrossVector *pCross;
		D3DXVECTOR3 Pos;

		pCross = CFcWorld::GetInstance().GetHeroHandle()->GetCrossVector();
		Pos = pCross->m_PosVector;
		Pos += pCross->m_ZVector * 200.0f;
		g_FcItemManager.CreateItemToWorld( atoi( pFindPtr+strlen("dropitem")+1 ), Pos );

		return;
	}
#ifdef _LOAD_MAP_CHECK_
	pFindPtr = ( char * ) strstr(pString, "loadmemcheck");
	if(pFindPtr)
	{
		g_bMemCheckPrint = true;
		return;
	}
#endif

	pFindPtr = ( char * ) strstr(pString, "clipupdate");
	if(pFindPtr)
	{
		if( atoi( pFindPtr+strlen("clipupdate")+1 ) == 0 )	g_BsClipTest.SetClipUpdate( false );
		else g_BsClipTest.SetClipUpdate( true );

		return;
	}

	pFindPtr = ( char * ) strstr(pString, "reloadmaterial");
	if(pFindPtr)
	{
		KERNEL_COMMAND cmd;
		cmd.nCommand = KERNEL_COMMAND_RELOAD_MATERIAL;
		g_BsKernel.GetKernelCommand().push_back(cmd);
		return;
	}

	pFindPtr = ( char * ) strstr(pString, "showheropos");
	if(pFindPtr)
	{
		if( atoi( pFindPtr+strlen("showheropos")+1 ) == 0 ) g_FCGameData.bShowHeroPos = false;
		else g_FCGameData.bShowHeroPos = true;

		return;
	}

	pFindPtr = ( char * ) strstr(pString, "setdie");
	if(pFindPtr)
	{
		g_FcWorld.GetHeroHandle()->SetDie();
		return;
	}

	pFindPtr = ( char * ) strstr(pString, "qqq");
	if(pFindPtr)
	{
		g_FCGameData.bGod = true;
		g_FCGameData.bOneShotMode = true;
		GameObjHandle hHero = CFcWorld::GetInstance().GetHeroHandle();
		hHero->AddExp( 250000, hHero );
		CFcWorld::GetInstance().GetHeroHandle()->AddOrbSpark(30000);
		CFcWorld::GetInstance().GetHeroHandle()->AddTrueOrbSpark(30000);

		return;
	}

	pFindPtr = ( char * ) strstr(pString, "www");
	if(pFindPtr)
	{
		g_FCGameData.bGod = false;
		g_FCGameData.bOneShotMode = false;

		return;
	}

	pFindPtr = ( char * ) strstr(pString, "charallopen");
	if(pFindPtr)
	{
		if( atoi( pFindPtr+strlen("charallopen")+1 ) == 0 ) g_FCGameData.bCharAllOpen = false;
		else g_FCGameData.bCharAllOpen = true;

		return;
	}

	pFindPtr = ( char * ) strstr(pString, "lightar");
	if(pFindPtr) {
		float fChannel = (float)atof( pFindPtr+strlen("lightar")+1 );
		D3DXVECTOR4 vecColor = *g_BsKernel.GetLightAmbient();
		vecColor.x = fChannel;
		g_BsKernel.SendMessage(g_BsKernel.GetDirectionLightIndex(), BS_SET_LIGHT_AMBIENT, DWORD(&vecColor));
		return;
	}
	pFindPtr = ( char * ) strstr(pString, "lightag");
	if(pFindPtr) {
		float fChannel = (float)atof( pFindPtr+strlen("lightag")+1 );
		D3DXVECTOR4 vecColor = *g_BsKernel.GetLightAmbient();
		vecColor.y = fChannel;
		g_BsKernel.SendMessage(g_BsKernel.GetDirectionLightIndex(), BS_SET_LIGHT_AMBIENT, DWORD(&vecColor));
		return;
	}
	pFindPtr = ( char * ) strstr(pString, "lightab");
	if(pFindPtr) {
		float fChannel = (float)atof( pFindPtr+strlen("lightab")+1 );
		D3DXVECTOR4 vecColor = *g_BsKernel.GetLightAmbient();
		vecColor.z = fChannel;
		g_BsKernel.SendMessage(g_BsKernel.GetDirectionLightIndex(), BS_SET_LIGHT_AMBIENT, DWORD(&vecColor));
		return;
	}
	
	pFindPtr = ( char * ) strstr(pString, "lightdr");
	if(pFindPtr) {
		float fChannel = (float)atof( pFindPtr+strlen("lightdr")+1 );
		D3DXVECTOR4 vecColor = *g_BsKernel.GetLightDiffuse();
		vecColor.x = fChannel;
		g_BsKernel.SendMessage(g_BsKernel.GetDirectionLightIndex(), BS_SET_LIGHT_DIFFUSE, DWORD(&vecColor));
		return;
	}
	pFindPtr = ( char * ) strstr(pString, "lightdg");
	if(pFindPtr) {
		float fChannel = (float)atof( pFindPtr+strlen("lightdg")+1 );
		D3DXVECTOR4 vecColor = *g_BsKernel.GetLightDiffuse();
		vecColor.y = fChannel;
		g_BsKernel.SendMessage(g_BsKernel.GetDirectionLightIndex(), BS_SET_LIGHT_DIFFUSE, DWORD(&vecColor));
		return;
	}
	pFindPtr = ( char * ) strstr(pString, "lightdb");
	if(pFindPtr) {
		float fChannel = (float)atof( pFindPtr+strlen("lightdb")+1 );
		D3DXVECTOR4 vecColor = *g_BsKernel.GetLightDiffuse();
		vecColor.z = fChannel;
		g_BsKernel.SendMessage(g_BsKernel.GetDirectionLightIndex(), BS_SET_LIGHT_DIFFUSE, DWORD(&vecColor));
		return;
	}

	pFindPtr = ( char * ) strstr(pString, "lightsr");
	if(pFindPtr) {
		float fChannel = (float)atof( pFindPtr+strlen("lightsr")+1 );
		D3DXVECTOR4 vecColor = *g_BsKernel.GetLightSpecular();
		vecColor.x = fChannel;
		g_BsKernel.SendMessage(g_BsKernel.GetDirectionLightIndex(), BS_SET_LIGHT_SPECULAR, DWORD(&vecColor));
		return;
	}
	pFindPtr = ( char * ) strstr(pString, "lightsg");
	if(pFindPtr) {
		float fChannel = (float)atof( pFindPtr+strlen("lightsg")+1 );
		D3DXVECTOR4 vecColor = *g_BsKernel.GetLightSpecular();
		vecColor.y = fChannel;
		g_BsKernel.SendMessage(g_BsKernel.GetDirectionLightIndex(), BS_SET_LIGHT_SPECULAR, DWORD(&vecColor));
		return;
	}
	pFindPtr = ( char * ) strstr(pString, "lightsb");
	if(pFindPtr) {
		float fChannel = (float)atof( pFindPtr+strlen("lightsb")+1 );
		D3DXVECTOR4 vecColor = *g_BsKernel.GetLightSpecular();
		vecColor.z = fChannel;
		g_BsKernel.SendMessage(g_BsKernel.GetDirectionLightIndex(), BS_SET_LIGHT_SPECULAR, DWORD(&vecColor));
		return;
	}

	pFindPtr = ( char * ) strstr(pString, "sceneorigin");
	if(pFindPtr) {
		float fChannel = (float)atof( pFindPtr+strlen("sceneorigin")+1 );
		g_BsKernel.GetImageProcess()->SetSceneIntensity(fChannel);
		return;
	}
	pFindPtr = ( char * ) strstr(pString, "sceneblur");
	if(pFindPtr) {
		float fChannel = (float)atof( pFindPtr+strlen("sceneblur")+1 );
		g_BsKernel.GetImageProcess()->SetBlurIntensity(fChannel);
		return;
	}
	pFindPtr = ( char * ) strstr(pString, "sceneglow");
	if(pFindPtr) {
		float fChannel = (float)atof( pFindPtr+strlen("sceneglow")+1 );
		g_BsKernel.GetImageProcess()->SetGlowIntensity(fChannel);
		return;
	}
	pFindPtr = ( char * ) strstr(pString, "scenethreshold");
	if(pFindPtr) {
		float fChannel = (float)atof( pFindPtr+strlen("scenethreshold")+1 );
		g_BsKernel.GetImageProcess()->SetHighlightThreshold(fChannel);
		return;
	}

	pFindPtr = (char*) strstr( pString , "ooo" );
	if( pFindPtr )
	{
		g_InterfaceManager.GetInstance().SetFadeEffect( true  , 0 );
		return;
	}

	pFindPtr = (char*) strstr( pString , "sii" );
	if( pFindPtr )
	{		
		bool bShow = ( atoi( pFindPtr+strlen("sii")+1 ) != 0 );
		CBsKernel::GetInstance().ShowFPS( !bShow,100,100);
		CBsKernel::GetInstance().ShowDebugStr( bShow );
		g_FcItemManager.SetDrawItemInfo( bShow );
		return;
		
	}

	pFindPtr = (char*) strstr( pString , "crashall" );
	if( pFindPtr )
	{		
		bool bEnable = ( atoi( pFindPtr+strlen("crashall")+1 ) != 0 );
		g_FCGameData.bCrashAll = bEnable;
		return;
	}

	pFindPtr = ( char * ) strstr(pString, "setfps");
	if(pFindPtr) {
		int nFps = (int)atoi( pFindPtr+strlen("setfps")+1 );
		g_FC.GetSkipper()->SetFrame(nFps);
		return;
	}

	pFindPtr = ( char * ) strstr(pString, "showsafearea");
	if(pFindPtr)
	{
		if( atoi( pFindPtr+strlen("showsafearea")+1 ) == 0 ) g_FCGameData.bShowSafeArea = false;
		else g_FCGameData.bShowSafeArea = true;

		return;
	}

	pFindPtr = ( char * ) strstr(pString, "showattr");
	if(pFindPtr)
	{
		if( atoi( pFindPtr+strlen("showattr")+1 ) == 0 ) g_FCGameData.bShowAttr = false;
		else g_FCGameData.bShowAttr = true;
		return;
	}

	pFindPtr = ( char * ) strstr(pString, "addbonus");
	if(pFindPtr)
	{	
		int nAddPoint = atoi( pFindPtr+strlen("addbonus")+1 );
		g_FCGameData.tempFixedSave.nTotalPoint += nAddPoint;
		return;
	}
		
	pFindPtr = ( char * ) strstr(pString, "showaistate");
	if( pFindPtr ) 
	{
		char szStr[512];
		sprintf( szStr, "Total : %d, E/D : ( %d/%d )\n", CFcAIObject::s_ObjectMng.Size(), CFcAIObject::s_pVecEnableList.size(), CFcAIObject::s_pVecDisableList.size() );
		g_BsKernel.AddConsoleString( szStr );
		return;
	}

	pFindPtr = ( char * ) strstr(pString, "setheropos");
	if( pFindPtr ) 
	{
		int x=100,y=100;
		sscanf( pFindPtr+strlen("setheropos")+1, "%d %d",  &x, &y);
		g_FcWorld.GetHeroHandle()->GetCrossVector()->SetPosition( D3DXVECTOR3(x * 100.f, 0, y*100.f) );
		return;
	}

	pFindPtr = ( char * ) strstr(pString, "setrank");
	if( pFindPtr ) 
	{
		g_FCGameData.bCheatRank = true;
		int nRank = atoi( pFindPtr+strlen("setrank")+1 );
		if( nRank < 0 )			nRank = 0;
		if( nRank > 5 )			nRank = 5;
		g_FCGameData.stageInfo.nPlayGrade = nRank;
		
		return;
	}

	

/*
	pFindPtr = ( char * ) strstr(pString, "liballopen");
	if(pFindPtr)
	{
		if( atoi( pFindPtr+strlen("liballopen")+1 ) == 0 ) g_FCGameData.bLibAllOpen = false;
		else g_FCGameData.bLibAllOpen = true;

		return;
	}
*/
	g_BsKernel.AddConsoleString( "Invalid Console String" );
}

void AddConsoleString( const char *pString )
{
	char cTemp[ 1024 ];

	strcpy( cTemp, pString );
	strcat( cTemp, "_" );
	g_BsKernel.AddConsoleString( cTemp );
}

void ChangeConsoleString( const char *pString )
{
	char cTemp[ 1024 ];

	strcpy( cTemp, pString );
	strcat( cTemp, "_" );
	g_BsKernel.ChangeConsoleString( cTemp );
}

void InputKeyboard(unsigned char cInput)
{
	bool bRender;
	int nStrlen;

	bRender=g_BsKernel.IsShowConsole();
	if( cInput == '`' )
	{
		bRender = !bRender;
		g_BsKernel.SetShowConsole( bRender );
		if( bRender )
		{
			AddConsoleString( g_szConsoleString );
		}
		else{
			if(strlen(g_szConsoleString)<=1){
				g_BsKernel.DeleteConsoleString();
			}
			else{
				g_BsKernel.ChangeConsoleString(g_szConsoleString);
			}
			g_szConsoleString[ 0 ] = 0;
		}
		g_nSearchCommand = -1;
		g_szSearchString[ 0 ] = 0;

		return;
	}
	else if( cInput == VK_TAB )
	{
		int nLoopCount;
		char *pFindPtr;

		nLoopCount = 0;
		pFindPtr = NULL;
		while( 1 )
		{
			g_nSearchCommand++;
			int nConsoleCommandCount = sizeof(g_szConsoleCommand) / sizeof(g_szConsoleCommand[0]);
			if( g_nSearchCommand >= nConsoleCommandCount )
			{
				g_nSearchCommand = 0;
			}
			if( strlen( g_szSearchString ) )
			{
				pFindPtr = strstr( g_szConsoleCommand[ g_nSearchCommand ], g_szSearchString );
			}
			else
			{
				pFindPtr = strstr( g_szConsoleCommand[ g_nSearchCommand ], g_szConsoleString );
			}
			if( pFindPtr == g_szConsoleCommand[ g_nSearchCommand ] )
			{
				if( g_szSearchString[ 0 ] == 0 )
				{
					strcpy( g_szSearchString, g_szConsoleString );
				}
				strcpy( g_szConsoleString, g_szConsoleCommand[ g_nSearchCommand ] );
				ChangeConsoleString( g_szConsoleString );
				return;
			}
			nLoopCount++;
			if( nLoopCount >= nConsoleCommandCount )
			{
				return;
			}
		}
	}
	if(!bRender){
		return;
	}
	switch(cInput){
		case 8:		// Backspace
			nStrlen = strlen( g_szConsoleString );
			if( nStrlen )
			{
				g_szConsoleString[ nStrlen - 1 ] = 0;
				ChangeConsoleString( g_szConsoleString );
				g_szSearchString[ 0 ] = 0;
			}
			break;
#ifdef _XBOX
		case 10:	// Enter
#else
		case 13:	// Enter
#endif
			g_BsKernel.ChangeConsoleString( g_szConsoleString );
			ProcessConsoleString( g_szConsoleString );
			g_szConsoleString[ 0 ] = 0;
			AddConsoleString( g_szConsoleString );
			g_nSearchCommand = -1;
			g_szSearchString[ 0 ] = 0;
			break;
		default:
			if( g_BsKernel.IsShowConsole() )
			{
				nStrlen = strlen( g_szConsoleString );
				g_szConsoleString[ nStrlen ] = cInput;
				g_szConsoleString[ nStrlen + 1 ] = 0;
				ChangeConsoleString(g_szConsoleString);
				g_szSearchString[ 0 ] = 0;
			}
			break;
	}
}


void ToggleShowConsole()
{
	bool bRender=g_BsKernel.IsShowConsole();
	if(bRender)
	{
		g_BsKernel.SetShowConsole(bRender);
	}
}

#else

void ProcessConsoleString(const char *pString) {}
void InputKeyboard(unsigned char cInput) {}
void ToggleShowConsole() {}

#endif