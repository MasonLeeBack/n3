// FantasyCore.cpp : Defines the entry point for the application.
//
#include "stdafx.h"
#include "FantasyCore.h"


#include "BsKernel.h"
#include "BsMem.h"
#include "BsMemTracker.h"
#include "BsBlockAllocator.h"
#include "BSFileIO.h"

#include "Skip.h"
#include "InputPad.h"
#include "BStreamExt.h"
#include "Parser.h"
#include "PerfCheck.h"

#include "FcSOXLoader.h"
#include "FcWorld.h"
#include "FcBaseObject.h"
#include "FcItem.h"
#include "FcAniObject.h"
#include "FcHeroObject.h"
#include "FcCameraObject.h"
#include "SmartPtr.h"
#include "FcAIObject.h"
#include "FcAIHardCodingFunction.h"
#include "FcDefines.h"
#include "FcSoundManager.h"
#include "FcTroopObject.h"
#include "FcHitMarkMgr.h"

#include "FcConsole.h"

#include "BsUIBase.h"
#include "BSuiSystem.h"
#include "BSuiGDISkin.h"
#include "BsUiGDIView.h"

#include "FcLiquidObject.h"
#include "FcTroopAIObject.h"
#include "FcGlobal.h"
#include "FcInterfaceManager.h"
#include "FcMenuDef.h"
#include "FcFXTidalWave.h"
#include "FcPhysicsLoader.h"
#include "FcRealtimeMovie.h"
#include "FCFxManager.h"
#include "BsMoviePlayer.h"
#include "n3_orb.h"
#include "FcAchievement.h"
#include "FcLiveManager.h"

#include "LocalLanguage.h"
#include "TextTable.h"
#include "FcMessageDef.h"


//--------------------------------------------------------------------------------------
// Title ID used by Nintey Nine Nights.
//--------------------------------------------------------------------------------------
#ifdef _XBOX
#include <xtitleidbegin.h>
XEX_TITLE_ID(0x4D5307DB)
#include <xtitleidend.h>
#include "N3GameConfig.spa.h"
#endif // #ifdef _XBOX

//#define DUMP_MEMORY_INFO

#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG

//#define _START_STAGE
//#define _START_STAGE_NAME		"PropTest2.bsmap"
//#define _START_STAGE_NAME		"TotalTest2.bsmap"
//#define _START_STAGE_NAME		"Vz_My0.bsmap"		// 부대 가다 서다 하는 문제
//#define _START_STAGE_NAME		"TGS(0908).bsmap"
//#define _START_STAGE_NAME		"VZ-AS.bsmap"
//#define _START_STAGE_NAME		"EV-AS.bsmap"
//#define _START_STAGE_NAME		"test1.bsmap"

HANDLE g_hPackedResTitle = NULL;
HANDLE g_hPackedTexTitle = NULL;
HANDLE g_hPortraitCache = NULL;
HANDLE g_hPackedRes1 = NULL;
HANDLE g_hPackedTex1 = NULL;
HANDLE g_hPackedRes3 = NULL;
HANDLE g_hPackedTex3 = NULL;
HANDLE g_hPackedTex2 = NULL;
HANDLE g_hPackedCharTex = NULL;
HANDLE g_hPackedWeaponTex = NULL;
HANDLE g_hPackedWeaponRes = NULL;


#ifdef MAKE_ALL_MISSION_RESLIST
#define ALL_MAP_NUM		36

int g_nCurMapListIndex = 0;

char g_MapList[ALL_MAP_NUM][32] =
{
	{ "ev_as.bsmap" },
	{ "ev_in.bsmap" },
	{ "ev_ty.bsmap" },
	{ "he_my.bsmap" },
	{ "ma_in.bsmap" },
	{ "ph_as.bsmap" },	// 5
	{ "ph_dw.bsmap" },
	{ "ph_in.bsmap" },
	{ "ph_kl.bsmap" },
	{ "ph_my.bsmap" },
	{ "ph_ty.bsmap" },	// 10
	{ "ph_vi.bsmap" },
	{ "vf_as.bsmap" },
	{ "vf_in2.bsmap" },
	{ "vf_in.bsmap" },
	{ "vg_dw.bsmap" },	// 15
	{ "vg_kl.bsmap" },
	{ "vg_my.bsmap" },
	{ "vg_ty.bsmap" },
	{ "vg_vi.bsmap" },
	{ "vz_as2.bsmap" },	// 20
	{ "vz_dw2.bsmap" },
	{ "vz_dw.bsmap" },
	{ "vz_my0.bsmap" },
	{ "vz_my.bsmap" },
	{ "wy_as.bsmap" },	// 25
	{ "wy_in.bsmap" },
	{ "wy_kl.bsmap" },
	{ "wy_ty.bsmap" },
	{ "yw_as.bsmap" },
	{ "yw_in.bsmap" },	// 30
	{ "awed.bsmap" },
	{ "CED01.bsmap" },
	{ "CEM.bsmap" },
	{ "dm_mp.bsmap" },
	{ "PFVED.bsmap" },	// 35

};

#endif

#ifndef _XBOX
#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name
HWND				g_hWnd=NULL;

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
#endif

CBsKernel			g_Kernel;
FcInterfaceManager	g_InterfaceManager;
CFcFXManager*		g_pFcFXManager = NULL;
CInputPad			g_InputPad;


CUnitSOXLoader			g_UnitSOX;
CLevelTableLoader		g_LevelTableSOX;
CHeroLevelTableLoader	g_HeroLevelTableSOX;
CTroopSOXLoader			g_TroopSOX;
CLibraryTableLoader		g_LibTableSox;
CLibCharLocalLoader		g_LibCharLocalSox;

CFantasyCore		g_FC;
CFcWorld			g_FcWorld;
CBsMoviePlayer      g_BsMoviePlayer;
CFcItemManager		g_FcItemManager;
CFcHitMarkMgr		g_FcHitMarkManager;

extern bool g_bPause;
extern int	g_nPauseRef;
extern LARGE_INTEGER	g_liStartPauseTime;
extern LARGE_INTEGER	g_liStagePauseTime;
bool g_bGotoTitle = true;

// Thread 관련 함수 & 전역 변수
#ifdef _XBOX
#define USE_RENDER_THREAD
#endif
DWORD WINAPI RenderThreadProc( LPVOID lpParameter );
HANDLE g_hRenderThread = 0;
DWORD  g_dwRenderThreadId = 0;
HANDLE g_hEndStageEvent = 0;
HANDLE g_hQuitRenderEvent = 0;
HANDLE g_hProcessCountSemaphore = 0;
HANDLE g_hReadyCountSemaphore = 0;
HANDLE g_hReleaseEvent = NULL;


HANDLE	CTimeRender::ms_hRenderProgressThread = NULL;
DWORD	CTimeRender::ms_dwRenderProgressThreadID = 0;

#ifdef _XBOX

#pragma optimize("", off)

//------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------

int StaticInitialize()
{
	BsMemTracker::Init();
	BsBlockAllocator::Init();

	return 0;
}

//------------------------------------------------------------------------------------------------------------------------

// Setup the function to call before the static initialization happens.

typedef int (__cdecl *_PIFV)(void);

#pragma data_seg(".CRT$XIB")
static _PIFV p_static_init = StaticInitialize;
#pragma data_seg()

#pragma optimize("", on)

//------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------


HANDLE g_hNewMainThread = 0;
DWORD  g_dwNewMainThreadId = 0;

HANDLE g_hStartupThread = 0;
DWORD  g_dwStartupThreadId = 0;
HANDLE g_hStartupDoneEvent = 0;
HANDLE g_hMovieLoadedEvent = 0;

int g_nCaptureIndex;
int g_nCaptureFolderIndex;


#define PT_VERTEX_FVF ( D3DFVF_XYZ | D3DFVF_TEX1 | D3DFVF_TEXCOORDSIZE2( 0 ) )
#define _FADETIME 5.f
#define _BLANKDELAY 1.f

typedef struct VERTEX_TC 
{
	float sx, sy, sz;  // Screen coordinates
	float tu, tv;      // Texture coordinates 
} VERTEX_TC;


static CONST CHAR szVS_POS_TEX_PASS[] = 
" struct VS_DATA                        "
" {                                     "
"     float4 Pos : POSITION;            "
"     float4 T0: TEXCOORD0;             "
" };                                    "
"                                       "
" VS_DATA main( VS_DATA In )            "
" {                                     "
"     return In;                        "
" }                                     ";

static CONST CHAR szPS_2DTEX[] =
" sampler tex : register( s0 );         "
" float4 Color: register( c0 );         "
"                                       "
" struct VS_OUT                         "
" {                                     "
"     float2 T0: TEXCOORD0;             "
" };                                    "
"                                       "
" float4 main( VS_OUT In ) : COLOR      "
" {                                     "
"   float4 t;                           "
"   float4 p;                           "
"   t   = tex2D( tex, In.T0 );          "
"   p   = Color * t;                    "
"   return p;                           "
" }                                     ";


void FadeInFadeOutTexture( const char *szFileName, D3DVertexShader *pVS, D3DPixelShader *pPS, float fFadeTime)
{
	C3DDevice *pDevice = g_BsKernel.GetDevice();
	if(pDevice ==NULL)
		return;
	pDevice->SaveState();

	LARGE_INTEGER StartTime;
	LARGE_INTEGER CurrentTime;
	LARGE_INTEGER TicksPerSecond;
	float fTicksSecond;
	float fTotalTime = 0.f;

	QueryPerformanceCounter( &StartTime );
	QueryPerformanceFrequency( &TicksPerSecond );
	fTicksSecond = (float)TicksPerSecond.QuadPart;

	// Clear the screen to black immediately
	pDevice->BeginScene();
	pDevice->GetD3DDevice()->Clear(0, 0, D3DCLEAR_TARGET, 0, 1.0f, 0);
	pDevice->EndScene();
	g_BsKernel.GetDevice()->ShowFrame();


	// NOTE: This code bypasses the BsTexture interface as the textures are
	//       only loaded and destroyed from this thread at startup....
	LPDIRECT3DTEXTURE9		pTexture;
	CHAR *					pTexData;
#ifdef _PACKED_RESOURCES
	char szCompressedFileName[MAX_PATH];
	char *pDot;

	// Change the file extension - A hack for now, but it means that the rest of the
	// game can be unaware of compressed files....
	memset(szCompressedFileName, 0, MAX_PATH);
	memcpy(szCompressedFileName, szFileName, strlen(szFileName));
	pDot = strstr(szCompressedFileName, ".");
	if(pDot == NULL)
	{
		return;
	}
	*pDot = NULL; //aleksger - safe string - converting to a strcat with known length
	strcat(szCompressedFileName, ".36t");
	DWORD					dwSize;
	pTexture = (LPDIRECT3DTEXTURE9)pDevice->LoadPackedTexture(szCompressedFileName, &dwSize, &pTexData);
#else
	pTexture = (LPDIRECT3DTEXTURE9)pDevice->CreateTextureFromFile(szFileName, 0, &pTexData );
#endif

	if(pTexture == NULL)
		return;

	D3DSURFACE_DESC d3dsdesc;
	pTexture->GetLevelDesc(0, &d3dsdesc);
	pDevice->SetTexture( 0,  pTexture);
	pDevice->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );
	pDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
	pDevice->SetSamplerState( 0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP );
	pDevice->SetSamplerState( 0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP );
	pDevice->SetSamplerState( 0, D3DSAMP_ADDRESSW, D3DTADDRESS_CLAMP );
#ifndef _BS_PERFORMANCE_CHECK
	pDevice->SetRenderState(D3DRS_PRESENTINTERVAL, D3DPRESENT_INTERVAL_ONE);
#endif
	pDevice->SetRenderState( D3DRS_ZENABLE, FALSE );
	pDevice->SetRenderState( D3DRS_ZWRITEENABLE, FALSE );
	pDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );
	pDevice->SetRenderState( D3DRS_VIEWPORTENABLE, 0 );
	pDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, 1 );
	pDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
	pDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
	pDevice->SetRenderState( D3DRS_ALPHATESTENABLE, 0);

	pDevice->GetD3DDevice()->SetFVF( PT_VERTEX_FVF );
	pDevice->SetVertexShader( pVS );
	pDevice->SetPixelShader( pPS );


	float fX = (pDevice->GetBackBufferWidth() - d3dsdesc.Width) / 2.0f;
	float fY = (pDevice->GetBackBufferHeight() - d3dsdesc.Height) / 2.0f;

	VERTEX_TC vertices[4];

	vertices[0].sx = fX;
	vertices[0].sy = fY;
	vertices[0].sz = 0.0F;
	vertices[0].tu = 0.0f;
	vertices[0].tv = 0.0f;
	vertices[1] = vertices[0];
	vertices[1].sx = fX + (float)d3dsdesc.Width;
	vertices[1].tu = 1.0f;
	vertices[2] = vertices[0];
	vertices[2].sy = fY + (float)d3dsdesc.Height;
	vertices[2].tv = 1.0f;
	vertices[3] = vertices[1];
	vertices[3].sy = vertices[2].sy;
	vertices[3].tv = 1.0f;

	// Pause with a black screen if we got here too fast.. This helps to
	// balance the fade-in.
	QueryPerformanceCounter( &CurrentTime );
	fTotalTime = (CurrentTime.QuadPart - StartTime.QuadPart) / fTicksSecond;
	while(fTotalTime < _BLANKDELAY)
	{
		QueryPerformanceCounter( &CurrentTime );
		fTotalTime = (CurrentTime.QuadPart - StartTime.QuadPart) / fTicksSecond;
	}

	float fColors[4] = {1.f, 1.f, 1.f, 1.f};
	fTotalTime = 0.f;
	QueryPerformanceCounter( &StartTime );

	while(fTotalTime < fFadeTime)
	{
		fColors[3] = sinf( PI * fTotalTime / fFadeTime );
		pDevice->SetPixelShaderConstantF(0, fColors, 1);

		pDevice->BeginScene();
		pDevice->GetD3DDevice()->Clear(0, 0, D3DCLEAR_TARGET, 0, 1.0f, 0);

		g_BsKernel.GetDevice()->DrawPrimitiveUP( D3DPT_TRIANGLESTRIP, 2, &vertices, sizeof(VERTEX_TC) );

		pDevice->EndScene();
		g_BsKernel.GetDevice()->ShowFrame();

		QueryPerformanceCounter( &CurrentTime );
		fTotalTime = (CurrentTime.QuadPart - StartTime.QuadPart) / fTicksSecond;
	}

	pDevice->SetVertexShader( NULL );
	pDevice->SetPixelShader( NULL );
	pDevice->SetTexture( 0,  NULL );
	pTexture->BlockUntilNotBusy();
	delete pTexture;
	XPhysicalFree(pTexData);
	pDevice->RestoreState();
}

