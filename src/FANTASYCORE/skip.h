#ifndef FRAMESKIPPER
#define FRAMESKIPPER

#define MAX_FRAME_RATE		60

class CSkipper
{
public:
	CSkipper( int nFrame );
	~CSkipper();

protected:
	bool		m_bSkip;
	bool		m_bStart;
	int			m_nFrameRate;
	float		m_fDeltaTimePerFrame;
	LARGE_INTEGER m_liFrequency;
	LARGE_INTEGER m_liStartTime;
	LARGE_INTEGER m_liPrevTime;
	LARGE_INTEGER m_liFramePerFreq;

public:
	float GetDeltaTime()	{	return m_fDeltaTimePerFrame;	}
	void SetFrame( int nFrame );
	void Start();
	void Stop();
	bool IsWait();
	void Wait();
	bool IsSkip();
};

#endif // #ifndef FRAMESKIPPER