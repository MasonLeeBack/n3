#pragma once

#include "BsCommon.h"
#include "3DDevice.h"
#include "BsTexture.h"
#include "Singleton.h"
#include "ObjectPoolMng.h"
#include "BsStringPool.h"
#include "BsLightObject.h"
#include "BsCamera.h"
#include "BsConsole.h"
#include "BsFXObject.h"
#include "BsFXLoadObject.h"
#include "BsPhysicsBase.h"
#include "BsUtil.h"
#include "BsShadowMgr.h"
#include "BsFileIO.h"

#ifdef _XBOX
#include "./DX9_Util/AtgFont.h"
#endif

#include "BsUiClick.h"

#define OUTHST	"OutHst.txt"

// TODO : Presentation interval conrol and Skip using animation cache for Performance check by jeremy
//#define _BS_PERFORMANCE_CHECK

#define MAX_MESH_COUNT						1000
#define MAX_ANI_COUNT						1000
#define MAX_MATERIAL_COUNT					1000
#define MAX_VERTEXDECLARATION_COUNT			400
#define MAX_TEXTURE_COUNT					1000
#define MAX_SKIN_COUNT						3000

#define MAX_FXTEMPLATE_COUNT				600
#define START_ARBITARY_FXTEMPLATE_INDEX		300

#define MAX_STRING_COUNT					300

#define DEFAULT_BS_OBJECT_COUNT				30000
#define FIRST_DRAW_BLOCK_COUNT				10

#define DEFAULT_SAVE_MATRIX_COUNT			5000
#define DEFAULT_SAVE_MATRIX_ADD_COUNT		500

#define MAX_PARTICLE_GROUP_COUNT			600
#define START_ARBITARY_PARTICLE_INDEX		300

#define MAX_PARTICLE_OBJECT_COUNT			600
#define MAX_ITERATE_POSTION_COUNT			50000
#define MAX_POINT_LIGHT_COUNT				400

#define DEFAULT_RENDER_PARTICLE_LIST_SIZE	400

#define DEFAULT_UPDATE_OBJECT_LIST_SIZE		1000
#define MAX_POINT_LIGHT 5

#ifdef _XBOX
	#ifdef _LTCG
		#define _PACKED_RESOURCES
	#elif PROFILE
		#define _PACKED_RESOURCES
	#endif
#endif

class CFrameChecker;
class CBsWorld;
class CBsHFWorld;
class CBsWater;
class CBsShadowMgr;
class CBsShadowVolumeMgr;
class CBsImageProcess;
class CBsRealMovie;
class CBsOffScreenManager;
class CBsUIManager;
class CBsCamera;
class CBsMesh;
class CBsAni;
class CBsMaterial;
class CBsVertexDeclaration;
class CBsTexture;
class CBsSkin;
class CBsFileLoader;
class CCrossVector;

class CBsObject;

#ifdef _XBOX
	class ATG::Font;
#endif

class CBsParticleGroup;
class CBsParticleObject;

class IBsPhysicsMgr;
class IBsPhysicsActor;

struct PHYSICS_DATA_CONTAINER;
struct PHYSICS_COLLISION_CONTAINER;

class CBsCustomRender;
class CBsDecalManager;

class CBsInstancingMgr;
class CBsObjectBillboardMgr;

struct RENDER_INFO {
	int		nObjIndex;
	float	fDistance;
};

// Excute After RenderFrame()
#define KERNEL_COMMAND_DELETE_OBJECT			0x00000001		// Post Command
#define KERNEL_COMMAND_DELETE_POINT_LIGHT		0x00000002		// Post Command
#define KERNEL_COMMAND_DELETE_CAMERA			0x00000003		// Post Command
#define KERNEL_COMMAND_DELETE_PARTICLE			0x00000004		// Post Command
#define KERNEL_COMMAND_RELOAD_MATERIAL			0x00000005		// Post Command
#define KERNEL_COMMAND_CREATE_RMRESOURCE		0x00000006		// Post Command
#define KERNEL_COMMAND_DELETE_RTTOBJECT			0x00000007		// Post Command for Removing RTT in another thread

// Excute Before RenderFrame()
#define OBJECT_COMMAND_LINKOBJECT				0x00000010		// Pre Command
#define OBJECT_COMMAND_UNLINKOBJECT				0x00000011		// Pre Command
#define OBJECT_COMMAND_UNLINKBONE				0x00000012		// Pre Command
#define OBJECT_COMMAND_ENABLEALPHABLEND			0x00000100		// Pre Command
#define OBJECT_COMMAND_SET_ALPHAWEIGHT			0x00000101		// Pre Command
#define OBJECT_COMMAND_RESTOREALPHABLEND		0x00000102		// Pre Command
#define KERNEL_COMMAND_SETGAMMA					0x00000103		// Pre Command


struct KERNEL_COMMAND
{
	int		nCommand;
	int		nBuffer[ 3 ];
	float	fBuffer;
};

#ifdef _XBOX
struct DeferredResource
{
	D3DResource *pResource;
	VOID		*pData;
	DWORD		dwSize;
#ifdef _DEBUG
	char		szName[MAX_PATH];
#endif
};
#endif

class CBsKernel : public CSingleton<CBsKernel>
{
public:
	CBsKernel();
	virtual ~CBsKernel();

#ifdef _XBOX
	HRESULT InitializeDevice();
	HRESULT Initialize();
	void Reset(D3DPRESENT_PARAMETERS *pParameters) { m_pDevice->Reset(pParameters); }

	void	BeginPIXEvent(int r, int g, int b, const char* szName)	{	PIXBeginNamedEvent(D3DCOLOR_ARGB(255, r, g, b), szName);	}
	void EndPIXEvent()											{	PIXEndNamedEvent();	}

#else
	HRESULT Initialize( HWND hWnd, int nWidth, int nHeight, bool bIsWindowed, bool bMultithreaded = false );
	HRESULT ReInitialize( HWND hWnd, int nWidth, int nHeight, bool bIsWindowed, bool bMultithreaded = false );

	int	BeginPIXEvent(int r, int g, int b, const char* szName)	{	return 0;	}
	int EndPIXEvent()											{	return 0;	}
#endif
	C3DDevice*	GetDevice()		
	{	
		return m_pDevice;	
	}
	bool		IsActive()		{	return m_bActive;	}

	void	OutHst(char* szFileName, char* szFmt,...);
	void	SaveScreen(char* szFileName);

