shared float4x4 viewProj					: ViewProjection;

float4	 CamPos;

texture Texture : Diffuse
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
float fLineWidth = 0.f;
float fTexAniSpd = 20.0f;
float fAlpha = 1.f;
//------------------------------------
struct vertexInput {
    float3 position				: POSITION;
    float3 tangent				: TANGENT;
    float2 TexCoord				: TEXCOORD;
};

struct vertexOutput {
    float4 hPosition		: POSITION;
    float3 TexCoord	     : TEXCOORD0;
};



//------------------------------------
vertexOutput VS_TransformAndTexture(vertexInput IN) 
{
    vertexOutput OUT;
    float passTime = (curTime - IN.TexCoord.x);
    OUT.TexCoord.z = 1.f;
//*    
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
		
	passTime = passTime*lifeRsq;
	
		
	float3 binormal = (CamPos.xyz - IN.position);
	//binormal = cross ( normalize(binormal), normalize(IN.tangent) );
	binormal = cross ( binormal, IN.tangent );
	binormal = normalize(binormal);
	float fWidth = fLineWidth* ((IN.TexCoord.y-0.5)*1.f);
	binormal = float3( (binormal.x*fWidth), (binormal.y*fWidth),  (binormal.z*fWidth));
	float3 pos = IN.position + binormal;
		
	OUT.hPosition = mul( float4(pos , 1.0) , viewProj);

	OUT.TexCoord.x = passTime;
	OUT.TexCoord.y = IN.TexCoord.y;
	
    return OUT;
}

vertexOutput VS_VolumeTexture(vertexInput IN) 
{
	vertexOutput OUT;
    float passTime = (curTime - IN.TexCoord.x);
    OUT.TexCoord.z = 1.f;

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
		

	OUT.TexCoord.z = curTime/fTexAniSpd;
	passTime = passTime*lifeRsq;
	
		
	float3 binormal = (CamPos.xyz - IN.position);
	binormal = cross ( binormal, IN.tangent );
	binormal = normalize(binormal);
	float fWidth = fLineWidth* ((IN.TexCoord.y-0.5)*1.f);
	binormal = float3( (binormal.x*fWidth), (binormal.y*fWidth),  (binormal.z*fWidth));
	float3 pos = IN.position + binormal;
		
	OUT.hPosition = mul( float4(pos , 1.0) , viewProj);

	OUT.TexCoord.x = passTime;
	OUT.TexCoord.y = IN.TexCoord.y;

	
    return OUT;
}

//------------------------------------
sampler Sampler = sampler_state 
{
    texture = <Texture>;
//	AddressU  = WRAP;        
//	AddressV  = WRAP;
//	AddressW  = WRAP;
};



//-----------------------------------
float4 PS_Textured( vertexOutput IN): COLOR
{
		
 	float4 fColor = tex2D( Sampler, IN.TexCoord );
 //	fColor.rg *= 0.7f;
 //	fColor.b *= 3.f;
 //	fColor.a = IN.TexCoord.z;
	fColor		*= IN.TexCoord.z;
	fColor.a	*= fAlpha;
	return fColor;

}

float4 PS_volumeTexture( vertexOutput IN): COLOR
{
	float4 fColor = tex3D( Sampler, IN.TexCoord );	
	//fColor.w -= 0.7f;
	fColor.a *= fAlpha;
	return fColor;
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
		PixelShader  = compile ps_3_0 PS_Textured();
    }
        
}

technique volumeTex
{

    pass p0
	{
		VertexShader = compile vs_3_0 VS_VolumeTexture();
		PixelShader  = compile ps_3_0 PS_volumeTexture();
	}
}