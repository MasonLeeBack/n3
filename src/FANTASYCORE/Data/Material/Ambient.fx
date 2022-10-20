/*
 *	Diffuse, Directional Light, Per-Vertex
 *	No Animation∏∏ æ≤¿”(No Physique Vertex)
 */
#include "CommonFunction.fxh"
//------------------------------------

shared float4x4 projection					: Projection;
shared float4	vecRcpWorldSize					: RcpWorldSize;

float4x4 world						: World;
float4x4 WorldView				: WorldView;
float4x4 worldViewProj				: WorldViewProjection;
float4x3 matBone[50]				: WORLDMATRIXARRAY;

// -FOG-
shared float4 fogFactor				    : FOGFACTOR;	// fogEnd, forRange, ?, ?
shared float4 fogColor						: FOGCOLOR;		// r,g,b,1

texture diffuseTexture : Diffuse
<
	string ResourceName = "default_color.dds";
>;

texture lightmapTexture : WORLDLIGHTMAP
<
	string ResourceName = "default_color.dds";
>;

shared float4 lightAmbient : LightAmbient
<
    string UIWidget = "Ambient Light Color";
    string Space = "material";
> = {0.0f, 0.0f, 0.0f, 1.0f};

float4 materialAmbient : MaterialAmbient
<
    string UIWidget = "Surface Color";
    string Space = "material";
> = { 1.f, 1.f, 1.f, 1.f};

//------------------------------------
sampler TextureSampler = sampler_state 
{
    texture = <diffuseTexture>;
};

sampler lightmapSampler = sampler_state
{
    texture = <lightmapTexture>;
    MINFILTER = LINEAR;
    MAGFILTER = LINEAR;
};

//------------------------------------
struct vertexInput0 {
    float3 position			: POSITION;
    float4 texCoordDiffuse		: TEXCOORD0;
};

struct vertexInput1 {
    float3 position			: POSITION;
    float4 texCoordDiffuse		: TEXCOORD0;
    int    nBoneIndex			: BLENDINDICES0;
};

struct vertexInput2 {
    float3 position				: POSITION;
    float4 texCoordDiffuse		: TEXCOORD0;
	int4	   nBoneIndex			: BLENDINDICES;
	float4	   fWeight			: BLENDWEIGHT;
};

struct vertexOutput {
    float4 hPosition		: POSITION;
    float4 texCoordDiffuse	: TEXCOORD0;
    float2 lightmapCoord	: TEXCOORD1;
    float3 pointValue			: TEXCOORD2;
    float4 AmbColor			: COLOR0;    
    float  FogValue	    	: COLOR1; // -FOG-
};

//------------------------------------
vertexOutput VS_TransformAndTexture_0(vertexInput0 IN) 
{
    vertexOutput OUT;

    float4 f4WorldPos = mul( float4(IN.position.xyz, 1.f), world);
    OUT.lightmapCoord.xy = f4WorldPos.xz*vecRcpWorldSize.xy;
    OUT.lightmapCoord.y = 1.f - OUT.lightmapCoord.y; 
    
	float3 viewVertPos = mul( float4(IN.position.xyz, 1.f), WorldView );
    OUT.pointValue = ApplyPointLight( viewVertPos, float3(0,1,0)) ;

    OUT.hPosition = mul( float4(IN.position.xyz , 1.0) , worldViewProj);
    OUT.texCoordDiffuse = IN.texCoordDiffuse;

	//output ambient
    OUT.AmbColor = materialAmbient * lightAmbient;
    OUT.AmbColor.w = materialAmbient.w;

	// -FOG-
	float fDist = fogFactor.x - OUT.hPosition.z; // x=fogEnd
	OUT.FogValue = clamp ( fDist*fogFactor.y, 0.0f, 1.0f ); // y=forRange
    return OUT;
}

//------------------------------------  Breakable Prop
vertexOutput VS_TransformAndTexture_1(vertexInput1 IN) 
{
    vertexOutput OUT;
    
    float3 Pos = mul( float4(IN.position.xyz , 1.0), matBone[IN.nBoneIndex]);    
	OUT.hPosition = mul( float4(Pos,1.f) , projection);

    float4 f4WorldPos = mul( float4(IN.position.xyz, 1.f), world);
    OUT.lightmapCoord.xy = f4WorldPos.xz*vecRcpWorldSize.xy;
    OUT.lightmapCoord.y = 1.f - OUT.lightmapCoord.y; 
    
    OUT.pointValue = ApplyPointLight( Pos, float3(0,1,0)) ;
    
    OUT.texCoordDiffuse = IN.texCoordDiffuse;

	//output ambient
    OUT.AmbColor = materialAmbient * lightAmbient;
    OUT.AmbColor.w = materialAmbient.w;

	// -FOG-
	float fDist = fogFactor.x - OUT.hPosition.z; // x=fogEnd
	OUT.FogValue = clamp ( fDist*fogFactor.y, 0.0f, 1.0f ); // y=forRange
    return OUT;
}


float3 ApplyWorldLightMap(float4 fLightMap)
{
	float3 fTemp;
	fTemp = fLightMap.xyz * 0.7f + 0.3f;
	return fTemp;
}

//-----------------------------------
float4 PS_Textured( vertexOutput IN): COLOR
{
	float4 diffuseTexture = tex2D( TextureSampler, IN.texCoordDiffuse );

	float4 LightColor = tex2D(lightmapSampler, IN.lightmapCoord);

	float4 result = (IN.AmbColor + float4(IN.pointValue*4,0))* diffuseTexture;
	result.xyz *= ApplyWorldLightMap(LightColor);
	
	// -FOG-
	result.xyz = lerp(fogColor, result, IN.FogValue);

	return result;
}

//-----------------------------------
technique textured_0				// No Physique
{
    pass p0 
    {		
		VertexShader = compile vs_1_1 VS_TransformAndTexture_0();
		PixelShader  = compile ps_1_1 PS_Textured();
    }
}

//-----------------------------------
technique textured_1				// No Physique
{
    pass p0 
    {		
		VertexShader = compile vs_1_1 VS_TransformAndTexture_1();
		PixelShader  = compile ps_1_1 PS_Textured();
    }
}