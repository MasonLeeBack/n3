#include "stdafx.h"
#include "BsUiUVManager.h"
#include "BsKernel.h"
#include "BsUtil.h"
#include "BsUiDefine.h"
#include "tpGrammer.h"

#include "LocalLanguage.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG


BsUiUVManager		g_UVMgr;
BsUiEditUVManager	g_EditUVMgr;

void ArrayRect(RECT& dscrect, RECT srcRect)
{
	dscrect.left = (srcRect.left <= srcRect.right) ? srcRect.left : srcRect.right;
	dscrect.top = (srcRect.top <= srcRect.bottom) ? srcRect.top : srcRect.bottom;

	dscrect.right = (srcRect.left < srcRect.right) ? srcRect.right : srcRect.left;
	dscrect.bottom = (srcRect.top < srcRect.bottom) ? srcRect.bottom : srcRect.top;
}

struct TFn_CompareID
{
	bool operator() ( BsUiEditUVManager::TextureUVList* A, BsUiEditUVManager::TextureUVList* B)
	{
		return A->nTexUnit <= B->nTexUnit;
	}
};

BsUiEditUVManager::BsUiEditUVManager()
{
	m_nCurUVId = -1;
	m_nCurTextureId = -1;
	m_bSelectedBox = false;
	m_rectSelected.left = m_rectSelected.right = 0;
	m_rectSelected.top = m_rectSelected.bottom = 0;
	m_EditUVState = _EditUV_SEL;
	m_bShowBlock = true;
	m_bShowID = true;
}

BsUiEditUVManager::~BsUiEditUVManager()
{
	Release();
}

void BsUiEditUVManager::Release()
{
	DWORD dwCount = m_UiTexUVList.size();
	for(DWORD i=0; i<dwCount; i++)
	{	
		TextureUVList* pTexUVList = m_UiTexUVList[i];
		delete[] pTexUVList->szTexFileName;
		delete[] pTexUVList->szTexName;

		SAFE_RELEASE_TEXTURE(pTexUVList->nTextureId);

		BsUiUVLIST* pUVList = &(pTexUVList->UVList);
		for(DWORD j=0; j<pUVList->size(); j++)
		{
			delete[] (*pUVList)[j].szName;
		}

		pUVList->clear();
		delete pTexUVList;
	}

	m_UiTexUVList.clear();
}

bool BsUiEditUVManager::Load()
{
	m_nCurUVId = -1;
	m_nCurTextureId = -1;
	
	g_BsKernel.chdir("Menu");

	char szfileName[MAX_PATH];
	sprintf(szfileName, "%s%s", g_BsKernel.GetCurrentDirectory(), _MENU_INTERFACE_UVLIST_FILE);

	g_BsKernel.chdir("..");

	return Load(szfileName);
}

bool BsUiEditUVManager::Save()
{	
	g_BsKernel.chdir("Menu");

	char szUVList[MAX_PATH];
	sprintf(szUVList, "%s%s", g_BsKernel.GetCurrentDirectory(), _MENU_INTERFACE_UVLIST_FILE);

	char szUVID[MAX_PATH];
	sprintf(szUVID, "%s%s", g_BsKernel.GetCurrentDirectory(), _MENU_INTERFACE_UVID_FILE);

	g_BsKernel.chdir("..");

    std::sort(m_UiTexUVList.begin(), m_UiTexUVList.end(), TFn_CompareID() );

	SaveforUVList(szUVList);
	return SaveforUVID(szUVID);
}


bool BsUiEditUVManager::Load(const char* szFileName)
{
	Release();

	tpGrammer grammer;
	
	grammer.Add(BsUiCMD_BEGIN_TEXTURE_NAME, "Set_TexFileName=%s\nSet_Texture=%s\nSet_TexUnit=%d\nSet_Local=%d");
	grammer.Add(BsUiCMD_BEGIN_UV_ID, "ID=%d,NAME=%s\nUV=%d,%d,%d,%d");

	char fullname[MAX_PATH];
	char dir[MAX_PATH];
	char fn[MAX_PATH];

	strcpy(fullname, szFileName);
	SplitDirAndFileName( fullname, dir, _countof(dir), fn, _countof(fn)); //aleksger - safe string
	grammer.Create(szFileName, dir);

	int ret = 0;

	char szTextureFileName[256];
	TextureUVList* pTexUVList = NULL;
	int nTextureId = -1;	
	do {
		ret = grammer.Get();

		switch(ret)
		{
		case BsUiCMD_BEGIN_UV_ID:
			{
				BsAssert(pTexUVList != NULL);
				BsAssert(pTexUVList->nTextureId != -1);

				UVImage	Img;
				Img.nId = atoi(grammer.GetParam(0));

				int len = (int)strlen((char*)grammer.GetParam(1));
				Img.szName = new char[len+1];
				strcpy_s(Img.szName, len+1, (char*)grammer.GetParam(1)); //aleksger - safe string

				Img.u1 = atoi(grammer.GetParam(2));
				Img.v1 = atoi(grammer.GetParam(3));
				Img.u2 = atoi(grammer.GetParam(4));
				Img.v2 = atoi(grammer.GetParam(5));

				Img.nTexId = pTexUVList->nTextureId;

				pTexUVList->UVList.push_back(Img);

				m_nCurUVId = Img.nId;

				break;
			}
		case BsUiCMD_BEGIN_TEXTURE_NAME:
			{
				strcpy(szTextureFileName, (char*)grammer.GetParam(0));
				bool bLocal = atoi(grammer.GetParam(3)) ? true : false;
				
				if(bLocal == true){
					char cLocalFileName[256];
					GetImageFileName(szTextureFileName, cLocalFileName,	_countof(cLocalFileName), bLocal); //aleksger - safe string
					nTextureId = g_BsKernel.LoadTexture(cLocalFileName);
				}
				else
				{
					g_BsKernel.chdir("Interface");
					nTextureId = g_BsKernel.LoadTexture(szTextureFileName);
					g_BsKernel.chdir("..");
				}

				BsAssert(nTextureId != -1);

				pTexUVList = new TextureUVList;
				//File Name -> 
				int len = (int)strlen((char*)(char*)grammer.GetParam(0));
				pTexUVList->szTexFileName = new char[len+1];
				strcpy_s(pTexUVList->szTexFileName, len+1, (char*)grammer.GetParam(0)); //aleksger - safe string

				//define
				len = (int)strlen((char*)grammer.GetParam(1));
				pTexUVList->szTexName = new char[len+1];
				strcpy_s(pTexUVList->szTexName, len+1, (char*)grammer.GetParam(1));//aleksger - safe string

				pTexUVList->nTextureId = nTextureId;
				pTexUVList->nTexUnit = atoi(grammer.GetParam(2));
				pTexUVList->bLocal = bLocal;

				m_UiTexUVList.push_back(pTexUVList);
				m_nCurUVId = -1;

				break;
			}
		case -1:
			{
				CBsConsole::GetInstance().AddFormatString("%s File Not Found", szFileName);
				return false;
			}
			break;
		}
	} while (ret != 0);

	m_nCurTextureId = nTextureId;

	return true;
}


