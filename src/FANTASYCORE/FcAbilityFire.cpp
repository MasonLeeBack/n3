#include "stdafx.h"
#include "FcAbilityFire.h"
#include "BsHFWorld.h"
#include "FcWorld.h"
#include "FcHeroObject.h"
#include "FcFXManager.h"
#include "FCUtil.h"
#include "FcSoundManager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG

METEORDEFINE g_sInfo;

class CFcGameObject;
typedef CSmartPtr<CFcGameObject> GameObjHandle;


CFcAbilityFire::CFcAbilityFire( GameObjHandle hParent ) : CFcAbilityBase( hParent )
{

	m_nDelTimer = 300;
	m_fMaxIntensity					= 0.0f;
	
//	m_hParent = CFcWorld::GetInstance().GetHeroHandle();

	m_sInfo._fSpeed					= 60.0f;						
	m_sInfo._UpHorizontalDist		= 500;				
	m_sInfo._nUpForwardDist			= -1500;				
	m_sInfo._DownForwardDist		= 3000;				
	m_sInfo._nUpRadius				= 150;						
	m_sInfo._nDownRadius			= 3000;					
	m_sInfo._nStartHeight			= 4000;					
	m_sInfo._nInterval				= 100;						
	m_sInfo._calledMeteorNum		= 0;

	m_sInfo._byOnceBeComingNumber[0]= 7;						// Min		
	m_sInfo._byOnceBeComingNumber[1]= 8;						// Max
	m_sInfo._byMaxBeComingNumber	= 3;						
	m_sInfo._fMeteorDamage[0]		= 1200.0f;					// 범위	
	m_sInfo._fMeteorDamage[1]		= 10000.0f;					// 강도	
	m_sInfo._fPiceOfMeteorDamage[0]	= 50.0f;					// 범위
	m_sInfo._fPiceOfMeteorDamage[1]	= 100.0f;					// 강도
	m_sInfo._fScale[0]				= 1.5f;						// 크기 스케일
	m_sInfo._fScale[1]				= 1.5f;						// 회전 크기 스케일
	m_sInfo._fExplodeScale			= 2.5f;					
	m_sInfo._fRadius[0]				= 500.0f;					// 메테오조각이 생성될 메테오 위치로부터의 거리 Min					
	m_sInfo._fRadius[1]				= 1000.0f;					// 메테오조각이 생성될 메테오 위치로부터의 거리 Max					
	m_sInfo._nDeltaTime				= 300;					
	m_sInfo._nDeltaTime2			= 150;					
	m_sInfo._fTrailValue[0]			= 50.0f;					// life
	m_sInfo._fTrailValue[1]			= 6.0f;						// width
	m_sInfo._fTrailValue[2]			= 20.0f;					// epsilon

	m_sInfo._fPiceOfMeteorValue[0]	= 0.06f;						// Min Speed
	m_sInfo._fPiceOfMeteorValue[1]	= 0.5f;					// Max Speed

	m_sInfo._fPiceOfMeteorValue[2]	= 0;						// Max y Direction  : Min = 0
	m_sInfo._fPiceOfMeteorValue[3]	= 30.0f;					// AniSpd
	m_sInfo._fPiceOfMeteorValue[4]	= 97;						// Incidence (%)
	m_sInfo._byUseSceneeffect		= 1;						// 2이상이면 nothing				
	m_sInfo._fSceneEft[0]			= 4500.0f;					// 반경
	m_sInfo._fSceneEft[1]			= 1.3f;		//1.3f				// 세기

}




CFcAbilityFire::~CFcAbilityFire()
{
	Finalize();
}

