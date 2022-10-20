texture SceneMap : RENDERCOLORTARGET;

sampler SceneSampler = sampler_state 
{
    texture = <SceneMap>;
    AddressU  = CLAMP;        
    AddressV  = CLAMP;
    AddressW  = CLAMP;
    MINFILTER = LINEAR;
    MAGFILTER = LINEAR;
};

///////////////////////////////////////////////////////////
/////////////////////////////////// data structures ///////
///////////////////////////////////////////////////////////

struct VS_OUTPUT_DOWNSAMPLE
{
    float4 Position   : POSITION;
    float2 TexCoord   : TEXCOORD0;
};

////////////////////////////////////////////////////////////
////////////////////////////////// vertex shaders //////////
////////////////////////////////////////////////////////////

// generate texture coordinates to sample 4 neighbours
VS_OUTPUT_DOWNSAMPLE VS_Downsample(float2 Position : POSITION,
								   float2 TexCoord : TEXCOORD0)
{
	VS_OUTPUT_DOWNSAMPLE OUT;
	float2 s = TexCoord;
	OUT.Position = float4(Position, 0.f, 1.f);
	OUT.TexCoord = s;
	return OUT;
}

//////////////////////////////////////////////////////
////////////////////////////////// pixel shaders /////
//////////////////////////////////////////////////////

half4 PS_Draw(VS_OUTPUT_DOWNSAMPLE IN,
				    uniform sampler2D tex) : COLOR
{
	half4 c;
	// box filter
	c = tex2D(tex, IN.TexCoord);
	return c;
}

////////////////////////////////////////////////////////////
/////////////////////////////////////// techniques /////////
////////////////////////////////////////////////////////////


technique SceneDraw
{
    pass p0
    {
//		cullmode = none;
//		ZEnable = false;
//		ZWriteEnable = false;		
		VertexShader = compile vs_2_0 VS_Downsample();
		PixelShader  = compile ps_2_0 PS_Draw(SceneSampler);
    }
}