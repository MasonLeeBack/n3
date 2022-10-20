#include "stdafx.h"

//=============================================================================
//
// 	Token.cpp   
//
// 	Author:	 James Boer
//
//	Copyright (C) 2001 James R. Boer  All Rights Reserved.
//	
//=============================================================================

#include "Token.h"
#include <string>
#include <iostream>
#include "BstreamExt.h"
#include "BsFileIO.h"

using namespace std;

Token::Token(const Token& tok)
{
	Clear();

#ifdef TOKEN_DEBUGGING_INFO
	m_iLineNumber = tok.m_iLineNumber;
	m_iFileIndex = tok.m_iFileIndex;
#endif // TOKEN_DEBUGGING_INFO

	m_Type = tok.m_Type;
	size_t tokLen = 0; //aleksger - safe string 
	switch(m_Type)
	{
	case STRING:
		tokLen = strlen(tok.m_pszString) + 1;//aleksger - safe string
		m_pszString = new char[tokLen];
		strcpy_s(m_pszString, tokLen, tok.m_pszString);
		break;
	case KEYWORD:
		tokLen = strlen(tok.m_pszKeyword) + 1;//aleksger - safe string
		m_pszKeyword = new char[tokLen];
		strcpy_s(m_pszKeyword, tokLen, tok.m_pszKeyword);
		break;
	case OPERATOR:
		tokLen = strlen(tok.m_pszOperator) + 1;//aleksger - safe string
		m_pszOperator = new char[tokLen];
		strcpy_s(m_pszOperator, tokLen, tok.m_pszOperator);
		break;
	case VARIABLE:
		tokLen = strlen(tok.m_pszVariable) + 1;//aleksger - safe string
		m_pszVariable = new char[tokLen];
		strcpy_s(m_pszVariable, tokLen, tok.m_pszVariable);
		break;
	case BOOLEAN:
		m_bBoolean = tok.m_bBoolean;
		break;
	case INTEGER:
		m_iInteger = tok.m_iInteger;
		break;
	case REAL:
		m_fReal = tok.m_fReal;
		break;
	case T_GUID:
		m_pguidGuid = new GUID;
		memcpy(m_pguidGuid, tok.m_pguidGuid, sizeof(GUID));
		break;
	};
		
}

void Token::operator =( const Token& tok )
{
	Destroy();

#ifdef TOKEN_DEBUGGING_INFO
	m_iLineNumber = tok.m_iLineNumber;
#endif
	m_Type = tok.m_Type;
	size_t tokLen = 0; //aleksger - safe string (and the strcpy_s below)
	switch(m_Type)
	{
	case STRING:
		tokLen = strlen(tok.m_pszString) + 1;//aleksger - safe string
		m_pszString = new char[strlen(tok.m_pszString) + 1];
		strcpy_s(m_pszString, tokLen, tok.m_pszString);
		break;
	case KEYWORD:
		tokLen = strlen(tok.m_pszKeyword) + 1;//aleksger - safe string
		m_pszKeyword = new char[strlen(tok.m_pszKeyword) + 1];
		strcpy_s(m_pszKeyword, tokLen,tok.m_pszKeyword);
		break;
	case OPERATOR:
		tokLen = strlen(tok.m_pszOperator) + 1;//aleksger - safe string
		m_pszOperator = new char[strlen(tok.m_pszOperator) + 1];
		strcpy_s(m_pszOperator, tokLen,tok.m_pszOperator);
		break;
	case VARIABLE:
		tokLen = strlen(tok.m_pszVariable) + 1;//aleksger - safe string
		m_pszVariable = new char[strlen(tok.m_pszVariable) + 1];
		strcpy_s(m_pszVariable, tokLen,tok.m_pszVariable);
		break;
	case BOOLEAN:
		m_bBoolean = tok.m_bBoolean;
		break;
	case INTEGER:
		m_iInteger = tok.m_iInteger;
		break;
	case REAL:
		m_fReal = tok.m_fReal;
		break;
	case T_GUID:
		m_pguidGuid = new GUID;
		memcpy(m_pguidGuid, tok.m_pguidGuid, sizeof(GUID));
		break;
	};
		
}


void Token::Clear()
{
	m_Type = UNKNOWN_TOKEN;
#ifdef TOKEN_DEBUGGING_INFO
	m_iLineNumber = 0;
	m_iFileIndex = 0;
#endif // TOKEN_DEBUGGING_INFO
	m_pszString = NULL;
}