//--- tohoshi
#define MOVIE_VOLUME_MGSLOGO					(65536/1)
bool SkipMovieCallback_MGSLOGO(void* pParam)
{
	CBsMoviePlayer* pMoviePlayer = (CBsMoviePlayer*)pParam;
	pMoviePlayer->SetVolume( (int)((g_FCGameData.ConfigInfo.nBGMVolume * 0.1f) * (float)_MAX_BINK_VOLUME) );
	return false;
}
//---

DWORD WINAPI StartupThreadProc( LPVOID lpParameter )
{
	g_BsKernel.GetDevice()->AcquireD3DThreadOwnership();

	char szFilename[_MAX_PATH];

	sprintf_s(szFilename,_countof(szFilename),"d:\\data\\movies\\");
	strcat_s(szFilename, _countof(szFilename), _MGS_LOGO_MOVIE);

	VOID * pMovieData;
	DWORD dwFileSize;
	DWORD dwMemFlags;

	if(FAILED(CBsFileIO::LoadFilePhysicalMemory(szFilename, &pMovieData, &dwFileSize, &dwMemFlags)))
		return E_FAIL;

	SetEvent(g_hMovieLoadedEvent);

	//Bink needs the movie endian swapped - This should really be done off-line
	XGEndianSwapMemory(pMovieData, pMovieData, XGENDIAN_8IN32, sizeof(DWORD), dwFileSize / sizeof(DWORD));
	g_BsMoviePlayer.PlayMovie((const char*)pMovieData, true, SkipMovieCallback_MGSLOGO, true);
	CBsFileIO::FreePhysicalMemory(pMovieData, dwMemFlags);

	D3DVertexShader * pD3DSimpleVS = NULL;
	D3DPixelShader  * pD3DSimplePS = NULL;
	ID3DXBuffer* buffer = 0;

	HRESULT hr = D3DXCompileShader( szVS_POS_TEX_PASS, sizeof( szVS_POS_TEX_PASS ),
		0, 0, "main", "vs_2_0", 0, &buffer, NULL, NULL );
	if ( SUCCEEDED( hr ) )
	{
		hr = g_BsKernel.GetD3DDevice()->CreateVertexShader( (DWORD*) buffer->GetBufferPointer(), &pD3DSimpleVS );
		buffer->Release();
	}

	hr = D3DXCompileShader( szPS_2DTEX, sizeof( szPS_2DTEX ), 0, 0,
		"main", "ps_2_0", 0, &buffer, NULL, NULL );
	if ( SUCCEEDED( hr ) )
	{
		hr = g_BsKernel.GetD3DDevice()->CreatePixelShader( (DWORD*) buffer->GetBufferPointer(), &pD3DSimplePS );
		buffer->Release();
	}

	if(pD3DSimpleVS && pD3DSimplePS)
	{
		sprintf_s(szFilename,_countof(szFilename),"d:\\Data\\interface\\MenuLogo\\");
		strcat_s(szFilename, _countof(szFilename), _PHANTA_Q_LOGO);

		FadeInFadeOutTexture(szFilename, pD3DSimpleVS, pD3DSimplePS, _FADETIME);
	}

	SAFE_RELEASE(pD3DSimpleVS);
	SAFE_RELEASE(pD3DSimplePS);

	g_BsKernel.GetDevice()->ReleaseD3DThreadOwnership();

	SetEvent(g_hStartupDoneEvent);

	return 0x0;
}


DWORD WINAPI NewMainProcThreadProc( LPVOID lpParameter )
{
	g_FC.InitGameFirstTime(false, g_FC.IsFrameSkip());
	g_FC.SetMainState( GAME_STATE_INIT_TITLE, 1, 1);

	//_TNT_
#ifdef _TNT_
#ifdef _TICKET_TRACKER_
	__try
	{
		LoadTicketTrackerSettings();
#endif
#endif

		while(1)
		{
			g_FC.MainGameLoop();
		}

		//_TNT_
#ifdef _TNT_
#ifdef _TICKET_TRACKER_
	}
	__except(TicketTrackerExceptionHandler(GetExceptionCode(),GetExceptionInformation()))
	{}
#endif
#endif

	return 0x0;
}

//일본 .net에서는 command arguments setting이 안되는 관계로
//부득이하게 이렇게 처리한다.(절대 지우지 말것)
//#define _START_DEBUG_MENU

#define _XBOX_FILECACHE_SIZE (1024*512)


int main()
{
	// Note the following must be here to avoid a problem with COMDAT folding and LTCG compiles.

	p_static_init = NULL;

	g_BsMemChecker.Start( "Start main" );

	// Set the presence strings to some known value...
	XUserSetContext( 0, 
		CONTEXT_GAME_RICHPRESENCE_LIST, CONTEXT_GAME_RICHPRESENCE_LIST_TITLESCENE );
	XUserSetContext( 0, 
		CONTEXT_GAME_SECOND_MSG_LIST, CONTEXT_GAME_SECOND_MSG_LIST_TITLEDESC );

	if(strstr( GetCommandLine(), "startdebugmenu")){
		g_bIsStartTitleMenu = FALSE;
		g_FC.CreateLiveManager();
	}

#ifdef _START_DEBUG_MENU
	g_bIsStartTitleMenu = FALSE;
#endif

	g_FC.SetFrameSkip( true );	// 프레임 스킵은 안쓰고 웨이트만 한다..
	/*#ifdef USE_RENDER_THREAD	// thread 쓸때는 frame skip 키면 더 느리다..
	g_FC.SetFrameSkip( false );
	#else
	g_FC.SetFrameSkip( true );
	#endif*/
#ifdef _BS_PERFORMANCE_CHECK
	g_FC.SetFrameSkip( false );
#endif

	XSetFileCacheSize(_XBOX_FILECACHE_SIZE);
	g_hNewMainThread = CreateThread( NULL, 65536*4, NewMainProcThreadProc, NULL, 0, &g_dwNewMainThreadId );
	XSetThreadProcessor( g_hNewMainThread, 2 );

	while(1){ Sleep(0xffffffff); }
	return 1;
}

#else

#define MOVIE_VOLUME_MGSLOGO					(65536/1)
bool SkipMovieCallback_MGSLOGO(void* pParam)
{
	CBsMoviePlayer* pMoviePlayer = (CBsMoviePlayer*)pParam;
	pMoviePlayer->SetVolume((int)((g_FCGameData.ConfigInfo.nBGMVolume * 0.1f) * (float)_MAX_BINK_VOLUME));
	return false;
}

#ifdef _NO_DEBUG_CONSOLE
int APIENTRY _tWinMain(HINSTANCE hInstance,
					   HINSTANCE hPrevInstance,
					   LPTSTR    lpCmdLine,
					   int       nCmdShow)
{
#else
int main(int argc, char* argv[])
{
	HINSTANCE hInstance = GetModuleHandle(NULL);
	LPTSTR lpCmdLine = GetCommandLine();
	int nCmdShow = 1;
#endif
#ifdef _ENABLE_CRT_DEBUG_
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF ); //_CRTDBG_DELAY_FREE_MEM_DF <-요거는 왠만하면 풀지마라..
	//	_CrtSetBreakAlloc( 146346 );
#endif

	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;
	BOOL bGotMsg;
	bool bWindow;

	g_BsMoviePlayer;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_FANTASYCORE, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow)) 
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, (LPCTSTR)IDC_FANTASYCORE);

	bWindow=true;

	g_FC.SetFrameSkip( true );
	if(strstr(lpCmdLine, "/window"))
	{
		bWindow=true;
	}
	if(strstr(lpCmdLine, "/noskip"))
	{
		g_FC.SetFrameSkip( false );
	}

	g_bIsStartTitleMenu = FALSE;
	g_FC.InitGameFirstTime( bWindow, g_FC.IsFrameSkip() );
	g_FC.SetMainState( GAME_STATE_INIT_TITLE, 1,1);

#ifdef _MOVIES_ENABLED
	char szFilename[_MAX_PATH];

	sprintf_s(szFilename, _countof(szFilename), "d:\\data\\movies\\");
	strcat_s(szFilename, _countof(szFilename), _MGS_LOGO_MOVIE);

	VOID* pMovieData;
	DWORD dwFileSize;
	DWORD dwMemFlags;

	if (FAILED(CBsFileIO::LoadFilePhysicalMemory(szFilename, &pMovieData, &dwFileSize, &dwMemFlags)))
		return E_FAIL;

	//SetEvent(g_hMovieLoadedEvent);

	//Bink needs the movie endian swapped - This should really be done off-line
	g_BsMoviePlayer.PlayMovie((const char*)pMovieData, true, SkipMovieCallback_MGSLOGO, true);
	CBsFileIO::FreePhysicalMemory(pMovieData, dwMemFlags);
#endif

	// Main message loop:
	PeekMessage(&msg, NULL, 0U, 0U, PM_NOREMOVE);
	while(WM_QUIT!=msg.message){
		if(CBsKernel::GetInstance().IsActive()){
			bGotMsg=PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE);
		}
		else{
			bGotMsg=GetMessage(&msg, NULL, 0U, 0U);
		}
		if(bGotMsg){
			if(0==TranslateAccelerator(g_hWnd, NULL, &msg)){
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		else{
			if(!g_FC.MainGameLoop()){
				break;
			}
		}
	}

	return (int) msg.wParam;
}
#endif // #ifdef _XBOX



#ifndef _XBOX

ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX); 

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= (WNDPROC)WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, (LPCTSTR)IDI_FANTASYCORE);
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, (LPCTSTR)IDI_SMALL);

	return RegisterClassEx(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	HWND hWnd;

	hInst = hInstance; // Store instance handle in our global variable

#ifdef _DEBUG
	hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, _SCREEN_WIDTH, _SCREEN_HEIGHT, NULL, NULL, hInstance, NULL);
#else
	hWnd = CreateWindow(szWindowClass, szTitle, WS_POPUP,
		0, 0, _SCREEN_WIDTH, _SCREEN_HEIGHT, NULL, NULL, hInstance, NULL);
#endif

	if (!hWnd)
	{
		return FALSE;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);
	g_hWnd=hWnd;

	return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message) {
		case WM_COMMAND:
			wmId    = LOWORD(wParam); 
			wmEvent = HIWORD(wParam); 
			// Parse the menu selections:
			switch (wmId)
			{
			case IDM_ABOUT:
				DialogBox(hInst, (LPCTSTR)IDD_ABOUTBOX, hWnd, (DLGPROC)About);
				break;
			case IDM_EXIT:
				DestroyWindow(hWnd);
				break;
			default:
				return DefWindowProc(hWnd, message, wParam, lParam);
			}
			break;
		case WM_PAINT:
			hdc = BeginPaint(hWnd, &ps);
			// TODO: Add any drawing code here...
			EndPaint(hWnd, &ps);
			break;
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		case WM_CHAR:
			InputKeyboard((char)wParam);
			break;
		case WM_KEYDOWN:
			if( wParam == VK_ESCAPE )
			{
				g_FC.FreeCam();
			}
			else if( wParam == VK_UP )
			{
				g_BsKernel.ScrollConsole( 1 );
			}
			else if( wParam == VK_DOWN )
			{
				g_BsKernel.ScrollConsole( -1 );
			}
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
LRESULT CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG:
		return TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) 
		{
			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;
		}
		break;
	}
	return FALSE;
}
#endif // #ifndef _XBOX


DWORD WINAPI RenderThreadProc( LPVOID lpParameter )
{
#ifdef _XBOX
	g_BsKernel.GetDevice()->AcquireD3DThreadOwnership();
#endif

#ifdef _TNT_
#ifdef _TICKET_TRACKER_
	__try
	{
//		LoadTicketTrackerSettings();
#endif
#endif
		while( 1 )
		{
			WaitForSingleObject( g_hProcessCountSemaphore, INFINITE );		
			if( WaitForSingleObject( g_hEndStageEvent, 0 ) == WAIT_OBJECT_0 )
			{
	#ifdef _XBOX
				DebugString("End Render Thread(%d) : Rest PreKernel Command = %d\n", CBsObject::GetRenderBufferIndex(), g_BsKernel.GetRenderPreKernelCommandCount() );
				DebugString("End Render Thread(%d) : Rest Post Kernel Command = %d\n", CBsObject::GetRenderBufferIndex(), g_BsKernel.GetRenderKernelCommandCount() );
				g_BsKernel.GetDevice()->ReleaseD3DThreadOwnership();
	#endif
				SetEvent( g_hQuitRenderEvent );
				break;
			}
			g_FC.MainRenderProcess();
		}
#ifdef _TNT_
#ifdef _TICKET_TRACKER_
	}
	__except(TicketTrackerExceptionHandler(GetExceptionCode(),GetExceptionInformation()))
	{}
#endif
#endif

	return 1;
}

bool g_bSkipSceneRendering = false;

