/*
 *	No Texture, Directional Light, Per-Vertex
 */

//------------------------------------
shared float4x4 projection					: Projection;

float4x4 worldViewProj			: WorldViewProjection;
float4x4 worldviewInverseTranspose	: WorldViewInverseTranspose;

float4x3 matBone[50]			: WORLDMATRIXARRAY;	

// -FOG-
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
};

struct vertexInput1 {
    float3 position				: POSITION;
    float3 normal				: NORMAL;
    int    nBoneIndex			: BLENDINDICES0;
};

struct vertexInput2 {
    float3 position				: POSITION;
    float3 normal				: NORMAL;
	int4	   nBoneIndex		: BLENDINDICES;
	float4	   fWeight			: BLENDWEIGHT;
};

struct vertexOutput {
    float4 hPosition			: POSITION;
    float4 diffAmbColor		: TEXCOORD0;
    float  FogValue	    		: COLOR0;
};

//------------------------------------
vertexOutput VS_TransformAndTexture_0(vertexInput0 IN) 
{
    vertexOutput OUT;
    OUT.hPosition = mul( float4(IN.position.xyz , 1.0) , worldViewProj);

	//calculate our vectors N, E, L, and H
	float4 N = mul(IN.normal, worldviewInverseTranspose); //normal vector
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
	OUT.FogValue = clamp ( fDist*fogFactor.y, 0.0f, 1.0f ); // y=forRange

    return OUT;
}

vertexOutput VS_TransformAndTexture_1(vertexInput1 IN) 
{
    vertexOutput OUT;
    float3 Pos = mul( float4(IN.position.xyz , 1.0), matBone[IN.nBoneIndex]);
	OUT.hPosition = mul( float4(Pos,1.f) , projection);

	//calculate our vectors N, E, L, and H
	float3 N = mul(IN.normal, matBone[IN.nBoneIndex]);		//normal vector
    float3 L = ( -viewlightDir.xyz); //light vector

	//calculate the diffuse
    float  diff = max(0 , dot(N,L));

	//output diffuse
    float4 ambColor = materialAmbient * lightAmbient;
    float4 diffColor = materialDiffuse * diff * lightColor ;
    OUT.diffAmbColor = diffColor + ambColor;
	OUT.diffAmbColor.w = materialAmbient.w;

	// -FOG-
	float fDist = fogFactor.x - Pos.z; // x=fogEnd
	OUT.FogValue = clamp ( fDist*fogFactor.y, 0.0f, 1.0f ); // y=forRange
	
    return OUT;
}

vertexOutput VS_TransformAndTexture_2(vertexInput2 IN) 
{
    vertexOutput OUT;
    float3 Pos = mul( float4(IN.position.xyz , 1.0), matBone[IN.nBoneIndex.x]) * IN.fWeight.x;
    Pos += mul( float4(IN.position.xyz, 1.f), matBone[IN.nBoneIndex.y]) * IN.fWeight.y;
	OUT.hPosition = mul( float4(Pos,1.f) , projection);

	//calculate our vectors N, E, L, and H
	float3 N = mul(IN.normal, matBone[IN.nBoneIndex.x]) * IN.fWeight.x;		//normal vector
	N += mul(IN.normal, matBone[IN.nBoneIndex.y])*IN.fWeight.y;
	N = normalize(N);
    float3 L = ( -viewlightDir.xyz); //light vector

	//calculate the diffuse
    float  diff = max(0 , dot(N,L));

	//output diffuse
    float4 ambColor = materialAmbient * lightAmbient;
    float4 diffColor = materialDiffuse * diff * lightColor ;
    OUT.diffAmbColor = diffColor + ambColor;
	    OUT.diffAmbColor.w = materialAmbient.w;
	// -FOG-
	float fDist = fogFactor.x - Pos.z; // x = fogEnd
	OUT.FogValue = clamp ( fDist*fogFactor.y, 0.0f, 1.0f ); // y = fogRange
	
    return OUT;
}

