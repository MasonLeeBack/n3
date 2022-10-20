

float4x4 matVP : VIEWPROJ;
float4x4 matInvV : INVERSEVIEW;
float4 vCommon : COMMON;

texture Tex0 : TEXTURE;

sampler g_Sampler = sampler_state {
	texture = <Tex0>;
    MIPFILTER = LINEAR;
    MINFILTER = LINEAR;
    MAGFILTER = LINEAR;
};

struct VS_IN {
	float4 WPos					:POSITION;
	float2 Tex					:TEXCOORD0;
	float2 LPos					:TEXCOORD1;
};

struct VS_OUT {
	float4 Pos					:POSITION;
	float2 Tex					:TEXCOORD0;
	float4 Color				:COLOR0;
};

VS_OUT DynamicFogVS( VS_IN In ) {
	VS_OUT Out = (VS_OUT)0;	

	float4 vRightVector = float4(matInvV._11, matInvV._12, matInvV._13, 1.0f);
	float4 vUpVector = float4(matInvV._21, matInvV._22, matInvV._23, 1.0f);
	float4 vEyeVector = float4(matInvV._31, matInvV._32, matInvV._33, 1.0f);

	float4 pBillboardPos;
	pBillboardPos = In.WPos + (In.LPos.x * vRightVector) + (In.LPos.y * vUpVector);
	pBillboardPos.w = 1.0f;
	
	pBillboardPos.x += sin(In.WPos.z + vCommon.x * 2.0f) * vCommon.y;
	pBillboardPos.y -= cos(In.WPos.z + vCommon.x * 2.0f) * vCommon.y;
	
	Out.Pos = mul( pBillboardPos, matVP );
	Out.Tex = In.Tex;
	Out.Color = float4(0.7f, 0.7f, 0.7f, vCommon.z);
	
	return Out;
}

struct PS_IN {
	float2 Tex					:TEXCOORD0;
	float4 Color				:COLOR0;
};

float4 DynamicFogPS( PS_IN In ) : COLOR {
	float4 PS_Result;

	PS_Result = tex2D(g_Sampler, In.Tex) * In.Color;

	return PS_Result;
}

technique DynamicFog {
	pass P0 {
		VertexShader = compile vs_3_0 DynamicFogVS();
		PixelShader = compile ps_3_0 DynamicFogPS();
	}
}