#pragma once

#include "BsRealMovieConst.h"

#pragma pack( push )
#pragma pack( 4 )

class CRMObject;

// ǥ�� �ִϸ��̼� ����Ÿ�� ���� �մϴ� 
class CRMFace 
{
public :
	enum 
	{ 
		eFileNameLength=64,
	};	// String �� �� ������ ũ��.

	CRMFace()
	{
		SubData.m_nAniIndex = 0;
		SubData.m_fTime = -1.f;
		SubData.m_fUseTime = -1.f;

		SubData.m_fComplement = 1.f;
		SubData.m_fCurIndex = 0.f;
		SubData.m_dwTick = -1;
		SubData.m_nUseFrame = -1;
	};
	~CRMFace(){};

	void	ResetAniCursor(void)	{	SubData.m_fCurIndex = 0.f;	}
	float	GetAniCursor(void)		{	return SubData.m_fCurIndex;	}
	void	IncAniCursor(void)		{	SubData.IncCurIndex();		}

	void	SetName(char *szName)	{	strcpy( m_szName, szName );	}
	void	SetAniIndex(int nIdx)	
	{	
		SubData.m_nAniIndex = nIdx;
		RecalcComplement();
	}

	void	SetStartTime(DWORD dwTick);
	void	SetStartTime(float fTime,DWORD dwTick)	
	{	
		SubData.m_fTime = fTime;		
		SubData.m_dwTick = dwTick;
	}
	void	SetUseTime(float fTime)		
	{	
		SubData.m_fUseTime = fTime;		
		SubData.m_nUseFrame = (int)(fTime * eFps40 );
	}

	void	SetUseFrame(int nFrame)		
	{	
		SubData.m_nUseFrame = nFrame;
		SubData.m_fUseTime = ( (float)nFrame / eFps40 );
	}

	void	SetComplement(float fTime);

	char*	GetName(void)			{	return m_szName;			}
	int		GetAniIndex(void)		{	return SubData.m_nAniIndex;			}
	float	GetStartTime(void)		{	return SubData.m_fTime;			}
	DWORD	GetStartTick(void)		{	return SubData.m_dwTick;		}
	float	GetUseTime(void)		{	return SubData.m_fUseTime;			}
	int		GetUseFrame(void)		{	return SubData.m_nUseFrame;			}
	float	GetComplement(void)		{	return SubData.m_fComplement;		}

	void	RecalcComplement(void);


	CRMObject*	GetParentObject(void)	{	return m_pParent;			}

	void		Save(BStream *pStream);
	void		Load(BStream *pStream);

#ifdef _USAGE_TOOL_
	void	ChangeFPS(void);
#endif


public :

	char   m_szName[eFileNameLength];

	int    m_nObjectID; // ����� ������Ʈ�� �ε��� �Դϴ� (0 .. m_Object.m_nFileMax-1 ���� �Դϴ�) = �ش� ��Ų

	struct 
	{
		int    m_nAniIndex; // �ش� ǥ�� �ִ��� �ε��� �Դϴ� (0(����ũ) 1..(�ִ�,ǥ��)) = 1 �̻��� ���Դϴ�, �ش� BA �� �ټ��� �ִϸ� �����ϰ� ���� ���Դϴ�, �ش� BA �� �����ϴ� �ִ� �ε����� ���� ������ �ȵ˴ϴ�
		float  m_fTime;		// ���� Ÿ��.

		float  m_fUseTime;		// ��� �ð�.
		float  m_fComplement;	// �߰� ������.

		//
		float  m_fAniLen;
		float  m_fAniInc;		// �ִ� ������ = ( AniLength - 1 ) / m_fComplement
		float  m_fCurIndex;		// �ִϰ�.

		DWORD	m_dwTick;
		int 	m_nUseFrame;

		void	IncCurIndex(void)
		{
			/*
			m_fCurIndex = 
			( m_fCurIndex < m_fAniLen ) ? m_fCurIndex + m_fAniInc :	m_fAniLen ;
			*/
			m_fCurIndex += m_fAniInc;

			if( m_fCurIndex > m_fAniLen )
				m_fCurIndex = m_fAniLen;
			else if( m_fCurIndex < 0.f )
				m_fCurIndex = 0.f;
		}

	} SubData;

	CRMObject*	m_pParent;
};

#pragma pack( pop )