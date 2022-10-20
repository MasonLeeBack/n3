#include "stdafx.h"
#include "BsUtil.h"
#include "BStreamExt.h"
#include "BsKernel.h"
#include "FrameChecker.h"
#include "BsMesh.h"
#include "BsAni.h"
#include "BsHFWorld.h"
#include "BsLODWorld.h"
#include "BsMaterial.h"
#include "BsTexture.h"
#include "BsSkin.h"
#include "BsObject.h"
#include "BsFXObject.h"
#include "BsCamera.h"
#include "BsUIBase.h"
#include "BsLightObject.h"

#include "BsPhysicsMgr.h"
#include "BsClothObject.h"

#include "BsImageProcess.h"
#include "BsShadowMgr.h"
#include "BsShadowVolumeMgr.h"
#include "BsOffScreenMgr.h"
#include "BsCustomRender.h"

#include "BsSkyBoxObject.h"
#include "BsAniCache.h"
#include "BsAniObject.h"
#include "BsFileIO.h"
#include "IntBox3Frustum.h"

#include "BsParticle.h"
#include "BsRealMovie.h"

#include "BsUserNxClass.h"
#include "BsDecal.h"
#include "BsWater.h"
#include "BsWaterObject.h"

#include "BsInstancingMgr.h"
#include "BsBillboardMgr.h"

#include "MS\CClipTest.h"

//#define _KERNEL_LOG_
//#define ENABLE_FULL_STAGE_CLEANOUT

extern int g_nProcessTick;

#ifdef _XBOX
extern DWORD g_dwRenderThreadId;

bool IsRenderThread()
{
	return (GetCurrentThreadId() == g_dwRenderThreadId );
}

HANDLE g_hSystemMonitorThread;
DWORD g_dwSystemMonitorThreadID;

DWORD WINAPI SystemMonitorThreadProc( LPVOID lpParameter )
{
	const float c_fOOOMeg = 1.f / (1024.f * 1024.f);
	while(1) {
		//Output the texture memory usage to PIX
		PIXAddNamedCounter( CBsTexture::GetTextureMmUsage() * c_fOOOMeg, "Texture Memory Usage (MB)");

		//Output how much memory (in MB) remains free
		MEMORYSTATUS mem;
		GlobalMemoryStatus(&mem);
		PIXAddNamedCounter( mem.dwAvailPhys * c_fOOOMeg, "Memory Free (MB)");

		Sleep(1000);
	}
	return 0x0;
}
#else

bool IsRenderThread()
{
	return false;
}
#endif

inline bool CompareDistance(const RENDER_INFO& lhs,const RENDER_INFO& rhs)
{
	if(lhs.fDistance<rhs.fDistance)
		return true;
	return false;
}

#ifndef _LTCG
int CBsKernel::s_nTrackingIndex = -1;
#endif

int CBsKernel::s_nDrawTotalAniCount = 0;
int CBsKernel::s_nDrawTotalCount = 0;

#define KERNEL_COMMANDBUFFER_SIZE	5000

CBsKernel::CBsKernel()
: m_ParticlePool(MAX_PARTICLE_OBJECT_COUNT)
{
	int i;

	m_dwRenderTick[0] = 0;
	m_dwRenderTick[1] = 0;
	m_bActive = false;

	m_pDevice = NULL;

	m_pActiveCamera = NULL;

	SetDirectionalLight(-1);

	memset(m_pMeshList, 0, MAX_MESH_COUNT*sizeof(CBsMesh *));
	memset(m_pAniList, 0, MAX_ANI_COUNT*sizeof(CBsAni *));
	memset(m_pMaterialList, 0, MAX_MATERIAL_COUNT*sizeof(CBsMaterial *));
	memset(m_pVertexDeclList, 0, MAX_VERTEXDECLARATION_COUNT*sizeof(CBsVertexDeclaration *));
	memset(m_pTextureList, 0, MAX_TEXTURE_COUNT*sizeof(CBsTexture *));
#ifdef _XBOX
	memset(m_DeferredDeleteList, 0, MAX_TEXTURE_COUNT*sizeof(DeferredResource));
	m_nDeferredTextureCount = 0;
#endif
	memset(m_pSkinList, 0, MAX_SKIN_COUNT*sizeof(CBsSkin*));
	memset(m_pFXTemplateList, 0, MAX_FXTEMPLATE_COUNT*sizeof(CBsFXTemplate*));

	m_nPoolSize=DEFAULT_BS_OBJECT_COUNT;
	m_ppObjectPool=new CBsObject *[DEFAULT_BS_OBJECT_COUNT];
	memset(m_ppObjectPool, 0, sizeof(CBsObject *)*m_nPoolSize);

	m_pWorld = NULL;
	m_pShadowMgr = NULL;
	m_pShadowVolumeMgr = NULL;
	m_pImageProcess = NULL;
	m_pFXManager = NULL;

	m_pInstancingMgr = NULL;

	m_bShowFPS=false;
	m_bShowDebugStr=false;

	m_nFPSX				= 450;
	m_nFPSY				= 50;
#ifdef _USE_CONSOLE_COMMAND_
	m_pFont				= NULL;
#endif
	m_nBlankTexture		=-1;
	m_nStringCount		= 0;

	for( i=0 ; i<MAX_STRING_COUNT ; i++ ) {
		m_pStringBuffer[i]=NULL;
		m_nStringBufferLength[i]=0;
	}

	m_IteratePos.reserve( MAX_ITERATE_POSTION_COUNT );
	m_ParticleGroupList.resize( MAX_PARTICLE_GROUP_COUNT );
	m_Particles.reserve( DEFAULT_RENDER_PARTICLE_LIST_SIZE );

	m_UpdateBuffer[ 0 ].reserve( DEFAULT_UPDATE_OBJECT_LIST_SIZE );
	m_UpdateBuffer[ 1 ].reserve( DEFAULT_UPDATE_OBJECT_LIST_SIZE );

	strcpy(m_szShaderDir, "");

	m_pChecker = new CFrameChecker;
	m_pDirManager=new CBsDirManager();

	m_pPhysicsMgr = NULL;
	m_pHFActor = NULL;	
	for( i = 0; i < 4; i++) {
		m_pPlaneActor[i] = NULL;
	}

	m_pRTTManager = NULL;
	m_pRTTManager = new CBsOffScreenManager();

	m_pUIManager=NULL;	// 2D UI 용 입니다
	m_pClickFxEffect = NULL; //2D UI 용 effect

	m_pRealMovie = NULL;

	m_bPhysicsEnable = true;
	m_pDecalManager = NULL;
	m_pBillboardMgr = NULL;

	m_nPointLightCount = 0;

	for( i = 0; i < MAX_POINT_LIGHT_COUNT; i++) {
		m_PointLightIndices[i] = -1;
	}

	for( i = 0; i < MAX_POINT_LIGHT; i++) {
		m_nSelectedPointLights[i] = -1;
	}

	m_pWater = new CBsWater;

	for ( int bufferIndex = 0; bufferIndex < DOUBLE_BUFFERING; ++bufferIndex ) {
		m_PreKernelCommand[ bufferIndex ].reserve( KERNEL_COMMANDBUFFER_SIZE );
		m_PostKernelCommand[ bufferIndex ].reserve( KERNEL_COMMANDBUFFER_SIZE );
	}

	m_TransparentObjectList.reserve(50);

	SetCurInputPort( -1 );
}

CBsKernel::~CBsKernel() 
{
	int i;

	SAFE_RELEASE_TEXTURE(m_nBlankTexture);

	//---------------
	SAFE_DELETE(m_pUIManager);

	SAFE_DELETE(m_pClickFxEffect);

	SAFE_DELETE(m_pRTTManager);

	SAFE_DELETE(m_pRealMovie);
	//---------------

	SAFE_DELETE(m_pWorld);

	SAFE_DELETE(m_pShadowMgr);

	SAFE_DELETE(m_pShadowVolumeMgr);

	SAFE_DELETE(m_pImageProcess);

#ifdef _XBOX
#ifdef _USE_CONSOLE_COMMAND_
	SAFE_DELETE(m_pFont);
#endif
#else
	SAFE_RELEASE(m_pFont);
#endif

	SAFE_DELETE(m_pChecker);

	for(i=0;i<MAX_STRING_COUNT;i++){
		SAFE_DELETEA(m_pStringBuffer[i]);
	}

	SAFE_DELETE(m_pDirManager);

	SAFE_DELETE(m_pHFActor);
	for( i = 0; i < 4; i++) {
		SAFE_DELETE( m_pPlaneActor[i] );
	}

	SAFE_DELETE(m_pPhysicsMgr);

	SAFE_DELETE(m_pDecalManager);

	SAFE_DELETE(m_pWater);

	SAFE_DELETE(m_pInstancingMgr);

	SAFE_DELETE(m_pBillboardMgr);

	/////////////////////////////////////////////////////////////////
	DeleteAllFXTemplates();
	DeleteAllObject();

	DeleteAllSkin();

	DeleteAllMesh();

	DeleteAllAni();

	DeleteAllMaterial();
	CBsMaterial::DestroyEffectPool();

	DeleteAllTexture();

	CBsParticleGroup::DeleteEffect();

	g_BsKernel.FlushFXTemplates();
	g_BsKernel.FlushParticleGroups();
	g_BsKernel.FlushSkins();
	g_BsKernel.FlushVertexDeclarations();
	g_BsKernel.FlushMaterials();
	g_BsKernel.FlushAnis();

	std::for_each(m_ParticleGroupList.begin(), m_ParticleGroupList.end(), std_delete());

	delete [] m_ppObjectPool;
	m_ppObjectPool = NULL;

	/////////////////////////////////////////////////////////////////
	// Last work!!
	/////////////////////////////////////////////////////////////////
	SAFE_DELETE(m_pDevice);
}

#ifdef _XBOX
HRESULT CBsKernel::InitializeDevice()
{
	BsAssert(!m_pDevice && "Duplicated Device Allocation!!");
	m_pDevice = new C3DDevice;
	if(!m_pDevice)
		return E_FAIL;
	m_pDevice->Initialize();

#ifdef _USE_CONSOLE_COMMAND_
	if(!m_pFont)
		m_pFont = new ATG::Font;
	m_pFont->Create("d:\\data\\Arial_16.xpr");
#endif

	m_bActive = true;
	return S_OK;
}

