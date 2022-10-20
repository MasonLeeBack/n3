#include "stdafx.h"
#include "BsUiFontAttrManager.h"
#include "bstreamext.h"
#include <algorithm>

#include "BsUiDefine.h"
#include "tpGrammer.h"

#include "BsUiFont.h"
#include "BsKernel.h"
#include "LocalLanguage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG


BsUiFontAttrManager	g_FontAttrMgr;

struct TFn_CompareID
{
	bool operator() ( BsUiFontAttr& A, BsUiFontAttr& B)
	{
		return A.nAttrId <= B.nAttrId;
	}
};


BsUiFontAttrManager::BsUiFontAttrManager()
{
	m_nCurIndex = -1;
	m_FontAttrInfoList = NULL;
}


BsUiFontAttrManager::~BsUiFontAttrManager()
{	
	Release();
}

void BsUiFontAttrManager::Release()
{
	if(m_FontAttrInfoList == NULL){
		return;
	}

	for (int i=0; i<MAX_LANGAUGES; i++ )
	{
		BsUiFontAttrLIST* pFontAttrInfo = &(m_FontAttrInfoList[i]);
		
		for (DWORD j=0; j<pFontAttrInfo->size(); j++ )
		{
			delete[] (*pFontAttrInfo)[j].szName;
			delete[] (*pFontAttrInfo)[j].szAttr;
			delete (*pFontAttrInfo)[j].psAttr;
		}
		pFontAttrInfo->clear();
	}

	delete[] m_FontAttrInfoList;
	m_FontAttrInfoList = NULL;

}


bool BsUiFontAttrManager::Load()
{
	Release();

	m_FontAttrInfoList = new BsUiFontAttrLIST[MAX_LANGAUGES];

	g_BsKernel.chdir("FontAttr");

	bool bFlag = false;
	for(int i=0; i<MAX_LANGAUGES; i++)
	{
		GAME_LANGUAGE nLocal = (GAME_LANGUAGE)i;

#ifndef _BSUI_TOOL
		if(g_LocalLanguage.GetLanguage() != nLocal){
			continue;
		}
#endif

		if(g_LocalLanguage.GetLocalEnable(nLocal) == false){
			continue;
		}

		char szBsUiFontAttr[256];
		sprintf(szBsUiFontAttr, _MENU_FONT_ATTR_FILE,
			g_LocalLanguage.GetLanguageDir(nLocal),
			g_LocalLanguage.GetLanguageStr(nLocal));

		bFlag = Load( g_BsKernel.GetFullName(szBsUiFontAttr), &(m_FontAttrInfoList[i]));
		BsAssert(bFlag == true);
	}

	g_BsKernel.chdir("..");

	return bFlag;
}


bool BsUiFontAttrManager::Load(char* szFullName, BsUiFontAttrLIST* pFontAttrList)
{
	tpGrammer grammer;

	grammer.Add(BsUiCMD_BEGIN_FONTATTR_ID, "ID=%d,NAME=%s,EDIT=%d\nATTR=%s");
	grammer.Add(BsUiCMD_BEGIN_FONTATTR_ID_OLD, "ID=%d,NAME=%s\nATTR=%s");

	grammer.Create(szFullName, ".");

	int ret = 0;
	
	do {
		ret = grammer.Get();

		switch(ret)
		{
		case BsUiCMD_BEGIN_FONTATTR_ID:
			{
				BsUiFontAttr FontAttrInfo;
				FontAttrInfo.nAttrId = atoi(grammer.GetParam(0));

				int len = (int)strlen((char*)grammer.GetParam(1));
				FontAttrInfo.szName = new char[len+1];
				strcpy_s(FontAttrInfo.szName, len+1, (char*)grammer.GetParam(1)); //aleksger - safe string

				FontAttrInfo.bEnableEdit = atoi(grammer.GetParam(2)) ? true : false;

				len = (int)strlen((char*)grammer.GetParam(3));
				FontAttrInfo.szAttr = new char[len+1];
				strcpy_s(FontAttrInfo.szAttr, len +1, (char*)grammer.GetParam(3));//aleksger - safe string

				FontAttrInfo.psAttr = new SFontAttribute();
				g_pFont->GetFontAttrStruct(FontAttrInfo.psAttr, FontAttrInfo.szAttr);

				pFontAttrList->push_back(FontAttrInfo);

				break;
			}
		case BsUiCMD_BEGIN_FONTATTR_ID_OLD:
			{
				BsUiFontAttr FontAttrInfo;
				FontAttrInfo.nAttrId = atoi(grammer.GetParam(0));

				int len = (int)strlen((char*)grammer.GetParam(1));
				FontAttrInfo.szName = new char[len+1];
				strcpy_s(FontAttrInfo.szName, len+1, (char*)grammer.GetParam(1));//aleksger - safe string

				len = (int)strlen((char*)grammer.GetParam(2));
				FontAttrInfo.szAttr = new char[len+1];
				strcpy_s(FontAttrInfo.szAttr, len+1, (char*)grammer.GetParam(2));//aleksger - safe string

				FontAttrInfo.psAttr = new SFontAttribute();
				g_pFont->GetFontAttrStruct(FontAttrInfo.psAttr, FontAttrInfo.szAttr);

				pFontAttrList->push_back(FontAttrInfo);

				break;
			}
		case -1:
			{
				CBsConsole::GetInstance().AddFormatString("%s File Not Found", szFullName);
				return false;
			}
			break;
		}
	} while (ret != 0);

	std::sort(pFontAttrList->begin(),pFontAttrList->end(), TFn_CompareID() );

	return true;
}