bool Token::Save( BStream* pStream )
{
	int nSize;
	pStream->Write( &m_Type, sizeof(int), 4 );

	switch( m_Type )
	{
	case UNKNOWN_TOKEN:
		BsAssert(0);
		break;
	case KEYWORD:
		nSize = strlen( m_pszKeyword )+1;
		pStream->Write( &nSize, sizeof(int), 4 );
		pStream->Write( m_pszKeyword, nSize );
		break;
	case OPERATOR:
		nSize = strlen( m_pszOperator )+1;
		pStream->Write( &nSize, sizeof(int), 4 );
		pStream->Write( m_pszOperator, nSize );
		break;
	case VARIABLE:
		nSize = strlen( m_pszVariable )+1;
		pStream->Write( &nSize, sizeof(int), 4 );
		pStream->Write( m_pszVariable , nSize );
		break;
	case STRING:
		nSize = strlen( m_pszString )+1;
		pStream->Write( &nSize, sizeof(int), 4 );
		pStream->Write( m_pszString, nSize );
		break;
	case INTEGER:
		pStream->Write( &m_iInteger, sizeof( int ), 4 );
		break;
	case REAL:
		pStream->Write( &m_fReal, sizeof( float ), 4 );
		break;
	case BOOLEAN:
		pStream->Write( &m_bBoolean, sizeof( bool ) );
		break;
	case T_GUID:
		BsAssert(0);
		break;
	}
	pStream->Write( &m_iLineNumber, sizeof(int), 4 );
	pStream->Write( &m_iFileIndex, sizeof(int), 4 );

	return true;
}

bool Token::Load( BStream* pStream )
{
	int nSize;
	pStream->Read( &m_Type, sizeof(int), 4 );

	switch( m_Type )
	{
	case UNKNOWN_TOKEN:
		BsAssert(0);
		break;
	case KEYWORD:
		pStream->Read( &nSize, sizeof(int), 4 );
		m_pszKeyword = new char[nSize];
		pStream->Read( m_pszKeyword, nSize );
		break;
	case OPERATOR:
		pStream->Read( &nSize, sizeof(int), 4 );
		m_pszOperator = new char[nSize];
		pStream->Read( m_pszOperator, nSize );
		break;
	case VARIABLE:
		pStream->Read( &nSize, sizeof(int), 4 );
		m_pszVariable = new char[nSize];
		pStream->Read( m_pszVariable , nSize );
		break;
	case STRING:
		pStream->Read( &nSize, sizeof(int), 4 );
		m_pszString = new char[nSize];
		pStream->Read( m_pszString, nSize );
		break;
	case INTEGER:
		pStream->Read( &m_iInteger, sizeof( int ), 4 );
		break;
	case REAL:
		pStream->Read( &m_fReal, sizeof( float ), 4 );
		break;
	case BOOLEAN:
		pStream->Read( &m_bBoolean, sizeof( bool ) );
		break;
	case T_GUID:
		BsAssert(0);
		break;
	}
	pStream->Read( &m_iLineNumber, sizeof(int), 4 );
	pStream->Read( &m_iFileIndex, sizeof(int), 4 );

	return true;
}


void Token::Destroy()
{
	switch(m_Type)
	{
	case STRING:
		delete[] m_pszString;
		m_pszString = 0;
		break;
	case KEYWORD:
		delete[] m_pszKeyword;
		m_pszKeyword = 0;
		break;
	case OPERATOR:
		delete[] m_pszOperator;
		m_pszOperator = 0;
		break;
	case VARIABLE:
		delete[] m_pszVariable;
		m_pszVariable = 0;
		break;
	case T_GUID:
		delete[] m_pguidGuid;
		m_pguidGuid = 0;
		break;
	};
	Clear();
}


