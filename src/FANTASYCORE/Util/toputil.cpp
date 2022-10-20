#include "stdafx.h"
#include "toputil.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bstreamext.h"

char*	strword(int *nCursor, char *pBuf, int nBufSize)
{
	static	char	String[1024];
	char			*pCsr1;
	char			Val;
	bool			bBegan;
	static	bool	bIgnore = false;

	pCsr1 = String;
	bBegan = false;

	while(1)
	{
		Val = pBuf[*nCursor];
		(*nCursor)++;


		if (Val == ';')
			bIgnore = true;

		if (((*nCursor) >= nBufSize) || (pCsr1 - String) >= sizeof(String)) // aleksger: prefix bug 879: possible buffer overflow
		{
			bIgnore = false;
			break;
		}

		if (bIgnore == true)
		{
			if (Val == '\n')
				bIgnore = false;
			else
				continue;
		}

		if (Val > 32 || Val < 0)
		{
			if (bBegan == false)
				bBegan = true;

			*pCsr1 = Val;
			pCsr1++;
		}
		else
		{
			if (bBegan == true)
				break;
		}
	}

	*pCsr1 = '\x0';

	if (String[0] == '\x0')
		return NULL;
	else
		return String;
}


char**	headerFileToStringTable(char *pFileName, int *nSize)
{
	unsigned long	nFileSize;
	int		nTableSIze;
	char	**ppRValue;
	char	**ppTemp;
	char	*pKeyword;
	char	*pBuf = NULL; //aleksger: prefix bug 883: freeing an unitialized variable.
	char	Macro[1024];
	int		nId;
	int		nCursor;

	BFileStream bfStream(pFileName);

	nTableSIze = 0;
	nCursor = 0;
	ppRValue = NULL;

	if (bfStream.Valid())
	{
		nFileSize = bfStream.Length();

		pBuf = (char*) malloc(nFileSize + 2);
		if(pBuf == NULL)
		{
			BsAssert(pBuf && "falied to allocate memory");
			return ppRValue;
		}

		bfStream.Read(pBuf,nFileSize);

		pKeyword = strword(&nCursor,pBuf,nFileSize);

		while(pKeyword != NULL)
		{
			if (!strcmp(pKeyword,"#define"))
			{
				pKeyword = strword(&nCursor, pBuf, nFileSize);
				
				if (pKeyword == NULL) //aleksger: prefix bug 882: strcmp does not take NULL.
					break;

				if (!strcmp(pKeyword,"#define"))
					continue;

				strncpy(Macro, pKeyword,1024);

				pKeyword = strword(&nCursor, pBuf, nFileSize);

				if (pKeyword == NULL) //aleksger: prefix bug 882: strcmp does not take NULL.
					break;

				if (!strcmp(pKeyword,"#define"))
					continue;

				nId = atoi(pKeyword);

				if (nId >= nTableSIze)
				{
					ppTemp = (char **)malloc(sizeof(char*) * (nId + 1));
					memset(ppTemp,0,sizeof(char*) * (nId + 1));

					if (nTableSIze > 0)
					{
						memcpy(ppTemp, ppRValue, sizeof(char*) * nTableSIze);
						free(ppRValue);
					}

					ppRValue = ppTemp;
					nTableSIze = nId + 1;
				}

				if (ppRValue[nId] != NULL)
					free (ppRValue[nId]);

				int templen=strlen(Macro);
				ppRValue[nId] = (char*) malloc(templen + 2);
				strcpy_s(ppRValue[nId],templen + 2,Macro);

			}
			
			pKeyword = strword(&nCursor,pBuf,nFileSize);
		}

	}

	*nSize = nTableSIze;

	free(pBuf);

	return (ppRValue);
}

char*	briefDir(char *pFullDir)
{
	static	char	RValue[_MAX_PATH];
	int		nDepth;
	char	*pDir[256];
	char	*pCsr;
	char	*pCopyFrom;
	char	*pCopyTo;
	

	nDepth = 0;

	strncpy(RValue,pFullDir, _MAX_PATH);

	pDir[nDepth] = RValue;

	while(1)
	{
		if ((*pDir[nDepth]) == '\\' || (*pDir[nDepth]) == '\x0')
			break;

		pDir[nDepth]++;
	}

	nDepth++;
	pDir[nDepth] = pDir[0] + 1;
	pCsr = pDir[nDepth];


	while(1)
	{
		if (*pDir[nDepth] == '\\')
		{
			pCsr++;

			if (pCsr[0] == '\\')
			{
				nDepth ++;
				pDir[nDepth] = pCsr;
			}

			if (pCsr[0] == '.' && pCsr[1] == '.' && pCsr[2] == '\\')
			{
				nDepth --;

				if (nDepth >= 0)
				{
					pCopyFrom = pCsr + 3;
					pCopyTo = pDir[nDepth] + 1;

					while(1)
					{
						*pCopyTo = *pCopyFrom;

						if (*pCopyFrom == '\x0')
							break;

						pCopyFrom++;
						pCopyTo++;
					}
				}

				pCsr = pDir[nDepth];
			}
		}
		else
		{
			pCsr++;
			pDir[nDepth]++;
		}

		if (*pCsr == '\x0')
			break;
	}

	return RValue;
}

char*	getDir(char *pFileNameWithDir)
{
	static	char	RValue[_MAX_PATH];
	char			*p;

	strncpy(RValue,pFileNameWithDir,_MAX_PATH);

	p = strrchr(RValue,'\\');

	if (p)
		(*(p+1)) = '\x0';

	return RValue;
}

char*	getFileName(char *pFileNameWithDir)
{
	static	char	str[_MAX_PATH];
	char			*p;

	strncpy(str,pFileNameWithDir,_MAX_PATH);

	p = strrchr(str,'\\');

	if (p)
		p++;

	return p;

}

