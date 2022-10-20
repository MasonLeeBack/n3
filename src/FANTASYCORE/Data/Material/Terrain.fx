/*
 *    No Texture, Directional Light, Per-Pixel
 */
//------------------------------------
float4x4 worldViewProj					: WorldViewProjection;
float4x4 worldviewInverseTranspose		: WorldViewInverseTranspose;

float4x4 WorldView						: WorldView;

float4x4 shadowAll						: SHADOW_ALL;

float4 fogFactor						: FOGFACTOR; // fogEnd, fogRange, ?, ?
float4 fogColor							: FOGCOLOR; // r,g,b,1

const float fShadowMapTexelSize = 1.f/1600.f;
const float fShadowMapSize = 1600.f;

float4 lightDir : Direction
<
    string Object = "DirectionalLight";
    string Space = "World";
> = {0.0f, -1.0f, 0.0f, 0.0f};

float4 lightDiffuse : LightDiffuse
<
    string UIName = "Diffuse Light Color";
    string Object = "DirectionalLight";
> = {0.8f, 0.8f, 0.8f, 1.0f};

float4 lightSpecular : LightSpecular
<
    string UIWidget = "Ambient Light Color";
    string Space = "material";
> = {1.f, 1.f, 1.0f, 1.f};

float4 lightAmbient : LightAmbient
<
    string UIWidget = "Ambient Light Color";
    string Space = "material";
> = {0.1f, 0.1f, 0.1f, 1.0f};

float4 materialAmbient : MaterialAmbient
<
    string UIWidget = "Surface Color";
    string Space = "material";
> = { 1.f, 1.f, 1.f, 1.f};

float4 materialDiffuse : MaterialDiffuse
<
    string UIWidget = "Surface Color";
    string Space = "material";
> = { 0.753f, 0.753f, 0.753f, 1.0f};

float4 materialSpecular : MaterialSpecular
<
    string UIWidget = "Surface Specular";
    string Space = "material";
> = {1.f, 1.f, 1.f, 1.0f};

texture LightmapTexture : Diffuse
<
    string ResourceName = "default_color.dds";
>;

sampler LightmapSampler = sampler_state
{
    texture = <LightmapTexture>;
    MIPFILTER = LINEAR;
    MINFILTER = LINEAR;
    MAGFILTER = LINEAR;
};

texture Layer0texture : Diffuse
<
    string ResourceName = "default_color.dds";
>;

sampler Layer0Sampler = sampler_state 
{
	texture = <Layer0texture>;
	AddressU  = WRAP;        
	AddressV  = WRAP;
	AddressW  = WRAP;
	MIPFILTER = LINEAR;
	MINFILTER = LINEAR;
	MAGFILTER = LINEAR;
};

texture Layer1texture : Diffuse
<
    string ResourceName = "default_color.dds";
>;

sampler Layer1Sampler = sampler_state 
{
    texture = <Layer1texture>;
    AddressU  = WRAP;
    AddressV  = WRAP;
    AddressW  = WRAP;
    MIPFILTER = LINEAR;
    MINFILTER = LINEAR;
    MAGFILTER = LINEAR;
};

texture Layer2texture : Diffuse
<
    string ResourceName = "default_color.dds";
>;

sampler Layer2Sampler = sampler_state 
{
    texture = <Layer2texture>;
    AddressU  = WRAP;        
    AddressV  = WRAP;
    AddressW  = WRAP;
    MIPFILTER = LINEAR;
    MINFILTER = LINEAR;
    MAGFILTER = LINEAR;
};

texture Layer3texture : Diffuse
<
    string ResourceName = "default_color.dds";
>;

sampler Layer3Sampler = sampler_state 
{
    texture = <Layer3texture>;
    AddressU  = WRAP;        
    AddressV  = WRAP;
    AddressW  = WRAP;
    MIPFILTER = LINEAR;
    MINFILTER = LINEAR;
    MAGFILTER = LINEAR;
};

texture Layer4texture : Diffuse
<
    string ResourceName = "default_color.dds";
>;