const char* Token::GetDescriptiveString() const
{
	static char pszBuffer[256];
	static string sBuffer;

	switch(GetType())
	{
	case Token::INTEGER:
		sBuffer = "Integer: ";
		_itoa(GetInteger(), pszBuffer, 10);
		sBuffer += pszBuffer;
		break;
	case Token::REAL:
		sBuffer = "Real: ";
		_gcvt(GetReal(), 10, pszBuffer);
		sBuffer += pszBuffer;
		break;
	case Token::STRING:
		sBuffer = "String: ";
		sBuffer += GetString();
		break;
	case Token::KEYWORD:
		sBuffer = "Keyword: ";
		sBuffer += GetKeyword();
		break;
	case Token::VARIABLE:
		sBuffer = "Variable: ";
		sBuffer += GetVariable();
		break;
	case Token::T_GUID:
		{
			sBuffer = "Guid: ";
			sBuffer += "{";
			sprintf(pszBuffer, "%.8x", m_pguidGuid->Data1);
			sBuffer += pszBuffer;
			sBuffer += "-";
			sprintf(pszBuffer, "%.4x", m_pguidGuid->Data2);
			sBuffer += pszBuffer;
			sBuffer += "-";
			sprintf(pszBuffer, "%.4x", m_pguidGuid->Data3);
			sBuffer += pszBuffer;
			sBuffer += "-";
			sprintf(pszBuffer, "%.2x", m_pguidGuid->Data4[0]);
			sBuffer += pszBuffer;
			sprintf(pszBuffer, "%.2x", m_pguidGuid->Data4[1]);
			sBuffer += pszBuffer;
			sBuffer += "-";
			for(int i = 2; i < 8; i++)
			{
				sprintf(pszBuffer, "%.2x", m_pguidGuid->Data4[i]);
				sBuffer += pszBuffer;
			}
			sBuffer += "}";
		}
		break;
	case Token::OPERATOR:
		sBuffer = "Operator: ";
		if(string("\n") == GetOperator())
			sBuffer += "<newline>";
		else
			sBuffer += GetOperator();
		break;
	case Token::BOOLEAN:
		if(GetBoolean())
		{  sBuffer = "Boolean: true";  }
		else
		{  sBuffer = "Boolean: false";  }
		break;
	};

	return sBuffer.c_str();
}

/*
//-----------------------------------------------------------
// Mighty stream operators
//-----------------------------------------------------------
ostream& operator<<( std::ostream& os, Token& tok )
{
	// start with line number and token type
#ifdef TOKEN_DEBUGGING_INFO
	os << tok.GetLineNumber() << ' ' 
#endif
	os << tok.GetType() << endl;
	// now, write the type
	switch( tok.GetType() )
	{
	case Token::KEYWORD:
		os << tok.GetKeyword() << endl;
		break;
	case Token::OPERATOR:
		os << tok.GetOperator() << endl;
		break;
	case Token::VARIABLE:
		os << tok.GetVariable() << endl;
		break;
	case Token::STRING:
		os << tok.GetString() << endl;
		break;
	case Token::INTEGER:
		os << tok.GetInteger() << endl;
		break;
	case Token::REAL:
		os << tok.GetReal() << endl;
		break;
	case Token::BOOLEAN:
		os << (int)(tok.GetBoolean()) << endl;
		break;
	case Token::T_GUID:
		os << tok.GetGuid().Data1 << ' ' << tok.GetGuid().Data2 << ' ' 
		   << tok.GetGuid().Data3 << ' ' << tok.GetGuid().Data4 << endl;
		break;
	default:
		BsAssert(0);	// weird
		os << endl;	// skip a line anyway
	}

	return os;
}

istream& operator>>( std::istream& is, Token& tok )
{
	// this is a bit of a pain because I can't figure out how to get the friend declaration to work
	// for stream operators :-(  so we must use accessors.  Would be better to have direct access
	// as lots of string copying would go away!

	// clear anything out of existing token
	tok.Destroy();
	tok.Clear();

	// start with line number and token type
	int iTemp;
	char c;
//	Token::TOKEN_TYPE	eType;

	is >> iTemp;
	tok.SetLineNumber(iTemp);
	is >> iTemp;
	is.get(c);	// get rid of newline character
	BsAssert(c == '\n');	// um, it was a newline character, right?
//	eType = ;

	static char buf[256];		// slightly bad here that we have a size limit, but hard to do otherwize with stream operations

	switch( (Token::TOKEN_TYPE)iTemp )
	{
	case Token::KEYWORD:
		is.getline(buf, 256);
		tok.CreateKeyword(buf);
		break;
	case Token::OPERATOR:
		is.getline(buf, 256);
		tok.CreateOperator(buf);
		break;
	case Token::VARIABLE:
		is.getline(buf, 256);
		tok.CreateVariable(buf);
		break;
	case Token::STRING:
		is.getline(buf, 256);
		tok.CreateString(buf);
		break;
	case Token::INTEGER:
		is >> iTemp;
		tok.CreateInteger(iTemp);
		break;
	case Token::REAL:
		{
			double fTemp;
			is >> fTemp;
			tok.CreateReal(fTemp);
		}
		break;
	case Token::BOOLEAN:
		is >> iTemp;
		tok.CreateBoolean((bool)iTemp);
		break;
	case Token::T_GUID:
		{
			GUID guid;
			is >> guid.Data1 >> guid.Data2 >> guid.Data3 >> guid.Data4;
			tok.CreateGuid(guid);
			// get to end of line (for some reason is is left pointing at the start of Data4 - ??)
			is.getline(buf, 256);
		}
		break;
	default:
		BsAssert(0);	// weird
	}

	return is;
}
*/
string TokenList::GetDescriptiveString(TokenListItor itr) const
{
	string sBuffer(itr->GetDescriptiveString());

#ifdef TOKEN_DEBUGGING_INFO
	if(itr->GetLineNumber() != 0)
	{
		char cBuffer[64];
		sBuffer += " at line ";
		_itoa(itr->GetLineNumber(), cBuffer, 10);
		sBuffer += cBuffer;
		sBuffer += " in file ";
		sBuffer += m_aFileRef[itr->GetFileIndex()];
	}
#endif

	return sBuffer;
}

