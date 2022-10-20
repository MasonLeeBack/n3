#include "stdafx.h"
#include "FcGameObject.h"
#include "FcAbilitySoul.h"
#include "FcFxManager.h"
#include "FcFXCommon.h"
#include "FcWorld.h"
#include "FcGlobal.h"
#include "FcUtil.h"
#include "BsImageProcess.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG


#ifdef _DEBUG
#include "toputil.h"	
#endif

CAbSoulSceneIntensityTable::CAbSoulSceneIntensityTable(float fStartValue)
{
	m_nTable = 4;

	m_pTable = new TimeValue[m_nTable];

	m_pTable[0] = TimeValue(0,  fStartValue);
	m_pTable[1] = TimeValue(5, 0.4f);
	m_pTable[2] = TimeValue(15, 0.4f);
	m_pTable[3] = TimeValue(20, fStartValue);

}



CAbSoulGlowIntensityTable::CAbSoulGlowIntensityTable(float fStartValue)
{
	m_nTable = 4;

	m_pTable = new TimeValue[m_nTable];

	m_pTable[0] = TimeValue(0,  fStartValue);
	m_pTable[1] = TimeValue(5, 0.7f);
	m_pTable[2] = TimeValue(15, 0.7f);
	m_pTable[3] = TimeValue(20, fStartValue);
}

CAbSoulHighlightTable::CAbSoulHighlightTable(float fStartValue)
{
	m_nTable = 4;

	m_pTable = new TimeValue[m_nTable];
	m_pTable[0] = TimeValue(0,  fStartValue);
	m_pTable[1] = TimeValue(5,  0.7f);
	m_pTable[2] = TimeValue(15,  0.7f);
	m_pTable[3] = TimeValue(20,  fStartValue);
}


CFcSoulTrail::CFcSoulTrail() 
{

	m_state = OBSOUL_READY;
	m_nTargetIdx = -1;
	m_nFXLineIdx = -1;

}

CFcSoulTrail:: ~CFcSoulTrail() 
{
	/*if(m_nFXLineIdx != -1)
	{	
		g_pFcFXManager->SendMessage(m_nFXLineIdx, FX_DELETE_OBJECT);
		m_nFXLineIdx = -1;
	}*/
	SAFE_DELETE_FX(m_nFXLineIdx, FX_TYPE_LINETRAIL);
	
}

void CFcSoulTrail::Initialize( D3DXVECTOR3* pStartPos )
{
	m_fSpeed = 0.f;
	m_nFXLineIdx = -1;

	m_vPos = *pStartPos;
	m_vDir = D3DXVECTOR3( 0.f, 0.f, 1.f);
	m_nFXLineIdx = g_pFcFXManager->Create(FX_TYPE_LINETRAIL);
	float nE = 50.f; 
	float nW = 5.f;
	m_fSpeed = 5.f;
	char szFile[128] = "SoulTrail.dds";

	g_pFcFXManager->SendMessage(m_nFXLineIdx, FX_INIT_OBJECT, 30, (DWORD)&nW, (DWORD)&nE );
	g_pFcFXManager->SendMessage(m_nFXLineIdx, FX_SET_TEXTURE,(DWORD)szFile);
	g_pFcFXManager->SendMessage(m_nFXLineIdx, FX_PLAY_OBJECT);
//	g_pFcFXManager->SendMessage(m_nFXLineIdx, FX_SET_BLENDOP, D3DBLENDOP_REVSUBTRACT);


	m_fRand[0] = RandomNumberInRange( 2.f, 7.f);
	m_fRand[1] = RandomNumberInRange( 2.f, 4.f);
	m_fRand[2] = RandomNumberInRange( 2.f, 7.f);
	m_fRand[3] = RandomNumberInRange( 0.5f, 2.f);
	m_fRand[4] = RandomNumberInRange( 0.2f, 1.f);
	m_fRand[5] = RandomNumberInRange( 0.5f, 2.f);
}

void CFcSoulTrail::UpdateTarget(D3DXVECTOR3* pPos)
{
	m_vTarget = *pPos;

}


