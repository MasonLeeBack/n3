#pragma once

#include "FcFXBase.h"
#include "CrossVector.h"


/*-----------------------------------------------------------------------------------
-
-				CFcFXMeteor						Choi Jae Young
-												2005. 8. 11 ( 단지 수정함 )
-												v 0.01
-							In Blue-Side
-
-
-			note : 메테오 클래스. 메테오에 대한 전반적인 이동및 폭발, 트레일 생성등의 처리를 한다.
-				   
-
------------------------------------------------------------------------------------*/

class CFcFXMeteor : public CFcFXBase
{
public:

	void				Process();
	void				Render( C3DDevice *pDevice );
	void				SetPos( D3DXVECTOR3* pvDir, D3DXVECTOR3* pvPos, float* fVel );
	int					ProcessMessage( int nCode,DWORD dwParam1 = 0, DWORD dwParam2 = 0, DWORD dwParam3 = 0 );
	virtual void		PreRender();
	void 				InitDeviceData();
	void 				ReleaseDeviceData();






private:

	void				GenBaby();
	virtual void		Explode();
	virtual void		Initialize( float fMaxScale );






private:

	int					m_nPGId[4];
	int					m_nDelTimer;
	int					m_nSavedDelTimer;
	int					m_nPiceOfMeteor;																// CPiceOfMeteor 의 핸들
	float				m_fPiceOfMeteorDamage[2];														// 메테오조각 데미지 반경 및 파워
	float				m_fScale[2];																	// 0 : 크기 1 : 회전크기
	float				m_fPiceOfMeteorRadius[2];														// 메테오 조각이 메테오 중심으로부터 생성될 위치 Min , Max
	float				m_fSceneValue[2];																// 화면 이펙트 값 0 : 반경 1 : 강도
	float				m_fVel;
	float				m_fFXScale;
	float				m_fTrailValue[3];																// 새로 추가된 변수 trail의 값을 세팅하기 위한 값 (life, width, epsilon)
	float				m_fPiceOfMeteorValue[5];														// PiceOfMeteor 값 세팅 ( 수평방향의 속도 Min, Max  수직방향의 속도 Max .. min은 0 )
	float				m_fIntensity;
	DWORD				m_dwBlendValue[3];																// PiceOfMeteor의 블랜드 인수 값
	D3DXVECTOR3			m_vDir;
	D3DXMATRIX			m_ScaleMat;
	CCrossVector		m_Cross;
	class CBsFXObject*	m_pFXObject[4];
	






public :
	CFcFXMeteor();
	virtual ~CFcFXMeteor();
};

//-----------------------------------------------------------------------------------









/*-----------------------------------------------------------------------------------
-
-				CPieceOfMeteor					Choi Jae Young
-												2005. 8. 5
-												v 0.01
-							In Blue-Side
-
-
-			note : 메테오의 조각 클래스. 단지 CFcFXLineTrail 클래스를 관리한다.
-				   
-
------------------------------------------------------------------------------------*/
struct	STrail									// Pice Of Meteor 정보저장 구조체.
{
	D3DXVECTOR3 _vPos;
	D3DXVECTOR3 _vDir;
	int			_nTrail;
	int			_nLife;
	float		_Speed;
	float		_fAniSpd;
	STrail()
	{
		memset(this, 0, sizeof(STrail));
	}
};	
struct SShock									// 메테오 조각이 바닥에 떨어질때 나타날 폭발물 구조체.
{
	int			_nHandle;
	D3DXVECTOR3 _vPos;
	D3DXVECTOR4 _ScaleRot;						// x,y,z => 스케일 w => 로테이트값
	SShock()
	{
		memset(this, 0, sizeof(SShock));
	}
};
class CPieceOfMeteor : public CFcFXBase
{
public:
	void									InitDeviceData();
	void									PreRender();
	void									Process();
	void									Render(C3DDevice *pDevice);											
	int										ProcessMessage(int nCode,DWORD dwParam1=0, DWORD dwParam2=0,DWORD dwParam3=0);



private:

	void									ProcessShock();
	void									Initialize( D3DXVECTOR3 p_vMeteorPos, D3DXVECTOR3 p_vMeteorDir, float p_fMeteorSpd );
	void									AddTrail( STrail* p_pTrail );										// Metero 추가
	void									AddShock( SShock* p_pShock );										// Shock 추가
	void									DeleteTrail( STrail* p_pTrail );									// Metero 삭제
	void									DeleteShock( SShock* p_pShock );									// Shock 삭제
	void									Clear();															// 모두 삭제한후, 삭제를 대기한다.
	


private:

	std::list<STrail*>						m_listTrail;														// PiceOfMeteor 의 정보를 가지고 있다.
	std::list<SShock*>						m_listShock;														// PiceOfMeteor 의 폭발 정보를 가지고 있다.
	int										m_nParentState;														// 부모가 죽으면 자식은 현재 그리고 있는것을 모두 그린후 죽는다.
	float									m_fGravity;															// 중력가속도 ( Per Tick )
	float									m_fPiceOfMeteorDamage[2];
	float									m_fPiceOfMeteorRadius[2];
	STrail*									m_ptmpTrail;														// List에 넣기전에 값을 세팅하기 위하여.
	D3DXVECTOR3								m_tmpDirVec;
	
	static int	m_nCount;
public:
	CPieceOfMeteor();
	virtual ~CPieceOfMeteor();
};

//----------------------------------------------------------------------------------


