#include "stdafx.h"
#include "FcAchievement.h"

#ifdef _XBOX

#include "AtgSignIn.h"
#include "BsKernel.h"

#include "N3GameConfig.spa.h"

#include <Xbox.h>


CFcAchievement::CFcAchievement()
{
	m_LastUserXuid = m_CurrentUserXuid = INVALID_XUID;
	m_dwUserIndex = (DWORD)-1;

	m_dwAchievementCount = 0;
	m_dwGamesPlayed = 0;
	m_dwWinningStreak = 0;
	m_dwSelectedAchievement = 0;
	m_Achievements = NULL;

	for( DWORD i = 0; i < ACHIEVEMENT_COUNT; ++i )
		m_AchievementPictures[i].nTextureID = -1;

}

CFcAchievement::~CFcAchievement()
{
	for( DWORD i = 0; i < ACHIEVEMENT_COUNT; ++i )
	{
		if(m_AchievementPictures[i].nTextureID != -1)
			g_BsKernel.DeleteTexture(m_AchievementPictures[i].nTextureID);
		m_AchievementPictures[i].nTextureID = -1;
	}
}

bool CFcAchievement::Initialize()
{
	m_Achievements = new BYTE[XACHIEVEMENT_SIZE_FULL * ACHIEVEMENT_COUNT];
	if( m_Achievements == NULL )
		return false;

	if( FAILED( InitializePictureTextures( ACHIEVEMENT_COUNT ) ) )
		return false;
	return true;
}

bool CFcAchievement::ShowAchievementUI()
{
	if( ATG::SignIn::AreUsersSignedIn() == FALSE )
		return false;

	DWORD dwUserID = ATG::SignIn::GetSignedInUser();
//	if( dwUserID == INVALID_XUID )
//		return true;

	DWORD dwErr = XShowAchievementsUI( dwUserID );
	assert( dwErr == ERROR_SUCCESS );
	return true;
}

bool CFcAchievement::AwardGamerPicture()
{
	DWORD dwErr = XUserAwardGamerPicture(
		m_dwUserIndex,          // Currently signed in gamer
		m_AchievementPictures[m_dwSelectedAchievement].dwImageId, // Selected picture
		0,                      // Reserved parameter
		NULL );                 // Not using overlapped I/O

	if( dwErr != ERROR_SUCCESS )
		return false;

	return true;
}

void CFcAchievement::SetUserIndex(DWORD dwUserIndex) 
{
	m_dwUserIndex = dwUserIndex;
}

void CFcAchievement::SetAspharrClear()
{
	SetEvent( ACHIEVEMENT_ASPHARRCLEAR );
}

void CFcAchievement::SetInphyyClear()
{
	SetEvent( ACHIEVEMENT_INPHYYCLEAR );
}

void CFcAchievement::SetMyifeeClear()
{
	SetEvent( ACHIEVEMENT_MYIFEECLEAR );
}

void CFcAchievement::SetTyurruClear()
{
	SetEvent( ACHIEVEMENT_TYURRUCLEAR );
}

void CFcAchievement::SetKlarrannClear()
{
	SetEvent( ACHIEVEMENT_KLARRANNCLEAR );
}

void CFcAchievement::SetDwingvattClear()
{
	SetEvent( ACHIEVEMENT_DWINGVATTCLEAR );
}

void CFcAchievement::SetVigkvagkClear()
{
	SetEvent( ACHIEVEMENT_VIGKVAGKCLEAR );
}

void CFcAchievement::SetHellClear() 
{
	SetEvent(ACHIEVEMENT_HELLCLEAR);
}
void CFcAchievement::SetAllARankClear()
{
	SetEvent(ACHIEVEMENT_ALLARANKCLEAR);
}
void CFcAchievement::SetAllHeroOfLevel9Clear()
{
	SetEvent(ACHIEVEMENT_ALLHEROOFLEVEL9CLEAR);
}

void CFcAchievement::Update()
{
	bool bNewUser = false;
	if( ATG::SignIn::AreUsersSignedIn() )
    {
        for( DWORD dwCnt = 0; dwCnt < XUSER_MAX_COUNT; ++dwCnt )
        {
            if( ATG::SignIn::IsUserSignedIn( dwCnt ) )
            {
                if( dwCnt != m_dwUserIndex )
                {
                    m_LastUserXuid = m_CurrentUserXuid;
                    XUserGetXUID( dwCnt, &m_CurrentUserXuid );
					bNewUser = true;
                }
                m_dwUserIndex = dwCnt;
                break;
            }
        }
		if( bNewUser )
            EnumerateAchievements( m_LastUserXuid );
	}
}

