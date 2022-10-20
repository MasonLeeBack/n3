#include "stdafx.h"
#include "BsUiClassIDManager.h"
#include "bstreamext.h"
#include <algorithm>
#include "BsConsole.h"

#include "BsUiDefine.h"
#include "tpGrammer.h"

#include "BsKernel.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG


BsUiClassIDManager	g_ClassIdMgr;

struct TFn_CompareID
{
	bool operator() ( BsUiClassIDManager::_CLASSID_INFO& A, BsUiClassIDManager::_CLASSID_INFO& B)
	{
		return A.hClass <= B.hClass;
	}
};


BsUiClassIDManager::BsUiClassIDManager()
{

}


BsUiClassIDManager::~BsUiClassIDManager()
{
	Release();
}

void BsUiClassIDManager::Release()
{
	DWORD dwCount = m_pClassInfo.size();
	for(DWORD i=0; i<dwCount; i++){
		delete[] m_pClassInfo[i].szName;
	}

	m_pClassInfo.clear();
}

bool BsUiClassIDManager::Load()
{
	Release();

	g_BsKernel.chdir("Menu");
	
	tpGrammer grammer;

	grammer.Add(BsUiCMD_BEGIN_CLASS_ID, "ID=%d,TYPE=%d,Name=%s");
	grammer.Create( g_BsKernel.GetFullName(_MENU_CLASSID_FILE), ".");

	int ret = 0;
	do {
		ret = grammer.Get();

		switch(ret)
		{
		case BsUiCMD_BEGIN_CLASS_ID:
			{
				_CLASSID_INFO ClassInfo;
				ClassInfo.hClass = atoi(grammer.GetParam(0));

				ClassInfo.nType = (BsUiTYPE)(atoi(grammer.GetParam(1)));

				int len = (int)strlen((char*)grammer.GetParam(2));
				ClassInfo.szName = new char[len+1];
				strcpy_s (ClassInfo.szName, len+1, (char*)grammer.GetParam(2)); //aleksger - safe string

				m_pClassInfo.push_back(ClassInfo);

				break;
			}
		case -1:
			{
				CBsConsole::GetInstance().AddFormatString("%s File Not Found", _MENU_CLASSID_FILE);
				g_BsKernel.chdir("..");
				return false;
			}
			break;
		}
	} while (ret != 0);


	std::sort(m_pClassInfo.begin(),m_pClassInfo.end(), TFn_CompareID() );

	g_BsKernel.chdir("..");
	
	return true;
}

bool BsUiClassIDManager::Save()
{	
	g_BsKernel.chdir("Menu");
	char *pFullName = g_BsKernel.GetFullName(_MENU_CLASSID_FILE);
	FILE *fp = fopen(pFullName, "wt");

	if(fp==NULL)
	{
		CBsConsole::GetInstance().AddFormatString("%s File Not Found", pFullName);
		g_BsKernel.chdir("..");

		BsAssert(0);
		return false;
	}

	char szTag[256];
	_CLASSID_INFO ClassInfo;
	DWORD dwCount = m_pClassInfo.size();
	for(DWORD i=0; i<dwCount; i++)
	{
		ClassInfo = m_pClassInfo[i];

		memset(szTag, 0, 256);
		sprintf(szTag, "ID = %d, TYPE = %d, Name = \"%s\"\n", ClassInfo.hClass, ClassInfo.nType, ClassInfo.szName);
		fwrite(&szTag, sizeof(char)*strlen(szTag), 1, fp);
	}

	fclose(fp);
	
	pFullName = g_BsKernel.GetFullName(_MENU_CONTROLID_FILE);
	fp = fopen(pFullName, "wt");
	if(fp==NULL)
	{
		CBsConsole::GetInstance().AddFormatString("%s File Not Found", pFullName);
		BsAssert(0);
		g_BsKernel.chdir("..");
		return false;
	}

	char szSpace[64];
	int ndeflen = (int)strlen("#define ");
	dwCount = m_pClassInfo.size();
	for(DWORD i=0; i<dwCount; i++)
	{
		ClassInfo = m_pClassInfo[i];
		int nNamelen = (int)strlen(ClassInfo.szName);
		
		memset(szSpace, 0, 64);
		for(int j = ndeflen+nNamelen; j<64; j++) //aleksger: prefix bug 681: multiple variable declaration.
		{
			strcat(szSpace, " ");
		}
		
		memset(szTag, 0, 256);
		sprintf(szTag, "#define %s%s0x%x\n",
			ClassInfo.szName,
			szSpace,
			ClassInfo.hClass);

		fwrite(&szTag, sizeof(char)*strlen(szTag), 1, fp);
	}

	fclose(fp);

	g_BsKernel.chdir("..");

	return true;
}

