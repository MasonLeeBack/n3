#include "stdafx.h"
#include "BSTriggerDefaultData.h"
#include "BSTriggerManager.h"
#include "BSTriggerCodeDefine.h"
#include "bstreamext.h"
#ifdef _USAGE_TOOL_
#include "BSMisc.h"
#endif


#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG



CBSTriggerParam::CBSTriggerParam()
{
	m_iDataType		 = BS_TG_TYPE_NONE;
	m_iCtrlType		 = BS_TG_CTRL_NONE;
	m_iCodeIndex	 = -1;
#ifdef _USAGE_TOOL_
	memset(m_szConvertText,0,MAX_BS_SHORT_STR);
#endif
}

CBSTriggerParam::~CBSTriggerParam()
{
	DeleteString();
}

#ifdef _USAGE_TOOL_
char *CBSTriggerParam::GetDataToText()
{
	switch(m_iDataType)
	{
	case BS_TG_TYPE_INT:
		sprintf(m_szConvertText,"%d",m_iInteger);
		return m_szConvertText;
	case BS_TG_TYPE_FLOAT:
		sprintf(m_szConvertText,"%f",m_fDecimal);
		return m_szConvertText;
	case BS_TG_TYPE_STRING:
		return m_Str;
		break;
	}
	BSTriggerAssert(FALSE,"Not defined data type");
	return NULL;
}

void CBSTriggerParam::SetDataToText(char *szData)
{
	strcpy(m_szConvertText, szData);
}
#endif


int CBSTriggerParam::GetInteger()
{
	if(BS_TG_TYPE_INT != m_iDataType){BSTriggerAssert(FALSE,"It might not be integeger type");}
	return m_iInteger;
}

float CBSTriggerParam::GetDecimal()
{
	if(BS_TG_TYPE_FLOAT != m_iDataType){BSTriggerAssert(FALSE,"It might not be decimal type");}
	return m_fDecimal;
}

char *CBSTriggerParam::GetString()
{
	if(BS_TG_TYPE_STRING != m_iDataType){BSTriggerAssert(FALSE,"It might not be string type");}
	return m_Str;
}

void CBSTriggerParam::SetData(int iData)
{
	if(BS_TG_TYPE_INT != m_iDataType){BSTriggerAssert(FALSE,"It might not be integeger type");}
	m_iInteger = iData;
}

void CBSTriggerParam::SetData(float fData)
{
	if(BS_TG_TYPE_FLOAT != m_iDataType){BSTriggerAssert(FALSE,"It might not be decimal type");}
	m_fDecimal = fData;
}

void CBSTriggerParam::SetData(char *szData)
{
	if(BS_TG_TYPE_STRING != m_iDataType){BSTriggerAssert(FALSE,"It might not be string type");}
	if(szData)
	{
		if(strlen(szData) > MAX_BS_SHORT_STR){
			BsAssert(0 && "입력된 문자열이 너무 김");
		}
		else
		{
			SAFE_DELETEA(m_Str);
			int iCount = strlen(szData);
			m_Str = new char[iCount+1];
			memcpy(m_Str,szData,iCount);
			m_Str[iCount] = 0;
		}
	}
}

void CBSTriggerParam::GetData(int &iData)
{
	if(BS_TG_TYPE_INT != m_iDataType){BSTriggerAssert(FALSE,"It might not be integeger type");}
	iData = m_iInteger;
}
void CBSTriggerParam::GetData(float &fData)
{
	if(BS_TG_TYPE_FLOAT != m_iDataType){BSTriggerAssert(FALSE,"It might not be decimal type");}
	fData = m_fDecimal;
}

bool CBSTriggerParam::IsString()
{
	if(m_iDataType == BS_TG_TYPE_STRING)return true;
	return false;
}

bool CBSTriggerParam::IsInteger()
{
	if(m_iDataType == BS_TG_TYPE_INT)return true;
	return false;
}

void CBSTriggerParam::GetData(char *szData, const size_t szData_len)
{
	if(BS_TG_TYPE_STRING != m_iDataType){BSTriggerAssert(FALSE,"It might not be string type");}
	if(m_Str)
		strcpy_s(szData, szData_len, m_Str); //aleksger - safe string
}


void CBSTriggerParam::SetDataType(int iType)
{
	switch(iType)
	{
	case BS_TG_TYPE_INT:
		m_iInteger = -1;
		break;
	case BS_TG_TYPE_FLOAT:
		m_fDecimal = -1.f;
		break;
	case BS_TG_TYPE_STRING:
		m_Str = NULL;
		break;
	}
	m_iDataType = iType;
}



int CBSTriggerParam::GetDataType()
{
	return m_iDataType;
}

int CBSTriggerParam::GetCtrlType()
{
	return m_iCtrlType;
}

void CBSTriggerParam::SetCtrlType(int iType)
{
	m_iCtrlType = iType;
}


