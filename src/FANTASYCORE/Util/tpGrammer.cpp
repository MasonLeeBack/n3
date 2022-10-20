#include "stdafx.h"

#include <fstream>
#include <tchar.h>
#include <stdio.h>
#include <iostream>
#include <strstream>
#include <io.h>

#include "tpGrammer.h"
#include "BsConsole.h"
#include "bstreamext.h"
#include "BSFileIO.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG


/////////////////////////////////////////////////////////////////////////////////////////////
//

// pFilename은 Fullpath로 들어와야만 한다.
bool tpGrammer::Create(const char* pFilename, const char* pHeaderPath)
{
	BsAssert(pFilename && pHeaderPath);

	ClearTokens();
	m_tokens.reserve(200);

#ifndef _BSUI_TOOL
	char szBinFileName[MAX_PATH];
	sprintf( szBinFileName, "%sb", pFilename );

	TokenBuffer tb;
	

	// 헤더가 바껴도 갱신되지 않는 문제가 있다!!
	if( tb.Load( szBinFileName ) == false ) {	
		BsAssert( 0 && "Binary 파일없음");
		return false;
	}

	while(!tb.IsEnd())
	{
		tpLEXTOKEN* pToken = new tpLEXTOKEN;
		BsAssert(pToken);

		pToken->line = tb.GetLineNumber();
		pToken->str = tb.GetText();
		pToken->type = tb.GetType();

		m_tokens.push_back(pToken);

		tb++;
	}

	m_index = 0;

#else
	// 모든걸 하나씩 떼어서 넣어둔다.
	Parser parser;
	TokenList toklist;

	parser.Create();
	parser.EnableComments( true );

	DWORD dwFileSize;
	VOID *pData;

	if( FAILED(CBsFileIO::LoadFile( pFilename, &pData, &dwFileSize ) ) )
	{
		BsAssert( 0 && "Unable to read file" );
	}

	bool result = parser.ProcessSource((const char*)pData, &toklist, dwFileSize );
	BsAssert( result && "Unable to read file" );
	CBsFileIO::FreeBuffer(pData);
	if(result == false){

		CBsConsole::GetInstance().AddFormatString("%s File Not Found", pFilename);
		return false;

	}
	parser.SetDirectory( pHeaderPath );
	// ProcessHeaders() is only needed if you're using the built-in header file
	// processing mechanism.
	result = parser.ProcessHeaders(&toklist);
	BsAssert( result && "Error processing script header" );
	
	// Macros need to be processed explicitly if they are used in headers and
	// source text.
	parser.ProcessMacros(&toklist);


	Token token;
	TokenList::iterator itr=toklist.begin();

	while(itr!=toklist.end())
	{
		tpLEXTOKEN* pToken = new tpLEXTOKEN;
		BsAssert(pToken);

		token = (*itr++);

		pToken->line = token.GetLineNumber();
		pToken->str = token.GetText();
		pToken->type = token.GetType();

		m_tokens.push_back(pToken);
	}

	m_index = 0;
#endif

	return true;
}

void tpGrammer::Release()
{
	Clear();
}

void tpGrammer::ClearTokens()
{
	while (m_tokens.empty() == false)
	{
		tpLEXTOKEN* pToken = m_tokens.back();
		BsAssert(pToken);

		delete pToken;
		m_tokens.pop_back();
	}
	m_tokens.clear();
}

void tpGrammer::Clear()
{
	tpGRAMMERMAPITOR itr = m_grammer.begin();

	while (itr != m_grammer.end())
		delete (*itr++).second;


	ClearTokens();
	m_grammer.clear();
	m_macro.clear();
	m_index = 0;
}

// 등록된 문법을 가지고 비교해보자. 단순 무식하게 비교한다. 아아 yacc ㅜㅜ
int tpGrammer::Get()
{
	// 이미 다 했다면 끝낸다.
	if (m_index >= (int)m_tokens.size())
		return 0;

	m_params.clear();

	tpGRAMMERMAPITOR itr = m_grammer.begin();

	while (itr != m_grammer.end())
	{
		int key = (*itr).first;

		tpGRAMMERITEM* pItem = (*itr++).second;
		BsAssert(pItem);

		if (IsSame(pItem->type[0], pItem->str[0], m_tokens[m_index]->type, m_tokens[m_index]->str))
		{
			int j;

			// 이후로도 쭉 같은지 검사한다.
			for (j=1; j<pItem->size; j++)
			{
				if (IsSame(pItem->type[j], pItem->str[j], m_tokens[m_index + j]->type, m_tokens[m_index + j]->str) == false)
					goto next_grammer;							// 가끔 goto도 --
			}

			// 여기까지 왔다면 문법과 일치하는 것이다. 인자를 구성시켜준다.
			for (j=0; j<pItem->size; j++)
			{
				if (pItem->str[j].empty())		// 인자로 써야 한다.
					m_params.push_back(m_tokens[m_index + j]->str);
			}

			m_line = m_tokens[m_index]->line;
			m_index += pItem->size;
			return key;
		}
next_grammer:
		;
	}

	m_line = m_tokens[m_index]->line;
	m_unknown = m_tokens[m_index]->str;

	m_index++;		// 해당 토큰은 아무런 문법과 만나지 않는다. 따라서 건너뛴다.

	return -1;
}

