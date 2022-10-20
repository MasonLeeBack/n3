#ifndef __BS_TRIGGER_VARIABLE_H__
#define __BS_TRIGGER_VARIABLE_H__
#include "BSTriggerManager.h"
#include "Token.h"

class BStream;
class CBSTriggerParam;
class BFileStream;
#define BSTG_VAR_NAME	0
#define BSTG_VAR_TYPE	1
#define BSTG_VAR_VAL	2
#define BSTG_VAR_DESC	3



class CBSVariable 
{
public:
	CBSVariable();
	~CBSVariable();
	int GetParamCount(){return (int)m_ParamList.size();}
	CBSTriggerParam *GetParamData(int nIndex){return m_ParamList[nIndex];}	
	void CreateDefaultParameter();

	void SaveVariable(BFileStream *fp,int nVer);
	void SaveVariableToText(FILE *fp);
	void LoadVariable(BStream *fp,int nVer);
	void LoadVariableToText(TokenList::iterator &itr);

	void FillToBuffer(char *pBuf,int nIndex);
	void FillToVariable(char *pBuf);
	int  GetTotalVariableSize();
#ifdef _USAGE_TOOL_
	char *GetSentence();
#endif
	void AddParameter(CBSTriggerParam *pParam);

protected:
	std::vector<CBSTriggerParam *>m_ParamList;
};

void memcpyVar(CBSVariable *pDest,CBSVariable *pSource);
#endif
