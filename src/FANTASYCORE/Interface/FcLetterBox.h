#ifndef __FC_LETTER_BOX_H__
#define __FC_LETTER_BOX_H__
#define STR_LETTERBOX_MAX	1024

#include "FCinterfaceObj.h"

class CFcLetterBox : public CFcInterfaceObj
{
public:
	CFcLetterBox();
	~CFcLetterBox();
	
	void Process();
	void Update();
	void Show(bool bShow)				{ ShowLetterBox(bShow); }
	void Clear();
	void ShowLetterBox(bool bShow);
	void ShowText( int nTextTableID );
	bool IsShowText();
	void HideText();
	bool IsShowLetterBox()				{ return m_bLetterBoxShow; }
	float GetAlpha()					{ return m_fAlpha; }
	void SetLetterOn(bool bOn)			{ m_bLetterOn = bOn; }

protected:
	RECT	m_Top;
	RECT	m_Bottom;
	float	m_fAlpha;
	char	m_szStr[STR_LETTERBOX_MAX];
	int		m_nPlayTime;
	int		m_nSaveTick;
	bool	m_bSoundUse;
	bool	m_bLetterBoxShow;
	int		m_nSoundHandle;
	bool	m_bMsgShow;
	bool	m_bLetterOn;
	int		m_nLineCount;
};

#endif