const char*	tpGrammer::GetParam(int index)
{
	BsAssert((int)m_params.size() > index);

	string& str = m_params[index];

	// macro에 있으면 해당하는 값으로 바꿔서 돌려준다.
	tpGRAMMERMACROMAPITOR itr = m_macro.find(str);
	
	if (itr != m_macro.end())
		return (*itr).second.data();

	return str.data();
}

const char* tpGrammer::GetParamNotUseMacro(int index)
{
	BsAssert((int)m_params.size() > index);

	string& str = m_params[index];
	return str.data();
}

void tpGrammer::PrintGrammer()
{
/*
#ifdef _DEBUG
	// 등록된 문법을 다 찍어보자.
	tpGRAMMERMAPITOR itr = m_grammer.begin();

	while (itr != m_grammer.end())
	{
		_debug("Grammer - %d\n", (*itr).first);

		tpGRAMMERITEM* pItem = (*itr++).second;
		BsAssert(pItem);

		int index = 0;

		while (pItem->type[index] != 0)
		{
			_debug("0x%x - %s, %d\n", pItem->type[index], pItem->str[index].data(), pItem->size);
			index++;
		}

		_debug("\n");
	}
#endif
*/
}

bool tpGrammer::Add(int code, const char* pGrammer)
{
	BsAssert(code);
	BsAssert(pGrammer);

	Parser parser;
	TokenList toklist;

	parser.Create();
	parser.EnableComments( false );

	istrstream istr(pGrammer);

	bool result = parser.ProcessSource(&istr, &toklist );
	BsAssert( result && "Unable to read file" );

	Token token;
	TokenList::iterator itr=toklist.begin();

	tpGRAMMERITEM* pItem = new tpGRAMMERITEM;
	BsAssert(pItem);

	int index = 0;
	
	while(itr!=toklist.end())
	{
		token = (*itr++);

		if (token.IsOperator() && strcmp(token.GetOperator(), "%") == 0)
		{
			// 숫자 혹은 문자로 구성된다.
			token = (*itr++);

			if (token.IsVariable())
			{
				BsAssert(strlen(token.GetVariable()) == 1);

				// 타입별 정의이다.
				switch (*token.GetVariable())
				{
				case 'd':
					pItem->type[index] = Token::INTEGER;
					pItem->str[index++] = "";
					break;
//				case 'x':
//					pItem->type[index] = LX_DIGIT_HEXA;
//					pItem->str[index++] = "";
//					break;
				case 's':
					pItem->type[index] = Token::STRING;
					pItem->str[index++] = "";
					break;
				case 'c':	// 일반 c와 틀리다.
					pItem->type[index] = Token::VARIABLE;
					pItem->str[index++] = "";
					break;
				case 'f':	// 실수
					pItem->type[index] = Token::REAL;
					pItem->str[index++] = "";
					break;
				default:
					BsAssert(false && "Unknown format identifier.");
					break;
				}
			}
			else
			{
				BsAssert(false && "Variable type (format identifier) must be preceding '%'");
			}

			continue;
		}
		else
		{
			// 나머지는 그대로
			pItem->type[index] = token.GetType();
			pItem->str[index++] = token.GetText();
		}
	}

	m_grammer[code] = pItem;

	// 마지막임
	pItem->type[index] = 0;
	pItem->size = index;
	return true;
}

void tpGrammer::AddMacro(char* pMacro, char* pReal)
{
	BsAssert(pMacro && pReal);
	m_macro[pMacro] = pReal;
}

bool tpGrammer::IsSame(int t1, string& s1, int t2, string& s2)
{
	if (t1 == t2)
	{
		if (s1.empty())
			return true;
		else
		{
			if (_stricmp(s1.data(), s2.data()) == 0)
				return true;
		}
	}

	return false;
}
