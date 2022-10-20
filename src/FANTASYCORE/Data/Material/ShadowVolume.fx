/*
 *	For Shadow Volume
 */
float4x4 worldViewProj				: WorldViewProjection;
shared float4x4 projection					: Projection;

float4x4 WorldView					: WorldView;

float4x3 matBone[50]					: WORLDMATRIXARRAY;		

float4 shadowViewlightDir : ShadowLightDirectionViewSpace
<
	string Object = "DirectionalLight";
    string Space = "World";
> = {1.0f, -1.0f, 1.0f, 0.0f};

//------------------------------------
struct vertexInput0 {
    float3 position				: POSITION;
    float3 normal				: NORMAL;
};

struct vertexInput1 {
    float3 position				: POSITION;
    float3 normal				: NORMAL;
    int    nBoneIndex			: BLENDINDICES0;
};

struct vertexInput2 {
    float3 position				: POSITION;
    float3 normal				: NORMAL;
	int4	   nBoneIndex		: BLENDINDICES;
	float4	   fWeight			: BLENDWEIGHT;
};

struct vertexInput3 {
    float3 position				: POSITION;
};

struct vertexOutput {
    float4 hPosition			: POSITION;
};

const float fExtrudeScaleMin = 100.0f;
const float fExtrudeScaleMax = 100000.0f;

vertexOutput VS_ShadowVolume_0(vertexInput0 IN) 
{
    // Compute view space normal
    
    vertexOutput OUT;
    float3 N = mul( IN.normal.xyz, WorldView );

    // Obtain view space position
    float4 PosView = mul( float4(IN.position,1), WorldView );

    // Perform reverse vertex extrusion
    // Extrude the vertex away from light if it's facing away from the light.
        
    if( dot( N, shadowViewlightDir ) > 0.0f )
    {        
            PosView.xyz += shadowViewlightDir.xyz * fExtrudeScaleMax;
            OUT.hPosition= mul( PosView, projection );
    }     
    else {    		
    		OUT.hPosition= mul( float4(IN.position, 1) , worldViewProj );    
    }
    
    return OUT;
}

vertexOutput VS_ShadowVolume_1(vertexInput1 IN) 
{
    vertexOutput OUT;
    float3 N = mul( IN.normal.xyz, matBone[IN.nBoneIndex]);
    N = normalize(N);

    // Obtain view space position
    float3 PosView = mul( float4(IN.position,1), matBone[IN.nBoneIndex]  );

    // Perform reverse vertex extrusion    
    // Extrude the vertex away from light if it's facing away from the light.    
    if( dot( N, shadowViewlightDir ) > 0.0f )
    {        
            PosView.xyz += shadowViewlightDir.xyz *fExtrudeScaleMax;
	}
    OUT.hPosition= mul( float4(PosView,1), projection );
    
    return OUT;
}

vertexOutput VS_ShadowVolume_2(vertexInput2 IN) 
{
    vertexOutput OUT;
    float3 N = mul( IN.normal.xyz, matBone[IN.nBoneIndex.x]);
    N = normalize(N);
    // Obtain view space position
    float3 PosView = mul( float4(IN.position,1), matBone[IN.nBoneIndex.x]  );

    // Perform reverse vertex extrusion
    // Extrude the vertex away from light if it's facing away from the light.
    float fZ = clamp(1-PosView.z, 0, 1);
    if( dot( N, shadowViewlightDir ) > 0.0f )
    {        
            PosView.xyz += shadowViewlightDir.xyz *lerp( fExtrudeScaleMin, fExtrudeScaleMax, fZ) ;
	}
    OUT.hPosition= mul( float4(PosView,1), projection );
    return OUT;
}

vertexOutput VS_ShadowVolumeRendering( vertexInput3 IN )
{
	vertexOutput OUT; 
	OUT.hPosition = float4(IN.position, 1);
	return OUT;
}

//-----------------------------------
float4 PS_Textured( vertexOutput IN): COLOR
{
	return float4(0,0,0,0.25);
}

// Xbox360 에서Pixel Shader NULL 로 하면  Fillrate 2배가 된다..
technique textured_0
{
    pass p0 
    {		
		VertexShader = compile vs_1_1 VS_ShadowVolume_0();
		PixelShader  = NULL;//compile ps_1_1 PS_Textured();
    }
}

technique textured_1
{
    pass p0 
    {		
		VertexShader = compile vs_1_1 VS_ShadowVolume_1();
		PixelShader  = NULL;//compile ps_1_1 PS_Textured();
    }
}

technique textured_2
{
    pass p0 
    {		
		VertexShader = compile vs_1_1 VS_ShadowVolume_2();
		PixelShader  = NULL;//compile ps_1_1 PS_Textured();
    }
}

technique textured_3
{
    pass p0 
    {		
		VertexShader = compile vs_1_1 VS_ShadowVolumeRendering();
		PixelShader  = compile ps_1_1 PS_Textured();
    }
}
