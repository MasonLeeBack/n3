#include "stdafx.h"

#ifdef _XBOX

#include <xtl.h>
#include <xmp.h>
#include "FcUtil.h"
#include "XMPBGMPlayer.h"
#include "BsUtil.h"
#include "FcCommon.h"


CXMPBGMPlayer::CXMPBGMPlayer()
{
	m_bFade      = false;
	m_fDelta     = 0.f;
	m_fTargetVol = 0.f;

	m_fSaveCurVol	 = 0.f;
	m_nSongCount = 0;
	memset(m_SongDescList,0,sizeof(XMP_SONGDESCRIPTOR)*MAX_SONGID_NUM);
}


CXMPBGMPlayer::~CXMPBGMPlayer()
{
	Clear();
}

void CXMPBGMPlayer::PlayNext()
{
	XMPNext();
}

void CXMPBGMPlayer::PlayPrev()
{
	XMPPrevious();
}

bool CXMPBGMPlayer::Initialize()
{
	DWORD dwStatus;

	m_hXMPPlaylist = NULL;
	memset(&m_songInfo, 0, sizeof(XMP_SONGINFO));


	// Create the notification listener to listen for XMP notifications
	m_hNotificationListener = XNotifyCreateListener( XNOTIFY_XMP );
	if( !m_hNotificationListener )
		DebugString( "Error calling XNotifyCreateListener\n");

	// Initialize XMP state variable
	dwStatus = XMPGetStatus( &m_XMPState );
	BsAssert( dwStatus == ERROR_SUCCESS );

	// Create a playlist
	dwStatus = XMPCreateTitlePlaylist( m_SongDescList,
		m_nSongCount,
		XMP_CREATETITLEPLAYLISTFLAG_NONE,
		L"NNN_BGM_PLAY", 
		NULL, 
		&m_hXMPPlaylist );
	BsAssert( dwStatus == ERROR_SUCCESS );

	// Set the playback behavior to be in order and repeat the entire playlist
	XMPSetPlaybackBehavior( XMP_PLAYBACKMODE_INORDER,
		XMP_REPEATMODE_PLAYLIST,
		0 );

	//고쳐야 함... 디폴트 음향으로
	SetVolume( BGM_DEFALT_VOL ); //Default Volue Setting

	return true;
}



int CXMPBGMPlayer::GetEmptySongID()
{

	DebugString("BGM index is not enough");
	BsAssert(0);
	return -1;
}

void CXMPBGMPlayer::AddSong(char *pFileName,char *pTitleName,char *pArtist,char *pAlbum, int nTrackNum,bool bWma,int nDuration)
{
	/*int nIndex = GetEmptySongID();
	DWORD dwStatus;
	WCHAR wcFileName[ MAX_PATH ];
	GetWide( pFileName, wcFileName, MAX_PATH );
	dwStatus = XMPAddSong( m_hXMPPlaylist, wcFileName, &m_SongIDList[nIndex] );*/
	//BsAssert( dwStatus == ERROR_SUCCESS );
	
	XMP_SONGDESCRIPTOR *pSongDesc = &m_SongDescList[m_nSongCount];
	pSongDesc->pwszFilePath = new WCHAR[ strlen(pFileName)+1  ];
	pSongDesc->pwszTitle    = new WCHAR[ strlen(pTitleName)+1 ];
	pSongDesc->pwszArtist   = new WCHAR[ strlen(pArtist)+1	  ];
	pSongDesc->pwszAlbum	= new WCHAR[ strlen(pAlbum)+1	  ];
	pSongDesc->pwszAlbumArtist = new WCHAR[12];
	memset((void *)pSongDesc->pwszAlbumArtist,0,sizeof(WCHAR)*12);
	pSongDesc->pwszGenre	= new WCHAR[12];
	memset((void *)pSongDesc->pwszGenre,0,sizeof(WCHAR)*12);

	MultiByteToWideChar(CP_ACP,0,pFileName,strlen(pFileName)+1,(WCHAR *)pSongDesc->pwszFilePath,strlen(pFileName)+1);
	MultiByteToWideChar(CP_ACP,0,pTitleName,strlen(pTitleName)+1,(WCHAR *)pSongDesc->pwszTitle ,strlen(pTitleName)+1);
	MultiByteToWideChar(CP_ACP,0,pArtist,strlen(pArtist)+1,(WCHAR *)pSongDesc->pwszArtist,strlen(pArtist)+1);
	MultiByteToWideChar(CP_ACP,0,pAlbum,strlen(pAlbum)+1,(WCHAR *)pSongDesc->pwszAlbum,strlen(pAlbum)+1);

	pSongDesc->dwTrackNumber = nTrackNum;
	pSongDesc->dwDuration    = nDuration;

	(bWma)? pSongDesc->eSongFormat  = XMP_SONGFORMAT_WMA : pSongDesc->eSongFormat  = XMP_SONGFORMAT_MP3;

	m_nSongCount++;
	
}


