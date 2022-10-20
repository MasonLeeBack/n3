#include "stdafx.h"
#include "BSTriggerManager.h"
#include "BSTriggerCondition.h"
#include "BSTriggerDefaultData.h"
#include "bstreamext.h"

#ifdef _USAGE_TOOL_
#include "BSMisc.h"
#endif


#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG


CBSConditionActionBase::CBSConditionActionBase()
{
	m_iCodeIndex	= -1;
	m_iMyNumber		= -1;	//등록가능한 목록에서의 내 위치
	m_bEnable		= true;
	m_iORIndex		= -1;
	m_bSkip			= FALSE;
}

CBSConditionActionBase::~CBSConditionActionBase()
{
	ClearParam();
}


int CBSConditionActionBase::GetParamInt(int iIndex)
{
	BSTriggerAssert(iIndex < (int)m_ParamList.size(),"It's over to get param index");
	return m_ParamList[iIndex]->GetInteger();
}


char *CBSConditionActionBase::GetParamStr(int iIndex)
{
	BSTriggerAssert(iIndex < (int)m_ParamList.size(),"It's over to get param index");
	return m_ParamList[iIndex]->GetString();
}

float CBSConditionActionBase::GetParamFloat(int iIndex)
{
	BSTriggerAssert(iIndex < (int)m_ParamList.size(),"It's over to get param index");
	return m_ParamList[iIndex]->GetDecimal();
}

void CBSConditionActionBase::AddParameterData(CBSTriggerParam *pData)
{
	CBSTriggerParam *pDataTmp = new CBSTriggerParam;
	memcpyTrigDefaultData(pDataTmp,pData);
	m_ParamList.push_back(pDataTmp);
}

void CBSConditionActionBase::GetParamInt(int iIndex,int &iData)
{
	BSTriggerAssert(iIndex < (int)m_ParamList.size(),"It's over to get param index");
	m_ParamList[iIndex]->GetData(iData);

}

void CBSConditionActionBase::GetParamStr(int iIndex,char *szData, const size_t szData_len)
{
	BSTriggerAssert(iIndex < (int)m_ParamList.size(),"It's over to get param index");
	m_ParamList[iIndex]->GetData(szData, szData_len); //aleksger - safe string
}

void CBSConditionActionBase::GetParamFloat(int iIndex,float &fData)
{
	BSTriggerAssert(iIndex < (int)m_ParamList.size(),"It's over to get param index");
	m_ParamList[iIndex]->GetData(fData);
}

void CBSConditionActionBase::Save(BFileStream *fp,int nVer)
{
	fp->Write(&m_iCodeIndex,sizeof(int),ENDIAN_FOUR_BYTE);

	int iCount = m_ParamList.size();
	fp->Write(&iCount,sizeof(int),ENDIAN_FOUR_BYTE);
	for(int i = 0;i < (int)m_ParamList.size();i++){
		m_ParamList[i]->SaveData(fp,nVer);
	}
	fp->Write(&m_iMyNumber,sizeof(int),ENDIAN_FOUR_BYTE);
	fp->Write(&m_bSkip,sizeof(int),ENDIAN_FOUR_BYTE);
}

void CBSConditionActionBase::Load(BStream *fp,int nVer)
{
	fp->Read(&m_iCodeIndex,sizeof(int),ENDIAN_FOUR_BYTE);

	int iCount = 0;
	fp->Read(&iCount,sizeof(int),ENDIAN_FOUR_BYTE);	
	for(int i = 0;i < iCount;i++)
	{
		CBSTriggerParam *pDataTmp = new CBSTriggerParam;
		pDataTmp->LoadData(fp,nVer);
		m_ParamList.push_back(pDataTmp);
	}
	fp->Read(&m_iMyNumber,sizeof(int),ENDIAN_FOUR_BYTE);
	if(nVer >= 1100){ //새 버젼에서만 읽자
		fp->Read(&m_bSkip, sizeof(int), ENDIAN_FOUR_BYTE);
	}

}