void CFcAbilityFire::Initialize(DWORD dwParam1 /*= 0*/, DWORD dwParam2 /*=0*/)
{
	m_nDelTimer		 = 300;
	m_vCharPos		 = m_hParent->GetPos();
	m_vCharDir		 = m_hParent->GetCrossVector()->m_ZVector;
	D3DXVec3Cross(&m_vCharRight, &m_vCharDir, &D3DXVECTOR3(0,1,0) );
	D3DXVec3Normalize(&m_vCharRight,&m_vCharRight);

	D3DXVECTOR3 vZ	 = m_vCharDir*(float)m_sInfo._DownForwardDist;
	D3DXVec3Add( &m_vCastPoint, &m_vCharPos,  &vZ);

	float	fMapXSize, fMapYSize;
	g_FcWorld.GetMapSize( fMapXSize, fMapYSize );
	if (m_vCastPoint.x >= 0.0f && m_vCastPoint.x < fMapXSize && 
		m_vCastPoint.z >= 0.0f && m_vCastPoint.z < fMapYSize)
        m_vCastPoint.y	 = g_FcWorld.GetLandHeight( m_vCastPoint.x, m_vCastPoint.z );
	else
		m_vCastPoint.y	 = 0.0f;

	if( m_sInfo._byUseSceneeffect == 1 )
        m_nSceneIntensityBuffer = g_BsKernel.GetSceneIntensity();
	else if( m_sInfo._byUseSceneeffect == 2 )
        m_vSceneIntensityBuffer = g_BsKernel.GetSceneAddColor();
	else
	{
		m_nSceneIntensityBuffer = g_BsKernel.GetSceneIntensity();
		m_vSceneIntensityBuffer = D3DXVECTOR4(0,0,0,0);
	}

	g_FcWorld.InitTrueOrbDamagedTroops();
}

void CFcAbilityFire::Finalize()
{
	if( m_sInfo._byUseSceneeffect == 1 )
	{
		g_BsKernel.SetSceneIntensity(m_nSceneIntensityBuffer);

	}
	else if( m_sInfo._byUseSceneeffect == 2 )
	{
		g_BsKernel.SetSceneAddColor(0,0,0);
	}

	int ii, nCount  = m_listMeteorInfo.size();
	for( ii = 0 ; ii < nCount ; ++ii)
	{
		MeteorInfo* ptmp = m_listMeteorInfo[ii];
		FX_STATE	state;
		if( ptmp->nFxIdx == -1 ) continue;
		g_pFcFXManager->SendMessageQueue(ptmp->nFxIdx, FX_GET_STATE, (DWORD)&state);
		g_pFcFXManager->SendMessageQueue(ptmp->nFxIdx, FX_FORCED_DELETE );
		delete m_listMeteorInfo[ii];
	}
	m_listMeteorInfo.clear();
	g_FcWorld.FinishTrueOrbDamagedTroops();
}

