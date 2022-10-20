
float4x4 worldInvTrans : WorldInverseTranspose;
shared float4x4 viewInverse : ViewInverse;
float4x4 wvp : WorldViewProjection;
float4x4 world : World;


texture cubeMap : Environment
<
	string ResourceName = "default_reflection.dds";
	string ResourceType = "Cube";
>;



texture diffuseTex : ScreenBuffer
<
>;

samplerCUBE environmentMapSampler = sampler_state
{
	Texture = <cubeMap>;
	MinFilter = Linear;
	MagFilter = Linear;
	MipFilter = Linear;
};

sampler2D ScreenSampler = sampler_state
{
	Texture = <diffuseTex>;
	MinFilter = Linear;
	MagFilter = Linear;
	MipFilter = Linear;
};

float fTime :TIME;
/************* DATA STRUCTS **************/

/* data from application vertex buffer */
struct appdata {
    float4 Position	: POSITION;
    float4 UV		: TEXCOORD0;
    float3 Normal	: NORMAL;
};

/* data passed from vertex shader to pixel shader */
struct vertexOutput {
    float4 HPosition	: POSITION;
    float4 TexCoord		: TEXCOORD0;
    float3 WorldNormal	: TEXCOORD1;
    float3 WorldView	: TEXCOORD2;
};

/*********** vertex shader ******/

vertexOutput mainVS(appdata IN) 
{
    vertexOutput OUT;
    float3 normal = normalize(IN.Normal);
    OUT.WorldNormal = mul(normal, (float3x3) worldInvTrans);
    float3 Pw = mul(IN.Position, world).xyz;
    OUT.TexCoord = IN.UV;
    OUT.WorldView = viewInverse[3].xyz - Pw;
    
    float fTime = fTime;//*40.f;
    float4 p = IN.Position;
	p.xyz += IN.Normal*(sin(IN.Position.x*10.f+fTime*4.2f)*0.03f + sin(IN.Position.x*20.f+fTime*8.4f)*0.01f );
	p.xyz += IN.Normal*(sin(IN.Position.y*6.f+fTime*7.2f)*0.03f  + sin(IN.Position.x*12.f+fTime*14.4f)*0.01f );
	p.xyz += IN.Normal*(sin(IN.Position.z*4.f+fTime*5.2f)*0.03f + sin(IN.Position.z*8.f+fTime*10.4f)*0.01f );
    OUT.HPosition = mul(p, wvp);
    
    p = IN.Position;
  
    p.xyz*=-1;
    p = mul(p,wvp);
    OUT.TexCoord.x = (p.x/p.w)*0.5f+0.5f;
    OUT.TexCoord.y = (p.y/p.w)*-0.5f+0.5f;
    return OUT;
}

/********* pixel shader ********/

// modified refraction function that returns boolean for total internal reflection
float3
refract2( float3 I, float3 N, float eta, out bool fail )
{
	float IdotN = dot(I, N);
	float k = 1 - eta*eta*(1 - IdotN*IdotN);
//	return k < 0 ? (0,0,0) : eta*I - (eta*IdotN + sqrt(k))*N;
	fail = k < 0;
	return eta*I - (eta*IdotN + sqrt(k))*N;
}


float4 mainPS(vertexOutput IN) : COLOR
{
	float3 N = normalize(IN.WorldNormal);
	float3 V = normalize(IN.WorldView);
	
 	// reflection

	float4 refract = tex2D(ScreenSampler, IN.TexCoord.xy);
	// wavelength colors
        
	// transmission
 	float4 transColor = 0;
  	bool fail = false;
	
	float3 T = refract2(V, N, 0.95f, fail);
    transColor += texCUBE(environmentMapSampler, T);// * colors[0];
    
//    T = refract2(-V, N, etas[1], fail);
//    transColor += texCUBE(environmentMapSampler, T) * colors[1];
    
//    T = refract2(-V, N, etas[2], fail);
//    transColor += texCUBE(environmentMapSampler, T) * colors[2];
    		
	
	
    float f = abs(dot(-V,N));
    float4 fResult = lerp(transColor*1.3f, refract, f);
	fResult.w = 1.f;
	return fResult;
}

/*************/

technique dx9  
{
	pass p0   
	{		
        VertexShader = compile vs_2_0 mainVS();
        PixelShader = compile ps_2_0 mainPS();
	}
}

/***************************** eof ***/
