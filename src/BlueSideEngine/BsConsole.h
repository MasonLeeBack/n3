#pragma once

#include "Singleton.h"

#define MAX_CONSOLE_STRING_COUNT	500
#define DEBUG_FONT_HEIGHT			16
#define SCREEN_HORI_GAP				60

class C3DDevice;

class CBsConsole : public CSingleton<CBsConsole>
{
public:
	CBsConsole();
	~CBsConsole();

	enum ADD_STRING_FLAG {
		NONE = 0,
		ASSERT = 1,
	};

protected:
	int m_nCurString;
	int m_nStringCount;
	int m_nScrollIndex;
	std::string m_StringList[MAX_CONSOLE_STRING_COUNT];
	std::vector<int> m_UIHandleList;

	int m_nConsoleWidth;
	int m_nConsoleHeight;
	int m_nConsoleLineCount;
	D3DCOLORVALUE m_Color;

	bool m_bShow;

public:
	void Initialize(C3DDevice *pDevice);

	void ChangeString(const char *pString);
	void AddString(const char *pString, ADD_STRING_FLAG Flag = NONE );
	void AddFormatStringV( const char* format, va_list pArg );
	void AddFormatString( const char* format, ... );
	void DeleteString();
	void Scroll(int nLine);

	bool IsShowConsole()			{ return m_bShow; }
	void SetShowConsole(bool bShow)	{ m_bShow = bShow; }
	void ProcessConsole();
};