HRESULT CBsKernel::Initialize()
{
	InitializeFileLoader("d:\\");
	if(strlen(m_szShaderDir)<=0){
		SetShaderDirectory("d:\\data\\Material\\");
	}

	g_BsMemChecker.Start( "CreateEffectPool" );

	CBsMaterial::CreateEffectPool();

	g_BsMemChecker.End();

	g_BsMemChecker.Start( "CreateEffect" );

	CBsParticleGroup::CreateEffect();

	g_BsMemChecker.End();

	m_nBlankTexture = LoadTexture("data\\blank.dds");

	{

		g_BsMemChecker.Start( "BsImageProcess" );
		// Create Screen Effect!
		m_pImageProcess = new CBsImageProcess;
		m_pImageProcess->Create();
		g_BsMemChecker.End();

		// Create Shadow Manager!
		m_bShowShadow = true;
		g_BsMemChecker.Start( "CBsShadowMgr" );
		m_pShadowMgr = new CBsShadowMgr;
		m_pShadowMgr->Create();
		CBsObject::SetShadowMgr(m_pShadowMgr);
		g_BsMemChecker.End();

		g_BsMemChecker.Start( "CBsShadowVolumeMgr" );
		m_pShadowVolumeMgr = new CBsShadowVolumeMgr;
		m_pShadowVolumeMgr->Create();
		CBsObject::SetShadowVolumeMgr( m_pShadowVolumeMgr );
		g_BsMemChecker.End();
	}

	g_BsMemChecker.Start( "CBsPhysicsMgr::Create" );
	m_pPhysicsMgr =  CBsPhysicsMgr::Create();
	m_pPhysicsMgr->InitPhysics();
	g_BsMemChecker.End();

	g_BsMemChecker.Start( "CBsDecalManager" );
	m_pDecalManager = new CBsDecalManager;
	g_BsMemChecker.End();

	// Create instancing manager
	g_BsMemChecker.Start( "CBsInstancingMgr" );
	m_pInstancingMgr = new CBsInstancingMgr();
	CBsObject::SetInstancingMgr( m_pInstancingMgr );
	g_BsMemChecker.End();

	// Create Billboard manager
	g_BsMemChecker.Start( "CBsObjectBillboardMgr" );
	m_pBillboardMgr = new CBsObjectBillboardMgr();
	g_BsMemChecker.End();

	//	g_hSystemMonitorThread = CreateThread( NULL, 65536, SystemMonitorThreadProc, NULL, 0, &g_dwSystemMonitorThreadID );
	//	if(g_hSystemMonitorThread)
	//	{
	//		SetThreadPriority(g_hSystemMonitorThread, THREAD_PRIORITY_BELOW_NORMAL);
	//		XSetThreadProcessor( g_hSystemMonitorThread, 3 );
	//	}
	return S_OK;
}
#else
HRESULT CBsKernel::Initialize( HWND hWnd, int nWidth, int nHeight, bool bIsWindowed, bool bMultithreaded/*= false*/ )
{
	char szCurDir[_MAX_PATH];

	BsAssert(!m_pDevice && "Duplicated Device Allocation!!");

	m_pDevice = new C3DDevice;
	if(!m_pDevice)
		return E_FAIL;

	m_pDevice->Initialize(hWnd, nWidth, nHeight, bIsWindowed, bMultithreaded);
	m_bActive = true;

	CBsMaterial::CreateEffectPool();

	D3DXCreateFont( m_pDevice->GetD3DDevice(), 15, 0, FW_BOLD, 1, FALSE, DEFAULT_CHARSET, 
		OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Arial", &m_pFont );

	::GetCurrentDirectory(_MAX_PATH, szCurDir);
	InitializeFileLoader(szCurDir);
	strcat(szCurDir, "\\data\\Material\\");
	if(strlen(m_szShaderDir)<=0) {
		SetShaderDirectory(szCurDir);
	}

	CBsParticleGroup::CreateEffect();

	m_nBlankTexture = LoadTexture("data\\blank.dds");

	{
		m_pImageProcess = new CBsImageProcess;
		m_pImageProcess->Create();

		m_bShowShadow = true;
		m_pShadowMgr = new CBsShadowMgr;
		m_pShadowMgr->Create();
		CBsObject::SetShadowMgr(m_pShadowMgr);

		m_pShadowVolumeMgr = new CBsShadowVolumeMgr;
		m_pShadowVolumeMgr->Create();
		CBsObject::SetShadowVolumeMgr( m_pShadowVolumeMgr );
	}

	m_pPhysicsMgr =  CBsPhysicsMgr::Create();
	m_pPhysicsMgr->InitPhysics();

	m_pDecalManager = new CBsDecalManager;

	// Create instancing manager
	m_pInstancingMgr = new CBsInstancingMgr();
	CBsObject::SetInstancingMgr( m_pInstancingMgr );

	// Create Billboard manager
	m_pBillboardMgr = new CBsObjectBillboardMgr();

	return S_OK;
}

HRESULT CBsKernel::ReInitialize( HWND hWnd, int nWidth, int nHeight, bool bIsWindowed, bool bMultithreaded/*= false*/ )
{
	if(!m_bActive){
		return E_FAIL;
	}

	SAFE_RELEASE(m_pFont);

	SAFE_DELETE(m_pShadowMgr);
	SAFE_DELETE(m_pShadowVolumeMgr);
	SAFE_DELETE(m_pImageProcess);

	//	LostAllTexture();

	CBsMaterial::DestroyEffectPool();

	SAFE_DELETE(m_pDevice);

	m_pDevice=new C3DDevice();
	HRESULT hRet=m_pDevice->Initialize(hWnd, nWidth, nHeight, bIsWindowed, bMultithreaded);
	if(hRet!=S_OK)
		return hRet;

	CBsMaterial::CreateEffectPool();

	int nCameraCount = m_CameraIndices.size();
	for(int i=0;i<nCameraCount; ++i) {
		int nCameraIndex = m_CameraIndices[i];
		((CBsCamera*)m_ppObjectPool[nCameraIndex])->AttachDevice(m_pDevice);
	}

	{
		// Create Screen Effect!
		m_pImageProcess = new CBsImageProcess;
		m_pImageProcess->Create();
		// Create Shadow Manager!
		m_bShowShadow = true;
		m_pShadowMgr = new CBsShadowMgr;
		m_pShadowMgr->Create();
		CBsObject::SetShadowMgr(m_pShadowMgr);

		m_pShadowVolumeMgr = new CBsShadowVolumeMgr;
		m_pShadowVolumeMgr->Create();
		CBsObject::SetShadowVolumeMgr( m_pShadowVolumeMgr );
	}

	D3DXCreateFont( m_pDevice->GetD3DDevice(), 15, 0, FW_BOLD, 1, FALSE, DEFAULT_CHARSET, 
		OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Arial", &m_pFont );

	m_bActive = true;

	Reload();

	return S_OK;
}
#endif

void CBsKernel::OutHst(char* szFileName, char* szFmt,...)
{
#ifndef _KERNEL_LOG_
	return;
#else
	FILE *Hst;
	char buf[256];

	Hst=fopen(szFileName,"a+");
	if(Hst<=0)return;	// mruete: prefix bug 334: changed to <=
	DWORD dwTickCount;
	dwTickCount = GetTickCount();
	va_list argptr;
	va_start(argptr, szFmt);

	sprintf_s(buf,_countof(buf),"%d Tick : ", dwTickCount);
	fputs(buf,Hst);	vsprintf_s(buf,_countof(buf),szFmt,argptr);
	fputs(buf, Hst);
	va_end(argptr);
	sprintf_s(buf,_countof(buf),"\n");
	fputs(buf,Hst);
	fclose(Hst);
#endif
}

void CBsKernel::SaveScreen(char* szFileName)
{
	BsAssert( 0 && "No Support!!");
}

void CBsKernel::UpdateObject( int nObjectIndex, D3DXMATRIX* pObjectMatrix/* = NULL */ )
{
	BsAssert( nObjectIndex >= 0 && nObjectIndex < m_nPoolSize ); // mruete: prefix bug 335: added nObjectIndex < m_nPoolSize, and changed to BsAssert
	if( nObjectIndex < 0 || nObjectIndex >= m_nPoolSize )
	{
		return;
	}

	if( m_ppObjectPool[ nObjectIndex ] )
	{
		if( pObjectMatrix )
		{
			m_ppObjectPool[ nObjectIndex ]->SetObjectMatrix( pObjectMatrix );
		}
		if(nObjectIndex >= FIRST_DRAW_BLOCK_COUNT)
			GetUpdateListByProcess()->push_back( nObjectIndex );
		m_ppObjectPool[nObjectIndex]->Show(true);
	}
}

void CBsKernel::ShowObject( int nObjectIndex, bool bShow )
{
	assert( nObjectIndex >= 0 );

	if( m_ppObjectPool[ nObjectIndex ] )
	{
		m_ppObjectPool[ nObjectIndex ]->Show( bShow );
	}
}

void CBsKernel::Process()
{
	CBsFXObject::m_dwProcessTick++;	
	UpdateParticleObjectList();
}

void CBsKernel::SetPreCalcAni(int nObjectIndex, bool bEnable)
{	
	int i, nSize;
	nSize = m_PreCalcAniObjList.size();

	for( i = 0; i < nSize ; i++) {
		if( m_PreCalcAniObjList[ i ] == nObjectIndex) {
			if( bEnable == false)  {
				m_PreCalcAniObjList.erase(m_PreCalcAniObjList.begin() + i);
				return;
			} 
			else {
				return;
			}
		}
	}
	if( bEnable ) {
		m_PreCalcAniObjList.push_back( nObjectIndex );
	}
}

void CBsKernel::PreInitRenderFrame()
{
	int i, nSize;

#ifdef _USAGE_TOOL_
	AddRenderTick();
#else
	SetRenderTick(g_nProcessTick);
#endif

	nSize = m_PreCalcAniObjList.size();
	for( i = 0; i < nSize; i++) {
		int nIndex = m_PreCalcAniObjList[i];
#ifndef _LTCG
		s_nTrackingIndex = nIndex;
#endif
		CBsObject *pObject = m_ppObjectPool[ nIndex ];				
		if( pObject ) {		
			pObject->ProcessLinkedPhysicsObject();
		}
	}	
}

void CBsKernel::PostInitRenderFrame()
{
	g_BsAniCache.EraseGarbageCash();
	g_LargeBsAniCache.EraseGarbageCash();
	CBsClothObject::PostInitRenderObjects();
	CBsObject::FlipProcessBuffer();
	CBsUIManager::FlipProcessBuffer();
}

void CBsKernel::InitRenderFrame()
{
	BeginPIXEvent(255, 0, 0, "Frustum Culling and Animation");

	// 모든 Camera UpdateFrustum()호출!! by jeremy
	s_nDrawTotalCount = 0;
	s_nDrawTotalAniCount = 0;	
	m_nCameraCount = m_CameraIndices.size();
	for(int i=0;i<m_nCameraCount;++i) {
		//********************************************************************
		int nCameraIndex = m_CameraIndices[i];
		CBsCamera* pCamera = (CBsCamera*)m_ppObjectPool[nCameraIndex];
		if(nCameraIndex != -1) {
			pCamera->UpdateFrustum();
		}
		else {
			assert(0 && "Invalid Camera!!");
			continue; 
		}

		//********************************************************************
		//	반드시 고쳐야 되는 부분!! by jeremy
		//********************************************************************
#ifdef	_XBOX
		InitIsVisibleObjectLite( pCamera );
#endif
		InitPointLightObjects();

		int j, nObj;
		CBsObject* pObject;
		float fViewDistance;
		std::vector< int >	*pUpdateList;

		//m_pBillboardMgr->SetProcessBufferIndex(CBsObject::GetProcessBufferIndex());		
		pUpdateList = GetUpdateListByProcess();
		for( j = 0; j < (int)pUpdateList->size(); j++ ) {
#ifndef _LTCG
			s_nTrackingIndex = j;
#endif
			nObj = ( *pUpdateList )[ j ];
			pObject = m_ppObjectPool[ nObj ];
			if( pObject && pObject->IsRootObject() ) {
#ifdef	_XBOX
				bool bVisibleShadow = false;
				bool	bVisible	= IsVisibleObjectLite( pObject, &fViewDistance, &bVisibleShadow );
#else
				bool	bVisible	= IsVisibleObject( nCameraIndex, nObj, &fViewDistance );
				bool bVisibleShadow = bVisible;
#endif
				if( bVisible || bVisibleShadow ) {					
					if( pObject->InitRender( fViewDistance ) ) { // fViewDistance 값을 LOD 에 바로 사용 할수 있습니다
						if( bVisible ) {			
							// CBsObject를 통한 Rendering!
							pCamera->AddDrawObject( nObj ); 
						}
						if( bVisibleShadow) {	
							// CBsObject를 통한 Shadow Rendering!
							pCamera->AddShadowObject( nObj ); 
						}
					}
					else {
						// Animation Billboard를 통한 Rendering!
					}
					s_nDrawTotalCount++;
				}
				else {
					pObject->Process();
				}
			}
		}	
	}	// End for()

	// Calculate Animation for Off screen!!
	m_pRTTManager->InitRenderRTTs();

	// Update Point Light Decal
	UpdatePointLightDecal();
	// Update Decal List
	if( m_pDecalManager ) 
		m_pDecalManager->Update();

	EndPIXEvent();	
}

#ifdef _LOAD_MAP_CHECK_
bool g_bMemCheckPrint = false;
#endif

void CBsKernel::RenderFrame()
{
	PreKernelCommand();	

#ifdef _LOAD_MAP_CHECK_
	if( g_bMemCheckPrint ) {
		int i, nTotal, nAllTotal;
		char szInfo[ 1024 ];

		nAllTotal = 0;
		nTotal = 0;
		for( i = 0; i < MAX_SKIN_COUNT; i++ ) {
			if( m_pSkinList[ i ] ) {
				int nMeshIndex, nLODCount;

				nMeshIndex = m_pSkinList[ i ]->GetMeshIndex();
				if( m_pMeshList[ nMeshIndex ] ) {
					nLODCount = m_pMeshList[ nMeshIndex ]->GetLODCount();
				}
				else {
					nLODCount = 1;
				}

				sprintf_s( szInfo, _countof(szInfo), "Skin = %d Use = %d %s LODCount = %d Mem = %d\n", i, m_pSkinList[ i ]->m_nUseCount, m_pSkinList[ i ]->GetSkinFileName(), nLODCount, m_pSkinList[ i ]->m_nLoadSize / 1024 );
				DebugString( szInfo );
				nTotal += m_pSkinList[ i ]->m_nLoadSize / 1024;
			}
		}
		sprintf_s( szInfo, _countof(szInfo), "Skin Use %dKB\n\n", nTotal );
		DebugString( szInfo );
		for( i = 0; i < MAX_TEXTURE_COUNT; i++ ) {
			if( m_pTextureList[ i ] ) {
				sprintf_s( szInfo, _countof(szInfo), "Texture = %d : %s\n", i, m_pTextureList[ i ]->GetTextureName() );
				DebugString( szInfo );
			}
		}
		DebugString( "\n\n" );
		nAllTotal += nTotal;
		nTotal = 0;
		for( i = 0; i < MAX_ANI_COUNT; i++ ) {
			if( m_pAniList[ i ] ) {
				sprintf_s( szInfo, _countof(szInfo), "Ani = %d : %s Mem = %d\n", i, m_pAniList[ i ]->GetAniFileName(), m_pAniList[ i ]->m_nLoadSize /1024 );
				DebugString( szInfo );
				nTotal += m_pAniList[ i ]->m_nLoadSize / 1024;
			}
		}
		nAllTotal += nTotal;
		sprintf_s( szInfo, _countof(szInfo), "Ani Use %dKB\n\n", nTotal );
		DebugString( szInfo );

		nTotal = 0;
		for( i = 0; i < MAX_FXTEMPLATE_COUNT; i++ )	{
			if( m_pFXTemplateList[ i ] ) {
				sprintf_s( szInfo, _countof(szInfo), "FX = %d : %s Mem = %d\n", i, m_pFXTemplateList[ i ]->GetFileName(), m_pFXTemplateList[ i ]->m_nLoadSize / 1024 );
				DebugString( szInfo );
				nTotal += m_pFXTemplateList[ i ]->m_nLoadSize / 1024;
			}
		}
		nAllTotal += nTotal;
		sprintf_s( szInfo, _countof(szInfo), "FX Use %dKB\n\n", nTotal );
		DebugString( szInfo );

		sprintf_s( szInfo, _countof(szInfo), "All Use %dKB\n\n", nAllTotal );
		DebugString( szInfo );
		g_bMemCheckPrint = false;
	}
#endif
	if(m_bShowFPS) {
		char cBuf[100];
		PrintString(m_nFPSX, m_nFPSY, m_pChecker->GetFPSString(), 0xffffff00);
		sprintf_s(cBuf, _countof(cBuf), "Draw : %d(%d) / UpdateObject Count : %d", s_nDrawTotalCount, s_nDrawTotalAniCount, GetUpdateListByRender()->size() );
		PrintString(m_nFPSX, m_nFPSY+20, cBuf, 0xffffff00);
#ifdef _XBOX
		MEMORYSTATUS mem;
		GlobalMemoryStatus(&mem);
		sprintf_s(cBuf, _countof(cBuf), "Avail Memory Info : %dKB / %dKB", mem.dwAvailPhys/1024, mem.dwTotalPhys/1024);
		PrintString(m_nFPSX, m_nFPSY+40, cBuf, 0xff000000);				
#endif
	}

	///////////////////////////////////////////////////////////////////////
	RenderRTT();
	if(m_pUIManager) {
		int nIndex = CBsObject::GetRenderBufferIndex();
		m_pUIManager->ClearElementException(nIndex);
	}

	for (int i=0;i<m_nCameraCount;++i) {
		LockActiveCamera(m_CameraIndices[i]);
		RenderScene();
		UnlockActiveCamera();
	}

	BeginPIXEvent(255, 0, 0, "ScreenCapture And Render Alpha Scene");

	// Water 관련 렌더 루틴 변경으로 Off시켰습니다. by jeremy
	//	if(m_pImageProcess) {
	//		m_pImageProcess->ScreenCaptureBeforeAlphaBlendPass(m_pDevice);
	//	}

	for (int i=0;i<m_nCameraCount;++i) {
		LockActiveCamera(m_CameraIndices[i]);
		RenderTransparentScene();
		UnlockActiveCamera();
	}

	if(m_pImageProcess) {
		m_pImageProcess->ScreenCaptureBeforeAlphaBlendPass(m_pDevice);
	}

	for (int i=0;i<m_nCameraCount;++i) {
		LockActiveCamera(m_CameraIndices[i]);

		RenderCustom();

		UnlockActiveCamera();
	}	

	EndPIXEvent();

	if(m_pImageProcess) {
		BeginPIXEvent(0, 255, 0, "ScreenCapture And Image Process");
		m_pImageProcess->ScreenCaptureFinal(m_pDevice);
		m_pImageProcess->Render(m_pDevice);
		EndPIXEvent();
	}

	RenderSceneUI();

	/////////////////////////////////////////////////////////////////////// Camera Reset!!
	for (int i=0;i<m_nCameraCount;++i) {
		CBsCamera* pCamera = (CBsCamera*)m_ppObjectPool[m_CameraIndices[i]];
		pCamera->ResetDrawObjectList();
	}

	BeginPIXEvent(0, 255, 255, "Render Text");
	if(m_bShowFPS) {
		char szBuf[100];
		sprintf_s(szBuf, _countof(szBuf), "DrawCall : %d, Primitive : %d", m_pDevice->m_dwDrawCallCount, m_pDevice->m_dwDrawPrimitiveCount);
		PrintString(m_nFPSX, m_nFPSY+60, szBuf, 0xff000000);

		m_pDevice->m_dwDrawCallCount = 0;
		m_pDevice->m_dwDrawPrimitiveCount = 0;

		// Particle Count
		sprintf_s( szBuf, _countof(szBuf), "Particle : %03d / %03d", m_ParticlePool.Count(), m_ParticlePool.Size() );
		PrintString( m_nFPSX, m_nFPSY+80, szBuf, 0xff000000);

		sprintf_s( szBuf, _countof(szBuf), "FOV : %2.4f, BillBoard Range : %6.2f", m_nCameraCount ? ((CBsCamera*)m_ppObjectPool[m_CameraIndices[0]])->GetFOVByRender() : 0.f, m_pBillboardMgr->GetCurrentBillboardRange() );
		PrintString( m_nFPSX, m_nFPSY+100, szBuf, 0xff000000);
		RenderText();
	}
	if(m_bShowDebugStr) {	
		RenderText();
	}
	EndPIXEvent();

#ifdef ON_SCREEN_ASSERT
	_OnScreenAssert();
#endif
	CBsWaterObject::s_bCaptureRefractMap = FALSE;
	m_pChecker->AddFrame(1);
}

void CBsKernel::RenderRTTFrame(CBsRTTCamera* pRTTCamera, C3DDevice* pDevice)
{
	pRTTCamera->Render(pDevice);

	m_pDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);

	RenderObjectList();
	RenderTransparentScene(FALSE);

	pRTTCamera->ResetDrawObjectList();
}

void CBsKernel::InitRenderRTTFrame(CBsRTTCamera* pRTTCamera)
{
	pRTTCamera->UpdateFrustum();

	std::vector<int>& RegisteredObjectList = pRTTCamera->GetRegisteredDrawObjectList();
	int nRegisterObjectCount = RegisteredObjectList.size();

	CBsObject* pObject = NULL;
	for(int i = 0 ; i<nRegisterObjectCount ; ++i) {
		pObject = GetEngineObjectPtr(RegisteredObjectList[i]);
		pObject->InitRender(0.f);
		pRTTCamera->AddDrawObject(RegisteredObjectList[i]);
	}
}

void CBsKernel::RenderScene()
{
	BeginPIXEvent(255, 0, 0, "RenderScene");

	D3DVIEWPORT9 savedviewport;
	m_pDevice->GetViewport(&savedviewport);
	m_pDevice->SetViewport(m_pActiveCamera->GetCameraViewport());

	m_pDevice->BeginScene();

	RenderFirstBlock();

	RenderObjectList();

	if(m_pWorld) {
		m_pWorld->Render(m_pDevice);
	}

	m_pBillboardMgr->Render(m_pDevice);

	if( m_pDecalManager ) {
		m_pDecalManager->Render(m_pDevice);
	}

	m_pPhysicsMgr->Visualize();

	m_pDevice->EndScene();
	m_pDevice->SetViewport(&savedviewport);

	EndPIXEvent();
}

void CBsKernel::RenderTransparentScene(BOOL bIsRenderParticle/* = TRUE*/)
{
	D3DVIEWPORT9 savedviewport;
	m_pDevice->GetViewport(&savedviewport);
	m_pDevice->SetViewport(m_pActiveCamera->GetCameraViewport());

	m_pDevice->BeginScene();

	RenderAlphaObjectList();

	// 일단 물이 그려진 경우에만 한번더 캡쳐 한다.
	if(m_pImageProcess && m_pWater->IsWaterRendered() ) {	
		m_pImageProcess->ScreenCaptureBeforeAlphaBlendPass(m_pDevice);
	}

	if(bIsRenderParticle)
		RenderParticleObjectList();

	m_pDevice->EndScene();
	m_pDevice->SetViewport(&savedviewport);
}

void CBsKernel::RenderSceneUI()
{
	BeginPIXEvent(0, 0, 255, "Render UI");

	D3DVIEWPORT9 savedviewport;
	m_pDevice->GetViewport(&savedviewport);

	m_pDevice->BeginScene();

	if (m_pUIManager) {
		m_pUIManager->RenderBGMovie(m_pDevice);
	}

	if (m_pUIManager) {
		m_pUIManager->Render(m_pDevice);
	}

	// clickfx
	if(m_pClickFxEffect != NULL)
	{
		m_pClickFxEffect->Render(g_BsKernel.GetDevice());
	}

	m_pDevice->EndScene();
	m_pDevice->SetViewport(&savedviewport);

	EndPIXEvent();
}

void CBsKernel::RenderExceptionUI(int nExceptionType)
{
	if (m_pUIManager) {
		m_pUIManager->RenderException(m_pDevice, nExceptionType);
	}
}

void CBsKernel::SetUIException(int nExceptionType)
{
	if (m_pUIManager) {
		m_pUIManager->SetUIException(nExceptionType);
	}
}

void CBsKernel::RenderRTT()
{
	BeginPIXEvent(255, 0, 0, "Render RTT");
	if(m_pRTTManager != NULL) {
		m_pRTTManager->Render(m_pDevice);
	}

	int nCameraCount = m_CameraIndices.size();

	for (int i=0;i<nCameraCount;++i) {
		LockActiveCamera(m_CameraIndices[i]);			
		if( m_pWorld ) {						
			if(m_pShadowMgr && m_bShowShadow) {
				m_pShadowMgr->RenderShadowMap(m_pDevice);
			}			
			if(m_pWater) {
				m_pWater->Render( m_pDevice , GetUpdateListByRender() );
			}
		}		
		UnlockActiveCamera();
	}
	EndPIXEvent();
}

void	CBsKernel::RenderCustom()
{
	D3DVIEWPORT9 savedviewport;
	m_pDevice->GetViewport(&savedviewport);
	m_pDevice->SetViewport(m_pActiveCamera->GetCameraViewport());
	m_pDevice->BeginScene();

	if( m_pShadowVolumeMgr ) {	
		m_pShadowVolumeMgr->RenderShadowVolume( m_pDevice );
	}

	if(m_pFXManager)
		m_pFXManager->Render(m_pDevice);	

	m_pDevice->EndScene();
	m_pDevice->SetViewport(&savedviewport);
}

void CBsKernel::RenderFirstBlock()
{
	// Ignoring Z test and write!!
	// 따로 Alpha blend 처리 안하고 이곳에서 해준다.
	for(int i=0;i<=FIRST_DRAW_BLOCK_COUNT-1;++i) {
		if(m_ppObjectPool[i]) {
			m_ppObjectPool[i]->PreRender(m_pDevice);
			m_ppObjectPool[i]->Render(m_pDevice);
			m_ppObjectPool[i]->RenderAlpha(m_pDevice);
			m_ppObjectPool[i]->PostRender(m_pDevice);
		}
	}

	// Z 무시하는 오브젝트는 Render,RenderAlpha를 동시 하므로 여기시 Reset!
	ResetTransparentObjectList();
}

#ifdef _USE_CONSOLE_COMMAND_
void CBsKernel::RenderText()
{
	m_pDevice->BeginScene();
#ifdef _XBOX	
	for(int i=0;i<m_nStringCount;i++) {
		m_pFont->DrawText(float(m_nStringX[i]), float(m_nStringY[i]), 0xffffffff, m_pStringBuffer[i]);
	}
	m_nStringCount=0;
#else
	RECT rect = {0, 0, 0, 0};

	for(int i=0;i<m_nStringCount;i++) {
		rect.left = m_nStringX[i];
		rect.top  = m_nStringY[i];

		m_pFont->DrawText(NULL, m_pStringBuffer[i] , -1, &rect, DT_LEFT | DT_TOP | DT_NOCLIP, 0xffffffff ); // m_pStringBuffer[i]
	}
	m_nStringCount=0;
#endif
	m_pDevice->EndScene();
}
#endif

void CBsKernel::RenderObjectList()
{
	m_pInstancingMgr->SetCurrentUsingBufferIndex( CBsInstancingMgr::INSTANCING_NORMAL );
	std::vector<int>& RenderObjectList = m_pActiveCamera->GetDrawObjectList(); // 카메라에 등록된 리스트 입니다
	int nRenderCount = RenderObjectList.size();
	CBsObject* pObject = NULL;
	for ( int i=0; i<nRenderCount; i++ ) {
#ifndef _LTCG
		s_nTrackingIndex = i;
#endif
		pObject = GetEngineObjectPtr(RenderObjectList[i]);
		if(pObject) {
			pObject->PreRender(m_pDevice);
			pObject->Render(m_pDevice);
			pObject->PostRender(m_pDevice);
			pObject->ProcessLinkParticle();
			pObject->ProcessLinkFX();
		}
		else {
			BsAssert( 0 && "Invalid Pointer!!");
		}
	}
	m_pInstancingMgr->RenderInstancingObjectList( m_pDevice );
	m_pInstancingMgr->SetCurrentUsingBufferIndex( CBsInstancingMgr::INSTANCING_NONE );
}

void CBsKernel::RenderAlphaObjectList()
{
	int nRenderAlphaCount = m_TransparentObjectList.size();
	CBsObject* pObject = NULL;
	for( int i=0 ; i<nRenderAlphaCount ; ++i) {
#ifndef _LTCG
		s_nTrackingIndex = i;
#endif
		pObject = m_TransparentObjectList[i];
		pObject->PreRenderAlpha(m_pDevice);
		pObject->RenderAlpha(m_pDevice);
		pObject->PostRender(m_pDevice);
	}
	// Transparent Objects의 Render가 끝났으므로 Reset
	ResetTransparentObjectList();
}

//------------------------------------------------------------------------------------------------
void CBsKernel::ResetDoubleBuffer()
{
	g_BsAniCache.ClearCache();
	g_LargeBsAniCache.ClearCache();

	m_UpdateBuffer[ 0 ].clear();
	m_UpdateBuffer[ 1 ].clear();
	PreKernelCommand();
	ProcessKernelCommand();
	CBsObject::FlipRenderBuffer();
	PreKernelCommand();
	ProcessKernelCommand();
	CBsObject::ResetDoubleBuffer();
	CBsUIManager::ResetDoubleBuffer();

#if defined(ENABLE_FULL_STAGE_CLEANOUT)
	CBsAniObject::ClearAllocSaveMatrixBuffer( 0 );
	CBsAniObject::ClearAllocSaveMatrixBuffer( 1 );
#endif
}

void CBsKernel::ResetFrame()
{
	std::vector< int >	*pUpdateList;

	// TODO: Move this into a utility function on C3DDevice.  May want to consider doing
	//       this work just after the call to Present.

#if defined(_XBOX)
	unsigned int maxSamplers = D3DSAMP_MAXSAMPLERS;
	unsigned int maxStreams = D3DMAXSTREAMS;
#else
	D3DCAPS9 caps;
	GetDevice()->GetD3DDevice()->GetDeviceCaps(&caps);

	unsigned int maxSamplers = caps.MaxSimultaneousTextures;
	unsigned int maxStreams = caps.MaxStreams;
#endif

	// Remove any references to textures

	for ( unsigned int samplerIndex = 0; samplerIndex < maxSamplers; ++samplerIndex ) {
		GetDevice()->SetTexture( samplerIndex, NULL );
	}

	// Remove any references to vertex/index buffers

	for ( unsigned int streamIndex = 0; streamIndex < maxStreams; ++streamIndex )
	{
		GetDevice()->SetStreamSource(streamIndex, NULL, 0, 0);
	}

	GetDevice()->SetIndices(NULL);

	// Remove any references to vertex/pixel shaders
	GetDevice()->SetVertexShader(NULL);
	GetDevice()->SetPixelShader(NULL);

	// TODO: Should also release references to render-targets here
	pUpdateList = GetUpdateListByRender();
	pUpdateList->erase( pUpdateList->begin(), pUpdateList->end() );
	ProcessKernelCommand();
	CBsAniObject::ResetAllocSaveMatrixBuffer();	// 순서상으로 FlipRenderBuffer()하기 전에 와야 한다..
	CBsObject::FlipRenderBuffer();
	CBsUIManager::FlipRenderBuffer();

#ifdef _XBOX
	DeferredTextureDelete();
#endif
}

void CBsKernel::Show()
{
	m_pDevice->ShowFrame();
}

//-----------------------------------------------------------------------------
// Name: CBsKernel::InitIsVisibleObjectLite
//			Init IsVisibleObject.
// Arguments:
//			int nCameraIndex
// Returns:
//			bool
//-----------------------------------------------------------------------------
static	BSVECTOR	s_vPlane[ 8 ];
static	BSVECTOR	s_vEye;

void	CBsKernel::InitIsVisibleObjectLite( CBsCamera* pCamera )
{
#ifdef	_XBOX
	pCamera->GetClipPlane( s_vPlane );
	pCamera->GetClipEye( &s_vEye );
	CClipTest::SetClipPlane( s_vPlane );
#endif
}

//-----------------------------------------------------------------------------
// Name: CBsKernel::IsVisibleObjectLite
//			Use after SetVisibleObjectTest.
// Arguments:
//			int		nIndex
//			float*	PfViewDistance
// Returns:
//			bool
//-----------------------------------------------------------------------------
bool	CBsKernel::IsVisibleObjectLite( CBsObject* pObject, float* PfViewDistance , bool *pbShadow )
{
#ifdef	_XBOX
	Box3		B;
	if( false == pObject->IsEnableObjectCull() ) {
		if( pObject->GetMeshPt() ) {
			pObject->GetBox3( B );
			*PfViewDistance	=	BsVec3Length( &(B.C - s_vEye) );
		}
		*pbShadow = true;
		return	true;
	}

	if( pObject->GetBox3( B ) ) {
		//	B.compute_vertices();
		if( pbShadow && pObject->GetShadowCastType() != BS_SHADOW_NONE ) {
			BSVECTOR vCenter = B.C;
			BSVECTOR vDown = BSVECTOR(vCenter.x, vCenter.y - (B.E[1]*B.A[1].y), vCenter.z);

			D3DXVECTOR4 LightDir = m_pShadowMgr->GetLightDirection();

			BSMATRIX matShadow;

			matShadow._11 = LightDir.y;
			matShadow._12 = matShadow._13 = matShadow._14 = 0.f;
			matShadow._21 = -LightDir.x;
			matShadow._23 = -LightDir.z;
			matShadow._22 = matShadow._24 = 0.f;
			matShadow._31 = matShadow._32 = matShadow._34 = 0.f;
			matShadow._33 = LightDir.y;
			matShadow._41 = LightDir.x * vDown.y;
			matShadow._42 = LightDir.y * vDown.y;
			matShadow._43 = LightDir.z * vDown.y;
			matShadow._44 = LightDir.y;

			BsVec3TransformCoord(&vCenter, &vCenter, &matShadow);

			float fRadiusXRadius = BsMax(B.E[0], B.E[2]) + BsVec3LengthSq(&(vDown-vCenter));
			*pbShadow = CClipTest::TestFar1( vCenter, fRadiusXRadius) != FALSE;
		}

		float	fRadius			= pObject->GetMeshRadius();		
		// breakable object's bounding box can bigger than original mesh, so calculate radius every frame
		// scale apply
		BSVECTOR vCorner = B.E[0]*B.A[0] + B.E[1]*B.A[1] + B.E[2]*B.A[2];
		float	fRadiusXRadius	= BsVec3LengthSq(&vCorner);//fRadius*fRadius;
		if( fRadius < 50.f ) {
			if( FALSE == CClipTest::Test( B.C, fRadiusXRadius ) ) {
				return	false;
			}

			*PfViewDistance	=	BsVec3Length( &(B.C - s_vEye) );
		}
		else if( fRadius < 500.f ) {
			if( FALSE == CClipTest::TestFar1( B.C, fRadiusXRadius ) ) {
				return	false;
			}

			*PfViewDistance	=	BsVec3Length( &(B.C - s_vEye) );
		}
		else {
			if( FALSE == CClipTest::TestFar2( B.C, fRadiusXRadius ) ) {
				return	false;
			}

			*PfViewDistance	=	BsVec3Length( &(B.C - s_vEye) );
		}
	}
	else {
		return	false;
	}

	return	true;
#else
	return	true;
#endif
}

bool CBsKernel::IsVisibleObject( int nCameraIndex, int nIndex, float* PfViewDistance)
{
	Box3	B;
	CBsObject* pObject = m_ppObjectPool[nIndex];

	if( !pObject->IsEnableObjectCull() ) {
		if(pObject->GetMeshPt()) {
			pObject->GetBox3(B);
			const Frustum	& frustum = ((CBsCamera*)(m_ppObjectPool[nCameraIndex]))->GetFrustum();
			*PfViewDistance=BsVec3Length( &(B.C - frustum.E) );
		}
		return true;
	}
	if( pObject->GetBox3( B ) ) {
		B.compute_vertices();
		float fRadius = pObject->GetMeshRadius();
		if(fRadius < 50.f) {
			const Frustum	& frustum = ((CBsCamera*)(m_ppObjectPool[nCameraIndex]))->GetFrustum();
			if( TestIntersection( B, frustum ) == false ) {
				return false;
			}
			*PfViewDistance=BsVec3Length( &(B.C - frustum.E) );
		}
		else if(fRadius < 500.f) {
			const Frustum	& frustum = ((CBsCamera*)(m_ppObjectPool[nCameraIndex]))->GetFrustumForBig();
			if( TestIntersection( B, frustum ) == false ) {
				return false;
			}
			*PfViewDistance=BsVec3Length( &(B.C - frustum.E) );
		}
		else {
			const Frustum	& frustum = ((CBsCamera*)(m_ppObjectPool[nCameraIndex]))->GetFrustumForGiant();
			if( TestIntersection( B, frustum ) == false ) {
				return false;
			}
			*PfViewDistance=BsVec3Length( &(B.C - frustum.E) );
		}
	}
	else {
		return false;
	}
	return true;
}

bool CBsKernel::IsVisibleObject( CBsCamera* pCamera, int nIndex, float* PfViewDistance)
{
	Box3	B;
	CBsObject* pObject = m_ppObjectPool[nIndex];

	if( !pObject->IsEnableObjectCull() ) {
		return true;
	}
	if( pObject->GetBox3( B ) ) {
		B.compute_vertices();
		if(pObject->GetMeshRadius() < 50.f) {
			const Frustum	& frustum = pCamera->GetFrustum();
			if( TestIntersection( B, frustum ) == false ) {
				return false;
			}
			*PfViewDistance=BsVec3Length( &(B.C - frustum.E) );
		}
		else {
			const Frustum	& frustum = pCamera->GetFrustumForBig();
			if( TestIntersection( B, frustum ) == false ) {
				return false;
			}
			*PfViewDistance=BsVec3Length( &(B.C - frustum.E) );
		}
	}
	else {
		return false;
	}
	return true;
}

bool CBsKernel::IsVisibleTestFromCamera(int nCameraIndex, Box3* pBox)	// Box3는 반드시 compute_vertices()를 호출된걸로 넘겨야 한다. by jeremy
{
	const Frustum	& frustum = ((CBsCamera*)(m_ppObjectPool[nCameraIndex]))->GetFrustum();
	if( TestIntersection( *pBox, frustum, 0.f ) == false ) {
		return false;
	}
	return true;
}

bool CBsKernel::IsVisibleTestFromCamera( Box3* pBox )
{
	const Frustum	& frustum = m_pActiveCamera->GetFrustum();
	if( TestIntersection( *pBox, frustum, 0.f ) == false ) {
		return false;
	}
	return true;
}

bool CBsKernel::IsVisibleTestFromCameraForGiant( Box3* pBox)
{
	const Frustum	& frustum = m_pActiveCamera->GetFrustumForGiant();
	if( TestIntersection( *pBox, frustum, 0.f ) == false ) {
		return false;
	}
	return true;
}

bool CBsKernel::IsVisibleTestFromCameraForGiantByRender(Box3* pBox)
{
	const Frustum	& frustum = m_pActiveCamera->GetFrustumForGiantByRender();
	if( TestIntersection( *pBox, frustum, 0.f ) == false ) {
		return false;
	}
	return true;
}

void CBsKernel::FrameCheckStart()
{
	m_pChecker->Start();
}

void CBsKernel::LockActiveCamera(int nCameraIndex)
{
	//	CheckValidCameraIndex( nCameraIndex );
	m_pActiveCamera = ((CBsCamera*)m_ppObjectPool[nCameraIndex]);

	// mruete: prefix bug 336: made this if() statement encompass more, and added else{} with assert
	if(m_pActiveCamera) {
		m_pActiveCamera->Render(m_pDevice);
		int nRenderTargetTexture = m_pActiveCamera->GetRenderTargetTexture();
		if(nRenderTargetTexture>=0) {
			CBsTexture* pTexture = m_pTextureList[nRenderTargetTexture];
			LPDIRECT3DSURFACE9 pRenderTarget = pTexture->GetRenderTargetSurface();
			LPDIRECT3DSURFACE9 pDepthStencil = pTexture->GetDepthStencilSurface();
			m_pDevice->SetRenderTarget(0, pRenderTarget);
			m_pDevice->SetDepthStencilSurface( pDepthStencil );
		}
		CBsMaterial::SetSharedParameter(m_pDevice);
	}
	else {
		BsAssert2( false, "CBsKernel::LockActiveCamera() called with invalid nCameraIndex" );
	}
}

void CBsKernel::UnlockActiveCamera()
{

	int nRenderTargetTexture = m_pActiveCamera->GetRenderTargetTexture();
	if(nRenderTargetTexture>=0) {
#ifdef _XBOX
		CBsTexture* pTexture = m_pTextureList[nRenderTargetTexture];
		D3DCOLOR color = m_pActiveCamera->GetClearColor();
		D3DVECTOR4 clear;
		clear.x = 0.f;
		clear.y = 0.f;
		clear.z = 0.f;
		clear.w = 0.f;

		pTexture->Resolve(m_pDevice, D3DRESOLVE_RENDERTARGET0 | D3DRESOLVE_CLEARRENDERTARGET, NULL, NULL, &clear, 1.f, 0L);

		m_pDevice->SetRenderTarget( 0, m_pDevice->GetBackBuffer() );
		m_pDevice->SetDepthStencilSurface( m_pDevice->GetDepthStencilSurface() );
#else
		m_pDevice->SetRenderTarget( 0, m_pDevice->GetBackBuffer() );
		m_pDevice->SetDepthStencilSurface( m_pDevice->GetDepthStencilSurface() );
#endif
	}
	m_pActiveCamera=NULL;
}

const D3DXMATRIX* CBsKernel::GetParamInvViewMatrix()
{
	BsAssert(m_pActiveCamera);
	return m_pActiveCamera->GetObjectMatrix();
}

const D3DXVECTOR4* CBsKernel::GetLightPosition()
{
	if(m_nDirectionalLightIndex == -1)
		return NULL;
	return (D3DXVECTOR4*)&(m_ppObjectPool[m_nDirectionalLightIndex]->GetObjectMatrix()->_41);
}

const D3DXVECTOR4* CBsKernel::GetLightDirection()
{
	// 이거 Point Light정보 가져오는걸로 바꾸어야 합니다. by jeremy
	if(m_nDirectionalLightIndex==-1)
		return NULL;
	return (D3DXVECTOR4*)&(m_ppObjectPool[m_nDirectionalLightIndex]->GetObjectMatrix()->_31);
}

const D3DXVECTOR4* CBsKernel::GetLightDiffuse()
{
	if(m_nDirectionalLightIndex == -1)
		return NULL;
	return (D3DXVECTOR4*)( ( (CBsLightObject *)(m_ppObjectPool[m_nDirectionalLightIndex]) ) ->GetDiffuse()); 
}

const D3DXVECTOR4* CBsKernel::GetLightSpecular()
{
	if(m_nDirectionalLightIndex==-1)
		return NULL;
	return (D3DXVECTOR4*)( ( (CBsLightObject *)(m_ppObjectPool[m_nDirectionalLightIndex]) ) ->GetSpecular()); 
}

const D3DXVECTOR4* CBsKernel::GetLightAmbient()
{
	if(m_nDirectionalLightIndex==-1)
		return NULL;
	return (D3DXVECTOR4*)( ( (CBsLightObject *)(m_ppObjectPool[m_nDirectionalLightIndex]) ) ->GetAmbient());
}

void CBsKernel::InitPointLightObjects()
{
	int nBufferIndex = CBsObject::GetProcessBufferIndex();
	m_ActivePointLightInfo[ nBufferIndex ].clear();
	m_ActivePointLightColor[ nBufferIndex ].clear();

	for( int i = 0; i < m_nPointLightCount; i++) {
		// MAX 개수 넘으면 영향도 높은순으로 골라서 리턴하는것 나중에 추가
		int nIndex = m_PointLightIndices[i];
		float fViewDistance;
		CBsLightObject *pLightObj = (CBsLightObject*)m_ppObjectPool[nIndex];
		if(  IsVisibleObjectLite( pLightObj, &fViewDistance, NULL ) ) {
			m_ActivePointLightInfo[ nBufferIndex ].push_back( *pLightObj->GetPointLightInfo() );
			m_ActivePointLightColor[ nBufferIndex ].push_back( *(D3DXVECTOR4*)pLightObj->GetPointLightColor() );
		}
	}
}

int CBsKernel::ApplyPointLight( D3DXVECTOR3 Pos, float fRadius)
{
	int i, nSize;
	int nSelectNum = 0;

	int nBufferIndex = CBsObject::GetRenderBufferIndex();
	nSize = m_ActivePointLightInfo[nBufferIndex].size();

	for( i = 0; i < nSize; i++) {
		D3DXVECTOR4 PointInfo = m_ActivePointLightInfo[nBufferIndex][i];
		if( PointInfo.w == 0.f || fRadius == 0.f ) continue;

		D3DXVECTOR3 PointPos = D3DXVECTOR3(PointInfo.x, PointInfo.y, PointInfo.z);

		float fRemainLength = ((PointInfo.w+fRadius) * (PointInfo.w+fRadius)) - D3DXVec3LengthSq(&(Pos - PointPos));
		if( fRemainLength > 0.f ) {	
			m_nSelectedPointLights[nSelectNum++] = i;
			if( nSelectNum >= MAX_POINT_LIGHT) break;		
		}
	}
	m_nSelectedPointLights[nSelectNum] = -1;

	return nSelectNum;
}

int CBsKernel::GetActivePointLightNum()
{
	for( int i = 0; i <= MAX_POINT_LIGHT; i++) {
		if(m_nSelectedPointLights[i] == -1) return i;
	}
	return 0;
}

D3DXVECTOR4* CBsKernel::GetActivePointLight(int nIndex)
{
	int nBufferIndex = CBsObject::GetRenderBufferIndex();
	return &m_ActivePointLightInfo[nBufferIndex][nIndex];
}

D3DXVECTOR4* CBsKernel::GetActivePointLightColor(int nIndex)
{
	int nBufferIndex = CBsObject::GetRenderBufferIndex();
	return &m_ActivePointLightColor[nBufferIndex][nIndex];
}

void CBsKernel::UpdatePointLightDecal()
{
	if( !m_pWorld ) return ;
	for( int i = 0; i < m_nPointLightCount; i++) {	
		CBsLightObject *pLightObj = ((CBsLightObject*)GetEngineObjectPtr( m_PointLightIndices[i] ));
		D3DXVECTOR4 pointLightInfo = *pLightObj->GetPointLightInfo();
		D3DCOLORVALUE pointLightColor = *pLightObj->GetPointLightColor();

		float fDist = pointLightInfo.y - GetLandHeight(pointLightInfo.x, pointLightInfo.z);	
		if( fDist > 0.f ) {			
			// 포인트 라이트(구) 가 지면에 닿는 면적(반지름) 계산
			float fRange =  (pointLightInfo.w*pointLightInfo.w) - (fDist*fDist);
			if( fRange > 0.f ) {
				fRange = sqrtf( fRange );
				float fRatio = 1.f - (fDist / pointLightInfo.w);				
				BYTE r = (BYTE)(pointLightColor.r*fRatio*255);
				BYTE g = (BYTE)(pointLightColor.g*fRatio*255);
				BYTE b = (BYTE)(pointLightColor.b*fRatio*255);
				g_BsKernel.AddDecal(pointLightInfo.x, pointLightInfo.z, POINTLIGHT_DECAL, fRange, 0, 0.f, D3DCOLOR_XRGB(r,g,b), 1);
			}
		}
	}
}

void CBsKernel::EnableImageProcess(BOOL bEnable)
{
	if(m_pImageProcess) {
		m_pImageProcess->EnableImageProcess(bEnable);
	}
}

BOOL CBsKernel::IsEnableImageProcess()
{
	if(m_pImageProcess) {
		return m_pImageProcess->IsEnableImageProcess();
	}
	return false;
}

void CBsKernel::EnableShowDebugTexture(BOOL bIsShow)
{
#ifdef _SHOW_DEBUG_TEXTURE_
	if(m_pImageProcess) {
		m_pImageProcess->EnableShowDebugTexture(bIsShow);
	}
#endif //_SHOW_DEBUG_TEXTURE_
}



int	CBsKernel::CreateStaticObject( int nMeshIndex, bool bCreateDummyObject/*=false*/)
{
	int nIndex;

	if((!m_pMeshList[nMeshIndex])&&(!bCreateDummyObject)) {
		return -1;
	}
	nIndex=SearchEmptyObjectIndex();
	m_ppObjectPool[nIndex]=new CBsObject();
	if(!bCreateDummyObject) {
		int nSubMeshCount = m_ppObjectPool[nIndex]->AttachMesh(m_pMeshList[nMeshIndex]);
	}
	m_ppObjectPool[nIndex]->SetKernelPoolIndex(nIndex);
//	DebugString("CreateStaticObject : %d\n", nIndex);

	return nIndex;
}

int	CBsKernel::CreateStaticObjectFromSkin( int nSkinIndex)
{
	int nIndex;

	if (!m_pSkinList[nSkinIndex])
		return -1;

	nIndex=SearchEmptyObjectIndex();
	m_ppObjectPool[nIndex]=new CBsObject();

	m_ppObjectPool[nIndex]->SetKernelPoolIndex(nIndex);
//	DebugString("CreateStaticObjectFromSkin : %d\n", nIndex);

	// Mesh 부착
	CBsMesh* pMesh = m_pMeshList[m_pSkinList[nSkinIndex]->GetMeshIndex()];
	BsAssert(pMesh);
	int nSubMeshCount = m_ppObjectPool[nIndex]->AttachMesh(pMesh);

	// Material 부착
	LinkSkinToObject(nIndex, nSkinIndex);

#ifdef _LOAD_MAP_CHECK_
	m_pSkinList[nSkinIndex]->m_nUseCount++;
#endif

	return nIndex;
}

void CBsKernel::ResetWaterObject()
{
	m_pWater->Clear();
}

int	CBsKernel::CreateWaterObjectFromSkin( int nSkinIndex)
{
	int nIndex;

	if (!m_pSkinList[nSkinIndex])
		return -1;

	nIndex=SearchEmptyObjectIndex();
	m_ppObjectPool[nIndex]=new CBsWaterObject();
	m_ppObjectPool[nIndex]->SetKernelPoolIndex(nIndex);
//	DebugString("CreateWaterObjectFromSkin : %d\n", nIndex);

	// Mesh 부착
	CBsMesh* pMesh = m_pMeshList[m_pSkinList[nSkinIndex]->GetMeshIndex()];
	BsAssert(pMesh);
	int nSubMeshCount = m_ppObjectPool[nIndex]->AttachMesh(pMesh);

	// Water AABB Setting
	m_pWater->AddWaterBoundingBox( pMesh->GetBoundingBox(),  ((CBsWaterObject*)m_ppObjectPool[nIndex])->GetWaterCheckBufferPtr()  );

	// Material 부착
	LinkSkinToObject(nIndex, nSkinIndex);

	return nIndex;
}

bool CBsKernel::IsWaterInside( D3DXVECTOR3 Pos )
{
	if( !m_pWater )
		return false;
	return m_pWater->IsWaterInside( Pos );
}


float CBsKernel::GetWaterHeight()
{
	if( !m_pWater )
		return 0.f;
	return m_pWater->GetWaterHeight();
}

float CBsKernel::GetWaterDistance( D3DXVECTOR3 Pos )
{
	if( !m_pWater )
		return 1000000.f;
	return m_pWater->GetWaterDistance( Pos );
}

int CBsKernel::CreateAniObject(int nMeshIndex, int nAniIndex)
{
	int nIndex;
	CBsAniObject *pEmptySlot;
	CBsMesh *pShadowMesh=NULL;

	if(!m_pMeshList[nMeshIndex]){
		return -1;
	}
	if(!m_pAniList[nAniIndex]){
		return -1;
	}

	nIndex=SearchEmptyObjectIndex();
	pEmptySlot=new CBsAniObject();
	pEmptySlot->AttachMesh(m_pMeshList[nMeshIndex]);
	pEmptySlot->SetAniPtr( m_pAniList[nAniIndex], nAniIndex );

	m_ppObjectPool[nIndex]=pEmptySlot;
	m_ppObjectPool[nIndex]->SetKernelPoolIndex(nIndex);
//	DebugString("CreateAniObject : %d\n", nIndex);

	m_pMeshList[nMeshIndex]->LinkBone(m_pAniList[nAniIndex]);

	return nIndex;	
}

int CBsKernel::CreateAniObjectFromSkin(int nSkinIndex, int nAniIndex)
{
	int nIndex;
	CBsAniObject *pEmptySlot;
	CBsMesh *pShadowMesh=NULL;

	if(!m_pSkinList[nSkinIndex]) {
		return -1;
	}
	if(!m_pAniList[nAniIndex]) {
		return -1;
	}

	nIndex=SearchEmptyObjectIndex();
	pEmptySlot=new CBsAniObject();

	int nMeshIndex = m_pSkinList[nSkinIndex]->GetMeshIndex();

	m_ppObjectPool[nIndex]=pEmptySlot;

	m_ppObjectPool[nIndex]->SetKernelPoolIndex(nIndex);
//	DebugString("CreateAniObjectFromSkin : %d\n", nIndex);

	// Mesh 부착
	CBsMesh* pMesh = m_pMeshList[nMeshIndex];
	BsAssert(pMesh);
	int nSubMeshCount = m_ppObjectPool[nIndex]->AttachMesh(pMesh);

	// Animation 부착
	pEmptySlot->SetAniPtr( m_pAniList[nAniIndex], nAniIndex );
	m_pMeshList[nMeshIndex]->LinkBone(m_pAniList[nAniIndex]);

	// Material 부착
	LinkSkinToObject(nIndex, nSkinIndex);
	( ( CBsAniObject * )m_ppObjectPool[nIndex] )->SetAniFileIndex( nAniIndex );

#ifdef _LOAD_MAP_CHECK_
	m_pSkinList[nSkinIndex]->m_nUseCount++;
#endif

	return nIndex;
}

int CBsKernel::CreateRMAniObjectFromSkin( int nSkinIndex, int nAniIndex )
{
	int nIndex;
	CBsAniObject *pEmptySlot;
	CBsMesh *pShadowMesh=NULL;

	if(!m_pSkinList[nSkinIndex]) {
		return -1;
	}
	if(!m_pAniList[nAniIndex]) {
		return -1;
	}

	nIndex=SearchEmptyObjectIndex();
	pEmptySlot=new CBsRMAniObject();

	int nMeshIndex = m_pSkinList[nSkinIndex]->GetMeshIndex();

	m_ppObjectPool[nIndex]=pEmptySlot;

	m_ppObjectPool[nIndex]->SetKernelPoolIndex(nIndex);
	//	DebugString("CreateRMAniObjectFromSkin : %d\n", nIndex);

	// Mesh 부착
	CBsMesh* pMesh = m_pMeshList[nMeshIndex];
	BsAssert(pMesh);
	int nSubMeshCount = m_ppObjectPool[nIndex]->AttachMesh(pMesh);

	// Animation 부착
	pEmptySlot->SetAniPtr( m_pAniList[nAniIndex], nAniIndex );
	m_pMeshList[nMeshIndex]->LinkBone(m_pAniList[nAniIndex]);

	// Material 부착
	LinkSkinToObject(nIndex, nSkinIndex);
	( ( CBsAniObject * )m_ppObjectPool[nIndex] )->SetAniFileIndex( nAniIndex );

#ifdef _LOAD_MAP_CHECK_
	m_pSkinList[nSkinIndex]->m_nUseCount++;
#endif

	return nIndex;
}

int CBsKernel::CreateCameraObject(int nRenderTargetIndex,  float fStartX, float fStartY, float fWidth, float fHeight, bool bIsRTT/*=false*/)
{
	int nIndex;
	nIndex=SearchEmptyObjectIndex();

	if(bIsRTT) {
		m_ppObjectPool[nIndex]=new CBsRTTCamera(nRenderTargetIndex, fStartX, fStartY, fWidth, fHeight, bIsRTT);
	}
	else {
		m_ppObjectPool[nIndex]=new CBsCamera(nRenderTargetIndex, fStartX, fStartY, fWidth, fHeight, bIsRTT);
		m_CameraIndices.push_back(nIndex);
	}
	m_ppObjectPool[nIndex]->SetKernelPoolIndex(nIndex);
//	DebugString("CreateCameraObject : %d\n", nIndex);

	((CBsCamera*)m_ppObjectPool[nIndex])->AttachDevice(m_pDevice);

	return nIndex;
}

int CBsKernel::CreateLightObject(LIGHT_TYPE type)
{
	if(m_nPointLightCount>=MAX_POINT_LIGHT_COUNT) {
		BsAssert( 0 && "Point Light Count exceed!!" );
		return -1;
	}
	int nIndex;
	nIndex=SearchEmptyObjectIndex();
	m_ppObjectPool[nIndex]=new CBsLightObject();
	m_ppObjectPool[nIndex]->SetKernelPoolIndex(nIndex);
//	DebugString("CreateLightObject : %d\n", nIndex);

	switch( type ) {
	case DIRECTIONAL_LIGHT :
		SetDirectionalLight(nIndex);
		break;
	case POINT_LIGHT:
		BsAssert(m_nPointLightCount < MAX_POINT_LIGHT_COUNT );
		m_PointLightIndices[m_nPointLightCount++] = nIndex;
		break;
	default:
		BsAssert( 0 && "Invalid Light Type!!");
		break;
	}

	((CBsLightObject*)m_ppObjectPool[ nIndex ])->SetLightType( type );

	return nIndex;
}

int CBsKernel::CreateSkyBoxObjectFromSkin( int nSkinIndex)
{
	int i = 0;
	int nIndex = -1;
	if(!m_pSkinList[nSkinIndex])
		return -1;
	if(CBsSkyBoxObject::s_nKernelIndex != -1) {
		DeleteObject(CBsSkyBoxObject::s_nKernelIndex);
		CBsSkyBoxObject::s_nKernelIndex = -1;
	}

	for(i=0;i<FIRST_DRAW_BLOCK_COUNT;i++){
		if(!m_ppObjectPool[i]){
			nIndex=i;
			break;
		}
	}
	if(i==-1){
		return -1;
	}
	m_ppObjectPool[nIndex]=new CBsSkyBoxObject();
	m_ppObjectPool[nIndex]->SetKernelPoolIndex(nIndex);
//	DebugString("CreateSkyBoxObjectFromSkin : %d\n", nIndex);

	CBsSkyBoxObject::s_nKernelIndex = nIndex;

	// Mesh 부착
	CBsMesh* pMesh = m_pMeshList[m_pSkinList[nSkinIndex]->GetMeshIndex()];
	BsAssert(pMesh);
	int nSubMeshCount = m_ppObjectPool[nIndex]->AttachMesh(pMesh);

	// Material 부착
	LinkSkinToObject(nIndex, nSkinIndex);

	return nIndex;
}

int	CBsKernel::LoadFXTemplate( int nFXTemplateIndex, char *pszFXFileName )
{
	BsAssert( pszFXFileName && "Invalid FX Template File Name!" );
	int nResultIndex = -1;

	if(nFXTemplateIndex == -1) {
		int nFindIndex = FindFXTemplate(pszFXFileName);
		if(nFindIndex == -1) {
			nResultIndex = FindEmptyFXTemplateIndex();
		}
		else {
			// 이미 로드된 데이타이므로 AddRef()만 하고, Index 리턴!
			m_pFXTemplateList[nFindIndex]->AddRef();
			return nFindIndex;
		}
	}
	else {
		// specific slot index
		BsAssert( nFXTemplateIndex>=0 && nFXTemplateIndex<MAX_FXTEMPLATE_COUNT);
		if(m_pFXTemplateList[nFXTemplateIndex]) {
			if ( _stricmp(pszFXFileName, m_pFXTemplateList[nFXTemplateIndex]->GetFileName()) != 0 ) {
				BsAssert( 0 && "Index already used for a different FX Template" );
				return -1;
			}
			else {
				m_pFXTemplateList[nFXTemplateIndex]->AddRef();
				return nFXTemplateIndex;
			}
		}
		else {
			// 로드가 안된 데이타 이므로 로드합니다.
			nResultIndex = nFXTemplateIndex;
		}
	}

#ifdef _LOAD_MAP_CHECK_
	MEMORYSTATUS mem, mem2;
	GlobalMemoryStatus(&mem);
#endif

	// Create the new FX template
	CBsFXTemplate *pFXTemplate = new CBsFXTemplate;
	if( pFXTemplate->Load( pszFXFileName ) == FALSE ) {
		delete pFXTemplate;
		return -1;
	}

	BsAssert( m_pFXTemplateList[nResultIndex] == NULL );
	m_pFXTemplateList[nResultIndex] = pFXTemplate;

#ifdef _LOAD_MAP_CHECK_
	GlobalMemoryStatus(&mem2);
	m_pFXTemplateList[nResultIndex]->m_nLoadSize = mem.dwAvailPhys - mem2.dwAvailPhys;
#endif

	return nResultIndex;
}

int CBsKernel::FindFXTemplate(char* pszFXFileName)
{
	for( int i = 0; i < MAX_FXTEMPLATE_COUNT; ++i ) {
		if( m_pFXTemplateList[i] ) {
			if ( _stricmp( pszFXFileName, m_pFXTemplateList[i]->GetFileName() ) == 0 ) {
				return i;
			}
		}
	}
	return -1;
}

int CBsKernel::FindEmptyFXTemplateIndex()
{
	int i;
	for(i=START_ARBITARY_FXTEMPLATE_INDEX ; i<MAX_PARTICLE_GROUP_COUNT ; ++i){
		if(!(m_pFXTemplateList[i])) {
			return i;
		}
	}

	BsAssert(0 && "No find empty FXTemplate slot!");
	return -1;
}

void CBsKernel::AddFXTemplateRef(int nIndex)
{
#ifdef _XBOX
	if(GetCurrentThreadId()==g_dwRenderThreadId) {
		DebugString( "Invalid Thread: AddFXTemplateRef(%d) in Render Thread\n", nIndex);
		_DEBUGBREAK;
	}
#endif

	BsAssert( nIndex>=0 && nIndex<MAX_FXTEMPLATE_COUNT );
	BsAssert( m_pFXTemplateList[nIndex] );

	m_pFXTemplateList[nIndex]->AddRef();
}

void CBsKernel::ReleaseFXTemplate(int nIndex)
{
#ifdef _XBOX
	if(GetCurrentThreadId()==g_dwRenderThreadId) {
		DebugString( "Invalid Thread: ReleaseFXTemplate(%d) in Render Thread\n", nIndex);
		_DEBUGBREAK;
	}
#endif

	if ( nIndex < 0 )
		return;

	BsAssert( nIndex>=0 && nIndex<MAX_FXTEMPLATE_COUNT );
	BsAssert( m_pFXTemplateList[nIndex] );

	m_pFXTemplateList[nIndex]->Release();
}

// This is a debugging function to facilitate catching problems with the reference counting system.  It is there
// to make sure that we are not releasing an FX template that is currently used by an FX object.

bool IsFXTemplateInUse( CBsFXTemplate * fxTemplate )
{
#ifndef _LTCG
	int nObjectCount = g_BsKernel.GetEngineObjectCount();
	for ( int nIndex = 0; nIndex < nObjectCount; ++nIndex ) {
		CBsObject* obj = g_BsKernel.GetEngineObjectPtr( nIndex );
		if( obj ) {
			if ( obj->GetObjectType() == CBsObject::BS_FX_OBJECT ) {
				CBsFXObject* fxObj = (CBsFXObject*)obj;
				if ( fxObj->GetFXTemplate() == fxTemplate )
					return true;
			}
		}
	}
#endif
	return false;
}

void CBsKernel::FlushFXTemplates(void)
{
	BsAssert( !IsRenderThread() );

	// Remove all particles that are either marked as being ready for deletion, or are marked as auto-delete.  The auto-
	// delete particles do not have an owner and need to be removed from the system before we release the FX templates
	// as they could potentially be holding pointers into these templates.

	int nActiveParticleCount = 0;

	int nSize = m_ParticlePool.Size();
	for( int i = 0; i < nSize; ++i ) {
		if ( m_ParticlePool[i]->GetUse() ) {
			if ( m_ParticlePool[i]->GetAutoDelete() || m_ParticlePool[ i ]->IsDelete() ) {
				int nHandle = m_ParticlePool.GetHandle(i);
				m_ParticlePool.DeleteObject( nHandle );

				if ( m_ParticlePool[i]->GetIterate() )
					DeleteIteratePosition( m_ParticlePool[i]->GetSaveIteratePosition(), m_ParticlePool[i]->GetTotalParticleCount() );

				m_ParticlePool[i]->ResetParticleInfo();
			}
			else {
				++nActiveParticleCount;
			}
		}
	}

	//BsAssert( nActiveParticleCount == 0 );

	for ( int nTemplateIndex = 0; nTemplateIndex < MAX_FXTEMPLATE_COUNT; ++nTemplateIndex ) {
		if ( (m_pFXTemplateList[nTemplateIndex] != NULL) && (m_pFXTemplateList[nTemplateIndex]->GetRefCount() <= 0) ) {
			BsAssert( m_pFXTemplateList[nTemplateIndex]->GetRefCount() == 0 );
			BsAssert( !IsFXTemplateInUse(m_pFXTemplateList[nTemplateIndex]) );
#ifndef _LTCG
			char const * templateName = m_pFXTemplateList[nTemplateIndex]->GetFileName();
			DebugString("FlushFXTemplates: deleting FX template : %s (%d)\n", templateName ? templateName : "<unknown>", nTemplateIndex);
#endif
			delete m_pFXTemplateList[ nTemplateIndex ];
			m_pFXTemplateList[ nTemplateIndex ] = NULL;
		}
	}
}

void CBsKernel::DeleteFXTemplate( int nIndex )
{
	if( nIndex < 0 || nIndex >= MAX_FXTEMPLATE_COUNT ) {
		BsAssert( 0 && "Invalid FX Template Index!");
		return;
	}
	delete m_pFXTemplateList[nIndex];
	m_pFXTemplateList[nIndex] = NULL;
}

void CBsKernel::DeleteAllFXTemplates()
{
	for(int i=0;i<MAX_FXTEMPLATE_COUNT;++i) {
		SAFE_DELETE(m_pFXTemplateList[i]);
	}
}

CBsFXTemplate* CBsKernel::GetFXTemplatePtr( int nIndex )
{
	if( nIndex < 0 || nIndex >= MAX_FXTEMPLATE_COUNT ) {
		BsAssert( 0 && "Invalid FXTemplate Index!!" );
		return -0;
	}
	if( m_pFXTemplateList[nIndex] == NULL ) {
		BsAssert( 0 && "Can't reference at Invalid FXTemplate Index!!" );
		return 0;
	}

	return m_pFXTemplateList[nIndex];
}


int	CBsKernel::CreateFXObject( int nFXTemplateIndex )
{
	if( nFXTemplateIndex < 0 || nFXTemplateIndex >= MAX_FXTEMPLATE_COUNT ) {
		DebugString("Error : CreateFXObject(%d) => Invalid FXTemplate Index!!", nFXTemplateIndex);
		BsAssert( 0 && "Error : CreateFXObject => Invalid FXTemplate Index!!" );
		return -1;
	}
	if( m_pFXTemplateList[nFXTemplateIndex] == NULL ) {
		BsAssert( 0 && "Error : CreateFXObject => Invalid FXTemplate Data!!" );
		return -1;
	}

	int nIndex;

	nIndex=SearchEmptyObjectIndex();
	m_ppObjectPool[nIndex]=new CBsFXObject();
	m_ppObjectPool[nIndex]->SetKernelPoolIndex(nIndex);
//	DebugString("CreateFXObject & nFXTemplateIndex: %d , %d \n", nIndex, nFXTemplateIndex);

	((CBsFXObject*)m_ppObjectPool[nIndex])->Initialize( m_pFXTemplateList[nFXTemplateIndex] );

	return nIndex;
}

void CBsKernel::AttachCustomRender(CBsCustomRender* pRender)
{
	if(m_pFXManager) {
		BsAssert(0 && "Error!!");
	}
	m_pFXManager = pRender;
}

void CBsKernel::DetachCustomRender()
{
	if(m_pFXManager) {
		m_pFXManager = NULL;
	}
	else {
		BsAssert( 0 && "Error!!");
	}
}

void CBsKernel::AddDecal(float fX, float fZ, int nTexture, float fRadius, int nLifeTime, float fRotAngle, D3DCOLOR dwColor, int nMode)
{ 
	m_pDecalManager->AddDecal(fX, fZ, nTexture, fRadius, nLifeTime, fRotAngle, dwColor, nMode); 
}

void CBsKernel::ResetDecalObject()
{
	m_pDecalManager->ClearAll();
}

void CBsKernel::DeleteObject( int nObjectIndex )
{
#ifdef _XBOX
#ifndef _USAGE_TOOL_
	if(GetCurrentThreadId()==g_dwRenderThreadId) {
		DebugString("Invalid call : DeleteObject(%d) in Render Thread\n", nObjectIndex);
	}
#endif
#endif

	if( nObjectIndex == -1 ) return;

	int i, nSize;
	nSize = m_PreCalcAniObjList.size();
	for( i = 0; i < nSize; i++) {
		if( m_PreCalcAniObjList[i] == nObjectIndex ) {
			m_PreCalcAniObjList.erase( m_PreCalcAniObjList.begin() + i);
			break;
		}
	}

	CBsObject *pObject = m_ppObjectPool[ nObjectIndex ];
	if( pObject ) {
		pObject->Release();
	}

#if defined(WIN32) && defined(_USAGE_TOOL_)
	DeallocateObject(nObjectIndex);
#else

#ifdef _XBOX
	if( GetCurrentThreadId() == g_dwRenderThreadId ) {
		DeallocateObject(nObjectIndex);
	}
	else {
		KERNEL_COMMAND Command;

		int nBufferIndex = CBsObject::GetProcessBufferIndex();

		Command.nCommand = KERNEL_COMMAND_DELETE_OBJECT;
		Command.nBuffer[ 0 ] = nObjectIndex;
		m_PostKernelCommand[ nBufferIndex ].push_back( Command );
	}
#else
	KERNEL_COMMAND Command;

	int nBufferIndex = CBsObject::GetProcessBufferIndex();

	Command.nCommand = KERNEL_COMMAND_DELETE_OBJECT;
	Command.nBuffer[ 0 ] = nObjectIndex;
	m_PostKernelCommand[ nBufferIndex ].push_back( Command );
#endif	// _XBOX

#endif
}

void CBsKernel::DeletePointLightIndex( int nIndex )
{
	int i;	
	for( i = 0; i < m_nPointLightCount; i++ ) {
		if( m_PointLightIndices[ i ] == nIndex ) {
			memcpy(m_PointLightIndices+i, m_PointLightIndices+i+1, sizeof(int)*(m_nPointLightCount-i-1));
			m_PointLightIndices[ m_nPointLightCount - 1 ] = -1;
			m_nPointLightCount--;
			break;
		}
	}
}

void CBsKernel::DeleteCameraIndex( int nIndex )
{
	int i, nSize;
	nSize = ( int ) m_CameraIndices.size();
	for( i = 0; i < nSize; i++ ) {
		if( m_CameraIndices[ i ] ==  nIndex ) {
			m_CameraIndices.erase( m_CameraIndices.begin()+i );
			break;
		}
	}
}

void CBsKernel::DeleteAllObject()
{
	for(int i=0;i<m_nPoolSize;i++) {
		DeallocateObject(i);
	}	
	m_CameraIndices.clear();
}

int CBsKernel::SendMessage(int nObjectIndex, DWORD dwCode, DWORD dwParam1/*=0*/, DWORD dwParam2/*=0*/, DWORD dwParam3/*=0*/)
{
	BsAssert( nObjectIndex>=0 && "Invalid Object Index!!" );

	switch( nObjectIndex & 0xf0000000 )
	{
	case 0: // StrObject Process Message
		return m_ppObjectPool[nObjectIndex]->ProcessMessage(dwCode, dwParam1, dwParam2, dwParam3);
	case BS_PARTICLE_PROCESS: // ParticleObject Process Message
		return 0;
	case BS_KERNEL_PROCESS: // Kernel Process Message
		return ProcessMessage(dwCode, dwParam1, dwParam2, dwParam3 );
	default:
		BsAssert( !"Error Invalid Handle!!");
		break;
	}
	return 0;
}

int CBsKernel::CreateHeightField( int nXSize, int nZSize, short int* pHeight, const char **ppFileList, const char* pszLightMapFileName, DWORD *pMulColor, DWORD *pAddColor)
{
	if( m_pWorld ) {
		delete m_pWorld;
	}

	m_pWorld = new CBsLODWorld();
	( ( CBsLODWorld * ) m_pWorld )->Create( nXSize, nZSize, pHeight, ppFileList, pszLightMapFileName, pMulColor, pAddColor );	

	return 1;
}

void CBsKernel::CreateHeightFieldActor( int nXSize, int nZSize, short int *pHeight )
{
	m_pHFActor = CBsPhysicsHeightField::Create( nXSize, nZSize, pHeight);
	m_pPlaneActor[0] = CBsPhysicsPlane::Create( D3DXPLANE(1, 0, 0, 0) );
	m_pPlaneActor[1] = CBsPhysicsPlane::Create( D3DXPLANE(0, 0, 1, 0) );
	m_pPlaneActor[2] = CBsPhysicsPlane::Create( D3DXPLANE(-1, 0, 0, -nXSize * 200.f) );
	m_pPlaneActor[3] = CBsPhysicsPlane::Create( D3DXPLANE(0, 0, -1, -nZSize * 200.f) );
}

void CBsKernel::DeleteWorld()
{
	SAFE_DELETE( m_pWorld );
	SAFE_DELETE( m_pHFActor );
	for( int i = 0; i < 4; i++) {
		SAFE_DELETE( m_pPlaneActor[i] );
	}
}

float CBsKernel::GetLandHeight( float fX, float fZ, D3DXVECTOR3* N/* = NULL*/ )
{
	if(m_pWorld) {
		return ((CBsHFWorld *)m_pWorld)->GetLandHeight(fX, fZ, N);
	}
	else {
		return 0.0f;
	}
}

float CBsKernel::GetFlatHeight( int nX, int nZ)
{
	if(m_pWorld){
		return ((CBsHFWorld *)m_pWorld)->GetFlatHeight(nX, nZ);
	}
	else{
		return 0;
	}
}

float CBsKernel::GetHeight(float fX, float fZ)
{
	return ((CBsHFWorld*)m_pWorld)->GetHeight(fX, fZ);
}

void CBsKernel::GetLandSlope( float fX, float fZ, float& fVX, float& fVZ )
{
}

void CBsKernel::Reload()
{
	ReloadAllBsVertexDeclaration();
	ReloadAllMaterial();

	ReloadAllTexture();

	ReloadAllMesh();
	ReLoadAllParticleGroup();

	for(int i=0;i<m_nPoolSize;i++){
		if(m_ppObjectPool[i]){
			m_ppObjectPool[i]->ProcessMessage(BS_REINIT_OBJECT);
		}
	}

	if(m_pWorld) {
		((CBsHFWorld*)m_pWorld)->Reload();
	}

	if(m_pShadowMgr) {
		m_pShadowMgr->Reload();
	}

	if(m_pImageProcess) {
		m_pImageProcess->Reload();
	}

	if(m_pRTTManager)
	{
		m_pRTTManager->ReInitialize();
	}

	//---------------
	if (m_pUIManager)
		m_pUIManager->ReInitialize();
}

HRESULT CBsKernel::CreateEffectFromFile(const char* pszFileName, LPD3DXEFFECT* ppEffect, LPD3DXEFFECTPOOL pEffectPool/*=NULL*/)
{
	return m_pDevice->CreateEffectFromFile(pszFileName, ppEffect, pEffectPool);
}

// return value: -1=>BM 파일 Load Fail, -2=>딴 이름으로 Load된 Mesh index(Load Fail)
int CBsKernel::LoadMesh(int nIndex, const char* pFileName)
{
	int nRet = 0;
	char *pFullName;

	BsAssert( nIndex < MAX_MESH_COUNT );

	if(!pFileName) {
		return -1;
	}

	if(nIndex==-1) {
		nIndex=FindMesh(pFileName);
		if(nIndex==-1){
			nIndex=FindEmptyMeshSlot();
		}
		else{
			m_pMeshList[nIndex]->AddRef();			
			return nIndex;
		}
	}

	pFullName=m_pDirManager->GetFullName(pFileName);
	if(m_pMeshList[nIndex]){
		if(_strcmpi(pFileName, m_pMeshList[nIndex]->GetMeshFileName())==0){
			m_pMeshList[nIndex]->AddRef();
			return nIndex;
		}
		else {
			BsAssert( false && "Different mesh already in mesh slot" );
			return -1;
		}
	}

	DWORD dwFileSize;
	VOID *pData;

#ifdef ENABLE_RES_CHECKER
	g_BsResChecker.AddResList( pFullName );
#endif

	if( FAILED(CBsFileIO::LoadFile( pFullName, &pData, &dwFileSize ) ) ) {
		CBsConsole::GetInstance().AddFormatString("%s File Not Found", pFullName);
		return -1;
	}

	BMemoryStream Stream(pData, dwFileSize);
	m_pMeshList[nIndex]=new CBsMesh();
	nRet=m_pMeshList[nIndex]->LoadMesh(&Stream);
	m_pMeshList[nIndex]->SetMeshFileName(pFullName);

	CBsFileIO::FreeBuffer(pData);

	if(!nRet) {
		delete m_pMeshList[nIndex];
		m_pMeshList[nIndex]=NULL;
		return -1;
	}

	return nIndex;
}

int CBsKernel::FindMesh(const char *pFileName)
{
	int i;
	const char *pSource;
	pSource=strrchr(pFileName, '\\');
	if(pSource){
		pSource++;
	}
	else{
		pSource=pFileName;
	}
	for(i=0;i<MAX_MESH_COUNT;i++){
		if(m_pMeshList[i]){
			if(strcmp(pSource, m_pMeshList[i]->GetMeshFileName())==0){
				return i;
			}
		}
	}

	return -1;
}

int CBsKernel::FindEmptyMeshSlot()
{
	for(int i=0;i<MAX_MESH_COUNT;i++){
		if(!m_pMeshList[i]){
			return i;
		}
	}
	BsAssert(0 && "No find mesh slot!!");
	return -1;
}

void CBsKernel::ReleaseMesh( int nIndex )
{
	if(m_pMeshList[nIndex]){
		DebugString("DeleteMesh : [%s]\n", m_pMeshList[nIndex]->GetMeshFileName());
		int nRefCount = m_pMeshList[nIndex]->Release();
		if( nRefCount <= 0 ) {
			m_pMeshList[nIndex]=NULL;
		}
		DebugString("Reference Count : %d\n", nRefCount);
	}
}

void CBsKernel::DeleteMesh( int nIndex )
{
	if(m_pMeshList[nIndex]){
		delete m_pMeshList[nIndex];
		m_pMeshList[nIndex] = NULL;
	}
}

void CBsKernel::DeleteAllMesh()
{
	for(int i=0; i< MAX_MESH_COUNT; ++i) {
		DeleteMesh(i);
	}
}

void CBsKernel::ReloadAllMesh()
{
	for(int i=0;i<MAX_MESH_COUNT;i++){
		if(m_pMeshList[i]){
			m_pMeshList[i]->Reload();
		}
	}
}

int CBsKernel::FindAni(const char *pFileName)
{
	int i;
	const char *pSource;

	pSource=strrchr(pFileName, '\\');
	if(pSource){
		pSource++;
	}
	else{
		pSource=pFileName;
	}
	for(i=0;i<MAX_ANI_COUNT;i++){
		if(m_pAniList[i]){
			if(strcmp(pSource, m_pAniList[i]->GetAniFileName())==0){
				return i;
			}
		}
	}

	return -1;
}

int CBsKernel::LoadAni(int nIndex, const char *pFileName, bool bUseRootBone/*=true*/)
{
	BsAssert( nIndex == -1 );

	int nRet = 0;
	char *pFullName;

	if(nIndex==-1){
		nIndex=FindAni(pFileName);
		if(nIndex==-1){
			nIndex=FindEmptyAniIndex();
		}
		else{
			m_pAniList[nIndex]->AddRef();
			return nIndex;
		}
	}

	pFullName=m_pDirManager->GetFullName(pFileName);
	if(m_pAniList[nIndex]){
		if(_strcmpi(pFileName, m_pAniList[nIndex]->GetAniFileName())==0){
			m_pAniList[nIndex]->AddRef();
			return nIndex;
		}
		else {
			BsAssert( false && "Different animation already in animation slot" );
			return -1;
		}
	}

#ifdef _LOAD_MAP_CHECK_
	MEMORYSTATUS mem, mem2;
	GlobalMemoryStatus(&mem);
#endif
	DWORD dwFileSize;
	VOID *pData;


#ifdef ENABLE_MEM_CHECKER
	char cStr[256];
	sprintf_s( cStr, _countof(cStr), "Load Ani %s", pFileName );
	g_BsMemChecker.Start( cStr );
#endif

#ifdef ENABLE_RES_CHECKER
	g_BsResChecker.AddResList( pFullName );
#endif

	if( FAILED(CBsFileIO::LoadFile( pFullName, &pData, &dwFileSize ) ) ) {
		BsAssert( m_pAniList[nIndex] == NULL );
		CBsConsole::GetInstance().AddFormatString("%s File Not Found", pFullName);

#ifdef ENABLE_MEM_CHECKER
		g_BsMemChecker.End();
#endif
		return -1;
	}

	BMemoryStream Stream(pData, dwFileSize);
	m_pAniList[nIndex]=new CBsAni();
	nRet=m_pAniList[nIndex]->LoadSAF( &Stream, bUseRootBone );
	m_pAniList[nIndex]->SetAniFileName(pFileName);

	CBsFileIO::FreeBuffer(pData);

	if(!nRet){
		delete m_pAniList[nIndex];
		m_pAniList[nIndex]=NULL;

#ifdef ENABLE_MEM_CHECKER
		g_BsMemChecker.End();
#endif
		return -1;
	}

#ifdef ENABLE_MEM_CHECKER
	g_BsMemChecker.End();
#endif

#ifdef _LOAD_MAP_CHECK_
	GlobalMemoryStatus(&mem2);
	m_pAniList[nIndex]->m_nLoadSize = mem.dwAvailPhys - mem2.dwAvailPhys;
#endif
	return nIndex;
}

int CBsKernel::FindEmptyAniIndex()
{
	for(int i=0;i<MAX_ANI_COUNT;i++){
		if(!(m_pAniList[i])){
			return i;
		}
	}

	BsAssert(0 && "No find ani slot!");
	return -1;
}

void CBsKernel::ReleaseAni(int nIndex)
{
	if ( nIndex < 0 )
		return;

	if ( m_pAniList[nIndex] == 0 )
		return;

	m_pAniList[nIndex]->Release();
}

void CBsKernel::FlushAnis(void)
{
	BsAssert( !IsRenderThread() );

	for ( int nAniIndex = 0; nAniIndex < MAX_ANI_COUNT; ++nAniIndex ) {
		if ( (m_pAniList[ nAniIndex ] != NULL) && (m_pAniList[ nAniIndex ]->GetRefCount() <= 0) ) {
			BsAssert( m_pAniList[ nAniIndex ]->GetRefCount() == 0 );

#if !defined(LTCG)
			char const * aniName = m_pAniList[nAniIndex]->GetAniFileName();
			DebugString("FlushAnis: deleting ani : %s (%d)\n", aniName ? aniName : "<unknown>", nAniIndex);
#endif
			delete m_pAniList[ nAniIndex ];
			m_pAniList[ nAniIndex ] = 0;
		}
	}
}

void CBsKernel::DeleteAni(int nIndex)
{
	if(m_pAniList[nIndex]){
		delete m_pAniList[nIndex];
		m_pAniList[nIndex]=NULL;
	}
}

void CBsKernel::DeleteAllAni()
{
	int i;

	for(i=0;i<MAX_ANI_COUNT;i++){
		DeleteAni(i);
	}
}

void CBsKernel::DeleteBone(int nAniIndex, int nBoneIndex)
{
	BsAssert(m_pAniList[nAniIndex]);
	m_pAniList[nAniIndex]->DeleteBone(nBoneIndex);
}

int	CBsKernel::GetAniCount(int nAniIndex)
{
	return m_pAniList[nAniIndex]->GetAniCount();
}

int CBsKernel::LoadMaterial(const char* pFileName, BOOL bUseMaterialSystem/*=TRUE*/)
{
	int nIndex;
	char *pFullName;
	HRESULT hr;

	nIndex=FindMaterial(pFileName);
	if(nIndex!=-1){
		m_pMaterialList[nIndex]->AddRef();
		return nIndex;
	}
	nIndex=FindEmptyMaterialIndex();
	BsAssert(nIndex!=-1);
	m_pMaterialList[nIndex]=new CBsMaterial();

	pFullName=m_pDirManager->GetFullName(pFileName);

	hr=m_pMaterialList[nIndex]->LoadMaterial(pFullName, bUseMaterialSystem);
	if(hr!=S_OK) {
#ifdef _USAGE_TOOL_
		DebugString( "File [%s] Failed Compile Effect File!!\n",pFullName);
#else
		BsAssert( 0 && "Failed Compile Effect File!!");
#endif
		delete m_pMaterialList[nIndex];
		m_pMaterialList[nIndex] = NULL;
		return -1;
	}

	return nIndex;
}

int CBsKernel::FindMaterial(const char* pFullFileName)
{
	int i;
	const char* pFileName=strrchr(pFullFileName, '\\');

	if(pFileName) {
		pFileName++;
	}
	else {
		pFileName = pFullFileName;
	}

	for(i=0;i<MAX_MATERIAL_COUNT;i++){
		if(m_pMaterialList[i]){
			if(strcmp(m_pMaterialList[i]->GetMaterialName(), pFileName)==0){
				return i;
			}
		}
	}
	return -1;
}

int CBsKernel::FindEmptyMaterialIndex()
{
	for(int i=0;i<MAX_MATERIAL_COUNT;i++) {
		if(!(m_pMaterialList[i])) {
			return i;
		}
	}
	return -1;
}

void CBsKernel::ReleaseMaterial(int nIndex)
{
	if(nIndex < 0)
		return;

	if(m_pMaterialList[nIndex]){
		m_pMaterialList[nIndex]->Release();
	}
}

void CBsKernel::DeleteMaterial(int nMaterialIndex)
{

	if(m_pMaterialList[nMaterialIndex]){
		delete m_pMaterialList[nMaterialIndex];
		m_pMaterialList[nMaterialIndex]=NULL;
	}
}

void CBsKernel::DeleteAllMaterial()
{
	for(int i=0;i<MAX_MATERIAL_COUNT;i++){
		if(m_pMaterialList[i]){
			DeleteMaterial(i);
		}
	}
}

void CBsKernel::ReloadAllMaterial()
{
	for(int i=0;i<MAX_MATERIAL_COUNT;i++){
		if(m_pMaterialList[i]){
			m_pMaterialList[i]->Reload();
		}
	}
}

void CBsKernel::FlushMaterials()
{
	for(int i=0 ; i<MAX_MATERIAL_COUNT; ++i ) {
		if( m_pMaterialList[i] && (m_pMaterialList[i]->GetRefCount() <= 0) ) {
			BsAssert( m_pMaterialList[i]->GetRefCount() == 0 );
#ifndef _LTCG
			DebugString("FlushMaterials: delete CBsMaterial(%d)\n", i);
#endif
			delete m_pMaterialList[i];
			m_pMaterialList[i] = NULL;
		}
	}
}

void CBsKernel::ReloadAllBsVertexDeclaration()
{
	for(int i=0;i<MAX_VERTEXDECLARATION_COUNT;i++){
		if(m_pVertexDeclList[i]){
			m_pVertexDeclList[i]->Reload();
		}
	}
}

int CBsKernel::LoadVertexDeclaration(D3DVERTEXELEMENT9* pVertexElements)
{
	int nIndex;
	HRESULT hr;

	nIndex=FindVertexDeclaration(pVertexElements);
	if(nIndex!=-1){
		m_pVertexDeclList[nIndex]->AddRef();
		return nIndex;
	}
	nIndex=FindEmptyVertexDeclarationIndex();
	BsAssert( nIndex!=-1 && "No empty skin slot!!");

	m_pVertexDeclList[nIndex]=new CBsVertexDeclaration();

	hr=m_pVertexDeclList[nIndex]->LoadVertexDeclaration(pVertexElements);
	if(hr!=S_OK){
		BsAssert( 0 && "Failed Create Vertex Declaration");
		delete m_pVertexDeclList[nIndex];
		m_pVertexDeclList[nIndex] = NULL;
		return -1;
	}
	return nIndex;
}

int	CBsKernel::FindVertexDeclaration(D3DVERTEXELEMENT9* pVertexElements)
{
	int i;

	for(i=0;i<MAX_VERTEXDECLARATION_COUNT;++i){
		if(m_pVertexDeclList[i]){
			if(m_pVertexDeclList[i]->IsEqualVertexElements(pVertexElements)) {
				return i;
			}
		}
	}
	return -1;
}

int	CBsKernel::FindEmptyVertexDeclarationIndex()
{
	for(int i=0;i<MAX_VERTEXDECLARATION_COUNT;i++){
		if(!(m_pVertexDeclList[i])){
			return i;
		}
	}
	return -1;
}

void CBsKernel::ReleaseVertexDeclaration(int nIndex)
{
	if(nIndex < 0)
		return;
	BsAssert(m_pVertexDeclList[nIndex]);
	m_pVertexDeclList[nIndex]->Release();
}

void CBsKernel::FlushVertexDeclarations()
{
	BsAssert( !IsRenderThread() );

	for ( int nVertexDeclIndex = 0; nVertexDeclIndex < MAX_VERTEXDECLARATION_COUNT; ++nVertexDeclIndex ) {
		if ( (m_pVertexDeclList[ nVertexDeclIndex ] != NULL) && (m_pVertexDeclList[ nVertexDeclIndex ]->GetRefCount() <= 0) ) {
			BsAssert( m_pVertexDeclList[ nVertexDeclIndex ]->GetRefCount() == 0 );

#ifndef _LTCG
			DebugString("FlushVertexDeclarations: deleting VertexDeclaraion(%d)\n", nVertexDeclIndex);
#endif
			delete m_pVertexDeclList[ nVertexDeclIndex ];
			m_pVertexDeclList[ nVertexDeclIndex ] = NULL;
		}
	}
}

int CBsKernel::LoadTexture(const char *pFileName, DWORD dwUsage /* =0 */, const bool bGetFullName /*= true*/)
{
	int nIndex, nRet=0;
	char *pFullName;
	HRESULT hr;

	nIndex=FindTexture(pFileName);
#ifdef _USAGE_TOOL_
	if (g_nSkipSameTexture) {
		nIndex=-1;
		g_nSkipSameTexture=0;
	}
#endif
	if(nIndex!=-1){
		m_pTextureList[nIndex]->AddRef();
		return nIndex;
	}
	nIndex=FindEmptyTextureIndex();
	BsAssert(nIndex!=-1);
	m_pTextureList[nIndex]=new CBsTexture();

	if(bGetFullName)
		pFullName=m_pDirManager->GetFullName(pFileName);
	else
		pFullName = (char*)pFileName;

#ifdef ENABLE_MEM_CHECKER
	char cStr[256];
	sprintf_s( cStr, _countof(cStr), "Load Texture : %s", pFileName );
	g_BsMemChecker.Start( cStr );
#endif

#ifdef ENABLE_RES_CHECKER
	g_BsResChecker.AddResList( pFullName );
#endif
	if( !m_pTextureList[nIndex] ) {	// 이값이 널인 경우가 있다.. 버그 확인후 삭제.. by mapping
		_DEBUGBREAK;
	}

	hr=m_pTextureList[nIndex]->LoadTexture(m_pDevice, pFullName, TEXTURE_NORMAL, dwUsage);
	if(hr!=S_OK) {
		DebugString("%s Texture File Not Found!!\n", pFullName);
		delete m_pTextureList[nIndex];
		m_pTextureList[nIndex] = NULL;
		nIndex = -1;
	}

#ifdef ENABLE_MEM_CHECKER
	g_BsMemChecker.End();
#endif

	return nIndex;
}

int	CBsKernel::LoadCubeTexture( const char* pFileName)
{
	int nIndex, nRet=0;
	char *pFullName;
	HRESULT hr;

	nIndex=FindTexture(pFileName);
#ifdef _USAGE_TOOL_
	if (g_nSkipSameTexture) {
		nIndex=-1;
		g_nSkipSameTexture=0;
	}
#endif
	if(nIndex!=-1){
		m_pTextureList[nIndex]->AddRef();
		return nIndex;
	}
	nIndex=FindEmptyTextureIndex();
	BsAssert(nIndex!=-1);
	m_pTextureList[nIndex]=new CBsTexture();

	pFullName=m_pDirManager->GetFullName(pFileName);

#ifdef ENABLE_MEM_CHECKER
	char cStr[256];
	sprintf_s( cStr, _countof(cStr), "Load cube texture : %s", pFileName );
	g_BsMemChecker.Start( cStr );
#endif

#ifdef ENABLE_RES_CHECKER
	g_BsResChecker.AddResList( pFullName );
#endif

	hr=m_pTextureList[nIndex]->LoadTexture(m_pDevice, pFullName, TEXTURE_CUBE);
	if(hr!=S_OK) {
		DebugString("%s Cube Texture File Not Found!!\n", pFullName);
		delete m_pTextureList[nIndex];
		m_pTextureList[nIndex] = NULL;
		nIndex = -1;
	}

#ifdef ENABLE_MEM_CHECKER
	g_BsMemChecker.End();
#endif

	return nIndex;
}

int CBsKernel::LoadVolumeTexture( const char* pFileName)
{
	int nIndex, nRet=0;
	char *pFullName;
	HRESULT hr;

	nIndex=FindTexture(pFileName);
#ifdef _USAGE_TOOL_
	if (g_nSkipSameTexture) {
		nIndex=-1;
		g_nSkipSameTexture=0;
	}
#endif
	if(nIndex!=-1){
		m_pTextureList[nIndex]->AddRef();
		return nIndex;
	}
	nIndex=FindEmptyTextureIndex();
	BsAssert(nIndex!=-1);
	m_pTextureList[nIndex]=new CBsTexture();

	pFullName=m_pDirManager->GetFullName(pFileName);

#ifdef ENABLE_MEM_CHECKER
	char cStr[256];
	sprintf_s( cStr, _countof(cStr), "Load volume texture : %s", pFileName );
	g_BsMemChecker.Start( cStr );
#endif

#ifdef ENABLE_RES_CHECKER
	g_BsResChecker.AddResList( pFullName );
#endif

	hr=m_pTextureList[nIndex]->LoadTexture(m_pDevice, pFullName, TEXTURE_VOLUME);
	if(hr!=S_OK) {
		DebugString("%s Volume Texture File Not Found!!\n", pFullName);
		delete m_pTextureList[nIndex];
		m_pTextureList[nIndex] = NULL;
		nIndex = -1;
	}

#ifdef ENABLE_MEM_CHECKER
	g_BsMemChecker.End();
#endif

	return nIndex;
}

// add by yooty
void CBsKernel::GetTexResInfo( char* pFileName, D3DRESOURCETYPE* pType, UINT* pDepth )
{
#if defined(_XBOX) && defined(_PACKED_RESOURCES)
	BYTE *pTextureResource;
	if((pTextureResource = CBsFileIO::BsFileIsPhysicalLoaded(pFileName)) != NULL) {
		D3DBaseTexture *pTexture = m_pDevice->RegisterPreLoadedTexture(pTextureResource);
		XGTEXTURE_DESC Desc;
		ZeroMemory( &Desc, sizeof(XGTEXTURE_DESC) );
		XGGetTextureDesc( (D3DBaseTexture*)pTexture, 0, &Desc );
		delete pTexture;
		*pType = Desc.ResourceType;
		*pDepth = Desc.Depth;
	}
	else {
		D3DXIMAGE_INFO ImageInfo;
		D3DXGetImageInfoFromFile( pFileName, &ImageInfo );
		*pType = ImageInfo.ResourceType;
		*pDepth = ImageInfo.Depth;
	}
#else
	D3DXIMAGE_INFO ImageInfo;
	D3DXGetImageInfoFromFile( pFileName, &ImageInfo );
	*pType = ImageInfo.ResourceType;
	*pDepth = ImageInfo.Depth;
#endif
}

#ifdef _XBOX
int CBsKernel::LoadArrayTexture(const char* pFolderName)
{
	int nIndex, nRet=0;
	char *pFullName;
	HRESULT hr;

	nIndex=FindTexture(pFolderName);

	if(nIndex!=-1){
		m_pTextureList[nIndex]->AddRef();
		return nIndex;
	}
	nIndex=FindEmptyTextureIndex();
	BsAssert(nIndex!=-1);
	m_pTextureList[nIndex]=new CBsTexture();

	pFullName=m_pDirManager->GetFullName(pFolderName);

#ifdef ENABLE_MEM_CHECKER
	char cStr[256];
	sprintf_s( cStr, _countof(cStr), "Load array texture : %s", pFolderName );
	g_BsMemChecker.Start( cStr );
#endif

	hr=m_pTextureList[nIndex]->LoadArrayTexture(m_pDevice, pFolderName);
	if(hr!=S_OK) {
		delete m_pTextureList[nIndex];
		m_pTextureList[nIndex] = NULL;
		nIndex = -1;
	}

#ifdef ENABLE_MEM_CHECKER
	g_BsMemChecker.End();
#endif

	return nIndex;
}
#endif

int CBsKernel::CreateTexture(int nWidth, int nHeight, DWORD dwUsage, D3DFORMAT format, D3DPOOL pool/* =D3DPOOL_DEFAULT */)
{
	HRESULT hr;
	int nIndex = FindEmptyTextureIndex();
	BsAssert( nIndex != -1);
	m_pTextureList[nIndex] = new CBsTexture();
	hr = m_pTextureList[nIndex]->CreateTexture(m_pDevice, nWidth, nHeight, dwUsage, format, pool);
	if(hr != S_OK) {
		delete m_pTextureList[nIndex];
		m_pTextureList[nIndex] = NULL;
		nIndex = -1;
	}
	return nIndex;
}


int CBsKernel::FindEmptyTextureIndex()
{
	int i;

	for(i=0;i<MAX_TEXTURE_COUNT;i++){
		if(!(m_pTextureList[i])){
			return i;
		}
	}
	return -1;
}

int CBsKernel::FindTexture(const char *pFileName)
{
	int i;
	const char* pSrc=strrchr(pFileName, '\\');
	if(pSrc) {
		pSrc++;
	} else {
		pSrc = pFileName;
	}

	for(i=0;i<MAX_TEXTURE_COUNT;i++){
		if(m_pTextureList[i] && m_pTextureList[i]->GetTextureName()){
			if(_stricmp(m_pTextureList[i]->GetTextureName(), pSrc)==0){
				return i;
			}
		}
	}
	return -1;
}

void CBsKernel::DeleteTexture(int nIndex)
{
	if(m_pTextureList[nIndex]){
		delete m_pTextureList[nIndex];
		m_pTextureList[nIndex]=NULL;
	}
}

void CBsKernel::DeleteTexture(char *pFileName)
{
	int nIndex;
	nIndex=FindTexture(pFileName);
	if(nIndex!=-1){
		DeleteTexture(nIndex);
	}
}

#ifdef _XBOX
#ifdef _DEBUG
void CBsKernel::AddDeferredDeleteResource( D3DResource *pResource, VOID *pData, char *pName, DWORD dwSize )
#else
void CBsKernel::AddDeferredDeleteResource( D3DResource *pResource, VOID *pData, DWORD dwSize )
#endif
{
	THREAD_AUTOLOCK(&CBsTexture::s_csDeferredDeletedTexture);
	for(int i=0;i<MAX_TEXTURE_COUNT;i++) {
		if(m_DeferredDeleteList[i].pResource == NULL) {
			m_DeferredDeleteList[i].pResource = pResource;
			m_DeferredDeleteList[i].pData = pData;
			m_DeferredDeleteList[i].dwSize = dwSize;
#ifdef _DEBUG
			strcpy(m_DeferredDeleteList[i].szName, pName ? pName : "(null)");	// mruete: was crashing here, so changed to pName ? pName : ""
#endif
			m_nDeferredTextureCount++;
			return;
		}
	}
	BsAssert( 0 && "No free slots for deferred delete resource");
}


void CBsKernel::DeferredTextureDelete()
{
	THREAD_AUTOLOCK(&CBsTexture::s_csDeferredDeletedTexture);
	int nCount = 0;
	int nDeleteTextureCount = m_nDeferredTextureCount;
	for(int i=0;i<MAX_TEXTURE_COUNT && nCount < nDeleteTextureCount;i++) {
		D3DResource *pResource = m_DeferredDeleteList[i].pResource;
		if(m_DeferredDeleteList[i].pResource != NULL) {
			nCount++;

			unsigned int refCount = (pResource->Common & D3DCOMMON_REFCOUNT_MASK);
#if _XDK_VER < 2571
			unsigned int internalRefCount = (pResource->Common & D3DCOMMON_INTREFCOUNT_MASK) >> D3DCOMMON_INTREFCOUNT_SHIFT;
			if( (refCount == 1) && (internalRefCount == 0) && (pResource->IsBusy() == FALSE) )	{
#else
			if( (refCount == 1) && (pResource->IsBusy() == FALSE) ) {
#endif
				delete pResource;
				if(m_DeferredDeleteList[i].pData != NULL) {
#ifdef _DEBUG
					DebugString("Texture Resource Free : %s\n", m_DeferredDeleteList[i].szName );
#endif
					XPhysicalFree(m_DeferredDeleteList[i].pData);
					CBsTexture::TextureMmReleased(m_DeferredDeleteList[i].dwSize);
				}
				m_DeferredDeleteList[i].pResource = NULL;
				m_DeferredDeleteList[i].pData = NULL;
				m_nDeferredTextureCount--;
				BsAssert( m_nDeferredTextureCount >= 0 && "Invalid deferred texture count");
			}
		}
	}
}
#endif //#ifdef _XBOX

void CBsKernel::DeleteAllTexture()
{
	int i;

	for(i=0;i<MAX_TEXTURE_COUNT;i++){
		DeleteTexture(i);
	}
}

void CBsKernel::LostAllTexture()
{
	int i;

	for(i=0;i<MAX_TEXTURE_COUNT;i++){
		if(m_pTextureList[i]) {
			m_pTextureList[i]->LostResource();
		}
	}
}

SIZE CBsKernel::GetTextureSize(int nIndex)
{
	SIZE Size;
	if(m_pTextureList[nIndex]){
		Size=m_pTextureList[nIndex]->GetTextureSize(0);
	}
	else{
		Size.cx=0;
		Size.cy=0;
	}
	return Size;
}

void CBsKernel::ReloadAllTexture()
{
	for(int i=0;i<MAX_TEXTURE_COUNT;++i) {
		if(m_pTextureList[i]) {
			m_pTextureList[i]->Reload(m_pDevice);
		}
	}
}

void CBsKernel::AddTextureRef(int nIndex)
{
	if(m_pTextureList[nIndex]){
		m_pTextureList[nIndex]->AddRef();
	}
}

void CBsKernel::SetTexture(DWORD dwSampler, int nTextureIndex)
{
	BsAssert(m_pTextureList[nTextureIndex]);
	m_pDevice->SetTexture(dwSampler, m_pTextureList[nTextureIndex]->GetTexturePtr());
}

void CBsKernel::SetVertexDeclaration(int nVertexDeclIndex)
{
	m_pDevice->SetVertexDeclaration(GetBsVertexDeclaration(nVertexDeclIndex)->GetVertexDeclaration());
}

void CBsKernel::SetGammaRamp(float fGamma)
{
	KERNEL_COMMAND cmd;
	cmd.nCommand = KERNEL_COMMAND_SETGAMMA;
	cmd.fBuffer = fGamma;

	g_BsKernel.GetPreKernelCommand().push_back(cmd);
}


int CBsKernel::LoadSkin(int nSkinIndex, const char* pFileName)		// 파일명만 넘길 것
{
	if (!pFileName)
		return -1;

#ifndef _USAGE_TOOL_
	BsAssert( nSkinIndex == -1 );

	nSkinIndex = FindSkin(pFileName);
	if (nSkinIndex != -1) {
		m_pSkinList[nSkinIndex]->AddRef();
		return nSkinIndex;
	}

	nSkinIndex = FindEmptySkinIndex();
	BsAssert(nSkinIndex != -1 && "No empty skin slot!!");
#else
	if( nSkinIndex == -1 ) {
		nSkinIndex = FindEmptySkinIndex();
		BsAssert(nSkinIndex != -1 && "No empty skin slot!!");
	}
	else {
		if( m_pSkinList[nSkinIndex] != NULL ) {
			m_pSkinList[nSkinIndex]->AddRef();
			return nSkinIndex;
		}
	}
#endif

#ifdef _LOAD_MAP_CHECK_
	MEMORYSTATUS mem, mem2;
	GlobalMemoryStatus(&mem);
#endif

	DWORD dwFileSize;
	VOID *pData;
	const char* pFullName=m_pDirManager->GetFullName(pFileName);

#ifdef ENABLE_MEM_CHECKER
	char cStr[256];
	sprintf_s( cStr, _countof(cStr), "Load skin %s", pFileName );
	g_BsMemChecker.Start( cStr );
#endif

#ifdef ENABLE_RES_CHECKER
	g_BsResChecker.AddResList( (char*)pFullName );
#endif


	if( FAILED(CBsFileIO::LoadFile( pFullName, &pData, &dwFileSize ) ) ) {
		CBsConsole::GetInstance().AddFormatString("%s File Not Found", pFullName);
#ifdef ENABLE_MEM_CHECKER
		g_BsMemChecker.End();
#endif
		return -1;
	}
	BMemoryStream Stream(pData, dwFileSize);
	m_pSkinList[nSkinIndex] = new CBsSkin();
	BsAssert(m_pSkinList[nSkinIndex]);
	BackupCurDir((char *)pFullName);
	int nRet = m_pSkinList[nSkinIndex]->LoadSkin(&Stream);
	if( nRet == -1 ) {
		DebugString( "Load skin fail %s\n", pFileName );
		delete m_pSkinList[nSkinIndex];
		m_pSkinList[nSkinIndex] = NULL;
		CBsFileIO::FreeBuffer(pData);

#ifdef ENABLE_MEM_CHECKER
		g_BsMemChecker.End();
#endif
		return -1;
	}

	m_pSkinList[nSkinIndex]->SetSkinFileName(FnaPoolAdd(pFileName));
	RestoreCurDir();
	CBsFileIO::FreeBuffer(pData);

#ifdef ENABLE_MEM_CHECKER
	g_BsMemChecker.End();
#endif

#ifdef _LOAD_MAP_CHECK_
	GlobalMemoryStatus(&mem2);
	m_pSkinList[nSkinIndex]->m_nLoadSize = mem.dwAvailPhys - mem2.dwAvailPhys;
#endif

	return nSkinIndex;
}

void CBsKernel::AddSkinRef(int nSkinIndex)
{
	// Only AddRef() call and Valid input Index return
	if(nSkinIndex < 0) {
		DebugString("CBsKernel::GetSkin() : Invalid Skin Index!\n");
		return;
	}

	if(!m_pSkinList[nSkinIndex]) {
		DebugString("CBsKernel::GetSkin() : Invalid Skin Pointer!\n");
		return;
	}

	m_pSkinList[nSkinIndex]->AddRef();
}

int CBsKernel::FindSkin(const char* pSkinFile)
{
	const char* pFileName=strrchr(pSkinFile, '\\');
	if(pFileName) {
		pFileName++;
	} else {
		pFileName = pSkinFile;
	}

	for (int i=0; i<MAX_SKIN_COUNT; ++i){
		if (m_pSkinList[i]){
			if(_strcmpi(m_pSkinList[i]->GetSkinFileName(), pFileName)==0)
				return i;
		}
	}

	return -1;
}

int	CBsKernel::FindEmptySkinIndex()
{
	for (int i=0; i<MAX_SKIN_COUNT; ++i) {
		if (!m_pSkinList[i])
			return i;
	}
	return -1;
}

void CBsKernel::ReleaseSkin(int nSkinIndex)
{
	if ( nSkinIndex < 0 )
		return;

	BsAssert( m_pSkinList[ nSkinIndex ] );
	int nRefCount = m_pSkinList[ nSkinIndex ]->Release();
}

void CBsKernel::FlushSkins(void)
{
	BsAssert( !IsRenderThread() );

	for ( int nSkinIndex = 0; nSkinIndex < MAX_SKIN_COUNT; ++nSkinIndex ) {
		if ( (m_pSkinList[ nSkinIndex ] != NULL) && (m_pSkinList[ nSkinIndex ]->GetRefCount() <= 0) ) {
			BsAssert( m_pSkinList[ nSkinIndex ]->GetRefCount() == 0 );
#ifndef _LTCG
			char const * skinName = m_pSkinList[nSkinIndex]->GetSkinFileName();
			DebugString("FlushSkins: deleting skin : %s (%d)\n", skinName ? skinName : "<unknown>", nSkinIndex);
#endif
			delete m_pSkinList[ nSkinIndex ];
			m_pSkinList[ nSkinIndex ] = NULL;
		}
	}
}

void CBsKernel::DeleteSkin(int nIndex)
{
	if (m_pSkinList[nIndex]) {
		delete m_pSkinList[nIndex];		
		m_pSkinList[nIndex] = NULL;
	}
}

void CBsKernel::DeleteSkin(char* pFilename)
{
	int nIndex;
	nIndex=FindSkin(pFilename);
	if(nIndex!=-1){
		DeleteSkin(nIndex);
	}
}

void CBsKernel::DeleteAllSkin()
{
	int i;

	for(i=0;i<MAX_SKIN_COUNT;i++){
		DeleteSkin(i);
	}
}

void CBsKernel::InitializeFileLoader(const char *pResourceDir)
{
	m_pDirManager->Initialize(pResourceDir);
}

void CBsKernel::SetCurrentDirectory(const char *pPathName)
{
	m_pDirManager->SetCurrentDirectory(pPathName);
}

DWORD CBsKernel::GetCurrentDirectory(DWORD nBufferLength, char *pBuffer)
{
	char *pDir;
	DWORD dwCopyLength, dwPathLength;

	pDir=m_pDirManager->GetCurrentDirectory();
	dwPathLength=strlen(pDir);
	if(dwPathLength>nBufferLength-1){
		dwCopyLength=nBufferLength-1;
	}
	else{
		dwCopyLength=dwPathLength;
	}
	memcpy(pBuffer, pDir, dwCopyLength);
	pBuffer[dwCopyLength]=0;

	return dwCopyLength;
}

const char *CBsKernel::GetCurrentDirectory()
{
	return m_pDirManager->GetCurrentDirectory();
}

int CBsKernel::chdir(const char *pDirectoryName)
{
	return m_pDirManager->ChangeDir(pDirectoryName);
}

char* CBsKernel::GetFullName(const char *pFileName)
{
	return m_pDirManager->GetFullName(pFileName);
}

void CBsKernel::SetShaderDirectory(const char *pPathName)
{ 
	int nStrLen;

	strcpy(m_szShaderDir, pPathName); 
	nStrLen=strlen(m_szShaderDir);
	if(m_szShaderDir[nStrLen-1]!='\\'){
		strcat(m_szShaderDir, "\\");
	}
}

static bool bBackup=false;
static char szBackDir[_MAX_PATH];
void CBsKernel::BackupCurDir(char *pFileName)
{
	char *pFindPtr;
	if((pFileName[1]!=':')||((pFileName[0]!='\\')&&(pFileName[1]!='\\'))){
		bBackup=true;
		strcpy(szBackDir, m_pDirManager->GetCurrentDirectory());
		pFindPtr=strrchr(pFileName, '\\');
		if(pFindPtr){
			*pFindPtr=0;
			m_pDirManager->SetCurrentDirectory(pFileName);
			*pFindPtr='\\';
		}
		else{
			BsAssert(0);
		}
	}
}

void CBsKernel::RestoreCurDir()
{
	if(bBackup){
		bBackup=false;
		m_pDirManager->SetCurrentDirectory(szBackDir);
	}
}

void CBsKernel::ShowFPS(bool bShow, int nX/*=450*/, int nY/*=50*/)
{
	m_bShowFPS=bShow;
	m_nFPSX=nX;
	m_nFPSY=nY;
}

void CBsKernel::PrintString(int nX, int nY, const char *pString, DWORD dwColor/*=0xffffffff*/)
{
	int nLength;

	if(MAX_STRING_COUNT <= m_nStringCount)	return;

	nLength=strlen(pString)+1;
	m_nStringX[m_nStringCount]=nX;
	m_nStringY[m_nStringCount]=nY;
	m_dwStringColor[m_nStringCount]=dwColor;
	if((!m_pStringBuffer[m_nStringCount])||(m_nStringBufferLength[m_nStringCount]<nLength)){
		if(m_pStringBuffer[m_nStringCount]){
			delete [] m_pStringBuffer[m_nStringCount];
		}
		m_pStringBuffer[m_nStringCount]=new char[nLength];
		m_nStringBufferLength[m_nStringCount]=nLength;
	}
	strcpy_s(m_pStringBuffer[m_nStringCount], nLength, pString); //aleksger - safe string
	m_nStringCount++;
}

const char *CBsKernel::GetTextureName(int nIndex)
{
	if(m_pTextureList[nIndex]){
		return m_pTextureList[nIndex]->GetTextureName();
	}
	else{
		return NULL;
	}
}

void CBsKernel::ReleaseTexture(int nIndex)
{
	if(m_pTextureList[nIndex]){
		if(m_pTextureList[nIndex]->Release()==0){
			m_pTextureList[nIndex]=NULL;
		}		
	}
}

// CreateXXXXObjectFromSkin()에서 공용으로 Skin처리 하는 루틴입니다.
void CBsKernel::LinkSkinToObject(int nObjectIndex, int nSkinIndex )
{
	int nMeshIndex = m_pSkinList[nSkinIndex]->GetMeshIndex();
	CBsMesh* pMesh = m_pMeshList[nMeshIndex];
	BsAssert(pMesh);
	int nSubMeshCount = pMesh->GetSubMeshCount();
	{	// Attach Material!!
		if( nSubMeshCount != m_pSkinList[nSkinIndex]->GetSubMeshCount()) {
			DebugString("Invalid Skin Data : %s\n", pMesh->GetMeshFileName());
			BsAssert( 0 && "Submesh count is Not equal to Skin's submesh count" );
		}
		for( int i=0;i<m_pSkinList[nSkinIndex]->GetSubMeshCount(); ++i) {
			SBsSubMesh_Info* pInfo = m_pSkinList[nSkinIndex]->GetSubMeshInfo(i);
			BsAssert(pInfo);

			m_ppObjectPool[nObjectIndex]->AttachMaterial(i, pInfo->m_nFxIndex);

			int j;
			for ( j=0; j<pInfo->m_nSamplerCount; ++j) {				
				int nSamplerIndex = pInfo->m_ppSamplers[j]->m_nSamplerIndex;
				if( nSamplerIndex != -1) {
					SendMessage(nObjectIndex, BS_SET_SAMPLER, i, j, nSamplerIndex);
				}
				else {
					SendMessage(nObjectIndex, BS_SET_SAMPLER, i, j, m_nBlankTexture);	// 없는경우 blank texture 로 대체한다..
				}
			}
			int nSize = pInfo->m_CustomParameters.size();
			for( j=0 ; j<nSize; ++j) {
				SendMessage(nObjectIndex, BS_SET_EDITABLE_PARAMETER, i, pInfo->m_CustomParameters[j]->m_nCustomParamIndex, (DWORD)(pInfo->m_CustomParameters[j]->m_pValue));
			}
			SendMessage(nObjectIndex, BS_SET_SUBMESH_ALPHABLENDENABLE,	i, pInfo->m_bUseAlphaBlend);
			SendMessage(nObjectIndex, BS_SET_SUBMESH_ALPHA,				i, (DWORD)&pInfo->m_fAlpha);
			SendMessage(nObjectIndex, BS_SET_SUBMESH_TWOSIDEENABLE,		i, pInfo->m_bUseTwoSideRender);
			SendMessage(nObjectIndex, BS_SET_SUBMESH_ALPHA_ALIGN_ENABLE,i, pInfo->m_bUseAlphaAlignRender);
		}
	}
	m_ppObjectPool[nObjectIndex]->SetSkinIndex( nSkinIndex );
}

int CBsKernel::SearchEmptyObjectIndex( bool bUseFirstBlock/*= false*/ )
{
	// First Type Object 관리 필요!! by jeremy
	int i, nStartIndex=FIRST_DRAW_BLOCK_COUNT;
	int nIndex=-1;

	if(bUseFirstBlock){
		nStartIndex=0;
	}
	for(i=nStartIndex;i<m_nPoolSize;i++){
		if(m_ppObjectPool[i]==NULL){
			nIndex=i;
			break;
		}
	}
	if(nIndex == -1) {
		// I removed the reallocation from here as it is not thread safe and
		// will likely cause a crash... If this assert fires we will need to
		// increase DEFAULT_BS_OBJECT_COUNT - aflavell@microsoft.com
		BsAssert(0 && "Object Pool Overflow - Increase DEFAULT_BS_OBJECT_COUNT");
	}
	return nIndex;
}

void CBsKernel::GetParticleCount(int& nCurParticleCount, int& nMaximumParticleCount)
{
	nCurParticleCount=m_ParticlePool.Count();
	nMaximumParticleCount=m_ParticlePool.Size();
}

CBsParticleGroup *CBsKernel::GetParticleGroup(int nIndex)
{
	if( m_ParticleGroupList[nIndex] )
		return m_ParticleGroupList[nIndex];

	return NULL;
}

CBsParticleObject *CBsKernel::GetParticleObject( int nHandle )
{
	int nIndex;

	nIndex=nHandle&0xffff;
	if(nHandle!=m_ParticlePool.GetHandle(nIndex)){
		return NULL;
	}

	return m_ParticlePool[nIndex];
}

void CBsKernel::AddParticleGroupRef(int nIndex)
{
	if( nIndex < 0 ) {
		DebugString("CBsKernel::AddParticleGroupRef() : Invalid ParticleGroup Index!\n");
		return;
	}

	// If this assert fires then the particle group is not valid (possibly already freed?)
	BsAssert( m_ParticleGroupList[nIndex] );

	if( m_ParticleGroupList[nIndex] == NULL ) {
		DebugString("CBsKernel::AddParticleGroupRef() : Invalid ParticleGroup Index!\n");
		return;
	}

	m_ParticleGroupList[nIndex]->AddRef();
}

int	CBsKernel::FindParticleGroup(const char* pFileName)
{
	int i;
	const char *pSource;

	pSource=strrchr(pFileName, '\\');
	if(pSource){
		pSource++;
	}
	else{
		pSource=pFileName;
	}
	for(i=0;i<MAX_PARTICLE_GROUP_COUNT;i++){
		if(m_ParticleGroupList[i]){
			if(strcmp(pSource, m_ParticleGroupList[i]->GetParticleFileName())==0){
				return i;
			}
		}
	}

	return -1;
}

int	CBsKernel::LoadParticleGroup(int nIndex, const char *pFileName)
{
	int nRet = 0;
	char *pFullName;

	if(nIndex==-1) {
		nIndex=FindParticleGroup(pFileName);
		if(nIndex!=-1){
			m_ParticleGroupList[nIndex]->AddRef();
			return nIndex;
		}

		nIndex=FindEmptyParticleGroupIndex();
	}

	pFullName=m_pDirManager->GetFullName(pFileName);
	if(m_ParticleGroupList[nIndex]){
		if(_strcmpi(pFileName, m_ParticleGroupList[nIndex]->GetParticleFileName())==0){
			m_ParticleGroupList[nIndex]->AddRef();
			return nIndex;
		}
		else {
			BsAssert( false && "Particle group index already used by another group" );
			return -1;
		}
	}

	DWORD dwFileSize;
	VOID *pData;


#ifdef ENABLE_MEM_CHECKER
	char cStr[256];
	sprintf_s( cStr, _countof(cStr), "Load particle : %s", pFileName );
	g_BsMemChecker.Start( cStr );
#endif

#ifdef ENABLE_RES_CHECKER
	g_BsResChecker.AddResList( pFullName );
#endif

	if( FAILED(CBsFileIO::LoadFile( pFullName, &pData, &dwFileSize ) ) ) {
		CBsConsole::GetInstance().AddFormatString("%s File Not Found", pFullName);

#ifdef ENABLE_MEM_CHECKER
		g_BsMemChecker.End();
#endif

		return -1;
	}

	BMemoryStream Stream(pData, dwFileSize);
	m_ParticleGroupList[nIndex]=new CBsParticleGroup();
	m_ParticleGroupList[nIndex]->SetParticleFileName(pFullName);

	BackupCurDir((char *)pFullName);

	m_ParticleGroupList[nIndex]->LoadParticleGroup(&Stream);

	RestoreCurDir();

	CBsFileIO::FreeBuffer(pData);

#ifdef ENABLE_MEM_CHECKER
	g_BsMemChecker.End();
#endif

	return nIndex;
}


void CBsKernel::UseParticleAlphaFog( int nHandle, bool bFlag )
{
	int nIndex;

	nIndex=nHandle&0xffff;
	if( nIndex < 0 ) {
		return;
	}

	if( m_ParticlePool[nIndex]->GetUse() ) {
		BsAssert( m_ParticlePool.GetHandle(nIndex) == nHandle );
		m_ParticlePool[nIndex]->SetUseAlphaFog( bFlag );
	}
}

bool CBsKernel::IsParticleAlphaFog( int nHandle )
{
	int nIndex;
	nIndex=nHandle&0xffff;
	if( nIndex < 0 ) {
		return false;
	}

	if( m_ParticlePool[nIndex]->GetUse() ) {
		BsAssert( m_ParticlePool.GetHandle(nIndex) == nHandle );
		return m_ParticlePool[nIndex]->IsUsingAlphaFog();
	}
	return false;
}

void CBsKernel::ReLoadAllParticleGroup()
{
	int i;

	CBsParticleGroup::DeleteEffect();
	CBsParticleGroup::CreateEffect();
	for(i=0;i<MAX_PARTICLE_GROUP_COUNT;i++){
		if(m_ParticleGroupList[i]){
			m_ParticleGroupList[i]->RecreateParticleBuffer();
		}
	}
}

int CBsKernel::ReLoadParticleGroup(int nIndex)
{
	char szParticleName[_MAX_PATH];

	BsAssert( nIndex < MAX_PARTICLE_GROUP_COUNT );

	if(!m_ParticleGroupList[nIndex]){
		return -1;
	}

	strcpy(szParticleName, m_ParticleGroupList[nIndex]->GetParticleFileFullName());

	DWORD dwFileSize;
	VOID *pData;

	if( FAILED(CBsFileIO::LoadFile( szParticleName, &pData, &dwFileSize ) ) ) {
		CBsConsole::GetInstance().AddFormatString("%s File Not Found", szParticleName);
		return -1;
	}

	BMemoryStream Stream(pData, dwFileSize);
	m_ParticleGroupList[nIndex]->LoadParticleGroup(&Stream);

	CBsFileIO::FreeBuffer(pData);

	return nIndex;
}

int CBsKernel::FindEmptyParticleGroupIndex()
{
	int i;

	for(i=START_ARBITARY_PARTICLE_INDEX;i<MAX_PARTICLE_GROUP_COUNT;i++){
		if(!(m_ParticleGroupList[i])){
			return i;
		}
	}
	BsAssert(0 && "No find particle group slot!");

	return -1;
}

void CBsKernel::ReleaseParticleGroup(int nIndex)
{
	if ( nIndex < 0 )
		return;

	BsAssert( m_ParticleGroupList[ nIndex ] );
	int nRefCount = m_ParticleGroupList[ nIndex ]->Release();
}

// This is a debugging function to facility catching errors with the ref counting system.  It is there to make sure that
// we do not release a particle group that is currently in use by a particle.
bool IsParticleGroupInUse( CObjectPoolMng<CBsParticleObject> & particles, CBsParticleGroup const * group )
{
	int nSize = particles.Size();
	for( int i = 0; i < nSize; ++i ) {
		if ( particles[i]->GetUse() ) {
			if ( particles[i]->GetParticleGroupPtr() == group )
				return true;
		}
	}

	return false;
}

void CBsKernel::FlushParticleGroups(void)
{
	BsAssert( !IsRenderThread() );

	// The following code is there to make sure that there are no particles active when we are flushing the particle groups.

#ifndef _LTCG
	int numParticlesInUse = 0;
	int numParticleGroupsInUse = 0;

	int nSize = m_ParticlePool.Size();
	for( int i = 0; i < nSize; ++i ) {
		if ( m_ParticlePool[i]->GetUse() ) {

			++numParticlesInUse;

			CBsParticleGroup * group = m_ParticlePool[i]->GetParticleGroupPtr();
			if ( group ) {
				++numParticleGroupsInUse;

				const char * particleName = group->GetParticleFileName();
				DebugString( "Particle in use by: %s\n", particleName ? particleName : "<unknonw>");
			}
		}
	}

	if ( numParticlesInUse || numParticleGroupsInUse )
		DebugString ( "Num particles: %d\tNum particle groups: %d\n", numParticlesInUse, numParticleGroupsInUse );

	//BsAssert( numParticleGroupsInUse == 0 );

#endif

	unsigned int nNumParticleGroups = m_ParticleGroupList.size();
	for ( unsigned int nIndex = 0; nIndex < nNumParticleGroups; ++nIndex ) {
		if ( (m_ParticleGroupList[ nIndex ] != NULL) && (m_ParticleGroupList[ nIndex ]->GetRefCount() <= 0) ) {
			BsAssert( m_ParticleGroupList[ nIndex ]->GetRefCount() == 0 );
#ifndef _LTCG
			bool inUse = IsParticleGroupInUse( m_ParticlePool, m_ParticleGroupList[ nIndex ] );
			const char * particleName = m_ParticleGroupList[nIndex]->GetParticleFileName();
			DebugString("FlushParticleGroups: deleting particle group : %s (%d)%s\n", particleName ? particleName : "<unknown>", nIndex, inUse ? " [WARNING: GROUP STILL IN USE]" : "");
#endif
			delete m_ParticleGroupList[ nIndex ];
			m_ParticleGroupList[ nIndex ] = 0;
		}
	}
	m_IteratePos.clear();
}

void CBsKernel::DeleteParticleGroup(int nIndex)
{
	if(m_ParticleGroupList[nIndex]){
		delete m_ParticleGroupList[nIndex];
		m_ParticleGroupList[nIndex]=NULL;
	}
}

void CBsKernel::DeleteAllParticleGroup()
{
	int i;

	for(i=0;i<MAX_PARTICLE_GROUP_COUNT;i++){
		DeleteParticleGroup(i);
	}
}

//// 파티클 Create 함수 너무 많다.. 필요한거만 남기고 정리하자..
int	CBsKernel::CreateParticleObject(int nParticleGroupIndex, bool bLoop, bool bIterate, D3DXMATRIX *pParticleMat, float fScale/*=1.f*/, D3DCOLORVALUE *pColor/*=NULL*/, bool bAutoDelete /*= true*/)
{
	int nIndex, nHandle;

	if( nParticleGroupIndex == -1 || nParticleGroupIndex >= (int)m_ParticleGroupList.size() ) {
		DebugString( "Particle Group Index Out of Range!!!\n" );
		return 0;
	}
	BsAssert( nParticleGroupIndex != -1);

	if(!m_ParticleGroupList[nParticleGroupIndex]){
		return -1;
	}

	if(m_ParticlePool.Count()>=MAX_PARTICLE_OBJECT_COUNT){
		return -1;
	}

	{
		THREAD_AUTOLOCK( &CBsParticleObject::s_csSharedParticleData );
		nHandle = m_ParticlePool.CreateObject();
	}

	nIndex=nHandle&0xffff;

	m_ParticlePool[nIndex]->SetLoop(bLoop);
	m_ParticlePool[nIndex]->SetParticleGroupPtr(m_ParticleGroupList[nParticleGroupIndex]);

	m_ParticlePool[nIndex]->SetIterate(bIterate);
	if(pParticleMat){
		D3DXVECTOR3 NormalVec(0.0f, 1.0f, 0.0f);

		m_ParticlePool[nIndex]->SetParticleMatrix(pParticleMat);
		D3DXVec3TransformNormal(&NormalVec, &NormalVec, pParticleMat);
		m_ParticlePool[nIndex]->SetParticleScale(D3DXVec3Length(&NormalVec));
	}
	if(pColor){
		m_ParticlePool[nIndex]->SetParticleColor(pColor);
	}
	m_ParticlePool[nIndex]->SetParticleScale(fScale);
	m_ParticlePool[nIndex]->SetAutoDelete( bAutoDelete );

	// Thread 간섭 줄이기 위해 Setting을 마지막에 합니다. by mapping
	m_ParticlePool[nIndex]->SetUse(true);	

	return nHandle;
}

bool CBsKernel::IsPlayParticleObject(int nHandle)
{
	int nIndex;
	nIndex=nHandle&0xffff;
	if(nHandle!=m_ParticlePool.GetHandle(nIndex)){
		return false;
	}
	return m_ParticlePool[nIndex]->GetUse();
}

int	CBsKernel::AllocIteratePosition( int nCount )
{
	int nIterateCount;

	nIterateCount = ( int )m_IteratePos.size();

	BsAssert( nIterateCount + nCount <= MAX_ITERATE_POSTION_COUNT );

	if( nIterateCount + nCount > MAX_ITERATE_POSTION_COUNT ) {
		return -1;
	}

	m_IteratePos.resize( nIterateCount + nCount );

	return nIterateCount;
}

void CBsKernel::DeleteIteratePosition(int nIndex, int nCount)
{
	int i, nSize;

	if (nIndex < 0 )
		return;

	if( nCount < 1 )
		return;

	nSize=m_ParticlePool.Size();
	if(nIndex>=0) {
		for(i=0;i<nSize;i++) {
			if(m_ParticlePool[i]->GetUse()) {
				m_ParticlePool[i]->AdjustSaveIteratePosition(nIndex, nCount);
			}
		}
		m_IteratePos.erase( m_IteratePos.begin() + nIndex, m_IteratePos.begin() + nIndex + nCount );
	}
}

void CBsKernel::StopParticlePlay(int nHandle, int nStopTick/*=-1*/)
{
	int nIndex;

	if( nHandle == -1 ) {
		return;
	}
	nIndex=nHandle&0xffff;

	if(m_ParticlePool[nIndex]->GetUse()){
		BsAssert( m_ParticlePool.GetHandle(nIndex) == nHandle );
		m_ParticlePool[nIndex]->StopPlay(nStopTick);
	}
}

void CBsKernel::ShowParticle(int nHandle, bool bShow)
{
	int nIndex;

	nIndex=nHandle&0xffff;

	if( m_ParticlePool[nIndex]->GetUse() ) {
		// TODO: 임시로 끕니다. 반드시 다시 가동되야 되는 Assert입니다. by jeremy
		//BsAssert( m_ParticlePool.GetHandle(nIndex) == nHandle );
		if(m_ParticlePool.GetHandle(nIndex)!=nHandle) {
			DebugString("!!! ShowParticle Index Not Valid : %d(%d) \n", nIndex, nHandle);
			_DEBUGBREAK;
		}
		m_ParticlePool[nIndex]->ShowParticle( bShow );
	}
}

bool CBsKernel::IsShowParticle(int nHandle)
{
	int nIndex;
	nIndex=nHandle&0xffff;

	if( m_ParticlePool[nIndex]->GetUse() ) {
		BsAssert( m_ParticlePool.GetHandle(nIndex) == nHandle );
		return m_ParticlePool[nIndex]->IsShowParticle();
	}
	return false;
}

void CBsKernel::PauseParticle(int nHandle, bool bPause)
{
	int nIndex;

	nIndex=nHandle&0xffff;

	if( m_ParticlePool[nIndex]->GetUse() ) {
		BsAssert( m_ParticlePool.GetHandle(nIndex) == nHandle );
		m_ParticlePool[nIndex]->PauseParticle( bPause );
	}
}

bool CBsKernel::IsPauseParticle(int nHandle)
{
	int nIndex;

	nIndex=nHandle&0xffff;

	if( m_ParticlePool[nIndex]->GetUse() ) {
		BsAssert( m_ParticlePool.GetHandle(nIndex) == nHandle );
		return m_ParticlePool[nIndex]->IsPauseParticle();
	}
	return false;
}

void CBsKernel::DeleteParticleObject( int nHandle )
{
	int nIndex;

	nIndex = nHandle & 0xffff;
	if( nIndex < 0) {
		return;
	}
	if( nHandle < 0 ) {
		return;
	}
	if( m_ParticlePool[ nIndex ]->GetUse() ) {
		if( m_ParticlePool.GetHandle(nIndex) != nHandle ) {
			// TODO : 지워야 할 내용 입니다. 
			DebugString("Invalid Particle Object Handle : %d(%d)\n", nHandle, nIndex);
		}
		BsAssert( m_ParticlePool.GetHandle(nIndex) == nHandle );
		m_ParticlePool[ nIndex ]->Delete();
	}
}

void CBsKernel::DeleteAllParticleObject()
{
	int i, nSize, nHandle;

	nSize = m_ParticlePool.Size();
	for( i = 0; i < nSize; i++ ) {
		nHandle = m_ParticlePool.GetHandle( i );
		if( ( nHandle & 0xffff ) == i )	{
			DeleteParticleObject( nHandle );
		}
	}
}

void CBsKernel::UpdateParticle(int nHandle, D3DXMATRIX *pMatParticle, bool bChangeScale/*=false*/, D3DCOLORVALUE *pColor/*=NULL*/)
{
	int nIndex;
	float fScale;
	D3DXMATRIX MatScale;
	D3DXVECTOR3 NormalVec(0.0f, 1.0f, 0.0f);

	nIndex=nHandle&0xffff;
	if(nIndex<0){
		return;
	}

	if( nIndex >= m_ParticlePool.Size() || m_ParticlePool[nIndex] == NULL )	{
		// TODO : 임시로 잘못된 Data 들어오는것 검사합니다. 버그 수정후 복구 by jeremy
		//		BsAssert( 0 && "CBsKernel::UpdateParticle() : Invalid Particle Index!" );
		DebugString("!!! UpdateParticle Index Not Valid : %d(%d) \n", nIndex, nHandle);
		_DEBUGBREAK;
	}
	else if((m_ParticlePool[nIndex]->GetUse())&&(pMatParticle)) {
		if( m_ParticlePool.GetHandle(nIndex) != nHandle ) {
			DebugString( "each Differ value %d, %d\n", m_ParticlePool.GetHandle(nIndex) , nHandle);
			_DEBUGBREAK;
		}
		BsAssert( m_ParticlePool.GetHandle(nIndex) == nHandle );
		if(bChangeScale){
			m_ParticlePool[nIndex]->SetParticleMatrix(pMatParticle);
			D3DXVec3TransformNormal(&NormalVec, &NormalVec, pMatParticle);
			m_ParticlePool[nIndex]->SetParticleScale(D3DXVec3Length(&NormalVec));
		}
		else{
			fScale=m_ParticlePool[nIndex]->GetParticleScale();
			if(fScale == 1.0f) {
				m_ParticlePool[nIndex]->SetParticleMatrix(pMatParticle);
			}
			else{
				D3DXMatrixScaling(&MatScale, fScale, fScale, fScale);
				D3DXMatrixMultiply(&MatScale, pMatParticle, &MatScale);
				m_ParticlePool[nIndex]->SetParticleMatrix(&MatScale);
			}
		}
		if(pColor){
			m_ParticlePool[nIndex]->SetParticleColor(pColor);
		}
	}
}

void CBsKernel::UpdateParticle(int nHandle, const D3DXVECTOR3& pos)
{
	int nIndex;

	nIndex=nHandle&0xffff;
	if(nIndex<0){
		return;
	}

	if(m_ParticlePool[nIndex]->GetUse()) {
		// TODO : 임시로 잘못된 Data 들어오는것 검사합니다. 버그 수정후 if문 제거 by jeremy
		//		BsAssert( m_ParticlePool.GetHandle(nIndex) == nHandle );
		if(m_ParticlePool.GetHandle(nIndex)!=nHandle) {
			DebugString("!!! UpdateParticle Index Not Valid : %d(%d) \n", nIndex, nHandle);
			_DEBUGBREAK;
		}

		D3DXMATRIX matrix;
		D3DXMatrixTranslation( &matrix, pos.x, pos.y, pos.z );

		float scale=m_ParticlePool[nIndex]->GetParticleScale();
		matrix._11 = matrix._22 = matrix._33 = scale;

		m_ParticlePool[nIndex]->SetParticleMatrix(&matrix);
	}
}

void CBsKernel::SetParticleTick( int nHandle, int nTick )
{
	int nIndex;

	nIndex=nHandle&0xffff;
	if(nIndex<0){
		return;
	}

	if(m_ParticlePool[nIndex]->GetUse()){
		BsAssert( m_ParticlePool.GetHandle(nIndex) == nHandle );
		m_ParticlePool[nIndex]->SetParticleTick(nTick);
	}
}

int	CBsKernel::GetParticleTick( int nHandle )
{
	int nIndex;

	nIndex=nHandle&0xffff;
	if( nIndex < 0 ) {
		return -1;
	}

	if(m_ParticlePool[nIndex]->GetUse()) {
		BsAssert( m_ParticlePool.GetHandle(nIndex) == nHandle );
		return m_ParticlePool[nIndex]->GetParticleTick();
	}
	return -1;
}

bool CBsKernel::IsParticleCull( int nHandle )
{
	int nIndex;
	nIndex=nHandle&0xffff;
	if( nIndex < 0 ) {
		return false;
	}

	if( m_ParticlePool[nIndex]->GetUse() ) {
		BsAssert( m_ParticlePool.GetHandle(nIndex) == nHandle );
		return m_ParticlePool[nIndex]->UseCull();
	}
	return false;
}

void CBsKernel::SetParticleCull( int nHandle, bool bUse )
{
	int nIndex;

	nIndex=nHandle&0xffff;
	if( nIndex < 0 ) {
		return;
	}

	if( m_ParticlePool[nIndex]->GetUse() ) {
		BsAssert( m_ParticlePool.GetHandle(nIndex) == nHandle );
		m_ParticlePool[nIndex]->SetUseCull( bUse );
	}
}


void CBsKernel::SetSceneIntensity(float fSceneIntensity)
{
	if( m_pImageProcess )
		m_pImageProcess->SetSceneIntensity(fSceneIntensity);
}

float CBsKernel::GetSceneIntensity()
{
	if( m_pImageProcess )
		return m_pImageProcess->GetSceneIntensity();
	return 0.f;
}

void CBsKernel::SetBlurIntensity(float fBlur)
{
	if( m_pImageProcess )
		m_pImageProcess->SetBlurIntensity(fBlur);
}

float CBsKernel::GetBlurIntensity()
{
	if( m_pImageProcess )
		return m_pImageProcess->GetBlurIntensity();
	return 0.f;
}

void CBsKernel::SetGlowIntensity(float fGlow)
{
	if( m_pImageProcess )
		m_pImageProcess->SetGlowIntensity(fGlow);
}

float CBsKernel::GetGlowIntensity()
{
	if( m_pImageProcess )
		return m_pImageProcess->GetGlowIntensity();
	return 0.f;
}

void CBsKernel::SetHighlightThreshold(float fValue)
{
	if( m_pImageProcess )
		m_pImageProcess->SetHighlightThreshold(fValue);
}

float CBsKernel::GetHighlightThreshold()
{
	if( m_pImageProcess )
		return m_pImageProcess->GetHighlightThreshold();
	return 0.f;
}

void CBsKernel::SetSceneAddColor(float r /*= 0.0f*/, float g /*= 0.0f*/, float b /*= 0.0f*/, float a /*= 0.0f*/ )
{
	if( m_pImageProcess )
		m_pImageProcess->SetAddColor(r,g,b,a);
}

D3DXVECTOR4	CBsKernel::GetSceneAddColor()
{
	if( m_pImageProcess )
		return m_pImageProcess->GetAddColor();
	return D3DXVECTOR4(0,0,0,0);
}

void CBsKernel::SetDOFFocus(float fFocus)
{
	if( m_pImageProcess )
		m_pImageProcess->SetDOFFocus(fFocus);
}

float CBsKernel::GetDOFFocus()
{
	if( m_pImageProcess )
		return m_pImageProcess->GetDOFFocus();
	return 0.f;
}

void CBsKernel::SetDOFFocusRange(float fFocusRange)
{
	if( m_pImageProcess ) {
		m_pImageProcess->SetDOFFocusRangeNear(fFocusRange); 
		m_pImageProcess->SetDOFFocusRangeFar(fFocusRange);
	}
}

BOOL CBsKernel::IsEnabledDOF()
{
	if( m_pImageProcess )
		return m_pImageProcess->IsEnabledDOF();
	return FALSE;
}

void CBsKernel::EnableDOF(int nOn)
{
	if( m_pImageProcess )
		m_pImageProcess->EnableDOF(nOn);
}

void CBsKernel::SetDOFFocusNear(float fNear)
{ 
	if( m_pImageProcess )
		m_pImageProcess->SetDOFFocusRangeNear(fNear); 
}

void CBsKernel::SetDOFFocusFar(float fFar)
{ 
	if( m_pImageProcess )
		m_pImageProcess->SetDOFFocusRangeFar(fFar); 
}

float CBsKernel::GetDOFFocusNear()
{
	if( m_pImageProcess )
		return m_pImageProcess->GetDOFFocusRangeNear();
	return 0.f;
}

float CBsKernel::GetDOFFocusFar()
{
	if( m_pImageProcess )
		return m_pImageProcess->GetDOFFocusRangeFar();
	return 0.f;
}

void CBsKernel::SetDOFFocusOutNear(float fNear)
{ 
	if( m_pImageProcess )
		m_pImageProcess->SetDOFFocusOutNear(fNear); 
}

void CBsKernel::SetDOFFocusOutFar(float fFar)
{ 
	if( m_pImageProcess )
		m_pImageProcess->SetDOFFocusOutFar(fFar); 
}

float CBsKernel::GetDOFFocusOutNear()
{
	if( m_pImageProcess )
		return m_pImageProcess->GetDOFFocusOutNear();
	return 0.f;
}

float CBsKernel::GetDOFFocusOutFar()
{
	if( m_pImageProcess )
		return m_pImageProcess->GetDOFFocusOutFar();
	return 0.f;
}

void CBsKernel::SetDOFFocusOutNearDef()
{ 
	if( m_pImageProcess )
		m_pImageProcess->SetDOFFocusOutNearDef(); 
}

void CBsKernel::SetDOFFocusOutFarDef()
{ 
	if( m_pImageProcess )
		m_pImageProcess->SetDOFFocusOutFarDef(); 
}

int CBsKernel::GetWorldLightMap()
{
	return m_pWorld ? ((CBsLODWorld*)m_pWorld)->GetWorldLightMap() : -1;
}

void CBsKernel::LoadWorldLightMap(const char* szLightMapFileName)
{
	BsAssert( m_pWorld && "No World error!!");
	((CBsLODWorld*)m_pWorld)->LoadWorldLightMap(szLightMapFileName);
}

float CBsKernel::GetRcpWorldXSize()
{
	return ((CBsLODWorld*)m_pWorld)->GetRcpWorldXSize();
}

float CBsKernel::GetRcpWorldZSize()
{
	return ((CBsLODWorld*)m_pWorld)->GetRcpWorldZSize();
}

LPDIRECT3DTEXTURE9	CBsKernel::GetReflectMap()
{
	return m_pWater ? m_pWater->GetReflectMap() : NULL;
}

void CBsKernel::EnableBillboard(bool bFlag)
{
}

void CBsKernel::EnableInstancing(bool bFlag) 
{
	m_pInstancingMgr->SetEnable( bFlag );
}

void CBsKernel::SetInstancingBufferSize( int nSize )
{
	m_pInstancingMgr->SetBufferSize( nSize );
}

void CBsKernel::SetBillboardRange(float fRange)
{
	m_pBillboardMgr->SetCurrentBillboardRange(fRange);
}

int CBsKernel::RegisterAniBillboardType(const char* szFolderName, int* pnSkinIndices, int nSkinCount,  int nAniCount, int nBillboardSize)
{
	return m_pBillboardMgr->RegisterAniBillboardType(szFolderName, pnSkinIndices, nSkinCount, nAniCount, nBillboardSize);
}

void CBsKernel::SetBillboardAniOffset(int nBillBoardIndex, int nAniIndex, float fAniOffset, int nAniType, int nAniAttr)
{
	m_pBillboardMgr->SetBillboardAniOffset(nBillBoardIndex, nAniIndex, fAniOffset, nAniType, nAniAttr);
}

void CBsKernel::ProcessParticleObjectList()
{
	int i, nSize, nHandle;

	nSize = m_ParticlePool.Size();
	for( i = 0; i < nSize; i++ ) {
		if( m_ParticlePool[ i ]->GetUse() )	{
			if( m_ParticlePool[ i ]->ProcessParticle() ) {
				nHandle = m_ParticlePool.GetHandle( i );
				DeleteParticleObject( nHandle );
			}
		}
	}
}

void CBsKernel::UpdateParticleObjectList()
{
	int i, nSize;

	nSize=m_ParticlePool.Size();
	for(i=0;i<nSize;i++){
		if(m_ParticlePool[i]->GetUse()){
			m_ParticlePool[i]->UpdateParticle();
		}
	}
}

void CBsKernel::RenderParticleObjectList()
{
	BeginPIXEvent(255, 0, 0, "RenderParticleObjectList");

	int i, nSize;
	D3DXVECTOR3 ParticleVec, ParticlePos;
	float fDist;

	RENDER_INFO obj;
	const D3DXVECTOR3* pVecEye=(D3DXVECTOR3*)&(GetParamInvViewMatrix()->_41);
	const D3DXVECTOR3* pVecLookAt=(D3DXVECTOR3*)&(GetParamInvViewMatrix()->_31);

	nSize=m_ParticlePool.Size();
	for(i=0;i<nSize;++i) {
		if(m_ParticlePool[i]->GetUse()) {
			ParticlePos=*((D3DXVECTOR3 *)&(m_ParticlePool[i]->GetParticleMatrix()->_41));
			ParticleVec=ParticlePos-*pVecEye;
			fDist=D3DXVec3Length(&ParticleVec);
			D3DXVec3Normalize(&ParticleVec, &ParticleVec);
			if( m_ParticlePool[ i ]->UseCull() )
			{
				if( D3DXVec3Dot( &ParticleVec, pVecLookAt ) < 0.7071f )
				{
					continue;
				}
			}
			obj.nObjIndex=i;
			obj.fDistance=fDist;
			m_Particles.push_back(obj);
		}
	}

	m_pDevice->SetRenderState(D3DRS_ALPHAREF, 0x01);
	m_pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	m_pDevice->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
	m_pDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
	m_pDevice->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL);
	m_pDevice->SetRenderState(D3DRS_ALPHAREF, 0x01);

	std::sort( m_Particles.begin(), m_Particles.begin() + m_Particles.size(), CompareDistance );
	int nIndex = m_Particles.size() - 1;
	CBsParticleGroup::SetCommonParticleConstant();

	// 멀리있는것부터 Draw
	for( i = nIndex; i >= 0; --i ) {
		m_ParticlePool[ m_Particles[ i ].nObjIndex ]->RenderParticle( m_pDevice );
	}
	m_Particles.erase( m_Particles.begin(), m_Particles.end() );

	m_pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	m_pDevice->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
	m_pDevice->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
	m_pDevice->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL);
	m_pDevice->SetRenderState(D3DRS_ALPHAREF, m_pDevice->GetAlphaRefValue());

	EndPIXEvent();
}