bool BsUiFontAttrManager::Save()
{
	g_BsKernel.chdir("FontAttr");

	for(int i=0; i<MAX_LANGAUGES; i++)
	{
		GAME_LANGUAGE nLocal = (GAME_LANGUAGE)i;
		char szBsUiFontAttr[256];
		sprintf(szBsUiFontAttr, _MENU_FONT_ATTR_FILE,
			g_LocalLanguage.GetLanguageDir(nLocal),
			g_LocalLanguage.GetLanguageStr(nLocal));

		char *szFullName = g_BsKernel.GetFullName(szBsUiFontAttr);

		if(g_LocalLanguage.GetLocalEnable(nLocal) == false){
			continue;
		}

		SaveforFontAttr(szFullName, &(m_FontAttrInfoList[i]));
	}

	if(g_LocalLanguage.GetLanguage() == LANGUAGE_ENG){
		SaveforFontDef(&(m_FontAttrInfoList[g_LocalLanguage.GetLanguage()]));
	}

	g_BsKernel.chdir("..");

	return true;
}

bool BsUiFontAttrManager::SaveforFontAttr(char* szFullName, BsUiFontAttrLIST* pFontAttrList)
{	
	FILE *fp = fopen(szFullName, "wt");
	if(fp==NULL)
	{
		CBsConsole::GetInstance().AddFormatString("%s File Not Found", szFullName);
		BsAssert(0);
		return false;
	}

	std::sort(pFontAttrList->begin(), pFontAttrList->end(), TFn_CompareID() );

	char szTag[256], szTag1[256];
	for(int i=0; i<(int)pFontAttrList->size(); i++)
	{
		memset(szTag, 0, 256);

		BsUiFontAttr FontAttrInfo = (*pFontAttrList)[i];
		sprintf(szTag, "ID = %d, Name = \"%s\", EDIT = %d\n",
			FontAttrInfo.nAttrId,
			FontAttrInfo.szName,
			(int)FontAttrInfo.bEnableEdit);
		sprintf(szTag1, "Attr = \"%s\"\n\n", FontAttrInfo.szAttr);
		
		fwrite(&szTag, sizeof(char)*strlen(szTag), 1, fp);
		fwrite(&szTag1, sizeof(char)*strlen(szTag1), 1, fp);
	}

	fclose(fp);
	

	return true;
}


