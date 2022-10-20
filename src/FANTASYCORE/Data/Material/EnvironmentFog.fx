float4x4 viewInverse 				: ViewInverse;
float4x4 viewProj					: ViewProjection;

float4 ParticleInfo : PARTICLEINFO = { 1.f, 1.f, 1.f, 1.f };		// x = 1.f, y = Up Offset, z = sinf(time), w=cos(time)
float4 FogColor : FOGCOLOR = { 1.f, 1.f, 1.f, 1.f};

texture FogTexture : Diffuse;

sampler FogSampler = sampler_state {
	texture = <FogTexture>;
};

struct VS_IN {
	float3 Position					: POSITION;
	float2 Offset					: TEXCOORD0;
	float2 TexCoord					: TEXCOORD1;
};

struct VS_OUT {
    float4 hPosition				: POSITION;
    float2 texCoordDiffuse			: TEXCOORD0;
};

VS_OUT FogVertexShader( VS_IN IN ) {
	VS_OUT Out = (VS_OUT)0;

	float3 vecPitch	= float3(viewInverse._11, viewInverse._12, viewInverse._13);
	float3 vecYaw	= float3(viewInverse._21, viewInverse._22, viewInverse._23);
	
	// sinf, cosf제거위해 똑같은 Rotate
	float fXOffset = IN.Offset.x*ParticleInfo.w - IN.Offset.y*ParticleInfo.z;
	float fYOffset = IN.Offset.x*ParticleInfo.z + IN.Offset.y*ParticleInfo.w;
	
	float3 vecWorldPos = IN.Position + fXOffset*vecPitch + fYOffset*vecYaw;
	vecWorldPos.y += ParticleInfo.y;
	
	Out.hPosition = mul(float4(vecWorldPos, 1.f), viewProj);
	Out.texCoordDiffuse = IN.TexCoord;
	
	return Out;
}

float4 FogPixelShader( VS_OUT IN): COLOR
{
	return FogColor*tex2D( FogSampler, IN.texCoordDiffuse );
}

technique EnvironmentFog {
	pass P0 {
		VertexShader = compile vs_3_0 FogVertexShader();
		PixelShader = compile ps_3_0 FogPixelShader();
	}
}