bool BsUiEditUVManager::SaveforUVList(const char* szFileName)
{
	FILE *fp = fopen(szFileName, "wt");

	if(fp==NULL)
	{
		CBsConsole::GetInstance().AddFormatString("%s File Not Found", szFileName);
		BsAssert(0);
		return false;
	}

	char szTag[256];
	DWORD dwCount = m_UiTexUVList.size();
	for(DWORD i=0; i<dwCount; i++)
	{	
		memset(szTag, 0, 256);
		strcpy(szTag, "\n//------------------------------------\n");//aleksger - more efficient.
		fwrite(&szTag, sizeof(char)*strlen(szTag), 1, fp);

		TextureUVList* pTexUVList = m_UiTexUVList[i];
/*
		int	nTextureId = pTexUVList->nTextureId;
		const char* szTextureName = g_BsKernel.GetTextureName(nTextureId);
		char cTempTextureName[256];
		if(szTextureName != NULL)
		{
			strcpy(cTempTextureName, szTextureName);
			if(pTexUVList->bLocal == true){
				ChangeTexFileNameforLocal(cTempTextureName);
			}
		}
*/
		memset(szTag, 0, 256);
		sprintf(szTag, "Set_TexFileName = \"%s\"\nSet_Texture = \"%s\"\nSet_TexUnit = %d\nSet_Local = %d\n",
			pTexUVList->szTexFileName,
			pTexUVList->szTexName,
			pTexUVList->nTexUnit,
			(int)pTexUVList->bLocal);

		fwrite(&szTag, sizeof(char)*strlen(szTag), 1, fp);

		memset(szTag, 0, 256);
		strcpy(szTag, "//------------------------------------\n");//aleksger - safe string
		fwrite(&szTag, sizeof(char)*strlen(szTag), 1, fp);
	
		BsUiUVLIST* pUVList = &(pTexUVList->UVList);
		for(DWORD j=0; j<pUVList->size(); j++)
		{
			UVImage Img = (*pUVList)[j];
			memset(szTag, 0, 256);
			sprintf(szTag, "ID = %d, Name = \"%s\"\nUV = %d, %d, %d, %d\n",
				Img.nId, Img.szName, Img.u1, Img.v1, Img.u2, Img.v2);
			fwrite(&szTag, sizeof(char)*strlen(szTag), 1, fp);
		}
	}

	fclose(fp);

	return true;
}


bool BsUiEditUVManager::SaveforUVID(const char* szFileName)
{
	FILE *fp = fopen(szFileName, "wt");

	if(fp==NULL)
	{
		CBsConsole::GetInstance().AddFormatString("%s File Not Found", szFileName);
		BsAssert(0);
		return false;
	}

	char szSpace[64], szTag[256];

	memset(szTag, 0, 256);
	strcpy(szTag, "//---------------------------------------------\n"); //aleksger - safe string
	fwrite(&szTag, sizeof(char)*strlen(szTag), 1, fp);
	strcpy(szTag, "//define Texture Id\n"); //aleksger - safe string
	fwrite(&szTag, sizeof(char)*strlen(szTag), 1, fp);

	int ndeflen = (int)strlen("#define ");
	DWORD dwCount = m_UiTexUVList.size();
	for(DWORD i=0; i<dwCount; i++)
	{
		TextureUVList* pTexUVList = m_UiTexUVList[i];

		int nNamelen = (int)strlen(pTexUVList->szTexName);

		memset(szSpace, 0, 64);
		for(int k = ndeflen+nNamelen; k<64; k++)
		{
			strcat(szSpace, " ");
		}

		memset(szTag, 0, 256);
		sprintf(szTag, "#define %s%s%d\n",
			pTexUVList->szTexName,
			szSpace,
			pTexUVList->nTexUnit);

		fwrite(&szTag, sizeof(char)*strlen(szTag), 1, fp);
	}

	memset(szTag, 0, 256);
	strcpy(szTag, "\n\n//---------------------------------------------\n"); //aleksger - safe string
	fwrite(&szTag, sizeof(char)*strlen(szTag), 1, fp);
	strcpy(szTag, "//define UV Info\n");//aleksger - safe string
	fwrite(&szTag, sizeof(char)*strlen(szTag), 1, fp);

	dwCount = m_UiTexUVList.size();
	for(DWORD i=0; i<dwCount; ++i)
	{
		TextureUVList* pTexUVList = m_UiTexUVList[i];

		memset(szTag, 0, 256);
		sprintf(szTag, "// %s\n", pTexUVList->szTexName);
		fwrite(&szTag, sizeof(char)*strlen(szTag), 1, fp);

		BsUiUVLIST* pUVList = &(pTexUVList->UVList);
		for(DWORD j=0; j<pUVList->size(); j++)
		{
			UVImage Img = (*pUVList)[j];

			int nNamelen = (int)strlen(Img.szName);

			memset(szSpace, 0, 64);
			for(int k = ndeflen+nNamelen; k<64; k++)
			{
				strcat(szSpace, " ");
			}

			memset(szTag, 0, 256);
			sprintf(szTag, "#define %s%s%d\n",
				Img.szName,
				szSpace,
				Img.nId);

			fwrite(&szTag, sizeof(char)*strlen(szTag), 1, fp);

		}
	}

	fclose(fp);

	return true;
}


