#include "stdafx.h"
#include "BsKernel.h"
#include "BsMoviePlayer.h"
#include <xaudio2.h>
#include <dsound.h>
#ifdef _XBOX
#include <xtl.h>
#include <xboxmath.h>
#include <AtgUtil.h>
#include "BsKernel.h"
#include "xgraphics.h"
#endif

#include "MGSToolbox\MGSToolbox.h"


#ifndef _LTCG
#pragma comment(lib, "../BlueSideEngine/BinkXenon/binkxenon.lib")
#else
#pragma comment(lib, "../BlueSideEngine/BinkXenon/binkxenon.LTCG.lib")
#endif


//
// structure and definition for uploading our texture verts
//

#define POS_TC_VERTEX_FVF ( D3DFVF_XYZ | D3DFVF_TEX1 | D3DFVF_TEXCOORDSIZE2( 0 ) )

//
// simple pass through vertex shader
//

static CONST CHAR StrPositionAndTexCoordPassThrough[] = 
" struct VS_DATA                                        "
" {                                                     "
"     float4 Pos : POSITION;                            "
"     float4 T0: TEXCOORD0;                             "
" };                                                    "
"                                                       "
" VS_DATA main( VS_DATA In )                            "
" {                                                     "
"     return In;                                        "
" }                                                     ";

//
// simple pixel shader to apply the yuvtorgb matrix
//

static const char StrYCrCbToRGBNoPixelAlpha[] =
" sampler tex0   : register( s0 );      "
" sampler tex1   : register( s1 );      "
" sampler tex2   : register( s2 );      "
" float4  tor    : register( c0 );      "
" float4  tog    : register( c1 );      "
" float4  tob    : register( c2 );      "
" float4  consts : register( c3 );      "
"                                       "
" struct VS_OUT                         "
" {                                     "
"     float2 T0: TEXCOORD0;             "
" };                                    "
"                                       "
" float4 main( VS_OUT In ) : COLOR      "
" {                                     "
"   float4 c;                           "
"   float4 p;                           "
"   c.x = tex2D( tex0, In.T0 ).x;       "
"   c.y = tex2D( tex1, In.T0 ).x;       "
"   c.z = tex2D( tex2, In.T0 ).x;       "
"   c.w = consts.x;                     "
"   p.x = dot( tor, c );                "
"   p.y = dot( tog, c );                "
"   p.z = dot( tob, c );                "
"   p.w = consts.w;                     "
"   return p;                           "
" }                                     ";

//
// simple pixel shader to apply the yuvtorgb matrix with alpha
//

static const char StrYCrCbAToRGBA[] =
" sampler tex0   : register( s0 );      "
" sampler tex1   : register( s1 );      "
" sampler tex2   : register( s2 );      "
" sampler tex3   : register( s3 );      "
" float4  tor    : register( c0 );      "
" float4  tog    : register( c1 );      "
" float4  tob    : register( c2 );      "
" float4  consts : register( c3 );      "
"                                       "
" struct VS_OUT                         "
" {                                     "
"     float2 T0: TEXCOORD0;             "
" };                                    "
"                                       "
" float4 main( VS_OUT In ) : COLOR      "
" {                                     "
"   float4 c;                           "
"   float4 p;                           "
"   c.x = tex2D( tex0, In.T0 ).x;       "
"   c.y = tex2D( tex1, In.T0 ).x;       "
"   c.z = tex2D( tex2, In.T0 ).x;       "
"   c.w = consts.x;                     "
"   p.w = tex2D( tex3, In.T0 ).x;       "
"   p.x = dot( tor, c );                "
"   p.y = dot( tog, c );                "
"   p.z = dot( tob, c );                "
"   p.w *= consts.w;                    "
"   return p;                           "
" }                                     ";


//
// matrix to convert yuv to rgb
// not const - we change the final value to reflect global alpha
//

static float yuvtorgb[] = 
{
	1.164123535f,  1.595794678f,  0.0f,         -0.87065506f,
		1.164123535f, -0.813476563f, -0.391448975f,  0.529705048f,
		1.164123535f,  0.0f,          2.017822266f, -1.081668854f,
		1.0f, 0.0f, 0.0f, 0.0f
};


