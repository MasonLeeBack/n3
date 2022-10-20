#pragma	   once

#include "3DDevice.h"
#include "Box3.h"

class CBsMesh;
class CBsMaterial;
class CBsObject;
class CBsImplMaterial;
class CBsShadowMgr;
class CBsShadowVolumeMgr;
class CBsInstancingMgr;
struct AABB;
struct KERNEL_COMMAND;

typedef void (*CustomRenderCallBackPtr)(void *, C3DDevice *, D3DXMATRIX *);

#define DOUBLE_BUFFERING	2
//#define BONE_SAVE_MATRIX_COUNT 48;

struct LINKEDOBJECT
{
	int nLinkDummyIndex;
	int nLinkObjectHandle;
};

class CBsObject
{
public:
	enum BS_OBJECT_TYPE
	{
		BS_STATIC_OBJECT = 0,
		BS_ANIMATION_OBJECT,
		BS_SKYBOX_OBJECT,
		BS_CAMERA_OBJECT,
		BS_LIGHT_OBJECT,
		BS_FX_OBJECT,
		BS_SIMULATE_OBJECT,
	};
	CBsObject();
	virtual ~CBsObject();

	virtual void ReInit();


protected:
	static	CBsShadowMgr*		s_pShadowMgr;
	static CBsShadowVolumeMgr*	s_pShadowVolumeMgr;
	static CBsInstancingMgr*	s_pInstancingMgr;

	// TODO : 마지막에 관련 코드를 제거해야합니다.
	static int					s_nTraceIndex;

	static int					m_sCurrentRenderBuffer;
	static int					m_sCurrentProcessBuffer;

	static CBsCriticalSection	s_csProtectLinkedObject;

	int						m_nSkinIndex;
	int						m_nKernelPoolIndex;
	BS_OBJECT_TYPE			m_BsObjectType;
	D3DXMATRIX				m_matObject[ DOUBLE_BUFFERING ];
	
	
	CBsMesh*				m_pMesh; // 메쉬 1 개의 포인터 입니다 
	CBsImplMaterial*		m_pImplMaterials; // 서브 메쉬 갯수 만큼 가지고 있습니다

	bool					m_bShow;
	bool					m_bShowBoundingBox;
	bool					m_bShowGrid;
	bool					m_bRootObject;		// Link Object 여부?
	bool					m_bEnableCull;
	bool					m_bEnableBillBoard;
	bool					m_bEnableInstancing;
	bool					m_bEnableLOD;

	bool					m_bObjectAlphaBlend;
	bool					m_bRestoreAlphaBlend;

	float					m_fObjectAlpha;

	DWORD					m_dwBoundingBoxColor;
	D3DXVECTOR3				m_vecCenter;
	BS_CLIP_TYPE			m_ClipType;

	//Box3					m_ObjectOBB;

	float					m_fDistanceFromCamera[ DOUBLE_BUFFERING ];		// For LOD & Shadow Casting

	BS_SHADOW_TYPE			m_ShadowCastType;

	//DWORD					m_nPredicationMask [ DOUBLE_BUFFERING ];			// Predicated Tiling Mask

#ifdef _USAGE_TOOL_
	// User Define Render Function!!
	CustomRenderCallBackPtr	m_pCallBack;
	void*					m_pCallBackObject;
#endif

	//*********************************************************
	//		Link 관련 멤버
	//*********************************************************
	std::vector<LINKEDOBJECT>	m_LinkedObjectList;

	//int						m_nParentIndex;
	
	virtual void LinkObject(int nLinkDummyIndex, int nObjectIndex);
	virtual const D3DXMATRIX *UnlinkObject(int nObjectIndex, D3DXVECTOR3 *unlinkVelocity = NULL );

	void	EnableBillBoard( bool bEnable )	{	m_bEnableBillBoard = bEnable;	}
	bool	UsingBillBoard()	{	return m_bEnableBillBoard;	}

public:

	void	SetCenter( D3DXVECTOR3 vCenter ) { m_vecCenter = vCenter; }

	virtual void	Show(bool bIsShow);
	bool			IsShow()								{	return m_bShow;	}

	void			EnableLOD(bool bIsEnable)				{	m_bEnableLOD = bIsEnable;	}
	bool			IsEnableLOD()							{	return m_bEnableLOD;		}