bool BsUiEditUVManager::New(const char* szImgFileName)
{
	/*
	m_UiUVList.clear();
	m_nCurTextureId = g_BsKernel.LoadTexture(szImgFileName);
	if(m_nCurTextureId < 0)
	{
	BsAssert(0);
	return false;
	}
	*/

	return true;
}

int BsUiEditUVManager::AddTexture(char* szTextureFileName, bool bLocal)
{
	char cDscFileName[256];
	GetImageFileName(szTextureFileName, cDscFileName, _countof(cDscFileName), bLocal); //aleksger - safe string

	int nTextureId = -1;
	
	if(bLocal == false)
	{
		g_BsKernel.chdir("Interface");
		nTextureId = g_BsKernel.LoadTexture(cDscFileName);
		g_BsKernel.chdir("..");
	}
	else{
		nTextureId = g_BsKernel.LoadTexture(cDscFileName);
	}
	
	if(nTextureId < 0)
	{
		BsAssert(0);
		return -1;
	}

	DWORD dwCount = m_UiTexUVList.size();
	for(DWORD i=0; i<dwCount; i++)
	{	
		TextureUVList* pTexUVList = m_UiTexUVList[i];

		if(pTexUVList->nTextureId == nTextureId)
			return -1;
	}	

	TextureUVList* pTexUVList = new TextureUVList;
	const size_t cDscFileName_len = strlen(cDscFileName)+1; //aleksger - safe string
	pTexUVList->szTexFileName = new char[cDscFileName_len];
	sprintf_s(pTexUVList->szTexFileName, cDscFileName_len, cDscFileName);

	pTexUVList->nTextureId = nTextureId;
	pTexUVList->bLocal = bLocal;

	char szName[64];
	GenerateNewTexUnitName(szName, _countof(szName), nTextureId);//aleksger - safe string
	const size_t szName_len = strlen(szName)+1; //aleksger - safe string
	pTexUVList->szTexName = new char[szName_len];
	sprintf_s(pTexUVList->szTexName, szName_len, szName);

	pTexUVList->nTexUnit = GenerateNewTextureUnit();

	m_UiTexUVList.push_back(pTexUVList);

	m_nCurTextureId = nTextureId;
	m_nCurUVId = -1;

	return nTextureId;
}

bool BsUiEditUVManager::RemoveTexture(int nTextureId)
{
	if(nTextureId == -1)
		return false;

	DWORD dwCount = m_UiTexUVList.size();
	for(DWORD i=0; i<dwCount; i++)
	{	
		TextureUVList* pTexUVList = m_UiTexUVList[i];

		if(pTexUVList->nTextureId != nTextureId)
			continue;

		BsUiUVLIST* pUVList = &(pTexUVList->UVList);
		for(DWORD j=0; j<pUVList->size(); j++)
		{
			delete[] (*pUVList)[j].szName;
		}

		pUVList->clear();
		delete pTexUVList;

		m_UiTexUVList.erase(m_UiTexUVList.begin()+i);

		m_nCurTextureId = -1;
		m_nCurUVId = -1;

		return true;
	}

	BsAssert(0);
	return false;
}

BsUiUVId BsUiEditUVManager::AddInfo(int u1, int v1, int u2, int v2)
{
	int nId = GenerateNewUVId();
	if(nId == -1)
	{
		BsAssert(0);
		return -1;
	}

	UVImage Img;
	Img.nId = nId;
	Img.u1 = u1;	Img.v1 = v1;
	Img.u2 = u2;	Img.v2 = v2;
	Img.nTexId = m_nCurTextureId;

	char szName[64];
	GenerateNewName(szName, _countof(szName), Img.nId); //aleksger - safe string
	const size_t szName_len = strlen(szName)+1;
	Img.szName = new char[szName_len];
	strcpy_s(Img.szName, szName_len, szName); //aleksger - safe string

	DWORD dwCount = m_UiTexUVList.size();
	for(DWORD i=0; i<dwCount; i++)
	{	
		if(m_UiTexUVList[i]->nTextureId == m_nCurTextureId)
		{
			BsUiUVLIST* pUVList = &(m_UiTexUVList[i]->UVList);
			pUVList->push_back(Img);
			return Img.nId;
		}
	}

	BsAssert(0);
	return -1;
}

