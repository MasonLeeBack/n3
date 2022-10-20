//--------------------------------------------------------------------------------------
// AtgPostProcess.cpp
//
// Commonly used post-processing effects (like bloom, blur, etc.)
//
// Xbox Advanced Technology Group.
// Copyright (C) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

#include "stdafx.h"
#include "AtgPostProcess.h"
#include "AtgUtil.h"
#include "BsKernel.h"

extern LPDIRECT3DDEVICE9 g_pd3dDevice;

namespace ATG
{
	#ifdef _XBOX
	//-----------------------------------------------------------------------------
	// Name: CreateRenderTarget()
	// Desc: Helper function to set a render target for a texture
	//-----------------------------------------------------------------------------
	inline LPDIRECT3DSURFACE9 CreateRenderTarget( const LPDIRECT3DTEXTURE9 pTexture,
		D3DSURFACE_PARAMETERS* pSurfaceParams = NULL )
	{
		XGTEXTURE_DESC Desc;
		XGGetTextureDesc( pTexture, 0, &Desc );

		static D3DSURFACE_PARAMETERS SurfaceParams = {0};
		if( NULL == pSurfaceParams )
			pSurfaceParams = &SurfaceParams;

		// Some texture formats can't be used as render target formats.  Special
		// case the ones we use
		if (Desc.Format == D3DFMT_A16B16G16R16F_EXPAND)
			Desc.Format = D3DFMT_A16B16G16R16F;

		LPDIRECT3DSURFACE9 pRenderTarget;
		g_pd3dDevice->CreateRenderTarget( Desc.Width, Desc.Height, Desc.Format,
			D3DMULTISAMPLE_NONE, 0, FALSE,
			&pRenderTarget, pSurfaceParams );
		return pRenderTarget;
	}


	//-----------------------------------------------------------------------------
	// Name: PushRenderTarget()/PopRenderTarget()
	// Desc: Helper functions for setting/restoring render targets
	//-----------------------------------------------------------------------------
	extern std::stack<LPDIRECT3DSURFACE9> g_pRenderTargetStack;


	// Pushes a render target onto a stack of saved states
	inline VOID PushRenderTarget( DWORD dwRenderTargetID,
		const LPDIRECT3DSURFACE9 pNewRenderTarget = NULL )
	{
		LPDIRECT3DSURFACE9 pOldRenderTarget;
		g_pd3dDevice->GetRenderTarget( dwRenderTargetID, &pOldRenderTarget );
		if( pNewRenderTarget )
			g_pd3dDevice->SetRenderTarget( dwRenderTargetID, pNewRenderTarget );

		g_pRenderTargetStack.push( pOldRenderTarget );
		g_pRenderTargetStack.push( pNewRenderTarget );
	}


	// Pops a saved render target from a stack of saved states
	inline LPDIRECT3DSURFACE9 PopRenderTarget( DWORD dwRenderTargetID )
	{
		LPDIRECT3DSURFACE9 pNewRenderTarget = g_pRenderTargetStack.top(); g_pRenderTargetStack.pop();
		LPDIRECT3DSURFACE9 pOldRenderTarget = g_pRenderTargetStack.top(); g_pRenderTargetStack.pop();

		g_pd3dDevice->SetRenderTarget( dwRenderTargetID, pOldRenderTarget );
		if( pOldRenderTarget )
			pOldRenderTarget->Release();

		return pNewRenderTarget;
	}


	//-----------------------------------------------------------------------------
	// Name: PushRenderState()/PopRenderState()
	// Desc: Helper functions for setting/restoring render targets
	//-----------------------------------------------------------------------------
	struct RENDERSTATE
	{
		D3DRENDERSTATETYPE dwRenderState;
		DWORD              dwValue;
	};

	extern std::stack<RENDERSTATE> g_dwRenderStateStack;

	// Pushes a render state onto a stack of saved states
	inline VOID PushRenderState( DWORD dwRenderState, DWORD dwValue )
	{
		DWORD dwOriginalValue;
		g_pd3dDevice->GetRenderState( (D3DRENDERSTATETYPE)dwRenderState, &dwOriginalValue );

		RENDERSTATE rs;
		rs.dwRenderState = (D3DRENDERSTATETYPE)dwRenderState;
		rs.dwValue       = dwOriginalValue;
		g_dwRenderStateStack.push( rs );

		g_pd3dDevice->SetRenderState( (D3DRENDERSTATETYPE)dwRenderState, dwValue );
	}


