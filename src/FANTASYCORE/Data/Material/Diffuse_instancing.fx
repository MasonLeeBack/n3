/*
 *	Diffuse_Instancing, Directional Light, Per-Vertex
 */

//------------------------------------

shared float4x4 viewProj					: ViewProjection;
shared float4	vecRcpWorldSize					: RcpWorldSize;

float4x4 shadowAll				    : SHADOW_ALL;
float fShadowMapTexelSize = 1.f/1024.f;
float fShadowMapSize = 1024.f;

// -FOG-
shared float4 fogFactor				    : FOGFACTOR;	// fogEnd, forRange, ?, ?
shared float4 fogColor						: FOGCOLOR;		// r,g,b,1

// Instancing Verts count
float	Verts_Per_Instance		: VERTS_PER_INSTANCE;

texture diffuseTexture : Diffuse
<
	string ResourceName = "default_color.dds";
>;

texture lightmapTexture : WORLDLIGHTMAP
<
	string ResourceName = "default_color.dds";
>;

texture shadowTexture : SHADOWMAP
<
	string ResourceName = "default_color.dds";
>;

shared float4 lightDir : Direction
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
> = {0.588f, 0.588f, 0.588f, 1.0f};

float4 materialDiffuse : MaterialDiffuse
<
    string UIWidget = "Surface Color";
    string Space = "material";
> = {1.0f, 1.0f, 1.0f, 1.0f};

//------------------------------------
struct vertexInput0 {
    float3 position				: POSITION;
    float3 normal				: NORMAL;
    float4 texCoordDiffuse		: TEXCOORD0;
};

struct vertexOutput {
    float4 hPosition		: POSITION;
    float4 texCoordDiffuse	: TEXCOORD0;
    float4 diffAmbColor		: TEXCOORD1;
    float2 lightmapCoord	: TEXCOORD2;
    float4  FogValue	    	: COLOR0; // -FOG-
};
//------------------------------------
vertexOutput VS_TransformAndTexture_0(int i : INDEX) 
{
    vertexOutput OUT;
    
    float4 ObjPos;
    float4 Normal;
    float4 Texcoord;
	
	int Instance =  (i + 0.5f) / Verts_Per_Instance;
	int Index = i - Instance * Verts_Per_Instance;

	float4 InstanceMat1;
	float4 InstanceMat2;
	float4 InstanceMat3;
	float4 InstanceMat4;
	float4x4 InstanceMat;

	asm {
			vfetch ObjPos, Index, position0
			vfetch Normal, Index, normal0
			vfetch Texcoord, Index, texcoord0
			vfetch InstanceMat1, Instance, texcoord1
			vfetch InstanceMat2, Instance, texcoord2
			vfetch InstanceMat3, Instance, texcoord3
			vfetch InstanceMat4, Instance, texcoord4
	};

	InstanceMat._11_12_13_14 = InstanceMat1;
	InstanceMat._21_22_23_24 = InstanceMat2;
	InstanceMat._31_32_33_34 = InstanceMat3;
	InstanceMat._41_42_43_44 = InstanceMat4;
	
    float4 f4WorldPos = mul( float4(ObjPos.xyz, 1.f), InstanceMat);
    
    OUT.lightmapCoord.xy = f4WorldPos.xz*vecRcpWorldSize.xy;
    OUT.lightmapCoord.y = 1.f - OUT.lightmapCoord.y; 
       
    OUT.hPosition = mul( f4WorldPos , viewProj );
    OUT.texCoordDiffuse = Texcoord;

	//calculate our vectors N, E, L, and H
	// 월드에서 라이트 계산!!	
	float4 N = mul(Normal.xyz, InstanceMat); //normal vector
	N.xyz = normalize(N.xyz);
    float3 L = ( -lightDir.xyz); //light vector

	//calculate the diffuse
    float  diff = max(0 , dot(N,L));

	//output diffuse
    float4 ambColor = materialAmbient * lightAmbient;
    float4 diffColor = materialDiffuse * diff * lightColor ;
    OUT.diffAmbColor = diffColor + ambColor;
    OUT.diffAmbColor.w = materialAmbient.w;

	// -FOG-
	float fDist = fogFactor.x - OUT.hPosition.z; // x=fogEnd
	OUT.FogValue.x = clamp ( fDist*fogFactor.y, 0.0f, 1.0f ); // y=forRange

	float4 fShadowTexCoord = mul(float4(ObjPos.xyz, 1.f), shadowAll);
    OUT.FogValue.yzw = fShadowTexCoord.xyz/fShadowTexCoord.w;
	
    return OUT;
}

//------------------------------------
sampler TextureSampler = sampler_state 
{
    texture = <diffuseTexture>;
};

sampler lightmapSampler = sampler_state
{
    texture = <lightmapTexture>;
    MIPFILTER = LINEAR;
    MINFILTER = LINEAR;
    MAGFILTER = LINEAR;
};

sampler shadowSampler = sampler_state 
{
    texture = <shadowTexture>;
    AddressU  = CLAMP;        
    AddressV  = CLAMP;
};

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

	float4 result = IN.diffAmbColor * diffuseTexture;

	//*******************************************************************	
    float  shadowTestDepth = IN.FogValue.w;
	float2 fFrac = frac(IN.FogValue.yz*fShadowMapSize);
        
    float4  shadowDepth;
    shadowDepth.x = tex2D(shadowSampler, IN.FogValue.yz).r;
    shadowDepth.y = tex2D(shadowSampler, IN.FogValue.yz + float2(fShadowMapTexelSize, 0.f)).r;
    shadowDepth.z = tex2D(shadowSampler, IN.FogValue.yz + float2(0, fShadowMapTexelSize)).r;
    shadowDepth.w = tex2D(shadowSampler, IN.FogValue.yz + float2(fShadowMapTexelSize, fShadowMapTexelSize)).r;
    
    
    float4 inLight = (shadowTestDepth < shadowDepth);

    float LightAmount = lerp( lerp( inLight[0], inLight[1], fFrac.x ),
                            lerp( inLight[2], inLight[3], fFrac.x ),
                            fFrac.y );
    if(IN.FogValue.y>=1.f || IN.FogValue.y<=0.f || IN.FogValue.z>=1.f || IN.FogValue.z<=0.f) {
		LightAmount = 1.f;
    }
    if(shadowTestDepth>=1.f) {
		LightAmount = 1.f;
    }

	LightAmount*=0.5f;
	LightAmount+=0.5f;
	//*******************************************************************	

	//*******************************************************************	
//	float fLightmap = clamp(LightColor.x * 2.f , 0.f, 1.f);
//    fLightmap *=0.8f;
//    fLightmap +=0.2f;
	result.xyz *= (ApplyWorldLightMap(LightColor)*LightAmount);
	//*******************************************************************	
	// -FOG-
	result.xyz = lerp(fogColor, result, IN.FogValue.x);

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