CBsMoviePlayer::CBsMoviePlayer()
{
	m_YCrCbToRGBNoPixelAlpha = 0;
	m_YCrCbAToRGBA = 0;
	m_PositionAndTexCoordPassThrough = 0;
	m_hBink = 0;
	memset(&m_BTS, 0, sizeof(m_BTS));
}

CBsMoviePlayer::~CBsMoviePlayer()
{
	if ( m_YCrCbToRGBNoPixelAlpha )
	{
		m_YCrCbToRGBNoPixelAlpha->Release();
		m_YCrCbToRGBNoPixelAlpha = 0;
	}

	if ( m_YCrCbAToRGBA )
	{
		m_YCrCbAToRGBA->Release();
		m_YCrCbAToRGBA = 0;
	}

	if ( m_PositionAndTexCoordPassThrough == 0 )
	{
		m_PositionAndTexCoordPassThrough->Release();
		m_PositionAndTexCoordPassThrough = 0;
	}

}


void CBsMoviePlayer::Initialize()
{
	HRESULT hr;
	ID3DXBuffer* buffer = 0;

	//
	// create a pixel shader that goes from YcRcB to RGB (without alpha)
	//
	if ( m_YCrCbToRGBNoPixelAlpha == 0 )
	{
		hr = D3DXCompileShader( StrYCrCbToRGBNoPixelAlpha, sizeof( StrYCrCbToRGBNoPixelAlpha ),
			0, 0, "main", "ps_2_0", 0, &buffer, NULL, NULL );
		if ( SUCCEEDED( hr ) )
		{
			hr = g_BsKernel.GetD3DDevice()->CreatePixelShader( (DWORD*) buffer->GetBufferPointer(), &m_YCrCbToRGBNoPixelAlpha );
			buffer->Release();

			if ( FAILED( hr ) )
			{
				return;
			}
		}
	}

	//
	// create a pixel shader that goes from YcRcB to RGB with an alpha plane
	//
	if ( m_YCrCbAToRGBA == 0 )
	{
		hr = D3DXCompileShader( StrYCrCbAToRGBA, sizeof( StrYCrCbAToRGBA ),
			0, 0, "main", "ps_2_0", 0, &buffer, NULL, NULL );
		if ( SUCCEEDED( hr ) )
		{
			hr = g_BsKernel.GetD3DDevice()->CreatePixelShader( (DWORD*) buffer->GetBufferPointer(), &m_YCrCbAToRGBA );
			buffer->Release();

			if ( FAILED( hr ) )
			{
				return;
			}
		}
	}

	//
	// create a vertex shader that just passes the m_Vertices straight through
	//
	if ( m_PositionAndTexCoordPassThrough == 0 )
	{
		hr = D3DXCompileShader( StrPositionAndTexCoordPassThrough, sizeof( StrPositionAndTexCoordPassThrough ),
			0, 0, "main", "vs_2_0", 0, &buffer, NULL, NULL );
		if ( SUCCEEDED( hr ) )
		{
			hr = g_BsKernel.GetD3DDevice()->CreateVertexShader( (DWORD*) buffer->GetBufferPointer(), &m_PositionAndTexCoordPassThrough );
			buffer->Release();

			if ( FAILED( hr ) )
			{
				return;
			}
		}
		else {
			MessageBox(NULL, "Failed to compile shader", "BlueSideEngine Failure", MB_OK);
		}
	}
}


void CBsMoviePlayer::FreeBinkTextures()
{
	// free the texture memory and then the textures directly
	/*
	for( int i = 0 ; i < m_BTS.binkfbuffers.TotalFrames ; i++ )
	{
		BINKFRAMETEXTURES * pBFT = &m_BTS.textures[i];

		if ( pBFT->Ytexture )
		{
			XPhysicalFree( m_BTS.binkfbuffers.Frames[ i ].YPlane.Buffer );
			delete pBFT->Ytexture;
			pBFT->Ytexture = 0;
		}
		if ( pBFT->cRtexture )
		{
			XPhysicalFree( m_BTS.binkfbuffers.Frames[ i ].cRPlane.Buffer );
			delete pBFT->cRtexture;
			pBFT->cRtexture = 0;
		}
		if ( pBFT->cBtexture )
		{
			XPhysicalFree( m_BTS.binkfbuffers.Frames[ i ].cBPlane.Buffer );
			delete pBFT->cBtexture;
			pBFT->cBtexture = 0;
		}
		if ( pBFT->Atexture )
		{
			XPhysicalFree( m_BTS.binkfbuffers.Frames[ i ].APlane.Buffer );
			delete pBFT->Atexture;
			pBFT->Atexture = 0;
		}
	}*/
}


