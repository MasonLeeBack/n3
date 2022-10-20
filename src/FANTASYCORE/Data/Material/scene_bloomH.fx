///////////////////////////////////////////////////////////
///////////////////////////// Render-to-Texture Data //////
///////////////////////////////////////////////////////////

float4 ScreenSize : VIEWPORTPIXELSIZE;

float BlurWidth <
    string UIName = "Blur width";
    string UIWidget = "slider";
    float UIMin = 0.0f;
    float UIMax = 10.0f;
    float UIStep = 0.5f;
> = 2.0f;

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

struct VS_OUTPUT_BLUR
{
    float4 Position   : POSITION;
    float2 TexCoord[8]: TEXCOORD0;
};

////////////////////////////////////////////////////////////
////////////////////////////////// vertex shaders //////////
////////////////////////////////////////////////////////////

// generate texture coordinates to sample 4 neighbours
VS_OUTPUT_BLUR VS_BlurH(float3 Position : POSITION,
								   float2 TexCoord : TEXCOORD0)
{

	VS_OUTPUT_BLUR OUT = (VS_OUTPUT_BLUR)0;
	float2 texelSize = BlurWidth / ScreenSize;
	float2 s = TexCoord-texelSize*(7-1)*0.5f*float2(1,0);
	OUT.Position = float4(Position, 1.f);
	for(int i=0;i<7;i++) {
		OUT.TexCoord[i] = s + texelSize*i*float2(1,0);
	}
	return OUT;
}

//////////////////////////////////////////////////////
////////////////////////////////// pixel shaders /////
//////////////////////////////////////////////////////

const half weight[7] = 
{
	0.05,
	0.1,
	0.2,
	0.3,
	0.2,
	0.1,
	0.05,
};

half4 PS_BloomH(VS_OUTPUT_BLUR IN) : COLOR
{
	half4 c = 0;
	
	for(int i=0;i<7;i++) {
		c += tex2D(SceneSampler, IN.TexCoord[i])*weight[i];
	}
	return c;
}

////////////////////////////////////////////////////////////
/////////////////////////////////////// techniques /////////
////////////////////////////////////////////////////////////

technique Downsample
{
    pass p0
    {
		cullmode = none;
		ZEnable = false;
		ZWriteEnable = false;		
		VertexShader = compile vs_2_0 VS_BlurH();
		PixelShader  = compile ps_2_0 PS_BloomH();
    }
}