vertexOutput VS_TransformAndTexture_3(vertexInput2 IN) 
{
    vertexOutput OUT;
    float3 Pos = mul( float4(IN.position.xyz , 1.0), matBone[IN.nBoneIndex.x]) * IN.fWeight.x;
    Pos += mul( float4(IN.position.xyz, 1.f), matBone[IN.nBoneIndex.y]) * IN.fWeight.y;
    Pos += mul( float4(IN.position.xyz, 1.f), matBone[IN.nBoneIndex.z]) * IN.fWeight.z;
	OUT.hPosition = mul( float4(Pos,1.f) , projection);

	//calculate our vectors N, E, L, and H
	float3 N = mul(IN.normal, matBone[IN.nBoneIndex.x]) * IN.fWeight.x;		//normal vector
	N += mul(IN.normal, matBone[IN.nBoneIndex.y])*IN.fWeight.y;
	N += mul(IN.normal, matBone[IN.nBoneIndex.z])*IN.fWeight.z;
	N = normalize(N);
    float3 L = ( -viewlightDir.xyz); //light vector

	//calculate the diffuse
    float  diff = max(0 , dot(N,L));

	//output diffuse
    float4 ambColor = materialAmbient * lightAmbient;
    float4 diffColor = materialDiffuse * diff * lightColor ;
    OUT.diffAmbColor = diffColor + ambColor;
	    OUT.diffAmbColor.w = materialAmbient.w;
	// -FOG-
	float fDist = fogFactor.x - Pos.z; // x = fogEnd
	OUT.FogValue = clamp ( fDist*fogFactor.y, 0.0f, 1.0f ); // y = fogRange
	
    return OUT;
}

vertexOutput VS_TransformAndTexture_4(vertexInput2 IN) 
{
    vertexOutput OUT;
    float3 Pos = mul( float4(IN.position.xyz , 1.0), matBone[IN.nBoneIndex.x]) * IN.fWeight.x;
    Pos += mul( float4(IN.position.xyz, 1.f), matBone[IN.nBoneIndex.y]) * IN.fWeight.y;
    Pos += mul( float4(IN.position.xyz, 1.f), matBone[IN.nBoneIndex.z]) * IN.fWeight.z;
    Pos += mul( float4(IN.position.xyz, 1.f), matBone[IN.nBoneIndex.w]) * IN.fWeight.w;
	OUT.hPosition = mul( float4(Pos,1.f) , projection);

	//calculate our vectors N, E, L, and H
	float3 N = mul(IN.normal, matBone[IN.nBoneIndex.x]) * IN.fWeight.x;		//normal vector
	N += mul(IN.normal, matBone[IN.nBoneIndex.y])*IN.fWeight.y;
	N += mul(IN.normal, matBone[IN.nBoneIndex.z])*IN.fWeight.z;
	N += mul(IN.normal, matBone[IN.nBoneIndex.w])*IN.fWeight.w;
	N = normalize(N);
    float3 L = ( -viewlightDir.xyz); //light vector

	//calculate the diffuse
    float  diff = max(0 , dot(N,L));

	//output diffuse
    float4 ambColor = materialAmbient * lightAmbient;
    float4 diffColor = materialDiffuse * diff * lightColor ;
    OUT.diffAmbColor = diffColor + ambColor;
	    OUT.diffAmbColor.w = materialAmbient.w;
	// -FOG-
	float fDist = fogFactor.x - Pos.z; // x = fogEnd
	OUT.FogValue = clamp ( fDist*fogFactor.y, 0.0f, 1.0f ); // y = fogRange
	
    return OUT;
}


//-----------------------------------
float4 PS_Textured( vertexOutput IN): COLOR
{
	float4 result = IN.diffAmbColor;
	
	// -FOG-
	result.xyz = lerp(fogColor, result, IN.FogValue);
	
	return result;
}


//-----------------------------------
technique textured_0
{
    pass p0 
    {		
		VertexShader = compile vs_1_1 VS_TransformAndTexture_0();
		PixelShader  = compile ps_2_0 PS_Textured();
    }
}

technique textured_1
{
    pass p0 
    {		
		VertexShader = compile vs_1_1 VS_TransformAndTexture_1();
		PixelShader  = compile ps_2_0 PS_Textured();
    }
}

technique textured_2
{
    pass p0 
    {		
		VertexShader = compile vs_1_1 VS_TransformAndTexture_2();
		PixelShader  = compile ps_2_0 PS_Textured();
    }
}

technique textured_3
{
    pass p0 
    {		
		VertexShader = compile vs_1_1 VS_TransformAndTexture_3();
		PixelShader  = compile ps_2_0 PS_Textured();
    }
}

technique textured_4
{
    pass p0 
    {		
		VertexShader = compile vs_1_1 VS_TransformAndTexture_4();
		PixelShader  = compile ps_2_0 PS_Textured();
    }
}