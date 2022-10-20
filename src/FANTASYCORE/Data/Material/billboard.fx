texture Tex0 : TEXTURE;
texture lightmapTexture : WORLDLIGHTMAP;
texture ObjTex : OBJTEXTURE;
float4x4 matVP : VIEWPROJ;
float4x4 matInvV : INVERSEVIEW;
float4 vCommon : COMMON;
float4 fogFactor : FOGFACTOR;
float4 fogColor : FOGCOLOR;
float4 lightAmbient : LIGHTAMBIENT;

float  fRcpWorldXSize				: RCPWORLDXSIZE;
float  fRcpWorldZSize				: RCPWORLDZSIZE;

#define OnePoint 0.1f

sampler2D g_Sampler = sampler_state {
	Texture = <Tex0>;
};

sampler lightmapSampler = sampler_state {
    Texture = <lightmapTexture>;
};

sampler3D g_ObjSampler = sampler_state {
	Texture = <ObjTex>;
	AddressW  = CLAMP;
};

struct VS_IN {
	float3 WPos					:POSITION;
	float2 Tex					:TEXCOORD0;
	float2 LPos					:TEXCOORD1;
};

struct OBJ_VS_IN {
	float4 WPos					:POSITION;
	float2 Tex					:TEXCOORD0;
	float2 LPos					:TEXCOORD1;
	float4 Color				:COLOR0;
};

struct VS_OUT {
	float4 Pos					:POSITION;
	float2 Tex					:TEXCOORD0;
	float2 LightMapCoord		:TEXCOORD1;
	float4 Color					:COLOR0;
	float FogColor					:COLOR1;
};

struct OBJ_VS_OUT {
	float4 Pos					:POSITION;
	float3 Tex					:TEXCOORD0;
	float2 LightMapCoord		:TEXCOORD1;
	float4 Color					:COLOR0;
	float FogColor					:COLOR1;
};

float3 ApplyWorldLightMap(float4 fLightMap)
{
	float3 fTemp;
	fTemp = fLightMap.xyz * 0.7f + 0.3f;
	return fTemp;
}

VS_OUT BillboardVS( VS_IN In ) {
	VS_OUT Out = (VS_OUT)0;	

	float4 vRightVector = float4(matInvV._11, matInvV._12, matInvV._13, vCommon.w);
	float4 vUpVector = float4(matInvV._21, matInvV._22, matInvV._23, vCommon.w);

	float4 pBillboardPos;
	pBillboardPos = float4(In.WPos, vCommon.w) + (In.LPos.x * vRightVector) + (In.LPos.y * vUpVector);
	pBillboardPos.w = vCommon.w;

	Out.Tex = In.Tex;
	Out.Color = lightAmbient;

	pBillboardPos.x += In.LPos.y * sin( In.WPos.x + In.WPos.z + vCommon.x ) * OnePoint;
	pBillboardPos.z -= In.LPos.y * cos( In.WPos.x + In.WPos.z + vCommon.x ) * OnePoint;
	
	Out.Pos = mul( pBillboardPos, matVP );
	
	float fLength = Out.Pos.z;
	if ( fLength > vCommon.y )
	{
		Out.Color.a = clamp(vCommon.y / fLength, 0.0f, vCommon.w);
	}
	//------------------------------------------
	
	Out.LightMapCoord.x = pBillboardPos.x*fRcpWorldXSize;
    Out.LightMapCoord.y = vCommon.w - pBillboardPos.z*fRcpWorldZSize;

	float fDist = fogFactor.x - Out.Pos.z; // x=fogEnd
	Out.FogColor = clamp ( fDist*fogFactor.y, 0.0f, vCommon.w ); // y=forRange

	return Out;
}

struct PS_IN {
	float2 Tex					: TEXCOORD0;
	float2 LightMapCoord		: TEXCOORD1;
	float4 Color				: COLOR0;
	float FogColor				: COLOR1;
};

float4 BillboardPS( PS_IN In ) : COLOR {
	float4 PS_Result;

	float4 LightColor = tex2D(lightmapSampler, In.LightMapCoord);
	PS_Result = tex2D(g_Sampler, In.Tex) * In.Color;
	PS_Result.xyz *= ApplyWorldLightMap(LightColor);
	PS_Result.xyz = lerp(fogColor, PS_Result, In.FogColor);

	return PS_Result;
}

OBJ_VS_OUT ObjBillboardVS( OBJ_VS_IN In ) {
	OBJ_VS_OUT Out = (OBJ_VS_OUT)0;	

	float4 vRightVector = float4(matInvV._11, matInvV._12, matInvV._13, vCommon.w);
	float4 vUpVector = float4(matInvV._21, matInvV._22, matInvV._23, vCommon.w);

	float4 pBillboardPos;
	pBillboardPos = float4(In.WPos.xyz, vCommon.w) + (In.LPos.x * vRightVector) + (In.LPos.y * vUpVector);
	pBillboardPos.w = vCommon.w;
	Out.Pos = mul( pBillboardPos, matVP );

	Out.Tex.xy = In.Tex;
	Out.Tex.z = In.WPos.w;
	Out.Color = lightAmbient;
	Out.LightMapCoord.x = pBillboardPos.x*fRcpWorldXSize;
    Out.LightMapCoord.y = vCommon.w - pBillboardPos.z*fRcpWorldZSize;

	float fDist = fogFactor.x - Out.Pos.z; // x=fogEnd
	Out.FogColor = clamp ( fDist*fogFactor.y, 0.0f, vCommon.w ); // y=forRange

	return Out;
}

struct OBJ_PS_IN {
	float3 Tex					: TEXCOORD0;
	float2 LightMapCoord		: TEXCOORD1;
	float4 Color				: COLOR0;
	float FogColor				: COLOR1;
};

float4 ObjBillboardPS( OBJ_PS_IN In ) : COLOR {
	float4 PS_Result;

	float4 LightColor = tex2D(lightmapSampler, In.LightMapCoord);
	PS_Result = tex3D(g_ObjSampler, In.Tex) * In.Color;
	PS_Result.xyz *= ApplyWorldLightMap(LightColor);
	PS_Result.xyz = lerp(fogColor, PS_Result, In.FogColor);

	return PS_Result;
}

technique Billboard {
	pass P0 {
		VertexShader = compile vs_1_1 BillboardVS();
		PixelShader = compile ps_2_0 BillboardPS();
	}
}

technique ObjBillboard {
	pass P0 {
		VertexShader = compile vs_1_1 ObjBillboardVS();
		PixelShader = compile ps_2_0 ObjBillboardPS();
	}
}