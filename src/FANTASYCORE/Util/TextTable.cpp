#include "stdafx.h"

#include "TextTable.h"
#include "Parser.h"
#include "Token.h"
#include <vector>
#include "LocalLanguage.h"
#include "BsKernel.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG


FCTextTable* g_TextTable=NULL;

struct TFn_CompareID
{
	bool operator() ( TextElement& A, TextElement& B)
	{
		return A.nID <= B.nID;
	}
};

TextTable::TextTable()
{
	m_pListIndex = NULL;
}

TextTable::~TextTable()
{
	Destroy();
}

void TextTable::Destroy()
{
	if( m_pListIndex )
		delete []m_pListIndex;

	for( DWORD i=0; i<m_VecElemList.size(); i++ )
	{
		if( m_VecElemList[i].szStr )
		{
			delete []m_VecElemList[i].szStr;
		}

		if( m_VecElemList[i].szSoundStr )
		{
			delete []m_VecElemList[i].szSoundStr;
		}
	}
	m_VecElemList.clear();
}

int debugValue = 0;

bool TextTable::Initialize( char *szFileName )
{
	////DebugString("%s\n",szFileName);
	m_VecElemList.clear();

	Parser parser;
	TokenList toklist;

	parser.Create();
	parser.EnableComments( true );

	// Parsing happens in three optional stages.  ProcessSource() extracts the
	// text from the file and organizes all elements into token categories.

	char dir[MAX_PATH];
	char fn[MAX_PATH];
	SplitDirAndFileName( szFileName, dir, _countof(dir), fn, _countof(fn)); //aleksger - safe string

	bool result = parser.ProcessSource( dir, fn, &toklist );
	if(result==false) {
		BsAssert( result && "Unable to read script" );
		return false;
	}

	result = parser.ProcessHeaders(&toklist);
	if(result==false) {
		BsAssert( result && "Error processing script header" );
		return false;
	}

	parser.ProcessMacros(&toklist);

	m_nMinIndex = m_nMaxIndex = 0;

	bool bFlag = false;
	for(TokenList::iterator itr = toklist.begin(); itr != toklist.end(); ++itr)	{
		TextElement Struct;
		memset( &Struct, 0, sizeof( TextElement ) );

		if( !(*itr).IsInteger() )
		{
			DebugString( "Error TextFile %s : Line : %d\n", fn, (*itr).GetLineNumber() );
		}

		Struct.nID = (*itr).GetInteger();
		Struct.bEnable = true;

		itr++;
		if( bFlag == false ) {
			m_nMinIndex = m_nMaxIndex = Struct.nID;
			bFlag = true;
		}
		else {
			if( m_nMinIndex > Struct.nID ) m_nMinIndex = Struct.nID;
			if( m_nMaxIndex < Struct.nID ) m_nMaxIndex = Struct.nID;
		}

		if( itr == toklist.end() ) break;

		if( !(*itr).IsString() )
		{
			DebugString( "Error TextFile %s : Line : %d\n", fn, (*itr).GetLineNumber() );
		}

		if (strlen((*itr).GetString()) > 0)
		{
			const size_t itr_len = strlen( (*itr).GetString() ) + 1;
			Struct.szStr = new char[ itr_len ];
			strcpy_s( Struct.szStr, itr_len, (*itr).GetString() );//aleksger - safe string
		}
		else
		{
			Struct.szStr = new char[1];
			Struct.szStr[0] = '\x0';
		}

		debugValue+=(int)strlen( (*itr).GetString() );

		itr++;

		if(itr == toklist.end())
		{
			Struct.szSoundStr = NULL;
		}
		else
		{
			if( (*itr).IsOperator() ) {
				itr++;
				Token token = *itr;

				if( !(*itr).IsString() )
				{
					DebugString( "Error TextFile %s, %d : Line : %d\n", fn, (*itr).GetLineNumber() );
				}
				
				const size_t itr_len = strlen( (*itr).GetString() ) + 1;//aleksger - safe string
				Struct.szSoundStr = new char[ itr_len ];
				strcpy_s( Struct.szSoundStr,itr_len, (*itr).GetString() );

			debugValue+=(int)strlen( (*itr).GetString() );

			}
			else {
				itr--;
				Struct.szSoundStr = NULL;
			}
		}

		m_VecElemList.push_back( Struct );

		if( itr == toklist.end() ) break;
	}
	m_pListIndex = NULL;
	GenerateRefArray();
	return true;
}

