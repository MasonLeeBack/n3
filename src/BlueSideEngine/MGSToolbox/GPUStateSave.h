// Copyright © 2005 Microsoft Corporation

#pragma once

#include "MGSToolboxConfig.h"
#ifdef USE_MGSTOOLBOX

namespace MGSToolbox {

//-----------------------------------------------------------------------------

class GPUStateSave
{
public:

	GPUStateSave ( void );
	~GPUStateSave ();

	//----------

	void SetRenderState ( D3DRENDERSTATETYPE name, DWORD value );

	void SetFVF ( DWORD fvf );

	void SetVertexDeclaration ( IDirect3DVertexDeclaration9 * pDecl );

	void SetPixelShader ( IDirect3DPixelShader9 * pShader );
	void SetVertexShader ( IDirect3DVertexShader9 * pShader );

	void SetTexture ( DWORD sampler, IDirect3DBaseTexture9 * pTexture );

	//----------

protected:

	int m_renderStateCount;
	D3DRENDERSTATETYPE m_renderStateNames[32];
	DWORD m_renderStateValues[32];

	int m_textureCount;
	int m_textureNames[32];
	IDirect3DBaseTexture9 * m_textureValues[32];

	IDirect3DVertexDeclaration9 * m_pVertexDecl;
	IDirect3DVertexShader9 * m_pVertexShader;
	IDirect3DPixelShader9 * m_pPixelShader;
};

//-----------------------------------------------------------------------------
// Prefab states for convenience

// Pass through color with alpha blending

class RenderState_2D_PC : public GPUStateSave
{
public:

	RenderState_2D_PC ( void );
};

//----------

// Multiply color * texture with alpha blending

class RenderState_2D_PCT : public GPUStateSave
{
public:

	RenderState_2D_PCT ( IDirect3DTexture9 * pTexture );
};

//----------

// Multiply color * texture without alpha blending

class RenderState_2D_PCTS : public GPUStateSave
{
public:

	RenderState_2D_PCTS ( IDirect3DTexture9 * pTexture );
};

//-----------------------------------------------------------------------------

}; // namespace MGSToolbox {

#endif // USE_MGSTOOLBOX