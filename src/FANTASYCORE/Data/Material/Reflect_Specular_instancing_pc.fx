/*
 *	Diffuse Specular, Directional Light, Per-Vertex
 */
//------------------------------------

#include "CommonFunction.fxh"

shared float4x4 projection					: Projection;
shared float4	vecRcpWorldSize					: RcpWorldSize;
shared float4x4 viewInverse 					: ViewInverse;

shared float4x4 view						: View;
shared float4x4 viewProj					: ViewProjection;

float4x4 worldviewInverseTranspose	: WorldViewInverseTranspose;

float4x3 matBone[50]			: WORLDMATRIXARRAY;			

shared float4 fogFactor				    : FOGFACTOR; // fogEnd, forRange, ?, ?
shared float4 fogColor						: FOGCOLOR; // r,g,b,1

texture diffuseTexture : Diffuse
<
	string ResourceName = "default_color.dds";
>;

texture specularTexture : Specular
<
	string ResourceName = "default_color.dds";
>;

texture lightmapTexture : WORLDLIGHTMAP
<
	string ResourceName = "default_color.dds";
>;

texture envTexture : Environment;

shared float4 viewlightDir : LightDirectionViewSpace
<
	string Object = "DirectionalLight";
    string Space = "World";
> = {1.0f, -1.0f, 1.0f, 0.0f};

shared float4 lightColor : LightDiffuse
<
    string UIName = "Diffuse Light Color";
    string Object = "DirectionalLight";
> = {1.0f, 1.0f, 1.0f, 1.0f};

shared float4 lightAmbient : LightAmbient
<
    string UIWidget = "Ambient Light Color";
    string Space = "material";
> = {0.0f, 0.0f, 0.0f, 1.0f};

shared float4 lightSpecular : LightSpecular
<
    string UIWidget = "Ambient Light Color";
    string Space = "material";
> = {1.f, 1.f, 1.0f, 1.f};

float4 materialAmbient : MaterialAmbient
<
    string UIWidget = "Surface Color";
    string Space = "material";
> = { 1.f, 1.f, 1.f, 1.f};

float4 materialDiffuse : MaterialDiffuse
<
    string UIWidget = "Surface Color";
    string Space = "material";
> = { 0.753f, 0.753f, 0.753f, 1.0f};

float4 materialSpecular : MaterialSpecular
<
	string UIWidget = "Surface Specular";
	string Space = "material";
> = {1.0f, 1.0f, 1.0f, 1.0f};

float shininess : SpecularPower
<
    string UIWidget = "slider";
    float UIMin = 1.0;
    float UIMax = 128.0;
    float UIStep = 1.0;
    string UIName = "specular power";
> = 15.0;

float reflectpower : ReflectPower
<
    string UIWidget = "slider";
    float UIMin = 1.0;
    float UIMax = 128.0;
    float UIStep = 1.0;
    string UIName = "reflect power";
> = 1.f;

//------------------------------------
struct vertexInput0 {
    float3 position				: POSITION;
    float3 normal				: NORMAL;
    float4 texCoordDiffuse		: TEXCOORD0;
    float4 matWorld1 : TEXCOORD1;
    float4 matWorld2 : TEXCOORD2;
    float4 matWorld3 : TEXCOORD3;
    float4 matWorld4 : TEXCOORD4;
};

struct vertexOutput {
    float4 hPosition			: POSITION;
    float4 texCoordDiffuse	: TEXCOORD0;
    float4 diffAmbColor		: TEXCOORD1;
    float4 specularColor		: TEXCOORD2;
    float3 worldReflect		: TEXCOORD3;
    float3  FogValue			: TEXCOORD4;
};

//------------------------------------
vertexOutput VS_TransformAndTexture_0(  vertexInput0 IN  ) 
{
    vertexOutput OUT;

    	float4x4 matWorld;
	matWorld._11_12_13_14 = IN.matWorld1;
	matWorld._21_22_23_24 = IN.matWorld2;
	matWorld._31_32_33_34 = IN.matWorld3;
	matWorld._41_42_43_44 = IN.matWorld4;
    
    float4 f4WorldPos = mul( float4(IN.position.xyz, 1.f), matWorld);
    
    OUT.hPosition = mul( f4WorldPos , viewProj);   
    OUT.FogValue.y = f4WorldPos.x*vecRcpWorldSize.x;
    OUT.FogValue.z = 1.f - f4WorldPos.z*vecRcpWorldSize.y; 
	//calculate our vectors N, E, L, and H
    float4 viewVertPos = mul( f4WorldPos, view);
	float4x4 worldViewMat = mul(matWorld, view);
	float3 N = (mul(IN.normal.xyz, worldViewMat)); //normal vector
	N.xyz = normalize(N.xyz);
    float3 E = normalize( -viewVertPos.xyz); //eye vector
    float3 L = ( -viewlightDir.xyz); //light vector
    float3 H = normalize(E + L); //half angle vector

	//calculate the diffuse
    float  diff = max(0 , dot(N,L));
    //calculate the spcular
    float  spec = pow( max(0 , dot(N,H) ) , shininess );
 
	//output diffuse & specular
    float4 ambColor = materialAmbient * lightAmbient;
    float4 diffColor = materialDiffuse * diff * lightColor ;
    OUT.diffAmbColor = diffColor + ambColor;
	OUT.diffAmbColor.w = materialAmbient.w;
    OUT.specularColor = lightSpecular * materialSpecular * spec * lightColor;

    float3 viewReflect = reflect(-E, N);
	OUT.worldReflect = mul( float4(viewReflect,0.f), viewInverse);

    OUT.texCoordDiffuse = IN.texCoordDiffuse;

	float fDist = fogFactor.x - viewVertPos.z; // x=fogEnd
	OUT.FogValue.x = clamp ( fDist*fogFactor.y, 0.0f, 1.0f ); // y=forRange

    return OUT;
}

//------------------------------------
sampler TextureSampler = sampler_state 
{
    texture = <diffuseTexture>;
};

sampler SpecularSampler = sampler_state 
{
    texture = <specularTexture>;
};

samplerCUBE EnvSampler = sampler_state
{
    texture = <envTexture>;
};

sampler lightmapSampler = sampler_state
{
    texture = <lightmapTexture>;
};

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

	float4 glossTexture = tex2D( SpecularSampler, IN.texCoordDiffuse );
	float4 envTexture = texCUBE( EnvSampler, IN.worldReflect);
	float4 LightColor = tex2D(lightmapSampler, IN.FogValue.yz);

	envTexture.xyz *= glossTexture.www;				// 반사도 조절맵 적용!!
	glossTexture.xyz += (envTexture.xyz*reflectpower.xxx);

	float4 result = IN.diffAmbColor*diffuseTexture + float4(IN.specularColor.xyz*glossTexture.xyz, 0.f);
	result.xyz*=ApplyWorldLightMap(LightColor);
	result.xyz = lerp(fogColor, result, IN.FogValue.x);
	return result;
}

//-----------------------------------
technique textured_0				// No Physique
{
    pass p0 
    {		
		VertexShader = compile vs_3_0 VS_TransformAndTexture_0();
		PixelShader  = compile ps_2_0 PS_Textured();
    }
}
