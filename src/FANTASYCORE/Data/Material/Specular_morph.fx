/*
 *	Diffuse Specular, Directional Light, Per-Vertex
 */

//------------------------------------
shared float4x4 projection			: Projection;

float4x4 worldViewProj				: WorldViewProjection;
float4x4 worldviewInverseTranspose  : WorldViewInverseTranspose;

float4x3 matBone[50]				: WORLDMATRIXARRAY;		// 50 -> 256 개로 변경

// -FOG-
float4x4 WorldView					: WorldView;
shared float4 fogFactor				    : FOGFACTOR; // fogEnd, forRange, ?, ?
shared float4 fogColor						: FOGCOLOR; // r,g,b,1

float fMorphWeight					: morph_weight;


texture diffuseTexture : Diffuse
<
	string ResourceName = "default_color.dds";
>;

texture specularTexture : Specular
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
    float3 position0				: POSITION0;
    float3 position1				: POSITION1;
    float3 normal0				: NORMAL0;
    float3 normal1				: NORMAL1;
    float4 texCoordDiffuse		: TEXCOORD0;
};

struct vertexInput1 {
    float3 position0				: POSITION0;
    float3 position1				: POSITION1;
    float3 normal0				: NORMAL0;
    float3 normal1				: NORMAL1;
    float4 texCoordDiffuse		: TEXCOORD0;
    int    nBoneIndex			: BLENDINDICES0;
};

struct vertexInput2 {
    float3 position0				: POSITION0;
    float3 position1				: POSITION1;
    float3 normal0				: NORMAL0;
    float3 normal1				: NORMAL1;
    float4 texCoordDiffuse		: TEXCOORD0;
	int4	   nBoneIndex			: BLENDINDICES;
	float4	   fWeight			: BLENDWEIGHT;
};

struct vertexOutput {
    float4 hPosition		: POSITION;
    float4 texCoordDiffuse	: TEXCOORD0;
    float4 diffAmbColor		: TEXCOORD1;
    float4 specularColor    : COLOR0;
    float  FogValue	    	: COLOR1; // -FOG-
};

//------------------------------------
vertexOutput VS_TransformAndTexture_0(vertexInput0 IN) 
{
    vertexOutput OUT;
	float3 blendres = lerp(IN.position0, IN.position1, fMorphWeight);
     float3 blendnormal=lerp(IN.normal0, IN.normal1, fMorphWeight);
    blendnormal = normalize(blendnormal);  
   
    OUT.hPosition = mul( float4(blendres , 1.0) , worldViewProj);

	//calculate our vectors N, E, L, and H
    float3 viewVertPos = mul(float4(blendres ,1.f), WorldView).xyz;
	float4 N = mul(blendnormal , worldviewInverseTranspose); //normal vector
	N.xyz = normalize(N.xyz);
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

    OUT.specularColor = lightSpecular * materialSpecular * spec;

    OUT.texCoordDiffuse = IN.texCoordDiffuse;

	// -FOG-
	float fDist = fogFactor.x - viewVertPos.z; // x=fogEnd
	OUT.FogValue = clamp ( fDist*fogFactor.y, 0.0f, 1.0f ); // y=forRange
	
    return OUT;
}

vertexOutput VS_TransformAndTexture_1(vertexInput1 IN) 
{
    vertexOutput OUT;
    float3 blendres = lerp(IN.position0, IN.position1, fMorphWeight);
    float3 blendnormal=lerp(IN.normal0, IN.normal1, fMorphWeight);
    blendnormal = normalize(blendnormal);  


    float3 Pos = mul( float4(blendres , 1.0), matBone[IN.nBoneIndex]);
	OUT.hPosition = mul( float4(Pos,1.f) , projection);

	//calculate our vectors N, E, L, and H
	float3 N = normalize(mul(blendnormal , matBone[IN.nBoneIndex]));		//normal vector
    float3 E = normalize( -Pos); //eye vector
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

    OUT.specularColor = lightSpecular * materialSpecular * spec;

    OUT.texCoordDiffuse = IN.texCoordDiffuse;

	// -FOG-
	float fDist = fogFactor.x - Pos.z; // x=fogEnd
	OUT.FogValue = clamp ( fDist*fogFactor.y, 0.0f, 1.0f ); // y=forRange
	
    return OUT;
}

vertexOutput VS_TransformAndTexture_2(vertexInput2 IN) 
{
    vertexOutput OUT;
    float3 blendres = lerp(IN.position0, IN.position1, fMorphWeight);
    float3 blendnormal=lerp(IN.normal0, IN.normal1, fMorphWeight);
    blendnormal = normalize(blendnormal);  


    float3 Pos = mul( float4(blendres , 1.0), matBone[IN.nBoneIndex.x]) * IN.fWeight.x;
    Pos += mul( float4(blendres , 1.0), matBone[IN.nBoneIndex.y]) * IN.fWeight.y;
	OUT.hPosition = mul( float4(Pos,1.f) , projection);

	//calculate our vectors N, E, L, and H
	float3 N = mul(blendnormal, matBone[IN.nBoneIndex.x]) * IN.fWeight.x;		//normal vector
	N += mul(blendnormal, matBone[IN.nBoneIndex.y]) * IN.fWeight.y;
	N =normalize(N);
    float3 E = normalize( -Pos); //eye vector
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

    OUT.specularColor =lightSpecular * materialSpecular * spec;

    OUT.texCoordDiffuse = IN.texCoordDiffuse;

	// -FOG-
	float fDist = fogFactor.x - Pos.z; // x=fogEnd
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


//-----------------------------------
float4 PS_Textured( vertexOutput IN): COLOR
{
	float4 diffuseTexture = tex2D( TextureSampler, IN.texCoordDiffuse );
	float4 glossTexture = tex2D( SpecularSampler, IN.texCoordDiffuse );
	float4 result = IN.diffAmbColor*diffuseTexture + float4(IN.specularColor.xyz*glossTexture.xyz, 0.f);

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

technique textured_2				// 1 Bone Link
{
    pass p0 
    {		
		VertexShader = compile vs_1_1 VS_TransformAndTexture_2();
		PixelShader  = compile ps_2_0 PS_Textured();
    }
}