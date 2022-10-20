#include "stdafx.h"

//=============================================================================
//
// 	TokenFile.cpp  
//
// 	Author:	 James Boer
//
//	Copyright (C) 2001 James R. Boer  All Rights Reserved.
//	
//=============================================================================

#include "TokenFile.h"
#include <iostream>
#include <fstream>
#include <tchar.h>
#include <stdio.h>
#include "BsUtil.h"

const char* PCS_HEADER = "PCSFILE";
const unsigned short PCS_MAJOR_VERSION = 1;
const unsigned short PCS_MINOR_VERSION = 0;

using namespace std;


void TokenFile::Clear()
{
	m_pTokenList = NULL;
	m_bCaseSensitive = false;
	m_bWordKeywordIndex = false;
	m_bWordOperatorIndex = false;
	m_bWordVariableIndex = false;
	m_vecKeyword.clear();
	m_vecOperator.clear();
	m_vecVariable.clear();
}

void TokenFile::Destroy()
{
}



bool TokenFile::Write(ostream &Output, TokenList* pTokenList, DWORD dwFlags)
{
	Clear();
	
	if(dwFlags & TF_CASE_SENSITIVE)
		m_bCaseSensitive = true;
	else
		m_bCaseSensitive = false;

	// clear the queue
	while(!m_queBuffer.empty())
		m_queBuffer.pop();

	m_pTokenList = pTokenList;
	WriteHeader();
	GenerateTables();
	WriteTables();
	WriteTokens();

	while(!m_queBuffer.empty())
	{
		Output.put(m_queBuffer.front());
		m_queBuffer.pop();
	}

	Output.flush();
	return true;
}

bool TokenFile::Write(const string &sFilename, TokenList* pTokenList, DWORD dwFlags) //aleksger: prefix bug 875: Pass strings by reference
{
	ofstream File;

	if(pTokenList == NULL)
		return HandleError("Function requires a valid token list");

	File.open(sFilename.c_str(), ios::out | ios::binary | ios::trunc);

	if(!File.good())
		return HandleError("ERROR: file %s can't be opened for writing.\n",(const char*)sFilename.c_str());

	return Write(File, pTokenList);
}



void TokenFile::WriteHeader()
{
	for(int i = 0; i < 7; i++)
		m_queBuffer.push(PCS_HEADER[i]);

	WriteWord(PCS_MAJOR_VERSION);
	WriteWord(PCS_MINOR_VERSION);
}


// Iterate through the list and pre-generate string-based tokens
void TokenFile::GenerateTables()
{
	for(TokenListItor itor = m_pTokenList->begin(); itor != m_pTokenList->end(); ++itor)
	{
		if(itor->IsKeyword())
		{
			bool bFound = false;
			int keywordListSize = m_vecKeyword.size();
			for(int i = 0; i < keywordListSize; i++)
			{
				if(m_bCaseSensitive)
				{
					if(strcmp(m_vecKeyword[i].c_str(), itor->GetKeyword()) == 0)
					{
						bFound = true;
						break;
					}
				}
				else
				{
					if(_stricmp(m_vecKeyword[i].c_str(), itor->GetKeyword()) == 0)
					{
						bFound = true;
						break;
					}
				}
			}
			if(!bFound)
				m_vecKeyword.push_back(itor->GetKeyword());
		}
		else if(itor->IsOperator())
		{
			bool bFound = false;
			int operatorSize = m_vecOperator.size();
			for(int i = 0; i < operatorSize; i++)
			{
				if(m_bCaseSensitive)
				{
					if(strcmp(m_vecOperator[i].c_str(), itor->GetOperator()) == 0)
					{
						bFound = true;
						break;
					}
				}
				else
				{
					if(_stricmp(m_vecOperator[i].c_str(), itor->GetOperator()) == 0)
					{
						bFound = true;
						break;
					}
				}
			}
			if(!bFound)
				m_vecOperator.push_back(itor->GetOperator());
		}
		else if(itor->IsVariable())
		{
			bool bFound = false;
			int variableListSize = m_vecVariable.size();
			for(int i = 0; i < variableListSize; i++)
			{
				if(m_bCaseSensitive)
				{
					if(strcmp(m_vecVariable[i].c_str(), itor->GetVariable()) == 0)
					{
						bFound = true;
						break;
					}
				}
				else
				{
					if(_stricmp(m_vecVariable[i].c_str(), itor->GetVariable()) == 0)
					{
						bFound = true;
						break;
					}
				}
			}
			if(!bFound)
				m_vecVariable.push_back(itor->GetVariable());
		}
	}

	// If there are fewer than 255 unique tokens of a given type, then we can 
	// index that type with a byte instead of a word.  Might as well save space 
	// where we can, eh?
	if(m_vecKeyword.size() > 255)
		m_bWordKeywordIndex = true;
	if(m_vecOperator.size() > 255)
		m_bWordOperatorIndex = true;
	if(m_vecVariable.size() > 255)
		m_bWordVariableIndex = true;
}