void CBSTriggerParam::SaveData(BFileStream *fp,int nVer,bool bVariable)
{
	fp->Write(&m_iDataType,sizeof(int),ENDIAN_FOUR_BYTE);
	fp->Write(&m_iCtrlType,sizeof(int),ENDIAN_FOUR_BYTE);
	fp->Write(&m_iCodeIndex,sizeof(int),ENDIAN_FOUR_BYTE);
	switch(m_iDataType)
	{
	case BS_TG_TYPE_INT:		
		if(m_iInteger == -1){	
			if(bVariable == true){
				//BSTriggerAssert(FALSE , "Data was found -1");
			}
		}
		fp->Write(&m_iInteger,sizeof(int),ENDIAN_FOUR_BYTE);
		break;
	case BS_TG_TYPE_FLOAT:
		if(m_fDecimal == -1.f){		
			//BSTriggerAssert(FALSE , "Data was found -1.f");
		}
		fp->Write(&m_fDecimal,sizeof(float),ENDIAN_FOUR_BYTE);
		break;
	case BS_TG_TYPE_STRING:
		if(bVariable)
		{
			if(m_Str == NULL){			
				SetData("Desc : ");
			}
		}
		int StrLen = TrgStrlen(m_Str);
		if(m_Str == NULL){		
			BSTriggerAssert(FALSE , "Data was found EMPTY String");
		}
		else if(StrLen < 1){
			BSTriggerAssert(FALSE , "Data was found EMPTY String");
		}
		fp->Write(&StrLen,sizeof(int),ENDIAN_FOUR_BYTE);
		if(m_Str){
			fp->Write(m_Str,(int)strlen(m_Str));
		}		
	}

}

void CBSTriggerParam::GetDataTypeToString(char *szBuf, const size_t szBuf_len)
{
	//aleksger - safe string
	switch(m_iDataType)
	{
	case BS_TG_TYPE_INT:
		strcpy_s(szBuf, szBuf_len, "INT");
		return;
	case BS_TG_TYPE_FLOAT:
		strcpy_s(szBuf, szBuf_len, "FLOAT");
		return;
	case BS_TG_TYPE_STRING:
		strcpy_s(szBuf, szBuf_len, "STRING");
		return;
	}
	BsAssert(0 && "Data type Not define");
}

int CBSTriggerParam::GetTypeByString(char *szStr)
{
	if(strcmp(szStr,"INT") == 0)
		return BS_TG_TYPE_INT;
	else if(strcmp(szStr,"FLOAT") == 0)
		return BS_TG_TYPE_FLOAT;
	else if(strcmp(szStr,"STRING") == 0)
		return BS_TG_TYPE_STRING;

	BsAssert(0 && "Data type Not define");
	return -1;
}

void CBSTriggerParam::GetCompareStr(char *szBuf, const size_t szBuf_len)
{
	//aleksger - safe string - converted all strcpy to strcpy_s
	switch(m_iInteger)
	{
	case OPER_EQUAL:
		strcpy_s(szBuf,szBuf_len,"==");
		return;
	case OPER_BIGGER:
		strcpy_s(szBuf,szBuf_len,">");
		return;
	case OPER_SMALLER:
		strcpy_s(szBuf,szBuf_len,"<");
		return;
	case OPER_BIGGER_EQUAL:
		strcpy_s(szBuf,szBuf_len,">=");
		return;
	case OPER_SMALLER_EQUAL:
		strcpy_s(szBuf,szBuf_len,"<=");
		return;
	}
	BsAssert(0 && "Data type is not compare");
	return;
}

void CBSTriggerParam::GetCtrlTypeToString(char *szBuf, const size_t szBuf_len)
{
	switch(m_iCtrlType)
	{
	case BS_TG_CTRL_EDIT:
		strcpy_s(szBuf,szBuf_len,"EDIT_BOX"); //aleksger - safe string
		return;
	case BS_TG_CTRL_COMBOBOX:
		strcpy_s(szBuf,szBuf_len,"COMBO_BOX");//aleksger - safe string
		return;
	}
}

int CBSTriggerParam::GetCtrlTypeByStr(char *szBuf)
{
	if(strcmp(szBuf,"EDIT_BOX") == 0){
		return BS_TG_CTRL_EDIT;
	}
	else if(strcmp(szBuf,"COMBO_BOX") == 0){
		return BS_TG_CTRL_COMBOBOX;
	}
	BsAssert(0 && "It's not define ctrl type");
	return -1;
}

int CBSTriggerParam::GetCompareByStr(char *szBuf)
{
	if(strcmp(szBuf,"==") == 0)
		return OPER_EQUAL;
	else if(strcmp(szBuf,">") == 0)
		return OPER_BIGGER;
	else if(strcmp(szBuf,"<") == 0)
		return OPER_SMALLER;
	else if(strcmp(szBuf,">=") == 0)
		return OPER_BIGGER_EQUAL;
	else if(strcmp(szBuf,"<=") == 0)
		return OPER_SMALLER_EQUAL;
	BsAssert(0 && "Data type Not define");
	return -1;
}

