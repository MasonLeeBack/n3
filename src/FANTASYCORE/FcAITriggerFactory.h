#pragma once

class CFcAIBase;
struct AITriggerDefine {
	int nTypeIndex;
	int nParamCount;
	char *szString;	// 나중 툴공유 위해서 일단 나둔당..
};

int GetAIConParamNum( int nID );
int GetAIActionParamNum( int nID );
int GetAISearchParamNum( int nID );