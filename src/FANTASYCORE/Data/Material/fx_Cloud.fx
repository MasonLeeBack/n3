/*
 *	Diffuse, Directional Light, Per-Vertex
 */

//------------------------------------
shared float4x4 projection			: Projection;

float4x4 worldviewInverseTranspose	: WorldViewInverseTranspose;
float4x4 WorldView					: WorldView;

float4x3 matBone[50]				: WORLDMATRIXARRAY;

// -FOG-

float fTime : TIME;

texture diffuseTexture : Volume
<
	string ResourceName = "default_color.dds";
>;

shared float4 viewlightDir : LightDirectionViewSpace
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
    int4    nBoneIndex			: BLENDINDICES0;
};

struct vertexInput2 {
    float3 position				: POSITION;
    float3 normal				: NORMAL;
	int4	   nBoneIndex			: BLENDINDICES;
	float4	   fWeight			: BLENDWEIGHT;
};

struct vertexOutput {
    float4 hPosition		: POSITION;
    float4 TexCoord			: TEXCOORD0;
    float  PSize            : PSIZE;
};


float fPsize  : fx_custom0;
float fAlpha : fx_custom1;
//------------------------------------
vertexOutput VS_TransformAndTexture_0(vertexInput0 IN) 
{
    vertexOutput OUT;
    	
	float3 Pos =  mul( float4(IN.position.xyz , 1.0) , WorldView);
	//calculate our vectors N, E, L, and H
	// 월드에서 라이트 계산!!
	float4 N = mul(IN.normal, worldviewInverseTranspose); //normal vector
	N.xyz = normalize(N.xyz);
	float3 L = ( -viewlightDir.xyz); //light vector
		//calculate the diffuse
	float  diff = max(0 , dot(N,L));
	OUT.hPosition = mul( float4(Pos , 1.0) , projection);
	OUT.TexCoord = float4(diff, 0.f, 0.f, 0.f);
	
	float fSum = Pos.x + Pos.y +Pos.z;
	OUT.TexCoord.y = step( 25.f,  fmod( fSum, 50.f) )*0.5f;
	OUT.TexCoord.z =  step(10.f,  fmod( fSum, 20.f) )*0.5f;
		

	OUT.PSize   = fPsize/ OUT.hPosition.z;
    return OUT;
}

//------------------------------------
vertexOutput VS_TransformAndTexture_1(vertexInput1 IN) 
{
	vertexOutput OUT;
    
    // Matrix Array로 넘어오는것 WorldViewMatrix이다.
    float3 Pos = mul( float4(IN.position.xyz , 1.0), matBone[IN.nBoneIndex.x]) * 0.999;
    Pos += mul( float4(IN.position.xyz , 1.f), matBone[IN.nBoneIndex.y]) * 0.001;
    
    	//calculate our vectors N, E, L, and H
	// 뷰공간에서 라이트 계산
	float3 N = normalize(mul(IN.normal, matBone[IN.nBoneIndex.x])) * 0.999;		//normal vector
	N += normalize(mul(IN.normal, matBone[IN.nBoneIndex.y])) * 0.001;		//normal vector
	N = normalize(N);
   
	float3 L = ( -viewlightDir.xyz);					//light vector

	OUT.hPosition = mul( float4(Pos,1.f) , projection);
	//calculate the diffuse
	float  diff = max(0 , dot(N,L));
	OUT.TexCoord = float4(diff, 0.f, 0.f, 0.f);
   	float fSum = Pos.x + Pos.y +Pos.z;
	OUT.TexCoord.y = step( 25.f,  fmod( fSum, 50.f) )*0.5f;
	OUT.TexCoord.z =  step(10.f,  fmod( fSum, 20.f) )*0.5f;
     // Look dot Normal 

	OUT.PSize   = fPsize / OUT.hPosition.z;
	return OUT;
}

