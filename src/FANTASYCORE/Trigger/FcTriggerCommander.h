#ifndef __FC_TRIGGER_COMMANDER_H__
#define __FC_TRIGGER_COMMANDER_H__
#include "BSTriggerCondition.h"
#include "BSTriggerAction.h"
#include "InputPad.h"


enum FCTRG_COMMAND_STATE{
	//FCS_ACTION_IS_NOT_END,

};

class CBSTriggerManager;
class CFcTriggerEventBase;
class CBSTriggerEventBase;
class CBSVariable;
enum BS_EVENT_CODE;

enum
{
	ACTION_STATE_FINISH,
	ACTION_STATE_WORKING,
	ACTION_STATE_RESET,
};


class CFcTriggerEventBase
{
public:
	CFcTriggerEventBase();
	virtual ~CFcTriggerEventBase();
	void Reset();

	void AllCopyFromToolData(CBSTriggerEventBase *pTrigger);
	CBSConditionActionBase *AllocateConditionByType(int iCodeIndex);
	CBSConditionActionBase *AllocateActionByType(int iCodeIndex);
	CBSConditionActionBase *GetCondition(int iIndex){return m_ConditionList[iIndex];}
	int GetConditionCount(){return m_ConditionList.size();}
	int GetActionCount(){return m_ActionList.size();}
	CBSConditionActionBase *GetAction(int iIndex){return m_ActionList[iIndex];}
	void SetEnable(bool bEnable){m_bEnable = bEnable;}
	bool IsEnable(){return m_bEnable;}
	void SetLoop(bool bLoop){m_bLoop = bLoop;}
	bool IsLoop(){return m_bLoop;}
	void SetConSucd(bool bSucd){m_bConSucd = bSucd;}
	bool IsConSucd(){return m_bConSucd;}

protected:	
	std::vector<CBSConditionActionBase *>m_ConditionList;
	std::vector<CBSConditionActionBase *>m_ActionList;
	bool	m_bEnable;
	bool	m_bLoop;
	bool	m_bConSucd;

};


//-----------------------------------------------------------------------------------------------------
class CFcTriggerCommander
{
public:
	CFcTriggerCommander( int nCheckTick = 40 );
	~CFcTriggerCommander();
	void Load(char *FileName);
	void Process();
	void HookOnProcess();
	void DoEvent(BS_EVENT_CODE EventType);

	int GetVariableCount();
	CBSVariable *GetVariable( int iIndex );
	int  GetVariableType(int iIndex);
	char *GetVariableValStr(int iIndex);
	int  GetVariableValInteger(int iIndex);
	float GetVariableValFloat(int iIndex);

	void SetVariableVal(int iIndex, int  nValue);
	void SetVariableVal(int iIndex, float fValue);
	void SetVariableVal(int iIndex, char *pStr);

	void ResetTrigger(int iIndex);
	void ResetAll();
	void EnableTrigger(int iIndex,bool bEnable);
	void SetLoopCurTrg(bool bLoop);
	void ReSetKeyHookOn();
	short  GetKeyHookOn(int nKey);
	void SetStopTrigger(bool bStop){ m_bStop = bStop; }
	
protected:
	std::vector<CFcTriggerEventBase *>m_TriggerList;
	std::vector<CFcTriggerEventBase *>m_EventList;
	std::vector<CBSVariable		*>m_ValList;
	int m_nCheckTick;

	int m_nCurTriggerIndex;
	short  m_nKeyTypeForHookOn[PAD_INPUT_RSTICK+1];
	bool m_bStop;
protected:
	bool ConditionCheck(CFcTriggerEventBase *pTrigger);
};

#endif