bool TextTable::SaveTable(char *szFileName)
{
	std::sort(m_VecElemList.begin(),m_VecElemList.end(), TFn_CompareID() );

	FILE *fp = fopen(szFileName, "wt");
	if(fp==NULL)
	{
		return false;
	}

	char szTag[256];
	TextElement textInfo;
	for(int i=0; i<(int)m_VecElemList.size(); i++)
	{
		textInfo = m_VecElemList[i];
		BsAssert((int)strlen(textInfo.szStr)!=0);

		if(textInfo.bEnable == false){
			strcpy(szTag, "// ");
			fwrite(&szTag, sizeof(char)*strlen(szTag), 1, fp);
		}

		memset(szTag, 0, 256);
		if(textInfo.szSoundStr != NULL && strlen(textInfo.szSoundStr) != 0)
		{
			sprintf(szTag, "%d	\"%s\", \"%s\"\n",
				textInfo.nID,
				textInfo.szStr,
				textInfo.szSoundStr);
		}
		else
		{
			sprintf(szTag, "%d	\"%s\"\n",
				textInfo.nID,
				textInfo.szStr);
		}

		fwrite(&szTag, sizeof(char)*strlen(szTag), 1, fp);
	}

	fclose(fp);

	return true;
}


bool TextTable::SaveSoundTable(char *szFileName)
{
	FILE *fp = fopen(szFileName, "wt");
	if(fp==NULL)
	{
		return false;
	}

	char szTag[256], szSpace[64];
	int ndeflen = (int)strlen("#define ");
	TextElement textInfo;
	for(int i=0; i<(int)m_VecElemList.size(); i++)
	{
		textInfo = m_VecElemList[i];
		BsAssert((int)strlen(textInfo.szStr)!=0);
		
		if(textInfo.szSoundStr == NULL){
			continue;
		}

		int nNamelen = (int)strlen(textInfo.szSoundStr);

		memset(szSpace, 0, 64);
		for(int i = ndeflen+nNamelen; i<64; i++)
		{
			strcat(szSpace, " ");
		}

		memset(szTag, 0, 256);
		sprintf(szTag, "#define %s%s%d\n",
			textInfo.szSoundStr,
			szSpace,
			textInfo.nID);

		fwrite(&szTag, sizeof(char)*strlen(szTag), 1, fp);
	}

	fclose(fp);

	return true;
}


void TextTable::GenerateRefArray()
{
	int nCount = m_nMaxIndex - m_nMinIndex + 1;
	m_pListIndex = new short[ nCount ];
	for( int i=0; i<nCount; i++ ) {
		m_pListIndex[i] = -1;
		for( DWORD j=0; j<m_VecElemList.size(); j++ ) {
			if( m_VecElemList[j].nID == m_nMinIndex + i ) {
				m_pListIndex[i] = (short)j;
				break;
			}
		}
	}
}