void CBsKernel::SaveIteratePosition(int nIndex, ITERATE_SAVE *pSave)
{
	int nIterateCount;

	nIterateCount = ( int )m_IteratePos.size();

	BsAssert( nIndex < nIterateCount );
	if( nIndex >= nIterateCount ) {
		return;
	}
	m_IteratePos[ nIndex ] = *pSave;
}

ITERATE_SAVE *CBsKernel::GetIteratePosition( int nIndex )
{
	int nIterateCount;

	nIterateCount = ( int )m_IteratePos.size();

	BsAssert( nIndex < nIterateCount );
	if( nIndex >= nIterateCount ) {
		return NULL;
	}

	return &( m_IteratePos[ nIndex ] );
}

static int fun_BS_GET_FX_NAME_LIST(char *szSearch, char *szFxNameList) // szFxNameList 해당 서치 결과를 모두 넣어 줍니다
{
	WIN32_FIND_DATA FileData;
	HANDLE hFind = FindFirstFile(szSearch, &FileData);
	int nIndex=0;

	if (hFind == INVALID_HANDLE_VALUE) {
		BsAssert(0 && "FX 리스트 에러 1");
	} 
	else {
		while (1) {
			// TODO: Assert that size is multiple of 256
			strcpy_s(szFxNameList + nIndex*256, 256, FileData.cFileName); // 문자열 대입 입니다 = char [][256]  
			nIndex++;

			if (!FindNextFile(hFind, &FileData)) {
				if (GetLastError() != ERROR_NO_MORE_FILES) 
				{
					BsAssert(0 && "FX 리스트 에러 2");
				}

				break;
			}
		}
		FindClose(hFind);
	}

	//strcpy_s(szFxNameList + nIndex*256, 256, ""); // 마지막 라인 입니다
	(szFxNameList + nIndex*256)[0]=0; //aleksger - Appending empty string

	return nIndex;
}