sampler Layer4Sampler = sampler_state 
{
    texture = <Layer4texture>;
    AddressU  = WRAP;        
    AddressV  = WRAP;
    AddressW  = WRAP;
    MIPFILTER = LINEAR;
    MINFILTER = LINEAR;
    MAGFILTER = LINEAR;
};

texture BlendTexture : Diffuse
<
    string ResourceName = "default_color.dds";
>;

sampler BlendSampler = sampler_state 
{
    texture = <BlendTexture>;
    AddressU  = WRAP;        
    AddressV  = WRAP;
    AddressW  = WRAP;
    MIPFILTER = LINEAR;
    MINFILTER = LINEAR;
    MAGFILTER = LINEAR;
};

texture ShadowMap : Diffuse
<
    string ResourceName = "default_color.dds";
>;

sampler ShadowMapSampler = sampler_state
{
	Texture = <ShadowMap>;
	AddressU  = BORDER;        
	AddressV  = BORDER;
	MINFILTER = POINT;
	MAGFILTER = POINT;
	BorderColor = 0xffffffff;
};

texture AttributeTexture : Diffuse
<
	string ResourceName = "default_color.dds";
>;

sampler AttributeSampler = sampler_state 
{
    texture = <AttributeTexture>;
    AddressU  = CLAMP;        
    AddressV  = CLAMP;
    AddressW  = CLAMP;
    MINFILTER = POINT;
    MAGFILTER = POINT;
};

//------------------------------------
struct vertexInput {
    float3 position				: POSITION;
    float3 normal				: NORMAL;
    float3 Mulcolor				: COLOR0;
    float3 Addcolor				: COLOR1;
    float2 texUV				: TEXCOORD0;
};

struct vertexOutput0 {
	float4 hPosition			: POSITION;
	float2 Layer4UV				: TEXCOORD0;
	float4 BlendUV				: TEXCOORD1;    // z : fog weight, w : shadow density
	float4 TexCoord0			: TEXCOORD2;	// Shadowmap Coordinate
	float3 Mulcolor				: COLOR0;
	float3 Addcolor				: COLOR1;
	float4 diffAmbColor			: TEXCOORD3;
	float4 specularColor		: TEXCOORD4;
};

struct vertexOutput1 {
    float4 hPosition			: POSITION;
    float2 Layer4UV				: TEXCOORD0;
    float3 BlendUV				: TEXCOORD1;    // z : fog weight
    float3 Mulcolor				: COLOR0;
    float3 Addcolor				: COLOR1;
    float4 diffAmbColor			: TEXCOORD2;
    float4 specularColor		: TEXCOORD3;
};

//------------------------------------
vertexOutput0 VS_TransformAndTexture_0( int index : INDEX )
{
	vertexOutput0 OUT;
	// vfetch optimization by takogura
	float4	position, normal, Mulcolor, Addcolor, texUV;
	asm{
		vfetch	position,	index.x, position
		vfetch	normal,		index.x, normal
		vfetch	Mulcolor,	index.x, color0
		vfetch	Addcolor,	index.x, color1
		vfetch	texUV,		index.x, texcoord0
	};
	
	OUT.hPosition = mul( float4(position.xyz , 1.0) , worldViewProj);

	float4 N = mul(normal.xyz, worldviewInverseTranspose); //normal vector at View space
	float3 L = ( -lightDir.xyz); //light vector at View space

	float3 viewVertPos = mul(float4(position.xyz,1.f), WorldView).xyz;
	float3 E = normalize( -viewVertPos); //eye vector
	float3 H = normalize(E + L); //half angle vector

	float  diffuse = max(0 , dot(N,L));
	float  spec = pow( max(0 , dot(N,H) ) , 6.f );

	float4 ambColor = materialAmbient * lightAmbient;
	float4 diffColor = materialDiffuse * diffuse * lightDiffuse ;
	OUT.diffAmbColor = diffColor + ambColor;
	OUT.specularColor = lightSpecular * materialSpecular * spec;

	// 1 m => 64 pixel
	// 1 sell(terrain) => 2 m 
	// 1 texture => 8 m
	// 1/ 800cm
	OUT.Layer4UV = 0.002f*position.xz;

	OUT.BlendUV.xy = texUV.xy;
	OUT.Mulcolor = Mulcolor;
	OUT.Addcolor = Addcolor;

	// Fog Density
	float fDist = fogFactor.x - OUT.hPosition.z; // x=fogEnd
	OUT.BlendUV.z = clamp ( fDist*fogFactor.y, 0.0f, 1.0f ); // y=forRange
	
	// Shadow Density 
	fDist = 5000.f - OUT.hPosition.z;
	OUT.BlendUV.w = clamp(fDist*0.0005f, 0.f, 1.f);

	OUT.TexCoord0 = mul(float4(position.xyz , 1.f), shadowAll);
	OUT.TexCoord0.xy /= OUT.TexCoord0.w;
	
	return OUT;
}

