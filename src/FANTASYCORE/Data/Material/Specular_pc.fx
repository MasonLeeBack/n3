/*
 *	Diffuse Specular, Directional Light, Per-Vertex
 */

//------------------------------------
float4x4 world						: World;
float4x4 worldViewProj				: WorldViewProjection;
float4x4 worldviewInverseTranspose  : WorldViewInverseTranspose;
float4x4 viewInverse : ViewInverse;

float4x4 projection					: Projection;
float4x3 matBone[50]				: WORLDMATRIXARRAY;

// -FOG-
float4x4 WorldView					: WorldView;
float4 fogFactor				    : FOGFACTOR; // fogEnd, forRange, ?, ?
float4 fogColor						: FOGCOLOR; // r,g,b,1

float  fRcpWorldXSize				: RcpWorldXSize;
float  fRcpWorldZSize				: RcpWorldZSize;

#include "commonFunction.fxh"

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

float4 viewlightDir : LightDirectionViewSpace
<
	string Object = "DirectionalLight";
    string Space = "World";
> = {1.0f, -1.0f, 1.0f, 0.0f};

float4 lightColor : LightDiffuse
<
    string UIName = "Diffuse Light Color";
    string Object = "DirectionalLight";
> = {1.0f, 1.0f, 1.0f, 1.0f};

float4 lightAmbient : LightAmbient
<
    string UIWidget = "Ambient Light Color";
    string Space = "material";
> = {0.0f, 0.0f, 0.0f, 1.0f};

float4 lightSpecular : LightSpecular
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
    float3	pointValue		: TEXCOORD3;
    float4 specularColor    : COLOR0;    
    float  FogValue	    	: COLOR1; // -FOG- 
};

//------------------------------------
vertexOutput VS_TransformAndTexture_0(vertexInput0 IN) 
{
    vertexOutput OUT;
    
    float4 f4WorldPos = mul( float4(IN.position.xyz, 1.f), world);
    OUT.lightmapCoord.x = f4WorldPos.x*fRcpWorldXSize;
    OUT.lightmapCoord.y = 1.f - f4WorldPos.z*fRcpWorldZSize;
    
    OUT.hPosition = mul( float4(IN.position.xyz , 1.0) , worldViewProj);

	//calculate our vectors N, E, L, and H
    float3 viewVertPos = mul(float4(IN.position,1.f), WorldView).xyz;
	float4 N = mul(IN.normal, worldviewInverseTranspose); //normal vector
	N.xyz = normalize(N.xyz);
    float3 E = normalize( -viewVertPos); //eye vector
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

    OUT.specularColor = lightSpecular * materialSpecular * spec;
    
    OUT.pointValue = ApplyPointLight( viewVertPos, N) ;
      
    OUT.texCoordDiffuse = IN.texCoordDiffuse;

	// -FOG-
	float fDist = fogFactor.x - viewVertPos.z; // x=fogEnd
	OUT.FogValue = clamp ( fDist*fogFactor.y, 0.0f, 1.0f ); // y=forRange
	
    return OUT;
}

vertexOutput VS_TransformAndTexture_1(vertexInput1 IN) 
{
    vertexOutput OUT;
        
        // юс╫ц
    float3 Pos = mul( float4(IN.position.xyz , 1.0), matBone[IN.nBoneIndex]);    
	OUT.hPosition = mul( float4(Pos,1.f) , projection);

    float4 f4WorldPos = mul( float4(Pos, 1.f), viewInverse);
    OUT.lightmapCoord.x = f4WorldPos.x*fRcpWorldXSize;
    OUT.lightmapCoord.y = 1.f - f4WorldPos.z*fRcpWorldZSize;

	//calculate our vectors N, E, L, and H
	float3 N = mul(IN.normal, matBone[IN.nBoneIndex]);
    float3 E = normalize( -Pos); //eye vector
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
    OUT.specularColor = lightSpecular * materialSpecular * spec;
    
    OUT.pointValue = ApplyPointLight( Pos, N) ;
      
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
    float3 Pos = mul( float4(IN.position.xyz , 1.0), matBone[IN.nBoneIndex.x]) * IN.fWeight.x;
    Pos += mul( float4(IN.position.xyz , 1.0), matBone[IN.nBoneIndex.y]) * IN.fWeight.y;
	OUT.hPosition = mul( float4(Pos,1.f) , projection);

    float4 f4WorldPos = mul( float4(Pos, 1.f), viewInverse);
    OUT.lightmapCoord.x = f4WorldPos.x*fRcpWorldXSize;
    OUT.lightmapCoord.y = 1.f - f4WorldPos.z*fRcpWorldZSize;

	//calculate our vectors N, E, L, and H
	float3 N = mul(IN.normal, matBone[IN.nBoneIndex.x]) * IN.fWeight.x;		//normal vector
	N += mul(IN.normal, matBone[IN.nBoneIndex.y]) * IN.fWeight.y;
	N =normalize(N);
    float3 E = normalize( -Pos); //eye vector
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
    OUT.specularColor =lightSpecular * materialSpecular * spec;
    
     OUT.pointValue = ApplyPointLight( Pos, N) ;
      
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
    float3 Pos = mul( float4(IN.position.xyz , 1.0), matBone[IN.nBoneIndex.x]) * IN.fWeight.x;
    Pos += mul( float4(IN.position.xyz , 1.0), matBone[IN.nBoneIndex.y]) * IN.fWeight.y;
    Pos += mul( float4(IN.position.xyz , 1.0), matBone[IN.nBoneIndex.z]) * IN.fWeight.z;
	OUT.hPosition = mul( float4(Pos,1.f) , projection);

    float4 f4WorldPos = mul( float4(Pos, 1.f), viewInverse);
    OUT.lightmapCoord.x = f4WorldPos.x*fRcpWorldXSize;
    OUT.lightmapCoord.y = 1.f - f4WorldPos.z*fRcpWorldZSize;

	//calculate our vectors N, E, L, and H
	float3 N = mul(IN.normal, matBone[IN.nBoneIndex.x]) * IN.fWeight.x;		//normal vector
	N += mul(IN.normal, matBone[IN.nBoneIndex.y]) * IN.fWeight.y;
	N += mul(IN.normal, matBone[IN.nBoneIndex.z]) * IN.fWeight.z;
	N =normalize(N);
    float3 E = normalize( -Pos); //eye vector
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

    OUT.specularColor =lightSpecular * materialSpecular * spec;
    
    OUT.pointValue = ApplyPointLight( Pos, N) ;
      
    OUT.texCoordDiffuse = IN.texCoordDiffuse;

	// -FOG-
	float fDist = fogFactor.x - Pos.z; // x=fogEnd
	OUT.FogValue = clamp ( fDist*fogFactor.y, 0.0f, 1.0f ); // y=forRange
	
    return OUT;
}