bool IsSkipSceneRendering() 
{
	return g_bSkipSceneRendering;
}


CTimeRender::CTimeRender( BOOL bUseThread, LONG lPeriod, BOOL bManualReset)
{
	m_bUseThread = bUseThread;
	m_nLogoTexID = -1;
	m_nTexImgID = -1;
	m_nBGImgTexID = -1;
	m_nRandValue = -1;
	
	m_fStartLoadingTime = GetPerformanceTime();

#ifdef _XBOX
	g_FC.SaveLiveContext();
	g_FC.SetLiveContext( LIVE_CONTEXT_MENU , LIVE_CONST_LOADING );
#endif
	
	if( bUseThread )
	{
		//-------------------------------------------------------------------
		char cLogoFileName[256];
		sprintf(cLogoFileName, "%s\\%s_%s.dds",
			g_LocalLanguage.GetLanguageDir(),
			_LOADING_MISSION_LOGO,
			g_LocalLanguage.GetLanguageStr());

		g_BsKernel.chdir("interface");
		g_BsKernel.chdir("Local");
		m_nLogoTexID = g_BsKernel.LoadTexture(cLogoFileName);
		BsAssert(m_nLogoTexID != -1);
		g_BsKernel.chdir("..");
		g_BsKernel.chdir("..");

		//-------------------------------------------------------------------
		char cTextFileName[256];
		sprintf(cTextFileName, "%s\\%s_%s.dds",
			g_LocalLanguage.GetLanguageDir(),
			_LOADING_MISSION_TEXT,
			g_LocalLanguage.GetLanguageStr());

		g_BsKernel.chdir("interface");
		g_BsKernel.chdir("Local");
		m_nTexImgID = g_BsKernel.LoadTexture(cTextFileName);
		BsAssert(m_nTexImgID != -1);
		g_BsKernel.chdir("..");
		g_BsKernel.chdir("..");

		//-------------------------------------------------------------------
		char cFileName[256];
		switch(g_FCGameData.nPlayerType)
		{
		case MISSION_TYPE_INPHYY:		strcpy(cFileName, _LOADING_MISSION_INPHYY); break;
		case MISSION_TYPE_ASPHARR:		strcpy(cFileName, _LOADING_MISSION_ASPHARR); break;
		case MISSION_TYPE_MYIFEE:		strcpy(cFileName, _LOADING_MISSION_MYIFEE); break;
		case MISSION_TYPE_TYURRU:		strcpy(cFileName, _LOADING_MISSION_TYURRU); break;
		case MISSION_TYPE_KLARRANN:		strcpy(cFileName, _LOADING_MISSION_KLARRANN); break;
		case MISSION_TYPE_DWINGVATT:	strcpy(cFileName, _LOADING_MISSION_DWINGVATT); break;
		case MISSION_TYPE_VIGKVAGK:		strcpy(cFileName, _LOADING_MISSION_VIGKVAGK); break;
		default:  strcpy(cFileName, _LOADING_MISSION_INPHYY); break;
		}
		
		m_nRandValue = Random(5) + 1;
		BsAssert(m_nRandValue >= 1 && m_nRandValue <= 5);

		char cFullName[256];
		sprintf(cFullName, "%s_%d.dds", cFileName, m_nRandValue);

		g_BsKernel.chdir("interface");
		g_BsKernel.chdir("Loading");
#ifdef ENABLE_RES_CHECKER
		g_BsResChecker.SkipNextFile();
#endif
		m_nBGImgTexID = g_BsKernel.LoadTexture(cFullName);
		BsAssert(m_nBGImgTexID != -1);
		g_BsKernel.chdir("..");
		g_BsKernel.chdir("..");
	}

	g_bSkipSceneRendering = true;

	m_hEndRequest = CreateEvent( NULL, FALSE, FALSE, NULL );
	m_hEndAcknowledge = CreateEvent( NULL, FALSE, FALSE, NULL );
	ms_hRenderProgressThread = CreateThread( NULL, 65536, RenderProgressThreadProc, this, 0, &ms_dwRenderProgressThreadID );

	//_DebugString( "finish CTimeRender::CTimeRender()\n" );
}

CTimeRender::~CTimeRender()
{
	//_DebugString( "enter CTimeRender::~CTimeRender()\n" );

	SetEvent( m_hEndRequest );
	WaitForSingleObject( m_hEndAcknowledge, INFINITE );

	CloseHandle( m_hEndRequest );
	m_hEndRequest = NULL;
	CloseHandle( m_hEndAcknowledge );
	m_hEndAcknowledge = NULL;
	CloseHandle( ms_hRenderProgressThread );
	ms_hRenderProgressThread = NULL;
	ms_dwRenderProgressThreadID = 0;
	SAFE_RELEASE_TEXTURE(m_nLogoTexID);
	SAFE_RELEASE_TEXTURE(m_nTexImgID);
	SAFE_RELEASE_TEXTURE(m_nBGImgTexID);

	// Render Thread 가 남은작업을 다끝내도록 해준다.
	WaitForSingleObject( g_hReadyCountSemaphore, INFINITE );
	WaitForSingleObject( g_hReadyCountSemaphore, INFINITE );

	g_bSkipSceneRendering	 = false;		
	CBsUIManager::ResetDoubleBuffer();

	 DebugString( " Total Loading Elapsed Time %.1lf \n ", GetPerformanceTime() - m_fStartLoadingTime );

	ReleaseSemaphore( g_hReadyCountSemaphore, 1, NULL );
	ReleaseSemaphore( g_hReadyCountSemaphore, 1, NULL );

	g_FC.RestoreLiveContext();

	//_DebugString( "leave CTimeRender::~CTimeRender()\n" );
}

DWORD WINAPI CTimeRender::RenderProgressThreadProc( LPVOID lpParameter )
{
	CTimeRender*	self = static_cast<CTimeRender*>( lpParameter );
	bool			bLoop = true;
	DWORD			dwStartTime = GetTickCount();

#ifdef _XBOX
	if( !self->m_bUseThread ) {
		g_BsKernel.GetDevice()->AcquireD3DThreadOwnership();	
	}
#endif

	while (bLoop)
	{
		if( self->m_bUseThread )
		{
			// wait for CFantasyCore::MainRenderProcess(); ReleaseSemaphore( g_hReadyCountSemaphore )
			WaitForSingleObject( g_hReadyCountSemaphore, INFINITE );			
		}

		if( WaitForSingleObject( self->m_hEndRequest, 0 ) == WAIT_OBJECT_0 )
		{
			// if detect m_EndRequest event, render blank screen once and exit.
			// ( m_EndRequest is signal from ~CTimeRender() )
			bLoop = false;
		}
		else
		{
			RenderLoading(self, dwStartTime);
		}		
		CBsUIManager::FlipProcessBuffer();

		if( self->m_bUseThread )
		{
			// rendering request signal to RenderThreadProc() 			
			ReleaseSemaphore( g_hProcessCountSemaphore, 1, NULL );			
		}
		else
		{
			g_FC.MainRenderProcess();
		}

	}
	// thread end acknowledge to ~CTimeRender()
	SetEvent( self->m_hEndAcknowledge );

	return 1;
}
// [endmodify] junya


#define _LOADING_LOGO_POS_X		70
#define _LOADING_LOGO_POS_Y		40
#define _LOADING_TEXT_POS_X		1016
#define _LOADING_TEXT_POS_Y		626

void CTimeRender::RenderLoading(CTimeRender* pSelf, DWORD dwStartTime)
{
	float fTime = (float)( GetTickCount()-dwStartTime ) * 0.001f;

	if ( pSelf->m_nBGImgTexID != -1 )
	{
		SIZE size = g_BsKernel.GetTextureSize( pSelf->m_nBGImgTexID );
		g_BsKernel.DrawUIBox_s(_Ui_Mode_Image,
			0, 0, _SCREEN_WIDTH, _SCREEN_HEIGHT,
			0.f, D3DXCOLOR(1.f,1.f,1.f,1.f),
			0.f, pSelf->m_nBGImgTexID,
			0, 0, size.cx, size.cy);
	}

	if ( pSelf->m_nLogoTexID != -1 )
	{
		SIZE size = g_BsKernel.GetTextureSize( pSelf->m_nLogoTexID );
		g_BsKernel.DrawUIBox_s(_Ui_Mode_Image,
			_LOADING_LOGO_POS_X, _LOADING_LOGO_POS_Y,
			size.cx, size.cy,
			0.f, D3DXCOLOR(1.f,1.f,1.f,1.f),
			0.f, pSelf->m_nLogoTexID,
			0, 0, size.cx, size.cy);
	}

	if( pSelf->m_nTexImgID != -1 )
	{
		float fAlpha = sinf(fTime*2.5f-0.8f)*0.5f + 0.5f;
		SIZE size = g_BsKernel.GetTextureSize( pSelf->m_nTexImgID );
		g_BsKernel.DrawUIBox_s(_Ui_Mode_Image,
			_LOADING_TEXT_POS_X, _LOADING_TEXT_POS_Y,
			size.cx, size.cy,
			0.f, D3DXCOLOR(1.f,1.f,1.f,fAlpha),
			0.f, pSelf->m_nTexImgID,
			0, 0, size.cx, size.cy);
	}

	if(pSelf->m_nRandValue > 0)
	{
		g_pFont->DrawUIText(-1, -1, -1, -1, "@(reset)@(scale=0.6,0.6)@(align=2)");
		int nMissinoType = (g_FCGameData.nPlayerType == -1) ? MISSION_TYPE_INPHYY : g_FCGameData.nPlayerType;
		int nLV1 = g_LoadingLV[nMissinoType][pSelf->m_nRandValue-1][0];
		int nLV2 = g_LoadingLV[nMissinoType][pSelf->m_nRandValue-1][1];

		char cLevel[256];
		g_TextTable->GetText(_TEX_SUM_LEVEL, cLevel, _countof(cLevel));

		char cText[256];
		sprintf(cText, "%s %d", cLevel, nLV1);
		g_pFont->DrawUIText(0, 339, 174, 339, cText);

		sprintf(cText, "%s %d", cLevel, nLV2);
		g_pFont->DrawUIText(0, 620, 174, 620, cText);
	}

	switch(g_FCGameData.SelStageId)
	{
	case STAGE_ID_NONE:
	case STAGE_ID_TUTORIAL:				//tutorial
	case STAGE_ID_ENDING_MYIFEE:		//미피 엔딩맵
	case STAGE_ID_ENDING_DWINGVATT:		//딩그바트 엔딩맵
	case STAGE_ID_ENDING_VIGKVAGK:		//비그바그 엔딩맵
	case STAGE_ID_ENDING_MA_INPHYY:		//인피 마계 엔딩맵
		{
			break;
		}
	default:
		{
			g_pFont->DrawUIText(-1, -1, -1, -1, "@(reset)@(scale=0.8,0.8)");

			char cStageName[64];
			int nHeroID = g_MissionToSoxID[g_FCGameData.nPlayerType];
			g_FCGameData.GetStageName(g_FCGameData.SelStageId, cStageName, _countof(cStageName), nHeroID);

			char cLevel[64];
			g_TextTable->GetText(_TEX_SUM_LEVEL, cLevel, _countof(cLevel));

			int nEnemyLevelUp = 3;
			HeroRecordInfo* pHeroRecordInfo = g_FCGameData.tempFixedSave.GetHeroRecordInfo(nHeroID);
			if(pHeroRecordInfo != NULL)
			{
				StageResultInfo* pStageResultInfo = pHeroRecordInfo->GetStageResultInfo(g_FCGameData.SelStageId);
				if(pStageResultInfo != NULL){
					nEnemyLevelUp += pStageResultInfo->nEnemyLevelUp;
				}
			}

			char cFullText[256];
			sprintf(cFullText, "%s   %s %d", cStageName, cLevel, nEnemyLevelUp);
			g_pFont->DrawUIText(80, 160, 1280, 720, cFullText);
		}
	}
}


////////////////////////////////////
// CFantasyCore class
CFantasyCore::CFantasyCore()
{
	m_bUseThread = false;
	m_bIsFrameSkip = true;
	m_bTestFakeLiquid = true;
	m_bFreeCam = false;

	m_pSkip = new CSkipper( DEFAULT_FRAME_RATE );
	m_pOrb = NULL;
	m_pLiveManager = NULL;
	m_hTexInit = NULL;
	m_nPhysicsEnable = -1;
	m_nCubemap = -1;
	m_nCubemap1 = -1;
	m_nLogoTexID = -1;
	m_nLevelUpTexID = -1;

	memset(m_nComboEffectTexID, -1, sizeof(int) * _COMBO_EFFECT_COUNT);
	memset(m_nKillEffectTexID, -1, sizeof(int) * _KILL_EFFECT_COUNT);
	memset(m_nKillwEffectTexID, -1, sizeof(int) * _KILL_EFFECT_COUNT);

	m_nTexImgID = -1;
	m_bReleaseTitleData = true;
}


CFantasyCore::~CFantasyCore()
{
	ReleaseGame();
	//	BsAssert( _CrtCheckMemory( ) ); 
}

