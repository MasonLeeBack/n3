#include "StdAfx.h"
#ifdef _XBOX
#include "FcWaveStream.h"
#include "AtgUtil.h"

CFcWaveStream::CFcWaveStream()
{
	for(int i = 0;i < MAX_WAVE_STREAM_NUM;i++){
		m_pStream[i] = NULL;
	}
}

CFcWaveStream::~CFcWaveStream()
{
	m_Wavebank.Close();
	for(int i = 0;i < MAX_WAVE_STREAM_NUM;i++){
		m_pStream[i] = NULL;
	}
}

void CFcWaveStream::LoadWaveBank(char *szFileName)
{
	HRESULT hr;
	if( FAILED( hr = m_Wavebank.Open( szFileName ) ) )
		ATG::FatalError( "Error %#X opening WAV file\n", hr );
}

int CFcWaveStream::MakeStream()
{
	int nIndex = GetEmptyStreamIndex();
	m_Wavebank.CreateStream( 0, &m_pStream[nIndex] );
	return nIndex;
}

void CFcWaveStream::Play(int nStreamIndex,int nIndex)
{
	if(m_pStream[nStreamIndex]) {	
		m_pStream[nStreamIndex]->Stop();
	}	
	m_Wavebank.ResetStream( nStreamIndex, nIndex);	
// [PREFIX:beginmodify] 2006/2/25 junyash PS#5826 dereferencing NULL pointer
	if( m_pStream[nStreamIndex] && FAILED(m_pStream[nStreamIndex]->Start()) ) { //aleksger: prefix bug 848: HRESULT should be checked with FAILED() macro.
		assert(0);
	}
// [PREFIX:endmodify] junyash
}

void CFcWaveStream::Stop( int nStreamIndex )
{
	m_pStream[nStreamIndex]->Stop();
}

int CFcWaveStream::GetEmptyStreamIndex()
{
	for(int i = 0;i < MAX_WAVE_STREAM_NUM;i++)
	{
		if ( m_pStream[i] == NULL )
			return i;
	}
	BsAssert(0 && "Wave stream count is over");
	return -1;
}

#endif