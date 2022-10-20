/*
 *	Diffuse, Directional Light, Per-Vertex
 */

//------------------------------------

#include "CommonFunction.fxh"

shared float4x4 projection					: Projection;
shared float4x4 viewInverse 					: ViewInverse;
shared float4	vecRcpWorldSize					: RcpWorldSize;

float4x4 WorldView					: WorldView;
float4x4 world						: World;
float4x4 worldViewProj				: WorldViewProjection;
float4x4 worldviewInverseTranspose	: WorldViewInverseTranspose;

float4x3 matBone[50]				: WORLDMATRIXARRAY;

// -FOG-
shared float4 fogFactor			    : FOGFACTOR;	// fogEnd, forRange, ?, ?
shared float4 fogColor						: FOGCOLOR;		// r,g,b,1

texture diffuseTexture : Diffuse
<
	string ResourceName = "default_color.dds";
>;

texture lightmapTexture : WORLDLIGHTMAP
<
	string ResourceName = "default_color.dds";
>;

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

//------------------------------------
struct vertexInput0 {
    float3 position				: POSITION;
    float3 normal				: NORMAL;
    float4 texCoordDiffuse		: TEXCOORD0;
};

struct vertexInput1 {
    float3 position				: POSITION;
    float3 normal				: NORMAL;
    float4 texCoordDiffuse		: TEXCOORD0;
    int    nBoneIndex			: BLENDINDICES0;
};

struct vertexInput2 {
    float3 position				: POSITION;
    float3 normal				: NORMAL;
    float4 texCoordDiffuse		: TEXCOORD0;
	int4	   nBoneIndex			: BLENDINDICES;
	float4	   fWeight			: BLENDWEIGHT;
};

struct vertexOutput {
    float4 hPosition		: POSITION;
    float4 texCoordDiffuse	: TEXCOORD0;
    float4 diffAmbColor		: TEXCOORD1;
    float2 lightmapCoord	: TEXCOORD2;
    float3 pointValue 		: TEXCOORD3;
    float  FogValue	    	: COLOR0; // -FOG-
};

//------------------------------------
vertexOutput VS_TransformAndTexture_0(vertexInput0 IN) 
{
    vertexOutput OUT;
    float4 f4WorldPos = mul( float4(IN.position.xyz, 1.f), world);
    
    OUT.lightmapCoord.xy = f4WorldPos.xz*vecRcpWorldSize.xy;
    OUT.lightmapCoord.y = 1.f - OUT.lightmapCoord.y; 
    
    OUT.hPosition = mul( float4(IN.position.xyz , 1.0) , worldViewProj);
    OUT.texCoordDiffuse = IN.texCoordDiffuse;

	//calculate our vectors N, E, L, and H
	// 월드에서 라이트 계산!!
	float4 N = mul(IN.normal, worldviewInverseTranspose); //normal vector
	N.xyz = normalize(N.xyz);
    float3 L = ( -viewlightDir.xyz); //light vector

	//calculate the diffuse
    float  diff = max(0 , dot(N,L));

	//output diffuse
    float4 ambColor = materialAmbient * lightAmbient;
    float4 diffColor = materialDiffuse * diff * lightColor ;
    OUT.diffAmbColor = diffColor + ambColor;
    OUT.diffAmbColor.w = materialAmbient.w;
    
    float3 viewVertPos = mul( float4(IN.position.xyz, 1.f), WorldView );
    OUT.pointValue = ApplyPointLight( viewVertPos, N) ;
    // diffuse 만 이렇게    

	// -FOG-
	float fDist = fogFactor.x - OUT.hPosition.z; // x=fogEnd
	OUT.FogValue = clamp ( fDist*fogFactor.y, 0.0f, 1.0f ); // y=forRange
	
    return OUT;
}