	// Pops a saved render state from a stack of saved states
	inline VOID PopRenderStates()
	{
		for( int i=0; !g_dwRenderStateStack.empty(); ++i, g_dwRenderStateStack.pop() )
		{
			RENDERSTATE rs = g_dwRenderStateStack.top();

			g_pd3dDevice->SetRenderState( rs.dwRenderState, rs.dwValue );
		}
	}

//--------------------------------------------------------------------------------------
// Mappings to shader constants that are used in the HLSL shaders
//--------------------------------------------------------------------------------------
const DWORD PSCONST_fMiddleGray         =  5;
const DWORD PSCONST_fElapsedTime        =  7;

const DWORD PSCONST_avSampleOffsets     =  0;
const DWORD PSCONST_avSampleWeights     = 16;


//--------------------------------------------------------------------------------------
// Constants and external variables
//--------------------------------------------------------------------------------------
const DWORD MAX_SAMPLES = 16;      // Maximum number of texture grabs

// A stack of render targets used for convenient pushing/popping of render targets
std::stack<LPDIRECT3DSURFACE9> g_pRenderTargetStack;

// A stack of render states used for convenient pushing/popping of render state
std::stack<RENDERSTATE> g_dwRenderStateStack;


//--------------------------------------------------------------------------------------
// Name: GaussianDistribution()
// Desc: Computes a two-parameter (x,y) Gaussian distrubution using the given
//       standard deviation (rho)
//--------------------------------------------------------------------------------------
inline FLOAT GaussianDistribution( FLOAT x, FLOAT y, FLOAT rho )
{
    return expf( -(x*x + y*y)/(2*rho*rho) ) / sqrtf( 2*XM_PI*rho*rho );
}


//--------------------------------------------------------------------------------------
// Name: GetSampleOffsets_DownScale4x4()
// Desc: Get the texcoord offsets to be used inside the DownScale4x4 pixel shader.
//--------------------------------------------------------------------------------------
VOID PostProcess::GetSampleOffsets_DownScale4x4( DWORD dwTexWidth, DWORD dwTexHeight,
                                                 XMVECTOR* pvSampleOffsets )
{
    FLOAT tu = 1.0f / (FLOAT)dwTexWidth;
    FLOAT tv = 1.0f / (FLOAT)dwTexHeight;

    // Sample from the 16 surrounding points. Since the center point will be in the
    // exact center of 16 texels, a 1.5f offset is needed to specify a texel center.
    for( int y = 0; y < 4; y++ )
    {
        for( int x = 0; x < 4; x++ )
        {
            pvSampleOffsets->x = ((FLOAT)x-1.5f) * tu;
            pvSampleOffsets->y = ((FLOAT)y-1.5f) * tv;
            pvSampleOffsets++;
        }
    }
}


//--------------------------------------------------------------------------------------
// Name: GetSampleOffsets_DownScale3x3()
// Desc: Get the texcoord offsets to be used inside the DownScale3x3 pixel shader.
//--------------------------------------------------------------------------------------
VOID PostProcess::GetSampleOffsets_DownScale3x3( DWORD dwTexWidth, DWORD dwTexHeight,
                                                 XMVECTOR* pvSampleOffsets )
{
    FLOAT tu = 1.0f / (FLOAT)dwTexWidth;
    FLOAT tv = 1.0f / (FLOAT)dwTexHeight;

    // Sample from the 9 surrounding points. Since the center point will be in the exact
    // center of 4 texels, a 1.0f offset is needed to specify a texel center.
    for( int x = 0; x < 3; x++ )
    {
        for( int y = 0; y < 3; y++ )
        {
            pvSampleOffsets->x = (x-1.0f) * tu;
            pvSampleOffsets->y = (y-1.0f) * tv;
            pvSampleOffsets++;
        }
    }
}


//--------------------------------------------------------------------------------------
// Name: GetSampleOffsets_DownScale2x2()
// Desc: Get the texcoord offsets to be used inside the DownScale2x2 pixel shader.
//--------------------------------------------------------------------------------------
VOID PostProcess::GetSampleOffsets_DownScale2x2( DWORD dwTexWidth, DWORD dwTexHeight,
                                                 XMVECTOR* pvSampleOffsets )
{
    FLOAT tu = 1.0f / (FLOAT)dwTexWidth;
    FLOAT tv = 1.0f / (FLOAT)dwTexHeight;

    // Sample from the 4 surrounding points. Since the center point will be in the exact
    // center of 4 texels, a 0.5f offset is needed to specify a texel center.
    for( int y = 0; y < 2; y++ )
    {
        for( int x = 0; x < 2; x++ )
        {
            pvSampleOffsets->x = ((FLOAT)x-0.5f) * tu;
            pvSampleOffsets->y = ((FLOAT)y-0.5f) * tv;
            pvSampleOffsets++;
        }
    }
}


//--------------------------------------------------------------------------------------
// Name: GetSampleOffsets_GaussBlur5x5()
// Desc: Get the texcoord offsets to be used inside the GaussBlur5x5 pixel shader.
//--------------------------------------------------------------------------------------
VOID PostProcess::GetSampleOffsets_GaussBlur5x5( DWORD dwTexWidth, DWORD dwTexHeight,
                                                 XMVECTOR* pvTexCoordOffsets,
                                                 XMVECTOR* pvSampleWeights,
                                                 FLOAT fMultiplier )
{
    FLOAT tu = 1.0f / (FLOAT)dwTexWidth;
    FLOAT tv = 1.0f / (FLOAT)dwTexHeight;

    XMVECTOR vWhite = XMVectorSet( 1.0f, 1.0f, 1.0f, 1.0f );
    
    FLOAT fTotalWeight = 0.0f;
    DWORD index=0;
    for( int x = -2; x <= 2; x++ )
    {
        for( int y = -2; y <= 2; y++ )
        {
            // Exclude pixels with a block distance greater than 2. This will
            // create a kernel which approximates a 5x5 kernel using only 13
            // sample points instead of 25; this is necessary since 2.0 shaders
            // only support 16 texture grabs.
            if( fabs((FLOAT)x) + fabs((FLOAT)y) > 2.0f )
                continue;

            // Get the unscaled Gaussian intensity for this offset
            pvTexCoordOffsets[index].x = (FLOAT)x * tu;
            pvTexCoordOffsets[index].y = (FLOAT)y * tv;
            pvTexCoordOffsets[index].z = 0.0f;
            pvTexCoordOffsets[index].w = 0.0f;
            
            pvSampleWeights[index]     = vWhite * GaussianDistribution( (FLOAT)x, (FLOAT)y, 1.0f );
            
            fTotalWeight += pvSampleWeights[index].x;

            index++;
        }
    }

    // Divide the current weight by the total weight of all the samples; Gaussian
    // blur kernels add to 1.0f to ensure that the intensity of the image isn't
    // changed when the blur occurs. An optional multiplier variable is used to
    // add or remove image intensity during the blur.
    for( DWORD i=0; i<index; i++ )
    {
        pvSampleWeights[i] /= fTotalWeight;
        pvSampleWeights[i] *= fMultiplier;
    }
}


//--------------------------------------------------------------------------------------
// Name: GetSampleOffsets_Bloom()
// Desc: Get the texcoord offsets to be used inside the Bloom pixel shader.
//--------------------------------------------------------------------------------------
VOID PostProcess::GetSampleOffsets_Bloom( DWORD dwTexSize, XMVECTOR* pvTexCoordOffsets,
                                          XMVECTOR* pvColorWeights, FLOAT fDeviation,
                                          FLOAT fMultiplier )
{
    FLOAT tu = 1.0f / (FLOAT)dwTexSize;

    // Fill the center texel
    FLOAT fWeight = fMultiplier * GaussianDistribution( 0, 0, fDeviation );
    pvColorWeights[0]    = XMVectorSet( fWeight, fWeight, fWeight, 1.0f );
    pvTexCoordOffsets[0] = XMVectorSet( 0.0f, 0.0f, 0.0f, 0.0f );
    
    // Fill the first half
    for( DWORD i=1; i<8; i++ )
    {
        // Get the Gaussian intensity for this offset
        fWeight = fMultiplier * GaussianDistribution( (FLOAT)i, 0, fDeviation );
        pvColorWeights[i]    = XMVectorSet( fWeight, fWeight, fWeight, 1.0f );
        pvTexCoordOffsets[i] = XMVectorSet( i * tu, 0.0f, 0.0f, 0.0f );
    }

    // Mirror to the second half
    for( DWORD i=8; i<15; i++ )
    {
        pvColorWeights[i]    =  pvColorWeights[i-7];
        pvTexCoordOffsets[i] = -pvTexCoordOffsets[i-7];
    }
}


//--------------------------------------------------------------------------------------
// Name: GetSampleOffsets_Star()
// Desc: Get the texcoord offsets to be used inside the Star pixel shader.
//--------------------------------------------------------------------------------------
VOID PostProcess::GetSampleOffsets_Star( DWORD dwTexSize, XMVECTOR* pvTexCoordOffsets,
                                         XMVECTOR* pvColorWeights, FLOAT fDeviation )
{
    FLOAT tu = 1.0f / (FLOAT)dwTexSize;

    // Fill the center texel
    FLOAT fWeight = 1.0f * GaussianDistribution( 0, 0, fDeviation );
    pvColorWeights[0]    = XMVectorSet( fWeight, fWeight, fWeight, 1.0f );
    pvTexCoordOffsets[0] = XMVectorSet( 0.0f, 0.0f, 0.0f, 0.0f );
    
    // Fill the first half
    for( DWORD i=1; i<8; i++ )
    {
        // Get the Gaussian intensity for this offset
        fWeight = 1.0f * GaussianDistribution( (FLOAT)i, 0, fDeviation );
        pvColorWeights[i]    = XMVectorSet( fWeight, fWeight, fWeight, 1.0f );
        pvTexCoordOffsets[i] = XMVectorSet( i * tu, 0.0f, 0.0f, 0.0f );
    }

    // Mirror to the second half
    for( DWORD i=8; i<15; i++ )
    {
        pvColorWeights[i]    =  pvColorWeights[i-7];
        pvTexCoordOffsets[i] = -pvTexCoordOffsets[i-7];
    }
}


//-----------------------------------------------------------------------------
// Name: ClearTexture()
// Desc: Helper function for RestoreDeviceObjects to clear a texture surface
//-----------------------------------------------------------------------------
VOID PostProcess::ClearTexture( LPDIRECT3DTEXTURE9 pTexture, DWORD dwClearColor )
{
    // Make sure that the required shaders and objects exist
    assert( pTexture );

    // Create and set a render target for the texture
    PushRenderTarget( 0, CreateRenderTarget( pTexture ) );

    g_pd3dDevice->Clear( 0L, NULL, D3DCLEAR_TARGET, dwClearColor, 1.0f, 0L );

    g_pd3dDevice->Resolve( D3DRESOLVE_RENDERTARGET0, NULL, pTexture, NULL,
                           0, 0, NULL, 0.0f, 0, NULL );

    // Cleanup and exit
    PopRenderTarget( 0 )->Release();
}


//--------------------------------------------------------------------------------------
// Name: DrawScreenSpaceQuad()
// Desc: Draw a viewport-aligned quad using screen-space coordinates
//--------------------------------------------------------------------------------------
VOID PostProcess::DrawScreenSpaceQuad( FLOAT w, FLOAT h, FLOAT fMaxU, FLOAT fMaxV )
{
    // Make sure that the required shaders and objects exist
    assert( m_pQuadVtxDecl );
    assert( m_pScreenSpaceVS );

    // Define vertices for the screen-space rect
    XMFLOAT4 v[3];
    v[0] = XMFLOAT4( 0, 0,    0.0f,  0.0f );
    v[1] = XMFLOAT4( w, 0,   fMaxU,  0.0f );
    v[2] = XMFLOAT4( 0, h,    0.0f, fMaxV );

    // Set states for drawing the quad
    PushRenderState( D3DRS_VIEWPORTENABLE,  FALSE );
    PushRenderState( D3DRS_ZENABLE,         FALSE );
    PushRenderState( D3DRS_CULLMODE,        D3DCULL_CCW );
    PushRenderState( D3DRS_HALFPIXELOFFSET, TRUE );

    // Draw the quad
    g_pd3dDevice->SetVertexDeclaration( m_pQuadVtxDecl );
    g_pd3dDevice->SetVertexShader( m_pScreenSpaceVS );
    g_pd3dDevice->DrawPrimitiveUP( D3DPT_RECTLIST, 1, v, sizeof(v[0]) );
    
    PopRenderStates();
}


//--------------------------------------------------------------------------------------
// Name: DrawFullScreenQuad()
// Desc: Draw a viewport-aligned quad covering the entire render target
//--------------------------------------------------------------------------------------
VOID PostProcess::DrawFullScreenQuad()
{
    // Make sure that the required shaders and objects exist
    assert( m_pQuadVtxDecl );
    assert( m_pQuadVB );
    assert( m_pScreenSpaceVS );

    // Set states for drawing the rect
    PushRenderState( D3DRS_VIEWPORTENABLE,  TRUE );
    PushRenderState( D3DRS_ZENABLE,         FALSE );
    PushRenderState( D3DRS_CULLMODE,        D3DCULL_CCW );
    PushRenderState( D3DRS_HALFPIXELOFFSET, TRUE );

    // Draw the rect
    g_pd3dDevice->SetVertexDeclaration( m_pQuadVtxDecl );
    g_pd3dDevice->SetVertexShader( m_pScreenSpaceVS );
    g_pd3dDevice->SetStreamSource( 0, m_pQuadVB, 0, sizeof(XMFLOAT4) );
    g_pd3dDevice->DrawPrimitive( D3DPT_RECTLIST, 0, 1 );
    
    PopRenderStates();
}


//--------------------------------------------------------------------------------------
// Name: BrightPassFilterTexture()
// Desc: Run the bright-pass filter on m_pScaledSceneTexture and place the result
//       in m_pBrightPassTexture
//--------------------------------------------------------------------------------------
VOID PostProcess::BrightPassFilterTexture( LPDIRECT3DTEXTURE9 pSrcTexture,
                                           LPDIRECT3DTEXTURE9 pAdaptedLuminanceTexture, 
                                           FLOAT              fMiddleGrayKeyValue,
                                           LPDIRECT3DTEXTURE9 pDstTexture )
{
    // Make sure that the required shaders and objects exist
    assert( m_pBrightPassFilterPS );
    assert( pSrcTexture && pDstTexture );

    XGTEXTURE_DESC SrcDesc;
    XGGetTextureDesc( pSrcTexture, 0, &SrcDesc );

    // Create and set a render target
    PushRenderTarget( 0L, CreateRenderTarget( pDstTexture ) );

    // Get the offsets to be used within the GaussBlur5x5 pixel shader
    XMVECTOR avSampleOffsets[MAX_SAMPLES];
    XMVECTOR avSampleWeights[MAX_SAMPLES];
    GetSampleOffsets_GaussBlur5x5( SrcDesc.Width, SrcDesc.Height, avSampleOffsets, avSampleWeights );
    g_pd3dDevice->SetPixelShaderConstantF( PSCONST_avSampleOffsets, (FLOAT*)avSampleOffsets, MAX_SAMPLES );
    g_pd3dDevice->SetPixelShaderConstantF( PSCONST_avSampleWeights, (FLOAT*)avSampleWeights, MAX_SAMPLES );
    // mruete: prefix bug 483: storing in a vector for proper usage of SetPixelShaderConstantF
    FLOAT vMiddleGrayKeyValue[4] = { fMiddleGrayKeyValue, 0.f, 0.f, 0.f };
    g_pd3dDevice->SetPixelShaderConstantF( PSCONST_fMiddleGray, vMiddleGrayKeyValue, 1 );
    g_pd3dDevice->SetPixelShader( m_pBrightPassFilterPS );

    g_pd3dDevice->SetTexture( 0, pSrcTexture );
    g_pd3dDevice->SetTexture( 1, pAdaptedLuminanceTexture );
    g_pd3dDevice->SetSamplerState( 0, D3DSAMP_ADDRESSU,  D3DTADDRESS_CLAMP );
    g_pd3dDevice->SetSamplerState( 0, D3DSAMP_ADDRESSV,  D3DTADDRESS_CLAMP );
    g_pd3dDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_POINT );
    g_pd3dDevice->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_POINT );
    g_pd3dDevice->SetSamplerState( 1, D3DSAMP_ADDRESSU,  D3DTADDRESS_CLAMP );
    g_pd3dDevice->SetSamplerState( 1, D3DSAMP_ADDRESSV,  D3DTADDRESS_CLAMP );
    g_pd3dDevice->SetSamplerState( 1, D3DSAMP_MINFILTER, D3DTEXF_POINT );
    g_pd3dDevice->SetSamplerState( 1, D3DSAMP_MAGFILTER, D3DTEXF_POINT );
       
    // Draw a fullscreen quad to sample the RT
    DrawFullScreenQuad();

    g_pd3dDevice->Resolve( D3DRESOLVE_RENDERTARGET0, NULL, pDstTexture, NULL, 
                           0, 0, NULL, 1.0f, 0L, NULL );

    // Cleanup and return
    PopRenderTarget( 0L )->Release();
    g_pd3dDevice->SetPixelShader( NULL );
}


