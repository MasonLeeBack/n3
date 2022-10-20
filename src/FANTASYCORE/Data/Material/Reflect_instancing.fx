/*
 *	Diffuse Specular, Directional Light, Per-Vertex
 */

//------------------------------------

shared float4x4 view						: View;
shared float4x4 viewInverse 					: ViewInverse;
shared float4x4 projection					: Projection;
shared float4	vecRcpWorldSize					: RcpWorldSize;

// -FOG-
shared float4 fogFactor				    : FOGFACTOR; // fogEnd, forRange, ?, ?
shared float4 fogColor						: FOGCOLOR; // r,g,b,1

// Instancing Verts count
float	Verts_Per_Instance		: VERTS_PER_INSTANCE;

texture diffuseTexture : Diffuse
<
	string ResourceName = "default_color.dds";
>;

texture specularTexture : Specular
<
	string ResourceName = "default_color.dds";
>;

texture envTexture : Environment;

texture lightmapTexture : WORLDLIGHTMAP
<
	string ResourceName = "default_color.dds";
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

shared float4 lightSpecular : LightSpecular
<
    string UIWidget = "Ambient Light Color";
    string Space = "material";
> = {1.f, 1.f, 1.0f, 1.f};

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

float4 materialSpecular : MaterialSpecular
<
	string UIWidget = "Surface Specular";
	string Space = "material";
> = {1.0f, 1.0f, 1.0f, 1.0f};

float shininess : SpecularPower
<
    string UIWidget = "slider";
    float UIMin = 1.0;
    float UIMax = 128.0;
    float UIStep = 1.0;
    string UIName = "specular power";
> = 15.0;

float reflectpower : ReflectPower
<
    string UIWidget = "slider";
    float UIMin = 1.0;
    float UIMax = 128.0;
    float UIStep = 1.0;
    string UIName = "reflect power";
> = 1.f;

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
    float4 specularColor    : TEXCOORD2;
    float3 worldReflect		: TEXCOORD3;
    float2 lightmapCoord	: TEXCOORD4;
    float  FogValue    	    : COLOR0; // -FOG-
};

//------------------------------------
vertexOutput VS_TransformAndTexture_0(int i : INDEX) 
{
    vertexOutput OUT;
    
    float4 ObjPos;
    float4 Normal;
    float4 Texcoord;
	int Instance =  (i + 0.5f) / (int) Verts_Per_Instance;
	int Index = i - Instance * (int) Verts_Per_Instance;
    
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

  	//calculate our vectors N, E, L, and H
    	float4x4 worldViewMat = mul(InstanceMat, view);
	
	float3 viewVertPos = mul( float4(ObjPos.xyz, 1.f), worldViewMat).xyz;
    float4 f4WorldPos = mul( float4(viewVertPos, 1.f), viewInverse);
	
    OUT.hPosition = mul( float4(viewVertPos , 1.0) , projection);
  
    OUT.lightmapCoord.xy = f4WorldPos.xz*vecRcpWorldSize.xy;
    OUT.lightmapCoord.y = 1.f - OUT.lightmapCoord.y; 

	float3 N = (mul(Normal.xyz, worldViewMat)); //normal vector
	N = normalize(N);
    float3 E = normalize( -viewVertPos); //eye vector
    float3 L = ( -viewlightDir.xyz); //light vector
    float3 H = normalize(E + L); //half angle vector

	//calculate the diffuse
    float  diff = max(0 , dot(N,L));
    //calculate the spcular
    float  spec = pow( max(0 , dot(N,H) ) , shininess );
 
	//output diffuse & specular
    float4 ambColor = materialAmbient * lightAmbient;
    float4 diffColor = materialDiffuse * diff * lightColor ;
    OUT.diffAmbColor = diffColor + ambColor;
    OUT.diffAmbColor.w = materialAmbient.w;
    OUT.specularColor = lightSpecular * materialSpecular * spec * lightColor;

    float3 viewReflect = reflect(-E, N);
	OUT.worldReflect = mul( float4(viewReflect,0.f), viewInverse);

    OUT.texCoordDiffuse = Texcoord;

	float fDist = fogFactor.x - viewVertPos.z; // x=fogEnd
	OUT.FogValue = clamp ( fDist*fogFactor.y, 0.0f, 1.0f ); // y=forRange

    return OUT;
}

//------------------------------------
sampler TextureSampler = sampler_state 
{
    texture = <diffuseTexture>;
};

sampler SpecularSampler = sampler_state 
{
    texture = <specularTexture>;
};

samplerCUBE EnvSampler = sampler_state
{
    texture = <envTexture>;
};

sampler lightmapSampler = sampler_state
{
    texture = <lightmapTexture>;
    AddressU  = CLAMP;        
    AddressV  = CLAMP;
    MIPFILTER = LINEAR;
    MINFILTER = LINEAR;
    MAGFILTER = LINEAR;
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
	float4 glossTexture = tex2D( SpecularSampler, IN.texCoordDiffuse );
	float4 envTexture = texCUBE( EnvSampler, IN.worldReflect);
	float4 LightColor = tex2D(lightmapSampler, IN.lightmapCoord);

	//*******************************************************************	
//	LightColor*=0.8f;
//	LightColor+=0.2f;
	//*******************************************************************	
	
	float4 result = IN.diffAmbColor*diffuseTexture + float4(IN.specularColor.xyz*glossTexture.xyz*reflectpower*envTexture.xyz, 0.f);
	result.xyz*=ApplyWorldLightMap(LightColor);
	result.xyz = lerp(fogColor, result, IN.FogValue);
	return result;
}

//-----------------------------------
//-----------------------------------
technique textured_0				// No Physique
{
    pass p0 
    {		
		VertexShader = compile vs_3_0 VS_TransformAndTexture_0();
		PixelShader  = compile ps_2_0 PS_Textured();
    }
}