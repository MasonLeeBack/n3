string description = "Basic Vertex Lighting with a Texture";

float4x4 worldViewProj:		WorldViewProjection;
float4x4 projection:			Projection;
float4x4 view:			  	View;
float4x4 world:			  	World;


texture diffuseTexture : Diffuse0
<
	string ResourceName = "default_color.dds";
>;
//texture tableTexture : Diffuse1;
//texture noiseTexture : Diffuse2;

float fInterpolate = 0.f;
//float fTime = 0.f;




float4 lightDir : Direction
<
	string Object = "DirectionalLight";
    string Space = "World";
> = {1.0f, -1.0f, 1.0f, 0.0f};

//------------------------------------
struct vertexInput {
    float3 position0				: POSITION0;
    float3 normal0				: NORMAL0;
//  float3 tangent0				: TANGENT0;
    
    float3 position1				: POSITION1;
    float3 normal1				: NORMAL1;
//    float3 tangent1				: TANGENT1;
};

struct vertexOutputP {
    float4 Position		: POSITION;
    float4 Diffuse          : COLOR0;
    float  PSize            : PSIZE;
};


//------------------------------------
vertexOutputP VS_PointSprite(vertexInput IN) 
{
    vertexOutputP OUT;
//    float3  interPos = Hermite(IN.position0, IN.tangent0, IN.position1, IN.tangent1, fInterpolate);
     float3  interPos = lerp(IN.position0,  IN.position1, fInterpolate);
    float3  interNor = lerp(IN.normal0,  IN.normal1, fInterpolate);
  //  float4x4 mtx;
//    mtx = mul( world , view);
//    mtx = mul( mtx, projection);
    OUT.Position = mul( float4(interPos, 1.0) ,  worldViewProj);
//    interNor = mul( float4(interNor, 1.0) , world);
    
//    faceforward(interNor, CamDir, interNor);
    
    float	fDot=	dot(interNor, -lightDir);
	fDot	= max(fDot, 0.5f);
    OUT.Diffuse = float4(fDot*0.72f, fDot*0.63f, fDot*0.41f, 1.0f);
        
    if(interPos.y > 0.005f )
    		OUT.PSize   = 720000.0f / OUT.Position.z;
    	else
    		OUT.PSize   = 0.f;
    		
 
    return OUT;
}


//------------------------------------
sampler SpriteSampler = sampler_state 
{
    texture = <diffuseTexture>;
     AddressU  = WRAP;        
    AddressV  = WRAP;
    AddressW  = WRAP;
    MIPFILTER = LINEAR;
    MINFILTER = LINEAR;
    MAGFILTER = LINEAR;
};


//-----------------------------------
float4 PS_PointSprite( const float2 TexCoord0 : TEXCOORD0,
                      const float4 Diffuse   : COLOR0 ) : COLOR
{
	float4 fColor = tex2D( SpriteSampler, TexCoord0 );

	fColor.a *= 0.40f;//0.36f;
	return fColor*Diffuse;
}


//-----------------------------------


//------------------------------------
struct vertexOutputA {
    float4 hPosition		: POSITION;
    float4 Diffuse          : COLOR0;
//    float2  texCoord		:TEXCOORD0;

};


//------------------------------------
vertexOutputA VS_AnimateTexture(vertexInput IN) 
{
	vertexOutputA OUT;
	float3  interPos = lerp(IN.position0,  IN.position1, fInterpolate);
	float3  interNor = lerp(IN.normal0,  IN.normal1, fInterpolate);
	interPos.y -= 0.1f;

//	float4x4 mtx;
//	mtx = mul( world , view);
//	mtx = mul( mtx, projection);
    	
    	OUT.hPosition =  mul( float4(interPos.xyz, 1.0) , worldViewProj);
//	OUT.texCoord.x = sqrt( dot(interPos.xz, interPos.xz)  )-fTime;

	float fDist = dot(interPos.xz, interPos.xz);

	float	fDot=	dot(interNor, -lightDir);
	fDot	= max(fDot, 0.5f);
    OUT.Diffuse = float4(fDot*0.576f, fDot*0.504f, fDot*0.328f, 1.0f);
    

	if(interPos.y > 0.005f && fDist<3.5f)
		OUT.Diffuse.a = 1.0f;
	else
		OUT.Diffuse.a = 0.0f;
	
	
	return OUT;
}

/*
//------------------------------------
sampler AnimateSampler = sampler_state 
{
    texture = <tableTexture>;
    AddressU  = WRAP;        
    AddressV  = WRAP;
    AddressW  = WRAP;
    MIPFILTER = LINEAR;
    MINFILTER = LINEAR;
    MAGFILTER = LINEAR;
};


sampler NoiseSampler = sampler_state 
{
    texture = <noiseTexture>;
    AddressU  = WRAP;        
    AddressV  = WRAP;
    AddressW  = WRAP;
    MIPFILTER = LINEAR;
    MINFILTER = LINEAR;
    MAGFILTER = LINEAR;
};*/


//-----------------------------------
float4 PS_AnimateTexture( vertexOutputA IN): COLOR
{
	float4 fcolor = IN.Diffuse;
//	float2 coord;
//	coord.x = IN.texCoord.y;
//	coord.y = fTime;
//	float noisy = tex2D(NoiseSampler, coord);
// 	noisy =  noisy*0.3 + IN.texCoord.x;
//	fcolor = tex2D(AnimateSampler, float2(noisy, 0.f) );


//	if( fcolor.a > 0.0 )
//		fcolor.a = 1.0f;
//	else
//		fcolor.a = 0.0f;
	return fcolor;
}




//-----------------------------------

technique PointSprite
{
    pass p0 
    {		
		VertexShader = compile vs_1_1 VS_PointSprite();
		PixelShader = compile ps_2_0 PS_PointSprite();
    }
}



technique AnimateTexture
{
    pass p0 
    {		
		VertexShader = compile vs_1_1 VS_AnimateTexture();
		PixelShader  = compile ps_2_0 PS_AnimateTexture();
    }
}