void CXMPBGMPlayer::Play(int nIndex,BOOL bLoop)
{
	if(bLoop)
	{
	}
	else
	{
	}
	//XMPPlayTitlePlaylist( m_hXMPPlaylist, m_SongIDList[nIndex] );
	XMPPlayTitlePlaylist( m_hXMPPlaylist, NULL );
}

void CXMPBGMPlayer::Play()
{
	XMPPlayTitlePlaylist( m_hXMPPlaylist, NULL );
}

void CXMPBGMPlayer::Clear()
{
	DWORD dwStatus;

	// Stop the music
	XMPStop();

	// Wait for the music to stop
	dwStatus = XMPGetStatus( &m_XMPState );
	BsAssert( dwStatus == ERROR_SUCCESS );

	while( m_XMPState != XMP_STATE_IDLE )
	{
		Sleep( 1 );
		dwStatus = XMPGetStatus( &m_XMPState );
		BsAssert( dwStatus == ERROR_SUCCESS );
	}

	if( m_hXMPPlaylist )
	{
		// Delete the playlist
		dwStatus = XMPDeleteTitlePlaylist( m_hXMPPlaylist );
		BsAssert( dwStatus == ERROR_SUCCESS );   
	}


	for(int i = 0;i < MAX_SONGID_NUM;i++)
	{
		SAFE_DELETE(m_SongDescList[i].pwszAlbum);
		SAFE_DELETE(m_SongDescList[i].pwszAlbumArtist);
		SAFE_DELETE(m_SongDescList[i].pwszArtist);
		SAFE_DELETE(m_SongDescList[i].pwszFilePath);
		SAFE_DELETE(m_SongDescList[i].pwszGenre);
		SAFE_DELETE(m_SongDescList[i].pwszTitle);
	}
}


void CXMPBGMPlayer::Pause( bool bPause )
{
	if( bPause )
	{
		XMPPause();
	}
	else
	{
		XMPContinue();
	}
}

void CXMPBGMPlayer::Stop()
{
	XMPStop();
}

float CXMPBGMPlayer::GetVolume()
{
	float   fVolume;
	DWORD   dwResult = XMPGetVolume( &fVolume );
	BsAssert( dwResult == ERROR_SUCCESS );
	return fVolume;
}

void CXMPBGMPlayer::SetVolume( float fVolume )
{
	DWORD dwResult = XMPSetVolume( fVolume );
	BsAssert( dwResult == ERROR_SUCCESS );
}

void CXMPBGMPlayer::Process()
{
	 //DWORD dwStatus;
	// Check for new media player notifications.  All we're really looking for is if the
	// state has changed.
	DWORD      dwMsgFilter;
	ULONG_PTR  param;
	if( XNotifyGetNext( m_hNotificationListener, 0, &dwMsgFilter, &param ) )
	{
		switch( dwMsgFilter )
		{   
		case XN_XMP_STATECHANGED:
			XMPGetStatus(&m_XMPState);

			// Get the currently playing song
			if( m_XMPState != XMP_STATE_IDLE )
			{
				//memset(&m_songInfo, 0, sizeof(XMP_SONGINFO));

				//dwStatus = XMPGetCurrentPlaylist( NULL, &m_songInfo );
				//BsAssert( dwStatus == ERROR_SUCCESS );  
			}

			break;
		}
	}

	if( m_bFade )
	{
		float fVol = GetVolume();

		if(fabsf( m_fTargetVol - fVol) <= fabsf(m_fDelta)){		
			SetVolume( m_fTargetVol );
			m_bFade = false;
		}
		else{
			fVol += m_fDelta;
			SetVolume( fVol );
		}

		if( m_fDelta < 0.f ){		
			if( fVol < 0.f )
			{
				SetVolume( 0.f );
				m_bFade = false;
			}
			else{			
				SetVolume( fVol );
			}
		}
		else{		
			if( fVol > m_fTargetVol )
			{
				SetVolume( m_fTargetVol );
				m_bFade = false;
			}
			else{			
				SetVolume( fVol );
			}
		}
	}
}

void CXMPBGMPlayer::Mute( bool bMute )
{
	if( bMute )
	{
		m_fSaveCurVol = GetVolume();
		SetVolume( 0.f );
	}
	else
	{
		SetVolume( m_fSaveCurVol );
	}
}

/*void CXMPBGMPlayer::FadeIn( int nTick, float fVolume )
{
	m_bFade		 = true;
	m_fTargetVol = fVolume;
	m_fDelta	 = m_fTargetVol / (float)nTick;
}

void CXMPBGMPlayer::FadeOut( int nTick )
{
	m_bFade = true;
	// float fVol = GetVolume();
	m_fTargetVol = 0.f;
	m_fDelta = -( m_fTargetVol / (float)nTick );
}*/


void CXMPBGMPlayer::Fade( int nTick,int nTargetPercent)
{
	m_bFade = true;
	float fVolume;
	m_fTargetVol = (float)nTargetPercent / 100.f;	
	XMPGetVolume( &fVolume );
	m_fDelta = (m_fTargetVol - fVolume) / (float)nTick;

}
#endif