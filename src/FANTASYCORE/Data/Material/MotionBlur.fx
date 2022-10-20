string description = "Screen motion blur";

//------------------------------------


//------------------------------------

texture noiseTexture : Diffuse0
<
	string ResourceName = "default_color.dds";
>;

texture backTexture : Diffuse1
<
	string ResourceName1 = "default_color.dds";
>;

texture tableTexture : Diffuse2
<
	string ResourceName1 = "default_color.dds";
>;

sampler noiseSampler = sampler_state 
{
    texture = <noiseTexture>;
//    AddressU  = WRAP;        
//    AddressV  = MIRROR;
//    AddressW  = MIRROR;
};

sampler backSampler = sampler_state 
{
    texture = <backTexture>;
//    AddressU  = CLAMP;        
//    AddressV  = CLAMP;
//    AddressW  = CLAMP;
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
float4 vConst;// = { 0.0 , 0.0, 0.9, 1.0 }; //Rumble X, Y,  blend, scale,
float4 vConst2;	//	float fNoiseIntensity = 0.3;
					//	float fLomoThick	= 0.95;
					//	float fNoisySmooth = 2.f;
					//	float fSceneBlendRate = 1.2f;

float4 vColor  = {0.8f, 0.6f, 0.6f,1.0f};


//------------------------------------
vertexOutput VS_Main(vertexInput  IN) 
{
	vertexOutput OUT;
	OUT.position = float4(IN.position, 0.0, 1.0);
	OUT.texCoord0.x =  (IN.texCoord0.x-0.5f)*vConst.w+0.5f+vConst.x;
	OUT.texCoord0.y =  (IN.texCoord0.y-0.5f)*vConst.w+0.5f+vConst.y;
	OUT.texCoord1 = IN.position;
	return OUT;
}

//-----------------------------------
float4 PS_MotionBlur( vertexOutput IN): COLOR
{
  	float4 	diffuseTexture= 	tex2D( backSampler, IN.texCoord0 );
  	float 	fDot = dot( IN.texCoord1,  IN.texCoord1)*0.6;

	float2 coord;
	coord.x = IN.texCoord1.x / IN.texCoord1.y;
	coord.x /= vConst2.z;
	coord.y = fTime;
	float4 noisy = tex2D(noiseSampler, coord);
	noisy.r = noisy.r*vConst2.x+(1.0f-vConst2.x);
   	
   	float fInvLomo = (1.f - vConst2.y);
   	float fU = noisy.r*fDot -fInvLomo;
   	fU /= fInvLomo;
 	
// 	float4 table = tex2D( tableSampler, float2(fU, 0.f) );
//	fU = smoothstep( 0.f, 1.f, fU);
//	diffuseTexture = lerp( diffuseTexture, table, fU );

	diffuseTexture.a = 	max(fInvLomo, fDot) - fInvLomo;
	diffuseTexture.a *= vConst2.w;
//	diffuseTexture.a = 1.0 - diffuseTexture.a;
	diffuseTexture.a = min(diffuseTexture.a, 1.f);
//	diffuseTexture.a *= vConst.z;
///	diffuseTexture = table;
//	diffuseTexture = float4(diffuseTexture.a, diffuseTexture.a, diffuseTexture.a, 1.f );
	float4 vResult = vColor * diffuseTexture;
  	return vResult;
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