	// Rendering 관련 Interface...
#ifdef INV_Z_TRANSFORM
	void ClearBuffer(int nR, int nG, int nB, float fZ = 0.f, DWORD dwStencil=0)	{ 	m_pDevice->ClearBuffer(D3DCOLOR_ARGB(0xff, nR, nG, nB), fZ, dwStencil); }
#else
	void ClearBuffer(int nR, int nG, int nB, float fZ = 1.f, DWORD dwStencil=0)	{ 	m_pDevice->ClearBuffer(D3DCOLOR_ARGB(0xff, nR, nG, nB), fZ, dwStencil); }
#endif
	void UpdateObject( int nObjectIndex, D3DXMATRIX* pMatrix = NULL );
	void ShowObject( int nObjectIndex, bool bShow );
	// 커널용  Process 함수에 ProcessParticleObjectList 같은걸 넣어서 외부에선 Process만 한번 호출하게 바꿨으면..
	void Process();
	void SetPreCalcAni(int nObjectIndex, bool bEnable);
	void PreInitRenderFrame();
	void PostInitRenderFrame();
	void InitRenderFrame();
	void InitRenderRTTFrame(CBsRTTCamera* pRTTCamera);
	void RenderFrame();	
	void RenderRTTFrame(CBsRTTCamera* pRTTCamera, C3DDevice* pDevice);
	void ResetDoubleBuffer();
	void ResetFrame();
	void Show();
	void FrameCheckStart();
	
#ifdef _USAGE_TOOL_
	const D3DXMATRIX*	GetViewMatrix(int nCameraIndex)				{	CheckValidCameraIndex( nCameraIndex ); return ((CBsCamera*)m_ppObjectPool[nCameraIndex])->GetViewMatrix();	}
	const D3DXMATRIX*	GetViewProjectionMatrix(int nCameraIndex)	{	CheckValidCameraIndex( nCameraIndex ); return ((CBsCamera*)m_ppObjectPool[nCameraIndex])->GetViewProjMatrix();	}
	const D3DXVECTOR4*	GetFogFactor(int nCameraIndex)	{ CheckValidCameraIndex( nCameraIndex ); return ((CBsCamera*)m_ppObjectPool[nCameraIndex])->GetFogFactor(); }
	const D3DXMATRIX*	GetProjectionMatrix(int nCameraIndex)		{	return ((CBsCamera*)m_ppObjectPool[nCameraIndex])->GetProjectionMatrix();	}
#endif

	CBsTexture*			GetBsTexture(int nIndex)	{	return m_pTextureList[nIndex];	}
	void GetScreenPos(int nCameraIndex, D3DXVECTOR3* pVecPos, D3DXVECTOR3* pRes )			{	CheckValidCameraIndex( nCameraIndex );	((CBsCamera*)m_ppObjectPool[nCameraIndex])->GetScreenPos(pVecPos, pRes);	}
	void GetProjectionPos(int nCameraIndex, D3DXVECTOR3* pVecPos, D3DXVECTOR3* pRes )		{	CheckValidCameraIndex( nCameraIndex );	((CBsCamera*)m_ppObjectPool[nCameraIndex])->GetProjectionPos(pVecPos, pRes);	}

	void SetProjectionMatrix( int nCameraIndex, float fNearZ, float fFarZ )		{	((CBsCamera*)m_ppObjectPool[nCameraIndex])->SetProjectionMatrixByProcess(fNearZ, fFarZ); }
	void SetCameraFov(int nCameraIndex, float fFov)								{	((CBsCamera*)m_ppObjectPool[nCameraIndex])->SetFOVByProcess(fFov);	}

	void SetFogFactor(int nCameraIndex, float fStart, float fEnd)	{	((CBsCamera*)m_ppObjectPool[nCameraIndex])->SetFogFactor(fStart, fEnd);	}
	void SetFogColor(int nCameraIndex, D3DXVECTOR4* pVec)			{	((CBsCamera*)m_ppObjectPool[nCameraIndex])->SetFogColor(pVec); }

	void EnableClearScreen(int nCameraIndex, BOOL bIsUse)			{	CheckValidCameraIndex( nCameraIndex );	((CBsCamera*)m_ppObjectPool[nCameraIndex])->EnableScreenClear(bIsUse);	}
	void SetCrearColor(int nCameraIndex, int nR, int nG, int nB)	{	CheckValidCameraIndex( nCameraIndex );	((CBsCamera*)m_ppObjectPool[nCameraIndex])->SetClearColor(nR, nG, nB);	}

	float GetFogStart(int nCameraIndex)	{ CheckValidCameraIndex( nCameraIndex );	return ((CBsCamera*)m_ppObjectPool[nCameraIndex])->GetFogStart(); }
	float GetFogEnd(int nCameraIndex)	{ CheckValidCameraIndex( nCameraIndex );	return ((CBsCamera*)m_ppObjectPool[nCameraIndex])->GetFogEnd(); }
	const D3DXVECTOR4* GetFogColor(int nCameraIndex)	{	CheckValidCameraIndex( nCameraIndex );	return ((CBsCamera*)m_ppObjectPool[nCameraIndex])->GetFogColor(); }

	void				LockActiveCamera(int nCameraIndex);
	void				UnlockActiveCamera();

	void				SetActiveCamera( CBsCamera *pCamera) { m_pActiveCamera = pCamera; } 
	CBsCamera*			GetActiveCamera()					{	return m_pActiveCamera;	}
	CBsCamera*			GetCamera(int nCameraIndex) { CheckValidCameraIndex( nCameraIndex );	return ((CBsCamera*)m_ppObjectPool[nCameraIndex]); } // 2D UI 용 입니다
	int					GetCameraHandle(int nIndex) { return m_CameraIndices[nIndex]; }
	int					GetCameraCount() { return (int)m_CameraIndices.size(); }
	
	//************************************************************************************ Scene Render때만 쓰이는 함수들....
	const D3DXMATRIX*	GetParamViewMatrix()						{ return m_pActiveCamera->GetViewMatrix();	}
	const D3DXMATRIX*	GetParamInvViewMatrix();
	const D3DXMATRIX*	GetParamProjectionMatrix()					{ return m_pActiveCamera->GetProjectionMatrix();	}
	const D3DXMATRIX*	GetParamViewProjectionMatrix()				{ return m_pActiveCamera->GetViewProjMatrix();	}
	const D3DXVECTOR4*	GetParamFogFactor()	{ return m_pActiveCamera->GetFogFactor(); }
	const D3DXVECTOR4*	GetParamFogColor()	{ return m_pActiveCamera->GetFogColor(); }
	float				GetParamFarZ()		{ return m_pActiveCamera->GetFarZ();	}
	const D3DXMATRIX*	GetParamShadowAllForCast()	{	return GetShadowMgr()->GetLightViewProject();	}
	const D3DXMATRIX*	GetParmaShadowAllForSampling()	{	return GetShadowMgr()->GetLightViewProjectForSampling();	}
	//************************************************************************************

