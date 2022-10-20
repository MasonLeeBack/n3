#include "stdafx.h"
#include "FcInterfaceMisc.h"
#include "BSKernel.h"
#include "BsUiFont.h"
#include "FcGlobal.h"
#include "TextTable.h"
#include "FcSoundManager.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG

/*
FcCommonInterface::FcCommonInterface()
{
}

void FcCommonInterface::Load(char *FileName)
{
    g_BsKernel.chdir("interface");
    m_TexInfo.Load(FileName);
    g_BsKernel.chdir("..");
}
*/

//----------------------------------------------------------------------------------------------------
CFcFadeEffect::CFcFadeEffect()
{
    m_Color       = D3DXVECTOR3(0,0,0);
    m_nTargetTick = -1;
    m_nCurTick    = 0;
    m_fAlpha      = 0.f;
    m_bFadeOut     = false;
    g_BsKernel.GetDevice()->GetViewport( &m_ViewPort);
	m_nSaveTick = -1;
	m_bStop = true;

}

CFcFadeEffect::~CFcFadeEffect()
{
}

void CFcFadeEffect::SetFadeOut(int nSec,D3DXVECTOR3 *pColor)
{
	if( m_bStop == false && m_bFadeOut == false )
	{
//		BsAssert( 0 && "Fade error!" );		// 트리거 액션에서 동시에 FadeOut, FadeIn에 들어왔을 경우 하나는 제대로 처리 안된다.
		DebugString( "Fade error! 트리거가 제대로 작동 안 할 수 있습니다.\n" );
	}

    m_Color = D3DXVECTOR3(0,0,0);
    if(pColor){ m_Color = *pColor; }

    m_nTargetTick   = nSec * FRAME_PER_SEC;
    m_nCurTick      = 0;
	if(m_nTargetTick == 0){
		m_fAlpha = 1.f;
	}
	else{
		m_fAlpha = 0.f;
	}
    m_bFadeOut      = true;
	m_nSaveTick		= GetProcessTick();
	m_bStop			= false;
}

void CFcFadeEffect::SetFadeIn(int nSec)
{
	if( m_bStop == false && m_bFadeOut == true )
	{
//		BsAssert( 0 && "Fade error!" );		// 트리거 액션에서 동시에 FadeOut, FadeIn에 들어왔을 경우 하나는 제대로 처리 안된다.
		DebugString( "Fade error! 트리거가 제대로 작동 안 할 수 있습니다.\n" );
	}

    m_nTargetTick = nSec * FRAME_PER_SEC;
    m_nCurTick    = m_nTargetTick;
    m_bFadeOut    = false;
	m_nSaveTick = GetProcessTick();
	m_bStop		  = false;
}

void CFcFadeEffect::Process()
{
	if(m_bStop)
		return;

	if(GetProcessTick() - m_nSaveTick == 0){
		return;
	}
	else{
		m_nSaveTick = GetProcessTick();
	}

	if( m_bFadeOut ){
		if(m_nCurTick < m_nTargetTick){           
			++m_nCurTick;            
		}
		if( m_nTargetTick == 0) {		
			m_fAlpha = 1.f;
		}
	}
	else{
		if(m_nCurTick > 0){
			--m_nCurTick;
		}
		else{
			m_bStop = true;
		}

		if( m_nTargetTick == 0) {		
			m_fAlpha = 0.f;
			m_bStop = true;
		}
	}
	
	if(m_nTargetTick != 0) {	
		m_fAlpha = (float)m_nCurTick / (float)m_nTargetTick;
	}
}

void CFcFadeEffect::Update()
{    
	D3DXCOLOR color = D3DXCOLOR(m_Color.x,m_Color.y,m_Color.z,m_fAlpha);
	if(m_bStop == false){
		g_BsKernel.DrawUIBox_s(_Ui_Mode_Box, 0 ,0 , m_ViewPort.Width , m_ViewPort.Height, 0.f, color);
	}
}

bool CFcFadeEffect::IsFadeInStop()
{
	if(m_nCurTick == 0)
		return true;
	return false;
}

bool CFcFadeEffect::IsFadeOutStop()
{
	if(m_nCurTick == m_nTargetTick)
		return true;
	return false;
}
//----------------------------------------------------------------------------------------------------
CFcMsgShow::CFcMsgShow()
{
	m_nX = 0;
	m_nY = 0;
	memset( m_szStr , 0 , MSG_SHOW_MAX_STR );
	m_nCurSec = 0;
	m_nSaveProcessTick = -1;
	m_TargetSec = 0;	
	m_bSoundUse = false;
	m_nSoundHandle = -1;
}

void CFcMsgShow::SetMsgShowXY( int nX,int nY,int nSec,int nTextTableID )
{
	char szSoundCueName[64] = {0};
	m_nX = nX;
	m_nY = nY;
	m_nSaveProcessTick = -1;
	m_nCurSec = 0;	

	g_TextTable->GetText( nTextTableID, m_szStr, _countof(m_szStr), szSoundCueName, _countof(szSoundCueName));

	if( strlen(szSoundCueName) > 0 )
	{
		m_bSoundUse = true;
		m_TargetSec = 0;
		m_nSoundHandle = g_pSoundManager->PlaySound( SB_VOICE_IN_GAME,szSoundCueName,&m_nSoundHandle );
		BsAssert(m_nSoundHandle != -1 && "None voice handle");
	}
	else
	{
		m_nSoundHandle = -1;
		m_TargetSec = nSec;
		m_bSoundUse = false;
	}

}

void CFcMsgShow::Process()
{
	if(m_bSoundUse == false)
	{
		if(m_nCurSec < m_TargetSec){
			if(m_nSaveProcessTick == -1){	
				m_nSaveProcessTick = GetProcessTick();
			}
			if(GetProcessTick() - m_nSaveProcessTick >= FRAME_PER_SEC){
				m_nSaveProcessTick = GetProcessTick();
				m_nCurSec++;
			}
		}
	}
}

void CFcMsgShow::Draw()
{
	if((m_nSoundHandle != -1) || (m_nCurSec < m_TargetSec)) {		
		sprintf(m_szStrTemp,"%s%s","@(color=255,255,255,255)@(scale=1.0,1.0)@(align=1)",m_szStr);
		g_pFont->DrawUIText(m_nX,m_nY,-1,-1,m_szStrTemp);
	}

}

bool CFcMsgShow::IsMsgShowXYEnd()
{
	if(m_bSoundUse)
	{
		if(m_nSoundHandle == -1){ return true; }
	}
	else{
		if(m_nCurSec >= m_TargetSec){ return true; }
	}
	return false;
}