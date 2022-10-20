#include "stdafx.h"
#include "FcGameObject.h"
#include "FcAbilityWind.h"
#include "FcFxManager.h"
#include "FcFXCommon.h"
#include "BsImageProcess.h"
#include "FCUtil.h"

#include "FcWorld.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG

CFcAbilityWind::CFcAbilityWind( GameObjHandle hParent ) : CFcAbilityBase( hParent )
{
	m_nFXId[0] = -1;
	m_nFXId[1] = -1;
// [PREFIX:beginmodify] 2006/2/15 junyash PS#5193 reports Buffer Overrun writing to m_nFXId[2] in CFcAbilityWind::CFcAbilityWind(). it seems not necessary for other method... Commented out.
	// m_nFXId[2] = -1;
// [PREFIX:endmodify] junyash

	m_fFarfromCaster = 800.f;

	m_nDustEffect	=	-1;
	m_nDustEffect2	=	-1;

	m_dwTick = -1;

	int nScan;

	memset(m_Trail,0,sizeof(_TRAIL) * TRAILS);
	memset(m_BB,0,sizeof(_BILLBOARD) * BILLBOARDS);

	for(nScan = 0;nScan < TRAILS; nScan++)
		m_Trail[nScan].nHandle = -1;

	m_nBBID = -1;
	m_nBBID2 = -1;


	m_fTrailLen = 500.0f;
	m_nMotionBlurIndex = -1;
	m_fBlurAlpha = 0.7f;
	m_dwStartTick = 150;
	m_dwEndTick = m_dwStartTick + 270 + m_dwStartTick / 3;
	
}

CFcAbilityWind::~CFcAbilityWind()
{
	Finalize();
}

