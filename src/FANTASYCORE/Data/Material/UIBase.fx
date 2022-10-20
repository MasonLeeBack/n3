/*
 *	Texture
 */

//------------------------------------
float time : Time
<
    string UIWidget = "slider";
    float UIMin = -30.0;
    float UIMax = 180.0;
    float UIStep = 0.001;
    string UIName = "time";
> = 0;

texture LayerTexture : Diffuse
<
	string ResourceName = "default_color.dds";
>;

texture DummyTexture : Diffuse1
<
	string ResourceName = "test.dds";
>;

texture VolumnTexture : Diffuse2
<
	string ResourceName = "test.dds";
>;



//------------------------------------
struct vertexInput {
    float3 position			: POSITION;
    float2 texUV				: TEXCOORD0;
    float4 color				: COLOR0;
};

//------------------------------------
struct vertexOutput {
    float4 hPosition		: POSITION;
    float3 Layer0UV		: TEXCOORD0;
    float4 hColor			: COLOR0;
};

struct vertexOutputDUV {
    float4 hPosition		: POSITION;
    float3 Layer0UV		: TEXCOORD0;
    float4 ScreenUV		: TEXCOORD1;
};

struct vertexOutputAlpha {
    float4 hPosition		: POSITION;
    float3 LayerUV		: TEXCOORD0;
    float3 AlphaUV		: TEXCOORD1;
    float4 hColor			: COLOR0;
};

//------------------------------------
vertexOutput VS_TransformAndTexture_0_1(vertexInput IN) 
{
	vertexOutput OUT;
    	
	OUT.hPosition = float4(IN.position.xyz, 1);
	OUT.Layer0UV.xy = IN.texUV; 
	OUT.Layer0UV.z = 1.f;
	OUT.hColor = IN.color;
   
	return OUT;
}

vertexOutput VS_TransformAndTexture_3(vertexInput IN) 
{
	vertexOutput OUT;
	
	OUT.hPosition = float4(IN.position.xyz , 1.0) ;
	OUT.Layer0UV.xy = IN.texUV; 
	OUT.Layer0UV.z = 1.f;
    OUT.hColor = IN.color;
    return OUT;
}

vertexOutputAlpha VS_TransformAndTexture_4(vertexInput IN) 
{
	vertexOutputAlpha OUT;
	
	OUT.hPosition = float4(IN.position.xyz , 1.0) ;
	OUT.LayerUV =  float3(IN.texUV.xy, 1.f);
	OUT.AlphaUV =  float3(IN.texUV.xy, 1.f);
    OUT.hColor = IN.color;
    
    return OUT;
}

//-----------------------------------
sampler LayerSampler = sampler_state 
{
    texture = <LayerTexture>;
    //AddressU  = WRAP;        
    //AddressV  = WRAP;
    //AddressW  = WRAP;
    MINFILTER = LINEAR;
    MAGFILTER = LINEAR;
};

sampler DummySampler = sampler_state 
{
    texture = <DummyTexture>;
    //AddressU  = WRAP;        
    //AddressV  = WRAP;
    //AddressW  = WRAP;
    MINFILTER = LINEAR;
    MAGFILTER = LINEAR;
};

sampler3D VolumnSampler = sampler_state 
{
    texture = <VolumnTexture>;
    //AddressU  = WRAP;        
    //AddressV  = WRAP;
    //AddressW  = WRAP;
    MINFILTER = LINEAR;
    MAGFILTER = LINEAR;
};


//------------------------------------
float4 PS_Textured_0(vertexOutput IN): COLOR
{
	return IN.hColor;  
}

float4 PS_Textured_1_2(vertexOutput IN): COLOR
{	
	float4 Lay0 = tex2D(LayerSampler, IN.Layer0UV.xyz);
	return Lay0 *= IN.hColor;
}

float4 PS_Textured_4(vertexOutputAlpha IN): COLOR
{ 
	float4 SrcTexture = tex2D( LayerSampler,  IN.LayerUV);
   	float4 AlphaTexture = tex2D( DummySampler, IN.AlphaUV );
  	
  	// 0 - 1.f => 0.2f - 1.f
  	float fAlpha = (AlphaTexture.w * 0.7f) + 0.3f;
  	SrcTexture.w *= fAlpha;
  	SrcTexture *= IN.hColor;
  	 	
  	return SrcTexture;
}

#ifdef _XBOX
float4 tex2DOffset(sampler2D ss, float2 uv, float2 offset)
{
    float4 result;
    float offsetX = offset.x;
    float offsetY = offset.y;
    asm {
        tfetch2D result, uv, ss, OffsetX=offsetX,
            OffsetY=offsetY
    };
    return result;
}
#endif

float4 PS_Textured_5(vertexOutput IN): COLOR
{
#ifdef _XBOX	
	float4 Lay0 = tex2D(LayerSampler, IN.Layer0UV.xyz)*0.4f;
	Lay0 = Lay0 + tex2DOffset(LayerSampler, IN.Layer0UV.xy, float2(-2, 0))*0.1;
	Lay0 = Lay0 + tex2DOffset(LayerSampler, IN.Layer0UV.xy, float2(-1, 0))*0.2;
	Lay0 = Lay0 + tex2DOffset(LayerSampler, IN.Layer0UV.xy, float2(0, 1))*0.2;
	Lay0 = Lay0 + tex2DOffset(LayerSampler, IN.Layer0UV.xy, float2(0, 2))*0.1;
	Lay0 = Lay0 + tex2DOffset(LayerSampler, IN.Layer0UV.xy, float2(0, -2))*0.1;
	Lay0 = Lay0 + tex2DOffset(LayerSampler, IN.Layer0UV.xy, float2(0, -1))*0.2;
	Lay0 = Lay0 + tex2DOffset(LayerSampler, IN.Layer0UV.xy, float2(0, 1))*0.2;
	Lay0 = Lay0 + tex2DOffset(LayerSampler, IN.Layer0UV.xy, float2(0, 2))*0.1;
	Lay0 = Lay0/1.5f;
	
	return Lay0 *= IN.hColor;
#else
	float4 Lay0 = tex2D(LayerSampler, IN.Layer0UV.xyz);
	return Lay0 *= IN.hColor;
#endif
}

//-----------------------------------
//_Ui_Mode_Box
technique textured_0			
{
    pass p0 
    {		
		VertexShader = compile vs_1_1 VS_TransformAndTexture_0_1();
		PixelShader  = compile ps_2_0 PS_Textured_0();
    }
}

//_Ui_Mode_Image, _Ui_Mode_Text, _Ui_Mode_Movie
technique textured_1
{
    pass p0 
    {		
		VertexShader = compile vs_1_1 VS_TransformAndTexture_0_1();
		PixelShader  = compile ps_2_0 PS_Textured_1_2();
    }
}

// Alpha map
technique textured_2//4
{
    pass p0 
    {
		VertexShader = compile vs_1_1 VS_TransformAndTexture_4();
		PixelShader  = compile ps_2_0 PS_Textured_4();
    }
}

// Anti-Aliacing for Using 3D Object in UI
technique textured_3
{
    pass p0 
    {
		VertexShader = compile vs_1_1 VS_TransformAndTexture_4();
		PixelShader  = compile ps_2_0 PS_Textured_5();
    }
}