DWORD CBsMoviePlayer::MakeBinkTexture( DWORD dwWidth, DWORD dwHeight, D3DFORMAT D3Dfmt, 
								  LPDIRECT3DTEXTURE9 * pTexture, void ** ppBits, U32 * pPitch, DWORD * pdwSize )

{
	DWORD size;
	IDirect3DTexture9 * texture;
	void * ptr;

	g_BsKernel.GetD3DDevice()->CreateTexture(dwWidth, dwHeight, 1, D3DUSAGE_DYNAMIC, D3Dfmt, D3DPOOL_DEFAULT, &texture, nullptr);
	if ( texture == 0 )
		return( 0 );
	// TODO: Create PC texture with read/write capability
	/*size = XGSetTextureHeader(dwWidth, dwHeight, 1, D3DUSAGE_CPU_CACHED_MEMORY, D3Dfmt, 0, 0, XGHEADER_CONTIGUOUS_MIP_OFFSET, 0, texture, 0, 0);

	size = ( ( size + GPU_TEXTURE_ALIGNMENT - 1 ) / GPU_TEXTURE_ALIGNMENT ) * GPU_TEXTURE_ALIGNMENT;

	//
	// Bink textures must be allocated in read-write cacheable memory!
	//
	ptr = XPhysicalAlloc( size, MAXULONG_PTR, GPU_TEXTURE_ALIGNMENT, PAGE_READWRITE );
	if ( ptr == 0 )
	{
		delete texture;
		return( 0 );
	}

	XGOffsetResourceAddress( texture, ptr );*/

	*pTexture = texture;
	*ppBits = ptr;
	*pdwSize = size;

	//
	// use lockrect to get the d3d pitch
	//
	D3DLOCKED_RECT lr;

	if ( SUCCEEDED( texture->LockRect( 0, &lr, 0, 0 ) ) )
	{
		*pPitch = lr.Pitch;
		texture->UnlockRect( 0 );
	}

	return( 1 );
}


DWORD CBsMoviePlayer::CreateBinkTextures()
{
	for( int i = 0 ; i < m_BTS.binkfbuffers.TotalFrames ; i++ )
	{
		m_BTS.textures[i].Ytexture = 0;
		m_BTS.textures[i].cBtexture = 0;
		m_BTS.textures[i].cRtexture = 0;
		m_BTS.textures[i].Atexture = 0;

		if ( m_BTS.binkfbuffers.Frames[i].YPlane.Allocate )
		{
			// create Y plane
			if ( ! MakeBinkTexture( m_BTS.binkfbuffers.YABufferWidth, 
				m_BTS.binkfbuffers.YABufferHeight,
				D3DFMT_L8,
				&m_BTS.textures[i].Ytexture, 
				&m_BTS.binkfbuffers.Frames[i].YPlane.Buffer, 
				&m_BTS.binkfbuffers.Frames[i].YPlane.BufferPitch, 
				&m_BTS.textures[i].Ysize ) )
			{
				FreeBinkTextures();
				return(0);
			}
		}

		if ( m_BTS.binkfbuffers.Frames[i].cRPlane.Allocate )
		{
			// create cR plane
			if ( ! MakeBinkTexture( m_BTS.binkfbuffers.cRcBBufferWidth, 
				m_BTS.binkfbuffers.cRcBBufferHeight,
				D3DFMT_L8,
				&m_BTS.textures[i].cRtexture, 
				&m_BTS.binkfbuffers.Frames[i].cRPlane.Buffer, 
				&m_BTS.binkfbuffers.Frames[i].cRPlane.BufferPitch, 
				&m_BTS.textures[i].cRsize ) )
			{
				FreeBinkTextures();
				return(0);
			}
		}

		if ( m_BTS.binkfbuffers.Frames[i].cBPlane.Allocate )
		{
			// create cB plane
			if ( ! MakeBinkTexture( m_BTS.binkfbuffers.cRcBBufferWidth, 
				m_BTS.binkfbuffers.cRcBBufferHeight,
				D3DFMT_L8,
				&m_BTS.textures[i].cBtexture, 
				&m_BTS.binkfbuffers.Frames[i].cBPlane.Buffer, 
				&m_BTS.binkfbuffers.Frames[i].cBPlane.BufferPitch, 
				&m_BTS.textures[i].cBsize ) )
			{
				FreeBinkTextures();
				return(0);
			}
		}

		if ( m_BTS.binkfbuffers.Frames[i].APlane.Allocate )
		{
			// create alpha plane
			if ( ! MakeBinkTexture( m_BTS.binkfbuffers.YABufferWidth, 
				m_BTS.binkfbuffers.YABufferHeight,
				D3DFMT_L8,
				&m_BTS.textures[i].Atexture, 
				&m_BTS.binkfbuffers.Frames[i].APlane.Buffer, 
				&m_BTS.binkfbuffers.Frames[i].APlane.BufferPitch, 
				&m_BTS.textures[i].Asize ) )
			{
				FreeBinkTextures();
				return(0);
			}
		}  
	}

	return(1);
}