bool BsUiFontAttrManager::SaveforFontDef(BsUiFontAttrLIST* pFontAttrList)
{
	char szBsUiFontAttrID[256];
	strcpy(szBsUiFontAttrID, _MENU_FONT_ATTR_ID_FILE);

	char *pFullName = g_BsKernel.GetFullName(szBsUiFontAttrID);
	FILE *fp = fopen(pFullName, "wt");
	if(fp==NULL)
	{
		CBsConsole::GetInstance().AddFormatString("%s File Not Found", pFullName);
		BsAssert(0);
		return false;
	}

	char szTag[256], szSpace[64];
	int ndeflen = (int)strlen("#define ");
	for(int i=0; i<(int)pFontAttrList->size(); i++)
	{
		BsUiFontAttr FontAttrInfo = (*pFontAttrList)[i];
		int nNamelen = (int)strlen(FontAttrInfo.szName);

		memset(szSpace, 0, 64);
		for(int i = ndeflen+nNamelen; i<64; i++)
		{
			strcat(szSpace, " ");
		}

		memset(szTag, 0, 256);
		sprintf(szTag, "#define %s%s%d\n",
			FontAttrInfo.szName,
			szSpace,
			FontAttrInfo.nAttrId);

		fwrite(&szTag, sizeof(char)*strlen(szTag), 1, fp);
	}

	fclose(fp);

	return true;
}


BsUiAttrId BsUiFontAttrManager::AddInfo(const char* szName, char* szAttr)
{
	if(IsSameAttr(szAttr) == true)
		return -1;

	BsUiAttrId nAttrId = GenerateNewIndex();

	for(int i=0; i<MAX_LANGAUGES; i++)
	{
		GAME_LANGUAGE nLocal = (GAME_LANGUAGE)i;
		if(g_LocalLanguage.GetLocalEnable(nLocal) == true)
		{
			BsUiFontAttr FontAttrInfo;
			FontAttrInfo.nAttrId = nAttrId;

			int nNameSize = (int)strlen(szName);
			FontAttrInfo.szName = new char[nNameSize+1];
			strcpy_s(FontAttrInfo.szName, nNameSize+1, szName); //aleksger - safe string

			int nAttrSize = (int)strlen(szAttr);
			FontAttrInfo.szAttr = new char[nAttrSize+1];
			strcpy_s(FontAttrInfo.szAttr, nAttrSize+1, szAttr); //aleksger - safe string

			FontAttrInfo.psAttr = new SFontAttribute();
			g_pFont->GetFontAttrStruct(FontAttrInfo.psAttr, FontAttrInfo.szAttr);

			BsUiFontAttrLIST* pFontAttrInfo = &(m_FontAttrInfoList[i]);
			pFontAttrInfo->push_back(FontAttrInfo);
		}
	}

	return nAttrId;
}


bool BsUiFontAttrManager::RemoveInfo(BsUiAttrId nAttrId)
{	
	bool bFlag = false;
	for(int i=0; i<MAX_LANGAUGES; i++)
	{
		GAME_LANGUAGE nLocal = (GAME_LANGUAGE)i;
		if(g_LocalLanguage.GetLocalEnable(nLocal) == false){
			continue;
		}

		bFlag = false;

		BsUiFontAttrLIST* pFontAttrInfo = &(m_FontAttrInfoList[i]);
		for(DWORD j=0; j<pFontAttrInfo->size(); j++)
		{
			if((*pFontAttrInfo)[j].nAttrId == nAttrId)
			{
				delete[] (*pFontAttrInfo)[j].szName;
				delete[] (*pFontAttrInfo)[j].szAttr;
				delete (*pFontAttrInfo)[j].psAttr;
				pFontAttrInfo->erase(pFontAttrInfo->begin()+j);
				bFlag = true;
				break;
			}
		}
		BsAssert(bFlag == true);
	}

	return bFlag;
}


void BsUiFontAttrManager::SetName(BsUiAttrId nAttrId, char* szName)
{
	BsAssert(szName != NULL);
	
	for(int i=0; i<MAX_LANGAUGES; i++)
	{
		GAME_LANGUAGE nLocal = (GAME_LANGUAGE)i;
		if(g_LocalLanguage.GetLocalEnable(nLocal) == false){
			continue;
		}

		bool bFlag = false;

		BsUiFontAttrLIST* pFontAttrInfo = &(m_FontAttrInfoList[i]);
		for(DWORD j=0; j<pFontAttrInfo->size(); j++)
		{
			if((*pFontAttrInfo)[j].nAttrId != nAttrId){
				continue;
			}

			delete[] (*pFontAttrInfo)[j].szName;

			int nNameSize = (int)strlen(szName);
			(*pFontAttrInfo)[j].szName = new char[nNameSize+1];
			strcpy_s((*pFontAttrInfo)[j].szName, nNameSize+1, szName); //aleksger - safe string
			bFlag = true;
			break;
		}
		BsAssert(bFlag == true);
	}

	return;
}