void CFcAbilityWind :: Initialize(DWORD dwParam1 /*= 0*/, DWORD dwParam2 /*= 0*/)
{
	m_fFarfromCaster = 2500.0f;

	m_nFXId[0] = g_pFcFXManager->Create(FX_TYPE_WINDEXPLODE);
	g_pFcFXManager->SendMessage(m_nFXId[0], FX_INIT_OBJECT,7,30,1);


	m_nFXId[1] = g_pFcFXManager->Create(FX_TYPE_WINDEXPLODE);
	g_pFcFXManager->SendMessage(m_nFXId[1], FX_INIT_OBJECT,10,5,0);
	
	D3DXVECTOR3 Org = (*m_hParent->GetCrossVector()).m_PosVector;
	D3DXVECTOR3 vZ = m_hParent->GetCrossVector()->m_ZVector;
	D3DXVECTOR3 vZ1,vZ2;
	float		fRotY = 70.0f * 3.1415f / 180.0f;
	float		fOff = 0.4f;

	vZ.y = 0.0f;

	vZ1.x = cosf(fRotY) * vZ.x - sinf(fRotY) * vZ.z;
	vZ1.z = sinf(fRotY) * vZ.x + cosf(fRotY) * vZ.z;
	vZ1.y = 0.0f;

	vZ2.x = cosf(-fRotY) * vZ.x - sinf(-fRotY) * vZ.z;
	vZ2.z = sinf(-fRotY) * vZ.x + cosf(-fRotY) * vZ.z;
	vZ2.y = 0.0f;


	Org += vZ * m_fFarfromCaster * 2.0f;


	m_Cross[0].m_PosVector = Org - vZ * m_fFarfromCaster * .5f;

	float	fMapXSize, fMapYSize;
	g_FcWorld.GetMapSize( fMapXSize, fMapYSize );
	if (m_Cross[0].m_PosVector.x >= 0.0f && m_Cross[0].m_PosVector.x < fMapXSize && 
		m_Cross[0].m_PosVector.z >= 0.0f && m_Cross[0].m_PosVector.z < fMapYSize)
        m_Cross[0].m_PosVector.y = g_FcWorld.GetLandHeight( m_Cross[0].m_PosVector.x, m_Cross[0].m_PosVector.z );
	else 
		m_Cross[0].m_PosVector.y = 0.0f;

	m_Cross[0].m_ZVector = vZ;
	m_Cross[0].UpdateVectors();

	m_Cross[1].m_PosVector = Org - vZ * m_fFarfromCaster;

	if (m_Cross[1].m_PosVector.x >= 0.0f && m_Cross[1].m_PosVector.x < fMapXSize && 
		m_Cross[1].m_PosVector.z >= 0.0f && m_Cross[1].m_PosVector.z < fMapYSize)
        m_Cross[1].m_PosVector.y = g_FcWorld.GetLandHeight( m_Cross[1].m_PosVector.x, m_Cross[1].m_PosVector.z );
	else
		m_Cross[1].m_PosVector.y = 0.0f;

	m_Cross[1].m_ZVector = vZ;
	m_Cross[1].UpdateVectors();


	m_fDir = 0.0f;
	m_dwTick = 0;

	m_nMotionBlurIndex = g_pFcFXManager->Create(FX_TYPE_2DFSEFFECT);
	g_pFcFXManager->SendMessage(m_nMotionBlurIndex, FX_INIT_OBJECT);
	g_pFcFXManager->SendMessage(m_nMotionBlurIndex, FX_SET_FSDEVIDE, 1);
	g_pFcFXManager->SendMessage(m_nMotionBlurIndex, FX_SET_TEXTURE, g_BsKernel.GetImageProcess()->GetBackBufferTexture() );
	g_pFcFXManager->SendMessage(m_nMotionBlurIndex, FX_PLAY_OBJECT);
	g_pFcFXManager->SendMessage(m_nMotionBlurIndex, FX_SET_FSALPHA, (DWORD)&m_fBlurAlpha);

	int nScan;
	float fSize = 200.f; 
	float fLife = 999.f;
	char szFile[128] = "WindOrbDustBillboard.dds";

	CCrossVector C = (*m_hParent->GetCrossVector());

	m_nBBID = g_pFcFXManager->Create(FX_TYPE_BBOARD);
	g_pFcFXManager->SendMessage(m_nBBID, FX_INIT_OBJECT, BILLBOARDS, (DWORD)&fSize, (DWORD)&fLife );
	g_pFcFXManager->SendMessage(m_nBBID, FX_SET_TEXTURE,(DWORD)szFile);
	g_pFcFXManager->SendMessage(m_nBBID, FX_PLAY_OBJECT);

	float	fAlpha = 0.0f;
	g_pFcFXManager->SendMessage(m_nBBID, FX_SET_ALPHA, (DWORD)&fAlpha);

	fSize = 250.f; 
	strcpy(szFile,"WindOrbDustBillboard2.dds");

	m_nBBID2 = g_pFcFXManager->Create(FX_TYPE_BBOARD);
	g_pFcFXManager->SendMessage(m_nBBID2, FX_INIT_OBJECT, BILLBOARDS, (DWORD)&fSize, (DWORD)&fLife );
	g_pFcFXManager->SendMessage(m_nBBID2, FX_SET_TEXTURE,(DWORD)szFile);

	for(nScan = 0;nScan < BILLBOARDS;nScan++)
	{
		m_BB[nScan].Pos.x = (float)(Random(2000) - 1000);
		m_BB[nScan].Pos.y = (float)(Random(100));
		m_BB[nScan].Pos.z = (float)(Random(1000) - 200);

		m_BB[nScan].fSpeed = 0.0f;
		g_pFcFXManager->SendMessage(m_nBBID, FX_UPDATE_OBJECT, (DWORD) &(m_BB[nScan].Pos) );
	}


	int		nPointCount = TRAILPOINTS;
	float	fEpsilon = 5.0f;
	float	fLineWidth = 25.0f;

	_TRAIL	*pT;

	for(nScan = 0;nScan < TRAILS;nScan++)
	{
		pT = &(m_Trail[nScan]);

		pT->nHandle = g_pFcFXManager->Create( FX_TYPE_LINECURVE );

		if (pT->nHandle == -1)
			continue;

		g_pFcFXManager->SendMessage( pT->nHandle, FX_SET_TEXTURE, ( DWORD )"Trail.dds", 1 );
		g_pFcFXManager->SendMessage( pT->nHandle, FX_INIT_OBJECT, ( DWORD )nPointCount, ( DWORD )&fEpsilon, ( DWORD )&fLineWidth );
	}

	g_FcWorld.InitTrueOrbDamagedTroops();
}

