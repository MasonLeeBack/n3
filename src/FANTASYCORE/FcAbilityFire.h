#pragma once
#include "FcAbilityBase.h"
#include "CDefinition.h"

class CFcGameObject;
typedef CSmartPtr<CFcGameObject> GameObjHandle;


struct MeteorInfo
{
	int						nFxIdx;
	int						nFxTrailIdx;
	float					fIntensity;
	BOOL					bExplode;
	D3DXVECTOR3				vPos;
	D3DXVECTOR3				vDir;
};

/*-----------------------------------------------------------------------------------
-																					-
-		METEORDEFINE	:	���׿��� �Ϲ��� �Ӽ��� ���� ����ü ����					-
-										2005.8.12 v.3								-
-																					-
-------------------------------------------------------------------------------------*/
struct METEORDEFINE
{
	float			_fSpeed;											// ���׿� �ӵ�.
	int				_UpHorizontalDist;									// ĳ���� ��ġ�������� ������ �ϴ��� �ݰ� �߽ɱ����� ����Ÿ�
	int				_nUpForwardDist;									// ���׿��� ��Ÿ�� �ϴ��� ���� �Ÿ�
	int				_DownForwardDist;									// ĳ���� ��ġ�������� ������ ������ �ݰ� �߽ɱ����� ����Ÿ�
	int				_nUpRadius;											// ���׿��� ��Ÿ�� �ϴ��� �ݰ�
	int				_nDownRadius;										// ������ ������ �ݰ�
	int				_nStartHeight;										// ���׿��� ��Ÿ�� ���� ����
	int				_nInterval;											// ���� ���׿��� ������ ���͹� Per Tick.
	int				_calledMeteorNum;									// ���� ���׿��� ������ Ƚ��.
	int				_byOnceBeComingNumber[2];							// �ѹ��� ������ ���׿� ����.	0 : �ּҰ���, 1 : �ִ밹��
	int				_byMaxBeComingNumber;								// ���׿��� ��� ���� �Ǵ°������� ��. ( ���׿� �ִ밹���� �ƴ�)
	float			_fMeteorDamage[2];									// ���׿��� ������		0 : ����,  1 : ũ��
	float			_fPiceOfMeteorDamage[2];							// ���׿������� ������		0 : ����,  1 : ũ��
	float			_fScale[2];											// ���׿� ������		0 : ũ�⽺���� , 1 : ȸ���ϸ鼭 Ŀ�� ������
	float			_fExplodeScale;										// ���� ������
	float			_fRadius[2];										// ���׿� ������ ���׿� �߽����� ���� ������ �ݰ�	0 : �ּ� , 1 : �ִ�
	int				_nDeltaTime;										// ���׿� ��ü�� ������� Ÿ�� Per Tick
	int				_nDeltaTime2;										// ���׿� �ϳ��� ������� Ÿ�� Per Tick
	float			_fTrailValue[3];									// Trail ��, life, width, epsilon
	float			_fPiceOfMeteorValue[5];								// Pice Of Meteor �� value, �������ӵ� Min, Max ,�߷� , VolumeTex-AniSpd, Incidence( 100% => �ȳ��� ) 
	float			_fSceneEft[2];										// 0 : SceneEftRadius	1 : SceneEft ����.
	int				_byUseSceneeffect;									// 0 : SceneIntensity	1 : SceneAddcolor	2 : nothing
	METEORDEFINE()
	{
		memset( this, 0, sizeof(METEORDEFINE));
	}
};
/*------------------------------------------------------------------------------------*/






/*-----------------------------------------------------------------------------------
-
-				CFcAbilityFire					Choi Jae Young
-												2005. 8. 5
-												v 0.01
-							In Blue-Side
-
-
-			note : ���׿��� �����ϴ� Ŭ����. ���׿��� ������ �̵�, ����Ÿ�̹��� ����.
-				   
-
------------------------------------------------------------------------------------*/
class CFcAbilityFire : public CFcAbilityBase
{
	
public:
	
	virtual void				Process();
	virtual void				Finalize();																	
	virtual void				Initialize(DWORD dwParam1 = 0, DWORD dwParam2 = 0);																	// �ʱ�ȭ �Լ�. Meteor.mete ������ �о ���� �����Ѵ�.
	void						ReadSOX();
	void						CallMeteor();

	D3DXVECTOR3* GetPosition() { return &m_vCastPoint; };	

	PRIVATE_GETSET_PROP(METEORDEFINE, Mete, m_sInfo);






private:

	std::vector<MeteorInfo*>		m_listMeteorInfo;
	D3DXVECTOR3						m_vCastPoint;																// ���׿��� ��Ÿ�� ��ġ
	D3DXVECTOR3						m_vCharPos;																	// ���׿��� �����ɶ��� ĳ���� ��ġ
	D3DXVECTOR3						m_vCharDir;																	// ���׿��� �����ɶ��� ĳ���� ���溤��
	D3DXVECTOR3						m_vCharRight;																// ���׿��� �����ɶ��� ĳ���� ���溤��
	int								m_nDelTimer;																// 
	float							m_fMaxIntensity;
	float							m_nSceneIntensityBuffer;
	D3DXVECTOR4						m_vSceneIntensityBuffer;




public:
	CFcAbilityFire( GameObjHandle hParent );
	virtual ~CFcAbilityFire();

};
/*----------------------------------------------------------------------------------*/