bool BsUiEditUVManager::RemoveInfo(BsUiUVId nUVId)
{
	int nUnit = nUVId / _UNIT_OF_TEXTURE;
	DWORD dwCount = m_UiTexUVList.size();
	for(DWORD i=0; i<dwCount; i++)
	{
		if(m_UiTexUVList[i]->nTexUnit == nUnit)
		{
			BsUiUVLIST* pUVList = &(m_UiTexUVList[i]->UVList);
			for(DWORD j=0; j<pUVList->size(); j++)
			{	
				if(	(*pUVList)[j].nId == nUVId)
				{
					delete[] (*pUVList)[j].szName;
					pUVList->erase(pUVList->begin()+j);
					return true;
				}
			}
		}
	}	

	return false;
}


bool BsUiEditUVManager::SetName(BsUiUVId nUVId, char* szName)
{
	BsAssert(szName != NULL);

	int nUnit = nUVId / _UNIT_OF_TEXTURE;
	DWORD dwCount = m_UiTexUVList.size();
	for(DWORD i=0; i<dwCount; i++)
	{
		if(m_UiTexUVList[i]->nTexUnit == nUnit)
		{
			BsUiUVLIST* pUVList = &(m_UiTexUVList[i]->UVList);
			for(DWORD j=0; j<pUVList->size(); j++)
			{	
				if(	(*pUVList)[j].nId == nUVId)
				{
					delete[] (*pUVList)[j].szName;

					const size_t szName_len = strlen(szName)+1; //aleksger - safe string
					(*pUVList)[j].szName = new char[szName_len];
					strcpy_s((*pUVList)[j].szName, szName_len, szName);
					return true;
				}
			}
		}
	}

	return false;
}

int BsUiEditUVManager::GenerateNewTextureUnit()
{
	int nUnit = 0;

	while(1)
	{
		if( IsSameTextureUnit( nUnit ) == false )
			break;

		nUnit++;
	}

	return nUnit;
}

bool BsUiEditUVManager::IsSameTextureUnit(int nUnit)
{	
	DWORD dwCount = m_UiTexUVList.size();
	for(DWORD i=0; i<dwCount; i++)
	{
		if(m_UiTexUVList[i]->nTexUnit == nUnit)
			return true;
	}

	return false;
}

BsUiUVId BsUiEditUVManager::GenerateNewUVId()
{
	BsUiUVId nId = -1;
	DWORD dwCount = m_UiTexUVList.size();
	for(DWORD i=0; i<dwCount; i++)
	{
		if(m_UiTexUVList[i]->nTextureId == m_nCurTextureId)
		{
			nId = m_UiTexUVList[i]->nTexUnit * _UNIT_OF_TEXTURE;
			break;
		}
	}

	BsAssert(nId != -1);

	int nCount = 0;
	while(1)
	{
		if( IsSameId( nId ) == false )
			break;

		nId++;
		nCount++;
		if(nCount >= _UNIT_OF_TEXTURE){
			return -1;
		}
	}

	return nId;
}

bool BsUiEditUVManager::IsSameId(BsUiUVId nUVId)
{
	int nUnit = nUVId / _UNIT_OF_TEXTURE;
	DWORD dwCount = m_UiTexUVList.size();
	for(DWORD j=0; j<dwCount; j++) //aleksger: prefix bug 708: Redeclaration of a variable.
	{
		if(m_UiTexUVList[j]->nTexUnit == nUnit)
		{
			BsUiUVLIST* pUVList = &(m_UiTexUVList[j]->UVList);
			for(DWORD i=0; i<pUVList->size(); i++)
			{
				if(	(*pUVList)[i].nId == nUVId)
				{	
					return true;
				}
			}

			return false;
		}
	}

	BsAssert(0);
	return false;
}


void BsUiEditUVManager::GenerateNewName(char* szName, const size_t szName_len, BsUiUVId nId)
{
	int nUnit = -1;
	DWORD dwCount = m_UiTexUVList.size();
	for(DWORD i=0; i<dwCount; i++)
	{
		if(m_UiTexUVList[i]->nTextureId == m_nCurTextureId)
		{
			nUnit = m_UiTexUVList[i]->nTexUnit * _UNIT_OF_TEXTURE;
			break;
		}
	}

	BsAssert(nUnit != -1);
	if(nUnit > nId || nUnit+_UNIT_OF_TEXTURE <= nId)
	{
		BsAssert(nId != -1);
		sprintf_s(szName,szName_len, "");//aleksger - safe string
		return;
	}


	int nCount = 0;
	while(1)
	{
		if(nCount == 0)
		{
			sprintf_s(szName, szName_len, "_UV_%d", nId);//aleksger - safe string
		}
		else
		{
			sprintf_s(szName,szName_len, "_UV_%d_%d", nId, nCount);//aleksger - safe string
		}

		if( IsSameName( szName ) == false )
			break;

		nCount++;
	}
}

bool BsUiEditUVManager::IsSameName(char* szName)
{
	DWORD dwCount = m_UiTexUVList.size();
	for(DWORD j=0; j<dwCount; j++) //aleksger: prefix bug 709: Redeclaration of a variable.
	{	
		BsUiUVLIST* pUVList = &(m_UiTexUVList[j]->UVList);
		for(DWORD i=0; i<pUVList->size(); i++)
		{
			if((*pUVList)[i].szName == NULL)
				continue;

			if(	strcmp((*pUVList)[i].szName, szName) == NULL)
			{	
				return true;
			}
		}
	}

	return false;
}

