#pragma once

#pragma pack( push )
#pragma pack( 4 )


class CRMWave 
{
public :

	enum 
	{ 
		eWave32=32,
		eFileNameLength=64,
		eWaveFirstSize=eFileNameLength*2
	};

	CRMWave(); // : m_nEvent(0), m_fTime( -1.f) {};
	~CRMWave(){};

	void	ResetEventFlag(void)	{	m_nEvent = 0;				}

	void	SetName(char *szName)	
	{	
		if( szName == NULL || szName[0] == NULL )
			memset( m_szName, 0, eFileNameLength );
		else
            strcpy( m_szName, szName );	
	}

	char*	GetName(void)			{	return m_szName;			}

	void	SetWaveFileName(char *szFile)	
	{	
		if( szFile == NULL || szFile[0] == NULL )
			memset( m_szFileName, 0, eFileNameLength );
		else
            strcpy( m_szFileName, szFile );	
	}

	char*	GetWaveFileName(void)			{	return m_szFileName;			}

	void	SetLinkedObjName(char *szName)	{	strcpy( m_szObjName, szName );	}
	char*	GetLinkedObjName(void)			{	return m_szObjName;				}

	void	SetLoop(int nLoopFlag)			{	SubData.m_bLoop = nLoopFlag;		}
	int		GetLoop(void)					{	return SubData.m_bLoop;				}

	void	SetVolume(float fVolume)		{	SubData.m_fVolume = fVolume;		}
	float	GetVolume(void)					{	return SubData.m_fVolume;			}

	void	SetStartTime(float fTime,DWORD dwTick)	
	{	
		SubData.m_fTime = fTime;		
		SubData.m_dwTick = dwTick;
	}
	float	GetStartTime(void)			{	return SubData.m_fTime;			}
	DWORD	GetStartTick(void)			{	return SubData.m_dwTick;		}

	void	SetDisable(int nSet)			{	SubData.m_bDisable = nSet;		}
	int		GetDisable(void)				{	return SubData.m_bDisable;		}

	void	SetTextID(int nID)			{	SubData.m_nTextID = nID;		}
	int		GetTextID(void)				{	return SubData.m_nTextID;		}
	
	void	SetUseVoiceCur(int nUse)	{	SubData.m_nUseVoiceCue = nUse;	}
	int		GetUseVoiceCue(void)		{	return SubData.m_nUseVoiceCue;	}

	bool	HasWave(void)				
	{	
		if( m_szFileName[0] != NULL )	
			return true;
		return false;
	}

	void		Save(BStream *pStream);
	void		Load(BStream *pStream);

	void		SaveEx(BStream *pStream);
	void		LoadEx(BStream *pStream);

	void	SetWavePath(char* szPath)
	{
		if( szPath == NULL || szPath[0] == NULL )
			memset( m_szFilePath , 0, eFileNameLength);
		else
			strcpy( m_szFilePath, szPath );
	}

	char*	GetWavePath(void)			{	return m_szFilePath;			}

#ifdef _USAGE_TOOL_
	void	ChangeFPS(void);
#endif

	void	GetKeyString(char* pszRetKey, const size_t pszRetKey_len); //aleksger - safe string

	void	SetRMObjPtr(DWORD dwObjPtr)	{	m_dwRMObjPtr = dwObjPtr;	}
	DWORD	GetRMObjPtr(void)			{	return m_dwRMObjPtr;		}


public :

	char		m_szName[eFileNameLength];
	char        m_szFileName[eFileNameLength]; // 배경음, 이펙트음

	// 추가
	char		m_szFilePath[eFileNameLength];	// 파일이 담겨 있는 패스.

	char		m_szObjName[eFileNameLength];
	// int         m_nObjectID; // 적용될 오브젝트의 인덱스 입니다 (0 .. m_Object.m_nFileMax-1 기준 입니다) = 해당 스킨

	struct  
	{
		D3DXVECTOR3 m_Xyz; // 사운드 포지션
		int         m_bLoop;
		float       m_fVolume;
		float       m_fTime;
		int         m_bDisable; // 이 값이 설정 되면 구동되지 않습니다
		DWORD		m_dwTick;
		int			m_nTextID;
		int			m_nUseVoiceCue;	// Lip인데 Wave로 등록할 경우. (0:일반, 1:Lip)
	} SubData;

	float       m_fFadeIn[eWave32]; // BGM 에서 주로 사용 될 것입니다
	float       m_fFadeOut[eWave32];

	// 이벤트용.
	int           m_nEvent; // 초기값 0, 이벤트 통보가 이미 되었다면 1 입니다

	// 추가. RMObject
	DWORD		m_dwRMObjPtr;
};

#pragma pack( pop )