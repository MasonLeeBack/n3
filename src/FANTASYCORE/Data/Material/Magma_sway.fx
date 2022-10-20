/*
 *	Diffuse Specular, Directional Light, Per-Vertex
 */

//------------------------------------
shared float4x4 projection			: Projection;
shared float4x4 viewInverse 					: ViewInverse;

float4x4 world						: World;
float4x4 worldViewProj				: WorldViewProjection;
float4x4 worldviewInverseTranspose  : WorldViewInverseTranspose;

float4x3 matBone[50]				: WORLDMATRIXARRAY;

float time : Time;
float fMorphWeight					: morph_weight = 80.f;

float4	 vecUVAnimationPosition		: UVANIMATION_POSITION; 
float4	 vecUVAnimationRotation		: UVANIMATION_ROTATION; 
float4	 vecUVAnimationScale		: UVANIMATION_SCALE   ; 

// -FOG-
float4x4 WorldView					: WorldView;

shared float4 fogFactor			    : FOGFACTOR; // fogEnd, forRange, ?, ?
shared float4 fogColor				: FOGCOLOR; // r,g,b,1
shared float4 vecRcpWorldSize		: RcpWorldSize;

texture diffuseTexture : Diffuse
<
	string ResourceName = "default_color.dds";
>;

texture specularTexture : Specular
<
	string ResourceName = "default_color.dds";
>;

texture alphaTexture : Specular0
<
	string ResourceName = "default_color.dds";
>;

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

//------------------------------------
struct vertexInput0 {
    float3 position				: POSITION;
    float3 normal				: NORMAL;
    float4 texCoordDiffuse		: TEXCOORD0;
};

struct vertexInput1 {
    float3 position				: POSITION;
    float3 normal				: NORMAL;
    float4 texCoordDiffuse		: TEXCOORD0;
    int    nBoneIndex			: BLENDINDICES0;
};

struct vertexOutput {
    float4 hPosition		: POSITION;
    float4 texCoordDiffuse	: TEXCOORD0;
    float4 diffAmbColor		: TEXCOORD1;
    float2 lightmapCoord	: TEXCOORD2;
    float2 texCoordDiffuse0	: TEXCOORD3;
    float4 specularColor    : COLOR0;
    float  FogValue	    	: COLOR1; // -FOG-
};

//------------------------------------
vertexOutput VS_TransformAndTexture_0(vertexInput0 IN) 
{
    vertexOutput OUT;
    
    float3 fInPos = IN.position;
    
    float fVal = frac(  (IN.position.x + IN.position.z + IN.position.y)*0.001f + time*0.6f);
    fVal = smoothstep(0,1,fVal);
//	fInPos.y += sin(fVal * 3.141592) * fMorphWeight;
	fInPos += (sin(fVal*3.141592)*fMorphWeight*IN.normal);

    float4 f4WorldPos = mul( float4(fInPos, 1.f), world);

    OUT.lightmapCoord.xy = f4WorldPos.xz*vecRcpWorldSize.xy;
    OUT.lightmapCoord.y = 1.f - OUT.lightmapCoord.y; 
    
    OUT.hPosition = mul( float4(fInPos , 1.0) , worldViewProj);

	//calculate our vectors N, E, L, and H
    float3 viewVertPos = mul(float4(IN.position,1.f), WorldView).xyz;
	float4 N = mul(IN.normal, worldviewInverseTranspose); //normal vector
	N.xyz = normalize(N.xyz);
    float3 E = normalize( -viewVertPos); //eye vector
    float3 L = ( -viewlightDir.xyz); //light vector
    float3 H = normalize(E + L); //half angle vector

	//calculate the diffuse
    float  diff = max(0 , dot(N,L));
    //calculate the spcular
    float  spec = pow( max(0 , dot(N,E) ) , shininess );
 
	//output diffuse & specular
    float4 ambColor = materialAmbient * lightAmbient;
    float4 diffColor = materialDiffuse * diff * lightColor ;
    OUT.diffAmbColor = diffColor + ambColor;
    OUT.diffAmbColor.w = materialAmbient.w;

    OUT.specularColor = lightSpecular * materialSpecular * spec;

    OUT.texCoordDiffuse = vecUVAnimationScale*IN.texCoordDiffuse.xyxy;
    OUT.texCoordDiffuse0 =IN.texCoordDiffuse.xy; 
	OUT.texCoordDiffuse += (vecUVAnimationPosition*time);

	// -FOG-
	float fDist = vecUVAnimationRotation.x*fogFactor.x - viewVertPos.z; // x=fogEnd
	OUT.FogValue = clamp( fDist*fogFactor.y, 0.0f, 1.0f ); // y=forRange
	
    return OUT;
}

