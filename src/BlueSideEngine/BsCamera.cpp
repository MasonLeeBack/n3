#include "stdafx.h"
#include "BsCamera.h"
#include "BsKernel.h"

#include "Box3.h"
#include "IntBox3Frustum.h"
#include "CClipTest.h"

CBsCamera::CBsCamera(int nRenderTarget, float fStartX, float fStartY, float fWidth, float fHeight, bool bIsRTT/*=false*/)
{
	SetObjectType(BS_CAMERA_OBJECT);

	m_nRenderTargetTexture = nRenderTarget;
	m_bIsRTT = bIsRTT;
	m_fViewStartX = fStartX;
	m_fViewStartY = fStartY;
	m_fViewWidth = fWidth;
	m_fViewHeight= fHeight;

	ReInitViewport();

	m_fFov[0] = D3DX_PI * .25f;
	m_fFov[1] = D3DX_PI * .25f;

	m_fNearZ	= 10.f;
	m_fFarZ		= 5000.f;

	SetFogFactor(10000.f, 20000.f);

	m_FogColor = D3DXVECTOR4( 1.f, 1.f, 1.f, 1.f);

	m_bScreenClear = FALSE;
	m_ClearColor = D3DCOLOR_ARGB(0xff, 40, 40, 40);

	m_CameraType = PERSPECTIVE;

	m_DrawObjectList[ 0 ].reserve( DEFAULT_DRAW_LIST_SIZE );
	m_DrawObjectList[ 1 ].reserve( DEFAULT_DRAW_LIST_SIZE );

	memset( &m_Frustum, 0, sizeof( Frustum ) );
	memset( &m_FrustumForBig[0], 0, sizeof( Frustum ) );
	memset( &m_FrustumForBig[1], 0, sizeof( Frustum ) );
	memset( &m_FrustumForGiant[0], 0, sizeof( Frustum ) );
	memset( &m_FrustumForGiant[1], 0, sizeof( Frustum ) );

	memset( &m_vPlane, 0, sizeof( BSVECTOR )*8 );
	m_vEye	= BSVECTOR( 0.0f, 0.0f, 0.0f );

	SetClippingDistance(BS_CLIP_TYPE0, GetFogEnd()*0.5f);
	SetClippingDistance(BS_CLIP_TYPE1, GetFogEnd());
	SetClippingDistance(BS_CLIP_TYPE2, GetFarZ());
}

CBsCamera::~CBsCamera()
{	
}

void CBsCamera::Release()
{
	g_BsKernel.DeleteCameraIndex( m_nKernelPoolIndex );
}

void CBsCamera::GetScreenPos(D3DXVECTOR3* pVecPos, D3DXVECTOR3* pRes)
{
	// input data는 world space, output은 화면좌표계
	D3DXVec3Project(pRes, pVecPos, &m_viewport, &m_matViewProj, NULL, NULL);
}

void CBsCamera::GetProjectionPos(D3DXVECTOR3* pVecPos, D3DXVECTOR3* pRes)
{
	// input data는 world space, output은 Projection 공간
	D3DXVec3Project(pRes, pVecPos, NULL, &m_matViewProj, NULL, NULL);
}

void CBsCamera::SetProjectionMatrix(float fNear, float fFar)
{
	SetProjectionMatrix(fNear, fFar, GetFOVByRender());
}

void CBsCamera::SetProjectionMatrix(float fNear, float fFar, float fFovY)
{
	m_CameraType = PERSPECTIVE;
	m_fAspect=float(m_viewport.Width)/float(m_viewport.Height);
	m_fNearZ=fNear;
	m_fFarZ=fFar;
	SetClippingDistance(BS_CLIP_TYPE2, m_fFarZ);

	SetFOVByRender(fFovY);
	D3DXMatrixPerspectiveFovLH(&m_matProj, fFovY, m_fAspect, m_fNearZ, m_fFarZ);

#ifndef _XBOX
	m_pDevice->SetTransform(BS_TRANSFORM_PROJECTION, &m_matProj);
#endif

	m_fScaleWidth=m_matProj._11;
	m_fScaleHeight=m_matProj._22;
}

