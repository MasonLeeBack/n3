#pragma once

#include "bstreamext.h"
#include "BsAniObject.h"
#include "BsFileIO.h"
#include "BsKernel.h"

#include <MAP>
#include <STRING>

//------------------------------------------------------------------------------------------------

class CBsFileText // 공용클래스입니다 (순가상 클래스 입니다)
{
public:	
	float GetFloat  (char *str, int &nIndex, int nSize);
	int   GetInt    (char *str, int &nIndex, int nSize);

	//-----------------------

	void  FSave     (char *szFileName) { // 바이너리 
		char *pFullName=g_BsKernel.GetDirManager()->GetFullName(szFileName);
		BFileStream Stream(pFullName, BFileStream::create);

		if (Stream.Valid()) Save(&Stream);
		else ErrorNotFile(pFullName);
	}
	void  FLoad     (char *szFileName) { // 바이너리 (메모리 자동 소거)
		char *pFullName=g_BsKernel.GetDirManager()->GetFullName(szFileName);
		BFileStream Stream(pFullName, BFileStream::openRead);

		if (Stream.Valid()) Load(&Stream);
		else ErrorNotFile(pFullName);
	}
	void  FLoad_text(char *szFileName) { // 텍스트 파일을 읽어서 바이너리로 유지 합니다 (메모리 자동 소거)
		char *pFullName=g_BsKernel.GetDirManager()->GetFullName(szFileName);
		BFileStream Stream(pFullName, BFileStream::openRead);

		if (Stream.Valid()) Load_text(&Stream);
		else ErrorNotFile(pFullName);
	}

	//-----------------------

	virtual void  Save     (BStream *pStream)=0; // 바이너리 
	virtual void  Load     (BStream *pStream)=0; // 바이너리 (메모리 자동 소거)					  
	virtual void  Load_text(BStream *pStream)=0; // 텍스트 파일을 읽어서 바이너리로 유지 합니다 (메모리 자동 소거)

	//-----------------------

	static void ErrorNotFile(char *szFileName);
};

//------------------------------------------------------------------------------------------------

class CBsFileLTF : public CBsFileText // 음성 애니메이션 파일 입니다 (Impersonator Studio 에서 만들어 집니다)
{
public:	
	int    m_nMax;      // 전체 갯수 입니다

	struct SData {
		int   m_nID;	// (4)
		float m_fBegin;	// (4)
		float m_fEnd;	// (4)
	}
	*m_pData;           // m_nMax 만큼 할당 됩니다

public:	
	CBsFileLTF() { m_nMax=0; m_pData=NULL; 	}
	~CBsFileLTF() { Clear(); }

	void  FLoad_text_(char *szFileName); // empty.ltf 처리를 위한 함수

	void  Clear();

	void  Save     (BStream *pStream); // 바이너리 
	void  Load     (BStream *pStream); // 바이너리 (메모리 자동 소거)					  
	void  Load_text(BStream *pStream); // Impersonator Studio 에서 만든 파일 입니다 (메모리 자동 소거 : LTF -> LTF_B)					  
};

//------------------------------------------------------------------------------------------------

class CBsFileBALM : public CBsFileText // LTF (43개 음) 의 해당 립싱크(BA=1) 매칭 데이타 입니다
{
public:	
	enum { m_nIndex_max=43, };

	int    m_nAniBuf   [m_nIndex_max];
	float  m_fWeightBuf[m_nIndex_max];

public:	
	void  Save     (BStream *pStream); // 바이너리 
	void  Load     (BStream *pStream); // 바이너리 					  
	void  Load_text(BStream *pStream); // 매칭 파일 (텍스트 파일을 읽어서 바이너리로 유지 합니다 : BALM -> BALM_B)
};

// Error 관련
class CBsErrorMsgManager
{
public :
	CBsErrorMsgManager(){};
	~CBsErrorMsgManager()			{	ClearAllMsg();			}

	void	AddErrMsg(char* szMsg)	{	m_vecEMsgPool.push_back( szMsg );	}
	void	ClearAllMsg(void)		{	m_vecEMsgPool.clear();	}

	int		GetMsgPoolSize(void)	{	return m_vecEMsgPool.size();	}
	const char*	GetErrorMsg(int nIndex)	{	return m_vecEMsgPool[nIndex].c_str();}

protected :
	std::vector< std::string >	m_vecEMsgPool;
};

#ifdef _USAGE_TOOL_
extern VOID	InsertPathBetweenHeadAndTail(char* szSrcPath,char* szInsPath);
#endif