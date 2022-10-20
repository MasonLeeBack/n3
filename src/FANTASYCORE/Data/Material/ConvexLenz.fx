/*
 *	Diffuse, Directional Light, Per-Vertex
 */

//------------------------------------
float4x4 worldViewProj	: WorldViewProjection;



texture Offsetmap : Diffuse
<
	string ResourceName = "default_color.dds";
>;

texture screenTexture : ScreenBuffer;




//------------------------------------
struct vertexInput0 {
    float3 position				: POSITION;
    float4 texCoord		: TEXCOORD0;
};

struct vertexOutput {
    float4 hPosition		: POSITION;
    float4 texCoordOffset	: TEXCOORD0;
    float2 texCoordBack	: TEXCOORD1;
};

//------------------------------------
vertexOutput VS_TransformAndTexture_0(vertexInput0 IN) 
{
    vertexOutput OUT;
    OUT.hPosition = mul( float4(IN.position.xyz , 1.0) , worldViewProj);
    OUT.texCoordOffset = IN.texCoord;
    OUT.texCoordBack.x = (OUT.hPosition.x/OUT.hPosition.w)*0.5f+0.5f;
    OUT.texCoordBack.y = (OUT.hPosition.y/OUT.hPosition.w)*(-0.5f)+0.5f;

    return OUT;
}

//------------------------------------
sampler Offset = sampler_state 
{
    texture = <Offsetmap>;
    AddressU  = WRAP;        
    AddressV  = WRAP;
    AddressW  = WRAP;
    MIPFILTER = LINEAR;
    MINFILTER = LINEAR;
    MAGFILTER = LINEAR;
};

sampler screenSampler = sampler_state 
{
    texture = <screenTexture>;
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
  float4 vOffset = tex2D( Offset, IN.texCoordOffset.xy);
  vOffset =  vOffset - float4(0.5f, 0.5f, 0.5f, 0.f);
  vOffset *= float4(0.5f, 0.5f, 0.5f, 1.f);
  float2 vTexcoord = IN.texCoordBack.xy+ vOffset.xy;
  float4 vResult = tex2D( screenSampler, vTexcoord);
  vResult.a = vOffset.a;
  return vResult;
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