void BsUiFontAttrManager::SetAttr(BsUiAttrId nAttrId, char* szAttr)
{
	BsAssert(szAttr != NULL);
	
	for(int i=0; i<MAX_LANGAUGES; i++)
	{
		GAME_LANGUAGE nLocal = (GAME_LANGUAGE)i;
		if(g_LocalLanguage.GetLocalEnable(nLocal) == false){
			continue;
		}

		if(g_LocalLanguage.GetLanguage() != LANGUAGE_ENG &&
			g_LocalLanguage.GetLanguage() != nLocal){
			continue;
		}

		BsUiFontAttrLIST* pFontAttrInfo = &(m_FontAttrInfoList[i]);
		bool bFlag = false;
		for(DWORD j=0; j<pFontAttrInfo->size(); j++)
		{
			if((*pFontAttrInfo)[j].nAttrId != nAttrId){
				continue;
			}

			if((*pFontAttrInfo)[j].bEnableEdit == false)
			{
				bFlag = true;
				break;
			}
			
			delete[] (*pFontAttrInfo)[j].szAttr;

			int nAttrSize = (int)strlen(szAttr);
			(*pFontAttrInfo)[j].szAttr = new char[nAttrSize+1];
			strcpy_s((*pFontAttrInfo)[j].szAttr, nAttrSize+1, szAttr); //aleksger - safe string

			g_pFont->GetFontAttrStruct((*pFontAttrInfo)[j].psAttr, szAttr);
			strcpy_s((*pFontAttrInfo)[j].szAttr, nAttrSize+1, szAttr);

			bFlag = true;
			break;
		}
		BsAssert(bFlag == true);
	}
	
	return;
}

void BsUiFontAttrManager::SetAttr(BsUiAttrId nAttrId, SFontAttribute* psAttr)
{
	BsAssert(psAttr != NULL);

	for(int i=0; i<MAX_LANGAUGES; i++)
	{
		GAME_LANGUAGE nLocal = (GAME_LANGUAGE)i;
		if(g_LocalLanguage.GetLocalEnable(nLocal) == false){
			continue;
		}

		if(g_LocalLanguage.GetLanguage() != LANGUAGE_ENG &&
			g_LocalLanguage.GetLanguage() != nLocal){
			continue;
		}

		BsUiFontAttrLIST* pFontAttrInfo = &(m_FontAttrInfoList[i]);
		bool bFlag = false;

		for(DWORD j=0; j<pFontAttrInfo->size(); j++)
		{
			if((*pFontAttrInfo)[j].nAttrId != nAttrId){
				continue;
			}

			if((*pFontAttrInfo)[j].bEnableEdit == false)
			{	
				bFlag = true;
				break;
			}

			delete[] (*pFontAttrInfo)[j].szAttr;
			(*pFontAttrInfo)[j].szAttr = new char[256];
			g_pFont->GetFontAttrText((*pFontAttrInfo)[j].szAttr, 256,  psAttr);
			g_pFont->GetFontAttrStruct((*pFontAttrInfo)[j].psAttr, (*pFontAttrInfo)[j].szAttr);

			bFlag = true;
			break;
		}

		BsAssert(bFlag == true);
	}

	
	return;
}


int BsUiFontAttrManager::GetInfoCount()
{
	BsUiFontAttrLIST* pFontAttrInfo = &(m_FontAttrInfoList[g_LocalLanguage.GetLanguage()]);
	return (int)pFontAttrInfo->size();
}


BsUiAttrId BsUiFontAttrManager::GetAttrId(char* szName)
{
	BsUiFontAttrLIST* pFontAttrInfo = &(m_FontAttrInfoList[g_LocalLanguage.GetLanguage()]);
	for(DWORD i=0; i<pFontAttrInfo->size(); i++)
	{
		if(	strcmp((*pFontAttrInfo)[i].szName, szName) == NULL)
		{	
			return (*pFontAttrInfo)[i].nAttrId;
		}
	}

	if(strcmp(szName, _TEXT_DEFAULT_ATTR_NAME) != 0)
	{
		BsAssert(0);
	}

	return -1;
}