/*
string TokenList::GetFileName(TokenListItor itr) const
{
	string sBuffer;

	return sBuffer;
}
*/

string TokenList::GetFileName(int index) const
{
	return m_aFileRef[index];
}

#ifdef TOKEN_DEBUGGING_INFO
int TokenList::GetLineNumber(TokenListItor itr) const
{
	return itr->GetLineNumber();
}
#else
int TokenList::GetLineNumber(TokenListItor itr) const
{
	return 0;
}

#endif

int TokenList::GetFileIndex(const std::string &sFileName) //aleksger: prefix bug 873: Pass strings by reference
{
	int fileRefSize = m_aFileRef.size();
	for(int i = 0; i < fileRefSize; i++)
	{
		if(m_aFileRef[i] == sFileName)
			return i;
	}
	m_aFileRef.push_back(sFileName);
	return m_aFileRef.size() - 1;
}

int TokenList::GetNumFileRefs() const
{
	return m_aFileRef.size();
}

void TokenList::clear()
{
	m_aFileRef.clear();
	list<Token>::clear();
}



// TokenBuffer class
// 소스가 txt, bin 둘다 호환되게 하기 위해, 바이너리 저장을 쉽게 하기 위해 만듬
TokenBuffer::TokenBuffer()
{
	m_nNumToken = 0;
	m_nCurIndex = -1;
	m_pToken = NULL;
	m_nNumFileRef = 0;
	m_pFileRef = NULL;
}

TokenBuffer::~TokenBuffer()
{
	Clear();
}

void TokenBuffer::Clear()
{
	delete[] m_pToken;
	m_pToken = NULL;
	m_nNumToken = 0;
	m_nCurIndex = -1;

	delete[] m_pFileRef;
	m_pFileRef = NULL;
	m_nNumFileRef = 0;
}


Token& TokenBuffer::operator ++()
{
	if( m_nCurIndex < m_nNumToken )		// 끝에 하나 넘는다 end체크 위해
		++m_nCurIndex;

	if( m_nCurIndex == m_nNumToken )
		return m_pToken[0];

	return m_pToken[m_nCurIndex];
}

Token TokenBuffer::operator ++(int)
{
	if( m_nCurIndex < m_nNumToken )		// 끝에 하나 넘는다 end체크 위해
		++m_nCurIndex;

	if( m_nCurIndex == m_nNumToken )
		return m_pToken[0];

	return m_pToken[m_nCurIndex];
}

Token& TokenBuffer::operator --()
{
	if( m_nCurIndex > 0 )
		--m_nCurIndex;

	return m_pToken[m_nCurIndex];
}

Token TokenBuffer::operator --(int)
{
	if( m_nCurIndex > 0 )
		--m_nCurIndex;

	return m_pToken[m_nCurIndex];
}