void CFcAbilityFire::Process()
{
	int ii, nCount  = m_listMeteorInfo.size();
	static int count2=0;
	static int TmpInterval = 0;
	if( m_sInfo._calledMeteorNum < m_sInfo._byMaxBeComingNumber )
	{
		if(TmpInterval <= 0 )
		{
			TmpInterval = m_sInfo._nInterval;
			int count,num;
			count = num = 0;
			if( m_sInfo._byOnceBeComingNumber[0] >1 ) num = RandomNumberInRange(m_sInfo._byOnceBeComingNumber[0],m_sInfo._byOnceBeComingNumber[1]);
			else num = 1;
			while( num > count)
			{
				static bool b = true;
				CallMeteor();
				count++;
			}
			nCount  = m_listMeteorInfo.size();
			m_sInfo._calledMeteorNum++;
		}
		else
			--TmpInterval;
	}
	else if(m_sInfo._calledMeteorNum == m_sInfo._byMaxBeComingNumber)
	{
		if( m_nDelTimer-- == 0 )
		{
			for( ii = 0 ; ii < nCount ; ++ii)
			{	
				if(m_listMeteorInfo[ii]->bExplode == FALSE)
					break;
			}

			if(ii == nCount)
				m_bFinished = TRUE;
		}
		else
			m_nDelTimer--;
	}

	
	float fHeight;
	D3DXVECTOR3 vPos;
	float fIntensity2 = 0.0f;
	
	float	fMapXSize, fMapYSize;
	g_FcWorld.GetMapSize( fMapXSize, fMapYSize );

	for( ii = 0 ; ii < nCount ; ++ii)
	{
		if (m_listMeteorInfo[ii]->vPos.x >= 0.0f && m_listMeteorInfo[ii]->vPos.x < fMapXSize && 
			m_listMeteorInfo[ii]->vPos.z >= 0.0f && m_listMeteorInfo[ii]->vPos.z < fMapYSize)
			fHeight = g_FcWorld.GetLandHeight( m_listMeteorInfo[ii]->vPos.x, m_listMeteorInfo[ii]->vPos.z );
		else
			fHeight = 0.0f;

		vPos = m_listMeteorInfo[ii]->vPos;
		if( m_listMeteorInfo[ii]->vPos.y+400.0f < fHeight)
		{
			if(m_listMeteorInfo[ii]->bExplode == FALSE)
			{
				g_pSoundManager->PlaySystemSound( SB_COMMON,"AT_C_MYIF_ORB_S_01_EXPL" );

				m_listMeteorInfo[ii]->vPos += m_listMeteorInfo[ii]->vDir*m_sInfo._fSpeed;
				g_pFcFXManager->SendMessage(m_listMeteorInfo[ii]->nFxIdx, FX_UPDATE_OBJECT, 
					(DWORD)&m_listMeteorInfo[ii]->vDir, (DWORD)&m_listMeteorInfo[ii]->vPos, (DWORD)&m_sInfo._fSpeed );
				m_listMeteorInfo[ii]->vPos.y -= 500.0f;
				g_pFcFXManager->SendMessage( m_listMeteorInfo[ii]->nFxIdx, FX_FIRE_EXPLODE);
				/*g_pFcFXManager->SendMessage( m_listMeteorInfo[ii]->nFxIdx , FX_DELETE_OBJECT, (DWORD)&m_listMeteorInfo[ii]->vDir, (DWORD)&m_listMeteorInfo[ii]->vPos, (DWORD)&m_sInfo._fSpeed);
				m_listMeteorInfo[ii]->nFxIdx = -1;*/
				SAFE_DELETE_FX(m_listMeteorInfo[ii]->nFxIdx, FX_TYPE_METEOR);
				m_listMeteorInfo[ii]->bExplode = TRUE;
				m_nDelTimer = m_sInfo._nDeltaTime;
				//Damage
				_FC_RANGE_DAMAGE rd;
				rd.Attacker = m_hParent;
				rd.pPosition = &vPos;
				rd.fRadius = m_sInfo._fMeteorDamage[0];
				rd.nPower = (int)m_sInfo._fMeteorDamage[1];
				rd.nPushY = 60; 
				rd.nPushZ = 8;
				rd.nTeam = m_hParent->GetTeam();
				rd.nNotGenOrb = m_nOrbGenType;
				rd.nNotLookAtHitUnit = 1;
				rd.nType = HT_ABIL_FIRE;
				rd.nOrbGenPer = m_nOrbGenPer;
				rd.nHitRemainFrame = 80;

				CFcWorld::GetInstance().GiveRangeDamage( rd );
				CFcWorld::GetInstance().GivePhysicsRange( m_hParent, &vPos, m_sInfo._fMeteorDamage[0] * 2.f, m_sInfo._fMeteorDamage[1], 1000000, PROP_BREAK_TYPE_SPECIAL_ATTACK );
//				CFcWorld::GetInstance().GivePhysicsRange( m_hParent, &vPos, 2000, 100 );
				m_listMeteorInfo[ii]->fIntensity = 0.0f;
				count2++;
				continue;
			}
		}
		else
		{
			m_listMeteorInfo[ii]->vPos += m_listMeteorInfo[ii]->vDir*m_sInfo._fSpeed;
			m_listMeteorInfo[ii]->fIntensity = -1.0f;
		}

				
		if(m_listMeteorInfo[ii]->bExplode != TRUE)
		g_pFcFXManager->SendMessage(m_listMeteorInfo[ii]->nFxIdx, FX_UPDATE_OBJECT, 
			(DWORD)&m_listMeteorInfo[ii]->vDir, (DWORD)&m_listMeteorInfo[ii]->vPos, (DWORD)&m_sInfo._fSpeed );

		float fIntensity = 0.0f;
		
		g_pFcFXManager->SendMessage(m_listMeteorInfo[ii]->nFxIdx, FX_GET_INTENSITY, (DWORD)&fIntensity );

		if( fIntensity > fIntensity2 )
		{
			fIntensity2 = fIntensity;
		}

		m_fMaxIntensity = fIntensity2;
		
	}

	if( m_sInfo._byUseSceneeffect == 1 )
	{
		float pow = m_nSceneIntensityBuffer;
		if( m_fMaxIntensity > 0.0f )
		{
			pow += m_fMaxIntensity;
		}
		if( m_sInfo._byMaxBeComingNumber == count2 )
		{
			g_BsKernel.SetSceneIntensity(m_nSceneIntensityBuffer);
		}
		else
		{
			g_BsKernel.SetSceneIntensity(pow);
		}
	}
	else if( m_sInfo._byUseSceneeffect == 2 )
	{
		float pow = m_vSceneIntensityBuffer.x;
		if( m_fMaxIntensity > 0.0f )
		{
			pow += m_fMaxIntensity;
		}
		if( m_sInfo._byMaxBeComingNumber == count2 )
		{
			g_BsKernel.SetSceneAddColor(m_vSceneIntensityBuffer.x,m_vSceneIntensityBuffer.x,m_vSceneIntensityBuffer.x);
		}
		else
		{
			g_BsKernel.SetSceneAddColor(pow,pow,pow);
		}
	}
}


