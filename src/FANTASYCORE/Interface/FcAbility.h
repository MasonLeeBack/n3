#ifndef __FC_ABILITY_H__
#define __FC_ABILITY_H__
#include "FCinterfaceObj.h"
#include "FcInterfaceDataDefine.h"
#include "FcMenu3DObject.h"
#include "FcSpeech.h"

class CFcUVList;

#define MAX_ICON_NUM	4
#define ICON_LEFT		0
#define ICON_RIGHT		1
#define ICON_TOP		2
#define ICON_BOTTOM		3

enum{
	_GUADIAN_IN,
	_GUADIAN_OUT,
	_GUADIAN_ATTACK,
	_GUADIAN_DEFANCE,
};


enum GUARDIAN_VOICE_TYPE
{
	GUARDIAN_ADD = 0,
	GUARDIAN_REMOVE,
	GUARDIAN_ATTACK,
	GUARDIAN_DEFENSE,
};


struct VOICEQUE_INFO
{
	char cName[32];
	int nCurTick;
	int nPlayerSoundID;
	GUARDIAN_VOICE_TYPE Type;
};

class FcAbilInterface : public CFcInterfaceObj
{	
public:
	FcAbilInterface(CFcPortrait *pPortrait);
	~FcAbilInterface();
	
	void Process();
	void KeyInput(int nPlayerID, int KeyType,int PressTick);
	void Update();

protected:
	void ReleaseData();

	void DrawGuardianTarget();
	void DrawEnemyTarget();
	void DrawGuardianList();
	void DrawDurationItem();
	
	void ProcessGuardianVoice();

	void SetGuardianSign(CFcTroopObject* pTroopObj, int nState, bool bLeft);
	void DrawGuardianSign();

	void GetGuardianVoiceQueStr( GUARDIAN_VOICE_TYPE Type, char* pStr );

	bool IsEnableGuardian();

	bool IsEnablePlayerVoice();


protected:
	bool	   m_bPlay;
	int		   m_nCurPos;
	CFcMenu3DObject*	m_pEnemyTarget;

	int			m_nDecalTexIndex;
	int			m_nDecalTargetTexIndex;

	//draw guadian sign			// TGS version : 나중에 음성이 들어가면 얼굴은 뺀다.
	CFcPortrait*	m_pPortrait;
	int			m_nCmdPortraitId;
	int			m_nAnswerPortraitId;
	char		m_cCmdText[256];
	char		m_cAnswerText[256];
	int			m_nGuadianSignTick;

	int			m_nPlayerVoiceHandle[4];
	int			m_nGuardianVoiceHandle[4];

	std::vector<VOICEQUE_INFO> m_vecGuardianVoice;
};

#endif