	void			SetSkinIndex( int nIndex );
	int				GetSkinIndex()							{	return m_nSkinIndex; }

	void			SetKernelPoolIndex(int nIndex)			{	m_nKernelPoolIndex = nIndex;	}
	int				GetKernelPoolIndex()					{	return m_nKernelPoolIndex;		}

	virtual void	ReqLinkObject(KERNEL_COMMAND* pCmd);
	virtual void	ReqUnlinkObject(KERNEL_COMMAND* pCmd);

	virtual void Process()						{}
	virtual void ProcessLinkedPhysicsObject( DWORD dwParam1 = 0, DWORD dwParam2 = 0 );	
	virtual void ProcessRagdollUnlink( D3DXVECTOR3 *unlinkVelocity )		{}
	virtual void ProcessLinkParticle()			{}
	virtual void ProcessLinkFX()				{}
	virtual void PostCalculateAnimation()		{}
	
	// Render Thread 에서 지워지기전에 Process Thread 에서 먼저Relase 처리해주어야 되는것 용도입니다
	virtual void Release()							{}

	void	SetRootObject(bool bIsRoot)			{	m_bRootObject = bIsRoot;	}
	bool	IsRootObject()						{	return m_bRootObject;		}

	BS_OBJECT_TYPE	GetObjectType()					{	return m_BsObjectType;	}
	void			SetObjectType(BS_OBJECT_TYPE type)	{	m_BsObjectType = type;	}

	BS_SHADOW_TYPE GetShadowCastType()				{	return m_ShadowCastType;	}
	void	SetShadowCastType(BS_SHADOW_TYPE type)	{	m_ShadowCastType = type;	}

	BS_CLIP_TYPE	GetClipType()					{	return m_ClipType;			}
	void			SetClipType(BS_CLIP_TYPE type)	{	m_ClipType = type;			}

	void	EnableObjectCull( bool bCull ) { m_bEnableCull = bCull; }
	bool	IsEnableObjectCull() { return m_bEnableCull; }

	void SetEntireObjectMatrix(D3DXMATRIX *pMatrix)	
	{	
		m_matObject[ 0 ] = *pMatrix;	
		m_matObject[ 1 ] = *pMatrix;	
	}
	virtual void SetObjectMatrix(D3DXMATRIX *pMatrix)	{	m_matObject[ m_sCurrentProcessBuffer ] = *pMatrix;	}
	void SetObjectMatrixByRender( D3DXMATRIX *pMatrix ) { m_matObject[ m_sCurrentRenderBuffer ] = *pMatrix; }
	virtual D3DXMATRIX* GetObjectMatrix()			{	return m_matObject + m_sCurrentRenderBuffer;	}
	D3DXMATRIX* GetObjectMatrixByProcess()			{	return m_matObject + m_sCurrentProcessBuffer;	}

	virtual void SaveMatrix( const D3DXMATRIX* pMatrix, int nSaveIndex ) { }
	virtual D3DXMATRIX*	GetRenderMatrix( int nIndex ) { return NULL; }
	virtual void		ResetRenderMatrix()	{}
	virtual void		ResetProcessMatrix()	{}
	virtual bool IsValidRenderMatrix() { return true; }
	
	//void SetPredicationMask( DWORD dwPredicationMask ) { m_nPredicationMask[ m_sCurrentProcessBuffer ] = dwPredicationMask;}
	//DWORD GetPredicationMask() { return m_nPredicationMask[ m_sCurrentRenderBuffer ]; }

#ifdef _USAGE_TOOL_
	void* GetCallBackObject()					{	return m_pCallBackObject;	}
#endif

	virtual int AttachMesh(CBsMesh* pMesh);
	int AttachMaterial(int nSubMeshIndex, int nMaterialIndex);

	virtual bool GetBox3     ( Box3 & B );
	//virtual const Box3* GetOBB()	{	return &m_ObjectOBB;	}
	//virtual bool UpdateOBB();
	virtual bool GetBox3_grid( Box3 & B );

	virtual float	GetMeshRadius();