void CFcAbilityFire::ReadSOX()
{
}



void CFcAbilityFire::CallMeteor()
{
	static D3DXVECTOR3 vStart;
	static D3DXVECTOR3 vTarget;
	
	static byte count = 0;

	if( count > 3 ) count = 0;

	vStart = m_vCharPos + (float)m_sInfo._nUpForwardDist*m_vCharDir;// + (m_vCharRight*(float)m_sInfo._UpHorizontalDist) + m_vCharPos;
	switch( count )
	{
	case 0:					// 캐릭터 위치에서부터 제일 우측에 위치함
		{
			vStart += ( m_vCharRight*( (float)m_sInfo._UpHorizontalDist * 2.0f + (float)m_sInfo._nUpRadius * 3.0f ));
		}
		break;
	case 1:
		{
			vStart += ( m_vCharRight*( (float)m_sInfo._UpHorizontalDist + (float)m_sInfo._nUpRadius ));
		}
		break;
	case 2:
		{
			vStart += ( -m_vCharRight*( (float)m_sInfo._UpHorizontalDist + (float)m_sInfo._nUpRadius ));
		}
		break;
	case 3:
		{
			vStart += ( -m_vCharRight*( (float)m_sInfo._UpHorizontalDist * 2.0f + (float)m_sInfo._nUpRadius * 3.0f ));
		}
		break;
	}
	float fRandom = RandomNumberInRange( -(float)m_sInfo._nUpRadius, (float)m_sInfo._nUpRadius);

	vStart.x +=  fRandom;
	fRandom = RandomNumberInRange( -(float)m_sInfo._nUpRadius, (float)m_sInfo._nUpRadius);
	vStart.z +=  fRandom;
	vStart.y =  m_vCharPos.y + (float)m_sInfo._nStartHeight;


	
	D3DXVECTOR3 tmpV1,tmpV2;
	vTarget = m_vCastPoint + m_vCharDir*(float)m_sInfo._calledMeteorNum*900.0f;
	tmpV1 = m_vCharRight*(float)m_sInfo._nDownRadius + ( m_vCharDir*((float)m_sInfo._nDownRadius/3.0f) );
	tmpV2 = -m_vCharRight*(float)m_sInfo._nDownRadius + ( -m_vCharDir*((float)m_sInfo._nDownRadius/3.0f) );
	vTarget.x +=  RandomNumberInRange( tmpV1.x, tmpV2.x );
	vTarget.z +=  RandomNumberInRange( tmpV1.z, tmpV2.z );
	
	float	fMapXSize, fMapYSize;
	g_FcWorld.GetMapSize( fMapXSize, fMapYSize );
	if (vTarget.x >= 0.0f && vTarget.x < fMapXSize && 
		vTarget.z >= 0.0f && vTarget.z < fMapYSize)
		vTarget.y = g_FcWorld.GetLandHeight( vTarget.x, vTarget.z );
	else
		vTarget.y = 0.0f;
	

	

	MeteorInfo* pInfo = new MeteorInfo;
	pInfo->vPos = vStart;

	D3DXVec3Subtract( &vStart, &vTarget, &vStart);
	D3DXVec3Normalize(&vStart, &vStart);
	pInfo->vDir = vStart;

	

	int idx = g_pFcFXManager->Create(FX_TYPE_METEOR);
	g_pFcFXManager->SendMessageQueue(idx, FX_INIT_OBJECT, (DWORD)m_sInfo._fScale, (DWORD)&m_sInfo._fExplodeScale , (DWORD)&m_sInfo._nDeltaTime2 );
	g_pFcFXManager->SendMessageQueue(idx, FX_ADD_VALUE, (DWORD)m_sInfo._fTrailValue, (DWORD)m_sInfo._fPiceOfMeteorValue );
	g_pFcFXManager->SendMessageQueue(idx, FX_ADD_VALUE2, (DWORD)m_sInfo._fPiceOfMeteorDamage, (DWORD)m_sInfo._fRadius ,(DWORD)m_sInfo._fSceneEft);
	g_pFcFXManager->SendMessageQueue(idx, FX_PLAY_OBJECT);
	pInfo->nFxIdx = idx;
	pInfo->bExplode = FALSE;

	m_listMeteorInfo.push_back(pInfo);
	count++;
}