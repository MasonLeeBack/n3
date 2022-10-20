#pragma		once

#include <d3d9.h>

struct AchievementPicture
{
	int				nTextureID;	// BsKernel texture id of the picture
	DWORD			dwImageId;	// Id of the picture
};

static const DWORD ACHIEVEMENT_COUNT = 10;

#ifdef _XBOX
class CFcAchievement
{
public:
	CFcAchievement();
	~CFcAchievement();

	bool Initialize();
	bool ShowAchievementUI();
	bool AwardGamerPicture();

	void SetUserIndex(DWORD dwUserIndex);

	void SetAspharrClear();
	void SetInphyyClear();
	void SetMyifeeClear();
	void SetTyurruClear();
	void SetKlarrannClear();
	void SetDwingvattClear();
	void SetVigkvagkClear();

	void SetHellClear();
	void SetAllARankClear();
	void SetAllHeroOfLevel9Clear();


	void Update();

protected:
	HRESULT InitializePictureTextures( DWORD dwTextureCount );
	void SetEvent( DWORD dwID );
	void EnumerateAchievements( XUID xuidUser );


protected:

	XUID m_LastUserXuid;
	XUID m_CurrentUserXuid;
	DWORD m_dwUserIndex;

	BYTE *m_Achievements;

	DWORD m_dwAchievementCount;
	DWORD m_dwGamesPlayed;
	DWORD m_dwWinningStreak;

	DWORD m_dwSelectedAchievement;
	AchievementPicture      m_AchievementPictures[ACHIEVEMENT_COUNT]; // Picture textures
};
#else 

class CFcAchievement
{
public:
	CFcAchievement() {}
	~CFcAchievement() {}

	bool Initialize() {}
	bool ShowAchievementUI() {}
	bool AwardGamerPicture() {}

	void SetUserIndex(DWORD dwUserIndex) {}

	void SetAspharrClear() {}
	void SetInphyyClear() {}
	void SetMyifeeClear() {}
	void SetTyurruClear() {}
	void SetKlarrannClear() {}
	void SetDwingvattClear() {}
	void SetVigkvagkClear() {}
};
#endif //_XBOX