//--------------------------------------------------------------------------------------
// Name: SampleLuminance()
// Desc: Measure the average log luminance in the scene.
//--------------------------------------------------------------------------------------
VOID PostProcess::SampleLuminance( LPDIRECT3DTEXTURE9 pSrcTexture, BOOL bInitial,
                                   LPDIRECT3DTEXTURE9 pDstTexture )
{
    // Make sure that the required shaders and objects exist
    assert( m_pSampleLumInitialPS );
    assert( m_pSampleLumFinalPS );
    assert( pSrcTexture && pDstTexture );
    
    XGTEXTURE_DESC SrcDesc;
    XGGetTextureDesc( pSrcTexture, 0, &SrcDesc );

    // Create and set a render target
    PushRenderTarget( 0L, CreateRenderTarget( pDstTexture ) );

    // Sample initial luminance
    if( bInitial )
    {
        // Initialize the sample offsets for the initial luminance pass.
        XMVECTOR avSampleOffsets[MAX_SAMPLES];
        GetSampleOffsets_DownScale3x3( SrcDesc.Width, SrcDesc.Height, avSampleOffsets );
        g_pd3dDevice->SetPixelShaderConstantF( PSCONST_avSampleOffsets, (FLOAT*)avSampleOffsets, MAX_SAMPLES );
        g_pd3dDevice->SetPixelShader( m_pSampleLumInitialPS );

        g_pd3dDevice->SetTexture( 0, pSrcTexture );
        g_pd3dDevice->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );
        g_pd3dDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
        g_pd3dDevice->SetSamplerState( 0, D3DSAMP_ADDRESSU,  D3DTADDRESS_CLAMP );
        g_pd3dDevice->SetSamplerState( 0, D3DSAMP_ADDRESSU,  D3DTADDRESS_CLAMP );
    }
    else // ( bIntial == FALSE )
    {
        // Perform the final pass of the average luminance calculation. This pass
        // performs an exp() operation to return a single texel cooresponding to the
        // average luminance of the scene in m_pToneMapTexture.

        XMVECTOR avSampleOffsets[MAX_SAMPLES];
        GetSampleOffsets_DownScale4x4( SrcDesc.Width, SrcDesc.Height, avSampleOffsets );
        g_pd3dDevice->SetPixelShaderConstantF( PSCONST_avSampleOffsets, (FLOAT*)avSampleOffsets, MAX_SAMPLES );
        g_pd3dDevice->SetPixelShader( m_pSampleLumFinalPS );

        g_pd3dDevice->SetTexture( 0, pSrcTexture );
        g_pd3dDevice->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_POINT );
        g_pd3dDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_POINT );
        g_pd3dDevice->SetSamplerState( 0, D3DSAMP_ADDRESSU,  D3DTADDRESS_CLAMP );
        g_pd3dDevice->SetSamplerState( 0, D3DSAMP_ADDRESSV,  D3DTADDRESS_CLAMP );
    }

    // Draw a fullscreen quad to sample the RT
    DrawFullScreenQuad();

    g_pd3dDevice->Resolve( D3DRESOLVE_RENDERTARGET0, NULL, pDstTexture, NULL, 
                           0, 0, NULL, 1.0f, 0L, NULL );

    // Cleanup and return
    PopRenderTarget( 0L )->Release();
    g_pd3dDevice->SetPixelShader( NULL );
}