BsUiCLASS BsUiClassIDManager::GetClass(int nIndex)
{
	BsAssert((int)m_pClassInfo.size() > nIndex);

	return m_pClassInfo[nIndex].hClass;
}

BsUiTYPE BsUiClassIDManager::GetType(BsUiCLASS hClass)
{
	DWORD dwCount = m_pClassInfo.size();
	for(DWORD i=0; i<dwCount; i++)
	{
		if(m_pClassInfo[i].hClass == hClass)
			return m_pClassInfo[i].nType;
	}

	BsAssert(0);
	return BsUiTYPE_NONE;
}


char* BsUiClassIDManager::GetName(BsUiCLASS hClass)
{
	DWORD dwCount = m_pClassInfo.size();
	for(DWORD i=0; i<dwCount; i++)
	{
		if(m_pClassInfo[i].hClass == hClass)
			return m_pClassInfo[i].szName;
	}

	BsAssert(0);
	return NULL;
}


void BsUiClassIDManager::GenerateNewName( char* szName, const size_t szName_len )
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


bool BsUiClassIDManager::IsSameName( char* szName )
{
	DWORD dwCount = m_pClassInfo.size();
	for(DWORD i=0; i<dwCount; i++)
	{
		if(	strcmp(m_pClassInfo[i].szName, szName) == NULL)
		{	
			return true;
		}
	}

	return false;
}

void BsUiClassIDManager::GenerateNewClassID( BsUiCLASS& hClass, BsUiTYPE nType)
{	
	while(1) {
		if(nType == BsUiTYPE_LAYER)
		{
			hClass += 1<<20;
		}
		else
		{
			hClass ++;
		}

		if( IsSameClassID( hClass ) == false )
			return;
	}

	BsAssert(0);
}


bool BsUiClassIDManager::IsSameClassID( const BsUiCLASS hClass )
{
	DWORD dwCount = m_pClassInfo.size();
	for(DWORD i=0; i<dwCount; i++)
	{
		if(	m_pClassInfo[i].hClass == hClass )
		{	
			return true;
		}
	}

	return false;
}


BsUiCLASS BsUiClassIDManager::AddInfo(BsUiTYPE nType, const char* szName, BsUiCLASS hParentClass)
{
	BsUiCLASS hClass = 0;
	if(nType != BsUiTYPE_LAYER)
	{
		hClass = hParentClass | nType<<12;
	}

	GenerateNewClassID(hClass, nType);

	_CLASSID_INFO ClassInfo;
	ClassInfo.hClass = hClass;
	ClassInfo.nType = nType;
	ClassInfo.nNameSize = (int)strlen(szName);
	ClassInfo.szName = new char[ClassInfo.nNameSize+1];
	strcpy_s(ClassInfo.szName, ClassInfo.nNameSize+1, szName);  //aleksger - safe string

	m_pClassInfo.push_back(ClassInfo);

	std::sort(m_pClassInfo.begin(),m_pClassInfo.end(), TFn_CompareID() );

	return ClassInfo.hClass;
}


bool BsUiClassIDManager::AddInfo(BsUiCLASS hClass, BsUiTYPE nType, const char* szName)
{
	_CLASSID_INFO ClassInfo;
	ClassInfo.hClass = hClass;
	ClassInfo.nType = nType;
	ClassInfo.nNameSize = (int)strlen(szName);
	ClassInfo.szName = new char[ClassInfo.nNameSize+1];
	strcpy_s(ClassInfo.szName, ClassInfo.nNameSize+1, szName); //aleksger - safe string

	m_pClassInfo.push_back(ClassInfo);

	std::sort(m_pClassInfo.begin(),m_pClassInfo.end(), TFn_CompareID() );

	return true;
}


bool BsUiClassIDManager::RemoveInfo(BsUiCLASS hClass)
{
	DWORD dwCount = m_pClassInfo.size();
	for(DWORD i=0; i<dwCount; i++)
	{
		if(m_pClassInfo[i].hClass == hClass)
		{
			delete[] m_pClassInfo[i].szName;
			m_pClassInfo.erase(m_pClassInfo.begin()+i);
			return true;
		}
	}
	
	return false;
}


void BsUiClassIDManager::SetName(BsUiCLASS hClass, char* szName)
{
	BsAssert(szName != NULL);

	DWORD dwCount = m_pClassInfo.size();
	for(DWORD i=0; i<dwCount; i++)
	{
		if(m_pClassInfo[i].hClass == hClass)
		{
			delete[] m_pClassInfo[i].szName;
			m_pClassInfo[i].nNameSize = (int)strlen(szName);
			m_pClassInfo[i].szName = new char[m_pClassInfo[i].nNameSize+1];
			strcpy_s(m_pClassInfo[i].szName, m_pClassInfo[i].nNameSize+1, szName);  //aleksger - safe string
			return;
		}
	}

	BsAssert(0);
	return;
}