void CFcSoulTrail::Process()
{
	if(m_state == OBSOUL_READY)
		return;

	D3DXVECTOR3 vEra = D3DXVECTOR3(0.f, 0.f, 0.f);
	D3DXVECTOR3 vTargetDir;
	D3DXVec3Normalize( &vTargetDir, &(m_vTarget- m_vPos) );




	D3DXMATRIX rotMtx;
	DWORD dwTick = ::GetProcessTick();
	float f = D3DXVec3Length(&(m_vTarget- m_vPos));
	if( f < m_fSpeed *1.3f)
	{
		m_vPos = m_vTarget;
		if(m_state == OBSOUL_TRACE)
		{
			m_state = OBSOUL_CATCH;
		}
		g_pFcFXManager->SendMessage(m_nFXLineIdx, FX_UPDATE_OBJECT,(DWORD)&(m_vPos) );
	}
	else
	{
		D3DXVECTOR3 CrossVec;
		float fDot=D3DXVec3Dot(&vTargetDir, &m_vDir);

		
		float fmaxcurve = 0.03f;

		if(m_state == OBSOUL_TRACE)
			fmaxcurve = 1.f;
		else if(m_state == OBSOUL_RETURN)
			fmaxcurve = 0.01f;
			


		float fTheta = 1.f-sinf(fmaxcurve);
		if( fDot<  fTheta){
			D3DXVec3Cross(&CrossVec, &vTargetDir, &m_vDir );
//			if(fDot < 0.f){
				D3DXMatrixRotationAxis( &rotMtx, &CrossVec,  -fmaxcurve);
				D3DXVec3TransformCoord( &m_vDir, &m_vDir, &rotMtx);
//			}
//			else{
//				D3DXMatrixRotationAxis( &rotMtx, &CrossVec,  0.1f);
//				D3DXVec3TransformCoord( &m_vDir, &m_vDir, &rotMtx);
//			}

			fDot=D3DXVec3Dot(&vTargetDir, &m_vDir);
			if( fDot>fTheta){
				m_vDir = vTargetDir;
			}
		}
		else{
			m_vDir = vTargetDir;
		}


		//		D3DXVECTOR3 vDir;
		m_vPos += m_vDir*m_fSpeed*0.8f;
		//		m_vPos.x += sin( ((float)dwTick/123.f) )*m_fSpeed;//*0.5f;
		//		m_vPos.y += sin( ((float)dwTick/254.f) )*m_fSpeed;//*0.5f;
		//		m_vPos.z += cos( ((float)dwTick/73.f ) )*m_fSpeed;//*0.5f;

		if(m_state != OBSOUL_RETURN)
		{
			vEra.x = sin( ((float)dwTick/ m_fRand[0]) )*m_fSpeed*m_fRand[3]*0.2f;//*sin( ((float)dwTick/3.f) );
			vEra.y = sin( ((float)dwTick/ m_fRand[1]) )*m_fSpeed*m_fRand[4]*0.2f;//*sin( ((float)dwTick/7.f) );
			vEra.z = cos( ((float)dwTick/ m_fRand[2]) )*m_fSpeed*m_fRand[5]*0.2f;//*sin( ((float)dwTick/11.f) );
		}

		CrossVec = m_vPos + vEra;

		float	fMapXSize, fMapYSize;
		g_FcWorld.GetMapSize( fMapXSize, fMapYSize );
		float fHeight = CrossVec.y;
		if (CrossVec.x >= 0.0f && CrossVec.x < fMapXSize && CrossVec.z >= 0.0f && CrossVec.z < fMapYSize)
			fHeight = g_FcWorld.GetLandHeight(CrossVec.x, CrossVec.z);

		if( fHeight > CrossVec.y )
			CrossVec.y = fHeight + (fHeight - CrossVec.y);

		g_pFcFXManager->SendMessage(m_nFXLineIdx, FX_UPDATE_OBJECT,(DWORD)&(CrossVec) );

	}
}	




