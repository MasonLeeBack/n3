float4x4 worldMatrix				: World;
float4x4 wvpMatrix					: WorldViewProjection;	// Model*View*Projection
float4x4 viewInverseMatrix			: ViewInverse;

float4x4 projection					: Projection;
float4x3 matBone[50]				: WORLDMATRIXARRAY;			

// -FOG-
//float4x4 WorldView					: WorldView;
float4 fogFactor				    : FOGFACTOR; // fogEnd, forRange, ?, ?
float4 fogColor						: FOGCOLOR; // r,g,b,1

float time : Time < string UIWidget = "none"; >;

texture normalMap : Normal
<
	string ResourceName = "waves2.dds";
	string TextureType = "2D";
>;

texture cubeMap : Environment
<
	string ResourceName = "CloudyHillsCubemap2.dds";
	string TextureType = "Cube";
>;

sampler2D normalMapSampler = sampler_state
{
	Texture = <normalMap>;
	MagFilter = Linear;	
	MinFilter = Linear;
	MipFilter = Linear;
};

samplerCUBE envMapSampler = sampler_state
{
	Texture = <cubeMap>;
	MinFilter = Linear;
	MagFilter = Linear;
	MipFilter = Linear;
    AddressU = Clamp;
    AddressV = Clamp;
};

float bumpHeight
<
	string UIWidget = "slider";
	float UIMin = 0.0; float UIMax = 2.0; float UIStep = 0.01;
	string UIName = "Bump Height";
> = 1.2;

float4 textureScale : UVANIMATION_SCALE
<
    string UIName = "Texture scale";
> = { 0.8, 0.4 , 0.f, 0.f};

float4 bumpSpeed : UVANIMATION_POSITION
<
    string UIName = "Bumpmap translation speed";
> = { -0.05, 0.0, 0.0, 0.0 };

float fresnelBias
<
    string UIName = "Fresnel bias";
	string UIWidget = "slider";
	float UIMin = 0.0; float UIMax = 1.0; float UIStep = 0.01;
> = 0.1;

float fresnelPower
<
    string UIName = "Fresnel exponent";
	string UIWidget = "slider";
	float UIMin = 1.0; float UIMax = 10.0; float UIStep = 0.01;
> = 4.0;

float hdrMultiplier
<
    string UIName = "HDR multiplier";
	string UIWidget = "slider";
	float UIMin = 0.0; float UIMax = 100.0; float UIStep = 0.01;
> = 2.0;

float4 deepColor : MaterialAmbient
<
    string UIName = "Deep water color";
> = {0.8f, 0.0f, 0.0f, 1.0f};

float4 shallowColor : MaterialDiffuse
<
    string UIName = "Shallow water color";
> = {1.f, 0.2f, 0.2f, 1.0f};

float4 reflectionColor : MaterialSpecular
<
    string UIName = "Reflection color";
> = {0.5f, 0.5f, 0.5f, 0.5f};

// these are redundant, but makes the ui easier:
float reflectionAmount
<
    string UIName = "Reflection amount";
	string UIWidget = "slider";    
	float UIMin = 0.0; float UIMax = 2.0; float UIStep = 0.01;    
> = 1.0f;

float waterAmount
<
    string UIName = "Water color amount";
	string UIWidget = "slider";    
	float UIMin = 0.0; float UIMax = 2.0; float UIStep = 0.01;    
> = 1.0f;

float waveAmp
<
    string UIName = "Wave amplitude";
	string UIWidget = "slider";
	float UIMin = 0.0; float UIMax = 10.0; float UIStep = 0.1;
> = 1.0;

float waveFreq
<
    string UIName = "Wave frequency";
	string UIWidget = "slider";
	float UIMin = 0.0; float UIMax = 1.0; float UIStep = 0.001;
> = 0.1;



struct vertexInput0 {
	float4 Position : POSITION;   // in object space
	float2 TexCoord : TEXCOORD0;
	float3 Tangent  : TEXCOORD1;
	float3 Binormal : TEXCOORD2;
	float3 Normal   : NORMAL;
};

