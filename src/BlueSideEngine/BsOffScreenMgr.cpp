#include "stdafx.h"
#include "BsOffScreenMgr.h"
#include "BsKernel.h"
#include "BsGenerateTexture.h"
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
CBsCriticalSection CBsOffScreenManager::s_csOffScreenManager;

CBsOffScreenManager::CBsOffScreenManager()
{
	m_bEnable = true;
}

CBsOffScreenManager::~CBsOffScreenManager()
{
	Clear();
}

DWORD CBsOffScreenManager::Create(CBsGenerateTexture* pBsGenerateTexture, int nWidth, int nHeight, float fStartX, float fStartY, float fWidth, float fHeight, bool bAlpha)
{
	THREAD_AUTOLOCK(&s_csOffScreenManager);
	m_pRttList.push_back(pBsGenerateTexture);
	DWORD hHandle = (DWORD)m_pRttList.back();

	pBsGenerateTexture->Create(nWidth, nHeight, fStartX, fStartY, fWidth, fHeight, bAlpha);
	pBsGenerateTexture->SetHandle(hHandle);

	return hHandle;
}

void CBsOffScreenManager::ReInitialize()
{
	UINT uiCount = m_pRttList.size();
	for(UINT i=0; i<uiCount; i++)
	{
		CBsGenerateTexture* pBsGTex = m_pRttList[i];
		pBsGTex->ReInitialize();
	}
}

void CBsOffScreenManager::Clear()
{
	UINT uiCount = m_pRttList.size();
	for(UINT i=0; i<m_pRttList.size(); i++) {
		CBsGenerateTexture* pBsGTex = m_pRttList[i];
		pBsGTex->Release();
		delete pBsGTex;
	}

	m_pRttList.clear();
}

bool CBsOffScreenManager::Release(DWORD hHandle)
{
	THREAD_AUTOLOCK(&s_csOffScreenManager);

	UINT uiCount = m_pRttList.size();
	for (UINT i=0; i<m_pRttList.size(); i++) {
		if (m_pRttList[i] == (CBsGenerateTexture*)hHandle) {
			m_pRttList[i]->Release();
			delete m_pRttList[i];
			m_pRttList.erase(m_pRttList.begin()+i);
			return true;
		}
	}

	return false;
}

void CBsOffScreenManager::InitRenderRTTs()
{
	THREAD_AUTOLOCK(&s_csOffScreenManager);
	UINT uiCount = m_pRttList.size();
	for (UINT i=0; i<m_pRttList.size(); i++) {
		CBsGenerateTexture* pBsGTex = m_pRttList[i];
		if(pBsGTex->IsEnable() != true) {
			continue;
		}
		pBsGTex->InitRenderRTT();
	}
}

void CBsOffScreenManager::Render(C3DDevice *pDevice)
{
	THREAD_AUTOLOCK(&s_csOffScreenManager);

	if(IsEnable() == false) {
		return;
	}

	/////////////////////////////////////////////////////////////
	// Camera Lock하는 곳으로 옮겨야합니다. by jeremy
	D3DVIEWPORT9 savedviewport;
	pDevice->GetViewport(&savedviewport);
	/////////////////////////////////////////////////////////////

	UINT uiCount = m_pRttList.size();
	for (UINT i=0; i<m_pRttList.size(); i++)
	{
		CBsGenerateTexture* pBsGTex = m_pRttList[i];
		if(pBsGTex->IsEnable() != true){
			continue;
		}


		pBsGTex->Render(pDevice);

	}

	/////////////////////////////////////////////////////////////
	// Camera Lock하는 곳으로 옮겨야합니다. by jeremy
	pDevice->SetViewport(&savedviewport);
	/////////////////////////////////////////////////////////////
}

int	CBsOffScreenManager::GetRTTextureID(DWORD hHandle)
{
	CBsGenerateTexture* pBsGt = GetRTTextrue(hHandle);
	if(pBsGt == NULL)
	{
		return -1;
	}

	return pBsGt->GetTextureID();
}

SIZE CBsOffScreenManager::GetRTTSize(DWORD hHandle)
{
	SIZE size = { 0, 0 };
	int nTexId = GetRTTextureID(hHandle);
	if(nTexId != -1){
		size = g_BsKernel.GetTextureSize(nTexId);
	}

	return size;
}

/*
LPDIRECT3DSURFACE9 CBsOffScreenManager::GetRTSurface(DWORD hHandle)
{
CBsGenerateTexture* pBsGt = GetRTTextrue(hHandle);
if(pBsGt == NULL)
{
return NULL;
}

return pBsGt->GetSurface();
}
*/

CBsGenerateTexture* CBsOffScreenManager::GetRTTextrue(DWORD hHandle)
{
	if(IsValid(hHandle) == false) {
		return NULL;
	}

	return (CBsGenerateTexture*)hHandle;
}

bool CBsOffScreenManager::IsValid(DWORD hHandle)
{
	if (hHandle == NULL)
		return false;
	return (((CBsGenerateTexture*)hHandle)->GetTextureID() != -1);
}