/*
 *	Diffuse, Directional Light, Per-Vertex
 */

//------------------------------------
float4x4 world						: World;
float4x4 worldViewProj				: WorldViewProjection;
float4x4 worldviewInverseTranspose	: WorldViewInverseTranspose;
//float4x4 viewInverse				: ViewInverse;
float4x4 WorldView					: WorldView;

float4x4 shadowAll				    : SHADOW_ALL;
float fShadowMapTexelSize = 1.f/1024.f;
float fShadowMapSize = 1024.f;

//float4x4 projection					: Projection;
//float4x3 matBone[50]				: WORLDMATRIXARRAY;

// -FOG-
shared float4 fogFactor				    : FOGFACTOR;	// fogEnd, forRange, ?, ?
shared float4 fogColor						: FOGCOLOR;		// r,g,b,1

float  fRcpWorldXSize				: RcpWorldXSize;
float  fRcpWorldZSize				: RcpWorldZSize;

float fTime : TIME 
<
    string UIWidget = "slider";
    float UIMin = 0.0;
    float UIMax = 50.0;
    float UIStep = 1.0;
    string UIName = "time";
> = 00.0;



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

texture noiseTexture : Diffuse1
<
	string ResourceName = "noise.dds";
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




float fP0 = 1.5f;
float fP1=  0.5f;

float fLength = 2000.f;
float fRcpLength = 0.0005f;
float fT = 0.3f;
float fInvT = 0.7;
//float fLife = 3.f;
//float fRcpLife = 0.3333f;

float fHeight0  : fx_custom0;
float fHeight1	: fx_custom1;
float fHeight2	: fx_custom2;
float fLerp		: fx_custom3;   
// = {1.0f, 0.2f,  0.5f, 0.0f}; // y1, y2, y3,
float4 fCustomEx = {1/0.6f, 1/(1-0.6f), 0.6f, 0.f};
//------------------------------------
vertexOutput VS_TransformAndTexture_0(vertexInput0 IN) 
{
    vertexOutput OUT;
    float3 Pos = IN.position.xyz;
    

	float fLength = length(Pos);
	float fRcpLength = 1.f/fLength;
	float fLengthXZ = length(Pos.xz);

	//float fNorXZ = fLengthXZ/fLength;
	float fNorXZ = fLengthXZ*fRcpLength;
//		fTime %= fLife;
	//float fNorLife =fTime/fLife;
	
	float fGetY;
	if(fNorXZ < fCustomEx.z)
	{
		fGetY = cos(3.14*fNorXZ/**fCustomEx.x*/)+1.f;
		fGetY = fGetY*0.5f*(fHeight0-fHeight1)+fHeight1;
	}
	else
	{
		float fcurX = fNorXZ - fCustomEx.z;
		fGetY = cos( 6.28*fcurX*fCustomEx.y + 3.14 )+1.f;
		fGetY = fGetY*0.5f*(fHeight2-fHeight1);
		fGetY += fHeight1* (1.f- fcurX*fCustomEx.y);

	}

// float fP =lerp(fP0, fP1, sin( clamp( fNorLife, 0.f, 0.5f)*4.f -1.f) *0.5f+0.5f ); // To APP
/*		float fGenY = 0;
	if(fNorXZ <= fT)
	{
    		fGenY += (cos( 3.145* (fNorXZ*fP/fT) )+ 1.f-fNorXZ )*fLength*(2.f-fP)*0.5f;
	}
	else
	{
		fGenY += (cos( 3.145* ( (fNorXZ-fT)*(2.5f-fP)/fInvT +fP) ) + 1.f-fNorXZ )*fLength*fP*0.3f;
	}*/
	
	fGetY = fGetY*fLength*sign(Pos.y);
	Pos.y = lerp( fGetY, Pos.y, fLerp);	
//		Pos.y *= fScale;
//  	Pos.y *= fScale;
   	
    	
    float4 f4WorldPos = mul( float4(IN.position.xyz, 1.f), world);
    
    OUT.lightmapCoord.x = f4WorldPos.x*fRcpWorldXSize;
    OUT.lightmapCoord.y = 1.f - f4WorldPos.z*fRcpWorldZSize;
    OUT.texCoordDiffuse = IN.texCoordDiffuse;

	//calculate our vectors N, E, L, and H
	float3 N = mul(IN.normal, worldviewInverseTranspose); //normal vector
     float3 L = ( -viewlightDir.xyz); //light vector
     N = normalize(N);
    
    	//calculate the diffuse
    float  diff = max(0 , dot(N,L));

	//output diffuse
    float4 ambColor = materialAmbient * lightAmbient;
    float4 diffColor = materialDiffuse * diff * lightColor ;
    OUT.diffAmbColor = diffColor + ambColor;
    OUT.diffAmbColor.w = materialAmbient.w;
    
     // Look dot Normal
     float4 PosViewSpace = mul( float4(IN.position, 1.f) , WorldView);
     float3 E = normalize( -PosViewSpace.xyz); //eye vector
	
	float fEyeNDot = dot(N,E);
	float fstep = smoothstep( -0.2f, 0.1f, fEyeNDot);
//	OUT.texCoordDiffuse.z = fEyeNDot;
//	float3 fN = IN.normal*fstep;
//	Pos = Pos - fN*length(IN.position)*0.1f;
	
//	OUT.texCoordDiffuse.z =  1.f-OUT.texCoordDiffuse.w;
//	fEyeNDot += 0.2;
//	fEyeNDot /= 1.2;
		
	OUT.hPosition = mul( float4(Pos, 1.0f) , worldViewProj);
	
//	OUT.texCoordDiffuse = IN.texCoordDiffuse;
//	OUT.texCoordDiffuse = float4( IN.position.xy, 0.f, 1.f);
	//OUT.texCoordDiffuse.z =  1.f;
	OUT.texCoordDiffuse.z =  fstep;// max(0 , fEyeNDot );
	
	// -FOG-
	float fDist = fogFactor.x - OUT.hPosition.z; // x=fogEnd
	OUT.FogValue.x = clamp ( fDist*fogFactor.y, 0.0f, 1.0f ); // y=forRange

	float4 fShadowTexCoord = mul(float4(IN.position.xyz, 1.f), shadowAll);
    OUT.FogValue.yzw = fShadowTexCoord.xyz/fShadowTexCoord.w;
	
    return OUT;
}