void CBsCamera::SetProjectionMatrixByProcess(float fNear, float fFar)
{
	SetProjectionMatrixByProcess(fNear, fFar, GetFOVByProcess());
}

void CBsCamera::SetProjectionMatrixByProcess(float fNear, float fFar, float fFovY)
{
	m_CameraType = PERSPECTIVE;
	m_fAspect=float(m_viewport.Width)/float(m_viewport.Height);
	m_fNearZ=fNear;
	m_fFarZ=fFar;
	SetClippingDistance(BS_CLIP_TYPE2, m_fFarZ);
	SetFOVByProcess(fFovY);
	D3DXMatrixPerspectiveFovLH(&m_matProj, fFovY, m_fAspect, m_fNearZ, m_fFarZ);

	m_fScaleWidth=m_matProj._11;
	m_fScaleHeight=m_matProj._22;
}

void CBsCamera::SetOrthoProjectionMatrix(float w, float h, float zn, float zf)
{
	m_CameraType = ORTHOGONAL;
	D3DXMatrixOrthoLH(&m_matProj, w, h, zn, zf);
	m_fOrthoWidth = w;
	m_fOrthoHeight = h;
	m_fNearZ = zn;
	m_fFarZ = zf;
	SetClippingDistance(BS_CLIP_TYPE2, m_fFarZ);

#ifndef _XBOX
	m_pDevice->SetTransform(BS_TRANSFORM_PROJECTION, &m_matProj);
#endif
}

void CBsCamera::SetFOVByProcess(float fFov)
{
	m_fFov[m_sCurrentProcessBuffer] = fFov;
#ifndef _BS_PERFORMANCE_CHECK
	// TODO : 이런 코드는 프로젝트 끝나면 Clean!! => Fov에 따른 빌보드 Range Control
	const float fBillboardMinimumFov = 0.3f;
	const float fBillboardMaximumFov = D3DX_PI*0.2f;
	const float fRcpLength = 1.f / (fBillboardMaximumFov - fBillboardMinimumFov);
	const float fBillboardMaxAddRange = 5000.f;

	fFov = BsMax(fFov, fBillboardMinimumFov);		// 100 m
	fFov = BsMin(fFov, fBillboardMaximumFov);		// 50  m
	float fRatio = fBillboardMaximumFov - fFov;
	fRatio = fRatio*fRcpLength;
	fRatio = fRatio*fBillboardMaxAddRange;
	g_BsKernel.SetBillboardRange(5000.f+fRatio);
#endif
}

void CBsCamera::SetFogFactor(float fStart, float fEnd) 
{ 
	m_fFogStart=fStart; 
	m_FogFactor.x=m_fFogEnd=fEnd;
	m_FogFactor.y=1.f/(fEnd-fStart);
	SetClippingDistance(BS_CLIP_TYPE0, m_fFogEnd*0.5f);
	SetClippingDistance(BS_CLIP_TYPE1, m_fFogEnd);
}

void CBsCamera::Render(C3DDevice *pDevice)
{
	D3DXMatrixInverse(&m_matView, NULL, GetObjectMatrix());
#ifndef _XBOX
	pDevice->SetTransform(BS_TRANSFORM_VIEW, &m_matView);
#endif

	if(m_CameraType==PERSPECTIVE) {
		SetProjectionMatrix(m_fNearZ, m_fFarZ);
	}
	else if(m_CameraType==ORTHOGONAL) {
		SetOrthoProjectionMatrix(m_fOrthoWidth, m_fOrthoHeight, m_fNearZ, m_fFarZ);
	}

	SetFogFactor(m_fFogStart, m_fFogEnd);

	SetViewProjMatrix();
}

