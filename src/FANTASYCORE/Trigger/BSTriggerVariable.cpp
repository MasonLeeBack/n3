#include "stdafx.h"
#include "BSTriggerVariable.h"
#include "bstreamext.h"
#include "BSTriggerDefaultData.h"
#include "BSTriggerCodeDefine.h"
#ifdef _USAGE_TOOL_
#include "BSMisc.h"
#endif

extern BSParamCaptionInfo g_ParamCaptionTable[];

#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG

CBSVariable::CBSVariable()
{

}

CBSVariable::~CBSVariable()
{
	for(int i = 0;i < (int)m_ParamList.size();i++)
	{
		delete m_ParamList[i];
	}
	m_ParamList.clear();
}

void CBSVariable::CreateDefaultParameter()
{
	//변수 이름
	CBSTriggerParam *pData = new CBSTriggerParam;
	pData->SetDataType(BS_TG_TYPE_STRING);
	pData->SetCtrlType(BS_TG_CTRL_EDIT);
	pData->SetCodeIndex(0);
	m_ParamList.push_back(pData);

	//변수 타입
	pData = new CBSTriggerParam;
	pData->SetDataType(BS_TG_TYPE_INT);
	pData->SetCtrlType(BS_TG_CTRL_COMBOBOX);
	pData->SetCodeIndex(1);
	m_ParamList.push_back(pData);

    //변수 값
	pData = new CBSTriggerParam;
	pData->SetDataType(BS_TG_TYPE_INT);
	pData->SetCtrlType(BS_TG_CTRL_EDIT);
	pData->SetCodeIndex(2);
	m_ParamList.push_back(pData);

	//변수 설명
	pData = new CBSTriggerParam;
	pData->SetDataType(BS_TG_TYPE_STRING);
	pData->SetCtrlType(BS_TG_CTRL_EDIT);
	pData->SetData("Desc : ");
	pData->SetCodeIndex(3);
	m_ParamList.push_back(pData);
}

void CBSVariable::SaveVariable(BFileStream *fp,int nVer)
{
	int iCount = (int)m_ParamList.size();
	fp->Write(&iCount,sizeof(int),ENDIAN_FOUR_BYTE);
	for(int i = 0;i < (int)m_ParamList.size();i++)
	{
		m_ParamList[i]->SaveData(fp,nVer,true);
	}
}

void CBSVariable::SaveVariableToText(FILE *fp)
{
	fprintf(fp,"\"%s\" ",m_ParamList[0]->GetString());
	fprintf(fp,"%d ",m_ParamList[1]->GetInteger());
	fprintf(fp,"\"%s\" ",m_ParamList[3]->GetString());

	char szDataType[64];
	m_ParamList[2]->GetDataTypeToString(szDataType, _countof(szDataType));
	fprintf(fp,"%s ",szDataType);

	switch(m_ParamList[2]->GetDataType())
	{
	case BS_TG_TYPE_INT:
		fprintf(fp,"%d",m_ParamList[2]->GetInteger());
		break;
	case BS_TG_TYPE_FLOAT:
		fprintf(fp,"%f",m_ParamList[2]->GetDecimal());
		break;
	case BS_TG_TYPE_STRING:
		fprintf(fp,"\"%s\"",m_ParamList[2]->GetString());
		break;
	}
	fprintf(fp,"\n");
	
	
}



void CBSVariable::LoadVariableToText(TokenList::iterator &itr)
{
	CreateDefaultParameter();
	m_ParamList[0]->SetData((char *)itr->GetString());itr++;	
	m_ParamList[1]->SetData(itr->GetInteger());itr++;
	m_ParamList[3]->SetData((char *)itr->GetString());itr++;

	int iDataType = m_ParamList[2]->GetTypeByString((char *)itr->GetVariable());itr++;
	switch(iDataType)
	{
	case BS_TG_TYPE_INT:
		m_ParamList[2]->SetData(itr->GetInteger());itr++;
		break;
	case BS_TG_TYPE_FLOAT:
		m_ParamList[2]->SetData(itr->GetReal());itr++;
		break;
	case BS_TG_TYPE_STRING:
		m_ParamList[2]->SetData((char *)itr->GetString());itr++;
		break;
	}	
}

void CBSVariable::LoadVariable(BStream *fp,int nVer)
{
	int iCount = 0;
	fp->Read(&iCount,sizeof(int),ENDIAN_FOUR_BYTE);		
	CBSTriggerParam *pDataTmp;
	for(int i = 0;i < (int)iCount;i++)
	{
		pDataTmp = new CBSTriggerParam;
		pDataTmp->LoadData(fp,nVer);
		m_ParamList.push_back(pDataTmp);
	}
}

void CBSVariable::FillToBuffer(char *pBuf,int nIndex)
{
	int iParamNum = (int)m_ParamList.size();
	memcpy(&pBuf[nIndex],&iParamNum,sizeof(int));
	nIndex += sizeof(int);
	for(int i = 0;i < (int)m_ParamList.size();i++)
	{
		memcpyTrigDefaultData((CBSTriggerParam *)&pBuf[nIndex], m_ParamList[i]);
		nIndex += sizeof(CBSTriggerParam);
	}
}

void CBSVariable::FillToVariable(char *pBuf)
{
	int iSize;
    int nIndex = 0;
	memcpy(&iSize,&pBuf[nIndex],sizeof(int));
	nIndex += sizeof(int);
	for(int i = 0;i < iSize;i++)
	{
		CBSTriggerParam *pDataTmp = new CBSTriggerParam;
		memcpyTrigDefaultData(pDataTmp,(CBSTriggerParam *)&pBuf[nIndex]);		
		nIndex += sizeof(CBSTriggerParam);
        m_ParamList.push_back(pDataTmp);
	}
}

int CBSVariable::GetTotalVariableSize()
{    
	int iSize = sizeof(CBSTriggerParam)*(int)m_ParamList.size();
    iSize += (int)m_ParamList.size();
	return iSize;
}

#ifdef _USAGE_TOOL_
char *CBSVariable::GetSentence()
{
	return m_ParamList[0]->GetDataToText(); //첫번째 인자가 이름이기 때문에
}
#endif
void CBSVariable::AddParameter(CBSTriggerParam *pParam)
{
	CBSTriggerParam *pParamTmp = new CBSTriggerParam;
	memcpyTrigDefaultData(pParamTmp,pParam);
	m_ParamList.push_back(pParamTmp);
}

void memcpyVar(CBSVariable *pDest,CBSVariable *pSource)
{
    for(int i = 0;i < pSource->GetParamCount();i++)
    {
        pDest->AddParameter( pSource->GetParamData(i) );
    }
}