void TokenBuffer::Initialize( TokenList* pList )
{
	m_nNumToken = pList->size();
	m_nCurIndex = 0;
	m_pToken = new Token[m_nNumToken];

	TokenList::iterator itr = pList->begin();

	for( int i=0; i<m_nNumToken; i++ )
	{
		m_pToken[i] = *itr;
		itr++;
	}

	m_nNumFileRef = pList->GetNumFileRefs();
	m_pFileRef = new std::string[m_nNumFileRef];
	for( int i=0; i<m_nNumFileRef; i++ )
	{
		m_pFileRef[i] = pList->GetFileName( i );
	}
}



bool TokenBuffer::Save( char* pFileName, int nTime )
{
	// BFileStream::create해도 전에 파일 사이즈가 크면 그 사이즈 크기만큼 파일이 남는다.	
	DeleteFile( pFileName );

	BFileStream Stream( pFileName, BFileStream::create );
	if( !Stream.Valid() )
		return false;

	Stream.Write( &m_nNumToken, sizeof(int), 4 );

	for( int i=0; i<m_nNumToken; i++ )
		m_pToken[i].Save( &Stream );

	Stream.Write( &m_nNumFileRef, sizeof(int), 4 );
	for( int i=0; i<m_nNumFileRef; i++ ) {
		int nLength = m_pFileRef[i].length() + 1;
		Stream.Write( &nLength, sizeof(int) ,4 );
		Stream.Write( m_pFileRef[i].c_str(), nLength );
	}

	Stream.Write( &nTime, sizeof(int), 4 );
	DebugString("%s Binary Parser File Save...\n", pFileName);
	return true;
}

bool TokenBuffer::Load( char* pFileName, int nTime )
{
	DWORD dwFileSize;
	VOID *pData;

	if( FAILED(CBsFileIO::LoadFile( pFileName, &pData, &dwFileSize ) ) )
		return false;

	BMemoryStream Stream(pData, dwFileSize);
	if( nTime != -1 ) {
		int nWriteTime;
		Stream.Seek( -4, BStream::fromEnd );

		Stream.Read( &nWriteTime, sizeof(int), 4 );
		
		if( abs(nWriteTime-nTime) > 5 )
		{	
			CBsFileIO::FreeBuffer(pData);
			return false;
		}

		Stream.Seek( 0, SEEK_SET );
	}
	Stream.Read( &m_nNumToken, sizeof(int), 4 );
	BsAssert( m_pToken == NULL );
	m_pToken = new Token[ m_nNumToken ];

	for( int i=0; i<m_nNumToken; i++ )
		m_pToken[i].Load( &Stream );

	m_nCurIndex = 0;

	Stream.Read( &m_nNumFileRef, sizeof(int), 4 );
	m_pFileRef = new std::string[m_nNumFileRef];
	for( int i=0; i<m_nNumFileRef; i++ )
	{
		char cStr[256];
		int nLength;
		Stream.Read( &nLength, sizeof(int) ,4 );
		Stream.Read( cStr, nLength );
		m_pFileRef[i] = cStr;
	}
	CBsFileIO::FreeBuffer(pData);
	return true;
}

std::string TokenBuffer::GetFileName(int index) const
{
	return m_pFileRef[index];
}

int TokenBuffer::GetFileIndex(const std::string & sFileName) const //aleksger: prefix bug 872: Pass strings by reference
{
	for(int i = 0; i < m_nNumFileRef; i++)
	{
		if(m_pFileRef[i] == sFileName)
			return i;
	}
	BsAssert(0);
	return -1;
}

int TokenBuffer::GetNumFileRefs() const
{
	return m_nNumFileRef;
}


string TokenBuffer::GetDescriptiveString()
{
	BsAssert( m_nCurIndex >= 0 );
	Token* pData = &(m_pToken[m_nCurIndex]);

	string sBuffer(pData->GetDescriptiveString());

#ifdef TOKEN_DEBUGGING_INFO
	if(pData->GetLineNumber() != 0)
	{
		char cBuffer[64];
		sBuffer += " at line ";
		_itoa(pData->GetLineNumber(), cBuffer, 10);
		sBuffer += cBuffer;
		sBuffer += " in file ";
		sBuffer += m_pFileRef[pData->GetFileIndex()];
	}
#endif

	return sBuffer;
}