void CBSTriggerParam::SaveDataToText(FILE *fp)
{
	char szTmp[32];	
	fprintf(fp,"%d ",m_iCodeIndex);
	GetCtrlTypeToString(szTmp, _countof(szTmp)); //컨트롤 타입
	fprintf(fp,"%s ",szTmp);
	GetDataTypeToString(szTmp, _countof(szTmp)); //테이터 타입
	fprintf(fp,"%s ",szTmp);

	switch(m_iDataType)
	{
	case BS_TG_TYPE_INT:
		{
			if(GetCodeIndex() == COMPARE_PARAM_CODE_INDEX)
			{
				GetCompareStr(szTmp, _countof(szTmp));
				fprintf(fp,"%s\n",szTmp);
			}
			else{
				fprintf(fp,"%d\n",m_iInteger);
			}			
		}
		break;
	case BS_TG_TYPE_FLOAT:
		fprintf(fp,"%f\n",m_fDecimal);
		break;
	case BS_TG_TYPE_STRING:
		fprintf(fp,"\"%s\"\n",m_Str);
		break;
	}
}


void CBSTriggerParam::LoadDataToText(TokenList::iterator &it)
{
	m_iCodeIndex = it->GetInteger();
	it++;
	m_iCtrlType  = GetCtrlTypeByStr((char *)it->GetVariable());
	it++;
	m_iDataType  = GetTypeByString((char *)it->GetVariable());
	it++;

	switch(m_iDataType)
	{
	case BS_TG_TYPE_INT:
		if(it->GetType() == Token::OPERATOR){
			m_iInteger = GetCompareByStr((char *)it->GetOperator());
		}
		else{
			m_iInteger = it->GetInteger();
		}		
		it++;
		break;
	case BS_TG_TYPE_FLOAT:
		m_fDecimal = it->GetReal();
		it++;
		break;
	case BS_TG_TYPE_STRING:
		m_Str = NULL;
		SetData((char *)it->GetString());
		it++;
	}
}



void CBSTriggerParam::LoadData(BStream *fp,int nVer)
{	
	fp->Read(&m_iDataType,sizeof(int),ENDIAN_FOUR_BYTE);
	fp->Read(&m_iCtrlType,sizeof(int),ENDIAN_FOUR_BYTE);
	fp->Read(&m_iCodeIndex,sizeof(int),ENDIAN_FOUR_BYTE);
	int iStrCount;
	switch(m_iDataType)
	{
	case BS_TG_TYPE_INT:
		fp->Read(&m_iInteger,sizeof(int),ENDIAN_FOUR_BYTE);
		if(m_iCodeIndex == 11)
		{
			m_iDataType = BS_TG_TYPE_STRING;
			m_Str = NULL;
		}
		break;
	case BS_TG_TYPE_FLOAT:
		fp->Read(&m_fDecimal,sizeof(float),ENDIAN_FOUR_BYTE);
		break;
	case BS_TG_TYPE_STRING:		
		fp->Read(&iStrCount,sizeof(int),ENDIAN_FOUR_BYTE);
		if(iStrCount)
		{
			m_Str = new char[iStrCount+1];
			fp->Read(m_Str,sizeof(char)*iStrCount);
			m_Str[iStrCount] = 0;
		}
		else
		{
			m_Str = NULL;
		}
		break;
	}
}

void CBSTriggerParam::DeleteString()
{
	if(m_iDataType == BS_TG_TYPE_STRING)
		SAFE_DELETEA(m_Str);
}


bool CBSTriggerParam::IsParamErrCheck()
{
	switch(m_iDataType)
	{
	case BS_TG_TYPE_INT:
		if(m_iInteger == -1)return false;
	case BS_TG_TYPE_FLOAT:
		if(m_fDecimal == -1.f)return false;
		break;
	case BS_TG_TYPE_STRING:
		if(m_Str == NULL)return false;
		break;
	}
	return true;
}

void memcpyTrigDefaultData(CBSTriggerParam *pDest,CBSTriggerParam *pSrc)
{	
	pDest->SetDataType(pSrc->GetDataType());
	pDest->SetCodeIndex(pSrc->GetCodeIndex());
	pDest->SetCtrlType(pSrc->GetCtrlType());

	if(pSrc->GetDataType() == BS_TG_TYPE_INT)
	{
		pDest->SetData(pSrc->GetInteger());
	}
	else if(pSrc->GetDataType() == BS_TG_TYPE_FLOAT)
	{
		pDest->SetData(pSrc->GetDecimal());
	}
	else if(pSrc->GetDataType() == BS_TG_TYPE_STRING)
	{
		pDest->SetData(pSrc->GetString());
	}
}


//----------------------------------------------------------------------------------------------------
CBSTriggerParamEx::CBSTriggerParamEx()
{
	ZeroMemory(m_szName,MAX_BS_TINY_STR);
	ZeroMemory(m_szCaption,MAX_BS_TINY_STR);

}

CBSTriggerParamEx::~CBSTriggerParamEx()
{
}

void CBSTriggerParamEx::SetName(const char *szName)
{
	BsAssert(strlen(szName) < MAX_BS_TINY_STR);
	strcpy(m_szName,szName);

}

void CBSTriggerParamEx::SetCaption(const char *szCaption)
{
	BsAssert(strlen(szCaption) < MAX_BS_SHORT_STR);
	strcpy(m_szCaption,szCaption);
}