vertexOutput VS_TransformAndTexture_2(vertexInput2 IN) 
{
    vertexOutput OUT;
    // Matrix Array로 넘어오는것 WorldViewMatrix이다.
    float3 Pos = mul( float4(IN.position.xyz , 1.f), matBone[IN.nBoneIndex.x]) * IN.fWeight.x;
    Pos += mul( float4(IN.position.xyz , 1.f), matBone[IN.nBoneIndex.y]) * IN.fWeight.y;
        
    //calculate our vectors N, E, L, and H
	float3 N = mul(IN.normal, matBone[IN.nBoneIndex.x]) * IN.fWeight.x;		//normal vector
	N += mul(IN.normal, matBone[IN.nBoneIndex.y]) * IN.fWeight.y;
	N = normalize(N);
	
	float3 L = ( -viewlightDir.xyz);					//light vector

	//calculate the diffuse
	float  diff = max(0 , dot(N,L));
	OUT.hPosition = mul( float4(Pos,1.f) , projection);
    	OUT.TexCoord = float4(diff, 0.f, 0.f, 0.f);
	float fSum = Pos.x + Pos.y +Pos.z;
	OUT.TexCoord.y = step( 0.5f,  IN.fWeight.x )*0.5f;
	OUT.TexCoord.z =  step(0.5f,  IN.fWeight.y )*0.5f;

	OUT.PSize   = fPsize / OUT.hPosition.z;
    return OUT;
}

vertexOutput VS_TransformAndTexture_3(vertexInput2 IN) 
{
    vertexOutput OUT;
    // Matrix Array로 넘어오는것 WorldViewMatrix이다.
    float3 Pos = mul( float4(IN.position.xyz , 1.f), matBone[IN.nBoneIndex.x]) * IN.fWeight.x;
    Pos += mul( float4(IN.position.xyz , 1.f), matBone[IN.nBoneIndex.y]) * IN.fWeight.y;
    Pos += mul( float4(IN.position.xyz , 1.f), matBone[IN.nBoneIndex.z]) * IN.fWeight.z;

	//calculate our vectors N, E, L, and H
	float3 N = mul(IN.normal, matBone[IN.nBoneIndex.x]) * IN.fWeight.x;		//normal vector
	N += mul(IN.normal, matBone[IN.nBoneIndex.y]) * IN.fWeight.y;
	N += mul(IN.normal, matBone[IN.nBoneIndex.z]) * IN.fWeight.z;
	N = normalize(N);
    float3 L = ( -viewlightDir.xyz);					//light vector

	//calculate the diffuse
    float  diff = max(0 , dot(N,L));
    	OUT.hPosition = mul( float4(Pos,1.f) , projection);
    	OUT.TexCoord = float4(diff, 0.f, 0.f, 0.f);
    	float fSum = Pos.x + Pos.y +Pos.z;
	OUT.TexCoord.y = step( 0.5f,  IN.fWeight.x )*0.5f;
	OUT.TexCoord.z =  step(0.5f,  IN.fWeight.y )*0.5f;

	OUT.PSize   = fPsize / OUT.hPosition.z;
    return OUT;
}

vertexOutput VS_TransformAndTexture_4(vertexInput2 IN) 
{
    vertexOutput OUT;
    // Matrix Array로 넘어오는것 WorldViewMatrix이다.
    float3 Pos = mul( float4(IN.position.xyz , 1.f), matBone[IN.nBoneIndex.x]) * IN.fWeight.x;
    Pos += mul( float4(IN.position.xyz , 1.f), matBone[IN.nBoneIndex.y]) * IN.fWeight.y;
    Pos += mul( float4(IN.position.xyz , 1.f), matBone[IN.nBoneIndex.z]) * IN.fWeight.z;
    Pos += mul( float4(IN.position.xyz , 1.f), matBone[IN.nBoneIndex.w]) * IN.fWeight.w;
    	OUT.hPosition = mul( float4(Pos,1.f) , projection);
	
	//calculate our vectors N, E, L, and H
	float3 N = mul(IN.normal, matBone[IN.nBoneIndex.x]) * IN.fWeight.x;		//normal vector
	N += mul(IN.normal, matBone[IN.nBoneIndex.y]) * IN.fWeight.y;
	N += mul(IN.normal, matBone[IN.nBoneIndex.z]) * IN.fWeight.z;
	N += mul(IN.normal, matBone[IN.nBoneIndex.w]) * IN.fWeight.w;
	N = normalize(N);
    float3 L = ( -viewlightDir.xyz);					//light vector

	//calculate the diffuse
	float  diff = max(0 , dot(N,L));

	OUT.TexCoord = float4(diff, 0.f, 0.f, 0.f);
    	float fSum = Pos.x + Pos.y +Pos.z;
	OUT.TexCoord.y = step( 0.5f,  IN.fWeight.x )*0.5f;
	OUT.TexCoord.z =  step(0.5f,  IN.fWeight.y )*0.5f;
	
	OUT.PSize   = fPsize / OUT.hPosition.z;
    return OUT;
}


