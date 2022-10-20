///////////////////////////////////////////////////////////
///////////////////////////// Render-to-Texture Data //////
///////////////////////////////////////////////////////////

texture DepthMap;
texture NearMap;
texture FarMap;

float4 ScreenSize : VIEWPORTPIXELSIZE;

float fTap = 1.0f;

float2 vPixelSizeLow  = { 0.25f/1280.f, 0.25f/720.f};
float2 vPixelSizeHigh ={ 1.00f/1280.f, 1.00f/720.f};


float2 vPoisson[] =
    {
        { 0.000000f, 0.000000f },
        { 0.527837f,-0.085868f },
        {-0.040088f, 0.536087f },
        {-0.670445f,-0.179949f },
        {-0.419418f,-0.616039f },
        { 0.440453f,-0.639399f },
        {-0.757088f, 0.349334f },
        { 0.574619f, 0.685879f },
    };

float4 vMaxCoC     = { 5.0f, 10.0f, 0.f, 0.f };
float fRadiusScale =  0.4f;
 int NUM_POISSON_TAPS = 8;
 float fMaxBlur = 0.90f;

float4 vFocus;
//float4 fFocus;

sampler DepthSampler = sampler_state 
{
    texture = <DepthMap>;
    AddressU  = CLAMP;        
    AddressV  = CLAMP;
    AddressW  = CLAMP;
    MINFILTER = POINT;
    MAGFILTER = POINT;
};


sampler NearSampler = sampler_state 
{
    texture = <NearMap>;
    AddressU  = CLAMP;        
    AddressV  = CLAMP;
    AddressW  = CLAMP;
    MINFILTER = POINT;
    MAGFILTER = POINT;
};


sampler FarSampler = sampler_state 
{
	texture = <FarMap>;
	AddressU  = CLAMP;        
	AddressV  = CLAMP;
	AddressW  = CLAMP;
	MINFILTER = LINEAR;
	MAGFILTER = LINEAR;
};
///////////////////////////////////////////////////////////
/////////////////////////////////// data structures ///////
///////////////////////////////////////////////////////////

struct VS_OUTPUT_BLUR
{
    float4 Position	: 	POSITION;
    float2 TexCoord0 :	TEXCOORD0;
    float4 TexCoord1 :	TEXCOORD1;
    float4 TexCoord2 :	TEXCOORD2;
    float4 TexCoord3 :	TEXCOORD3;
//    float2 TexCoord4 :	TEXCOORD4;
};


struct VS_OUTPUT_DEPTH
{
   float4 Position	: 	POSITION;
   float2 TexCoord0 :	TEXCOORD0;
};

////////////////////////////////////////////////////////////
////////////////////////////////// vertex shaders //////////
////////////////////////////////////////////////////////////

// generate texture coordinates to sample 4 neighbours
VS_OUTPUT_BLUR  VS_DOF(float3 Position : POSITION,	float2 TexCoord : TEXCOORD0)
{

	VS_OUTPUT_BLUR OUT = (VS_OUTPUT_BLUR)0;
	OUT.Position = float4(Position, 1.f);
	OUT.TexCoord0 = TexCoord;
	OUT.TexCoord1 = TexCoord.xyxy;
	OUT.TexCoord2 = TexCoord.xyxy;
	OUT.TexCoord3 = TexCoord.xyxy;

	
	float fcos = fTap*vPixelSizeHigh.y*0.5f;
	float fsin =  fTap*vPixelSizeHigh.y*0.7f;
	float fOffsetX = fTap*vPixelSizeHigh.x;
	float fOffsetY = fTap*vPixelSizeHigh.y;
	
	OUT.TexCoord1.x -= fOffsetX;
	OUT.TexCoord1.zw += float2( -fcos, -fsin);
	
	OUT.TexCoord2.xy += float2( fcos, -fsin);
	OUT.TexCoord2.z += fOffsetX;
	
	OUT.TexCoord3.xy += float2( fcos, fsin);
	OUT.TexCoord3.zw -= float2( -fcos, fsin);
	
	return OUT;
}