int	BsUiEditUVManager::GetTextureIdforIndex(int nIndex)
{
	if(nIndex >= (int)m_UiTexUVList.size())
	{
		BsAssert(0);
		return -1;
	}

	return m_UiTexUVList[nIndex]->nTextureId;
}

bool BsUiEditUVManager::SetCurTextureId(int nTextureId)
{
	DWORD dwCount = m_UiTexUVList.size();
	for(DWORD i=0; i<dwCount; i++)
	{
		if(m_UiTexUVList[i]->nTextureId == nTextureId)
		{
			m_nCurTextureId = nTextureId;
			return true;
		}
	}

	return false;
}

BsUiUVLIST*	BsUiEditUVManager::GetUVList(int nTextureId)
{	
	DWORD dwCount = m_UiTexUVList.size();
	for(DWORD i=0; i<dwCount; i++)
	{
		if(m_UiTexUVList[i]->nTextureId == nTextureId)
		{
			return &(m_UiTexUVList[i]->UVList);
		}
	}

	BsAssert(0);
	return NULL;
}


UVImage* BsUiEditUVManager::GetUVImage(BsUiUVId nUVId)
{
	int nUnit = nUVId / _UNIT_OF_TEXTURE;
	DWORD dwCount = m_UiTexUVList.size();
	for(DWORD j=0; j<dwCount; j++) //aleksger: prefix bug 710: Redeclaration of a variable.
	{
		if(m_UiTexUVList[j]->nTexUnit == nUnit)
		{
			BsUiUVLIST* pUVList = &(m_UiTexUVList[j]->UVList);
			for(DWORD i=0; i<pUVList->size(); i++)
			{
				if(	(*pUVList)[i].nId == nUVId)
				{	
					return &(*pUVList)[i];
				}
			}

			//BsAssert(0);
			return NULL;
		}
	}

	//BsAssert(0);
	return NULL;
}


bool BsUiEditUVManager::SetCurUVId(BsUiUVId nUVId)
{
	if(nUVId == -1)
	{
		m_nCurUVId = nUVId;
		return true;
	}

	int nUnit = nUVId / _UNIT_OF_TEXTURE;
	DWORD dwCount = m_UiTexUVList.size();
	for(DWORD j=0; j<dwCount; j++) //aleksger: prefix bug 711: Redeclaration of a variable.
	{
		if(m_UiTexUVList[j]->nTexUnit == nUnit)
		{
			BsUiUVLIST* pUVList = &(m_UiTexUVList[j]->UVList);
			for(DWORD i=0; i<pUVList->size(); i++)
			{
				if(	(*pUVList)[i].nId == nUVId)
				{	
					m_nCurUVId = nUVId;
					return true;
				}
			}

			BsAssert(0);
			return false;
		}
	}

	BsAssert(0);
	return false;
}

void BsUiEditUVManager::SetSelectedBox(bool bFlag, POINT* point)
{
	if(bFlag)
	{	
		m_bSelectedBox = true;
		m_rectSelected.left = m_rectSelected.right = point->x;
		m_rectSelected.top = m_rectSelected.bottom = point->y;
	}
	else
	{
		m_bSelectedBox = false;

		m_rectSelected.left = m_rectSelected.right = 0;
		m_rectSelected.top = m_rectSelected.bottom = 0;
	}
}

void BsUiEditUVManager::MoveSelectedBox(POINT* point)
{
	m_rectSelected.right = point->x;
	m_rectSelected.bottom = point->y;
}

BsUiUVId BsUiEditUVManager::SetAddUV()
{
	switch(m_EditUVState)
	{
	case _EditUV_AUTO_ADD: break;
	case _EditUV_AUTO_HAND:
		{	
			RECT rect;
			ArrayRect(rect, m_rectSelected);

			return AddInfo(rect.left, rect.top, rect.right, rect.bottom);
		}
	}

	return -1;
}

bool BsUiEditUVManager::SetAddUVforAuto()
{
	return true;
}

void BsUiEditUVManager::ChangeTexFileNameforLocal(char* szFileName)
{
	int nLen = strlen(szFileName);
	//aleksger - PREFIX bug 712
	BsAssert(nLen>8 && "ChangeTexFileNameforLocal did not receive sufficiently big buffer");
	szFileName[nLen-8] = '\0';
	// aleksger - explicitly writing after the original buffer to avoid the undefined behavior of having 
	//			both source and destination buffer overlap for sprintf.
	sprintf_s(szFileName+(nLen-8), 9, "_%s.dds", g_LocalLanguage.GetLanguageStr());
}

void BsUiEditUVManager::GetImageFileName(char* pSrcFileName, char* pDscFileName, size_t pDscFileName_size, bool bLocal)
{	
	char *pFindPtr;
	pFindPtr=(char*)strchr(pSrcFileName, '\\');

	bool bDataFolder = false;
	if(pFindPtr)
	{	
		while(1)
		{			
			if(pFindPtr == NULL){
				break;
			}

			pFindPtr = pFindPtr + 1;

			if(bLocal == false)
			{
				if(_strnicmp(pFindPtr, "Data\\Interface\\", 15) == 0)
				{
					strcpy_s(pDscFileName, pDscFileName_size, pFindPtr+15); //aleksger - safe string
					bDataFolder = true;
					break;
				}
			}
			else
			{
				if(_strnicmp(pFindPtr, "Data\\", 5) == 0)
				{
					strcpy_s(pDscFileName, pDscFileName_size, pFindPtr+5); //aleksger - safe string
					bDataFolder = true;
					break;
				}
			}

			pFindPtr = strchr(pFindPtr, '\\');
		}
	}

	if(bDataFolder == false){
		strcpy_s(pDscFileName, pDscFileName_size, pSrcFileName); //aleksger - safe string
	}

	if(bLocal){
		GetFileNameforLocal(pDscFileName);
	}
}

