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
-			note : 혼 클래스. 
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

	bool					m_bIsBig;															// 큰 오브인지 작은 오브인지의 flag
	int						m_nTrailIndex;														// 오브꼬리
	int						m_nHaleEffIndex;													// 오브가 흡수될때 나타날 이펙트
	int						m_nOrbType;															// 파란오브 인지 붉은 오브인지
	int						m_nAddOrb;															// 더해질 오브의 양
	int						m_nMode;															// 혼의 움직임 상태 ( 올라가는상태, 정지상태, 다가가는상태 )
	int						m_nFxIndex;
	float 					m_fSpeed;															// 이동 속도					
	D3DXVECTOR3 			m_DirVector;														
	D3DXVECTOR3 			m_Position;
	HeroObjHandle			m_AbsorbTarget;														// 혼이 다가갈 영웅
	





public:

	CSpritObject( CCrossVector *pCross );
	virtual ~CSpritObject(void);

};


typedef CSmartPtr< CSpritObject > OrbObjHandle;

//*********************************************************************************************
//*********************************************************************************************