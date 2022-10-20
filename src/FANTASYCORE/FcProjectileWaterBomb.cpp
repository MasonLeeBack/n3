#include "StdAfx.h"
#include "FcProjectileWaterBomb.h"
#include "FcFXManager.h"
#include "FcUnitObject.h"
#include "FcTroopObject.h"
#include "FcGlobal.h"
#include "ASData.h"
#include "DebugUtil.h"
#include "Data/SignalType.h"
#include "Box3.h"
#include "Ray3.h"
#include "BsPhysicsMgr.h"
#include "IntLin3Box3.h"
#include "FcWorld.h"
#include ".\\Data\\\Sound\\FcSoundPlayDefinitions.h"
#include "FcSoundManager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG

ASSignalData *CFcProjectileWaterBomb::s_pHitSignal = NULL;


//일반
const float g_fScale1[9] = { 100.0f, 100.0f, 100.0f, 100.0f, 100.0f, 100.0f, 100.0f, 100.0f, 100.0f};
const float g_fDamageRadius1[9] = { 260.0f, 260.0f, 260.0f, 270.0f, 270.0f, 280.0f, 280.0f, 290.0f, 300.0f};
const int g_nNormalPower[9] = { 180, 180, 180, 180, 180, 180, 180, 180, 180};
// 홀딩
const float g_fScale2[9] = { 150.0f, 160.0f, 170.0f, 180.0f, 190.0f, 200.0f, 210.0f, 220.0f, 230.0f };
const float g_fDamageRadius2[9] = { 280.0f, 300.0f, 300.0f, 320.0f, 320.0f, 330.0f, 330.0f, 340.0f, 350.0f};
const int g_nSpecialPower[9] = { 250, 250, 250, 250, 250, 250, 250, 250, 250};
// 오브공격시
const float g_fScale3[9] = { 180.0f, 190.0f, 200.0f, 210.0f, 220.0f, 230.0f, 240.0f, 250.0f, 260.0f };
const float g_fDamageRadius3[9] = { 350.0f, 360.0f, 370.0f, 380.0f, 390.0f, 400.0f, 410.0f, 420.0f, 430.0f};
const int g_nOrbPower[9] = { 500, 500, 500, 500, 500, 500, 500, 500, 500};

// 일반 ( 전방밀기값 과 상방 밀기값 )
const float g_fPushZsmall[9] = { -8.0f, -8.0f, -8.0f, -8.0f, -8.0f, -8.0f, -8.0f, -8.0f, -8.0f };
const float g_fPushYsmall[9] = { 32.0f, 32.0f, 32.0f, 32.0f, 32.0f, 32.0f, 32.0f, 32.0f, 32.0f };

// 홀딩 ( 전방밀기값 과 상방 밀기값 )
const float g_fPushZBig[9] = { -10.0f, -10.0f, -10.0f, -10.0f, -10.0f, -10.0f, -10.0f, -10.0f, -10.0f };
const float g_fPushYBig[9] = { 32.0f, 32.0f, 32.0f, 34.0f, 34.0f, 36.0f, 38.0f, 38.0f, 40.0f };

// 오브공격시 나가는 물덩어리 ( 전방밀기값 과 상방 밀기값 )
const float g_fPushZorb[9] = { -14.0f, -14.0f, -14.0f, -14.0f, -14.0f, -14.0f, -14.0f, -14.0f, -14.0f };
const float g_fPushYorb[9] = { 42.0f, 42.0f, 42.0f, 42.0f, 42.0f, 42.0f, 42.0f, 42.0f, 42.0f };

// 물폭탄 투하
const float g_fPushZmini[9] = { -4.0f, -4.0f, -4.0f, -4.0f, -4.0f, -4.0f, -4.0f, -4.0f, -4.0f };
const float g_fPushYmini[9] = { 5.0f, 5.0f, 5.0f, 5.0f, 5.0f, 5.0f, 5.0f, 5.0f, 5.0f };
const int g_nMiniPower[9] = { 140, 140, 140, 140, 140, 140, 140, 140, 140};