void CFcAbilityWind :: Finalize()
{
	int nScan;

	for(nScan = 0;nScan < 2;nScan++)
	{
		/*if( m_nFXId[nScan] != -1)
		{
			g_pFcFXManager->SendMessage(m_nFXId[nScan],FX_DELETE_OBJECT);
			m_nFXId[nScan] = -1;
		}*/
		SAFE_DELETE_FX(m_nFXId[nScan], FX_TYPE_WINDEXPLODE);
	}


	for(nScan = 0;nScan < TRAILS; nScan++)
	{
		//g_pFcFXManager->SendMessage(m_Trail[nScan].nHandle,FX_DELETE_OBJECT);
		//m_Trail[nScan].nHandle = -1;//FX_TYPE_LINECURVE
		SAFE_DELETE_FX(m_Trail[nScan].nHandle, FX_TYPE_LINECURVE);
	}

	/*if( m_nMotionBlurIndex != -1)
	{
		g_pFcFXManager->SendMessage(m_nMotionBlurIndex,FX_DELETE_OBJECT);
		m_nMotionBlurIndex = -1;
	}*/
	SAFE_DELETE_FX(m_nMotionBlurIndex,FX_TYPE_2DFSEFFECT);

	/*if( m_nBBID != -1)
	{
		g_pFcFXManager->SendMessage(m_nBBID,FX_DELETE_OBJECT);
		m_nBBID = -1;
	}*/
	SAFE_DELETE_FX(m_nBBID,FX_TYPE_BBOARD);

	//if( m_nBBID2 != -1)
	//{
	//	g_pFcFXManager->SendMessage(m_nBBID2,FX_DELETE_OBJECT);
	//	m_nBBID2 = -1;
	//}
	SAFE_DELETE_FX(m_nBBID2,FX_TYPE_BBOARD);

	g_FcWorld.FinishTrueOrbDamagedTroops();
}

