float SceneIntensity = 0.5f;
float GlowIntensity = 0.5f;
float HighlightIntensity = 0.5f;

float2 ScreenSize : VIEWPORTPIXELSIZE;
float  downsampleScale = 0.25;
float4 addColor = 0.0f;

texture BackBufferTexture;
sampler BackBufferSampler = sampler_state {
   texture = <BackBufferTexture>;
    AddressU  = CLAMP;        
    AddressV  = CLAMP;
    AddressW  = CLAMP;
    MINFILTER = LINEAR;
    MAGFILTER = LINEAR;
};

texture FinalTexture;
sampler FinalSampler = sampler_state {
   texture = <FinalTexture>;
    AddressU  = CLAMP;        
    AddressV  = CLAMP;
    AddressW  = CLAMP;
    MINFILTER = LINEAR;
    MAGFILTER = LINEAR;
};

struct VS_OUTPUT
{
	float4 Position : POSITION;
	float2 TexCoord0 : TEXCOORD0;
	float2 TexCoord1 : TEXCOORD1;
};

VS_OUTPUT VS_Quad(float3 Position : POSITION, float2 TexCoord : TEXCOORD0)
{
	VS_OUTPUT OUT;
	float2 texelSize = 1.f/ScreenSize;
	OUT.Position = float4(Position, 1.f);
	OUT.TexCoord0 = TexCoord + texelSize*0.5;
	OUT.TexCoord1 = TexCoord + texelSize*0.5/downsampleScale;
	return OUT;
}

half4 PS_Comp(VS_OUTPUT IN) : COLOR
{
	half4 orig = tex2D(BackBufferSampler, IN.TexCoord0) + addColor;
	half4 blur = tex2D(FinalSampler, IN.TexCoord1);
	return SceneIntensity*orig + GlowIntensity*blur+HighlightIntensity*blur.aaaa*blur.rgba;
}

technique FinalComp
{
	pass p0
	{
		cullmode = none;
		ZEnable = false;
		ZWriteEnable = false;
		VertexShader = compile vs_2_0 VS_Quad();
		PixelShader = compile ps_2_0 PS_Comp();
	}
}