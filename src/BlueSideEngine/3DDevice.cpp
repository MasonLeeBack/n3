#include "stdafx.h"
#include "3DDevice.h"
#include "BsCommon.h"
#include "BStreamExt.h"
#include "BsConsole.h"
#include "BsKernel.h"
#include "BsFileIO.h"
#include "zlib\zlibdecode.h"

#include "MGSToolBox\MGSToolBox.h"

//#define _NV_PERFORMANCE_
#define _RENDER_DEBUG					// Draw(), Primitive Count Display!!!
//#define _SHADER_DEBUG
//#define _REF

LPDIRECT3DDEVICE9     g_pd3dDevice = NULL;

//#define _TIME_TEXTURE_LOADING
static float fTimeTotal = 0.f;
#ifdef _TIME_TEXTURE_LOADING
class TextureLoadTimer
{
public:
	TextureLoadTimer( const char *pszOPString, float *pfCumulativeTime )
	{
		BsAssert(pszOPString);
		memset( m_pszOPString, 0 , MAX_PATH);
		strcpy(m_pszOPString, pszOPString);
		QueryPerformanceCounter( &m_startTime );
		if(pfCumulativeTime)
			m_pfCumulativeTime = pfCumulativeTime;
		else
			m_pfCumulativeTime = NULL;
		if(!m_bInited)
		{
			LARGE_INTEGER TicksPerSecond;
			QueryPerformanceFrequency( &TicksPerSecond );
			m_fTicksPerMillisecond = (float)TicksPerSecond.QuadPart * 0.001f;
			m_bInited = true;
		}
	}
	~TextureLoadTimer()
	{
		char szBuffer[MAX_PATH*2];
		LARGE_INTEGER CurrentTime;
		QueryPerformanceCounter( &CurrentTime );
		float fTicks = (CurrentTime.QuadPart - m_startTime.QuadPart) / m_fTicksPerMillisecond;
		if(m_pfCumulativeTime)
		{
			*m_pfCumulativeTime+=fTicks;
			sprintf(szBuffer,"Total: %6f :%s took %f ms\n", fTimeTotal, m_pszOPString, fTicks);
		}
		else
		{
			sprintf(szBuffer,"%s took %f ms\n", m_pszOPString, fTicks);
		}
		OutputDebugString(szBuffer);
	}
protected:
	LARGE_INTEGER m_startTime;
	static bool m_bInited;
	static float m_fTicksPerMillisecond;
	char m_pszOPString[MAX_PATH];
	float * m_pfCumulativeTime;
private:
};

bool TextureLoadTimer::m_bInited =false;
float TextureLoadTimer::m_fTicksPerMillisecond = 0.f;

#define TIME_TEXTURE_LOAD(str, ptot) TextureLoadTimer TLT(str, ptot)
#else
#define TIME_TEXTURE_LOAD(str, tot) ((VOID)(str, tot))
#endif

static struct BsMaterialInclude : public ID3DXInclude
{
#if _XDK_VER < 2571
	STDMETHOD(Open)(THIS_ D3DXINCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID *ppData, UINT *pBytes) 
#else
	STDMETHOD(Open)(THIS_ D3DXINCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID *ppData, UINT *pBytes,
        LPSTR pFullPath, DWORD cbFullPath )
#endif
	{
		char szFullPath[ MAX_PATH ];
		sprintf(szFullPath, "%s%s", g_BsKernel.GetShaderDirectory(), pFileName);
		BFileStream Stream( szFullPath );
		if(!Stream.Valid()){
			BsAssert( 0 && "Invalid File Open!!" );
			return S_FALSE;
		}
		int nLength = Stream.Length();
// [PREFIX:beginmodify] 2006/2/16 junyash PS#5163 reports BsMaterialInclude::Open() illegal value use of malloc().
		//VOID* pCode = malloc( nLength);
		VOID* pCode = malloc( static_cast<size_t>(nLength) );
// [PREFIX:endmodify] junyash
		Stream.Read(pCode, nLength );
		*ppData = pCode;
		*pBytes = nLength;
		return S_OK;
	}
	STDMETHOD(Close)(THIS_ LPCVOID pData) 
	{
		free( (LPVOID)pData );
		return S_OK;
	}
} s_MaterialInclude;

static struct BsMaterialIncludeLocal : public ID3DXInclude
{
#if _XDK_VER < 2571
	STDMETHOD(Open)(THIS_ D3DXINCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID *ppData, UINT *pBytes) 
#else
	STDMETHOD(Open)(THIS_ D3DXINCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID *ppData, UINT *pBytes,
		LPSTR pFullPath, DWORD cbFullPath )
#endif
	{
		BFileStream Stream( pFileName );
		if(!Stream.Valid()){
			BsAssert( 0 && "Invalid File Open!!" );
			return S_FALSE;
		}
		int nLength = Stream.Length();
// [PREFIX:beginmodify] 2006/2/16 junyash PS#5164 reports BsMaterialInclude::Open() illegal value use of malloc().
		//VOID* pCode = malloc( nLength);
		VOID* pCode = malloc( static_cast<size_t>(nLength) );
// [PREFIX:endmodify] junyash
		Stream.Read(pCode, nLength );
		*ppData = pCode;
		*pBytes = nLength;
		return S_OK;
	}
	STDMETHOD(Close)(THIS_ LPCVOID pData) 
	{
		free( (LPVOID)pData );
		return S_OK;
	}
} s_MaterialIncludeLocal;


C3DDevice::C3DDevice()
{
	m_pD3D=NULL;
	m_pD3DDevice=NULL;
	m_pBackBuffer=NULL;
	m_pDepthBuffer=NULL;
	m_pDepthBufferRTT=NULL;
	m_nCaptureCount=0;
	m_bReleaseDevice=false;
	m_hReleaseEvent=NULL;

	int i;
	for(i=0;i<MAX_STATE_SAVE;i++){
		m_pSaveStateToken[i]=NULL;
	}
	m_dwAlphaRefValue=0x7f;

	m_fGamma = 0.5f;

	m_dwDrawCallCount = 0;
	m_dwDrawPrimitiveCount = 0;

	m_pDefaultVertexShader = NULL;
	m_pDefaultPixelShader  = NULL;
	m_pDefaultVertexDeclaration  = NULL;
}