int CFantasyCore::InitGameFirstTime(bool bWindow, bool bIsFrameSkip)
{
	//---------------------------
#ifndef _XBOX

#ifdef _DEBUG
	bWindow = true;
#endif
	
	g_BsKernel.SetNovodexEnable( true );
	CBsKernel::GetInstance().Initialize(g_hWnd, _SCREEN_WIDTH, _SCREEN_HEIGHT, bWindow);

	g_BsMoviePlayer.Initialize();

	//---------------------------
#else

#ifndef _LTCG
	g_FCGameData.bShowSafeArea = true;
#endif

	CBsFileIO::Init();
	CBsFileIO::SetDirtyDiskHandlerCallBackFunc( ( void (*)( void ) )&CFantasyCore::DirtyDiskHandlerCallBackFunc );

	g_BsResChecker.SetGroup( RES_GROUP_FIRST_INIT );
	g_BsMemChecker.Start( "Init 3DDevice" );
	g_BsKernel.InitializeDevice();
	g_hReleaseEvent = g_BsKernel.GetDevice()->GetReleaseEventHandle();
	g_BsMemChecker.End();

	g_BsMemChecker.Start( "Init movie player" );
	g_BsMoviePlayer.Initialize();
	g_BsMemChecker.End();

	g_BsKernel.GetDevice()->ReleaseD3DThreadOwnership();
	g_hMovieLoadedEvent = CreateEvent( NULL, FALSE, FALSE, NULL );
	g_hStartupDoneEvent = CreateEvent( NULL, FALSE, FALSE, NULL );
	g_hStartupThread = CreateThread( NULL, 65536, StartupThreadProc, NULL, 0, &g_dwStartupThreadId );
	XSetThreadProcessor( g_hStartupThread, 0 );

	// Wait for the movie to load as we don't want any other IO taking place
	// right now....
	WaitForSingleObject(g_hMovieLoadedEvent,INFINITE);

	HANDLE hResInit = CBsFileIO::BsMountCompressedPackFile("d:\\data\\PackedResources\\Res_Init.pak");
	m_hTexInit = CBsFileIO::BsMountCompressedPackFile("d:\\data\\PackedResources\\Tex_Init.pak", true);
	g_hPackedWeaponTex = CBsFileIO::BsMountCompressedPackFile( "d:\\data\\PackedResources\\Tex_Weapons.pak", true);

	if( g_hPackedResTitle == NULL )
		g_hPackedResTitle = CBsFileIO::BsMountCompressedPackFile("d:\\data\\PackedResources\\Res_Title.pak");
	if( g_hPackedTexTitle == NULL )
		g_hPackedTexTitle = CBsFileIO::BsMountCompressedPackFile("d:\\data\\PackedResources\\Tex_Title.pak", true);

	// It need on game
	g_hPackedWeaponRes = CBsFileIO::BsMountCompressedPackFile("d:\\data\\PackedResources\\Res_Weapons.pak");

	// Prevent stalls when showing portrait pictures..
	g_hPortraitCache = CBsFileIO::BsMountPackFile("d:\\data\\PackedResources\\Portrait_Cache.pak");


	g_BsMemChecker.Start( "Init BsKernel" );
	g_BsKernel.SetNovodexEnable( true );
	CBsKernel::GetInstance().Initialize();
	g_BsMemChecker.End();


	if(g_bIsStartTitleMenu){
		CBsKernel::GetInstance().ShowFPS(false, 100, 100);
	}
	else{
		CBsKernel::GetInstance().ShowFPS(true, 100, 100);
	}

#endif // #ifndef _XBOX
	//---------------------------
	g_BsMemChecker.Start( "Init debug console" );
	g_BsKernel.ConsoleInitialize();
	g_BsMemChecker.End();


	//	CBsKernel::GetInstance().SetProjectionMatrix(10, 20000.0f);
	CBsKernel::GetInstance().FrameCheckStart();
	CBsKernel::GetInstance().chdir("data");
	
	//loading Image
	InitFirstTimeImg();

	g_BsMemChecker.Start( "Init SOX data" );
	LoadSOX();
	LoadLocalSOX();
	g_BsMemChecker.End();

	g_BsMemChecker.Start( "Load terrain particle list" );
	LoadTerrainParticleList();
	g_BsMemChecker.End();
	CBsKernel::GetInstance().chdir("..");

	g_BsMemChecker.Start( "Init UI" );
	CBsKernel::GetInstance().CreateUI();

	CBsKernel::GetInstance().chdir("data");
	BsUiSystem::CreateInstance();
	g_BsUiSystem.Create(new BsUiGDISkin(), new BsUiGDIView(), new BsUiLoader(), new BsUiStore(), 0, 0);

	g_BsMemChecker.End();

	// AI Class Initialize
	g_BsMemChecker.Start( "Init AI" );
	CFcAIObject::InitializeObjects();
	g_BsMemChecker.End();

	g_BsMemChecker.Start( "Init Inputpad" );

#ifndef _XBOX
#pragma warning(disable:4311)
	g_InputPad.Create((int)g_hWnd);
#pragma warning(default:4311)
#else
	g_InputPad.Create(0);
#endif // #ifndef _XBOX

	g_BsMemChecker.End();


	g_BsMemChecker.Start( "Init Sound" );

#ifdef _XBOX
	char cSoundDir[MAX_PATH];
	sprintf( cSoundDir, "%ssound\\", g_BsKernel.GetCurrentDirectory() );
	g_pSoundManager = new FcSoundManager( cSoundDir );
	g_pSoundManager->Initialize("NNN_SOUND.xgs","FcWaveBankList.txt");
#endif
	g_BsMemChecker.End();

	g_BsMemChecker.Start( "Load item data" );
	g_FcItemManager.LoadItemData();
	g_BsMemChecker.End();

	g_BsMemChecker.Start( "Load global physics setting" );
	CFcPhysicsLoader::LoadGlobalPhysicsSetting("PhysicsSetting.txt");
	g_BsMemChecker.End();

	if(bIsFrameSkip) {
		m_pSkip->Start();
	}

	g_BsMemChecker.Start( "Init interfacemanager" );
	g_InterfaceManager.Initialize();
	g_BsMemChecker.End();

	g_BsMemChecker.Start( "Init fxmanager" );
	g_pFcFXManager = new CFcFXManager;
	g_BsMemChecker.End();
	//SAFE_DELETE(m_pOrb);

	//m_pOrb = new Orb;
	//m_pOrb->Initialize(g_BsKernel.GetDevice());
	// 실기 무비 미리 Load - 

	//_TNT_
#ifdef _TNT_
#ifdef _XCR_
	// Initialize signal callback and set signal message to null string.
	XCRSetSignalCallback(&HandleSignal, (void*) this);
	g_signalMessage[0] = L'\0';
#endif
#endif

	//option
	CInputPad::GetInstance().SetVibrationOn(g_FCGameData.ConfigInfo.bVibration);	
	g_BsMoviePlayer.SetVolume( (int)((g_FCGameData.ConfigInfo.nBGMVolume * 0.1f) * (float)_MAX_BINK_VOLUME) );
	g_pSoundManager->SetVolume(FC_MENU_VOL_BGM, g_FCGameData.ConfigInfo.nBGMVolume * 0.1f);
	g_pSoundManager->SetVolume(FC_MENU_VOL_EFT_SOUND, g_FCGameData.ConfigInfo.nSoundVolume * 0.1f);
	g_pSoundManager->SetVolume(FC_MENU_VOL_VOICE, g_FCGameData.ConfigInfo.nVoiceVolume * 0.1f);
	g_BsKernel.SetGammaRamp(float(g_FCGameData.ConfigInfo.nContrast)/10.f);
#ifdef _XBOX
	XNotifyPositionUI(XNOTIFYUI_POS_TOPRIGHT);
#endif

#ifdef _XBOX
	WaitForSingleObject(g_hStartupDoneEvent,INFINITE);
	g_BsKernel.GetDevice()->AcquireD3DThreadOwnership();

	if(g_hStartupThread)
	{
		g_dwStartupThreadId = 0;
		CloseHandle(g_hStartupThread);
		g_hStartupThread = NULL;
	}
	if(g_hStartupDoneEvent)
	{
		CloseHandle(g_hStartupDoneEvent);
		g_hStartupDoneEvent = NULL;
	}
	if(g_hMovieLoadedEvent)
	{
		CloseHandle(g_hMovieLoadedEvent);
		g_hMovieLoadedEvent = NULL;
	}
#endif

#ifdef _XBOX
	CBsFileIO::BsDismountPackFile(hResInit);
#endif
	//g_BsResChecker.PopGroup();

	//g_FCGameData.LoadDashBoardCaption();

	return 1;
}


void CFantasyCore::InitFirstTimeImg()
{
	// Force Cubemap and Cubemap1 textures to load here are they
	// are used throughout the game by almost all units
	g_BsKernel.GetInstance().chdir("CubeMap");
	m_nCubemap = g_BsKernel.LoadCubeTexture("Cubemap.dds");
	BsAssert(m_nCubemap != -1);
	m_nCubemap1 = g_BsKernel.LoadCubeTexture("Cubemap1.dds");
	BsAssert(m_nCubemap1 != -1);
	g_BsKernel.chdir("..");

	
	//-------------------------------------------------------------------
	//interface local Image
	g_BsKernel.chdir("interface");
	g_BsKernel.chdir("Local");

	//-------------------------
	char cLogoFileName[256];
	sprintf(cLogoFileName, "%s\\%s_%s.dds",
		g_LocalLanguage.GetLanguageDir(),
		_LOADING_MISSION_LOGO,
		g_LocalLanguage.GetLanguageStr());

	m_nLogoTexID = g_BsKernel.LoadTexture(cLogoFileName);
	BsAssert(m_nLogoTexID != -1);

	//-------------------------
	char cLevelUpImage[256];
	sprintf(cLevelUpImage, "%s\\gm_LvUp_%s.dds",
		g_LocalLanguage.GetLanguageDir(),
		g_LocalLanguage.GetLanguageStr());	
	
	m_nLevelUpTexID = g_BsKernel.LoadTexture(cLevelUpImage);
	BsAssert(m_nLevelUpTexID != -1);
	
	//-------------------------
	//limit combo
	char cComboImg[256];
	for(int i=0; i<_COMBO_EFFECT_COUNT; i++)
	{
		sprintf(cComboImg, "%s\\gm_com_%d_%s.dds",
			g_LocalLanguage.GetLanguageDir(),
			g_ComboEffectUnit[i],
			g_LocalLanguage.GetLanguageStr());

		m_nComboEffectTexID[i] = g_BsKernel.LoadTexture(cComboImg);
		BsAssert(m_nTexImgID != m_nComboEffectTexID[i]);
	}

	//-------------------------
	//limit kill effect
	char cKillImg[256], cKillwImg[256];
	for(int i=0; i<_KILL_EFFECT_COUNT; i++)
	{
		sprintf(cKillImg, "%s\\gm_kill_%d_%s.dds",
			g_LocalLanguage.GetLanguageDir(),
			g_KillEffectUnit[i],
			g_LocalLanguage.GetLanguageStr());

		sprintf(cKillwImg, "%s\\gm_kill_%d_w_%s.dds",
			g_LocalLanguage.GetLanguageDir(),
			g_KillEffectUnit[i],
			g_LocalLanguage.GetLanguageStr());

		m_nKillEffectTexID[i] = g_BsKernel.LoadTexture(cKillImg);
		BsAssert(m_nKillEffectTexID[i] != m_nComboEffectTexID[i]);

		m_nKillwEffectTexID[i] = g_BsKernel.LoadTexture(cKillwImg);
		BsAssert(m_nKillwEffectTexID[i] != m_nComboEffectTexID[i]);
	}

	//----------------------------
	//load image
	char cTextFileName[256];
	sprintf(cTextFileName, "%s\\%s_%s.dds",
		g_LocalLanguage.GetLanguageDir(),
		_LOADING_MISSION_TEXT,
		g_LocalLanguage.GetLanguageStr());
	
	m_nTexImgID = g_BsKernel.LoadTexture(cTextFileName);
	BsAssert(m_nTexImgID != -1);

	//----------------------------
	g_BsKernel.chdir("..");
	g_BsKernel.chdir("..");
}

void CFantasyCore::ReleaseGame()
{	
	SAFE_DELETE( g_pFcFXManager );
	SAFE_DELETE( g_pFcRealMovie );

	CFcBaseObject::ReleaseObjects();
	CFcAIObject::ReleaseObjects( true );
	CFcTroopAIObject::ReleaseObjects();

	g_FCGameData.Clear();

	//CFcPhysicsLoader::ClearAll();
#ifdef _XBOX
	//	SAFE_DELETE( g_pSoundManager );
#endif

	SAFE_DELETE(m_pSkip);
	//SAFE_DELETE(m_pOrb);
	SAFE_DELETE(m_pLiveManager);

	CBsFileIO::Finalize();

	SAFE_RELEASE_TEXTURE(m_nCubemap);
	SAFE_RELEASE_TEXTURE(m_nCubemap1);

	SAFE_RELEASE_TEXTURE(m_nLogoTexID);
	SAFE_RELEASE_TEXTURE(m_nLevelUpTexID);
	SAFE_RELEASE_TEXTURE(m_nTexImgID);

	for(int i=0; i<_COMBO_EFFECT_COUNT; i++){
		SAFE_RELEASE_TEXTURE(m_nComboEffectTexID[i]);
	}

	for(int i=0; i<_KILL_EFFECT_COUNT; i++){
		SAFE_RELEASE_TEXTURE(m_nKillEffectTexID[i]);
		SAFE_RELEASE_TEXTURE(m_nKillwEffectTexID[i]);
	}

	if(m_hTexInit != NULL)
	{
		CBsFileIO::BsDismountPackFile(m_hTexInit);
		m_hTexInit = NULL;
	}

	if( g_hPackedWeaponTex )
	{
		CBsFileIO::BsDismountPackFile( g_hPackedWeaponTex );
		g_hPackedWeaponTex = NULL;
	}

	if( g_hPackedWeaponRes )
	{
		CBsFileIO::BsDismountPackFile( g_hPackedWeaponRes );
		g_hPackedWeaponRes = NULL;
	}

	if( g_hPortraitCache )
	{
		CBsFileIO::BsDismountPackFile( g_hPortraitCache );
		g_hPortraitCache = NULL;
	}
}

