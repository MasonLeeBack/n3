#pragma once

class CFcEventSequencerElement;
class CFcEventSequencerMng {
public:
	CFcEventSequencerMng( void *pVoid = NULL );
	virtual ~CFcEventSequencerMng();

protected:
	void *m_pVoid;
	std::vector<CFcEventSequencerElement *> m_pVecList;
	int m_nResetTime;

public:
	virtual void Reset();
	void Process();
	DWORD GetEventCount();

	void AddEvent( CFcEventSequencerElement *pEvent );
	void *GetVoid();
};

class CFcEventSequencerElement {
public:
	CFcEventSequencerElement( CFcEventSequencerMng *pMng );
	virtual ~CFcEventSequencerElement();

protected:
	CFcEventSequencerMng *m_pMng;

public:
	int m_nTime;
	bool m_bResetEvent;

public:
	int GetTime();
	bool IsResetEvent();
	virtual bool IsRun();
	virtual void Run();
	virtual bool IsDestroy();
};
