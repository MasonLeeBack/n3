

shared float4x4 viewInverse : ViewInverse;
shared float4x4 view: View;

float4x4 worldInvTrans : WorldInverseTranspose;
float4x4 wvp : WorldViewProjection;
float4x4 world : World;
float4x4 worldview : WorldView;






texture cubeMap : Environment
<
	string ResourceName = "default_reflection.dds";
	string ResourceType = "Cube";
>;





texture DiffuseTexture : Volume
<
	string ResourceName = "RyviusCore.dds";
	string ResourceType = "3D";
>;

texture DiffuseTexture1 : Volume
<
	string ResourceName = "RyviusLight.dds";
	string ResourceType = "3D";
>;

texture RefractTex : ScreenBuffer
<
	string ResourceName = "default_reflection.dds";
>;

samplerCUBE environmentMapSampler = sampler_state
{
	Texture = <cubeMap>;
};



sampler3D Diffuse = sampler_state
{
	Texture = <DiffuseTexture>;
};

sampler3D Diffuse1 = sampler_state
{
	Texture = <DiffuseTexture1>;
};

sampler2D ScreenSampler = sampler_state
{
	Texture = <RefractTex>;
};

float fTime :TIME;
/************* DATA STRUCTS **************/

/* data from application vertex buffer */
struct appdata {
    float4 Position	: POSITION;
    float4 UV			: TEXCOORD0;
    float3 Normal		: NORMAL;
};

/* data passed from vertex shader to pixel shader */
struct vertexOutput {
    float4 HPosition		: POSITION;
    float4 TexCoord		: TEXCOORD0;
    float3 WorldNormal	: TEXCOORD1;
    float3 WorldView		: TEXCOORD2;
    float4 TexCoord2	: TEXCOORD3;
    
    float4 TexDepth		: TEXCOORD4;
    float4 Intensity	: TEXCOORD5;
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
	
	
	float3x3  normailzeWV = (float3x3) worldview;
	normailzeWV._11_12_13 /= length( float3(normailzeWV._11,normailzeWV._12, normailzeWV._13) );
	normailzeWV._21_22_23 /= length( float3(normailzeWV._21,normailzeWV._22, normailzeWV._23) );
	normailzeWV._31_32_33 /= length( float3(normailzeWV._31,normailzeWV._32, normailzeWV._33) );
	
	p.xyz = mul( (p.xyz) , normailzeWV );
	OUT.TexCoord.z = p.x*0.7f+0.5f;
	OUT.TexCoord.w = p.y*-0.7f+0.5f;
	
	

	OUT.TexCoord2.xy = p.xy*float2(0.5f, -0.5f)+float2(0.5f, 0.5f);
	OUT.TexCoord2.zw = OUT.TexCoord2.xy;

	
    
	p = IN.Position;
	p.xyz*=-1;
	p = mul(p,wvp);
	OUT.TexCoord.x = (p.x/p.w)*0.5f+0.5f;
	OUT.TexCoord.y = (p.y/p.w)*-0.5f+0.5f;
	
	OUT.TexDepth.x = (sin(fTime*0.012f));
	OUT.TexDepth.y = (sin(fTime*0.025f));
	OUT.TexDepth.z = (sin(fTime*0.05f));
	OUT.TexDepth.w = (sin(fTime));
	OUT.TexDepth.xyz = OUT.TexDepth.xyz*0.5f+(0.5f).xxx;
	
	OUT.Intensity.x = 0.4f;//(sin(fTime*0.823f));
	OUT.Intensity.y = 0.4f;//(sin(fTime*0.485f));
	OUT.Intensity.z = 0.4f;//(sin(fTime*0.815f));
	OUT.Intensity.w = (sin(fTime*0.500f));
//	OUT.Intensity.yz = OUT.Intensity.yz*0.2f+(0.3f).xx;
	
	
	return OUT;
}

/********* pixel shader ********/

// modified refraction function that returns boolean for total internal reflection
float3
refract2( float3 I, float3 N, float eta )
{
	float IdotN = dot(I, N);
	float k = 1 - eta*eta*(1 - IdotN*IdotN);
	return eta*I - (eta*IdotN + sqrt(k))*N;
}


float4 mainPS(vertexOutput IN) : COLOR
{
	float3 N = normalize(IN.WorldNormal);
	float3 V = normalize(IN.WorldView);
	
 	// reflection
 	float4 Diff = tex3D(Diffuse,    float3( IN.TexCoord.zw, IN.TexDepth.x) );
 	Diff.xyz *= Diff.a;
 	float fDepth = frac(fTime*0.6f);
 	float4 Diff1 = tex3D(Diffuse1, float3( IN.TexCoord2.zw,fDepth));// IN.TexDepth.y) );
 	Diff1.xyz *= Diff1.a;
 	float4 Diff2 = tex3D(Diffuse1, float3( IN.TexCoord2.xy, fDepth));//IN.TexDepth.z) );
 	Diff2.xyz *= Diff2.a;
//	return Diff;
	float4 refract = tex2D(ScreenSampler, IN.TexCoord.xy);
       
	// transmission
 	float4 transColor = 0;
  	
  	float3 T = refract2(V, N, 0.95f);
    transColor += texCUBE(environmentMapSampler, T);
    	
    float f = abs(dot(-V,N))*1.2f;
    float4 fResult = lerp(transColor*1.0f, refract, f);
	fResult.w = 1.f;
	
	return fResult*0.5f+ Diff*IN.Intensity.x+Diff1*IN.Intensity.y+Diff2*IN.Intensity.z;
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
