#pragma once

class CFcAIBase;
struct AITriggerDefine {
	int nTypeIndex;
	int nParamCount;
	char *szString;	// ���� ������ ���ؼ� �ϴ� ���д�..
};

int GetAIConParamNum( int nID );
int GetAIActionParamNum( int nID );
int GetAISearchParamNum( int nID );