//------------------------------------
sampler TextureSampler = sampler_state 
{
    texture = <diffuseTexture>;
    AddressU  = MIRROR;        
    AddressV  = MIRROR;
    AddressW  = WRAP;
    MIPFILTER = LINEAR;
    MINFILTER = LINEAR;
    MAGFILTER = LINEAR;
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

sampler shadowSampler = sampler_state 
{
    texture = <shadowTexture>;
    AddressU  = CLAMP;        
    AddressV  = CLAMP;
};

sampler NoiseSampler = sampler_state 
{
    texture = <noiseTexture>;
    AddressU  = MIRROR;        
    AddressV  = MIRROR;
    AddressW  = WRAP;
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
//	fTime %= 60.f;
//	fTime /= 60.f;
	
	float3 texNoiseCoord =  IN.texCoordDiffuse;
	float 	fPNoise = 0.f;
	float3 Pos = texNoiseCoord+ tex2D( NoiseSampler, texNoiseCoord.xy)*0.1f;// + float3(0,fTime*.3,0)*0.05;

	int i;
	float val=0, l = 2.3;
	for (i = 0; i < 4; i += 1){
		fPNoise += tex2D( NoiseSampler,Pos*l+ float2(0.f, -fTime)*0.01 ).a*0.5f;
		l *= 0.3;
	}
		
//	fPNoise *= fPNoise;
	fPNoise = fPNoise;//*IN.texCoordDiffuse.z;// + IN.texCoordDiffuse.w;
//	return float4(IN.texCoordDiffuse.w, IN.texCoordDiffuse.w, IN.texCoordDiffuse.w, 1.f);
	
	float4 diffuseTexture = tex2D( TextureSampler, IN.texCoordDiffuse + float2(0.f, -fTime*0.1f) );
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
	result.a *= fPNoise;
	
//	result.x = fPNoise;
//	result.y = fPNoise;
//	result.z = fPNoise;
//	result.a = 1.f;
//	result =  float4(fPNoise, fPNoise, fPNoise, 1.f);
	return result;
}

//-----------------------------------
technique textured_0				// No Physique
{
	pass p0 
	{		
//		alphablendenable = TRUE;
//		srcblend = srcalpha;
//		destblend = invsrcalpha;
		VertexShader = compile vs_2_0 VS_TransformAndTexture_0();
		PixelShader  = compile ps_2_0 PS_Textured();
	}
}