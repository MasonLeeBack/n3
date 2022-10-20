//--------------------------------------------------------------------------------------
// AtgPostProcess.h
//
// Commonly used post-processing effects (like bloom, blur, etc.)
//
// Xbox Advanced Technology Group.
// Copyright (C) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------
#pragma once
#ifdef _XBOX
#ifndef ATGPOSTPROCESS_H
#define ATGPOSTPROCESS_H
#include <xtl.h>
#include <xboxmath.h>
#include <stack>


namespace ATG
{

// External access to the global D3D device




//--------------------------------------------------------------------------------------
// Name: class PostProcess
// Desc: Library of commonly used post-processing effects
//--------------------------------------------------------------------------------------
class PostProcess
{
    LPDIRECT3DVERTEXBUFFER9      m_pQuadVB;
    LPDIRECT3DVERTEXDECLARATION9 m_pQuadVtxDecl;
    LPDIRECT3DVERTEXSHADER9      m_pScreenSpaceVS;
    LPDIRECT3DPIXELSHADER9       m_pBloomPS;
    LPDIRECT3DPIXELSHADER9       m_pStarPS;
    LPDIRECT3DPIXELSHADER9       m_pSampleLumInitialPS;
    LPDIRECT3DPIXELSHADER9       m_pSampleLumFinalPS;
    LPDIRECT3DPIXELSHADER9       m_pCalculateAdaptedLumPS;
    LPDIRECT3DPIXELSHADER9       m_pDownScale4x4PS;
    LPDIRECT3DPIXELSHADER9       m_pDownScale2x2PS;
    LPDIRECT3DPIXELSHADER9       m_pGaussBlur5x5PS;
    LPDIRECT3DPIXELSHADER9       m_pBrightPassFilterPS;
    LPDIRECT3DPIXELSHADER9       m_pMergeTexturesPS[9];

    // Sample offset calculation. These offsets are passed to corresponding pixel shaders.
    VOID    GetSampleOffsets_GaussBlur5x5( IN DWORD dwTextureWidth, IN DWORD dwTextureHeight,
                                           OUT XMVECTOR* pvTexCoordOffsets,
                                           OUT XMVECTOR* vSampleWeights,
                                           IN OPTIONAL FLOAT fMultiplier=1.0f );
    VOID    GetSampleOffsets_Bloom( IN DWORD dwTextureSize,
                                    OUT XMVECTOR* pvTexCoordOffsets,
                                    OUT XMVECTOR* pvColorWeights, IN FLOAT fDeviation, 
                                    IN FLOAT OPTIONAL fMultiplier=1.0f );
    VOID    GetSampleOffsets_Star( IN DWORD dwTextureSize,
                                   OUT XMVECTOR* pvTexCoordOffsets,
                                   OUT XMVECTOR* pvColorWeights, IN FLOAT fDeviation );
    VOID    GetSampleOffsets_DownScale4x4( IN DWORD dwWidth, IN DWORD dwHeight,
                                           OUT XMVECTOR* pvSampleOffsets );
    VOID    GetSampleOffsets_DownScale3x3( IN DWORD dwWidth, IN DWORD dwHeight,
                                           OUT XMVECTOR* pvSampleOffsets );
    VOID    GetSampleOffsets_DownScale2x2( IN DWORD dwWidth, IN DWORD dwHeight,
                                           OUT XMVECTOR* pvSampleOffsets );

public:
    HRESULT Initialize();

    // Post-processing source textures creation
    VOID    ClearTexture( IN LPDIRECT3DTEXTURE9 pTexture,
                          IN OPTIONAL DWORD dwClearColor = 0x00000000 );
    VOID    Downsample2x2Texture( IN LPDIRECT3DTEXTURE9 pSrcTexture,
                                  IN LPDIRECT3DTEXTURE9 pDstTexture );
    VOID    Downsample4x4Texture( IN LPDIRECT3DTEXTURE9 pSrcTexture,
                                  IN LPDIRECT3DTEXTURE9 pDstTexture );
    VOID    GaussBlur5x5Texture( IN LPDIRECT3DTEXTURE9 pSrcTexture,
                                 IN LPDIRECT3DTEXTURE9 pDstTexture );
    VOID    BrightPassFilterTexture( IN LPDIRECT3DTEXTURE9 pSrcTexture,
                                     IN LPDIRECT3DTEXTURE9 pAdaptedLuminanceTexture, 
                                     IN FLOAT              fMiddleGrayKeyValue,
                                     IN LPDIRECT3DTEXTURE9 pDstTexture );
    VOID    MergeTextures( IN LPDIRECT3DTEXTURE9* ppSrcTextures,
                           IN DWORD dwNumSrcTextures,
                           IN LPDIRECT3DTEXTURE9 pDstTexture );
    VOID    AdaptLuminance( IN LPDIRECT3DTEXTURE9 pAdaptedLuminanceTexture, 
                            IN LPDIRECT3DTEXTURE9 pToneMapTexture, 
                            IN FLOAT fElapsedTime,
                            IN LPDIRECT3DTEXTURE9 pDstTexture );
    VOID    SampleLuminance( IN LPDIRECT3DTEXTURE9 pSrcTexture, IN BOOL bInitial,
                             IN LPDIRECT3DTEXTURE9 pDstTexture );
    VOID    BloomTexture( IN LPDIRECT3DTEXTURE9 pSrcTexture, IN BOOL bBloomAcrossWidth,
                          IN LPDIRECT3DTEXTURE9 pDstTexture );
    VOID    RenderStarLine( IN LPDIRECT3DTEXTURE9 pSrcTexture, IN DWORD dwNumSamples,
                            IN FLOAT fAttenuation, IN FLOAT fAttnPowScale,
                            IN XMVECTOR* colors, IN DWORD pass,
                            IN FLOAT fStepU, IN FLOAT fStepV, 
                            IN LPDIRECT3DTEXTURE9 pDstTexture );

    VOID    DrawScreenSpaceQuad( IN FLOAT fWidth, IN FLOAT fHeight,
                                 IN FLOAT fMaxU=1.0f, IN FLOAT fMaxV=1.0f );
    VOID    DrawFullScreenQuad();
};

}; // namespace ATG

#endif // ATGPOSTPROCESS_H

#endif // XBOX