void CBsCamera::ReInitViewport()
{
	float fScreenWidth, fScreenHeight;
	if(m_nRenderTargetTexture<0) {
		// 전체 Screen을 타겟으로 하는 Camera!!
		fScreenWidth = float(g_BsKernel.GetDevice()->GetBackBufferWidth());
		fScreenHeight= float(g_BsKernel.GetDevice()->GetBackBufferHeight());
	}
	else {
		// Offscreen
		SIZE size = g_BsKernel.GetTextureSize(m_nRenderTargetTexture);
		fScreenWidth = float(size.cx);
		fScreenHeight= float(size.cy);
	}

	m_viewport.X = int(fScreenWidth*m_fViewStartX);
	m_viewport.Y = int(fScreenHeight*m_fViewStartY);
	m_viewport.Width	= int(fScreenWidth*m_fViewWidth);
	m_viewport.Height	= int(fScreenHeight*m_fViewHeight);
#ifdef INV_Z_TRANSFORM
	m_viewport.MinZ = 1.f;
	m_viewport.MaxZ = 0.f;
#else
	m_viewport.MinZ = 0.f;
	m_viewport.MaxZ = 1.f;
#endif
}

void CBsCamera::Refresh()
{
	Render(g_BsKernel.GetDevice());
}

BOOL CBsCamera::IsVisibleObjectTest(CBsObject* pObject)
{
	Box3	B;
	if( pObject->GetBox3( B ) ) {
		B.compute_vertices();
		if( TestIntersection( B, m_Frustum ) == false )
		{
			return false;
		}
		// Visible Test와 분리할 필요가 있습니다 by jeremy (왜냐하면, 화면 분할시 문제가 생깁니다.)
		pObject->SetDistanceFromCam( BsVec3Length( &(B.C - m_Frustum.E) ) );
	}
	return true;
}

void CBsCamera::ResetDrawObjectList()
{
	// 이런 류의 애들은 clear() 로 지우면 메모리가 해제 된다.. 메모리 해제 안할려면 erase() 로 지워야 한다..
	m_DrawObjectList[ m_sCurrentRenderBuffer ].erase( m_DrawObjectList[ m_sCurrentRenderBuffer ].begin(), m_DrawObjectList[ m_sCurrentRenderBuffer ].end() );
	m_ShadowObjectList[ m_sCurrentRenderBuffer ].erase( m_ShadowObjectList[ m_sCurrentRenderBuffer ].begin(), m_ShadowObjectList[ m_sCurrentRenderBuffer ].end() );
}



