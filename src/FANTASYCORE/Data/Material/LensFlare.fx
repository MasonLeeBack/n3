
float4		LightColor : 	Diffuse;
float4x4	worldViewProj : WorldViewProjection;

float4		f4SrcRect; // left, top. width, height UV coord

texture InputTexture;
sampler InputSampler = sampler_state {
   texture = <InputTexture>;
    AddressU  = CLAMP;        
    AddressV  = CLAMP;
    MINFILTER = LINEAR;
    MAGFILTER = LINEAR;
};

struct VS_OUTPUT
{
    float4 Position	: POSITION;
    float2 TexCoord0	: TEXCOORD0;
};

VS_OUTPUT VS_Quad(float3 Position : POSITION, float2 TexCoord : TEXCOORD0)
{
    VS_OUTPUT OUT;
    OUT.Position = mul(float4(Position, 1.f), worldViewProj);
    OUT.TexCoord0 = TexCoord*f4SrcRect.zw+f4SrcRect.xy;
    return OUT;
}

float4 PS_DRAW(VS_OUTPUT IN) : COLOR
{
	float4 Color = LightColor;
	Color.w *= tex2D(InputSampler, IN.TexCoord0).w;
	return Color;
}


/*
float4 PS_ClearAlpha(VS_OUTPUT IN) : COLOR
{
    return tex2D(InputSampler, IN.TexCoord0);
}

float4 PS_Quad(VS_OUTPUT IN) : COLOR
{
    float4 TextureColor = tex2D(InputSampler, IN.TexCoord0);
    return f4QuadColor*TextureColor;
}
*/
technique VisibleTest
{
	pass p0
	{
		VertexShader = compile vs_2_0 VS_Quad();
		PixelShader = compile ps_2_0 PS_DRAW();
	}
}
/*
technique ClearAlpha
{
	pass p0
	{
		VertexShader = compile vs_2_0 VS_Quad();
		PixelShader = compile ps_2_0 PS_ClearAlpha();
	}
}

technique DrawQuad
{
	pass p0
	{
		VertexShader = compile vs_2_0 VS_Quad();
		PixelShader = compile ps_2_0 PS_Quad();
	}
}
*/