bool CBSConditionActionBase::Compare( int iValue1, int iOper, int iValue2 )
{
	if(iValue2 == iValue1 && iOper == OPER_EQUAL)
		return true;
	else if(iValue2 < iValue1 && iOper == OPER_BIGGER)
		return true;
	else if(iValue2 > iValue1 && iOper == OPER_SMALLER)
		return true;
	else if(iValue2 <= iValue1 && iOper == OPER_BIGGER_EQUAL)
		return true;
	else if(iValue2 >= iValue1 && iOper == OPER_SMALLER_EQUAL)
		return true;

	return false;
}	



bool CBSConditionActionBase::IsInRect( float fX, float fY, float fSX, float fSY, float fEX, float fEY )
{
	return ( ( fX > fSX ) && ( fX < fEX ) && ( fY > fSY ) && ( fY < fEY ) );
}

void CBSConditionActionBase::ClearParam()
{
	for(int i = 0;i < (int)m_ParamList.size();i++)
	{
		delete m_ParamList[i];
	}
	m_ParamList.clear();
}


void CBSConditionActionBase::SetSkip()
{
	if(m_bSkip == FALSE){
		m_bSkip = TRUE;
	}
	else{
		m_bSkip = FALSE;
	}
}


//----------------------------------------------------------------------------------------------------
//CLASS : CBSConditionActionForToolBase
CBSConditionActionForToolBase::CBSConditionActionForToolBase()
{
	memset(m_Sentence,0,sizeof(MAX_BS_STR_SENTENCE));	
	memset(m_Caption,0,sizeof(MAX_BS_STR_SENTENCE));	
}

bool CBSConditionActionForToolBase::GetSentence(char *szBuf,int iMax)
{
	if((int)strlen(m_Sentence) < iMax)
	{
		strcpy_s(szBuf,iMax, m_Sentence); //aleksger - safe string
		return true;
	}
	return false;	
}

bool CBSConditionActionForToolBase::SetSentence(const char *szSentence)
{
	if(strlen(szSentence) < MAX_BS_STR_SENTENCE)
	{
		strcpy(m_Sentence,szSentence);	
		return true;
	}	
	return false;
}

#ifdef _USAGE_TOOL_
void CBSConditionActionForToolBase::GetCompleteSentence(char *szBuf,char *szOrg,CBSTriggerParam *pData,int iSequence,char *pReplceStr)
{
	if(pReplceStr)
	{
		MakeCompleteSentence(szBuf,szOrg,iSequence,pReplceStr);
	}
	else
	{
		MakeCompleteSentence(szBuf,szOrg,iSequence,pData->GetDataToText());
	}	
}
#endif


void CBSConditionActionForToolBase::FillToBuffer(char *pBuf,int iStartPos)
{
	int nIndex = iStartPos;
	memcpy(&pBuf[nIndex],&m_iCodeIndex,sizeof(int));
	nIndex += sizeof(int);
	memcpy(&pBuf[nIndex],m_Sentence,sizeof(char)*MAX_BS_STR_SENTENCE);
	nIndex += sizeof(char)*MAX_BS_STR_SENTENCE;
	memcpy(&pBuf[nIndex],m_Caption,sizeof(char)*MAX_BS_STR_SENTENCE);
	nIndex += sizeof(char)*MAX_BS_STR_SENTENCE;

	int iNum = GetParamCount();
	memcpy(&pBuf[nIndex],&iNum,sizeof(int));
	nIndex += sizeof(int);
	for(int i = 0;i < iNum;i++)
	{
		memcpyTrigDefaultData((CBSTriggerParam *)&pBuf[nIndex],m_ParamList[i]);
		nIndex += sizeof(CBSTriggerParam);
	}
	memcpy(&pBuf[nIndex],&m_iMyNumber,sizeof(int));
	nIndex += sizeof(int);
}

void CBSConditionActionForToolBase::FillFromBuffer(char *pBuf)
{
	int iIndex=0;
	memcpy(&m_iCodeIndex,&pBuf[iIndex],sizeof(int));
	iIndex += sizeof(int);
	memcpy(m_Sentence,&pBuf[iIndex],sizeof(char)*MAX_BS_STR_SENTENCE);
	iIndex += sizeof(char)*MAX_BS_STR_SENTENCE;
	memcpy(m_Caption,&pBuf[iIndex],sizeof(char)*MAX_BS_STR_SENTENCE);
	iIndex += sizeof(char)*MAX_BS_STR_SENTENCE;
	int iCount;
	memcpy(&iCount,&pBuf[iIndex],sizeof(int));
	iIndex += sizeof(int);
	for(int i = 0;i < iCount;i++)
	{
		CBSTriggerParam *DataTmp = new CBSTriggerParam;
		memcpyTrigDefaultData(DataTmp,(CBSTriggerParam *)&pBuf[iIndex]);
		AddParameterData(DataTmp);
		delete DataTmp;
		iIndex += sizeof(CBSTriggerParam);
	}
	memcpy(&m_iMyNumber,&pBuf[iIndex],sizeof(int));
	iIndex += sizeof(int);
}

