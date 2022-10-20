/*
 *	Diffuse, Directional Light, Per-Vertex
 *	No Animation만 쓰임(No Physique Vertex)
 */

//------------------------------------
float4x4 world						: World;
float4x4 worldViewProj				: WorldViewProjection;
float4x4 worldviewInverseTranspose	: WorldViewInverseTranspose;

float time : Time  < string UIWidget = "none"; >;
float wPos  : morph_weight < string UIWidget = "WValue"; > = 0.0f;

texture diffuseTexture : Volume
<
	string ResourceName = "default_color.dds";
	string Resourcetype = "3d";
>;

shared float4 viewlightDir : LightDirectionViewSpace
<
	string Object = "DirectionalLight";
    string Space = "World";
> = {1.0f, -1.0f, 1.0f, 0.0f};

shared float4 lightColor : LightDiffuse
<
    string UIName = "Diffuse Light Color";
    string Object = "DirectionalLight";
> = {1.0f, 1.0f, 1.0f, 1.0f};

shared float4 lightAmbient : LightAmbient
<
    string UIWidget = "Ambient Light Color";
    string Space = "material";
> = {0.0f, 0.0f, 0.0f, 1.0f};

float4 materialAmbient : MaterialAmbient
<
    string UIWidget = "Surface Color";
    string Space = "material";
> = { 1.f, 1.f, 1.f, 1.f};

float4 materialDiffuse : MaterialDiffuse
<
    string UIWidget = "Surface Color";
    string Space = "material";
> = { 0.753f, 0.753f, 0.753f, 1.0f};

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
    float3 normal			: NORMAL;
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
    OUT.texCoordDiffuse.z = wPos;


//calculate our vectors N, E, L, and H
	// 월드에서 라이트 계산!!
	float4 N = mul(IN.normal, worldviewInverseTranspose); //normal vector
	N.xyz = normalize(N.xyz);
    float3 L = ( -viewlightDir.xyz); //light vector

	//calculate the diffuse
    float  diff = max(0 , dot(N,L));

	//output diffuse
    float4 ambColor = materialAmbient * lightAmbient;
    float4 diffColor = materialDiffuse * diff * lightColor ;
    OUT.AmbColor = diffColor + ambColor;
    OUT.AmbColor.w = materialAmbient.w;


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