void CBsMoviePlayer::RenderBinkTextures( float fA )
{
	//
	// Turn on texture filtering and texture clamping
	//

	for( int i = 0 ; i < 4 ; i++ )
	{
		g_BsKernel.GetDevice()->SetSamplerState( i, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );
		g_BsKernel.GetDevice()->SetSamplerState( i, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
		g_BsKernel.GetDevice()->SetSamplerState( i, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP );
		g_BsKernel.GetDevice()->SetSamplerState( i, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP );
		g_BsKernel.GetDevice()->SetSamplerState( i, D3DSAMP_ADDRESSW, D3DTADDRESS_CLAMP );
	}

	//
	// Select the texture.
	//

	g_BsKernel.GetDevice()->SetTexture( 0, m_BTS.textures[ m_BTS.binkfbuffers.FrameNum ].Ytexture );
	g_BsKernel.GetDevice()->SetTexture( 1, m_BTS.textures[ m_BTS.binkfbuffers.FrameNum ].cRtexture );
	g_BsKernel.GetDevice()->SetTexture( 2, m_BTS.textures[ m_BTS.binkfbuffers.FrameNum ].cBtexture );

	//
	// turn off Z buffering, culling, and projection (since we are drawing orthographically)
	//

	g_BsKernel.GetDevice()->SetRenderState( D3DRS_ZENABLE, FALSE );
	g_BsKernel.GetDevice()->SetRenderState( D3DRS_ZWRITEENABLE, FALSE );
	g_BsKernel.GetDevice()->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );


	//
	// upload the YUV to RGB matrix
	//

	yuvtorgb[15] = fA;
	g_BsKernel.GetD3DDevice()->SetPixelShaderConstantF( 0, yuvtorgb, 4 );


	//
	// set the vertex format and turn on our vertex shader
	//

	g_BsKernel.GetD3DDevice()->SetFVF( POS_TC_VERTEX_FVF );
	g_BsKernel.GetDevice()->SetVertexShader( m_PositionAndTexCoordPassThrough );


	//
	// are we using an alpha plane? if so, turn on the 4th texture and set our pixel shader
	//

	if ( m_BTS.textures[ m_BTS.binkfbuffers.FrameNum ].Atexture )
	{
		//
		// set the alpha texture
		//

		g_BsKernel.GetDevice()->SetTexture( 3, m_BTS.textures[ m_BTS.binkfbuffers.FrameNum ].Atexture );

		//
		// turn on our pixel shader
		//

		g_BsKernel.GetDevice()->SetPixelShader( m_YCrCbAToRGBA );
		g_BsKernel.GetDevice()->SetRenderState( D3DRS_ALPHABLENDENABLE, 1 );
		g_BsKernel.GetDevice()->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
		g_BsKernel.GetDevice()->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
	}
	else
	{
		//
		// turn on our pixel shader
		//

		g_BsKernel.GetDevice()->SetPixelShader( m_YCrCbToRGBNoPixelAlpha );
	}

	//
	// are we completely opaque or somewhat transparent?
	//
	if ( fA >= 0.999f )
	{
		g_BsKernel.GetDevice()->SetRenderState( D3DRS_ALPHABLENDENABLE, 0 );
	}
	else
	{
		g_BsKernel.GetDevice()->SetRenderState( D3DRS_ALPHABLENDENABLE, 1 );
		g_BsKernel.GetDevice()->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
		g_BsKernel.GetDevice()->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
	}

	g_BsKernel.GetDevice()->DrawPrimitiveUP( D3DPT_TRIANGLESTRIP, 2, &m_Vertices, sizeof(POS_TC_VERTEX) );

	g_BsKernel.GetDevice()->SetTexture( 0, 0 );
	g_BsKernel.GetDevice()->SetTexture( 1, 0 );
	g_BsKernel.GetDevice()->SetTexture( 2, 0 );

	if ( m_BTS.textures[ m_BTS.binkfbuffers.FrameNum ].Atexture )
	{
		g_BsKernel.GetDevice()->SetTexture( 3, 0 );
	}


	// Restore Address mode
	for( int i = 0 ; i < 4 ; i++ )
	{
		g_BsKernel.GetDevice()->SetSamplerState( i, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP );
		g_BsKernel.GetDevice()->SetSamplerState( i, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP );
		g_BsKernel.GetDevice()->SetSamplerState( i, D3DSAMP_ADDRESSW, D3DTADDRESS_WRAP );
	}
}


