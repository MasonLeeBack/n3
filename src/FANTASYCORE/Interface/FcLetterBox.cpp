#include "stdafx.h"
#include "FcLetterBox.h"
#include "BSKernel.h"
#include "FcGlobal.h"
#include "BsUiFont.h"
#include "TextTable.h"
#include "FcSoundManager.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG



#define LETTERBOX_SCALE				108
#define LETTERBOX_DELAY_MIN			50
#define LETTERBOX_SCREEN_DELAY		40.f

CFcLetterBox::CFcLetterBox()
{
	D3DVIEWPORT9 ViewPort;

	g_BsKernel.GetDevice()->GetViewport( &ViewPort );
	m_Top.left = 0;
	m_Top.top = 0;
	m_Top.right = (int)ViewPort.Width;
	m_Top.bottom = LETTERBOX_SCALE;

	m_Bottom.left = 0;
	m_Bottom.top = (int)ViewPort.Height-LETTERBOX_SCALE;
	m_Bottom.right = (int)ViewPort.Width;
	m_Bottom.bottom = LETTERBOX_SCALE;

	m_fAlpha = 0.f;
	m_bLetterBoxShow = false;
	m_nPlayTime = 0;
	m_nSaveTick = 0;
	m_nSoundHandle = -1;
	m_bMsgShow = false;
	m_bLetterOn = true;

	m_nLineCount = 0;
}

CFcLetterBox::~CFcLetterBox()
{
}

void CFcLetterBox::ShowLetterBox(bool bShow)
{
	m_bLetterBoxShow = bShow;
}

bool CFcLetterBox::IsShowText()
{
	if(m_bSoundUse)
	{
		if(m_nSoundHandle == -1)
			return true;
	}
	else{
		if(m_nPlayTime <= 0){
			return true;
		}
	}
	return false;
}

void CFcLetterBox::HideText()
{
	m_nPlayTime = 0;
	if( m_nSoundHandle != -1 )
	{
#ifdef _XBOX
		g_pSoundManager->StopSound( m_nSoundHandle, false );
#endif
		m_nSoundHandle = -1;
	}
}


void CFcLetterBox::ShowText( int nTextTableID )
{
	m_bMsgShow = true;
	char szSoundCueName[64] = {0};
	g_TextTable->GetText( nTextTableID, m_szStr, _countof(m_szStr), szSoundCueName, _countof(szSoundCueName));

	int nWidth(0), nHeight(0), nWordCount(0);
	g_pFont->GetTextLengthInfo(nWidth, nHeight, m_nLineCount, nWordCount,
		0, 0, -1, -1, m_szStr);

	if(strlen(szSoundCueName) > 0)
	{
		m_nPlayTime = 0;
		m_bSoundUse = true;
		m_nSoundHandle = g_pSoundManager->PlaySound( SB_VOICE_IN_GAME,szSoundCueName,&m_nSoundHandle );
		BsAssert(m_nSoundHandle != -1 && "None voice handle");
	}
	else
	{
		m_bSoundUse = false;
		m_nSoundHandle = -1;
		
		m_nPlayTime = (int)(nWordCount * 0.2f);

		if(m_nPlayTime < LETTERBOX_DELAY_MIN){
			m_nPlayTime = LETTERBOX_DELAY_MIN;
		}
	}
}

void CFcLetterBox::Process()
{
	if(m_nSaveTick == GetProcessTick()) { return; }
	m_nSaveTick = GetProcessTick();

	if(m_bLetterBoxShow)
	{
		if(m_fAlpha < 1.f){
			m_fAlpha += (1.f/LETTERBOX_SCREEN_DELAY);
			if(m_fAlpha > 1.f){
				m_fAlpha = 1.f;
			}
		}
	}
	else{
		if(m_fAlpha > 0.f){
			m_fAlpha -= (1.f/LETTERBOX_SCREEN_DELAY);
			if(m_fAlpha < 0.f){
				m_fAlpha = 0.f;
			}
		}
	}

	if(m_bMsgShow)
	{
		if(m_bSoundUse)
		{
			if(m_nSoundHandle == -1)
				m_bMsgShow = false;
		}
		else{
			if(m_nPlayTime > 0){	
				m_nPlayTime --; 
			}
			else{
				m_bMsgShow = false;
			}
		}
	}
}

void CFcLetterBox::Update()
{
	if(m_bLetterBoxShow)
	{
		if(m_fAlpha > 0.f)
		{	
			g_BsKernel.DrawUIBox_s(_Ui_Mode_Box,
				m_Top.left, m_Top.top, m_Top.right, m_Top.bottom,
				0.f, D3DXCOLOR(0,0,0,m_fAlpha));

			g_BsKernel.DrawUIBox_s(_Ui_Mode_Box,
				m_Bottom.left, m_Bottom.top, m_Bottom.right, m_Bottom.bottom,
				0.f, D3DXCOLOR(0,0,0,m_fAlpha));
		}
	}

	if(m_bLetterOn && m_bMsgShow)
	{
		char szStrTemp[STR_LETTERBOX_MAX];
		int nY = m_Bottom.top+(m_Bottom.bottom / 2)-30;
		switch(m_nLineCount)
		{
		case 1:
			{
				sprintf(szStrTemp,"@(reset)@(scale=0.8,0.8)@(align=1)@(space=-3)%s", m_szStr);
				break;
			}
		case 2:
			{
				nY -= 20;
				sprintf(szStrTemp,"@(reset)@(scale=0.8,0.8)@(align=1)@(space=-3)@(linespace=-5)%s", m_szStr);
				break;
			}
		default:
			{
				nY -= 20;
				sprintf(szStrTemp,"@(reset)@(scale=0.6,0.6)@(align=1)@(space=-3)@(linespace=-10)%s", m_szStr);
				break;
			}
		}

		g_pFont->DrawUIText(m_Bottom.left, nY, m_Bottom.right, -1, szStrTemp);
	}

}


void CFcLetterBox::Clear()
{
	m_fAlpha = 0.f;
	m_bLetterBoxShow = false;
	m_nPlayTime = 0;
	m_nSaveTick = 0;
	m_nSoundHandle = -1;
	m_bMsgShow = false;
	m_bLetterOn = true;
}