int CBsKernel::ProcessMessage(DWORD dwCode, DWORD dwParam1, DWORD dwParam2, DWORD dwParam3)
{
	switch(dwCode) {
		case BS_GET_TEXTURESIZE:
			{
				if(dwParam1<0)
					return 0;
				SIZE* pSize=(SIZE*)dwParam2;
				*pSize=GetTextureSize(dwParam1);
				return 1;
			}
		case BS_GET_FX_NAME_LIST:
			return fun_BS_GET_FX_NAME_LIST((char *)dwParam1, (char *)dwParam2);    
		case BS_GET_USE_TEXTURE_NAME:
			if( m_pSkinList[ dwParam1 ] ) {
				m_pSkinList[ dwParam1 ]->GetTextureList( ( std::vector< std::string > * )dwParam2 );
			}
			return 1;
		default:
			BsAssert( !"Error Invalid Kernel Command!!");
	}
	return 0;
}

void CBsKernel::DeallocateObject(int nObjectIndex)
{
	BsAssert( nObjectIndex != - 1);
	SAFE_DELETE(m_ppObjectPool[nObjectIndex]);
}

/////////////////////////////////////////////////////////////////////////////////////////////
// Physics Member
/////////////////////////////////////////////////////////////////////////////////////////////

int	CBsKernel::CreateClothObjectFromSkin( int nSkinIndex, PHYSICS_DATA_CONTAINER *pPhysicsInfo, D3DXMATRIX *matObject)
{
	int nIndex;

	if (!m_pSkinList[nSkinIndex])
		return -1;

	nIndex=SearchEmptyObjectIndex();
	m_ppObjectPool[nIndex]=new CBsClothObject();
	m_ppObjectPool[nIndex]->SetKernelPoolIndex( nIndex );
//	DebugString("CreateClothObjectFromSkin : %d\n", nIndex);


	// Mesh Attach
	CBsMesh* pMesh = m_pMeshList[m_pSkinList[nSkinIndex]->GetMeshIndex()];
	BsAssert(pMesh);
	int nSubMeshCount = m_ppObjectPool[nIndex]->AttachMesh(pMesh);

		
	// Material Link
	LinkSkinToObject(nIndex, nSkinIndex);

	( ( CBsClothObject * ) m_ppObjectPool[nIndex] )->SetEntireObjectMatrix ( matObject );


	if( _stricmp(pMesh->GetMeshFileName(), TEMP_FILENAME_CLOTH_LS_SL3 ) == 0 ) 
		m_pPhysicsMgr->SetCustomValue( CBsPhysicsMgr::CUSTOM_LS_SL3_CLOTH );
	if( _stricmp(pMesh->GetMeshFileName(), TEMP_FILENAME_CLOTH_LP_PM ) == 0 ) 
		m_pPhysicsMgr->SetCustomValue( CBsPhysicsMgr::CUSTOM_LP_PM_CLOTH );

	( ( CBsClothObject * ) m_ppObjectPool[nIndex] )->CreateActors( pPhysicsInfo );

	m_pPhysicsMgr->SetCustomValue( CBsPhysicsMgr::CUSTOM_NONE );


	if( _stricmp(pMesh->GetMeshFileName(), TEMP_FILENAME_PROP_MP_CL17 ) == 0 ) 
		( ( CBsClothObject * ) m_ppObjectPool[nIndex] )->MakeActorToMesh();

	return nIndex;
}