CFcProjectileWaterBomb::CFcProjectileWaterBomb( CCrossVector *pCross )
: CFcProjectile( pCross )
{
	m_nDestroyTick = -1;

	if( s_pHitSignal == NULL ) {
		s_pHitSignal = new ASSignalData;
		s_pHitSignal->m_pParam = new int[HIT2_SIGNAL_LENGTH];
		s_pHitSignal->m_nID	   = SIGNAL_TYPE_HIT_TEMP;
		s_pHitSignal->m_NumParam = HIT2_SIGNAL_LENGTH;
		s_pHitSignal->m_Hit2WeaponMaterial = WT_TYURRU;
		s_pHitSignal->m_Hit2SoundAction = ATT_LITTLE;
		

		memset( s_pHitSignal->m_pParam, 0, sizeof(int) * HIT2_SIGNAL_LENGTH );

		s_pHitSignal->m_Hit2AniType = ANI_TYPE_DOWN;
		s_pHitSignal->m_Hit2AniIndex = 0;
		s_pHitSignal->m_Hit2Particle = -1;
	}

	m_nExpFxId = -1;
	m_nExpFxTemplate = -1;
	m_fScale = 0.f;
	m_pActor = NULL;
	SetRtti(8);

	m_bOnce = false;

	m_nCurrentTick = 0;
	SetTotalTick(-1);
	SetAddScale(0.0f);
	SetClassify(-1);
}

CFcProjectileWaterBomb::~CFcProjectileWaterBomb()
{	
	SAFE_DELETE( m_pActor );

	if(m_nExpFxId != -1)
	{
		g_BsKernel.DeleteObject(m_nExpFxId);
		m_nExpFxId = -1;
	}

	//if(m_nBubbleFxId != -1)
	//{
	//	g_BsKernel.DeleteObject(m_nBubbleFxId);
	//	m_nBubbleFxId = -1;
	//}

	if(m_nExpFxTemplate != -1) {
		g_BsKernel.ReleaseFXTemplate(m_nExpFxTemplate);
		m_nExpFxTemplate = -1;
	}
}

void CFcProjectileWaterBomb::ProcessDamage()
{
	std::vector<GameObjHandle> VecList;

	float fRadius = 0.0f;
	if( GetClassify() == 1 || GetClassify() == 3) {		// 일반적인 큰 녀석이면.

		if( GetTotalTick() < 30 )
		{
			fRadius = g_fDamageRadius1[m_Param.hParent->GetLevel()];
		}
		else
		{
			fRadius = g_fDamageRadius2[m_Param.hParent->GetLevel()];
		}
	}
	else if( GetClassify() == 4 ) {			// 오브공격시.
		fRadius = g_fDamageRadius3[m_Param.hParent->GetLevel()];
	}
	else /*if( GetClassify() == 2 )*/ {			// 작은 물덩어리 이면.
		fRadius = 150.0f;
	}
	

	
	CFcWorld::GetInstance().GetEnemyObjectListInRange( m_Param.hParent->GetTeam(), &m_Cross.m_PosVector, fRadius, VecList,false );
	if( VecList.empty() ) return;
	int nSize = VecList.size();
	D3DXVECTOR3 vPos;

	bool bOk = false;
	for(int ii = 0; ii < nSize; ++ii)
	{
		if( VecList[ii]->GetHP() <= 0 ) continue;

		vPos = VecList[ii]->GetPos();
		if( fabs(m_Cross.m_PosVector.y - vPos.y) < m_fScale*1.5f	)
		{
			bOk = true;
			break;
		}
	}
	VecList.clear();

	if(bOk)
	{
		ProcessSplashDamage();
		m_nDestroyTick = 1;
	}

	// 반경안에 물덩이 검출 있으면 물덩이 폭발 시킨다. 없으면 넘어간다.
	std::vector<CFcProjectile*> vecProjectile;
	ProjetileHandle Handle; 
	Handle->GetProjectileList(1000, vecProjectile);
	int nPorjectileSize = vecProjectile.size();
	CFcProjectile* tmpProjectile = NULL;
	if( nPorjectileSize > 0 )
	{
		for( int j = 0 ; j < nPorjectileSize ; j++ )
		{
			if( vecProjectile[j] == this ) continue;

			D3DXVECTOR3 ProjectilePos = vecProjectile[j]->GetPos();
			D3DXVECTOR3 vDist = ProjectilePos - m_Cross.m_PosVector;
			float fRad = D3DXVec3Length(&vDist );
			
			if( fRad <= fRadius )
			{
				tmpProjectile = vecProjectile[j];
				((CFcProjectileWaterBomb*)tmpProjectile)->SetDestoryTick(1);
			}

		}
	}
}

