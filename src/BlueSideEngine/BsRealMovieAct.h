#pragma once

#pragma pack( push )
#pragma pack( 4 )

class CRMObject;

// 동작 애니메이션 데이타를 수록 합니다 
class CRMAct 
{
public :
	enum 
	{ 
		eActPackSize = 8,		// String 을 뺀 나머지 크기.
		eFileNameLength=64,
	};

	CRMAct() 
	{
		//SubData.m_fTime = -1.f;
		//SubData.m_nAniIndex = 0;
	};
	~CRMAct(){};

	void	Clear(void);

	void	SetName(char *szName)	{	strcpy( m_szName, szName );	}
	void	SetAniIndex(int nIdx)	{	SubData.m_nAniIndex = nIdx;			}

	void	SetStartTime(DWORD dwTick);
	void	SetStartTime(float fTime,DWORD dwTick)	
	{	
		SubData.m_fTime = fTime;		
		SubData.m_dwTick = dwTick;
	}
	void	SetRepeat(int nVal)		{	SubData.m_nRepeat = nVal;		}
	void	SetStartFrame(int nVal)	{	SubData.m_nStartFrame = nVal;	}
	void	SetEndFrame(int nVal)	{	SubData.m_nEndFrame = nVal;		}

	char*	GetName(void)			{	return m_szName;				}
	int		GetAniIndex(void)		{	return SubData.m_nAniIndex;		}
	float	GetStartTime(void)		{	return SubData.m_fTime;			}
	DWORD	GetStartTick(void)		{	return SubData.m_dwTick;		}
	int		GetRepeat(void)			{	return SubData.m_nRepeat;		}
	int		GetStartFrame(void)		{	return SubData.m_nStartFrame;	}
	int		GetEndFrame(void)		{	return SubData.m_nEndFrame;		}

	CRMObject*	GetParentObject(void)	{	return m_pParent;			}

	int		GetAniLength(void);
	int		GetFrameSize(void);
	BOOL	RecalcAniFrameSize(void);

	void		Save(BStream *pStream);
	void		Load(BStream *pStream);

#ifdef _USAGE_TOOL_
	void	ChangeFPS(void);
#endif

public :

	char   m_szName[eFileNameLength];

	struct CRMActData
	{
		int    m_nAniIndex; // 해당 표정 애니의 인덱스 입니다 (0(립싱크) 1..(애니,표정)) = 1 이상의 값입니다, 해당 BA 는 다수의 애니를 포함하고 있을 것입니다, 해당 BA 가 제공하는 애니 인덱스를 벗어 나서는 안됩니다
		float  m_fTime;
		int    m_nRepeat;	// 반복 횟수.

		// 내부 변수들.
		float	m_fBlendTime;	// Blend 들어갈 타임.

		DWORD  m_dwTick;

		// 추가됨.
		int		m_nStartFrame;	// 시작 인덱스.
		int		m_nEndFrame;		// 끝 인덱스.

		CRMActData()
		{
			m_nAniIndex = 0;
			m_fTime = -1.f;
			m_nRepeat = 1;
			m_dwTick = -1;

			m_nStartFrame = 0;
			m_nEndFrame = 0;
		}
	} SubData;



	CRMObject*	m_pParent;
};

#pragma pack( pop )
