/*
 *	Shadow Instancing
 *	No Animation∏∏ æ≤¿”(No Physique Vertex)
 */

//------------------------------------
float4x4 world					: World;
float4x4 viewProj				: SHADOW_NOWORLD_ALL;

//------------------------------------
struct vertexInput0 {
    float3 position				: POSITION;
    float4 matWorld1 : TEXCOORD1;
    float4 matWorld2 : TEXCOORD2;
    float4 matWorld3 : TEXCOORD3;
    float4 matWorld4 : TEXCOORD4;
};

struct vertexOutput {
    float4 hPosition			: POSITION;
    float4 TexCoord				: TEXCOORD0;
};
//------------------------------------
vertexOutput VS_TransformAndTexture_0(vertexInput0 IN) 
{
    vertexOutput OUT;

	float4x4 matWorld;
	matWorld._11_12_13_14 = IN.matWorld1;
	matWorld._21_22_23_24 = IN.matWorld2;
	matWorld._31_32_33_34 = IN.matWorld3;
	matWorld._41_42_43_44 = IN.matWorld4;
	
	float4x4 InstanceMat = mul(world, matWorld);	
	float4 f4WorldPos = mul( float4(IN.position.xyz , 1.0) , InstanceMat);    
    OUT.hPosition = mul(f4WorldPos , viewProj);
    OUT.TexCoord = float4( OUT.hPosition.zzz, OUT.hPosition.w);

    return OUT;
}

//-----------------------------------
float4 PS_Textured( vertexOutput IN): COLOR
{
    float depth = IN.TexCoord.b / IN.TexCoord.a;
    return float4(depth.xxx, 1.0);
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