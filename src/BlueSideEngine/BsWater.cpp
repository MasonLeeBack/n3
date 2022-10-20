#include "stdafx.h"
#include "BsKernel.h"
#include "BsObject.h"
#include "BsMesh.h"
#include "BsWorld.h"
#include "BsLODWorld.h"
#include "BsWater.h"
#include "BsInstancingMgr.h"
#include "IntBox3Frustum.h"


static const int cWaterTextureSize = 512;

CBsWater::CBsWater()
{
//	m_pDepthSurface = NULL;
	m_pReflectSurface = NULL;
	m_pReflectTexture = NULL;
	m_bReady = false;
	m_fWaterHeight = 0.f;	
	m_bWaterRendered = false;
}

CBsWater::~CBsWater()
{
	Clear();
}

void CBsWater::Clear()
{
	m_fWaterHeight = 0.f;

	m_BoxList.clear();
	SAFE_DELETE_ARRAY_PVEC( m_WaterCheckBufferList );

	//SAFE_RELEASE( m_pDepthSurface );
	SAFE_RELEASE( m_pReflectSurface );
	SAFE_RELEASE( m_pReflectTexture );
	m_bReady = false;
}

void CBsWater::AddWaterBoundingBox( AABB *pBox, BYTE *pWaterCheckBuffer )
{
	// 여러번 호출시 합치기...
	D3DXVECTOR3	Vmin, Vmax;
	Box3 Box;

	BSVECTOR	vExt = pBox->GetExtent();
	Box.E[0] = vExt.x;
	Box.E[1] = vExt.y+1.f;
	Box.E[2] = vExt.z;

	Box.C = pBox->GetCenter();

	Box.A[0] = BSVECTOR(1,0,0);
	Box.A[1] = BSVECTOR(0,1,0);
	Box.A[2] = BSVECTOR(0,0,1);

	Box.compute_vertices();
	
	m_fWaterHeight = Box.C.y;

	m_BoxList.push_back( Box );

	m_WaterCheckBufferList.push_back( pWaterCheckBuffer );
}

float CBsWater::GetWaterDistance( D3DXVECTOR3 Pos )
{
	const float fVeryLongDistance = 1000000.f;

	if( m_BoxList.empty() ) return fVeryLongDistance;

	int i, nSize;

	nSize = m_BoxList.size();

	float fLength = fVeryLongDistance;

	for( i = 0; i < nSize; i++) {
		Box3 Box = m_BoxList[i];
		if(Box.C.x - Box.E[0] < Pos.x && Box.C.x + Box.E[0] > Pos.x &&	
			Box.C.z - Box.E[2] < Pos.z && Box.C.z + Box.E[2] > Pos.z ) 
		{
			int nGridSize = m_WaterCheckBufferList[i][ 0 ];

			float fStartX = Box.C.x - Box.E[0];
			float fStartZ = Box.C.z - Box.E[2];
			float fSizeX = Box.E[0] * 2;
			float fSizeZ = Box.E[2] * 2;
			int nX = (int)(nGridSize * (Pos.x - fStartX) / fSizeX);
			int nY = (int)(nGridSize * (Pos.z - fStartZ) / fSizeZ);
			nX = BsMax(BsMin(nX, nGridSize-1), 0);
			nY = BsMax(BsMin(nY, nGridSize-1), 0);

			int nMinDist = -1;
			int nFindX, nFindY;

			for( int n = 0; n < nGridSize; n++)
			for( int m = 0; m < nGridSize; m++)
			{
				if( m_WaterCheckBufferList[i][ m + n * nGridSize + 1 ] != 0 )
				{
                    int x = (m - nX);
					int y = (n - nY);
					int nDist = x * x + y * y;
					if( nMinDist < 0 || nDist < nMinDist ) {
                        nFindX = m;
						nFindY = n;
						nMinDist = nDist;
					}
				}
			}

			if( nMinDist == -1) continue;

			float fDistX = (fStartX + ( ( nFindX + 0.5f ) / nGridSize ) * fSizeX) - Pos.x;
			float fDistZ = (fStartZ + ( ( nFindY + 0.5f ) / nGridSize ) * fSizeZ) - Pos.z;

            float fCurrLength = sqrtf( fDistX * fDistX + fDistZ * fDistZ );
			if( fCurrLength < fLength ) {
				fLength = fCurrLength;
			}
		}
	}

	return fLength;
}

bool CBsWater::IsWaterInside( D3DXVECTOR3 Pos )
{
    if( m_BoxList.empty() ) return false;

	int i, nSize;

	nSize = m_BoxList.size();

	for( i = 0; i < nSize; i++) {
		Box3 Box = m_BoxList[i];
		if(Box.C.x - Box.E[0] < Pos.x && Box.C.x + Box.E[0] > Pos.x &&
												 Box.C.y + Box.E[1] > Pos.y &&
			Box.C.z - Box.E[2] < Pos.z && Box.C.z + Box.E[2] > Pos.z ) 
		{
			int nGridSize = m_WaterCheckBufferList[i][ 0 ];
			int nX = (int)(nGridSize * (Pos.x - (Box.C.x - Box.E[0])) / (Box.E[0]*2));
			int nY = (int)(nGridSize * (Pos.z - (Box.C.z - Box.E[2])) / (Box.E[2]*2));
			nX = BsMax(BsMin(nX, nGridSize-1), 0);
			nY = BsMax(BsMin(nY, nGridSize-1), 0);

			if( m_WaterCheckBufferList[i][ nX + nY * nGridSize + 1 ] == 0 ) {
				continue;
			}
			return true;
		}
	}
	return false;
}