//
// simple routine to store the cache of a memory range
//

#ifdef _XBOX

#include <ppcintrinsics.h>

static __forceinline void store_cache( void * ptr, U32 size )
{
	U32 i;

	for ( i = 0 ; i < size ; i += 128 )
	{
		__dcbst( 0, ptr );
		ptr = ( (U8*) ptr ) + 128;
	}
}


// sync the textures out to main memory, so that the GPU can see them (this is fast)
void CBsMoviePlayer::SyncBinkTextures()
{
	// store out the cache
	for( int i = 0 ; i < m_BTS.binkfbuffers.TotalFrames ; i++ )
	{
		store_cache( m_BTS.binkfbuffers.Frames[i].YPlane.Buffer,  m_BTS.textures[i].Ysize );
		store_cache( m_BTS.binkfbuffers.Frames[i].cRPlane.Buffer, m_BTS.textures[i].cRsize );
		store_cache( m_BTS.binkfbuffers.Frames[i].cBPlane.Buffer, m_BTS.textures[i].cBsize );
		store_cache( m_BTS.binkfbuffers.Frames[i].APlane.Buffer,  m_BTS.textures[i].Asize );
	}
}


// make sure the GPU is done with the textures that we are about to write info
void CBsMoviePlayer::WaitOnBinkTextures()
{
	S32 next;

	next = m_BTS.binkfbuffers.FrameNum + 1;
	if ( ( next >= m_BTS.binkfbuffers.TotalFrames ) || ( m_BTS.textures[ next ].Ytexture == 0 ) )
		next = 0;

	if ( m_BTS.textures[ next ].Ytexture )
	{
		//
		// block until the texture is ready
		//

		m_BTS.textures[ next ].Ytexture->BlockUntilNotBusy();
	}
}
#else
void CBsMoviePlayer::SyncBinkTextures()
{

}
void CBsMoviePlayer::WaitOnBinkTextures()
{

}
#endif

void CBsMoviePlayer::GetBinkSummary(BINKSUMMARY PTR4* sum)
{
	if(!m_hBink)
	{
		BsAssert(0 && "BinkOpen Function failed to call");
		return;
	}
	BinkGetSummary( m_hBink, sum);
}


