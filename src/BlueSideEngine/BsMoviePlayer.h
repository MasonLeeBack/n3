#pragma once

//--- tohoshi
//typedef bool (*SKIPMOVIECALLBACK)();
typedef bool (*SKIPMOVIECALLBACK)(void* pParam);

#include <bink.h>

typedef struct BINKFRAMETEXTURES
{
	LPDIRECT3DTEXTURE9 Ytexture;
	LPDIRECT3DTEXTURE9 cRtexture;
	LPDIRECT3DTEXTURE9 cBtexture;
	LPDIRECT3DTEXTURE9 Atexture;
	DWORD Ysize;
	DWORD cRsize;
	DWORD cBsize;
	DWORD Asize;
} BINKFRAMETEXTURES;


typedef struct BINKTEXTURESET
{
	BINKFRAMETEXTURES textures[ BINKMAXFRAMEBUFFERS ];
	BINKFRAMEBUFFERS binkfbuffers;
} BINKTEXTURESET;

typedef struct POS_TC_VERTEX 
{
	float sx, sy, sz;  // Screen coordinates
	float tu, tv;      // Texture coordinates 
} POS_TC_VERTEX;


class CBsMoviePlayer
{
public:
	CBsMoviePlayer();
	~CBsMoviePlayer();
	void Initialize();
	void PlayMovie(	const char *szMoviePath,
					const bool bInMemory = false,
					SKIPMOVIECALLBACK pCallback = NULL,
					const bool bSkippable = true);
	void Stop();
	void Pause();
	void Resume();
	HRESULT InitBinkMovie( const VOID *pMovie, const bool bInMemory = false);
	void GetBinkSummary(BINKSUMMARY PTR4* sum);
	void RenderNextFrame();
	
	//--- tohoshi
	void SetVolume(S32 nVolume);
	//---

protected:
	DWORD CreateBinkTextures();
	void FreeBinkTextures();
	DWORD MakeBinkTexture( DWORD dwWidth, DWORD dwHeight, D3DFORMAT D3Dfmt, 
		LPDIRECT3DTEXTURE9 * pTexture, void ** ppBits, U32 * pdwPitch, DWORD * pdwSize );
	void RenderBinkTextures( float fA );
	void SyncBinkTextures();
	void WaitOnBinkTextures();

#ifdef _XBOX
	D3DPixelShader  * m_YCrCbToRGBNoPixelAlpha;
	D3DPixelShader  * m_YCrCbAToRGBA;
	D3DVertexShader * m_PositionAndTexCoordPassThrough;
#else
	IDirect3DPixelShader9* m_YCrCbToRGBNoPixelAlpha;
	IDirect3DPixelShader9* m_YCrCbAToRGBA;
	IDirect3DVertexShader9* m_PositionAndTexCoordPassThrough;
#endif
	BINKTEXTURESET  m_BTS;
	HBINK			m_hBink;
	POS_TC_VERTEX	m_Vertices[4];
};


extern CBsMoviePlayer	g_BsMoviePlayer;