void TokenFile::WriteTables()
{
	// Write the number of entries in each table, 
	// and then write the strings sequentially.  
	unsigned int index;
	WriteWord((WORD)m_vecKeyword.size());
	for(index = 0; index < m_vecKeyword.size(); index++)
		WriteString(m_vecKeyword[index]);
	WriteWord((WORD)m_vecOperator.size());
	for(index = 0; index < m_vecOperator.size(); index++)
		WriteString(m_vecOperator[index]);
	WriteWord((WORD)m_vecVariable.size());
	for(index = 0; index < m_vecVariable.size(); index++)
		WriteString(m_vecVariable[index]);
}

void TokenFile::WriteTokens()
{
	
	unsigned int index;
	for(TokenListItor itor = m_pTokenList->begin(); itor != m_pTokenList->end(); ++itor)
	{
		// Write the token type
		WriteByte(itor->GetType());
		switch(itor->GetType())
		{
		case Token::KEYWORD:
			for(index = 0; index < m_vecKeyword.size(); index++)
			{
				if(m_bCaseSensitive)
				{
					if(strcmp(itor->GetKeyword(), m_vecKeyword[index].c_str()) == 0)
					{
						if(m_bWordKeywordIndex)
							WriteWord(index);
						else
							WriteByte(index);
						break;
					}
				}
				else
				{
					if(_stricmp(itor->GetKeyword(), m_vecKeyword[index].c_str()) == 0)
					{
						if(m_bWordKeywordIndex)
							WriteWord(index);
						else
							WriteByte(index);
						break;
					}
				}
			}
			break;

		case Token::OPERATOR:
			for(index = 0; index < m_vecOperator.size(); index++)
			{
				if(m_bCaseSensitive)
				{
					if(strcmp(itor->GetOperator(), m_vecOperator[index].c_str()) == 0)
					{
						if(m_bWordOperatorIndex)
							WriteWord(index);
						else
							WriteByte(index);
						break;
					}
				}
				else
				{
					if(_stricmp(itor->GetOperator(), m_vecOperator[index].c_str()) == 0)
					{
						if(m_bWordOperatorIndex)
							WriteWord(index);
						else
							WriteByte(index);
						break;
					}
				}
			}
			break;

		case Token::VARIABLE:
			for(index = 0; index < m_vecVariable.size(); index++)
			{
				if(m_bCaseSensitive)
				{
					if(strcmp(itor->GetVariable(), m_vecVariable[index].c_str()) == 0)
					{
						if(m_bWordVariableIndex)
							WriteWord(index);
						else
							WriteByte(index);
						break;
					}
				}
				else
				{
					if(_stricmp(itor->GetVariable(), m_vecVariable[index].c_str()) == 0)
					{
						if(m_bWordVariableIndex)
							WriteWord(index);
						else
							WriteByte(index);
						break;
					}
				}
			}
			break;

		case Token::STRING:
			WriteString(itor->GetString());
			break;

		case Token::INTEGER:
			WriteDword(itor->GetInteger());
			break;

		case Token::REAL:
			WriteFloat(itor->GetReal());
			break;

		case Token::BOOLEAN:
			WriteByte(itor->GetBoolean());
			break;

		case Token::T_GUID:
			WriteGuid(itor->GetGuid());
			break;

		};
	}
}