	bool	IsVisibleObject( int nCameraIndex, int nIndex, float* pfDistanceFromCamera);
	bool	IsVisibleObject( CBsCamera* pCamera, int nIndex, float* pfDistanceFromCamera);
	bool	IsVisibleTestFromCamera( int nCameraIndex, Box3* pBox );
	bool	IsVisibleTestFromCamera( Box3* pBox );
	bool	IsVisibleTestFromCameraForGiant( Box3* pBox);

	bool	IsVisibleTestFromCameraForGiantByRender(Box3* pBox);

	void	InitIsVisibleObjectLite( CBsCamera* pCamera );
	bool	IsVisibleObjectLite( CBsObject* pObject, float* PfViewDistance, bool *PbShadow );

	const D3DXVECTOR4* GetLightPosition();	 
	const D3DXVECTOR4* GetLightDirection();
	const D3DXVECTOR4* GetLightDiffuse();
	const D3DXVECTOR4* GetLightSpecular();
	const D3DXVECTOR4* GetLightAmbient();

	void InitPointLightObjects();
	int ApplyPointLight( D3DXVECTOR3 Pos, float fRadius);
	int GetActivePointLightNum();
	D3DXVECTOR4* GetActivePointLight(int nIndex);
	D3DXVECTOR4* GetActivePointLightColor(int nIndex);
	void UpdatePointLightDecal();

	void	EnableImageProcess(BOOL bEnable);
	BOOL	IsEnableImageProcess();
	CBsImageProcess* GetImageProcess()	{	return m_pImageProcess;	}
	void	EnableShowDebugTexture(BOOL bIsShow);

public:
	//**********************************************************************
	// Object 관련 Interface
	//**********************************************************************
	int			CreateStaticObject( int nMeshIndex, bool bCreateDummyObject=false);
	int			CreateAniObject( int nMeshIndex, int nAniIndex );
	int			CreateStaticObjectFromSkin( int nSkinIndex);
	int			CreateAniObjectFromSkin( int nSkinIndex, int nAniIndex );
	// TODO : 제거대상 프로젝트 끝나면, Code Clean!!!!
	// Desc : 실기무비용 Animation Object 생성(GetBox3()만 Overriding했습니다.)
	int			CreateRMAniObjectFromSkin( int nSkinIndex, int nAniIndex );

	int			CreateCameraObject(int nRenderTargetIndex, float fStartX, float fStartY, float fWidth, float fHeight, bool bIsRTT = false);
	int			CreateLightObject(LIGHT_TYPE type);
	int			CreateSkyBoxObjectFromSkin( int nSkinIndex);
	void		ResetWaterObject();
	int			CreateWaterObjectFromSkin(int nSkinIndex);
	bool		IsWaterInside( D3DXVECTOR3 Pos );
	float		GetWaterHeight();
	float		GetWaterDistance( D3DXVECTOR3 Pos );

	int			LoadFXTemplate(int nIndex, char *pszFXFileName);
	int			FindFXTemplate(char* pszFXFileName);
	int			FindEmptyFXTemplateIndex();
	void		AddFXTemplateRef(int nIndex);
	void		ReleaseFXTemplate(int nIndex);
	void		FlushFXTemplates();
	void		DeleteFXTemplate( int nIndex );
	void		DeleteAllFXTemplates();

	CBsFXTemplate* GetFXTemplatePtr( int nIndex );
	int            GetFXTemplateCount() { return MAX_FXTEMPLATE_COUNT; }

	int			CreateFXObject( int nObjectIndex );

	void		AttachCustomRender(CBsCustomRender* pRender);
	void		DetachCustomRender();

	void		AddDecal(float fX, float fZ, int nTexture, float fRadius, int nLifeTime, float fRotAngle, D3DCOLOR dwColor = D3DCOLOR_XRGB(255,255,255), int nMode = 0);	// nMode = 1 (Blend Add Mode)
	void		ResetDecalObject();

	// 보장되지 않는 참조이므로 실시간으로 만들어서 사용 해야 함에 주의 합니다  
	CBsObject*			GetEngineObjectPtr(int nObjectIndex) { return m_ppObjectPool[nObjectIndex]; }
	int			GetEngineObjectCount() { return m_nPoolSize; }
	CBsParticleObject*	GetEngineParticlePtr(int nParticleHandle) { return m_ParticlePool[nParticleHandle&0xffff]; } 
	BM_MATERIAL_HEADER*	GetEngineObjectMaterialPtr(int nObjectIndex, int nSubMesh); // 리얼무비 클래스 + 매트리얼 툴에서 사용 합니다

	void		DeleteObject( int nObjectIndex );

	void		SetDirectionalLight(int nLightIndex)	{	m_nDirectionalLightIndex = nLightIndex;	}
	void		DeletePointLightIndex( int nIndex );
	void		DeleteCameraIndex( int nIndex );
	void		DeleteAllObject();

	int			SendMessage( int nObjectIndex, DWORD dwCode, DWORD dwParam1 = 0, DWORD dwParam2 = 0, DWORD dwParam3 = 0 );

	int			CreateHeightField( int nXSize, int nZSize, short int* pHeight, const char **ppFileList, const char* pszLightMapFileName, DWORD *pMulColor, DWORD *pAddColor);
	void		CreateHeightFieldActor( int nXSize, int nZSize, short int *pHeight );
	void		DeleteWorld();
	float		GetLandHeight( float fX, float fZ, D3DXVECTOR3* N = NULL );
	float		GetFlatHeight( int nX, int nZ);
	float		GetHeight(float fX, float fZ);
	void		GetLandSlope( float fX, float fZ, float& fVX, float& fVZ );

	CBsMesh    *Get_pMesh(int nMeshIndex) { return m_pMeshList[nMeshIndex]; }  
	CBsSkin	   *Get_pSkin(int nSkinIndex) { return m_pSkinList[nSkinIndex]; }
	CBsAni     *Get_pAni (int nIndex    ) { return m_pAniList[nIndex]; } // BM에 BA의 본 강제 적용시킬때 사용 됩니다

