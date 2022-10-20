/*
 *	No Texture, Directional Light, Per-Vertex
 */

//------------------------------------
shared float4x4 view						: View;
shared float4x4 projection					: Projection;

float4x4 WorldViewMat				: WorldView;

float fParticleTime : TimeValue0;
float fResistTime	: TimeValue1;
float fResistLength	: TimeValue2;
float fLifeTime		: TimeValue3;
float fGravityAccel	: FloatValue0;
float4 GravityVec	: Vector4Value0;
float4 Origin		: Vector4Value1;

float fSpinStart	: FloatValue1;
float fSpinRange	: FloatValue2;
float fScaleTable[21] : FloatValue3;
float4 ColorTable[21] : Vector4Value2;

float fParticleScale : FloatValue4;
float4 VertexOffset[4] : FloatValue;
float4 UVTable[4]	: FloatValue5;
float4 SinTable[64] : FloatValue6;
float4 ParticleColor : Vector4Value3;
float fTextureRepeat : FloatValue7;

texture diffuseTexture : Diffuse0;
texture BackBuffer : Diffuse1;

//------------------------------------
struct VertexInput0 {
    float3 Direction			: NORMAL;
    float  fBirthTime			: TEXCOORD0;
    int2   nVertexIndex		: BLENDINDICES;
};

struct VertexInput1 {
    float3 Direction			: NORMAL0;
    float  fBirthTime			: TEXCOORD0;
    int2   nVertexIndex		: BLENDINDICES;
    float3 Position				: POSITION;
    float3 Direction2			: NORMAL1;
};

struct VertexOutput {
    float4 Position				: POSITION;
    float4 texCoordDiffuse		: TEXCOORD0;
    float4 DiffuseColor			: COLOR0;
};

struct VertexOutputAlpha {
    float4 Position				: POSITION;
    float4 texCoordDiffuse		: TEXCOORD0;
    float4 texCoordScreen		: TEXCOORD1;
    float4 DiffuseColor			: COLOR0;
};

//------------------------------------
VertexOutput VS_ParticleNormal(VertexInput0 IN) 
{
    VertexOutput OUT;
    float4 Direction;
    float4 Gravity;
    float fSpinResult;
    float fScaleResult;
    float fTablePrev;
    float fTableNext;
    float fTableWeight;
    float4 Position;
    float fTime;
    float fOriginalTime;
    float fHalfTime;
    float fGravityWeight;
    float fSintableIndex;
    
    Direction=mul(float4(IN.Direction, 0.0), WorldViewMat);
    Gravity=mul(GravityVec, WorldViewMat);
    fOriginalTime=frac(fParticleTime-IN.fBirthTime);
    
    if(fOriginalTime>fResistTime){
		fTime=fOriginalTime-(fOriginalTime-fResistTime)*fResistLength;
    }
    else{
		fTime=fOriginalTime;
    }
    fTime=fTime*fLifeTime;
    fHalfTime=fTime*0.5f;
    fGravityWeight=pow(fGravityAccel, fHalfTime+1);
	Direction=Direction*fTime+Origin;    
	Gravity=Gravity*fTime;
	Direction=fGravityWeight*Gravity+Direction;
	
	fSpinResult=(fSpinRange*fOriginalTime+fSpinStart)*IN.nVertexIndex.y;
	fTablePrev=floor(fOriginalTime*20);
	fTableNext=floor(fOriginalTime*20)+1;
	fTableWeight=frac(fOriginalTime*20);
	fScaleResult=lerp(fScaleTable[fTablePrev], fScaleTable[fTableNext], fTableWeight)*fParticleScale;
	
	Position=VertexOffset[IN.nVertexIndex.x]*fScaleResult;
	OUT.texCoordDiffuse=UVTable[IN.nVertexIndex.x];
	
	fSintableIndex=frac(fSpinResult/360.0f)*64.0f;
	Direction.x=Direction.x+dot(Position, SinTable[fSintableIndex].xy);
	Direction.y=Direction.y+dot(Position, SinTable[fSintableIndex].zw);
	
	OUT.Position=mul(Direction, projection);

	OUT.DiffuseColor=lerp(ColorTable[fTablePrev], ColorTable[fTableNext], fTableWeight)*ParticleColor;

	OUT.texCoordDiffuse.z=frac(fOriginalTime*fTextureRepeat);
	
    return OUT;
}