bool CFantasyCore::InitTitle()
{
	m_bReleaseTitleData = false;
	ReleaseStage();

	// After ReleaseStage
#if defined(_XBOX)
	g_BsKernel.GetDevice()->SetRenderState(D3DRS_PRESENTINTERVAL, D3DPRESENT_INTERVAL_TWO);
	m_pSkip->Stop();
#endif

	ResetProcessTick();

#if defined(DUMP_MEMORY_INFO)
	static int s_initCount = 0;

	char filename[512];
	sprintf_s( filename, _countof(filename), "d:\\title_%d.meminfo", s_initCount );
	++s_initCount;

	BsMem::WriteInfo( filename );
#endif

	g_BsKernel.SetUIViewerIndex(_UI_FULL_VIEWER);

	g_MenuHandle->RemoveAllMenu();
	g_MenuHandle->ReleaseLoadMenu();
	BsUiSystem::GetInstance().RemoveAllWindow();
	BsUiSystem::GetInstance().GetLoader()->Release();

	if(g_bIsStartTitleMenu)
	{	
		g_FCGameData.bShowEnemyGauge = false;

		if(g_bGotoTitle == true)
		{
			g_FCGameData.tempUserSave.Clear();
			g_FcItemManager.Finalize();

			if(g_FCGameData.bOpenLogo == true)
			{
#ifndef _XBOX
				g_MenuHandle->PostMessage(fcMSG_LOGO_START);
#else
				// Don't do logos etc on XBOX. They are now done in a separate thread
				// to hide file loading
				g_MenuHandle->PostMessage(fcMSG_OPENING_START);
#endif
				g_FCGameData.bOpenLogo = false;
			}
			else
			{
				g_MenuHandle->PostMessage(fcMSG_TITLE_START);
				//g_pSoundManager->LoadWaveBank( WB_M_SELECT );
			}
		}
		else
		{
			g_MenuHandle->PostMessage(fcMSG_SELECT_STAGE_START);
			//g_pSoundManager->LoadWaveBank( WB_M_MAP );
		}
		g_bGotoTitle = false;
	}
	else
	{
#ifdef _START_STAGE
		strcpy( g_FCGameData.cMapFileName, _START_STAGE_NAME );
		g_FC.SetMainState( GAME_STATE_INIT_STAGE, ( DWORD )_START_STAGE_NAME );
		return false;
#else
		g_MenuHandle->PostMessage(fcMSG_DEBUG_MENU_TOGGLE);
#endif		
	}

	return true;
}

bool CFantasyCore::InitStage()
{
	if(g_FCGameData.bChangeProfile)
	{
		g_MenuHandle->PostMessage(fcMSG_GO_TO_TITLE);
		return false;
	}

	m_bReleaseTitleData = true;
	ReleaseStage();

	g_MenuHandle->RemoveAllMenu();
	g_MenuHandle->ReleaseLoadMenu();
	BsUiSystem::GetInstance().RemoveAllWindow();
	BsUiSystem::GetInstance().GetLoader()->Release();

	g_nWaitTickPauseMenu = _WAIT_TICK_PAUSEMENU;

	char cMissionName[32];
	strcpy( cMissionName, g_FCGameData.cMapFileName );
	char* cTempStr = strstr( cMissionName, "." );
	if( cTempStr )
		cTempStr[0] = NULL;

	// RES1 Pack원래 자리

	g_BsResChecker.SetGroup( RES_GROUP_RES1 );

#if defined(_XBOX)
	g_BsKernel.GetDevice()->SetRenderState(D3DRS_PRESENTINTERVAL, D3DPRESENT_INTERVAL_IMMEDIATE);
	if(m_bIsFrameSkip)
		m_pSkip->Start();
#endif

#if defined(DUMP_MEMORY_INFO)
	static int s_initCount = 0;

	char filename[512];
	sprintf_s( filename, _countof(filename), "d:\\stage_%d.meminfo", s_initCount );
	++s_initCount;

	BsMem::WriteInfo( filename );
#endif

	if( g_FCGameData.bSaveInput ){
		CInputPad::GetInstance().Save();
	}

	if( g_FCGameData.lSaveRandomSeed ){
		SetRandomSeed( g_FCGameData.lSaveRandomSeed );
	}
	srand( 0 );

	ResetProcessTick();
	g_FcHitMarkMgr.Reset();
	g_BsKernel.SetRenderTick( GetProcessTick() );
	g_InputPad.InitState();
	CInputPad::GetInstance().SetVibrationOn(g_FCGameData.ConfigInfo.bVibration);

	g_FCGameData.AutoSave();

	//아스파 미션에서 game중에 다음 stage에 대한 분기를 처리하는 menu
	//trigger에서 check하는 flag
	//g_FCGameData.SpeaialSelStageId = STAGE_ID_NONE;
	g_FCGameData.nSpecialSelStageState = SPECIAL_STAGE_MENU_WAIT;

	if(m_bIsFrameSkip) {
		m_pSkip->Start();
	}

#ifdef USE_RENDER_THREAD
	CreateRenderThread();
#endif
	{
#ifdef _XBOX
		CTimeRender TimeRender( m_bUseThread, 20, false );
#endif
		char cPakStr[256];
		sprintf( cPakStr, "d:\\data\\PackedResources\\%s_Res1_Res.pak", cMissionName );
		g_hPackedRes1 = CBsFileIO::BsMountCompressedPackFile( cPakStr );
		sprintf( cPakStr, "d:\\data\\PackedResources\\%s_Res1_Tex.pak", cMissionName );
		g_hPackedTex1 = CBsFileIO::BsMountCompressedPackFile( cPakStr, true);

		g_BsResChecker.AddIgnoreFolderList( "D:\\data\\weapons\\" );

		CFcWorld::GetInstance().LoadWorld( g_FCGameData.cMapFileName );		
		g_pFcRealMovie = new FcRealtimeMovie();
		g_InterfaceManager.GetInstance().InitStage();
		g_MenuHandle->LoadGameMenuData();

		g_FcItemManager.LoadHeroItemSkin();
		g_FcItemManager.UpdateItemEffect();

		if( g_hPackedRes3 )
		{
			CBsFileIO::BsDismountPackFile( g_hPackedRes3 );
			g_hPackedRes3 = NULL;
		}

/*
		if( g_hPackedTex3 )
		{
			CBsFileIO::BsDismountPackFile( g_hPackedTex3 );
			g_hPackedRes3 = NULL;
		}
*/

		g_BsResChecker.PopGroup();
		g_BsResChecker.SetGroup( RES_GROUP_AFTER_GAME );

		SetPause( false );

#ifdef _XBOX	
		if( g_FCGameData.nCapture )
		{
			g_nCaptureIndex = 0;
			g_nCaptureFolderIndex = 0;

			CreateDirectory( "e:\\Capture\\0\\", NULL );
		}
#endif //_XBOX

	}
	return true;
}

bool CFantasyCore::IsMissionEnd()
{
	MISSION_FINISH_TYPE nMissionFinish = CFcWorld::GetInstance().GetMissionFinishType();
	if(nMissionFinish != MISSION_FINISH_TYPE_NON)
	{
		return true;
	}

	return false;
}


int CFantasyCore::InitPauseMenu()
{	
	if( IsPause() == true )
	{
		if(g_bIsStartTitleMenu)
		{
			if(g_MenuHandle->GetGamePauseMenu() == _FC_MENU_PAUSE ||
				g_MenuHandle->GetGamePauseMenu() == _FC_MENU_STATUS)
			{
				return 1;
			}
		}
		else{
			return 1;
		}
	}

	return 0;
}

bool CFantasyCore::IsPauseMenu()
{
	if(g_bIsStartTitleMenu)
	{
		if(g_MenuHandle->GetGamePauseMenu() == _FC_MENU_PAUSE ||
			g_MenuHandle->GetGamePauseMenu() == _FC_MENU_STATUS)
		{
			BsAssert(IsPause() == true);
			return true;
		}
	}
	else
	{
		if( IsPause() == true ){
			return true;
		}
	}

	return false;
}


void CFantasyCore::CheckActiveXUI()
{
	if(m_pLiveManager == NULL){
		return;
	}

#ifdef _XBOX
	if(IsPause() == false && m_pLiveManager->IsUIActive()){
		SetPauseForXUI(true);
	}
#endif
}

bool CFantasyCore::InitSpecialSelStageMenu()
{
	if( g_pFcRealMovie && g_pFcRealMovie->IsPlay() == true ){
		return false;
	}

	g_MenuHandle->PostMessage(fcMSG_SPECIAL_SELSTAGE_START);
	return true;
}

bool CFantasyCore::IsSpecialSelStageMenu()
{
	if(g_FCGameData.nSpecialSelStageState == SPECIAL_STAGE_MENU_ON){
		return true;
	}

	return false;
}


bool CFantasyCore::InitGameResultSign()
{
	MISSION_FINISH_TYPE nMissionFinish = CFcWorld::GetInstance().GetMissionFinishType();

	if(nMissionFinish != MISSION_FINISH_TYPE_NON)
	{
		//mission이 종료되면 vibration은 강제로 off한다.
		for(int i = 0; i < MAX_XBOX_PAD_COUNT; i++ ){
			CInputPad::GetInstance().SetVibration( i, 0, 0 );
		}

		g_FCGameData.nMissionFinish = nMissionFinish;

		if(nMissionFinish == MISSION_FINISH_TYPE_SUCCESS)
		{
			//ending map이 끝나면 결과sign이나 결과창없이 바로 진행
			if(IsEndingCharStage()){
				g_MenuHandle->PostMessage(fcMSG_END_START);
			}
			else
			{
				SaveGlobalGameState();		// 게임 클리어일 때만 처리
				g_MenuHandle->PostMessage(fcMSG_GAME_CLEAR_START);
			}
		}
		else if(nMissionFinish == MISSION_FINISH_TYPE_FAIL)
		{
			BsAssert(IsEndingCharStage() == false);

			g_FCGameData.stageInfo.nPlayGrade = 5;		//실패는 무조건 F!
			g_MenuHandle->PostMessage(fcMSG_GAME_OVER_START);
		}

		return true;
	}

	return false;

}

bool CFantasyCore::IsGameResultSign()
{
	switch(g_MenuHandle->GetCurMenuType())
	{
	case _FC_MENU_GAME_CLEAR:
	case _FC_MENU_GAME_OVER:	return true;
	}

	return false;
}

bool CFantasyCore::InitGameResult()
{
	m_bReleaseTitleData = false;
	ReleaseStage();

#if defined(DUMP_MEMORY_INFO)
	static int s_initCount = 0;

	char filename[512];
	sprintf_s( filename, _countof(filename), "d:\\game_%d.meminfo", s_initCount );
	++s_initCount;

	BsMem::WriteInfo( filename );
#endif

	/*if(g_FCGameData.nMissionFinish == MISSION_FINISH_TYPE_SUCCESS){		
	g_pSoundManager->LoadWaveBank( WB_M_RESULT ); //성공시에
	g_pSoundManager->PlayBGM("M_Result");
	}
	else if(g_FCGameData.nMissionFinish == MISSION_FINISH_TYPE_FAIL){
	g_pSoundManager->LoadWaveBank( WB_M_GAMEOVER ); //실패시에
	g_pSoundManager->PlayBGM("M_Gameover");
	}*/

	g_FCGameData.nMissionFinish = MISSION_FINISH_TYPE_NON;
	g_MenuHandle->PostMessage(fcMSG_STAGE_RESULT_START);




	return true;
}

bool CFantasyCore::IsGameResult()
{
	if(g_MenuHandle->GetCurMenuType() == _FC_MENU_STAGE_RESULT){
		return true;
	}

	return false;
}

int CFantasyCore::InputProcess()
{
	if(!g_InputPad.Process()){
		return 0;
	}

#ifdef _USE_CONSOLE_COMMAND_
#ifdef _XBOX
	XINPUT_KEYSTROKE Key;

	if(XInputGetKeystroke(XUSER_INDEX_ANY, XINPUT_FLAG_KEYBOARD, &Key)==ERROR_SUCCESS){
		if(Key.Flags&XINPUT_KEYSTROKE_KEYDOWN){
			if( Key.VirtualKey == VK_TAB )
			{
				Key.Unicode = Key.VirtualKey;
			}
			if( Key.VirtualKey == VK_ESCAPE )
			{
				FreeCam();
			}
			else
			{
				InputKeyboard( ( unsigned char )Key.Unicode );
			}
		}
	}
#endif // #ifdef _XBOX
#endif // _USE_CONSOLE_COMMAND_

	ProcessStartBack();

	return 1;
}

void CFantasyCore::CreateRenderThread()
{
	g_hEndStageEvent = CreateEvent( NULL, FALSE, FALSE, NULL );
	g_hQuitRenderEvent = CreateEvent( NULL, FALSE, FALSE, NULL );

	g_hProcessCountSemaphore = CreateSemaphore( NULL, 0, 2, NULL );
	g_hReadyCountSemaphore = CreateSemaphore( NULL, 2, 2, NULL );		

	g_hRenderThread = CreateThread( NULL, 65536 * 2, RenderThreadProc, NULL, 0, &g_dwRenderThreadId );
	BsAssert( g_hRenderThread && "Create Thread Failed" );
#ifdef _XBOX
	g_BsKernel.GetDevice()->ReleaseD3DThreadOwnership();
	XSetThreadProcessor( g_hRenderThread, 0 );
#endif

	m_bUseThread = true;
}

