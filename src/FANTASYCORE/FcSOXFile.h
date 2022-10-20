#pragma once
#pragma warning(disable:4786)	// vector warning disable

#include <vector>
#include <map>
#include "bstreamext.h"
#include "assocarray.h"

#define MAX_FIELD_COUNT 128

class CFcSOXFile
{
public:
	CFcSOXFile();
	CFcSOXFile( const char *szFileName );
	CFcSOXFile( BStream *pStream );
	virtual ~CFcSOXFile();

	enum FIELD_TYPE {
		FT_NA	= 0,
		FT_STRING = 1,
		FT_BOOL = 2,
		FT_INT = 3,
		FT_PER = 4,
		FT_FLOAT = 5,

		FT_PRIMARY_KEY = 255,
	};

	struct SOX_Field {
		void Init() {
			pName=NULL;
			nSize=0;
			nFldTyp=0;
			uData.nValue=0;
		}

		char *pName;
		int nFldTyp;
		int	nSize;
		union {
			char* pValue;
			int	  nValue;
			float fValue;
		} uData;
	};

public:
	bool Load( const char *szFileName );
	bool Load( BStream* pStream );
    void Reset();

	// 주의 !: lItemID는 0이 아니라 1부터 시작한다. 그리고, DB에 따라 항상 1,2,3 은 아님. 하지만 DB 내에선 Unique한 숫자임.
    bool GetField( int nItemID, int nFieldNum, SOX_Field& rSOXField );										// item ID와 field 인덱스를 넣으면 field 구조체를 세팅해준다
	SOX_Field *GetFieldPtr( int nItemID, int nFieldNum );
	bool GetFieldFromLable( int nItemID, const char* cFieldLabel, SOX_Field& rSOXField);					// item ID와 field label을 넣으면 field 구조체를 세팅해준다
	SOX_Field *GetFieldFromLablePtr( int nItemID, const char* cFieldLable );
	int GetDB_ID() { return m_lDB_ID; }
	int GetItemCount() { return m_SOXprimaryidx.size(); }
	int GetFieldCount() { return m_nFieldCount; }
	const char* GetFieldLabel( int nField ) { return m_Label[nField].String; }
	int GetFieldNum( const char* cFieldLabel);
	int GetPrimaryKey( int nIndex) { return m_SOXLinearDB[nIndex * (GetFieldCount()+1) ].uData.nValue ; }	// 레코드 인덱스로부터 그 레코드의 primary key (lItemID)를 얻는다.
    int GetIDXprimary( int nPrimaryKey );																	// 해당 primary key (lItemID) 의 레코드 인덱스를 되돌린다.

protected:

	struct SOX_STR64 {
		char	String[64];
	};

	std::vector<SOX_Field> m_SOXLinearDB; // DB
	std::map<int, int> m_SOXprimaryidx; // Item ID to Index
	std::vector<SOX_STR64> m_Label;						// 구분자
	assoc_array<int, string_mod_hash_fn> m_LabelHash;	// 구분자해시

	int m_lDB_ID;
	int m_nFieldCount;
};