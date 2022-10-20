#include "stdafx.h"
#include "BStreamExt.h"
#include "BsStringPool.h"

//------------------------------------------------------------------------------------------------

char *BsStringPool::Add(const char *szName)
{
	BsAssert(szName && "���ڿ� ��ȿ��");

	int nName=strlen(szName)+1;	// ���ڿ� ���� + 1
	char *pAlloc = new char [nName]; // ������ 1 �̻� �Դϴ�

	memcpy(pAlloc, szName, nName);

	return pAlloc;
}

//------------------------------------------------------------------------------------------------

char *BsStringPool::ReadAdd(BStream *pStream)
{
	int nName;
	pStream->Read(&nName, sizeof(int), ENDIAN_FOUR_BYTE); // ���ڿ� ���� + 1 

	BsAssert(nName && "���ڿ� ���� ��ȿ��");

	char *pAlloc = new char [nName]; // ������ 1 �̻� �Դϴ�
	
	pStream->Read(pAlloc, nName);

	return pAlloc;	 
}

//------------------------------------------------------------------------------------------------

void BsStringPool::Clear(char *szName)
{
	if (szName) delete [] szName; // NULL �� �з����ͷ� ȣ�� �Ǵ� ��쵵 �ֽ��ϴ�
}
				
//------------------------------------------------------------------------------------------------

	  