void CFcProjectileWaterBomb::ProcessSplashDamage()
{
	std::vector<GameObjHandle> VecList;

	float fRadius = 0.0f;
	if( GetClassify() == 1 || GetClassify() == 3) {		// 일반적인 큰 녀석이면.

		if( GetTotalTick() < 30 )
		{
			fRadius = g_fDamageRadius1[m_Param.hParent->GetLevel()];
		}
		else
		{
			fRadius = g_fDamageRadius2[m_Param.hParent->GetLevel()];
		}
	}
	else if( GetClassify() == 4 ) {			// 오브어택.
		fRadius = g_fDamageRadius3[m_Param.hParent->GetLevel()];
	}
	else /*if( GetClassify() == 2 )*/ {			// 작은 물덩어리 이면.
		fRadius = 150.0f;
	}

	CFcWorld::GetInstance().GetEnemyObjectListInRange( m_Param.hParent->GetTeam(), &m_Cross.m_PosVector, fRadius, VecList,false );
	if( VecList.empty() ) return;


	for( DWORD i=0; i<VecList.size(); i++ ) 
	{
		if( VecList[i] == m_Param.hParent ) 
			continue;

		if( VecList[i]->GetHP() <= 0 ) continue;

		if( !VecList[i]->IsHittable( m_Param.hParent ) ) 
			continue;


		HIT_PARAM HitParam;
		//		s_pHitSignal->m_pParam[1] = m_Param.pSignal->m_pParam[4];
		//		s_pHitSignal->m_pParam[6] = m_Param.pSignal->m_pParam[5];
		HitParam.ObjectHandle = m_Param.hParent;
		if( GetClassify() == 1 ) {		// L트리거 Y.

			HitParam.nAttackPower = g_nNormalPower[m_Param.hParent->GetLevel()];//m_Param.hParent->GetAttackPower();
			HitParam.fVelocityY = g_fPushYsmall[m_Param.hParent->GetLevel()];
			HitParam.fVelocityZ = -g_fPushZsmall[m_Param.hParent->GetLevel()];
		}
		else if( GetClassify() == 3 ) {			// L트리거 Y홀딩.
			HitParam.nAttackPower = g_nSpecialPower[m_Param.hParent->GetLevel()];//m_Param.hParent->GetAttackPower();
			HitParam.fVelocityY = g_fPushYBig[m_Param.hParent->GetLevel()];
			HitParam.fVelocityZ = -g_fPushZBig[m_Param.hParent->GetLevel()];
		}
		else if( GetClassify() == 4 ) {			// 오브어택
			HitParam.nAttackPower = g_nOrbPower[m_Param.hParent->GetLevel()];//m_Param.hParent->GetAttackPower();
			HitParam.fVelocityY = g_fPushYorb[m_Param.hParent->GetLevel()];
			HitParam.fVelocityZ = -g_fPushZorb[m_Param.hParent->GetLevel()];
		}
		else {			// 물폭탄 투하 ( -_-; )
			HitParam.nAttackPower = g_nMiniPower[m_Param.hParent->GetLevel()];//m_Param.hParent->GetAttackPower();
			HitParam.fVelocityY = g_fPushYmini[m_Param.hParent->GetLevel()];//20.0f * m_fScale * 0.2f;
			HitParam.fVelocityZ = -g_fPushZmini[m_Param.hParent->GetLevel()];//-10.0f * m_fScale * 0.2f;
		}
		
		HitParam.pHitSignal = s_pHitSignal;
		HitParam.Position = m_Cross.m_PosVector;
		HitParam.nHitRemainFrame = 6;
		HitParam.pHitSignal->m_Hit2AttackPoint = 100;
		
		VecList[i]->CmdHit( &HitParam );
	}
	VecList.clear();
}


