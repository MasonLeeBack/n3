// BBoard.fx for Unit, Grass Billboard

shared float4x4 viewProj					: ViewProjection;
shared float4	vecRcpWorldSize					: RcpWorldSize;

float4	 CamAxisX;
float4	 CamAxisY;

texture Texture : Volume
<
	string ResourceName = "default_color.dds";
>;

float curTime
<
	string UIWidget = "slider";
	float  UIMin = 0.0;
	float  UIMax = 10.0;
	float  UIStep = 0.01;
	string UIName = "Current Time";
> = 0.0;

float life
<
	string UIWidget = "slider";
	float  UIMin = 0.0;
	float  UIMax = 3.0;
	float  UIStep = 0.01;
	string UIName = "life";
> = 0.0;
float lifeRsq = 1.0;
float fRadius = 0.f;
float fAlpha = 1.0f;

//------------------------------------
struct vertexInput {
    float3 position				: POSITION;
    float3 TexCoord				: TEXCOORD0;
};

struct vertexOutput {
    float4 hPosition		: POSITION;
    float4 TexCoord	     : TEXCOORD0;
};



//------------------------------------
vertexOutput VS_TransformAndTexture(vertexInput IN) 
{
    vertexOutput OUT;
    float passTime = (curTime - IN.TexCoord.z);
    OUT.TexCoord.z = 1.f;
/*    
    OUT.TexCoord.z *= step( passTime, life);
	OUT.TexCoord.z *= step( 0.f, passTime);
	passTime = clamp(passTime, 0.f, life);
/*/	
    if(passTime > life )
	{
		passTime = life;
		OUT.TexCoord.z = 0.f;
	}
	else if(passTime < 0 )
	{
		passTime = 0;
		OUT.TexCoord.z = 0.f;
	}
//*/		
		
	passTime = -abs(passTime - life*0.5)+ life*0.5;
	passTime= passTime *6 *lifeRsq;
	OUT.TexCoord.w= min( passTime, 1.f);
	
	float fx = (IN.TexCoord.x-0.5f)*2.f*fRadius;
	float fy = (IN.TexCoord.y-0.5f)*2.f*fRadius;
	float3 pos = IN.position + (fx*CamAxisX.xyz) +(fy*CamAxisY.xyz);
		
	OUT.hPosition = mul( float4(pos , 1.0) , viewProj);
	OUT.TexCoord.x = IN.TexCoord.x;
	OUT.TexCoord.y = IN.TexCoord.y;
	OUT.TexCoord.z = cos(IN.position.x);
	return OUT;
}


//------------------------------------
sampler3D Sampler = sampler_state 
{
    texture = <Texture>;
//	AddressU  = WRAP;        
//	AddressV  = WRAP;
//	AddressW  = WRAP;
};



//-----------------------------------
float4 PS_Textured( vertexOutput IN): COLOR
{
		
 	float4 fColor = tex3D( Sampler, IN.TexCoord.xyz);
 //	fColor.rg *= 0.7f;
 //	fColor.b *= 3.f;
// 	fColor *= IN.TexCoord.w;
	fColor.a *= fAlpha;
	return fColor;
}

//-----------------------------------
technique textured
{
    pass p0 
    {		 
		VertexShader = compile vs_3_0 VS_TransformAndTexture();
		PixelShader  = compile ps_3_0 PS_Textured();
    }
}