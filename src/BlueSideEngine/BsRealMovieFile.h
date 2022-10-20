#pragma once

#include "bstreamext.h"
#include "BsAniObject.h"
#include "BsFileIO.h"
#include "BsKernel.h"

#include <MAP>
#include <STRING>

//------------------------------------------------------------------------------------------------

class CBsFileText // ����Ŭ�����Դϴ� (������ Ŭ���� �Դϴ�)
{
public:	
	float GetFloat  (char *str, int &nIndex, int nSize);
	int   GetInt    (char *str, int &nIndex, int nSize);

	//-----------------------

	void  FSave     (char *szFileName) { // ���̳ʸ� 
		char *pFullName=g_BsKernel.GetDirManager()->GetFullName(szFileName);
		BFileStream Stream(pFullName, BFileStream::create);

		if (Stream.Valid()) Save(&Stream);
		else ErrorNotFile(pFullName);
	}
	void  FLoad     (char *szFileName) { // ���̳ʸ� (�޸� �ڵ� �Ұ�)
		char *pFullName=g_BsKernel.GetDirManager()->GetFullName(szFileName);
		BFileStream Stream(pFullName, BFileStream::openRead);

		if (Stream.Valid()) Load(&Stream);
		else ErrorNotFile(pFullName);
	}
	void  FLoad_text(char *szFileName) { // �ؽ�Ʈ ������ �о ���̳ʸ��� ���� �մϴ� (�޸� �ڵ� �Ұ�)
		char *pFullName=g_BsKernel.GetDirManager()->GetFullName(szFileName);
		BFileStream Stream(pFullName, BFileStream::openRead);

		if (Stream.Valid()) Load_text(&Stream);
		else ErrorNotFile(pFullName);
	}

	//-----------------------

	virtual void  Save     (BStream *pStream)=0; // ���̳ʸ� 
	virtual void  Load     (BStream *pStream)=0; // ���̳ʸ� (�޸� �ڵ� �Ұ�)					  
	virtual void  Load_text(BStream *pStream)=0; // �ؽ�Ʈ ������ �о ���̳ʸ��� ���� �մϴ� (�޸� �ڵ� �Ұ�)

	//-----------------------

	static void ErrorNotFile(char *szFileName);
};

//------------------------------------------------------------------------------------------------

class CBsFileLTF : public CBsFileText // ���� �ִϸ��̼� ���� �Դϴ� (Impersonator Studio ���� ����� ���ϴ�)
{
public:	
	int    m_nMax;      // ��ü ���� �Դϴ�

	struct SData {
		int   m_nID;	// (4)
		float m_fBegin;	// (4)
		float m_fEnd;	// (4)
	}
	*m_pData;           // m_nMax ��ŭ �Ҵ� �˴ϴ�

public:	
	CBsFileLTF() { m_nMax=0; m_pData=NULL; 	}
	~CBsFileLTF() { Clear(); }

	void  FLoad_text_(char *szFileName); // empty.ltf ó���� ���� �Լ�

	void  Clear();

	void  Save     (BStream *pStream); // ���̳ʸ� 
	void  Load     (BStream *pStream); // ���̳ʸ� (�޸� �ڵ� �Ұ�)					  
	void  Load_text(BStream *pStream); // Impersonator Studio ���� ���� ���� �Դϴ� (�޸� �ڵ� �Ұ� : LTF -> LTF_B)					  
};

//------------------------------------------------------------------------------------------------

class CBsFileBALM : public CBsFileText // LTF (43�� ��) �� �ش� ����ũ(BA=1) ��Ī ����Ÿ �Դϴ�
{
public:	
	enum { m_nIndex_max=43, };

	int    m_nAniBuf   [m_nIndex_max];
	float  m_fWeightBuf[m_nIndex_max];

public:	
	void  Save     (BStream *pStream); // ���̳ʸ� 
	void  Load     (BStream *pStream); // ���̳ʸ� 					  
	void  Load_text(BStream *pStream); // ��Ī ���� (�ؽ�Ʈ ������ �о ���̳ʸ��� ���� �մϴ� : BALM -> BALM_B)
};

// Error ����
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