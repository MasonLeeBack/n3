#pragma once

#pragma pack( push )
#pragma pack( 4 )

class CRMObject;

class CRMBone
{
public :

	enum 
	{ 
		eBoneMax = 4,
		eFileNameLength=64,
	};	// String 을 뺀 나머지 크기.

	CRMBone() : m_pParent( NULL )
	{
		SubData.m_nBoneIndex = -1;
		SubData.m_fRotX = 0.f;
		SubData.m_fRotY = 0.f;
		SubData.m_fRotZ = 0.f;
		SubData.m_fTime = -1.f;
		SubData.m_fUseTime = -1.f;
		SubData.m_dwTick = -1;

		memset( m_szBone, 0, eBoneMax * eFileNameLength );
		memset( m_bUseBone , 0, eBoneMax * sizeof(BOOL) );
	}

	~CRMBone(){};

	void	SetName(char *szName)	{	strcpy( m_szName, szName );	}
	void	SetBoneIndex(int nIdx)	{	SubData.m_nBoneIndex = nIdx;		}
	void	SetRotX(float fVal)		{	SubData.m_fRotX = fVal;				}
	void	SetRotY(float fVal)		{	SubData.m_fRotY = fVal;				}
	void	SetRotZ(float fVal)		{	SubData.m_fRotZ = fVal;				}
	void	SetStartTime(float fTime,DWORD dwTick)
	{	
		SubData.m_fTime = fTime;			
		SubData.m_dwTick = dwTick;
	}

	void	SetUseTime(float fTime)	{	SubData.m_fUseTime = fTime;			}

	void	SetBone(int nIndex,char* szName)
	{
		if( szName == NULL || szName[0] == NULL )
			memset( m_szBone[nIndex] , 0, eFileNameLength);
		else
			strcpy( m_szBone[nIndex], szName );
	}

	void	SetBoneUse(int nIndex,BOOL bSet)
	{
		m_bUseBone[ nIndex ] = bSet;
	}

	char*	GetName(void)			{	return m_szName;			}
	int		GetBoneIndex(void)		{	return SubData.m_nBoneIndex;		}
	float	GetRotX(void)			{	return SubData.m_fRotX;				}
	float	GetRotY(void)			{	return SubData.m_fRotY;				}
	float	GetRotZ(void)			{	return SubData.m_fRotZ;				}
	float	GetStartTime(void)		{	return SubData.m_fTime;				}
	DWORD	GetStartTick(void)		{	return SubData.m_dwTick;			}
	float	GetUseTime(void)		{	return SubData.m_fUseTime;			}
	char*	GetBone(int nIndex)
	{
		return m_szBone[nIndex];
	}
	BOOL	GetBoneUse(int nIndex)	{	return m_bUseBone[ nIndex ];		}

	CRMObject*	GetParentObject(void)	{	return m_pParent;			}
	void		Save(BStream *pStream);
	void		Load(BStream *pStream);

	void		SaveEx(BStream *pStream);
	void		LoadEx(BStream *pStream);

#ifdef _USAGE_TOOL_
	void	ChangeFPS(void);
#endif


public :

	char   m_szName[eFileNameLength];

	struct 
	{
		int		m_nBoneIndex;
		float	m_fRotX;
		float	m_fRotY;
		float	m_fRotZ;

		float	m_fTime;
		float	m_fUseTime;
		DWORD	m_dwTick;
	} SubData;

	char	m_szBone[eBoneMax][eFileNameLength];
	BOOL	m_bUseBone[eBoneMax];


	CRMObject*	m_pParent;
};

#pragma pack( pop )