#pragma once

struct UnitDataInfo;
class ASData;
class CAniInfoData;
class CFcGameObject;

class CFcUnitJobChange {
public:
	CFcUnitJobChange();
	virtual ~CFcUnitJobChange();

	void Initialize( CFcGameObject *pParent, int nSubJobSOXIndex );

protected:
	CFcGameObject *m_pParent;
//	GameObjHandle m_hHandle;
	bool m_bSubJob;
	int m_nSubUnitSOXIndex;
	UnitDataInfo	*m_pSubUnitSOX;
	ASData			*m_pSubASData;
	CAniInfoData	*m_pSubUnitInfoData;
	int m_nSubEngineIndex;

	int m_nPriEngineIndex;
	ASData			*m_pPriASData;
	CAniInfoData	*m_pPriUnitInfoData;

//	int m_nPriSkinIndex;
//	int m_nPriAniIndex;

public:
	void ChangeJob( bool bSubJob );
	bool IsSubJob();

};