// No Shadow Map
vertexOutput1 VS_TransformAndTexture_1( int index : INDEX )
{
    vertexOutput1 OUT;
    
	// vfetch optimization by takogura
   	float4	position, normal, Mulcolor, Addcolor, texUV;
	asm{
		vfetch	position,	index.x, position
		vfetch	normal,		index.x, normal
		vfetch	Mulcolor,	index.x, color0
		vfetch	Addcolor,	index.x, color1
		vfetch	texUV,		index.x, texcoord0
	};
    
    OUT.hPosition = mul( float4(position.xyz , 1.0) , worldViewProj);

    float4 N = mul(normal.xyz, worldviewInverseTranspose); //normal vector at View space
    float3 L = ( -lightDir.xyz); //light vector at View space

    float3 viewVertPos = mul(float4(position.xyz,1.f), WorldView).xyz;
    float3 E = normalize( -viewVertPos); //eye vector
    float3 H = normalize(E + L); //half angle vector


    float  diffuse = max(0 , dot(N,L));
    float  spec = pow( max(0 , dot(N,H) ) , 6.f );

    float4 ambColor = materialAmbient * lightAmbient;
    float4 diffColor = materialDiffuse * diffuse * lightDiffuse ;
    OUT.diffAmbColor = clamp(diffColor + ambColor, 0, 1);
    OUT.specularColor = lightSpecular * materialSpecular * spec;

    // 1 m => 64 pixel
    // 1 sell(terrain) => 2 m 
    // 1 texture => 8 m
    // 1/ 800cm
    OUT.Layer4UV = 0.002f*position.xz;
    
    OUT.BlendUV.xy = texUV.xy;
    OUT.Mulcolor = Mulcolor;
    OUT.Addcolor = Addcolor;

    // -FOG-
    float fDist = fogFactor.x - OUT.hPosition.z; // x=fogEnd
    OUT.BlendUV.z = clamp ( fDist*fogFactor.y, 0.0f, 1.0f ); // y=forRange

    return OUT;
}

float3 ApplyWorldLightMap(float4 fLightMap)
{
	float3 fTemp;
	fTemp = fLightMap.xyz * 0.8f + 0.2f;
	return fTemp;
}

float3 ApplyWorldLightMapWithShadow(float fShadow, float4 fLightMap)
{
	float3 fTemp;
	fTemp = ((fShadow.xxx*0.6f + 0.4f)*fLightMap.xyz) * 0.7f + 0.3f;
	return fTemp;
}

