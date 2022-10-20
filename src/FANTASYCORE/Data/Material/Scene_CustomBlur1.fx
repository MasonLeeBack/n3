//------------------------------------

texture diffuseTexture : Diffuse
<
	string ResourceName = "wave.dds";
>;

texture backTexture : Diffuse1
<
	string ResourceName = "image2.bmp";
>;







//------------------------------------
struct vertexInput {
    float2 position				: POSITION;
//    float4 texCoordDiffuse		: TEXCOORD0;
};

struct vertexOutput {
    float4 hPosition		: POSITION;
    float4 texCoord	: TEXCOORD0;
    float4 texCoord1: TEXCOORD1;
    float4 texCoord2: TEXCOORD2;
};

float fTime : TIME;

//------------------------------------
vertexOutput VS_TransformAndTexture(vertexInput IN) 
{
    vertexOutput OUT;
    float fTime =  fTime*0.2f;
    OUT.hPosition =  float4(IN.position.xy , 0.f, 1.0);
    float2 fUV = OUT.hPosition.xy*float2(0.5f, -0.5f)+(0.5).xx;
    
    OUT.texCoord = float4(fUV, IN.position.xy);
        
    OUT.texCoord1.xy = fUV-float2(frac(fTime), -frac(fTime));
    OUT.texCoord1.zw = fUV-float2(-frac(fTime), -frac(fTime));
    
    float2 Emit;
    	Emit.x = sin(fTime/2)*sin(fTime)*0.25f;
	Emit.y = cos(fTime/2)*sin(fTime)*0.25f;
	    
	float2 fTmp;    
	fTmp.x = OUT.hPosition.x - Emit.x;
	fTmp.y = -(OUT.hPosition.y -  Emit.y);
    OUT.texCoord2.xy =  normalize(fTmp);
    OUT.texCoord2.zw = abs(fTmp);

    return OUT;
}


//------------------------------------
sampler TextureSampler = sampler_state 
{
    texture = <diffuseTexture>;
};

//------------------------------------
sampler BackSampler = sampler_state 
{
    texture = <backTexture>;
//    AddressU  = MIRROR;        
//    AddressV  = MIRROR;
};


const half weight[5] = 
{
	0.3,
	0.25,
	0.2,
	0.15,
	0.1,
};

//-----------------------------------
float4 PS_Textured( vertexOutput IN): COLOR
{
	int n =5;
	float2 fBlur, fCoord;
	fBlur.x = 0.01f;//RsqScreenSize.x;
	fBlur.y = 0.01f;//RsqScreenSize.y;
	
	if( IN.texCoord2.z < fBlur.x * float(n) )
		fBlur.x = IN.texCoord2.z/(float)n;
		
	if( IN.texCoord2.w < fBlur.y * float(n) )
		fBlur.y =IN.texCoord2.w/(float)n;
	
	float4 fColor = float4(0.f, 0.f, 0.f, 0.f);
	for(int i=0;i<n;i++) 
	{
		fCoord =  IN.texCoord.xy  - (IN.texCoord2.xy * fBlur * i);
		fColor += tex2D(BackSampler, fCoord)*weight[i];
	}

	float fTap = IN.texCoord.z *IN.texCoord.z;
	fTap +=IN.texCoord.w *IN.texCoord.w;

	float fColorTap = smoothstep( .5f, 2.f, fTap);;
	float fLerp = smoothstep( -.1f, 1.2f, fTap);
	
	float4 diffuseTexture0   = tex2D( TextureSampler, IN.texCoord1.xy )*0.5f;
	diffuseTexture0 += tex2D( TextureSampler, IN.texCoord1.zw )*0.5f;

	float4 fResult = diffuseTexture0;
	fResult = lerp( float4(0.5f, 0.5f, 1.f, 1.f), fResult, fLerp );
	fResult = ((fResult)-(0.5f).xxxx) *2.f;
	
	float4 fBack = tex2D( BackSampler, IN.texCoord.xy + fResult.xy*0.08f );
	fBack = fBack*0.3f + fColor*0.7f;
	
	fBack.xyz += float3(0.8f, 0.1f, 0.1f)*fColorTap*0.5f;
	fBack.a = 0.5f;
	return  fBack;
}


//-----------------------------------
technique textured
{
    pass p0 
    {		
		VertexShader = compile vs_2_0 VS_TransformAndTexture();
		PixelShader  = compile ps_2_0 PS_Textured();
    }
}