void CBsKernel::RunPhysics(float fDeltaTime)
{

	m_pPhysicsMgr->RunPhysics( fDeltaTime );
}

#ifdef _USAGE_TOOL_
BM_MATERIAL_HEADER*	CBsKernel::GetEngineObjectMaterialPtr(int nObjectIndex, int nSubMesh) // 매트리얼 툴에서 사용 합니다
{
	return &m_ppObjectPool[nObjectIndex]->GetMeshPt()->GetSubMeshPt(nSubMesh)->mtr_header;
}

int CBsKernel::GetPrimitiveType(int nMeshIndex)
{
	if(!(m_pMeshList[nMeshIndex]))
		return -1;
	return m_pMeshList[nMeshIndex]->GetPrimitiveType(0);
}
#endif

void CBsKernel::SetTextureName(int nTextureIndex, const char* pTextureName) {
	if(m_pTextureList[nTextureIndex]) {
		m_pTextureList[nTextureIndex]->SetTextureName(pTextureName);
	}
}

void CBsKernel::ShowFXObject( int nIndex, bool bShow )
{
	if( m_ppObjectPool[nIndex] ) {
		CBsFXObject *pObject = (CBsFXObject *)m_ppObjectPool[nIndex];
		pObject->Show( bShow );
	}
}

