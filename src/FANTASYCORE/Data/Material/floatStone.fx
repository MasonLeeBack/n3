string description = "Earth";


//------------------------------------
float4x4 worldViewProj : WorldViewProjection;

texture diffuseTexture : Diffuse
<
	string ResourceName = "stones16_N.dds";
>;

float4 vCenterPos;
float4 vCameraPos;
float4 vInhalePos;
float Interval = 0.0f;
float lerpvalue = 0.0f;
float AccmulatePos;
//------------------------------------
struct vertexInput {
    float4 position				: POSITION0;
    float3 normal          			: NORMAL0;
};

struct vertexOutput {
    float4 hPosition				: POSITION;
    float3 Diff	          			: COLOR;
    float  PSize          			: PSIZE;	
};


//------------------------------------
vertexOutput VS_StoneUp(vertexInput IN) 
{
    vertexOutput OUT;
    OUT.hPosition =  IN.position;
    OUT.hPosition.y += AccmulatePos;
    OUT.hPosition = mul( OUT.hPosition , worldViewProj);
    OUT.Diff = IN.normal;
    float diff = length(vCameraPos - IN.position);
    diff *= 0.01f;
    if( diff <= 1 ) diff = 1;
    if( diff >= 20.0f ) diff = 20.0f;
    OUT.PSize = IN.normal.y/diff * 5.0f;
        
    return OUT;
}

vertexOutput VS_StoneHale(vertexInput IN) 
{
	vertexOutput OUT;
	OUT.hPosition = IN.position;
     OUT.hPosition.y += AccmulatePos;
     OUT.hPosition = mul( OUT.hPosition , worldViewProj);
     
     float4 HalePos = float4( vInhalePos.xyz, 1.0f);
     HalePos = mul( HalePos , worldViewProj);
     OUT.hPosition = lerp(OUT.hPosition, HalePos, lerpvalue);
     
     OUT.Diff = IN.normal;
     float diff = length(vCameraPos - IN.position);
     diff *= 0.01f;
     if( diff <= 1 ) diff = 1;
     if( diff >= 20.0f ) diff = 20.0f;
     OUT.PSize = IN.normal.y/diff * 5.0f;
     
	return OUT;
}
//------------------------------------
sampler SpriteSampler = sampler_state 
{
    texture = <diffuseTexture>;
//    AddressU  = WRAP;        
//    AddressV  = WRAP;
//    AddressW  = WRAP;
};

//-----------------------------------
float4 PS_Textured( const float2 TexCoord0 : SPRITETEXCOORD ,
 							vertexOutput In ): COLOR
{
	float3 tmpTex = float3(TexCoord0.xy, In.Diff.x);
	float4 fColor = tex3D( SpriteSampler,tmpTex);
  	return fColor;
}


//-----------------------------------
technique t0
{
    pass p0 
    {		
		VertexShader = compile vs_2_0 VS_StoneUp();
		PixelShader  = compile ps_2_0 PS_Textured();
    }
}

//-----------------------------------
technique t1
{
    pass p0 
    {		
		VertexShader = compile vs_2_0 VS_StoneHale();
		PixelShader  = compile ps_2_0 PS_Textured();
    }
}