	//**********************************************************************
	// Resource 관련 Interface
	//**********************************************************************
    void	Reload();

	void	CreateVertexDeclaration(CONST D3DVERTEXELEMENT9* pVertexElements, IDirect3DVertexDeclaration9** ppDecl )
			{	m_pDevice->CreateVertexDeclaration(pVertexElements, ppDecl);	}
	void	CreateVertexBuffer( UINT Length, DWORD Usage, DWORD FVF, D3DPOOL Pool, IDirect3DVertexBuffer9** ppVertexBuffer )
			{	m_pDevice->CreateVertexBuffer(Length, Usage, FVF, Pool, ppVertexBuffer); }
	void	LockVertexBuffer(LPDIRECT3DVERTEXBUFFER9 pVB, UINT Offset, UINT LockSize, void** ppData, DWORD dwFlags)
			{
				BsAssert( !IsLockedVertexBuffer() );
				SetLockedVertexBuffer(true);
				pVB->Lock(Offset, LockSize, ppData, dwFlags);
			}
	void UnlockVertexBuffer(LPDIRECT3DVERTEXBUFFER9 pVB)
			{
				BsAssert( IsLockedVertexBuffer() );
				SetLockedVertexBuffer(false);
				pVB->Unlock();
			}
	void	CreateIndexBuffer( UINT Length, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DIndexBuffer9** ppIndexBuffer )
			{ m_pDevice->CreateIndexBuffer(Length, Usage, Format, Pool, ppIndexBuffer); }
	HRESULT CreateEffectFromFile(const char* pszFileName, LPD3DXEFFECT* ppEffect, LPD3DXEFFECTPOOL pEffectPool=NULL);
#ifdef _XBOX
	HRESULT CreateFXLEffectFromFile(const char* pszFileName, FXLEffect** ppEffect, FXLEffectPool* pEffectPool=NULL) {	return m_pDevice->CreateFXLEffectFromFile(pszFileName, ppEffect, pEffectPool);	}
#endif

	// Mesh Resource Management
	int		LoadMesh(int nIndex, const char* pFileName/* Software Vertex Transform은 작업안됨*/);
	int		FindMesh( const char* pFileName );
	int		FindEmptyMeshSlot();
	int		ReloadMesh( int nIndex, bool bUseSoftware = false );
	void	DeleteMesh( int nIndex );
	void	ReleaseMesh( int nIndex );
	void	DeleteAllMesh();
	void	ReloadAllMesh();

	// Ani Resource Management
	int		FindAni( const char* pFileName );
	int		LoadAni( int nIndex, const char* pFileName, bool bUseRootBone = true );
	void    AddAniRef();
	int		FindEmptyAniIndex();
	void    ReleaseAni( int nIndex );
	void    FlushAnis();
	void	DeleteAni( int nIndex );
	void	DeleteAllAni();
	void	DeleteBone( int nAniIndex, int nBoneIndex );

	int	GetAniCount(int nAniIndex);

	// Material Resource Management
	int			LoadMaterial(const char* pFileName, BOOL bUseMaterial=TRUE);
	int			FindMaterial(const char* pFileName);
	int			FindEmptyMaterialIndex();
	CBsMaterial*	GetMaterialPtr(int nMaterialIndex)	{	return m_pMaterialList[nMaterialIndex];	}
	void		ReleaseMaterial(int nIndex);
	void		DeleteMaterial(int nMaterialIndex);
	void		DeleteAllMaterial();
	void		ReloadAllMaterial();
	void		FlushMaterials();

	// Vertex Declaration Resource Management
	int			LoadVertexDeclaration(D3DVERTEXELEMENT9* pVertexElements);
	int			FindVertexDeclaration(D3DVERTEXELEMENT9* pVertexElements);
	int			FindEmptyVertexDeclarationIndex();
	CBsVertexDeclaration*	GetBsVertexDeclaration(int nVertexDeclIndex)	{	return m_pVertexDeclList[nVertexDeclIndex];	}
	void		ReleaseVertexDeclaration(int nIndex);
	void        FlushVertexDeclarations();
	void		ReloadAllBsVertexDeclaration();

	// Texture Resource Management
	int			LoadTexture( const char* pFileName, DWORD dwUsage=0, const bool bGetFullName = true);
	int			LoadCubeTexture( const char* pFileName);
	int			LoadVolumeTexture( const char* pFileName);

	void GetTexResInfo( char* pFileName, D3DRESOURCETYPE* pType, UINT* pDepth );

#ifdef _XBOX
	int			LoadArrayTexture( const char* pFolderName );
#ifdef _DEBUG
	void		AddDeferredDeleteResource( D3DResource *pResource, VOID *pData, char *pName, DWORD dwSize );
#else
	void		AddDeferredDeleteResource( D3DResource *pResource, VOID *pData, DWORD dwSize );
#endif
protected:
	int			m_nDeferredTextureCount;
	void		DeferredTextureDelete();
public:
#endif

	int			CreateTexture(int nWidth, int nHeight, DWORD dwUsage, D3DFORMAT format, D3DPOOL pool = D3DPOOL_DEFAULT);
	int			FindTexture( const char* pFileName );
	int			FindEmptyTextureIndex();
	void		DeleteTexture( int nIndex );
	void		DeleteTexture( char* pFileName );
	void		DeleteAllTexture();
	void		LostAllTexture();
	SIZE		GetTextureSize( int nTextureIndex );
	const char *GetTextureName(int nIndex);
	void		ReleaseTexture(int nIndex);
	void*		GetTexturePtr(int nIndex)	{	return (nIndex==-1) ? NULL : m_pTextureList[nIndex]->GetTexturePtr();	}
	void		ReloadAllTexture();
	void		AddTextureRef(int nIndex);

	void		SetTexture(DWORD dwSampler, int nTextureIndex);
	void		SetVertexDeclaration(int nVertexDeclIndex);

	void		SetGammaRamp(float fGamma);
	float		GetGammaRamp()				{	return m_pDevice->GetGammaRamp();	}

	// Skin Resource Management
	int			LoadSkin(int nIndex, const char* pFilename);
	void		AddSkinRef(int nSkinIndex);
	int			FindSkin(const char* pSkinFile);
	int			FindEmptySkinIndex();
	void		ReleaseSkin(int nIndex);
	void        FlushSkins(void);
	void		DeleteSkin(int nIndex);
	void		DeleteSkin(char* pFilename);
	void		DeleteAllSkin();