VS_OUTPUT_DEPTH VS_DEPTH(float3 Position : POSITION,
									float2 TexCoord : TEXCOORD0)
{

	VS_OUTPUT_DEPTH OUT = (VS_OUTPUT_DEPTH)0;
	OUT.Position = float4(Position, 1.f);
	OUT.TexCoord0 = TexCoord;
	
	return OUT;
}

//////////////////////////////////////////////////////
////////////////////////////////// pixel shaders /////
//////////////////////////////////////////////////////



float4 PS_PASS(VS_OUTPUT_BLUR IN) : COLOR
{
	float4 c = 0.f;
	
	c = tex2D(NearSampler, IN.TexCoord0)*0.1f;
	c += tex2D(NearSampler, IN.TexCoord1.xy)*0.15f;
	c += tex2D(NearSampler, IN.TexCoord1.zw)*0.15f;
	c += tex2D(NearSampler, IN.TexCoord2.xy)*0.15f;
	c += tex2D(NearSampler, IN.TexCoord2.zw)*0.15f;
	c += tex2D(NearSampler, IN.TexCoord3.xy)*0.15f;
	c += tex2D(NearSampler, IN.TexCoord3.zw)*0.15f;
	c.w = 0.5f;
	return c;
}



float4 PS_DOF2(VS_OUTPUT_DEPTH IN) : COLOR
{
	float4 Depth = tex2D(DepthSampler, IN.TexCoord0);
//	float4 Near = tex2D(NearSampler, IN.TexCoord0);
//	float4 Far = tex2D(FarSampler, IN.TexCoord0);

//	float  fDepth = Depth.r*16711680+Depth.g*65280+Depth.g*256;
//	fDepth /= 16777215.f;
	float	fDepth = Depth.r;
	  
	float	 fBlurRate;
	if( fDepth > vFocus.x )
	{
		fBlurRate = ( vFocus.x - fDepth) / ( vFocus.x - vFocus.y );
	}
	else
	{
		fBlurRate = ( vFocus.x - fDepth) / ( vFocus.x - vFocus.z );
	}

	fBlurRate = clamp(fBlurRate , 0, 1.f );
/*	
	float fDiscRadius = abs( fBlurRate * vMaxCoC.y - vMaxCoC.x );
	float fDiscRadiusLow = fDiscRadius * fRadiusScale;
	float4 vOutColor = 0;

    for( int t=0; t<NUM_POISSON_TAPS; t++ )
    {
  
        float2 vCoordLow = IN.TexCoord0 + ( vPixelSizeLow * vPoisson[t] * fDiscRadiusLow );
        float4 vTapLow   = tex2D( FarSampler, vCoordLow );
        
  
        float2 vCoordHigh = IN.TexCoord0 + ( vPixelSizeHigh * vPoisson[t] * fDiscRadius );
        float4 vTapHigh   = tex2D( NearSampler, vCoordHigh );
  
        float4 TapDepth = tex2D(DepthSampler,vCoordHigh);
        float fTapDepth =  TapDepth.r+TapDepth.g*0.03907f;
        
		if( fTapDepth < vFocus.x )
		{
			fBlurRate = ( fTapDepth - vFocus.x ) / ( vFocus.x - vFocus.y );
		}
		else
		{
			fBlurRate = ( fTapDepth - vFocus.x ) / (  vFocus.z - vFocus.x );
		}
        fBlurRate = abs(fBlurRate);	
        fBlurRate = clamp(fBlurRate , 0, fMaxBlur );
        float4 vTap = lerp( vTapHigh, vTapLow, fBlurRate );
                
//        vTap.a = ( fTapDepth >= fDepth ) ? 1.0f : abs( fBlurRate);
	   vTap.a = 1.0f;
       
        vOutColor.rgb += vTap.rgb * vTap.a;
        vOutColor.a   += vTap.a;
    }
    
	return float4(fBlurRate, fBlurRate, fBlurRate, 1.f);
//    return ( vOutColor / vOutColor.a );
*/
	float2 vCoordLow = IN.TexCoord0;
	float4 vTapLow   = tex2D( FarSampler, vCoordLow );
  
	float2 vCoordHigh = IN.TexCoord0;
	float4 vTapHigh   = tex2D( NearSampler, vCoordHigh );
	
	float4 vTap = lerp( vTapHigh, vTapLow, fBlurRate );
	vTap.a = 1.f;
	
	return vTap;
}


