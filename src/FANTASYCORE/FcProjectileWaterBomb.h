#pragma once

#include "FcProjectile.h"
#include "ASData.h"


class CFcProjectileWaterBomb : public CFcProjectile
{
public:
	CFcProjectileWaterBomb( CCrossVector *pCross );
	virtual ~CFcProjectileWaterBomb();

	static ASSignalData *s_pHitSignal;

	void SetDestoryTick( int nTick );
	int GetDestoryTick() { return m_nDestroyTick;};
protected:

	int m_nDestroyTick;
	int	m_nExpFxId;
	int m_nExpFxTemplate;
	float m_fScale;

	bool m_bOnce;

	D3DXVECTOR3 m_PrevPos;

	IBsPhysicsActor	*m_pActor;


	void ProcessDamage();
	void ProcessSplashDamage();

	void AddForce(D3DXVECTOR3* vForce) 
	{
		if(m_pActor && vForce)
			m_pActor->AddVelocity( vForce , NULL);		
	};

	void AddForce2(D3DXVECTOR3* vForce) 
	{
		if(m_pActor && vForce)
			m_pActor->AddVelocity( NULL , vForce);		
	};

	
	void SetScale( float p_f ) { m_fScale = p_f;	};

	float m_fAddScale;
	float GetAddScale() { return m_fAddScale;	};
	void SetAddScale( float p_f ) { m_fAddScale = p_f;	};
	int m_nCurrentTick;
	int m_nTotalTick;
	int GetTotalTick() { return m_nTotalTick;	};
	void SetTotalTick( int p_n ) { m_nTotalTick = p_n; 	};

	int m_nClassify;
	int GetClassify() { return m_nClassify;	};
	void SetClassify( int p_n ) { m_nClassify = p_n; 	};

public:
	void SetCustomValue( void* p_pValue );
	float GetScale() { return m_fScale;	};

public:
	virtual int Initialize( int nSkinIndex, PROJECTILE_PARAM *pParam );
	virtual void Process();

};