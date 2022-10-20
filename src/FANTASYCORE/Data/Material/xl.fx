//-----------------------------------------------------------------------------
// Name: Texture
// Arguments:
//	float3	vP	: POSITION
//	float3	vN	: NORMAL
//	float2	vT0	: TEXCOORD0
// Returns:
//	float4	vC0 : COLOR
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// data from program
//-----------------------------------------------------------------------------
float4x4	g_mWVP;
float4		g_vColor;

texture		g_tColor;
sampler		g_tColorSampler	= sampler_state {
	Texture		= <g_tColor>;
	AddressU	= WRAP;
	AddressV	= WRAP;
	MinFilter	= LINEAR;
	MagFilter	= LINEAR;
	MipFilter	= LINEAR;
};

texture		g_tSprite;
sampler		g_tSpriteSampler	= sampler_state {
	Texture		= <g_tSprite>;
	AddressU	= CLAMP;
	AddressV	= CLAMP;
	MinFilter	= LINEAR;
	MagFilter	= LINEAR;
	MipFilter	= LINEAR;
};

//-----------------------------------------------------------------------------
// data from vertex shader to pixel shader
//-----------------------------------------------------------------------------
struct	VT_OUTPUT
{
	float4	vP	: POSITION;
	float2	vT0	: TEXCOORD0;
};


//-----------------------------------------------------------------------------
// vertex shader
//-----------------------------------------------------------------------------
VT_OUTPUT	TextureVS(
	float3	vP	: POSITION,
	float3	vN	: NORMAL,
	float2	vT0	: TEXCOORD0
){
	VT_OUTPUT	Out = ( VT_OUTPUT )0;
		
	Out.vP	= mul( float4( vP, 1.0f ), g_mWVP );
	Out.vT0	= vT0;
	
	return Out;
}


//-----------------------------------------------------------------------------
// pixel shader
//-----------------------------------------------------------------------------
float4		TexturePS( VT_OUTPUT In )	: COLOR
{
	float4	vC	= tex2D( g_tColorSampler, In.vT0 );

	return	vC;
}


//-----------------------------------------------------------------------------
// techniques
//-----------------------------------------------------------------------------
technique	Texture
{
	pass P0
	{
		VertexShader = compile vs_3_0 TextureVS();
		PixelShader  = compile ps_3_0 TexturePS();
	}
}


//-----------------------------------------------------------------------------
// Name: Color
// Arguments:
//	float3	vP	: POSITION
//	float3	vN	: NORMAL
//	float2	vT0	: TEXCOORD0
// Returns:
//	float4	vC0 : COLOR
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// data from vertex shader to pixel shader
//-----------------------------------------------------------------------------
struct	VC_OUTPUT
{
	float4	vP	: POSITION;
};


//-----------------------------------------------------------------------------
// vertex shader
//-----------------------------------------------------------------------------
VC_OUTPUT	ColorVS(
	float3	vP	: POSITION,
	float3	vN	: NORMAL,
	float2	vT0	: TEXCOORD0
){
	VC_OUTPUT	Out = ( VC_OUTPUT )0;
		
	Out.vP	= mul( float4( vP, 1.0f ), g_mWVP );
	
	return Out;
}


//-----------------------------------------------------------------------------
// pixel shader
//-----------------------------------------------------------------------------
float4		ColorPS( VC_OUTPUT In )	: COLOR
{
	float4	vC	= g_vColor;

	return	vC;
}

//-----------------------------------------------------------------------------
// techniques
//-----------------------------------------------------------------------------
technique	Color
{
	pass P0
	{
		VertexShader = compile vs_3_0 ColorVS();
		PixelShader  = compile ps_3_0 ColorPS();
	}
}


//-----------------------------------------------------------------------------
// Name: Sprite
// Arguments:
//	float4	vPT	: POSITION
//	float2	vT0	: TEXCOORD0
// Returns:
//	float4	vC0 : COLOR
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// data from vertex shader to pixel shader
//-----------------------------------------------------------------------------
struct	VS_OUTPUT
{
	float4	vPT	: POSITION;
	float2	vT0	: TEXCOORD0;
};


//-----------------------------------------------------------------------------
// vertex shader
//-----------------------------------------------------------------------------
VS_OUTPUT	SpriteVS(
	float4	vPT	: POSITION,
	float2	vT0	: TEXCOORD0
){
	VS_OUTPUT	Out = ( VS_OUTPUT )0;
	Out.vPT	= vPT;
	Out.vT0	= vT0;
	
	return Out;
}


//-----------------------------------------------------------------------------
// pixel shader
//-----------------------------------------------------------------------------
float4		SpritePS( VS_OUTPUT In )	: COLOR
{
	float4	vC	= tex2D( g_tSpriteSampler, In.vT0 );

	return	vC;
}

//-----------------------------------------------------------------------------
// techniques
//-----------------------------------------------------------------------------
technique	Sprite
{
	pass P0
	{
		VertexShader = compile vs_3_0 SpriteVS();
		PixelShader  = compile ps_3_0 SpritePS();
	}
}

//-----------------------------------------------------------------------------
// Name: Line
// Arguments:
//	float3	vP	: POSITION
//	float3	vN	: NORMAL
//	float2	vT0	: TEXCOORD0
// Returns:
//	float4	vC0 : COLOR
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// data from vertex shader to pixel shader
//-----------------------------------------------------------------------------
struct	VL_OUTPUT
{
	float4	vP	: POSITION;
};


//-----------------------------------------------------------------------------
// vertex shader
//-----------------------------------------------------------------------------
VL_OUTPUT	LineVS(
	float3	vP	: POSITION
){
	VL_OUTPUT	Out = ( VL_OUTPUT )0;
		
	Out.vP	= mul( float4( vP, 1.0f ), g_mWVP );
	
	return Out;
}


//-----------------------------------------------------------------------------
// pixel shader
//-----------------------------------------------------------------------------
float4		LinePS( VC_OUTPUT In )	: COLOR
{
	float4	vC	= g_vColor;

	return	vC;
}

//-----------------------------------------------------------------------------
// techniques
//-----------------------------------------------------------------------------
technique	Line
{
	pass P0
	{
		VertexShader = compile vs_3_0 LineVS();
		PixelShader  = compile ps_3_0 LinePS();
	}
}

