#ifndef __BS_TRIGGER_H__
#define __BS_TRIGGER_H__
#include "BSTriggerStrDefinition.h"
#include "Token.h"
class CBSConditionForTool;
class CBSActionForTool;
class BStream;
class TokenList;
class BFileStream;


class CBSTriggerEventBase
{
public:
	CBSTriggerEventBase();
	virtual ~CBSTriggerEventBase();
	void AddCondition(CBSConditionForTool *Data);
	void InsertCondition(int InsertNum,CBSConditionForTool *Data);
	void InsertAction(int InsertNum,CBSActionForTool *Data);
	void AddAction(CBSActionForTool *Data);
	int  GetConditionCount(){return (int)m_ConditionList.size();}
	int  GetActionCount(){return (int)m_ActionList.size();}
	CBSConditionForTool *GetCondition(int nIndex){return m_ConditionList[nIndex];}
	CBSActionForTool *GetAction(int nIndex){return m_ActionList[nIndex];}
	void DeleteCondition(int iConditionIndex);
	void DeleteAction(int iActionIndex);
	void ReplaceCondition(int Srcindex,int DestIndex);
	void ReplaceAction(int Srcindex,int DestIndex);
	bool FixDataName(char *pOldName,char *pNewName);
protected:
	std::vector<CBSConditionForTool *>m_ConditionList;
	std::vector<CBSActionForTool	*>m_ActionList;	
};

class CBSTriggerForTool : public CBSTriggerEventBase
{
public:
	CBSTriggerForTool();
	~CBSTriggerForTool();
	bool SetName(char *szName);
	char *GetName(){return m_Name;}

	void SaveTrigger(BFileStream *fp,int nVer);
	void SaveTriggerToText(FILE *fp);
	void LoadTrigger(BStream *fp,int nVer);
	void LoadTriggerToText(TokenList::iterator &itr);	
	BOOL IsEnable();
	void SetEnable(BOOL bEnable);
	void GetConditionList(std::vector<CBSConditionForTool *> &ConditionList);
	void GetActionList(std::vector<CBSActionForTool	*> &ActionList	);
	void Clear();

protected:
	char m_Name[MAX_BS_TRIGGER_NAME];

};

void memcpyTrigger(CBSTriggerForTool *pBuf,CBSTriggerForTool *pSrc);
void ClearTrg(CBSTriggerForTool *pBuf);


#endif