void CBsKernel::SetFXObjectState( int nIndex, CBsFXObject::STATE State, int nParam )
{
	if( m_ppObjectPool[nIndex] && nIndex != -1 ) {
		CBsFXObject *pObject = (CBsFXObject *)m_ppObjectPool[nIndex];
		pObject->SetState( State, nParam );
	}
}

bool CBsKernel::IsShowFXObject( int nIndex )
{
	if( m_ppObjectPool[nIndex] )
		return m_ppObjectPool[nIndex]->IsShow();
	return false;
}

CBsFXObject::STATE CBsKernel::GetFXObjectState( int nIndex )
{
	if( m_ppObjectPool[nIndex] ) {
		CBsFXObject *pObject = (CBsFXObject *)m_ppObjectPool[nIndex];
		return pObject->GetCurState();
	}
	return CBsFXObject::STOP;
}

void CBsKernel::CheckValidCameraIndex( int nIndex )
{
	int nCameraCount = m_CameraIndices.size();
	for(int i=0;i<nCameraCount;++i) {
		int nCameraIndex = m_CameraIndices[i];
		if( nIndex == nCameraIndex ) {
			return;
		}
	}
	BsAssert( 0 && "Invalid CameraIndex" );
}

std::vector< int > *CBsKernel::GetUpdateListByProcess()
{
	return m_UpdateBuffer + CBsObject::GetProcessBufferIndex();
}