struct vertexInput1 {
	float4 Position : POSITION;   // in object space
	float2 TexCoord : TEXCOORD0;
	float3 Tangent  : TEXCOORD1;
	float3 Binormal : TEXCOORD2;
	float3 Normal   : NORMAL;
    int    nBoneIndex			: BLENDINDICES0;
};

struct vertexInput2 {
	float3 Position : POSITION;   // in object space
	float2 TexCoord : TEXCOORD0;
	float3 Tangent  : TEXCOORD1;
	float3 Binormal : TEXCOORD2;
	float3 Normal   : NORMAL;
	int2	   nBoneIndex			: BLENDINDICES;
	float2	   fWeight			: BLENDWEIGHT;
};


struct v2f {
	float4 Position   : POSITION;  // in clip space
	float2 TexCoord   : TEXCOORD0;
	float3 TexCoord1  : TEXCOORD1; // first row of the 3x3 transform from tangent to cube space
	float3 TexCoord2  : TEXCOORD2; // second row of the 3x3 transform from tangent to cube space
	float3 TexCoord3  : TEXCOORD3; // third row of the 3x3 transform from tangent to cube space

	float2 bumpCoord0 : TEXCOORD4;
	float2 bumpCoord1 : TEXCOORD5;
	float2 bumpCoord2 : TEXCOORD6;
	
	float3 eyeVector  : TEXCOORD7;
	
	float  FogValue	  	  : COLOR1; // -FOG-
};

// wave functions

struct Wave {
  float freq;  // 2*PI / wavelength
  float amp;   // amplitude
  float phase; // speed * 2*PI / wavelength
  float2 dir;
};

#define NWAVES 2
Wave wave[NWAVES] = {
	{ 1.0, 1.0, 0.5, float2(-1, 0) },
	{ 2.0, 0.5, 1.3, float2(-0.7, 0.7) }	
};

float evaluateWave(Wave w, float2 pos, float t)
{
  return w.amp * sin( dot(w.dir, pos)*w.freq + t*w.phase);
}

// derivative of wave function
float evaluateWaveDeriv(Wave w, float2 pos, float t)
{
  return w.freq*w.amp * cos( dot(w.dir, pos)*w.freq + t*w.phase);
}

// sharp wave functions
float evaluateWaveSharp(Wave w, float2 pos, float t, float k)
{
  return w.amp * pow(sin( dot(w.dir, pos)*w.freq + t*w.phase)* 0.5 + 0.5 , k);
}

float evaluateWaveDerivSharp(Wave w, float2 pos, float t, float k)
{
  return k*w.freq*w.amp * pow(sin( dot(w.dir, pos)*w.freq + t*w.phase)* 0.5 + 0.5 , k - 1) * cos( dot(w.dir, pos)*w.freq + t*w.phase);
}

