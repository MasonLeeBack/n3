#include "stdafx.h"
#include "BStreamExt.h"
#include "BsStringPool.h"

//------------------------------------------------------------------------------------------------

char *BsStringPool::Add(const char *szName)
{
	BsAssert(szName && "문자열 유효성");

	int nName=strlen(szName)+1;	// 문자열 길이 + 1
	char *pAlloc = new char [nName]; // 언제나 1 이상 입니다

	memcpy(pAlloc, szName, nName);

	return pAlloc;
}

//------------------------------------------------------------------------------------------------

char *BsStringPool::ReadAdd(BStream *pStream)
{
	int nName;
	pStream->Read(&nName, sizeof(int), ENDIAN_FOUR_BYTE); // 문자열 길이 + 1 

	BsAssert(nName && "문자열 길이 유효성");

	char *pAlloc = new char [nName]; // 언제나 1 이상 입니다
	
	pStream->Read(pAlloc, nName);

	return pAlloc;	 
}

//------------------------------------------------------------------------------------------------

void BsStringPool::Clear(char *szName)
{
	if (szName) delete [] szName; // NULL 이 패러미터로 호출 되는 경우도 있습니다
}
				
//------------------------------------------------------------------------------------------------

	  





