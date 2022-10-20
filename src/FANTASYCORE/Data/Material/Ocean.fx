/*
 *	Ocean
 */
 #include "CommonFunction.fxh"

/*
shared float4x4 view						: View;
shared float4x4 projection					: Projection;
shared float4x4 viewInverse 					: ViewInverse;
shared float4x4 viewInvTrans					: ViewInverseTranspose;
shared float4x4 viewProj					: ViewProjection;
shared float4	vecRcpWorldSize					: RcpWorldSize;
*/
float4x4 worldViewProj			: WorldViewProjection;
float4x4 WorldView					: WorldView;
float4x4 worldviewInverseTranspose  : WorldViewInverseTranspose;

float fTime : Time;

texture diffuseTexture : Diffuse
<
	string ResourceName = "default_color.dds";
>;

texture normalTexture : NORMAL
<
	string ResourceName = "default_color.dds";
>;

texture screenTexture : ScreenBuffer;

texture reflectTexture : REFLECTMAP;

//------------------------------------
struct vertexInput {
    float3 position				: POSITION;    
    float3 normal				: NORMAL;    
    float2 texCoordDiffuse : TEXCOORD0;
};

struct vertexOutput0 {
    float4 hPosition			: POSITION;    
    float3 viewDir				: TEXCOORD0;    
    float4	texCoord			: TEXCOORD1;
    float2 texCoordDiffuse : TEXCOORD2;
    float4	texCoordNoise1	: TEXCOORD3;
    float4	texCoordNoise2	: TEXCOORD4;
    float4	texCoordNoise3	: TEXCOORD5;
};

float noisePower : NoisePower
<
    string UIName = "Noise  Power";
	string UIWidget = "slider";
	float UIMin = 0.0; float UIMax = 1.0; float UIStep = 0.05;
> = 0.5;

float waterSpeed : WaterSpeed
<
    string UIName = "Wave Speed";
	string UIWidget = "slider";
	float UIMin = 0.0; float UIMax = 5.0; float UIStep = 0.1;
> = 1.0;

vertexOutput0 VS_Water(vertexInput IN) 
{
	vertexOutput0 OUT;

	float3 worldPos = IN.position.xyz;

	float fVal = frac(  (IN.position.x + IN.position.y)*0.003 + fTime*0.3*waterSpeed);
	fVal = smoothstep(0,1,fVal);
	worldPos.y += sin(fVal * 3.141592) * 10.0;

      float4 hPos = mul( float4(worldPos , 1.0) , worldViewProj); 
      OUT.hPosition = hPos;
      
      float4 viewPos = mul( float4(worldPos , 1.0) , WorldView);

	float3 Pos = mul(float4(IN.position,1.f), WorldView).xyz;	
	OUT.viewDir = normalize( -Pos); //eye vector


	 hPos.y = - hPos.y;

	OUT.texCoord.xy = (hPos.xy + hPos.w) * 0.5;
	OUT.texCoord.zw =  hPos.zw ;
	
	OUT.texCoordDiffuse = IN.texCoordDiffuse;

	OUT.texCoordNoise1 = float4( float2(IN.position.x , IN.position.z)*0.00015*0.21, 0, 1);
	OUT.texCoordNoise2 = float4( float2(IN.position.x , IN.position.z)*0.00015*6.1, 0, 1);
	OUT.texCoordNoise3 = float4( float2(IN.position.x , IN.position.z)*0.00015*0.87, 0, 1);

	

	return OUT;
}

sampler ScreenSampler = sampler_state
{
	texture = <screenTexture>;
	MipFilter = NONE; 
	MinFilter = LINEAR; 
	MagFilter = LINEAR;
	AddressU = MIRROR; 
	AddressV = MIRROR;
};

sampler TextureSampler = sampler_state 
{
    texture = <diffuseTexture>;
};

sampler normalSampler = sampler_state 
{
    texture = <normalTexture>;
    	MipFilter = NONE; 
	MinFilter = LINEAR; 
	MagFilter = LINEAR;
	AddressU = MIRROR; 
	AddressV = MIRROR;
};

sampler reflectmapSampler = sampler_state 
{
	texture = <reflectTexture>;
	MipFilter = NONE; 
	MinFilter = LINEAR; 
	MagFilter = LINEAR;
	AddressU = MIRROR; 
	AddressV = MIRROR;
};

static half fWave1 = fTime *  0.0185f  * waterSpeed;
static half fWave2 = fTime *  0.0325f  * waterSpeed;
static half fWave3 = fTime *  0.023f * waterSpeed;
static half fWave4 = fTime *  0.040f  * waterSpeed;

half3 skyColor = half3(0.7, 0.8, 0.9);

half4 PS_Water( vertexOutput0 IN): COLOR
{
	half3 noisy1 =tex2Dproj( normalSampler, IN.texCoordNoise1 + half4(fWave1, fWave2,0,0));				
	half3 noisy2 =tex2Dproj( normalSampler,  IN.texCoordNoise2 + half4(fWave3, fWave4,0,0));			
	half3 noisy3 =tex2Dproj( normalSampler,  IN.texCoordNoise3 + half4(fWave2, fWave3,0,0));
	half3 noisy4 =tex2Dproj( normalSampler,  IN.texCoordNoise3 + half4(fWave3, fWave1,0,0));
	
	float4 diffuseTexture = tex2D( TextureSampler, IN.texCoordDiffuse );
	
	half3 bump = (noisy1+noisy2+noisy3)*0.66 - 1;
	half3 bumpHigh = half3(bump.x, bump.z+15, bump.y);	
	bumpHigh = normalize( bumpHigh);

	half3 viewVec = normalize(IN.viewDir);
	half3 R = normalize(reflect( viewVec,bumpHigh));
	half fDot = 1-max(0,(dot(R, bumpHigh)));

	float fresnelFactor = 0.45;
	half fresnel = fresnelFactor * pow(fDot, 5);		
	half fdiff = max(0, dot(float3(0.577,0.577,0.577), 0.5*(noisy3+noisy4)));
	half3 texCoord = IN.texCoord.xyz/ IN.texCoord.w;	
	half fDist = texCoord.z;
	fDist = 1/(fDist*fDist*fDist*fDist);

	half4 refl = tex2D( reflectmapSampler,   texCoord  -  bumpHigh.xz * noisePower* fDist  );	
	half3 refractColor = tex2D( ScreenSampler, texCoord  - bumpHigh.xz *  noisePower* fDist  ) * 0.7 + diffuseTexture.xyz * 0.3;
	half3 reflectColor = lerp(skyColor,refl.rgb, (1-refl.a)*0.3+0.7) * half3(0.8, 0.8, 0.8);	
	half3 result = lerp(refractColor ,reflectColor,  fresnel );
	
	return half4( fdiff * result, diffuseTexture.a);
	
}

technique textured_0
{
	pass p0 
	{
		VertexShader = compile vs_3_0 VS_Water();
		PixelShader  = compile ps_3_0 PS_Water();
	}
}