void CFcGhostTrail::Initialize( D3DXVECTOR3* pStartPos )
{
	m_fSpeed = 0.f;
	m_nFXLineIdx = -1;

	m_vPos = *pStartPos;
	m_vDir = D3DXVECTOR3( 0.f, 0.f, 1.f);
	m_nFXLineIdx = g_pFcFXManager->Create(FX_TYPE_LINETRAIL);
	float nE = 10.f; 
	float nW = 10.f;
	m_fSpeed = 5.f;
	char szFile[128] = "SoulGhost.dds";

	g_pFcFXManager->SendMessage(m_nFXLineIdx, FX_INIT_OBJECT, 40, (DWORD)&nW, (DWORD)&nE );
	g_pFcFXManager->SendMessage(m_nFXLineIdx, FX_SET_TEXTURE,(DWORD)szFile);
	g_pFcFXManager->SendMessage(m_nFXLineIdx, FX_PLAY_OBJECT);


	m_fRand[0] = RandomNumberInRange( 1.f, 3.f);
	m_fRand[1] = RandomNumberInRange( 1.f, 2.f);
	m_fRand[2] = RandomNumberInRange( 1.f, 3.f);
	m_fRand[3] = RandomNumberInRange( 2.0f, 4.f);
	m_fRand[4] = RandomNumberInRange( 0.8f, 2.f);
	m_fRand[5] = RandomNumberInRange( 2.0f, 4.f);

}



CFcAbilitySoul::CFcAbilitySoul( GameObjHandle hParent ) : CFcAbilityBase( hParent )
{
	m_SoulTrails = NULL;
	m_GhostTrails = NULL;
	m_dwCounter = 0;

	m_vReturnPoint = D3DXVECTOR3( 0.f, 0.f, 0.f) ;
	m_nTarget	 = -1;
	m_nSoulTrail = -1;
	m_nFxboardID = -1;
	m_nFxFilterID = -1;

	m_nSoulTrail = 150;	

	m_pSceneTable = NULL;
	m_pGlowTable = NULL;
	m_pHlightTable = NULL;

	m_nInitedTrail = 0;

#ifdef _DEBUG
	m_nKill = 0;
#endif
}

CFcAbilitySoul::~CFcAbilitySoul()
{
	Finalize();
	
}

void CFcAbilitySoul::Initialize(DWORD dwParam1 /*= 0*/, DWORD dwParam2 /*= 0*/)
{

	CCrossVector* pCross =  m_hParent->GetCrossVector();
	m_vPos = pCross->GetPosition();
	m_vPos += pCross->m_ZVector*3000.f;

	m_vReturnPoint = m_vPos + pCross->m_ZVector*3000.f;
	m_vReturnPoint.y += 10000.f;
	CFcWorld::GetInstance().GetEnemyObjectListInRange( m_hParent->GetTeam(), &m_vPos, 5500.0f, m_Targets,false );
	m_nTarget = m_Targets.size();
	//	for( i = 0; i < nSize; i++)
	//	{
	//
	//		if( !Result[ i ]->IsHittable( m_Handle ) )
	//		{
	//			continue;
	//		}
	//	}

//	if( m_nTarget > m_nSoulTrail)
//		m_nTarget = m_nSoulTrail;

#ifdef _DEBUG
//	DebugString("AbliSoulTagetCount %d\n", m_nTarget ); 
#endif


	m_dwCounter = 0;


	m_nFxboardID = g_pFcFXManager->Create(FX_TYPE_BBOARD);
	float fSize = 50.f; 
	float fLife = 30.f;
	char szFile[128] = "Soultwinkle.dds";

	g_pFcFXManager->SendMessage(m_nFxboardID, FX_INIT_OBJECT, 1000, (DWORD)&fSize, (DWORD)&fLife );
	g_pFcFXManager->SendMessage(m_nFxboardID, FX_SET_TEXTURE,(DWORD)szFile);
	g_pFcFXManager->SendMessage(m_nFxboardID, FX_PLAY_OBJECT);
	g_pFcFXManager->SendMessage(m_nFxboardID, FX_SET_BLENDOP,  D3DBLENDOP_ADD);
	g_pFcFXManager->SendMessage(m_nFxboardID, FX_SET_SRCBLEND, D3DBLEND_SRCCOLOR);
	g_pFcFXManager->SendMessage(m_nFxboardID, FX_SET_DSTBLEND, D3DBLEND_ONE);



	m_nFxFilterID = g_pFcFXManager->Create(FX_TYPE_XM);
	g_pFcFXManager->SendMessage(m_nFxFilterID, FX_INIT_OBJECT);
	g_pFcFXManager->SendMessage(m_nFxFilterID, FX_SET_TEXTURE);
	g_pFcFXManager->SendMessage(m_nFxFilterID, FX_PLAY_OBJECT);


	m_SoulTrails = new CFcSoulTrail[m_nSoulTrail];

	m_GhostTrails = new CFcGhostTrail[m_nSoulTrail];




	m_fOrgSceneIntensity = g_BsKernel.GetImageProcess()->GetSceneIntensity();
	m_fOrgBlurIntensity = g_BsKernel.GetImageProcess()->GetBlurIntensity();
	m_fOrgGlowIntensity = g_BsKernel.GetImageProcess()->GetGlowIntensity();
	m_fOrgHighlightTredhold = g_BsKernel.GetImageProcess()->GetHighlightThreshold();

	m_pSceneTable = new CAbSoulSceneIntensityTable(1.f);
	m_pGlowTable =	new CAbSoulGlowIntensityTable(m_fOrgGlowIntensity);
	m_pHlightTable =new CAbSoulHighlightTable(m_fOrgHighlightTredhold);

	m_nInitedTrail = 0;

	
	m_hParent->SetOrbSparkGodMode(true);

	g_FcWorld.InitTrueOrbDamagedTroops();
}