//------------------------------------
vertexOutput VS_TransformAndTexture_1(vertexInput1 IN) 
{
    vertexOutput OUT;
    // Matrix Array로 넘어오는것 WorldViewMatrix이다.
    float3 Pos = mul( float4(IN.position.xyz , 1.0), matBone[IN.nBoneIndex]);    
	OUT.hPosition = mul( float4(Pos,1.f) , projection);

    float4 f4WorldPos = mul( float4(Pos, 1.f), viewInverse);
    OUT.lightmapCoord.xy = f4WorldPos.xz*vecRcpWorldSize.xy;
    OUT.lightmapCoord.y = 1.f - OUT.lightmapCoord.y; 

	//calculate our vectors N, E, L, and H
	// 뷰공간에서 라이트 계산
	float3 N = normalize(mul(IN.normal, matBone[IN.nBoneIndex]));		//normal vector
	
	float3 viewVertPos = mul( Pos, WorldView );
    OUT.pointValue = ApplyPointLight( Pos, N) ;
    
    float3 L = ( -viewlightDir.xyz);					//light vector

	//calculate the diffuse
    float  diff = max(0 , dot(N,L));

	//output diffuse
    float4 ambColor = materialAmbient * lightAmbient;
    float4 diffColor = materialDiffuse * diff * lightColor ;
    OUT.diffAmbColor = diffColor + ambColor;
    OUT.diffAmbColor.w = materialAmbient.w;

    OUT.texCoordDiffuse = IN.texCoordDiffuse;
	
	// -FOG-
	float fDist = fogFactor.x - Pos.z; // x=fogEnd
	OUT.FogValue = clamp ( fDist*fogFactor.y, 0.0f, 1.0f ); // y=forRange
	
    return OUT;
}

vertexOutput VS_TransformAndTexture_2(vertexInput2 IN) 
{
    vertexOutput OUT;
    // Matrix Array로 넘어오는것 WorldViewMatrix이다.
    float3 Pos = mul( float4(IN.position.xyz , 1.f), matBone[IN.nBoneIndex.x]) * IN.fWeight.x;
    Pos += mul( float4(IN.position.xyz , 1.f), matBone[IN.nBoneIndex.y]) * IN.fWeight.y;
	OUT.hPosition = mul( float4(Pos,1.f) , projection);

    float4 f4WorldPos = mul( float4(Pos, 1.f), viewInverse);
    OUT.lightmapCoord.xy = f4WorldPos.xz*vecRcpWorldSize.xy;
    OUT.lightmapCoord.y = 1.f - OUT.lightmapCoord.y; 

	//calculate our vectors N, E, L, and H
	float3 N = mul(IN.normal, matBone[IN.nBoneIndex.x]) * IN.fWeight.x;		//normal vector
	N += mul(IN.normal, matBone[IN.nBoneIndex.y]) * IN.fWeight.y;
	N = normalize(N);
	
    OUT.pointValue = ApplyPointLight( Pos, N) ;
    
    float3 L = ( -viewlightDir.xyz);					//light vector

	//calculate the diffuse
    float  diff = max(0 , dot(N,L));

	//output diffuse
    float4 ambColor = materialAmbient * lightAmbient;
    float4 diffColor = materialDiffuse * diff * lightColor ;
    OUT.diffAmbColor = diffColor + ambColor;
    OUT.diffAmbColor.w = materialAmbient.w;

    OUT.texCoordDiffuse = IN.texCoordDiffuse;

	// -FOG-
	float fDist = fogFactor.x - Pos.z; // x=fogEnd
	OUT.FogValue = clamp ( fDist*fogFactor.y, 0.0f, 1.0f ); // y=forRange
	
    return OUT;
}

vertexOutput VS_TransformAndTexture_3(vertexInput2 IN) 
{
    vertexOutput OUT;
    // Matrix Array로 넘어오는것 WorldViewMatrix이다.
    float3 Pos = mul( float4(IN.position.xyz , 1.f), matBone[IN.nBoneIndex.x]) * IN.fWeight.x;
    Pos += mul( float4(IN.position.xyz , 1.f), matBone[IN.nBoneIndex.y]) * IN.fWeight.y;
    Pos += mul( float4(IN.position.xyz , 1.f), matBone[IN.nBoneIndex.z]) * IN.fWeight.z;
    
	OUT.hPosition = mul( float4(Pos,1.f) , projection);

    float4 f4WorldPos = mul( float4(Pos, 1.f), viewInverse);
    OUT.lightmapCoord.xy = f4WorldPos.xz*vecRcpWorldSize.xy;
    OUT.lightmapCoord.y = 1.f - OUT.lightmapCoord.y; 

	//calculate our vectors N, E, L, and H
	float3 N = mul(IN.normal, matBone[IN.nBoneIndex.x]) * IN.fWeight.x;		//normal vector
	N += mul(IN.normal, matBone[IN.nBoneIndex.y]) * IN.fWeight.y;
	N += mul(IN.normal, matBone[IN.nBoneIndex.z]) * IN.fWeight.z;
	N = normalize(N);
	
	OUT.pointValue = ApplyPointLight( Pos, N) ;
	
    float3 L = ( -viewlightDir.xyz);					//light vector

	//calculate the diffuse
    float  diff = max(0 , dot(N,L));

	//output diffuse
    float4 ambColor = materialAmbient * lightAmbient;
    float4 diffColor = materialDiffuse * diff * lightColor ;
    OUT.diffAmbColor = diffColor + ambColor;
    OUT.diffAmbColor.w = materialAmbient.w;

    OUT.texCoordDiffuse = IN.texCoordDiffuse;

	// -FOG-
	float fDist = fogFactor.x - Pos.z; // x=fogEnd
	OUT.FogValue = clamp ( fDist*fogFactor.y, 0.0f, 1.0f ); // y=forRange
	
    return OUT;
}

