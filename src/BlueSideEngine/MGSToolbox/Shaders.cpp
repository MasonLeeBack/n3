// Copyright 2005 Microsoft Corporation

#include "stdafx.h"
#include "Shaders.h"

#ifdef USE_MGSTOOLBOX

#include <xtl.h>
#include <xgraphics.h>

namespace MGSToolbox {
namespace Shaders {

IDirect3DVertexShader9 * g_vsPassthruPC;
IDirect3DVertexShader9 * g_vsPassthruPCT;

IDirect3DPixelShader9 * g_psPassthruC;
IDirect3DPixelShader9 * g_psMultiplyCT;

//-----------------------------------------------------------------------------

IDirect3DVertexShader9 * CreateVertexShader ( const DWORD * pFunction )
{
	XGMICROCODESHADERPARTS parts;
	XGGetMicrocodeShaderParts( pFunction, &parts );

	IDirect3DVertexShader9* pVS = (IDirect3DVertexShader9*) new char[parts.cbCachedPartSize];
	XGSetVertexShaderHeader( pVS, parts.cbCachedPartSize, &parts );

	VOID* pPhysicalAddress = XPhysicalAlloc( parts.cbPhysicalPartSize, 
											MAXULONG_PTR,
											D3DSHADER_ALIGNMENT,
											PAGE_READWRITE );
	memcpy( pPhysicalAddress, parts.pPhysicalPart, parts.cbPhysicalPartSize );
	XGRegisterVertexShader( pVS, pPhysicalAddress );

	return pVS;
}

//----------

IDirect3DPixelShader9 * CreatePixelShader ( const DWORD * pFunction )
{
	XGMICROCODESHADERPARTS parts;
	XGGetMicrocodeShaderParts( pFunction, &parts );

	IDirect3DPixelShader9* pPS = (IDirect3DPixelShader9*) new char[parts.cbCachedPartSize];
	XGSetPixelShaderHeader( pPS, parts.cbCachedPartSize, &parts );

	VOID* pPhysicalAddress = XPhysicalAlloc( parts.cbPhysicalPartSize, 
											MAXULONG_PTR,
											D3DSHADER_ALIGNMENT,
											PAGE_READWRITE | PAGE_WRITECOMBINE );
	memcpy( pPhysicalAddress, parts.pPhysicalPart, parts.cbPhysicalPartSize );
	XGRegisterPixelShader( pPS, pPhysicalAddress );

	return pPS;
}

//-----------------------------------------------------------------------------

void Install ( void )
{
	g_vsPassthruPC = CreateVertexShader ( g_vsdPassthruPC );
	g_vsPassthruPCT = CreateVertexShader ( g_vsdPassthruPCT );

	g_psPassthruC = CreatePixelShader ( g_psdPassthruC );
	g_psMultiplyCT = CreatePixelShader ( g_psdMultiplyCT );
}

void Remove ( void )
{
	delete g_vsPassthruPC;
	delete g_vsPassthruPCT;

	delete g_psPassthruC;
	delete g_psMultiplyCT;

	g_vsPassthruPC = 0;
	g_vsPassthruPCT = 0;

	g_psPassthruC = 0;
	g_psMultiplyCT = 0;
}

//-------------------------------------------------------------------------------------

/*
struct VS_IN
{
	float4 P : POSITION;
	float4 C : COLOR;
};

struct VS_OUT
{
	float4 P : POSITION;
	float4 C : COLOR;
};

VS_OUT main( VS_IN In )
{
	VS_OUT Out;
	Out.P =	In.P;
	Out.C =	In.C;
	return Out;
}
*/

const DWORD g_vsdPassthruPC[] =
{
	0x102A0E01,	0x000000B0,	0x00000054,	0x00000000,	0x00000018,	0x00000074,	0x00000058,	0x0000001C,
	0x00000023,	0xFFFE0300,	0x00000000,	0x00000000,	0x00000100,	0x0000001C,	0x76735F33,	0x5F30004D,
	0x6963726F,	0x736F6674,	0x20285229,	0x2058626F,	0x78203336,	0x30205368,	0x61646572,	0x20436F6D,
	0x70696C65,	0x7220322E,	0x302E3231,	0x33352E30,	0x00ABABAB,	0x00010001,	0x00000000,	0x00000000,
	0x00000000,	0x00001021,	0x00000000,	0x00000001,	0x00000002,	0x00000001,	0x00000000,	0x00000290,
	0x00100003,	0x0030A004,	0x0000F0A0,	0x00001006,	0x30052003,	0x00001200,	0xC2000000,	0x00001005,
	0x00001200,	0xC4000000,	0x00001006,	0x00002200,	0x00000000,	0x05F81000,	0x00000688,	0x00000000,
	0x05F80000,	0x00000688,	0x00000000,	0xC80F803E,	0x00000000,	0xC2010100,	0xC80F8000,	0x00000000,
	0xC2000000,
};

//-----------------------------------------------------------------------------

/*
struct PS_IN
{
	float4 C : COLOR_centroid;
};

float4 main( PS_IN In )	: COLOR
{
	return In.C;
}
*/


const DWORD g_psdPassthruC[] =
{
	0x102A0E00,	0x00000098,	0x00000018,	0x00000000,	0x00000018,	0x00000074,	0x00000058,	0x0000001C,
	0x00000023,	0xFFFF0300,	0x00000000,	0x00000000,	0x00000100,	0x0000001C,	0x70735F33,	0x5F30004D,
	0x6963726F,	0x736F6674,	0x20285229,	0x2058626F,	0x78203336,	0x30205368,	0x61646572,	0x20436F6D,
	0x70696C65,	0x7220322E,	0x302E3231,	0x33352E30,	0x00ABABAB,	0x10000000,	0x00000000,	0x00000000,
	0x00000000,	0x00001021,	0x00000000,	0x00000001,	0x00000001,	0x0000F0A0,	0x00000000,	0x1001C400,
	0x22000000,	0xC80F8000,	0x00000000,	0xC2000000,
};




//-------------------------------------------------------------------------------------

/*
struct VS_IN
{
	float4 P : POSITION;
	float4 C : COLOR;
	float4 T : TEXCOORD;
};

struct VS_OUT
{
	float4 P : POSITION;
	float4 C : COLOR;
	float4 T : TEXCOORD0;
};

VS_OUT main( VS_IN In )
{
	VS_OUT Out;
	Out.P =	In.O;
	Out.C =	In.C;
	Out.T =	In.T;
	return Out;
}
*/

const DWORD g_vsdPassthruPCT[] =
{
	0x102a0e01,	0x000000bc,	0x0000006c,	0x00000000,	0x00000018,	0x00000074,	
	0x00000058,	0x0000001c,	0x00000023,	0xfffe0300,	0x00000000,	0x00000000,	
	0x00000100,	0x0000001c,	0x76735f33,	0x5f30004d,	0x6963726f,	0x736f6674,	
	0x20285229,	0x2058626f,	0x78203336,	0x30205368,	0x61646572,	0x20436f6d,	
	0x70696c65,	0x7220322e,	0x302e3231,	0x33352e30,	0x00ababab,	0x00110002,	
	0x00000000,	0x00000000,	0x00000000,	0x00002042,	0x00000000,	0x00000001,	
	0x00000003,	0x00000002,	0x00000000,	0x00000290,	0x00100003,	0x0000a004,	
	0x00305005,	0x0000f050,	0x0001f1a0,	0x00001008,	0x00001007,	0x70153003,	
	0x00001200,	0xc2000000,	0x00001006,	0x00001200,	0xc4000000,	0x00002007,	
	0x00002200,	0x00000000,	0x05f82000,	0x00000688,	0x00000000,	0x05f81000,	
	0x00000688,	0x00000000,	0x05f80000,	0x00000688,	0x00000000,	0xc80f803e,	
	0x00000000,	0xc2020200,	0xc80f8001,	0x00000000,	0xc2010100,	0xc80f8000,	
	0x00000000,	0xc2000000
};

//-------------------------------------------------------------------------------------

/*
sampler	tex0 : register(s0)

struct PS_IN
{
	float4 C : COLOR_centroid;
	float4 T : TEXCOORD_centroid;
};

float4 main( PS_IN In )	: COLOR
{
	return In.C	* tex2D(tex0,In.T).r;
}
*/

const DWORD	g_psdMultiplyCT[] =
{
    0x102a0e00, 0x000000c8, 0x00000030, 0x00000000, 0x00000018, 0x000000a0, 
    0x00000084, 0x0000001c, 0x0000004f, 0xffff0300, 0x00000001, 0x0000001c, 
    0x00000100, 0x00000048, 0x00000030, 0x00030000, 0x00010000, 0x00000038, 
    0x00000000, 0x74657830, 0x00ababab, 0x0004000c, 0x00010001, 0x00010000, 
    0x00000000, 0x70735f33, 0x5f30004d, 0x6963726f, 0x736f6674, 0x20285229, 
    0x2058626f, 0x78203336, 0x30205368, 0x61646572, 0x20436f6d, 0x70696c65, 
    0x7220322e, 0x302e3231, 0x33352e30, 0x00ababab, 0x10020100, 0x00000000, 
    0x00000000, 0x00000000, 0x00002042, 0x5c005700, 0x00000003, 0x00000001, 
    0x0000f050, 0x0000f1a0, 0x00011002, 0x00001200, 0xc4000000, 0x00001003, 
    0x00002200, 0x00000000, 0x10080001, 0x1f1ffff8, 0x00004000, 0xc80f8000, 
    0x006c0000, 0xc1000100
};

//-----------------------------------------------------------------------------

}; // namespace Shaders {
}; // namespace MGSToolbox {

#endif // USE_MGSTOOLBOX