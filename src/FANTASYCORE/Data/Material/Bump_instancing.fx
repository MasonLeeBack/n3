/*
 *	Diffuse Specular, Directional light, Per-Pixel NormalMap
 */

shared float4x4 view							: View;
shared float4x4 viewProj						: ViewProjection;
shared float4	vecRcpWorldSize					: RcpWorldSize;

shared float4 fogFactor						: FOGFACTOR;	// fogEnd, forRange, ?, ?
shared float4 fogColor						: FOGCOLOR;		// r,g,b,1

// Instancing Verts count
float	Verts_Per_Instance		: VERTS_PER_INSTANCE;

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
> = {0.588f, 0.588f, 0.588f, 1.0f};

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

float SpecExpon : SpecularPower
<
    string UIWidget = "slider";
    float UIMin = 1.0;
    float UIMax = 128.0;
    float UIStep = 1.0;
    string UIName =  "specular power";
> = 8.0;

float Bumpy : BumpPower
<
    string UIWidget = "slider";
    float UIMin = 0.0;
    float UIMax = 10.0;
    float UIStep = 0.1;
    string UIName =  "bump power";
> = 3.0;

//////////
texture diffuseTexture : Diffuse
<
	string ResourceName = "default_color.dds";
>;

texture specularTexture : Specular
<
	string ResourceName = "default_color.dds";
>;

texture normalTexture : NORMAL
<
    string ResourceName = "default_bump_normal.dds";
    string ResourceType = "2D";
>;

texture lightmapTexture : WORLDLIGHTMAP
<
	string ResourceName = "default_color.dds";
>;

sampler2D diffuseSampler = sampler_state
{
	Texture = <diffuseTexture>;
};

sampler2D specularSampler = sampler_state
{
	Texture = <specularTexture>;
	AddressU  = WRAP;        
	AddressV  = WRAP;
};

sampler2D normalSampler = sampler_state
{
	Texture = <normalTexture>;
	AddressU  = WRAP;        
	AddressV  = WRAP;
};

sampler lightmapSampler = sampler_state
{
    texture = <lightmapTexture>;
    MINFILTER = LINEAR;
    MAGFILTER = LINEAR;
};

/************* DATA STRUCTS **************/

/* data from application vertex buffer */
struct appdata0 {
    float3 Position	: POSITION;
    float4 Normal	: NORMAL;
    float2 UV		: TEXCOORD0;
    float4 Tangent	: TANGENT0;
    float4 Binormal	: BINORMAL0;
};

/* data passed from vertex shader to pixel shader */
struct vertexOutput {
    float4 HPosition		: POSITION;
    float2 TexCoord		: TEXCOORD0;
    float3 LightVec		: TEXCOORD1;
    float3 ViewNormal	: TEXCOORD2;
    float3 ViewEyeVec	: TEXCOORD3;
    float3 ViewTangent	: TEXCOORD4;
    float3 ViewBinorm	: TEXCOORD5;
    float2 lightmapCoord: TEXCOORD6;
    float4  FogValue		: COLOR1;
};

/*********** vertex shader ******/

vertexOutput mainVS_0(int i : INDEX)
{
	vertexOutput OUT;
	
	float4 ObjPos;
    float4 Normal;
    float4 Texcoord;
    float4 Tangent;
    float4 Binormal;
	int Instance =  (i + 0.5f) / (int)Verts_Per_Instance;
	int Index = i - Instance * (int)Verts_Per_Instance;

	float4 InstanceMat1;
	float4 InstanceMat2;
	float4 InstanceMat3;
	float4 InstanceMat4;
	float4x4 InstanceMat;

	asm {
			vfetch ObjPos, Index, position0
			vfetch Normal, Index, normal0
			vfetch Texcoord, Index, texcoord0
			vfetch Tangent, Index, tangent0
			vfetch Binormal, Index, binormal0
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
	
	float4x4 worldView = mul( InstanceMat, view);
	
	OUT.ViewNormal = mul(Normal, worldView).xyz;
	OUT.ViewTangent = mul(Tangent, worldView).xyz;
	OUT.ViewBinorm = mul(Binormal, worldView).xyz;
	float4 tempPos = float4(ObjPos.x, ObjPos.y, ObjPos.z,1.0);
	float3 ViewSpacePos = mul(tempPos, worldView).xyz;
	OUT.LightVec = -viewlightDir;
	OUT.TexCoord.x = Texcoord.x;
	OUT.TexCoord.y = Texcoord.y;
	// Normalize.
	OUT.ViewEyeVec = -ViewSpacePos;
	OUT.HPosition = mul(f4WorldPos, viewProj);
	
	// -FOG-
	float fDist = fogFactor.x - ViewSpacePos.z; // x=fogEnd
	OUT.FogValue = clamp ( fDist*fogFactor.y, 0.0f, 1.0f ); // y=forRange
	
	return OUT;
}

/********* pixel shader ********/

float3 ApplyWorldLightMap(float4 fLightMap)
{
	float3 fTemp;
	fTemp = fLightMap.xyz * 0.7f + 0.3f;
	return fTemp;
}

float4 mainPS(vertexOutput IN) : COLOR
{
	float3 bumps = Bumpy * (tex2D(normalSampler,IN.TexCoord.xy).xyz-(0.5).xxx);
	float4 spec = tex2D(specularSampler, IN.TexCoord.xy);
	float4 LightColor = tex2D(lightmapSampler, IN.lightmapCoord);

	float3 Ln = normalize(IN.LightVec);
	float3 Nn = normalize(IN.ViewNormal);
	float3 Tn = normalize(IN.ViewTangent);
	float3 Bn = normalize(IN.ViewBinorm);
	float3 Nb = Nn + (bumps.x * Tn + bumps.y * Bn);
	Nb = normalize(Nb);
	float3 Vn = normalize(IN.ViewEyeVec);
	float3 Hn = normalize(Vn + Ln);
//	float4 lighting = lit(dot(Ln,Nb),dot(Hn,Nb),SpecExpon);
	float	d = max(dot(Ln, Nb), 0.f);
	float	s = pow(max(dot(Hn, Nb), 0.f), SpecExpon);
	float4 diff = tex2D(diffuseSampler, IN.TexCoord.xy);

	float4 result = (lightAmbient*materialAmbient + lightColor*materialDiffuse*d)*diff + lightSpecular*materialSpecular*spec*s;// AmbiColor + diffContrib + specContrib;
	result.xyz *= ApplyWorldLightMap(LightColor);
	result.w = materialAmbient.w*diff.w;

	// -FOG-
	result.xyz = lerp(fogColor, result, IN.FogValue.x);

	return result;
}

technique dx9textured_0
{
	pass p0	{
		VertexShader = compile vs_3_0 mainVS_0();
		PixelShader = compile ps_3_0 mainPS();
	}
}