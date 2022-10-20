/*
 *	For Shadow map
 */

float4x4 worldViewProj			: SHADOW_ALL;

float4x4 projectionForShadow		: SHADOW_NOVIEW_ALL;
float4x3 matBone[50]			: WORLDMATRIXARRAY;		

//------------------------------------
struct vertexInput0 {
    float3 position				: POSITION;
};

struct vertexInput1 {
    float3 position				: POSITION;
    int    nBoneIndex			: BLENDINDICES0;
};

struct vertexInput2 {
    float3		position		: POSITION;
	int4		nBoneIndex		: BLENDINDICES;
	float4		fWeight			: BLENDWEIGHT;
};

struct vertexOutput {
    float4	hPosition			: POSITION;
    float	TexCoord			: TEXCOORD0;
};

//------------------------------------
vertexOutput VS_TransformAndTexture_0(vertexInput0 IN) 
{
    vertexOutput OUT;
    OUT.hPosition = mul( float4(IN.position.xyz , 1.0) , worldViewProj);
    
    OUT.TexCoord = OUT.hPosition.z/OUT.hPosition.w;

    return OUT;
}

vertexOutput VS_TransformAndTexture_1(vertexInput1 IN) 
{
    vertexOutput OUT;
    
    float3 Pos = mul( float4(IN.position.xyz , 1.0), matBone[IN.nBoneIndex]);
       
	OUT.hPosition = mul( float4(Pos,1.f) , projectionForShadow);
	
    OUT.TexCoord = OUT.hPosition.z/OUT.hPosition.w;

    return OUT;
}

vertexOutput VS_TransformAndTexture_2(vertexInput2 IN) 
{
    vertexOutput OUT;
    float3 Pos = mul( float4(IN.position.xyz , 1.0), matBone[IN.nBoneIndex.x]) * IN.fWeight.x;
    Pos += mul( float4(IN.position.xyz, 1.f), matBone[IN.nBoneIndex.y]) * IN.fWeight.y;
	OUT.hPosition = mul( float4(Pos,1.f) , projectionForShadow);

    OUT.TexCoord = OUT.hPosition.z/OUT.hPosition.w;

    return OUT;
}

vertexOutput VS_TransformAndTexture_3(vertexInput2 IN) 
{
    vertexOutput OUT;
    float3 Pos = mul( float4(IN.position.xyz , 1.0), matBone[IN.nBoneIndex.x]) * IN.fWeight.x;
    Pos += mul( float4(IN.position.xyz, 1.f), matBone[IN.nBoneIndex.y]) * IN.fWeight.y;
    Pos += mul( float4(IN.position.xyz, 1.f), matBone[IN.nBoneIndex.z]) * IN.fWeight.z;
	OUT.hPosition = mul( float4(Pos,1.f) , projectionForShadow);

    OUT.TexCoord = OUT.hPosition.z/OUT.hPosition.w;

    return OUT;
}

vertexOutput VS_TransformAndTexture_4(vertexInput2 IN) 
{
    vertexOutput OUT;
    float3 Pos = mul( float4(IN.position.xyz , 1.0), matBone[IN.nBoneIndex.x]) * IN.fWeight.x;
    Pos += mul( float4(IN.position.xyz, 1.f), matBone[IN.nBoneIndex.y]) * IN.fWeight.y;
    Pos += mul( float4(IN.position.xyz, 1.f), matBone[IN.nBoneIndex.z]) * IN.fWeight.z;
    Pos += mul( float4(IN.position.xyz, 1.f), matBone[IN.nBoneIndex.w]) * IN.fWeight.w;
	OUT.hPosition = mul( float4(Pos,1.f) , projectionForShadow);

    OUT.TexCoord = OUT.hPosition.z/OUT.hPosition.w;

    return OUT;
}

//-----------------------------------
float4 PS_Textured( vertexOutput IN): COLOR
{
    return float4(IN.TexCoord.xxx, 1.0);
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