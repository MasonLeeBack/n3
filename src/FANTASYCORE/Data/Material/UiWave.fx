/*
 *	ui wave
 */

//------------------------------------
texture LayerTexture : Diffuse
<
	string ResourceName = "default_color.dds";
>;

texture WaveTexture : Diffuse1
<
	string ResourceName = "Wave_Test_Source.dds";
>;

//------------------------------------
struct vertexInput {
    float3 position			: POSITION;
    float2 texUV			: TEXCOORD0;
    float  time				: TEXCOORD1;
    float  hAlpha			: TEXCOORD2;
};

struct vertexOutputDUV {
    float4 hPosition	: POSITION;
    float3 Layer0UV		: TEXCOORD0;
    float4 ScreenUV		: TEXCOORD1;
    float  hAlpha		: TEXCOORD2;
};

//------------------------------------
vertexOutputDUV VS_TransformAndTexture_Wave(vertexInput IN) 
{
    vertexOutputDUV OUT;
    
    OUT.hPosition = float4(IN.position.xyz, 1); 
    OUT.Layer0UV.xy = IN.texUV.xy;
    OUT.Layer0UV.z = IN.time;
    
    OUT.ScreenUV.x = (OUT.hPosition.x * 0.5f) + 0.5f;
    OUT.ScreenUV.y = (OUT.hPosition.y * (-0.5f)) + 0.5f;
    
    OUT.ScreenUV.zw = float2(0.f, 0.f);
    
    OUT.hAlpha = IN.hAlpha;
   
    return OUT;
}

//-----------------------------------
sampler LayerSampler = sampler_state 
{
    texture = <LayerTexture>;
    AddressU  = CLAMP;
    AddressV  = CLAMP;
    AddressW  = CLAMP;
    MINFILTER = LINEAR;
    MAGFILTER = LINEAR;
};

sampler3D WaveSampler = sampler_state 
{
    texture = <WaveTexture>;
    AddressU  = CLAMP;
    AddressV  = CLAMP;
    AddressW  = CLAMP;
    MINFILTER = LINEAR;
    MAGFILTER = LINEAR;
};

//------------------------------------
float4 PS_Textured_Wave(vertexOutputDUV IN): COLOR
{
	float4 diffuseTexture = tex3D( WaveSampler, IN.Layer0UV );
	
	float4 OutTexture = tex2D( LayerSampler, IN.ScreenUV + diffuseTexture );
	OutTexture.w = IN.hAlpha;
		
	return OutTexture;
	
}

//-----------------------------------
technique textured
{
    pass p0 
    {
		VertexShader = compile vs_1_1 VS_TransformAndTexture_Wave();
		PixelShader  = compile ps_2_0 PS_Textured_Wave();
    }
}
