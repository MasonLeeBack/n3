/*
	Terrain Decal 
*/

float4x4 viewProjection				:  ViewProjectionCustom;

texture diffuseTexture : Diffuse
<
	string ResourceName = "default_color.dds";
>;

struct vertexInput {
    float3 position				: POSITION;    
    float2 texCoord			: TEXCOORD0;
    float4 color					: COLOR0;
};

struct vertexOutput {
    float4 hPosition		: POSITION;
    float2 texCoord	: TEXCOORD0; 
    float4 color			: COLOR0;
};

vertexOutput VS_TransformAndTexture(vertexInput IN)
{
    vertexOutput OUT ; 
    OUT.hPosition = mul( float4(IN.position , 1) , viewProjection); 
    OUT.texCoord = IN.texCoord;
    OUT.color = IN.color;
    return OUT;
}

sampler TextureSampler = sampler_state 
{
	texture = <diffuseTexture>;
};

float4 PS_Textured( vertexOutput IN): COLOR
{
	float4 diffuseTexture = tex2D( TextureSampler, IN.texCoord );
	float4 result = diffuseTexture *IN.color;;
	return result;
}

technique textured_0				// No Physique
{
    pass p0
    {
		VertexShader = compile vs_1_1 VS_TransformAndTexture();
		PixelShader  = compile ps_1_1 PS_Textured();
    }
}

