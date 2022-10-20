/*
 *	Diffuse, Directional Light, Per-Vertex
 */

//------------------------------------
float4x4 world						: World;
float4x4 worldViewProj				: WorldViewProjection;
float4x4 worldviewInverseTranspose	: WorldViewInverseTranspose;
float4x4 viewInverse				: ViewInverse;

float time : Time;

float4x4 projection					: Projection;
float4x3 matBone[50]				: WORLDMATRIXARRAY;

// -FOG-
shared float4 fogFactor				    : FOGFACTOR;	// fogEnd, forRange, ?, ?
shared float4 fogColor						: FOGCOLOR;		// r,g,b,1

texture diffuseTexture : Diffuse
<
	string ResourceName = "default_color.dds";
>;

texture noiseTexture : Volume
<
	string ResourceName = "default_color.dds";
	string resourcetype = "3D";
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
    int4    nBoneIndex			: BLENDINDICES0;
};

struct vertexInput2 {
    float3 position				: POSITION;
    float3 normal				: NORMAL;
    float4 texCoordDiffuse		: TEXCOORD0;
	int4	   nBoneIndex			: BLENDINDICES;
	float4	   fWeight			: BLENDWEIGHT;
};

struct vertexOutput {
    float4 hPosition			: POSITION;
    float4 texCoordDiffuse	: TEXCOORD0;
    float4 diffAmbColor		: TEXCOORD1;
    float4  FogValue	 	   	: TEXCOORD2;
};

//------------------------------------
vertexOutput VS_TransformAndTexture_0(vertexInput0 IN) 
{
    vertexOutput OUT;
    float4 f4WorldPos = mul( float4(IN.position.xyz, 1.f), world);
    
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

	// -FOG-
	float fDist = fogFactor.x - OUT.hPosition.z; // x=fogEnd
	OUT.FogValue.x = clamp ( fDist*fogFactor.y, 0.0f, 1.0f ); // y=forRange
	OUT.FogValue.yzw =float3(2.f*(IN.texCoordDiffuse.xy), 1.f);

    return OUT;
}

vertexOutput VS_TransformAndTexture_1(vertexInput1 IN) 
{
    vertexOutput OUT;
    // Matrix Array로 넘어오는것 WorldViewMatrix이다.
    float3 Pos = mul( float4(IN.position.xyz , 1.0), matBone[IN.nBoneIndex.x]) * 0.999;
    Pos += mul( float4(IN.position.xyz , 1.f), matBone[IN.nBoneIndex.y]) * 0.001;
	OUT.hPosition = mul( float4(Pos,1.f) , projection);

	//calculate our vectors N, E, L, and H
	// 뷰공간에서 라이트 계산
	float3 N = normalize(mul(IN.normal, matBone[IN.nBoneIndex.x]));		//normal vector
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
	OUT.FogValue.x = clamp ( fDist*fogFactor.y, 0.0f, 1.0f ); // y=forRange
//	OUT.FogValue.yzw =OUT.hPosition.xyz/OUT.hPosition.w;
	OUT.FogValue.yzw =float3(2.f*(IN.texCoordDiffuse.xy), 1.f);
	
    return OUT;
}

