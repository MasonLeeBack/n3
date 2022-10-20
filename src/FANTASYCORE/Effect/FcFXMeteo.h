#pragma once

#include "FcFXBase.h"
#include "CrossVector.h"


/*-----------------------------------------------------------------------------------
-
-				CFcFXMeteor						Choi Jae Young
-												2005. 8. 11 ( ���� ������ )
-												v 0.01
-							In Blue-Side
-
-
-			note : ���׿� Ŭ����. ���׿��� ���� �������� �̵��� ����, Ʈ���� �������� ó���� �Ѵ�.
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
	int					m_nPiceOfMeteor;																// CPiceOfMeteor �� �ڵ�
	float				m_fPiceOfMeteorDamage[2];														// ���׿����� ������ �ݰ� �� �Ŀ�
	float				m_fScale[2];																	// 0 : ũ�� 1 : ȸ��ũ��
	float				m_fPiceOfMeteorRadius[2];														// ���׿� ������ ���׿� �߽����κ��� ������ ��ġ Min , Max
	float				m_fSceneValue[2];																// ȭ�� ����Ʈ �� 0 : �ݰ� 1 : ����
	float				m_fVel;
	float				m_fFXScale;
	float				m_fTrailValue[3];																// ���� �߰��� ���� trail�� ���� �����ϱ� ���� �� (life, width, epsilon)
	float				m_fPiceOfMeteorValue[5];														// PiceOfMeteor �� ���� ( ��������� �ӵ� Min, Max  ���������� �ӵ� Max .. min�� 0 )
	float				m_fIntensity;
	DWORD				m_dwBlendValue[3];																// PiceOfMeteor�� ���� �μ� ��
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
-			note : ���׿��� ���� Ŭ����. ���� CFcFXLineTrail Ŭ������ �����Ѵ�.
-				   
-
------------------------------------------------------------------------------------*/
struct	STrail									// Pice Of Meteor �������� ����ü.
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
struct SShock									// ���׿� ������ �ٴڿ� �������� ��Ÿ�� ���߹� ����ü.
{
	int			_nHandle;
	D3DXVECTOR3 _vPos;
	D3DXVECTOR4 _ScaleRot;						// x,y,z => ������ w => ������Ʈ��
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
	void									AddTrail( STrail* p_pTrail );										// Metero �߰�
	void									AddShock( SShock* p_pShock );										// Shock �߰�
	void									DeleteTrail( STrail* p_pTrail );									// Metero ����
	void									DeleteShock( SShock* p_pShock );									// Shock ����
	void									Clear();															// ��� ��������, ������ ����Ѵ�.
	


private:

	std::list<STrail*>						m_listTrail;														// PiceOfMeteor �� ������ ������ �ִ�.
	std::list<SShock*>						m_listShock;														// PiceOfMeteor �� ���� ������ ������ �ִ�.
	int										m_nParentState;														// �θ� ������ �ڽ��� ���� �׸��� �ִ°��� ��� �׸��� �״´�.
	float									m_fGravity;															// �߷°��ӵ� ( Per Tick )
	float									m_fPiceOfMeteorDamage[2];
	float									m_fPiceOfMeteorRadius[2];
	STrail*									m_ptmpTrail;														// List�� �ֱ����� ���� �����ϱ� ���Ͽ�.
	D3DXVECTOR3								m_tmpDirVec;
	
	static int	m_nCount;
public:
	CPieceOfMeteor();
	virtual ~CPieceOfMeteor();
};

//----------------------------------------------------------------------------------


