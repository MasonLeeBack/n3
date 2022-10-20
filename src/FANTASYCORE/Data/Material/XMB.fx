///////////////////////////////////////////////////////////
///////////////////////////// Render-to-Texture Data //////
///////////////////////////////////////////////////////////
string description = "XMB";

float2 RsqScreenSize = { 1000.f/1280.f, 700.f/720.f };

texture SceneMap : DIFFUSE0
< 
	string ResourceName = "default_color.dds";
>;

sampler SceneSampler = sampler_state 
{
    texture = <SceneMap>;
};

float fTime : TIME;
float2 Emit;

int nRandom;
///////////////////////////////////////////////////////////
/////////////////////////////////// data structures ///////
///////////////////////////////////////////////////////////

struct VS_OUTPUT_BLUR
{
    float4 Position   : POSITION;
    float2 TexCoord[3]: TEXCOORD0;
};

////////////////////////////////////////////////////////////
////////////////////////////////// vertex shaders //////////
////////////////////////////////////////////////////////////

// generate texture coordinates to sample 4 neighbours
VS_OUTPUT_BLUR VS_BlurH(float2 Position : POSITION)
{
	Emit.x = sin(fTime/2)*sin(fTime);
	Emit.y = cos(fTime/2)*sin(fTime);
	VS_OUTPUT_BLUR OUT = (VS_OUTPUT_BLUR)0;

	OUT.Position = float4(Position, 0.f, 1.f);
	OUT.TexCoord[0].x = OUT.Position.x*0.5f + 0.5f;
	OUT.TexCoord[0].y = -OUT.Position.y*0.5f + 0.5f;;
	
	float2 fTmp;
	fTmp.x = OUT.Position.x - Emit.x;
	fTmp.y = -OUT.Position.y -  -Emit.y;
	OUT.TexCoord[1] = normalize(fTmp);
	OUT.TexCoord[2] = abs(fTmp);

	return OUT;
}

//////////////////////////////////////////////////////
////////////////////////////////// pixel shaders /////
//////////////////////////////////////////////////////

const half weight[7] = 
{
	0.3,
	0.2,
	0.15,
	0.1,
	0.1,
	0.1,
	0.05
};

half4 PS_BloomH(VS_OUTPUT_BLUR IN) : COLOR
{
	half4 c = 0;
	int n =7;
	
	float2 fScreen = IN.TexCoord[0] * float2(1280.f, 720.f);
	int nSumXY = (int)fScreen.x+(int)fScreen.y;
	

	float2 fBlur, fCooard;
	fBlur.x = 0.02f;//RsqScreenSize.x;
	fBlur.y = 0.02f;//RsqScreenSize.y;
	
//	fBlur.x =  RsqScreenSize.x;
//	fBlur.y =  RsqScreenSize.y;
	
	if( IN.TexCoord[2].x < fBlur.x * float(n) )
		fBlur.x = IN.TexCoord[2].x/(float)n;
		
	if( IN.TexCoord[2].y < fBlur.y * float(n) )
		fBlur.y =IN.TexCoord[2].y/(float)n;
	
	for(int i=0;i<n;i++) 
	{
		fCooard =  IN.TexCoord[0]  - (IN.TexCoord[1] * fBlur * i);
		c += tex2D(SceneSampler, fCooard)*weight[i];
	}
	
	c.a = 0.6f;

/*	
	c = tex2D(SceneSampler, IN.TexCoord[0]);
	if( nSumXY%2 < 1)
	{
//		c.a = 0.f;
	}
*/
	return c;
}

////////////////////////////////////////////////////////////
/////////////////////////////////////// techniques /////////
////////////////////////////////////////////////////////////

technique Downsample
{
    pass p0
    {	
//    		ALPHABLENDENABLE =true;
//   		SRCBLEND = SRCALPHA;
//    		DESTBLEND = INVSRCALPHA;
		VertexShader = compile vs_2_0 VS_BlurH();
		PixelShader  = compile ps_2_0 PS_BloomH();
    }
}