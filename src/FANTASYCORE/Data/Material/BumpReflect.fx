/*
 *	Diffuse Specular, Directional light, Per-Pixel NormalMap
 */
#include "commonFunction.fxh"

shared float4x4 viewInverse 					: ViewInverse;
shared float4x4 projection					: Projection;
shared float4	vecRcpWorldSize					: RcpWorldSize;
 
float4x4 WorldViewIT				: WorldViewInverseTranspose;
float4x4 WorldViewProj				: WorldViewProjection;

float4x3 matBone[50]				: WORLDMATRIXARRAY;

// -FOG-
float4x4 WorldView					: WorldView;
shared float4 fogFactor				    : FOGFACTOR;	// fogEnd, forRange, ?, ?
shared float4 fogColor						: FOGCOLOR;		// r,g,b,1

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

float SpecExpon : SpecularPower
<
    string UIWidget = "slider";
    float UIMin = 1.0;
    float UIMax = 128.0;
    float UIStep = 1.0;
    string UIName =  "specular power";
> = 8.0;

float reflectpower : ReflectPower
<
    string UIWidget = "slider";
    float UIMin = 1.0;
    float UIMax = 128.0;
    float UIStep = 1.0;
    string UIName = "reflect power";
> = 1.f;

float Bumpy : BumpPower
<
    string UIWidget = "slider";
    float UIMin = 0.0;
    float UIMax = 10.0;
    float UIStep = 0.1;
    string UIName =  "bump power";
> = 3.0;

//////////
texture diffuseTexture : Diffuse
<
	string ResourceName = "default_color.dds";
>;

texture specularTexture : Specular
<
	string ResourceName = "default_color.dds";
>;

texture normalTexture : NORMAL
<
    string ResourceName = "default_bump_normal.dds";
    string ResourceType = "2D";
>;

texture envTexture : Environment;

texture lightmapTexture : WORLDLIGHTMAP
<
	string ResourceName = "default_color.dds";
>;

sampler2D diffuseSampler = sampler_state
{
	texture = <diffuseTexture>;
};

sampler2D specularSampler = sampler_state
{
	texture = <specularTexture>;
};

sampler2D normalSampler = sampler_state
{
	texture = <normalTexture>;
    MIPFILTER = LINEAR;
};

samplerCUBE EnvSampler = sampler_state
{
    texture = <envTexture>;
};

sampler lightmapSampler = sampler_state
{
    texture = <lightmapTexture>;
    MINFILTER = LINEAR;
    MAGFILTER = LINEAR;
};

/************* DATA STRUCTS **************/

/* data from application vertex buffer */
struct appdata0 {
    float3 Position	: POSITION;
    float4 UV		: TEXCOORD0;
    float3 Normal	: NORMAL;
    float3 Tangent	: TANGENT0;
    float3 Binormal	: BINORMAL0;
};

struct appdata1 {
    float3 Position	: POSITION;
    float4 UV		: TEXCOORD0;
    float3 Normal	: NORMAL;
    float3 Tangent	: TANGENT0;
    float3 Binormal	: BINORMAL0;
    int    nBoneIndex			: BLENDINDICES0;
};

struct appdata2 {
    float3 Position	: POSITION;
    float4 UV		: TEXCOORD0;
    float3 Normal	: NORMAL;
    float3 Tangent	: TANGENT0;
    float3 Binormal	: BINORMAL0;
	int4	   nBoneIndex			: BLENDINDICES;
	float4	   fWeight			: BLENDWEIGHT;
};


/* data passed from vertex shader to pixel shader */
struct vertexOutput {
    float4 HPosition	: POSITION;
    float4 TexCoord		: TEXCOORD0;
    float3 LightVec		: TEXCOORD1;	
    float3 ViewNormal	: TEXCOORD2;
    float3 ViewEyeVec	: TEXCOORD3;
    float3 ViewTangent	: TEXCOORD4;
    float3 ViewBinorm	: TEXCOORD5;
    float3 pointValue		: TEXCOORD6;
    float3 FogValue		: COLOR1;
};

/*********** vertex shader ******/

