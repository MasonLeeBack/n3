//------------------------------------
texture LayerTexture : Diffuse
<
	string ResourceName = "default_color.dds";
>;

texture VolumnTexture : Diffuse1
<
	string ResourceName = "test.dds";
>;

float fPos_X : POS_X = 0.5;
float fPos_Y : POS_Y = 0.5;

float fMeshOffset0 : MESH_OFFSET0 = 0.f;
float fMeshOffset1 : MESH_OFFSET1 = 0.f;
float fMeshOffset2 : MESH_OFFSET2 = 0.f;
float fMeshOffset3 : MESH_OFFSET3 = 0.f;
float4 fMeshColor : MESH_COLOR = float4(1,1,1,1);

//------------------------------------
struct vertexInputMesh {
    float3 position			: POSITION;
    float2 texUV			: TEXCOORD0;
};

struct vertexOutput {
    float4 hPosition		: POSITION;
    float3 Layer0UV		: TEXCOORD0;
    float4 hColor			: COLOR0;
};

//------------------------------------
vertexOutput VS_TransformAndTexture_0(vertexInputMesh IN) 
{
	vertexOutput OUT;
	
	OUT.hPosition = float4(IN.position.xyz, 1.0);
	OUT.hPosition.x = (OUT.hPosition.x * fMeshOffset0) * 720.f/1280.f + fPos_X;
	OUT.hPosition.y = (OUT.hPosition.y * fMeshOffset1) + fPos_Y;
	OUT.hPosition.z = 0.f;
	
	OUT.Layer0UV.xy = IN.texUV;
	OUT.Layer0UV.z = 0.f;
    
    OUT.hColor = fMeshColor;
    return OUT;
}

vertexOutput VS_TransformAndTexture_1(vertexInputMesh IN)
{
	vertexOutput OUT;

	OUT.hPosition = float4(IN.position.xyz, 1.0);
	OUT.hPosition.x = (OUT.hPosition.x * fMeshOffset0)*720.f/1280.f + fPos_X;
	OUT.hPosition.y = (OUT.hPosition.y * fMeshOffset1) + fPos_Y;
	OUT.hPosition.z = 0.f;
	
	OUT.Layer0UV.xy = IN.texUV;
	OUT.Layer0UV.x = (OUT.Layer0UV.x/2.f) + 0.5f + fMeshOffset2;
	OUT.Layer0UV.z = fMeshOffset3;
    
    OUT.hColor = fMeshColor;
    return OUT;
}

//-----------------------------------
sampler LayerSampler = sampler_state 
{
    texture = <LayerTexture>;
    AddressU  = WRAP;        
    AddressV  = WRAP;
    AddressW  = WRAP;
    MINFILTER = LINEAR;
    MAGFILTER = LINEAR;
};

sampler3D VolumnSampler = sampler_state 
{
    texture = <VolumnTexture>;
    AddressU  = WRAP;        
    AddressV  = WRAP;
    AddressW  = WRAP;
    MINFILTER = LINEAR;
    MAGFILTER = LINEAR;
};


//------------------------------------
float4 PS_Textured(vertexOutput IN): COLOR
{	
	float4 Lay0 = tex2D(LayerSampler, IN.Layer0UV);
	return Lay0 *= IN.hColor;
}

float4 PS_Textured_Volumn(vertexOutput IN): COLOR
{	
	float4 Lay0 = tex3D(VolumnSampler, IN.Layer0UV);
	return Lay0 *= IN.hColor;
}


//-----------------------------------
technique textured_0
{
    pass p0 
    {
		VertexShader = compile vs_1_1 VS_TransformAndTexture_0();
		PixelShader  = compile ps_2_0 PS_Textured();
    }
}

technique textured_1
{
    pass p0 
    {
		VertexShader = compile vs_1_1 VS_TransformAndTexture_1();
		PixelShader  = compile ps_2_0 PS_Textured();
    }
}

technique textured_2
{
    pass p0 
    {
		VertexShader = compile vs_1_1 VS_TransformAndTexture_1();
		PixelShader  = compile ps_2_0 PS_Textured_Volumn();
    }
}