void CFantasyCore::DestroyRenderThread()
{
	if( g_hRenderThread )
	{
		SetEvent( g_hEndStageEvent );
		ReleaseSemaphore( g_hProcessCountSemaphore, 1, NULL ); // 무한루프 방지		
		WaitForSingleObject( g_hQuitRenderEvent, INFINITE );
		CloseHandle( g_hEndStageEvent );
		g_hEndStageEvent = NULL;
		CloseHandle( g_hQuitRenderEvent );
		g_hQuitRenderEvent = NULL;

		CloseHandle( g_hRenderThread );
		g_dwRenderThreadId = 0;
		g_hRenderThread = NULL;

		m_bUseThread = false;

		CloseHandle( g_hProcessCountSemaphore );
		CloseHandle( g_hReadyCountSemaphore );
#ifdef _XBOX
		g_BsKernel.GetDevice()->AcquireD3DThreadOwnership();
#endif
		// TODO : Kernel에서의 Clear루틴 정리해야 할것 같습니다.
		g_BsKernel.ResetKernelCommand();
		CBsObject::FlipRenderBuffer();
		CBsUIManager::FlipRenderBuffer();
		g_BsKernel.ResetKernelCommand();
	}
}

// test
//int nTestGameTick = 0;

void CFantasyCore::MainProcess()
{
g_BsKernel.BeginPIXEvent( 255, 0, 0, "Process Start" );
	g_BsKernel.EndPIXEvent();

	g_BsKernel.ProcessConsole();

	ProcessMainState();	// 이거는 제일 앞에 와야 한다.. 언제나..

	if( m_bUseThread )
	{
		PROFILE_TIME_TEST( WaitForSingleObject( g_hReadyCountSemaphore, INFINITE ) );		
		//		WaitForSingleObject( g_hReadyCountSemaphore, INFINITE );
	}	

	InputProcess();

	g_pSoundManager->Process();

	CameraObjHandle CamHandle = CFcBaseObject::GetCameraObjectHandle( 0 );
	if( CamHandle ){
		CamHandle->UpdateUI();
	}

	g_nPerfCounter++;
	//	DebugString( "Process : %d\n", g_nPerfCounter );

	if( g_pFcRealMovie) {	
		(*g_pFcRealMovie->m_pfnRenderProcess)( g_pFcRealMovie );
		(*g_pFcRealMovie->m_pfnWorkProcess)( g_pFcRealMovie );
		g_pFcRealMovie->Process();	
	}

	if( IsPause() == false )
	{
		if( !m_bFreeCam )
		{
			//			g_BsKernel.BeginPIXEvent(255, 0, 0, "Process Event");
			g_BsKernel.Process();

			g_BsKernel.BeginPIXEvent( 255, 0, 0, "Object & Troop Process" );
			CFcBaseObject::ProcessObjects();
			PROFILE_TIME_TEST( g_FcWorld.Process() );
			//			g_FcWorld.Process();
			g_BsKernel.EndPIXEvent();

			if( g_pFcRealMovie ) {	
			}

			g_BsKernel.BeginPIXEvent( 255, 0, 0, "AI Process" );
			PROFILE_TIME_TEST( CFcAIObject::ProcessObjects() );
			//			CFcAIObject::ProcessObjects();
			g_BsKernel.EndPIXEvent();

			CFcTroopAIObject::ProcessObjects();

			if( g_pFcFXManager )
				g_pFcFXManager->Process();

			g_BsKernel.BeginPIXEvent( 255, 0, 0, "Object & Troop PostProcess" );
			CFcBaseObject::PostProcessObjects();
			PROFILE_TIME_TEST( g_FcWorld.PostProcess() );
			//			g_FcWorld.PostProcess();
			g_BsKernel.EndPIXEvent();

			//CBsKernel::GetInstance().Process();			
			//			PROFILE_TIME_TEST( CFcLayerObject::ProcessObjects() );
			g_FcItemManager.Process();
			//			g_BsKernel.EndPIXEvent();

		}
		else{
			CameraObjHandle CamHandle;
			CamHandle=CFcBaseObject::GetCameraObjectHandle(0);
			if( CamHandle )
			{
				CamHandle->Process();
				//			CFcLayerObject::ProcessObjects();
				CamHandle->PostProcess(); //aleksger: prefix bug id 537: CamHandle may be NULL based upon the check above.
			}
		}
		++g_nProcessTick;
	}
	else		// Pause 상태
	{
		if(g_InputPad.IsEnable()){
			//			CFcLayerObject::ProcessObjects();
		}
	}	
	g_InterfaceManager.Process();

	g_BsKernel.BeginPIXEvent( 255, 0, 0, "Object & Troop Update" );
	PROFILE_TIME_TEST( g_FcWorld.Update() );
	//	g_FcWorld.Update();
	
	CFcBaseObject::RenderObjects();
	g_BsKernel.EndPIXEvent();

	// 뒤로 옮김.
	if( g_pFcRealMovie) {
		g_pFcRealMovie->Update();
	}

	if(g_pFcFXManager)
		g_pFcFXManager->Update();

	

	g_BsKernel.BeginPIXEvent( 255, 0, 0, "Novodex & Animation Calculate" );
	PROFILE_TIME_TEST( g_BsKernel.PreInitRenderFrame() );
	//	g_BsKernel.PreInitRenderFrame();
	g_BsKernel.RunPhysics ( m_pSkip->GetDeltaTime() );
	PROFILE_TIME_TEST(g_BsKernel.InitRenderFrame());		
	PROFILE_TIME_TEST( g_BsKernel.GetPhysicsMgr()->Flush() );
	PROFILE_TIME_TEST( g_BsKernel.PostInitRenderFrame() );	
	//	g_BsKernel.PostInitRenderFrame();
	g_BsKernel.EndPIXEvent();

	if( m_nPhysicsEnable != -1 ) {
		g_BsKernel.GetPhysicsMgr()->SetEnable( m_nPhysicsEnable != 0  );
		m_nPhysicsEnable = -1;
	}
	
	if( m_bUseThread )
	{
		ReleaseSemaphore( g_hProcessCountSemaphore, 1, NULL );		
	}

	ProcessLive();

	if( g_pFcRealMovie ) // && g_pFcRealMovie->GetIsDelete() )
	{
		(*g_pFcRealMovie->m_pfnPostRenderProcess)( g_pFcRealMovie );
	}

	g_BsKernel.SetCurInputPort( g_FCGameData.nEnablePadID );

	g_FcHitMarkMgr.ProcessHitMarkMgr( GetProcessTick() );

	g_BsKernel.BeginPIXEvent( 255, 0, 0, "Process End" );
	g_BsKernel.EndPIXEvent();
}

void CFantasyCore::MainRenderProcess()
{
	m_pSkip->IsSkip();

	if( !IsSkipSceneRendering()) {

		CBsKernel::GetInstance().ProcessParticleObjectList();

		g_BsKernel.BeginPIXEvent(0, 255, 0, "FX Manager PreUpdate");	
		if( g_pFcFXManager ) {
			g_pFcFXManager->PreUpdate();
		}
		g_BsKernel.EndPIXEvent();

		g_FcWorld.GetTroopmanager()->RenderObjects();
	}

	g_BsKernel.BeginPIXEvent(0, 255, 0, "BsKernel Render");	
	//			PROFILE_TIME_TEST( CBsKernel::GetInstance().RenderFrame() );
	CBsKernel::GetInstance().ClearBuffer(0, 0, 0);

	if( IsSkipSceneRendering() )  {
		g_BsKernel.RenderSceneUI();
	}
	else {
		g_BsKernel.RenderFrame();
	}	

#ifdef _XBOX
	if( ( g_nCaptureIndex != -1 ) && ( GetProcessTick() % g_FCGameData.nCapture == 0 ) )
	{
		char szCapture[ 1024 ];
		sprintf(szCapture, "e:\\Capture\\%d\\%06d.jpg", g_nCaptureFolderIndex, g_nCaptureIndex );
		g_BsKernel.SaveScreen( szCapture );
		g_nCaptureIndex++;

		if( g_nCaptureIndex / 2000 != g_nCaptureFolderIndex )
		{
			++g_nCaptureFolderIndex;
			char szFolder[ 1024 ];
			sprintf(szFolder, "e:\\Capture\\%d\\", g_nCaptureFolderIndex);
			CreateDirectory( szFolder, NULL );
		}
	}
#endif //_XBOX

	g_BsKernel.EndPIXEvent();
	//			PROFILE_TIME_TEST( CustomRender() );

	if( IsSkipSceneRendering() ) {
		CBsUIManager::FlipRenderBuffer();
	}
	else {
		CustomRender();
		//	PROFILE_TIME_TEST( CBsKernel::GetInstance().ResetFrame() );
		CBsKernel::GetInstance().ResetFrame();
	}

	if( m_bUseThread )	// 이부분과 g_BsKernel.RenderFrame(); 사이에 아무것도 넣지 마시오... 
	{
		ReleaseSemaphore( g_hReadyCountSemaphore, 1, NULL );		
	}

	//			PROFILE_TIME_TEST( CBsKernel::GetInstance().Show() );
	if(g_FCGameData.State==GAME_STATE_LOOP_STAGE) {
		if( !m_bUseThread || m_pSkip->IsWait() )
		{
#ifdef _XBOX
#ifndef _BS_PERFORMANCE_CHECK
			// 여기는 수직복귀 기다린다..
			g_BsKernel.GetDevice()->SetRenderState(D3DRS_PRESENTINTERVAL, D3DPRESENT_INTERVAL_ONE);
		}
		else
		{
			// 여기는 수직복귀 안기다린다..
			if(g_pFcRealMovie->IsPlay()) {
				g_BsKernel.GetDevice()->SetRenderState(D3DRS_PRESENTINTERVAL, D3DPRESENT_INTERVAL_ONE);
			}
			else {
				g_BsKernel.GetDevice()->SetRenderState(D3DRS_PRESENTINTERVAL, D3DPRESENT_INTERVAL_IMMEDIATE);
			}
#endif
#endif
		}
	}

	CBsKernel::GetInstance().Show();
#ifdef _XBOX
	if(g_pFcRealMovie && g_pFcRealMovie->IsPlay())
        g_BsKernel.GetDevice()->SetRenderState(D3DRS_PRESENTINTERVAL, D3DPRESENT_INTERVAL_IMMEDIATE);
#endif
	m_pSkip->Wait();
}


int CFantasyCore::MainGameLoop()
{
	// 메모리 릭잡을땨 여기 주석풀어주고..
	// ProcessMainState() <- 요고 주석처리
	// 릭난 Index 번호를 
	// _tWinMain() 에 _CrtSetBreakAlloc( Index ); 너준당..
	/*
	static int aaa = 0;
	if( aaa == 0 ) {
	sprintf( g_FCGameData.cMapFileName, "sivatest3.bsmap" );
	InitStage();

	SetMainState( GAME_STATE_LOOP_STAGE );
	}
	else if( aaa == 100 ) {
	sprintf( g_FCGameData.cMapFileName, "ev-in_1.bsmap" );
	InitStage();

	SetMainState( GAME_STATE_LOOP_STAGE );
	}
	else if( aaa == 102 ) {
	return 0;
	}
	aaa++;
	*/
	///////////////////////////////////////////////////////////////////

	PROFILE_TIME_TEST( MainProcess() );
	//MainProcess();
	if( !m_bUseThread )
	{
		//		PROFILE_TIME_TEST( MainRenderProcess() );
		MainRenderProcess();

		g_BsKernel.FlushFXTemplates();
		g_BsKernel.FlushParticleGroups();
		g_BsKernel.FlushSkins();
		g_BsKernel.FlushVertexDeclarations();
		g_BsKernel.FlushMaterials();
		g_BsKernel.FlushAnis();
	}

	return 1;
}

// 지형관련 파티클 로드는 Stage 로드에서 해야 하지만 임시로 여기서 한다..
bool CFantasyCore::LoadTerrainParticleList()
{
	int nIndex, i;
	Parser parser;
	TokenList toklist;

	parser.Create();
	parser.EnableComments( true );

	bool result = parser.ProcessSource( g_BsKernel.GetCurrentDirectory(), "TerrainParticle.txt", &toklist );
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

		for( i = 0; i < 10; i++ )
		{
			BsAssert( itr->IsString() );
			if( _stricmp( itr->GetString(), "none" ) )
			{
				int nParticleGroup = g_BsKernel.LoadParticleGroup( nIndex * 10 + i + TERRAIN_PARTICLE_START_INDEX, itr->GetString() );
				//if ( nParticleGroup != -1 )
				//	m_vecParticleGroupoList.push_back( nParticleGroup );
			}
			itr++;
		}
	}

	g_BsKernel.chdir("..");

	return true;
}


bool CFantasyCore::LoadSOX()
{
	g_UnitSOX.Load();
	g_LevelTableSOX.Load();
	g_HeroLevelTableSOX.Load();
	g_TroopSOX.Load();
	g_LibTableSox.Load();
	g_LibCharLocalSox.Load();

	return true;
}

bool CFantasyCore::LoadLocalSOX()
{
	g_FcItemManager.LoadLocalItemData();

	return true;
}

void CFantasyCore::ReLoadLocal()
{
	g_pFont->LoadLocalFontTable();
	g_FontAttrMgr.Load();
	LoadLocalSOX();

	if(g_TextTable != NULL)
	{
		delete g_TextTable;
		g_TextTable = NULL;
	}

	g_TextTable = new FCTextTable;
	g_TextTable->SetDefaultTextTable(0); //현재 디폴트 텍스트 테이블
}