vertexOutput mainVS_0(appdata0 IN)
{
    vertexOutput OUT;
    OUT.ViewNormal = mul(IN.Normal, WorldViewIT).xyz;
    OUT.ViewTangent = mul(IN.Tangent, WorldViewIT).xyz;
    OUT.ViewBinorm = mul(IN.Binormal, WorldViewIT).xyz;
    float4 tempPos = float4(IN.Position.x,IN.Position.y,IN.Position.z,1.0);
    float4 ViewSpacePos = mul(tempPos, WorldView);
	
    float4 f4WorldPos = mul(ViewSpacePos, viewInverse);

    OUT.FogValue.y = f4WorldPos.x*vecRcpWorldSize.x;
    OUT.FogValue.z = 1.f - f4WorldPos.z*vecRcpWorldSize.y;

    OUT.LightVec = -viewlightDir;    
    OUT.TexCoord = IN.UV;
    
    OUT.pointValue = ApplyPointLight( ViewSpacePos, OUT.ViewNormal) ;
    // Normalize.
    OUT.ViewEyeVec = -ViewSpacePos.xyz;
    OUT.HPosition = mul(tempPos, WorldViewProj);
    
    // -FOG-
	float fDist = fogFactor.x - ViewSpacePos.z; // x=fogEnd
	OUT.FogValue.x = clamp ( fDist*fogFactor.y, 0.0f, 1.0f ); // y=forRange
	
    return OUT;
}

vertexOutput mainVS_1(appdata1 IN)
{
	vertexOutput OUT;
	OUT.ViewNormal = mul(IN.Normal, matBone[IN.nBoneIndex]);
	OUT.ViewTangent = mul(IN.Tangent, matBone[IN.nBoneIndex]).xyz;
	OUT.ViewBinorm = mul(IN.Binormal, matBone[IN.nBoneIndex]).xyz;
	float4 tempPos = float4(IN.Position.x,IN.Position.y,IN.Position.z,1.0);
	float3 viewSpacePos = mul(tempPos, WorldView).xyz;

    float4 f4WorldPos = mul( float4(viewSpacePos, 1.f), viewInverse);
    OUT.FogValue.y = f4WorldPos.x*vecRcpWorldSize.x;
    OUT.FogValue.z = 1.f - f4WorldPos.z*vecRcpWorldSize.y;

	OUT.LightVec = -viewlightDir;	
	OUT.TexCoord = IN.UV;
	
	OUT.pointValue = ApplyPointLight( viewSpacePos, OUT.ViewNormal);	
	// Normalize.
	OUT.ViewEyeVec = -viewSpacePos;
	float3 Pos = mul(tempPos, matBone[IN.nBoneIndex]);

	OUT.HPosition = mul( float4(Pos,1.f) , projection);

	// -FOG-
	float fDist = fogFactor.x - Pos.z; // x=fogEnd
	OUT.FogValue.x = clamp ( fDist*fogFactor.y, 0.0f, 1.0f ); // y=forRange

	return OUT;
}

vertexOutput mainVS_2(appdata2 IN)
{
    vertexOutput OUT;
    float3 N = mul(IN.Normal, matBone[IN.nBoneIndex.x]) * IN.fWeight.x;
    N +=  mul(IN.Normal, matBone[IN.nBoneIndex.y]) * IN.fWeight.y;
    OUT.ViewNormal = N;
    
    float3 vTangent = mul(IN.Tangent, matBone[IN.nBoneIndex.x]) * IN.fWeight.x;
    vTangent += mul(IN.Tangent, matBone[IN.nBoneIndex.y]) * IN.fWeight.y;
    OUT.ViewTangent = vTangent;

    float3 vBinormal = mul(IN.Binormal, matBone[IN.nBoneIndex.x]) * IN.fWeight.x;
    vBinormal += mul(IN.Binormal, matBone[IN.nBoneIndex.y]) * IN.fWeight.y;
    OUT.ViewBinorm = vBinormal;

    float4 tempPos = float4(IN.Position.x,IN.Position.y,IN.Position.z,1.0);
    OUT.LightVec = -viewlightDir;
    OUT.TexCoord = IN.UV;

    float3 Pos = mul(tempPos, matBone[IN.nBoneIndex.x]) * IN.fWeight.x;
    Pos += mul(tempPos, matBone[IN.nBoneIndex.y]) * IN.fWeight.y;
    
    OUT.pointValue = ApplyPointLight( Pos, N);	

    float4 f4WorldPos = mul( float4(Pos, 1.f), viewInverse);
    OUT.FogValue.y = f4WorldPos.x*vecRcpWorldSize.x;
    OUT.FogValue.z = 1.f - f4WorldPos.z*vecRcpWorldSize.y;

    // Normalize.
    OUT.ViewEyeVec = -Pos;
    
    OUT.HPosition = mul( float4(Pos,1.f) , projection);
    
    // -FOG-
	float fDist = fogFactor.x - Pos.z; // x=fogEnd
	OUT.FogValue.x = clamp ( fDist*fogFactor.y, 0.0f, 1.0f ); // y=forRange
	
    return OUT;
}