HRESULT CBsMoviePlayer::InitBinkMovie( const VOID *pMovie, const bool bInMemory )
{
	HRESULT hr = S_OK;
	U32 u32Flags = BINKNOFRAMEBUFFERS;

	if(bInMemory)
		u32Flags |= BINKFROMMEMORY;

#if 1
	//--- tohoshi
//	XAUDIOENGINEINIT pXAudio = {0};
	//pXAudio.SubmixStageCount = 1;	// DSP
	//XAudioInitialize( &pXAudio );
	
	//--- 	
#endif	

//	BinkSoundUseDirectSound();
	m_hBink = BinkOpen( (const char*)pMovie, u32Flags);

	if(!m_hBink)
	{
		char *pError = BinkGetError();
		BsAssert2(0, pError );
		hr = E_FAIL;
	}
	else
	{
		//XAudioInitialize( NULL );

		BinkGetFrameBuffersInfo( m_hBink, &m_BTS.binkfbuffers );

		if (!CreateBinkTextures())
		{
			BsAssert(0 && "Unable to create Bink buffers");
			BinkClose(m_hBink);
			m_hBink = NULL;
			hr = E_FAIL;
		}
		else
		{
			//
			// Register our locked texture pointers with Bink
			//
			BinkRegisterFrameBuffers(m_hBink, &m_BTS.binkfbuffers);

			m_Vertices[0].sx = 0.f;
			m_Vertices[0].sy = 0.f;
			m_Vertices[0].sz = 0.f;
			m_Vertices[0].tu = 0.f;
			m_Vertices[0].tv = 0.f;
			m_Vertices[1] = m_Vertices[0];
			m_Vertices[1].sx = (float)m_hBink->Width;
			m_Vertices[1].tu = 1.f;
			m_Vertices[2] = m_Vertices[0];
			m_Vertices[2].sy = (float)m_hBink->Height;
			m_Vertices[2].tv = 1.f;
			m_Vertices[3] = m_Vertices[1];
			m_Vertices[3].sy = m_Vertices[2].sy;
			m_Vertices[3].tv = 1.f;
		}
	}
	return hr;
}


void CBsMoviePlayer::RenderNextFrame()
{
	g_BsKernel.BeginPIXEvent(0,0,0, "CBsMoviePlayer::RenderNextFrame()");
	if(!m_hBink)
	{
		BsAssert(0 && "No Bink object");
		g_BsKernel.EndPIXEvent();
		return;
	}

	if (!BinkWait(m_hBink))
	{
		WaitOnBinkTextures();
		BinkDoFrame( m_hBink );

		if (!BinkShouldSkip( m_hBink ))
		{
			SyncBinkTextures();
			RenderBinkTextures( 1.0f );
		}
		else
		{
			DebugString( "Bink Skipped Frame Number: %d\n", m_hBink->FrameNum );
		}

		BinkNextFrame(m_hBink);
	}
	else
	{
		SyncBinkTextures();
		RenderBinkTextures( 1.0f );
	}
	g_BsKernel.EndPIXEvent();
}

void CBsMoviePlayer::Stop()
{
	WaitOnBinkTextures();
	FreeBinkTextures();
	memset(&m_BTS, 0, sizeof(m_BTS));

	if(m_hBink)
	{
		//XAudioShutDown();
		BinkClose(m_hBink);
		m_hBink = NULL;
	}
}

void CBsMoviePlayer::Pause()
{
	if(m_hBink)
		BinkPause(m_hBink, 1);
}


void CBsMoviePlayer::Resume()
{
	if(m_hBink)
		BinkPause(m_hBink, 0);
}

//--- tohoshi
void CBsMoviePlayer::SetVolume(S32 nVolume)
{
	if(m_hBink)
	{
		for(int i = 0; i < 6; i ++)
			BinkSetVolume(m_hBink, i, nVolume);
	}
}
//---