C3DDevice::~C3DDevice()
{
	if ( m_pD3DDevice )
		MGSToolbox::Remove();
		
	int i;

	for(i=0;i<MAX_STATE_SAVE;i++){
		SAFE_RELEASE(m_pSaveStateToken[i]);
	}

	SAFE_RELEASE(m_pDepthBufferRTT);
	SAFE_RELEASE(m_pDepthBuffer);
	SAFE_RELEASE(m_pBackBuffer);

	SAFE_RELEASE(m_pDefaultVertexShader);
	SAFE_RELEASE(m_pDefaultPixelShader);
	SAFE_RELEASE(m_pDefaultVertexDeclaration);

	if(m_pD3DDevice){
		int nRefCount = m_pD3DDevice->Release();
		m_pD3DDevice=NULL;
	}
	SAFE_RELEASE(m_pD3D);

	if(m_hReleaseEvent)
		CloseHandle(m_hReleaseEvent);
}

#ifdef _XBOX
//-------------------------------------------------------------------------------------
// Name: Initialize(int nWidth, int nHeight)
// Desc: Initializes Direct3D => Support 480mode and 720mode
//-------------------------------------------------------------------------------------
HRESULT C3DDevice::Initialize()
{
	// Create the D3D object.
	LPDIRECT3D9 m_pD3D = Direct3DCreate9( D3D_SDK_VERSION );

	// Set up the structure used to create the D3DDevice.
	ZeroMemory( &m_Param, sizeof(m_Param) );

#if _XDK_VER < 1838
	m_Param.BackBufferWidth        = ( XGetVideoFlags() & XC_VIDEO_FLAGS_HDTV_720p ) ? 1280 : 640;
	m_Param.BackBufferHeight       = ( XGetVideoFlags() & XC_VIDEO_FLAGS_HDTV_720p ) ?  720 : 480;
#else
	XVIDEO_MODE vidmode;
	XGetVideoMode(&vidmode);

	m_Param.BackBufferWidth        = 1280;
	m_Param.BackBufferHeight       = 720;
#endif

	m_Param.BackBufferFormat       = D3DFMT_A8R8G8B8;
	m_Param.BackBufferCount        = 1;
	m_Param.EnableAutoDepthStencil = TRUE;

	m_Param.AutoDepthStencilFormat = D3DFMT_D24FS8;
	m_Param.SwapEffect             = D3DSWAPEFFECT_DISCARD;
	m_Param.PresentationInterval   = D3DPRESENT_INTERVAL_IMMEDIATE;

	m_nBackBufferWidth = m_Param.BackBufferWidth;
	m_nBackBufferHeight = m_Param.BackBufferHeight;

	// Create the Direct3D device.
	if( FAILED( m_pD3D->CreateDevice( 0, D3DDEVTYPE_HAL, NULL, D3DCREATE_BUFFER_2_FRAMES, &m_Param, &m_pD3DDevice ) ) ) {
		BsAssert( 0 && "CreateDevice() Failed!!" );
		return E_FAIL;
	}

	// TODO	:	Predication Tiling사용시 아주 많은 Draw 호출되면 Ring Buffer Out of Memory 상태로 되는것 방지
	//			Predication Tiling Restrict 참조
	D3DRING_BUFFER_PARAMETERS ringbuffer_param;
	ringbuffer_param.Flags = 0;
	ringbuffer_param.PrimarySize = 64 * 1024; 
	ringbuffer_param.pPrimary = XPhysicalAlloc( 64 * 1024, MAXULONG_PTR, GPU_COMMAND_BUFFER_ALIGNMENT,
		PAGE_READWRITE | PAGE_WRITECOMBINE | MEM_LARGE_PAGES );
	ringbuffer_param.SecondarySize = 4 * 1024 * 1024;
	ringbuffer_param.pSecondary = XPhysicalAlloc( 4 * 1024 * 1024, MAXULONG_PTR, GPU_COMMAND_BUFFER_ALIGNMENT,
		PAGE_READWRITE | PAGE_WRITECOMBINE | MEM_LARGE_PAGES );
	ringbuffer_param.SegmentCount = 64;
	HRESULT hr = m_pD3DDevice->SetRingBufferParameters(&ringbuffer_param);
	if(hr!=S_OK) {
		BsAssert( 0 && "Failed SetRingBufferParamters()!!");
	}

	g_pd3dDevice = m_pD3DDevice;

	m_pD3DDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &m_pBackBuffer);
	m_pD3DDevice->GetDepthStencilSurface(&m_pDepthBuffer);
	m_pD3DDevice->GetDepthStencilSurface(&m_pDepthBufferRTT);

	InitDefaultState();

	ClearBuffer(0x00000000);

	m_pD3DDevice->Present(NULL, NULL, NULL, NULL);	

	m_ViewPort.X = 0;
	m_ViewPort.Y = 0;
	m_ViewPort.Width = 1280;
	m_ViewPort.Height = 720;
#ifdef INV_Z_TRANSFORM
	m_ViewPort.MinZ = 1.f;
	m_ViewPort.MaxZ = 0.f;
#else
	m_ViewPort.MinZ = 0.f;
	m_ViewPort.MaxZ = 1.f;
#endif
	CreateDefaultShader();

	MGSToolbox::Install( m_pD3DDevice );

	m_hReleaseEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
#ifdef _XBOX
#endif


	return S_OK;
}

HRESULT C3DDevice::Reset(D3DPRESENT_PARAMETERS* pPresentationParameters)
{
	SAFE_RELEASE(m_pBackBuffer);
	SAFE_RELEASE(m_pDepthBuffer);
	HRESULT hr=m_pD3DDevice->Reset(pPresentationParameters);
	m_pD3DDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &m_pBackBuffer);
	m_pD3DDevice->GetDepthStencilSurface(&m_pDepthBuffer);
	m_Param=*pPresentationParameters;

	InitDefaultState();

	m_pD3DDevice->GetViewport( &m_ViewPort );

	return hr;
}
#else	// For PC
HRESULT C3DDevice::Initialize(HWND hWnd, int nWidth, int nHeight, bool bIsWindowed, bool bMultiThreaded/* = false*/)
{
	HRESULT hr;
	D3DDISPLAYMODE D3DDM;

	m_nBackBufferWidth=nWidth;
	m_nBackBufferHeight=nHeight;
	m_pD3D=Direct3DCreate9(D3D_SDK_VERSION);

	if(FAILED(m_pD3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &D3DDM))){
		return E_FAIL;
	}
	// Xenon에 매칭하기 위해 백버퍼 포맷으로 D3DFMT_A8R8G8B8을 사용합니다.
	D3DDM.Format = D3DFMT_A8R8G8B8;
	memset(&m_Param, 0, sizeof(D3DPRESENT_PARAMETERS));
	//	m_Param.MultiSampleType = D3DMULTISAMPLE_4_SAMPLES;
	m_Param.Windowed=bIsWindowed;
	m_Param.BackBufferWidth=m_nBackBufferWidth;
	m_Param.BackBufferHeight=m_nBackBufferHeight;
	m_Param.BackBufferFormat=D3DDM.Format;
	m_Param.BackBufferCount=1;
	m_Param.EnableAutoDepthStencil=true;
	m_Param.AutoDepthStencilFormat=D3DFMT_D24S8;
	m_Param.SwapEffect=D3DSWAPEFFECT_DISCARD;
	if( bIsWindowed ) {
		m_Param.FullScreen_RefreshRateInHz = 0;
		m_Param.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
	} else {
		m_Param.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
		m_Param.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
	}

	DWORD mulithreaded = bMultiThreaded ? D3DCREATE_MULTITHREADED : 0;