VertexOutput VS_ParticleIterate(VertexInput1 IN) 
{
    VertexOutput OUT;
    float4 Direction;
    float4 Gravity;
    float fSpinResult;
    float fScaleResult;
    float fTablePrev;
    float fTableNext;
    float fTableWeight;
    float4 Position;
    float fTime;
    float fOriginalTime;
    float fHalfTime;
    float fGravityWeight;
    float fSintableIndex;
    
    float4 Pos;
    
    Direction=mul(float4(IN.Direction2, 0.0), view);
    Gravity=mul(GravityVec, view);
    fOriginalTime=frac(fParticleTime-IN.fBirthTime);
    
    if(fOriginalTime>fResistTime){
		fTime=fOriginalTime-(fOriginalTime-fResistTime)*fResistLength;
    }
    else{
		fTime=fOriginalTime;
    }
    fTime=fTime*fLifeTime;
    fHalfTime=fTime*0.5f;
    fGravityWeight=pow(fGravityAccel, fHalfTime+1);
    Pos=mul(float4(IN.Position, 1.0), view);
	Direction=Direction*fTime+Pos;
	Gravity=Gravity*fTime;
	Direction=fGravityWeight*Gravity+Direction;
	
	fSpinResult=(fSpinRange*fOriginalTime+fSpinStart)*IN.nVertexIndex.y;
	fTablePrev=floor(fOriginalTime*20);
	fTableNext=floor(fOriginalTime*20)+1;
	fTableWeight=frac(fOriginalTime*20);
	fScaleResult=lerp(fScaleTable[fTablePrev], fScaleTable[fTableNext], fTableWeight)*fParticleScale;
	
	Position=VertexOffset[IN.nVertexIndex.x]*fScaleResult;
	OUT.texCoordDiffuse=UVTable[IN.nVertexIndex.x];
	
	fSintableIndex=frac(fSpinResult/360.0f)*64.0f;
	Direction.x=Direction.x+dot(Position, SinTable[fSintableIndex].xy);
	Direction.y=Direction.y+dot(Position, SinTable[fSintableIndex].zw);
	
	OUT.Position=mul(Direction, projection);

	OUT.DiffuseColor=lerp(ColorTable[fTablePrev], ColorTable[fTableNext], fTableWeight)*ParticleColor;

	OUT.texCoordDiffuse.z=frac(fOriginalTime*fTextureRepeat);
	
    return OUT;
}

