//------------------------------------------------------------------------------
// pointlight.fxh
//
// $Header$
//
//------------------------------------------------------------------------------

//==============================================================================
//	Shared Parameter List!!!!
//	Desc : 사용하기 위해서는 반드시 자료형, 변수명, Semantic명이 같아야 합니다.
//==============================================================================
//shared float4x4 view							: View;
//shared float4x4 projection					: Projection;
//shared float4x4 viewInverse 					: ViewInverse;
//shared float4x4 viewInvTrans					: ViewInverseTranspose;
//shared float4x4 viewProj						: ViewProjection;
//shared float4	vecRcpWorldSize					: RcpWorldSize;
//shared float4 fogFactor						: FOGFACTOR;	// fogEnd, forRange, ?, ?
//shared float4 fogColor						: FOGCOLOR;		// r,g,b,1
//shared float4 viewlightDir					: LightDirectionViewSpace
//shared float4 lightColor						: LightDiffuse
//shared float4 lightAmbient					: LightAmbient
//shared float4 lightSpecular					: LightSpecular

//********************************************************************************
//********************************************************************************

float4 pointLight[5] : PointLightInfo;
float4 pointLightColor[5] : PointLightColor;
// calculate point light's  diffuse and specular

float3 ApplyPointLight( float3 viewPos, float3 N)
{
	int nCount = (int)( pointLightColor[0].w );
	
	float3 result = float3(0,0,0);
		
	for( int i = 0; i < nCount; i++ ) {
		float3 L = pointLight[i].xyz - viewPos;
		float Dist = length(L);
    		L = L / Dist;
    		float diff = max(0 , dot(N, L));
    		float dist =  saturate(Dist  * pointLight[i].w);    		//  range 를 Rcp 로 넘김
    		dist * dist;
    		dist = 1-dist;    		
    		result += (dist*diff) * pointLightColor[i];
	} 
	return result;
}

