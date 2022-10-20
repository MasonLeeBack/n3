#pragma once

#include "SmartPtr.h"
#include "FcCommon.h"

class CFcBaseObject;

struct KEY_EVENT_PARAM
{
	int nPosX;
	int nPosY;
};

#define PAD_INPUT_A							0
#define PAD_INPUT_B							1
#define PAD_INPUT_X							2
#define PAD_INPUT_Y							3
#define PAD_INPUT_BLACK						4			// PAD_INPUT_R
#define PAD_INPUT_WHITE						5			// PAD_INPUT_L
#define PAD_INPUT_UP						6
#define PAD_INPUT_DOWN						7
#define PAD_INPUT_LEFT						8
#define PAD_INPUT_RIGHT						9
#define PAD_INPUT_START						10
#define PAD_INPUT_BACK						11
#define PAD_INPUT_LTHUMB					12
#define PAD_INPUT_RTHUMB					13
#define PAD_INPUT_LTRIGGER					14
#define PAD_INPUT_RTRIGGER					15
#define PAD_INPUT_LSTICK					16
#define PAD_INPUT_RSTICK					17

#define PAD_INPUT_COUNT						18
#define PAD_INPUT_EVENT_RESTORE_KEY			-1
#define PAD_MAX_COUNT						4

class CInput
{
public:
	CInput();
	virtual ~CInput();

protected:
	bool m_bEnable;
	bool m_bBreak; //이벤트시에나 레터박스 같은 상황을 위해
	bool m_bHookOn;
	CSmartPtr<CFcBaseObject> m_AttachHandle[MAX_PLAYER_COUNT];


public:
	void InitState();
	virtual int Create( int nPort )=0;
	virtual int Process()=0;
	void Enable(bool bEnable) { m_bEnable=bEnable; }
	bool IsEnable() { return m_bEnable; }
	void HookOn(bool bHookOn){m_bHookOn = bHookOn;}
	void Break(bool bBreak);
	bool IsBreak(){return m_bBreak;}
	bool IsHookOn(){return m_bHookOn;}
	void ClearAttachObject();
	void AttachObject(int nPort, CSmartPtr<CFcBaseObject> Handle) { m_AttachHandle[nPort]=Handle; }
	virtual void SendKeyEvent(int nPort, int nKeyCode, KEY_EVENT_PARAM *pParam);
};