#ifdef _REF
	if(FAILED(m_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_REF, hWnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING | mulithreaded,
		&m_Param, &m_pD3DDevice)))
		return E_FAIL;
#else

#ifdef _NV_PERFORMANCE_
	if(FAILED(m_pD3D->CreateDevice( m_pD3D->GetAdapterCount()-1, D3DDEVTYPE_REF, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING,
		&m_Param, &m_pD3DDevice))){
			return E_FAIL;
		}
#else
	hr = m_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_MIXED_VERTEXPROCESSING | mulithreaded, &m_Param, &m_pD3DDevice);
	if(hr != D3D_OK ){
		BsAssert( 0 && "Failed CreateDevice()" );
		if(FAILED(m_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING | mulithreaded, &m_Param, &m_pD3DDevice))) {
			BsAssert( 0 && "Failed CreateDevice!!" );
			return E_FAIL;
		}
	}
#endif

#endif

	m_pD3DDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &m_pBackBuffer);
	m_pD3DDevice->GetDepthStencilSurface(&m_pDepthBuffer);
	m_pD3DDevice->GetDepthStencilSurface(&m_pDepthBufferRTT);

	InitDefaultState();

	m_pD3DDevice->GetViewport( &m_ViewPort );

	return S_OK;
}
#endif

void C3DDevice::InitDefaultState()
{
	int i;
	D3DCOLORVALUE Diffuse={1.0f, 1.0f, 1.0f, 1.0f};
	D3DCOLORVALUE Specular={1.0f, 1.0f, 1.0f, 1.0f};
	D3DCOLORVALUE Ambient={0.4f, 0.4f, 0.4f, 1.0f};
	D3DXVECTOR3 Light(0.0f, 0.0f, 1.0f);

	m_pD3DDevice->SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE);
	m_pD3DDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
#ifndef _XBOX
	m_pD3DDevice->SetRenderState(D3DRS_AMBIENT, D3DCOLOR_COLORVALUE(0.0f, 0.0f, 0.0f, 1.0f));
	m_pD3DDevice->SetRenderState(D3DRS_LIGHTING, true);
	m_pD3DDevice->SetRenderState(D3DRS_FOGVERTEXMODE,   D3DFOG_LINEAR);