//table의 ID로 처리 한다.
bool TextTable::GetText( int nID, char *szStr,  const size_t szStr_len, char *szSoundStr, const size_t szSoundStr_len )
{
	const char* INVALID_STR_MSG = "Text Not Found: %d";

	//BsAssert( ( nID - m_nMinIndex >= 0 ) && ( nID - m_nMinIndex <= m_nMaxIndex - m_nMinIndex ) );
	if ( ( nID - m_nMinIndex < 0) || (nID - m_nMinIndex > m_nMaxIndex - m_nMinIndex) ) {
		sprintf_s( szStr, szStr_len, INVALID_STR_MSG, nID ); //aleksger - safe string
		if( szSoundStr )
			sprintf_s( szSoundStr, szSoundStr_len, INVALID_STR_MSG, nID ); //aleksger - safe string

		////DebugString( "Text Not found: %d\n", nID );
		return false;
	}
        
	int nIndex = m_pListIndex[ nID - m_nMinIndex ];
	if(nIndex==-1) {
		sprintf_s( szStr, szStr_len,  INVALID_STR_MSG, nID ); //aleksger - safe string
		if( szSoundStr )
			sprintf_s( szSoundStr, szSoundStr_len, INVALID_STR_MSG, nID ); //aleksger - safe string

		//DebugString("Text Not found: %d\n", nID);
		return false;
	}

	if(m_VecElemList[nIndex].bEnable == false){
		sprintf_s( szStr, szStr_len,  INVALID_STR_MSG, nID ); //aleksger: prefix bug 800
		if( szSoundStr )
			sprintf_s( szSoundStr, szSoundStr_len, INVALID_STR_MSG, nID ); //aleksger - safe string

		BsAssert(0&&"Element is disabled and we previously just failed here.");
		return false;
	}

	size_t nLength = 0;
	if(szSoundStr) {
		if(m_VecElemList[nIndex].szSoundStr) {
			nLength = (int)strlen( m_VecElemList[nIndex].szSoundStr );
			BsAssert( szSoundStr_len > nLength ); //aleksger - use proper length
			strcpy_s( szSoundStr, szSoundStr_len,  m_VecElemList[nIndex].szSoundStr ); //aleksger - safe string
		}
		else {
			szSoundStr[0]=NULL;
		}
	}
	nLength = (int)strlen( m_VecElemList[nIndex].szStr );
	BsAssert( szStr_len > nLength ); //aleksger - use proper length

	strcpy_s( szStr, szStr_len, m_VecElemList[nIndex].szStr );//aleksger - safe string
	return true;
}

bool TextTable::SetText( int nID, char *szStr, char *szSoundStr )
{
	if(IsSameTextID(nID) == true)
		return false;

	TextElement Struct;
	memset( &Struct, 0, sizeof( TextElement ) );
	Struct.nID = nID;
	Struct.bEnable = true;

	const size_t szStr_len = strlen(szStr) + 1; //aleksger - safe string
	Struct.szStr = new char[ szStr_len ];
	strcpy_s( Struct.szStr, szStr_len, szStr );

	if(szSoundStr != NULL)
	{
		const size_t szSoundStr_len = strlen(szSoundStr) + 1;//aleksger - safe string
		Struct.szSoundStr = new char[ szSoundStr_len ];
		strcpy_s( Struct.szSoundStr, szSoundStr_len, szSoundStr );
	}
	else
	{
		Struct.szSoundStr = NULL;
	}

	m_VecElemList.push_back( Struct );

	if( m_nMinIndex > Struct.nID ) m_nMinIndex = Struct.nID;
	if( m_nMaxIndex < Struct.nID ) m_nMaxIndex = Struct.nID;

	if( m_pListIndex )
	{
		delete []m_pListIndex;
		m_pListIndex = NULL;
	}

	std::sort(m_VecElemList.begin(),m_VecElemList.end(), TFn_CompareID() );

	GenerateRefArray();

	return true;
}


bool TextTable::ChangeID( int nSrcID, int nDscID )
{
	if(IsSameTextID(nDscID) == true){
		return false;
	}

	for(DWORD i=0; i<m_VecElemList.size(); i++)
	{
		if(m_VecElemList[i].bEnable == false){
			continue;
		}

		if(	m_VecElemList[i].nID == nSrcID )
		{
			m_VecElemList[i].nID = nDscID;
 
			std::sort(m_VecElemList.begin(),m_VecElemList.end(), TFn_CompareID() );
			return true;
		}
	}

	return false;
}


bool TextTable::ChangeText( int nID, char *szStr)
{
	for(DWORD i=0; i<m_VecElemList.size(); i++)
	{
		if(m_VecElemList[i].bEnable == false){
			continue;
		}

		if(	m_VecElemList[i].nID == nID )
		{	
			if(m_VecElemList[i].szStr != NULL){
				delete[] m_VecElemList[i].szStr;
			}

			const size_t szStr_len = strlen(szStr) + 1; //aleksger - safe string
			m_VecElemList[i].szStr = new char[szStr_len ];
			strcpy_s(m_VecElemList[i].szStr, szStr_len, szStr);

			return true;
		}
	}

	return false;
}

