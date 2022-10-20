#pragma once

#include "CrossVector.h"
#include "FcFxTidalWave.h"
#include "FcAbilityBaseObject.h"

class CFcGameObject;
typedef CSmartPtr<CFcGameObject> GameObjHandle;

/*class CDamageObj
{
public:
	CDamageObj(){};
	void Process();
	GameObjHandle GetObjHandle(){return m_ObjHandle;}
	void SetObjHandle(GameObjHandle Handle){m_ObjHandle = Handle;}
	void SetDir(D3DXVECTOR3 *pDir){m_Dir = *pDir;}
protected:
	GameObjHandle m_ObjHandle;
	D3DXVECTOR3 m_Dir;	
};

class CDamageObjManager
{
public:
	CDamageObjManager(){};
	void Process();
	void AddObj(std::vector<GameObjHandle> &ObjList);
	void Clear();
protected:
	std::vector<CDamageObj *>m_ObjList;
};*/

class CFcTidalWaveObject : public CFcAbilityBaseObject
{
public:
	CFcTidalWaveObject();
	virtual ~CFcTidalWaveObject();
	virtual void Initialize();
	virtual void Finalize();
	virtual void Process(){};
protected:
	int m_nTidalWaveFxID;
	_VERTEX		*m_pVList;
	CDamageObjManager m_DamageObjManager;
};
extern CFcTidalWaveObject  g_TidalWaveObj;

//typedef CSmartPtr<CFcTidalWaveObject> TidalWaveObjHandle;