vertexOutput mainVS_3(appdata2 IN)
{
    vertexOutput OUT;
    float3 N = mul(IN.Normal, matBone[IN.nBoneIndex.x]) * IN.fWeight.x;
    N +=  mul(IN.Normal, matBone[IN.nBoneIndex.y]) * IN.fWeight.y;
    N +=  mul(IN.Normal, matBone[IN.nBoneIndex.z]) * IN.fWeight.z;
    OUT.ViewNormal = N;
    
    float3 vTangent = mul(IN.Tangent, matBone[IN.nBoneIndex.x]) * IN.fWeight.x;
    vTangent += mul(IN.Tangent, matBone[IN.nBoneIndex.y]) * IN.fWeight.y;
    vTangent += mul(IN.Tangent, matBone[IN.nBoneIndex.z]) * IN.fWeight.z;
    OUT.ViewTangent = vTangent;

    float3 vBinormal = mul(IN.Binormal, matBone[IN.nBoneIndex.x]) * IN.fWeight.x;
    vBinormal += mul(IN.Binormal, matBone[IN.nBoneIndex.y]) * IN.fWeight.y;
    vBinormal += mul(IN.Binormal, matBone[IN.nBoneIndex.z]) * IN.fWeight.z;
    OUT.ViewBinorm = vBinormal;
    
    float4 tempPos = float4(IN.Position.x,IN.Position.y,IN.Position.z,1.0);
    OUT.LightVec = -viewlightDir;
    OUT.TexCoord = IN.UV;

    float3 Pos = mul(tempPos, matBone[IN.nBoneIndex.x]) * IN.fWeight.x;
    Pos += mul(tempPos, matBone[IN.nBoneIndex.y]) * IN.fWeight.y;
    Pos += mul(tempPos, matBone[IN.nBoneIndex.z]) * IN.fWeight.z;
    
    OUT.pointValue = ApplyPointLight( Pos, N);	

    float4 f4WorldPos = mul( float4(Pos, 1.f), viewInverse);
    OUT.FogValue.y = f4WorldPos.x*vecRcpWorldSize.x;
    OUT.FogValue.z = 1.f - f4WorldPos.z*vecRcpWorldSize.y;

    // Normalize.
    OUT.ViewEyeVec = -Pos;
    OUT.HPosition = mul( float4(Pos,1.f) , projection);
    
    // -FOG-
	float fDist = fogFactor.x - Pos.z; // x=fogEnd
	OUT.FogValue.x = clamp ( fDist*fogFactor.y, 0.0f, 1.0f ); // y=forRange
	
    return OUT;
}

vertexOutput mainVS_4(appdata2 IN)
{
    vertexOutput OUT;
    float3 N = mul(IN.Normal, matBone[IN.nBoneIndex.x]) * IN.fWeight.x;
    N +=  mul(IN.Normal, matBone[IN.nBoneIndex.y]) * IN.fWeight.y;
    N +=  mul(IN.Normal, matBone[IN.nBoneIndex.z]) * IN.fWeight.z;
    N +=  mul(IN.Normal, matBone[IN.nBoneIndex.w]) * IN.fWeight.w;
    OUT.ViewNormal = N;
    
    float3 vTangent = mul(IN.Tangent, matBone[IN.nBoneIndex.x]) * IN.fWeight.x;
    vTangent += mul(IN.Tangent, matBone[IN.nBoneIndex.y]) * IN.fWeight.y;
    vTangent += mul(IN.Tangent, matBone[IN.nBoneIndex.z]) * IN.fWeight.z;
    vTangent += mul(IN.Tangent, matBone[IN.nBoneIndex.w]) * IN.fWeight.w;
    OUT.ViewTangent = vTangent;

    float3 vBinormal = mul(IN.Binormal, matBone[IN.nBoneIndex.x]) * IN.fWeight.x;
    vBinormal += mul(IN.Binormal, matBone[IN.nBoneIndex.y]) * IN.fWeight.y;
    vBinormal += mul(IN.Binormal, matBone[IN.nBoneIndex.z]) * IN.fWeight.z;
    vBinormal += mul(IN.Binormal, matBone[IN.nBoneIndex.w]) * IN.fWeight.w;
    OUT.ViewBinorm = vBinormal;

    OUT.LightVec = -viewlightDir;    
    OUT.TexCoord = IN.UV;

    float4 tempPos = float4(IN.Position.x,IN.Position.y,IN.Position.z,1.0);
    float3 Pos = mul(tempPos, matBone[IN.nBoneIndex.x]) * IN.fWeight.x;
    Pos += mul(tempPos, matBone[IN.nBoneIndex.y]) * IN.fWeight.y;
    Pos += mul(tempPos, matBone[IN.nBoneIndex.z]) * IN.fWeight.z;
    Pos += mul(tempPos, matBone[IN.nBoneIndex.w]) * IN.fWeight.w;
    
    OUT.pointValue = ApplyPointLight( Pos, N);	

    float4 f4WorldPos = mul( float4(Pos, 1.f), viewInverse);
    OUT.FogValue.y = f4WorldPos.x*vecRcpWorldSize.x;
    OUT.FogValue.z = 1.f - f4WorldPos.z*vecRcpWorldSize.y;

    // Normalize.
    OUT.ViewEyeVec = -Pos;
    
    OUT.HPosition = mul( float4(Pos,1.f) , projection);
    
    // -FOG-
	float fDist = fogFactor.x - Pos.z; // x=fogEnd
	OUT.FogValue.x = clamp ( fDist*fogFactor.y, 0.0f, 1.0f ); // y=forRange
	
    return OUT;
}