float4 PS_DEPTH(VS_OUTPUT_DEPTH IN) : COLOR
{
	float4 Depth = tex2D(DepthSampler, IN.TexCoord0);

//	float  fDepth = Depth.r*16711680+Depth.g*65280+Depth.b*256;
//	fDepth /= 16777215.f;
	float	fDepth = Depth.r;
	  
	float	 fBlurRate;
	if( fDepth > vFocus.x )
	{
		fBlurRate = ( vFocus.x - fDepth) / ( vFocus.x - vFocus.y );
	}
	else
	{
		fBlurRate = ( vFocus.x - fDepth) / ( vFocus.x - vFocus.z );
	}

	fBlurRate = clamp(fBlurRate , 0, 1.f );

	float2 vCoordLow = IN.TexCoord0;
	float4 vTapLow   = tex2D( FarSampler, vCoordLow );
  
	float2 vCoordHigh = IN.TexCoord0;
	float4 vTapHigh   = tex2D( NearSampler, vCoordHigh );
	
//	float4 vTap = lerp( vTapHigh, vTapLow, fBlurRate );
//	vTap.a = 1.f;
	float4 vTap = float4( 0.f, 1.f-fBlurRate, 0.f, 1.f);
	return vTap;
}

float avSampleWeights[] =
{
	0.0248824656f,
	0.0676375553f,
	0.111515477f,
	0.0676375553f,
	0.0248824656f,
	0.111515477f,
	0.183857948f,
	0.111515477f,
	0.0248824656f,
	0.0676375553f,
	0.111515477f,
	0.0676375553f,
	0.0248824656f
};


float2 avSampleOffsets[] =
{
	{-0.00625000009, 0.000000000},
	{-0.00312500005, -0.00555555569},
	{-0.00312500005, 0.000000000},
	{-0.00312500005, 0.00555555569},
	{0.000000000, -0.0111111114},
	{0.000000000, -0.00555555569},
	{0.000000000, 0.000000000},
	{0.000000000, 0.00555555569},
	{0.000000000, 0.0111111114},
	{0.00312500005, -0.00555555569},
	{0.00312500005, 0.000000000},
	{0.00312500005, 0.00555555569},
	{0.00625000009, 0.000000000}
};

float4 PS_Gauss5x5(VS_OUTPUT_DEPTH IN) : COLOR
{
    float4 vColor = 0.0f;

    for( int i=0; i < 13; i++ )
    {
        vColor += avSampleWeights[i] * tex2D( FarSampler, IN.TexCoord0 + avSampleOffsets[i].xy );
    }
//	vColor =  tex2D( FarSampler, IN.TexCoord0  );
//	vColor.a =1.f;
    return vColor;
}

////////////////////////////////////////////////////////////
/////////////////////////////////////// techniques /////////
////////////////////////////////////////////////////////////

technique DOF
{
    pass p0
    {
		VertexShader = compile vs_2_0 VS_DOF();
		PixelShader  =  compile ps_2_0 PS_PASS();
    }
}


technique DOF2
{
    pass p0
    {
		VertexShader = compile vs_3_0 VS_DEPTH();
		PixelShader  =  compile ps_3_0 PS_DOF2();
    }
}


technique Gauss5x5
{
    pass p0
    {
		VertexShader = compile vs_2_0 VS_DEPTH();
		PixelShader  =  compile ps_2_0 PS_Gauss5x5();
    }
}

technique DEPTH
{
    pass p0
    {
		VertexShader = compile vs_3_0 VS_DEPTH();
		PixelShader  =  compile ps_3_0 PS_DEPTH();
    }
}

