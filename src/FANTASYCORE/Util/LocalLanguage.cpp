#include "stdafx.h"
#include "LocalLanguage.h"
#include "BsKernel.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG


LocalLanguage g_LocalLanguage;

LocalLanguage::LocalLanguage()
{
	SetLanguage(LANGUAGE_JPN);
	SetVoiceLanguage(LANGUAGE_JPN);
}

void LocalLanguage::SetLanguage( GAME_LANGUAGE language )
{
	m_languageDir = LANGUAGE_DIR_STRINGS[language];
	m_language = language;
}

void LocalLanguage::SetVoiceLanguage( GAME_LANGUAGE language )
{
	m_voiceLanguageDir = LANGUAGE_DIR_STRINGS[language];
	//m_config.voiceLanguage = language;
}