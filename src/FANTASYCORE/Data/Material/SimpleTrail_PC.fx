string description = "Sword Trail";

//------------------------------------
float4x4 ViewProj : ViewProjection;
//float4x4 world   : World;
//float4x4 worldInverseTranspose : WorldInverseTranspose;
//float4x4 viewInverse : ViewInverse;

texture normalTexture : Diffuse1
<
	string ResourceName = "default_color.dds";
>;

texture envTexture:Diffuse2;
texture diffTexture:Diffuse3;

float curTime : elapsedtime
<
	string UIWidget = "slider";
	float  UIMin = 0.0;
	float  UIMax = 10.0;
	float  UIStep = 0.01;
	string UIName = "Current Time";
> = 0.0;

float life : unitsscale
<
	string UIWidget = "slider";
	float  UIMin = 0.0;
	float  UIMax = 3.0;
	float  UIStep = 0.01;
	string UIName = "life";
> = 0.0;


float4 fColor : Diffuse0;
//------------------------------------
struct vertexInput {
    float3 position				: POSITION;
//  float3 diffColor				: COLOR0;
    float2  texCoordDiffuse		: TEXCOORD;
};

struct vertexOutput {
    float4 hPosition		: POSITION;
    float3 texCoord	: TEXCOORD0;
    float2 texCoordEnv	: TEXCOORD1;
//    float4 diffColor		: TEXCOORD2;
};

float lifeRsq = 1.0;

//------------------------------------
vertexOutput VS_TransformAndTexture(vertexInput IN) 
{
    vertexOutput OUT;
    float passTime = (curTime - IN.texCoordDiffuse.x);
    		
	float fAlpha = 1.f;
/*	
	fAlpha *= step( passTime, life);
	fAlpha *= step( 0.f, passTime);
	passTime = clamp(passTime, 0.f, life);
/*/	
	if(passTime > life)
	{
		passTime = life;
		fAlpha = 0.f;
	}
	else if( passTime < 0.f)
	{
		passTime = 0.f;
		fAlpha = 0.f;
	}
//*/	
		
	passTime = passTime*lifeRsq;

	OUT.hPosition = mul( float4(IN.position.xyz , 1.0) , ViewProj);
	OUT.texCoordEnv.x = (OUT.hPosition.x/OUT.hPosition.w)*0.5+0.5f;
	OUT.texCoordEnv.y = (-OUT.hPosition.y/OUT.hPosition.w)*0.5+0.5f;
	OUT.texCoord.x = passTime;
	OUT.texCoord.y = IN.texCoordDiffuse.y;
//	OUT.diffColor = float4( IN.diffColor , fAlpha);
	OUT.texCoord.z = fAlpha;
//	OUT.diffColor.a = (1.0 - passTime);
	
    return OUT;
}


//------------------------------------
sampler normalSampler = sampler_state 
{
    texture = <normalTexture>;
	    AddressU  = WRAP;        
		AddressV  = WRAP;
//		AddressW  = CLAMP;
    MIPFILTER = LINEAR;
    MINFILTER = LINEAR;
    MAGFILTER = LINEAR;
};

sampler envSampler = sampler_state 
{
    texture = <envTexture>;
    AddressU  = MIRROR;        
    AddressV  = MIRROR;
    AddressW  = MIRROR;
    MIPFILTER = LINEAR;
    MINFILTER = LINEAR;
    MAGFILTER = LINEAR;
};


sampler diffSampler = sampler_state 
{
    texture = <diffTexture>;
    AddressU  = MIRROR;        
    AddressV  = MIRROR;
    AddressW  = CLAMP;
    MIPFILTER = LINEAR;
    MINFILTER = LINEAR;
    MAGFILTER = LINEAR;
};


//-----------------------------------
float4 PS_Textured( vertexOutput IN): COLOR
{
 	float4 diffTexture = tex2D( diffSampler, IN.texCoord );
 	float4 envTexture = tex2D( envSampler, IN.texCoordEnv );
 
 	
 	float4 result = envTexture*0.9+fColor;
	result.a *=IN.texCoord.z;
	return  result; 
}

float4 PS_DiffuseTextured( vertexOutput IN): COLOR
{
   	float4 fRet = tex2D( diffSampler, IN.texCoord);
 //  	fRet *= float4(fColor.x, fColor.y, fColor.z, 1.f);
   //	fRet.a *= fRet.r*2.f* IN.texCoordMask.z;
 	return fRet ;
}


float4 PS_NormalMapTextured( vertexOutput IN): COLOR
{
   	float4 normal = tex2D( normalSampler, IN.texCoord );
   	float4 diffuse = tex2D( diffSampler, IN.texCoord );

   	normal.x -= 0.5f;
   	normal.y -= 0.5f;
   	normal *= 0.1f;

	float4 envTexture = tex2D( envSampler, IN.texCoordEnv+normal.xy);//*(float4(1.f, 1.f, 1.f, 1.f ) - diffuse);
//	float4 nega = float4(1.f, 1.f, 1.f, 1.f ) - tex2D( envSampler, IN.texCoordEnv );
//	envTexture *= float4(1.f, 1.f, 1.f, 1.f ) - diffuse;
//	envTexture += nega*diffuse*2.f;

//	envTexture += diffuse*(1.f- diffuse.w);
//	envTexture += fColor*diffuse.w;
	envTexture += diffuse;
	envTexture.a = IN.texCoord.z;
 	return envTexture;
}

//-----------------------------------
technique textured
{

    pass p0 
    {		 
//  		AlphaBlendEnable =true;
//		SrcBlend = SRCALPHA;
//  		DestBlend = INVSRCALPHA;  		
		VertexShader = compile vs_3_0 VS_TransformAndTexture();
		PixelShader  = compile ps_3_0 PS_NormalMapTextured();
    }
}