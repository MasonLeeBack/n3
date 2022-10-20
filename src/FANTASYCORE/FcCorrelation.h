#pragma once

//#include "FcGameObject.h"
#include "FcCommon.h"

typedef CSmartPtr<CFcGameObject> GameObjHandle;

class CFcCorrelationMng;
class CFcCorrelation {
public:
	enum CORRELATION_TYPE {
		CT_NONE = 0x00,
		CT_TARGETING_COUNTER = 0x01,
		CT_HIT = 0x02,
	};

protected:
	CFcCorrelation( CFcCorrelationMng *pMng ) {	
		m_Type = CT_NONE;
		m_pMng = pMng;	
	}

public:
	virtual ~CFcCorrelation() { m_pMng = NULL; }

	virtual void Influence( void *pValue ) {}
	virtual void Reset() {}
	virtual void *GetResult() { return NULL; }

	CORRELATION_TYPE GetType() { return m_Type; }


protected:
	CORRELATION_TYPE m_Type;
	CFcCorrelationMng *m_pMng;
};

class CFcCorrelationTargeting : public CFcCorrelation 
{
public:
	CFcCorrelationTargeting( CFcCorrelationMng *pMng );
	virtual ~CFcCorrelationTargeting();

	struct ParamStruct {
		GameObjHandle *pHandle;
		bool bAddRemoveFlag;
	};

protected:
	std::vector<GameObjHandle> m_VecHandle;
	int m_nTotalCount;

public:
	virtual void Influence( void *pValue );
	virtual void Reset();
	virtual void *GetResult();
	// юс╫ц..
	std::vector<GameObjHandle> *GetList() { return &m_VecHandle; }

	void InfluenceTargeting( GameObjHandle *pHandle, bool bAddRemoveFlag );
};

class CFcCorrelationHit : public CFcCorrelation
{
public:
	CFcCorrelationHit( CFcCorrelationMng *pMng );
	virtual ~CFcCorrelationHit();

	struct ParamStruct {
		GameObjHandle *pHandle;
		int nDamage;
	};

	struct HitStruct {
		GameObjHandle Handle;
		int nDamage;
		int nCount;
	};
protected:
	std::vector<HitStruct *> m_pVecHit;
	int m_nLastHit;

public:
	virtual void Influence( void *pValue );
	virtual void Reset();
	virtual void *GetResult();

	HitStruct *FindResultFromHandle( GameObjHandle &Handle );

	void InfluenceHit( GameObjHandle *pHandle, int nDamage );

};

class CFcCorrelationMng {
public:
	CFcCorrelationMng( GameObjHandle hParent, int Type );
	virtual ~CFcCorrelationMng();

	CFcCorrelation *GetCorrelation( CFcCorrelation::CORRELATION_TYPE Type );

protected:
	GameObjHandle m_hParent;
	std::vector<CFcCorrelation *> m_pVecCorrelation;
	int *m_nIndexTable;

	CFcCorrelation *FindCorrelation( CFcCorrelation::CORRELATION_TYPE Type );

public:
	virtual void Influence( CFcCorrelation::CORRELATION_TYPE Type, void *pValue );
	virtual void Reset( CFcCorrelation::CORRELATION_TYPE Type );
	virtual void *GetResult( CFcCorrelation::CORRELATION_TYPE Type );
	GameObjHandle &GetParent() { return m_hParent; }
};
