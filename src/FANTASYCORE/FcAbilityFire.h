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
-		METEORDEFINE	:	메테오의 일반적 속성에 대한 구조체 정의					-
-										2005.8.12 v.3								-
-																					-
-------------------------------------------------------------------------------------*/
struct METEORDEFINE
{
	float			_fSpeed;											// 메테오 속도.
	int				_UpHorizontalDist;									// 캐릭터 위치에서부터 떨어질 하늘의 반경 중심까지의 수평거리
	int				_nUpForwardDist;									// 메테오가 나타날 하늘의 전방 거리
	int				_DownForwardDist;									// 캐릭터 위치에서부터 떨어질 지상의 반경 중심까지의 전방거리
	int				_nUpRadius;											// 메테오가 나타날 하늘의 반경
	int				_nDownRadius;										// 떨어질 지상의 반경
	int				_nStartHeight;										// 메테오가 나타날 시작 높이
	int				_nInterval;											// 다음 메테오가 생성될 인터벌 Per Tick.
	int				_calledMeteorNum;									// 현재 메테오가 생성된 횟수.
	int				_byOnceBeComingNumber[2];							// 한번에 생성될 메테오 갯수.	0 : 최소갯수, 1 : 최대갯수
	int				_byMaxBeComingNumber;								// 메테오가 몇번 생성 되는가에대한 수. ( 메테오 최대갯수가 아님)
	float			_fMeteorDamage[2];									// 메테오의 데미지		0 : 범위,  1 : 크기
	float			_fPiceOfMeteorDamage[2];							// 메테오조각의 데미지		0 : 범위,  1 : 크기
	float			_fScale[2];											// 메테오 스케일		0 : 크기스케일 , 1 : 회전하면서 커질 스케일
	float			_fExplodeScale;										// 폭파 스케일
	float			_fRadius[2];										// 메테오 조각이 메테오 중심으로 부터 생성될 반경	0 : 최소 , 1 : 최대
	int				_nDeltaTime;										// 메테오 전체가 사라지는 타임 Per Tick
	int				_nDeltaTime2;										// 메테오 하나가 사라지는 타임 Per Tick
	float			_fTrailValue[3];									// Trail 값, life, width, epsilon
	float			_fPiceOfMeteorValue[5];								// Pice Of Meteor 의 value, 수평방향속도 Min, Max ,중력 , VolumeTex-AniSpd, Incidence( 100% => 안나옴 ) 
	float			_fSceneEft[2];										// 0 : SceneEftRadius	1 : SceneEft 강도.
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
-			note : 메테오를 관리하는 클래스. 매테오의 생성및 이동, 폭파타이밍을 결정.
-				   
-
------------------------------------------------------------------------------------*/
class CFcAbilityFire : public CFcAbilityBase
{
	
public:
	
	virtual void				Process();
	virtual void				Finalize();																	
	virtual void				Initialize(DWORD dwParam1 = 0, DWORD dwParam2 = 0);																	// 초기화 함수. Meteor.mete 파일을 읽어서 값을 세팅한다.
	void						ReadSOX();
	void						CallMeteor();

	D3DXVECTOR3* GetPosition() { return &m_vCastPoint; };	

	PRIVATE_GETSET_PROP(METEORDEFINE, Mete, m_sInfo);






private:

	std::vector<MeteorInfo*>		m_listMeteorInfo;
	D3DXVECTOR3						m_vCastPoint;																// 메테오가 나타날 위치
	D3DXVECTOR3						m_vCharPos;																	// 메테오가 시전될때의 캐릭터 위치
	D3DXVECTOR3						m_vCharDir;																	// 메테오가 시전될때의 캐릭터 전방벡터
	D3DXVECTOR3						m_vCharRight;																// 메테오가 시전될때의 캐릭터 측방벡터
	int								m_nDelTimer;																// 
	float							m_fMaxIntensity;
	float							m_nSceneIntensityBuffer;
	D3DXVECTOR4						m_vSceneIntensityBuffer;




public:
	CFcAbilityFire( GameObjHandle hParent );
	virtual ~CFcAbilityFire();

};
/*----------------------------------------------------------------------------------*/