void CFcAbilityWind :: Process()
{
	if (m_dwTick == m_dwStartTick)
		StartEff();

	if(m_dwTick == m_dwEndTick)
		m_bFinished = TRUE;

	if (m_dwTick > m_dwStartTick)
	{
		g_pFcFXManager->SendMessage(m_nFXId[0], FX_GET_POS, (DWORD)&(m_Cross[0].m_PosVector));
		g_pFcFXManager->SendMessage(m_nFXId[1], FX_GET_POS, (DWORD)&(m_Cross[1].m_PosVector));
	}

	if(m_dwTick == m_dwStartTick + 15)
	{

		std::vector< GameObjHandle > Enemy;
		g_FcWorld.GetObjectListInRange( &m_Cross[0].m_PosVector, 1000.f, Enemy );
		
		int nSize = Enemy.size();
		for( int ii = 0; ii < nSize ; ii++)
		{
			if(Enemy[ii] == m_hParent || Enemy[ii]->GetTeam() == 0)
			{
				continue;
			}

			D3DXVECTOR3	Err;
			Err = Enemy[ii]->GetPos() - (m_Cross[0].m_PosVector);
			float fDist = D3DXVec3Length(&Err);

			float fForce = fDist/m_fFarfromCaster;
			fForce = cosf(fForce*1.8f);
			fForce = (fForce+1.f)*0.5f;
		

			D3DXVECTOR3 vPush = D3DXVECTOR3(0.f, 200.f*fForce, 0.f);
			CFcWorld::GetInstance().GiveDamage( m_hParent, Enemy[ii], 1, ANI_TYPE_DOWN, 0, &vPush , m_nOrbGenType, 1,NULL,HT_ABIL_WIND, m_nOrbGenPer );
		}

		std::vector<CFcProp *> vecProps;
		g_FcWorld.GetActiveProps(m_Cross[0].m_PosVector,1000.f,vecProps);
		if(vecProps.size()){
			for( int ii = 0; ii < (int)vecProps.size() ; ii++)
			{
				CFcBreakableProp* pBreakable = (CFcBreakableProp*) vecProps[ii];
				if(pBreakable->IsCrushProp() || pBreakable->IsDynamicProp())
				{	
					D3DXVECTOR3 PropVec(pBreakable->GetPosV2().x,0,pBreakable->GetPosV2().y);			
					D3DXVECTOR3 Test = PropVec - m_hParent->GetCrossVector()->m_PosVector;
					D3DXVec3Normalize(&Test,&Test);
					Test = Test * 10.0f;
					pBreakable->AddForce( &Test, NULL, 1000000, PROP_BREAK_TYPE_SPECIAL_ATTACK );
				}
			}
		}
		vecProps.clear();
	}
	

	if(m_dwTick == m_dwStartTick + 25)
	{
		CCrossVector Cross = m_Cross[1];

		std::vector< GameObjHandle > Enemy;
		g_FcWorld.GetObjectListInRange( &m_Cross[1].m_PosVector, 2000.f, Enemy );

		int nSize = Enemy.size();
		for( int ii = 0; ii < nSize ; ii++)
		{
			if(Enemy[ii] == m_hParent || Enemy[ii]->GetTeam() == 0)
			{
				continue;
			}

			D3DXVECTOR3	Err;
			Err = Enemy[ii]->GetPos() - (m_Cross[1].m_PosVector);
			float fDist = D3DXVec3Length(&Err);

			float fForce = fDist/m_fFarfromCaster;
			fForce = cosf(fForce*1.8f);
			fForce = (fForce+1.f)*0.5f;


			D3DXVECTOR3 vPush = D3DXVECTOR3(0.f, 200.f*fForce, 0.f);
			CFcWorld::GetInstance().GiveDamage( m_hParent, Enemy[ii], 1, ANI_TYPE_DOWN, 0, &vPush , 1, 1, NULL, HT_ABIL_WIND );
		}

		std::vector<CFcProp *> vecProps;
		g_FcWorld.GetActiveProps(m_Cross[1].m_PosVector,2000.f,vecProps);
		if(vecProps.size()){
			for( int ii = 0; ii < (int)vecProps.size() ; ii++)
			{
				CFcBreakableProp* pBreakable = (CFcBreakableProp*) vecProps[ii];
				if(pBreakable->IsCrushProp() || pBreakable->IsDynamicProp())
				{	
					D3DXVECTOR3 PropVec(pBreakable->GetPosV2().x,0,pBreakable->GetPosV2().y);			
					D3DXVECTOR3 Test = PropVec - m_hParent->GetCrossVector()->m_PosVector;
					D3DXVec3Normalize(&Test,&Test);
					Test = Test * 10.0f;
					pBreakable->AddForce( &Test, NULL, 1000000, PROP_BREAK_TYPE_SPECIAL_ATTACK );
				}
			}
		}
		vecProps.clear();
	}

	if(m_dwTick == m_dwStartTick + 88)
	{
		CCrossVector Cross = m_Cross[1];
		D3DXVECTOR3 vZ = m_Cross[1].m_ZVector*m_fFarfromCaster;
		Cross.m_PosVector.x += vZ.x;
		Cross.m_PosVector.z += vZ.z;


		std::vector< GameObjHandle > Enemy;
		g_FcWorld.GetObjectListInRange( &Cross.m_PosVector, 4000.f, Enemy );

		int nSize = Enemy.size();
		for( int ii = 0; ii < nSize ; ii++)
		{
			if(Enemy[ii] == m_hParent || Enemy[ii]->GetTeam() == 0)
			{
				continue;
			}


			D3DXVECTOR3 vPush = D3DXVECTOR3(0.f, 40.f, -50.f);
			CFcWorld::GetInstance().GiveDamage( m_hParent, Enemy[ii], 2000/*Enemy[ii]->GetMaxHP() + 100*/, ANI_TYPE_DOWN, 0, &vPush , 1, 1, NULL, HT_ABIL_WIND );
		}

		std::vector<CFcProp *> vecProps;
		g_FcWorld.GetActiveProps(Cross.m_PosVector,4000.f,vecProps);
		if(vecProps.size()){
			for( int ii = 0; ii < (int)vecProps.size() ; ii++)
			{
				CFcBreakableProp* pBreakable = (CFcBreakableProp*) vecProps[ii];
				if(pBreakable->IsCrushProp() || pBreakable->IsDynamicProp())
				{	
					D3DXVECTOR3 PropVec(pBreakable->GetPosV2().x,0,pBreakable->GetPosV2().y);			
					D3DXVECTOR3 Test = PropVec - m_hParent->GetCrossVector()->m_PosVector;
					D3DXVec3Normalize(&Test,&Test);
					Test = Test * 10.0f;
					pBreakable->AddForce( &Test, NULL, 1000000, PROP_BREAK_TYPE_SPECIAL_ATTACK );
				}
			}
		}
		vecProps.clear();
	}

	int	nScan,nScan2;
	_TRAIL	*pT;

	int	nWindChange = m_dwStartTick + 185;
	float fDir = 0.0f;
	float fFactor = 1.0f;
	float fStep = 0.1f;
	float fVelMax = 50.0f;

	if (m_dwTick == nWindChange - 20 || m_dwTick == nWindChange + 30)
		g_pFcFXManager->SendMessage(m_nBBID2, FX_STOP_OBJECT);

	float	fAlpha;

	if (m_dwTick <= m_dwStartTick / 2)
	{
		fAlpha = min(1.0f, ((float)m_dwTick) / ((float)m_dwStartTick / 2));
		g_pFcFXManager->SendMessage(m_nBBID, FX_SET_ALPHA, (DWORD)&fAlpha);
	}

	if (m_dwTick >= nWindChange + 20 && m_dwTick < nWindChange + 30)
	{
		fAlpha = ((float)(nWindChange - m_dwTick)) * .1f;
		g_pFcFXManager->SendMessage(m_nBBID2, FX_SET_ALPHA, (DWORD)&fAlpha);
	}

	if (m_dwTick > m_dwEndTick - 40 && m_dwTick < m_dwEndTick)
	{
		fAlpha = ((float)(m_dwEndTick - m_dwTick)) / 40.0f;
		g_pFcFXManager->SendMessage(m_nBBID, FX_SET_ALPHA, (DWORD)&fAlpha);

		for(nScan = 0;nScan < TRAILS;nScan++)
			g_pFcFXManager->SendMessage(m_Trail[nScan].nHandle, FX_SET_ALPHA, (DWORD)&fAlpha);
	
	}

	if (m_dwTick == nWindChange)
		StartEff2();

	if (m_dwTick < m_dwStartTick)
	{
		fDir = -1.0f;
		fStep = 0.5f;
		fVelMax = 15.0f;
	}
	else if (m_dwTick >= m_dwStartTick && m_dwTick <= nWindChange - 20)
	{
		fDir = -30.0f;
		fStep = 0.2f;
		fVelMax = 50.0f;
	}
	else if (m_dwTick > nWindChange - 20 && m_dwTick <= nWindChange)
	{
		fDir = 0.0f;
		fFactor = 0.0f;
		fStep = 0.5f;
		fVelMax = 50.0f;
	}
	else
	{
		fDir = 200.0f;
		fStep = 0.5f;
		fVelMax = 100.0f;
	}

	m_fDir += (fDir - m_fDir) * fStep;

	D3DXVECTOR3 Wind;
	float		fRad;
	D3DXVECTOR3	Diff;
	float	fDist;

	if (m_dwTick >= m_dwStartTick)
	{
	
		for(nScan = 0;nScan < TRAILS;nScan++)
		{
			pT = &(m_Trail[nScan]);

			fRad = (float)(m_dwTick + pT->nSeed[0]);

			pT->Pos[0] = pT->P + 15.0f * cosf(fRad * .1f) * D3DXVECTOR3(sinf(fRad * .175f), 3.0f * cosf(fRad * .35f), sinf(fRad * -.175f));

			for(nScan2 = 1;nScan2 < TRAILPOINTS;nScan2++)
			{
				Wind = D3DXVECTOR3(0.0f,0.0f,m_fDir);

				fRad = (float)(m_dwTick + pT->nSeed[nScan2]);

				Wind += (fFactor + .1f) * 6.0f * cosf(fRad * .1f) * D3DXVECTOR3(sinf(fRad * .175f), 3.0f * cosf(fRad * .35f), sinf(fRad * -.175f));

				pT->Vel[nScan2] += Wind; 

				fDist = D3DXVec3Length(&(pT->Vel[nScan2]));
				if (fDist > 300.0f)
					pT->Vel[nScan2] = pT->Vel[nScan2] / fDist * 300.0f;

				pT->Pos[nScan2] += pT->Vel[nScan2];

				Diff = pT->Pos[nScan2] - pT->Pos[nScan2-1];
				fDist = D3DXVec3Length(&Diff);

				if (fDist > m_fTrailLen)
					pT->Pos[nScan2] = pT->Pos[nScan2-1] + Diff / fDist * m_fTrailLen;

			}
		}
	}

	for(nScan = 0; nScan < BILLBOARDS;nScan++)
	{
		if (m_fDir < 0.0f)
			m_BB[nScan].fSpeed += 2.0f * m_fDir;
		else
			m_BB[nScan].fSpeed += m_fDir;

		m_BB[nScan].fSpeed *= 0.9f;

		if (m_BB[nScan].fSpeed > fVelMax)
			m_BB[nScan].fSpeed = fVelMax;
			
		if (m_BB[nScan].fSpeed < -fVelMax)
			m_BB[nScan].fSpeed = -fVelMax;

		m_BB[nScan].Pos.z += m_BB[nScan].fSpeed;


		fDist = m_BB[nScan].Pos.z;

		if (m_fDir < 0.0f  && fDist < - 750.0f)
			m_BB[nScan].Pos.z += (float)(Random(500) + 500.0f - fDist);
		else if (m_fDir > 0.0f && fDist > 750.0f)
			m_BB[nScan].Pos.z -= (fDist + 500.0f + Random(250));
	}

	CCrossVector C = (*m_hParent->GetCrossVector());
	C.m_ZVector = m_Cross[0].m_PosVector - C.m_PosVector;
	D3DXVec3Normalize(&C.m_ZVector, &C.m_ZVector);
	C.UpdateVectors();

	D3DXVECTOR4	Temp;
	D3DXMATRIX	*Mat = (D3DXMATRIX*)(&C);
	D3DXVECTOR3	P;
	D3DXVECTOR3	TP[TRAILPOINTS];

	float	fMapXSize, fMapYSize;
	g_FcWorld.GetMapSize( fMapXSize, fMapYSize );

	if (m_dwTick >= m_dwStartTick)
	{
		for (nScan = 0;nScan < TRAILS;nScan++)
		{
			for(nScan2 = 0;nScan2 < TRAILPOINTS;nScan2++)
			{
				D3DXVec3Transform(&Temp, &(m_Trail[nScan].Pos[nScan2]), Mat);

				TP[nScan2].x = Temp.x;
				TP[nScan2].z = Temp.z;
				
				if (TP[nScan2].x >= 0.0f && TP[nScan2].x < fMapXSize && 
					TP[nScan2].z >= 0.0f && TP[nScan2].z < fMapYSize)
                    TP[nScan2].y = g_FcWorld.GetLandHeight( TP[nScan2].x, TP[nScan2].z ) + m_Trail[nScan].Pos[nScan2].y;
				else 
					TP[nScan2].y = 0.0f;
			}

			g_pFcFXManager->SendMessage( m_Trail[nScan].nHandle, FX_UPDATE_OBJECT, TRAILPOINTS, ( DWORD )(TP) );
		}
	}
	
	for(nScan = 0;nScan < BILLBOARDS;nScan++)
	{
		D3DXVec3Transform(&Temp, &(m_BB[nScan].Pos), Mat);
		P.x = Temp.x;
		P.z = Temp.z;
		if (P.x >= 0.0f && P.x < fMapXSize && 
			P.z >= 0.0f && P.z < fMapYSize)
            P.y = g_FcWorld.GetLandHeight( P.x, P.z ) + m_BB[nScan].Pos.y;
		else 
			P.y = 0.0f;

		g_pFcFXManager->SendMessage(m_nBBID, FX_UPDATE_OBJECT, (DWORD)(&P) );
	}


	FX_STATE	State;

	g_pFcFXManager->SendMessage( m_nBBID2, FX_GET_STATE, (DWORD)&State );

	if (m_dwTick >= m_dwStartTick && State == PLAY)
	{
		float	fMapXSize, fMapYSize;
		g_FcWorld.GetMapSize( fMapXSize, fMapYSize );

		for(nScan = 0; nScan < BILLBOARDS2;nScan++)
		{
			m_BB2[nScan].Pos.z += m_BB2[nScan].fSpeed;

			D3DXVec3Transform(&Temp, &(m_BB2[nScan].Pos), Mat);
			P.x = Temp.x;
			P.z = Temp.z;
			
			
			if (P.x >= 0.0f && P.x < fMapXSize && 
				P.z >= 0.0f && P.z < fMapYSize)
				P.y = g_FcWorld.GetLandHeight( P.x, P.z ) + m_BB2[nScan].Pos.y;
			else
				P.y = 0.0f;

			g_pFcFXManager->SendMessage(m_nBBID2, FX_UPDATE_OBJECT, (DWORD)(&P) );
		}
	}

	m_dwTick++;
}


