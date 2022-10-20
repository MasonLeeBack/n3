#pragma once

#pragma pack( push )
#pragma pack( 4 )

class CRMObject;

// �Ը�� �ִϸ��̼� ����Ÿ�� ���� �մϴ� 
class CRMLip 
{
public :
	enum 
	{ 
		eLipPackSize = 4 , 
		eFileNameLength=64,
	};		// String �� �� ������ ũ��.

	CRMLip() : m_nEvent(0)
	{
		SubData.m_fTime = -1.f;
		SubData.m_dwTick = -1;
		SubData.m_nTextID = -1;
		memset( m_szLTFFileName, 0, eFileNameLength );
		memset( m_szWAVFileName, 0, eFileNameLength );
		memset( m_szFilePath , 0, eFileNameLength );
	};
	~CRMLip(){};

	void	ResetEventFlag(void)	{	m_nEvent = 0;				}

	BOOL	ChangeDataFolder(VOID);
	BOOL	LoadLipResource(void);

	void	SetName(char *szName)	{	strcpy( m_szName, szName );	}
	void	SetLTFFileName(char *szFile)	{	strcpy( m_szLTFFileName, szFile );	}
	void	SetWavFileName(char *szFile)	{	strcpy( m_szWAVFileName, szFile );	}
	void	SetStartTime(float fTime,DWORD dwTick)	
	{	
		SubData.m_fTime = fTime;		
		SubData.m_dwTick = dwTick;
	}

	void	SetStartTick(DWORD dwTick)
	{
		SubData.m_dwTick = dwTick;
		SubData.m_fTime = (float)dwTick / 40.f;
	}

	char*	GetName(void)			{	return m_szName;			}
	char*	GetLTFFileName(void)			{	return m_szLTFFileName;			}
	BOOL	HasLTFFileName(void)	{	return ( m_szLTFFileName[0] != NULL ) ? TRUE : FALSE;	}
	char*	GetWavFileName(void)			{	return m_szWAVFileName;			}
	float	GetStartTime(void)			{	return SubData.m_fTime;			}
	DWORD	GetStartTick(void)			{	return SubData.m_dwTick;		}

	CBsFileLTF&		GetLTFObject()		{	return m_LTF;			}
	CRMObject*	GetParentObject(void)	{	return m_pParent;			}

	void	GetKeyString(char* pszRetKey, const size_t pszRetKey_len); //aleksger - safe string
	bool	HasWave(void);

	void		Save(BStream *pStream);
	void		Load(BStream *pStream);

	void		SaveEx(BStream *pStream);
	void		LoadEx(BStream *pStream);

	void	SetLipPath(char* szPath)
	{
		if( szPath == NULL || szPath[0] == NULL )
			memset( m_szFilePath , 0, eFileNameLength);
		else
			strcpy( m_szFilePath, szPath );
	}

	char*	GetLipPath(void)			{	return m_szFilePath;			}

	void	SetTextID(int nID)			{	SubData.m_nTextID = nID;		}
	int		GetTextID(void)				{	return SubData.m_nTextID;		}

#ifdef _USAGE_TOOL_
	void	ChangeFPS(void);
#endif


public :

	char   m_szName[eFileNameLength];
	char   m_szLTFFileName[eFileNameLength]; // ����ũ ���� (������ ���� ������� ����)
	char   m_szWAVFileName[eFileNameLength]; // ���� ����

	char	m_szFilePath[eFileNameLength];	// �� ������ ��� �ִ� �н�.

	struct
	{
		float  m_fTime;
		DWORD  m_dwTick;
		int	   m_nTextID;

	} SubData;

	CBsFileLTF    m_LTF;
	int           m_nEvent; // �ʱⰪ 0, �̺�Ʈ �뺸�� �̹� �Ǿ��ٸ� 1 �Դϴ�
	LARGE_INTEGER m_liStartTime;


	CRMObject*	m_pParent;
};


#pragma pack( pop )