//-----------------------------------
float4 PS_Textured0( vertexOutput0 IN): COLOR
{
	float4 Lay0 = tex2D(Layer0Sampler, IN.Layer4UV.xy);
	float4 Lay1 = tex2D(Layer1Sampler, IN.Layer4UV.xy);
	float4 Lay2 = tex2D(Layer2Sampler, IN.Layer4UV.xy);
	float4 Lay3 = tex2D(Layer3Sampler, IN.Layer4UV.xy);
	float4 Lay4 = tex2D(Layer4Sampler, IN.Layer4UV.xy);

	float4 BlendWeight = tex2D(BlendSampler, IN.BlendUV.xy);

	// lerp(x, y, s) => x + s(y - x)
	float4 resultBlend = lerp(Lay0, Lay1, BlendWeight.x);
	resultBlend = lerp(resultBlend, Lay2, BlendWeight.y);
	resultBlend = lerp(resultBlend, Lay3, BlendWeight.z);
	resultBlend = lerp(resultBlend, Lay4, BlendWeight.w);

	float4 MulColor = float4(IN.Mulcolor.xyz, 1.f);
	float4 AddColor = float4(IN.Addcolor.xyz, 0.f);
	float4 specular = float4(IN.specularColor.xyz*resultBlend.www, 0.f);
	float4 result = resultBlend*(IN.diffAmbColor+specular)*MulColor + AddColor;

	// Shadow!!
	float  shadowTestDepth = IN.TexCoord0.z / IN.TexCoord0.w;

	float2 fFrac = frac(IN.TexCoord0.xy*fShadowMapSize);
	    
	float4  shadowDepth;
	shadowDepth.x = tex2D(ShadowMapSampler, IN.TexCoord0.xy).r;
	shadowDepth.y = tex2D(ShadowMapSampler, IN.TexCoord0.xy + float2(fShadowMapTexelSize, 0.f)).r;
	shadowDepth.z = tex2D(ShadowMapSampler, IN.TexCoord0.xy + float2(0, fShadowMapTexelSize)).r;
	shadowDepth.w = tex2D(ShadowMapSampler, IN.TexCoord0.xy + float2(fShadowMapTexelSize, fShadowMapTexelSize)).r;

	float4 inLight = (shadowTestDepth < shadowDepth);

	float LightAmount = lerp( lerp( inLight[0], inLight[1], fFrac.x ),
							lerp( inLight[2], inLight[3], fFrac.x ),
							fFrac.y );
	if(IN.TexCoord0.x>=1.f || IN.TexCoord0.x<=0.f || IN.TexCoord0.y>=1.f || IN.TexCoord0.y<=0.f) {
		LightAmount = 1.f;
	}
	if(shadowTestDepth>=1.f) {
		LightAmount = 1.f;
	}
	LightAmount*=0.5f;
	LightAmount+=0.5f;
	result.xyz*=LightAmount;

	// -FOG-
	result.xyz = lerp(fogColor, result, IN.BlendUV.z);

	return result;
}

float4 tex2DOffset(sampler2D ss, float2 uv, float2 offset)
{
    float4 result;
    float offsetX = offset.x;
    float offsetY = offset.y;
    asm {
        tfetch2D result, uv, ss, OffsetX=offsetX,
            OffsetY=offsetY
    };
    return result;
}

//-----------------------------------
float4 PS_Textured2( vertexOutput0 IN): COLOR
{
	float4 Lay0 = tex2D(Layer0Sampler, IN.Layer4UV.xy);
	float4 Lay1 = tex2D(Layer1Sampler, IN.Layer4UV.xy);
	float4 Lay2 = tex2D(Layer2Sampler, IN.Layer4UV.xy);
	float4 Lay3 = tex2D(Layer3Sampler, IN.Layer4UV.xy);
	float4 Lay4 = tex2D(Layer4Sampler, IN.Layer4UV.xy);

	float4 BlendWeight = tex2D(BlendSampler, IN.BlendUV.xy);
	float4 LightColor = tex2D(LightmapSampler, IN.BlendUV.xy);

	// lerp(x, y, s) = x + s(y - x)
	float4 resultBlend = lerp(Lay0, Lay1, BlendWeight.x);
	resultBlend = lerp(resultBlend, Lay2, BlendWeight.y);
	resultBlend = lerp(resultBlend, Lay3, BlendWeight.z);
	resultBlend = lerp(resultBlend, Lay4, BlendWeight.w);

	float4 MulColor = float4(IN.Mulcolor.xyz, 1.f);
	float4 AddColor = float4(IN.Addcolor.xyz, 0.f);
	float4 specular = float4(IN.specularColor.xyz*resultBlend.www, 0.f);
	float4 result = resultBlend*(IN.diffAmbColor+specular)*MulColor + AddColor;

	// Shadow!!
	float  shadowTestDepth = IN.TexCoord0.z / IN.TexCoord0.w;

	float2 fFrac = frac(IN.TexCoord0.xy*fShadowMapSize);
	    
	float4  shadowDepth;
	shadowDepth.x = tex2D(ShadowMapSampler, IN.TexCoord0.xy).r;
	shadowDepth.y = tex2DOffset(ShadowMapSampler, IN.TexCoord0.xy, float2(1.f, 0.f)).r;
	shadowDepth.z = tex2DOffset(ShadowMapSampler, IN.TexCoord0.xy, float2(0.f, 1.f)).r;
	shadowDepth.w = tex2DOffset(ShadowMapSampler, IN.TexCoord0.xy, float2(1.f, 1.f)).r;

	float4 inLight = (shadowTestDepth < shadowDepth);

	float LightAmount = lerp( lerp( inLight[0], inLight[1], fFrac.x ),
							lerp( inLight[2], inLight[3], fFrac.x ),
							fFrac.y );
	if(shadowTestDepth>=1.f) {
		LightAmount = 1.f;
	}
	LightAmount = lerp(1.f, LightAmount, IN.BlendUV.w);

	float3 temp = ApplyWorldLightMapWithShadow(LightAmount, LightColor);
	result.xyz*=temp;

	// -FOG-
	result.xyz = lerp(fogColor, result, IN.BlendUV.z);

	return result;
}

