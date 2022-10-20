#ifndef __FC_SPEECH_H__
#define __FC_SPEECH_H__

//#define ConvertToString(x) #x
class CFcImageInfo;
//struct UVINFO;
#define STR_SPEECH_MAX	512
#include "FCinterfaceObj.h"


struct FC_SPEECH_DATA
{
	int nPortraitID;
	int nTextTableID;
	int nPlayTime;
	int  nSoundHandle;
	int nSoundCueIndex;

	bool bSoundUse;
	bool bAddX;
	bool bRenderOut;
};

class CFcSpeech : public CFcInterfaceObj
{
public:
	CFcSpeech(CFcPortrait *pPortrait);
	~CFcSpeech();
	
	void Process();
	void Update();
	
	void SetSpeech(int nPortraitID,int nTextTableID);
	bool IsPlay();

	void SetSpeechOn(bool bOn)		{ m_bSpeechOn = bOn; }
	bool GetSpeechOn()				{ return m_bSpeechOn; }

protected:
	void ProcessSpeech();
	void DrawSpeech();

	void ProcessOnlyText();
	void DrawOnlyText();

protected:
	CFcPortrait *m_pPortrait;
	int			m_PortX;
	int			m_nSaveTick;
	char		m_szStr[STR_SPEECH_MAX];
	bool		m_bSpeechOn;

	std::vector<FC_SPEECH_DATA *>m_TextList;
};


class CFcReport : public CFcInterfaceObj
{
public:
	CFcReport();
	void Process();
	void Update();

	void SetReport(int nTexId);
	void ProcessCheckReport();

	bool IsPlay()				{return m_bPlay;}

protected:
	int			m_iPlayTime;
	char		m_szStr[STR_SPEECH_MAX];
	bool		m_bPlay;
	int			m_nSaveTick;
	int			m_nReportX;
	int			m_nBoxLength;
	int			m_nSoundHandle;
	bool		m_bSoundUse;
	bool		m_bMoveLeft;

	std::vector<int>		m_nTexList;
	bool		m_bCheckRender;
};

class CFcItemReport : public CFcInterfaceObj
{
public:
	CFcItemReport();
	~CFcItemReport();
	void Process();
	void Update();

	void SetItemReport(char* szText);

	bool IsPlay()				{return m_bPlay;}

protected:
	int			m_iPlayTime; //10분의 1초단위
	char		m_szStr[STR_SPEECH_MAX];
	bool		m_bPlay;
	int			m_nSaveTick;
	int			m_nReportX;
	int			m_nBoxLength;
	std::vector<char*>m_nTexList;

	bool		m_bCheckRender;
};


class CFcMissionGoal : public CFcInterfaceObj
{
public:
	CFcMissionGoal();
	void Process();
	void Update();
	void SetMissionGoal(char *szStr);
	bool IsPlay(){return m_bPlay;}

protected:
	int			m_iPlayTime;
	char		m_szStr[STR_SPEECH_MAX];
	bool		m_bPlay;
	int			m_nSaveTick;
	int			m_nMoveY;
};


class CFcSpecialMissionText : public CFcInterfaceObj
{
public:
	CFcSpecialMissionText();
	void Process() {}
	void Update();
	void SetText(char *szStr);

protected:
	char		m_szStr[STR_SPEECH_MAX];
};


class CFcSpecialMissionGauge : public CFcInterfaceObj
{
public:
	CFcSpecialMissionGauge();
	void Process() {}
	void Update();
	void SetValue(int nValue, int nMaxValue, char* pCaption = NULL);

protected:
	char	m_szStr[STR_SPEECH_MAX];
	int		m_nValue;
	int		m_nMaxValue;
};

#endif