char* BsUiFontAttrManager::GetName(BsUiAttrId nAttrId)
{
	if(nAttrId == -1 || IsSameIndex(nAttrId) == false){
		return _TEXT_DEFAULT_ATTR_NAME;
	}

	BsUiFontAttrLIST* pFontAttrInfo = &(m_FontAttrInfoList[g_LocalLanguage.GetLanguage()]);
	for(DWORD i=0; i<pFontAttrInfo->size(); i++)
	{
		if((*pFontAttrInfo)[i].nAttrId == nAttrId){
			return (*pFontAttrInfo)[i].szName;
		}
	}

	BsAssert(0);
	return NULL;
}


char* BsUiFontAttrManager::GetAttrText(BsUiAttrId nAttrId)
{
	if(nAttrId == -1 || IsSameIndex(nAttrId) == false){
		return _TEXT_DEFAULT_ATTR;
	}

	BsUiFontAttrLIST* pFontAttrInfo = &(m_FontAttrInfoList[g_LocalLanguage.GetLanguage()]);
	for(DWORD i=0; i<pFontAttrInfo->size(); i++)
	{
		if((*pFontAttrInfo)[i].nAttrId == nAttrId){
			return (*pFontAttrInfo)[i].szAttr;
		}
	}

	BsAssert(0);
	return NULL;
}


BsUiFontAttr* BsUiFontAttrManager::GetAttrStruct(BsUiAttrId nAttrId)
{
	if(nAttrId == -1 || IsSameIndex(nAttrId) == false){
		return NULL;
	}

	BsUiFontAttrLIST* pFontAttrInfo = &(m_FontAttrInfoList[g_LocalLanguage.GetLanguage()]);
	for(DWORD i=0; i<pFontAttrInfo->size(); i++)
	{
		if((*pFontAttrInfo)[i].nAttrId == nAttrId){
			return &(*pFontAttrInfo)[i];
		}
	}

	BsAssert(0);
	return NULL;
}
		
BsUiAttrId BsUiFontAttrManager::GetAttrIdforIndex(int nIndex)
{ 
	BsAssert(GetInfoCount()>nIndex);
	BsUiFontAttrLIST* pFontAttrInfo = &(m_FontAttrInfoList[g_LocalLanguage.GetLanguage()]);
	return (*pFontAttrInfo)[nIndex].nAttrId;
}


char* BsUiFontAttrManager::GetNameforIndex(int nIndex)
{ 
	BsAssert(GetInfoCount()>nIndex);
	BsUiFontAttrLIST* pFontAttrInfo = &(m_FontAttrInfoList[g_LocalLanguage.GetLanguage()]);
	return (*pFontAttrInfo)[nIndex].szName;
}


char* BsUiFontAttrManager::GetAttrforIndex(int nIndex)
{ 
	BsAssert(GetInfoCount()>nIndex);
	BsUiFontAttrLIST* pFontAttrInfo = &(m_FontAttrInfoList[g_LocalLanguage.GetLanguage()]);
	return (*pFontAttrInfo)[nIndex].szAttr;
}
	

bool BsUiFontAttrManager::GetEnableEdit(int nIndex)
{
	BsAssert(GetInfoCount()>nIndex);
	BsUiFontAttrLIST* pFontAttrInfo = &(m_FontAttrInfoList[g_LocalLanguage.GetLanguage()]);
	return (*pFontAttrInfo)[nIndex].bEnableEdit;
}


