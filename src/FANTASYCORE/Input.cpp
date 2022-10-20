#include "StdAfx.h"
#include "input.h"

#include "FcBaseObject.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG


CInput::CInput()
{
	InitState();
}

CInput::~CInput()
{
}

void CInput::InitState()
{
	m_bEnable=true;
	m_bHookOn = false;
	m_bBreak = false;
}

void CInput::ClearAttachObject()
{
	int i;

	for( i = 0; i < MAX_PLAYER_COUNT; i++ )
	{
		m_AttachHandle[ i ].Identity();
	}
}

void CInput::Break(bool bBreak)
{
	if( bBreak == false && bBreak != m_bBreak ) {
		for( int i = 0; i < MAX_PLAYER_COUNT; i++ ) {
			if( m_AttachHandle[i] )
				m_AttachHandle[i]->ProcessKeyEvent(PAD_INPUT_EVENT_RESTORE_KEY, NULL);
		}
	}
	m_bBreak = bBreak;
}

void CInput::SendKeyEvent(int nPort, int nKeyCode, KEY_EVENT_PARAM *pParam)
{
	if(!m_bEnable){
		return;
	}
	if(IsBreak()){
		return;
	}
	if(IsHookOn()){
		return;
	}

	if(m_AttachHandle[nPort]){
		m_AttachHandle[nPort]->ProcessKeyEvent(nKeyCode, pParam);
	}
}

