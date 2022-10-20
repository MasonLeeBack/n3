#include "stdafx.h"

#include "BsUiImageCtrl.h"

#include "BsKernel.h"
#include "BsUiSystem.h"
#include "LocalLanguage.h"

#include "BsUiMenuInput.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG


////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

BsUiImageCtrl::BsUiImageCtrl() : BsUiWindow()
{
	m_bRtt = false;
	m_bPortrait = false;
}

BsUiImageCtrl::BsUiImageCtrl(BsUiCLASS hClass, POINT pos, BsUiWindow* pParent)
{
	BsAssert(hClass);

	BsUiWindow::Create(BsUiTYPE_IMAGECTRL,
		hClass,
		pos.x, pos.y,
		100, 100,
		XWATTR_SHOWWINDOW | XWATTR_MOVABLE | XWATTR_DISABLE,
		pParent);
}

void BsUiImageCtrl::Release()
{
	//Render Target Texture는 rttcamera에서 자동 삭제하므로 여기서 지우면 안된다.
	//Portrait는 manager에서 관리하므로 여기서 지우면 안된다.
	if(m_bRtt == false && m_bPortrait == false){
		SAFE_RELEASE_TEXTURE(m_Img.nTexId);
	}
}

bool BsUiImageCtrl::Create(BsUiCLASS hClass, int x, int y, BsUiWindow* pParent)
{
	BsAssert(hClass != 0);

	BsUiImageCtrlItem* pImageItem = static_cast <BsUiImageCtrlItem* > (g_BsUiSystem.GetLoader()->GetClassInstance(hClass));
	BsAssert(pImageItem);

	bool bLocal = (pImageItem->nLocal)? true:false;
	//Image
	if(strlen(pImageItem->szFileName) > 0)
	{	
		if(pImageItem->nImageMode == _IMAGE_MODE_FILE){
			m_Img.nTexId = LoadImage(pImageItem->szFileName, bLocal);
		}
		else{
			GetImageFileName(pImageItem->szFileName, m_Img.cFileName, _countof(m_Img.cFileName), bLocal); //aleksger - safe string
		}
	}

	m_Img.bLocal = bLocal;
	m_Img.bRealSize = (pImageItem->nRealSize)? true:false;
	m_Img.bReverse = (pImageItem->nReverse)? true:false;
	m_Img.u1 = pImageItem->u1;	m_Img.v1 = pImageItem->v1;
	m_Img.u2 = pImageItem->u2;	m_Img.v2 = pImageItem->v2;
	m_Img.nRot = pImageItem->nRot;
	

	return BsUiWindow::Create(pImageItem->nType, 
		hClass, x, y, 
		pImageItem->size.cx, pImageItem->size.cy, 
		pImageItem->nAttr, pParent);
}

DWORD BsUiImageCtrl::ProcMessage(xwMessageToken* pMsgToken)
{
	BsAssert(pMsgToken);

	BsUiCLASS hClass = BsUi::BsUiGetClass(pMsgToken->hWnd);
	BsAssert(hClass);

	switch (pMsgToken->message)
	{
	case XWMSG_LBUTTONDOWN:
		{
			BsUiSystem::SetGrabWindow(this);
		}
		break;
	}

	return BsUiWindow::ProcMessage(pMsgToken);
}


int BsUiImageCtrl::LoadImage(char* szFileName, bool bLocal)
{
	if(szFileName == NULL){
		return -1;
	}

	char cDscFileName[256];
	GetImageFileName(szFileName, cDscFileName, _countof(cDscFileName), bLocal); //aleksger - safe string
		
	int nTextureId = CBsKernel::GetInstance().LoadTexture(cDscFileName);
	if(nTextureId == -1){
		return -1;
	}
	
	SIZE size = g_BsKernel.GetTextureSize(nTextureId);

	m_Img.nTexId = nTextureId;
	m_Img.bLocal = bLocal;
	m_Img.u1 = 0;
	m_Img.v1 = 0;
	m_Img.u2 = size.cx;
	m_Img.v2 = size.cy;
	strcpy(m_Img.cFileName, cDscFileName);

	m_Img.bRealSize = false;
	m_Img.bReverse = false;
	m_Img.nRot = 0;
	
	return nTextureId;
}

