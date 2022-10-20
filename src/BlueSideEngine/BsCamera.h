#pragma	   once
#include "BsObject.h"
#include "Collision/Frustum.h"
#include "3DDevice.h"

#define DEFAULT_DRAW_LIST_SIZE		1000

class CBsCamera : public CBsObject
{
	enum eCAMERATYPE {
		PERSPECTIVE = 0,
		ORTHOGONAL, 
	};
public:
	CBsCamera(int nRenderTarget, float fStartX, float fStartY, float fWidth, float fHeight, bool bIsRTT=false);
	virtual ~CBsCamera();

protected:
	eCAMERATYPE			m_CameraType;

	bool				m_bIsRTT;
	int					m_nRenderTargetTexture;
	C3DDevice*			m_pDevice;

	float				m_fViewStartX;
	float				m_fViewStartY;
	float				m_fViewWidth;
	float				m_fViewHeight;
	D3DVIEWPORT9		m_viewport;

	float				m_fAspect;
	float				m_fScaleWidth;
	float				m_fScaleHeight;
	float				m_fNearZ;
	float				m_fFarZ;
	float				m_fFov[DOUBLE_BUFFERING];

	float				m_fOrthoWidth;
	float				m_fOrthoHeight;

	/////////////////////////////////////////////////////////////////////////////////////////
	// Valid value only at Rendering time!!!!
	D3DXMATRIX			m_matView;
	D3DXMATRIX			m_matProj;
	D3DXMATRIX			m_matViewProj;

	float				m_ClippingDistance[BS_CLIP_TYPE_COUNT];
	/////////////////////////////////////////////////////////////////////////////////////////

	float				m_fFogStart;
	float				m_fFogEnd;
	D3DXVECTOR4			m_FogFactor;				// x=FogEnd, y=1/(FogEnd-FogStart)
	D3DXVECTOR4			m_FogColor;

	BOOL				m_bScreenClear;
	D3DCOLOR			m_ClearColor;

	std::vector< int >	m_DrawObjectList[ DOUBLE_BUFFERING ];
	std::vector< int >	m_ShadowObjectList[ DOUBLE_BUFFERING ];

	Frustum				m_Frustum;				// for Small Size
	Frustum				m_FrustumForBig[DOUBLE_BUFFERING];		// For Big Size(Prop,...)
	Frustum				m_FrustumForGiant[DOUBLE_BUFFERING];		// For Terrain,... => Double Buffer

	BSVECTOR			m_vPlane[ 8 ];			// MS clip
	BSVECTOR			m_vEye;

	void ReInitViewport();
public:
	void	AttachDevice(C3DDevice* pDevice)	{	m_pDevice = pDevice;	}
	void	Refresh();
	virtual void Release();

	void UpdateFrustum( /*float fNear, float fFar, float fFov, float fAspect*/ );
	void UpdateFrustumByRender();
	const Frustum& GetFrustum()			{	return m_Frustum;			}
	const Frustum& GetFrustumForBig()	{	return m_FrustumForBig[m_sCurrentProcessBuffer];	}
	const Frustum& GetFrustumForGiant()	{	return m_FrustumForGiant[m_sCurrentProcessBuffer];	}

	const Frustum& GetFrustumForBigByRender()	{	return m_FrustumForBig[m_sCurrentRenderBuffer];	}
	const Frustum& GetFrustumForGiantByRender()	{	return m_FrustumForGiant[m_sCurrentRenderBuffer];	}


	BOOL  IsVisibleObjectTest(CBsObject* pObject);

	void				AddDrawObject(int nObjectIndex)	{	m_DrawObjectList[ m_sCurrentProcessBuffer ].push_back(nObjectIndex);	}
	std::vector<int>&	GetDrawObjectList()				{	return m_DrawObjectList[ m_sCurrentRenderBuffer ];	}
	void				AddShadowObject(int nObjectIndex)	{	m_ShadowObjectList[ m_sCurrentProcessBuffer ].push_back(nObjectIndex);	}
	std::vector<int>&	GetShadowObjectList()				{	return m_ShadowObjectList[ m_sCurrentRenderBuffer ];	}

	void				ResetDrawObjectList();
	virtual void	Render(C3DDevice *pDevice);
	virtual int		ProcessMessage(DWORD dwCode, DWORD dwParam1=0, DWORD dwParam2=0, DWORD dwParam3=0);