void TokenFile::WriteString(const std::string & sData) //aleksger: prefix bug 876: Pass strings by reference
{
	// Since a word is used as the data length, this implies a maximum
	// string length of 65,535 characters, which shouldn't be a problem
	// for most normal use.
	size_t size = sData.length();
	WriteWord((WORD)sData.length());
	int length = size;
	for(int i = 0; i < length; i++)
		m_queBuffer.push(sData[i]);
}

void TokenFile::WriteDword(DWORD dwData)
{
	// Note the dependence upon byte ordering - important if porting code
	BYTE* pByte = (BYTE*)&dwData;
	m_queBuffer.push(*pByte++);
	m_queBuffer.push(*pByte++);
	m_queBuffer.push(*pByte++);
	m_queBuffer.push(*pByte++);
}

void TokenFile::WriteWord(WORD wData)
{
	// Note the dependence upon byte ordering - important if porting code
	BYTE* pByte = (BYTE*)&wData;
	m_queBuffer.push(*pByte++);
	m_queBuffer.push(*pByte++);
}

void TokenFile::WriteByte(BYTE byteData)
{
	m_queBuffer.push(byteData);
}

void TokenFile::WriteFloat(float dData)
{
	// Note the dependence upon byte ordering - important if porting code
	BYTE* pByte = (BYTE*)&dData;
	m_queBuffer.push(*pByte++);
	m_queBuffer.push(*pByte++);
	m_queBuffer.push(*pByte++);
	m_queBuffer.push(*pByte++);
}

void TokenFile::WriteGuid(GUID &guidData)
{
	// Note the dependence upon byte ordering - important if porting code
	BYTE* pByte = (BYTE*)&guidData;
	m_queBuffer.push(*pByte++);
	m_queBuffer.push(*pByte++);
	m_queBuffer.push(*pByte++);
	m_queBuffer.push(*pByte++);
	m_queBuffer.push(*pByte++);
	m_queBuffer.push(*pByte++);
	m_queBuffer.push(*pByte++);
	m_queBuffer.push(*pByte++);
	m_queBuffer.push(*pByte++);
	m_queBuffer.push(*pByte++);
	m_queBuffer.push(*pByte++);
	m_queBuffer.push(*pByte++);
	m_queBuffer.push(*pByte++);
	m_queBuffer.push(*pByte++);
	m_queBuffer.push(*pByte++);
	m_queBuffer.push(*pByte++);
}




bool TokenFile::Read(istream& Input, TokenList* pTokenList)
{
	Clear();
	if(pTokenList == NULL)
	{
		return HandleError("Must have a valid token list to operate on.\n");
	}
	
	m_pTokenList = pTokenList;

	if(!Input.good())
		return HandleError("ERROR: file can't be opened for parsing.\n");

	// clear the queue
	while(!m_queBuffer.empty())
		m_queBuffer.pop();

	// read in the entire file and store in in the queue buffer
	int iData = Input.get();
	while(Input.good())
	{
		m_queBuffer.push(BYTE(iData));
		iData = Input.get();
	}

	if(!ReadHeader())
		return HandleError("Not a valid pre-compiled script file.");

	if(!ReadTables())
		return HandleError("Could not read script token table");

	if(!ReadTokens())
		return HandleError("Could not read script token table");

	return true;
}

bool TokenFile::Read(const string & sFilename, TokenList* pTokenList) //aleksger: prefix bug 877: Pass strings by reference
{
	ifstream File;

	if(pTokenList == NULL)
		return HandleError("Function requirest a valid token list");

	File.open(sFilename.c_str(), ios::binary);

	if( !File.good() ) {
		HandleError("%s   D1\n", sFilename);
		return HandleError("ERROR: file %s can't be opened for parsing.\n",(const char*)sFilename.c_str());
	}

	return Read(File, pTokenList);
}


