

shared float4 viewlightDir				: LightDirectionViewSpace
<
	string Object = "DirectionalLight";
    string Space = "World";
> = {1.0f, -1.0f, 1.0f, 0.0f};

float4x4 world							: World;
float4x4 worldViewProj					: WorldViewProjection;
float4x4 WorldView						: WorldView;
float fWvalue							: fx_custom0
 = 0.5f;
float4 materialAmbient					: MaterialAmbient
<
    string UIWidget = "Surface Color";
    string Space = "material";
> = { 0.4, 0.4, 0.4, 0.8f};

texture normalTexture					: volume;
texture screenTexture					: ScreenBuffer;

sampler ScreenSampler = sampler_state
{
	texture = <screenTexture>;
	MinFilter = LINEAR; 
	MagFilter = LINEAR;
	AddressU = MIRROR; 
	AddressV = MIRROR;
};

sampler3D normalSampler = sampler_state
{
	Texture = <normalTexture>;
	MinFilter = LINEAR; 
	MagFilter = LINEAR;
	AddressU = MIRROR; 
	AddressV = MIRROR;
	AddressW = MIRROR;
};



struct vertexInput {
    float3 position						: POSITION;
    float2 texCoordDiffuse				: TEXCOORD0;
    float2 texCoordDiffuse2				: TEXCOORD1;
};

struct vertexOutput {
    float4 hPosition					: POSITION;
    float3 texCoordDiffuse				: TEXCOORD0;
    float2 texCoordDiffuse2				: TEXCOORD1;
};






//------------------------------------
vertexOutput VS_TransformAndTexture(vertexInput IN) 
{
    vertexOutput OUT;
    
    float4 f4WorldPos		= mul( float4(IN.position.xyz, 1.f), world);  
    OUT.hPosition			= mul( float4(IN.position.xyz , 1.0) , worldViewProj);
    float3 viewVertPos		= mul(float4(IN.position,1.f), WorldView).xyz;
    OUT.texCoordDiffuse		= float3(IN.texCoordDiffuse,0);
    OUT.texCoordDiffuse.z	= fWvalue;
    OUT.texCoordDiffuse2	= OUT.texCoordDiffuse;

    return OUT;
}
//------------------------------------






//-----------------------------------
float4 PS_Textured( vertexOutput IN): COLOR
{
	float4 col		= { 0.5f, 0.5f, 0.5f, 1.0f };
	float4 offset	= tex3D( normalSampler, IN.texCoordDiffuse );
	offset.xyz		= (offset.xyz-(0.5).xxx) *2.f;
	float4 result	= tex2D( ScreenSampler, IN.texCoordDiffuse2+ offset.xy*0.2f);
	if(offset.z <= 1.f && offset.z >= 0.99f )
		offset.a =0.f;
	result.w		= offset.a * materialAmbient.a;
	float3 N		= mul(float3( offset.x, offset.y, offset.z),WorldView);
	float3 L		= float3( viewlightDir.x, viewlightDir.y, viewlightDir.z);
	float4 I		= col * max(0 , dot(N,L) );
	result			= result  + I * result.w * 0.3f;	
	return result; 
}
//-----------------------------------






technique textured_0				
{
    pass p0 
    {		
		VertexShader = compile vs_1_1 VS_TransformAndTexture();
		PixelShader  = compile ps_2_0 PS_Textured();
    }
}




/*
//shared float4x4 view							: View;
//shared float4x4 projection					: Projection;
//shared float4x4 viewInverse 					: ViewInverse;
//shared float4x4 viewInvTrans					: ViewInverseTranspose;
//shared float4x4 viewProj						: ViewProjection;
//shared float4	vecRcpWorldSize					: RcpWorldSize;
//shared float4 fogFactor						: FOGFACTOR;	// fogEnd, forRange, ?, ?
//shared float4 fogColor						: FOGCOLOR;		// r,g,b,1
//shared float4 viewlightDir					: LightDirectionViewSpace
//shared float4 lightColor						: LightDiffuse
//shared float4 lightAmbient					: LightAmbient
//shared float4 lightSpecular					: LightSpecular
*/