//--------------------------------------------------------------------------------------
// Name: AdaptLuminance()
// Desc: Adapt the luminance over time and place the result in pDstTexture
//--------------------------------------------------------------------------------------
VOID PostProcess::AdaptLuminance( LPDIRECT3DTEXTURE9 pAdaptedLuminanceTexture, 
                                  LPDIRECT3DTEXTURE9 pToneMapTexture, 
                                  FLOAT fElapsedTime,
                                  LPDIRECT3DTEXTURE9 pDstTexture )
{
    // Make sure that the required shaders and objects exist
    assert( m_pCalculateAdaptedLumPS );
    assert( pAdaptedLuminanceTexture && pToneMapTexture && pDstTexture );

    // Create and set a render target
    PushRenderTarget( 0L, CreateRenderTarget( pDstTexture ) );

    // This simulates the light adaptation that occurs when moving from a dark area to
    // a bright area, or vice versa. The m_pTexAdaptedLuminance texture stores a single
    // texel cooresponding to the user's adapted level.
    // mruete: prefix bug 484: storing in a vector for proper usage of SetPixelShaderConstantF
    FLOAT vElapsedTime[4] = { fElapsedTime, 0.f, 0.f, 0.f };
    g_pd3dDevice->SetPixelShaderConstantF( PSCONST_fElapsedTime, vElapsedTime, 1 );
    g_pd3dDevice->SetPixelShader( m_pCalculateAdaptedLumPS );
    
    g_pd3dDevice->SetTexture( 0, pAdaptedLuminanceTexture );
    g_pd3dDevice->SetTexture( 1, pToneMapTexture );
    g_pd3dDevice->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_POINT );
    g_pd3dDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_POINT );
    g_pd3dDevice->SetSamplerState( 1, D3DSAMP_MAGFILTER, D3DTEXF_POINT );
    g_pd3dDevice->SetSamplerState( 1, D3DSAMP_MINFILTER, D3DTEXF_POINT );
    g_pd3dDevice->SetSamplerState( 0, D3DSAMP_ADDRESSU,  D3DTADDRESS_CLAMP );
    g_pd3dDevice->SetSamplerState( 0, D3DSAMP_ADDRESSV,  D3DTADDRESS_CLAMP );
    g_pd3dDevice->SetSamplerState( 1, D3DSAMP_ADDRESSU,  D3DTADDRESS_CLAMP );
    g_pd3dDevice->SetSamplerState( 1, D3DSAMP_ADDRESSV,  D3DTADDRESS_CLAMP );

    // Draw a fullscreen quad to sample the RT
    DrawFullScreenQuad();

    g_pd3dDevice->Resolve( D3DRESOLVE_RENDERTARGET0, NULL, pDstTexture, NULL, 
                           0, 0, NULL, 1.0f, 0L, NULL );

    // Cleanup and return
    PopRenderTarget( 0L )->Release();
    g_pd3dDevice->SetPixelShader( NULL );
}


