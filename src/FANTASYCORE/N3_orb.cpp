//--------------------------------------------------------------------------------------
// HDRLighting.cpp
//
// This sample demonstrates some high dynamic range lighting effects using floating
// point textures.
//
// The algorithms described in this sample are based very closely on the lighting
// effects implemented in Masaki Kawase's Rthdribl sample and the tone mapping process
// described in the whitepaper "Tone Reproduction for Digital Images"
//
//    Real-Time High Dynamic Range Image-Based Lighting (Rthdribl)
//    Masaki Kawase
//    http://www.daionet.gr.jp/~masa/rthdribl
//
//    "Photographic Tone Reproduction for Digital Images"
//    Erik Reinhard, Mike Stark, Peter Shirley and Jim Ferwerda
//    http://www.cs.utah.edu/~reinhard/cdrom
//
// Xbox Advanced Technology Group.
// Copyright (C) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

#include "Stdafx.h"
#include "n3_orb.h"
#include "BsKernel.h"


#ifdef _XBOX
Orb::Orb()
{
}
Orb::~Orb()
{
	Release();
}
//--------------------------------------------------------------------------------------
// Name: Initialize()
// Desc: Initialize app-dependent objects
//--------------------------------------------------------------------------------------
HRESULT Orb::Initialize(C3DDevice* d3dDevice)
{
	m_pd3dDevice = d3dDevice->GetD3DDevice();
	m_d3dpp = *d3dDevice->GetPresentParameter();
	


    HRESULT hr;

	kussetu = 1.2f;

	m_frame = 0;
//	m_bToneMap = FALSE;

    m_bDrawHelp   = FALSE;


    m_fMiddleGrayKeyValue = 0.60f;    // Middle gray key value for tone mapping
    m_fBloomScale         = 3.0f;     // Scale factor for bloom
    m_fStarScale          = 2.0f;     // Scale factor for star
	m_fRadialScale			= 0.0f;

	g_BsKernel.chdir("Media");
	char szFullFileName[_MAX_PATH];
	sprintf(szFullFileName, "%s%s", g_BsKernel.GetCurrentDirectory(), "Resource.xpr" );
    // Create the textures resource
	if( FAILED( m_Resource.Create( szFullFileName ) ) )
    {
        ATG_PrintError( "DX9Sample: Couldn't create Resource.xpr\n" );
        return E_FAIL;
    }
	g_BsKernel.chdir("..");
    // Initialize the post-processing effects library (for blur, bloom, etc.)
    if( FAILED( m_PostProcess.Initialize() ) )
    {
        ATG_PrintError( "DX9Sample: Couldn't initialize the effects library\n" );
        return E_FAIL;
    }

    // Initialize the glare definition used for the star effect
    m_GlareDef.Initialize( GLT_FILTER_CROSSSCREEN );

    // Create the vertex declarations
    D3DVERTEXELEMENT9 decl[] =
    {
        { 0,  0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT,  D3DDECLUSAGE_POSITION, 0 },
        { 0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT,  D3DDECLUSAGE_NORMAL,   0 },
        { 0, 24, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT,  D3DDECLUSAGE_TEXCOORD, 0 },
        D3DDECL_END()
    };
    m_pd3dDevice->CreateVertexDeclaration( decl, &m_pTransformSceneVtxDecl );

	g_BsKernel.chdir("Media");
	g_BsKernel.chdir("Shaders");

    // Create vertex and pixel shaders
	sprintf(szFullFileName, "%s%s", g_BsKernel.GetCurrentDirectory(), "SceneVS.xvu" );
    V( ATG::LoadVertexShader( szFullFileName, &m_pSceneVS ) );

	sprintf(szFullFileName, "%s%s", g_BsKernel.GetCurrentDirectory(), "ParticlesVS.xvu" );
    V( ATG::LoadVertexShader( szFullFileName, &m_pParticlesVS ) );

	sprintf(szFullFileName, "%s%s", g_BsKernel.GetCurrentDirectory(), "FinalScenePassPS.xpu" );
    V( ATG::LoadPixelShader( szFullFileName,  &m_pFinalScenePassPS ) );

	sprintf(szFullFileName, "%s%s", g_BsKernel.GetCurrentDirectory(), "ScenePS.xpu" );
    V( ATG::LoadPixelShader( szFullFileName,  &m_pScenePS ) );

	sprintf(szFullFileName, "%s%s", g_BsKernel.GetCurrentDirectory(), "RenderSkyBox_PS.xpu" );
    V( ATG::LoadPixelShader( szFullFileName,  &m_pSkyBoxPS ) );

	sprintf(szFullFileName, "%s%s", g_BsKernel.GetCurrentDirectory(), "ParticlesPS.xpu" );
    V( ATG::LoadPixelShader( szFullFileName,  &m_pParticlesPS ) );

	sprintf(szFullFileName, "%s%s", g_BsKernel.GetCurrentDirectory(), "RadialPS.xpu" );
    V( ATG::LoadPixelShader( szFullFileName,  &m_pRadialPS ) );

	sprintf(szFullFileName, "%s%s", g_BsKernel.GetCurrentDirectory(), "FinalPS.xpu" );
    //V( ATG::LoadPixelShader( szFullFileName,  &m_pFinalPS ) );

	sprintf(szFullFileName, "%s%s", g_BsKernel.GetCurrentDirectory(), "m_pPS.xpu" );
    V( ATG::LoadPixelShader( szFullFileName,  &m_pPS ) );
	
	g_BsKernel.chdir("..");
	g_BsKernel.chdir("..");
    // Use the 7e3 scene format
//    D3DFORMAT d3dSceneFormat = D3DFMT_A2B10G10R10F_EDRAM;
//    D3DFORMAT d3dSceneFormat = D3DFMT_A16B16G16R16F;

    // Check need for predicated tiling
    DWORD dwTileWidth  = m_d3dpp.BackBufferWidth;
    DWORD dwTileHeight = m_d3dpp.BackBufferHeight;
    m_dwNumTiles = 4;
    for( DWORD i=0; i<m_dwNumTiles; i++ )
    {
        m_TilingRects[i].x1 = (i/2+0) * dwTileWidth;
        m_TilingRects[i].x2 = (i/2+1) * dwTileWidth;
        m_TilingRects[i].y1 = (i%2+0) * dwTileHeight;
        m_TilingRects[i].y2 = (i%2+1) * dwTileHeight;
    }
/*
	for( DWORD i=0; i<m_dwNumTiles; i++ )
    {
        m_TilingRects[i].x1 = (i+0) * dwTileWidth;
        m_TilingRects[i].x2 = (i+1) * dwTileWidth;
        m_TilingRects[i].y1 = 0;
        m_TilingRects[i].y2 = dwTileHeight;
    }
*/
    // Create the front buffer
    m_pd3dDevice->CreateTexture(  m_d3dpp.BackBufferWidth, m_d3dpp.BackBufferHeight,
                                  1, D3DUSAGE_RENDERTARGET, D3DFMT_LE_X8R8G8B8,
                                  D3DPOOL_DEFAULT, &m_pFrontBuffer[0], NULL ) ;

    m_pd3dDevice->CreateTexture(  m_d3dpp.BackBufferWidth, m_d3dpp.BackBufferHeight,
                                  1, D3DUSAGE_RENDERTARGET, D3DFMT_LE_X8R8G8B8,
                                  D3DPOOL_DEFAULT, &m_pFrontBuffer[1], NULL ) ;

	D3DSURFACE_PARAMETERS TileSurfaceParams;
	TileSurfaceParams.Base = 0;
	TileSurfaceParams.HierarchicalZBase = 0;
	TileSurfaceParams.ColorExpBias = 0;
	m_pd3dDevice->CreateRenderTarget( m_d3dpp.BackBufferWidth, m_d3dpp.BackBufferHeight, D3DFMT_A8R8G8B8,
                                      D3DMULTISAMPLE_NONE, 0, FALSE,
                                      &pSurfRT, &TileSurfaceParams );

	// Create the back buffer at base address 0
    D3DSURFACE_PARAMETERS SurfaceParameters = {0};
    SurfaceParameters.Base = 0;

    // Initialize the camera
	fEyeV = 0.0f;
	fEyeR = -130.0f;
    vFromPt   = XMVectorSet( 0.0f, 200.0f, fEyeR, 1.0f );
    vLookatPt = XMVectorSet( 0.0f, 200.0f,  0.0f, 1.0f );
    XMVECTOR vUp       = XMVectorSet( 0.0f, 1.0f,  0.0f, 1.0f );

    // Determine the aspect ratio
    FLOAT fAspectRatio = (FLOAT)m_d3dpp.BackBufferWidth / (FLOAT)m_d3dpp.BackBufferHeight;

	m_matWorldBase = XMMatrixScaling( 40.0f, 40.0f, 40.0f );
    m_matWorld = XMMatrixTranslation(0.0f,200.0f,0.0f);//XMMatrixIdentity();
	m_matWorld = XMMatrixMultiply( m_matWorldBase, m_matWorld );
    m_matView  = XMMatrixLookAtLH( vFromPt, vLookatPt, vUp );
	m_matViewT = XMMatrixTranspose( m_matView );
    m_matProj = XMMatrixPerspectiveFovLH( XM_PI/4.0f, fAspectRatio, 10.0f, 800.0f );
	m_matProjT = XMMatrixTranspose( m_matProj );

    // Create the white texture
    V( m_pd3dDevice->CreateTexture( 1, 1, 1, 0L, D3DFMT_A8R8G8B8,
                                           D3DPOOL_DEFAULT, &m_pWhiteTexture, NULL ) );
    m_PostProcess.ClearTexture( m_pWhiteTexture, 0xffffffff );

	V( m_pd3dDevice->CreateTexture( m_d3dpp.BackBufferWidth, m_d3dpp.BackBufferHeight,
										1, D3DUSAGE_RENDERTARGET, D3DFMT_A16B16G16R16F,
										D3DPOOL_DEFAULT, &g_pTexScene, NULL ) );

	V( m_pd3dDevice->CreateTexture( m_d3dpp.BackBufferWidth*2, m_d3dpp.BackBufferHeight*2,
										1, D3DUSAGE_RENDERTARGET, D3DFMT_A16B16G16R16F,
										D3DPOOL_DEFAULT, &g_pTexSceneL, NULL ) );

	V( m_pd3dDevice->CreateTexture( m_d3dpp.BackBufferWidth, m_d3dpp.BackBufferHeight,
										1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8,
										D3DPOOL_DEFAULT, &g_pTexRadial, NULL ) );

    // Scaled version of the HDR scene texture
    V( m_pd3dDevice->CreateTexture( m_d3dpp.BackBufferWidth / 4, m_d3dpp.BackBufferHeight / 4,
                                           1, D3DUSAGE_RENDERTARGET,
                                           D3DFMT_A16B16G16R16F, D3DPOOL_DEFAULT,
                                           &m_pScaledSceneTexture, NULL ) );

    // Create a texture to hold the intermediate results of the luminance calculation
    V( m_pd3dDevice->CreateTexture( 64, 64, 1, D3DUSAGE_RENDERTARGET, 
                                           D3DFMT_R32F, D3DPOOL_DEFAULT, 
                                           &m_pToneMapTexture64x64, NULL ) );
    V( m_pd3dDevice->CreateTexture( 16, 16, 1, D3DUSAGE_RENDERTARGET, 
                                           D3DFMT_R32F, D3DPOOL_DEFAULT, 
                                           &m_pToneMapTexture16x16, NULL ) );
    V( m_pd3dDevice->CreateTexture( 4, 4, 1, D3DUSAGE_RENDERTARGET, 
                                           D3DFMT_R32F, D3DPOOL_DEFAULT, 
                                           &m_pToneMapTexture4x4, NULL ) );
    V( m_pd3dDevice->CreateTexture( 1, 1, 1, D3DUSAGE_RENDERTARGET, 
                                           D3DFMT_R32F, D3DPOOL_DEFAULT, 
                                           &m_pToneMapTexture1x1, NULL ) );

    // Create a 1x1 texture to hold the luminance that the user is currently adapted to.
    // This allows for a simple simulation of light adaptation.
    V( m_pd3dDevice->CreateTexture( 1, 1, 1,
                                           D3DUSAGE_RENDERTARGET, D3DFMT_R32F, D3DPOOL_DEFAULT,
                                           &m_pAdaptedLuminanceTexture, NULL ) );

    // Create the bright-pass filter texture.
    // Texture has a black border of single texel thickness to fake border
    // addressing using clamp addressing
    V( m_pd3dDevice->CreateTexture( m_d3dpp.BackBufferWidth / 4 + 2, m_d3dpp.BackBufferHeight / 4 + 2,
                                           1, D3DUSAGE_RENDERTARGET,
                                           D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT,
                                           &m_pBrightPassTexture, NULL ) );

    // Create a texture to be used as the source for the star effect
    // Texture has a black border of single texel thickness to fake border
    // addressing using clamp addressing
    V( m_pd3dDevice->CreateTexture( m_d3dpp.BackBufferWidth / 4 + 2, m_d3dpp.BackBufferHeight / 4 + 2,
                                           1, D3DUSAGE_RENDERTARGET,
                                           D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT,
                                           &m_pStarSourceTexture, NULL ) );

    // Create a texture to be used as the source for the bloom effect
    V( m_pd3dDevice->CreateTexture( m_d3dpp.BackBufferWidth / 8, m_d3dpp.BackBufferHeight / 8,
                                           1, D3DUSAGE_RENDERTARGET,
                                           D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT,
                                           &m_pBloomSourceTexture, NULL ) );

    // Create the temporary blooming effect textures
    // Texture has a black border of single texel thickness to fake border
    // addressing using clamp addressing
    V( m_pd3dDevice->CreateTexture( m_d3dpp.BackBufferWidth/8, m_d3dpp.BackBufferHeight/8,
                                            1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8,
                                            D3DPOOL_DEFAULT, &m_pBloomTexture, NULL ) );

    // Create the star effect textures
    for( DWORD i=0; i < NUM_STAR_TEXTURES; i++ )
    {
        V( m_pd3dDevice->CreateTexture( m_d3dpp.BackBufferWidth /4, m_d3dpp.BackBufferHeight / 4,
                                               1, D3DUSAGE_RENDERTARGET,
                                               D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT,
                                               &m_apStarTextures[i], NULL ) );
    }

    // Textures with borders must be cleared since scissor rect testing will
    // be used to avoid rendering on top of the border
    m_PostProcess.ClearTexture( m_pBloomSourceTexture );
    m_PostProcess.ClearTexture( m_pAdaptedLuminanceTexture );
    m_PostProcess.ClearTexture( m_pBrightPassTexture );
    m_PostProcess.ClearTexture( m_pStarSourceTexture );

    m_PostProcess.ClearTexture( m_pBloomTexture );

	g_BsKernel.chdir("Media");
	g_BsKernel.chdir("Resources");

	sprintf(szFullFileName, "%s%s", g_BsKernel.GetCurrentDirectory(), "hikarigokeBokasi2.dds" );
	V( D3DXCreateCubeTextureFromFileEx( m_pd3dDevice, szFullFileName, D3DX_DEFAULT, 1, 0, D3DFMT_A8R8G8B8, 
                                                D3DPOOL_MANAGED, D3DX_FILTER_NONE, D3DX_FILTER_NONE, 0, NULL, NULL, &m_pTexSkyBox ) );

	sprintf(szFullFileName, "%s%s", g_BsKernel.GetCurrentDirectory(), "last3.dds" );
	V( D3DXCreateVolumeTextureFromFileEx( m_pd3dDevice, szFullFileName, D3DX_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT, 1, 0, D3DFMT_X8R8G8B8, 
                                                D3DPOOL_MANAGED, D3DX_FILTER_NONE, D3DX_FILTER_NONE, 0, NULL, NULL, &m_pTexVolume ) );

	sprintf(szFullFileName, "%s%s", g_BsKernel.GetCurrentDirectory(), "Volume2.dds" );
	V( D3DXCreateVolumeTextureFromFileEx( m_pd3dDevice, szFullFileName, D3DX_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT, 1, 0, D3DFMT_X8R8G8B8, 
                                                D3DPOOL_MANAGED, D3DX_FILTER_NONE, D3DX_FILTER_NONE, 0, NULL, NULL, &m_pTexSea ) );
/*
	V( D3DXCreateTextureFromFileEx( m_pd3dDevice, "d:\\Data\\Media\\Resources\\rogo.dds", D3DX_DEFAULT, D3DX_DEFAULT, 1, 0, D3DFMT_A8R8G8B8, 
                                                D3DPOOL_MANAGED, D3DX_FILTER_NONE, D3DX_FILTER_NONE, 0, NULL, NULL, &m_pTexRogo ) );
*/	
	g_BsKernel.chdir("..");
	g_BsKernel.chdir("..");


	g_BsKernel.chdir("Media");
	g_BsKernel.chdir("Meshes");
	// クリス?ル用メッシュの生成
	sprintf(szFullFileName, "%s%s", g_BsKernel.GetCurrentDirectory(), "orb.x" );
    V( LoadMesh( m_pd3dDevice, szFullFileName, &g_pMesh ) );

	g_BsKernel.chdir("..");
	g_BsKernel.chdir("..");


	D3DVERTEXELEMENT9 orbDecl[16];
	g_pMesh->GetDeclaration(orbDecl);
    m_pd3dDevice->CreateVertexDeclaration( orbDecl, &m_pOrbVtxDecl );

	// 不純物用メッシュの作成
	V( D3DXCreatePolygon( m_pd3dDevice, PARTICLES_SIZE, 4, &g_pPointMesh, NULL) );

	V( m_pd3dDevice->CreateVertexBuffer( 3*sizeof(XMFLOAT4), 
                                         D3DUSAGE_WRITEONLY, 0, 
                                         D3DPOOL_DEFAULT, &m_pVB, NULL ) );

    XMFLOAT4* v;
    m_pVB->Lock( 0, 0, (VOID**)&v, 0 );
    v[0] = XMFLOAT4( -1.0f,  1.0f,   0.0f, 0.0f ); // x, y, tu, tv
    v[1] = XMFLOAT4(  1.0f,  1.0f,   1.0f, 0.0f ); // x, y, tu, tv
    v[2] = XMFLOAT4( -1.0f, -1.0f,   0.0f, 1.0f ); // x, y, tu, tv
    m_pVB->Unlock();

    // Create common vertex declaration used by all the screen-space effects
    D3DVERTEXELEMENT9 aSkyboxDecl[] =
    {
        { 0, 0, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
        { 0, 8, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
        D3DDECL_END()
    };

    m_pd3dDevice->CreateVertexDeclaration( aSkyboxDecl, &m_pSkyBoxDecl );

	// Initialize the particles
	for(int i = 0; i < NUM_PARTICLES; i++)
	{
		 m_vParticlesPos[i] = XMVectorSet((float)rand() * PARTICLES_RANGE * 2.0f / (float)RAND_MAX - PARTICLES_RANGE, 
			((float)rand() * PARTICLES_RANGE * 2.0f / (float)RAND_MAX - PARTICLES_RANGE), 
			((float)rand() * PARTICLES_RANGE * 2.0f / (float)RAND_MAX - PARTICLES_RANGE),
			1.0f);
	}

	g_pTexMadara = m_Resource.GetTexture( "Particle" );

	m_pBackground = m_Resource.GetTexture( "Background" );

	m_pTexRogo = m_Resource.GetTexture( "rogo" );

	m_pTexInner[0] = m_Resource.GetTexture( "OrbInner0" );
	m_pTexInner[1] = m_Resource.GetTexture( "OrbInner1" );
	m_pTexInner[2] = m_Resource.GetTexture( "OrbInner2" );
	m_pTexInner[3] = m_Resource.GetTexture( "OrbInner3" );
	m_pTexInner[4] = m_Resource.GetTexture( "OrbInner4" );
	m_pTexInner[5] = m_Resource.GetTexture( "OrbInner5" );
	m_pTexInner[6] = m_Resource.GetTexture( "OrbInner6" );
	m_pTexInner[7] = m_Resource.GetTexture( "OrbInner7" );

	m_pTexUroko = m_Resource.GetTexture( "Uroko" );

	alpha = XMVectorSet( 2.04f, 1.53f, 0.748f, 1.7f );

	for( int i = 0; i < NUM_PARTICLES; i++ )
	{
		matataki[i] = (float)rand() / RAND_MAX;
	}

	fXRotateR = 0.0f;
	fYRotateR = 0.0f;

	af = 8.0f;

	g_color = XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f);

	return S_OK;
}


//--------------------------------------------------------------------------------------
// Name: Update()
// Desc: Called once per frame, the call is the entry point for animating the scene.
//--------------------------------------------------------------------------------------
HRESULT Orb::Update()
{
    fTime          = (FLOAT)m_Timer.GetAppTime();
	m_fElapsedTime = (FLOAT)m_Timer.GetElapsedTime();

    // Get the current gamepad status
    ATG::GAMEPAD* pGamepad = ATG::Input::GetMergedInput();
    FLOAT fLeftTrigger  = pGamepad->bLeftTrigger / 255.0f;
    FLOAT fRightTrigger = pGamepad->bRightTrigger / 255.0f;

    // Toggle help
    if( pGamepad->wPressedButtons & XINPUT_GAMEPAD_BACK )
        m_bDrawHelp = !m_bDrawHelp;

    if( pGamepad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT )
    {
        af = af + 0.1f;
    }

    if( pGamepad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT )
    {
        af = max( af - 0.1f, 0.0f );
    }

    if( pGamepad->wButtons & XINPUT_GAMEPAD_DPAD_UP )
    {
        kussetu = kussetu + 0.01f;
    }

    if( pGamepad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN )
    {
        kussetu = max(kussetu - 0.01f, 0.1f );
    }

	if( pGamepad->wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER )
    {
        g_color.x += 0.01f;
        g_color.x = min( g_color.x, 3.0f );
    }

    if( pGamepad->wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER )
    {
        g_color.x -= 0.01f;
        g_color.x = max( g_color.x, 0.0f );
    }

    // Allow the user to adjust the key value
    if( pGamepad->wButtons & XINPUT_GAMEPAD_X )
    {
        m_fMiddleGrayKeyValue += (fRightTrigger-fLeftTrigger) * m_fElapsedTime * 0.05f;
        m_fMiddleGrayKeyValue = max( 0.0f, m_fMiddleGrayKeyValue );
    }

    // Allow the user to adjust the bloom scale
    if( pGamepad->wButtons & XINPUT_GAMEPAD_A )
    {
        m_fBloomScale += (fRightTrigger-fLeftTrigger) * m_fElapsedTime * 0.5f;
        m_fBloomScale = max( 0.0f, m_fBloomScale );
    }

        // Allow the user to adjust the star scale
    if( pGamepad->wButtons & XINPUT_GAMEPAD_B )
    {
        m_fStarScale += (fRightTrigger-fLeftTrigger) * m_fElapsedTime * 0.5f;
        m_fStarScale = max( 0.0f, m_fStarScale );
    }

	if( pGamepad->wButtons & XINPUT_GAMEPAD_Y )
    {
        m_fRadialScale += (fRightTrigger-fLeftTrigger) * m_fElapsedTime * 1.5f;
        m_fRadialScale = max( 0.0f, m_fRadialScale );
    }

/*
    // Allow the user to toggle tone mapping
    if( pGamepad->wPressedButtons & XINPUT_GAMEPAD_START )
		m_bToneMap = !m_bToneMap;
*/

	// Move the model
    static XMVECTOR vUp        = XMVectorSet( 0.0f, 1.0f, 0.0f, 0.0f );

    // Rotate the camera
    static FLOAT fRotateY = 0.0f;
    XMMATRIX matRotateY;
    matRotateY = XMMatrixRotationY( pGamepad->fX2 * m_fElapsedTime );

    XMMATRIX matRotate;
    fXRotate += pGamepad->fX1*m_fElapsedTime*XM_PI*0.04f;fXRotate*=0.958f;
    fYRotate += pGamepad->fY1*m_fElapsedTime*XM_PI*0.04f;fYRotate*=0.958f;
    matRotate  = XMMatrixRotationRollPitchYaw( -fYRotate, -fXRotate, 0.0f );
	if( fXRotate != 0.0f || fYRotate != 0.0f )
	    m_matWorldBase = XMMatrixMultiply( m_matWorldBase, matRotate );
	matRotate = XMMatrixRotationY( -fTime * 0.2f + XM_PI * 0.5f );
	XMMATRIX matTrans = XMMatrixTranslation( 0.0f, 200.0f, 0.0f );
	m_matWorld = XMMatrixMultiply( m_matWorldBase, matTrans );
//	m_matWorld = XMMatrixMultiply( m_matWorld, matRotate );

	fEyeV += (fRightTrigger-fLeftTrigger) * m_fElapsedTime * 80.0f;
	fEyeV *= 0.94f;
	fEyeR += fEyeV * m_fElapsedTime;
	fEyeR = fEyeR > -400.0f ? fEyeR : -400.0f;
	fEyeR = fEyeR < -100.0f ? fEyeR : -100.0f;
    // Build the view matrix
	vFromPt = XMVectorSet( 0.0f/*fEyeR * cos(fTime * 0.2f)*/, vFromPt.y, fEyeR /* sin(fTime * 0.2f)*/, vFromPt.w );
    m_matView = XMMatrixLookAtLH( vFromPt, vLookatPt, vUp );

	fXRotateR *= 0.98f;
	fYRotateR *= 0.98f;
	if(abs( fXRotateR ) < abs( fXRotate / m_fElapsedTime ))
		fXRotateR += (fXRotate / m_fElapsedTime - fXRotateR) * 0.3f;
	if(abs( fYRotateR ) < abs( fYRotate / m_fElapsedTime ))
		fYRotateR += (fYRotate / m_fElapsedTime - fYRotateR) * 0.3f;

	mEyePtInv = XMVector3TransformCoord( vFromPt, mWorldInv );

	XMVECTOR det;
	mWorldInv = XMMatrixInverse( &det, m_matWorld );

	g_mCurModelWV = XMMatrixMultiply( m_matWorld, m_matView );
	m_matWVP = XMMatrixMultiply( g_mCurModelWV, m_matProj );

	g_mCurModelWV = XMMatrixTranspose( g_mCurModelWV );
	m_matWVP = XMMatrixTranspose( m_matWVP );
	m_matWorldT = XMMatrixTranspose( m_matWorld );
	m_matViewT  = XMMatrixTranspose( m_matView );
//	m_matProjT  = XMMatrixTranspose( m_matProj );

	g_mYuragi0 = XMMatrixRotationX(fTime*0.125f);//XMMatrixRotationRollPitchYaw( -fXRotateR, -fYRotateR, 0.0f );
	g_mYuragi1 = XMMatrixRotationY(fTime*0.125f);//XMMatrixRotationRollPitchYaw( -fTime/3.0f, cos(fTime/2) / 6.0f, fTime/4.0f );
	g_mYuragi0 = XMMatrixTranspose( g_mYuragi0 );
	g_mYuragi1 = XMMatrixTranspose( g_mYuragi1 );


	XMMATRIX matRot = XMMatrixRotationY( -m_fElapsedTime * 0.1f );
	XMMATRIX matRotL = XMMatrixRotationY( m_fElapsedTime * 0.1f );
	for(int i = 0; i < NUM_PARTICLES/2; i++)
	{
		m_vParticlesPos[i*2] = XMVector3TransformNormal( m_vParticlesPos[i*2], matRot );
		m_vParticlesPos[i*2+1] = XMVector3TransformNormal( m_vParticlesPos[i*2+1], matRotL );
/*		XMVECTOR t = XMVectorFalseInt();
		t = XMVectorGreater( m_vParticlesPos[i], t );
		if(!t.y)
			m_vParticlesPos[i] += XMVectorSet( 0.0f, PARTICLES_RANGE * 2.0f, 0.0f, 0.0f );*/
	}

	m_frame = 1-m_frame;

	return S_OK;
}


//--------------------------------------------------------------------------------------
// Name: Render()
// Desc: Called once per frame, the call is the entry point for 3D rendering. This
//       function sets up render states, clears the viewport, and renders the scene.
//--------------------------------------------------------------------------------------
HRESULT Orb::Render()
{
	//HRESULT hr;
    m_fElapsedTime = max( 1/60.0f, m_fElapsedTime );

    LPDIRECT3DSURFACE9 pBackBuffer;
	pBackBuffer = NULL;
    m_pd3dDevice->GetRenderTarget( 0, &pBackBuffer );

	m_pd3dDevice->SetRenderTarget( 0, pSurfRT );

	RenderScene();

	m_PostProcess.Downsample2x2Texture( g_pTexSceneL, g_pTexScene );

//	Downsample3x3Texture( g_pTexSceneL, g_pTexScene );

    // Create a scaled copy of the scene
    m_PostProcess.Downsample4x4Texture( g_pTexScene, m_pScaledSceneTexture );

	// Setup tone mapping technique
//	if( m_bToneMap )
		MeasureLuminance();

	m_PostProcess.AdaptLuminance( m_pAdaptedLuminanceTexture, m_pToneMapTexture1x1,
								m_fElapsedTime, m_pAdaptedLuminanceTexture );

    // Now that luminance information has been gathered, the scene can be bright-pass filtered
    // to remove everything except bright lights and reflections.
    m_PostProcess.BrightPassFilterTexture( m_pScaledSceneTexture, m_pAdaptedLuminanceTexture, 
                                           m_fMiddleGrayKeyValue, m_pBrightPassTexture );

    // Blur the bright-pass filtered image to create the source texture for the star effect
    m_PostProcess.GaussBlur5x5Texture( m_pBrightPassTexture, m_pStarSourceTexture );

    // Scale-down the source texture for the star effect to create the source texture
    // for the bloom effect
    m_PostProcess.Downsample2x2Texture( m_pStarSourceTexture, m_pBloomSourceTexture );

    // Render post-process lighting effects
    RenderBloom();
    RenderStar();

//	RenderRadial();

    // Draw the high dynamic range scene texture to the low dynamic range
    // back buffer. As part of this final pass, the scene will be tone-mapped
    // using the user's current adapted luminance, blue shift will occur
    // if the scene is determined to be very dark, and the post-process lighting
    // effect textures will be added to the scene.
    m_pd3dDevice->SetPixelShader( m_pFinalScenePassPS );
    m_pd3dDevice->SetPixelShaderConstantF( PSCONST_fMiddleGray, &m_fRadialScale, 1 );
    m_pd3dDevice->SetPixelShaderConstantF( PSCONST_mEyePt, (FLOAT*)&mEyePtInv, 1 );

    m_pd3dDevice->SetPixelShaderConstantF( PSCONST_fBloomScale,      &m_fBloomScale, 1 );
    m_pd3dDevice->SetPixelShaderConstantF( PSCONST_fStarScale,       &m_fStarScale,  1 );
//    FLOAT fValue = m_bToneMap ?  1.0f : 0.0f;
//    m_pd3dDevice->SetPixelShaderConstantF( PSCONST_bEnableToneMap,   &fValue, 1 );
    m_pd3dDevice->SetPixelShaderConstantF( PSCONST_fRadialScale, &m_fMiddleGrayKeyValue, 1 );

	m_pd3dDevice->SetRenderTarget( 0, pBackBuffer );
	pBackBuffer->Release();
	m_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER, 0x00000000, 1.0f, 0 );

    m_pd3dDevice->SetTexture( 0, g_pTexScene );
    m_pd3dDevice->SetTexture( 1, m_pBloomTexture );
    m_pd3dDevice->SetTexture( 2, m_apStarTextures[0] );
    m_pd3dDevice->SetTexture( 3, m_pTexRogo );
//    m_pd3dDevice->SetTexture( 4, m_pAdaptedLuminanceTexture );
//	m_pd3dDevice->SetTexture( 5, NULL );
    m_pd3dDevice->SetSamplerState( 0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP );
    m_pd3dDevice->SetSamplerState( 0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP );
    m_pd3dDevice->SetSamplerState( 1, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP );
    m_pd3dDevice->SetSamplerState( 1, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP );
    m_pd3dDevice->SetSamplerState( 2, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP );
    m_pd3dDevice->SetSamplerState( 2, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP );
    m_pd3dDevice->SetSamplerState( 3, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP );
    m_pd3dDevice->SetSamplerState( 3, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP );
//    m_pd3dDevice->SetSamplerState( 4, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP );
//    m_pd3dDevice->SetSamplerState( 4, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP );
    m_pd3dDevice->SetSamplerState( 5, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP );
    m_pd3dDevice->SetSamplerState( 5, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP );
    m_pd3dDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_POINT );
    m_pd3dDevice->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_POINT );
    m_pd3dDevice->SetSamplerState( 1, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );
    m_pd3dDevice->SetSamplerState( 1, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
    m_pd3dDevice->SetSamplerState( 2, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );
    m_pd3dDevice->SetSamplerState( 2, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
    m_pd3dDevice->SetSamplerState( 3, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );
    m_pd3dDevice->SetSamplerState( 3, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
//    m_pd3dDevice->SetSamplerState( 4, D3DSAMP_MAGFILTER, D3DTEXF_POINT );
//    m_pd3dDevice->SetSamplerState( 4, D3DSAMP_MINFILTER, D3DTEXF_POINT );
    m_pd3dDevice->SetSamplerState( 5, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );
    m_pd3dDevice->SetSamplerState( 5, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );

	m_PostProcess.DrawFullScreenQuad();

	// Show title, frame rate, and help
    m_Timer.MarkFrame();
   

    // Resolve to the front buffer
    m_pd3dDevice->Resolve( D3DRESOLVE_RENDERTARGET0, NULL, m_pFrontBuffer[m_frame], NULL, 0, 0, NULL, 0.0f, 0, NULL );

    // Show the frame on the primary surface.
    m_pd3dDevice->RenderSystemUI();
    m_pd3dDevice->SynchronizeToPresentationInterval();
    m_pd3dDevice->Swap( m_pFrontBuffer[m_frame], NULL );

    return S_OK;
}


//--------------------------------------------------------------------------------------
// Name: RenderScene()
// Desc: Render the world objects and lights
//--------------------------------------------------------------------------------------
HRESULT Orb::RenderScene()
{
//	HRESULT hr = S_OK;

    LPDIRECT3DSURFACE9 pOldRenderTarget;
    m_pd3dDevice->GetRenderTarget( 0, &pOldRenderTarget );

    LPDIRECT3DSURFACE9 pRenderTarget;
	m_pd3dDevice->CreateRenderTarget( m_d3dpp.BackBufferWidth, m_d3dpp.BackBufferHeight, D3DFMT_A16B16G16R16F,
                                      D3DMULTISAMPLE_NONE, 0, FALSE,
                                      &pRenderTarget, NULL );
	m_pd3dDevice->SetRenderTarget( 0, pRenderTarget );
	SAFE_RELEASE( pRenderTarget );

    // Begin tiling
    D3DXVECTOR4 ClearColor( 0, 0, 0, 0 );
    m_pd3dDevice->BeginTiling( 0, m_dwNumTiles, m_TilingRects, (D3DVECTOR4*)&ClearColor, 1.0f, 0L );

	RenderSkyBox();

    m_pd3dDevice->SetSamplerState( 0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP );
    m_pd3dDevice->SetSamplerState( 0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP );

    m_pd3dDevice->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );
    m_pd3dDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );

	m_pd3dDevice->SetRenderState( D3DRS_ZENABLE, TRUE );

    m_pd3dDevice->SetVertexDeclaration( m_pOrbVtxDecl );
	m_pd3dDevice->SetVertexShader( m_pSceneVS );
    m_pd3dDevice->SetPixelShader( m_pScenePS );

    m_pd3dDevice->SetVertexShaderConstantF( VSCONST_g_mWorldViewProj, (FLOAT*)&m_matWVP, 4 );
    m_pd3dDevice->SetVertexShaderConstantF( VSCONST_mWorldView, (FLOAT*)&g_mCurModelWV, 4 );

    m_pd3dDevice->SetVertexShaderConstantF( VSCONST_mWorld, (FLOAT*)&m_matWorldT, 4 );

	XMMATRIX m_matVP = XMMatrixTranspose( mWorldInv );
    m_pd3dDevice->SetVertexShaderConstantF( VSCONST_mProjection, (FLOAT*)&m_matVP, 4 );

    m_pd3dDevice->SetVertexShaderConstantF( VSCONST_mEyePt, (FLOAT*)&mEyePtInv, 1 );
    m_pd3dDevice->SetPixelShaderConstantF( PSCONST_mEyePt, (FLOAT*)&mEyePtInv, 1 );

    m_pd3dDevice->SetVertexShaderConstantF( VSCONST_mYuragi0, (FLOAT*)&g_mYuragi0, 4 );
    m_pd3dDevice->SetVertexShaderConstantF( VSCONST_mYuragi1, (FLOAT*)&g_mYuragi1, 4 );
    m_pd3dDevice->SetPixelShaderConstantF( VSCONST_mYuragi0, (FLOAT*)&g_mYuragi0, 4 );
    m_pd3dDevice->SetVertexShaderConstantF( VSCONST_Count, (FLOAT*)&kussetu, 1 );

	m_pd3dDevice->SetPixelShaderConstantF( PSCONST_perR, (FLOAT*)&alpha, 1 );
    m_pd3dDevice->SetPixelShaderConstantF( PSCONST_SPEQULAR, (FLOAT*)&af, 1 );
    m_pd3dDevice->SetPixelShaderConstantF( PSCONST_Time, (FLOAT*)&fTime, 1 );
	m_pd3dDevice->SetPixelShaderConstantF( PSCONST_COLOR, (FLOAT*)&g_color, 1 );

	m_pd3dDevice->SetTexture( 0, m_pTexSkyBox );
	m_pd3dDevice->SetTexture( 1, m_pTexSea );
	m_pd3dDevice->SetTexture( 2, m_pTexVolume );
	m_pd3dDevice->SetTexture( 3, m_pTexUroko );
    m_pd3dDevice->SetSamplerState( 2, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );
    m_pd3dDevice->SetSamplerState( 2, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
    m_pd3dDevice->SetSamplerState( 3, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );
    m_pd3dDevice->SetSamplerState( 3, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );

	for(int i = 0; i < 6; i++)
	{
		m_pd3dDevice->SetTexture( 4 + i, m_pTexInner[i] );
		m_pd3dDevice->SetSamplerState( 4 + i, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );
		m_pd3dDevice->SetSamplerState( 4 + i, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
		m_pd3dDevice->SetSamplerState( 4 + i, D3DSAMP_ADDRESSU, D3DTADDRESS_BORDER );
		m_pd3dDevice->SetSamplerState( 4 + i, D3DSAMP_ADDRESSV, D3DTADDRESS_BORDER );
	}

	FLOAT cos45 = sqrt(2.0f)*0.5f;
/*	XMVECTOR sampleVector[6] = {
		XMVectorSet(  1.0f,  0.0f,   0.0f, 1.0f),
		XMVectorSet(  0.0f,  1.0f,   0.0f, 1.0f),
		XMVectorSet(  0.0f,  0.0f,   1.0f, 1.0f),
		XMVectorSet( -1.0f,  0.0f,   0.0f, 1.0f),
		XMVectorSet(  0.0f, -1.0f,   0.0f, 1.0f),
		XMVectorSet(  0.0f,  0.0f,  -1.0f, 1.0f),
	};*/
	XMVECTOR sampleVector[6] = {
		XMVectorSet(  cos45,  cos45,   0.0f, 1.0f),
		XMVectorSet(  0.0f,    0.0f,   1.0f, 1.0f),
		XMVectorSet( -cos45,  cos45,   0.0f, 1.0f),
		XMVectorSet( -cos45, -cos45,   0.0f, 1.0f),
		XMVectorSet(  0.0f,    0.0f,  -1.0f, 1.0f),
		XMVectorSet(  cos45, -cos45,   0.0f, 1.0f),
	};

	FLOAT SceneAlpha[6];
	XMVECTOR normal = mEyePtInv / sqrt( mEyePtInv.x*mEyePtInv.x + mEyePtInv.y*mEyePtInv.y + mEyePtInv.z*mEyePtInv.z );
	for(int i = 0; i < 6; i++)
	{
		SceneAlpha[i] = pow(max(Dot( &normal, &sampleVector[i] ), 0.0f), 4);
	}
    m_pd3dDevice->SetPixelShaderConstantF( PSCONST_SceneAlpha, (FLOAT*)&SceneAlpha, 2 );

	g_pMesh->DrawSubset(0);

	RenderParticles();

    // End tiling, which resolves the render target to the texture
     m_pd3dDevice->EndTiling( D3DRESOLVE_RENDERTARGET0 | D3DRESOLVE_ALLFRAGMENTS | 
                              D3DRESOLVE_CLEARRENDERTARGET | D3DRESOLVE_CLEARDEPTHSTENCIL, 
                              NULL, g_pTexSceneL, (D3DVECTOR4*)&ClearColor, 1.0f, 0L, NULL );

	//m_pd3dDevice->EndTiling(NULL, NULL, g_pTexSceneL, (D3DVECTOR4*)&ClearColor, 1.0f, 0L, NULL );
	m_pd3dDevice->SetRenderTarget( 0, pOldRenderTarget );
	SAFE_RELEASE( pOldRenderTarget );

    return S_OK;
}

//--------------------------------------------------------------------------------------
// This function loads the mesh and ensures the mesh has normals; it also optimizes the 
// mesh for the graphics card's vertex cache, which improves performance by organizing 
// the internal triangle list for less cache misses.
//--------------------------------------------------------------------------------------
HRESULT Orb::LoadMesh( IDirect3DDevice9* pd3dDevice, LPCSTR strFileName, ID3DXMesh** ppMesh )
{
    ID3DXMesh* pMesh = NULL;
    HRESULT hr;

    V( D3DXLoadMeshFromX(strFileName, D3DXMESH_MANAGED, pd3dDevice, NULL, NULL, NULL, NULL, &pMesh) );

    DWORD *rgdwAdjacency = NULL;

    // Make sure there are normals which are required for lighting
    if( !(pMesh->GetFVF() & D3DFVF_NORMAL) )
    {
        ID3DXMesh* pTempMesh;
        V( pMesh->CloneMeshFVF( pMesh->GetOptions(), 
                                  pMesh->GetFVF() | D3DFVF_NORMAL, 
                                  pd3dDevice, &pTempMesh ) );
        V( D3DXComputeNormals( pTempMesh, NULL ) );

        SAFE_RELEASE( pMesh );
        pMesh = pTempMesh;
    }

    // Optimize the mesh for this graphics card's vertex cache 
    // so when rendering the mesh's triangle list the vertices will 
    // cache hit more often so it won't have to re-execute the vertex shader 
    // on those vertices so it will improve perf.     
    rgdwAdjacency = new DWORD[pMesh->GetNumFaces() * 3];
    if( rgdwAdjacency == NULL )
        return E_OUTOFMEMORY;
    V( pMesh->GenerateAdjacency(1e-6f,rgdwAdjacency) );
    V( pMesh->OptimizeInplace(D3DXMESHOPT_VERTEXCACHE, rgdwAdjacency, NULL, NULL, NULL) );
    delete []rgdwAdjacency;

    *ppMesh = pMesh;

    return S_OK;
}


VOID Orb::RenderParticles()
{
    static XMVECTOR vUp = XMVectorSet( 0.0f, 1.0f, 0.0f, 0.0f );
	XMMATRIX matVP = XMMatrixMultiply( m_matView, m_matProj );
	XMVECTOR det;

//	m_pd3dDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );
//	m_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
	m_pd3dDevice->SetVertexShader( m_pParticlesVS );
	m_pd3dDevice->SetPixelShader( m_pParticlesPS );
	for(int i = 0; i < NUM_PARTICLES; i++)
	{
		XMVECTOR v = XMVectorSet( 0.0f, 200.0f, fEyeR, 1.0f );
		XMMATRIX matP = XMMatrixLookAtLH( m_vParticlesPos[i], v, vUp );
		XMMATRIX m = XMMatrixScaling( 0.1f, 0.1f, 0.1f );
		matP = XMMatrixInverse( &det, matP );
		matP = XMMatrixMultiply( matP, m );
		m = XMMatrixTranslation( 0.0f, 200.0f, 0.0f );
		matP = XMMatrixMultiply( matP, m );
		m = XMMatrixLookAtLH( v, vLookatPt, vUp );
		matP = XMMatrixMultiply( matP, m );
		matP = XMMatrixMultiplyTranspose( matP, m_matProj );
/*		XMMATRIX m = XMMatrixTranslation( m_vParticlesPos[i].x, m_vParticlesPos[i].y + 200.0f, m_vParticlesPos[i].z );
		m = XMMatrixMultiplyTranspose( m, matVP );*/
		m_pd3dDevice->SetVertexShaderConstantF( VSCONST_g_mWorldViewProj, (FLOAT*)&matP, 4 );
		m_pd3dDevice->SetTexture( 0, g_pTexMadara );
//		m_pd3dDevice->SetVertexDeclaration( m_pSkyBoxDecl );

//		m_pd3dDevice->SetStreamSource( 0, m_pVB, 0, sizeof(XMFLOAT4) );
//		m_pd3dDevice->DrawPrimitive( D3DPT_RECTLIST, 0, 1 );
		g_pPointMesh->DrawSubset(0);
	}
//	m_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
//	m_pd3dDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_CCW );
}

//--------------------------------------------------------------------------------------
// Name: MeasureLuminance()
// Desc: Measure the average log luminance in the scene.
//--------------------------------------------------------------------------------------
HRESULT Orb::MeasureLuminance()
{
    // After this pass, the m_pToneMapTexture texture will contain a single-pixel 
    // grayscale value of the log of average luminance for the HDR scene.

    // Sample initial luminance
    m_PostProcess.SampleLuminance( m_pScaledSceneTexture, TRUE, m_pToneMapTexture64x64 );

    // Downsample to 16x16
    m_PostProcess.Downsample4x4Texture( m_pToneMapTexture64x64, m_pToneMapTexture16x16 );

    // Downsample to 4x4
    m_PostProcess.Downsample4x4Texture( m_pToneMapTexture16x16, m_pToneMapTexture4x4 );

    // Downsample to 1x1
    m_PostProcess.SampleLuminance( m_pToneMapTexture4x4, FALSE, m_pToneMapTexture1x1 );

    return S_OK;
}


//--------------------------------------------------------------------------------------
// Name: RenderBloom()
// Desc: Render the blooming effect
//--------------------------------------------------------------------------------------
HRESULT Orb::RenderBloom()
{
    if( m_GlareDef.m_fGlareLuminance <= 0.0f || m_GlareDef.m_fBloomLuminance <= 0.0f )
    {
        m_PostProcess.ClearTexture( m_pBloomTexture );
        return S_OK;
    }

    // Render to first bloom texture (Gaussian blur 5x5 m_pBloomSourceTexture to m_pBloomTexture)
    m_PostProcess.GaussBlur5x5Texture( m_pBloomSourceTexture, m_pBloomTexture );

    // Render to second bloom texture across width (m_pBloomTexture to m_pBloomTexture)
    m_PostProcess.BloomTexture( m_pBloomTexture, TRUE, m_pBloomTexture );

    // Render to final bloom texture and height (m_pBloomTexture to m_pBloomTexture)
    m_PostProcess.BloomTexture( m_pBloomTexture, FALSE, m_pBloomTexture );
    
    return S_OK;
}


//--------------------------------------------------------------------------------------
// Name: RenderStar()
// Desc: Render the blooming effect
//--------------------------------------------------------------------------------------
HRESULT Orb::RenderStar()
{
    // Clear the star texture
    m_PostProcess.ClearTexture( m_apStarTextures[0] );

    // Avoid rendering the star if it's not being used in the current glare
    if( m_GlareDef.m_fGlareLuminance <= 0.0f || m_GlareDef.m_fStarLuminance <= 0.0f )
        return S_OK;

    // Initialize the constants used during the effect
    const        CStarDef&   starDef = m_GlareDef.m_starDef;
    const        FLOAT       fTanFoV = atanf(XM_PI/8);
    static const DWORD       MAX_PASSES = 3;
    static const DWORD       NUM_SAMPLES = 8;
    static       XMVECTOR    s_aaColor[MAX_PASSES][8];
    static const XMVECTOR    COLOR_WHITE = XMVectorSet( 0.63f, 0.63f, 0.63f, 0.0f );
    
    m_pd3dDevice->SetRenderState( D3DRS_SRCBLEND,  D3DBLEND_ONE );
    m_pd3dDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ONE );

    // Get the source texture dimensions
    D3DSURFACE_DESC desc;
    m_pStarSourceTexture->GetLevelDesc( 0, &desc );
    FLOAT fTextureWidth  = (FLOAT)desc.Width;
    FLOAT fTextureHeight = (FLOAT)desc.Height;
   
    for( DWORD p=0; p<MAX_PASSES; p++ )
    {
        FLOAT fRatio = (FLOAT)(p+1) / MAX_PASSES;
        
        for( DWORD s=0; s<NUM_SAMPLES; s++ )
        {
            XMVECTOR ChromaticAberrColor = XMLoadFloat4(&starDef.m_avChromaticAberrationColor[s]);
            ChromaticAberrColor = XMVectorLerp( ChromaticAberrColor, COLOR_WHITE,
                                                fRatio );

            s_aaColor[p][s] = XMVectorLerp( COLOR_WHITE, ChromaticAberrColor, 
                                            m_GlareDef.m_fChromaticAberration );
        }
    }

    FLOAT radOffset = m_GlareDef.m_fStarInclination + starDef.m_fInclination;

    // Direction loop
    for( DWORD d=0; d<starDef.m_dwNumStarLines; d++ )
    {
        CONST CStarDef::STARLINE& starLine = starDef.m_pStarLine[d];

        LPDIRECT3DTEXTURE9 pWorkTexture = m_pStarSourceTexture;
        
        FLOAT rad = radOffset + starLine.fInclination;
        FLOAT fStepU = sinf(rad) / fTextureWidth  * starLine.fSampleLength;
        FLOAT fStepV = cosf(rad) / fTextureHeight * starLine.fSampleLength;
        
        FLOAT fAttnPowScale = (fTanFoV + 0.1f) * 1.0f * (160.0f + 120.0f) / (fTextureWidth + fTextureHeight) * 1.2f;

        // 1 direction expansion loop
        m_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
        
        int iWorkTexture = 1;
        for( DWORD p=0; p<starLine.dwNumPasses; p++ )
        {
            LPDIRECT3DTEXTURE9 pDstTexture;

            if( p == starLine.dwNumPasses - 1 )
                pDstTexture = m_apStarTextures[d+4];
            else 
                pDstTexture = m_apStarTextures[iWorkTexture];

            m_PostProcess.RenderStarLine( pWorkTexture, NUM_SAMPLES,
                                          starLine.fAttenuation, fAttnPowScale,
                                          s_aaColor[starLine.dwNumPasses-1-p], p,
                                          fStepU, fStepV, pDstTexture );
        
            // Setup next expansion
            fStepU        *= NUM_SAMPLES;
            fStepV        *= NUM_SAMPLES;
            fAttnPowScale *= NUM_SAMPLES;

            // Set the work drawn just before to next texture source.
            pWorkTexture = m_apStarTextures[iWorkTexture];

            if( ++iWorkTexture > 2 ) 
                iWorkTexture = 1;
        }
    }

    m_PostProcess.MergeTextures( &m_apStarTextures[4], starDef.m_dwNumStarLines, m_apStarTextures[0] );

    return S_OK;
}

HRESULT Orb::RenderRadial()
{
    LPDIRECT3DSURFACE9 pOldRenderTarget;
    m_pd3dDevice->GetRenderTarget( 0, &pOldRenderTarget );

	LPDIRECT3DSURFACE9 pRenderTarget;
	m_pd3dDevice->CreateRenderTarget( m_d3dpp.BackBufferWidth, m_d3dpp.BackBufferHeight, D3DFMT_A8R8G8B8,
                                      D3DMULTISAMPLE_NONE, 0, FALSE,
                                      &pRenderTarget, NULL );
	m_pd3dDevice->SetRenderTarget( 0, pRenderTarget );
	SAFE_RELEASE( pRenderTarget );

	m_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER, 0x00000000, 1.0f, 0 );
    m_pd3dDevice->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );
    m_pd3dDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
	m_pd3dDevice->SetTexture( 0, m_pStarSourceTexture );
    m_pd3dDevice->SetPixelShader( m_pRadialPS );

	m_PostProcess.DrawFullScreenQuad();

	m_pd3dDevice->Resolve( D3DRESOLVE_RENDERTARGET0, NULL, g_pTexRadial, 
                            NULL, 0, 0, NULL, 0, 0, NULL );

	m_pd3dDevice->SetRenderTarget( 0, pOldRenderTarget );
	SAFE_RELEASE( pOldRenderTarget );

	return S_OK;
}

