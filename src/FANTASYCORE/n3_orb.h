#ifdef _XBOX
#include <xtl.h>
#include <xgraphics.h>
#include <xboxmath.h>
#include <fxl.h>
#include "AtgFont.h"
#include "AtgInput.h"
#include "AtgPostProcess.h"
#include "AtgResource.h"
#include "AtgUtil.h"
#include "GlareDefD3D.h"



class C3DDevice;


//--------------------------------------------------------------------------------------
// Mappings to shader constants that are used in the HLSL shaders
//--------------------------------------------------------------------------------------
const DWORD PSCONST_matataki            =  0;
const DWORD PSCONST_fEmissive           =  4;
const DWORD PSCONST_fMiddleGray         =  5;
//const DWORD PSCONST_fElapsedTime      =  7;
const DWORD PSCONST_bEnableBlueShift    =  8;
const DWORD PSCONST_bEnableToneMap      =  9;
const DWORD PSCONST_fBloomScale         = 10;
const DWORD PSCONST_fStarScale          = 11;
const DWORD PSCONST_fRadialScale		= 12;
const DWORD PSCONST_perR				= 33;

const DWORD VSCONST_g_mWorldViewProj	=  0;
const DWORD VSCONST_mWorldView			=  4;
const DWORD VSCONST_mWorldViewLast		=  8;
const DWORD VSCONST_mWorld				= 12;
const DWORD VSCONST_mProjection         = 16;
const DWORD VSCONST_mYuragi0			= 20;
const DWORD VSCONST_mYuragi1			= 24;
const DWORD VSCONST_mEyePt				= 28;
const DWORD VSCONST_Count				= 32;

const DWORD VSCONST_mInvWVP				= 0;
const DWORD VSCONST_mInvWVPLast			= 4;

const DWORD PSCONST_avSampleOffsets     =  0;
const DWORD PSCONST_avSampleWeights     = 16;
const DWORD PSCONST_Time				=  0;
const DWORD PSCONST_SPEQULAR			=  1;
const DWORD PSCONST_SceneAlpha			=  2;
const DWORD PSCONST_mEyePt				= 28;
const DWORD PSCONST_COLOR				= 37;
//--------------------------------------------------------------------------------------
// Constants
//--------------------------------------------------------------------------------------
const DWORD NUM_STAR_TEXTURES    = 12;      // Number of textures used for the star post-processing effect
const DWORD NUM_BLOOM_TEXTURES   = 3;       // Number of textures used for the bloom post-processing effect




#define MAX_SAMPLES           16      // Maximum number of texture grabs
// of light intensity                                    
#define NUM_STAR_TEXTURES     12      // Number of textures used for the star
// post-processing effect
#define NUM_BLOOM_TEXTURES    3       // Number of textures used for the bloom
// post-processing effect
#define NUM_PARTICLES		200			//number of particles                                    

#define PARTICLES_RANGE		1400.0f

#define PARTICLES_SIZE		2.4f

#define INNER_TEXTURES		8

//--------------------------------------------------------------------------------------
// Helper functions
//--------------------------------------------------------------------------------------
#define V(fn)    { if(FAILED(hr=(fn)))return hr;}
#define SAFE_RELEASE(p)      { if(p) { (p)->Release(); (p)=NULL; } }



//--------------------------------------------------------------------------------------
// Name: class Sample
// Desc: Main class to run this application.
//--------------------------------------------------------------------------------------
class Orb 
{


	ATG::Timer          m_Timer;                  // Application timer
	ATG::PackedResource m_Resource;               // Packed texture resources
	ATG::PostProcess    m_PostProcess;            // Commonly used effects (blur, etc.)
	BOOL                m_bDrawHelp;              // Whether to draw the help screen



	DWORD               m_dwNumTiles;             // Tile information for predicated tiling
	D3DRECT             m_TilingRects[4];

	LPDIRECT3DSURFACE9 pSurfRT;


	LPDIRECT3DTEXTURE9  m_pFrontBuffer[2];


	LPDIRECT3DVERTEXDECLARATION9 m_pTransformSceneVtxDecl;
	LPDIRECT3DVERTEXDECLARATION9 m_pOrbVtxDecl;
	LPDIRECT3DVERTEXSHADER9      m_pTransformSceneVS;
	LPDIRECT3DVERTEXSHADER9      m_pSceneVS;
	LPDIRECT3DVERTEXSHADER9      m_pSkyBoxVS;
	LPDIRECT3DVERTEXSHADER9      m_pParticlesVS;


	LPDIRECT3DPIXELSHADER9       m_pFinalScenePassPS;
	LPDIRECT3DPIXELSHADER9       m_pScenePS;
	LPDIRECT3DPIXELSHADER9		 m_pSkyBoxPS;
	LPDIRECT3DPIXELSHADER9		 m_pSkyBoxVelocityPS;
	LPDIRECT3DPIXELSHADER9		 m_pParticlesPS;
	LPDIRECT3DPIXELSHADER9       m_pPS;
	LPDIRECT3DPIXELSHADER9       m_pGaussBlur5x5PS;
	LPDIRECT3DPIXELSHADER9       m_pRadialPS;
	LPDIRECT3DPIXELSHADER9       m_pFinalPS;