//--------------------------------------------------------------------------------------
// Name: GaussBlur5x5Texture()
// Desc: Perform a 5x5 gaussian blur on pSrcTexture and place the result in pDstTexture
//--------------------------------------------------------------------------------------
VOID PostProcess::GaussBlur5x5Texture( LPDIRECT3DTEXTURE9 pSrcTexture,
                                       LPDIRECT3DTEXTURE9 pDstTexture )
{
    // Make sure that the required shaders and objects exist
    assert( m_pGaussBlur5x5PS );
    assert( pSrcTexture && pDstTexture );

    XGTEXTURE_DESC SrcDesc;
    XGGetTextureDesc( pSrcTexture, 0, &SrcDesc );

    // Create and set a render target
    PushRenderTarget( 0L, CreateRenderTarget( pDstTexture ) );

    XMVECTOR avSampleOffsets[MAX_SAMPLES];
    XMVECTOR avSampleWeights[MAX_SAMPLES];
    GetSampleOffsets_GaussBlur5x5( SrcDesc.Width, SrcDesc.Height, avSampleOffsets, avSampleWeights );
    g_pd3dDevice->SetPixelShaderConstantF( PSCONST_avSampleOffsets, (FLOAT*)avSampleOffsets, MAX_SAMPLES );
    g_pd3dDevice->SetPixelShaderConstantF( PSCONST_avSampleWeights, (FLOAT*)avSampleWeights, MAX_SAMPLES );
    g_pd3dDevice->SetPixelShader( m_pGaussBlur5x5PS );

    g_pd3dDevice->SetTexture( 0, pSrcTexture );
    g_pd3dDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_POINT );
    g_pd3dDevice->SetSamplerState( 0, D3DSAMP_ADDRESSU,  D3DTADDRESS_CLAMP );
    g_pd3dDevice->SetSamplerState( 0, D3DSAMP_ADDRESSV,  D3DTADDRESS_CLAMP );
   
    // Draw a fullscreen quad to sample the RT
    DrawFullScreenQuad();

    g_pd3dDevice->Resolve( D3DRESOLVE_RENDERTARGET0, NULL, pDstTexture, NULL, 
                           0, 0, NULL, 1.0f, 0L, NULL );

    // Cleanup and return
    PopRenderTarget( 0L )->Release();
    g_pd3dDevice->SetPixelShader( NULL );
}