HRESULT CFcAchievement::InitializePictureTextures( DWORD dwTextureCount )
{
	for( DWORD i = 0; i < dwTextureCount; ++i )
	{
		m_AchievementPictures[i].nTextureID = g_BsKernel.CreateTexture(64, 64, D3DUSAGE_CPU_CACHED_MEMORY, D3DFMT_LIN_A8R8G8B8);
		if( m_AchievementPictures[i].nTextureID == -1 )
			return E_FAIL;
	}
	return S_OK;
}

void CFcAchievement::SetEvent( DWORD dwID )
{
	HANDLE hEventComplete = CreateEvent( NULL, FALSE, FALSE, NULL );

	if( hEventComplete == NULL )
	{
		DebugString( "Couldn't create event.\n" );
		BsAssert( 0 );
	}
	XOVERLAPPED xov;
	ZeroMemory( &xov, sizeof(XOVERLAPPED) );
	xov.hEvent = hEventComplete;

	// determine which achievements have occured
	XUSER_ACHIEVEMENT   Achievements;
	Achievements.dwUserIndex = m_dwUserIndex;
	Achievements.dwAchievementId = dwID;

	// Write achievements
	DWORD dwStatus = XUserWriteAchievements( 1, &Achievements, &xov );
	BsAssert( dwStatus == ERROR_IO_PENDING );
	dwStatus = XGetOverlappedResult( &xov, NULL, TRUE );
	BsAssert( dwStatus == ERROR_SUCCESS );
	CloseHandle( hEventComplete );

	EnumerateAchievements( INVALID_XUID );
}


void CFcAchievement::EnumerateAchievements( XUID xuidUser )
{
	HANDLE hEnum;
	DWORD cbBuffer;

	// Create enumerator for the default device
	DWORD dwStatus;
	dwStatus = XUserCreateAchievementEnumerator( 0,
												m_dwUserIndex,
												xuidUser,
												XACHIEVEMENT_DETAILS_ALL,
												0,
												ACHIEVEMENT_COUNT,
												&cbBuffer,
												&hEnum );

	BsAssert( dwStatus == ERROR_SUCCESS );

	// Enumerate achievements
	m_dwAchievementCount = 0;
	DWORD dwItems;

	if( XEnumerate( hEnum, m_Achievements, XACHIEVEMENT_SIZE_FULL * ACHIEVEMENT_COUNT,
		&dwItems, NULL ) == ERROR_SUCCESS )
	{
		m_dwAchievementCount = dwItems;
	}

	// Retrieve achievement pictures
	XACHIEVEMENT_DETAILS* rgAchievements = (XACHIEVEMENT_DETAILS*)m_Achievements;

	for ( DWORD i = 0; i < m_dwAchievementCount; ++i )
	{
		m_AchievementPictures[i].dwImageId = i + GAMER_PICTURE_NNN;
		IDirect3DTexture9*  pTexture = (IDirect3DTexture9*)g_BsKernel.GetTexturePtr(m_AchievementPictures[i].nTextureID);
		D3DLOCKED_RECT      d3dlr;
		D3DSURFACE_DESC     d3dsdec;

		if(pTexture)
		{

			HRESULT hr = pTexture->GetLevelDesc( 0, &d3dsdec );

			BsAssert( SUCCEEDED( hr ) );

			hr = pTexture->LockRect( 0,	&d3dlr,	NULL, 0 );

			BsAssert( SUCCEEDED( hr ) );

			dwStatus = XUserReadAchievementPicture(
				m_dwUserIndex,					// Requesting user
				TITLEID_NINETY_NINE_NIGHTS,		// Title Id
				rgAchievements[i].dwImageId,	// Image Id
				(BYTE*)d3dlr.pBits,				// Texture bytes
				d3dlr.Pitch,
				d3dsdec.Height,
				NULL );

			BsAssert( dwStatus == ERROR_SUCCESS );

			pTexture->UnlockRect( 0 );
		}
	}
	CloseHandle( hEnum );
}

#endif //_XBOX