void CBsCamera::UpdateFrustum( /*float fNear, float fFar, float fFov, float fAspect*/ )
{
	// 반드시 Process Thread에서 호출되야 합니다.
	D3DXMATRIX *pMatrix;

	pMatrix = GetObjectMatrixByProcess();
	// Part 1
	memcpy(&m_Frustum.L, &(pMatrix->_11), sizeof(BSVECTOR));
	memcpy(&m_Frustum.U, &(pMatrix->_21), sizeof(BSVECTOR));
	memcpy(&m_Frustum.D, &(pMatrix->_31), sizeof(BSVECTOR));
	memcpy(&m_Frustum.E, &(pMatrix->_41), sizeof(BSVECTOR));

	m_Frustum.n = m_fNearZ;
	m_Frustum.f = GetClippingDistance(BS_CLIP_TYPE0);

	m_Frustum.u = tanf(GetFOVByProcess()*0.5f) * m_Frustum.n;
	m_Frustum.l = m_Frustum.u * m_fAspect;

	m_Frustum.ratio = m_Frustum.f/m_Frustum.n;

	m_Frustum.compute_vertices();	// update 8-vertices
	m_Frustum.update();				// update ratio, C

	// Part 2
	memcpy(&m_FrustumForBig[m_sCurrentProcessBuffer].L, &(pMatrix->_11), sizeof(BSVECTOR));
	memcpy(&m_FrustumForBig[m_sCurrentProcessBuffer].U, &(pMatrix->_21), sizeof(BSVECTOR));
	memcpy(&m_FrustumForBig[m_sCurrentProcessBuffer].D, &(pMatrix->_31), sizeof(BSVECTOR));
	memcpy(&m_FrustumForBig[m_sCurrentProcessBuffer].E, &(pMatrix->_41), sizeof(BSVECTOR));

	m_FrustumForBig[m_sCurrentProcessBuffer].n = m_fNearZ;
	m_FrustumForBig[m_sCurrentProcessBuffer].f = GetClippingDistance(BS_CLIP_TYPE1);

	m_FrustumForBig[m_sCurrentProcessBuffer].u = tanf(GetFOVByProcess()*0.5f) * m_FrustumForBig[m_sCurrentProcessBuffer].n;
	m_FrustumForBig[m_sCurrentProcessBuffer].l = m_FrustumForBig[m_sCurrentProcessBuffer].u * m_fAspect;

	m_FrustumForBig[m_sCurrentProcessBuffer].ratio = m_FrustumForBig[m_sCurrentProcessBuffer].f/m_FrustumForBig[m_sCurrentProcessBuffer].n;

	m_FrustumForBig[m_sCurrentProcessBuffer].compute_vertices();	// update 8-vertices
	m_FrustumForBig[m_sCurrentProcessBuffer].update();				// update ratio, C

	// Part3
	memcpy(&m_FrustumForGiant[m_sCurrentProcessBuffer].L, &(pMatrix->_11), sizeof(BSVECTOR));
	memcpy(&m_FrustumForGiant[m_sCurrentProcessBuffer].U, &(pMatrix->_21), sizeof(BSVECTOR));
	memcpy(&m_FrustumForGiant[m_sCurrentProcessBuffer].D, &(pMatrix->_31), sizeof(BSVECTOR));
	memcpy(&m_FrustumForGiant[m_sCurrentProcessBuffer].E, &(pMatrix->_41), sizeof(BSVECTOR));

	m_FrustumForGiant[m_sCurrentProcessBuffer].n = m_fNearZ;
	m_FrustumForGiant[m_sCurrentProcessBuffer].f = GetClippingDistance(BS_CLIP_TYPE2);

	m_FrustumForGiant[m_sCurrentProcessBuffer].u = tanf(GetFOVByProcess()*0.5f) * m_FrustumForGiant[m_sCurrentProcessBuffer].n;
	m_FrustumForGiant[m_sCurrentProcessBuffer].l = m_FrustumForGiant[m_sCurrentProcessBuffer].u * m_fAspect;

	m_FrustumForGiant[m_sCurrentProcessBuffer].ratio = m_FrustumForGiant[m_sCurrentProcessBuffer].f/m_FrustumForGiant[m_sCurrentProcessBuffer].n;

	m_FrustumForGiant[m_sCurrentProcessBuffer].compute_vertices();	// update 8-vertices
	m_FrustumForGiant[m_sCurrentProcessBuffer].update();				// update ratio, C

	// MS
#ifdef	_XBOX
//	XMMATRIX		mV		= _XMMATRIX( m_matView );		// when only rendering, m_matView is valid value.
	XMMATRIX mV = XMMatrixInverse( &XMVectorZero(), _XMMATRIX( *GetObjectMatrixByProcess() ) );

	CClipTest::ComputeClipPlane( GetFOVByProcess(),
								m_fAspect,
								m_fNearZ,
								GetClippingDistance(BS_CLIP_TYPE0),
								GetClippingDistance(BS_CLIP_TYPE1),
								GetClippingDistance(BS_CLIP_TYPE2),
								mV,
								m_vPlane,
								&m_vEye );
#endif
}

