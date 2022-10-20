#pragma once
#ifdef _XBOX
#define MAX_WAVE_STREAM_NUM		8


#include "CWavebankStream.h"

class CFcWaveStream
{
public:
	CFcWaveStream();
	~CFcWaveStream();
	void LoadWaveBank(char *szFileName);
	int MakeStream();
	void Play( int nStreamIndex,int nIndex );
	void Stop( int nStreamIndex );
	void SetVolume(int nStreamIndex,float fVol);
	float GetVolume(int nStreamIndex);
private:
	int GetEmptyStreamIndex();
protected:
	CWavebankStream			m_Wavebank;     // Wavebank streaming class
	CStreamVoice*			m_pStream[MAX_WAVE_STREAM_NUM];

};
#endif 