/********* pixel shader ********/
float3 ApplyWorldLightMap(float4 fLightMap)
{
	float3 fTemp;
	fTemp = fLightMap.xyz * 0.7f + 0.3f;
	return fTemp;
}

float4 mainPS(vertexOutput IN) : COLOR
{
	float3 bumps = Bumpy * (tex2D(normalSampler,IN.TexCoord.xy).xyz-(0.5).xxx);
	float4 spec = tex2D(specularSampler, IN.TexCoord.xy);
	
	float4 LightColor = tex2D(lightmapSampler, IN.FogValue.yz);
	
	float3 Ln = normalize(IN.LightVec.xyz);
	float3 Nn = normalize(IN.ViewNormal);
	float3 Tn = normalize(IN.ViewTangent);
	float3 Bn = normalize(IN.ViewBinorm);
	float3 Nb = Nn + (bumps.x * Tn + bumps.y * Bn);
	Nb = normalize(Nb);
	float3 Vn = normalize(IN.ViewEyeVec);
	float3 viewReflect = reflect(-Vn, Nn);

	viewReflect = mul(float4(viewReflect, 0.f), viewInverse);
	float4 envTexture = texCUBE( EnvSampler, viewReflect.xyz );
	spec.xyz *= (reflectpower.xxx*envTexture.xyz);

	float3 Hn = normalize(Vn + Ln);
//	float4 lighting = lit(dot(Ln,Nb),dot(Hn,Nb),SpecExpon);
	float	d = max(dot(Ln, Nb), 0.f);
	float	s = pow(max(dot(Hn, Nb), 0.f), SpecExpon);

	float4 diff = tex2D(diffuseSampler, IN.TexCoord.xy);
 	
	float4 result = (lightAmbient*materialAmbient + lightColor*materialDiffuse*d+float4(IN.pointValue*4,0))*diff + (lightSpecular*materialSpecular)*spec*s;// AmbiColor + diffContrib + specContrib;
	result.xyz *= ApplyWorldLightMap(LightColor);		
	result.w = materialAmbient.w*diff.w;

	// -FOG-
	result.xyz = lerp(fogColor, result, IN.FogValue.x);

	return result;
}

technique dx9textured_0
{
	pass p0	{
		VertexShader = compile vs_3_0 mainVS_0();
		PixelShader = compile ps_3_0 mainPS();
	}
}

technique dx9textured_1
{
	pass p0	{
		VertexShader = compile vs_3_0 mainVS_1();
		PixelShader = compile ps_3_0 mainPS();
	}
}

technique dx9textured_2
{
	pass p0	{
		VertexShader = compile vs_3_0 mainVS_2();
		PixelShader = compile ps_3_0 mainPS();
	}
}

technique dx9textured_3
{
	pass p0	{
		VertexShader = compile vs_3_0 mainVS_3();
		PixelShader = compile ps_3_0 mainPS();
	}
}

technique dx9textured_4
{
	pass p0	{
		VertexShader = compile vs_3_0 mainVS_4();
		PixelShader = compile ps_3_0 mainPS();
	}
}