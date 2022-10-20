#include "stdafx.h"
#include "BsKernel.h"
#include "BsSkyBoxObject.h"

int CBsSkyBoxObject::s_nKernelIndex = -1;

CBsSkyBoxObject::CBsSkyBoxObject()
{
	SetObjectType(BS_SKYBOX_OBJECT);

	m_fSaveNearZ=1.0f;
	m_fSaveFarZ=6000.f;
	SetShadowCastType(BS_SHADOW_NONE);
	m_bEnableInstancing = false;
}

CBsSkyBoxObject::~CBsSkyBoxObject()
{
	s_nKernelIndex = -1;
}

void CBsSkyBoxObject::PreRender(C3DDevice *pDevice)
{
	CBsCamera* pActiveCamera = g_BsKernel.GetActiveCamera();
	m_fSaveNearZ = pActiveCamera->GetNearZ();
	m_fSaveFarZ = pActiveCamera->GetFarZ();

	D3DXMATRIX matProj;
	pActiveCamera->SetProjectionMatrix(10.f, 5000000.0f);
	pActiveCamera->SetViewProjMatrix();
	pDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
	pDevice->SetRenderState(D3DRS_ZENABLE, FALSE);

	pDevice->SetRenderState(D3DRS_ALPHATESTENABLE, false);

	// 위치를 카메라 위치로...
	D3DXMATRIX *pMatrix;

	pMatrix = GetObjectMatrix();
	pMatrix->_41 = g_BsKernel.GetParamInvViewMatrix()->_41;
	pMatrix->_42 = g_BsKernel.GetParamInvViewMatrix()->_42;
	pMatrix->_43 = g_BsKernel.GetParamInvViewMatrix()->_43;
}

void CBsSkyBoxObject::PostRender(C3DDevice *pDevice)
{
	CBsCamera* pActiveCamera = g_BsKernel.GetActiveCamera();
	pActiveCamera->SetProjectionMatrix(m_fSaveNearZ, m_fSaveFarZ );
	pActiveCamera->SetViewProjMatrix();
	pDevice->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
	pDevice->SetRenderState(D3DRS_ZENABLE, TRUE);

	pDevice->SetRenderState(D3DRS_ALPHATESTENABLE, true);
}