void CFcAbilityWind :: ReadSOX()
{

}

void CFcAbilityWind :: StartEff()
{
	g_pFcFXManager->SendMessage(m_nFXId[0], FX_PLAY_OBJECT);
	g_pFcFXManager->SendMessage(m_nFXId[1], FX_PLAY_OBJECT);
	g_pFcFXManager->SendMessage(m_nFXId[0], FX_UPDATE_OBJECT, (DWORD)&m_Cross[0] );
	g_pFcFXManager->SendMessage(m_nFXId[1], FX_UPDATE_OBJECT, (DWORD)&m_Cross[1] );

	int nScan;

	int		nPointCount = TRAILPOINTS;
	float	fEpsilon = 5.0f;
	float	fLineWidth = 25.0f;

	_TRAIL	*pT;

	CCrossVector C = (*m_hParent->GetCrossVector());

	for(nScan = 0;nScan < TRAILS;nScan++)
	{
		pT = &(m_Trail[nScan]);

		g_pFcFXManager->SendMessage( pT->nHandle, FX_PLAY_OBJECT );

		float	fXOff;

		if (Random(3) > 0)
		{
			if (Random(5) < 2)
				fXOff = (float)(Random(3000) - 1500);
			else
				fXOff = (float)(Random(1500) - 750);

			pT->P.x = fXOff;
			pT->P.y = (fXOff * fXOff * 0.0005f + (float)(Random(50)));
			pT->P.z = (float)(Random(750) + 500);
		}
		else
		{

			fXOff = (float)(Random(1000) - 500);
			pT->P.x = fXOff;
			pT->P.y = (float)(Random(50) + 50);
			pT->P.z = -(float)(Random(200) + 100);
		}


		for(int nScan2 = 0;nScan2 < TRAILPOINTS;nScan2++)
		{
			pT->nSeed[nScan2] = Random(1000);

			pT->Pos[nScan2] = pT->P;
			pT->Pos[nScan2].x += (float)(Random(100) - 50) * 10.0f;
			pT->Pos[nScan2].y += (float)(10 * nScan2);
			pT->Pos[nScan2].z += (float)(Random(1500) - 500);
			pT->Vel[nScan2] = D3DXVECTOR3(0.0f,0.0f,0.0f);
		}
	}

	g_pFcFXManager->SendMessage(m_nBBID2, FX_PLAY_OBJECT);

	for(nScan = 0;nScan < BILLBOARDS2;nScan++)
	{
		m_BB2[nScan].Pos.x = (float)(Random(4000) - 2000);
		m_BB2[nScan].Pos.y = (float)(Random(100));
		m_BB2[nScan].Pos.z = (float)(Random(1500) + 2000);

		m_BB2[nScan].fSpeed = -(float)(Random(100) + 100);
	}
}

void	CFcAbilityWind :: StartEff2()
{
	int	nScan;

	CCrossVector C = (*m_hParent->GetCrossVector());
	g_pFcFXManager->SendMessage(m_nBBID2, FX_PLAY_OBJECT);

	for(nScan = 0;nScan < BILLBOARDS2;nScan++)
	{
		m_BB2[nScan].Pos.x = (float)(Random(10000) - 5000);
		m_BB2[nScan].Pos.y = (float)(Random(100)) + 50.0f;
		m_BB2[nScan].Pos.z = -(float)(Random(1000) + 1000);

		m_BB2[nScan].fSpeed = (float)(Random(50) + 80);

	}
}