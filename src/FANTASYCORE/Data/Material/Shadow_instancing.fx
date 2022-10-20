/*
 *	Shadow Instancing
 *	No Animation∏∏ æ≤¿”(No Physique Vertex)
 */

//------------------------------------
#include "CommonFunction.fxh"
float4x4 viewProjForShadowInstancing				: SHADOW_NOWORLD_ALL;

// Instancing Verts count
float	Verts_Per_Instance		: VERTS_PER_INSTANCE;

//------------------------------------
struct vertexInput0 {
    float3 position				: POSITION;
};

struct vertexOutput {
    float4 hPosition			: POSITION;
    float4 TexCoord				: TEXCOORD0;
};
//------------------------------------
vertexOutput VS_TransformAndTexture_0(int i : INDEX) 
{
    vertexOutput OUT;
    
    float4 ObjPos;
	float4 InstancePos;
	int Instance =  (i + 0.5f) / (int)Verts_Per_Instance;
	int Index = i - Instance * (int)Verts_Per_Instance;
	
	float4 InstanceMat1;
	float4 InstanceMat2;
	float4 InstanceMat3;
	float4 InstanceMat4;
	float4x4 InstanceMat;

	asm {
			vfetch ObjPos, Index, position0
			vfetch InstanceMat1, Instance, texcoord1
			vfetch InstanceMat2, Instance, texcoord2
			vfetch InstanceMat3, Instance, texcoord3
			vfetch InstanceMat4, Instance, texcoord4
	};

	InstanceMat._11_12_13_14 = InstanceMat1;
	InstanceMat._21_22_23_24 = InstanceMat2;
	InstanceMat._31_32_33_34 = InstanceMat3;
	InstanceMat._41_42_43_44 = InstanceMat4;
		
	float4 f4WorldPos = mul( float4(ObjPos.xyz , 1.0) , InstanceMat);    
    OUT.hPosition = mul(f4WorldPos , viewProjForShadowInstancing);
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