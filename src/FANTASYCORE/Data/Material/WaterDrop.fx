string description = "Screen Water Drop";

//------------------------------------


//------------------------------------

float4 vConst; // curtime, life
				  //  Pos.x Pos.y, size,

texture diffuseTexture : Diffuse
<
	string ResourceName = "default_color.dds";
>;

texture normTexture : Diffuse1
<
	string ResourceName1 = "default_color.dds";
>;

sampler TextureSampler = sampler_state 
{
    texture = <diffuseTexture>;
};

sampler NormTextureSampler = sampler_state 
{
    texture = <normTexture>;
};

struct vertexInputLine 
{
    float2 position		: POSITION;
    float3 texCoord		: TEXCOORD0;
};

struct vertexInputDrop
{
    float2 position		: POSITION;
    float2 texCoord		: TEXCOORD0;
};

struct vertexOutput 
{
    float4 hPosition		: POSITION;
    float3 texCoord		: TEXCOORD0;
};

//------------------------------------
vertexOutput VS_WaterLine(vertexInputLine IN) 
{
	vertexOutput OUT;
	OUT.hPosition = float4(IN.position.x, IN.position.y, 0.0 , 1.0);
	OUT.texCoord = float3( 1.0-IN.texCoord.x, IN.texCoord.y , 0.0);
    
	float fAlpha = (vConst.x - IN.texCoord.z)/vConst.y;
	clamp(fAlpha, 0.0, 1.0);
	OUT.texCoord.z = (1.0 - fAlpha)*0.9; // Alpha
	return OUT;
}

vertexOutput VS_WaterDrop(vertexInputDrop IN)
{
	vertexOutput OUT;
	float2 fScaled = float2(IN.position *vConst.z);
	fScaled += vConst.xy;	
	OUT.hPosition.x = IN.position.x*vConst.z+vConst.x;
	OUT.hPosition.y = IN.position.y*vConst.z+vConst.y;
	OUT.hPosition.z = 0.0;
	OUT.hPosition.w = 1.0;
	OUT.texCoord =  float3( IN.texCoord, 1.0);
	return OUT;
}

//-----------------------------------
float4 PS_Textured( vertexOutput IN): COLOR
{
  	float4 diffuseTexture = tex2D( TextureSampler, float2(IN.texCoord.xy) );
  	diffuseTexture.w =  IN.texCoord.z;
  	return diffuseTexture;
}


float4 PS_TexturedDrop( vertexOutput IN): COLOR
{
	float4	normTexture = 	tex2D( NormTextureSampler, IN.texCoord.xy );
	float4 diffuseTexture;
	if( normTexture.r  <= 0.4 || normTexture.r >=  0.6  ||
	    normTexture.g  <= 0.4  || normTexture.g >=  0.6 )
			diffuseTexture = tex2D( TextureSampler, float2(1.0-normTexture.rg) );
	else
  		diffuseTexture = float4(0.0, 0.0 , 0.0 ,0.0);
		
  	return diffuseTexture;
}


//-----------------------------------
technique WaterLine
{
    pass p0 
    {		
		VertexShader = compile vs_1_1 VS_WaterLine();
		PixelShader  = compile ps_1_4 PS_Textured();
    }
    
	
}

technique WaterDrop
{
	pass p0
	{
		VertexShader = compile vs_1_1 VS_WaterDrop();
		PixelShader  = compile ps_2_0 PS_TexturedDrop();
	}
}