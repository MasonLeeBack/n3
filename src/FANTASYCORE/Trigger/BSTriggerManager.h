#ifndef __BS_TRIGGER_MANAGER_H__
#define __BS_TRIGGER_MANAGER_H__

#include "BSTriggerStrDefinition.h"
#define ConvertToString(x) #x

class CFile;
class BStream;
class CBSTriggerForTool;
class CBSConditionForTool;
class CBSActionForTool;
class CBSTriggerParam;
class CBSVariable;
class CBSEventForTool;
class CBSConditionActionForToolBase;
class BFileStream;
class CBSTriggerParamEx;
class CBSTriggerEventBase;

#define BS_COMBO_STR_MAX			128
#define BS_COMBO_STR_VALTYPE_MAX	4

enum BST_TRGMODE
{
	BST_TRIGGER,
	BST_EVENT,
};

enum
{
	BS_TG_TYPE_NONE = -1,
	BS_TG_TYPE_INT,
	BS_TG_TYPE_FLOAT,
	BS_TG_TYPE_STRING,
	BS_TG_TYPE_VARIABLE,
};

enum
{
	BS_TG_CTRL_NONE = -1,
	BS_TG_CTRL_EDIT,
	BS_TG_CTRL_COMBOBOX,
};

enum
{	
	OPER_EQUAL,
	OPER_BIGGER,
	OPER_SMALLER,
	OPER_BIGGER_EQUAL,
	OPER_SMALLER_EQUAL,
};

enum
{
	BS_TG_VALTYPE_INT,
	BS_TG_VALTYPE_BOOL,
	BS_TG_VALTYPE_RANDOM,
	BS_TG_VALTYPE_STRING,
	BS_TG_VALTYPE_GLOBAL,

};

/**
	텍스트 형태로 읽기 위한 키워드 정의
**/
#define	BSKW_TRIGGER_COUNT			"TRIGGER_COUNT"
#define	BSKW_EVENT_COUNT			"EVENT_COUNT"
#define BSKW_VARIABLE_COUNT			"VARIABLE_COUNT"

#define BSKW_CONDITION_COUNT		"CONDITION_COUNT"
#define BSKW_ACTION_COUNT			"ACTION_COUNT"
#define BSKW_TRIGGER_NAME			"TRIGGER_NAME"
#define BSKW_EVENT_NAME				"EVENT_NAME"
#define BSKW_CONDITION_COUNT		"CONDITION_COUNT"
#define BSKW_ACTION_COUNT			"ACTION_COUNT"
#define BSKW_CONDITION_INFOMATION	"CONDITION_INFOMATION"
#define BSKW_ACTION_INFOMATION		"ACTION_INFOMATION"
#define BSKW_CODE_INDEX				"CODE_INDEX"
#define BSKW_PARAMETER_COUNT		"PARAM_COUNT"
#define BSKW_TOOL_NUMBER			"TOOL_NUMBER"
#define BSKW_TOOL_ORINDEX			"TOOL_ORINDEX"
#define BSKW_DATA_TYPE				"DATA_TYPE"
#define BSKW_CTRL_TYPE				"CTRL_TYPE"
#define BSKW_CODE_INDEX				"CODE_INDEX"
#define BSKW_DATA					"DATA"
#define BSKW_EVENT_HANDLE			"EVENT_HANDLE"


class CBSTriggerManager
{
public:
	CBSTriggerManager();
	~CBSTriggerManager();
public:
	bool Load(BStream *fp);
	bool Load(char *szFileName);
	bool LoadToText(char *szFileName);

	bool Save(BFileStream *fp);
	bool Save(char *szFileName);
	bool SaveToText(char *szFileName);
	void Clear();
#ifdef _USAGE_TOOL_
	void ReadParamList();
	void ReadParamInfoList();

	DWORD	DocCommand(UINT nMsg,DWORD dwParam1=0,DWORD dwParam2=0);

#endif

	void AddEmptyTrigger(char *szTriggerName);
	void AddEmptyCondition(CBSTriggerEventBase *pBase,int SelectableIndex);
	void AddEmptyAction(CBSTriggerEventBase *pBase,int SelectableIndex);

	int GetTriggerCount(){return (int)m_TriggerList.size();}
	CBSTriggerForTool *GetTrigger(int nIndex){return m_TriggerList[nIndex];}
	CBSConditionForTool *GetSelectableCondition(int nIndex){return m_SelectableConditionList[nIndex];}
	CBSActionForTool	*GetSelectableAction(int nIndex){return m_SelectableActionList[nIndex];}