	void	SetDistanceFromCam( float fDistanceFromCam )	{	m_fDistanceFromCamera[ m_sCurrentProcessBuffer ] = fDistanceFromCam;	}
	void	SetDistanceFromCamByRender( float fDistanceFromCam )	{	m_fDistanceFromCamera[ m_sCurrentRenderBuffer ] = fDistanceFromCam;	}
	float	GetDistanceFromCam()	{	return m_fDistanceFromCamera[ m_sCurrentRenderBuffer ];	} 
	float	GetDistanceFromCamByProcess()	{	return m_fDistanceFromCamera[ m_sCurrentProcessBuffer ];	} 
	virtual bool InitRender(float fDistanceFromCamera);	// Ani Object Draw Count 알기 위해서 리턴 밸류를 bool 로 바꾼다.. by mapping
	virtual void RefreshAni() { InitRender(0.f); }

	CBsMesh* GetMeshPt() { return m_pMesh; }

	void SetFadeInOut();

	virtual void PreRender(C3DDevice *pDevice) {	SetFadeInOut();	}
	virtual void Render(C3DDevice *pDevice);

	virtual void PreRenderAlpha(C3DDevice* pDevice)	{}
	virtual void RenderAlpha(C3DDevice *pDevice);

	virtual void PreRenderShadow(C3DDevice* pDevice)	{}
	virtual void RenderShadow(C3DDevice* pDevice);

	virtual void PreRenderShadowVolume(C3DDevice* pDevice)	{}
	virtual void RenderShadowVolume(C3DDevice* pDevice);

	virtual void PostRender(C3DDevice *pDevice) {}

	void	DrawBoundingBox(C3DDevice* pDevice);
	void	DrawGrid       (C3DDevice* pDevice);
	
	AABB*	GetBoundingBox();

	virtual int ProcessMessage(DWORD dwCode, DWORD dwParam1=0, DWORD dwParam2=0, DWORD dwParam3=0);

	static	void SetShadowMgr(CBsShadowMgr* pShadowMgr)	{	s_pShadowMgr = pShadowMgr;	}
	static	void SetShadowVolumeMgr(CBsShadowVolumeMgr* pShadowVolumeMgr)	{	s_pShadowVolumeMgr = pShadowVolumeMgr;	}
	static	void SetInstancingMgr(CBsInstancingMgr* pInstancingMgr)	{	s_pInstancingMgr = pInstancingMgr;	}

	void			RequestObjectAlphaBlend(bool bIsEnable);
	void			RequestRestoreAlphaBlend();
	void			RequestObjectAlphaWeight(float fAlpha);

	void			EnableObjectAlphaBlend(bool bIsEnable)	{	m_bObjectAlphaBlend = bIsEnable;	}
	virtual void	ObjectAlphaBlend(bool bIsEnable);
	bool			IsEnableObjectAlphaBlend()				{	return m_bObjectAlphaBlend;	}

	void			EnableRestoreAlphaBlend(bool bIsEnable)	{	m_bRestoreAlphaBlend = bIsEnable;	}
	virtual void	RestoreAlphaBlend();
	bool			IsEnableRestoreAlphaBlend()				{	return m_bRestoreAlphaBlend;	}

	float			GetAlphaWeight(void) { return m_fObjectAlpha; }
	void			SetAlphaWeight(float fAlpha)	{	m_fObjectAlpha = fAlpha;	}
	virtual void	SetObjectAlphaWeight(float fAlpha);

	virtual bool IsRagdoll() {return false;}	
	virtual void SetRagdollMatrix(	bool bLocalAxis, D3DXMATRIX *pMatrix) {}	

#ifdef _XBOX
	static void FlipRenderBuffer() { m_sCurrentRenderBuffer = 1 - m_sCurrentRenderBuffer; }
	static void FlipProcessBuffer() { m_sCurrentProcessBuffer = 1 - m_sCurrentProcessBuffer; }
#else
	static void FlipRenderBuffer() { }
	static void FlipProcessBuffer() { }
#endif
	static int GetRenderBufferIndex() { return m_sCurrentRenderBuffer; }
	static int GetProcessBufferIndex() { return m_sCurrentProcessBuffer; }
	static void ResetDoubleBuffer()
	{
		m_sCurrentRenderBuffer = 0;
		m_sCurrentProcessBuffer = 0;
	}
/*
	void SetParentIndex( int nIndex ) { m_nParentIndex = nIndex; }	
*/
};