void BsUiFontAttrManager::AddInfo(BsUiAttrId nAttrId, char* szName, char* szAttr)
{	
	BsUiFontAttr FontAttrInfo;
	FontAttrInfo.nAttrId = nAttrId;

	int nNameSize = (int)strlen(szName);
	FontAttrInfo.szName = new char[nNameSize+1];
	strcpy_s(FontAttrInfo.szName, nNameSize+1, szName);//aleksger - safe string

	int nAttrSize = (int)strlen(szAttr);
	FontAttrInfo.szAttr = new char[nAttrSize+1];
	strcpy_s(FontAttrInfo.szAttr, nAttrSize+1, szAttr); //aleksger - safe string

	FontAttrInfo.psAttr = new SFontAttribute();
	g_pFont->GetFontAttrStruct(FontAttrInfo.psAttr, FontAttrInfo.szAttr);

	for(int i=0; i<MAX_LANGAUGES; i++)
	{
		GAME_LANGUAGE nLocal = (GAME_LANGUAGE)i;
		if(g_LocalLanguage.GetLocalEnable(nLocal) == true)
		{
			BsUiFontAttrLIST* pFontAttrInfo = &(m_FontAttrInfoList[i]);
			pFontAttrInfo->push_back(FontAttrInfo);
		}
	}
}


void BsUiFontAttrManager::GenerateNewName( char* szName, const size_t szName_len )
{
	char szStr[256];
	strcpy(szStr, szName);
	int nCount = 1;

	while(1)
	{
		if( IsSameName( szStr ) == false )
			break;

		sprintf(szStr, "%s_%d", szName, nCount );
		nCount++;
	}
	strcpy_s(szName, szName_len, szStr); //aleksger - safe string
}


bool BsUiFontAttrManager::IsSameName( char* szName )
{
	BsUiFontAttrLIST* pFontAttrInfo = &(m_FontAttrInfoList[g_LocalLanguage.GetLanguage()]);
	for(DWORD i=0; i<pFontAttrInfo->size(); i++)
	{	
		if(	strcmp((*pFontAttrInfo)[i].szName, szName) == NULL){	
			return true;
		}
	}

	return false;
}


bool BsUiFontAttrManager::IsSameAttr( char* szAttr )
{
#ifndef _BSUI_TOOL
	if(	strcmp(szAttr, _TEXT_DEFAULT_ATTR) == NULL){
		return false;
	}
#endif

	BsUiFontAttrLIST* pFontAttrInfo = &(m_FontAttrInfoList[g_LocalLanguage.GetLanguage()]);
	for(DWORD i=0; i<pFontAttrInfo->size(); i++)
	{
		if(	strcmp((*pFontAttrInfo)[i].szAttr, szAttr) == NULL){	
			return true;
		}
	}

	return false;
}


bool BsUiFontAttrManager::IsSameAttr( BsUiAttrId nAttrId, char* szAttr )
{
	BsUiFontAttrLIST* pFontAttrInfo = &(m_FontAttrInfoList[g_LocalLanguage.GetLanguage()]);
	for(DWORD i=0; i<pFontAttrInfo->size(); i++)
	{	
		if((*pFontAttrInfo)[i].nAttrId == nAttrId){
			continue;
		}

		if(	strcmp((*pFontAttrInfo)[i].szAttr, szAttr) == NULL){	
			return true;
		}
	}

	return false;
}

BsUiAttrId BsUiFontAttrManager::GenerateNewIndex()
{	
	BsUiAttrId nAttrId = 0;
	while(1) {
		
		if( IsSameIndex( nAttrId ) == false )
			return nAttrId;
		++nAttrId;
	}

	BsAssert(0);
	return nAttrId;
}

bool BsUiFontAttrManager::IsSameIndex(BsUiAttrId nAttrId)
{
	BsUiFontAttrLIST* pFontAttrInfo = &(m_FontAttrInfoList[g_LocalLanguage.GetLanguage()]);
	for(DWORD i=0; i<pFontAttrInfo->size(); i++)
	{
		if(	(*pFontAttrInfo)[i].nAttrId == nAttrId ){	
			return true;
		}
	}

	return false;
}

bool BsUiFontAttrManager::IsSameAttrforStr(BsUiFontAttrLIST* pFontAttrInfo, BsUiFontAttr* pstrFontAttr)
{	
	for(DWORD i=0; i<pFontAttrInfo->size(); i++)
	{
		if((*pFontAttrInfo)[i].nAttrId != pstrFontAttr->nAttrId){
			continue;
		}

		if(	strcmp((*pFontAttrInfo)[i].szName, pstrFontAttr->szName) == NULL){
			return true;
		}
	}

	return false;
}