int CFcProjectileWaterBomb::Initialize( int nSkinIndex, PROJECTILE_PARAM *pParam )
{
	BsAssert( m_nEngineIndex == -1 );
	BsAssert( m_nExpFxTemplate == -1 );

	int nResult = CFcBaseObject::Initialize( nSkinIndex,-1,WATER_BOME );
	m_Param = *pParam;
		
	float fSize = m_Param.fSize;

	m_fScale = 0.f;
	m_PrevPos = m_Cross.m_PosVector;
	m_nDestroyTick = 600; 

	BOOL bCheckZBuffer = TRUE;
	g_BsKernel.SendMessage( m_nEngineIndex, BS_SET_SUBMESH_DEPTHCHECKENABLE, 0, (DWORD)bCheckZBuffer );

	return nResult;
}

struct sWaterBomb
{
	int _nDestroyTick;
	int _nTotalTick;
	int _nClassify;
	float _fAddScale;
};

void CFcProjectileWaterBomb::SetCustomValue( void* p_pValue )
{
	sWaterBomb* tmp = (sWaterBomb*)p_pValue;
	if( tmp->_fAddScale > 0 )
        SetAddScale(tmp->_fAddScale );
	if( tmp->_nTotalTick > 0 )
        SetTotalTick(tmp->_nTotalTick);
	if( tmp->_nDestroyTick > 0 )
        SetDestoryTick(tmp->_nDestroyTick);
	if( tmp->_nClassify > 0 )
        SetClassify(tmp->_nClassify);

	
	
	if( m_nExpFxTemplate == -1 ) {
		char szFxFilePath[MAX_PATH];
		g_BsKernel.chdir("fx");
		if( GetClassify() == 1 ) {			// 일반적인 큰 녀석들.
			sprintf(szFxFilePath,"%s%s",g_BsKernel.GetCurrentDirectory(),"water_boom.bfx" );//aaa.bfx");//
		}
		else if( GetClassify() == 2 )		// 물폭탄투하
		{
			sprintf(szFxFilePath,"%s%s",g_BsKernel.GetCurrentDirectory(),"water_boom_small.bfx" );//aaa.bfx");//
		}
		else if( GetClassify() == 3 )		// 홀딩 물덩이
		{
			sprintf(szFxFilePath,"%s%s",g_BsKernel.GetCurrentDirectory(),"water_boom.bfx" );//aaa.bfx");//
		}
		else if( GetClassify() == 4 )		// 오브어택 물덩이
		{
			sprintf(szFxFilePath,"%s%s",g_BsKernel.GetCurrentDirectory(),"water_boom.bfx" );//aaa.bfx");//
		}
		m_nExpFxTemplate = g_BsKernel.LoadFXTemplate(-1, szFxFilePath);
		g_BsKernel.chdir("..");
	}
	

}
void CFcProjectileWaterBomb::SetDestoryTick( int nTick )
{
	m_nDestroyTick = nTick;
}

