// Copyright 2005 Microsoft Corporation

#include "stdafx.h"
#include "GPUStateSave.h"

#ifdef USE_MGSTOOLBOX

#include <xtl.h>
#include <assert.h>

#include "Utils.h"
#include "Shaders.h"

namespace MGSToolbox
{

//-----------------------------------------------------------------------------

GPUStateSave::GPUStateSave ( void )
: m_renderStateCount(0),
  m_textureCount(0)
{
	g_pDevice->GetVertexDeclaration(&m_pVertexDecl);
	g_pDevice->GetVertexShader(&m_pVertexShader);
	g_pDevice->GetPixelShader(&m_pPixelShader);
}

//----------

GPUStateSave::~GPUStateSave ( void )
{
	g_pDevice->SetVertexShader(m_pVertexShader);
	g_pDevice->SetPixelShader(m_pPixelShader);
	g_pDevice->SetVertexDeclaration(m_pVertexDecl);

	if(m_pVertexShader) m_pVertexShader->Release();
	if(m_pPixelShader) m_pPixelShader->Release();
	if(m_pVertexDecl) m_pVertexDecl->Release();

	// set render states back in reverse order so we unroll the changes correctly.

	for(int i = m_renderStateCount - 1; i >= 0; i--)
	{
		g_pDevice->SetRenderState(m_renderStateNames[i],m_renderStateValues[i]);
	}

	// set textures back in reverse order so we unroll the changes correctly.

	for(int i = m_textureCount - 1; i >= 0; i--)
	{
		g_pDevice->SetTexture(m_textureNames[i],m_textureValues[i]);
		if(m_textureValues[i]) m_textureValues[i]->Release();
	}
}

//-----------------------------------------------------------------------------

void GPUStateSave::SetRenderState ( D3DRENDERSTATETYPE name, DWORD value )
{
	assert((m_renderStateCount < 32) && "GPUStateSave::SetRenderState - Trying to save more than 32 render states");

	DWORD oldValue;
	g_pDevice->GetRenderState(name,&oldValue);

	if(oldValue != value)
	{
		m_renderStateNames[m_renderStateCount] = name;
		m_renderStateValues[m_renderStateCount] = oldValue;

		m_renderStateCount++;

		g_pDevice->SetRenderState(name,value);
	}
}

//----------

void GPUStateSave::SetFVF ( DWORD fvf )
{
	g_pDevice->SetFVF(fvf);
}

//----------

void GPUStateSave::SetVertexDeclaration ( IDirect3DVertexDeclaration9 * pDecl )
{
	g_pDevice->SetVertexDeclaration(pDecl);
}

//----------

void GPUStateSave::SetPixelShader ( IDirect3DPixelShader9 * pShader )
{
	g_pDevice->SetPixelShader(pShader);
}

//----------

void GPUStateSave::SetVertexShader ( IDirect3DVertexShader9 * pShader )
{
	g_pDevice->SetVertexShader(pShader);
}

//----------

void GPUStateSave::SetTexture ( DWORD sampler, IDirect3DBaseTexture9 * pTexture )
{
	assert((m_textureCount < 32) && "GPUStateSave::SetTexture  - Trying to save more than 32 textures");

	IDirect3DBaseTexture9 * oldTexture;
	g_pDevice->GetTexture(sampler,&oldTexture);

	if(oldTexture != pTexture)
	{
		m_textureNames[m_textureCount] = sampler;
		m_textureValues[m_textureCount] = oldTexture;

		m_textureCount++;

		g_pDevice->SetTexture(sampler,pTexture);
	}
}

//-----------------------------------------------------------------------------

RenderState_2D_PCTS::RenderState_2D_PCTS ( IDirect3DTexture9 * pTexture )
{
	SetTexture(0,pTexture);

	SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX2);
	SetVertexShader( Shaders::g_vsPassthruPCT );
	SetPixelShader( Shaders::g_psMultiplyCT );