std::vector< int > *CBsKernel::GetUpdateListByRender()
{
	return m_UpdateBuffer + CBsObject::GetRenderBufferIndex();
}

void CBsKernel::ProcessDeleteObject( KERNEL_COMMAND *pCommand )
{
	int nIndex;

	nIndex = pCommand->nBuffer[ 0 ];
	if( m_ppObjectPool[ nIndex ] ) {
//		DebugString("call : Delete(%d) true in ProcessDelete()\n", nIndex);
		DeallocateObject(nIndex);
	}
}

void CBsKernel::ResetKernelCommand()
{
	PreKernelCommand();
	ProcessKernelCommand();
}

void CBsKernel::PreKernelCommand()
{
	// Call Before RenderFrame();
	int i;

	int nBufferIndex = CBsObject::GetRenderBufferIndex();

	for( i = 0; i < (int)m_PreKernelCommand[ nBufferIndex ].size(); ++i ) {
		switch( m_PreKernelCommand[ nBufferIndex ][ i ].nCommand ) {
		case OBJECT_COMMAND_ENABLEALPHABLEND:
			{
				CBsObject* pObject = m_ppObjectPool[m_PreKernelCommand[ nBufferIndex ][i].nBuffer[0]];
				BsAssert(pObject && "Pre Kernel Command : Invalid Object!!");
				if(pObject) {
					pObject->ObjectAlphaBlend(m_PreKernelCommand[ nBufferIndex ][i].nBuffer[1]!=0);
				}
			}
			break;
		case OBJECT_COMMAND_SET_ALPHAWEIGHT:
			{
				CBsObject* pObject = m_ppObjectPool[m_PreKernelCommand[ nBufferIndex ][i].nBuffer[0]];
				BsAssert(pObject && "Pre Kernel Command : Invalid Object!!");
				if(pObject) {
					pObject->SetObjectAlphaWeight(m_PreKernelCommand[ nBufferIndex ][i].fBuffer);
				}
			}
			break;
		case OBJECT_COMMAND_RESTOREALPHABLEND:
			{
				CBsObject* pObject = m_ppObjectPool[m_PreKernelCommand[ nBufferIndex ][i].nBuffer[0]];
				BsAssert(pObject && "Pre Kernel Command : Invalid Object!!");
				if(pObject) {
					pObject->RestoreAlphaBlend();
				}
			}
			break;
		case OBJECT_COMMAND_LINKOBJECT:
			{
				CBsObject* pObject = m_ppObjectPool[m_PreKernelCommand[ nBufferIndex ][i].nBuffer[0]];
				BsAssert(pObject && "Invalid Object!!");
				pObject->ReqLinkObject(&m_PreKernelCommand[nBufferIndex][i]);

				CBsObject *pIsFxObject;
				pIsFxObject = m_ppObjectPool[ m_PreKernelCommand[ nBufferIndex ][ i ].nBuffer[ 2 ] ];
				if( pIsFxObject->GetObjectType() == CBsObject::BS_FX_OBJECT )
				{
					( ( CBsFXObject * )pIsFxObject )->SetState( CBsFXObject::RESUME );
					pIsFxObject->Show( true );
				}
			}
			break;
		case KERNEL_COMMAND_SETGAMMA:
			{
				m_pDevice->SetGammaRamp(m_PreKernelCommand[ nBufferIndex ][i].fBuffer);
			}
			break;
		case OBJECT_COMMAND_UNLINKOBJECT:
			{
				CBsObject* pObject = m_ppObjectPool[m_PreKernelCommand[ nBufferIndex ][i].nBuffer[0]];

				if( pObject )
					pObject->ReqUnlinkObject(&m_PreKernelCommand[nBufferIndex][i]);
			}
			break;
		case OBJECT_COMMAND_UNLINKBONE:
			{
				CBsAniObject* pAniObject = (CBsAniObject*)m_ppObjectPool[m_PreKernelCommand[ nBufferIndex ][i].nBuffer[0]];

				if( pAniObject )
					pAniObject->ReqUnlinkBone(&m_PreKernelCommand[nBufferIndex][i]);
			}
			break;
		default:
			BsAssert( 0 && "Invalid Pre Kernel Command!!");
			break;
		}
	}
	m_PreKernelCommand[ nBufferIndex ].clear();
}