void BsUiEditUVManager::GetFileNameforLocal(char* pSrcFileName)
{
	if(pSrcFileName == NULL){
		return;
	}

	int nLen = strlen(pSrcFileName);
	if(nLen == 0){
		return;
	}

	char *ptempPtr = new char[nLen+1];
	strcpy_s(ptempPtr, nLen+1, pSrcFileName); //aleksger - safe string
	char* pFindPtr = ptempPtr;

	char cLocal[16];
	memset(cLocal, 16, 0);
	cLocal[0] = '\\';
	cLocal[1] = pSrcFileName[nLen-7];
	cLocal[2] = pSrcFileName[nLen-6];
	cLocal[3] = pSrcFileName[nLen-5];
	cLocal[4] = '\\';
	cLocal[5] = '\0';

	int nCount = 0;
	if(pFindPtr)
	{	
		while(1)
		{	
			if(strlen(pFindPtr) == 0){
				break;
			}

			if(pFindPtr == NULL){
				break;
			}

			if(_strnicmp(pFindPtr, cLocal, 5) == 0)
			{
				char cCurLocal[16];
				memset(cCurLocal, 16, 0);
				strcpy(cCurLocal, g_LocalLanguage.GetLanguageStr());

				pSrcFileName[nCount+1] = cCurLocal[0];
				pSrcFileName[nCount+2] = cCurLocal[1];
				pSrcFileName[nCount+3] = cCurLocal[2];

				pSrcFileName[nLen-7] = cCurLocal[0];
				pSrcFileName[nLen-6] = cCurLocal[1];
				pSrcFileName[nLen-5] = cCurLocal[2];
				break;
			}

			pFindPtr = pFindPtr + 1;
			nCount++;
		}
	}

	delete[] ptempPtr;
}


BsUiUVId BsUiEditUVManager::GetUVId(POINT point)
{	
	DWORD dwCount = m_UiTexUVList.size();
	for(DWORD i=0; i<dwCount; i++)
	{
		if(m_UiTexUVList[i]->nTextureId == m_nCurTextureId)
		{
			BsUiUVLIST* pUVList = &(m_UiTexUVList[i]->UVList);
			for(DWORD j=0; j<pUVList->size(); j++) //aleksger: prefix bug 714: Redeclaration of a variable.
			{
				UVImage Img = (*pUVList)[j];

				if(Img.u1 <= point.x && Img.v1 <= point.y && Img.u2 >= point.x && Img.v2 >= point.y )
				{
					return Img.nId;
				}
			}

			return -1;
		}
	}

	BsAssert(0);
	return -1;
}


BsUiUVId BsUiEditUVManager::GetUVId(char* szName)
{
	DWORD dwCount = m_UiTexUVList.size();
	for(DWORD i=0; i<dwCount; i++)
	{	
		BsUiUVLIST* pUVList = &(m_UiTexUVList[i]->UVList);
		for(DWORD j=0; j<pUVList->size(); j++) //aleksger: prefix bug 715: Redeclaration of a variable.
		{
			if((*pUVList)[j].szName == NULL)
				continue;

			if(	strcmp((*pUVList)[j].szName, szName) == NULL)
			{	
				return (*pUVList)[j].nId;
			}
		}
	}

	return -1;
}


SIZE BsUiEditUVManager::GetCurTextureSize()
{
	return g_BsKernel.GetTextureSize(m_nCurTextureId);
}

void BsUiEditUVManager::GenerateNewTexUnitName(char* szName, const size_t szName_len, int nTexUnitID)
{
	int nCount = 0;
	while(1)
	{
		if(nCount == 0)
		{
			sprintf_s(szName, szName_len, "_TEX_UNIT_%d", nTexUnitID);//aleksger - safe string
		}
		else
		{
			sprintf_s(szName, szName_len, "_TEX_UNIT_%d_%d", nTexUnitID, nCount);//aleksger - safe string
		}

		if( IsSameName( szName ) == false )
			break;

		nCount++;
	}
}

bool BsUiEditUVManager::IsSameTexUnitName(char* szName)
{
	DWORD dwCount = m_UiTexUVList.size();
	for(DWORD i=0; i<dwCount; i++)
	{	
		if(m_UiTexUVList[i]->szTexName == NULL)
			continue;

		if(	strcmp(m_UiTexUVList[i]->szTexName, szName) == NULL)
		{	
			return true;
		}
	}

	return false;
}

void BsUiEditUVManager::SetTextureUnitName(int nTextureID, char* szName)
{
	DWORD dwCount = m_UiTexUVList.size();
	for(DWORD i=0; i<dwCount; i++)
	{	
		if(m_UiTexUVList[i]->nTextureId == nTextureID){
			delete[] m_UiTexUVList[i]->szTexName;

			int len = (int)strlen(szName);
			m_UiTexUVList[i]->szTexName = new char[len+1];
			strcpy_s(m_UiTexUVList[i]->szTexName, len+1, szName); //aleksger - safe string
		}
	}
}