//--------------------------------------------------------------------------------------
// Name: Downsample4x4Texture()
// Desc: Scale down pSrcTexture by 1/4 x 1/4 and place the result in pDstTexture
//--------------------------------------------------------------------------------------
VOID PostProcess::Downsample4x4Texture( LPDIRECT3DTEXTURE9 pSrcTexture,
                                        LPDIRECT3DTEXTURE9 pDstTexture )
{
    // Make sure that the required shaders and objects exist
    assert( m_pDownScale4x4PS );
    assert( pSrcTexture && pDstTexture );

    XGTEXTURE_DESC SrcDesc;
    XGGetTextureDesc( pSrcTexture, 0, &SrcDesc );

    // Create and set a render target
    PushRenderTarget( 0L, CreateRenderTarget( pDstTexture ) );

    // Get the sample offsets used within the pixel shader
    XMVECTOR avSampleOffsets[MAX_SAMPLES];
    GetSampleOffsets_DownScale4x4( SrcDesc.Width, SrcDesc.Height, avSampleOffsets );
    g_pd3dDevice->SetPixelShaderConstantF( PSCONST_avSampleOffsets, (FLOAT*)avSampleOffsets, MAX_SAMPLES );
    g_pd3dDevice->SetPixelShader( m_pDownScale4x4PS );
    
    g_pd3dDevice->SetTexture( 0, pSrcTexture );
    g_pd3dDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_POINT );
    g_pd3dDevice->SetSamplerState( 0, D3DSAMP_ADDRESSU,  D3DTADDRESS_CLAMP );
    g_pd3dDevice->SetSamplerState( 0, D3DSAMP_ADDRESSV,  D3DTADDRESS_CLAMP );
   
    // Draw a fullscreen quad
    DrawFullScreenQuad();

    g_pd3dDevice->Resolve( D3DRESOLVE_RENDERTARGET0, NULL, pDstTexture, NULL,
                           0, 0, NULL, 1.0f, 0L, NULL );

    // Cleanup and return
    PopRenderTarget( 0L )->Release();
    g_pd3dDevice->SetPixelShader( NULL );
}


//--------------------------------------------------------------------------------------
// Name: Downsample2x2Texture()
// Desc: Scale down pSrcTexture by 1/2 x 1/2 and place the result in pDstTexture
//--------------------------------------------------------------------------------------
VOID PostProcess::Downsample2x2Texture( LPDIRECT3DTEXTURE9 pSrcTexture,
                                        LPDIRECT3DTEXTURE9 pDstTexture )
{
    // Make sure that the required shaders and objects exist
    assert( m_pDownScale2x2PS );
    assert( pSrcTexture && pDstTexture );

    XGTEXTURE_DESC SrcDesc;
    XGGetTextureDesc( pSrcTexture, 0, &SrcDesc );

    // Create and set a render target
    PushRenderTarget( 0L, CreateRenderTarget( pDstTexture ) );

    XMVECTOR avSampleOffsets[MAX_SAMPLES];
    GetSampleOffsets_DownScale2x2( SrcDesc.Width, SrcDesc.Height, avSampleOffsets );
    g_pd3dDevice->SetPixelShaderConstantF( PSCONST_avSampleOffsets, (FLOAT*)avSampleOffsets, MAX_SAMPLES );

    // Create an exact 1/2 x 1/2 copy of the source texture
    g_pd3dDevice->SetPixelShader( m_pDownScale2x2PS );

    g_pd3dDevice->SetTexture( 0, pSrcTexture );
    g_pd3dDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_POINT );
    g_pd3dDevice->SetSamplerState( 0, D3DSAMP_ADDRESSU,  D3DTADDRESS_CLAMP );
    g_pd3dDevice->SetSamplerState( 0, D3DSAMP_ADDRESSV,  D3DTADDRESS_CLAMP );
   
    // TODO: This should use border addressing with a black border!
    //m_pStarSourceTexture->Format.ClampX      = GPUCLAMP_CLAMP_TO_BORDER;
    //m_pStarSourceTexture->Format.ClampY      = GPUCLAMP_CLAMP_TO_BORDER;
    //m_pStarSourceTexture->Format.BorderColor = GPUBORDERCOLOR_ABGR_BLACK;
    //g_pd3dDevice->SetSamplerState( 0, D3DSAMP_ADDRESSU, D3DTADDRESS_BORDER );
    //g_pd3dDevice->SetSamplerState( 0, D3DSAMP_ADDRESSV, D3DTADDRESS_BORDER );

    // Draw a fullscreen quad to sample the RT
    DrawFullScreenQuad();

    g_pd3dDevice->Resolve( D3DRESOLVE_RENDERTARGET0, NULL, pDstTexture, NULL, 
                           0, 0, NULL, 1.0f, 0L, NULL );

    // Cleanup and return
    PopRenderTarget( 0L )->Release();
    g_pd3dDevice->SetPixelShader( NULL );
}


//--------------------------------------------------------------------------------------
// Name: BloomTexture()
// Desc: Bloom the pSrcTexture and place the result in pDstTexture
//--------------------------------------------------------------------------------------
VOID PostProcess::BloomTexture( LPDIRECT3DTEXTURE9 pSrcTexture,
                                BOOL bBloomAcrossWidth,
                                LPDIRECT3DTEXTURE9 pDstTexture )
{
    // Make sure that the required shaders and objects exist
    assert( m_pBloomPS );
    assert( pSrcTexture && pDstTexture );

    XGTEXTURE_DESC SrcDesc;
    XGGetTextureDesc( pSrcTexture, 0, &SrcDesc );

    // Create and set a render target
    PushRenderTarget( 0L, CreateRenderTarget( pDstTexture ) );

    XMVECTOR avSampleOffsets[MAX_SAMPLES];
    XMVECTOR avSampleWeights[MAX_SAMPLES];

    if( bBloomAcrossWidth )
        GetSampleOffsets_Bloom( SrcDesc.Width,  avSampleOffsets, avSampleWeights, 3.0f, 2.0f );
    else
        GetSampleOffsets_Bloom( SrcDesc.Height, avSampleOffsets, avSampleWeights, 3.0f, 2.0f );

    g_pd3dDevice->SetPixelShaderConstantF( PSCONST_avSampleOffsets, (FLOAT*)avSampleOffsets, MAX_SAMPLES );
    g_pd3dDevice->SetPixelShaderConstantF( PSCONST_avSampleWeights, (FLOAT*)avSampleWeights, MAX_SAMPLES );
    g_pd3dDevice->SetPixelShader( m_pBloomPS );
    
    g_pd3dDevice->SetTexture( 0, pSrcTexture );
    g_pd3dDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_POINT );
    g_pd3dDevice->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_POINT );
    g_pd3dDevice->SetSamplerState( 0, D3DSAMP_ADDRESSU,  D3DTADDRESS_CLAMP );
    g_pd3dDevice->SetSamplerState( 0, D3DSAMP_ADDRESSV,  D3DTADDRESS_CLAMP );
           
    // Draw a fullscreen quad to sample the RT
    DrawFullScreenQuad();

    g_pd3dDevice->Resolve( D3DRESOLVE_RENDERTARGET0, NULL, pDstTexture, NULL, 
                           0, 0, NULL, 1.0f, 0L, NULL );

    // Cleanup and return
    PopRenderTarget( 0L )->Release();
    g_pd3dDevice->SetPixelShader( NULL );
}


