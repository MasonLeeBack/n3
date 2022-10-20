#ifndef _BsUi_IMAGECTRL_H_
#define _BsUi_IMAGECTRL_H_

#include "BSuiWindow.h"

struct ImageCtrlInfo
{
public:
	ImageCtrlInfo()
	{
		nTexId = -1;
		bLocal = false;
		u1 = v1 = u2 = v2 = 0;
		bRealSize = false;
		bReverse = false;
		nRot = 0;
		memset(cFileName, 0, 256);
	};

	int		nTexId;
	bool	bLocal;
	int		u1, v1, u2, v2;
	bool	bRealSize;
	bool	bReverse;
	int		nRot;
	char	cFileName[256];
};

// 버튼 클래스
class BsUiImageCtrl : public BsUiWindow {
protected:

public:
	BsUiImageCtrl();
	BsUiImageCtrl(BsUiCLASS hClass, POINT pos, BsUiWindow* pParent);
	virtual ~BsUiImageCtrl() { Release(); }

	bool		Create(BsUiCLASS hClass, int x, int y, BsUiWindow* pParent);
	void		Release();

	virtual DWORD		ProcMessage(xwMessageToken* pMsgToken);

	int				LoadImage(char* szFileName, bool bLocal = false);
	bool			SetImageInfo(ImageCtrlInfo* Img);
	ImageCtrlInfo*	GetImageInfo()						{ return &m_Img; }

	void			SetRenderTargetTexture(bool bFlag)	{ m_bRtt = bFlag; }
	bool			IsRenderTargetTexture()				{ return m_bRtt; }

	void			SetPortraitTexture(bool bFlag)	{ m_bPortrait = bFlag; }
	bool			IsPortraitTexture()				{ return m_bPortrait; }
	
protected:
	void 			GetImageFileName(char* pSrcFileName, char* pDscFileName, const size_t pDscFileName_len, bool bLocal); //aleksger - safe string
	void			GetFileNameforLocal(char* pSrcFileName);

protected:
	ImageCtrlInfo		m_Img;
	bool				m_bRtt;
	bool				m_bPortrait;
};

#endif

