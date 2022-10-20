#pragma once

char*	strword(int *nCursor, char *pBuf, int nBufSize);		


// ;�� �ּ������� ó��, �� ���� ����.
// tab, space ���� ��� �����ϰ� �� �ܾ �̾Ƴ�
// ������ configuration file ���� �ܾ� ������ �о���� ���� ����


char**	headerFileToStringTable(char *pFileName, int *nSize);

// *.h ȭ�Ͽ��� define���� ���� �� int������ string table�� ����� �ش�.
// ��: #define ASS_RIPPER		10
// �� ������ ������ ���� �ǳ��� ���� char** ���� char *[10] = "ASS_RIPPER"�� �ȴ�. 
// macro �� ���� integer�� �ƴϸ� string table�� ������ �ǹǷ� ����

char*	briefDir(char *pFullDir);

// directory �̸����� ..�� ��� ���ּ� ���� directory�� ����. 

char*	getDir(char *pFileNameWithDir);
char*	getFileName(char *pFileNameWithDir);
