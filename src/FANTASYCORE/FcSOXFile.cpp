#include "StdAfx.h"
//#include "BsKernel.h"
#include "FcSOXFile.h"
#include "bstreamext.h"
#include "BsKernel.h"
#include "BsFileIO.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG

CFcSOXFile::CFcSOXFile()
{
	Reset();
}

CFcSOXFile::CFcSOXFile( const char *szFileName )
{
	Reset();
	Load( szFileName );
}

CFcSOXFile::CFcSOXFile( BStream *pStream )
{
	Reset();
	Load( pStream );
}


CFcSOXFile::~CFcSOXFile()
{
    Reset();
}

void CFcSOXFile::Reset()
{
    for( DWORD i=0; i<m_SOXLinearDB.size() ; i++ ) {
        if( m_SOXLinearDB[i].nFldTyp == FT_STRING ) {
			if( m_SOXLinearDB[i].uData.pValue ) {
                delete [] m_SOXLinearDB[i].uData.pValue;
			}
        }
    }

    m_SOXLinearDB.clear();
	m_SOXprimaryidx.clear();
	m_Label.clear();
	m_LabelHash.clear();

	m_lDB_ID = -1;
	m_nFieldCount = 0;
}

bool CFcSOXFile::Load( const char *szFileName )
{
	DWORD dwFileSize;
	VOID *pData;

	if( FAILED(CBsFileIO::LoadFile( szFileName, &pData, &dwFileSize ) ) )
	{
		BsAssert(0&&"File Not Found");
//		CBsConsole::GetInstance().AddFormatString("%s File Not Found!!!", szFileName);
		return false;
	}
	BMemoryStream Stream(pData, dwFileSize);;
	bool bRet = Load( &Stream );
	CBsFileIO::FreeBuffer(pData);
	return bRet;
}

bool CFcSOXFile::Load( BStream* pStream )
{
	SOX_Field	tmpCell;
	int			tmpInt;
	short       tmpShort;
	float		tmpFloat;

	if( pStream->Valid() == false ) return false;
    Reset();

	// 헤더 읽기
	int nDataCount;
	pStream->Read( &m_lDB_ID, sizeof(int), ENDIAN_FOUR_BYTE );
	pStream->Read( &m_nFieldCount, sizeof(int), ENDIAN_FOUR_BYTE );
	pStream->Read( &nDataCount, sizeof(int), ENDIAN_FOUR_BYTE );

	SOX_STR64 szFieldName[MAX_FIELD_COUNT];
	int nFieldType[MAX_FIELD_COUNT];

	for( int i=0; i<MAX_FIELD_COUNT; i++ ) {
		pStream->Read( &szFieldName[i], sizeof(SOX_STR64) );
		pStream->Read( &nFieldType[i], sizeof(int), ENDIAN_FOUR_BYTE );
	}

	// 구분자
	m_Label.resize( m_nFieldCount );
	for( int i=0; i<m_nFieldCount; i++ ) {
		m_Label[i] = szFieldName[i];
		m_LabelHash.insert( m_Label[i].String, i );
	}

	int cnt = 0;

	m_SOXLinearDB.resize( nDataCount * m_nFieldCount + nDataCount );
	for( int i=0; i<nDataCount; i++ ) {
		pStream->Read( &tmpInt, sizeof(int), ENDIAN_FOUR_BYTE );

		tmpCell.nSize        = sizeof(int);
		tmpCell.uData.nValue = tmpInt;
		tmpCell.nFldTyp      = FT_PRIMARY_KEY;

        m_SOXLinearDB[cnt++] = tmpCell;
        m_SOXprimaryidx.insert( std::map<int, int>::value_type( tmpInt, i * ( m_nFieldCount + 1 ) ) );

		for( int j=0; j<m_nFieldCount; ++j ) {
			tmpCell.pName = m_Label[j].String;
			tmpCell.nFldTyp = nFieldType[j];

			switch( tmpCell.nFldTyp ) {
			case FT_NA:
				pStream->Read( &tmpInt, sizeof(int), ENDIAN_FOUR_BYTE );
				tmpCell.nSize=0;
				tmpCell.uData.pValue=NULL;
				break;
			case FT_STRING:
				{				
					pStream->Read( &tmpShort, sizeof(short), ENDIAN_TWO_BYTE );

					if( tmpShort < 0 || tmpShort >= 4096 ) return false;

					char strRead[4096] = {0, };
					pStream->Read( strRead, tmpShort );
					strRead[tmpShort] = 0;

					short sLeng = (short)strlen(strRead);
					tmpCell.nSize = sLeng;
					tmpCell.uData.pValue = new char[sLeng + 1];
					strcpy_s(tmpCell.uData.pValue, sLeng+1, strRead); //aleksger - safe string
					tmpCell.uData.pValue[sLeng] = 0;
				}
				break;

			case FT_BOOL:
				pStream->Read( &tmpInt, sizeof(int), ENDIAN_FOUR_BYTE );
				tmpCell.nSize = sizeof(int);
				tmpCell.uData.nValue = tmpInt;
				break;

			case FT_INT:
				pStream->Read( &tmpInt, sizeof(int), ENDIAN_FOUR_BYTE );
				tmpCell.nSize = sizeof(int);
				tmpCell.uData.nValue= tmpInt;
				break;

			case FT_PER:
				pStream->Read( &tmpFloat, sizeof(float), ENDIAN_FOUR_BYTE );
				tmpCell.nSize = sizeof(float);
				tmpCell.uData.fValue = tmpFloat;
				break;

			case FT_FLOAT:
				pStream->Read( &tmpFloat, sizeof(float), ENDIAN_FOUR_BYTE );
				tmpCell.nSize = sizeof(float);
				tmpCell.uData.fValue = tmpFloat;
				break;

			default :
				BsAssert(0);	// invalid field type !
			}
            m_SOXLinearDB[cnt++] = tmpCell;
		}
	}
    return true;
}