void CFcProjectileWaterBomb::Process()
{
	int nState = 0;

	if( GetTotalTick() < 0 ) return;

	if( m_nCurrentTick < GetTotalTick() )
	{
		// 스케일을 점점 키운다.
		if( GetClassify() == 1 || GetClassify() == 2 || GetClassify() == 3 )
		{
			if( GetTotalTick() < 30 ){
				// 일반
				if( m_fScale < g_fScale1[ m_Param.hParent->GetLevel() ] )
					m_fScale = GetScale() + GetAddScale();
				else
					m_fScale = g_fScale1[ m_Param.hParent->GetLevel() ];
			}
			else{
				// 홀딩
				if( m_fScale < 100 ) m_fScale = 100;
				if( m_fScale < g_fScale2[ m_Param.hParent->GetLevel() ] )
					m_fScale = GetScale() + GetAddScale();
				else
					m_fScale = g_fScale2[ m_Param.hParent->GetLevel() ];
			}
		}
		else {
			if( m_fScale < g_fScale3[ m_Param.hParent->GetLevel() ] )
				m_fScale = GetScale() + GetAddScale();
			else
				m_fScale = g_fScale3[ m_Param.hParent->GetLevel() ];
		}
		

		m_nCurrentTick++;
	}
	else
	{
		// 스케일을 현재 스케일로 하고 물리엔진 시작
		nState = 1;
		if(!m_pActor) {
			m_pActor = CBsPhysicsSphere::Create( GetScale()*0.6f, *( ( D3DXMATRIX *)m_Cross ) );
			if( !m_pActor->GetActor() ) return;
			m_pActor->GetActor()->setLinearDamping( 2.0f );
			m_pActor->GetActor()->setAngularDamping( 0.0f );
			m_pActor->GetActor()->setMaxAngularVelocity( FLT_MAX );
			m_pActor->SetGroup("WaterBomb");
		}
	}

	D3DXMatrixScaling( &m_ScaleMat, m_fScale, m_fScale, m_fScale);
	
	if( nState != 1 ) return;

	if( m_nDestroyTick > 0 ) 
	{
		--m_nDestroyTick;
		if( m_nDestroyTick == 0 ) 
		{
			g_pSoundManager->Play3DSound(NULL,SB_COMMON,"AT_D_SWING_L",&m_Cross.m_PosVector );
			if( !m_bOnce ) {
				m_nExpFxId = g_BsKernel.CreateFXObject(m_nExpFxTemplate);
				g_BsKernel.SetFXObjectState( m_nExpFxId, CBsFXObject::PLAY, 1 ); 
				m_bUpdateObject = false;
				SAFE_DELETE( m_pActor );
				ProcessDamage();
				m_bOnce = true;
				SetRtti(2000);
			}
			
		}
	}

	m_PrevPos = m_Cross.m_PosVector;
	D3DXMATRIX ActorMat;
	if(m_pActor)
	{
		m_pActor->GetGlobalPose( ActorMat );
		memcpy( &m_Cross.m_XVector, &ActorMat._11, sizeof( D3DXVECTOR3 ) );
		memcpy( &m_Cross.m_YVector, &ActorMat._21, sizeof( D3DXVECTOR3 ) );
		memcpy( &m_Cross.m_ZVector, &ActorMat._31, sizeof( D3DXVECTOR3 ) );
		memcpy( &m_Cross.m_PosVector, &ActorMat._41, sizeof( D3DXVECTOR3 ) );
	}

	if(m_nExpFxId != -1)
	{
		D3DXMATRIX RenderMat;
		D3DXMatrixIdentity( &RenderMat);
		memcpy( &RenderMat._41, &m_Cross.m_PosVector,  sizeof( D3DXVECTOR3 ) );
		g_BsKernel.UpdateObject( m_nExpFxId, &RenderMat);
		if(g_BsKernel.GetFXObjectState( m_nExpFxId) == CBsFXObject::STOP)
			Delete();
	}

	if( m_bUpdateObject ) 
	{
		ProcessDamage();
	}
}