char* BsUiEditUVManager::GetTextureUnitName(int nTextureID)
{
	DWORD dwCount = m_UiTexUVList.size();
	for(DWORD i=0; i<dwCount; i++)
	{	
		if(m_UiTexUVList[i]->nTextureId == nTextureID){
			return m_UiTexUVList[i]->szTexName;
		}
	}

	return NULL;
}

int	BsUiEditUVManager::GetTextureUnitId(int nTextureID)
{
	DWORD dwCount = m_UiTexUVList.size();
	for(DWORD i=0; i<dwCount; i++)
	{	
		if(m_UiTexUVList[i]->nTextureId == nTextureID){
			return m_UiTexUVList[i]->nTexUnit;
		}
	}

	return -1;
}

bool BsUiEditUVManager::GetTextureLocal(int nTextureID)
{
	DWORD dwCount = m_UiTexUVList.size();
	for(DWORD i=0; i<dwCount; i++)
	{	
		if(m_UiTexUVList[i]->nTextureId == nTextureID){
			return m_UiTexUVList[i]->bLocal;
		}
	}

	return false;
}

//--------------------------------------------------------------------------------------
BsUiUVManager::BsUiUVManager()
: m_UVArray(NULL)
{	
}

BsUiUVManager::~BsUiUVManager()
{
	Release();
}

bool BsUiUVManager::Load(int nTexUnitID)
{
	g_BsKernel.chdir("Menu");

	char szfileName[MAX_PATH];
	sprintf(szfileName, "%s%s", g_BsKernel.GetCurrentDirectory(), _MENU_INTERFACE_UVLIST_FILE);

	g_BsKernel.chdir("..");

	if(Load(szfileName, nTexUnitID) == false){
		return false;
	}

	g_UVMgr.UpdateUVArray();

	return true;
}

bool BsUiUVManager::Load(const char* szFileName, int nTexUnitID)
{
	//Release();

	tpGrammer grammer;
	
	grammer.Add(BsUiCMD_BEGIN_TEXTURE_NAME, "\nSet_TexFileName=%s\nSet_Texture=%s\nSet_TexUnit=%d\nSet_Local=%d");
	grammer.Add(BsUiCMD_BEGIN_UV_ID, "ID=%d,NAME=%s\nUV=%d,%d,%d,%d");

	char fullname[MAX_PATH];
	char dir[MAX_PATH];
	char fn[MAX_PATH];

	strcpy(fullname, szFileName);
	SplitDirAndFileName( fullname, dir, _countof(dir), fn, _countof(fn)); //aleksger - safe string
	grammer.Create(szFileName, dir);

	int ret = 0;

	char szTextureFileName[256];
	int nTextureId = -1;
	bool bTexUnitFlag = false;

	do {
		ret = grammer.Get();

		switch(ret)
		{
		case BsUiCMD_BEGIN_UV_ID:
			{
				if(nTexUnitID != -1 && bTexUnitFlag == false){
					break;
				}

				UVImage	Img;
				Img.nId = atoi(grammer.GetParam(0));
				Img.szName = NULL;
				Img.u1 = atoi(grammer.GetParam(2));
				Img.v1 = atoi(grammer.GetParam(3));
				Img.u2 = atoi(grammer.GetParam(4));
				Img.v2 = atoi(grammer.GetParam(5));
				Img.nTexId = nTextureId;

				m_UiUVList.push_back(Img);

				break;
			}
		case BsUiCMD_BEGIN_TEXTURE_NAME:
			{
				int nTempTexUnitId = atoi(grammer.GetParam(2));
				if(nTexUnitID != -1 && nTexUnitID != nTempTexUnitId){
					bTexUnitFlag = false;
					break;
				}

				bTexUnitFlag = true;

				strcpy(szTextureFileName, (char*)grammer.GetParam(0));
				bool bLocal = atoi(grammer.GetParam(3)) ? true : false;
				if(bLocal == true){
					char cLocalFileName[256];
					GetImageFileName(szTextureFileName, cLocalFileName,_countof(cLocalFileName), bLocal); //aleksger - safe string
					nTextureId = g_BsKernel.LoadTexture(cLocalFileName);
				}
				else{
					g_BsKernel.chdir("Interface");
					nTextureId = g_BsKernel.LoadTexture(szTextureFileName);
					g_BsKernel.chdir("..");
				}

				BsAssert(nTextureId != -1);

				TextureUVList pTextureUVList;
				pTextureUVList.nTextureId = nTextureId;
				pTextureUVList.nTexUnit = nTexUnitID;
				m_TextureList.push_back(pTextureUVList);

				break;
			}
		case -1:
			{
			  DebugString("%s File Not Found", szFileName);
				CBsConsole::GetInstance().AddFormatString("%s File Not Found", szFileName);
				return false;
			}
			break;
		}
	} while (ret != 0);

	return true;
}

void BsUiUVManager::UpdateUVArray()
{
	ReleaseUVArray();

	DWORD dwCount = m_UiUVList.size();
	if(dwCount > 0)
	{
		int nTexIDCount = (int)(m_UiUVList[dwCount - 1].nId / _UNIT_OF_TEXTURE) + 1;
		m_UVArray = new int[nTexIDCount][_UNIT_OF_TEXTURE];
		memset(m_UVArray, -1, sizeof(int) * nTexIDCount * _UNIT_OF_TEXTURE);

		DWORD nCount = m_UiUVList.size();
		for(DWORD i=0; i<nCount; i++)
		{
			int nUnit = m_UiUVList[i].nId / _UNIT_OF_TEXTURE;
			BsUiUVId nId = m_UiUVList[i].nId % _UNIT_OF_TEXTURE;

			*(*(m_UVArray + nUnit) + nId) = i;
		}
	}
}