vertexOutput VS_TransformAndTexture_2(vertexInput2 IN) 
{
    vertexOutput OUT;
    // Matrix Array로 넘어오는것 WorldViewMatrix이다.
    float3 Pos = mul( float4(IN.position.xyz , 1.f), matBone[IN.nBoneIndex.x]) * IN.fWeight.x;
    Pos += mul( float4(IN.position.xyz , 1.f), matBone[IN.nBoneIndex.y]) * IN.fWeight.y;
	OUT.hPosition = mul( float4(Pos,1.f) , projection);

	//calculate our vectors N, E, L, and H
	float3 N = mul(IN.normal, matBone[IN.nBoneIndex.x]) * IN.fWeight.x;		//normal vector
	N += mul(IN.normal, matBone[IN.nBoneIndex.y]) * IN.fWeight.y;
	N = normalize(N);
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
//	OUT.FogValue.yzw =OUT.hPosition.xyz/OUT.hPosition.w;
	OUT.FogValue.yzw =float3(2.f*(IN.texCoordDiffuse.xy), 1.f);

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

	//calculate our vectors N, E, L, and H
	float3 N = mul(IN.normal, matBone[IN.nBoneIndex.x]) * IN.fWeight.x;		//normal vector
	N += mul(IN.normal, matBone[IN.nBoneIndex.y]) * IN.fWeight.y;
	N += mul(IN.normal, matBone[IN.nBoneIndex.z]) * IN.fWeight.z;
	N = normalize(N);
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
	OUT.FogValue.x = clamp ( fDist*fogFactor.y, 0.0f, 1.0f ); // y=forRange
//	OUT.FogValue.yzw =OUT.hPosition.xyz/OUT.hPosition.w;
	OUT.FogValue.yzw =float3(2.f*(IN.texCoordDiffuse.xy), 1.f);
	
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

	//calculate our vectors N, E, L, and H
	float3 N = mul(IN.normal, matBone[IN.nBoneIndex.x]) * IN.fWeight.x;		//normal vector
	N += mul(IN.normal, matBone[IN.nBoneIndex.y]) * IN.fWeight.y;
	N += mul(IN.normal, matBone[IN.nBoneIndex.z]) * IN.fWeight.z;
	N += mul(IN.normal, matBone[IN.nBoneIndex.w]) * IN.fWeight.w;
	N = normalize(N);
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
	OUT.FogValue.x = clamp ( fDist*fogFactor.y, 0.0f, 1.0f ); // y=forRange
//	OUT.FogValue.yzw =OUT.hPosition.xyz/OUT.hPosition.w;
	OUT.FogValue.yzw =float3(2.f*(IN.texCoordDiffuse.xy), 1.f);
	
    return OUT;
}

//------------------------------------
sampler TextureSampler = sampler_state 
{
	texture = <diffuseTexture>;
	ADDRESSU = WRAP;
	
	MIPFILTER = LINEAR;
	MINFILTER = LINEAR;
	MAGFILTER = LINEAR;
};

sampler3D noiseSampler = sampler_state 
{
	texture = <noiseTexture>;
	ADDRESSU = WRAP;
	ADDRESSV = WRAP;
	ADDRESSW = WRAP;
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
	float3 f3TexCoord;
	f3TexCoord.xy = IN.FogValue.yz* 0.95f;
	f3TexCoord.z = time * 0.05f;
	float noiseX = tex3D(noiseSampler, f3TexCoord) - 0.5;
	float noiseY = tex3D(noiseSampler, f3TexCoord + 0.5) - 0.5;
	
	// Offset our base noise with the bubble noise
	f3TexCoord.x = IN.FogValue.y + noiseX * 1.61f;
	f3TexCoord.y =IN.FogValue.z + noiseY * 1.61f;
	f3TexCoord.z = time * 0.11;
	
	float base = tex3D(noiseSampler, f3TexCoord);
   
	float4 f4Temp = float4( 0.f, 0.2f, 0.4f, 1.f)+2.f*tex1D(TextureSampler, 0.82f * base - 0.12f );
	return f4Temp;
}

//-----------------------------------
technique textured_0				// No Physique
{
	pass p0 
	{		
		VertexShader = compile vs_1_1 VS_TransformAndTexture_0();
		PixelShader  = compile ps_2_0 PS_Textured();
	}
}

technique textured_1				// 1 Bone Link
{
	pass p0 
	{		
		VertexShader = compile vs_1_1 VS_TransformAndTexture_1();
		PixelShader  = compile ps_2_0 PS_Textured();
	}
}

technique textured_2				// 2 Bone Link
{
	pass p0 
	{		
		VertexShader = compile vs_1_1 VS_TransformAndTexture_2();
		PixelShader  = compile ps_2_0 PS_Textured();
	}
}

technique textured_3				// 3 Bone Link
{
	pass p0 
	{		
		VertexShader = compile vs_1_1 VS_TransformAndTexture_3();
		PixelShader  = compile ps_2_0 PS_Textured();
	}
}

technique textured_4				// 4 Bone Link
{
	pass p0 
	{		
		VertexShader = compile vs_1_1 VS_TransformAndTexture_4();
		PixelShader  = compile ps_2_0 PS_Textured();
	}
}