v2f BumpReflectWaveVS_0(vertexInput0 IN,
					  uniform float BumpScale,
					  uniform float2 textureScale,
					  uniform float2 bumpSpeed,
					  uniform float time,
					  uniform float waveFreq,
					  uniform float waveAmp
                	  )
{
	v2f OUT;

    wave[0].freq = waveFreq;
    wave[0].amp = waveAmp;

    wave[1].freq = waveFreq*2.0;
    wave[1].amp = waveAmp*0.5;

    float4 P = IN.Position;

	// sum waves	
//	P.y = 0.0;
	float ddx = 0.0, ddy = 0.0;
	for(int i=0; i<NWAVES; i++) {
    	P.y += 1.5f*evaluateWave(wave[i], P.xz, 2.f*time);
    	float deriv = evaluateWaveDeriv(wave[i], P.xz, time);
    	ddx += deriv * wave[i].dir.x;
    	ddy += deriv * wave[i].dir.y;
    }
	// compute tangent basis
    float3 B = float3(1, ddx, 0);
    float3 T = float3(0, ddy, 1);
    float3 N = float3(-ddx, 1, -ddy);

	OUT.Position = mul(P, wvpMatrix);
	
	// pass texture coordinates for fetching the normal map
	OUT.TexCoord.xy = IN.TexCoord*textureScale;

	time = fmod(time, 100.0);
	OUT.bumpCoord0.xy = IN.TexCoord*textureScale + time*bumpSpeed;
	OUT.bumpCoord1.xy = IN.TexCoord*textureScale*2.0 + time*bumpSpeed*4.0;
	OUT.bumpCoord2.xy = IN.TexCoord*textureScale*4.0 + time*bumpSpeed*8.0;

	// compute the 3x3 tranform from tangent space to object space
	float3x3 objToTangentSpace;
	// first rows are the tangent and binormal scaled by the bump scale
	objToTangentSpace[0] = BumpScale * normalize(T);
	objToTangentSpace[1] = BumpScale * normalize(B);
//	objToTangentSpace[2] = normalize(N);
	objToTangentSpace[2] = IN.Normal;

	OUT.TexCoord1.xyz = mul(objToTangentSpace, worldMatrix[0].xyz);
	OUT.TexCoord2.xyz = mul(objToTangentSpace, worldMatrix[1].xyz);
	OUT.TexCoord3.xyz = mul(objToTangentSpace, worldMatrix[2].xyz);

	// compute the eye vector (going from shaded point to eye) in cube space
	float4 worldPos = mul(P, worldMatrix);
	OUT.eyeVector = viewInverseMatrix[3] - worldPos; // view inv. transpose contains eye position in world space in last row
	
	// -FOG-
	float fDist = fogFactor.x - OUT.Position.z; // x=fogEnd
	OUT.FogValue = clamp ( fDist*fogFactor.y, 0.0f, 1.0f ); // y=forRange
	
	return OUT;
}

v2f BumpReflectWaveVS_1(vertexInput1 IN,
					  uniform float BumpScale,
					  uniform float2 textureScale,
					  uniform float2 bumpSpeed,
					  uniform float time,
					  uniform float waveFreq,
					  uniform float waveAmp
                	  )
{
	v2f OUT;

    wave[0].freq = waveFreq;
    wave[0].amp = waveAmp;

    wave[1].freq = waveFreq*2.0;
    wave[1].amp = waveAmp*0.5;

    float3 P = IN.Position.xyz;

	// sum waves	
	float ddx = 0.0, ddy = 0.0;
	for(int i=0; i<NWAVES; i++) {
    	P.y += 1.5f*evaluateWave(wave[i], P.xz, 2.f*time);
    	float deriv = evaluateWaveDeriv(wave[i], P.xz, time);
    	ddx += deriv * wave[i].dir.x;
    	ddy += deriv * wave[i].dir.y;
    }
	// compute tangent basis
    float3 B = float3(1, ddx, 0);
    float3 T = float3(0, ddy, 1);
    float3 N = float3(-ddx, 1, -ddy);

	float3 tempPos = mul( float4(P, 1.f), matBone[IN.nBoneIndex]);
	OUT.Position =  mul( float4(tempPos, 1.f) , projection);
	
	// pass texture coordinates for fetching the normal map
	OUT.TexCoord.xy = IN.TexCoord*textureScale;

	time = fmod(time, 100.0);
	OUT.bumpCoord0.xy = IN.TexCoord*textureScale + time*bumpSpeed;
	OUT.bumpCoord1.xy = IN.TexCoord*textureScale*2.0 + time*bumpSpeed*4.0;
	OUT.bumpCoord2.xy = IN.TexCoord*textureScale*4.0 + time*bumpSpeed*8.0;

	// compute the 3x3 tranform from tangent space to object space
	float3x3 objToTangentSpace;
	// first rows are the tangent and binormal scaled by the bump scale
	objToTangentSpace[0] = BumpScale * normalize(T);
	objToTangentSpace[1] = BumpScale * normalize(B);
	objToTangentSpace[2] = IN.Normal;

	OUT.TexCoord1.xyz = mul(objToTangentSpace, worldMatrix[0].xyz);
	OUT.TexCoord2.xyz = mul(objToTangentSpace, worldMatrix[1].xyz);
	OUT.TexCoord3.xyz = mul(objToTangentSpace, worldMatrix[2].xyz);

	// compute the eye vector (going from shaded point to eye) in cube space
	float4 worldPos = mul(P, worldMatrix);
	OUT.eyeVector = viewInverseMatrix[3] - worldPos; // view inv. transpose contains eye position in world space in last row
	
	// -FOG-
	float fDist = fogFactor.x - tempPos.z; // x=fogEnd
	OUT.FogValue = clamp ( fDist*fogFactor.y, 0.0f, 1.0f ); // y=forRange
	
	return OUT;
}