	float		GetRenderTime()				{	return float(m_dwRenderTick[CBsObject::GetRenderBufferIndex()])/40.f;	}			// FrameChecker에서 가져와야 함 by jeremy
	DWORD		GetRenderTick()				{	return m_dwRenderTick[CBsObject::GetRenderBufferIndex()];				}
	DWORD		GetRenderTickByProcess()	{	return m_dwRenderTick[CBsObject::GetProcessBufferIndex()];				}

	void		SetRenderTick( DWORD dwTick )	{	m_dwRenderTick[CBsObject::GetProcessBufferIndex()] = dwTick; }
#ifdef _USAGE_TOOL_
	void		AddRenderTick()					{	m_dwRenderTick[CBsObject::GetProcessBufferIndex()]++;	}
#endif

	int			GetDirectionLightIndex()	{	return m_nDirectionalLightIndex;	}

	// Resource Load 관련 디렉토리 함수들
	void		InitializeFileLoader(const char *pResourceDir);
	void		SetCurrentDirectory( const char* pPathName );
	DWORD		GetCurrentDirectory( DWORD nBufferLength, char* pBuffer );
	const char *GetCurrentDirectory();
	int			chdir( const char* pDirectoryName );
	char	   *GetFullName(const char *pFileName);
	void		SetShaderDirectory(const char *pPathname);
	char	   *GetShaderDirectory() { return m_szShaderDir; }
	void		BackupCurDir(char *pFileName);
	void		RestoreCurDir();

	//	Text 출력 관련 함수
	void		ShowFPS( bool bShow, int nX = -1, int nY = -1 );
	void		ShowDebugStr( bool bShow ){ m_bShowDebugStr = bShow; }
	void		PrintString( int nX, int nY, const char* pString, DWORD dwColor = 0xffffffff );
			 
	// String Pool 관련 함수
	char	   *FnaPoolAdd(const char *szName) { return m_FnaPool.Add(szName); }	// 문자열을 추가 합니다 
	char	   *FnaPoolReadAdd(BStream *pStream) { return m_FnaPool.ReadAdd(pStream); }	// 파일에서 문자열을 읽어서 문자열을 추가 합니다
	void		FnaPoolClear(char *szName) { m_FnaPool.Clear(szName); }				// 위의 두함수를 사용시 반드시 적절한 클리어를 해줍니다  

// Particle 관련 함수들
	void		GetParticleCount(int& nCurParticleCount, int& nMaximumParticleCount);
	// Modify Siva
	CBsParticleGroup *GetParticleGroup(int nIndex);
	CBsParticleObject *GetParticleObject( int nHandle );

	// ParticleGroup resource management
	void		AddParticleGroupRef(int nIndex);
	int			FindParticleGroup(const char* pFileName);
	int			LoadParticleGroup(int nIndex, const char *pFileName);
	void		ReLoadAllParticleGroup();
	int			ReLoadParticleGroup(int nIndex);
	int			FindEmptyParticleGroupIndex();
	void		ReleaseParticleGroup(int nIndex);
	void		FlushParticleGroups();
	void		DeleteParticleGroup(int nIndex);
	void		DeleteAllParticleGroup();
	int			CreateParticleObject(int nParticleGroupIndex, bool bLoop, bool bIterate, D3DXMATRIX *pParticleMat, float fScale = 1.f, D3DCOLORVALUE *pColor=NULL, bool bAutoDelete = true );

	bool		IsPlayParticleObject(int nHandle);
	void		DeleteParticleObject(int nHandle);
	void		DeleteAllParticleObject();
	void		UpdateParticle(int nHandle, D3DXMATRIX *pMatParticle, bool bChangeScale=false, D3DCOLORVALUE *pColor=NULL);
	void		UpdateParticle(int nHandle, const D3DXVECTOR3& pos);
	void		ProcessParticleObjectList();
	void		UpdateParticleObjectList();
	void		RenderParticleObjectList();
	void		SaveIteratePosition( int nIndex, ITERATE_SAVE *pSave );
	ITERATE_SAVE *GetIteratePosition( int nIndex );
	int			AllocIteratePosition(int nCount);
	void		DeleteIteratePosition(int nIndex, int nCount);
	void		StopParticlePlay(int nHandle, int nStopTick=-1);
	void		ShowParticle(int nHandle, bool bShow);
	bool		IsShowParticle(int nHandle);
	void		PauseParticle(int nHandle, bool bPause);
	bool		IsPauseParticle(int nHandle);
	void		SetParticleTick( int nHandle, int nTick );
	int			GetParticleTick( int nHandle );
	bool		IsParticleCull( int nHandle );
	void		SetParticleCull( int nHandle, bool bUse );
	void		UseParticleAlphaFog( int nHandle, bool bFlag );
	bool		IsParticleAlphaFog( int nHandle );


// FX 관련 함수들
	void ShowFXObject( int nIndex, bool bShow );
	void SetFXObjectState( int nIndex, CBsFXObject::STATE State, int nParam = 0 );

	bool IsShowFXObject( int nIndex );
	CBsFXObject::STATE GetFXObjectState( int nIndex );


	///////////////////////////////////////////////////////////////////////////////////////////////////////
	// Post Screen Effect
	void		SetSceneIntensity(float fSceneIntensity);
	float		GetSceneIntensity();
	void		SetBlurIntensity(float fBlur);
	float		GetBlurIntensity();
	void		SetGlowIntensity(float fGlow);
	float		GetGlowIntensity();
	void		SetHighlightThreshold(float fValue);
	float		GetHighlightThreshold();
	void		SetSceneAddColor(float r = 0.0f, float g = 0.0f, float b = 0.0f, float a = 0.0f );
	D3DXVECTOR4	GetSceneAddColor();

	// DOF
	void		SetDOFFocus(float fFocus);
	float		GetDOFFocus();
	void		SetDOFFocusRange(float fFocusRange);
//	float		GetDOFFocusRange();
	BOOL		IsEnabledDOF();
	void		EnableDOF(int bOn);
	void		SetDOFFocusFar(float fTap);
	void		SetDOFFocusNear(float fTap);
	float		GetDOFFocusNear();
	float		GetDOFFocusFar();
	void		SetDOFFocusOutNear(float fNear);
	void		SetDOFFocusOutFar(float fFar);
	float		GetDOFFocusOutNear();
	float		GetDOFFocusOutFar();
	void		SetDOFFocusOutNearDef();
	void		SetDOFFocusOutFarDef();