	float GetfViewWidth() { return m_fViewWidth; } // 2D UI 용 입니다
	float GetfViewHeight() { return m_fViewHeight; }

	//**********************************************************************
	// Camera 관련 Interface
	//**********************************************************************
	void SetCameraViewport(const D3DVIEWPORT9* pViewport)	{	m_viewport = *pViewport;	}
	const D3DVIEWPORT9* GetCameraViewport()					{	return &m_viewport;	}

	void GetScreenPos( D3DXVECTOR3* pVecPos, D3DXVECTOR3* pRes);				// Output -> Screen space
	void GetProjectionPos( D3DXVECTOR3* pVecPos, D3DXVECTOR3* pRes);			// Output -> Projection space

	void SetProjectionMatrix(float fNear, float fFar, float fFovY);
	void SetProjectionMatrix(float fNear, float fFar);
	void SetProjectionMatrixByProcess(float fNear, float fFar, float fFovY);
	void SetProjectionMatrixByProcess(float fNear, float fFar);
	void SetOrthoProjectionMatrix(float w, float h, float zn, float zf);

	const D3DXMATRIX* GetProjectionMatrix()		{	return &m_matProj;	}
	void SetViewMatrix(D3DXMATRIX* pViewMatrix)	{	m_matView=*pViewMatrix;	}
	const D3DXMATRIX* GetViewMatrix()			{	return &m_matView;	}
	void SetViewProjMatrix()					{	m_matViewProj = m_matView * m_matProj;	}
	const D3DXMATRIX* GetViewProjMatrix()		{	return &m_matViewProj;	}

	void  SetFOVByProcess(float fFov);
	void  SetFOVByRender(float fFov)		{ m_fFov[m_sCurrentRenderBuffer] = fFov;}

	float GetNearZ()	{	return m_fNearZ;	}
	float GetFarZ()		{	return m_fFarZ;		}
	float GetFOVByProcess()		{	return m_fFov[m_sCurrentProcessBuffer]; }
	float GetFOVByRender()		{	return m_fFov[m_sCurrentRenderBuffer];	}
	float GetAspect(){ return m_fAspect; }

	float GetFogStart() { return m_fFogStart; }
	float GetFogEnd() { return m_fFogEnd; }
	const D3DXVECTOR4* GetFogFactor() { return &m_FogFactor; }
	const D3DXVECTOR4* GetFogColor() { return &m_FogColor; }

	void SetFogFactor(float fStart, float fEnd); // E 값을 중복 하여 사용 하였습니다
	void SetFogColor (D3DXVECTOR4* vec) { m_FogColor  = *vec; }	// x,y,z, ?

	void EnableScreenClear(BOOL bIsEnable)	{	m_bScreenClear = bIsEnable;	}
	void SetClearColor(int nR, int nG, int nB)	{	m_ClearColor = D3DCOLOR_ARGB( 0xff, nR, nG, nB);	}
	D3DCOLOR GetClearColor()					{	return m_ClearColor;	}

	void Reflect( D3DXPLANE *pPlane );		// 물반사에 쓰이는 반사용 View Matrix , Clip Plane 세팅

	int	 GetRenderTargetTexture()				{	return m_nRenderTargetTexture;	}
	void GetClipPlane( BSVECTOR* pPlane )		{	for( UINT i = 0; i < 8; i++ ){ pPlane[i] = m_vPlane[i]; }; }
	void GetClipEye( BSVECTOR* pEye )			{	*pEye	= m_vEye;	}

	void  SetClippingDistance(BS_CLIP_TYPE type, float fDistance)
	{
		BsAssert( type >= 0 && type < 3 && "Invalid Clipping Type");
		m_ClippingDistance[type] = fDistance;
	}
	float GetClippingDistance(BS_CLIP_TYPE type)
	{
		BsAssert( type >= 0 && type < 3 && "Invalid Clipping Type");
		return m_ClippingDistance[type];
	}
	
};

class CBsRTTCamera : public CBsCamera
{
public:
	CBsRTTCamera(int nRenderTarget, float fStartX, float fStartY, float fWidth, float fHeight, bool bIsRTT=false);
	virtual ~CBsRTTCamera();
protected:
	std::vector<int> m_RegisteredDrawObjectList;
public:
	void RegisterObject(int nEngineObject);
	void EliminateRegisteredObject(int nEngineObject);
	std::vector<int>& GetRegisteredDrawObjectList()	{	return m_RegisteredDrawObjectList;	}
};