int CFcSOXFile::GetIDXprimary( int nPrimaryKey )
{
	std::map<int, int>::iterator location = m_SOXprimaryidx.find(nPrimaryKey);

    return (location != m_SOXprimaryidx.end()) ? location->second : (-1);
}

bool CFcSOXFile::GetField( int nItemID, int nFieldNum, SOX_Field& rSOXField )
{
    int nIndex = GetIDXprimary(nItemID);
	if( nIndex == -1 || nFieldNum < 0 || nFieldNum >= GetFieldCount() ) {
		rSOXField.Init();
		return false;
	}

    rSOXField = m_SOXLinearDB[nIndex + nFieldNum + 1];
	return true;
}

CFcSOXFile::SOX_Field *CFcSOXFile::GetFieldPtr( int nItemID, int nFieldNum )
{
	int nIndex = GetIDXprimary(nItemID);
	if( nIndex == -1 || nFieldNum < 0 || nFieldNum >= GetFieldCount() ) return NULL;

	return &m_SOXLinearDB[nIndex + nFieldNum + 1];
}

bool CFcSOXFile::GetFieldFromLable( int nItemID, const char* cFieldLabel, SOX_Field& rSOXField )
{
    int nIndex = GetIDXprimary(nItemID);
	if( nIndex == -1 ) {
		rSOXField.Init();
		return false;
	}

	int i = GetFieldNum( cFieldLabel );
	if( i >= 0 ) {
		rSOXField = m_SOXLinearDB[ nIndex + i + 1 ];
		return true;
	}

	BsAssert(0 && "SOX 데이타 없오! 확인 하시오!!!");				// 잘못된 주소를 호출한 듯 하오.
	rSOXField.Init();
	return false;
}

CFcSOXFile::SOX_Field *CFcSOXFile::GetFieldFromLablePtr( int nItemID, const char* cFieldLable )
{
	int nIndex = GetIDXprimary(nItemID);
	if( nIndex == -1 ) return NULL;

	int i = GetFieldNum( cFieldLable );
	if( i >= 0 ) return &m_SOXLinearDB[ nIndex + i + 1 ];

	return NULL;
}

int	CFcSOXFile::GetFieldNum( const char* cFieldLabel)
{
	int *resultp;

	resultp = m_LabelHash.search( cFieldLabel );

	return ( resultp == NULL ) ? -1 : *resultp;
}