v2f BumpReflectWaveVS_2(vertexInput2 IN,
					  uniform float BumpScale,
					  uniform float2 textureScale,
					  uniform float2 bumpSpeed,
					  uniform float time,
					  uniform float waveFreq,
					  uniform float waveAmp
                	  )
{
	v2f OUT;

    wave[0].freq = waveFreq;
    wave[0].amp = waveAmp;

    wave[1].freq = waveFreq*2.0;
    wave[1].amp = waveAmp*0.5;

    float3 P = IN.Position.xyz;

	// sum waves	
//	P.y = 0.0;
	float ddx = 0.0, ddy = 0.0;
	for(int i=0; i<NWAVES; i++) {
    	P.y += 1.5f*evaluateWave(wave[i], P.xz, 2.f*time);
    	float deriv = evaluateWaveDeriv(wave[i], P.xz, time);
    	ddx += deriv * wave[i].dir.x;
    	ddy += deriv * wave[i].dir.y;
	}
	// compute tangent basis
    float3 B = float3(1, ddx, 0);
    float3 T = float3(0, ddy, 1);
    float3 N = float3(-ddx, 1, -ddy);

	float3 tempPos = mul( float4(P, 1.f), matBone[IN.nBoneIndex.x]) * IN.fWeight.x;
	tempPos += mul( float4(P, 1.f), matBone[IN.nBoneIndex.y]) * IN.fWeight.y;
	OUT.Position =  mul( float4(tempPos, 1.f) , projection);
	
	// pass texture coordinates for fetching the normal map
	OUT.TexCoord.xy = IN.TexCoord*textureScale;

	time = fmod(time, 100.0);
	OUT.bumpCoord0.xy = IN.TexCoord*textureScale + time*bumpSpeed;
	OUT.bumpCoord1.xy = IN.TexCoord*textureScale*2.0 + time*bumpSpeed*4.0;
	OUT.bumpCoord2.xy = IN.TexCoord*textureScale*4.0 + time*bumpSpeed*8.0;

	// compute the 3x3 tranform from tangent space to object space
	float3x3 objToTangentSpace;
	// first rows are the tangent and binormal scaled by the bump scale
	objToTangentSpace[0] = BumpScale * normalize(T);
	objToTangentSpace[1] = BumpScale * normalize(B);
	objToTangentSpace[2] = IN.Normal;

	OUT.TexCoord1.xyz = mul(objToTangentSpace, worldMatrix[0].xyz);
	OUT.TexCoord2.xyz = mul(objToTangentSpace, worldMatrix[1].xyz);
	OUT.TexCoord3.xyz = mul(objToTangentSpace, worldMatrix[2].xyz);

	// compute the eye vector (going from shaded point to eye) in cube space
	float4 worldPos = mul(P, worldMatrix);
	OUT.eyeVector = viewInverseMatrix[3] - worldPos; // view inv. transpose contains eye position in world space in last row
	
	// -FOG-
	float fDist = fogFactor.x - tempPos.z; // x=fogEnd
	OUT.FogValue = clamp ( fDist*fogFactor.y, 0.0f, 1.0f ); // y=forRange
	
	return OUT;
}



// Pixel Shaders

