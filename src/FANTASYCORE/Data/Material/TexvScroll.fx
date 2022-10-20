


float4x4 worldViewProj				: WorldViewProjection;
float4 materialAmbient				: MaterialAmbient
<
    string UIWidget = "Surface Color";
    string Space = "material";
> = { 0.4, 0.4, 0.4, 0.8f};
float fVvalue						: fx_custom0
 = 0.0f;
 


texture diffuseTexture : Diffuse
<
	string ResourceName = "default_color.dds";
>;

sampler diffuseSampler = sampler_state
{
	Texture = <diffuseTexture>;
};


struct vertexInput 
{
    float3 position					: POSITION;
    float2 texCoordDiffuse			: TEXCOORD0;
};

struct vertexOutput 
{
    float4 hPosition				: POSITION;
    float2 texCoordDiffuse			: TEXCOORD0;
};






//------------------------------------
vertexOutput VS_TransformAndTexture(vertexInput IN) 
{
    vertexOutput OUT;
    OUT.hPosition = mul( float4(IN.position.xyz , 1.0) , worldViewProj);

    OUT.texCoordDiffuse = IN.texCoordDiffuse;
    
    return OUT;
}
//-----------------------------------




//-----------------------------------
float4 PS_Textured( vertexOutput IN): COLOR
{
	float2 TexCoord = IN.texCoordDiffuse;
	TexCoord.y = IN.texCoordDiffuse.y + fVvalue;// * 0.01f;
	if(TexCoord.y > 1.0f ) TexCoord.y -= 1.0f;
		
	float4 result = tex2D( diffuseSampler, TexCoord );
	float  alpha = tex2D( diffuseSampler, IN.texCoordDiffuse ).w;
	result.a = alpha * materialAmbient.a;
			
	return result;
}
//-----------------------------------







technique textured_0				
{
    pass p0 
    {		
    		VertexShader = compile vs_1_1 VS_TransformAndTexture();
		PixelShader  = compile ps_2_0 PS_Textured();
    }
}
