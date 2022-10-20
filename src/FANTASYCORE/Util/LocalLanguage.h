#ifndef _LOCALLANGUAGE_H_
#define _LOCALLANGUAGE_H_

enum GAME_LANGUAGE {
	LANGUAGE_ENG = 0,
	LANGUAGE_KOR = 1,
	LANGUAGE_JPN = 2,		//Japanese
	LANGUAGE_GER = 3,		//German
	LANGUAGE_FRA = 4,		//French
	LANGUAGE_SPA = 5,		//Spanish
	LANGUAGE_ITA = 6,		//Italian
	LANGUAGE_CHN = 7,		//Chinese
	// ...
	MAX_LANGAUGES
};

static const char* LANGUAGE_DIR_STRINGS[MAX_LANGAUGES] =
{ "ENG", "KOR", "JPN", "GER", "FRA", "SPA", "ITA", "CHN"};

static bool LANGUAGE_Enable[MAX_LANGAUGES] =
{ true, true, true, false, false, false, false, false};

class LocalLanguage
{
public:
	LocalLanguage();
	~LocalLanguage()	{}

protected:
	GAME_LANGUAGE m_language;
	const char* m_languageDir;
	const char* m_voiceLanguageDir;

public:
	GAME_LANGUAGE GetLanguage()							{ return m_language; }
	void SetLanguage( GAME_LANGUAGE language );
	void SetVoiceLanguage( GAME_LANGUAGE language );

	const char* GetLanguageDir()						{ return m_languageDir; }
	const char* GetLanguageStr()						{ return m_languageDir; }
	const char* GetVoiceLanguageDir()					{ return m_voiceLanguageDir; }
	const char* GetVoiceLanguageStr()					{ return m_voiceLanguageDir; }
	const char* GetLanguageDir(GAME_LANGUAGE language)	{ return LANGUAGE_DIR_STRINGS[language]; }
	const char* GetLanguageStr(GAME_LANGUAGE language)	{ return LANGUAGE_DIR_STRINGS[language]; }

	bool		GetLocalEnable(GAME_LANGUAGE language)	{ return LANGUAGE_Enable[language]; }
};


extern LocalLanguage g_LocalLanguage;

#endif