	///////////////////////////////////////////////////////////////////////////////////////////////////////
	// Shadow Manager
	void			ShowShadow( bool bShow ) { 	m_bShowShadow = bShow; }
	CBsShadowMgr*	GetShadowMgr()			{	return m_pShadowMgr;	}

	CBsWorld*		GetWorld()				{	return	m_pWorld;		}
	int				GetWorldLightMap();
	void			LoadWorldLightMap(const char* szLightMapFileName);
	float			GetRcpWorldXSize();
	float			GetRcpWorldZSize();
	LPDIRECT3DTEXTURE9	GetReflectMap();

	int				GetBlankTexture()		{	return m_nBlankTexture;	}

	//-------------------------------------Instancing 관련-------------------------------------------------
	CBsInstancingMgr*	GetInstancingMgr(void) { return m_pInstancingMgr; }
	void				EnableInstancing(bool bFlag);
	void				SetInstancingBufferSize(int nSize);
	//-----------------------------------------------------------------------------------------------------
	//-------------------------------------Billboard 관련--------------------------------------------------
	CBsObjectBillboardMgr*	GetBillboardMgr(void) { return m_pBillboardMgr; }
	void				EnableBillboard(bool bFlag);
	void				SetBillboardRange(float fRange);
	int				RegisterAniBillboardType(const char* szFolderName, int* pnSkinIndices, int nSkinCount,  int nAniCount, int nBillboardSize);
	void				SetBillboardAniOffset(int iBillInd, int iCount, float fAnimOffset, int iAnimType, int nAniAttr);
	//-----------------------------------------------------------------------------------------------------

	// Input
	void SetCurInputPort( int nCurPort )	{ m_nCurInputPort = nCurPort; }
	int GetCurInputPort()					{ return m_nCurInputPort; }

#ifndef _LTCG
	static int s_nTrackingIndex;
#endif
	// Kernel상태 관련 Data
	static int					s_nDrawTotalCount;
	static int					s_nDrawTotalAniCount;

protected:
	BsStringPool	m_FnaPool;	// bm, ba 에서 사용되는 파일 이름들을 가지고 있습니다
	char			m_szShaderDir[_MAX_PATH];

	bool			m_bLockedVertexBuffer;
	void			SetLockedVertexBuffer(bool bLocked)	{	m_bLockedVertexBuffer = bLocked;	}
	bool			IsLockedVertexBuffer()				{	return m_bLockedVertexBuffer;		}

	// Text 출력 변수
	bool			m_bShowFPS;
	bool			m_bShowDebugStr;
	int				m_nFPSX;
	int				m_nFPSY;

#ifdef _XBOX
#ifdef _USE_CONSOLE_COMMAND_
	ATG::Font*		m_pFont;
#endif
#else
	ID3DXFont*		m_pFont;
#endif
	int					m_nBlankTexture;

	CBsWorld*			m_pWorld;
	CBsWater*			m_pWater;
	bool				m_bShowShadow;
	CBsShadowMgr*		m_pShadowMgr;
	CBsShadowVolumeMgr*	m_pShadowVolumeMgr;
	CBsImageProcess*	m_pImageProcess;

	CBsCustomRender*	m_pFXManager;
	CBsDecalManager*	m_pDecalManager;

	CBsConsole		m_Console;

	//------------------------------------Instancing 관련-------------------------
	CBsInstancingMgr*	m_pInstancingMgr;
	//----------------------------------------------------------------------------
	//------------------------------------Billboard 관련--------------------------
	CBsObjectBillboardMgr*	m_pBillboardMgr;
	//----------------------------------------------------------------------------

	int				m_nStringCount;
	int				m_nStringX[ MAX_STRING_COUNT ];
	int				m_nStringY[ MAX_STRING_COUNT ];
	DWORD			m_dwStringColor[ MAX_STRING_COUNT ];
	char*			m_pStringBuffer[ MAX_STRING_COUNT ];
	int				m_nStringBufferLength[ MAX_STRING_COUNT ];

	// Kernel상태 관련 Data
	DWORD				m_dwRenderTick[ DOUBLE_BUFFERING ];
	int					m_nCameraCount;
	bool				m_bActive;

	//*****************************************************************************
	std::vector<int>	m_CameraIndices;
	CBsCamera*			m_pActiveCamera;				// Rendering할때만 쓰임
	//*****************************************************************************
	std::vector< int >	m_UpdateBuffer[ DOUBLE_BUFFERING ];

	int					m_nDirectionalLightIndex;

	int					m_nPointLightCount;
	int					m_PointLightIndices[ MAX_POINT_LIGHT_COUNT ];
	int					m_nSelectedPointLights[ MAX_POINT_LIGHT + 1 ];
	std::vector< D3DXVECTOR4 > m_ActivePointLightInfo[ DOUBLE_BUFFERING ];
	std::vector< D3DXVECTOR4 > m_ActivePointLightColor[ DOUBLE_BUFFERING ];

	C3DDevice*			m_pDevice;

	CFrameChecker*		m_pChecker;
	CBsDirManager		*m_pDirManager;


	//**********************************************************************
	// Resource Pool
	//**********************************************************************
	CBsMesh*				m_pMeshList[MAX_MESH_COUNT];
	CBsAni*					m_pAniList[MAX_ANI_COUNT];
	CBsMaterial*			m_pMaterialList[MAX_MATERIAL_COUNT];
	CBsVertexDeclaration*	m_pVertexDeclList[MAX_VERTEXDECLARATION_COUNT];
	CBsTexture*				m_pTextureList[MAX_TEXTURE_COUNT];
#ifdef _XBOX
	DeferredResource		m_DeferredDeleteList[MAX_TEXTURE_COUNT];
#endif
	CBsSkin*				m_pSkinList[MAX_SKIN_COUNT];

	//**********************************************************************
	//	파티클 관련 멤버들..
	//**********************************************************************
	CObjectPoolMng<CBsParticleObject>	m_ParticlePool;
//	std::vector<CBsParticleObject *>	m_ParticlePool;
	std::vector<CBsParticleGroup *>		m_ParticleGroupList;
	std::vector< ITERATE_SAVE >			m_IteratePos;
//	int									m_nIterateCount;
	std::vector<RENDER_INFO>			m_Particles;

	//**********************************************************************
	// FX Template Data Pool
	//**********************************************************************
	CBsFXTemplate*						m_pFXTemplateList[MAX_FXTEMPLATE_COUNT];		