void CBsCamera::UpdateFrustumByRender()
{
	// 반드시 Render Thread에서 호출되야 합니다.
	D3DXMATRIX *pMatrix;

	pMatrix = GetObjectMatrixByProcess();
	// Part 1
	memcpy(&m_Frustum.L, &(pMatrix->_11), sizeof(BSVECTOR));
	memcpy(&m_Frustum.U, &(pMatrix->_21), sizeof(BSVECTOR));
	memcpy(&m_Frustum.D, &(pMatrix->_31), sizeof(BSVECTOR));
	memcpy(&m_Frustum.E, &(pMatrix->_41), sizeof(BSVECTOR));

	m_Frustum.n = m_fNearZ;
	m_Frustum.f = GetClippingDistance(BS_CLIP_TYPE0);

	m_Frustum.u = tanf(GetFOVByProcess()*0.5f) * m_Frustum.n;
	m_Frustum.l = m_Frustum.u * m_fAspect;

	m_Frustum.ratio = m_Frustum.f/m_Frustum.n;

	m_Frustum.compute_vertices();	// update 8-vertices
	m_Frustum.update();				// update ratio, C

	// Part 2
	memcpy(&m_FrustumForBig[m_sCurrentRenderBuffer].L, &(pMatrix->_11), sizeof(BSVECTOR));
	memcpy(&m_FrustumForBig[m_sCurrentRenderBuffer].U, &(pMatrix->_21), sizeof(BSVECTOR));
	memcpy(&m_FrustumForBig[m_sCurrentRenderBuffer].D, &(pMatrix->_31), sizeof(BSVECTOR));
	memcpy(&m_FrustumForBig[m_sCurrentRenderBuffer].E, &(pMatrix->_41), sizeof(BSVECTOR));

	m_FrustumForBig[m_sCurrentRenderBuffer].n = m_fNearZ;
	m_FrustumForBig[m_sCurrentRenderBuffer].f = GetClippingDistance(BS_CLIP_TYPE1);

	m_FrustumForBig[m_sCurrentRenderBuffer].u = tanf(GetFOVByProcess()*0.5f) * m_FrustumForBig[m_sCurrentRenderBuffer].n;
	m_FrustumForBig[m_sCurrentRenderBuffer].l = m_FrustumForBig[m_sCurrentRenderBuffer].u * m_fAspect;

	m_FrustumForBig[m_sCurrentRenderBuffer].ratio = m_FrustumForBig[m_sCurrentRenderBuffer].f/m_FrustumForBig[m_sCurrentRenderBuffer].n;

	m_FrustumForBig[m_sCurrentRenderBuffer].compute_vertices();	// update 8-vertices
	m_FrustumForBig[m_sCurrentRenderBuffer].update();				// update ratio, C

	// Part3
	memcpy(&m_FrustumForGiant[m_sCurrentRenderBuffer].L, &(pMatrix->_11), sizeof(BSVECTOR));
	memcpy(&m_FrustumForGiant[m_sCurrentRenderBuffer].U, &(pMatrix->_21), sizeof(BSVECTOR));
	memcpy(&m_FrustumForGiant[m_sCurrentRenderBuffer].D, &(pMatrix->_31), sizeof(BSVECTOR));
	memcpy(&m_FrustumForGiant[m_sCurrentRenderBuffer].E, &(pMatrix->_41), sizeof(BSVECTOR));

	m_FrustumForGiant[m_sCurrentRenderBuffer].n = m_fNearZ;
	m_FrustumForGiant[m_sCurrentRenderBuffer].f = GetClippingDistance(BS_CLIP_TYPE2);

	m_FrustumForGiant[m_sCurrentRenderBuffer].u = tanf(GetFOVByRender()*0.5f) * m_FrustumForGiant[m_sCurrentRenderBuffer].n;
	m_FrustumForGiant[m_sCurrentRenderBuffer].l = m_FrustumForGiant[m_sCurrentRenderBuffer].u * m_fAspect;

	m_FrustumForGiant[m_sCurrentRenderBuffer].ratio = m_FrustumForGiant[m_sCurrentRenderBuffer].f/m_FrustumForGiant[m_sCurrentRenderBuffer].n;

	m_FrustumForGiant[m_sCurrentRenderBuffer].compute_vertices();	// update 8-vertices
	m_FrustumForGiant[m_sCurrentRenderBuffer].update();				// update ratio, C
}

int CBsCamera::ProcessMessage(DWORD dwCode, DWORD dwParam1/*=0*/, DWORD dwParam2/*=0*/, DWORD dwParam3/*=0*/)
{
	switch(dwCode){
		case BS_REFRESH_CAMERA:
			Refresh();
			return 1;
		case BS_REINIT_OBJECT:
			ReInitViewport();
			return 1;
	}

	return CBsObject::ProcessMessage(dwCode, dwParam1, dwParam2, dwParam3);
}

