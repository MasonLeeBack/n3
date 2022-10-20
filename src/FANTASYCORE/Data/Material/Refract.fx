/*
 *	refract
 */

//------------------------------------

shared float4x4 projection					: Projection;

float4x4 world						: World;
float4x4 worldViewProj				: WorldViewProjection;

//float4x4 projection					: Projection;
float4x3 matBone[50]				: WORLDMATRIXARRAY;		// 50 -> 256 개로 변경

// -FOG-
float4x4 WorldView					: WorldView;
shared float4 fogFactor				    : FOGFACTOR; // fogEnd, forRange, ?, ?
shared float4 fogColor						: FOGCOLOR; // r,g,b,1

float4 materialAmbient : MaterialAmbient
<
    string UIWidget = "Surface Color";
    string Space = "material";
> = { 1.f, 1.f, 1.f, 1.f};


texture normalTexture : NORMAL;
texture screenTexture : ScreenBuffer;




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
    float4 hPosition		: POSITION;
    float4 texCoordDiffuse	: TEXCOORD0;
/*    float4 diffAmbColor		: TEXCOORD1;
    float2 lightmapCoord	: TEXCOORD2;
    float4 specularColor    : COLOR0;*/
    float  FogValue	    	: COLOR1; // -FOG-
};

//------------------------------------
vertexOutput VS_TransformAndTexture_0(vertexInput0 IN) 
{
    vertexOutput OUT;
    
    float4 f4WorldPos = mul( float4(IN.position.xyz, 1.f), world);
   
    OUT.hPosition = mul( float4(IN.position.xyz , 1.0) , worldViewProj);

	//calculate our vectors N, E, L, and H
    float3 viewVertPos = mul(float4(IN.position,1.f), WorldView).xyz;
    OUT.texCoordDiffuse = IN.texCoordDiffuse;
    OUT.texCoordDiffuse.z = (OUT.hPosition.x/OUT.hPosition.w)*0.5f+0.5f;
    OUT.texCoordDiffuse.w = (OUT.hPosition.y/OUT.hPosition.w)*(-0.5f)+0.5f;

	// -FOG-
	float fDist = fogFactor.x - viewVertPos.z; // x=fogEnd
	OUT.FogValue = clamp ( fDist*fogFactor.y, 0.0f, 1.0f ); // y=forRange
	
    return OUT;
}

vertexOutput VS_TransformAndTexture_1(vertexInput1 IN) 
{
    vertexOutput OUT;
        
        // 임시
    float3 Pos = mul( float4(IN.position.xyz , 1.0), matBone[IN.nBoneIndex.x]) * 0.999; 
    Pos += mul( float4(IN.position.xyz , 1.0), matBone[IN.nBoneIndex.y]) * 0.001;
	OUT.hPosition = mul( float4(Pos,1.f) , projection);

    OUT.texCoordDiffuse = IN.texCoordDiffuse;
    OUT.texCoordDiffuse.z = (OUT.hPosition.x/OUT.hPosition.w)*0.5f+0.5f;
    OUT.texCoordDiffuse.w = (OUT.hPosition.y/OUT.hPosition.w)*(-0.5f)+0.5f;

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

    OUT.texCoordDiffuse = IN.texCoordDiffuse;
	OUT.texCoordDiffuse.z = (OUT.hPosition.x/OUT.hPosition.w)*0.5f+0.5f;
     OUT.texCoordDiffuse.w = (OUT.hPosition.y/OUT.hPosition.w)*(-0.5f)+0.5f;
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


    OUT.texCoordDiffuse = IN.texCoordDiffuse;
    OUT.texCoordDiffuse.z = (OUT.hPosition.x/OUT.hPosition.w)*0.5f+0.5f;
    OUT.texCoordDiffuse.w = (OUT.hPosition.y/OUT.hPosition.w)*(-0.5f)+0.5f;

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

    OUT.texCoordDiffuse = IN.texCoordDiffuse;
    OUT.texCoordDiffuse.z = (OUT.hPosition.x/OUT.hPosition.w)*0.5f+0.5f;
    OUT.texCoordDiffuse.w = (OUT.hPosition.y/OUT.hPosition.w)*(-0.5f)+0.5f;
	// -FOG-
	float fDist = fogFactor.x - Pos.z; // x=fogEnd
	OUT.FogValue = clamp ( fDist*fogFactor.y, 0.0f, 1.0f ); // y=forRange
	
    return OUT;
}

//------------------------------------

sampler ScreenSampler = sampler_state
{
	texture = <screenTexture>;
};

sampler2D normalSampler = sampler_state
{
	Texture = <normalTexture>;
};



//-----------------------------------
float4 PS_Textured( vertexOutput IN): COLOR
{
	float4 offset = tex2D( normalSampler, IN.texCoordDiffuse );
//	float4 glossTexture = tex2D( SpecularSampler, IN.texCoordDiffuse );
//	float4 LightColor = tex2D(lightmapSampler, IN.lightmapCoord);

	float2 off = (offset.xy*2.f-float2(1.f, 1.f)) *0.2f;
	float4 result = tex2D( ScreenSampler, IN.texCoordDiffuse.zw+ off );
	result.w = offset.a * materialAmbient.a;

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