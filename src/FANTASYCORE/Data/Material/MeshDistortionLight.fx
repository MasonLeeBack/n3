
shared float4x4 viewInverse 					: ViewInverse;
shared float4 viewlightDir						: LightDirectionViewSpace
<
	string Object = "DirectionalLight";
    string Space = "World";
> = {1.0f, -1.0f, 1.0f, 0.0f};
/////////////////////////////////////////////////////////////////////// shared


float4x4 world									: World;
float4x4 worldViewProj							: WorldViewProjection;
float4 materialAmbient							: MaterialAmbient
<
    string UIWidget = "Surface Color";
    string Space = "material";
> = { 0.4, 0.4, 0.4, 0.8f};
/////////////////////////////////////////////////////////////////////// float


texture diffuseTexture							: Diffuse
<
	string ResourceName = "default_color.dds";
>;
texture screenTexture							: ScreenBuffer;
/////////////////////////////////////////////////////////////////////// texture


sampler diffuseSampler = sampler_state
{
	Texture = <diffuseTexture>;
};
sampler ScreenSampler = sampler_state
{
	texture = <screenTexture>;
	MinFilter = LINEAR; 
	MagFilter = LINEAR;
	AddressU = CLAMP; 
	AddressV = CLAMP;
};
/////////////////////////////////////////////////////////////////////// sampler


struct vertexInput {
    float3 position				: POSITION;
    float2 texCoordDiffuse		: TEXCOORD0;
    float3 Normal	: NORMAL;
};
struct vertexOutput {
    float4 hPosition			: POSITION;
    float2 texCoordDiffuse	: TEXCOORD0;
    float3 Nor					: TEXCOORD1;
    float3 L					: TEXCOORD2;
    float3 E					: TEXCOORD3;
    float3 BackTexCoord	: TEXCOORD4;
};
/////////////////////////////////////////////////////////////////////// define





//------------------------------------
vertexOutput VS_TransformAndTexture(vertexInput IN) 
{
    vertexOutput OUT;
    
    float4 f4WorldPos = mul( float4(IN.position.xyz, 1.f), world);  
    OUT.hPosition = mul( float4(IN.position.xyz , 1.0) , worldViewProj);
    OUT.texCoordDiffuse = IN.texCoordDiffuse;
    OUT.Nor = mul(IN.Normal, world) ;
    float3 L = mul( float3( viewlightDir.x, viewlightDir.y, viewlightDir.z), viewInverse);
    OUT.L = L;
    float3 E = float3( viewInverse._41,viewInverse._42,viewInverse._43) - f4WorldPos.xyz;
    OUT.E = E;  
    OUT.BackTexCoord = OUT.hPosition.xyz/OUT.hPosition.w;
    OUT.BackTexCoord.xy = OUT.BackTexCoord.xy*float2(0.5f, -0.5f) + float2(0.5f,0.5f);
    return OUT;
}
//------------------------------------






//-----------------------------------
float4 PS_Textured( vertexOutput IN): COLOR
{
	float4 col = { 1.0f, 1.0f, 1.0f, 1.0f };
	float3 N = normalize(IN.Nor);
	float3 L = -normalize(IN.L);
	float3 R = reflect(-normalize(IN.E), N);
	float3 tex = IN.BackTexCoord;
	
	float4 result = tex2D( ScreenSampler, tex + N.xy * 0.2f);
	float4 diff = tex2D( diffuseSampler, IN.texCoordDiffuse);
	
	float4 I = col * max(0 , dot(N,L) ) * 0.3f +  pow(max( 0, dot(R,L) ), 15 );
	result = diff * 0.02f + result*0.96f + I * 0.38f;
	
	result.a = diff.a * materialAmbient.a;
				
	return result;// 
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
