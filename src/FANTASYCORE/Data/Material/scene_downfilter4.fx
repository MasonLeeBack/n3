float HighlightThreshold <
    string UIName = "Highlight threshold";
    string UIWidget = "slider";
    float UIMin = 0.0f;
    float UIMax = 1.0f;
    float UIStep = 0.1f;
> = 0.9f;

float HighlightIntensity <
    string UIName = "Highlight intensity";
    string UIWidget = "slider";
    float UIMin = 0.0f;
    float UIMax = 10.0f;
    float UIStep = 0.1f;
> = 0.5f;

///////////////////////////////////////////////////////////
///////////////////////////// Render-to-Texture Data //////
///////////////////////////////////////////////////////////

float4 ScreenSize : VIEWPORTPIXELSIZE;
float downsampleScale = 0.25;

texture SceneMap : RENDERCOLORTARGET;

sampler SceneSampler = sampler_state 
{
    texture = <SceneMap>;
    AddressU  = CLAMP;        
    AddressV  = CLAMP;
    AddressW  = CLAMP;
    MINFILTER = POINT;
    MAGFILTER = LINEAR;
};

///////////////////////////////////////////////////////////
/////////////////////////////////// data structures ///////
///////////////////////////////////////////////////////////

struct VS_OUTPUT_DOWNSAMPLE
{
    float4 Position   : POSITION;
    float2 TexCoord[4]: TEXCOORD0;
};

////////////////////////////////////////////////////////////
////////////////////////////////// vertex shaders //////////
////////////////////////////////////////////////////////////

// generate texture coordinates to sample 4 neighbours
VS_OUTPUT_DOWNSAMPLE VS_Downsample(float3 Position : POSITION,
								   float2 TexCoord : TEXCOORD0)
{

	VS_OUTPUT_DOWNSAMPLE OUT;
	float2 texelSize = downsampleScale / ScreenSize;
	float2 s = TexCoord;
	OUT.Position = float4(Position, 1.f);
	OUT.TexCoord[0] = s;
	OUT.TexCoord[1] = s + float2(4, 0)*texelSize;
	OUT.TexCoord[2] = s + float2(4, 4)*texelSize;
	OUT.TexCoord[3] = s + float2(0, 4)*texelSize;	
	return OUT;
}

//////////////////////////////////////////////////////
////////////////////////////////// pixel shaders /////
//////////////////////////////////////////////////////

half luminance(half3 c)
{
//	return 0.333f*(c.x+c.y+c.z);
	return dot( c, float3(0.3, 0.59, 0.11) );
//	return dot( c, float3(0.3, 0.6, 0.2) );
}

// this function should be baked into a texture lookup for performance
half highlights(half3 c)
{
	return smoothstep(HighlightThreshold, 1.0, luminance(c.rgb));
}

float2 vOffset[] =
    {
        { -0.5f,  0.5f },
        { -0.5f,  1.5f },
        { -0.5f,  2.5f },
        { -0.5f,  3.5f },
        {  0.5f,  0.5f },
        {  0.5f,  1.5f },
        {  0.5f,  2.5f },
        {  0.5f,  3.5f },
        {  1.5f,  0.5f },
        {  1.5f,  1.5f },
        {  1.5f,  2.5f },
        {  1.5f,  3.5f },
        {  2.5f,  0.5f },
        {  2.5f,  1.5f },
        {  2.5f,  2.5f },
        {  2.5f,  3.5f },
    };
    
    
half4 PS_Downsample(VS_OUTPUT_DOWNSAMPLE IN,
				    uniform sampler2D tex) : COLOR
{
/*
	half4 c;
	// box filter
	c = tex2D(tex, IN.TexCoord[0]) * 0.25;
	c += tex2D(tex, IN.TexCoord[1]) * 0.25;
	c += tex2D(tex, IN.TexCoord[2]) * 0.25;
	c += tex2D(tex, IN.TexCoord[3]) * 0.25;
/*/	
    const int NUM_SAMPLES = 16;
	half4    c = 0.0f;

	half2 texelSize = 1.f / ScreenSize.xy;
    for( int i=0; i < NUM_SAMPLES; i++ )
    {
        c += tex2D( tex, IN.TexCoord[0] + vOffset[i]*texelSize )*0.0625f;
    }
    
//    c = c / NUM_SAMPLES;
//*/
	// store hilights in alpha
	c.a = highlights(c.rgb);

	return c;
}

half4 PS_Draw(VS_OUTPUT_DOWNSAMPLE IN,
				    uniform sampler2D tex) : COLOR
{
	half4 c;
	// box filter
	c = tex2D(tex, IN.TexCoord[0]);
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
		VertexShader = compile vs_2_0 VS_Downsample();
		PixelShader  = compile ps_2_0 PS_Downsample(SceneSampler);
    }
}

technique SceneDraw
{
    pass p0
    {
		cullmode = none;
		ZEnable = false;
		ZWriteEnable = false;		
		VertexShader = compile vs_2_0 VS_Downsample();
		PixelShader  = compile ps_2_0 PS_Draw(SceneSampler);
    }
}