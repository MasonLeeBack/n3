#pragma once
#include "FcBaseObject.h"


#define SPIRIT_MODE_UP		0
#define SPIRIT_MODE_WAIT	1
#define SPIRIT_MODE_ABSORB	2

#define ORB_TYPE_NORMAL		0
#define ORB_TYPE_TRUE		1

class CFcHeroObject;
typedef CSmartPtr< CFcHeroObject > HeroObjHandle;

/*-----------------------------------------------------------------------------------
-
-				CSpritObject					Choi Jae Young
-												2005. 9. 12
-												v 0.01
-							In Blue-Side
-
-
-			note : ȥ Ŭ����. 
-				   
-
------------------------------------------------------------------------------------*/
class CSpritObject : public CFcBaseObject
{

public:

	bool					Render();
	void					Process();
	int						Initialize( int nParticleGroupIndex, D3DXVECTOR3 *pPosition, int nType, 
		HeroObjHandle AbsorbTarget, int nGenerateOrbType );





protected:

	bool					m_bIsBig;															// ū �������� ���� ���������� flag
	int						m_nTrailIndex;														// ���겿��
	int						m_nHaleEffIndex;													// ���갡 ����ɶ� ��Ÿ�� ����Ʈ
	int						m_nOrbType;															// �Ķ����� ���� ���� ��������
	int						m_nAddOrb;															// ������ ������ ��
	int						m_nMode;															// ȥ�� ������ ���� ( �ö󰡴»���, ��������, �ٰ����»��� )
	int						m_nFxIndex;
	float 					m_fSpeed;															// �̵� �ӵ�					
	D3DXVECTOR3 			m_DirVector;														
	D3DXVECTOR3 			m_Position;
	HeroObjHandle			m_AbsorbTarget;														// ȥ�� �ٰ��� ����
	





public:

	CSpritObject( CCrossVector *pCross );
	virtual ~CSpritObject(void);

};


typedef CSmartPtr< CSpritObject > OrbObjHandle;

//*********************************************************************************************
//*********************************************************************************************