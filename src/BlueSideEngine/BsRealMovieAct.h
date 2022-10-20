#pragma once

#pragma pack( push )
#pragma pack( 4 )

class CRMObject;

// ���� �ִϸ��̼� ����Ÿ�� ���� �մϴ� 
class CRMAct 
{
public :
	enum 
	{ 
		eActPackSize = 8,		// String �� �� ������ ũ��.
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
		int    m_nAniIndex; // �ش� ǥ�� �ִ��� �ε��� �Դϴ� (0(����ũ) 1..(�ִ�,ǥ��)) = 1 �̻��� ���Դϴ�, �ش� BA �� �ټ��� �ִϸ� �����ϰ� ���� ���Դϴ�, �ش� BA �� �����ϴ� �ִ� �ε����� ���� ������ �ȵ˴ϴ�
		float  m_fTime;
		int    m_nRepeat;	// �ݺ� Ƚ��.

		// ���� ������.
		float	m_fBlendTime;	// Blend �� Ÿ��.

		DWORD  m_dwTick;

		// �߰���.
		int		m_nStartFrame;	// ���� �ε���.
		int		m_nEndFrame;		// �� �ε���.

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
