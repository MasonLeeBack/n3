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
	char        m_szFileName[eFileNameLength]; // �����, ����Ʈ��

	// �߰�
	char		m_szFilePath[eFileNameLength];	// ������ ��� �ִ� �н�.

	char		m_szObjName[eFileNameLength];
	// int         m_nObjectID; // ����� ������Ʈ�� �ε��� �Դϴ� (0 .. m_Object.m_nFileMax-1 ���� �Դϴ�) = �ش� ��Ų

	struct  
	{
		D3DXVECTOR3 m_Xyz; // ���� ������
		int         m_bLoop;
		float       m_fVolume;
		float       m_fTime;
		int         m_bDisable; // �� ���� ���� �Ǹ� �������� �ʽ��ϴ�
		DWORD		m_dwTick;
		int			m_nTextID;
		int			m_nUseVoiceCue;	// Lip�ε� Wave�� ����� ���. (0:�Ϲ�, 1:Lip)
	} SubData;

	float       m_fFadeIn[eWave32]; // BGM ���� �ַ� ��� �� ���Դϴ�
	float       m_fFadeOut[eWave32];

	// �̺�Ʈ��.
	int           m_nEvent; // �ʱⰪ 0, �̺�Ʈ �뺸�� �̹� �Ǿ��ٸ� 1 �Դϴ�

	// �߰�. RMObject
	DWORD		m_dwRMObjPtr;
};

#pragma pack( pop )