#endif

	for(i=0;i<MAX_TEXTURE_STAGE;i++){
		m_pD3DDevice->SetSamplerState(i, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
		m_pD3DDevice->SetSamplerState(i, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
		m_pD3DDevice->SetSamplerState(i, D3DSAMP_ADDRESSW, D3DTADDRESS_WRAP);

		m_pD3DDevice->SetSamplerState(i, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
		m_pD3DDevice->SetSamplerState(i, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
		m_pD3DDevice->SetSamplerState(i, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
	}

	m_pD3DDevice->SetRenderState( D3DRS_ALPHATESTENABLE, true);
	m_pD3DDevice->SetRenderState( D3DRS_ALPHAREF, m_dwAlphaRefValue);
	m_pD3DDevice->SetRenderState( D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL);

#ifdef _XBOX
	D3DXSetDXT3DXT5(TRUE);
	m_pD3DDevice->SetRenderState( D3DRS_PRIMITIVERESETENABLE, TRUE );
#endif

#ifdef INV_Z_TRANSFORM
	m_pD3DDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_GREATEREQUAL);
#endif

	for(i=0;i<MAX_STATE_SAVE;i++){
		m_pD3DDevice->CreateStateBlock(D3DSBT_ALL, &(m_pSaveStateToken[i]));
	}
}

//------------------------------------------------------------------------------------
//	Screen Clear Function
//  D3DRECT를 이용한 부분 Clear지원필요!!
//------------------------------------------------------------------------------------
void C3DDevice::ClearBuffer(D3DCOLOR COLOR, float fZ /*= 1.f or 0.f*/, DWORD dwStencil /*= 0*/)
{
#ifdef _XBOX
	D3DVECTOR4 vecColor;
	vecColor.x = float(D3DCOLOR_GETRED(COLOR))/255.f;
	vecColor.y = float(D3DCOLOR_GETGREEN(COLOR))/255.f;
	vecColor.z = float(D3DCOLOR_GETBLUE(COLOR))/255.f;
	vecColor.w = float(D3DCOLOR_GETALPHA(COLOR))/255.f;
	HRESULT hr = m_pD3DDevice->ClearF(D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL, NULL, &vecColor, fZ, dwStencil);
#else
	HRESULT hr = m_pD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL, COLOR, fZ, dwStencil);
#endif
	if(hr != D3D_OK) {
		BsAssert( 0 && "Clear Buffer Error!!");
	}
}

void C3DDevice::ClearTarget(D3DCOLOR COLOR)
{
#ifdef _XBOX
	D3DVECTOR4 vecColor;
	vecColor.x = float(D3DCOLOR_GETRED(COLOR))/255.f;
	vecColor.y = float(D3DCOLOR_GETGREEN(COLOR))/255.f;
	vecColor.z = float(D3DCOLOR_GETBLUE(COLOR))/255.f;
	vecColor.w = float(D3DCOLOR_GETALPHA(COLOR))/255.f;
	m_pD3DDevice->ClearF( D3DCLEAR_TARGET, NULL, &vecColor, 1.0f, 0 );
#else
	m_pD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET, COLOR, 1.0f, 0 );
#endif
}

void C3DDevice::ClearZStencilBuffer(float fZ, DWORD dwStencil)
{
#ifdef _XBOX
	m_pD3DDevice->ClearF( D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL, NULL, 0, fZ, dwStencil);
#else
	m_pD3DDevice->Clear(0, NULL, D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL, 0, fZ, dwStencil);
#endif
}

void C3DDevice::SaveFrontBuffer(char* szFileName)
{
#ifdef _XBOX
	LPDIRECT3DBASETEXTURE9 pFrontBuffer = NULL;
	m_pD3DDevice->GetFrontBuffer(&pFrontBuffer);
	D3DXSaveTextureToFile(szFileName, D3DXIFF_BMP, pFrontBuffer, NULL);
	SAFE_RELEASE(pFrontBuffer);
#else
	LPDIRECT3DSURFACE9 pSurface = NULL;
	m_pD3DDevice->GetFrontBufferData(0, pSurface);
	D3DXSaveSurfaceToFile(szFileName, D3DXIFF_BMP, pSurface, NULL, NULL);
	SAFE_RELEASE(pSurface);
#endif
}

void C3DDevice::SaveBackBuffer(char* szFileName)
{
	D3DXSaveSurfaceToFile(szFileName, D3DXIFF_BMP, GetBackBuffer(), NULL, NULL);
}

void C3DDevice::SetViewport( const D3DVIEWPORT9* pViewport )
{	
	memcpy( &m_ViewPort, pViewport, sizeof( D3DVIEWPORT9 ) );
	m_pD3DDevice->SetViewport( &m_ViewPort );	
}

void C3DDevice::GetViewport( D3DVIEWPORT9* pViewport )
{	
	memcpy( pViewport, &m_ViewPort, sizeof( D3DVIEWPORT9 ) );
}

void C3DDevice::SaveState()
{
	if(m_nCaptureCount>MAX_STATE_SAVE){
		BsAssert(0);
		return;
	}
	m_pSaveStateToken[m_nCaptureCount]->Capture();
	m_nCaptureCount++;
}

void C3DDevice::RestoreState()
{
	m_nCaptureCount--;
	m_pSaveStateToken[m_nCaptureCount]->Apply();
}

void C3DDevice::ShowFrame()
{	
	MGSToolbox::EndFrame();
	m_pD3DDevice->Present(NULL, NULL, NULL, NULL);
#ifdef _XBOX
	if(m_bReleaseDevice)
	{	// A fatal error has occurred so release the device, signal
		// the release and then sleep, sleep, sleep........
		m_pD3DDevice->BlockUntilIdle();
		ReleaseD3DThreadOwnership();
		SetEvent(m_hReleaseEvent);
		Sleep(INFINITE);
	}
#endif
	MGSToolbox::BeginFrame();
}

void C3DDevice::SetGammaRamp(float fGamma)
{
#ifdef _XBOX
	m_fGamma = fGamma;
	D3DGAMMARAMP ramp;
	float fMappedValue = m_fGamma + 0.5f;
	const float fDefaultGradient = 65472.f/255.f;
	float fResultGradient = fDefaultGradient*fMappedValue;
	for(int i=0;i<256;++i) {
		ramp.red[i]		= BsMin(int(fResultGradient*float(i)), 65472);
		ramp.green[i]	= BsMin(int(fResultGradient*float(i)), 65472);
		ramp.blue[i]	= BsMin(int(fResultGradient*float(i)), 65472);
	}
	m_pD3DDevice->SetGammaRamp(0, D3DSGR_IMMEDIATE, &ramp);
#endif
}

HRESULT C3DDevice::CreateEffectFromFile(const char* pszFileName, LPD3DXEFFECT* ppEffect, LPD3DXEFFECTPOOL pEffectPool/*=NULL*/)
{
	DWORD dwFlags = NULL;
	LPD3DXBUFFER	pError = NULL;
#ifdef _SHADER_DEBUG
	dwFlags |= D3DXSHADER_DEBUG;
	dwFlags |= D3DXSHADER_SKIPOPTIMIZATION;
#endif
	DWORD dwFileSize;
	VOID * pData;
	HRESULT hr;

	if(!FAILED( hr = CBsFileIO::LoadFile(pszFileName, &pData, &dwFileSize) ) )
	{		
		hr = D3DXCreateEffect( m_pD3DDevice, pData, dwFileSize, NULL, &s_MaterialInclude, dwFlags, pEffectPool, ppEffect, &pError );
		if( FAILED(hr)) {
			char *pErrorMsg = (char *)pError->GetBufferPointer();
			_com_error err(hr);
			LPCTSTR errMsg = err.ErrorMessage();
			char stream[256];
			sprintf(stream, "%s\n\n%s (0x%08X)\n\n%s", pszFileName, errMsg, hr, pErrorMsg);
			MessageBox(NULL, stream, "BlueSideEngine Shader Failure", MB_ICONERROR | MB_OK);
			BsAssert( 0 && "D3DXEffect Compile Error!!");
		}
		CBsFileIO::FreeBuffer(pData);
	}

	return hr;
}

#ifdef _XBOX
HRESULT C3DDevice::CreateFXLEffectFromFile(const char* pszFileName, FXLEffect** ppEffect, FXLEffectPool* pEffectPool/*=NULL*/)
{
	DWORD dwFileSize;
	VOID * pData;

	if(FAILED( CBsFileIO::LoadFile(pszFileName, &pData, &dwFileSize) ) )
	{
		return E_FAIL;
	}

	// Compile an effect
	LPD3DXBUFFER pEffectData = NULL;
	LPD3DXBUFFER pErrorList = NULL;

	if(FAILED(FXLCompileEffect((CHAR*)pData, dwFileSize, NULL, &s_MaterialInclude, NULL, &pEffectData, &pErrorList))) {
		DebugString("%s Compile Error : %s\n", pszFileName, (const char*)pErrorList->GetBufferPointer());
		BsAssert( 0 && "FXLEffect Compile Error!!" );
		SAFE_RELEASE( pEffectData );
		SAFE_RELEASE( pErrorList );
		CBsFileIO::FreeBuffer(pData);
		return E_FAIL;
	}
	// Create effect
	if( FAILED( FXLCreateEffect( m_pD3DDevice, pEffectData->GetBufferPointer(), pEffectPool, ppEffect))) {
		BsAssert( 0 && "Couldn't create effect\n" );
		SAFE_RELEASE( pEffectData );
		SAFE_RELEASE( pErrorList );
		CBsFileIO::FreeBuffer(pData);
		return E_FAIL;
	}
	CBsFileIO::FreeBuffer(pData);
	SAFE_RELEASE( pEffectData );
	SAFE_RELEASE( pErrorList );
	return D3D_OK;
}

HRESULT C3DDevice::CompileFXLEffectFromFile(const char* pszFileName, LPD3DXBUFFER *pEffectData )
{
	DWORD dwFileSize;
	VOID * pData;

	if(FAILED( CBsFileIO::LoadFile(pszFileName, &pData, &dwFileSize) ) )
	{
		return E_FAIL;
	}


	// Compile an effect
	LPD3DXBUFFER pErrorList = NULL;

	if(FAILED(FXLCompileEffect((CHAR*)pData, dwFileSize, NULL, &s_MaterialInclude, NULL, pEffectData, &pErrorList))) {
		BsAssert( 0 && "FXLEffect Compile Error!!" );
		SAFE_RELEASE(pErrorList);
		CBsFileIO::FreeBuffer(pData);
		return E_FAIL;
	}
	SAFE_RELEASE(pErrorList);
	CBsFileIO::FreeBuffer(pData);
	return D3D_OK;
}

HRESULT C3DDevice::CreateFXLEffectFromMemory(LPVOID pMemoryBuffer, FXLEffect** ppEffect, FXLEffectPool* pEffectPool)
{
	// Create effect
	if( FAILED( FXLCreateEffect( m_pD3DDevice, pMemoryBuffer, pEffectPool, ppEffect))) {
		BsAssert( 0 && "Couldn't compile effect\n" );
		return E_FAIL;
	}
	return D3D_OK;
}
#endif


#ifdef _PACKED_RESOURCES
LPDIRECT3DBASETEXTURE9 C3DDevice::RegisterPreLoadedTexture( BYTE * pTextureResource )
{
	D3DBaseTexture *pTexture = new D3DBaseTexture;
	BsAssert(pTexture);

	*pTexture = ((XBOX360TEXTUREFILEHEADER*)pTextureResource)->d3dTexHeader[0];
	XGOffsetResourceAddress( pTexture, pTextureResource +  GPU_TEXTURE_ALIGNMENT);
	return pTexture;
}


LPDIRECT3DBASETEXTURE9 C3DDevice::LoadPackedTexture(const char* pFileName, DWORD *pdwSize, CHAR ** ppTexData)
{
	TIME_TEXTURE_LOAD(pFileName, &fTimeTotal);

	if(pFileName == NULL || ppTexData == NULL || pdwSize == NULL)
		return NULL;

	*ppTexData = NULL;
	*pdwSize = 0;

	XBOX360TEXTUREFILEHEADER *pHeader;
	BYTE *pReadBuffer = (BYTE*)XPhysicalAlloc( STREAMING_BUFFER_SIZE, MAXULONG_PTR, 0, PAGE_READWRITE );
	if(pReadBuffer == NULL)
	{
		BsAssert( 0  && "LoadPackedTexture - No Streaming buffer\n");
		return NULL;
	}

	DWORD dwIPFileSize;
	HRESULT hr;

	hr = CBsFileIO::BsGetFileSize(pFileName, &dwIPFileSize);
	if( hr != S_OK)
	{
		XPhysicalFree(pReadBuffer);
		return NULL;
	}
	BsAssert((dwIPFileSize / _DVD_SECTOR_SIZE)!= 0);

	D3DBaseTexture *pTexture = new D3DBaseTexture;
	DWORD dwTextureSize;
	DWORD dwCompressedSize;
	BYTE *pTexData;

	if(dwIPFileSize < (STREAMING_BUFFER_SIZE >> 1) )
	{
		if(CBsFileIO::LoadFileNoBuffering( pFileName, (VOID**)&pReadBuffer, &dwIPFileSize ) != S_OK)
		{
			//		BsAssert( 0  && "LoadPackedTexture - failed to open compressed file\n");
			delete pTexture;
			XPhysicalFree(pReadBuffer);
			return NULL;
		}

		pHeader = (XBOX360TEXTUREFILEHEADER*)pReadBuffer;
		dwTextureSize = pHeader->dwTextureSize;
		dwCompressedSize =  pHeader->dwCompressedSize;
		*pTexture = pHeader->d3dTexHeader[0];

		pTexData = (BYTE*)XPhysicalAlloc( dwTextureSize, MAXULONG_PTR, GPU_TEXTURE_ALIGNMENT, PAGE_READWRITE );
		if(pTexData == NULL)
		{
			BsAssert( 0  && "LoadPackedTexture - error allocating texture memory\n");
			delete pTexture;
			XPhysicalFree(pReadBuffer);
			return NULL;
		}

		BYTE * pRead = pReadBuffer + GPU_TEXTURE_ALIGNMENT;
		DWORD dwDataSize = dwIPFileSize - GPU_TEXTURE_ALIGNMENT;

		if(dwCompressedSize != dwTextureSize)
		{
			ZLIBDecode dc;

			hr = dc.DecodeImmediate(pRead, pTexData, dwDataSize, dwTextureSize);
			if(hr != S_OK)
			{
				BsAssert( 0  && "LoadPackedTexture - error uncompressing file\n");
			}
			else
			{
				// Flush the cached data out and free up L2 cache space
				for( DWORD i = 0; i < dwTextureSize; i += 128)
					__dcbf( i, pTexData );
			}
		}
		else
		{	// Texture is not compressed, so just copy....
			XMemCpy(pTexData, pRead, dwTextureSize);
		}
	}
	else
	{
		DWORD dwFlags = FILE_FLAG_OVERLAPPED | FILE_FLAG_NO_BUFFERING |
			FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN;

		HANDLE ipFile = CBsFileIO::BsCreateFile( pFileName, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, dwFlags, NULL );
		if (ipFile == INVALID_HANDLE_VALUE)
		{
			//		BsAssert( 0  && "LoadPackedTexture - failed to open compressed file\n");
			delete pTexture;
			XPhysicalFree(pReadBuffer);
			return NULL;
		}

		OVERLAPPED ovrR;
		memset(&ovrR, 0, sizeof(OVERLAPPED));

		// Read in texture header - Read whole sector to get non-buffered
		// overlapped I/O...
		if(CBsFileIO::LoadSectorImmediate( ipFile, pReadBuffer, &ovrR ) != S_OK)
		{
			BsAssert( 0  && "LoadPackedTexture - file size or read failure\n");
			CBsFileIO::BsCloseFileHandle( ipFile );
			delete pTexture;
			XPhysicalFree(pReadBuffer);
			return NULL;
		}

		// Data starts at GPU_TEXTURE_ALIGNMENT
		ovrR.Offset += GPU_TEXTURE_ALIGNMENT;
		pHeader = (XBOX360TEXTUREFILEHEADER*)pReadBuffer;
		dwTextureSize = pHeader->dwTextureSize;
		dwCompressedSize =  pHeader->dwCompressedSize;
		*pTexture = pHeader->d3dTexHeader[0];

		pTexData = (BYTE*)XPhysicalAlloc( dwTextureSize, MAXULONG_PTR, GPU_TEXTURE_ALIGNMENT, PAGE_READWRITE );
		if(pTexData == NULL)
		{
			BsAssert( 0  && "LoadPackedTexture - error allocating texture memory\n");
			CBsFileIO::BsCloseFileHandle( ipFile );
			delete pTexture;
			XPhysicalFree(pReadBuffer);
			return NULL;
		}
		if(dwCompressedSize != dwTextureSize)
		{
			ZLIBDecode dc;
			dc.InitMmDecompress(pReadBuffer, pTexData, STREAMING_BUFFER_SIZE, dwTextureSize, ipFile, &ovrR, dwIPFileSize-GPU_TEXTURE_ALIGNMENT, true);
			hr = dc.DecompressStreamToMemory();
			if(hr != S_OK)
			{
				BsAssert( 0  && "LoadPackedTexture - error uncompressing file\n");
			}
			else
			{
				// Flush the cached data out and free up L2 cache space
				for( DWORD i = 0; i < dwTextureSize; i += 128)
					__dcbf( i, pTexData );
			}
		}
		else
		{	// Read in the rest of the file as it is not compressed..
			DWORD dwBytesRead;
			BOOL bOK = CBsFileIO::BsReadFile( ipFile, pTexData, dwTextureSize, &dwBytesRead, &ovrR );

			DWORD dwErr = GetLastError();

			if(( bOK && dwBytesRead != dwTextureSize ) || (!bOK && (dwErr != ERROR_IO_PENDING )))
			{
				BsAssert( 0  && "LoadPackedTexture - error reading file\n");
				hr = E_FAIL;
			}

			// Block until the I/O is complete
			if(hr == S_OK && dwErr == ERROR_IO_PENDING)
			{
				if(!CBsFileIO::BsGetOverlappedResult(ipFile, &ovrR, &dwBytesRead, true) || (dwBytesRead != dwTextureSize))
				{
					BsAssert( 0  && "LoadPackedTexture - error reading file\n");
					hr = E_FAIL;
				}
			}
		}

		CBsFileIO::BsCloseFileHandle( ipFile );
	}

	XPhysicalFree(pReadBuffer);

	if( hr == S_OK)
	{		
		XGOffsetResourceAddress( pTexture, pTexData );
		*ppTexData = (CHAR*)pTexData;
		*pdwSize = dwTextureSize;
		return pTexture;
	}
	else
	{
		delete pTexture;
		return NULL;
	}

}
#endif // _PACKED_RESOURCES

void* C3DDevice::CreateTextureFromFile(const char* pFileName, DWORD dwUsage/* =0 */, CHAR ** ppTexData /*= NULL*/)
{
//	AvailMemoryDifference temp(0, (char*)pFileName);
	TIME_TEXTURE_LOAD(pFileName, &fTimeTotal);

	HRESULT hr;
	LPDIRECT3DTEXTURE9 pTexture = NULL;

	D3DFORMAT format = D3DFMT_UNKNOWN;
	DWORD dwColorKey = 0xffff00ff;
	DWORD dwNumLevels = 0;

	const char *pszExt=strrchr(pFileName, '.');
	// Mipmap level결정하는 루틴 필요!! by jeremy

	if( pszExt == NULL )
		return NULL;

	if( _strcmpi(pszExt, ".dds") == 0 ) {
		// DDS 그대로 쓴다.
		format = D3DFMT_UNKNOWN;
		DWORD dwCaps1;
		BFileStream stream(pFileName, BFileStream::openRead);

		if(!stream.Valid()){
			CBsConsole::GetInstance().AddFormatString("%s File Not Found", pFileName);
			return NULL;
		}

		stream.Seek(108, BFileStream::fromBegin);
		stream.Read(&dwCaps1, sizeof(DWORD));
#ifdef _XBOX
		stream.ConvertEndian(ENDIAN_FOUR_BYTE, (char*)&dwCaps1, sizeof(DWORD));
#endif
		if(dwCaps1 & 0x00400000l) {
			stream.Seek(28, BFileStream::fromBegin);
			stream.Read(&dwNumLevels, sizeof(DWORD));
#ifdef _XBOX
			stream.ConvertEndian(ENDIAN_FOUR_BYTE, (char*)&dwNumLevels, sizeof(DWORD));
#endif
		}
		else {
			// DDS 이지만, No Mipmap은 mipmap 1로 load한다.
			dwNumLevels = 1;
		}

		dwColorKey = 0;
	}
	else if( _strcmpi(pszExt, ".tga") == 0) {
		// Color Key 안쓰고, DXT5를 쓴다.
		format = D3DFMT_DXT5;
		dwColorKey = 0;
	}
	else if( _strcmpi(pszExt, ".bmp") == 0) {
		// Color Key 쓰고, DXT1을 쓴다.
		format = D3DFMT_DXT1;
		dwColorKey = 0xffff00ff;
		dwNumLevels = 1;
	}
	else {
		BsAssert( 0 && "Invalid format!!");
		return NULL;
	}

	VOID * pInputData;
	DWORD dwFileSize;

	if(ppTexData)
		*ppTexData = NULL;

	if(CBsFileIO::LoadFile(pFileName, &pInputData, &dwFileSize) != S_OK)
	{
		return NULL;
	}

	hr = D3DXCreateTextureFromFileInMemoryEx(m_pD3DDevice, pInputData, dwFileSize, D3DX_DEFAULT_NONPOW2, D3DX_DEFAULT_NONPOW2, dwNumLevels, dwUsage,
											 format, D3DPOOL_DEFAULT, D3DX_FILTER_NONE, D3DX_DEFAULT, dwColorKey, NULL, NULL, &pTexture);
	CBsFileIO::FreeBuffer(pInputData);

	if( hr==D3D_OK )
	{
		// Linear Texture
		if( NULL != strstr( pFileName, "_LIN") )
			return	pTexture;

#ifdef _XBOX
		D3DSURFACE_DESC d3dsdesc;
		pTexture->GetLevelDesc(0, &d3dsdesc);

		if( d3dsdesc.Format == D3DFMT_A8R8G8B8 || d3dsdesc.Format == D3DFMT_LIN_A8R8G8B8 ) {
			DebugString("%s : Uncompressed Texture!!\n", pFileName);
		}

		if(!(d3dsdesc.Format & D3DFORMAT_TILED_MASK))
		{
			DWORD dwGPUFormat;
			D3DFORMAT d3dfmtDst;

			d3dfmtDst = (D3DFORMAT)(d3dsdesc.Format | D3DFORMAT_TILED_MASK);
			dwGPUFormat = XGGetGpuFormat(d3dfmtDst);

			LPDIRECT3DTEXTURE9 pTiledTexture = new IDirect3DTexture9;
			DWORD dwBaseSize;
			DWORD dwMipSize;

			memset(pTiledTexture, 0, sizeof(IDirect3DTexture9));
			pTiledTexture->AddRef();

			UINT uiSize = XGSetTextureHeaderEx( d3dsdesc.Width, d3dsdesc.Height, dwNumLevels, 0, 
				d3dfmtDst, 0, 0, 0, XGHEADER_CONTIGUOUS_MIP_OFFSET, 0,
				pTiledTexture, (UINT*)&dwBaseSize, (UINT*)&dwMipSize );

			uiSize = ( ( uiSize + GPU_TEXTURE_ALIGNMENT - 1 ) / GPU_TEXTURE_ALIGNMENT ) * GPU_TEXTURE_ALIGNMENT;

			CHAR* pBuffer = (CHAR*)XPhysicalAlloc( uiSize, MAXULONG_PTR, GPU_TEXTURE_ALIGNMENT, PAGE_READWRITE | PAGE_WRITECOMBINE );
//			memset(pBuffer, 42, uiSize);
			XGOffsetResourceAddress( pTiledTexture, pBuffer );

			for( DWORD dwLevel = 0; dwLevel < dwNumLevels; dwLevel++ )
			{
				D3DLOCKED_RECT srcRect;
				pTexture->LockRect( dwLevel, &srcRect, NULL, 0 );//D3DLOCK_READONLY );

				// Calculate the mip level offset
				DWORD dwMipLevelOffset = XGGetMipLevelOffset( pTiledTexture, 0, dwLevel );
				if( dwLevel > 0 && dwMipSize > 0 )
					dwMipLevelOffset += dwBaseSize;

				XGTileTextureLevel( d3dsdesc.Width, d3dsdesc.Height, dwLevel, dwGPUFormat, 0,
					pBuffer + dwMipLevelOffset, NULL, srcRect.pBits, srcRect.Pitch, NULL );

				pTexture->UnlockRect( dwLevel );
			}
			pTexture->Release();
			pTexture = pTiledTexture;
			if(ppTexData)
				*ppTexData = pBuffer;
		}
#endif
		return pTexture;
	}
	else
		return NULL;
}


void* C3DDevice::CreateCubeTextureFromFile(const char* pFileName)
{
	TIME_TEXTURE_LOAD(pFileName, &fTimeTotal);

	HRESULT hr;
	LPDIRECT3DCUBETEXTURE9 pTexture;
	VOID * pInputData;
	DWORD dwFileSize;

	// Use LoadFile() to trap file loading failures...
	if(CBsFileIO::LoadFile(pFileName, &pInputData, &dwFileSize) != S_OK)
	{
		return NULL;
	}

	hr=::D3DXCreateCubeTextureFromFileInMemoryEx(m_pD3DDevice, pInputData, dwFileSize, D3DX_DEFAULT, D3DX_DEFAULT, D3DPOOL_DEFAULT,
		D3DFMT_UNKNOWN, D3DPOOL_DEFAULT, D3DX_FILTER_NONE, D3DX_DEFAULT, 0, NULL, NULL, &pTexture);

	CBsFileIO::FreeBuffer(pInputData);

	if(hr==D3D_OK)
		return pTexture;
	else
		return NULL;
}

void* C3DDevice::CreateVolumeTextureFromFile(const char* pFileName)
{
	TIME_TEXTURE_LOAD(pFileName, &fTimeTotal);

	HRESULT hr;
	LPDIRECT3DVOLUMETEXTURE9 pTexture;
	VOID * pInputData;
	DWORD dwFileSize;

	// Use LoadFile() to trap file loading failures...
	if(CBsFileIO::LoadFile(pFileName, &pInputData, &dwFileSize) != S_OK)
	{
		return NULL;
	}

	hr=::D3DXCreateVolumeTextureFromFileInMemoryEx(m_pD3DDevice, pInputData, dwFileSize, D3DX_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT, 1, D3DPOOL_DEFAULT,
		D3DFMT_UNKNOWN, D3DPOOL_DEFAULT, D3DX_FILTER_NONE, D3DX_DEFAULT, 0, NULL, NULL, &pTexture);

	CBsFileIO::FreeBuffer(pInputData);

	if(hr==D3D_OK)
		return pTexture;
	else
		return NULL;
}


void* C3DDevice::CreateTexture(DWORD *pdwSize, CHAR ** ppTexData, int nWidth, int nHeight, DWORD dwUsage, D3DFORMAT format, D3DPOOL pool/* =D3DPOOL_DEFAULT */)
{
	BsAssert(pdwSize);
	BsAssert(ppTexData);

	*ppTexData = NULL;
	*pdwSize = 0;

#ifdef _XBOX
	LPDIRECT3DTEXTURE9 pTexture = new IDirect3DTexture9;
	DWORD dwBaseSize;
	DWORD dwMipSize;

	memset(pTexture, 0, sizeof(IDirect3DTexture9));
	pTexture->AddRef();

	UINT uiSize = XGSetTextureHeaderEx( nWidth, nHeight, 1, dwUsage, format, 0, 0, 0, XGHEADER_CONTIGUOUS_MIP_OFFSET, 0,
		pTexture, (UINT*)&dwBaseSize, (UINT*)&dwMipSize );

	uiSize = ( ( uiSize + GPU_TEXTURE_ALIGNMENT - 1 ) / GPU_TEXTURE_ALIGNMENT ) * GPU_TEXTURE_ALIGNMENT;

	CHAR *pTexData = (CHAR*)XPhysicalAlloc( uiSize, MAXULONG_PTR, GPU_TEXTURE_ALIGNMENT, PAGE_READWRITE );
	if(pTexData == NULL)
	{
		BsAssert( 0  && "CreateTexture - error allocating texture memory\n");
		delete pTexture;
		return NULL;
	}

	XGOffsetBaseTextureAddress( pTexture, pTexData, pTexData );

	*ppTexData = pTexData;
	*pdwSize = uiSize;
	return pTexture;

#else
	HRESULT hr;
	LPDIRECT3DTEXTURE9 pTexture;

	hr=m_pD3DDevice->CreateTexture(nWidth, nHeight, 1, dwUsage, format, pool, &pTexture, NULL);
	if(hr==D3D_OK)
		return pTexture;
	else
		return NULL;
#endif
}

void* C3DDevice::CreateCubeTexture(int nEdgeLength, DWORD dwUsage, D3DFORMAT format, D3DPOOL pool/* =D3DPOOL_DEFAULT */)
{
	HRESULT hr;
	LPDIRECT3DCUBETEXTURE9 pTexture;

	hr=m_pD3DDevice->CreateCubeTexture(nEdgeLength, 1, dwUsage, format, pool, &pTexture, NULL);
	if(hr==D3D_OK)
		return pTexture;
	else
		return NULL;
}

void* C3DDevice::CreateVolumeTexture(int nWidth, int nHeight, int nDepth, DWORD dwUsage, D3DFORMAT format, D3DPOOL pool/* =D3DPOOL_DEFAULT */)
{
	HRESULT hr;
	LPDIRECT3DVOLUMETEXTURE9 pTexture;

	hr=m_pD3DDevice->CreateVolumeTexture(nWidth, nHeight, nDepth, 1, dwUsage, format, pool, &pTexture, NULL);
	if(hr==D3D_OK)
		return pTexture;
	else
		return NULL;
}

void C3DDevice::DrawIndexedMeshVB(int nPrimitiveType, int nVertexCount, int nPrimitiveCount, 
								  int nStartIndex, int nStartVertex)
{
	if(nPrimitiveCount>0) {
		m_pD3DDevice->DrawIndexedPrimitive((D3DPRIMITIVETYPE)nPrimitiveType, nStartVertex, 0,
			nVertexCount, nStartIndex, nPrimitiveCount);
#ifdef _RENDER_DEBUG
		m_dwDrawCallCount++;
		m_dwDrawPrimitiveCount += nPrimitiveCount;
#endif
	}
}

void C3DDevice::DrawIndexedPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT uiMinIndex, UINT uiVertexCount, UINT uiPrimitiveCount, const void* pIndexData, D3DFORMAT IndexDataFormat,
									   const void* pVertexStreamZeroData, UINT uiVertexStride)
{
	if(uiPrimitiveCount>0) {
		m_pD3DDevice->DrawIndexedPrimitiveUP(PrimitiveType, uiMinIndex, uiVertexCount, uiPrimitiveCount, pIndexData, IndexDataFormat,
			pVertexStreamZeroData, uiVertexStride );
#ifdef _RENDER_DEBUG
		m_dwDrawCallCount++;
		m_dwDrawPrimitiveCount += uiPrimitiveCount;
#endif
	}
}