void CFantasyCore::ProcessMainState()
{
	switch( g_FCGameData.State )
	{
	case GAME_STATE_INIT_TITLE:
		if( InitTitle() == 1 )
		{			
			SetMainState( GAME_STATE_LOOP_TITLE );
		}
		break;

	case GAME_STATE_LOOP_TITLE:
		{
			;
#ifdef MAKE_ALL_MISSION_RESLIST
			g_FC.SetMainState( GAME_STATE_INIT_STAGE, ( DWORD )g_MapList[g_nCurMapListIndex] );
			g_BsResChecker.Create( g_MapList[g_nCurMapListIndex] );
			++g_nCurMapListIndex;
#endif
		}
		break;

	case GAME_STATE_INIT_STAGE:
		g_BsResChecker.Create( g_FCGameData.cMapFileName );

		//Special case for tutorial map...
#ifdef _XBOX
		if(strcmp(g_FCGameData.cMapFileName, _STAGE_TUTORIAL_FILENAME) == 0)
			SetLiveContext( LIVE_CONTEXT_MENU, LIVE_CONST_TUTORIAL );
		else
			SetLiveContext( LIVE_CONTEXT_MAP, g_FCGameData.cMapFileName );
#endif

		if( InitStage() == 1 )
		{
			SetMainState( GAME_STATE_LOOP_STAGE );
		}
		break;

	case GAME_STATE_LOOP_STAGE:

#ifdef MAKE_ALL_MISSION_RESLIST
		if( GetProcessTick() >= 1 )
		{
			g_FC.SetMainState( GAME_STATE_INIT_STAGE, ( DWORD )g_MapList[g_nCurMapListIndex] );
			g_BsResChecker.Create( g_MapList[g_nCurMapListIndex] );
			++g_nCurMapListIndex;

			if( g_nCurMapListIndex == ALL_MAP_NUM+1 )
			{
				XLaunchNewImage( XLAUNCH_KEYWORD_DEFAULT_APP, 0 );
			}
		}
#endif
		if(IsMissionEnd() == true){
			SetMainState(GAME_STATE_INIT_RESULT_SIGN);
		}
		else if(IsPauseMenu() == true)
		{
			SetMainState(GAME_STATE_INIT_PAUSE);
			InitPauseMenu();
		}
		else if(IsSpecialSelStageMenu() == true){
			SetMainState(GAME_STATE_INIT_NEXT_STAGE);
		}

		CheckActiveXUI();

		break;

		// pause menu
	case GAME_STATE_INIT_PAUSE:
		SetMainState(GAME_STATE_LOOP_PAUSE);
		break;

	case GAME_STATE_LOOP_PAUSE:
		if(IsPauseMenu() == false){	
			SetMainState(GAME_STATE_LOOP_STAGE);
		}
		break;

		//mission result sign (success/ failed)
	case GAME_STATE_INIT_RESULT_SIGN:
		if( InitGameResultSign() == true ){
			SetMainState(GAME_STATE_LOOP_RESULT_SIGN);
		}
		break;

	case GAME_STATE_LOOP_RESULT_SIGN:
		if(IsGameResultSign() == false){
			;
		}
		break;

		//mission result
	case GAME_STATE_INIT_RESULT:
		if( InitGameResult() == true ){
			SetMainState(GAME_STATE_LOOP_RESULT);
		}
		break;

	case GAME_STATE_LOOP_RESULT:
		if(IsGameResult() == false){
			;
		}
		break;

	case GAME_STATE_SYSTEM_MESSAGE:
		{
			;
		}
		break;
		/*
		case GAME_STATE_INIT_WORLDMAP:
		InitWorldMap();
		//GoToTitleorWorldMap
		break;

		case GAME_STATE_LOOP_WORLDMAP:
		break;
		*/

	case GAME_STATE_INIT_NEXT_STAGE:
		if( InitSpecialSelStageMenu() == true)
		{
			SetPause(true);
			SetMainState(GAME_STATE_LOOP_NEXT_STAGE);
		}
		break;

	case GAME_STATE_LOOP_NEXT_STAGE:
		if(IsSpecialSelStageMenu() == false)
		{
			SetPause(false);
			SetMainState(GAME_STATE_LOOP_STAGE);
		}
		break;

	default:
		BsAssert( 0 );
		break;
	}
}

void CFantasyCore::SetMainState( GAME_STATE State, DWORD dwParam1, DWORD dwParam2, DWORD dwParam3 )
{
	g_FCGameData.State = State;

	if(g_bIsStartTitleMenu)
	{
		switch(State)
		{
		case GAME_STATE_INIT_TITLE:
		case GAME_STATE_INIT_STAGE:
			{
				g_MenuHandle->RemoveAllMenu();
				break;
			}
		}
	}

	if( State == GAME_STATE_INIT_TITLE )
	{
		// true / false : go to title / go to select stage
		g_bGotoTitle = dwParam1 ? true : false;
		// show logo & movie / direct title
		g_FCGameData.bOpenLogo = dwParam2 ? true : false;
	}
	else if( State == GAME_STATE_INIT_STAGE )
	{
		if(dwParam1 != NULL)
		{
			BsAssert( strlen( (char*)dwParam1 ) < 64 );
			strcpy( g_FCGameData.cMapFileName, (char*)dwParam1 );
		}
	}
}


// Debug용
void CFantasyCore::CustomRender()
{
	g_FcWorld.DebugRender();

	if( g_FCGameData.bDebugRenderBaseObject == true ) {
		CFcBaseObject::DebugRenderObjects();
	}
	if( g_FCGameData.bDebugRenderTroopObject == true ) {
		CFcTroopObject::DebugRenderObjects();
	}
}



void CFantasyCore::SetPause( bool bPause )
{
	bool bBackup = g_bPause;
	if(bPause == true)
	{
		g_nPauseRef++;
		if(g_nPauseRef > 1){
			return;
		}
		QueryPerformanceCounter(&g_liStartPauseTime);		
	}
	else
	{
		g_nPauseRef--;
		if(g_nPauseRef > 0){
			return;
		}
		if(g_nPauseRef < 0){
			g_nPauseRef = 0;
			return;
		}
		LARGE_INTEGER liCurTime;
		QueryPerformanceCounter( &liCurTime );
		g_liStagePauseTime.QuadPart += ( liCurTime.QuadPart - g_liStartPauseTime.QuadPart );

	}

	if(g_bPause != bPause)
	{
		g_bPause = bPause;

		CInputPad::GetInstance().Enable( !bPause );		
		m_nPhysicsEnable = bPause ? 0 : 1;

		if( g_pSoundManager ){
			g_pSoundManager->SetPauseAll( bPause );
		}

		if(g_pFcRealMovie){
			g_pFcRealMovie->SetPause(bPause ? TRUE : FALSE);
		}
	}
	
	if( ( g_bPause == false ) && ( bBackup == true ) )
	{
		HeroObjHandle Hero;
		Hero = CFcWorld::GetInstance().GetHeroHandle( 0 );
		if( Hero ){
			Hero->Resume();
		}
	}
}

void CFantasyCore::SetPauseForXUI( bool bPause )
{
	if(IsPause() == bPause){
		return;
	}

	switch(g_FC.GetMainState())
	{
	case GAME_STATE_INIT_STAGE:
	case GAME_STATE_LOOP_STAGE:
	case GAME_STATE_SYSTEM_MESSAGE: break;
	default: return;
	}

	if( g_pFcRealMovie && g_pFcRealMovie->IsPlay() )
	{
		;
	}
	else if(IsPause() == true)
	{	
		//letter box작동 중에는 통과
		bool bLetterBoxOn = g_InterfaceManager.GetInstance().IsShowLetterBox();
		bool bFadeOn = g_InterfaceManager.GetInstance().IsFade();

		if(bLetterBoxOn == false && bFadeOn == false){
			return;
		}

		
	}
	else if(bPause == true){
		SetPausMenuforXboxGuide();
	}

	if(g_MenuHandle->IsSameMessageBox(fcMT_LossOfController, true)){
		return;
	}
	
	SetPause(bPause);
}

bool CFantasyCore::IsPause()
{
	return g_bPause;
}

void CFantasyCore::ReleaseStage()
{
	DestroyRenderThread();

	if(m_bReleaseTitleData)
	{
		if(g_hPackedResTitle != NULL)
		{
			CBsFileIO::BsDismountPackFile(g_hPackedResTitle);
			g_hPackedResTitle = NULL;
		}

		if( g_hPackedTexTitle != NULL )
		{
			CBsFileIO::BsDismountPackFile(g_hPackedTexTitle);
			g_hPackedTexTitle = NULL;
		}
	}

	if( g_hPackedTex1 )
	{
		CBsFileIO::BsDismountPackFile( g_hPackedTex1 );
		g_hPackedTex1 = NULL;
	}

	if( g_hPackedTex3 )
	{
		CBsFileIO::BsDismountPackFile( g_hPackedTex3 );
		g_hPackedTex3 = NULL;
	}

	if( g_hPackedCharTex )
	{
		CBsFileIO::BsDismountPackFile( g_hPackedCharTex );
		g_hPackedCharTex = NULL;
	}

	if( g_hPackedTex2 )
	{
		CBsFileIO::BsDismountPackFile( g_hPackedTex2 );
		g_hPackedTex2 = NULL;
	}
	
	if( g_pFcRealMovie && g_pFcRealMovie->IsPlay() ) {
		g_pFcRealMovie->DeleteContents();
	}

	g_pSoundManager->FinishStage();
	CInputPad::GetInstance().StopSave();
	g_InterfaceManager.GetInstance().FinishStage();
	g_BsKernel.ClearUIElemental();

	g_pFcFXManager->Reset();
	g_FcItemManager.FinishStage();
	CFcWorld::GetInstance().Clear();
	SAFE_DELETE( g_pFcRealMovie );
	g_BsKernel.GetPhysicsMgr()->Clear();	
	//CFcPhysicsLoader::ClearAll();
	CInputPad::GetInstance().ResetKeyBuffer();

#ifdef _XBOX
	g_nCaptureIndex = -1;
	g_nCaptureFolderIndex = -1;
#endif //_XBOX

	m_bFreeCam = NULL;
	SetPause( false );

	g_BsKernel.ResetFrame();
	g_BsKernel.ResetFrame();

	g_BsKernel.FlushFXTemplates();
	g_BsKernel.FlushParticleGroups();
	g_BsKernel.FlushSkins();
	g_BsKernel.FlushVertexDeclarations();
	g_BsKernel.FlushMaterials();
	g_BsKernel.FlushAnis();

	g_BsKernel.ResetFrame();
	g_BsKernel.ResetFrame();
	g_BsKernel.ResetDoubleBuffer();

	CFcPhysicsLoader::ClearAll();
}

void CFantasyCore::FreeCam()
{
	CameraObjHandle CamHandle;

	CamHandle = CFcBaseObject::GetCameraObjectHandle( 0 );
	if( CamHandle )
	{
		if( CamHandle->IsFreeCamMode() )
		{
			g_BsKernel.GetPhysicsMgr()->SetEnable( true );
			m_bFreeCam = false;
			CamHandle->SetFreeCamMode( m_bFreeCam );
			CInputPad::GetInstance().Enable( true );
		}
		else
		{
			g_BsKernel.GetPhysicsMgr()->SetEnable( false );
			m_bFreeCam = true;
			CamHandle->SetFreeCamMode( m_bFreeCam );
			CInputPad::GetInstance().Enable( false );
		}
	}
}

void CFantasyCore::StopFrameSkip()
{
	m_pSkip->Stop();
}

void CFantasyCore::StartFrameSkip()
{
	m_pSkip->Start();
}

void CFantasyCore::ProcessStartBack()
{
	if(g_bIsStartTitleMenu)
	{
		if(g_FCGameData.State == GAME_STATE_LOOP_STAGE || g_FCGameData.State == GAME_STATE_LOOP_PAUSE)
		{	
			//game이 시작되었을때, _WAIT_TICK_PAUSEMENU동안 대기
			if(g_nWaitTickPauseMenu > 0)
			{
				g_nWaitTickPauseMenu--;
				return;
			}

			if(g_InterfaceManager.IsShowLetterBox() == true){
				return;
			}
			if( g_pFcRealMovie && g_pFcRealMovie->IsPlay() == true ){
				return;
			}
			if(g_FCGameData.nSpecialSelStageState == SPECIAL_STAGE_MENU_ON){
				return;
			}
			if(!g_FcWorld.GetHeroHandle()){
				return;
			}
			if(g_FcWorld.GetHeroHandle()->GetHP() <= 0){
				return;
			}
			if(g_MenuHandle->GetCurMenuType() == _FC_MENU_MSGBOX){
				return;
			}
			if(IsEndingCharStage() == true){
				return;
			}


			if(g_InputPad.GetKeyPressTick(g_FCGameData.nEnablePadID, PAD_INPUT_START) == 1)
			{
				// pause on/off
				// It's not working on FadeIn, Out, and Start 1 second ago.
				if(g_MenuHandle->GetGamePauseMenu() == _FC_MENU_NONE &&
					g_MenuHandle->GetCurMenuType() == _FC_MENU_NONE &&
					IsPause() == false && g_InterfaceManager.IsFadeInStop() == true && GetProcessTick() > 40 )
				{
					g_MenuHandle->PostMessage(fcMSG_PAUSE_START);
				}
			}
			else if(g_InputPad.GetKeyPressTick(g_FCGameData.nEnablePadID, PAD_INPUT_BACK) == 1)
			{
				// status on/off
				if(g_MenuHandle->GetGamePauseMenu() == _FC_MENU_NONE &&
					g_MenuHandle->GetCurMenuType() == _FC_MENU_NONE &&
					IsPause() == false && g_InterfaceManager.IsFadeInStop() == true && GetProcessTick() > 40 )
				{	
					g_MenuHandle->PostMessage(fcMSG_STATUS_START);
				}
			}
		}
	}
	else
	{
		if( CInputPad::GetInstance().GetKeyPressTick(-1, PAD_INPUT_START) == 1 )
		{	
			SetPause(!IsPause());

			g_MenuHandle->PostMessage(fcMSG_DEBUG_MENU_TOGGLE);
		}

		if(g_InputPad.GetKeyPressTick(-1, PAD_INPUT_BACK) == 1)
		{
			InputKeyboard('`');
		}
	}
}