//--------------------------------------------------------------------------------------
// Name: RenderStarLine()
// Desc: Merge the ppSrcTextures and place the result in pDstTexture
//--------------------------------------------------------------------------------------
VOID PostProcess::RenderStarLine( LPDIRECT3DTEXTURE9 pSrcTexture, DWORD dwNumSamples,
                                  FLOAT fAttenuation, FLOAT fAttnPowScale,
                                  XMVECTOR* colors, DWORD pass,
                                  FLOAT fStepU, FLOAT fStepV, 
                                  LPDIRECT3DTEXTURE9 pDstTexture )
{
    // Make sure that the required shaders and objects exist
    assert( m_pStarPS );
    assert( pSrcTexture && pDstTexture );

    // Create and set a render target
    PushRenderTarget( 0L, CreateRenderTarget( pDstTexture ) );

    XMVECTOR avSampleOffsets[MAX_SAMPLES];
    XMVECTOR avSampleWeights[MAX_SAMPLES];

    // Sampling configration for each stage
    for( DWORD i=0; i<dwNumSamples; i++ )
    {
        FLOAT lum = powf( fAttenuation, fAttnPowScale * i );
        
        avSampleWeights[i] = colors[i] * lum * (pass+1.0f) * 0.5f;
        
        // Offset of sampling coordinate
        avSampleOffsets[i].x = fStepU * i;
        avSampleOffsets[i].y = fStepV * i;
        if( fabs(avSampleOffsets[i].x) >= 0.9f || fabs(avSampleOffsets[i].y) >= 0.9f )
        {
            avSampleOffsets[i].x = 0.0f;
            avSampleOffsets[i].y = 0.0f;
            avSampleWeights[i]  *= 0.0f;
        }
    }
    
    g_pd3dDevice->SetPixelShader( m_pStarPS );
    g_pd3dDevice->SetPixelShaderConstantF( PSCONST_avSampleOffsets, (FLOAT*)avSampleOffsets, MAX_SAMPLES );
    g_pd3dDevice->SetPixelShaderConstantF( PSCONST_avSampleWeights, (FLOAT*)avSampleWeights, MAX_SAMPLES );
    
    g_pd3dDevice->SetTexture( 0, pSrcTexture );
    g_pd3dDevice->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );
    g_pd3dDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
    g_pd3dDevice->SetSamplerState( 0, D3DSAMP_ADDRESSU,  D3DTADDRESS_CLAMP );
    g_pd3dDevice->SetSamplerState( 0, D3DSAMP_ADDRESSV,  D3DTADDRESS_CLAMP );

    // Draw a fullscreen quad to sample the RT
    DrawFullScreenQuad();

    g_pd3dDevice->Resolve( D3DRESOLVE_RENDERTARGET0, NULL, pDstTexture, NULL, 
                           0, 0, NULL, 1.0f, 0L, NULL );

    // Cleanup and return
    PopRenderTarget( 0L )->Release();
    g_pd3dDevice->SetPixelShader( NULL );
}


