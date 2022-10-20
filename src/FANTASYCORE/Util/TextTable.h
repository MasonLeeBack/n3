#ifndef _TEXT_TABLE_H_
#define _TEXT_TABLE_H_

#include <vector>
#include "bstreamext.h"

#define DEFUALT_START_INDEX		0
#define USER_START_INDEX		2000
#define REAL_START_INDEX		8000		//real movie

struct TextElement {
	TextElement(){
		nID = -1;
		szStr = NULL;
		szSoundStr = NULL;
		bEnable = true;
	};

	int nID;
	char *szStr;
	char *szSoundStr;
	bool bEnable;
};

class TextTable {
public:
	TextTable();
	~TextTable();

	bool Initialize( char *szFileName );
	bool SaveTable(char *szFileName);
	bool SaveSoundTable(char *szFileName);

	//table의 ID로 처리 한다.
	bool GetText( int nID, char *szStr,  const size_t szStr_len, char *szSoundStr = NULL, const size_t szSoundStr_len = 0);//aleksger - safe string
	bool SetText( int nID, char *szStr, char *szSoundStr = NULL );

	bool ChangeID( int nSrcID, int nDscID );
	bool ChangeText( int nID, char *szStr );
	bool ChangeSound( int nID, char *szSoundStr );
	
	bool RemoveText(int nID);

	void SetEnable(int nID, bool bFlag);
	bool IsEnable(int nID);

	int  GetNewID(int nMin, int nMax);
	bool IsSameTextID(int nID);
	int	 GetTextCount()					{ return (int)m_VecElemList.size(); }

	//vector의 index로 처리한다.
	bool GetTextforIndex(int nIndex, int* nID, char *szStr, const size_t szStr_len,  char *szSoundStr=NULL, const size_t szSoundStr_len=0);//aleksger - safe string
	int  GetIDforIndex(int nIndex);
	void SetEnableforIndex(int nIndex, bool bFlag);
	bool IsEnableforIndex(int nIndex);
	
private:
	
	void Destroy();
	void GenerateRefArray();

	std::vector<TextElement> m_VecElemList;
	short *m_pListIndex;
	int m_nMinIndex;
	int m_nMaxIndex;
};

//------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------
class FCTextTable {
public:
	FCTextTable();
	~FCTextTable();

	void Release();

	bool SetTextTable( int nDefaultID, int nUserID );

	//default
	bool SetDefaultTextTable( int nDefaultID );
	bool SaveDefaultTable(int nDefaultID);
	int	 GetDefaultTextCount();
	bool GetDefaultTextforIndex(int nIndex, int* nID, char *szStr, const size_t szStr_len,  char *szSoundStr = NULL, const size_t szSoundStr_len=0 ); //aleksger - safe string
	int  GetNewIDforDefaultText();
	bool IsDefaultTextEnableforIndex(int nIndex)		{ return true; }
		
	//user
	bool SetUserTextTable( char* pFileName );
	bool SetUserTextTable( int nUserID );
	bool SaveUserTable(int nUserID);
	int	 GetUserTextCount();
	bool GetUserTextforIndex(int nIndex, int* nID, char *szStr, const size_t szStr_len, char *szSoundStr = NULL, const size_t szSoundStr_len=0 ); //aleksger - safe string
	int  GetNewIDforUserText();

	//Real Movie
	bool SetRealMovieTextTable( char* pFileName );
	bool SetRealMovieTextTable( int nRealMovieID );
	int	 GetRealMovieTextCount();
	void ReleaseRealMovieTextTable();

public:
	bool GetText( int nID, char *szStr, const size_t szStr_len,char *szSoundStr = NULL, const size_t szSoundStr_len=0 );//aleksger - safe string
	bool SetText( int nID, char *szStr, char *szSoundStr = NULL );//aleksger - safe string

	bool ChangeID( int nSrcID, int nDscID);
	bool ChangeText( int nID, char *szStr );
	bool ChangeSound( int nID, char *szSoundStr);
	
	bool IsSameTextID(int nID);
	bool RemoveText(int nID);

protected:
	TextTable *m_pDefaultTextTable;
	TextTable *m_pUserTextTable;
	TextTable *m_pRealMovieTextTable;

	char m_defaultTextName[MAX_PATH];
	char m_userTextName[MAX_PATH];
	char m_realmovieTextName[MAX_PATH];

    	
protected:
	typedef struct _TextTableInfo
	{
		_TextTableInfo()
		{
			memset(m_szFileName, 0, MAX_PATH);
			m_pTable = NULL;
		};

		char m_szFileName[MAX_PATH];
		TextTable* m_pTable;
	} TextTableInfo;
	
	int				m_nTextTableCount;
	TextTableInfo*	m_pTextTableList;
};

extern FCTextTable* g_TextTable;

#endif