void CBsMoviePlayer::PlayMovie(const char *szMoviePath,
							   const bool bInMemory,
							   SKIPMOVIECALLBACK pCallback,
							   const bool bSkippable )
{
	/*
#if 1
	//--- tohoshi
	XAUDIOENGINEINIT pXAudio = {0};
	pXAudio.SubmixStageCount = 1;	// DSP
	XAudioInitialize( &pXAudio );
	//--- 	
#endif	
	
	BinkSoundUseXAudio();
	//
	// Tell Bink to play all of the 5.1 tracks
	//
	U32 TrackIDsToPlay[ 6 ] = { 0, 1, 2, 3, 4, 5 };
	BinkSetSoundTrack( 6, TrackIDsToPlay );*/

	U32 u32Flags = BINKSNDTRACK|BINKNOFRAMEBUFFERS;

	if(bInMemory)
		u32Flags |= BINKFROMMEMORY;

	m_hBink = BinkOpen( szMoviePath,  u32Flags);

	if(!m_hBink)
	{
		char *pError = BinkGetError();
		BsAssert2(0, pError );
	}
	else
	{
		//XAudioInitialize( NULL );
		BinkGetFrameBuffersInfo( m_hBink,&m_BTS.binkfbuffers );

		if ( CreateBinkTextures() )
		{
			//
			// Register our locked texture pointers with Bink
			//
			BinkRegisterFrameBuffers( m_hBink, &m_BTS.binkfbuffers );

			/*
			//
			// Now route the sound tracks to the correct speaker
			//
			// Now route the sound tracks to the correct speaker
			U32 bins[ 6 ] = {0};

			bins[ 0 ] = XAUDIOSPEAKER_FRONTLEFT;
			BinkSetMixBins( m_hBink, 0, bins, 1 );
			bins[ 0 ] = XAUDIOSPEAKER_FRONTRIGHT;
			BinkSetMixBins( m_hBink, 1, bins, 1 );
			bins[ 0 ] = XAUDIOSPEAKER_FRONTCENTER;
			BinkSetMixBins( m_hBink, 2, bins, 1 );
			bins[ 0 ] = XAUDIOSPEAKER_LOWFREQUENCY;
			BinkSetMixBins( m_hBink, 3, bins, 1 );
			bins[ 0 ] = XAUDIOSPEAKER_BACKLEFT;
			BinkSetMixBins( m_hBink, 4, bins, 1 );
			bins[ 0 ] = XAUDIOSPEAKER_BACKRIGHT;
			BinkSetMixBins( m_hBink, 5, bins, 1 );*/

			g_BsKernel.GetDevice()->SaveState();

#ifdef _XBOX
			DWORD dwPI;
			// On Xbox, we have to change the presentation interval to match the refresh
			// rate of the bink video
			g_BsKernel.GetDevice()->GetRenderState(D3DRS_PRESENTINTERVAL, &dwPI);
			g_BsKernel.GetDevice()->SetRenderState(D3DRS_PRESENTINTERVAL, D3DPRESENT_INTERVAL_ONE);
#endif

			float fX,fY;

			fX = (g_BsKernel.GetDevice()->GetBackBufferWidth() - m_hBink->Width) / 2.0f;
			fY = (g_BsKernel.GetDevice()->GetBackBufferHeight() - m_hBink->Height) / 2.0f;

			m_Vertices[0].sx = fX;
			m_Vertices[0].sy = fY;
			m_Vertices[0].sz = 0.0F;
			m_Vertices[0].tu = 0.0f;
			m_Vertices[0].tv = 0.0f;
			m_Vertices[1] = m_Vertices[0];
			m_Vertices[1].sx = fX + (float)m_hBink->Width;
			m_Vertices[1].tu = 1.0f;
			m_Vertices[2] = m_Vertices[0];
			m_Vertices[2].sy = fY + (float)m_hBink->Height;
			m_Vertices[2].tv = 1.0f;
			m_Vertices[3] = m_Vertices[1];
			m_Vertices[3].sy = m_Vertices[2].sy;
			m_Vertices[3].tv = 1.0f;

			while(1)
			{
				if (!BinkWait(m_hBink))
				{
					WaitOnBinkTextures();
					BinkDoFrame( m_hBink );

					if (!BinkShouldSkip( m_hBink ))
					{
						SyncBinkTextures();

						g_BsKernel.GetDevice()->BeginScene();
						g_BsKernel.GetD3DDevice()->Clear(0, 0, D3DCLEAR_TARGET, 0, 1.0f, 0);

						RenderBinkTextures( 1.0f );

						g_BsKernel.GetDevice()->EndScene();

						MGSToolbox::EndFrame();
						g_BsKernel.GetDevice()->ShowFrame();
						MGSToolbox::BeginFrame();

					}
					else
					{
						DebugString( "Bink Skipped Frame Number: %d\n", m_hBink->FrameNum );
					}
					
					if((pCallback && bSkippable && pCallback(this)) || m_hBink->FrameNum == m_hBink->Frames )
					{
						break;
					}
					BinkNextFrame(m_hBink);
				}
			}
#ifdef _XBOX
			g_BsKernel.GetDevice()->SetRenderState(D3DRS_PRESENTINTERVAL, dwPI);
#endif
			g_BsKernel.GetDevice()->RestoreState();
			WaitOnBinkTextures();
			FreeBinkTextures();
		}
		BinkClose(m_hBink);
		m_hBink = NULL;
	}
	//XAudioShutDown();
}
