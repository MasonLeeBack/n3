#include "stdafx.h"
#include "FcInterfaceDataDefine.h"
#include "BSKernel.h"
#include "Token.h"
#include "Parser.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG

#define _PORTRAIT_FILE_NAME		"PortraitUV.txt"

CFcUVList::CFcUVList()
{	
}

CFcUVList::~CFcUVList()
{	
	for(int i = 0;i < (int)m_UVList.size();i++)
	{	
		SAFE_RELEASE_TEXTURE(m_UVList[i]->nTexId);

		delete[] m_UVList[i]->szName;
		delete m_UVList[i];
	}

	m_UVList.clear();
}

void CFcUVList::LoadUVTable(char *szFileName)
{
	char szFullPath[MAX_PATH];
	sprintf(szFullPath,"%s%s",g_BsKernel.GetCurrentDirectory(),szFileName);

	Parser parser;
	TokenList toklist;

	parser.Create();
	parser.EnableComments( true );

	bool result;
	result = parser.ProcessSource( "", szFullPath, &toklist );
	if( result == false ){		
		BsAssert(0 && "UV테이블 못 읽어옴");
	}
	parser.ProcessMacros( &toklist );
	TokenList::iterator itr = toklist.begin();

	while( 1 )
	{
		UVImage *pUVInfo = new UVImage;

		//id
		pUVInfo->nId = itr->GetInteger();
		itr++;

		//define
		itr++;

		//texture file name
		int nLen = strlen(itr->GetString());
		pUVInfo->szName = new char[nLen + 1];
		strcpy_s(pUVInfo->szName, nLen + 1, itr->GetString()); //aleksger - safe string
		itr++;

		//uv info
		pUVInfo->u1 = itr->GetInteger(); itr++;
		pUVInfo->v1 = itr->GetInteger(); itr++;
		pUVInfo->u2 = itr->GetInteger(); itr++;
		pUVInfo->v2 = itr->GetInteger(); itr++;
		
		m_UVList.push_back(pUVInfo);

		if( itr == toklist.end() ){
			break;
		}
	}
}


UVImage *CFcUVList::GetUVInfo(int nId)
{
	BsAssert(m_UVList[nId]->nId == nId && "인덱스 참조 오류");
	return m_UVList[nId];
}

UVImage *CFcUVList::GetUVInfoforTex(int nId)
{
	BsAssert(m_UVList[nId]->nId == nId && "인덱스 참조 오류");

	if(m_UVList[nId]->nTexId == -1)
	{
		g_BsKernel.chdir("interface");
		g_BsKernel.chdir("portrait");
		m_UVList[nId]->nTexId = g_BsKernel.LoadTexture(m_UVList[nId]->szName);
		BsAssert(m_UVList[nId]->nTexId);
		g_BsKernel.chdir("..");
		g_BsKernel.chdir("..");
	}

	return m_UVList[nId];
}


//-----------------------------------------------------------------------------------------
CFcPortrait::CFcPortrait()
{
	
}

CFcPortrait::~CFcPortrait()
{
	;
}

void CFcPortrait::Load()
{
	g_BsKernel.chdir("interface");
	g_BsKernel.chdir("portrait");
	LoadUVTable(_PORTRAIT_FILE_NAME);
	g_BsKernel.chdir("..");
	g_BsKernel.chdir("..");
}

void CFcPortrait::ReleaseTexture()
{
	for(int i = 0;i < (int)m_UVList.size();i++)
	{
		SAFE_RELEASE_TEXTURE(m_UVList[i]->nTexId);
	}
}


void CFcPortrait::DrawPortrait(int nId, int iSX, int iSY, int iEX, int iEY, float fAlpha)
{
	UVImage *pInfo = GetUVInfoforTex(nId);

	g_BsKernel.DrawUIBox_s(_Ui_Mode_Image,
		iSX, iSY, //포지션
		iEX, iEY, //스케일
		0.f, D3DXCOLOR(1.f,1.f,1.f,fAlpha),
		0.f,
		pInfo->nTexId,
		pInfo->u1, pInfo->v1,
		pInfo->u2, pInfo->v2);
}

void CFcPortrait::DrawPortrait(int nId, int PosX, int PosY, float fAlpha)
{
	UVImage *pInfo = GetUVInfoforTex(nId);

	//스케일
	int sx = (pInfo->u2 - pInfo->u1);
	int sy = (pInfo->v2 - pInfo->v1);

	g_BsKernel.DrawUIBox_s(_Ui_Mode_Image,
		PosX, PosY, sx, sy,	
		0.f, D3DXCOLOR(1.f,1.f,1.f,fAlpha),
		0.f,
		pInfo->nTexId,
		pInfo->u1, pInfo->v1,
		pInfo->u2, pInfo->v2);
}