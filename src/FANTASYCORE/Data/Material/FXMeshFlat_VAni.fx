/*
 *	Diffuse, Directional Light, Per-Vertex
 *	No Animation∏∏ æ≤¿”(No Physique Vertex)
 */

//------------------------------------
float4x4 world						: World;
float4x4 worldViewProj				: WorldViewProjection;

float time : Time < string UIWidget = "none"; >;
float VOffset: morph_weight < string UIWidget = "VOffset"; > = 0.0f;

texture diffuseTexture : Diffuse
<
	string ResourceName = "default_color.dds";
>;

float4 materialAmbient : MaterialAmbient
<
    string UIWidget = "Surface Color";
    string Space = "material";
> = {1.0f,1.0f,1.0f,1.0f};

//------------------------------------
sampler TextureSampler = sampler_state 
{
    texture = <diffuseTexture>;
};

//------------------------------------
struct vertexInput0 {
    float3 position			: POSITION;
    float4 texCoordDiffuse		: TEXCOORD0;
};

struct vertexOutput {
    float4 hPosition		: POSITION;
    float4 texCoordDiffuse	: TEXCOORD0;
    float4 AmbColor			: COLOR0;
};

//------------------------------------
vertexOutput VS_TransformAndTexture_0(vertexInput0 IN) 
{
    vertexOutput OUT;

    OUT.hPosition = mul( float4(IN.position.xyz , 1.0) , worldViewProj);
    OUT.texCoordDiffuse = IN.texCoordDiffuse;
    OUT.texCoordDiffuse.y += VOffset;

	//output ambient
    OUT.AmbColor = materialAmbient;

    return OUT;
}

//-----------------------------------
float4 PS_Textured( vertexOutput IN): COLOR
{
	float4 diffuseTexture = tex2D( TextureSampler, IN.texCoordDiffuse );
	float4 result = IN.AmbColor * diffuseTexture;

	return result;
}

//-----------------------------------
technique textured_0				// No Physique
{
    pass p0 
    {		
		VertexShader = compile vs_1_1 VS_TransformAndTexture_0();
		PixelShader  = compile ps_1_1 PS_Textured();
    }
}