int CBSConditionActionForToolBase::GetTotalSizeForClibBoard()
{
	int nSize = 0;
	nSize += sizeof(m_iCodeIndex);	
	nSize += sizeof(m_Sentence);
	nSize += sizeof(m_Caption);
	nSize += sizeof(int); //파라메터 갯수
	nSize += sizeof(CBSTriggerParam)*GetParamCount();
	nSize += sizeof(int);
	return nSize;
}


bool CBSConditionActionForToolBase::SetCaption(const char *szCaption)
{
	if((int)strlen(szCaption) < MAX_BS_STR_SENTENCE)
	{
		strcpy(m_Caption,szCaption);
		return true;
	}
	return false;	
}

bool CBSConditionActionForToolBase::FixDataName(char *pOldName,char *pNewName)
{
	bool bResult = false;
	for(unsigned int i = 0;i < m_ParamList.size();i++)
	{
		if(m_ParamList[i]->GetDataType() == BS_TG_TYPE_STRING)
		{
			if(m_ParamList[i]->GetString())
			{
				if(strcmp(m_ParamList[i]->GetString(),pOldName) == 0){
					m_ParamList[i]->SetData(pNewName);
					bResult = true;
				}				
			}
		}
	}
	return bResult;
}

void CBSConditionActionForToolBase::GetParamList(std::vector<CBSTriggerParam *> &ParamList)
{
	for(unsigned int i = 0;i < m_ParamList.size();i++)
	{
		ParamList.push_back( m_ParamList[i] );		
	}
	
}

//----------------------------------------------------------------------------------------------------

CBSConditionForTool::CBSConditionForTool()
{
	m_iORIndex   = -1;
}



void CBSConditionForTool::FillToCondition(char *pBuf)
{
	CBSConditionActionForToolBase::FillFromBuffer(pBuf);
}

void CBSConditionForTool::SaveCondition(BFileStream *fp,int nVer)
{
	CBSConditionActionBase::Save(fp,nVer);
	fp->Write(&m_iORIndex,sizeof(int),ENDIAN_FOUR_BYTE);

}

void CBSConditionForTool::SaveConditionToText(FILE *fp)
{
	char szCodeName[128];
	GetConditionCodeNameByIndex(GetCodeIndex(),szCodeName, _countof(szCodeName));  //aleksger - safe string
	fprintf(fp,"%s %d PARAM_COUNT %d %d %d\n",szCodeName,m_iCodeIndex,m_ParamList.size(),m_iMyNumber,m_iORIndex);

	fprintf(fp,"\"%s\"\n",m_Sentence);
	for(int i = 0;i < (int)m_ParamList.size();i++)
	{
		m_ParamList[i]->SaveDataToText(fp);
	}	
	fprintf(fp,"\n");
}


void CBSConditionForTool::LoadConditionToText(TokenList::iterator &it)
{
	it++;
	m_iCodeIndex = it->GetInteger();
	it++;
	it++;
	int iParamCount = it->GetInteger();
	it++;
	m_iMyNumber = it->GetInteger();
	it++;
	m_iORIndex  = it->GetInteger();
	it++;
	it++;

	for(int i = 0;i < iParamCount;i++)
	{
		CBSTriggerParam *pData = new CBSTriggerParam;
		pData->LoadDataToText(it);
		m_ParamList.push_back(pData);
	}
}

void CBSConditionForTool::LoadCondition(BStream *fp,int nVer)
{
	CBSConditionActionBase::Load(fp,nVer);
	fp->Read(&m_iORIndex,sizeof(int),ENDIAN_FOUR_BYTE);
}



void memcpyCondition(CBSConditionForTool *pDest,CBSConditionForTool *pSrc)
{
	pDest->SetOperIndex(pSrc->GetOperIndex());
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

