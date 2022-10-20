#pragma	   once

#include "FcParamVariable.h"

enum TRIGGER_TYPE
{
	TRIGGER_TYPE_NON = -1,
	TRIGGER_TYPE_IF,
	TRIGGER_TYPE_CMD,
	TRIGGER_TYPE_LABEL,
};


struct TROOPAITRIGGERDATA
{
	TROOPAITRIGGERDATA()
	{
		Clear();
	}

	void Clear()
	{
		Type = TRIGGER_TYPE_NON;
		nID = -1;
		for( int i=0; i<16; i++ )
			Variable[i].Clear();
	}

	// 이 함수 왜 필요한거지?
	void operator = ( const TROOPAITRIGGERDATA &Obj )
	{
		Type = Obj.Type;
		nID = Obj.nID;
		for( int i=0; i<16; i++ )
			Variable[i] = (CFcParamVariable)Obj.Variable[i];
	}

	TRIGGER_TYPE Type;
	int nID;
	CFcParamVariable Variable[16];
};



struct TROOPAIGROUPDATA
{
	int nID;
	char cName[32];

	TROOPAIGROUPDATA()
	{
		nID = -1;
		cName[0] = NULL;
	}


	~TROOPAIGROUPDATA()
	{
		int nCnt = VecTrigger.size();
		for( int i=0; i<nCnt; i++ )
		{
			delete VecTrigger[i];
		}
		VecTrigger.clear();
	}

	std::vector<TROOPAITRIGGERDATA*> VecTrigger;
};



class FcTroopAIManager
{
public:
	FcTroopAIManager();
	~FcTroopAIManager();



};