bool TextTable::ChangeSound( int nID, char *szSoundStr)
{
	for(DWORD i=0; i<m_VecElemList.size(); i++)
	{
		if(m_VecElemList[i].bEnable == false){
			continue;
		}

		if(	m_VecElemList[i].nID == nID )
		{
			if(m_VecElemList[i].szSoundStr != NULL){
				delete[] m_VecElemList[i].szSoundStr;
			}

			const size_t szSoundStr_len = strlen(szSoundStr) + 1;//aleksger - safe string
			m_VecElemList[i].szSoundStr = new char[ szSoundStr_len ];
			strcpy_s(m_VecElemList[i].szSoundStr, szSoundStr_len, szSoundStr);			

			return true;
		}
	}

	return false;
}


bool TextTable::RemoveText(int nID)
{
	bool bFlag = false;
	for(int i=0; i < (int)m_VecElemList.size(); i++)
	{
		if(m_VecElemList[i].bEnable == false){
			continue;
		}

		if(m_VecElemList[i].nID == nID)
		{
			if( m_VecElemList[i].szStr )
				delete []m_VecElemList[i].szStr;
			if( m_VecElemList[i].szSoundStr )
				delete []m_VecElemList[i].szSoundStr;

			m_VecElemList.erase(m_VecElemList.begin()+i);
			bFlag = true;

			break;
		}
	}

	if(bFlag == true)
	{
		for(int i=0; i < (int)m_VecElemList.size(); i++)
		{
			if(m_VecElemList[i].bEnable == false){
				continue;
			}

			int tmpID = m_VecElemList[i].nID; //aleksger: prefix bug 861: Local declaration is hiding the parameter.
			if( m_nMinIndex > tmpID ) m_nMinIndex = tmpID;
			if( m_nMaxIndex < tmpID ) m_nMaxIndex = tmpID;
		}

		if( m_pListIndex )
		{
			delete []m_pListIndex;
			m_pListIndex = NULL;
		}

		GenerateRefArray();
	}

	return bFlag;
}

void TextTable::SetEnable(int nID, bool bFlag)
{
	for(DWORD i=0; i<m_VecElemList.size(); i++)
	{
		if(m_VecElemList[i].bEnable == false){
			continue;
		}

		if(	m_VecElemList[i].nID == nID )
		{
			m_VecElemList[i].bEnable = bFlag;
			return;
		}
	}
}

bool TextTable::IsEnable(int nID)
{
	for(DWORD i=0; i<m_VecElemList.size(); i++)
	{
		if(	m_VecElemList[i].nID != nID ){
			continue;
		}

		return m_VecElemList[i].bEnable;
	}

	return false;
}

int TextTable::GetNewID(int nMin, int nMax)
{	
	int nNewID = 0;
	while(1) {
		if(nNewID < nMin || (nMax != -1 && nNewID >= nMax))
			return -1;

		if( IsSameTextID( nNewID ) == false )
			return nNewID;

		++nNewID;
	}

	BsAssert(0);

	return -1;
}


bool TextTable::IsSameTextID(int nID)
{
	for(DWORD i=0; i<m_VecElemList.size(); i++)
	{
		if(m_VecElemList[i].bEnable == false){
			continue;
		}

		if(	m_VecElemList[i].nID == nID )
		{	
			return true;
		}
	}

	return false;
}


//vector의 index로 처리한다.
bool TextTable::GetTextforIndex(int nIndex,  int* nID, char *szStr, const size_t szStr_len, char *szSoundStr, const size_t szSoundStr_len )
{
	if(nIndex >= (int)m_VecElemList.size())
	{
		BsAssert(0);
		return false;
	}

	(*nID) = m_VecElemList[nIndex].nID;

	size_t nLength = 0;
	if(szSoundStr) {
		if(m_VecElemList[nIndex].szSoundStr) {
			nLength = (int)strlen( m_VecElemList[nIndex].szSoundStr );
			BsAssert( szSoundStr_len > nLength );
			strcpy_s( szSoundStr, szSoundStr_len, m_VecElemList[nIndex].szSoundStr );//aleksger - safe string
		}
		else {
			szSoundStr[0]=NULL;
		}
	}
	nLength = (int)strlen( m_VecElemList[nIndex].szStr );
	BsAssert( szStr_len > nLength );

	strcpy_s( szStr, szStr_len, m_VecElemList[nIndex].szStr );//aleksger - safe string

	return true;
}