VOID Orb::RenderSkyBox()
{
	m_pd3dDevice->SetPixelShader( m_pSkyBoxPS );
	m_pd3dDevice->SetPixelShaderConstantF( 0, (FLOAT*)&fTime, 1 );
	m_pd3dDevice->SetPixelShaderConstantF( 1, (FLOAT*)&g_color, 1 );
	m_pd3dDevice->SetTexture( 0, m_pBackground );
	m_pd3dDevice->SetSamplerState( 0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP );
    m_pd3dDevice->SetSamplerState( 0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP );
    m_pd3dDevice->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );
    m_pd3dDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
	m_PostProcess.DrawFullScreenQuad();
}


FLOAT Orb::Dot( XMVECTOR* v1, XMVECTOR* v2 )
{
	return v1->x * v2->x + v1->y * v2->y + v1->z * v2->z;
}

VOID Orb::Downsample3x3Texture( LPDIRECT3DTEXTURE9 pSrcTexture,
                                        LPDIRECT3DTEXTURE9 pDstTexture )
{

    XGTEXTURE_DESC SrcDesc;
    XGGetTextureDesc( pSrcTexture, 0, &SrcDesc );

    // Get the sample offsets used within the pixel shader
    XMVECTOR avSampleOffsets[MAX_SAMPLES];
    FLOAT tu = 1.0f / (FLOAT)SrcDesc.Width;
    FLOAT tv = 1.0f / (FLOAT)SrcDesc.Height;

    // Sample from the 9 surrounding points. Since the center point will be in the exact
    // center of 4 texels, a 1.0f offset is needed to specify a texel center.
	int i = 0;
    for( int x = 0; x < 4; x++ )
    {
        for( int y = 0; y < 1; y++ )
        {
            avSampleOffsets[i].x = (x-1.5f) * tu;
            avSampleOffsets[i].y = 0.0f;
            i++;
        }
    }
//    GetSampleOffsets_DownScale3x3( SrcDesc.Width, SrcDesc.Height, avSampleOffsets );
    m_pd3dDevice->SetPixelShaderConstantF( PSCONST_avSampleOffsets, (FLOAT*)avSampleOffsets, MAX_SAMPLES );
    m_pd3dDevice->SetPixelShader( m_pPS );
    
    m_pd3dDevice->SetTexture( 0, pSrcTexture );
    m_pd3dDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_POINT );
    m_pd3dDevice->SetSamplerState( 0, D3DSAMP_ADDRESSU,  D3DTADDRESS_CLAMP );
    m_pd3dDevice->SetSamplerState( 0, D3DSAMP_ADDRESSV,  D3DTADDRESS_CLAMP );
   
    // Draw a fullscreen quad
    m_PostProcess.DrawFullScreenQuad();

    m_pd3dDevice->Resolve( D3DRESOLVE_RENDERTARGET0, NULL, pDstTexture, NULL,
                           0, 0, NULL, 1.0f, 0L, NULL );
}