//------------------------------------
sampler3D TextureSampler = sampler_state 
{
    texture = <diffuseTexture>;
//    AddressU  = MIRROR;        
//    AddressV  = MIRROR;
//    AddressW  = WRAP;
};

//-----------------------------------
float4 PS_Textured(     const float2 SpriteCoord 	: SPRITETEXCOORD,
						const float4 TexCoord 		: TexCoord0 ): COLOR
{
	float2  fTex = SpriteCoord*(0.5f).xx + TexCoord.yz;
	float4 	texcoord= float4( fTex, TexCoord.x, 0.f);
	float4 diffuseTexture = tex3D( TextureSampler, 	texcoord );
	diffuseTexture.a *= 0.5f*fAlpha;
	return  diffuseTexture;
}

//-----------------------------------
technique textured_0				// No Physique
{
	pass p0 
	{		
//		POINTSPRITEENABLE = TRUE;
//		CULLMODE = NONE;
//		POINTSIZE_MAX = 256.f;
//		alphablendenable = TRUE;
//		srcblend = srcalpha;
//		destblend = invsrcalpha;
				
		VertexShader = compile vs_2_0 VS_TransformAndTexture_0();
		PixelShader  = compile ps_2_0 PS_Textured();
	}
}

technique textured_1				// 1 Bone Link
{
	pass p0 
	{	
//		POINTSPRITEENABLE = TRUE;
//		CULLMODE = NONE;
//		POINTSIZE_MAX = 256.f;
//		alphablendenable = TRUE;
//		srcblend = srcalpha;
//		destblend = invsrcalpha;
			
		VertexShader = compile vs_2_0 VS_TransformAndTexture_1();
		PixelShader  = compile ps_2_0 PS_Textured();
	}
}

technique textured_2				// 2 Bone Link
{
	pass p0 
	{	
//		POINTSPRITEENABLE = TRUE;
//		CULLMODE = NONE;
//		POINTSIZE_MAX = 256.f;
//		alphablendenable = TRUE;
//		srcblend = srcalpha;
//		destblend = invsrcalpha;
			
		VertexShader = compile vs_2_0 VS_TransformAndTexture_2();
		PixelShader  = compile ps_2_0 PS_Textured();
	}
}

technique textured_3				// 3 Bone Link
{
	pass p0 
	{		
//		POINTSPRITEENABLE = TRUE;
//		CULLMODE = NONE;
//		POINTSIZE_MAX = 256.f;
//		alphablendenable = TRUE;
//		srcblend = srcalpha;
//		destblend = invsrcalpha;
		
		VertexShader = compile vs_2_0 VS_TransformAndTexture_3();
		PixelShader  = compile ps_2_0 PS_Textured();
	}
}

technique textured_4				// 4 Bone Link
{
	pass p0 
	{	
//		POINTSPRITEENABLE = TRUE;
//		CULLMODE = NONE;
//		POINTSIZE_MAX = 256.f;
//		alphablendenable = TRUE;
//		srcblend = srcalpha;
//		destblend = invsrcalpha;
					
		VertexShader = compile vs_2_0 VS_TransformAndTexture_4();
		PixelShader  = compile ps_2_0 PS_Textured();
	}
}