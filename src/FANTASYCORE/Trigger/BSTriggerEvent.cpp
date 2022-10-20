#include "stdafx.h"
#include "BSTriggerEvent.h"
#include "bstreamext.h"
#ifdef _USAGE_TOOL_
#include "BSMisc.h"
#endif //_USAGE_TOOL_
#include "BSTriggerManager.h"
#include "BSTriggerCodeDefine.h"
#include "BsCommon.h"

extern BSEventCodeInfo g_EventCodeTable[BTEC_LAST];


#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG

CBSEventForTool::CBSEventForTool()
{
	m_iEventHandle = -1;
	m_iReservedData1 = -1;
}


void CBSEventForTool::SaveEvent(BFileStream *fp,int nVer)
{	
	int iCount = -1;
	m_iReservedData1 = -1;
	fp->Write(&m_iReservedData1,sizeof(int),ENDIAN_FOUR_BYTE);

	//fp->Write(m_Reserve,sizeof(char)*MAX_EVENT_CAPTION);
	
	fp->Write(&m_iEventHandle,sizeof(int),ENDIAN_FOUR_BYTE);

	iCount = (int)m_ConditionList.size();
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

void CBSEventForTool::SaveEventToText(FILE *fp)
{
	fprintf(fp,"%s\t\"%s\"\n",BSKW_EVENT_NAME,GetCaption());
	fprintf(fp,"%s\t%d\n",BSKW_EVENT_HANDLE,m_iEventHandle);

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

void CBSEventForTool::LoadEventToText(TokenList::iterator &itr)
{
	if( strcmp(itr->GetKeyword(),BSKW_EVENT_NAME) == 0){
		itr++; //이름은 받을 필요 없어서(Handle가지고 참조하는 방식이니까)
		itr++;
	}
	if( strcmp(itr->GetKeyword(),BSKW_EVENT_HANDLE) == 0)
	{
		itr++;
		m_iEventHandle = itr->GetInteger();
		itr++;
	}

	int iCount = 0;
	if( strcmp(itr->GetKeyword(),BSKW_CONDITION_COUNT) == 0)
	{
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
	if( strcmp(itr->GetKeyword(),BSKW_ACTION_COUNT) == 0)
	{
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



char *CBSEventForTool::GetCaption()
{
	return g_EventCodeTable[m_iEventHandle].szCaption;
}

void CBSEventForTool::LoadEvent(BStream *fp,int nVer)
{
	SAFE_DELETE_PVEC(m_ConditionList);
	SAFE_DELETE_PVEC(m_ActionList);
	
	fp->Read(&m_iReservedData1,sizeof(int),ENDIAN_FOUR_BYTE);
	if(m_iReservedData1 == 0) //지워야 할 코드
	{
		char Reserve[MAX_EVENT_CAPTION];
		fp->Read(Reserve,sizeof(char)*MAX_EVENT_CAPTION-sizeof(int));
	}
	fp->Read(&m_iEventHandle,sizeof(int),ENDIAN_FOUR_BYTE);

	int iCount = -1;
	int i;
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