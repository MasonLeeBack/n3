/*
 *	Diffuse, Directional Light, Per-Vertex
 *	No Animation∏∏ æ≤¿”(No Physique Vertex)
 */

//------------------------------------
float4x4 viewProj				: ViewProjection;

// -FOG-
float4 fogFactor				    : FOGFACTOR;	// fogEnd, forRange, ?, ?
float4 fogColor						: FOGCOLOR;		// r,g,b,1

float  fRcpWorldXSize				: RcpWorldXSize;
float  fRcpWorldZSize				: RcpWorldZSize;

texture diffuseTexture : Diffuse
<
	string ResourceName = "default_color.dds";
>;

texture lightmapTexture : WORLDLIGHTMAP
<
	string ResourceName = "default_color.dds";
>;

float4 lightAmbient : LightAmbient
<
    string UIWidget = "Ambient Light Color";
    string Space = "material";
> = {0.0f, 0.0f, 0.0f, 1.0f};

float4 materialAmbient : MaterialAmbient
<
    string UIWidget = "Surface Color";
    string Space = "material";
> = { 1.f, 1.f, 1.f, 1.f};

//------------------------------------
sampler TextureSampler = sampler_state 
{
    texture = <diffuseTexture>;
};

sampler lightmapSampler = sampler_state
{
    texture = <lightmapTexture>;
    AddressU  = CLAMP;        
    AddressV  = CLAMP;
    MIPFILTER = NONE;
    MINFILTER = LINEAR;
    MAGFILTER = LINEAR;
};

//------------------------------------
struct vertexInput0 {
    float3 position			: POSITION;
    float4 texCoordDiffuse		: TEXCOORD0;
    float4 matWorld1 : TEXCOORD1;
    float4 matWorld2 : TEXCOORD2;
    float4 matWorld3 : TEXCOORD3;
    float4 matWorld4 : TEXCOORD4;
};

struct vertexOutput {
    float4 hPosition		: POSITION;
    float4 texCoordDiffuse	: TEXCOORD0;
    float2 lightmapCoord	: TEXCOORD1;
    float4 AmbColor			: COLOR0;
    float  FogValue	    	: COLOR1; // -FOG-
};

//------------------------------------
vertexOutput VS_TransformAndTexture_0(vertexInput0 IN) 
{
    vertexOutput OUT;
	
	float4x4 matWorld;
	matWorld._11_12_13_14 = IN.matWorld1;
	matWorld._21_22_23_24 = IN.matWorld2;
	matWorld._31_32_33_34 = IN.matWorld3;
	matWorld._41_42_43_44 = IN.matWorld4;
	
    float4 f4WorldPos = mul( float4(IN.position.xyz, 1.f), matWorld);
    OUT.lightmapCoord.x = f4WorldPos.x*fRcpWorldXSize;
    OUT.lightmapCoord.y = 1.f - f4WorldPos.z*fRcpWorldZSize;

    OUT.hPosition = mul(f4WorldPos , viewProj);
    OUT.texCoordDiffuse = IN.texCoordDiffuse;

	//output ambient
    OUT.AmbColor = materialAmbient * lightAmbient;
    OUT.AmbColor.w = materialAmbient.w;

	// -FOG-
	float fDist = fogFactor.x - OUT.hPosition.z; // x=fogEnd
	OUT.FogValue = clamp ( fDist*fogFactor.y, 0.0f, 1.0f ); // y=forRange
    return OUT;
}

float3 ApplyWorldLightMap(float4 fLightMap)
{
	float3 fTemp;
	fTemp = fLightMap.xyz * 0.7f + 0.3f;
	return fTemp;
}

//-----------------------------------
float4 PS_Textured( vertexOutput IN): COLOR
{
	float4 diffuseTexture = tex2D( TextureSampler, IN.texCoordDiffuse );

	float4 LightColor = tex2D(lightmapSampler, IN.lightmapCoord);

	float4 result = IN.AmbColor * diffuseTexture;
	result.xyz *= ApplyWorldLightMap(LightColor);
	
	// -FOG-
	result.xyz = lerp(fogColor, result, IN.FogValue);

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