void CFantasyCore::SetPausMenuforXboxGuide()
{	
	if(g_FCGameData.State == GAME_STATE_LOOP_STAGE || g_FCGameData.State == GAME_STATE_LOOP_PAUSE)
	{	
		//game이 시작되었을때, _WAIT_TICK_PAUSEMENU동안 대기
		//STAGE_ID_TUTORIAL의 경우 대기 없이 바로 들어 가므로 제외
		if(g_nWaitTickPauseMenu > 0 && g_FCGameData.SelStageId	!= STAGE_ID_TUTORIAL)
		{
			g_nWaitTickPauseMenu--;
			return;
		}

		if(g_InterfaceManager.IsShowLetterBox() == true){
			return;
		}
		
		if(g_FCGameData.nSpecialSelStageState == SPECIAL_STAGE_MENU_ON){
			return;
		}
		if(!g_FcWorld.GetHeroHandle()){
			return;
		}
		if(g_FcWorld.GetHeroHandle()->GetHP() <= 0){
			return;
		}
		if(g_MenuHandle->GetCurMenuType() == _FC_MENU_MSGBOX){
			return;
		}
		if(IsEndingCharStage() == true){
			return;
		}

		if(g_MenuHandle->GetGamePauseMenu() != _FC_MENU_NONE){
			return;
		}
		
		if(g_MenuHandle->GetCurMenuType() != _FC_MENU_NONE){
			return;
		}
		
		if(IsPause() == false && g_InterfaceManager.IsFade() == false)
		{
			//STAGE_ID_TUTORIAL의 경우 대기 없이 바로 들어 가므로 제외
			if(GetProcessTick() > 40 || g_FCGameData.SelStageId	== STAGE_ID_TUTORIAL){
				g_MenuHandle->PostMessage(fcMSG_PAUSE_FOR_XBOXGUIDE_START);
			}
		}
	}
}

void CFantasyCore::SaveGlobalGameState()
{
	HeroObjHandle Handle = g_FcWorld.GetHeroHandle();
	BsAssert(Handle);

	StageResultInfo* pStageInfo = &(g_FCGameData.stageInfo);

	pStageInfo->nStageID = g_FCGameData.SelStageId;
	pStageInfo->nClearTime = g_FcWorld.GetStagePlayTime();
	pStageInfo->nKillCount = Handle->GetKillCount();
	pStageInfo->nNormalKillCount = pStageInfo->nKillCount - Handle->GetMaxOrbKillCount();
	pStageInfo->nOrbSparkKillCount = Handle->GetMaxOrbKillCount();
	pStageInfo->nGetOrbEXP = Handle->GetTotalOrbSpark();	
	pStageInfo->nMaxCombo = g_InterfaceManager.GetMaxComboCount();
	pStageInfo->nGuardianLiveRate = (int)g_FcWorld.GetFriendlyTroopsLiveRatePercent(); //(int)g_FcWorld.GetGuardianLiveRatePercent();
	pStageInfo->nHeroLevel = Handle->GetLevel();

	pStageInfo->nMissionClearCount = 0;
	pStageInfo->nMissionFaultCount = 0;

	pStageInfo->nGuardianOrbs = g_FcWorld.GetGuardianOrb();

	g_FCGameData.tempUserSave.nLevel = Handle->GetLevel();
	g_FCGameData.tempUserSave.nExp = Handle->GetEXP();
}

void	CFantasyCore::CreateLiveManager()
{
	if( !m_pLiveManager )
	{
		m_pLiveManager = new CFcLiveManager();
		m_pLiveManager->Initialize();
	}
}

void	CFantasyCore::SetLiveContext( int nCode, const char* szContext )
{
	if( m_pLiveManager )
	{
#ifdef _XBOX
		m_pLiveManager->SetRichPresenceInfo( nCode, szContext );
#endif
	}
}
void CFantasyCore::SaveLiveContext()
{
	if( m_pLiveManager )
	{
#ifdef _XBOX
		m_pLiveManager->SaveContext();
#endif
	}

}

void CFantasyCore::RestoreLiveContext()
{
	if( m_pLiveManager )
	{
#ifdef _XBOX
		m_pLiveManager->RestoreContext();
#endif
	}
}

void CFantasyCore::ProcessLive()
{
	if( m_pLiveManager )
	{	
		m_pLiveManager->Process();
	}
}


bool CFantasyCore::IsEndingCharStage()
{
	switch(g_FCGameData.SelStageId)
	{
	case STAGE_ID_ENDING_MYIFEE:
	case STAGE_ID_ENDING_DWINGVATT:
	case STAGE_ID_ENDING_VIGKVAGK:
	case STAGE_ID_ENDING_MA_INPHYY:	return true;
	}

	return false;
}


static CBsCriticalSection s_csDiskErrorLock;

#ifdef _XBOX
void CFantasyCore::DirtyDiskHandlerCallBackFunc( void )
{
	THREAD_AUTOLOCK(&s_csDiskErrorLock);

	DWORD dwCurrentThreadID = ::GetCurrentThreadId();
	DWORD dwCurrentD3DThreadID = g_BsKernel.GetD3DDevice()->QueryThreadOwnership();

	// Need to acquire D3D thread
	if( dwCurrentD3DThreadID != dwCurrentThreadID)
	{
		if(dwCurrentD3DThreadID == g_dwRenderThreadId)
		{
			g_FC.DestroyRenderThread();
		}
		else
		{
			g_BsKernel.GetDevice()->ReleaseDeviceNextShow();
			::WaitForSingleObject(g_hReleaseEvent, INFINITE);
			g_BsKernel.GetDevice()->AcquireD3DThreadOwnership();
		}
	}

	// Shut down other threads
	if(g_hRenderThread && dwCurrentThreadID != g_dwRenderThreadId)
		::SuspendThread(g_hRenderThread);
	if(g_hNewMainThread && dwCurrentThreadID != g_dwNewMainThreadId)
		::SuspendThread(g_hNewMainThread);
	if(g_hStartupThread && dwCurrentThreadID != g_dwStartupThreadId)
		::SuspendThread(g_hStartupThread);
	if(CTimeRender::ms_hRenderProgressThread && dwCurrentThreadID != CTimeRender::ms_dwRenderProgressThreadID)
		::SuspendThread(CTimeRender::ms_hRenderProgressThread);

	// Turn off all sound
	if (g_pSoundManager)
	{
		g_pSoundManager->StopSoundAll();
	}

	g_BsKernel.GetD3DDevice()->Suspend();

	DWORD dwUser = 0;
	for( ; dwUser < XUSER_MAX_COUNT; dwUser++ )
	{
		XUSER_SIGNIN_STATE State = XUserGetSigninState( dwUser );
		if( State != eXUserSigninState_NotSignedIn )
			break;
	}
	dwUser = dwUser == XUSER_MAX_COUNT ? 0 : dwUser;
	XShowDirtyDiscErrorUI(dwUser);
}
#endif

//_TNT_
#ifdef _TNT_
//JKB*************************************************************************
// Author:      rbonny - STOLEN BY JKBURNS!!!
// Created:     03/01/2005
// Abstract:    This is the central XeCR "signal" handler.  The 
//              signal mechanism requires a callback function.  The following 
//              example demonstrates a classic pattern for using a static 
//              method on a class as a function pointer.  The class instance 
//              can be passed to the callback as a void* pointer and 
//              cast to an instance pointer.  Since the static method is 
//              inside the class we can reference private and protected 
//              class members and methods through the pointer.
//
//				NOTE: this should spawn a separate thread to handle asynchronously,
//                    haven't implemented this yet...
//*************************************************************************
void CFantasyCore::HandleSignal(LPCSTR message, PVOID data)
{

	__try
	{
		CFantasyCore* pthis = (CFantasyCore*) data;

		MultiByteToWideChar( CP_ACP,          // ANSI code page
			0,               // No flags
			message,         // Character to convert
			-1,              // Convert entire null terminated string
			pthis->g_signalMessage, // Target buffer,
			256 );           // Size of target buffer

		// This might return an error code if an improper channel is specified,
		// but we don't really care
		// DmSendNotificationString(message);

		if (0 == lstrcmpiA(message, "clearmenus"))
		{
			pthis->HandleClearMenus();
		}
		else if (0 == lstrcmpiA(message, "debugmenu"))
		{
			pthis->HandleDebugMenu();
		}
		else if (0 == lstrcmpiA(message, "closestenemy"))
		{
			pthis->HandleClosestEnemy();
		}
		else if(0 == strncmp(message, "loadmap", 7))
		{
			pthis->HandleLoadMap(message);
		}
		else if(0 == strncmp(message, "changehero",10))
		{
			pthis->HandleChangeHero(message);
		}
		else if (0 == lstrcmpiA(message, "setinvincible"))
		{
			pthis->HandleSetInvincible();
		}
		else if (0 == lstrcmpiA(message, "getheroloc"))
		{
			pthis->HandleGetLoc();
		}
		else if (0 == strncmp(message, "warptospawntrigger", 18))
		{
			pthis->HandleWarpToTriggerPoint(message);
		}
		else if (0 == strncmp(message, "crashtest", 18))
		{
			throw -1;
		}
		else
		{
#ifndef _LTCG
			DmSendNotificationString("Handle signal was passed an invalid command!");
#endif
		}
	}
	__except(1)
	{
		//	This __try __except block is to prevent the debug channel from hanging should an exception be thrown.
		//throw -1;
	}



}

//JKB*************************************************************************
// Author:      jkburns
// Created:     06/17/2005
// Abstract:    hook to kick off debug menu.
//*************************************************************************
void CFantasyCore::HandleDebugMenu()
{
	if (g_MenuHandle)
		g_MenuHandle->PostMessage(fcMSG_DEBUG_MENU_START);
}

//JKB*************************************************************************
// Author:      jkburns
// Created:     06/17/2005
// Abstract:    hook to close all menus.
//*************************************************************************
void CFantasyCore::HandleClearMenus()
{
	if (g_MenuHandle)
		g_MenuHandle->PostMessage(fcMSG_REMOVE_ALL_MENU);
}

//JKB*************************************************************************
// Author:      jkburns
// Created:     06/17/2005
// Abstract:    hook to warp player 0 to nearest enemy.  Uses added hook "HeroWarp"
//              in CFcHeroObject that I added...
//*************************************************************************
void CFantasyCore::HandleClosestEnemy()
{
	if (&g_FcWorld)
		g_FcWorld.WarpHeroToEnemy(0, 30000.0f);
}

//CLL*************************************************************************
// Author:      chrilee
// Created:     8/30/2005
// Abstract:    hook to Load a map given the map name
//
//*************************************************************************
void CFantasyCore::HandleLoadMap(LPCSTR message)
{
	// The Incoming message should look like: "loadmap_EV-AS_1.bsmap" spaces will break
	//		the XeCR signal processor

	//Clear Menus
	if (g_MenuHandle)
		g_MenuHandle->PostMessage(fcMSG_REMOVE_ALL_MENU);

	//Get the map file name
	char fileName[256];
	strcpy(fileName, message+8);

	//Set the gamestate
	g_FC.SetMainState( GAME_STATE_INIT_STAGE, ( DWORD )fileName );

	return;
}

//CLL*************************************************************************
// Author:      chrilee
// Created:     8/31/2005
// Abstract:    hook to change player 1's hero
//
//*************************************************************************
void CFantasyCore::HandleChangeHero(LPCSTR message)
{
	// The incoming message should look like: "changehero_1" spaces will break the xecr
	//		signal processor

	g_FcWorld.GetInstance().CreateHero(0, atoi(message+11), 0);

	return;
}
//JKB*************************************************************************
// Author:      jkburns
// Created:     09/11/2005
// Abstract:    hook to call set invincible hook
//*************************************************************************
void CFantasyCore::HandleSetInvincible()
{
	if (&g_FcWorld)
	{
		HeroObjHandle hero = g_FcWorld.GetHeroHandle();
		if (hero)
			hero->SetInvincible(true);
	}
}
//JKB*************************************************************************
// Author:      jkburns
// Created:     09/11/2005
// Abstract:    hook to get current coordinates
//*************************************************************************
void CFantasyCore::HandleGetLoc()
{
	if (&g_FcWorld)
	{
		HeroObjHandle hero = g_FcWorld.GetHeroHandle();
		if (hero)
		{
			D3DXVECTOR2 heroPos = hero->GetPosV2();
			char xBuffer[50];
			_gcvt((double)heroPos.x,10,xBuffer);
			char yBuffer[50];
			_gcvt((double)heroPos.y,10,yBuffer);

			OutputDebugString("Hero Position (x,y) = (");
			OutputDebugString(xBuffer);
			OutputDebugString(",");
			OutputDebugString(yBuffer);
			OutputDebugString(").");
		}		
	}
}
//JKB*************************************************************************
// Author:      jkburns
// Created:     09/11/2005
// Abstract:    warp to map area that triggers enemy spawns
//*************************************************************************
void CFantasyCore::HandleWarpToTriggerPoint(LPCSTR message)
{
	if (&g_FcWorld)
	{
		//Get the int troop value
		char strTroopNum[50];
		strTroopNum[0] = '\0';
		strcpy(strTroopNum, message+19);

		int troopNum = 0;
		if (strTroopNum)
			troopNum = atoi(strTroopNum);
		else return;

		CBSMapCore* pMapInfo = g_FcWorld.GetMapInfo();
		if (pMapInfo)
		{
			if (troopNum >= 0 && troopNum < pMapInfo->GetTroopCount())
			{
				AREA_PACK * pAreaInfo = pMapInfo->GetAreaPack(troopNum);
				//TROOP_INFO* pTroopInfo = pMapInfo->GetTroopInfo( troopNum );

				HeroObjHandle hero = g_FcWorld.GetHeroHandle();
				if (hero)
					//hero->HeroWarp(pTroopInfo->m_areaInfo.fSX + 1.0f,pTroopInfo->m_areaInfo.fSZ + 1.0f);
					hero->HeroWarp(pAreaInfo->m_areaInfo.fSX + 1.0f,pAreaInfo->m_areaInfo.fSZ + 1.0f);
			}
		}
	}
}


#endif //_TNT_
