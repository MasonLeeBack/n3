#pragma	   once

class CFrameChecker
{
public:
	CFrameChecker();
	~CFrameChecker();

protected:
	bool m_bStart;
	DWORD m_dwStartTime;
	int m_nTotlaFrame;
	float m_fFps;

public:
	void Start();
	void End();
	float GetFPS();
	const char *GetFPSString();
	void AddFrame(int nCount);
};