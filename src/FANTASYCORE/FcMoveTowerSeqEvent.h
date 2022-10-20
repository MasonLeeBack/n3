#pragma once

#include "FcEventSequencer.h"

// ¼¼ÀÌ¶û¿¡¼­ Â÷·ÊÂ÷·Ê ³»¸®±â~

class CFcMoveTowerSeqGetOffEvent : public CFcEventSequencerElement 
{
public:
	CFcMoveTowerSeqGetOffEvent( CFcEventSequencerMng *pMng );
	virtual ~CFcMoveTowerSeqGetOffEvent();

public:
	int m_nFlag;
	int m_nCount;
	int m_nTick;

public:
	virtual void Run();
	virtual bool IsDestroy();

};