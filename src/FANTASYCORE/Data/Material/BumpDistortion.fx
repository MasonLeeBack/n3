/*
*  Bump, Distortion, Per Pixel Lighting
*/

shared float4x4 viewInverse 					: ViewInverse;
shared float4 viewlightDir						: LightDirectionViewSpace
<
	string Object = "DirectionalLight";
    string Space = "World";
> = {1.0f, -1.0f, 1.0f, 0.0f};

float4x4 WorldViewIT							: WorldViewInverseTranspose;
float4x4 world									: World;
float4x4 worldViewProj							: WorldViewProjection;
float4x4 WorldView								: WorldView;
float4 materialAmbient							: MaterialAmbient
<
    string UIWidget = "Surface Color";
    string Space = "material";
> = { 0.4, 0.4, 0.4, 0.8f};


texture normalTexture							: volume;
texture screenTexture							: ScreenBuffer;
sampler ScreenSampler = sampler_state
{
	texture = <screenTexture>;
	MinFilter = LINEAR; 
	MagFilter = LINEAR;
	AddressU = MIRROR; 
	AddressV = MIRROR;
};
sampler normalSampler = sampler_state
{
	Texture = <normalTexture>;
	MinFilter = LINEAR; 
	MagFilter = LINEAR;
	AddressU = MIRROR; 
	AddressV = MIRROR;
	AddressW = MIRROR;
};


struct vertexInput 
{
    float3 position								: POSITION;
    float2 texCoordDiffuse						: TEXCOORD0;
    float2 texCoordDiffuse2						: TEXCOORD1;
    float3 Normal								: NORMAL;
    float3 Tangent								: TANGENT0;
    float3 Binormal								: BINORMAL0;
};

struct vertexOutput 
{
    float4 hPosition							: POSITION;
    float2 texCoordDiffuse						: TEXCOORD0;
    float2 texCoordDiffuse2						: TEXCOORD1;
    float3 E									: TEXCOORD2;
    float3 L									: TEXCOORD3;	
};






//------------------------------------
vertexOutput VS_TransformAndTexture(vertexInput IN) 
{
    vertexOutput OUT;
    
    float4 f4WorldPos = mul( float4(IN.position.xyz, 1.f), world);  
    OUT.hPosition = mul( float4(IN.position.xyz , 1.0) , worldViewProj);
    float3 viewVertPos = mul(float4(IN.position,1.f), WorldView).xyz;
    OUT.texCoordDiffuse = IN.texCoordDiffuse;
    OUT.texCoordDiffuse2 = OUT.hPosition.xyz/OUT.hPosition.w;
    OUT.texCoordDiffuse2.xy = OUT.texCoordDiffuse2.xy*float2(0.5f, -0.5f) + float2(0.5f,0.5f);
	
    float3 T = mul(IN.Tangent, WorldViewIT).xyz;
    float3 B = mul(IN.Binormal, WorldViewIT).xyz;
    float3 N = IN.Normal;
    
    float3 E = float3( viewInverse._41,viewInverse._42,viewInverse._43) - f4WorldPos.xyz;
    OUT.E.x = dot(E,  T);
    OUT.E.y = dot(E,  B);
    OUT.E.z = dot(E,  N);
    
    float3 L = mul( float3( viewlightDir.x, viewlightDir.y, viewlightDir.z), viewInverse);
    OUT.L.x = dot(L,  T);
    OUT.L.y = dot(L,  B);
    OUT.L.z = dot(L,  N);
    
    return OUT;
}
//------------------------------------






//-----------------------------------
float4 PS_Textured( vertexOutput IN): COLOR
{
	float4 col = { 1.0f, 1.0f, 1.0f, 1.0f };
	float3 N = 2.0f * tex2D( normalSampler, IN.texCoordDiffuse ).xyz - 1.0f;
	float nAlpha = tex2D( normalSampler, IN.texCoordDiffuse ).w;
	float3 L = -normalize(IN.L);
	float3 R = reflect(-normalize(IN.E), N);
	
	//float4 diffMap = tex2D( diffuseSampler, IN.texCoordDiffuse);
	float4 result = tex2D( ScreenSampler, IN.texCoordDiffuse2+ N.xy * 0.2f);
	
	float a = 1.0f;
	if(N.z <= 1.f && N.z >= 0.99f )
		a =0.f;
	
	float4 amb = { 1.0f, 1.0f, 1.0f, 1.0f };
	float4 I = col * max(0 , dot(N,L) ) * 0.3f +  pow(max( 0, dot(R,L) ), 10 );
	//result = result * 0.8f + diffMap * 0.2f + I * 0.6f * a;
	result = (result + I * 0.6f * a);
	
	result.a = nAlpha * materialAmbient.a;
	if( a == 0 )
		result.a = a;
		
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