void BsUiUVManager::Release()
{
	ReleaseUVArray();

	DWORD dwCount = m_TextureList.size();
	for(DWORD i=0; i<dwCount; ++i)
	{
		SAFE_RELEASE_TEXTURE(m_TextureList[i].nTextureId);
	}

	m_TextureList.clear();
	m_UiUVList.clear();
}

void BsUiUVManager::ReleaseUVArray()
{
	delete [] m_UVArray;
	m_UVArray = NULL;
}


bool BsUiUVManager::Remove(int nTexUnitID)
{
	int nTextrueID = -1;
	DWORD dwCount = m_TextureList.size();
	for(DWORD i=0; i<dwCount; ++i)
	{
		if(m_TextureList[i].nTexUnit != nTexUnitID){
			continue;
		}
		nTextrueID = m_TextureList[i].nTextureId;

		SAFE_RELEASE_TEXTURE(m_TextureList[i].nTextureId);

		m_TextureList.erase( m_TextureList.begin() + i );
		break;
	}

	if(nTextrueID == -1){
		return false;
	}
	
	RemoveUVList(nTextrueID);

	return true;
}

void BsUiUVManager::RemoveUVList(int nTextureID)
{	
	DWORD dwCount = m_UiUVList.size();
	for(DWORD i=0; i<dwCount; i++)
	{
		if(m_UiUVList[i].nTexId == nTextureID)
		{
			m_UiUVList.erase( m_UiUVList.begin() + i );
			i--;
			continue;
		}
	}
}

UVImage* BsUiUVManager::GetUVImage(BsUiUVId nUVId)
{
	int nUnit = nUVId / _UNIT_OF_TEXTURE;
	BsUiUVId nId = nUVId % _UNIT_OF_TEXTURE;
	int nVecIndex = *(*(m_UVArray + nUnit) + nId);
	if(nVecIndex == -1){
		return NULL;
	}

	return &m_UiUVList[nVecIndex];
}

void BsUiUVManager::ChangeTexFileNameforLocal(char* szFileName)
{
	int nLen = strlen(szFileName);
	//aleksger - PREFix bug id 718 - asserting length
	BsAssert(nLen>8 && "ChangeTexFileNameforLocal did not receive sufficiently large buffer");
	szFileName[nLen-8] = '\0';
	// aleksger - explicitly writing after the original buffer to avoid the undefined behavior of having 
	//			both source and destination buffer overlap for sprintf.
	sprintf_s(szFileName+(nLen-8), 9, "_%s.dds", g_LocalLanguage.GetLanguageStr()); 
}

void BsUiUVManager::GetImageFileName(char* pSrcFileName, char* pDscFileName, size_t pDscFileName_size, bool bLocal)
{	
	char *pFindPtr;
	pFindPtr=(char*)strchr(pSrcFileName, '\\');

	bool bDataFolder = false;
	if(pFindPtr)
	{	
		while(1)
		{			
			if(pFindPtr == NULL){
				break;
			}

			pFindPtr = pFindPtr + 1;

			if(_strnicmp(pFindPtr, "Data\\", 5) == 0){
				strcpy_s(pDscFileName, pDscFileName_size, pFindPtr+5); //aleksger - safe string
				bDataFolder = true;
				break;
			}

			pFindPtr = strchr(pFindPtr, '\\');
		}
	}

	if(bDataFolder == false){
		strcpy_s(pDscFileName, pDscFileName_size, pSrcFileName); //aleksger - safe string
	}

	if(bLocal){
		GetFileNameforLocal(pDscFileName);
	}
}

void BsUiUVManager::GetFileNameforLocal(char* pSrcFileName)
{
	if(pSrcFileName == NULL){
		return;
	}

	int nLen = strlen(pSrcFileName);
	if(nLen == 0){
		return;
	}

	char *ptempPtr = new char[nLen+1];
	strcpy_s(ptempPtr, nLen+1, pSrcFileName);
	char* pFindPtr = ptempPtr;

	char cLocal[16];
	memset(cLocal, 16, 0);
	cLocal[0] = '\\';
	cLocal[1] = pSrcFileName[nLen-7];
	cLocal[2] = pSrcFileName[nLen-6];
	cLocal[3] = pSrcFileName[nLen-5];
	cLocal[4] = '\\';
	cLocal[5] = '\0';

	int nCount = 0;
	if(pFindPtr)
	{	
		while(1)
		{	
			if(strlen(pFindPtr) == 0){
				break;
			}

			if(pFindPtr == NULL){
				break;
			}

			if(_strnicmp(pFindPtr, cLocal, 5) == 0)
			{
				char cCurLocal[16];
				memset(cCurLocal, 16, 0);
				strcpy(cCurLocal, g_LocalLanguage.GetLanguageStr());

				pSrcFileName[nCount+1] = cCurLocal[0];
				pSrcFileName[nCount+2] = cCurLocal[1];
				pSrcFileName[nCount+3] = cCurLocal[2];

				pSrcFileName[nLen-7] = cCurLocal[0];
				pSrcFileName[nLen-6] = cCurLocal[1];
				pSrcFileName[nLen-5] = cCurLocal[2];
				break;
			}

			pFindPtr = pFindPtr + 1;
			nCount++;
		}
	}

	delete[] ptempPtr;
}