#ifndef __BS_TRIGGER_CONDITION_H__
#define __BS_TRIGGER_CONDITION_H__
#include "BSTriggerStrDefinition.h"
#include "BSTriggerDefaultData.h"
#include "Token.h"

class CBSTriggerParam;
class BStream;
class CFile;
class CBSTriggerManager;
class BFileStream;


class CBSConditionActionBase
{
public:
	CBSConditionActionBase();
	virtual ~CBSConditionActionBase();	
	int  GetCodeIndex(){return m_iCodeIndex;}
	void SetCodeIndex(int iIndex){m_iCodeIndex = iIndex;}
	void SetMyNumber(int iNumber){m_iMyNumber = iNumber;}
	int  GetMyNumber(){return m_iMyNumber;}
	void SetEnable(bool bPlayed){m_bEnable = bPlayed;}
	bool IsEnable(){return m_bEnable;}
	void AddParameterData(CBSTriggerParam *pData);
	CBSTriggerParam *GetParamData(int nIndex){return m_ParamList[nIndex];}
	int GetParamCount(){return (int)m_ParamList.size();}	


	void GetParamInt(int iIndex,int &iData);
	void GetParamStr(int iIndex,char *szData, const size_t szData_len); //aleksger - safe string
	void GetParamFloat(int iIndex,float &fData);

	int  GetParamInt(int iIndex);
	char *GetParamStr(int iIndex);
	float GetParamFloat(int iIndex);



	void Save(BFileStream *fp,int nVer);
	void Load(BStream *fp,int nVer);

	bool Compare( int iValue1, int iOper, int iValue2 );	
	bool IsInRect( float fX, float fY, float fSX, float fSY, float fEX, float fEY );
	void ClearParam();
	BOOL IsSkip() { return m_bSkip; }
	void SetSkip();
	void SetSkip(BOOL bSkip){m_bSkip =  bSkip;}
	
//공용
	virtual void Initialize(){};
//Condition전용
	int  GetOperIndex(){return m_iORIndex;}
	void  SetOperIndex(int iOperIndex){m_iORIndex = iOperIndex;}
	virtual bool IsTrue(){return false;}
//Action전용
	virtual int DoCommand(){return true;}
protected:
	std::vector<CBSTriggerParam *>m_ParamList;
	int  m_iCodeIndex;	
	int  m_iMyNumber;    //툴상 선택 목록에서의 내 위치
	bool m_bEnable;
	BOOL m_bSkip;
//Condition
	int  m_iORIndex;
};

//-----------------------------------------------------------------------------------------------------
class CBSConditionActionForToolBase : public CBSConditionActionBase //툴에서만 쓰는 베이스 클래스
{
public:
	CBSConditionActionForToolBase();
	bool SetSentence(const char *szSentence);	
	bool GetSentence(char *szBuf,int iMax);
	char *GetSentence(){return m_Sentence;}
	char *GetCaption(){return m_Caption;}	
	bool SetCaption(const char *szCaption);
#ifdef _USAGE_TOOL_
	void GetCompleteSentence(char *szBuf,char *szOrg,CBSTriggerParam *pData,int iSequence,char *pReplceStr=NULL);	
#endif
	void FillToBuffer(char *pBuf,int iStartPos);
	void FillFromBuffer(char *pBuf);
	int GetTotalSizeForClibBoard();
	bool FixDataName(char *pOldName,char *pNewName);
	void GetParamList(std::vector<CBSTriggerParam *> &ParamList);
protected:
	char m_Sentence[MAX_BS_STR_SENTENCE]; //완성된 트리거 문장
	char m_Caption[MAX_BS_STR_SENTENCE];  //사용자가 알아보기 쉽도록 쓴 글
};

//-----------------------------------------------------------------------------------------------------
class CBSConditionForTool : public CBSConditionActionForToolBase //Tool 용 클래스
{
public:
	CBSConditionForTool();	
	void FillToCondition(char *pBuf);
	void SaveCondition(BFileStream *fp,int nVer);
	void SaveConditionToText(FILE *fp);
	void LoadCondition(BStream *fp,int nVer);	
	void LoadConditionToText(TokenList::iterator &it);
	void ResetOperInfo(){m_iORIndex = -1;}
};

void memcpyCondition(CBSConditionForTool *pDest,CBSConditionForTool *pSrc);
#endif