void C3DDevice::DrawMeshVB(int nPrimitiveType, int nPrimitiveCount, const void *pVertexData, int nStride)
{
	if(nPrimitiveCount>0){
		SetStreamSource(0, (LPDIRECT3DVERTEXBUFFER9)pVertexData, nStride);
		m_pD3DDevice->DrawPrimitive((D3DPRIMITIVETYPE)nPrimitiveType, 0, nPrimitiveCount);
#ifdef _RENDER_DEBUG
		m_dwDrawCallCount++;
		m_dwDrawPrimitiveCount += nPrimitiveCount;
#endif
	}
}

void C3DDevice::DrawPrimitiveVB(int nPrimitiveType, int nStartVertex, int nPrimitiveCount)
{
	if(nPrimitiveCount>0) {
		m_pD3DDevice->DrawPrimitive((D3DPRIMITIVETYPE)nPrimitiveType, nStartVertex, nPrimitiveCount);
#ifdef _RENDER_DEBUG
		m_dwDrawCallCount++;
		m_dwDrawPrimitiveCount += nPrimitiveCount;
#endif
	}
}

extern const CHAR* g_strVertexShaderProgram;
extern const CHAR* g_strPixelShaderProgram;
void C3DDevice::CreateDefaultShader()
{
	// Buffers to hold compiled shaders and possible error messages
	ID3DXBuffer* pShaderCode = NULL;
	ID3DXBuffer* pErrorMsg = NULL;

	// Compile vertex shader.
	HRESULT hr = D3DXCompileShader( g_strVertexShaderProgram, (UINT)strlen( g_strVertexShaderProgram ),
		NULL, NULL, "main", "vs_2_0", 0,
		&pShaderCode, &pErrorMsg, NULL );
	BsAssert(! FAILED(hr) );

	// Create pixel shader.	
	m_pD3DDevice->CreateVertexShader( (DWORD*)pShaderCode->GetBufferPointer(), 
		&m_pDefaultVertexShader );

	// Shader code is no longer required.
	pShaderCode->Release();
	pShaderCode = NULL;

	// Compile pixel shader.
	hr = D3DXCompileShader( g_strPixelShaderProgram, (UINT)strlen( g_strPixelShaderProgram ),
		NULL, NULL, "main", "ps_2_0", 0,
		&pShaderCode, &pErrorMsg, NULL );
	BsAssert(! FAILED(hr) );

	// Create pixel shader.	
	m_pD3DDevice->CreatePixelShader( (DWORD*)pShaderCode->GetBufferPointer(),
		&m_pDefaultPixelShader );

	// Shader code no longer required.
	pShaderCode->Release();
	pShaderCode = NULL;

	D3DVERTEXELEMENT9 decl[] =
	{
		{ 0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
		{ 0, 12, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0 },
		D3DDECL_END()
	};
	CreateVertexDeclaration( decl, &m_pDefaultVertexDeclaration );
}

// 디버깅등에 사용합니다.
void C3DDevice::SetDefaultShader( const D3DXMATRIX *matWVP , D3DXVECTOR4 *diffColor)
{
	m_pD3DDevice->SetVertexShader( m_pDefaultVertexShader );
	m_pD3DDevice->SetPixelShader( m_pDefaultPixelShader );
#ifdef _XBOX 
	XMMATRIX XmatWVP;	
	XMVECTOR XdiffColor;
	memcpy(&XmatWVP, matWVP, sizeof(XMMATRIX));
	memcpy(&XdiffColor, diffColor, sizeof(XMVECTOR));

	m_pD3DDevice->SetVertexShaderConstantF( 0, (FLOAT*)&XmatWVP, 4 );
	m_pD3DDevice->SetPixelShaderConstantF( 0, (FLOAT*)&XdiffColor, 1 );
#else
	m_pD3DDevice->SetVertexShaderConstantF( 0, (FLOAT*)matWVP, 4 );
	m_pD3DDevice->SetPixelShaderConstantF( 0, (FLOAT*)diffColor, 1 );
#endif
	SetVertexDeclaration( m_pDefaultVertexDeclaration );
}

static const CHAR* g_strVertexShaderProgram = 
" float4x4 matWVP : register(c0);              "  
" struct VS_IN                                 "  
" {                                            " 
"     float4 ObjPos   : POSITION;              "  // Object space position 
" };                                           " 
" struct VS_OUT                                " 
" {                                            " 
"     float4 ProjPos  : POSITION;              "  // Projected space position 
" };                                           "  
" VS_OUT main( VS_IN In )                      "  
" {                                            "  
"     VS_OUT Out;                              "  
"     Out.ProjPos = mul( matWVP, In.ObjPos );  "  // Transform vertex into
"     return Out;                              "  // Transfer color
" }                                            ";

static const CHAR* g_strPixelShaderProgram = 
" float4 diffColor : register(c0);              " 
" struct PS_IN                                 "
" {                                            "
"     float4 ProjPos : POSITION;                    "  // Interpolated color from                      
" };                                           "  // the vertex shader
" float4 main( PS_IN In ) : COLOR              "  
" {                                            "  
"     return diffColor;                         "  // Output color
" }                                            "; 
