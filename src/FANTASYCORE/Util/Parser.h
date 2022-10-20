//=============================================================================
//
// 	Parser.h   
//
// 	Author:	 James Boer
//
//	Copyright (C) 2001 James R. Boer  All Rights Reserved.
//	
//=============================================================================

#pragma once


#include <string>
#include <set>
#include <stack>
#include "Token.h"
#include "TokenFile.h"

typedef std::stack<std::string> StringStack;


class Macro
{
public:
	Macro() {  Clear();  }
	virtual ~Macro() { }

	void Clear();

	std::string m_sName;
	StringVector m_ArgumentVector;
	TokenVector m_SubstitutionVector;
	TokenList m_MacroList;
	int m_iTotalArguments;
	int m_iCurrentArgument;
	
	bool operator < (const Macro& m) const
	{  return m_sName < m.m_sName;  }
	bool operator < (const char* psz) const
	{  return m_sName < psz;  }

	bool empty()
	{  return m_sName.length() ? false : true;  }
};

typedef std::set<Macro> MacroSet;
typedef MacroSet::iterator MacroSetItor;




class Parser
{
public:
	Parser()
	{  Clear();  }
	virtual ~Parser()
	{  Destroy();  }

	void Clear();
	bool Create(bool bReserveCommonOperators = true);
	void Destroy();

	// Reserve an operator or a keyword.
	void ReserveOperator(const std::string &sOperator);//aleksger: prefix bug 851: Pass strings by reference
	void ReserveKeyword(const std::string &sKeyword);//aleksger: prefix bug 852: Pass strings by reference
	void ReserveCommonOperators();

	// Parse a file or stream
	bool ProcessSource(std::istream*, TokenList* pTokenList);
	bool ProcessSource(const char* const szStream, TokenList* pTokenList, int cCount);
	bool ProcessSource(const std::string &sDirname, const std::string &sFilename, TokenList* pTokenList);  //aleksger: prefix bug 853: Pass strings by reference

	// Set current directory (no releative paths)
	void SetDirectory(const std::string & sDirname)			{ m_sDirname = sDirname; } //aleksger: prefix bug 857: Pass strings by reference

	// Insert any headers included in the token list.  Note that end of line
	// markers will automatically be turned on due to the fact that
	// macro definitions are delimited by the end of the line.
	bool ProcessHeaders(TokenList* pTokenList);

	// Substitute all macros for their actual values
	bool ProcessMacros(TokenList* pTokenList);

	// Clears all processed macros from the internal list
	void ClearMacros();

	void MarkEndOfLine(bool bEOL)					{  m_bMarkEndOfLine = bEOL;  }
	void EnableComments(bool bComments)				{  m_bEnableComments = bComments;  }
	int GetCurrentLineNumber() const				{  return m_iCurrentLineNumber;  }
	int GetTotalLinesParsed() const					{  return m_iTotalLinesParsed;  }

protected:
	void MakeReadyForFastSearchString();
	bool ParseLine(const std::string& sLine);
	bool MakeToken(const std::string& sToken, Token::TOKEN_TYPE Type = Token::UNKNOWN_TOKEN);
	bool ExpandMacro(TokenList* pTokenList, TokenListItor& itor, MacroSetItor itr, bool& bCheckFirst);
	bool ParseGUID(TokenList* pTokenList, TokenListItor& itor);

	inline bool IsDigit(char c)	const				{  return ((c >= '0') && (c <= '9')) ? true : false;  }
	inline bool IsHexDigit(char c) const			{  return (IsDigit(c) || ((c >= 'a') && (c <= 'f')) || ((c >= 'A') && (c <= 'F'))) ? true : false;  }
	inline bool IsWhitespace(char c) const			{  return ((c == char(32)) || (c == char(9))) ? true : false;  }

	bool IsReal(const std::string& sToken) const;
	inline float GetReal(const std::string& sToken){  return (float)atof(sToken.c_str());  }
	bool IsInteger(const std::string& sToken) const;
	inline int GetInteger(const std::string& sToken){  return atoi(sToken.c_str());  }
	bool IsHex(const std::string& sToken) const;
	DWORD GetHex(const std::string& sToken);
	bool IsBoolean(const std::string& sToken) const;
	bool GetBoolean(const std::string& sToken);
	bool IsKeyword(const std::string& sToken) const;
	bool IsOperator(const std::string& sToken) const;

	bool HandleError(const char* lpszFormat, ...);
	std::string GetLastErrorMessage()				{  return m_sErrorMessage;  }
	
protected:

	// indicates whether the end of line character should be 
	// included as a token
	bool m_bMarkEndOfLine;
	// Are C and C++ style comments supported (supported means they will be
	// excluded from the token list)?
	bool m_bEnableComments;
	// Is a C style comment in effect?
	bool m_bCStyleCommentActive;
	// The current file being parsed
	StringStack m_CurrentFileStack;
	// The current line number of the file being parsed
	int m_iCurrentLineNumber;
	// Total number of lines parsed by the parser
	int m_iTotalLinesParsed;
	// contains a vector of operators defined as strings
	StringVector m_OperatorVector;
	// contains a vector of keywords defined as strings
	StringVector m_KeywordVector;
	// pointer to list of tokens as defined by the current list of keywords and operators
	TokenList* m_pTokenList;
	// We store macros here sorted by their string definition.  
	MacroSet m_MacroSet;
	// last stored error message
	std::string m_sErrorMessage;
	// file directory
	std::string m_sDirname;

	// fast Search Operator String
	std::vector<const char*>		m_FastSearchString[256];
};