void CFcAbilitySoul::Finalize()
{
	delete[] m_SoulTrails, m_SoulTrails = NULL ;

	delete[] m_GhostTrails, m_GhostTrails = NULL ;

	/*g_pFcFXManager->SendMessage(m_nFxboardID, FX_DELETE_OBJECT);
	m_nFxboardID = -1;*/
	SAFE_DELETE_FX(m_nFxboardID, FX_TYPE_BBOARD);

	/*if(m_nFxFilterID != -1)
	{
		g_pFcFXManager->SendMessage(m_nFxFilterID, FX_DELETE_OBJECT);
		m_nFxFilterID= -1;
	}*/
	SAFE_DELETE_FX(m_nFxFilterID, FX_TYPE_XM);
	
	if(m_pSceneTable)
		delete m_pSceneTable, m_pSceneTable = NULL;
	if(m_pGlowTable)
		delete m_pGlowTable, m_pGlowTable = NULL;
	if(m_pHlightTable)
		delete m_pHlightTable, m_pHlightTable = NULL;
	m_hParent->SetOrbSparkGodMode(false);

	g_FcWorld.FinishTrueOrbDamagedTroops();
}


void CFcAbilitySoul::Process()
{
	if( m_nInitedTrail != m_nSoulTrail)
	{
		D3DXVECTOR3 vPos = m_hParent->GetPos();
		vPos.y += 100.f;
		int ii =  m_nInitedTrail;
		int nMax = BsMin(m_nInitedTrail+30, m_nSoulTrail);

		for( ; ii < nMax ; ++ii)
		{ 
			m_SoulTrails[ii].Initialize( &vPos );

			D3DXVECTOR3 vTap;
			vTap.x = RandomNumberInRange(-10000.f, 10000.f);
			vTap.y = RandomNumberInRange(-50.f, 10000.f);
			vTap.z = RandomNumberInRange(-10000.f, 10000.f);
			m_SoulTrails[ii].UpdateTarget( &(vTap + vPos) );
			m_SoulTrails[ii].SetState(OBSOUL_BORN);
			D3DXVec3Normalize( &vTap, &vTap);
			m_SoulTrails[ii].SetDirection(&vTap);
			m_SoulTrails[ii].SetSpeed(30.f);

			if(ii < m_nTarget)
				m_SoulTrails[ii].SetTargetIdx(ii);
		}

		m_nInitedTrail = nMax;
	}




	SOULSTATE curState;
	for(int ii = 0 ; ii < m_nInitedTrail ; ++ii)
	{ 
		m_SoulTrails[ii].Process();

		curState =  m_SoulTrails[ii].GetState();
		if(curState != OBSOUL_READY)
		{
			float frand =RandomNumberInRange(0.f, 100.f);
			if(frand > 90.f)
			{
				g_pFcFXManager->SendMessage(m_nFxboardID, FX_UPDATE_OBJECT, (DWORD)m_SoulTrails[ii].GetPos() );
			}

		}
	}

	for(int ii = 0 ; ii < m_nSoulTrail ; ++ii)
	{ 
		curState =  m_GhostTrails[ii].GetState();
		if(curState != OBSOUL_READY)
			m_GhostTrails[ii].Process();
	}
	


	D3DXVECTOR3 vTap;
	if(m_dwCounter >=  600)
	{
		for(int ii = 0 ; ii < m_nInitedTrail ; ++ii)
		{
			if( m_SoulTrails[ii].GetState() == OBSOUL_TRACE )
			{
				
				vTap.x = RandomNumberInRange(-10000.f, 10000.f);
				vTap.y = RandomNumberInRange(-200.f, 200.f);
				vTap.z = RandomNumberInRange(-10000.f, 10000.f);

				m_SoulTrails[ii].UpdateTarget( (D3DXVECTOR3*)&(m_vReturnPoint+vTap) );
				m_SoulTrails[ii].SetState(OBSOUL_RETURN);
				m_SoulTrails[ii].SetSpeed(50.f);
			}
		}
	}
//	else if(m_dwCounter >=  60)
	else if(m_dwCounter >=  140)
	{
		CCrossVector* pCross =  m_hParent->GetCrossVector();
		D3DXVECTOR3 vPos = pCross->GetPosition();
		vPos += pCross->m_ZVector*5000.f;


		for(int ii = 0 ; ii < m_nInitedTrail ; ++ii)
		{ 
			curState = m_SoulTrails[ii].GetState();

			// !! fix !!
			if( (curState == OBSOUL_TRACE || curState == OBSOUL_CATCH) && 
				( m_SoulTrails[ii].GetTargetIdx() > -1 ) &&
				CFcBaseObject::IsValid( m_Targets[ m_SoulTrails[ii].GetTargetIdx() ] ) == false )
			{
				m_SoulTrails[ii].SetTargetIdx(-1);
				m_SoulTrails[ii].SetState(OBSOUL_RETURN);
				continue;
			}

			switch(curState)
			{
			case OBSOUL_BORN:
				m_SoulTrails[ii].SetState(OBSOUL_TRACE);
			case OBSOUL_TRACE:
				if(m_SoulTrails[ii].GetTargetIdx() > -1)
				{
					vPos = m_Targets[ m_SoulTrails[ii].GetTargetIdx() ]->GetPos();
					vPos.y += 100.f;
					m_SoulTrails[ii].UpdateTarget( (D3DXVECTOR3*)&vPos );
					m_SoulTrails[ii].SetSpeed(100.f);
				}
				else
				{
					if(m_dwCounter == 140)
					{
						vTap.x = RandomNumberInRange(-500.f, 500.f);
						vTap.y = RandomNumberInRange(-100.f, 500.f);
						vTap.z = RandomNumberInRange(-500.f, 500.f);
						//vTap = D3DXVECTOR3( 0.f, 0.f, 3000.f);
						m_SoulTrails[ii].UpdateTarget( &(vTap + vPos) );
						m_SoulTrails[ii].SetSpeed(100.f);
					}
				}

				break;


			case OBSOUL_CATCH:
				{
					int nTargetIdx = m_SoulTrails[ii].GetTargetIdx();
					if( nTargetIdx == -1)
					{
						vTap.x = RandomNumberInRange(-10000.f, 10000.f);
						vTap.y = RandomNumberInRange(-200.f,	 200.f);
						vTap.z = RandomNumberInRange(-10000.f, 10000.f);

						m_SoulTrails[ii].UpdateTarget( (D3DXVECTOR3*)&(m_vReturnPoint+vTap) );
						m_SoulTrails[ii].SetState(OBSOUL_RETURN);
						m_SoulTrails[ii].SetSpeed(50.f);
					}
					else if( nTargetIdx + m_nSoulTrail < m_nTarget )
					{
						m_SoulTrails[ii].SetTargetIdx(nTargetIdx + m_nSoulTrail);

						vPos = m_Targets[ m_SoulTrails[ii].GetTargetIdx() ]->GetPos();
						vPos.y += 100.f;
						m_SoulTrails[ii].UpdateTarget( (D3DXVECTOR3*)&vPos );

						m_SoulTrails[ii].SetState(OBSOUL_TRACE);
					}
					else
					{
						m_SoulTrails[ii].SetTargetIdx(-1);

						vTap.x = RandomNumberInRange(-10000.f, 10000.f);
						vTap.y = RandomNumberInRange(-100.f, 2000.f);
						vTap.z = RandomNumberInRange(-10000.f, 10000.f);

						vTap = m_vReturnPoint+vTap;
						m_SoulTrails[ii].UpdateTarget( (D3DXVECTOR3*)&(vTap) );
						m_SoulTrails[ii].SetState(OBSOUL_RETURN);
						m_SoulTrails[ii].SetSpeed(50.f);

						D3DXVECTOR3 vPos;
						vPos = *m_SoulTrails[ii].GetPos();
						m_GhostTrails[ii].Initialize( &vPos );

						D3DXVec3Normalize( &vTap, &(vTap - vPos) );
						vTap.x *= 3000.f;
						vTap.y *= 3000.f;
						vTap.z *= 3000.f;

						m_GhostTrails[ii].UpdateTarget( &(vTap + vPos) );
						m_GhostTrails[ii].SetState(OBSOUL_BORN);
						vTap =D3DXVECTOR3 ( 0.f, 1.f, 0.f);
						m_GhostTrails[ii].SetDirection(&vTap);
						m_GhostTrails[ii].SetSpeed(10.f);
					}
				
					if(nTargetIdx > -1)
					{
						D3DXVECTOR3 vPush(0.f, 0.f, 0.f);
						CFcWorld::GetInstance().GiveDamage( m_hParent, m_Targets[nTargetIdx], 2000/*m_Targets[nTargetIdx]->GetMaxHP() + 100*/ , ANI_TYPE_HIT, 0, &vPush , m_nOrbGenType, 1,NULL,HT_ABIL_SOUL,m_nOrbGenPer);
					#ifdef _DEBUG
						m_nKill++;
//						DebugString("AbliSoulKillCount %d\n", m_nKill ); 
					#endif
					}


					//m_Targets[ii]->CalculateDamage(-1);
					//m_Targets[ii]->ChangeAnimation(ANI_TYPE_DIE, 0);
				}
				break;

			case OBSOUL_RETURN:
				{
				}
				break;

			}
		}
	}
	else if(m_dwCounter >=  45)
	{
		for(int ii = 0 ; ii < m_nSoulTrail ; ++ii)
		{ 
			m_SoulTrails[ii].SetSpeed(5.f);
		}
	}



	float fScene = m_pSceneTable->GetVaule(m_dwCounter);
	g_BsKernel.GetImageProcess()->SetSceneIntensity(fScene*m_fOrgSceneIntensity);
	g_BsKernel.GetImageProcess()->SetBlurIntensity(fScene*m_fOrgBlurIntensity);
	float fGlow = m_pGlowTable->GetVaule(m_dwCounter);
	g_BsKernel.GetImageProcess()->SetGlowIntensity(fGlow);
	float fHight = m_pHlightTable->GetVaule(m_dwCounter);
	g_BsKernel.GetImageProcess()->SetHighlightThreshold(fHight);


	if(m_dwCounter >= 160 && m_nFxFilterID != -1)
	{
		/*g_pFcFXManager->SendMessage(m_nFxFilterID, FX_DELETE_OBJECT);
		m_nFxFilterID= -1;*/
		SAFE_DELETE_FX(m_nFxFilterID, FX_TYPE_XM);
	}


	if(m_dwCounter == 700)
	{
		m_bFinished = TRUE;
		m_hParent->SetOrbSparkGodMode(false);
	}

	m_dwCounter++;
}