//-----------------------------------
float4 PS_Textured1( vertexOutput1 IN): COLOR
{
    float4 Lay0 = tex2D(Layer0Sampler, IN.Layer4UV.xy);
    float4 Lay1 = tex2D(Layer1Sampler, IN.Layer4UV.xy);
    float4 Lay2 = tex2D(Layer2Sampler, IN.Layer4UV.xy);
    float4 Lay3 = tex2D(Layer3Sampler, IN.Layer4UV.xy);
    float4 Lay4 = tex2D(Layer4Sampler, IN.Layer4UV.xy);
    
    float4 BlendWeight = tex2D(BlendSampler, IN.BlendUV.xy);
    
    // lerp(x, y, s) => x + s(y - x)
    float4 resultBlend = lerp(Lay0, Lay1, BlendWeight.x);
    resultBlend = lerp(resultBlend, Lay2, BlendWeight.y);
    resultBlend = lerp(resultBlend, Lay3, BlendWeight.z);
    resultBlend = lerp(resultBlend, Lay4, BlendWeight.w);
    
    float4 MulColor = float4(IN.Mulcolor.xyz, 1.f);
    float4 AddColor = float4(IN.Addcolor.xyz, 0.f);
    float4 specular = float4(IN.specularColor.xyz*resultBlend.www, 0.f);
    float4 result = resultBlend*(IN.diffAmbColor+specular)*MulColor + AddColor;

    // -FOG-
    result.xyz = lerp(fogColor, result, IN.BlendUV.z);
    
    return result;
}

//-----------------------------------
float4 PS_Textured3( vertexOutput1 IN): COLOR
{
    float4 Lay0 = tex2D(Layer0Sampler, IN.Layer4UV.xy);
    float4 Lay1 = tex2D(Layer1Sampler, IN.Layer4UV.xy);
    float4 Lay2 = tex2D(Layer2Sampler, IN.Layer4UV.xy);
    float4 Lay3 = tex2D(Layer3Sampler, IN.Layer4UV.xy);
    float4 Lay4 = tex2D(Layer4Sampler, IN.Layer4UV.xy);
    
    float4 BlendWeight = tex2D(BlendSampler, IN.BlendUV.xy);
	float4 LightColor = tex2D(LightmapSampler, IN.BlendUV.xy);

	float3 temp = ApplyWorldLightMapWithShadow(1.f, LightColor);

    // lerp(x, y, s) => x + s(y - x)
    float4 resultBlend = lerp(Lay0, Lay1, BlendWeight.x);
    resultBlend = lerp(resultBlend, Lay2, BlendWeight.y);
    resultBlend = lerp(resultBlend, Lay3, BlendWeight.z);
    resultBlend = lerp(resultBlend, Lay4, BlendWeight.w);
    
    float4 MulColor = float4(IN.Mulcolor.xyz, 1.f);
    float4 AddColor = float4(IN.Addcolor.xyz, 0.f);
    float4 specular = float4(IN.specularColor.xyz*resultBlend.www, 0.f);
    float4 result = resultBlend*(IN.diffAmbColor+specular)*MulColor + AddColor;
	result.xyz*=temp;

    // -FOG-
    result.xyz = lerp(fogColor, result, IN.BlendUV.z);
    
    return result;
}