int TextTable::GetIDforIndex(int nIndex)
{
	if(nIndex >= GetTextCount()){
		BsAssert(0);
		return -1;
	}

	return m_VecElemList[nIndex].nID;
}

void TextTable::SetEnableforIndex(int nIndex, bool bFlag)
{
	if(nIndex >= GetTextCount()){
		BsAssert(0);
		return;
	}

	m_VecElemList[nIndex].bEnable = bFlag;
}

bool TextTable::IsEnableforIndex(int nIndex)
{
	if(nIndex >= GetTextCount()){
		BsAssert(0);
		return false;
	}

	return m_VecElemList[nIndex].bEnable;
}


//------------------------------------------------------------------------------------------------
#ifndef _BSUI_TOOL
//------------------------------------------------------------------------------------------------
FCTextTable::FCTextTable()
{
	m_pDefaultTextTable = NULL;
	m_pUserTextTable = NULL;
	m_pRealMovieTextTable = NULL;

	m_defaultTextName[0] = 0;
	m_userTextName[0] = 0;
	m_realmovieTextName[0] = 0;

	m_nTextTableCount = 0;
	m_pTextTableList = NULL;
}

FCTextTable::~FCTextTable()
{
	Release();
}

void FCTextTable::Release()
{
	if( m_pDefaultTextTable ) {
		delete m_pDefaultTextTable;
		m_pDefaultTextTable = NULL;
	}

	if( m_pUserTextTable ) {
		delete m_pUserTextTable;
		m_pUserTextTable = NULL;
	}

	ReleaseRealMovieTextTable();
}


bool FCTextTable::SetTextTable( int nDefaultID, int nUserID )
{
	return SetDefaultTextTable( nDefaultID ) && SetUserTextTable( nUserID );
}


bool FCTextTable::SetDefaultTextTable( int nDefaultID )
{
	BsAssert( nDefaultID >= 0 );

	char szName[MAX_PATH];

	sprintf( szName, "%stext\\%s\\DefaultTextTable%04d_%s.txt",
		g_BsKernel.GetCurrentDirectory(),
		g_LocalLanguage.GetLanguageDir(), nDefaultID, g_LocalLanguage.GetLanguageStr());

	if ( strcmp( m_defaultTextName, szName ) ) {
		if( m_pDefaultTextTable )
			delete m_pDefaultTextTable;

		m_pDefaultTextTable = new TextTable();
		strcpy( m_defaultTextName, szName );
		return m_pDefaultTextTable->Initialize(szName);
	} else
	{
		//DebugString( "%s load skipped\n", szName );
	}

	return true;
}

bool FCTextTable::SaveDefaultTable(int nDefaultID)
{
	g_BsKernel.chdir("text");

	char szName[MAX_PATH];
	sprintf(szName, "%s\\%s\\DefaultTextTable%04d_%s.txt", 
		g_BsKernel.GetCurrentDirectory(),
		g_LocalLanguage.GetLanguageDir(),
		nDefaultID, 
		g_LocalLanguage.GetLanguageStr());

	g_BsKernel.chdir("..");

	return m_pDefaultTextTable->SaveTable(szName);
}

int FCTextTable::GetDefaultTextCount()
{
	if( m_pDefaultTextTable == NULL )
	{
		BsAssert( 0 );
	}

	return m_pDefaultTextTable->GetTextCount();
}

bool FCTextTable::GetDefaultTextforIndex(int nIndex, int* nID, char *szStr, const size_t szStr_len, char *szSoundStr, const size_t szSoundStr_len)
{
	if( m_pDefaultTextTable == NULL )
	{
		BsAssert( 0 );
	}

	return m_pDefaultTextTable->GetTextforIndex( nIndex, nID, szStr, szStr_len, szSoundStr, szSoundStr_len); //aleksger - safe string
}

int  FCTextTable::GetNewIDforDefaultText()
{
	if( m_pDefaultTextTable == NULL )
	{
		BsAssert( 0 );
	}

	return m_pDefaultTextTable->GetNewID(0, USER_START_INDEX);
}