bool TokenFile::ReadHeader()
{
	char szHeader[8];
	for(int i = 0; i < 7; i++)
	{
		szHeader[i] = m_queBuffer.front();
		m_queBuffer.pop();
	}
	szHeader[7] = NULL;
	if(strcmp(szHeader, PCS_HEADER) != 0)
		return HandleError("Header failed check");

	DWORD dwMajor = ReadWord();
	/*DWORD dwMinor =*/ ReadWord();
	if(dwMajor > PCS_MAJOR_VERSION)
		return HandleError("Incompatible file version");

	return true;
}

bool TokenFile::ReadTables()
{
	int index;

	// Read in the keyword table
	WORD wKeywordSize = ReadWord();
	m_vecKeyword.reserve((int)wKeywordSize);
	if(m_vecKeyword.size() > 255)
		m_bWordKeywordIndex = true;
	for(index = 0; index < wKeywordSize; index++)
		m_vecKeyword.push_back(ReadString());

	// Read in the operator table
	WORD wOperatorSize = ReadWord();
	m_vecOperator.reserve((int)wOperatorSize);
	if(m_vecOperator.size() > 255)
		m_bWordOperatorIndex = true;
	for(index = 0; index < wOperatorSize; index++)
		m_vecOperator.push_back(ReadString());

	// Read in the variable table
	WORD wVariableSize = ReadWord();
	m_vecVariable.reserve((int)wVariableSize);
	if(m_vecVariable.size() > 255)
		m_bWordVariableIndex = true;
	for(index = 0; index < wVariableSize; index++)
		m_vecVariable.push_back(ReadString());

	return true;
}

bool TokenFile::ReadTokens()
{
	// first read in the token type
	Token::TOKEN_TYPE type;
	Token tok;

	while(!m_queBuffer.empty())
	{
		// first read in the token type
		type = (Token::TOKEN_TYPE)ReadByte();

		switch(type)
		{
		case Token::KEYWORD:
			if(m_bWordKeywordIndex)
			{
				WORD index = ReadWord();
				BsAssert(index < m_vecKeyword.size());
				tok.CreateKeyword(m_vecKeyword[index].c_str());
			}
			else
			{
				BYTE index = ReadByte();
				BsAssert(index < m_vecKeyword.size());
				tok.CreateKeyword(m_vecKeyword[index].c_str());
			}
			break;

		case Token::OPERATOR:
			if(m_bWordOperatorIndex)
			{
				WORD index = ReadWord();
				BsAssert(index < m_vecOperator.size());
				tok.CreateOperator(m_vecOperator[index].c_str());
			}
			else
			{
				BYTE index = ReadByte();
				BsAssert(index < m_vecOperator.size());
				tok.CreateOperator(m_vecOperator[index].c_str());
			}
			break;

		case Token::VARIABLE:
			if(m_bWordVariableIndex)
			{
				WORD index = ReadWord();
				BsAssert(index < m_vecVariable.size());
				tok.CreateVariable(m_vecVariable[index].c_str());
			}
			else
			{
				BYTE index = ReadByte();
				BsAssert(index < m_vecVariable.size());
				tok.CreateVariable(m_vecVariable[index].c_str());
			}
			break;

		case Token::STRING:
			tok.CreateString(ReadString().c_str());
			break;

		case Token::INTEGER:
			tok.CreateInteger((int)ReadDword());
			break;

		case Token::REAL:
			tok.CreateReal(ReadFloat());
			break;

		case Token::BOOLEAN:
			tok.CreateBoolean((ReadByte() != 0) ? true : false);
			break;

		case Token::T_GUID:
			tok.CreateGuid(ReadGuid());
			break;

		default:
			return HandleError("Format error in pre-compiled file.");
		};

		m_pTokenList->push_back(tok);
		tok.Destroy();
	}
	return true;
}

