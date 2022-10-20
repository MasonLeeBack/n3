#include "stdafx.h"
#include "FcFXLensFlare.h"
#include "BsKernel.h"
#include "BsMaterial.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG


//-----------------------------------------------------------------------------
// Name: CFcFXLensFlare()
// Desc: Constructor
//-----------------------------------------------------------------------------
CFcFXLensFlare::CFcFXLensFlare()
: m_pFlareElements( NULL ),
m_uiNumFlareElements( 0 ),
m_pLightElement( NULL ),
m_nBackBufferCopy( -1 ),
m_uiNumFullLightPixels( 0 ),
m_bPreviouslyFullyVisible( false ),
m_pQuadVB( NULL )
{
	//--------------------------------------------
	// Initialize flare light and elements
	//--------------------------------------------
	g_BsKernel.chdir("FX");

	SLightElement* pLightElement = new SLightElement;

	pLightElement->BaseSize = D3DXVECTOR2( 0.1f, 0.1f );
	// Flare의 강조 조절	/////////////////////////////////
	pLightElement->FlareOutSize = D3DXVECTOR2( 1.f, 1.f );
	pLightElement->uiNumFlareOuts = 20;
	/////////////////////////////////////////////////////////
	pLightElement->nTexture = g_BsKernel.LoadTexture("sun.dds");

	pLightElement->Color = D3DXVECTOR4( 247.0f/255.0f, 247.0f/255.0f, 230.0f / 255.0f, 0.06f );
	pLightElement->ViewCenter = D3DXVECTOR2( 0, 0 );

	SFlareElement* pFlareElements = new SFlareElement[9];

	pFlareElements[0].Size = D3DXVECTOR2( 0.15f, 0.15f );
	pFlareElements[0].fDistance = 0.75f;
	pFlareElements[0].nTexture = g_BsKernel.LoadTexture("flare1.dds");
	pFlareElements[0].Color = D3DXVECTOR4( 0.023f, 0.63f, 0.118f, 1.f );

	pFlareElements[1].Size = D3DXVECTOR2( 0.3f, 0.3f );
	pFlareElements[1].fDistance = 0.51f;
	pFlareElements[1].nTexture = g_BsKernel.LoadTexture("flare2.dds");
	pFlareElements[1].Color = D3DXVECTOR4( 0.023f, 0.63f, 0.118f, 1.f );

	pFlareElements[2].Size = D3DXVECTOR2( 0.15f, 0.15f );
	pFlareElements[2].fDistance = 0.42f;
	pFlareElements[2].nTexture = g_BsKernel.LoadTexture("flare3.dds");
	pFlareElements[2].Color = D3DXVECTOR4( 0.32f, 0.157f, 0.04f, 1.f );

	pFlareElements[3].Size = D3DXVECTOR2( 0.2f, 0.2f );
	pFlareElements[3].fDistance = 0.33f;
	pFlareElements[3].nTexture = g_BsKernel.LoadTexture("flare1.dds");
	pFlareElements[3].Color = D3DXVECTOR4( 0.32f, 0.157f, 0.04f, 1.f );

	pFlareElements[4].Size = D3DXVECTOR2( 0.25f, 0.25f );
	pFlareElements[4].fDistance = 0.11f;
	pFlareElements[4].nTexture = g_BsKernel.LoadTexture("flare2.dds");
	pFlareElements[4].Color = D3DXVECTOR4( 0.32f, 0.157f, 0.04f, 1.f );

	pFlareElements[5].Size = D3DXVECTOR2( 0.3f, 0.3f );
	pFlareElements[5].fDistance = -0.22f;
	pFlareElements[5].nTexture = g_BsKernel.LoadTexture("flare3.dds");
	pFlareElements[5].Color = D3DXVECTOR4( 0.32f, 0.157f, 0.04f, 1.f );

	pFlareElements[6].Size = D3DXVECTOR2( 0.15f, 0.15f );
	pFlareElements[6].fDistance = -0.45f;
	pFlareElements[6].nTexture = g_BsKernel.LoadTexture("flare4.dds");
	pFlareElements[6].Color = D3DXVECTOR4( 0.16f, 0.24f, 0.04f, 1.f );

	pFlareElements[7].Size = D3DXVECTOR2( 0.4f, 0.4f );
	pFlareElements[7].fDistance = -0.6f;
	pFlareElements[7].nTexture = g_BsKernel.LoadTexture("flare2.dds");
	pFlareElements[7].Color = D3DXVECTOR4( 0.16f, 0.24f, 0.04f, 1.f );

	pFlareElements[8].Size = D3DXVECTOR2( 0.2f, 0.2f );
	pFlareElements[8].fDistance = -0.8f;
	pFlareElements[8].nTexture = g_BsKernel.LoadTexture("flare3.dds");
	pFlareElements[8].Color = D3DXVECTOR4( 0.16f, 0.24f, 0.04f, 1.f );

	g_BsKernel.chdir("..");


	D3DVERTEXELEMENT9	LensFlareDecl[] = {
		{ 0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
		{ 0, 12, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
		D3DDECL_END()
	};

	m_nVertexDeclIdx = g_BsKernel.LoadVertexDeclaration(LensFlareDecl);

	char fullName[_MAX_PATH];
	strcpy(fullName, g_BsKernel.GetInstance().GetShaderDirectory());
	strcat(fullName, "lensflare.fx");
	m_nMaterialIndex = g_BsKernel.GetInstance().LoadMaterial(fullName, FALSE);

	CBsMaterial* pMaterial = g_BsKernel.GetInstance().GetMaterialPtr(m_nMaterialIndex);

#ifdef _XBOX
	m_hMatWVP = pMaterial->GetParameterByName("WorldViewProjection");
	m_hTexture = pMaterial->GetParameterByName("InputSampler");
	m_hLightColor = pMaterial->GetParameterByName("Diffuse");
	m_hTexCoordOffset = pMaterial->GetParameterByName("f4SrcRect");

#else
	m_hMatWVP = pMaterial->GetParameterByName("WorldViewProj");
	m_hTexture = pMaterial->GetParameterByName("InputTexture");
	m_hLightColor = pMaterial->GetParameterByName("LightColor");
	m_hTexCoordOffset = pMaterial->GetParameterByName("f4SrcRect");
#endif

	Create( pLightElement, pFlareElements, 9, D3DXVECTOR2( 0.3f, 0.3f ) );

#ifndef _LTCG
	SetFxRtti(FX_TYPE_LENSFLARE);
#endif //_LTCG
}

//-----------------------------------------------------------------------------
// Name: ~CFcFXLensFlare()
// Desc: Destructor
//-----------------------------------------------------------------------------
CFcFXLensFlare::~CFcFXLensFlare()
{
	BsAssert(m_pLightElement);

	SAFE_RELEASE_TEXTURE(m_pLightElement->nTexture);
	SAFE_DELETE(m_pLightElement);

	// Free flare elements
	for( UINT i = 0; i < m_uiNumFlareElements; i++ ) {
		SAFE_RELEASE_TEXTURE(m_pFlareElements[i].nTexture);
	}

	SAFE_DELETEA(m_pFlareElements);

	// Free VB
	SAFE_RELEASE(m_pQuadVB);

	// Free back buffer copy
	SAFE_RELEASE_TEXTURE(m_nBackBufferCopy);

	SAFE_RELEASE_VD(m_nVertexDeclIdx);

	SAFE_RELEASE_MATERIAL(m_nMaterialIndex);
}


//-----------------------------------------------------------------------------
// Name: Create()
// Desc: Initialized the flare.  Takes ownership of the light element and flare
//       elements array
//-----------------------------------------------------------------------------
HRESULT CFcFXLensFlare::Create( SLightElement* pLightElement, 
								 SFlareElement* pFlareElements, UINT uiNumFlareElements,
								 const D3DXVECTOR2& MaxElementSize )
{
	C3DDevice* pDevice=g_BsKernel.GetDevice();
	// Light element
	m_pLightElement = pLightElement;

	// Flare elements
	m_pFlareElements =  pFlareElements;
	m_uiNumFlareElements =  uiNumFlareElements;

	// Max element size
	m_MaxElementSize = MaxElementSize;

	// Never been rendered
	m_bPreviouslyFullyVisible = false;

	//-----------------------------------------------
	// Initialize a texture to receive the portion of
	// the pack buffer that the light is rendered to
	//-----------------------------------------------

	// Get back buffer
	D3DSURFACE_DESC SrcDesc;
	IDirect3DSurface9* pSrcSurface;
	pSrcSurface=(IDirect3DSurface9*)pDevice->GetBackBuffer();
	pSrcSurface->GetDesc( &SrcDesc );

	// Initialize skybox projection
	D3DXMATRIX SkyBoxProjection;
	D3DXMatrixOrthoLH( &SkyBoxProjection, 2.0f, 2.0f * FLOAT(SrcDesc.Height)/FLOAT(SrcDesc.Width), 0.0f, 1.0f );

	// Get texture format and size in screen space
	D3DXVECTOR3 BaseSize( m_pLightElement->BaseSize.x, m_pLightElement->BaseSize.y, 0.0f );
	D3DXVec3TransformCoord( &BaseSize, &BaseSize, &SkyBoxProjection);
	UINT uiWidth =  UINT( SrcDesc.Width/2.0f  * BaseSize.x );  // Pixel width in screen space
	UINT uiHeight = UINT( SrcDesc.Height/2.0f * BaseSize.y );  // Pixel height in screen space
	D3DFORMAT Format = SrcDesc.Format;                         // Prefer swizzled format here

	// Create texture
	// TODO : 위험한 코드 => Xbox1에서는 64가 나왔는데.. 지금 계산으로는 63이 나온다.. 하드코딩으로 넣었습니다.. by jeremy
	m_nBackBufferCopy = g_BsKernel.CreateTexture(SrcDesc.Width, SrcDesc.Height, D3DUSAGE_RENDERTARGET, Format);

	//---------------------------------------
	// create flare VB
	//---------------------------------------
	UINT uiNumVerts = 8;
	SVert* pVerts;
	g_BsKernel.CreateVertexBuffer( uiNumVerts * sizeof(SVert), 0, 0, D3DPOOL_DEFAULT, &m_pQuadVB );

	m_pQuadVB->Lock( 0, 0, (void**)&pVerts, 0 );
	// Verts for swizzled texture
	pVerts[2].Pos = D3DXVECTOR3( -0.5f, -0.5f, 1 );
	pVerts[0].Pos = D3DXVECTOR3( -0.5f,  0.5f, 1 );
	pVerts[1].Pos = D3DXVECTOR3(  0.5f,  0.5f, 1 );
	pVerts[3].Pos = D3DXVECTOR3(  0.5f, -0.5f, 1 );

	pVerts[2].UV = D3DXVECTOR2( 0, 1 );
	pVerts[0].UV = D3DXVECTOR2( 0, 0 );
	pVerts[1].UV = D3DXVECTOR2( 1, 0 );
	pVerts[3].UV = D3DXVECTOR2( 1, 1 );

	// Verts for linear texture
	pVerts[6].Pos = D3DXVECTOR3(-0.5f, -0.5f, 1 );
	pVerts[4].Pos = D3DXVECTOR3(-0.5f,  0.5f, 1 );
	pVerts[5].Pos = D3DXVECTOR3( 0.5f,  0.5f, 1 );
	pVerts[7].Pos = D3DXVECTOR3( 0.5f, -0.5f, 1 );

	pVerts[6].UV = D3DXVECTOR2( 0, FLOAT(uiWidth) );
	pVerts[4].UV = D3DXVECTOR2( 0, 0 );
	pVerts[5].UV = D3DXVECTOR2( FLOAT( uiHeight ), 0 );
	pVerts[7].UV = D3DXVECTOR2( FLOAT( uiHeight ), FLOAT( uiWidth ) );

	m_pQuadVB->Unlock();

	// 이부분은 하드 코딩으로 계산된값을 m_uiNumFlareElements값에 넣어도 됩니다. by jeremy
//	SetNumFullLightPixels();		// FSAA때문에 가동시켰다. 
	m_uiNumFullLightPixels = 4096;	// 64x64로 4096이 나왔다.
	m_state = PLAY;
	m_DeviceDataState = FXDS_INITREADY;
	return S_OK;
}




//-----------------------------------------------------------------------------
// Name: SetNumFullLightPixels()
// Desc: Determines the number of pixels rendered for a fully visible light
//-----------------------------------------------------------------------------
HRESULT CFcFXLensFlare::SetNumFullLightPixels()
{
	BsAssert( 0 ); //오면 안되요
	g_BsKernel.GetDevice()->SaveState();
	g_BsKernel.GetDevice()->BeginScene();
	D3DXMATRIX MatW, MatV, MatProj;

	// Get back buffer desc
	D3DSURFACE_DESC SrcDesc;
	IDirect3DSurface9* pSrcSurface;
	C3DDevice* pDevice=g_BsKernel.GetDevice();

	pSrcSurface=(IDirect3DSurface9*)pDevice->GetBackBuffer();
	pSrcSurface->GetDesc( &SrcDesc );

	// Set world view projection matrices
	D3DXVECTOR3 Position( 0, 0, 0 );
	D3DXVECTOR3 Scale( m_pLightElement->BaseSize.x, m_pLightElement->BaseSize.y, 1.0f );
	D3DXMatrixTransformation( &MatW, NULL, NULL, &Scale, NULL, NULL, &Position );

	// Initialize projection matrix
	D3DXMatrixOrthoLH( &MatProj, 2.0f, 2.0f * FLOAT(SrcDesc.Height)/FLOAT(SrcDesc.Width), 0.0f, 1.0f );
	
	CBsMaterial* pMaterial = g_BsKernel.GetInstance().GetMaterialPtr(m_nMaterialIndex);

	D3DXMATRIX matWVP;
	matWVP = MatW * MatProj;
	
	g_BsKernel.SetVertexDeclaration(m_nVertexDeclIdx);
	pMaterial->BeginMaterial(0, 0);
	pMaterial->SetMatrix(m_hMatWVP, &matWVP);
	//------------------------------------------
	// Set render state
	//------------------------------------------
	pDevice->SetRenderState( D3DRS_CULLMODE,         D3DCULL_CCW );
	pDevice->SetRenderState( D3DRS_ALPHATESTENABLE,  FALSE );

	// Not using Z since we are looking for the number of pixels when the light is fully visible
	pDevice->SetRenderState( D3DRS_ZENABLE,          FALSE );

	pDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
	pDevice->SetRenderState( D3DRS_SRCBLEND,         D3DBLEND_SRCALPHA );
	pDevice->SetRenderState( D3DRS_DESTBLEND,        D3DBLEND_INVSRCALPHA );

	// Output color is the light color
	// Output alpha is the texture alpha
	D3DXVECTOR4 vClearColor ( 0.f, 0.f, 0.f, 1.f);
	pMaterial->SetTexture(m_hTexture, m_pLightElement->nTexture);
	pMaterial->SetVector(m_hLightColor, &vClearColor);

	D3DXVECTOR4 vVec (0.f, 0.f, 1.f, 1.f);
	pMaterial->SetVector(m_hTexCoordOffset, &vVec);

	pMaterial->BeginPass(0);
	// Visibility test
	IDirect3DQuery9* pQuery;
	pMaterial->CommitChanges();

	pDevice->GetD3DDevice()->CreateQuery(D3DQUERYTYPE_OCCLUSION, &pQuery);
	pQuery->Issue(D3DISSUE_BEGIN);

	pDevice->DrawMeshVB(D3DPT_TRIANGLESTRIP, 2, m_pQuadVB,sizeof(SVert));

	pQuery->Issue(D3DISSUE_END);
	while(S_FALSE == pQuery->GetData( &m_uiNumFullLightPixels , sizeof(DWORD), D3DGETDATA_FLUSH )) { ; }
	pQuery->Release();

	g_BsKernel.GetDevice()->EndScene();
	g_BsKernel.GetDevice()->RestoreState();
	pMaterial->EndPass();
	pMaterial->EndMaterial();
	return S_OK;
}


//-----------------------------------------------------------------------------
// Name: RasterPos()
// Desc: determines where a pixel will be rendered
//-----------------------------------------------------------------------------
int CFcFXLensFlare::RasterPos( FLOAT fNum )
{
	DOUBLE dFraction, dIntegerPortion;
	dFraction = modf( fNum, &dIntegerPortion );
	if( dFraction < 0.5 + 1.0 / 32.0 )
		return int( dIntegerPortion );
	else
		return int( dIntegerPortion + 1 );
};



void CFcFXLensFlare::SetLightIntensity(float fPercent)
{
	//	m_pLightElement->FlareOutSize*=fPercent;
	m_pLightElement->FlareOutSize.x= 2.2f+fPercent*0.6f;
	m_pLightElement->FlareOutSize.y= 2.2f+fPercent*0.6f;
	m_pLightElement->uiNumFlareOuts=UINT(fPercent*50.f);
}


//-----------------------------------------------------------------------------
// Name: Render()
// Desc: renders the lens flare
//-----------------------------------------------------------------------------
void CFcFXLensFlare::Render(C3DDevice* pDevice)
{
#ifndef _XBOX
	return;
#endif
	// Get back buffer and desc
	D3DSURFACE_DESC SrcDesc;
	IDirect3DSurface9* pSrcSurface;

	pDevice->SaveState();

	D3DXMATRIX	matView;
	matView = *(g_BsKernel.GetParamViewMatrix());

	// BlockUtilIdle() 쓸지 아직 결정 안됨(Test 필요)
	pSrcSurface=(IDirect3DSurface9*)pDevice->GetBackBuffer();
	pSrcSurface->GetDesc( &SrcDesc );

	// Get src light texture and desc
	D3DSURFACE_DESC DstDesc;
	IDirect3DSurface9* pDstSurface;
	LPDIRECT3DTEXTURE9 pTexture = (LPDIRECT3DTEXTURE9)g_BsKernel.GetTexturePtr(m_nBackBufferCopy);
	BsAssert(pTexture);

	pTexture->GetSurfaceLevel( 0, &pDstSurface );
	pDstSurface->GetDesc( &DstDesc );

	//--------------------------------------
	// Get light rect in screen space
	//--------------------------------------

	// Get view space light direction
	D3DXVECTOR3 ViewLightDirection;
	D3DXVec3TransformNormal( &ViewLightDirection, &m_vecLightDir, &matView );

	// Scale to skybox size
	D3DXVECTOR3 ViewLightPosition;
	D3DXVec3Normalize( &ViewLightPosition, &ViewLightDirection );
	ViewLightPosition *= sqrtf( 1.0*1.0f + 1.0f*1.0f + 1.0f*1.0f );

	// World matrix
	D3DXMATRIX World;
	D3DXVECTOR3 Position( ViewLightPosition.x, ViewLightPosition.y, 0 );
	D3DXVECTOR3 Scale( m_pLightElement->BaseSize.x, m_pLightElement->BaseSize.y, 1.0f );
	D3DXMatrixTransformation( &World, NULL, NULL, &Scale, NULL, NULL, &Position );


	// Skybox projection
	D3DXMATRIX SkyBoxProjection;
	D3DXMatrixOrthoLH( &SkyBoxProjection, 2.0f, 2.0f * FLOAT(SrcDesc.Height)/FLOAT(SrcDesc.Width), 0.0f, 1.0f );

	// Clip light if it is behind us
	D3DXVECTOR3 ProjectedLightPos;
	D3DXVec3TransformCoord( &ProjectedLightPos, &ViewLightPosition, &SkyBoxProjection );

	// Immediately clip if the light is behind us
	if( ProjectedLightPos.z <= 0 )
	{
		pDstSurface->Release();
		g_BsKernel.GetDevice()->RestoreState();
		return ;
	}

	// Viewport matrix 
	D3DXMATRIX ViewportMat;
	D3DVIEWPORT9 Viewport;
	ZeroMemory(&ViewportMat, sizeof( ViewportMat ) );
	pDevice->GetViewport(&Viewport);
	ViewportMat[0*4 + 0] = FLOAT( Viewport.Width )/2.0f;
	ViewportMat[1*4 + 1] = -FLOAT( Viewport.Height )/2.0f;
	ViewportMat[2*4 + 2] = Viewport.MaxZ - Viewport.MinZ;
	ViewportMat[3*4 + 0] = FLOAT( Viewport.X ) + FLOAT( Viewport.Width )/2.0f;
	ViewportMat[3*4 + 1] = FLOAT( Viewport.Y ) + FLOAT( Viewport.Height )/2.0f;
	ViewportMat[3*4 + 2] = Viewport.MinZ;
	ViewportMat[3*4 + 3] = 1.0f;

	// Get WVPS
	D3DXMATRIX WVPS = World * SkyBoxProjection * ViewportMat;

	// Transform quad corner points into screen space so we can copy the back buffer
	D3DXVECTOR3 Quad[2];
	Quad[0] = D3DXVECTOR3( -0.5f, -0.5f, 1 );
	Quad[1] = D3DXVECTOR3(  0.5f,  0.5f, 1 );

	// Get quad corner point in screen space
	const D3DXVECTOR4 ScreenSpaceOffset( 0.5f + 1.0f/32.0f, 0.5f + 1.0f/32.0f, 0.0f, 0.0f );
	D3DXVECTOR4 ScreenLightPosVerts[2];
	D3DXVec3Transform( &ScreenLightPosVerts[0], &Quad[0], &WVPS );
	D3DXVec3Transform( &ScreenLightPosVerts[1], &Quad[1], &WVPS );
	ScreenLightPosVerts[0] += ScreenSpaceOffset;
	ScreenLightPosVerts[1] += ScreenSpaceOffset;

	//----------------------------------------------
	// Compute source rectangle for back buffer copy
	//----------------------------------------------

	// Get raster portions of screen space coordinates
	int SrcX0 = RasterPos( ScreenLightPosVerts[0].x );
	int SrcX1 = RasterPos( ScreenLightPosVerts[1].x );
	int SrcY0 = RasterPos( ScreenLightPosVerts[1].y );
	int SrcY1 = RasterPos( ScreenLightPosVerts[0].y );

	// If light is not visible, return
	if(SrcX1 <= 0  || SrcX0 >= int( SrcDesc.Width ) ||
		SrcY1 <= 0  || SrcY0 >= int( SrcDesc.Height ) )
	{
		pDstSurface->Release();
		g_BsKernel.GetDevice()->RestoreState();
		return;
	}

	// Clamp the src rectangle and offset the dst rectangle if needed
	int DstX0 = 0;
	int DstX1 = DstDesc.Width;
	int DstY0 = 0;
	int DstY1 = DstDesc.Height;

	if( SrcX0 < 0 )	{
		DstX0 = -SrcX0;
		SrcX0 = 0;
	}
	if( SrcX1 > int( SrcDesc.Width ) ) {
		SrcX1 = int( SrcDesc.Width );
	}
	if( SrcY0 < 0 ) {
		DstY0 = -SrcY0;
		SrcY0 = 0;
	}
	if( SrcY1 > int( SrcDesc.Height ) ) {
		SrcY1 = int( SrcDesc.Height );
	}

	//--------------------------------------
	// Set general render state
	//--------------------------------------
	pDevice->SetRenderState( D3DRS_CULLMODE,  D3DCULL_CCW );
	pDevice->SetRenderState( D3DRS_ALPHATESTENABLE,  FALSE );


	//--------------------------------------------------
	// Compute visibility ratio and fill back buffer alpha
	//--------------------------------------------------

	// Set world matrix
	Position = D3DXVECTOR3( ViewLightPosition.x, ViewLightPosition.y, 0 );
	Scale = D3DXVECTOR3( m_pLightElement->BaseSize.x, m_pLightElement->BaseSize.y, 1.0f );
	D3DXMatrixTransformation( &World, NULL, NULL, &Scale, NULL, NULL, &Position );

	// Clear the alpha channel.  NOTE: this does not have to be done if alpha values are all 0.0f
//	g_BsKernel.GetDevice()->SetVertexDeclaration(m_pVD);
	g_BsKernel.SetVertexDeclaration(m_nVertexDeclIdx);
	CBsMaterial *pBsMaterial  = g_BsKernel.GetMaterialPtr( m_nMaterialIndex );
	pBsMaterial->BeginMaterial(0, 0);
	pBsMaterial->BeginPass(0);
	
	D3DXMATRIX mtxWP = World * SkyBoxProjection;
	pBsMaterial->SetMatrix(m_hMatWVP, &mtxWP);
	
	D3DXVECTOR4 vClearColor = m_pLightElement->Color;
	vClearColor.w = 0.f;
	pBsMaterial->SetVector(m_hLightColor, &vClearColor);

	pBsMaterial->SetTexture(m_hTexture, m_pLightElement->nTexture);
	D3DXVECTOR4 vTexCoord (0.f, 0.f, 1.f, 1.f);
	pBsMaterial->SetVector(m_hTexCoordOffset, &vTexCoord);

	pDevice->SetRenderState( D3DRS_ZENABLE,            D3DZB_FALSE );
	pDevice->SetRenderState( D3DRS_ALPHABLENDENABLE,	FALSE );
	pDevice->SetRenderState( D3DRS_COLORWRITEENABLE,   D3DCOLORWRITEENABLE_ALPHA );

	pBsMaterial->CommitChanges();
	pDevice->DrawMeshVB( D3DPT_TRIANGLESTRIP, 2, m_pQuadVB, sizeof(SVert));

	pDevice->SetRenderState( D3DRS_ZENABLE,            D3DZB_TRUE );
	vClearColor.w = 1.f;
	pBsMaterial->SetVector(m_hLightColor, &vClearColor);
	pBsMaterial->CommitChanges();
	pDevice->DrawMeshVB( D3DPT_TRIANGLESTRIP, 2, m_pQuadVB, sizeof(SVert));

	// Render light 
	pDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
	pDevice->SetRenderState( D3DRS_SRCBLEND,         D3DBLEND_SRCALPHA );
	pDevice->SetRenderState( D3DRS_DESTBLEND,        D3DBLEND_INVSRCALPHA );
	pDevice->SetRenderState( D3DRS_ZENABLE,          D3DZB_TRUE );
	pDevice->SetRenderState( D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE | D3DCOLORWRITEENABLE_ALPHA );

	pBsMaterial->CommitChanges();
	IDirect3DQuery9* pQuery;
	DWORD numberOfPixelsDrawn = 0;
	pDevice->GetD3DDevice()->CreateQuery(D3DQUERYTYPE_OCCLUSION, &pQuery);
	pQuery->Issue(D3DISSUE_BEGIN);

	pDevice->DrawPrimitiveVB(D3DPT_TRIANGLESTRIP, 0, 2);

	pQuery->Issue(D3DISSUE_END);
	while(S_FALSE == pQuery->GetData( &numberOfPixelsDrawn, sizeof(DWORD), D3DGETDATA_FLUSH )) { ; }

	pQuery->Release();

	FLOAT fVisRatio = FLOAT(numberOfPixelsDrawn)/FLOAT(m_uiNumFullLightPixels);

	// Clamp visibility ratio
	if( fVisRatio > 1.0f )
		fVisRatio = 1.0f;
	if( fVisRatio < 0 )
		fVisRatio = 0.0f;

	// If the light is not visible, no effect and return!!
	if( fVisRatio == 0.0f ) {
		pDstSurface->Release();
		g_BsKernel.GetDevice()->RestoreState();
		
		pBsMaterial->EndPass();
		pBsMaterial->EndMaterial();
		return;
	}

	//----------------------------------------------------
	// Copy back buffer portion that light was rendered too
	//----------------------------------------------------

	// We use the last back buffer copy if the visibility remains 100%
	RECT SrcRect;
	SrcRect.left   = UINT( SrcX0 );
	SrcRect.right  = UINT( SrcX1 );
	SrcRect.top    = UINT( SrcY0 );
	SrcRect.bottom = UINT( SrcY1 );

	UINT uiWidth, uiHeight;
	uiWidth = SrcRect.right - SrcRect.left;
	uiHeight= SrcRect.bottom- SrcRect.top;


	RECT DstRect;
	DstRect.left = UINT(DstX0);
	DstRect.top =  UINT(DstY0);
	DstRect.right = UINT(DstX0)+uiWidth;
	DstRect.bottom= UINT(DstY0)+uiHeight;
	// 이코드는 1280x720에 대한 문제입니다. 자세한건 Debugging필요 by jeremy
	if(DstRect.right>63)
		DstRect.right = 63;
	if(DstRect.bottom>63)
		DstRect.bottom = 63;


	vTexCoord.x = float(SrcRect.left+1)/float(SrcDesc.Width);
	vTexCoord.y = float(SrcRect.top+1)/float(SrcDesc.Height);
	vTexCoord.z = float(uiWidth-2)/float(SrcDesc.Width);
	vTexCoord.w = float(uiHeight-2)/float(SrcDesc.Height);
	pBsMaterial->SetVector(m_hTexCoordOffset, &vTexCoord);
	


//	if( fVisRatio == 1.0f && m_bPreviouslyFullyVisible ) {
//	} else {
		//------------------------------------
		// Copy back buffer 
		//------------------------------------
	
#ifdef _XBOX
/*
		D3DRECT rect;
		rect.x1 = (SrcRect.left/8)*8;
		rect.y1 = (SrcRect.top/8)*8;
		rect.x2 = (SrcRect.right/8)*8;
		rect.y2 = (SrcRect.bottom/8)*8;
		if(rect.x1 == rect.x2)
			rect.x2 += 8;
		if(rect.y1 == rect.y2)
			rect.y2 += 8;

		if( rect.x2- rect.x1 > 64)
			rect.x2 += rect.x1 + 64;

		if( rect.y2- rect.y1 > 64)
			rect.y2 += rect.y1 + 64;
		
		if(SrcRect.right%8)
			rect.x2 += 8;
		if(SrcRect.bottom%8)
			rect.y2 += 8;

		D3DPOINT destPoint;
		destPoint.x = 
		rect.x1 = 32;
		rect.y1 = 32;
		rect.x2 = 96;
		rect.y2 = 96;
*/
		// full screen resolve not good.
		pDevice->Resolve(D3DRESOLVE_RENDERTARGET0, NULL/*&rect*/, pTexture, NULL, 0, 0, NULL, 1.0f, 0, NULL);
#else
		pDevice->StretchRect( pSrcSurface, &SrcRect, pDstSurface, &DstRect , D3DTEXF_NONE);
#endif
//	}
	m_bPreviouslyFullyVisible = ( fVisRatio == 1.0f );

	//----------------------------------------
	// Compute flare parameters
	//----------------------------------------
	const FLOAT fInvisAlpha = 1.0f/255.0f;  // if alpha is below this, the element if not rendered
	const FLOAT fInvisSize  = 0.00001f;     // if size is below this, the element is not rendered

	// Compute distance ratio
	FLOAT fMaxDistX = m_pLightElement->ViewCenter.x < 0 ? 1.0f - m_pLightElement->ViewCenter.x : m_pLightElement->ViewCenter.x - -1.0f;
	FLOAT fMaxDistY = m_pLightElement->ViewCenter.y < 0 ? 1.0f - m_pLightElement->ViewCenter.y : m_pLightElement->ViewCenter.y - -1.0f;
	D3DXVECTOR2 FlareVec(ViewLightPosition.x - m_pLightElement->ViewCenter.x, ViewLightPosition.y - m_pLightElement->ViewCenter.y);
	FLOAT fMaxFlareDistance = D3DXVec2Length( &D3DXVECTOR2( fMaxDistX, fMaxDistY) );
	FLOAT fDistanceRatio = ( fMaxFlareDistance - D3DXVec2Length(&FlareVec) ) / fMaxFlareDistance;

	// Calculate light intensity
	D3DXVECTOR3 NormLightDirection;
	D3DXVECTOR3 NormViewForward(0, 0, 1.0f);
	D3DXVec3Normalize( &NormLightDirection, &ViewLightDirection );
	FLOAT fLightIntensity = D3DXVec3Dot( &NormLightDirection, &NormViewForward );

	//----------------------------------------
	// Render light "flare out"
	//-----------------------------------------
	// Set render state
	pDevice->SetRenderState( D3DRS_ZENABLE,          D3DZB_FALSE );
	pDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );                    // FALSE: shows back buffer copy
	pDevice->SetRenderState( D3DRS_SRCBLEND,         D3DBLEND_SRCALPHA );
	pDevice->SetRenderState( D3DRS_DESTBLEND,        D3DBLEND_ONE );

	pBsMaterial->SetTexture(m_hTexture, pTexture);

	// 현재 10개는 scale된 간격이 큰편이다. 더욱 촘촘히 해줄 필요가 있다...
	for( UINT i = 0; i < m_pLightElement->uiNumFlareOuts; i++ ) {
		FLOAT fScale = (i + 1) / FLOAT( m_pLightElement->uiNumFlareOuts );

		D3DXVECTOR3 FlarePosition( ViewLightPosition.x, ViewLightPosition.y, 0 );

		D3DXVECTOR2 FlareOutSize = m_pLightElement->BaseSize + ( m_pLightElement->FlareOutSize - m_pLightElement->BaseSize ) * fScale;
		Scale = D3DXVECTOR3( FlareOutSize.x, FlareOutSize.y, 1.0f );
		Scale*= powf( fLightIntensity, 2 );

		D3DXMatrixTransformation( &World, NULL, NULL, &Scale, NULL, NULL, &Position );
		//pDevice->SetTransform( D3DTS_WORLD, &World );
		mtxWP = World * SkyBoxProjection;
		pBsMaterial->SetMatrix(m_hMatWVP, &mtxWP);

		D3DXVECTOR4 Color( m_pLightElement->Color );
		FLOAT fAlphaScale = FLOAT( m_pLightElement->uiNumFlareOuts - i ) / FLOAT( m_pLightElement->uiNumFlareOuts );

		Color.w *= fAlphaScale * powf( fLightIntensity, 4 );//*fVisRatio;
		//pDevice->SetRenderState( D3DRS_TEXTUREFACTOR, Color );
		pBsMaterial->SetVector(m_hLightColor, &Color);//..

		pBsMaterial->CommitChanges();
		pDevice->DrawPrimitiveVB( D3DPT_TRIANGLESTRIP, 0, 2);
	}

	//----------------------------------------
	// Render flare elements
	//-----------------------------------------
	FLOAT fAlphaMult = fDistanceRatio * fVisRatio - powf(fLightIntensity, 50);

	// Skip rendering flares if they will be invisible
	if( fAlphaMult < fInvisAlpha ) {
		pDstSurface->Release();
		pDevice->RestoreState();

		pBsMaterial->EndPass();
		pBsMaterial->EndMaterial();
		return;
	}
	FLOAT fSizeMult = fDistanceRatio;
	if( fSizeMult < fInvisSize ) {
		pDstSurface->Release();
		pDevice->RestoreState();

		pBsMaterial->EndPass();
		pBsMaterial->EndMaterial();
		return;
	}

	// Flare element render states
	pDevice->SetRenderState( D3DRS_ZENABLE,          FALSE );
	pDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
	pDevice->SetRenderState( D3DRS_SRCBLEND,         D3DBLEND_SRCALPHA );
	pDevice->SetRenderState( D3DRS_DESTBLEND,        D3DBLEND_ONE );
	
	vTexCoord = D3DXVECTOR4(0.f, 0.f, 1.f, 1.f);
	pBsMaterial->SetVector(m_hTexCoordOffset, &vTexCoord);

	for( UINT i = 0; i < m_uiNumFlareElements; i++ ) {
		pBsMaterial->SetTexture(m_hTexture, m_pFlareElements[i].nTexture);
		D3DXVECTOR4 Color( m_pFlareElements[i].Color );
		Color.w = Color.w * fAlphaMult;

		// Skip if alpha is too small
		if( Color.w < fInvisAlpha )
			continue;

		//pDevice->SetRenderState( D3DRS_TEXTUREFACTOR, Color);

		// Set world view projection matrices
		Position = D3DXVECTOR3( FlareVec.x, FlareVec.y, 0.0f );
		Position *= m_pFlareElements[i].fDistance;

		D3DXVECTOR2 Size( m_pFlareElements[i].Size.x, m_pFlareElements[i].Size.y );

		Size *= fSizeMult;

		// Clamp to max size
		if( Size.x > m_MaxElementSize.x )
			Size *= m_MaxElementSize.x/Size.x;
		if( Size.y > m_MaxElementSize.y )
			Size *= m_MaxElementSize.y/Size.y;

		// Skip if size is too small
		if( Size.x < fInvisSize || Size.y < fInvisSize )
			continue;

		Scale = D3DXVECTOR3( Size.x, Size.y, 1.0f);
		D3DXMatrixTransformation( &World, NULL, NULL, &Scale, NULL, NULL, &Position );
		//pDevice->SetTransform( D3DTS_WORLD, &World);

		mtxWP = World * SkyBoxProjection;
		pBsMaterial->SetMatrix(m_hMatWVP, &mtxWP);
		pBsMaterial->SetVector(m_hLightColor, &Color);

		pBsMaterial->CommitChanges();
		pDevice->DrawPrimitiveVB(D3DPT_TRIANGLESTRIP, 0, 2);
	}

	pBsMaterial->EndPass();
	pBsMaterial->EndMaterial();

	// Cleanup
	pDstSurface->Release();
	pDevice->RestoreState();
}

//-----------------------------------------------------------------------------
// Name: ProcessMessage()
// Desc: FX Message Process
//-----------------------------------------------------------------------------
int CFcFXLensFlare::ProcessMessage(int nCode,DWORD dwParam1/*=0*/, DWORD dwParam2/*=0*/,DWORD dwParam3/*=0*/)
{
	switch(nCode){
	case FX_STOP_OBJECT:
		return 1;
	case FX_PLAY_OBJECT:
		return 1;
	case FX_SET_LIGHTDIRECTION:
		{
			D3DXVECTOR3* pVecLightDir=(D3DXVECTOR3*)dwParam1;
			m_vecLightDir.x=-pVecLightDir->x;
			m_vecLightDir.y=-pVecLightDir->y;
			m_vecLightDir.z=-pVecLightDir->z;
			return 1;
		}
	}

	if ( CFcFXBase::ProcessMessage( nCode, dwParam1, dwParam2, dwParam3) )
		return 1;
	return 0;
}