//--------------------------------------------------------------------------------------
// Name: MergeTextures()
// Desc: Merge the ppSrcTextures and place the result in pDstTexture
//--------------------------------------------------------------------------------------
VOID PostProcess::MergeTextures( LPDIRECT3DTEXTURE9* ppSrcTextures,
                                 DWORD dwNumSrcTextures,
                                 LPDIRECT3DTEXTURE9 pDstTexture )
{
    // Make sure that the required shaders and objects exist
    assert( m_pMergeTexturesPS[dwNumSrcTextures] );
    assert( ppSrcTextures && pDstTexture );

    // Create and set a render target
    PushRenderTarget( 0L, CreateRenderTarget( pDstTexture ) );

    XMVECTOR avSampleWeights[MAX_SAMPLES];
    const XMVECTOR vWhite = XMVectorSet( 1.0f, 1.0f, 1.0f, 1.0f );
    for( DWORD i=0; i<dwNumSrcTextures; i++ )
    {
        g_pd3dDevice->SetTexture( i, ppSrcTextures[i] );
        g_pd3dDevice->SetSamplerState( i, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );
        g_pd3dDevice->SetSamplerState( i, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
        g_pd3dDevice->SetSamplerState( i, D3DSAMP_ADDRESSU,  D3DTADDRESS_CLAMP );
        g_pd3dDevice->SetSamplerState( i, D3DSAMP_ADDRESSV,  D3DTADDRESS_CLAMP );

        avSampleWeights[i] = vWhite * 1.0f / (FLOAT)dwNumSrcTextures;
    }

    g_pd3dDevice->SetPixelShaderConstantF( PSCONST_avSampleWeights, (FLOAT*)avSampleWeights, dwNumSrcTextures );
    g_pd3dDevice->SetPixelShader( m_pMergeTexturesPS[dwNumSrcTextures] );

    // Draw a fullscreen quad to sample the RT
    DrawFullScreenQuad();

    g_pd3dDevice->Resolve( D3DRESOLVE_RENDERTARGET0, NULL, pDstTexture, NULL, 
                           0, 0, NULL, 1.0f, 0L, NULL );

    // Cleanup and return
    PopRenderTarget( 0L )->Release();
    g_pd3dDevice->SetPixelShader( NULL );
}


//--------------------------------------------------------------------------------------
// Name: Initialize()
// Desc: Initialize the effects library
//--------------------------------------------------------------------------------------
HRESULT PostProcess::Initialize()
{
    // Create a vertex buffer for screen-space effects
    g_pd3dDevice->CreateVertexBuffer( 3*sizeof(XMFLOAT4), 0L, 0L, D3DPOOL_DEFAULT,
                                      &m_pQuadVB, NULL );
    XMFLOAT4* v;
    m_pQuadVB->Lock( 0, 0, (VOID**)&v, 0 );
    v[0] = XMFLOAT4( -1.0f,  1.0f,   0.0f, 0.0f ); // x, y, tu, tv
    v[1] = XMFLOAT4(  1.0f,  1.0f,   1.0f, 0.0f ); // x, y, tu, tv
    v[2] = XMFLOAT4( -1.0f, -1.0f,   0.0f, 1.0f ); // x, y, tu, tv
    m_pQuadVB->Unlock();

    // Create common vertex declaration used by all the screen-space effects
    D3DVERTEXELEMENT9 decl[] =
    {
        { 0, 0, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
        { 0, 8, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
        D3DDECL_END()
    };

    g_pd3dDevice->CreateVertexDeclaration( decl, &m_pQuadVtxDecl );

    // Create shaders. Note that it's okay if these do not load, as long as the app does
    // not later try to use them. An assert will fire otherwise. This is so that the app
    // only needs to supply shaders that it actually intends to use.
	g_BsKernel.chdir("Media");
	g_BsKernel.chdir("Shaders");
	char szFullFileName[_MAX_PATH];
	sprintf(szFullFileName, "%s%s", g_BsKernel.GetCurrentDirectory(), "ScreenSpaceShader.xvu" );
    ATG::LoadVertexShader( szFullFileName,  &m_pScreenSpaceVS );

	sprintf(szFullFileName, "%s%s", g_BsKernel.GetCurrentDirectory(), "Bloom.xpu" );
    ATG::LoadPixelShader( szFullFileName,               &m_pBloomPS );

	sprintf(szFullFileName, "%s%s", g_BsKernel.GetCurrentDirectory(), "Star.xpu" );
    ATG::LoadPixelShader( szFullFileName,               &m_pStarPS );

	sprintf(szFullFileName, "%s%s", g_BsKernel.GetCurrentDirectory(), "SampleLumInitial.xpu" );
    ATG::LoadPixelShader( szFullFileName,    &m_pSampleLumInitialPS );

	sprintf(szFullFileName, "%s%s", g_BsKernel.GetCurrentDirectory(), "SampleLumFinal.xpu" );
    ATG::LoadPixelShader( szFullFileName,      &m_pSampleLumFinalPS );

	sprintf(szFullFileName, "%s%s", g_BsKernel.GetCurrentDirectory(), "CalculateAdaptedLum.xpu" );
    ATG::LoadPixelShader( szFullFileName, &m_pCalculateAdaptedLumPS );

	sprintf(szFullFileName, "%s%s", g_BsKernel.GetCurrentDirectory(), "DownScale4x4.xpu" );
    ATG::LoadPixelShader( szFullFileName,        &m_pDownScale4x4PS );

	sprintf(szFullFileName, "%s%s", g_BsKernel.GetCurrentDirectory(), "DownScale2x2.xpu" );
    ATG::LoadPixelShader( szFullFileName,        &m_pDownScale2x2PS );

	sprintf(szFullFileName, "%s%s", g_BsKernel.GetCurrentDirectory(), "GaussBlur5x5.xpu" );
    ATG::LoadPixelShader( szFullFileName,        &m_pGaussBlur5x5PS );

	sprintf(szFullFileName, "%s%s", g_BsKernel.GetCurrentDirectory(), "BrightPassFilter.xpu" );
    ATG::LoadPixelShader( szFullFileName,    &m_pBrightPassFilterPS );

	sprintf(szFullFileName, "%s%s", g_BsKernel.GetCurrentDirectory(), "MergeTextures_1.xpu" );
    ATG::LoadPixelShader( szFullFileName,     &m_pMergeTexturesPS[1] );

	sprintf(szFullFileName, "%s%s", g_BsKernel.GetCurrentDirectory(), "MergeTextures_2.xpu" );
    ATG::LoadPixelShader( szFullFileName,     &m_pMergeTexturesPS[2] );

	sprintf(szFullFileName, "%s%s", g_BsKernel.GetCurrentDirectory(), "MergeTextures_3.xpu" );
    ATG::LoadPixelShader( szFullFileName,     &m_pMergeTexturesPS[3] );

	sprintf(szFullFileName, "%s%s", g_BsKernel.GetCurrentDirectory(), "MergeTextures_4.xpu" );
    ATG::LoadPixelShader( szFullFileName,     &m_pMergeTexturesPS[4] );

	sprintf(szFullFileName, "%s%s", g_BsKernel.GetCurrentDirectory(), "MergeTextures_5.xpu" );
    ATG::LoadPixelShader( szFullFileName,     &m_pMergeTexturesPS[5] );

	sprintf(szFullFileName, "%s%s", g_BsKernel.GetCurrentDirectory(), "MergeTextures_6.xpu" );
    ATG::LoadPixelShader( szFullFileName,     &m_pMergeTexturesPS[6] );

	sprintf(szFullFileName, "%s%s", g_BsKernel.GetCurrentDirectory(), "MergeTextures_7.xpu" );
    ATG::LoadPixelShader( szFullFileName,     &m_pMergeTexturesPS[7] );

	sprintf(szFullFileName, "%s%s", g_BsKernel.GetCurrentDirectory(), "MergeTextures_8.xpu" );
    ATG::LoadPixelShader( szFullFileName,     &m_pMergeTexturesPS[8] );
	g_BsKernel.chdir("..");
	g_BsKernel.chdir("..");
    return S_OK;
}
#endif //XOBX

};

