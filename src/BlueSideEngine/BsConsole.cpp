#include "StdAfx.h"
#include "BsKernel.h"
#include "BsConsole.h"

CBsConsole::CBsConsole()
{
	m_nCurString=-1;
	m_nStringCount=0;
	m_nScrollIndex=0;
	m_bShow=false;
}

CBsConsole::~CBsConsole()
{
}

void CBsConsole::Initialize(C3DDevice *pDevice)
{
	m_nConsoleWidth=pDevice->GetBackBufferWidth();
	m_nConsoleHeight=pDevice->GetBackBufferWidth()/3;
	m_nConsoleLineCount=m_nConsoleHeight/DEBUG_FONT_HEIGHT;
	m_Color.r=0.5f;
	m_Color.g=0.5f;
	m_Color.b=0.5f;
	m_Color.a=0.7f;

	g_BsKernel.CreateUI();
}

void CBsConsole::ChangeString(const char *pString)
{
	if(m_nStringCount){
		m_StringList[m_nCurString]=pString;
		if(m_bShow){
			m_nScrollIndex=0;
		}
	}
}

void CBsConsole::AddString(const char *pString, ADD_STRING_FLAG Flag)
{
	m_nCurString++;
	if(m_nCurString>=MAX_CONSOLE_STRING_COUNT){
		m_nCurString=0;
	}
	if(m_nStringCount<MAX_CONSOLE_STRING_COUNT){
		m_nStringCount++;
	}

	m_StringList[m_nCurString]=pString;
	if(m_bShow){
		m_nScrollIndex=0;
	}
}

void CBsConsole::AddFormatStringV( const char* format, va_list pArg )
{
	char buf[1024];

	_vsnprintf( buf, 1024, format, pArg );
	AddString(buf);
}

void CBsConsole::DeleteString()
{
	if(m_nStringCount){
		m_StringList[m_nCurString]="";
		m_nCurString--;
		m_nStringCount--;
		if(m_nCurString<0){
			m_nCurString+=MAX_CONSOLE_STRING_COUNT;
		}
	}
}

void CBsConsole::Scroll(int nLine)
{
	int nScroll;

	if(!m_bShow){
		return;
	}

	nScroll=m_nScrollIndex+nLine;
	if(nScroll<0){
		return;
	}
	if(nScroll+m_nConsoleLineCount>m_nStringCount){
		return;
	}
	m_nScrollIndex+=nLine;
}

void CBsConsole::AddFormatString( const char* format, ... )
{
	va_list	arg;

	va_start( arg, format );
	AddFormatStringV( format, arg );
	va_end( arg );
}

void CBsConsole::ProcessConsole()
{	
	if(m_bShow){
		
		g_BsKernel.SetUIViewerIndex(_UI_FULL_VIEWER);
		
		g_BsKernel.DrawUIBox_s(_Ui_Mode_Box, 0, 0, m_nConsoleWidth, m_nConsoleHeight, 0.f, m_Color);
		
		for(int i=0;i<m_nConsoleLineCount;i++){
			if(i>=m_nStringCount){
				break;
			}
			int nIndex=m_nCurString-i-m_nScrollIndex;
			if(nIndex<0){
				nIndex+=MAX_CONSOLE_STRING_COUNT;
			}
			
			D3DCOLORVALUE color = {1.f, 1.f, 1.f, 1.f};
			g_BsKernel.CreateUIDebug(SCREEN_HORI_GAP,
				m_nConsoleHeight-(i+1)*DEBUG_FONT_HEIGHT,
				(char *)m_StringList[nIndex].c_str(),
				color);
		}
	}
}
