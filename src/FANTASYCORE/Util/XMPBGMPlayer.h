#pragma		once

#ifdef _XBOX

#include <xmp.h>

#define MAX_SONGID_NUM	50
#define BGM_DEFALT_VOL	0.2f

class CXMPBGMPlayer
{
public:
	CXMPBGMPlayer();
	~CXMPBGMPlayer();

	bool Initialize();
	void AddSong(char *pFileName,char *pTitleName,char *pArtist,char *pAlbum, int nTrackNum,bool bWma,int nDuration);
	void Play();	
	void Play(int nIndex,BOOL bLoop);
	void Clear();
	void Pause( bool bPause );
	void Stop();
	float GetVolume();
	void SetVolume( float fVolume );
	void Process();

	void Mute( bool bMute );

	//void FadeIn( int nTick, float fVolume );
	//void FadeOut( int nTick );

	void PlayNext();
	void PlayPrev();

	void Fade( int nTick, int nTargetPercent);

protected:
	int GetEmptySongID();

protected:
	XMP_HANDLE    m_hXMPPlaylist;            // The music player playlist
	HANDLE        m_hNotificationListener;
	XMP_SONGINFO  m_songInfo;                // Info about the current song.
	XMP_STATE     m_XMPState;                // The current status of the music player
	int           m_nSongCount;

	bool m_bFade;
	float m_fDelta;
	float m_fTargetVol;

	float m_fSaveCurVol;
	XMP_SONGDESCRIPTOR m_SongDescList[MAX_SONGID_NUM];
};

#endif