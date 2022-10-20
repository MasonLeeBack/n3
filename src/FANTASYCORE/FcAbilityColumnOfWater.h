
#include "FcAbilityBase.h"
#include "CDefinition.h"

class CFcGameObject;
typedef CSmartPtr<CFcGameObject> GameObjHandle;

enum WaterType
{
	BIG_COLUMNWATER = 0,
	SMALL_COLUMNWATER,

	COUNT_COLUMNWATER
};
/*-----------------------------------------------------------------------------------
-
-				CFcAbilityColumnOfWater			Choi Jae Young
-												2005. 9. 26
-												v 0.02
-							In Blue-Side
-
-
-			note : 
-				  
-
------------------------------------------------------------------------------------*/
class CFcAbilityColumnOfWater : public CFcAbilityBase
{

public:

	virtual void					Process();												//  
	virtual void					Initialize();											//  
	virtual void					Finalize();												//  
	D3DXVECTOR3*					GetPosition()		{ return NULL; };




protected:

	void							DamageProcess(D3DXVECTOR3 *Pos,float *fPower);			// ���� �������� ���� ������ �߰��ϴ� �Լ�.
	void							CreateColumnWater( WaterType  p_nWaterType );




protected:

	int								m_nFxTemplateID[COUNT_COLUMNWATER];												
	int								m_nFxID;
	D3DXVECTOR3						m_vCharPos;
	D3DXVECTOR3						m_vCharDir;
	D3DXVECTOR3						m_vCharRight;
	//CDamageObjManager				m_DamageObjManager;										// ������ �޴���. 


	int	m_nSplashPtc;


public:
	CFcAbilityColumnOfWater(GameObjHandle hParent);
	virtual ~CFcAbilityColumnOfWater();
};
//************************************************************************************
//************************************************************************************