vertexOutput VS_TransformAndTexture_4(vertexInput2 IN) 
{
    vertexOutput OUT;
    // Matrix Array로 넘어오는것 WorldViewMatrix이다.
    float3 Pos = mul( float4(IN.position.xyz , 1.f), matBone[IN.nBoneIndex.x]) * IN.fWeight.x;
    Pos += mul( float4(IN.position.xyz , 1.f), matBone[IN.nBoneIndex.y]) * IN.fWeight.y;
    Pos += mul( float4(IN.position.xyz , 1.f), matBone[IN.nBoneIndex.z]) * IN.fWeight.z;
    Pos += mul( float4(IN.position.xyz , 1.f), matBone[IN.nBoneIndex.w]) * IN.fWeight.w;
	OUT.hPosition = mul( float4(Pos,1.f) , projection);

    float4 f4WorldPos = mul( float4(Pos, 1.f), viewInverse);
    OUT.lightmapCoord.xy = f4WorldPos.xz*vecRcpWorldSize.xy;
    OUT.lightmapCoord.y = 1.f - OUT.lightmapCoord.y; 

	//calculate our vectors N, E, L, and H
	float3 N = mul(IN.normal, matBone[IN.nBoneIndex.x]) * IN.fWeight.x;		//normal vector
	N += mul(IN.normal, matBone[IN.nBoneIndex.y]) * IN.fWeight.y;
	N += mul(IN.normal, matBone[IN.nBoneIndex.z]) * IN.fWeight.z;
	N += mul(IN.normal, matBone[IN.nBoneIndex.w]) * IN.fWeight.w;
	N = normalize(N);
	
	OUT.pointValue = ApplyPointLight( Pos, N) ;
	
    float3 L = ( -viewlightDir.xyz);					//light vector

	//calculate the diffuse
    float  diff = max(0 , dot(N,L));

	//output diffuse
    float4 ambColor = materialAmbient * lightAmbient;
    float4 diffColor = materialDiffuse * diff * lightColor ;
    OUT.diffAmbColor = diffColor + ambColor;
    OUT.diffAmbColor.w = materialAmbient.w;

    OUT.texCoordDiffuse = IN.texCoordDiffuse;

	// -FOG-
	float fDist = fogFactor.x - Pos.z; // x=fogEnd
	OUT.FogValue = clamp ( fDist*fogFactor.y, 0.0f, 1.0f ); // y=forRange
	
    return OUT;
}


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

	float4 result = (IN.diffAmbColor  + float4(IN.pointValue*4, 0))* diffuseTexture;

	//*******************************************************************	
	result.xyz *= (ApplyWorldLightMap(LightColor));
	//*******************************************************************	
	// -FOG-
	result.xyz = lerp(fogColor, result, IN.FogValue);

	return result;
}

//-----------------------------------
technique textured_0				// No Physique
{
	pass p0 
	{		
		VertexShader = compile vs_3_0 VS_TransformAndTexture_0();
		PixelShader  = compile ps_3_0 PS_Textured();
	}
}

technique textured_1				// 1 Bone Link
{
	pass p0 
	{		
		VertexShader = compile vs_3_0 VS_TransformAndTexture_1();
		PixelShader  = compile ps_3_0 PS_Textured();
	}
}

technique textured_2				// 2 Bone Link
{
	pass p0 
	{		
		VertexShader = compile vs_3_0 VS_TransformAndTexture_2();
		PixelShader  = compile ps_3_0 PS_Textured();
	}
}

technique textured_3				// 3 Bone Link
{
	pass p0 
	{		
		VertexShader = compile vs_3_0 VS_TransformAndTexture_3();
		PixelShader  = compile ps_3_0 PS_Textured();
	}
}

technique textured_4				// 4 Bone Link
{
	pass p0 
	{		
		VertexShader = compile vs_3_0 VS_TransformAndTexture_4();
		PixelShader  = compile ps_3_0 PS_Textured();
	}
}