	int  GetSelectableConditionCount(){return (int)m_SelectableConditionList.size();}
	int  GetSelectableActionCount(){return (int)m_SelectableActionList.size();}
	int  GetVariableCount(){return (int)m_ValList.size();}
	void DeleteTrigger(int iTriggerIndex);
	void DeleteVariable(int iValIndex);
	void AddEmptyVariable();
	CBSVariable *GetVariable(int iIndex){return  m_ValList[iIndex];}
	void ReplaceVariable(int SrcIndex,int DestIndex);
	int  GetEventCount(){return (int)m_EventList.size();}
	CBSEventForTool *GetEvent(int iIndex){return m_EventList[iIndex];}
	void CreateEmptyEvents();
	void DeleteEvents();
	void CopySentence(CBSConditionForTool *pCondition);
	void CopySentence(CBSActionForTool *pAction);
	char *GetParamCodeName(int iCodeIndex);
	char *GetParamCaption(int iCodeIndex);
	void ReplaceNewCon(CBSTriggerEventBase *pTrigger,int iSelectableIndex,int iConIndex);
	void ReplaceNewAct(CBSTriggerEventBase *pTrigger,int iSelectableIndex,int iActIndex);

	void InsertCondition(BST_TRGMODE mode,int iTriggerEventIndex, int Insert,CBSConditionForTool *pCondition);
	void InsertAction(BST_TRGMODE mode,int iTriggerEventIndex, int Insert,CBSActionForTool *pAction);
	void InsertTrigger(int iIndex,CBSTriggerForTool *pTrigger);
    void InsertVar(int iIndex,CBSVariable *pVar,int nIndexOrg = -1);
	void BackupCondition(CBSConditionForTool *pCondition);
	void BackupAction(CBSActionForTool *pAction);
	void RestoreCondition(CBSTriggerEventBase *pBase,int iConditionIndex);
	void RestoreAction(CBSTriggerEventBase *pBase, int iActionIndex);
	void NotifyTrgListPaste(int nIndex);
	void NotifyTrgListDelete(int nIndex);
	void GetParamList(int nTrgIdx, std::vector<CBSTriggerParam *> &vtTrgParam );


protected:
	void Destroy(); //Call by destructor
	void SetSeparateParameter(char *szSentence,CBSConditionActionForToolBase *pDest); //파라메터 타입 식별해내기
	bool IsInValCodeIndex(int iCodeIndex);
	void SetParamProperty(char *szParamName,CBSTriggerParam *pParam);
	bool FixDataName(char *pOldName,char *pNewName);
protected:
	std::vector<CBSTriggerForTool *>m_TriggerList; //실제 Condition과 Action은 이곳에만 존재
	std::vector<CBSEventForTool   *>m_EventList;   //트리거에 속해있지 않으므로	
	std::vector<CBSVariable       *>m_ValList;	   //Variable은 트리거에 속해있지 않으므로 이곳으로 빼냄
	

protected:
	//툴상에서 등록시에 필요할뿐이고 이곳에서 추가하는 Condition이나 Action은 추가가 이뤄지면 m_TriggerList등록함.
	//Selectable List;
	std::vector<CBSConditionForTool *> m_SelectableConditionList; //툴에서 등록 가능한 리스트들
	std::vector<CBSActionForTool	*> m_SelectableActionList;
	std::vector<CBSTriggerParamEx   *> m_ParamInfoList;
	CBSConditionForTool				*m_pBackupCondition;
	CBSActionForTool				*m_pBackupAction;
};


bool GetLine(FILE *fp,char *Buffer,int Size);
void GetElementByCharacter(char *szOut, const size_t szOut_len, char *szOrg,int ElementNum,const int OutstrMax,char Character);//어떤 식별문자를 통해 파라메터를 분리
void MakeCompleteSentence(char *szBuf,char *szOrg,int Sequence,char *szParam); //파라메터를 어떤 문장과 합쳐 완성시키는 함수
int  GetConditionCodeIndexByName(const char *szCodeName);
int  GetActionCodeIndexByName(const char *szCodeName);

void GetConditionCodeNameByIndex(int iIndex,char *pBuf, const size_t pBuf_len);  //aleksger - safe string
void GetActionCodeNameByIndex(int iIndex,char *pBuf, const size_t pBuf_len);  //aleksger - safe string
int  TrgStrlen(char *szStr);
void BSTriggerAssert(BOOL bIsTrue,char *szMsg);


#endif