bool FCTextTable::SetUserTextTable( char* pFileName )
{
	char szName[MAX_PATH];
	strcpy( szName, pFileName );
	BsAssert( strlen( szName ) > 3 );
	char* pTemp = strstr( szName, "." );
	BsAssert(pTemp && (pTemp-szName)>3); //aleksger: prefix bug 866: Check that the dot is found and ther's room.
	*(pTemp-3) = NULL;

	strcat( szName, g_LocalLanguage.GetLanguageStr() );

	char szFullName[MAX_PATH];
	sprintf( szFullName, "%stext\\%s\\%s.txt", g_BsKernel.GetCurrentDirectory(), g_LocalLanguage.GetLanguageDir(), szName);

	if ( strcmp( m_userTextName, szFullName ) ) {
		if( m_pUserTextTable )
			delete m_pUserTextTable;

		m_pUserTextTable = new TextTable();
		strcpy( m_userTextName, szFullName );
		return m_pUserTextTable->Initialize(szFullName);
	} else
	{
		//DebugString( "%s load skipped\n", szName );
	}

	return true;
}

bool FCTextTable::SetUserTextTable( int nUserID )
{
	BsAssert( nUserID >= 0 );

	char szName[MAX_PATH];
	sprintf( szName, "%stext\\%s\\UserTextTable%04d_%s.txt",
				g_BsKernel.GetCurrentDirectory(),
				g_LocalLanguage.GetLanguageDir(), nUserID, g_LocalLanguage.GetLanguageStr());

	if ( strcmp( m_userTextName, szName ) ) {
		if( m_pUserTextTable )
			delete m_pUserTextTable;

		m_pUserTextTable = new TextTable();
		strcpy( m_userTextName, szName );
		return m_pUserTextTable->Initialize(szName);
	} else
	{
		//DebugString( "%s load skipped\n", szName );
	}

	return true;
}


bool FCTextTable::SaveUserTable(int nUserID)
{
	g_BsKernel.chdir("text");

	char szName[MAX_PATH];
	sprintf(szName, "%s\\%s\\UserTextTable%04d_%s.txt", 
		g_BsKernel.GetCurrentDirectory(),
		g_LocalLanguage.GetLanguageDir(),
		nUserID, 
		g_LocalLanguage.GetLanguageStr());

	g_BsKernel.chdir("..");

	return m_pUserTextTable->SaveTable(szName);
}


int FCTextTable::GetUserTextCount()
{
	if( m_pUserTextTable == NULL )
	{
		BsAssert( 0 );
	}

	return m_pUserTextTable->GetTextCount();
}


bool FCTextTable::GetUserTextforIndex(int nIndex, int* nID, char *szStr, const size_t szStr_len, char *szSoundStr, const size_t szSoundStr_len )
{
	if( m_pUserTextTable == NULL )
	{
		BsAssert( 0 );
	}

	return m_pUserTextTable->GetTextforIndex( nIndex,  nID, szStr, szStr_len, szSoundStr, szSoundStr_len );
}

int  FCTextTable::GetNewIDforUserText()
{
	if( m_pUserTextTable == NULL )
	{
		BsAssert( 0 );
	}
	return m_pUserTextTable->GetNewID(USER_START_INDEX, -1);
}

bool FCTextTable::SetRealMovieTextTable( char* pFileName )
{
	/*
	char szName[MAX_PATH];
	strcpy( szName, pFileName );
	char* pTemp = strstr( szName, "." );
	BsAssert( strlen( szName ) > 3 );
	*(pTemp-3) = NULL;

	strcat( szName, g_LocalLanguage.GetLanguageStr() );
	*/

	char szFullName[MAX_PATH];
	// sprintf( szFullName, "%stext\\%s\\%s.txt", g_BsKernel.GetCurrentDirectory(), g_LocalLanguage.GetLanguageDir(), szName);
	sprintf( szFullName, "%stext\\%s\\%s", 
			g_BsKernel.GetCurrentDirectory(), 
			g_LocalLanguage.GetLanguageDir(), 
			pFileName );

	if ( strcmp( m_realmovieTextName, szFullName ) )
	{
		if( m_pRealMovieTextTable ){
			delete m_pRealMovieTextTable;
		}

		m_pRealMovieTextTable = new TextTable();
		strcpy( m_realmovieTextName, szFullName );
		return m_pRealMovieTextTable->Initialize(szFullName);
	} else
	{
		//DebugString( "%s load skipped\n", szName );
	}

	return true;
}

