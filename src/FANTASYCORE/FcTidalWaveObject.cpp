#include "stdafx.h"
#include "FcTidalWaveObject.h"
#include "FcFxManager.h"
#include "FcFxBase.h"
#include "FcFxTidalWave.h"
#include "FcWorld.h"
#include "FcHeroObject.h"
#include "FcGameObject.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG


CFcTidalWaveObject::CFcTidalWaveObject()
{
	m_nTidalWaveFxID = -1;
}

CFcTidalWaveObject::~CFcTidalWaveObject()
{
}

void CFcTidalWaveObject::Finalize()
{
	m_nTidalWaveFxID = -1;
}


void CFcTidalWaveObject::Initialize()
{
	if(m_nTidalWaveFxID == -1){
		m_nTidalWaveFxID = g_pFcFXManager->Create(FX_TYPE_TIDALWAVE);
	}
	g_pFcFXManager->SendMessage(m_nTidalWaveFxID,FX_INIT_OBJECT);

	//int *pVertexCount = 0;
	//g_pFcFXManager->SendMessage(m_nTidalWaveFxID,FX_GET_VERTEXDATA,(DWORD)m_pVList,(DWORD)pVertexCount);

	//for(int i = 0;i < pVertexCount;i++)
	{
	}
	//std::vector<GameObjHandle>vtList;
	//g_FcWorld.GetEnemyObjectListInRange(0,&pV[i].Pos,30.f,vtList);

	//m_DamageObjManager.AddObj(vtList);

}

//---------------------------------------------------------------------------------------------------
/*
void CDamageObjManager::Process()
{
	for(unsigned int i = 0;i < m_ObjList.size();i++){
		m_ObjList[i]->Process();
	}

}

void CDamageObjManager::AddObj(std::vector<GameObjHandle> &ObjList)
{
	bool bExist = false;
	for(unsigned int i = 0;i < ObjList.size();i++)
	{
		for(unsigned int s = 0;s < m_ObjList.size();s++)
		{
			if(ObjList[i] == m_ObjList[s]->GetObjHandle())
			{
				bExist = true;
				break;
			}
		}
		if(bExist == false)
		{
			CDamageObj *pObj = new CDamageObj;
			pObj->SetObjHandle(ObjList[i]);
			m_ObjList.push_back(pObj);
			break;
		}
	}

}

void CDamageObj::Process()
{
	int nCurAni = m_ObjHandle->GetCurAniType();
	if(m_ObjHandle->GetHP())
	{
		m_ObjHandle->CalculateDamage(m_ObjHandle->GetMaxHP());
		m_ObjHandle->ChangeAnimation(ANI_TYPE_DOWN, 1);
		CCrossVector *pCross = m_ObjHandle->GetCrossVector();

		float fDist = D3DXVec3Length(&(pCross->m_PosVector - g_FcWorld.GetHeroHandle(0)->GetCrossVector()->m_PosVector));
		if(fDist > 3000.f)fDist = 3000.f;
		float fPower = (3000.f - fDist) / 3000.f;

		D3DXVECTOR3 Dir = pCross->m_PosVector - g_FcWorld.GetHeroHandle(0)->GetCrossVector()->m_PosVector;
		D3DXVec3Normalize(&Dir,&Dir);
		Dir	   = Dir * fPower * 30.f;
		Dir.y  = 50.f * fPower;
		m_ObjHandle->SetVelocity(&Dir);
	}
}*/