vertexOutput VS_TransformAndTexture_4(vertexInput2 IN) 
{
    vertexOutput OUT;
    float3 Pos = mul( float4(IN.position.xyz , 1.0), matBone[IN.nBoneIndex.x]) * IN.fWeight.x;
    Pos += mul( float4(IN.position.xyz , 1.0), matBone[IN.nBoneIndex.y]) * IN.fWeight.y;
    Pos += mul( float4(IN.position.xyz , 1.0), matBone[IN.nBoneIndex.z]) * IN.fWeight.z;
    Pos += mul( float4(IN.position.xyz , 1.0), matBone[IN.nBoneIndex.w]) * IN.fWeight.w;
	OUT.hPosition = mul( float4(Pos,1.f) , projection);

    float4 f4WorldPos = mul( float4(Pos, 1.f), viewInverse);
    OUT.lightmapCoord.x = f4WorldPos.x*fRcpWorldXSize;
    OUT.lightmapCoord.y = 1.f - f4WorldPos.z*fRcpWorldZSize;

	//calculate our vectors N, E, L, and H
	float3 N = mul(IN.normal, matBone[IN.nBoneIndex.x]) * IN.fWeight.x;		//normal vector
	N += mul(IN.normal, matBone[IN.nBoneIndex.y]) * IN.fWeight.y;
	N += mul(IN.normal, matBone[IN.nBoneIndex.z]) * IN.fWeight.z;
	N += mul(IN.normal, matBone[IN.nBoneIndex.w]) * IN.fWeight.w;
	N =normalize(N);
    float3 E = normalize( -Pos); //eye vector
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
    OUT.specularColor =lightSpecular * materialSpecular * spec;
    
        OUT.pointValue = ApplyPointLight( Pos, N) ;
      
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

sampler SpecularSampler = sampler_state 
{
    texture = <specularTexture>;
	AddressU  = WRAP;
	AddressV  = WRAP;
};

sampler lightmapSampler = sampler_state
{
    texture = <lightmapTexture>;
    MIPFILTER = LINEAR;
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
	float4 glossTexture = tex2D( SpecularSampler, IN.texCoordDiffuse );
	float4 LightColor = tex2D(lightmapSampler, IN.lightmapCoord);

	
	float4 result = (IN.diffAmbColor+float4(IN.pointValue*4,0))*diffuseTexture + float4((IN.specularColor.xyz+float4(IN.pointValue,0))*glossTexture.xyz, 0.f);
	result.xyz*=ApplyWorldLightMap(LightColor);

	// -FOG-
	result.xyz = lerp(fogColor, result, IN.FogValue);

	return result;
}

//-----------------------------------
//-----------------------------------
technique textured_0				// No Physique
{
    pass p0 
    {		
		VertexShader = compile vs_3_0 VS_TransformAndTexture_0();
		PixelShader  = compile ps_2_0 PS_Textured();
    }
}

technique textured_1				// 1 Bone Link
{
    pass p0 
    {		
		VertexShader = compile vs_3_0 VS_TransformAndTexture_1();
		PixelShader  = compile ps_2_0 PS_Textured();
    }
}

technique textured_2				// 2 Bone Link
{
    pass p0 
    {		
		VertexShader = compile vs_3_0 VS_TransformAndTexture_2();
		PixelShader  = compile ps_2_0 PS_Textured();
    }
}

technique textured_3				// 3 Bone Link
{
    pass p0 
    {		
		VertexShader = compile vs_3_0 VS_TransformAndTexture_3();
		PixelShader  = compile ps_2_0 PS_Textured();
    }
}

technique textured_4				// 4 Bone Link
{
    pass p0 
    {		
		VertexShader = compile vs_3_0 VS_TransformAndTexture_4();
		PixelShader  = compile ps_2_0 PS_Textured();
    }
}

