#include "stdafx.h"
#include "BSTrigger.h"
#include "bstreamext.h"
#include "BSTriggerManager.h"
#include "BSTriggerCondition.h"
#include "BSTriggerAction.h"
#ifdef _USAGE_TOOL_
#include "BSMisc.h"
#endif //_USAGE_TOOL_
#include "BsCommon.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG


CBSTriggerEventBase::CBSTriggerEventBase()
{
}


CBSTriggerEventBase::~CBSTriggerEventBase()
{
	int i;
	for( i = 0; i < (int)m_ConditionList.size(); i++ ){
		delete m_ConditionList[i];
	}
	m_ConditionList.clear();

	for(i = 0; i < (int)m_ActionList.size(); i++){
		delete m_ActionList[i];
	}
	m_ActionList.clear();
}


void CBSTriggerEventBase::AddCondition(CBSConditionForTool *pCondition)
{
	CBSConditionForTool *pConditionSrc = new CBSConditionForTool;	
	memcpyCondition(pConditionSrc,pCondition);
	m_ConditionList.push_back(pConditionSrc);
}


void CBSTriggerEventBase::AddAction(CBSActionForTool *Data)
{
	CBSActionForTool *pAction = new CBSActionForTool;
	memcpyAction(pAction,Data);
	m_ActionList.push_back(pAction);
}

void CBSTriggerEventBase::DeleteCondition(int iConditionIndex)
{
	delete m_ConditionList[iConditionIndex];
	m_ConditionList.erase(m_ConditionList.begin()+iConditionIndex);
}

void CBSTriggerEventBase::DeleteAction(int iActionIndex)
{
	delete m_ActionList[iActionIndex];
	m_ActionList[iActionIndex] = NULL;
	m_ActionList.erase(m_ActionList.begin()+iActionIndex);
}

void CBSTriggerEventBase::ReplaceCondition(int Srcindex,int DestIndex)
{
	CBSConditionForTool *pSrcData = m_ConditionList[Srcindex];
	m_ConditionList[Srcindex] = m_ConditionList[DestIndex];
	m_ConditionList[DestIndex] = pSrcData;
}

void CBSTriggerEventBase::ReplaceAction(int Srcindex,int DestIndex)
{
	CBSActionForTool *pSrcData = m_ActionList[Srcindex];
	m_ActionList[Srcindex] = m_ActionList[DestIndex];
	m_ActionList[DestIndex] = pSrcData;
}


void CBSTriggerEventBase::InsertCondition(int InsertNum,CBSConditionForTool *Data)
{
	CBSConditionForTool *pCon = new CBSConditionForTool;
	memcpyCondition(pCon,Data);
	m_ConditionList.insert(m_ConditionList.begin() + InsertNum,pCon);
	for(int i = 0; i < (int)m_ConditionList.size();i++)
	{
		if(InsertNum < i)
		{
			if(m_ConditionList[i]->GetOperIndex() != -1)
			{
				m_ConditionList[i]->SetOperIndex(m_ConditionList[i]->GetOperIndex()+1);
			}
		}
	}
}

void CBSTriggerEventBase::InsertAction(int InsertNum,CBSActionForTool *Data)
{
	CBSActionForTool *pAction = new CBSActionForTool;
	memcpyAction(pAction,Data);

	m_ActionList.insert(m_ActionList.begin() + InsertNum,pAction);
}

bool CBSTriggerEventBase::FixDataName(char *pOldName,char *pNewName)
{
	bool bResult=0; //aleksger: prefix bug 809: unitialized variable.
	unsigned int i = 0;
	for(i = 0;i < m_ConditionList.size();i++){
		bResult = m_ConditionList[i]->FixDataName(pOldName, pNewName);
	}
	for(i = 0;i < m_ActionList.size();i++){
		bResult |= m_ActionList[i]->FixDataName(pOldName, pNewName);
	}

	return bResult;
}

//-----------------------------------------------------------------------------------------------------

CBSTriggerForTool::CBSTriggerForTool()
{
}

CBSTriggerForTool::~CBSTriggerForTool()
{
}

void CBSTriggerForTool::SaveTrigger(BFileStream *fp,int nVer)
{	
	fp->Write(m_Name,sizeof(char)*MAX_BS_TRIGGER_NAME);

	int iCount = (int)m_ConditionList.size();
	fp->Write(&iCount,sizeof(int),ENDIAN_FOUR_BYTE);
	for(int i = 0;i < (int)m_ConditionList.size();i++)
	{
		m_ConditionList[i]->SaveCondition(fp,nVer);
	}

	iCount = (int)m_ActionList.size();
	fp->Write(&iCount,sizeof(int),ENDIAN_FOUR_BYTE);
	for(int i = 0;i < (int)m_ActionList.size();i++)
	{
		m_ActionList[i]->SaveAction(fp,nVer);
	}
}