bool BsUiImageCtrl::SetImageInfo(ImageCtrlInfo* pImg)
{
	if(m_bRtt == false && m_bPortrait == false){	
		SAFE_RELEASE_TEXTURE(m_Img.nTexId);
	}

	m_Img.nTexId = pImg->nTexId;
	m_Img.bLocal = pImg->bLocal;
	m_Img.bRealSize = pImg->bRealSize;
	m_Img.bReverse = pImg->bReverse;
	m_Img.u1 = pImg->u1;
	m_Img.v1 = pImg->v1;
	m_Img.u2 = pImg->u2;
	m_Img.v2 = pImg->v2;
	m_Img.nRot = pImg->nRot;
	strcpy(m_Img.cFileName, pImg->cFileName);

	return true;
}

void BsUiImageCtrl::GetImageFileName(char* pSrcFileName, char* pDscFileName, size_t pDscFileName_size, bool bLocal)
{	
	char *pFindPtr;
	pFindPtr=(char*)strchr(pSrcFileName, '\\');

	bool bDataFolder = false;
	if(pFindPtr)
	{	
		while(1)
		{			
			if(pFindPtr == NULL){
				break;
			}

			pFindPtr = pFindPtr + 1;

			if(_strnicmp(pFindPtr, "Data\\", 5) == 0){
				strcpy_s(pDscFileName, pDscFileName_size, pFindPtr+5); //aleksger - safe string
				bDataFolder = true;
				break;
			}

			pFindPtr = strchr(pFindPtr, '\\');
		}
	}
	
	if(bDataFolder == false){
		strcpy_s(pDscFileName, pDscFileName_size, pSrcFileName); //aleksger - safe string
	}
	
	if(bLocal){
		GetFileNameforLocal(pDscFileName);
	}
}

void BsUiImageCtrl::GetFileNameforLocal(char* pSrcFileName)
{
	if(pSrcFileName == NULL){
		return;
	}

	int nLen = strlen(pSrcFileName);
	if(nLen == 0){
		return;
	}

	char *ptempPtr = new char[nLen+1];
	strcpy_s(ptempPtr, nLen+1, pSrcFileName); //aleksger - safe string
	char* pFindPtr = ptempPtr;

	char cLocal[16];
	memset(cLocal, 16, 0);
	cLocal[0] = '\\';
	cLocal[1] = pSrcFileName[nLen-7];
	cLocal[2] = pSrcFileName[nLen-6];
	cLocal[3] = pSrcFileName[nLen-5];
	cLocal[4] = '\\';
	cLocal[5] = '\0';

	int nCount = 0;
	if(pFindPtr)
	{	
		while(1)
		{	
			if(strlen(pFindPtr) == 0){
				break;
			}
				
			if(pFindPtr == NULL){
				break;
			}

			if(_strnicmp(pFindPtr, cLocal, 5) == 0)
			{
				char cCurLocal[16];
				memset(cCurLocal, 16, 0);
				strcpy(cCurLocal, g_LocalLanguage.GetLanguageStr());

				pSrcFileName[nCount+1] = cCurLocal[0];
				pSrcFileName[nCount+2] = cCurLocal[1];
				pSrcFileName[nCount+3] = cCurLocal[2];

				pSrcFileName[nLen-7] = cCurLocal[0];
				pSrcFileName[nLen-6] = cCurLocal[1];
				pSrcFileName[nLen-5] = cCurLocal[2];
				break;
			}

			pFindPtr = pFindPtr + 1;
			nCount++;
		}
	}

	delete[] ptempPtr;
}