	LPDIRECT3DTEXTURE9  m_pWhiteTexture;                // A blank texture
	LPDIRECT3DTEXTURE9  m_pSceneTexture;                // HDR texture containing the scene
	LPDIRECT3DTEXTURE9  m_pScaledSceneTexture;          // Scaled copy of the HDR scene
	LPDIRECT3DTEXTURE9  m_pBrightPassTexture;           // Bright-pass filtered copy of the scene
	LPDIRECT3DTEXTURE9  m_pAdaptedLuminanceTexture;     // The luminance the user is adapted to
	LPDIRECT3DTEXTURE9  m_pStarSourceTexture;           // Star effect source texture
	LPDIRECT3DTEXTURE9  m_pBloomSourceTexture;          // Bloom effect source texture
	LPDIRECT3DTEXTURE9  m_pBloomTexture;                // Blooming effect texture
	LPDIRECT3DTEXTURE9  m_apStarTextures[NUM_STAR_TEXTURES]; // Star effect working textures
	LPDIRECT3DTEXTURE9  m_pToneMapTexture64x64;         // Average luminance samples from the HDR render target
	LPDIRECT3DTEXTURE9  m_pToneMapTexture16x16;         // Average luminance samples from the HDR render target
	LPDIRECT3DTEXTURE9  m_pToneMapTexture4x4;           // Average luminance samples from the HDR render target
	LPDIRECT3DTEXTURE9  m_pToneMapTexture1x1;           // Average luminance samples from the HDR render target
	LPDIRECT3DTEXTURE9  m_pBackground;
	LPDIRECT3DTEXTURE9  m_pTexInner[INNER_TEXTURES];
	LPDIRECT3DTEXTURE9  m_pTexRogo;
	LPDIRECT3DTEXTURE9  m_pTexUroko;
	LPDIRECT3DTEXTURE9  g_pTexRadial;

	PDIRECT3DTEXTURE9 g_pTexScene;            // HDR render target containing the scene
	PDIRECT3DTEXTURE9 g_pTexSceneL;            // HDR render target containing the scene
	PDIRECT3DTEXTURE9 g_pTexFinalScene;
	PDIRECT3DTEXTURE9 g_pTexVelocity;
	PDIRECT3DTEXTURE9 g_pTexMadara;
	PDIRECT3DCUBETEXTURE9 m_pTexSkyBox;
	PDIRECT3DVOLUMETEXTURE9 m_pTexVolume;
	PDIRECT3DVOLUMETEXTURE9 m_pTexSea;

	LPD3DXMESH			g_pMesh;			// クリス?ル用メッシュ
	LPD3DXMESH			g_pPointMesh;		// 不純物用メッシュ

	XMMATRIX			g_mCurModelWV;
	XMMATRIX			g_mYuragi0;
	XMMATRIX			g_mYuragi1;

	XMMATRIX            m_matWorld;
	XMMATRIX			m_matWorldBase;
	XMMATRIX            m_matView;
	XMMATRIX            m_matProj;
	XMMATRIX            m_matWorldT;
	XMMATRIX            m_matViewT;
	XMMATRIX            m_matProjT;
	XMMATRIX			m_matWVP;
	XMMATRIX			mWorldInv;

	XMVECTOR			m_vParticlesPos[NUM_PARTICLES];

	LPDIRECT3DTEXTURE9  m_pParticle;

	LPDIRECT3DVERTEXBUFFER9         m_pVB;
	IDirect3DVertexDeclaration9*    m_pSkyBoxDecl;

	CGlareDef           m_GlareDef;                // Glare defintion

	FLOAT               m_fMiddleGrayKeyValue;     // Middle gray key value for tone mapping
	FLOAT               m_fBloomScale;             // Scale factor for bloom
	FLOAT               m_fStarScale;              // Scale factor for star
	FLOAT				m_fRadialScale;
	FLOAT				kussetu;

	XMVECTOR vFromPt;
	XMVECTOR vLookatPt;
	XMVECTOR mEyePtInv;

	XMVECTOR alpha;

	FLOAT				matataki[NUM_PARTICLES];

	DWORD				m_frame;

	FLOAT fTime;
	FLOAT m_fElapsedTime;

	FLOAT fXRotateR;
	FLOAT fYRotateR;

	FLOAT fEyeV;
	FLOAT fEyeR;

	FLOAT af;

	FLOAT fXRotate;
	FLOAT fYRotate;

	XMVECTOR g_color;

	//    BOOL                m_bToneMap;                // True when scene is to be tone mapped

	// Tone mapping and post-process lighting effects
	HRESULT MeasureLuminance();
	HRESULT RenderStar();
	HRESULT RenderBloom();

	HRESULT RenderScene();

	HRESULT RenderRadial();

	HRESULT LoadMesh( IDirect3DDevice9* pd3dDevice, LPCSTR strFileName, ID3DXMesh** ppMesh );
	VOID	RenderParticles();
	VOID	RenderSkyBox();

	FLOAT	Dot( XMVECTOR* v1, XMVECTOR* v2 );
	VOID	Downsample3x3Texture( LPDIRECT3DTEXTURE9 pSrcTexture,
		LPDIRECT3DTEXTURE9 pDstTexture );

public:
	D3DPRESENT_PARAMETERS m_d3dpp;
	LPDIRECT3DDEVICE9     m_pd3dDevice;
	HRESULT Initialize( C3DDevice* d3dDevice );
	HRESULT Update();
	HRESULT Render();
	void	Release();

	Orb();
	virtual ~Orb();
};

#endif//XBOX