vertexOutput VS_TransformAndTexture_1(vertexInput1 IN)
{
    vertexOutput OUT;
    
    float3 fInPos = IN.position;
    float fVal = frac(  (IN.position.x + IN.position.z + IN.position.y)*0.001f + time*0.6f);
    fVal = smoothstep(0,1,fVal);
	fInPos += (sin(fVal*3.141592)*fMorphWeight*IN.normal);

    float3 Pos = mul( float4(fInPos , 1.0), matBone[IN.nBoneIndex]);    
	OUT.hPosition = mul( float4(Pos,1.f) , projection);


    float4 f4WorldPos = mul( float4(Pos, 1.f), viewInverse);

    OUT.lightmapCoord.xy = f4WorldPos.xz*vecRcpWorldSize.xy;
    OUT.lightmapCoord.y = 1.f - OUT.lightmapCoord.y; 

	//calculate our vectors N, E, L, and H
    float3 viewVertPos = mul(float4(IN.position,1.f), WorldView).xyz;
	float3 N = normalize(mul(IN.normal, matBone[IN.nBoneIndex]));		//normal vector
    float3 E = normalize( -Pos); //eye vector
    float3 L = ( -viewlightDir.xyz); //light vector
    float3 H = normalize(E + L); //half angle vector

	//calculate the diffuse
    float  diff = max(0 , dot(N,L));
    //calculate the spcular
    float  spec = pow( max(0 , dot(N,E) ) , shininess );
 
	//output diffuse & specular
    float4 ambColor = materialAmbient * lightAmbient;
    float4 diffColor = materialDiffuse * diff * lightColor ;
    OUT.diffAmbColor = diffColor + ambColor;
    OUT.diffAmbColor.w = materialAmbient.w;

    OUT.specularColor = lightSpecular * materialSpecular * spec;

    OUT.texCoordDiffuse = vecUVAnimationScale*IN.texCoordDiffuse.xyxy;
    OUT.texCoordDiffuse0 =IN.texCoordDiffuse.xy; 
	OUT.texCoordDiffuse += (vecUVAnimationPosition*time);

	// -FOG-
	float fDist = vecUVAnimationRotation.x*fogFactor.x - viewVertPos.z; // x=fogEnd
	OUT.FogValue = clamp( fDist*fogFactor.y, 0.0f, 1.0f ); // y=forRange
	
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
	AddressU  = WRAP;
	AddressV  = WRAP;
};

sampler alphaSampler = sampler_state 
{
    texture = <alphaTexture>;
	AddressU  = WRAP;
	AddressV  = WRAP;
};

sampler lightmapSampler = sampler_state
{
    texture = <lightmapTexture>;
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
	float4 diffuseTexture = tex2D( TextureSampler, IN.texCoordDiffuse.xy );
	float4 glossTexture = tex2D( SpecularSampler, IN.texCoordDiffuse.zw );
	float4 LightColor = tex2D(lightmapSampler, IN.lightmapCoord);
	float  alpha = tex2D(alphaSampler, IN.texCoordDiffuse0.xy).a;

	float4 result = IN.diffAmbColor*diffuseTexture + float4(IN.specularColor.xyz*glossTexture.xyz, 0.f);
	result.xyz*=ApplyWorldLightMap(LightColor);
	result.w = alpha;

	// -FOG-
	result.xyz = lerp(fogColor, result, IN.FogValue);

	return result;
}

//-----------------------------------
//-----------------------------------
technique textured_0				// No Physique
{
    pass p0 
    {		
		VertexShader = compile vs_1_1 VS_TransformAndTexture_0();
		PixelShader  = compile ps_2_0 PS_Textured();
    }
}

technique textured_1				// 1 Bone Link
{
    pass p0 
    {		
		VertexShader = compile vs_1_1 VS_TransformAndTexture_1();
		PixelShader  = compile ps_2_0 PS_Textured();
    }
}