bool CBsWater::IsVisible()
{
	CBsCamera  *pActiveCamera = g_BsKernel.GetActiveCamera();
	const Frustum	& frustum = pActiveCamera->GetFrustumForBigByRender();	// 불필요하게 보여진다고 판단하는경우가 많아서 Giant -> Big 으로 합니다.

	bool bVisible = false;
	int i, nSize;
    nSize = m_BoxList.size();

	float fMinDist = FLT_MAX;
	
	for( i = 0; i < nSize; i++ ) {		

		if( TestIntersection( m_BoxList[i], frustum ) ) {

			float fDistX = FLT_MAX;
			float fDistZ = FLT_MAX;

			fDistX = BsMin(fDistX, fabs((m_BoxList[i].C.x - m_BoxList[i].E[0]) - pActiveCamera->GetObjectMatrix()->_41)  );
			fDistX = BsMin(fDistX, fabs((m_BoxList[i].C.x + m_BoxList[i].E[0]) - pActiveCamera->GetObjectMatrix()->_41)  );

			fDistZ = BsMin(fDistZ, fabs((m_BoxList[i].C.z - m_BoxList[i].E[2]) - pActiveCamera->GetObjectMatrix()->_43)  );
			fDistZ = BsMin(fDistZ, fabs((m_BoxList[i].C.z + m_BoxList[i].E[2]) - pActiveCamera->GetObjectMatrix()->_43)  );

			float fDistance = fDistX * fDistX + fDistZ * fDistZ;
			bVisible = true;

			if( fMinDist > fDistance) {
				m_fWaterHeight = m_BoxList[i].C.y;
				fMinDist = fDistance;
			}
			// Water가 없는맵에서는 굳이 RenderTarget Texture 생성할 필요 없으므로
			// Visible 할때 최초로 한번 생성한다.
			if(!m_bReady) {
				MakeReadyForUse();
			}
		}
	}

	
	return bVisible;
}

bool CBsWater::IsWaterRendered()
{
	bool bResult = m_bWaterRendered;
	m_bWaterRendered = false;
	return bResult;
}

void CBsWater::MakeReadyForUse()
{
	LPDIRECT3DDEVICE9 pD3DDevice = g_BsKernel.GetD3DDevice();

	//  use R32F & shaders instead of depth textures

	if(FAILED(pD3DDevice->CreateTexture(cWaterTextureSize, cWaterTextureSize, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, 
		D3DPOOL_DEFAULT, &m_pReflectTexture, NULL))) {
			assert(0);
		}

		// Retrieve top-level surfaces of our shadow buffer (need these for use with SetRenderTarget)
#ifdef _XBOX
	D3DSURFACE_PARAMETERS TileSurfaceParams;
	memset(&TileSurfaceParams, 0, sizeof(D3DSURFACE_PARAMETERS));
	TileSurfaceParams.Base = 0;
	pD3DDevice->CreateRenderTarget( cWaterTextureSize, cWaterTextureSize, D3DFMT_A8R8G8B8,
	D3DMULTISAMPLE_NONE, 0, FALSE, &m_pReflectSurface, &TileSurfaceParams );
#else
	if(FAILED(m_pReflectTexture->GetSurfaceLevel(0, &m_pReflectSurface))) {
		assert(0);
	}

	if(!m_pReflectTexture || !m_pReflectSurface)
	{
		assert(0);
	}

#endif

	m_waterViewport.X = 0;
	m_waterViewport.Y = 0;

	m_waterViewport.Width  = cWaterTextureSize;
	m_waterViewport.Height = cWaterTextureSize;
#ifdef INV_Z_TRANSFORM
	m_waterViewport.MinZ = 1.f;
	m_waterViewport.MaxZ = 0.f;
#else
	m_waterViewport.MinZ = 0.f;
	m_waterViewport.MaxZ = 1.f;
#endif

	m_bReady = true;
}

