#ifndef __BS_TRIGGER_DEFAULT_DATA_H__
#define __BS_TRIGGER_DEFAULT_DATA_H__
#include "BSTriggerStrDefinition.h"
#include "Token.h"

class BStream;
class CFile;
class BFileStream;

class CBSTriggerParam
{
public:
	CBSTriggerParam();
	virtual ~CBSTriggerParam();
	void SetData(int iData);
	void SetData(float fData);
	void SetData(char *szData);		
	void GetData(int &iData);
	void GetData(float &fData);
	void GetData(char *szData, const size_t szData_len); //aleksger - safe string
#ifdef _USAGE_TOOL_
	char *GetDataToText();	
	void SetDataToText(char *szData);
#endif
	int  GetDataType();
	int  GetCtrlType();
	void SetCtrlType(int iType);
	void SetDataType(int iType);
	int GetCodeIndex(){return m_iCodeIndex;}
	void SetCodeIndex(int iIndex){m_iCodeIndex = iIndex;}

	/////////////////////////////////////////// 추가 함수
	int GetInteger();
	char *GetString();
	float GetDecimal();
	void SaveData(BFileStream *fp,int nVer,bool bVariable = false);
	void SaveDataToText(FILE *fp);
	void LoadData(BStream *fp,int nVer);
	void LoadDataToText(TokenList::iterator &it);
	void DeleteString();
	void GetDataTypeToString(char *szBuf, const size_t szBuf_len);  //aleksger - safe string
	int GetTypeByString(char *szStr);
	void GetCompareStr(char *szBuf, const size_t szBuf_len);//aleksger - safe string
	int GetCompareByStr(char *szBuf);
	void GetCtrlTypeToString(char *szBuf, const size_t szBuf_len);  //aleksger - safe string
	int GetCtrlTypeByStr(char *szBuf);
	bool IsString();
	bool IsInteger();

	bool IsParamErrCheck();

protected:
	union
	{
		int   m_iInteger;
		float m_fDecimal;
		char  *m_Str;
	};
	int   m_iDataType;
	int	  m_iCtrlType;
	int	  m_iCodeIndex;
#ifdef _USAGE_TOOL_
	char m_szConvertText[MAX_BS_SHORT_STR];
#endif
	
};

class CBSTriggerParamEx : public CBSTriggerParam //외부에서 파라메터 처리를 위해서
{
public:
	CBSTriggerParamEx();
	virtual ~CBSTriggerParamEx();
	void SetName(const char *szName);
	void SetCaption(const char *szCaption);
	char *GetName(){return m_szName;}
	char *GetCaption(){return m_szCaption;}
protected:
	char m_szCaption[MAX_BS_SHORT_STR];
	char m_szName[MAX_BS_TINY_STR];

};

void memcpyTrigDefaultData(CBSTriggerParam *pDest,CBSTriggerParam *pSrc);


#ifndef SAFE_DELETEA
#define SAFE_DELETEA(p) if(p) { delete []p; p = NULL; }
#endif //SAFE_DELETEA

#endif