void Orb::Release()
{
	SAFE_RELEASE(m_pFrontBuffer[0]);
	SAFE_RELEASE(m_pFrontBuffer[1]);
	SAFE_RELEASE(pSurfRT);

	SAFE_RELEASE(m_pSceneVS);
	SAFE_RELEASE(m_pSkyBoxVS);
	SAFE_RELEASE(m_pParticlesVS);

	SAFE_RELEASE(m_pFinalScenePassPS);
	SAFE_RELEASE(m_pScenePS);
	SAFE_RELEASE(m_pSkyBoxPS);
	SAFE_RELEASE(m_pParticlesPS);
	SAFE_RELEASE(m_pPS);
	SAFE_RELEASE(m_pRadialPS);
	SAFE_RELEASE(m_pFinalPS);

	SAFE_RELEASE(m_pWhiteTexture);                // A blank texture
	SAFE_RELEASE(m_pSceneTexture);                // HDR texture containing the scene
	SAFE_RELEASE(m_pScaledSceneTexture);          // Scaled copy of the HDR scene
	SAFE_RELEASE(m_pBrightPassTexture);           // Bright-pass filtered copy of the scene
	SAFE_RELEASE(m_pAdaptedLuminanceTexture);     // The luminance the user is adapted to
	SAFE_RELEASE(m_pStarSourceTexture);           // Star effect source texture
	SAFE_RELEASE(m_pBloomSourceTexture);          // Bloom effect source texture
	SAFE_RELEASE(m_pBloomTexture);                // Blooming effect texture
	for(int i = 0; i < NUM_STAR_TEXTURES; i++) //aleksger: prefix bug 672: Improper freeing of textures
		SAFE_RELEASE(m_apStarTextures[i]);		  // Star effect working textures
	SAFE_RELEASE(m_pToneMapTexture64x64);         // Average luminance samples from the HDR render target
	SAFE_RELEASE(m_pToneMapTexture16x16);         // Average luminance samples from the HDR render target
	SAFE_RELEASE(m_pToneMapTexture4x4);           // Average luminance samples from the HDR render target
	SAFE_RELEASE(m_pToneMapTexture1x1);           // Average luminance samples from the HDR render target
	SAFE_RELEASE(m_pBackground);
	for(int i = 0; i < INNER_TEXTURES; i++)
		SAFE_RELEASE(m_pTexInner[i]);//aleksger: prefix bug 672: Improper freeing of textures
	SAFE_RELEASE(m_pTexRogo);
	SAFE_RELEASE(m_pTexUroko);
	SAFE_RELEASE(g_pTexRadial);

	SAFE_RELEASE(g_pTexScene);            // HDR render target containing the scene
	SAFE_RELEASE(g_pTexSceneL);            // HDR render target containing the scene
	SAFE_RELEASE(g_pTexFinalScene);
	SAFE_RELEASE(g_pTexMadara);
	SAFE_RELEASE(m_pTexSkyBox);
	SAFE_RELEASE(m_pTexVolume);
	SAFE_RELEASE(m_pTexSea);

	SAFE_RELEASE(g_pMesh);			// クリス?ル用メッシュ
	SAFE_RELEASE(g_pPointMesh);		// 不純物用メッシュ
	SAFE_RELEASE(m_pVB);
}
#endif//XBOX