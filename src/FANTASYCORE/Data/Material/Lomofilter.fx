string description = "Screen motion blur";

//------------------------------------


//------------------------------------

texture noiseTexture : Diffuse0
<
	string ResourceName = "default_color.dds";
>;

texture tableTexture : Diffuse1
<
	string ResourceName1 = "default_color.dds";
>;

sampler noiseSampler = sampler_state 
{
    texture = <noiseTexture>;
//   AddressU  = WRAP;        
//    AddressV  = MIRROR;
//    AddressW  = MIRROR;
};


sampler tableSampler = sampler_state 
{
    texture = <tableTexture>;
//    AddressU  = CLAMP;        
//    AddressV  = CLAMP;
//    AddressW  = CLAMP;
};

struct vertexInput
{
	float2 position		: POSITION;
	float2 texCoord0	: TEXCOORD0;
};

struct vertexOutput 
{
    float4 position		: POSITION;
    float2 texCoord0		: TEXCOORD0;
    float2 texCoord1		: TEXCOORD1;
};

float  fTime;
//float4 vConst;// = { 0.0 , 0.0, 0.9, 1.0 }; //Rumble X, Y,  blend, scale,
float4 vConst;	//	float fNoiseIntensity = 0.3;
					//	float fLomoThick	= 0.95;
					//	float fNoisySmooth = 2.f;
					//	float fSceneBlendRate = 1.2f;
					
float fRandom;

float4 vColor  = {0.0f, 0.0f, 0.0f, 1.0f};


//------------------------------------
vertexOutput VS_Main(vertexInput  IN) 
{
	vertexOutput OUT;
	OUT.position = float4(IN.position, 0.0, 1.0);
	OUT.texCoord0.x =  (IN.texCoord0.x-0.5f)+0.5f;
	OUT.texCoord0.y =  (IN.texCoord0.y-0.5f)+0.5f;
	OUT.texCoord1 = IN.position;
	return OUT;
}

//-----------------------------------
float4 PS_MotionBlur( vertexOutput IN): COLOR
{

  	float 	fDot = dot( IN.texCoord1,  IN.texCoord1)*0.6;

	float2 coord;
	coord.x = IN.texCoord1.x / IN.texCoord1.y;
	coord.x = coord.x/vConst.z+fRandom*0.2f;
	coord.y = 0.1f;
	float4 noisy = tex2D(noiseSampler, coord);
	noisy.r = noisy.r*vConst.x+(1.0f-vConst.x);
   	
   	float fInvLomo = (1.f - vConst.y);
   	float fU = fRandom*noisy.r*fDot -fInvLomo;
//   	fU /= fInvLomo;
 	
 	//float4 table = tex2D( tableSampler, float2(fU, 0.f) );
 	fU = smoothstep( 0.f, 1.f, fU);
 	float4 result;
	result = float4( 0.f, 0.f, 0.f, 1.f);// lerp( float4( 0.2f, 0.2f, 0.2f, 1.f), float4( 0.f, 0.f, 0.f, 1.f), fU );
//	float fSceneThick = 1.0f - fLomoThick;
	result.a = 	max(fInvLomo, fDot) - fInvLomo+fU*0.5;
	result.a *= vConst.w;
//	result.a = 1.0 - diffuseTexture.a;
	result.a = min(result.a, 1.f);
//	result.a *= vConst.z;
//	result = table;
//	result = float4(diffuseTexture.a, diffuseTexture.a, diffuseTexture.a, 1.f );
  	return result;
}


//-----------------------------------
technique MotionBlur
{
    pass p0 
    {		
		VertexShader = compile vs_2_0 VS_Main();
		PixelShader  = compile ps_2_0 PS_MotionBlur();
    }	
}