bool FCTextTable::SetRealMovieTextTable( int nRealMovieID )
{
	BsAssert( nRealMovieID >= 0 );

	char szName[MAX_PATH];

	sprintf( szName, "%stext\\%s\\RealTextTable%04d_%s.txt",
		g_BsKernel.GetCurrentDirectory(),
		g_LocalLanguage.GetLanguageDir(), nRealMovieID, g_LocalLanguage.GetLanguageStr());

	if ( strcmp( m_realmovieTextName, szName ) )
	{
		if( m_pRealMovieTextTable ){
			delete m_pRealMovieTextTable;
		}

		m_pRealMovieTextTable = new TextTable();
		strcpy( m_realmovieTextName, szName );
		return m_pRealMovieTextTable->Initialize(szName);
	}
	else
	{
		//DebugString( "%s load skipped\n", szName );
	}

	return true;
}

int	FCTextTable::GetRealMovieTextCount()
{
	if( m_pRealMovieTextTable == NULL ){
		BsAssert( 0 );
	}

	return m_pRealMovieTextTable->GetTextCount();
}

void FCTextTable::ReleaseRealMovieTextTable()
{
	if( m_pRealMovieTextTable ) 
	{
		delete m_pRealMovieTextTable;
		m_pRealMovieTextTable = NULL;

		memset( m_realmovieTextName, 0, MAX_PATH );
	}
}


bool FCTextTable::GetText( int nID, char *szStr, const size_t szStr_len, char *szSoundStr, const size_t szSoundStr_len )
{
	TextTable *pTable = NULL;
	if( nID < USER_START_INDEX ){
		pTable = m_pDefaultTextTable;
	}
	else if( nID <  REAL_START_INDEX ){
		pTable = m_pUserTextTable;
	}
	else{
		pTable = m_pRealMovieTextTable;
	}

	if( pTable == NULL )
	{
		BsAssert( 0 );
	}
	return pTable->GetText( nID, szStr, szStr_len, szSoundStr, szSoundStr_len );
}

bool FCTextTable::SetText( int nID, char *szStr, char *szSoundStr )
{
	TextTable *pTable = NULL;
	if( nID < USER_START_INDEX ){
		pTable = m_pDefaultTextTable;
	}
	else if( nID <  REAL_START_INDEX ){
		pTable = m_pUserTextTable;
	}
	else{
		pTable = m_pRealMovieTextTable;
	}

	if( pTable == NULL )
	{
		BsAssert( 0 );
	}
	return pTable->SetText(  nID, szStr, szSoundStr  );
}

bool FCTextTable::ChangeID( int nSrcID, int nDscID)
{
	return false;
}

bool FCTextTable::ChangeText( int nID, char *szStr )
{
	TextTable *pTable = NULL;
	if( nID < USER_START_INDEX ){
		pTable = m_pDefaultTextTable;
	}
	else if( nID <  REAL_START_INDEX ){
		pTable = m_pUserTextTable;
	}
	else{
		pTable = m_pRealMovieTextTable;
	}

	if( pTable == NULL )
	{
		BsAssert( 0 );
	}
	return pTable->ChangeText( nID, szStr );
}

bool FCTextTable::ChangeSound( int nID, char *szSoundStr )
{
	return false;
}

bool FCTextTable::IsSameTextID(int nID)
{
	TextTable *pTable = NULL;
	if( nID < USER_START_INDEX ){
		pTable = m_pDefaultTextTable;
	}
	else if( nID <  REAL_START_INDEX ){
		pTable = m_pUserTextTable;
	}
	else{
		pTable = m_pRealMovieTextTable;
	}

	if( pTable == NULL )
	{
		BsAssert( 0 );
	}
	return pTable->IsSameTextID(nID);
}

bool FCTextTable::RemoveText( int nID )
{
	TextTable *pTable = NULL;
	if( nID < USER_START_INDEX ){
		pTable = m_pDefaultTextTable;
	}
	else if( nID <  REAL_START_INDEX ){
		pTable = m_pUserTextTable;
	}
	else{
		pTable = m_pRealMovieTextTable;
	}

	if( pTable == NULL )
	{
		BsAssert( 0 );
	}
	return pTable->RemoveText( nID );
}

//------------------------------------------------------------------------------------------------
#endif
//------------------------------------------------------------------------------------------------