/*
 *	Diffuse, Directional Light, Per-Vertex
 *	No Animation∏∏ æ≤¿”(No Physique Vertex)
 */

//------------------------------------

float4x4 worldViewProj				: WorldViewProjection;

float time : Time  < string UIWidget = "none"; >;
float Speed : morph_weight < string UIWidget = "Speed"; > = 0.16f;

texture diffuseTexture : Volume
<
	string ResourceName = "default_color.dds";
	string Resourcetype = "3d";
>;

float4 materialAmbient : MaterialAmbient
<
    string UIWidget = "Surface Color";
    string Space = "material";
> = {1.0f,1.0f,1.0f,1.0f};

sampler3D DiffuseMap = sampler_state 
{
    texture = <diffuseTexture>;
//    AddressU  = WRAP;        
//    AddressV  = WRAP;
//    AddressW  = WRAP;
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
    OUT.texCoordDiffuse.z = Speed * time;

	//output ambient
    OUT.AmbColor = materialAmbient;

    return OUT;
} 

//-----------------------------------
float4 PS_Textured( vertexOutput IN): COLOR
{
	float4 diffuseTexture = tex3D( DiffuseMap, IN.texCoordDiffuse.xyz  );
	float4 result = IN.AmbColor * diffuseTexture;

	return result;
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