//-----------------------------------
float4 PS_Textured4( vertexOutput1 IN): COLOR
{
    float4 Lay0 = tex2D(Layer0Sampler, IN.Layer4UV.xy);
    float4 Lay1 = tex2D(Layer1Sampler, IN.Layer4UV.xy);
    float4 Lay2 = tex2D(Layer2Sampler, IN.Layer4UV.xy);
    float4 Lay3 = tex2D(Layer3Sampler, IN.Layer4UV.xy);
    float4 Lay4 = tex2D(Layer4Sampler, IN.Layer4UV.xy);
    
	float4 BlendWeight = tex2D(BlendSampler, IN.BlendUV.xy);
	float4 LightColor = tex2D(LightmapSampler, IN.BlendUV.xy);

	float4 Attribute = tex2D(AttributeSampler , IN.BlendUV.xy);

	float3 temp = ApplyWorldLightMapWithShadow(1.f, LightColor);

    // lerp(x, y, s) => x + s(y - x)
    float4 resultBlend = lerp(Lay0, Lay1, BlendWeight.x);
    resultBlend = lerp(resultBlend, Lay2, BlendWeight.y);
    resultBlend = lerp(resultBlend, Lay3, BlendWeight.z);
    resultBlend = lerp(resultBlend, Lay4, BlendWeight.w);
    
    float4 MulColor = float4(IN.Mulcolor.xyz, 1.f);
    float4 AddColor = float4(IN.Addcolor.xyz, 0.f);
    float4 specular = float4(IN.specularColor.xyz*resultBlend.www, 0.f);
    float4 result = resultBlend*(IN.diffAmbColor+specular)*MulColor + AddColor;
	result.xyz*=temp;

	float4 fAttributeColor;
	if(Attribute.w < 0.1f) {
		fAttributeColor = float4(1.f, 1.f, 0.f, 1.f);
	}
	else if(Attribute.w < 0.2f) {
		fAttributeColor = float4(0.f, 1.f, 0.f, 1.f);
	}
	else if(Attribute.w < 0.4f) {
		fAttributeColor = float4(1.f, 0.f, 0.f, 1.f);
	}
	else if(Attribute.w < 0.6f) {
		fAttributeColor = float4(0.f, 0.f, 0.f, 1.f);
	}
	else if(Attribute.w < 0.8f) {
		fAttributeColor = float4(0.5f, 0.5f, 1.f, 1.f);
	}
	else {
		fAttributeColor = float4(1.f, 1.f, 1.f, 1.f);
	}
	


    // -FOG-
    result.xyz = lerp(fogColor, result, IN.BlendUV.z);
   
    return result*fAttributeColor;
}


//-----------------------------------
// Shadow map
technique textured_0
{
    pass p0 
    {        
        VertexShader = compile vs_2_0 VS_TransformAndTexture_0();
        ZEnable = true;
        ZWriteEnable = true;
        PixelShader  = compile ps_2_0 PS_Textured0();
    }
}

// NoShadow map
technique textured_1
{
    pass p0 
    {        
        VertexShader = compile vs_2_0 VS_TransformAndTexture_1();
        ZEnable = true;
        ZWriteEnable = true;
        PixelShader  = compile ps_2_0 PS_Textured1();
    }
}

// Shadow map + Light map
technique textured_2
{
    pass p0 
    {        
        VertexShader = compile vs_2_0 VS_TransformAndTexture_0();
        ZEnable = true;
        ZWriteEnable = true;
        PixelShader  = compile ps_2_0 PS_Textured2();
    }
}

//////////////////////////////////////////////////// For Map Tool
// Light map
technique textured_3
{
    pass p0 
    {        
        VertexShader = compile vs_2_0 VS_TransformAndTexture_1();
        ZEnable = true;
        ZWriteEnable = true;
        PixelShader  = compile ps_2_0 PS_Textured3();
    }
}

// Light map + Attribute map
technique textured_4
{
    pass p0 
    {        
        VertexShader = compile vs_2_0 VS_TransformAndTexture_1();
        ZEnable = true;
        ZWriteEnable = true;
        PixelShader  = compile ps_2_0 PS_Textured4();
    }
}