float4 BumpReflectPS20(v2f IN,
					   uniform sampler2D NormalMap,
					   uniform samplerCUBE EnvironmentMap) : COLOR
{
	// fetch the bump normal from the normal map
	float4 N = tex2D(NormalMap, IN.TexCoord.xy)*2.0 - 1.0;
	
	float3x3 m; // tangent to world matrix
	m[0] = IN.TexCoord1;
	m[1] = IN.TexCoord2;
	m[2] = IN.TexCoord3;
	float3 Nw = mul(m, N.xyz);
	
	float3 E = IN.eyeVector;
    float3 R = reflect(-E, Nw);

	return texCUBE(EnvironmentMap, R);
}

float4 OceanPS20(v2f IN,
				 uniform sampler2D NormalMap,
				 uniform samplerCUBE EnvironmentMap,
				 uniform half4 deepColor,
				 uniform half4 shallowColor,
				 uniform half4 reflectionColor,
				 uniform half4 reflectionAmount,
				 uniform half4 waterAmount,
				 uniform half fresnelPower,
				 uniform half fresnelBias,
				 uniform half hdrMultiplier
				 ) : COLOR
{
	// sum normal maps
    half4 t0 = tex2D(NormalMap, IN.bumpCoord0.xy)*2.0-1.0;
    half4 t1 = tex2D(NormalMap, IN.bumpCoord1.xy)*2.0-1.0;
    half4 t2 = tex2D(NormalMap, IN.bumpCoord2.xy)*2.0-1.0;
    half3 N = t0.xyz + t1.xyz + t2.xyz;

    half3x3 m; // tangent to world matrix
    m[0] = IN.TexCoord1;
    m[1] = IN.TexCoord2;
    m[2] = IN.TexCoord3;
    half3 Nw = mul(m, N.xyz);
    Nw = normalize(Nw);

	// reflection
    float3 E = normalize(IN.eyeVector);
    half3 R = reflect(-E, Nw);

    half4 reflection = texCUBE(EnvironmentMap, R);
    // hdr effect (multiplier in alpha channel)
    reflection.rgb *= (1.0 + reflection.a*hdrMultiplier);

	// fresnel - could use 1D tex lookup for this
    half facing = 1.0 - max(dot(E, Nw), 0);
    half fresnel = fresnelBias + (1.0-fresnelBias)*pow(facing, fresnelPower);

    half4 waterColor = lerp(deepColor, shallowColor, facing);

	float4 result = waterColor*waterAmount + reflection*reflectionColor*reflectionAmount*fresnel;
	result.w = deepColor.w;
	
	// -FOG-
	result.xyz = lerp(fogColor, result, IN.FogValue);
	
	return result;
}

technique PS20_0
{
	pass p0
	{
		VertexShader = compile vs_2_0 BumpReflectWaveVS_0( bumpHeight, textureScale, bumpSpeed, time,
                                                        waveFreq, waveAmp);
		PixelShader = compile ps_2_0 OceanPS20(normalMapSampler, envMapSampler,
                                               deepColor, shallowColor, reflectionColor, reflectionAmount, waterAmount,
                                               fresnelPower, fresnelBias, hdrMultiplier);
	}
}

technique PS20_1
{
	pass p0
	{
		VertexShader = compile vs_2_0 BumpReflectWaveVS_1( bumpHeight, textureScale, bumpSpeed, time,
                                                        waveFreq, waveAmp);
		PixelShader = compile ps_2_0 OceanPS20(normalMapSampler, envMapSampler,
                                               deepColor, shallowColor, reflectionColor, reflectionAmount, waterAmount,
                                               fresnelPower, fresnelBias, hdrMultiplier);
	}
}

technique PS20_2
{
	pass p0
	{
		VertexShader = compile vs_2_0 BumpReflectWaveVS_2( bumpHeight, textureScale, bumpSpeed, time,
                                                        waveFreq, waveAmp);
		PixelShader = compile ps_2_0 OceanPS20(normalMapSampler, envMapSampler,
                                               deepColor, shallowColor, reflectionColor, reflectionAmount, waterAmount,
                                               fresnelPower, fresnelBias, hdrMultiplier);
	}
}