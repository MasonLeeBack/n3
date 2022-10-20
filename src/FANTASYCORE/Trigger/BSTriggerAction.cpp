#include "stdafx.h"
#include "BSTriggerManager.h"
#include "BSTriggerAction.h"
#include "BSTriggerDefaultData.h"
#include "bstreamext.h"
#ifdef _USAGE_TOOL_
#include "BSMisc.h"
#endif //_USAGE_TOOL_


#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG



int CBSActionForTool::FillToBuffer(char *pBuf,int iStartPos) //리턴 값 : 버퍼로 채워지고 난 후의 최종 위치
{
	CBSConditionActionForToolBase::FillToBuffer(pBuf,iStartPos);	
	return -1;
}

void CBSActionForTool::FillToAction(char *pBuf)
{
	CBSConditionActionForToolBase::FillFromBuffer(pBuf);
}


void CBSActionForTool::LoadAction(BStream *fp,int nVer)
{
	CBSConditionActionBase::Load(fp,nVer);

}

void CBSActionForTool::SaveAction(BFileStream *fp,int nVer)
{
	CBSConditionActionBase::Save(fp,nVer);

}

void CBSActionForTool::SaveActionToText(FILE *fp)
{
	char szCodeName[128];
	GetActionCodeNameByIndex(GetCodeIndex(),szCodeName, _countof(szCodeName));  //aleksger - safe string
	fprintf(fp,"%s %d PARAM_COUNT %d %d\n",szCodeName,m_iCodeIndex,m_ParamList.size(),m_iMyNumber);

	fprintf(fp,"\"%s\"\n",m_Sentence);
	for(int i = 0;i < (int)m_ParamList.size();i++)
	{
		m_ParamList[i]->SaveDataToText(fp);
	}	
	fprintf(fp,"\n");

}

void CBSActionForTool::LoadActionToText(TokenList::iterator &itr)
{
	itr++;
	m_iCodeIndex = itr->GetInteger();
	itr++;
	itr++;
	int iParamCount = itr->GetInteger();
	itr++;
	m_iMyNumber = itr->GetInteger();
	itr++;
	itr++;

	for(int i = 0;i < iParamCount;i++)
	{
		CBSTriggerParam *pData = new CBSTriggerParam;
		pData->LoadDataToText(itr);
		m_ParamList.push_back(pData);
	}
}

void memcpyAction(CBSActionForTool *pDest,CBSActionForTool *pSrc)
{
	pDest->SetSentence(pSrc->GetSentence());
	pDest->SetCodeIndex(pSrc->GetCodeIndex());
	pDest->SetMyNumber(pSrc->GetMyNumber());
	pDest->ClearParam();
	pDest->SetSkip( pSrc->IsSkip() );

	for(int i = 0;i < pSrc->GetParamCount();i++)
	{
		pDest->AddParameterData(pSrc->GetParamData(i));
	}
}