VertexOutputAlpha VS_ParticleNormalAlpha(VertexInput0 IN) 
{
    VertexOutputAlpha OUT;
    float4 Direction;
    float4 Gravity;
    float fSpinResult;
    float fScaleResult;
    float fTablePrev;
    float fTableNext;
    float fTableWeight;
    float4 Position;
    float fTime;
    float fOriginalTime;
    float fHalfTime;
    float fGravityWeight;
    float fSintableIndex;
    
    Direction=mul(float4(IN.Direction, 0.0), WorldViewMat);
    Gravity=mul(GravityVec, WorldViewMat);
    fOriginalTime=frac(fParticleTime-IN.fBirthTime);
    
    if(fOriginalTime>fResistTime){
		fTime=fOriginalTime-(fOriginalTime-fResistTime)*fResistLength;
    }
    else{
		fTime=fOriginalTime;
    }
    fTime=fTime*fLifeTime;
    fHalfTime=fTime*0.5f;
    fGravityWeight=pow(fGravityAccel, fHalfTime+1);
	Direction=Direction*fTime+Origin;    
	Gravity=Gravity*fTime;
	Direction=fGravityWeight*Gravity+Direction;
	
	fSpinResult=(fSpinRange*fOriginalTime+fSpinStart)*IN.nVertexIndex.y;
	fTablePrev=floor(fOriginalTime*20);
	fTableNext=floor(fOriginalTime*20)+1;
	fTableWeight=frac(fOriginalTime*20);
	fScaleResult=lerp(fScaleTable[fTablePrev], fScaleTable[fTableNext], fTableWeight)*fParticleScale;
	
	Position=VertexOffset[IN.nVertexIndex.x]*fScaleResult;
	OUT.texCoordDiffuse=UVTable[IN.nVertexIndex.x];
	
	fSintableIndex=frac(fSpinResult/360.0f)*64.0f;
	Direction.x=Direction.x+dot(Position, SinTable[fSintableIndex].xy);
	Direction.y=Direction.y+dot(Position, SinTable[fSintableIndex].zw);
	
	OUT.Position=mul(Direction, projection);
	
    OUT.texCoordScreen.x = (OUT.Position.x / OUT.Position.w)*0.5f + 0.5f;
    OUT.texCoordScreen.y = (OUT.Position.y / OUT.Position.w)*(-0.5f) + 0.5f ;
    OUT.texCoordScreen.z = 0.0f;
    OUT.texCoordScreen.w = 0.0f;

	OUT.DiffuseColor=lerp(ColorTable[fTablePrev], ColorTable[fTableNext], fTableWeight)*ParticleColor;

	OUT.texCoordDiffuse.z=frac(fOriginalTime*fTextureRepeat);
	
    return OUT;
}

VertexOutputAlpha VS_ParticleIterateAlpha(VertexInput1 IN) 
{
    VertexOutputAlpha OUT;
    float4 Direction;
    float4 Gravity;
    float fSpinResult;
    float fScaleResult;
    float fTablePrev;
    float fTableNext;
    float fTableWeight;
    float4 Position;
    float fTime;
    float fOriginalTime;
    float fHalfTime;
    float fGravityWeight;
    float fSintableIndex;
    
    float4 Pos;
    
    Direction=mul(float4(IN.Direction2, 0.0), view);
    Gravity=mul(GravityVec, view);
    fOriginalTime=frac(fParticleTime-IN.fBirthTime);
    
    if(fOriginalTime>fResistTime){
		fTime=fOriginalTime-(fOriginalTime-fResistTime)*fResistLength;
    }
    else{
		fTime=fOriginalTime;
    }
    fTime=fTime*fLifeTime;
    fHalfTime=fTime*0.5f;
    fGravityWeight=pow(fGravityAccel, fHalfTime+1);
    Pos=mul(float4(IN.Position, 1.0), view);
	Direction=Direction*fTime+Pos;
	Gravity=Gravity*fTime;
	Direction=fGravityWeight*Gravity+Direction;
	
	fSpinResult=(fSpinRange*fOriginalTime+fSpinStart)*IN.nVertexIndex.y;
	fTablePrev=floor(fOriginalTime*20);
	fTableNext=floor(fOriginalTime*20)+1;
	fTableWeight=frac(fOriginalTime*20);
	fScaleResult=lerp(fScaleTable[fTablePrev], fScaleTable[fTableNext], fTableWeight)*fParticleScale;
	
	Position=VertexOffset[IN.nVertexIndex.x]*fScaleResult;
	OUT.texCoordDiffuse=UVTable[IN.nVertexIndex.x];
	
	fSintableIndex=frac(fSpinResult/360.0f)*64.0f;
	Direction.x=Direction.x+dot(Position, SinTable[fSintableIndex].xy);
	Direction.y=Direction.y+dot(Position, SinTable[fSintableIndex].zw);
	
	OUT.Position=mul(Direction, projection);
    OUT.texCoordScreen.x = (OUT.Position.x / OUT.Position.w)*0.5f + 0.5f;
    OUT.texCoordScreen.y = (OUT.Position.y / OUT.Position.w)*(-0.5f) + 0.5f ;
    OUT.texCoordScreen.z = 0.0f;
    OUT.texCoordScreen.w = 0.0f;

	OUT.DiffuseColor=lerp(ColorTable[fTablePrev], ColorTable[fTableNext], fTableWeight)*ParticleColor;

	OUT.texCoordDiffuse.z=frac(fOriginalTime*fTextureRepeat);
	
    return OUT;
}

