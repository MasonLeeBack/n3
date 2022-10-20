/*
 *	Diffuse, Directional Light, Per-Vertex
 */

//------------------------------------
float4x4 worldViewProj	: WorldViewProjection;
float4x4 worldviewInverseTranspose	: WorldViewInverseTranspose;
float time	: Time;

float4 f4UVSpeed : UVANIMATION_POSITION
<
    string UIName = "Bumpmap translation speed";
> = { 0.002f, 0.005f, 0.f, 0.f };

texture ani_back : Diffuse
<
	string ResourceName = "default_color.dds";
>;

texture noani_middle : Diffuse
<
	string ResourceName = "default_color.dds";
>;

texture ani_front : Diffuse
<
	string ResourceName = "default_color.dds";
>;

//------------------------------------
struct vertexInput0 {
    float3 position				: POSITION;
    float4 texCoordDiffuse		: TEXCOORD0;
};

struct vertexOutput {
    float4 hPosition		: POSITION;
    float4 texCoordDiffuse	: TEXCOORD0;
};

//------------------------------------
vertexOutput VS_TransformAndTexture_0(vertexInput0 IN) 
{
    vertexOutput OUT;
    OUT.hPosition = mul( float4(IN.position.xyz , 1.0) , worldViewProj);
    OUT.texCoordDiffuse = IN.texCoordDiffuse;

    return OUT;
}

//------------------------------------
sampler Ani_Back_00 = sampler_state 
{
    texture = <ani_back>;
    AddressU  = WRAP;        
    AddressV  = WRAP;
    AddressW  = WRAP;
    MIPFILTER = LINEAR;
    MINFILTER = LINEAR;
    MAGFILTER = LINEAR;
};

sampler Back = sampler_state 
{
    texture = <noani_middle>;
    AddressU  = WRAP;        
    AddressV  = WRAP;
    AddressW  = WRAP;
    MIPFILTER = LINEAR;
    MINFILTER = LINEAR;
    MAGFILTER = LINEAR;
};

sampler ani_02_Sampler = sampler_state 
{
    texture = <ani_front>;
    AddressU  = WRAP;        
    AddressV  = WRAP;
    AddressW  = WRAP;
    MIPFILTER = LINEAR;
    MINFILTER = LINEAR;
    MAGFILTER = LINEAR;
};

//-----------------------------------
float4 PS_Textured( vertexOutput IN): COLOR
{
  float2 uv = IN.texCoordDiffuse;
  float4 ani_back = tex2D ( Ani_Back_00, float2(uv.x + time*f4UVSpeed.x, uv.y));
  float4 Back_Texture = tex2D( Back, uv );
  uv.x += time*f4UVSpeed.y;
  float4 animatedTexture = tex2D( ani_02_Sampler, uv);

  float4 result = lerp(ani_back, Back_Texture, Back_Texture.w);
  result = lerp( result, animatedTexture, animatedTexture.w);

  return result;
}


//-----------------------------------
technique textured_0				// No Physique
{
    pass p0 
    {		
		VertexShader = compile vs_2_0 VS_TransformAndTexture_0();
		PixelShader  = compile ps_2_0 PS_Textured();
    }
}