void CBSTriggerForTool::SaveTriggerToText(FILE *fp)
{
	fprintf(fp,"%s\t\"%s\"\n",BSKW_TRIGGER_NAME,m_Name);
	
	fprintf(fp,"%s\t%d\n",BSKW_CONDITION_COUNT,m_ConditionList.size());
	for(int i = 0;i < (int)m_ConditionList.size();i++)
	{
		m_ConditionList[i]->SaveConditionToText(fp);
	}

	fprintf(fp,"%s\t%d\n",BSKW_ACTION_COUNT,m_ActionList.size());
	for(int i = 0;i < (int)m_ActionList.size();i++)
	{
		m_ActionList[i]->SaveActionToText(fp);
	}
}


void CBSTriggerForTool::LoadTriggerToText(TokenList::iterator &itr)
{
	if( strcmp(itr->GetKeyword(),BSKW_TRIGGER_NAME) == 0)
	{
		itr++;
		strcpy(m_Name,itr->GetString());
		itr++;
	}
	int iCount = 0;
	if( strcmp(itr->GetKeyword(),BSKW_CONDITION_COUNT) == 0){
		itr++;
		iCount = itr->GetInteger();
		itr++;
		for(int i = 0;i < iCount;i++)
		{
			CBSConditionForTool *pCondition = new CBSConditionForTool;
			pCondition->LoadConditionToText(itr);
			m_ConditionList.push_back(pCondition);
		}
	}
	if( strcmp(itr->GetKeyword(),BSKW_ACTION_COUNT) == 0){
		itr++;
		iCount = itr->GetInteger();
		itr++;
		for(int i = 0;i < iCount;i++)
		{
			CBSActionForTool *pAction = new CBSActionForTool;
			pAction->LoadActionToText(itr);
			m_ActionList.push_back(pAction);
		}
	}

	
}

void CBSTriggerForTool::LoadTrigger(BStream *fp,int nVer)
{
	SAFE_DELETE_PVEC(m_ConditionList);
	SAFE_DELETE_PVEC(m_ActionList);
	fp->Read(m_Name,sizeof(char)*MAX_BS_TRIGGER_NAME);	


	int iCount = 0;
	int i = 0;

	fp->Read(&iCount,sizeof(int),ENDIAN_FOUR_BYTE);	
	for(i = 0;i < iCount;i++)
	{
		CBSConditionForTool *pConTmp = new CBSConditionForTool;
		pConTmp->LoadCondition(fp,nVer);
		m_ConditionList.push_back(pConTmp);
	}

	fp->Read(&iCount,sizeof(int),ENDIAN_FOUR_BYTE);
	for(i = 0;i < iCount;i++)
	{
		CBSActionForTool *pActTmp = new CBSActionForTool;
		pActTmp->LoadAction(fp,nVer);
		m_ActionList.push_back(pActTmp);
	}

}

bool CBSTriggerForTool::SetName(char *szName)
{
	if(strlen(szName) > MAX_BS_TRIGGER_NAME)
	{		
		return false;
	}
	strcpy(m_Name,szName);
	return true;
}

BOOL CBSTriggerForTool::IsEnable()
{
	int i = 0;
	if(GetActionCount() == 0 && GetConditionCount() == 0){	
		return TRUE;
	}

	for(i = 0;i < GetConditionCount();i++)
	{
		if( GetCondition(i)->IsSkip() == false )
		{
			return TRUE;
		}
	}
	for(i = 0;i < GetActionCount();i++)
	{
		if( GetAction(i)->IsSkip() == false )
		{
			return TRUE;
		}
	}
	return FALSE;
}

void CBSTriggerForTool::SetEnable( BOOL bEnable )
{
	int i = 0;
	for(i = 0;i < GetConditionCount();i++)
	{
		GetCondition(i)->SetSkip(bEnable);
		
	}
	for(i = 0;i < GetActionCount();i++)
	{
		GetAction(i)->SetSkip(bEnable);
	}
}

void CBSTriggerForTool::GetConditionList(std::vector<CBSConditionForTool *> &ConditionList)
{
	for(unsigned int i = 0;i < m_ConditionList.size();i++)
	{
		ConditionList.push_back(m_ConditionList[i]);
	}
}

void CBSTriggerForTool::GetActionList(std::vector<CBSActionForTool	*> &ActionList	)
{
	for(unsigned int i = 0;i < m_ActionList.size();i++)
	{
		ActionList.push_back(m_ActionList[i]);
	}
}
void ClearTrg(CBSTriggerForTool *pBuf)
{
	for(int i = 0;i < pBuf->GetActionCount();i++){	
		pBuf->DeleteAction(i);
		i--;
	}

	for(int i = 0;i < pBuf->GetConditionCount();i++)
	{
		pBuf->DeleteCondition(i);
		i--;
	}
}


void memcpyTrigger(CBSTriggerForTool *pBuf,CBSTriggerForTool *pSrc)
{
	pBuf->SetName(pSrc->GetName());
	int i = 0;
	for(i = 0;i < pSrc->GetConditionCount();i++)
	{
		CBSConditionForTool *pCondition = new CBSConditionForTool;
		memcpyCondition(pCondition,pSrc->GetCondition(i));
		pBuf->AddCondition(pCondition);
	}
	for(i = 0;i < pSrc->GetActionCount();i++)
	{
		CBSActionForTool *pAction = new CBSActionForTool;
		memcpyAction(pAction,pSrc->GetAction(i));
		pBuf->AddAction(pAction);
	}
}