sampler TextureSampler = sampler_state 
{
    texture = <diffuseTexture>;
//    AddressU  = CLAMP;        
//    AddressV  = CLAMP;
//    AddressW  = CLAMP;
//    MIPFILTER = LINEAR;
//    MINFILTER = LINEAR;
//    MAGFILTER = LINEAR;
};

sampler ScreenSampler = sampler_state 
{
    texture = <BackBuffer>;
//    AddressU  = WRAP;        
//    AddressV  = WRAP;
//    AddressW  = WRAP;
};

//-----------------------------------
float4 PS_Textured( VertexOutput IN): COLOR
{
  float4 diffuseTexture = tex3D( TextureSampler, IN.texCoordDiffuse );
  return IN.DiffuseColor * diffuseTexture;
//  return IN.DiffuseColor;
}

float4 PS_TexturedAlpha( VertexOutputAlpha IN): COLOR
{
  float4 OutTexture;
  float4 diffuseTexture = tex3D( TextureSampler, IN.texCoordDiffuse );
  diffuseTexture = diffuseTexture - float4( 0.5, 0.5, 0.5, 0.0 );
  OutTexture = tex2D( ScreenSampler, IN.texCoordScreen + diffuseTexture );
  float4 ResultColor = IN.DiffuseColor * OutTexture;
  ResultColor.rgb += diffuseTexture.b * 2;
  ResultColor.a *= diffuseTexture.a;
  ResultColor = clamp( ResultColor, float4(0, 0, 0, 0), float4(1, 1, 1, 1) );
  return ResultColor;
//  return IN.DiffuseColor;
}


//-----------------------------------
technique textured_0
{
    pass p0 
    {		
		VertexShader = compile vs_2_0 VS_ParticleNormal();
		AlphaTestEnable = true;
		AlphaBlendEnable=true;
		ZWriteEnable=false;
		AlphaFunc = GREATEREQUAL;
		AlphaRef = 0x01;
		PixelShader  = compile ps_2_0 PS_Textured();
    }
}

technique textured_1
{
    pass p0 
    {		
		VertexShader = compile vs_2_0 VS_ParticleIterate();
		AlphaTestEnable = true;
		AlphaBlendEnable=true;
		ZWriteEnable=false;
		AlphaFunc = GREATEREQUAL;
		AlphaRef = 0x01;
		PixelShader  = compile ps_2_0 PS_Textured();
    }
}

technique textured_2
{
    pass p0 
    {		
		VertexShader = compile vs_2_0 VS_ParticleNormalAlpha();
		AlphaTestEnable = true;
		AlphaBlendEnable=true;
		ZWriteEnable=false;
		AlphaFunc = GREATEREQUAL;
		AlphaRef = 0x01;
		PixelShader  = compile ps_2_0 PS_TexturedAlpha();
    }
}

technique textured_3
{
    pass p0 
    {		
		VertexShader = compile vs_2_0 VS_ParticleIterateAlpha();
		AlphaTestEnable = true;
		AlphaBlendEnable=true;
		ZWriteEnable=false;
		AlphaFunc = GREATEREQUAL;
		AlphaRef = 0x01;
		PixelShader  = compile ps_2_0 PS_TexturedAlpha();
    }
}