void CBsCamera::Reflect( D3DXPLANE *pPlane )
{
	// Render Thread에서 호출!!
	D3DXMATRIX matOldView;
	D3DXMatrixInverse(&matOldView, NULL, GetObjectMatrix());

	D3DXMATRIX matReflect;
	D3DXMatrixReflect(&matReflect, pPlane);
	D3DXMatrixMultiply(GetObjectMatrix(), GetObjectMatrix(), &matReflect);
	*GetObjectMatrixByProcess() = *GetObjectMatrix();	

	D3DXMatrixInverse(&m_matView,NULL,GetObjectMatrix());
#ifndef _XBOX
	g_BsKernel.GetD3DDevice()->SetTransform(BS_TRANSFORM_VIEW, &m_matView);
#endif
	// 상황봐서 고려하자.. by jeremy
	m_CameraType = PERSPECTIVE;
	m_fAspect=float(m_viewport.Width)/float(m_viewport.Height);	

	D3DXMatrixPerspectiveFovLH(&m_matProj, GetFOVByRender(), m_fAspect, m_fNearZ, m_fFarZ);
#ifndef _XBOX
	m_pDevice->SetTransform(D3DTS_PROJECTION, &m_matProj);
#endif

	m_fScaleWidth=m_matProj._11;
	m_fScaleHeight=m_matProj._22;

	SetFogFactor(m_fFogStart, m_fFogEnd);

	//////////////////////////////////////////////////////////////////////////	
	D3DXPLANE plane = *pPlane;
	static const float fBorderHeight = -30.f;
	plane.d += fBorderHeight;


	D3DXVECTOR3 v1(0, 0 , 0);	
	D3DXVECTOR3 v2(10000.f, 0, 0);
	D3DXVECTOR3 v3(0, 0 , 10000.f);

	v1.y = -(D3DXVec3Dot(&v1, (D3DXVECTOR3*)&plane) + plane.d ) / plane.b;
	v2.y = -(D3DXVec3Dot(&v2, (D3DXVECTOR3*)&plane) + plane.d ) / plane.b;
	v3.y = -(D3DXVec3Dot(&v3, (D3DXVECTOR3*)&plane) + plane.d ) / plane.b;

	D3DXMATRIX matProj, matViewProj;

	D3DXMatrixPerspectiveFovLH(&matProj, GetFOVByRender(), m_fAspect, m_fNearZ, m_fFarZ);

	D3DXMatrixMultiply(&matViewProj, &matOldView, &matProj);

	D3DXVec3TransformCoord(&v1, &v1, &matViewProj );
	D3DXVec3TransformCoord(&v2, &v2, &matViewProj );
	D3DXVec3TransformCoord(&v3, &v3, &matViewProj );

	D3DXPlaneFromPoints(&plane, &v1, &v2, &v3);

	if(plane.c < 0.f) {
		plane = -plane;
	}
	g_BsKernel.GetD3DDevice()->SetClipPlane(0, (float*)&plane);
	//////////////////////////////////////////////////////////////////////////
	
	SetViewProjMatrix();	
	UpdateFrustumByRender();
}

CBsRTTCamera::CBsRTTCamera(int nRenderTarget, float fStartX, float fStartY, float fWidth, float fHeight, bool bIsRTT/*=false*/)
: CBsCamera(nRenderTarget, fStartX, fStartY, fWidth, fHeight, bIsRTT)
{
	SetObjectType(BS_CAMERA_OBJECT);
}

void CBsRTTCamera::RegisterObject(int nEngineObject)
{
	m_RegisteredDrawObjectList.push_back(nEngineObject);
}

void CBsRTTCamera::EliminateRegisteredObject(int nEngineObject)
{
	int nCount = m_RegisteredDrawObjectList.size();
	for(int i=0;i<nCount;++i) {
		if(nEngineObject == m_RegisteredDrawObjectList[i]) {
			m_RegisteredDrawObjectList.erase(m_RegisteredDrawObjectList.begin()+i);
			return;
		}
	}
	BsAssert( 0 && "Invalid Registered Object Index!!" );
}


CBsRTTCamera::~CBsRTTCamera()
{
	SAFE_RELEASE_TEXTURE(m_nRenderTargetTexture);
}