// Basic types
string TokenFile::ReadString()
{
	char* pszData;
	WORD wSize = ReadWord();
	pszData = new char[(int)wSize + 1];
	for(int i = 0; i < (int)wSize; i++)
	{
		//Input.read(pszData, (int)wSize);
		pszData[i] = m_queBuffer.front();
		m_queBuffer.pop();
	}
	pszData[(int)wSize] = NULL;
	string sData(pszData);
	delete[] pszData;
	return sData;
}

DWORD TokenFile::ReadDword()
{
	DWORD dwData;
	PBYTE p = (PBYTE)&dwData;
	*p++ = m_queBuffer.front();
	m_queBuffer.pop();
	*p++ = m_queBuffer.front();
	m_queBuffer.pop();
	*p++ = m_queBuffer.front();
	m_queBuffer.pop();
	*p++ = m_queBuffer.front();
	m_queBuffer.pop();
	return dwData;
}

WORD TokenFile::ReadWord()
{
	WORD wData;
	PBYTE p = (PBYTE)&wData;
	*p++ = m_queBuffer.front();
	m_queBuffer.pop();
	*p++ = m_queBuffer.front();
	m_queBuffer.pop();
	return wData;
}

BYTE TokenFile::ReadByte()
{
	BYTE byteData = m_queBuffer.front();
	m_queBuffer.pop();
	return byteData;
}

float TokenFile::ReadFloat()
{
	float dData;
	PBYTE p = (PBYTE)&dData;
	*p++ = m_queBuffer.front();
	m_queBuffer.pop();
	*p++ = m_queBuffer.front();
	m_queBuffer.pop();
	*p++ = m_queBuffer.front();
	m_queBuffer.pop();
	*p++ = m_queBuffer.front();
	m_queBuffer.pop();
	return dData;
}

GUID TokenFile::ReadGuid()
{
	GUID guidData;
	PBYTE p = (PBYTE)&guidData;
	*p++ = m_queBuffer.front();
	m_queBuffer.pop();
	*p++ = m_queBuffer.front();
	m_queBuffer.pop();
	*p++ = m_queBuffer.front();
	m_queBuffer.pop();
	*p++ = m_queBuffer.front();
	m_queBuffer.pop();
	*p++ = m_queBuffer.front();
	m_queBuffer.pop();
	*p++ = m_queBuffer.front();
	m_queBuffer.pop();
	*p++ = m_queBuffer.front();
	m_queBuffer.pop();
	*p++ = m_queBuffer.front();
	m_queBuffer.pop();
	*p++ = m_queBuffer.front();
	m_queBuffer.pop();
	*p++ = m_queBuffer.front();
	m_queBuffer.pop();
	*p++ = m_queBuffer.front();
	m_queBuffer.pop();
	*p++ = m_queBuffer.front();
	m_queBuffer.pop();
	*p++ = m_queBuffer.front();
	m_queBuffer.pop();
	*p++ = m_queBuffer.front();
	m_queBuffer.pop();
	*p++ = m_queBuffer.front();
	m_queBuffer.pop();
	*p++ = m_queBuffer.front();
	m_queBuffer.pop();
	return guidData;
}


//---------------------------------
// Error handling functions
bool TokenFile::HandleError(const char* lpszFormat, ...)
{
	va_list args;
	va_start(args, lpszFormat);
	int nBuf;
	char szBuffer[512];
	if( lpszFormat )
	{
		nBuf = _vsnprintf(szBuffer, 512, lpszFormat, args); //aleksger: prefix bug 878: Possible buffer overflow
		// was there an error? was the expanded string too long?
		BsAssert(nBuf >= 0);
	}
	else
		szBuffer[0] = NULL;	// no string passed in.
	DebugString(szBuffer);
	m_sErrorMessage = szBuffer;
	// store the error in the general error log
	va_end(args);
	return false;
}


