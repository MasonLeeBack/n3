#ifndef _FCFXMANAGER_H_
#define _FCFXMANAGER_H_

#include "BsCustomRender.h"
#include "FcFXCommon.h"

class CFcFXBase;
class C3DDevice;

struct BOOKEDMSG
{
	int nHandle;
	int nCode;
	DWORD dwParam1;
	DWORD dwParam2; 
	DWORD dwParam3;
	
	BOOKEDMSG()
	{
		ZeroMemory( this, sizeof(BOOKEDMSG) );
	}
};


struct BOOKEDFX
{
	FX_TYPE eType;
	int		nHandle;
};

class CFcFXManager : public CBsCustomRender
{
//	std::vector< CFcFXBase* >	m_FXVector;
	CFcFXBase**					m_FXVector;

	std::vector< BOOKEDMSG >	m_listMSG;
//	std::vector< BOOKEDFX >		m_listCreateBook;
	
	

	std::vector< int >	m_PreLoadTexIdxs;	
	std::vector< int >	m_PreMaterialIdxs;
//	std::vector< int >	m_PreLoadSkinIdxs;
//	std::vector< int >	m_PreLoadBfxIdxs;


	std::vector< int >	m_PreLoadTexIdxsChr;	
	std::vector< int >	m_PreMaterialIdxsChr;
	std::vector< int >	m_PreLoadSkinIdxsChr;
	std::vector< int >	m_PreLoadAniIdxsChr;
	std::vector< int >	m_PreLoadBfxIdxsChr;



	int		m_nMaxFx;
	DWORD		m_dwLastProcessTick;

	CBsCriticalSection	m_csFxManager;
public:
	CFcFXManager();
	virtual~CFcFXManager();

#ifdef _DEBUG
	int m_nCurrentFXCount;
#endif

	void Process();

	void Render(C3DDevice *pDevice);
	void Reset();
	int	GetEmptySlot();
	int Create( FX_TYPE eType);
	int SendMessage(int nHandle, int nCode, DWORD dwParam1=0, DWORD dwParam2=0, DWORD dwParam3=0);
	void PreUpdate();
	void Update();

//	int  CreateInRenderThread( FX_TYPE eType);
//	void CreateBookedFX();
	void ProcessBookedMessage();
	int  SendMessageQueue(int nHandle, int nCode, DWORD dwParam1=0, DWORD dwParam2=0, DWORD dwParam3=0);


	void PreLoadByChar(int nChar, bool p_bPlayer = true);
	void ReleasePreLoadedByChar();
};

extern CFcFXManager *g_pFcFXManager;

#endif