#include "stdafx.h"
// #include "BsrealMovie.h"
#include "BsKernel.h"
// #include "BsRealMovieObject.h"
#include "BsRealMovieFile.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG



CBsFileBALM st_BALM;
//------------------------------------------------------------------------------------------------------------------------

float CBsFileText::GetFloat(char *str, int &nIndex, int nSize)
{
	int nMode=0, nGet;
	char pNumber[128];
	int i;
	for ( i=nIndex ; i<nSize ; i++ ) {
		if (nMode==0) {
// [PREFIX:beginmodify] 2006/2/16 junyash PS#5173 reports CBsFileLTF::Load_text(class BStream *): bounds violation (overflow) using buffer 'pBuf' with GetFloat()
			//if (str[i]=='/' && str[i+1]=='/') {
			if (i+1<nSize && str[i]=='/' && str[i+1]=='/') {
// [PREFIX:endmodify] junyash
				nMode=1;
				i++;
			}
			else if (str[i]>='0' && str[i]<='9') {
				pNumber[0]=str[i];
				nGet=1;
				nMode=2;
			}
		}
		else if (nMode==1) {
			if (str[i]=='\n') {
				nMode=0;
			}
		}
		else {
			if ((str[i]>='0' && str[i]<='9') || str[i]=='.') {
				pNumber[nGet++]=str[i];
			}
			else {
				pNumber[nGet]=0;

				nIndex=i+1;
				return (float)atof(pNumber);
			}
		}
	}

	nIndex=i+1;
	return 0;
}

int CBsFileText::GetInt(char *str, int &nIndex, int nSize)
{
	int nMode=0, nGet;
	char pNumber[128];
	int i;
	for ( i=nIndex ; i<nSize ; i++ ) {
		if (nMode==0) {
// [PREFIX:beginmodify] 2006/2/16 junyash PS#5174 reports CBsFileLTF::Load_text(class BStream *): bounds violation (overflow) using buffer 'pBuf' with GetInt()
			//if (str[i]=='/' && str[i+1]=='/') {
			if (i+1<nSize && str[i]=='/' && str[i+1]=='/') {
// [PREFIX:endmodify] junyash
				nMode=1;
				i++;
			}
			else if (str[i]>='0' && str[i]<='9') {
				pNumber[0]=str[i];
				nGet=1;
				nMode=2;
			}
		}
		else if (nMode==1) {
			if (str[i]=='\n') {
				nMode=0;
			}
		}
		else {
			if (str[i]>='0' && str[i]<='9') {
				pNumber[nGet++]=str[i];
			}
			else {
				pNumber[nGet]=0;

				nIndex=i+1;
				return atoi(pNumber);
			}
		}
	}

	nIndex=i+1;
	return 0;
}

void CBsFileText::ErrorNotFile(char *szFileName)
{
	char str[256];
	sprintf(str,"%s NO FILE", szFileName);

#ifdef _XBOX
	g_BsKernel.AddConsoleString( str );
#else
	MessageBox(NULL,str,"Error",MB_OK);
#endif //_XBOX
}

//------------------------------------------------------------------------------------------------------------------------

void CBsFileLTF::FLoad_text_(char *szFileName)
{
	// 코드 추가 구현 필요

	FLoad_text(szFileName);
}

void CBsFileLTF::Clear()
{
	if (m_pData) delete [] m_pData;
	m_pData=NULL;

	m_nMax=0;
}

void CBsFileLTF::Save(BStream *pStream)
{
	if (m_pData) {
		pStream->Write(&m_nMax,  sizeof(int), ENDIAN_FOUR_BYTE);
		pStream->Write(m_pData,  sizeof(SData)*m_nMax, ENDIAN_FOUR_BYTE);
	}
}

void CBsFileLTF::Load(BStream *pStream)
{
	Clear();

	pStream->Read(&m_nMax,  sizeof(int), ENDIAN_FOUR_BYTE);

	m_pData = new SData [m_nMax];
	pStream->Read(m_pData,  sizeof(SData)*m_nMax, ENDIAN_FOUR_BYTE);
}

void CBsFileLTF::Load_text(BStream *pStream) // txt 파일을 읽어서, 정보를 만듭니다 (바이너리 파일)
{
	Clear();

	int nSize=pStream->Length();
	char *pBuf=new char[nSize];
	pStream->Read(pBuf, nSize, 0); // 파일은 자동으로 닫힙니다

	int nIndex=0;
	float fV=GetFloat(pBuf,nIndex,nSize); // 버전 1.0
	m_nMax=GetInt(pBuf,nIndex,nSize); // 갯수

	m_pData = new SData [m_nMax];

	for (int i=0; i<m_nMax; i++) {
		m_pData[i].m_nID   =GetInt  (pBuf,nIndex,nSize); // 0..42
		m_pData[i].m_fBegin=GetFloat(pBuf,nIndex,nSize); // 시작
		m_pData[i].m_fEnd  =GetFloat(pBuf,nIndex,nSize); // 끝
	}

	delete [] pBuf;
}

//------------------------------------------------------------------------------------------------------------------------

void CBsFileBALM::Save(BStream *pStream)
{
	pStream->Write(m_nAniBuf,    sizeof(int  )*m_nIndex_max, ENDIAN_FOUR_BYTE);
	pStream->Write(m_fWeightBuf, sizeof(float)*m_nIndex_max, ENDIAN_FOUR_BYTE);
}

void CBsFileBALM::Load(BStream *pStream)
{
	pStream->Read(m_nAniBuf,    sizeof(int  )*m_nIndex_max, ENDIAN_FOUR_BYTE);
	pStream->Read(m_fWeightBuf, sizeof(float)*m_nIndex_max, ENDIAN_FOUR_BYTE);
}

void CBsFileBALM::Load_text(BStream *pStream) // txt 파일을 읽어서, 정보를 만듭니다 (바이너리 파일)	
{
	int nSize=pStream->Length();
	char *pBuf=new char[nSize];
	pStream->Read(pBuf, nSize, 0); // 파일은 자동으로 닫힙니다

	int nIndex=0;

	for (int i=0; i<m_nIndex_max; i++) {
		m_nAniBuf   [i] = GetInt  (pBuf,nIndex,nSize); // 0 .. 해당 립싱크 BA의 애니 프레임 갯수 - 2
		m_fWeightBuf[i] = GetFloat(pBuf,nIndex,nSize); 
	}

	delete [] pBuf;
}

// Utility
#ifdef _USAGE_TOOL_
VOID	InsertPathBetweenHeadAndTail(char* szSrcPath,char* szInsPath)
{
	char szPath[256];
	char szHeadBuffer[64];
	char szTailBuffer[64];
	char* pdest;

	strcpy( szPath , szSrcPath );
	szPath[ strlen( szPath ) - 1 ] = NULL;
	// "RealMovie EVI01"

	pdest = strrchr( szPath, '\\' );

	if( pdest == NULL )
	{
		strcpy( szHeadBuffer , szPath );	// 머리 부분
		sprintf( szPath, "%s\\%s\\", szHeadBuffer, szInsPath );
		strcpy( szSrcPath , szPath );
	}
	else
	{
		*pdest = NULL;

		strcpy( szHeadBuffer , szPath );	// 머리 부분
		strcpy( szTailBuffer , pdest+1 );	// 꼬리 부분

		sprintf( szPath, "%s\\%s\\%s\\", szHeadBuffer, szInsPath, szTailBuffer );
		strcpy( szSrcPath , szPath );
	}
}
#endif