	SetRenderState(D3DRS_VIEWPORTENABLE,FALSE);
	SetRenderState(D3DRS_ALPHABLENDENABLE,FALSE);
	SetRenderState(D3DRS_ALPHATESTENABLE,FALSE);
	SetRenderState(D3DRS_ZENABLE,D3DZB_FALSE);
	SetRenderState(D3DRS_STENCILENABLE,FALSE);
	SetRenderState(D3DRS_ZWRITEENABLE,FALSE);
	SetRenderState(D3DRS_CLIPPLANEENABLE,0);
	SetRenderState(D3DRS_SCISSORTESTENABLE,FALSE);
	SetRenderState(D3DRS_COLORWRITEENABLE,D3DCOLORWRITEENABLE_ALL);
	SetRenderState(D3DRS_CULLMODE,D3DCULL_NONE);
	SetRenderState(D3DRS_HALFPIXELOFFSET,FALSE);
	SetRenderState(D3DRS_HIGHPRECISIONBLENDENABLE,FALSE);
}

//-----------------------------------------------------------------------------

RenderState_2D_PCT::RenderState_2D_PCT ( IDirect3DTexture9 * pTexture )
{
	SetTexture(0,pTexture);

	SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX2);
	SetVertexShader( Shaders::g_vsPassthruPCT );
	SetPixelShader( Shaders::g_psMultiplyCT );

	SetRenderState(D3DRS_VIEWPORTENABLE,FALSE);
	SetRenderState(D3DRS_ALPHABLENDENABLE,TRUE);
	SetRenderState(D3DRS_ALPHATESTENABLE,FALSE);
	SetRenderState(D3DRS_ZENABLE,D3DZB_FALSE);
	SetRenderState(D3DRS_STENCILENABLE,FALSE);
	SetRenderState(D3DRS_ZWRITEENABLE,FALSE);
	SetRenderState(D3DRS_CLIPPLANEENABLE,0);
	SetRenderState(D3DRS_SCISSORTESTENABLE,FALSE);
	SetRenderState(D3DRS_COLORWRITEENABLE,D3DCOLORWRITEENABLE_ALL);
	SetRenderState(D3DRS_CULLMODE,D3DCULL_NONE);
	SetRenderState(D3DRS_SRCBLEND,D3DBLEND_SRCALPHA);
	SetRenderState(D3DRS_DESTBLEND,D3DBLEND_INVSRCALPHA);
	SetRenderState(D3DRS_BLENDOP,D3DBLENDOP_ADD);
	SetRenderState(D3DRS_HALFPIXELOFFSET,FALSE);
	SetRenderState(D3DRS_HIGHPRECISIONBLENDENABLE,TRUE);
}

//-----------------------------------------------------------------------------

RenderState_2D_PC::RenderState_2D_PC ( void )
{
	SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE);
	SetVertexShader( Shaders::g_vsPassthruPC );
	SetPixelShader( Shaders::g_psPassthruC );

	SetRenderState(D3DRS_VIEWPORTENABLE,FALSE);
	SetRenderState(D3DRS_ALPHABLENDENABLE,TRUE);
	SetRenderState(D3DRS_ALPHATESTENABLE,FALSE);
	SetRenderState(D3DRS_ZENABLE,D3DZB_FALSE);
	SetRenderState(D3DRS_STENCILENABLE,FALSE);
	SetRenderState(D3DRS_ZWRITEENABLE,FALSE);
	SetRenderState(D3DRS_CLIPPLANEENABLE,0);
	SetRenderState(D3DRS_SCISSORTESTENABLE,FALSE);
	SetRenderState(D3DRS_COLORWRITEENABLE,D3DCOLORWRITEENABLE_ALL);
	SetRenderState(D3DRS_CULLMODE,D3DCULL_NONE);
	SetRenderState(D3DRS_SRCBLEND,D3DBLEND_SRCALPHA);
	SetRenderState(D3DRS_DESTBLEND,D3DBLEND_INVSRCALPHA);
	SetRenderState(D3DRS_BLENDOP,D3DBLENDOP_ADD);
	SetRenderState(D3DRS_HALFPIXELOFFSET,FALSE);
	SetRenderState(D3DRS_HIGHPRECISIONBLENDENABLE,TRUE);
}

//-----------------------------------------------------------------------------

}; // namespace MGSToolbox {

#endif