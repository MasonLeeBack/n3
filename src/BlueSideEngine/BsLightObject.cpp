#include "stdafx.h"
#include "BsKernel.h"
#include "BsLightObject.h"

CBsLightObject::CBsLightObject()
{
	SetObjectType(BS_LIGHT_OBJECT);
	m_LightType = DIRECTIONAL_LIGHT;

	m_Diffuse.r = 1.f;
	m_Diffuse.g = 1.f;
	m_Diffuse.b = 1.f;
	m_Diffuse.a = 1.f;

	m_Specular.r = 1.f;
	m_Specular.g = 1.f;
	m_Specular.b = 1.f;
	m_Specular.a = 1.f;

	m_Ambient.r = 1.f;
	m_Ambient.g = 1.f;
	m_Ambient.b = 1.f;
	m_Ambient.a = 1.f;
	
	m_PointLightColor.r = 1.f;
	m_PointLightColor.g = 1.f;
	m_PointLightColor.b = 1.f;
	m_PointLightColor.a = 1.f;
	
	m_PointLightInfo = D3DXVECTOR4(0,0,0,0);			
}

CBsLightObject::~CBsLightObject()
{
	if( m_LightType == DIRECTIONAL_LIGHT ) {
		g_BsKernel.SetDirectionalLight(-1);
	}
}

void CBsLightObject::Release()
{	
	if( m_LightType == POINT_LIGHT ) {
		g_BsKernel.DeletePointLightIndex( m_nKernelPoolIndex );
	}
}

void CBsLightObject::SetObjectMatrix(D3DXMATRIX *pMatrix)	
{	
	SetEntireObjectMatrix( pMatrix );
}

D3DXVECTOR4* CBsLightObject::GetPointLightInfo() 
{	
	D3DXMATRIX *pMatrix = GetObjectMatrix();
	m_PointLightInfo.x = pMatrix->_41;
	m_PointLightInfo.y = pMatrix->_42;
	m_PointLightInfo.z = pMatrix->_43;	
	return &m_PointLightInfo;
}

bool CBsLightObject::GetBox3( Box3& B )
{
	const D3DXMATRIX *pMatrix;

	pMatrix = GetObjectMatrixByProcess();
	B.C.x = pMatrix->_41;
	B.C.y = pMatrix->_42;
	B.C.z = pMatrix->_43;

	B.E[0] = m_PointLightInfo.w;
	B.E[1] = m_PointLightInfo.w;
	B.E[2] = m_PointLightInfo.w;

	B.A[0] = BSVECTOR(1,0,0);
	B.A[1] = BSVECTOR(0,1,0);
	B.A[2] = BSVECTOR(0,0,1);
	return true;
}

void CBsLightObject::Render(C3DDevice *pDevice)
{
}

int CBsLightObject::ProcessMessage(DWORD dwCode, DWORD dwParam1/*=0*/, DWORD dwParam2/*=0*/, DWORD dwParam3/*=0*/)
{
	switch(dwCode){
		case BS_SET_LIGHT_DIFFUSE:
			{
				if(dwParam1)
                    SetDiffuse(*(D3DCOLORVALUE*)dwParam1);
				BsAssert(!dwParam2);
				BsAssert(!dwParam3);
			}
			return 1;
		case BS_SET_LIGHT_SPECULAR:
			{
				if(dwParam1)
					SetSpecular(*(D3DCOLORVALUE*)dwParam1);
				BsAssert(!dwParam2);
				BsAssert(!dwParam3);
			}
			return 1;
		case BS_SET_LIGHT_AMBIENT:
			{
				if(dwParam1)
					SetAmbient(*(D3DCOLORVALUE*)dwParam1);
				BsAssert(!dwParam2);
				BsAssert(!dwParam3);
			}
			return 1;
		case BS_GET_LIGHT_DIFFUSE:
			return (int)&m_Diffuse;
		case BS_GET_LIGHT_SPECULAR:
			return (int)&m_Specular;
		case BS_GET_LIGHT_AMBIENT:
			return (int)&m_Ambient;

		case BS_SET_POINTLIGHT_INFO:			
			if(dwParam1) {
                SetPointLightInfo(*(D3DXVECTOR4*)dwParam1);
			}
			return 1;
		case BS_GET_POINTLIGHT_INFO:			
			return (int)&m_PointLightInfo;
		case BS_SET_POINTLIGHT_RANGE:
			m_PointLightInfo.w = *(float*)dwParam1;
			return 1;

		case BS_GET_POINTLIGHT_COLOR:
			return (int)&m_PointLightColor;
		case BS_SET_POINTLIGHT_COLOR:
			m_PointLightColor = *(D3DCOLORVALUE*)dwParam1;
			return 1;
	}
	return CBsObject::ProcessMessage(dwCode, dwParam1, dwParam2, dwParam3);
}