void CBsKernel::ProcessKernelCommand()
{
	int i, nSize, nHandle;

	int nBufferIndex = CBsObject::GetRenderBufferIndex();

	for( i = 0; i < (int)m_PostKernelCommand[ nBufferIndex ].size(); ++i ) {
		switch( m_PostKernelCommand[ nBufferIndex ][ i ].nCommand )
		{
		case KERNEL_COMMAND_DELETE_OBJECT:
			ProcessDeleteObject( &m_PostKernelCommand[ nBufferIndex ][ i ] );
			break;
		case KERNEL_COMMAND_RELOAD_MATERIAL:
			ReloadAllMaterial();
			break;
		case KERNEL_COMMAND_DELETE_RTTOBJECT:
			m_pRTTManager->Release(m_PostKernelCommand[nBufferIndex][i].nBuffer[0]);
			break;
		default:
			BsAssert( 0 && "Invalid Post Kernel Command!!");
			break;
		}
	}
	m_PostKernelCommand[ nBufferIndex ].clear();

	THREAD_AUTOLOCK( &CBsParticleObject::s_csSharedParticleData );
	nSize = m_ParticlePool.Size();
	for( i = 0; i < nSize; i++ ) {
		if( ( m_ParticlePool[ i ]->GetUse() ) && ( m_ParticlePool[ i ]->IsDelete() ) ) {
			nHandle = m_ParticlePool.GetHandle( i );
			m_ParticlePool.DeleteObject( nHandle );
			if( m_ParticlePool[ i ]->GetIterate() ) {
				DeleteIteratePosition( m_ParticlePool[ i ]->GetSaveIteratePosition(), 
					m_ParticlePool[ i ]->GetTotalParticleCount() );
			}
			m_ParticlePool[ i ]->ResetParticleInfo();
		}
	}
}

void CBsKernel::SetCurrentAni( int nObjectIndex, int nAniIndex, float fFrame )
{
	BsAssert( nObjectIndex >= 0 );

	if( m_ppObjectPool[ nObjectIndex ] ) {
		BsAssert( m_ppObjectPool[nObjectIndex]->GetObjectType()==CBsObject::BS_ANIMATION_OBJECT );
		( ( CBsAniObject * )m_ppObjectPool[ nObjectIndex ] )->SetCurrentAni( nAniIndex, fFrame, 0 );
	}
}

void CBsKernel::BlendAni( int nObjectIndex, int nBlendAni, float fBlendFrame, float fBlendWeight )
{
	BsAssert( nObjectIndex >= 0 );

	if( m_ppObjectPool[ nObjectIndex ] ) {
		BsAssert( m_ppObjectPool[nObjectIndex]->GetObjectType()==CBsObject::BS_ANIMATION_OBJECT );
		( ( CBsAniObject * )m_ppObjectPool[ nObjectIndex ] )->BlendAni( nBlendAni, fBlendFrame, fBlendWeight, 0 );
	}
}

void CBsKernel::SetCalcAniFlag( int nObjectIndex, int nFlag )
{
	BsAssert( nObjectIndex >= 0 );

	if( m_ppObjectPool[ nObjectIndex ] ) {
		BsAssert( m_ppObjectPool[nObjectIndex]->GetObjectType()==CBsObject::BS_ANIMATION_OBJECT );
		( ( CBsAniObject * )m_ppObjectPool[ nObjectIndex ] )->SetCalcAniFlag( nFlag );
	}
}

void CBsKernel::GetAniDistance( int nObjectIndex, GET_ANIDISTANCE_INFO *pInfo )
{
	BsAssert( nObjectIndex >= 0 );

	if( m_ppObjectPool[ nObjectIndex ] ) {
		BsAssert( m_ppObjectPool[nObjectIndex]->GetObjectType()==CBsObject::BS_ANIMATION_OBJECT );
		( ( CBsAniObject * )m_ppObjectPool[ nObjectIndex ] )->GetAniDistance( pInfo );
	}
}

int CBsKernel::GetAniLength( int nObjectIndex, int nAni )
{
	BsAssert( nObjectIndex >= 0 );

	if( m_ppObjectPool[ nObjectIndex ] ) {
		BsAssert( m_ppObjectPool[nObjectIndex]->GetObjectType()==CBsObject::BS_ANIMATION_OBJECT );
		return ( ( CBsAniObject * )m_ppObjectPool[ nObjectIndex ] )->GetAniLength( nAni );
	}

	return 0;
}

void CBsKernel::AssertCaptureScreen()
{
#ifdef ON_SCREEN_ASSERT
	if(g_bAssertScreenCapture) {
		SYSTEMTIME time;
		GetSystemTime(&time);
		char szFileName[64];
		sprintf_s(szFileName,_countof(szFileName),"d:\\Assert_%d_%d_%d_%d_%d.bmp",time.wMonth, time.wDay, time.wHour,time.wMinute,time.wSecond);
		SaveScreen( szFileName );
		g_bAssertScreenCapture = false;
	}
#endif
}

#include "MGSToolbox\MGSToolboxConfig.h"
#if defined(USE_MGSTOOLBOX)

#include "MGSToolbox\DebugValue.h"

unsigned int DumpTextureInfo( CBsTexture * texture )
{
	char const * textureName = texture->GetTextureName();
	char const * displayName = textureName ? textureName : "<unknown>";

	IDirect3DBaseTexture9 * baseTexture = texture->GetTexturePtr();
	D3DRESOURCETYPE type = baseTexture->GetType();

	unsigned int memorySize;

	char const * typeName;

	switch( type )
	{
	default:
	case D3DRTYPE_NONE:
	case D3DRTYPE_SURFACE:
	case D3DRTYPE_VOLUME:
	case D3DRTYPE_VERTEXBUFFER:
	case D3DRTYPE_INDEXBUFFER:
		typeName = "<unsupported>";

		memorySize = 0;

		break;

	case D3DRTYPE_TEXTURE:
		typeName = "texture";

		IDirect3DTexture9 * normalTexture;

		normalTexture = reinterpret_cast<IDirect3DTexture9*>(baseTexture);

		D3DSURFACE_DESC normalDesc;
		normalTexture->GetLevelDesc(0, &normalDesc);

		memorySize = XGSetTextureHeader(normalDesc.Width, normalDesc.Height, baseTexture->GetLevelCount(), 0, normalDesc.Format, 0, 0, XGHEADER_CONTIGUOUS_MIP_OFFSET, 0, NULL, NULL, NULL);

		break;

	case D3DRTYPE_VOLUMETEXTURE:
		typeName = "volume texture";

		IDirect3DVolumeTexture9 * volumeTexture;

		volumeTexture = reinterpret_cast<IDirect3DVolumeTexture9*>(baseTexture);

		D3DVOLUME_DESC volumeDesc;
		volumeTexture->GetLevelDesc(0, &volumeDesc);

		memorySize = XGSetVolumeTextureHeader( volumeDesc.Width, volumeDesc.Height, volumeDesc.Depth, volumeTexture->GetLevelCount(), 0, volumeDesc.Format, 0, 0, XGHEADER_CONTIGUOUS_MIP_OFFSET, NULL, NULL, NULL);

		break;

	case D3DRTYPE_CUBETEXTURE:
		typeName = "cube texture";

		IDirect3DCubeTexture9 * cubeTexture;

		cubeTexture = reinterpret_cast<IDirect3DCubeTexture9*>(baseTexture);

		D3DSURFACE_DESC cubeDesc;
		cubeTexture->GetLevelDesc(0, &cubeDesc);

		memorySize = XGSetCubeTextureHeader(cubeDesc.Width, cubeTexture->GetLevelCount(), 0, cubeDesc.Format, 0, 0, XGHEADER_CONTIGUOUS_MIP_OFFSET, NULL, NULL, NULL);

		break;

	case D3DRTYPE_ARRAYTEXTURE:
		typeName = "array texture";

		IDirect3DArrayTexture9 * arrayTexture;

		arrayTexture = reinterpret_cast<IDirect3DArrayTexture9*>(baseTexture);

		D3DSURFACE_DESC arrayDesc;
		arrayTexture->GetLevelDesc(0, &arrayDesc);

		memorySize = XGSetArrayTextureHeader(arrayDesc.Width, arrayDesc.Height, arrayTexture->GetArraySize(), arrayTexture->GetLevelCount(), 0, arrayDesc.Format, 0, 0, XGHEADER_CONTIGUOUS_MIP_OFFSET, 0, NULL, NULL, NULL); 

		break;
	}

	SIZE size = texture->GetTextureSize(0);

	memorySize = ( ( memorySize + GPU_TEXTURE_ALIGNMENT - 1 ) / GPU_TEXTURE_ALIGNMENT ) * GPU_TEXTURE_ALIGNMENT;

	printf( "name = %s (%s), size = %dx%d, mem = %4.2f KB ref = %d\n",
		displayName, typeName, size.cx, size.cy, static_cast<float>(memorySize) / 1024.f, texture->GetRefCount() );

	return memorySize;
}

void DumpAllKernelTextures()
{
	unsigned int totalSize = 0;

	for ( int index = 0; index < MAX_TEXTURE_COUNT; ++index )
	{
		CBsTexture* texture = CBsKernel::GetInstance().GetBsTexture( index );
		if ( texture )
		{
			totalSize += DumpTextureInfo( texture );
		}
	}

	printf( "\nTotal Memory: %f MB (%u Bytes)\n",
		static_cast<float>(totalSize) / (1024.f * 1024.f),
		totalSize );
}

void DumpSkinInfo( CBsSkin * skin )
{
	char const * skinName = skin->GetSkinFileName();
	char const * displayName = skinName ? skinName : "<unknown>";

	printf( "name = %s, ref = %d\n",
		displayName, skin->GetRefCount() );
}

void DumpAllKernelSkins()
{
	for ( int index = 0; index < MAX_SKIN_COUNT; ++index )
	{
		CBsSkin* skin = g_BsKernel.Get_pSkin( index );
		if ( skin )
		{
			DumpSkinInfo( skin );
		}
	}
}

void DumpParticleGroupInfo( CBsParticleGroup * group )
{
	char const * groupName = group->GetParticleFileName();
	char const * displayName = groupName ? groupName : "<unknown>";

	printf( "name = %s, ref = %d\n",
		displayName, group->GetRefCount() );
}

void DumpAllKernelParticleGroups()
{
	for ( int index = 0; index < MAX_PARTICLE_GROUP_COUNT; ++index )
	{
		CBsParticleGroup* group = g_BsKernel.GetParticleGroup( index );
		if ( group )
		{
			DumpParticleGroupInfo( group );
		}
	}
}

void DumpFXTemplateInfo( CBsFXTemplate * fx )
{
	char const * templateName = fx->GetFileName();
	char const * displayName = templateName ? templateName : "<unknown>";

	printf( "name = %s, ref = %d\n",
		displayName, fx->GetRefCount() );
}

void DumpAllFXTemplates()
{
	int nFXTemplateCount = g_BsKernel.GetFXTemplateCount();
	for ( int index = 0; index < nFXTemplateCount; ++index )
	{
		CBsFXTemplate* fx = g_BsKernel.GetFXTemplatePtr( index );
		if ( fx )
		{
			DumpFXTemplateInfo( fx );
		}
	}
}


void DumpAniInfo( CBsAni * ani )
{
	char const * aniName = ani->GetAniFileName();
	char const * displayName = aniName ? aniName : "<unknown>";

	printf( "name = %s, ref = %d\n",
		aniName, ani->GetRefCount() );
}

void DumpAllKernelAniInfo()
{
	for ( int index = 0; index < MAX_ANI_COUNT; ++index )
	{
		CBsAni * ani = g_BsKernel.Get_pAni( index );
		if ( ani )
		{
			DumpAniInfo( ani );
		}
	}
}

MGSToolbox::DebugValue g_textureDumpStats ( "Stats", "Show Textures", false);
MGSToolbox::DebugValue g_skinDumpStats ( "Stats", "Show Skins", false);
MGSToolbox::DebugValue g_particleDumpStats ( "Stats", "Show Particles", false);
MGSToolbox::DebugValue g_fxDumpStats ( "Stats", "Show FX", false);
MGSToolbox::DebugValue g_aniDumpStats ( "Stats", "Show Animations", false);
MGSToolbox::DebugValue g_allDumpStats ( "Stats", "Show All", false);

void BsKernelUpdate()
{
	if ( g_textureDumpStats.asBool() )
	{
		g_textureDumpStats.fromBool( false );

		printf( "Textures:\n" );
		DumpAllKernelTextures();
	}

	if ( g_skinDumpStats.asBool() )
	{
		g_skinDumpStats.fromBool( false );

		printf( "Skins:\n" );
		DumpAllKernelSkins();
	}

	if ( g_particleDumpStats.asBool() )
	{
		g_particleDumpStats.fromBool( false );

		printf( "Particle Groups:\n" );
		DumpAllKernelParticleGroups();
	}

	if ( g_fxDumpStats.asBool() )
	{
		g_fxDumpStats.fromBool( false );

		printf( "FX Templates:\n" );
		DumpAllFXTemplates();
	}

	if ( g_aniDumpStats.asBool() )
	{
		g_aniDumpStats.fromBool( false );

		printf( "Ani Objects:\n" );
		DumpAllKernelAniInfo();
	}

	if ( g_allDumpStats.asBool() )
	{
		g_allDumpStats.fromBool( false );

		printf( "Ani Objects:\n" );
		DumpAllKernelAniInfo();

		printf( "FX Templates:\n" );
		DumpAllFXTemplates();

		printf( "Particle Groups:\n" );
		DumpAllKernelParticleGroups();

		printf( "Skins:\n" );
		DumpAllKernelSkins();

		printf( "Textures:\n" );
		DumpAllKernelTextures();
	}
}

#endif
