
//------------------------------------


//------------------------------------

texture diffuseTexture : Diffuse0
<
	string ResourceName = "default_color.dds";
>;

sampler diffuseSampler = sampler_state 
{
    texture = <diffuseTexture>;
//    AddressU  = WRAP;        
//    AddressV  = MIRROR;
//    AddressW  = MIRROR;
};



struct vertexInput
{
	float2 position		: POSITION;
	float2 texCoord0	: TEXCOORD0;
};

struct vertexOutput 
{
    float4 position		: POSITION;
    float4 texCoord0		: TEXCOORD0;
};

float4 vFactor;  //x :ImageIdx, y : Alpha
//------------------------------------
vertexOutput VS_SPRITE(vertexInput  IN) 
{
	vertexOutput OUT;
	OUT.position 	= float4(IN.position, 0.0, 1.0);
	OUT.texCoord0.xy 	= IN.texCoord0*vFactor.w + float2(fmod(vFactor.x,vFactor.z), floor(vFactor.x/vFactor.z))*vFactor.w;
	OUT.texCoord0.zw	= float2(0.f, 0.f);
	return OUT;
}

//-----------------------------------
float4 PS_SPRITE( vertexOutput IN): COLOR
{
  	float4 fColor = tex2D( diffuseSampler, IN.texCoord0 );
  	fColor.a  *=  vFactor.y;
  	return fColor;
}




vertexOutput VS_RADICAL(vertexInput  IN) 
{
	vertexOutput OUT;
	OUT.position 	= float4(IN.position, 0.0, 1.0);
	OUT.texCoord0.xy= IN.texCoord0;
	OUT.texCoord0.zw = IN.texCoord0*2.f-(1.f).xx;
	return OUT;
}

//-----------------------------------
float4 PS_RADICAL( vertexOutput IN): COLOR
{
  	float4 fColor = tex2D( diffuseSampler, IN.texCoord0.xy );
  	float fAlpha = sqrt( dot(IN.texCoord0.zw, IN.texCoord0.zw) )*1.5f-0.3f;
  	fColor.a  *=  vFactor.y*max( min(fAlpha, 1.f), 0.f);
  	return fColor;
}





//-----------------------------------
technique Sprite
{
    pass p0 
    {		
		VertexShader = compile vs_2_0 VS_SPRITE();
		PixelShader  = compile ps_2_0 PS_SPRITE();
    }	
}


//-----------------------------------
technique Radical
{
    pass p0 
    {		
		VertexShader = compile vs_2_0 VS_RADICAL();
		PixelShader  = compile ps_2_0 PS_RADICAL();
    }	
}