	//**********************************************************************
	// Object Pool
	//**********************************************************************
	int					m_nPoolSize;
	CBsObject**			m_ppObjectPool;

	std::vector< KERNEL_COMMAND >	m_PreKernelCommand[ DOUBLE_BUFFERING ];
	std::vector< KERNEL_COMMAND	>	m_PostKernelCommand[ DOUBLE_BUFFERING ];

	// it is valid in only RenderFrame()
	std::vector< CBsObject* >		m_TransparentObjectList;

	// PreCalc Animation Objects
	std::vector< int > m_PreCalcAniObjList;

	// Input
	int m_nCurInputPort;


	// CreateXXXXObjectBySkin()에서 공용으로 Skin처리 하는 루틴..
	void	LinkSkinToObject(int nObjectIndex, int nSkinIndex );

	int		SearchEmptyObjectIndex( bool bUseFirstBlock = false );

	void	RenderScene();
	void	RenderTransparentScene(BOOL bIsRenderParticle = TRUE);	
	void	RenderRTT();

	void	RenderCustom();
public:
	void	RenderSceneUI();

	void	RenderFirstBlock();
	void	RenderExceptionUI(int nExceptionType);
	void	SetUIException(int nExceptionType);

#ifdef _USE_CONSOLE_COMMAND_
	void	RenderText();
#else
	void	RenderText(){};
#endif

protected:
	void	RemoveInvalidObjectLinkParticle();
	void	RenderObjectList();

	void	RenderAlphaObjectList();

	int		ProcessMessage(DWORD dwCode, DWORD dwParam1=0, DWORD dwParam2=0, DWORD dwParam3=0);
	
	void	DeallocateObject(int nObjectIndex);



public:
	// 실기무비에서 필요한 함수 입니다. 
	//DeallocateObject 를 내부에서 호출하지만 나중을 위해서 같은 함수로 하지 않고 따로 함수를 뺏습니다.
	void	FxDeallocateObject(int nObjectIndex) { DeallocateObject(nObjectIndex); };		
	

protected:
	
	IBsPhysicsMgr	*m_pPhysicsMgr;
	IBsPhysicsActor *m_pHFActor;
	IBsPhysicsActor *m_pPlaneActor[4];
	
	bool	m_bPhysicsEnable;

protected:
	CBsRealMovie*			m_pRealMovie;
	CBsOffScreenManager*	m_pRTTManager;
	CBsUiClickTexture*		m_pClickFxEffect;

public:
	CBsOffScreenManager*	GetRTTManager()			{ return m_pRTTManager; }
	CBsUiClickTexture*		GetClickFxEffect()		{ return m_pClickFxEffect; }

protected:
	//********************************************************************************************
	// CBsUIElement, CBsUIManager, 유저 인터 페이스
	//********************************************************************************************
	CBsUIManager* m_pUIManager; // <Data/Material/UIBase.fx> 와 <Data/Controls.dds> 를 사용 합니다 
public: 
	
	//********************************************************************************************
	// UI 관련 Member들
	//********************************************************************************************
	void    CreateUI();
	void	CreateUIforTool();
	void    SetUIViewerIndex(int nIndex);		// 이후에 생성되는 모든 Element 에 적용 됩니다
	void	ClearUIElemental();

	//----------------------------------------
	bool	DrawUIMesh(BS_UI_MODE mode, int nMeshIndex, int nX, int nY, D3DXCOLOR color, int nTexId,
		float param1 = 0.f, float param2 = 0.f, float param3 = 0.f, float param4 = 0.f);

	//----------------------------------------
	//drawbox(직접 texture를 호출하는 경우를 뺀 모든 mode)
	bool	DrawUIBox(BS_UI_MODE mode, 
		int x1, int y1, int x2, int y2, 
		float fz, D3DXCOLOR color, 
		float fRot = 0.f,
		int nTextureID = -1,
		int ux1=0, int uy1=0,
		int ux2=0, int uy2=0,
		int nDummyTexId = -1,
		bool bRTT = false);

	bool	DrawUIBox_s(BS_UI_MODE mode,
		int x1, int y1, int sx, int sy, 
		float fz, D3DXCOLOR color,
		float fRot = 0.f,
		int nTextureID = -1,
		int ux1=0, int uy1=0,
		int ux2=0, int uy2=0,
		int nDummyTexId = -1,
		bool bRTT = false);

	bool	DrawUIBox_Pos4(BS_UI_MODE mode,
		int x1, int y1, int x2, int y2,
		int x3, int y3, int x4, int y4,
		float fz, D3DXCOLOR color,
		int nTextureID = -1,
		int ux1=0, int uy1=0,
		int ux2=0, int uy2=0,
		int nDummyTexId = -1,
		bool bRTT = false);

	bool CBsKernel::DrawUIBox_PosColor4(BS_UI_MODE mode, 
		int x1, int y1, D3DXCOLOR color1,
		int x2, int y2, D3DXCOLOR color2,
		int x3, int y3, D3DXCOLOR color3,
		int x4, int y4, D3DXCOLOR color4,
		float fz,
		int nTextureID = -1,
		int ux1=0, int uy1=0,
		int ux2=0, int uy2=0,
		int nDummyTexId = -1,
		bool bRTT = false);
	
	//---------------------------------------- 
	char         *GetUIFPSString();
	
	// 해당 텍스쳐의 사이즈를 넘겨 줍니다
	void          GetUITextureSize(int nTextureId, int &nWeight, int &nHeight);
	
	//---------------------------------------- 
	//debug
	// 윈도우 문자열 출력 루틴 입니다
#ifdef _USE_CONSOLE_COMMAND_
	bool		CreateUIDebug(int x,  int y,  char *str, D3DXCOLOR color);
	// 따로 호출하지 않습니다 (렌더링 부분에 바로 적용되는 루틴 입니다) 
	void		RenderUIDebug(int x, int y, char *str, DWORD dwColor); 
#else
	bool		CreateUIDebug(int x,  int y,  char *str, D3DXCOLOR color){return false;};
	void		RenderUIDebug(int x, int y, char *str, DWORD dwColor){};
#endif
	//---------------------------------------- 
	
	//bool		IsUIRTTRenderEnable();
	//void		SetUIRTTRenderEnable(bool bEnable);

	//---------------------------------------- 

	CBsDirManager *GetDirManager() { return m_pDirManager; } // 파일 로드시 폴더 관련해서 사용 합니다

