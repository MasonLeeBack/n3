#pragma once
#include "BsObject.h"

class CBsLightObject : public CBsObject
{
public:
	CBsLightObject();
	virtual ~CBsLightObject();

protected:
	LIGHT_TYPE	m_LightType;

	// directional light	
	D3DCOLORVALUE m_Diffuse;
	D3DCOLORVALUE m_Specular;
	D3DCOLORVALUE m_Ambient;

	// point lights
	D3DXVECTOR4		m_PointLightInfo;
	D3DCOLORVALUE m_PointLightColor;

public:
	// TODO : CBsKernel::m_PointLightIndices 가 Thread Sync문제를 일으키고 있습니다. 임시로 보호해놨습니다.
	// 해결책은 Point Light 연산을 모두 렌더 쓰레드에서 처리하게 하는게 어떨까요?	
	virtual bool GetBox3( Box3& B );
	virtual void SetObjectMatrix(D3DXMATRIX *pMatrix);
	void SetLightType( LIGHT_TYPE LightType) { m_LightType = LightType;}

	D3DCOLORVALUE* GetDiffuse()	{	return &m_Diffuse;	}
	D3DCOLORVALUE* GetSpecular()	{	return &m_Specular;	}
	D3DCOLORVALUE* GetAmbient()	{	return &m_Ambient;	}
	void SetDiffuse(D3DCOLORVALUE Diffuse)	{ m_Diffuse=Diffuse; }
	void SetSpecular(D3DCOLORVALUE Specular) { m_Specular=Specular; }
	void SetAmbient(D3DCOLORVALUE Ambient)	{ m_Ambient=Ambient; }

	D3DXVECTOR4* GetPointLightInfo();
	void SetPointLightInfo(D3DXVECTOR4 PointInfo) { m_PointLightInfo= PointInfo;}

	D3DCOLORVALUE* GetPointLightColor() {return &m_PointLightColor; }
	void SetPointLightColor(D3DCOLORVALUE Color) { m_PointLightColor = Color;}

	virtual void Release();

	virtual void Render(C3DDevice *pDevice);
	virtual int ProcessMessage(DWORD dwCode, DWORD dwParam1=0, DWORD dwParam2=0, DWORD dwParam3=0);
};
