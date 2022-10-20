#pragma once

#include "Singleton.h"

#define MAX_HITMAKR_BUF_SIZE		100
#define MIN_HITMARK_DISTANCE		70.0f
#define MIN_HITMARK_FRAME_DIST		5

struct HitMarkInfo
{
	D3DXVECTOR3 Position;
	int nFrame;
	int nHitMarkIndex;
};

class CFcHitMarkMgr : public CSingleton< CFcHitMarkMgr >
{
public:
	CFcHitMarkMgr(void);
	virtual ~CFcHitMarkMgr(void);

protected:
	HitMarkInfo m_HitMarkList[ MAX_HITMAKR_BUF_SIZE ];
	int m_nCurHitMarkCount;

public:
	void Reset() { m_nCurHitMarkCount = 0; }
	void AddHitMark( CCrossVector &Cross, int nFrame, int nHitMarkIndex );
	void ProcessHitMarkMgr( int nProcessTick );
};

#define g_FcHitMarkMgr	CFcHitMarkMgr::GetInstance()