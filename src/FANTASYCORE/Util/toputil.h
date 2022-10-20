#pragma once

char*	strword(int *nCursor, char *pBuf, int nBufSize);		


// ;는 주석문으로 처리, 그 줄을 무시.
// tab, space 등을 모두 무시하고 한 단어씩 뽑아냄
// 간단한 configuration file 등을 단어 단위로 읽어오는 데에 용이


char**	headerFileToStringTable(char *pFileName, int *nSize);

// *.h 화일에서 define으로 정의 된 int값들을 string table로 만들어 준다.
// 예: #define ASS_RIPPER		10
// 인 문장이 있으면 위에 건네저 오는 char** 에서 char *[10] = "ASS_RIPPER"가 된다. 
// macro 의 값이 integer가 아니면 string table이 개판이 되므로 주의

char*	briefDir(char *pFullDir);

// directory 이름에서 ..를 모두 없애서 실제 directory를 얻어낸다. 

char*	getDir(char *pFileNameWithDir);
char*	getFileName(char *pFileNameWithDir);