void CBsWater::Render( C3DDevice *pDevice ,std::vector< int >	*pRenderUpdateList )
{	
	if( !IsVisible() ) return;

	m_bWaterRendered = true;

	CBsCamera *pOldActiveCamera = g_BsKernel.GetActiveCamera();
		
	CBsCamera reflectCamera(-1, 0, 0, 1, 1);

	reflectCamera.AttachDevice( g_BsKernel.GetDevice() );
	reflectCamera.SetObjectMatrixByRender( pOldActiveCamera->GetObjectMatrix() );
	reflectCamera.SetProjectionMatrix(pOldActiveCamera->GetNearZ(), pOldActiveCamera->GetFarZ(), pOldActiveCamera->GetFOVByRender());
	reflectCamera.SetFogFactor( pOldActiveCamera->GetFogStart(), pOldActiveCamera->GetFogEnd() );
	reflectCamera.SetFogColor( (D3DXVECTOR4*)pOldActiveCamera->GetFogColor() );

	D3DXPLANE planeWater(0, 1, 0, -m_fWaterHeight);
	reflectCamera.Reflect( &planeWater );

	g_BsKernel.SetActiveCamera( &reflectCamera );

	D3DVIEWPORT9 viewportOrigin;
	{
		//  preserve old viewport
		pDevice->GetViewport(&viewportOrigin);		
		pDevice->SetViewport(&m_waterViewport);
		pDevice->SetRenderTarget(0, m_pReflectSurface);		
		pDevice->SetDepthStencilSurface( pDevice->GetDepthStencilRTTSurface() ); 
		pDevice->ClearBuffer(D3DCOLOR_XRGB(192,192,255), 1, 0);
	}

	CBsWorld *pWorld = g_BsKernel.GetWorld();
	if(pWorld) {	
#ifdef INV_Z_TRANSFORM
		pDevice->SetRenderState( D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
#else
		pDevice->SetRenderState( D3DRS_ZFUNC, D3DCMP_GREATEREQUAL);
#endif
		pDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
		pDevice->SetRenderState( D3DRS_CLIPPLANEENABLE, D3DCLIPPLANE0);
		pDevice->BeginScene();
		pDevice->SetRenderState( D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE );
		g_BsKernel.RenderFirstBlock();
		pDevice->SetRenderState( D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE | D3DCOLORWRITEENABLE_ALPHA);
		//((CBsLODWorld*)pWorld)->ForceLODLevel( CBsLODWorld::LEVEL_0 );
		pWorld->Render( g_BsKernel.GetDevice());
		//((CBsLODWorld*)pWorld)->ForceLODLevel( -1 );

		int i, nSize;
		
		nSize = pRenderUpdateList->size();

		const Frustum	& frustum = reflectCamera.GetFrustum();

		g_BsKernel.GetInstancingMgr()->SetCurrentUsingBufferIndex( CBsInstancingMgr::INSTANCING_REFLECT );

		int nDrawCount = 0;
		for( i = 0; i < nSize; i++ )
		{
			int nObj = ( *pRenderUpdateList )[ i ];
			CBsObject *pObject = g_BsKernel.GetEngineObjectPtr(nObj);
			if( pObject &&
				pObject->IsRootObject() &&
				pObject->IsEnableObjectCull() &&
				pObject->GetObjectType() == CBsObject::BS_STATIC_OBJECT &&	
				pObject->GetMeshRadius() > 50.f  )
			{
				Box3 B;
				if( pObject->GetBox3( B ) ) {
					B.compute_vertices();
					if( TestIntersection( B, frustum ) ) {						
						pObject->PreRender(pDevice);	
						pDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
						pObject->Render(pDevice);
						pObject->PostRender(pDevice);			
						nDrawCount++;
					}
				}
			}
		}
		// Render함수만 호출하고, Transparent Object 목록 Reset!
		g_BsKernel.ResetTransparentObjectList();	

		pDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
		g_BsKernel.GetInstancingMgr()->RenderInstancingObjectList( pDevice );

		g_BsKernel.GetInstancingMgr()->SetCurrentUsingBufferIndex( CBsInstancingMgr::INSTANCING_NONE );

		/**/
		pDevice->EndScene();

		g_BsKernel.GetD3DDevice()->SetRenderState( D3DRS_CLIPPLANEENABLE, FALSE);
		pDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
#ifdef INV_Z_TRANSFORM
		pDevice->SetRenderState( D3DRS_ZFUNC, D3DCMP_GREATEREQUAL);
#else
		pDevice->SetRenderState( D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
#endif
	}

#ifdef _XBOX
	D3DVECTOR4 clearColor;
	clearColor.x = 0.f;
	clearColor.y = 0.f;
	clearColor.z = 0.f;
	clearColor.w = 0.f;
	pDevice->Resolve(D3DRESOLVE_RENDERTARGET0 | D3DRESOLVE_CLEARDEPTHSTENCIL  | D3DRESOLVE_CLEARRENDERTARGET , NULL, m_pReflectTexture, NULL, 0, 0, &clearColor, 0.0f, 0, NULL );
	//pDevice->ClearBuffer(D3DCOLOR_XRGB(0,0,0));
#endif
	{		
		//set render target back to normal back buffer / depth buffer

		pDevice->SetRenderTarget(0, g_BsKernel.GetDevice()->GetBackBuffer());			
		pDevice->SetDepthStencilSurface(g_BsKernel.GetDevice()->GetDepthStencilSurface());		
		pDevice->SetViewport(&viewportOrigin);	
	}

	g_BsKernel.SetActiveCamera( pOldActiveCamera );
}