	//********************************************************************************************
	// CBsRealMovie = 실기 무비 (사용법은 실기무비툴의 CBSReal 클래스 참조 바랍니다)
	//********************************************************************************************
	CBsRealMovie*	GetRealMovie()	{	return m_pRealMovie;	}
	void RealMovieSave(BStream *pStream); // 저장 합니다                g_BsKernel.chdir(dir) 지정 필요 합니다
	void RealMovieLoad(BStream *pStream); // 로드 합니다 (메모리 할당)  g_BsKernel.chdir(dir) 지정 필요 합니다
	// void RealMoviePlay(float fTime, D3DXMATRIX *pMatrix, CCrossVector *pCameraCross, CCrossVector *pLightCross); // 플레이 합니다

	void RealMovieProcess(float fTime,int nTick);
	void RealMovieProcess(DWORD dwTick);

	void RealMovieClear(); // 사용후 반드시 클리어 합니다 (메모리 소거)	= 프로그램 종료시에는 자동 클리어 됩니다
	void RealMovieNew(); // 클리어 이후 빈 문서를 만들어 줍니다

	//---------------------------------------- 

#ifdef _USAGE_TOOL_
	static int g_nSkipSameTexture; // 그래픽툴과 매트리얼툴을 동시에 사용할때 같은 이름의 텍스쳐 교환을 위해 사용 됩니다
	int LoadTextureTool(const char* pFileName, DWORD dwUsage=0) { g_nSkipSameTexture=1; return LoadTexture(pFileName,dwUsage); }
	int	LoadCubeTextureTool(const char* pFileName) { g_nSkipSameTexture=1; return LoadCubeTexture(pFileName); }
	int LoadVolumeTextureTool(const char* pFileName) { g_nSkipSameTexture=1; return LoadVolumeTexture(pFileName); }
#endif
	void SetTextureName(int nTextureIndex, const char* pTextureName);

	//**********************************************************************

	///////////////////////////////////////////////////////////////////////
	// Console 관련 함수들..
	void ConsoleInitialize()			{ m_Console.Initialize(m_pDevice); }
	bool IsShowConsole()				{ return m_Console.IsShowConsole(); }
	void SetShowConsole(bool bShow)		{ m_Console.SetShowConsole(bShow); }
	void ProcessConsole()				{ m_Console.ProcessConsole(); }

	void AddConsoleString(const char *pString)
	{
#ifndef _NO_DEBUG_CONSOLE
		printf("%s\n", pString);
#endif
		m_Console.AddString(pString); 
	}
	void AssertConsoleString(const char *pString)
	{
#ifndef _NO_DEBUG_CONSOLE
		printf("%s\n", pString);
#endif
		m_Console.AddString(pString, CBsConsole::ASSERT );
	}
	void ChangeConsoleString(const char *pString)	{ m_Console.ChangeString(pString); }
	void DeleteConsoleString()						{ m_Console.DeleteString(); }
	void ScrollConsole(int nLine)					{ m_Console.Scroll(nLine); }

	///////////////////////////////////////////////////////////////////////
	// Novodex Physice Member Function
	int	CreateClothObjectFromSkin( int nSkinIndex, PHYSICS_DATA_CONTAINER *pPhysicsInfo, D3DXMATRIX *matObject);
	//std::vector<int> CreateCollisionMesh( int nEngineIndex, PHYSICS_COLLISION_CONTAINER *pCollisionMeshInfo);
	IBsPhysicsMgr*	GetPhysicsMgr() {	return m_pPhysicsMgr;	}
	
	// Initialize 하기 전에 해주어야 합니다.
	void SetNovodexEnable(bool bEnable) { m_bPhysicsEnable = bEnable;}
	bool GetNovodexEnable() { return m_bPhysicsEnable; }

	void RunPhysics(float fDeltaTime);

public:
	LPDIRECT3DDEVICE9		GetD3DDevice() { return m_pDevice->GetD3DDevice(); }
	D3DPRESENT_PARAMETERS	*GetPresentParameer() { return m_pDevice->GetPresentParameter(); }

#ifdef _USAGE_TOOL_
public:
	CBsAni*				GetAni( int nIndex )					{ return m_pAniList[nIndex]; }
	int					GetPrimitiveType(int nMeshIndex);
#endif
	void CheckValidCameraIndex( int nIndex );		// debug용


	std::vector< int > *GetUpdateListByProcess();
	std::vector< int > *GetUpdateListByRender();

	void ResetKernelCommand();
	void PreKernelCommand();		// Call Before RenderFrame();
	void ProcessKernelCommand();	// Call After RenderFrame();

	std::vector<KERNEL_COMMAND>&	GetKernelCommand()	{	return m_PostKernelCommand[CBsObject::GetProcessBufferIndex()];	}	// Only use process thread
	int	 GetRenderKernelCommandCount()	{	return (int)m_PostKernelCommand[CBsObject::GetRenderBufferIndex()].size();	}

	std::vector<KERNEL_COMMAND>&	GetPreKernelCommand()	{	return m_PreKernelCommand[CBsObject::GetProcessBufferIndex()];	} // Only use Render Thread
	int	 GetRenderPreKernelCommandCount()	{	return (int)m_PreKernelCommand[CBsObject::GetRenderBufferIndex()].size();	}

	void ResetTransparentObjectList()
	{
		m_TransparentObjectList.erase(m_TransparentObjectList.begin(), m_TransparentObjectList.end());
	}
	void AddTranparentObject(CBsObject* pObject)	{	m_TransparentObjectList.push_back(pObject);	}

	void ProcessDeleteObject( KERNEL_COMMAND *pCommand );
	void ProcessDeletePointLight( KERNEL_COMMAND *pCommand );
	void ProcessDeleteCamera( KERNEL_COMMAND *pCommand );

	// 자주 쓰는 AniObject 메세지를 펑션으로 대체..
	void SetCurrentAni( int nObjectIndex, int nAniIndex, float fFrame );
	void BlendAni( int nObjectIndex, int nBlendAni, float fBlendFrame, float fBlendWeight );
	void SetCalcAniFlag( int nObjectIndex, int nFlag );
	void GetAniDistance( int nObjectIndex, GET_ANIDISTANCE_INFO *pInfo );
	int GetAniLength( int nObjectIndex, int nAni );

	// Debug
	void AssertCaptureScreen();
};

#define g_BsKernel	CBsKernel::GetInstance()