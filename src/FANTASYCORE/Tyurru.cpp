#include "StdAfx.h"
#include ".\tyurru.h"
#include "FcWorld.h"
#include "FcHeroObject.h"
#include "FcAbilityManager.h"
#include "FcFxManager.h"
#include "FcUtil.h"
#include "FcProjectile.h"
#include "FcProjectileWaterBomb.h"
#include "InputPad.h"
#include "FcGlobal.h"
#include "FcInterfaceManager.h"
#include "FcRealtimeMovie.h"
#include "Data\\FxList.h"
#include ".\\data\\SignalType.h"
#include "FcSoundManager.h"
#include "FcAdjutantObject.h"
#include "Ray3.h"
#include "IntLin3Box3.h"


// 레벨에 따른 공력관련 value table 사정거리.
const float g_fvalue1[MAX_HERO_LEVEL] = { 600.0f, 700.0f, 800.0f, 800.0f, 800.0f, 800.0f, 800.0f, 800.0f, 800.0f };

// 레벨에 따른 공력관련 value table 쏘는발수.
const int g_nvalue2[MAX_HERO_LEVEL] = { 1, 1, 1, 2, 2, 2, 3, 3, 3 };

// 물기둥 공격 거리 ( 캐릭터와 물기둥 생성 위치 사이의 거리 )
const float g_fDistColumnWater = 370.0f;

// 튜르르 일반공격 적 검출 각도
const float g_fShotAngle[MAX_HERO_LEVEL] = { 0.996f , 0.996f, 0.991f, 0.991f, 0.984f, 0.984f, 0.976f, 0.976f, 0.965f };

// 튜르르 오브어택 공격 각도
const float g_fOrbShotAngle[MAX_HERO_LEVEL] = { 5.0f , 5.0f, 7.5f, 7.5f, 10.0f, 10.0f, 12.5f, 12.5f, 15.0f };

// 튜르르 일반공격시 물덩이 던지기 Tick (값이 작아질수록 빨리 던진다.)
const int g_nShotLumpOfWaterTick = 12;


// 튜르르 물덩이 날리기 레벨에 따른 날아가는 거리 ( 현재 위치에서 전방으로 : 100 이 1 미터 )
const float g_fShotWaterDistance[MAX_HERO_LEVEL] = { 400.0f, 400.0f, 420.0f, 420.0f, 450.0f, 450.0f, 480.0f, 480.0f, 510.0f };

// 튜르르 의 바로 앞쪽에 있는 적은 검출각을 높여서 어색하지 않게 한다. 그 거리
const float g_fMinimumDist = 180.0f;

// 튜르르 홀딩 물덩어리 생성시 튜르르 전방 앞에서 생성될 거리 값.
const float g_fDist[MAX_HERO_LEVEL] = { 80.0f, 80.0f, 90.0f, 90.0f, 100.0f, 100.0f, 105.0f, 120.0f, 120.0f };

float Min( float a, float b)
{
	return a >= b ? b : a;
}

D3DXVECTOR3 Vec3RotationAxis( D3DXVECTOR3 *vOut, D3DXVECTOR3 vIn, float fAngle, D3DXVECTOR3 vAxis )
{
	D3DXMATRIX mat;
	D3DXMatrixIdentity(&mat);
	D3DXMatrixRotationAxis(&mat, &vAxis, fAngle);
	D3DXVec3TransformCoord(vOut,&vIn, &mat);

	return *vOut;
}
/*---------------------------------------------------------------------------------
-
-			CTyurru::CTyurru()
-					; 생성자.
-					  
-
---------------------------------------------------------------------------------*/
CTyurru::CTyurru(CCrossVector *pCross) : CFcHeroObject( pCross )
{
	SetHeroClassID( Class_ID_Hero_Tyurru );
	m_nFxIndex = -1;
	m_bOrbAttack = false;

	m_nNoramlAttackFx = -1;
	m_nColumnWater = -1;
	m_nWaterIndex	= -1;
	m_nColumnWaterID[SMALL_COLUMNWATER] = -1;
	m_nColumnWaterID[BIG_COLUMNWATER] = -1;

	m_nSecondAni = -1;
	m_fSecondFrame = 0.0f;
	m_bUseSecondAni = false;
	m_nWaterSphereSkin = -1;
	m_nShotWaterDrop = -1;
	m_sShotWaterDrop._bShotWaterDrop = false;
	m_sShotWaterDrop._nTick = 0;
	m_sShotWaterDrop._bNormal = false;
	//m_ProjectileHandle = NULL;
}






/*---------------------------------------------------------------------------------
-
-			CTyurru::~CTyurru()
-					; 소멸자.
-					  
-
---------------------------------------------------------------------------------*/
CTyurru::~CTyurru(void)
{
	if( m_nWaterSphereSkin != -1 )
	{
		g_BsKernel.ReleaseSkin(m_nWaterSphereSkin);
		m_nWaterSphereSkin = -1;
	}
}


void CTyurru::Initialize( INITGAMEOBJECTDATA* pData, int nForce, int nTeam, TroopObjHandle hTroop, float fScaleMin, float fScaleMax )
{
	if( m_nNoramlAttackFx == -1 )
	{
		m_nNoramlAttackFx = g_pFcFXManager->Create(FX_TYPE_SPOUTSOFWATER);
		g_pFcFXManager->SendMessage(m_nNoramlAttackFx,FX_INIT_OBJECT, (DWORD)&((GameObjHandle)m_Handle) );
		g_pFcFXManager->SendMessage(m_nNoramlAttackFx,FX_PLAY_OBJECT);
	}

	if( m_nColumnWater == -1 )
	{
		m_nColumnWater = g_pFcFXManager->Create(FX_TYPE_COLUMNWATER);
		g_pFcFXManager->SendMessage(m_nColumnWater,FX_INIT_OBJECT, -99, (DWORD)&((GameObjHandle)m_Handle));
		//g_pFcFXManager->SendMessage(m_nColumnWater,FX_PLAY_OBJECT);

		if( m_nColumnWaterID[SMALL_COLUMNWATER] == -1)
		{
			m_nColumnWaterID[SMALL_COLUMNWATER] = FX_TYURURUWATER01;
			m_nColumnWaterID[BIG_COLUMNWATER] = FX_TYURURUWATER02;
		}
			
	}// 여기까지 초기화에 들어갈 부분.

	CFcHeroObject::Initialize( pData, nForce, nTeam, hTroop, fScaleMin, fScaleMax );

	m_nSecondBoneIndex = g_BsKernel.SendMessage( m_nEngineIndex, BS_GET_BONE_INDEX, ( DWORD )"Bip01 Spine" );

	m_Levia.Initialize(m_Handle);
	m_pParent = (GameObjHandle)m_Handle;	

	if( m_nWaterSphereSkin == -1 )
	{
		g_BsKernel.chdir("fx");
        m_nWaterSphereSkin = g_BsKernel.LoadSkin( -1, "WaterBombSphere.skin" );
		g_BsKernel.chdir("..");
	}
	if( m_nShotWaterDrop == -1 )
	{
		m_nShotWaterDrop = g_pFcFXManager->Create(FX_TYPE_SHOTSKIN);
		g_pFcFXManager->SendMessage(m_nShotWaterDrop,FX_INIT_OBJECT, (DWORD)m_nWaterSphereSkin, FX_TYRURUWATERBOOM ,(DWORD)&((GameObjHandle)m_Handle) );
		g_pFcFXManager->SendMessage(m_nShotWaterDrop,FX_PLAY_OBJECT);
	}
}




/*---------------------------------------------------------------------------------
-
-			CTyurru::Render()
-					; .
-					  
-
---------------------------------------------------------------------------------*/
bool CTyurru::Render()
{
	bool bRet;
	D3DXVECTOR3 vRotate;

	bRet = CFcHeroObject::Render();
	if( m_nSecondAni != -1 )
	{
		SET_ANI_INFO AniInfo;

		g_BsKernel.SetCurrentAni( m_nEngineIndex, m_nSecondAni, ( float )( int )( m_fSecondFrame ) );
		AniInfo.nAniIndex = m_nAniIndex;
		AniInfo.fFrame = m_fFrame;
		AniInfo.nBoneIndex = m_nSecondBoneIndex;
		g_BsKernel.SendMessage( m_nEngineIndex, BS_SETCURRENTANI_BONE, ( DWORD )&AniInfo );
	}

	g_BsKernel.SendMessage( m_nEngineIndex, BS_SET_CLEAR_BONE_ROTATION );
	if( m_nSecondAni == 81 )
	{
		vRotate = D3DXVECTOR3( 0.0f, 60.0f, 0.0f );
		g_BsKernel.SendMessage( m_nEngineIndex, BS_SET_BONE_ROTATION, ( DWORD )"Bip01 Spine", (DWORD)&vRotate );
	}
	else if( m_nSecondAni == 82 )
	{
		vRotate = D3DXVECTOR3( 0.0f, -60.0f, 0.0f );
		g_BsKernel.SendMessage( m_nEngineIndex, BS_SET_BONE_ROTATION, ( DWORD )"Bip01 Spine", (DWORD)&vRotate );
	}

	if( m_Levia.GetLive() && IsEnable() )
        m_Levia.Update();

	return bRet;
}




/*---------------------------------------------------------------------------------
-
-			CTyurru::Process()
-					; 키보드 처리 추가.
-					  
-
---------------------------------------------------------------------------------*/
void CTyurru::Process()
{
	static int nOrbCheck = 0;
	static bool bFirst = true;
	if( GetOrbAttack() )
	{
		nOrbCheck = 1;
		
		if(bFirst)
		{
			if( m_nFxIndex == -1 )
			{
				m_nFxIndex = g_pFcFXManager->Create(FX_TYPE_DROPOFWATER);
				g_pFcFXManager->SendMessage(m_nFxIndex,FX_INIT_OBJECT, (DWORD)&m_Cross.m_PosVector, (DWORD)&((GameObjHandle)m_Handle) );
				g_pFcFXManager->SendMessage(m_nFxIndex,FX_PLAY_OBJECT);
			}
			
			bFirst = false;
		}
		
	}
	else
	{
		if( nOrbCheck == 1)
		{
			nOrbCheck = 0;
			SAFE_DELETE_FX( m_nFxIndex, FX_TYPE_DROPOFWATER);
			//g_pFcFXManager->SendMessage(m_nFxIndex,FX_DELETE_OBJECT);
			//m_nFxIndex = -1;
			bFirst = true;
		}
	}

	static int nTick = 0; // 하드코딩된것 -_-;; 수정예정.

	if( m_sShotWaterDrop._bShotWaterDrop )
	{
		if( ++nTick >= m_sShotWaterDrop._nTick )
		{
			m_sShotWaterDrop._bShotWaterDrop = false;
			m_sShotWaterDrop._nTick = 0;
			m_sShotWaterDrop._bNormal = false;
			nTick = 0;
		}
		else
		{
			if( m_sShotWaterDrop._bNormal ) {
				float faddscale = 20.0f;
				g_pFcFXManager->SendMessage(m_nShotWaterDrop, FX_ADD_VALUE, 0, (DWORD)&faddscale);
			}
			else {
				float faddscale = 6.0f;
				g_pFcFXManager->SendMessage(m_nShotWaterDrop, FX_ADD_VALUE, (DWORD)&faddscale, 0);
			}
			
		}
	}
	else nTick=0;
	PushWaterBomb();

	if( m_Levia.GetLive() && IsEnable() )
        m_Levia.Process();
	CFcHeroObject::Process();

	static float s_fLandGap = 50.0f;
	if( m_nAniIndex == 17 )		// 공중에 떠다니는 상태
	{
		if( (m_Cross.m_PosVector.y - CFcWorld::GetInstance().GetLandHeight2( m_Cross.m_PosVector.x, m_Cross.m_PosVector.z, m_Cross.m_PosVector.y ) < s_fLandGap ) ||
			IsFloatLog() )
		{
			ChangeAnimationByIndex( 18 );
			m_fFrame = m_nCurAniLength * 0.4f;
		}
	}
	//int nPressTick;

	if( ( m_nAniIndex == 66 ) || ( m_nAniIndex == 67 ) || ( m_nAniIndex == 77 ) )
	{
		if( CInputPad::GetInstance().GetKeyPressTick( g_FCGameData.nEnablePadID, PAD_INPUT_LTRIGGER ) )
		{
			m_bEnableRotation = false;
		}
		m_bUseSecondAni = true;
	}
	else
	{
		if( m_bUseSecondAni )
		{
			m_MoveDir = m_Cross.m_ZVector;
		}
		m_bUseSecondAni = false;
		m_nSecondAni = -1;
	}
	if( m_nSecondAni != -1 )
	{
		int nAniLength;

		nAniLength = g_BsKernel.GetAniLength( m_nEngineIndex, m_nSecondAni );
		m_fSecondFrame += GetFrameAdd();
		if( ( nAniLength - 1 ) < m_fSecondFrame )
		{
			m_fSecondFrame = 0.0f;
		}
	}

}

void CTyurru::PushWaterBomb()
{
	std::vector<CFcProjectile*> vecProjectile;
	ProjetileHandle Handle; 
	Handle->GetProjectileList2(1000, 1001, vecProjectile);
	int nPorjectileSize = vecProjectile.size();
	D3DXVECTOR3 vTarget2(0,0,0);

	bool bChangeAni1 = false;
	bool bChangeAni2 = false;

	if( nPorjectileSize > 0 )
	{
		for( int j = 0 ; j < nPorjectileSize ; j++ )
		{
			D3DXVECTOR3 ProjectilePos = vecProjectile[j]->GetPos();
			D3DXVECTOR3 vDist = ProjectilePos - m_Cross.m_PosVector;
			float fDist = D3DXVec3Length(&vDist);
			vDist.y = 0;
			D3DXVec3Normalize(&vDist, &vDist);
			float fDot = D3DXVec3Dot( &m_Cross.m_ZVector, &vDist);

			float fScale = ((CFcProjectileWaterBomb*)vecProjectile[j])->GetScale();
			
			float fCross = 1.45f;

			if( fScale <= 100 ) fCross = 2.0f;

			if( fDot <= 1.0f && fDot >= 0.9f && fDist <= fScale*fCross )
			{
				if( !m_bOrbAttack && (GetAniIndex()==3 || GetAniIndex()==5 || GetAniIndex()==86 || GetAniIndex()==85 ) ) {
					if(vecProjectile[j]->GetRtti() == 1000 ) { // 일반물덩어리
						int nTick = ((CFcProjectileWaterBomb*)vecProjectile[j])->GetDestoryTick() ;
						((CFcProjectileWaterBomb*)vecProjectile[j])->SetDestoryTick(++nTick);
						bChangeAni1 = true;
						
					}
					else if( vecProjectile[j]->GetRtti() == 1001 ){ // 홀딩 물덩어리
						int nTick = ((CFcProjectileWaterBomb*)vecProjectile[j])->GetDestoryTick() ;
						((CFcProjectileWaterBomb*)vecProjectile[j])->SetDestoryTick(++nTick);
						bChangeAni2 = true;
						
					}
					
				}
			}//if( fDot <= 1.0f && fDot >= 0.9f && fDist <= ((CFcProjectileWaterBomb*)vecProjectile[j])->GetScale()*1.85f )
		}// for
	}// if( nPorjectileSize > 0 )
	

	if( bChangeAni1 )
	{
		ChangeAnimationByIndex(85);
		ChangeSecondAniByIndex( -1 );
	}
	else if( bChangeAni2 )
	{
		ChangeAnimationByIndex(86);
		ChangeSecondAniByIndex( -1 );
	}
	else if( GetAniIndex()==86 || GetAniIndex()==85 )
	{
		ChangeAnimationByIndex( 0 );
	}
}

int g_Testxxx = -1;
bool CTyurru::IsFloatLog()
{
	BsAssert( m_hTroop );

	std::vector<CFcProp*> vecProps;
	D3DXVECTOR3 Pos = GetPos();
	g_FcWorld.GetActiveProps( Pos, 1000.f, vecProps );
	int nCnt = vecProps.size();
	for( int i=0; i<nCnt; i++ )
	{
		CFcProp* pProp = vecProps[i];
		if( pProp == NULL )
			continue;

		// 공중에 있는 통나무 418번 하드코딩
		if( g_Testxxx  = pProp->GetTypeDataIndex() == 418 )
		{
			Box3 Box;
			Segment3 Seg3;
			pProp->GetBox3( Box );

			Seg3.P.x = m_Cross.m_PosVector.x;
			Seg3.P.y = m_Cross.m_PosVector.y;
			Seg3.P.z = m_Cross.m_PosVector.z;

			Seg3.D.x = m_Cross.m_PosVector.x - m_PrevPos.x;
			Seg3.D.y = m_Cross.m_PosVector.y - m_PrevPos.y;
			Seg3.D.z = m_Cross.m_PosVector.z - m_PrevPos.z;
			if( TestIntersection( Seg3, Box ) == true ) {
				return true;
			}
		}
	}
	return false;
}

bool CTyurru::IsFlockable( GameObjHandle Handle )
{
	// 활강중일때는 Flocking 안한다.
	if( Handle == m_Handle ) return false;
	if( Handle->GetParentLinkHandle() == m_Handle ) return false;

	switch( Handle->GetClassID() ) {
		case Class_ID_Hero:
			switch( ((CFcHeroObject*)Handle.GetPointer())->GetHeroClassID() ) {
				case Class_ID_Hero_VigkVagk:	return true;
				default:	break;
			}
			break;
		case Class_ID_Catapult: return true;
		case Class_ID_MoveTower: return true;
		case Class_ID_Adjutant:	
			switch( ((CFcAdjutantObject*)Handle.GetPointer())->GetAdjutantClassID() ) {
				case Class_ID_Adjutant_KingFrog:	return true;
				default: break;
			}
			break;
		default:
			break;
	}
	if( m_nCurAniType == ANI_TYPE_WALK && m_nCurAniTypeIndex == 1 ) return false;
	return CFcHeroObject::IsFlockable( Handle );
}

void CTyurru::ProcessFlocking()
{
	if( m_nCurAniType == ANI_TYPE_WALK && m_nCurAniTypeIndex == 1 ) return;
	return CFcHeroObject::ProcessFlocking();
}

void CTyurru::StartOrbSpark()
{
	m_nSecondAni = -1;
	CFcHeroObject::StartOrbSpark();
}

void CTyurru::Resume()
{
	if( ( m_nCurAniType == ANI_TYPE_WALK ) && ( m_nCurAniTypeIndex == 1 ) )
	{
		return;
	}

	CFcHeroObject::Resume();
}

void CTyurru::ShotWater( int p_nCount, float p_fDist, float p_fDist2, D3DXVECTOR3* p_vPos /*= NULL*/ )
{
	//g_pSoundManager->Play3DSound(NULL,SB_COMMON,"AT_D_SWING_S",p_vPos);
	// 물 줄기 작은거.
	D3DXVECTOR3 vRight;
	D3DXVec3Cross(&vRight, &m_Cross.m_ZVector, &D3DXVECTOR3(0,1,0));
	D3DXVec3Normalize(&vRight,&vRight);
	D3DXVECTOR3 vPreTarget(0,0,0);
	D3DXVECTOR3 vHeroDir = m_Cross.m_ZVector;
	vHeroDir.y = 0;
	D3DXVec3Normalize(&vHeroDir, &vHeroDir);
	float fDotValue = g_fShotAngle[GetLevel()];
	for( int i = 0 ; i < p_nCount ; i++ ) 
	{
		D3DXVECTOR3 vTarget(0,0,0);
		// target1 은 일반 오브젝트 검색
		
		D3DXVECTOR3 vTarget1 = g_FcWorld.GetEnemyObjectInRangeDirNear( m_pParent->GetTeam(), &m_Cross.m_PosVector, &vHeroDir, fDotValue, p_fDist, i , g_fMinimumDist );//g_fvalue1[m_nLevel]*2.0f

		// target2는 물덩어리 검색
		std::vector<CFcProjectile*> vecProjectile;
		ProjetileHandle Handle; 
		Handle->GetProjectileList(1001, vecProjectile);
		int nPorjectileSize = vecProjectile.size();
		D3DXVECTOR3 vTarget2(0,0,0);
		CFcProjectile* tmpProjectile = NULL;
		float min = p_fDist;
		if( nPorjectileSize > 0 )
		{
			for( int j = 0 ; j < nPorjectileSize ; j++ )
			{
				D3DXVECTOR3 ProjectilePos = vecProjectile[j]->GetPos();
				D3DXVECTOR3 vDist = ProjectilePos - m_Cross.m_PosVector;
				vDist.y = 0;
				D3DXVec3Normalize(&vDist, &vDist);
				float fDot = D3DXVec3Dot( &vHeroDir, &vDist);
				if( fDot <= 1.0f && fDot >= fDotValue )
				{
					float fDist = D3DXVec3Length( &(m_Cross.m_PosVector - ProjectilePos) );
					if( fDist <= min ) {
						min = fDist;
						vTarget2 = ProjectilePos;
						tmpProjectile = vecProjectile[j];
					}
				}
				else
				{
					continue;
				}

			}
		}

		// target3는 작은 부셔질 오브젝트 검색.
		D3DXVECTOR3 vTarget3(0,0,0);
		std::vector<CFcProp*> vecProp;
		g_FcWorld.GetActiveProps( m_Cross.m_PosVector, p_fDist, vecProp);
		int nPropSize = vecProp.size();
		float min2 = p_fDist;
		if( nPropSize > 0 )
		{
			for( int k = 0 ; k < nPropSize ; k++ )
			{
				D3DXVECTOR3 PropPos = vecProp[k]->GetPos();
				D3DXVECTOR3 vDist = PropPos - m_Cross.m_PosVector;
				vDist.y = 0.0f;
				D3DXVec3Normalize(&vDist, &vDist);
				float fDot = D3DXVec3Dot( &vHeroDir, &vDist);
				if( fDot <= 1.0f && fDot >= fDotValue )
				{
					float fDist = D3DXVec3Length( &(m_Cross.m_PosVector - PropPos) );
					if( fDist <= min2 ) {
						min = fDist;
						vTarget3 = PropPos;
					}
				}
				else
				{
					continue;
				}

			}
		}
		//////////////////////////////////////// 3가지에대한 모든 가장가까운 위치를 구했다.

		float fDist1 = D3DXVec3Length( &(m_Cross.m_PosVector - vTarget1) ); // 적과 나와의 거리
		float fDist2 = D3DXVec3Length( &(m_Cross.m_PosVector - vTarget2) ); // 물덩어리와 나와의 거리
		float fDist3 = D3DXVec3Length( &(m_Cross.m_PosVector - vTarget3) ); // prop랑 나와의 거리
		if( fDist1 < fDist2 )
		{
			if( fDist1 < fDist3 )
			{
				// fDist1이 젤루 작다.
				vTarget = vTarget1;
			}
			else
			{
				// fDist3이 젤루 작다.
				vTarget = vTarget3;
			}
		}
		else
		{
			if( fDist2 < fDist3 )
			{
				// fDist2이 젤루 작다.
				vTarget = vTarget2;
				if( tmpProjectile )
				{
					((CFcProjectileWaterBomb*)tmpProjectile)->SetDestoryTick(7);
				}
			}
			else
			{
				// fDist3이 젤루 작다.
				vTarget = vTarget3;
			}
		}

		if( D3DXVec3Length( &(vPreTarget - vTarget) ) > 2.0f )
		{
			vPreTarget = vTarget;
		}
		else
		{
			vTarget = g_FcWorld.GetEnemyObjectInRangeDirNear( m_pParent->GetTeam(), &m_Cross.m_PosVector, &vHeroDir, fDotValue, p_fDist,i, g_fMinimumDist);
		}

		D3DXVECTOR3 Pos;
		if( !p_vPos ){
			D3DXVECTOR3 Max = m_Cross.m_PosVector + vHeroDir * 80.0f - vRight * 6.0f;
			D3DXVECTOR3 Min = m_Cross.m_PosVector + vHeroDir * 75.0f + vRight * 23.0f;
			float fMaxHeight = m_Cross.m_PosVector.y + 140.0f;
			float fMinHeight = m_Cross.m_PosVector.y + 100.0f;
			Pos = D3DXVECTOR3( RandomNumberInRange(Min.x, Max.x), RandomNumberInRange(fMinHeight, fMaxHeight), RandomNumberInRange(Min.z, Max.z) );
		}
		else {
			Pos = *p_vPos;
		}

        
		bool bGround = false;

		if( vTarget.x == 0 && vTarget.y == 0 && vTarget.z == 0 )
		{
			vTarget = m_Cross.m_PosVector + vHeroDir * p_fDist2;
			vTarget.y = g_FcWorld.GetLandHeight2(vTarget.x, vTarget.z, m_Cross.m_PosVector.y);
			bGround = true;
		}
		else
		{
			vTarget.y += 80.0f;
		}

		int nValue[4];
		nValue[0] = i * 3;
		if( bGround ) nValue[1] = 1;
		else nValue[1] = -1;
		nValue[2] = m_nLevel;
		nValue[3] = -1;
		g_pFcFXManager->SendMessage(m_nNoramlAttackFx,FX_UPDATE_OBJECT, (DWORD)&Pos, (DWORD)&vTarget, (DWORD)nValue );
	}
}

void CTyurru::ShotWaterDrop(float p_fAngle, D3DXVECTOR3 p_vStartPos)
{
	D3DXVECTOR3 vTarGetDir;
	D3DXVECTOR3 vDir = m_Cross.m_ZVector;
	vDir.y = 0;
	D3DXVec3Normalize(&vDir, &vDir);
	D3DXVECTOR3 vAxis;

	Vec3RotationAxis(&vTarGetDir, vDir, p_fAngle ,D3DXVECTOR3(0,1,0));
	vTarGetDir = vTarGetDir*g_fvalue1[GetLevel()] + m_Cross.m_PosVector;
	vTarGetDir.y = g_FcWorld.GetLandHeight2(vTarGetDir.x, vTarGetDir.z, m_Cross.m_PosVector.y) - 50.0f;
	vTarGetDir = vTarGetDir - m_Cross.m_PosVector;
	D3DXVec3Normalize(&vTarGetDir, &vTarGetDir);
	vTarGetDir *= 35.0f;
	if( m_nFxIndex != -1 ) g_pFcFXManager->SendMessage(m_nFxIndex,FX_UPDATE_OBJECT,  (DWORD)&p_vStartPos, (DWORD)&vTarGetDir,(DWORD)&m_Cross.m_PosVector );
}

/*---------------------------------------------------------------------------------
-
-			CTyurru::SignalCheck()
-					; 시그널 체크 처리.
-					  
-
---------------------------------------------------------------------------------*/
void CTyurru::SignalCheck( ASSignalData *pSignal )
{
	switch( pSignal->m_nID )
	{
	case SIGNAL_TYPE_CUSTOM_MESSAGE:
		{
			switch( pSignal->m_pParam[0] )
			{
			case 0:
				{
					if( ( pSignal->m_nFrame >= m_fFrame ) && ( pSignal->m_nFrame < m_fFrame + GetFrameAdd() ) )
					{
						if( GetOrbAttack() )
						{
							g_pSoundManager->Play3DSound(NULL,SB_COMMON,"AT_D_TYUR_ORB_S_01_SHOT",&m_Cross.m_PosVector);
							
							D3DXVECTOR3 vRight;
							D3DXVec3Cross(&vRight, &m_Cross.m_ZVector, &D3DXVECTOR3(0,1,0));
							D3DXVECTOR3 Pos;
							D3DXVECTOR3 Max = m_Cross.m_PosVector + m_Cross.m_ZVector * 80.0f - vRight * 6.0f;
							D3DXVECTOR3 Min = m_Cross.m_PosVector + m_Cross.m_ZVector * 75.0f + vRight * 23.0f;
							float fMaxHeight = m_Cross.m_PosVector.y + 140.0f;
							float fMinHeight = m_Cross.m_PosVector.y + 100.0f;
							Pos = D3DXVECTOR3( RandomNumberInRange(Min.x, Max.x), RandomNumberInRange(fMinHeight, fMaxHeight), RandomNumberInRange(Min.z, Max.z) );
							
							float fAngle = RandomNumberInRange(-1,1)>0 ?  RandomNumberInRange(0.0f, 0.0174f*g_fOrbShotAngle[GetLevel()] ) : RandomNumberInRange(6.28f - 0.0174f*g_fOrbShotAngle[GetLevel()], 6.28f);
							ShotWaterDrop(fAngle, Pos);
						}
						else
						{
							g_pSoundManager->Play3DSound(NULL,SB_COMMON,"AT_D_SWING_S",&m_Cross.m_PosVector);
							ShotWater(g_nvalue2[m_nLevel],g_fvalue1[m_nLevel]*2.0f, 500.0f);
						}
					}
	
				}
				break;
			case 1:
				{	// 물 덩어리 
					if( ( pSignal->m_nFrame >= m_fFrame ) && ( pSignal->m_nFrame < m_fFrame + GetFrameAdd() ) )
					{
						g_pSoundManager->Play3DSound(NULL,SB_COMMON,"AT_D_SWING_M",&m_Cross.m_PosVector);
						PROJECTILE_PARAM Param;

						CCrossVector Cross;

						Cross = *m_Handle->GetCrossVector();

						Cross.m_PosVector.y += 150.f;
						Cross.m_PosVector += m_Handle->GetCrossVector()->m_ZVector * 50.0f;

						Param.hParent = m_Handle;

						m_ProjectileHandle = CFcProjectile::CreateObject( CFcProjectile::WATERBOMB, &Cross );
						m_ProjectileHandle->SetRtti(1000);

						m_ProjectileHandle->Initialize( m_nWaterSphereSkin, &Param );
						struct sWaterBomb
						{
							int _nDestroyTick;
							int _nTotalTick;
							int _nClassify;
							float _fAddScale;
						};
						sWaterBomb* tmp = new sWaterBomb;
						tmp->_nDestroyTick = 600;
						tmp->_nTotalTick = 20;
						tmp->_fAddScale = 10;
						tmp->_nClassify = 1;
						m_ProjectileHandle->SetCustomValue((void*)tmp);
						delete tmp;
					}
				}
				break;
			case 2:
			case 3:
			case 4:
				{	
					if( ( pSignal->m_nFrame >= m_fFrame ) && ( pSignal->m_nFrame < m_fFrame + GetFrameAdd() ) )
					{
						g_pSoundManager->Play3DSound(NULL,SB_COMMON,"AT_D_SWING_04",&m_Cross.m_PosVector);
						GameObjHandle hParent = m_Handle;
						g_FcWorld.GetAbilityMng()->Start(FC_ABILITY_WATER, hParent, true );
					}
					
				}
				break;
			case 5:
				{	//작은거 1개 중앙
					if( ( pSignal->m_nFrame >= m_fFrame ) && ( pSignal->m_nFrame < m_fFrame + GetFrameAdd() ) )
					{
						g_pSoundManager->Play3DSound(NULL,SB_COMMON,"AT_D_SWING_02",&m_Cross.m_PosVector);
						D3DXVECTOR3 vTarget = m_Cross.m_PosVector + m_Cross.m_ZVector * g_fDistColumnWater;
						CreateColumnWater(SMALL_COLUMNWATER, vTarget);
					}
					
				}
				break;
			case 6:
				{	//작은거 3개 군대군대
					if( ( pSignal->m_nFrame >= m_fFrame ) && ( pSignal->m_nFrame < m_fFrame + GetFrameAdd() ) )
					{
						g_pSoundManager->Play3DSound(NULL,SB_COMMON,"AT_D_SWING_02",&m_Cross.m_PosVector);
						D3DXVECTOR3 vTarget;
						D3DXVECTOR3 vMin, vMax;
						D3DXVECTOR3 vRight;
						D3DXVec3Cross(&vRight, &m_Cross.m_ZVector, &D3DXVECTOR3(0,1,0));
						D3DXVec3Normalize(&vRight,&vRight);
						vMax = m_Cross.m_PosVector + vRight * 400.0f + m_Cross.m_ZVector*(g_fDistColumnWater + 100.0f);
						vMin = m_Cross.m_PosVector - vRight * 400.0f + m_Cross.m_ZVector*(g_fDistColumnWater - 100.0f);
						for( int i = 0 ; i < 3 ; i++ )
						{
							int n = i%2;
							if( n == 0 )
							{
								vMax = m_Cross.m_PosVector + vRight * 500.0f + m_Cross.m_ZVector*(g_fDistColumnWater + 100.0f);
								vMin = m_Cross.m_PosVector + m_Cross.m_ZVector*(g_fDistColumnWater - 100.0f);
							}
							else
							{
								vMax = m_Cross.m_PosVector + m_Cross.m_ZVector*(g_fDistColumnWater + 100.0f);
								vMin = m_Cross.m_PosVector - vRight * 500.0f + m_Cross.m_ZVector*(g_fDistColumnWater - 100.0f);
							}

							float x = RandomNumberInRange( vMin.x, vMax.x);
							float z = RandomNumberInRange( vMin.z, vMax.z);
							vTarget = D3DXVECTOR3( x, 0, z );
							CreateColumnWater(SMALL_COLUMNWATER, vTarget, i*10 );
						}

					}
				}
				break;
			case 7:
				{	//큰거 한개 중앙
					if( ( pSignal->m_nFrame >= m_fFrame ) && ( pSignal->m_nFrame < m_fFrame + GetFrameAdd() ) )
					{
						//오브스파크
						g_pSoundManager->Play3DSound(NULL,SB_COMMON,"AT_D_TYUR_ORB_S",&m_Cross.m_PosVector);
						D3DXVECTOR3 vTarget = m_Cross.m_PosVector + m_Cross.m_ZVector * g_fDistColumnWater;
						CreateColumnWater(BIG_COLUMNWATER, vTarget);
					}
				}
				break;

			case 8:
				{
					if( ( pSignal->m_nFrame >= m_fFrame ) && ( pSignal->m_nFrame < m_fFrame + GetFrameAdd() ) )
					{
						g_pSoundManager->Play3DSound(NULL,SB_COMMON,"AT_D_SWING_M",&m_Cross.m_PosVector);
						// 물덩어리 날리기다.
						D3DXVECTOR3 vPos = m_Cross.m_PosVector;
						vPos.y += 240.0f;
						vPos -= m_Cross.m_ZVector * 30.0f;
						D3DXVECTOR3 vTPos = m_Cross.m_PosVector + m_Cross.m_ZVector*500.0f;
						m_sShotWaterDrop._nTick = g_nShotLumpOfWaterTick;
						m_sShotWaterDrop._bShotWaterDrop = true;
						m_sShotWaterDrop._bNormal = true;
						g_pFcFXManager->SendMessage(m_nShotWaterDrop, FX_UPDATE_OBJECT, (DWORD)&vPos, (DWORD)&vTPos, (DWORD)m_sShotWaterDrop._nTick );
					}
				}
				break;
			case 9:
				{
					if( ( pSignal->m_nFrame >= m_fFrame ) && ( pSignal->m_nFrame < m_fFrame + GetFrameAdd() ) )
					{
						g_pSoundManager->Play3DSound(NULL,SB_COMMON,"AT_D_SWING_M",&m_Cross.m_PosVector);
						// 물덩어리 날리기 홀딩.
						if( m_sShotWaterDrop._bNormal && m_sShotWaterDrop._bShotWaterDrop )
						{
							m_sShotWaterDrop._bShotWaterDrop = true;
							m_sShotWaterDrop._bNormal = false;
							m_sShotWaterDrop._nTick = 85;
							g_pFcFXManager->SendMessage(m_nShotWaterDrop, FX_ADD_VALUE, 0,0,80);
						}
						else if( !m_sShotWaterDrop._bShotWaterDrop )
						{
							D3DXVECTOR3 vPos = m_Cross.m_PosVector;
							vPos.y += 280.0f;
							vPos -= m_Cross.m_ZVector * 80.0f;
							D3DXVECTOR3 vTPos = m_Cross.m_PosVector + m_Cross.m_ZVector*g_fShotWaterDistance[GetLevel()];
							m_sShotWaterDrop._nTick = 85;
							m_sShotWaterDrop._bShotWaterDrop = true;
							m_sShotWaterDrop._bNormal = false;
							g_pFcFXManager->SendMessage(m_nShotWaterDrop, FX_UPDATE_OBJECT, (DWORD)&vPos, (DWORD)&vTPos, (DWORD)m_sShotWaterDrop._nTick );
						}
						
					}
				}
				break;
			case 10:
				{
					if( ( pSignal->m_nFrame >= m_fFrame ) && ( pSignal->m_nFrame < m_fFrame + GetFrameAdd() ) )
					{
						if( !m_sShotWaterDrop._bShotWaterDrop ) break;
						g_pSoundManager->Play3DSound(NULL,SB_COMMON,"AT_D_SWING_M",&m_Cross.m_PosVector);
						// 물덩어리 홀딩중 날리기.
						m_sShotWaterDrop._bNormal = false;
						m_sShotWaterDrop._bShotWaterDrop = false;
						D3DXVECTOR3 vTPos = m_Cross.m_PosVector + m_Cross.m_ZVector*g_fShotWaterDistance[GetLevel()];
						g_pFcFXManager->SendMessage(m_nShotWaterDrop, FX_ADD_VALUE2, (DWORD)&vTPos);
					}
				}
				break;
			case 11:
				{
					if( ( pSignal->m_nFrame >= m_fFrame ) && ( pSignal->m_nFrame < m_fFrame + GetFrameAdd() ) )
					{
						g_pSoundManager->Play3DSound(NULL,SB_COMMON,"AT_D_SWING_M",&m_Cross.m_PosVector);
						// 물덩어리 생성 의 홀딩.
						PROJECTILE_PARAM Param;

						CCrossVector Cross;

						Cross = *m_Handle->GetCrossVector();

						Cross.m_PosVector.y += 150.f;
						Cross.m_PosVector += m_Handle->GetCrossVector()->m_ZVector * g_fDist[m_nLevel];;

						Param.hParent = m_Handle;

						m_ProjectileHandle = CFcProjectile::CreateObject( CFcProjectile::WATERBOMB, &Cross );
						m_ProjectileHandle->SetRtti(1001);

						m_ProjectileHandle->Initialize( m_nWaterSphereSkin, &Param );
						struct sWaterBomb
						{
							int _nDestroyTick;
							int _nTotalTick;
							int _nClassify;
							float _fAddScale;
						};
						sWaterBomb* tmp = new sWaterBomb;
						tmp->_nDestroyTick = 600;
						tmp->_nTotalTick = 60;
						tmp->_fAddScale = 5;
						tmp->_nClassify = 3;
						m_ProjectileHandle->SetCustomValue((void*)tmp);
						delete tmp;
					}
				}
				break;
			case 12:
				{
					if( ( pSignal->m_nFrame >= m_fFrame ) && ( pSignal->m_nFrame < m_fFrame + GetFrameAdd() ) )
					{
						//g_pSoundManager->Play3DSound(NULL,SB_COMMON,"AT_D_SWING_M",&m_Cross.m_PosVector);
						// 물덩어리 생성 홀딩중 취소.
						struct sWaterBomb
						{
							int _nDestroyTick;
							int _nTotalTick;
							int _nClassify;
							float _fAddScale;
						};
						sWaterBomb* tmp = new sWaterBomb;
						tmp->_nDestroyTick = -1;
						tmp->_nTotalTick = 5;
						tmp->_fAddScale = -1;
						tmp->_nClassify = 3;
						m_ProjectileHandle->SetCustomValue((void*)tmp);
						delete tmp;
					}
				}
				break;
			case 13:
				{	//큰거 1개 작은거 2개
					if( ( pSignal->m_nFrame >= m_fFrame ) && ( pSignal->m_nFrame < m_fFrame + GetFrameAdd() ) )
					{
						g_pSoundManager->Play3DSound(NULL,SB_COMMON,"AT_D_SWING_02",&m_Cross.m_PosVector);
						D3DXVECTOR3 vTarget;
						D3DXVECTOR3 vMin, vMax;
						D3DXVECTOR3 vRight;
						D3DXVec3Cross(&vRight, &m_Cross.m_ZVector, &D3DXVECTOR3(0,1,0));
						D3DXVec3Normalize(&vRight,&vRight);
						vMax = m_Cross.m_PosVector + vRight * 400.0f + m_Cross.m_ZVector*(g_fDistColumnWater + 100.0f);
						vMin = m_Cross.m_PosVector - vRight * 400.0f + m_Cross.m_ZVector*(g_fDistColumnWater - 100.0f);						
						for( int i = 0 ; i < 3 ; i++ )
						{
							int n = i%2;
							if( n == 0 )
							{
								vMax = m_Cross.m_PosVector + vRight * 500.0f + m_Cross.m_ZVector*(g_fDistColumnWater + 100.0f);
								vMin = m_Cross.m_PosVector + m_Cross.m_ZVector*(g_fDistColumnWater - 100.0f);
							}
							else
							{
								vMax = m_Cross.m_PosVector + m_Cross.m_ZVector*(g_fDistColumnWater + 100.0f);
								vMin = m_Cross.m_PosVector - vRight * 500.0f + m_Cross.m_ZVector*(g_fDistColumnWater - 100.0f);
							}

							float x = RandomNumberInRange( vMin.x, vMax.x);
							float z = RandomNumberInRange( vMin.z, vMax.z);
							vTarget = D3DXVECTOR3( x, 0, z );
							if( i<2)
							{
                                CreateColumnWater(SMALL_COLUMNWATER, vTarget, i*10 );
							}
							else
							{
								vTarget = m_Cross.m_PosVector + m_Cross.m_ZVector * g_fDistColumnWater;
								CreateColumnWater(BIG_COLUMNWATER, vTarget, i*10 );
							}
						}		

					}
				}
				break;
			case 14:
				{	//큰거 3개.
					if( ( pSignal->m_nFrame >= m_fFrame ) && ( pSignal->m_nFrame < m_fFrame + GetFrameAdd() ) )
					{
						g_pSoundManager->Play3DSound(NULL,SB_COMMON,"AT_D_SWING_02",&m_Cross.m_PosVector);
						D3DXVECTOR3 vTarget;
						D3DXVECTOR3 vMin, vMax;
						D3DXVECTOR3 vRight;
						D3DXVec3Cross(&vRight, &m_Cross.m_ZVector, &D3DXVECTOR3(0,1,0));
						D3DXVec3Normalize(&vRight,&vRight);
						vMax = m_Cross.m_PosVector + vRight * 400.0f + m_Cross.m_ZVector*(g_fDistColumnWater + 100.0f);
						vMin = m_Cross.m_PosVector - vRight * 400.0f + m_Cross.m_ZVector*(g_fDistColumnWater - 100.0f);
						for( int i = 0 ; i < 3 ; i++ )
						{
							int n = i%2;
							if( n == 0 )
							{
								vMax = m_Cross.m_PosVector + vRight * 500.0f + m_Cross.m_ZVector*(g_fDistColumnWater + 100.0f);
								vMin = m_Cross.m_PosVector + m_Cross.m_ZVector*(g_fDistColumnWater - 100.0f);
							}
							else
							{
								vMax = m_Cross.m_PosVector + m_Cross.m_ZVector*(g_fDistColumnWater + 100.0f);
								vMin = m_Cross.m_PosVector - vRight * 500.0f + m_Cross.m_ZVector*(g_fDistColumnWater - 100.0f);
							}

							float x = RandomNumberInRange( vMin.x, vMax.x);
							float z = RandomNumberInRange( vMin.z, vMax.z);
							vTarget = D3DXVECTOR3( x, 0, z );
							CreateColumnWater(BIG_COLUMNWATER, vTarget, i*10 );
						}

					}
				}
				break;
			case 15:
				{	//오브어택 밀기공격.
					if( ( pSignal->m_nFrame >= m_fFrame ) && ( pSignal->m_nFrame < m_fFrame + GetFrameAdd() ) )
					{
						g_pSoundManager->Play3DSound(NULL,SB_COMMON,"AT_D_TYUR_ORB_S_02",&m_Cross.m_PosVector);
						g_pSoundManager->Play3DSound(NULL,SB_COMMON,"AT_D_SWING_03",&m_Cross.m_PosVector);							
						
						PROJECTILE_PARAM Param;

						CCrossVector Cross;

						Cross = *m_Handle->GetCrossVector();

						Cross.m_PosVector.y += 80.f;
						Cross.m_PosVector += m_Handle->GetCrossVector()->m_ZVector * 90.0f;

						Param.hParent = m_Handle;

						m_ProjectileHandle = CFcProjectile::CreateObject( CFcProjectile::WATERBOMB, &Cross );
						m_ProjectileHandle->SetRtti(1001);

						m_ProjectileHandle->Initialize( m_nWaterSphereSkin, &Param );
						struct sWaterBomb
						{
							int _nDestroyTick;
							int _nTotalTick;
							int _nClassify;
							float _fAddScale;
						};
						sWaterBomb* tmp = new sWaterBomb;
						tmp->_nDestroyTick = 150;
						tmp->_nTotalTick = 2;
						tmp->_fAddScale = 125.0f;
						tmp->_nClassify = 4;
						m_ProjectileHandle->SetCustomValue((void*)tmp);
						delete tmp;

					}
				}
				break;
			}
		}
		break;
	default:
		CFcHeroObject::SignalCheck( pSignal );
		break;
	}
}


/*---------------------------------------------------------------------------------
-
-			CTyurru::OrbAttackFxOn()
-					; .
-					  
-
---------------------------------------------------------------------------------*/
bool CTyurru::OrbAttackFxOn()
{
	SetOrbAttack(true );
	return true;
}




/*---------------------------------------------------------------------------------
-
-			CTyurru::OrbAttackFxOn()
-					; .
-					  
-
---------------------------------------------------------------------------------*/
void CTyurru::InitOrbAttack2DFx()
{
	m_nMotionBlurIndex = g_pFcFXManager->Create( FX_TYPE_INPHYMOTIONBLUR );
	g_pFcFXManager->SendMessage( m_nMotionBlurIndex, FX_INIT_OBJECT,1 );
}





/*---------------------------------------------------------------------------------
-
-			CTyurru::OrbAttackFxOff()
-					; .
-					  
-
---------------------------------------------------------------------------------*/
bool CTyurru::OrbAttackFxOff()
{
	SetOrbAttack(false );
		
	return true;
}




/*---------------------------------------------------------------------------------
-
-			CTyurru::OrbAttack()
-					; 오브어택 시 y 버튼 누르면 들어온다.
-					  
-
---------------------------------------------------------------------------------*/
void CTyurru::OrbAttack()
{
	if( !GetOrbAttack() ) return;
	/*CCrossVector cross = *m_pParent->GetCrossVector();
	D3DXVECTOR3 vPos = cross.m_PosVector;
	vPos.y += 300.0f;
	
	for( int i = 0 ; i < 3 ; i++ )
	{
		D3DXVECTOR3 vDir1 = cross.m_ZVector;
		Vec3RotationAxis(&vDir1, cross.m_ZVector, RandomNumberInRange(0.0f, 6.28f) ,D3DXVECTOR3(0,1,0));
		D3DXVECTOR3 vDir = vDir1 * 30.0f;
		vDir.y -= RandomNumberInRange(5.0f, 20.0f);
		if( m_nFxIndex != -1 ) g_pFcFXManager->SendMessage(m_nFxIndex,FX_UPDATE_OBJECT,  (DWORD)&vPos, (DWORD)&vDir, (DWORD)i );
	}*/
}





/*---------------------------------------------------------------------------------
-
-			CTyurru::NormalAttack()
-					; x 버튼 누르면 들어온다..
-					  
-
---------------------------------------------------------------------------------*/
void CTyurru::NormalAttack()
{
	if( GetAniIndex() == MIN_FLYING_TYURRU_ANI && GetLevel() >= MIN_SHOT_ATTACK_LEVEL ) {
		D3DXVECTOR3 vPos = m_Cross.m_PosVector + m_Cross.m_ZVector * 200.0f;
		vPos.y += 80.0f;
		g_pSoundManager->Play3DSound(NULL,SB_COMMON,"AT_D_SWING_S",&m_Cross.m_PosVector);
		ShotWater(g_nvalue2[m_nLevel],1000.0f, 1000.0f, &vPos);
	}

	if( GetAniIndex() == 85 && GetLevel() >= 2 ) {
		ChangeAnimationByIndex(80);
		ChangeSecondAniByIndex( -1 );

	}

	if( GetAniIndex() == 86 && GetLevel() >= 4 ) {
		ChangeAnimationByIndex(89);
		ChangeSecondAniByIndex( -1 );

	}
	
}




struct sInfo
{
	bool _bSmall;
	int _nLevel;
	int _nDelayTick;
};

/*---------------------------------------------------------------------------------
-
-			CTyurru::CreateColumnWater()
-					; .
-					  
-
---------------------------------------------------------------------------------*/
void CTyurru::CreateColumnWater( WaterType  p_nWaterType, D3DXVECTOR3 p_vPos, int p_nDelayTick )
{
	sInfo tmp;
	if( p_nWaterType == SMALL_COLUMNWATER )
	{
		g_pSoundManager->Play3DSound(NULL,SB_COMMON,"AT_D_SWING_02",&m_Cross.m_PosVector);
		p_vPos.y = g_FcWorld.GetLandHeight2(  p_vPos.x,  p_vPos.z, m_Cross.m_PosVector.y );
		p_vPos.y -= 30.0f;
		g_pFcFXManager->SendMessage(m_nColumnWater,FX_PLAY_OBJECT);
		tmp._bSmall = true;
		tmp._nDelayTick = p_nDelayTick;
		tmp._nLevel = GetLevel();
		g_pFcFXManager->SendMessage(m_nColumnWater,FX_UPDATE_OBJECT, (DWORD)&p_vPos /*Pos*/, (DWORD)m_nColumnWaterID[SMALL_COLUMNWATER], (DWORD)&tmp );
	
	}
	else
	{
		g_pSoundManager->Play3DSound(NULL,SB_COMMON,"AT_D_SWING_02",&m_Cross.m_PosVector);
		p_vPos.y = g_FcWorld.GetLandHeight2(  p_vPos.x,  p_vPos.z, m_Cross.m_PosVector.y );
		p_vPos.y -= 50.0f;
		tmp._bSmall = false;
		tmp._nDelayTick = p_nDelayTick;
		tmp._nLevel = GetLevel();
		g_pFcFXManager->SendMessage(m_nColumnWater,FX_PLAY_OBJECT);
		g_pFcFXManager->SendMessage(m_nColumnWater,FX_UPDATE_OBJECT, (DWORD)&p_vPos /*Pos*/, (DWORD)m_nColumnWaterID[BIG_COLUMNWATER], (DWORD)&tmp );
	}

}


void CTyurru::PhysicsCheck( ASSignalData *pSignal )
{
	CFcHeroObject::PhysicsCheck(pSignal);

	bool bPush = false;

	if( GetAniIndex() == 80 || GetAniIndex() == 89 || GetAniIndex() == 22 ) {
		pSignal->m_PhysicsPower = 170;
		bPush = true;
	}
	else
	{
		switch(m_nLoopInputCount)
		{
		case 0:
			pSignal->m_PhysicsPower = 35;
			break;
		case 1:
			pSignal->m_PhysicsPower = 40;
			break;
		case 2:
			pSignal->m_PhysicsPower = 45;
			break;
		case 3:
			pSignal->m_PhysicsPower = 85;
			break;
		case 4:
			pSignal->m_PhysicsPower = 80;
			break;
		case 5:
			pSignal->m_PhysicsPower = 85;
			break;
		case 6:
			pSignal->m_PhysicsPower = 85;
			break;
		}
	}

	
	if( ( pSignal->m_nFrame >= m_fFrame ) && ( pSignal->m_nFrame < m_fFrame + GetFrameAdd() ) )
		CFcProjectile::PhysicsCheck(pSignal, m_Handle, bPush);
	return;
}

void CTyurru::ChangeSecondAniByIndex( int nAniIndex )
{
	if( nAniIndex == -1 )
	{
		m_nSecondAni = -1;
	}
	else
	{
		if( nAniIndex == m_nSecondAni )
		{
			return;
		}
		m_nSecondAni = nAniIndex;
		m_fSecondFrame = 0.0f;
	}
}

int CTyurru::CalculateWalkAni( KEY_EVENT_PARAM *pKeyParam )
{
	float fDot;
	D3DXVECTOR3 KeyVector;

	KeyVector.x = ( float )pKeyParam->nPosX;
	KeyVector.y = 0.0f;
	KeyVector.z = ( float )pKeyParam->nPosY;
	D3DXVec3Normalize( &KeyVector, &KeyVector );

	if( D3DXVec3LengthSq( &KeyVector ) <= 0.0f )
	{
		return 2;
	}

	if( s_CameraHandle[ m_nPlayerIndex ] )
	{
		D3DXVec3TransformNormal( &KeyVector, &KeyVector, *s_CameraHandle[ m_nPlayerIndex ]->GetCrossVector() );
		KeyVector.y = 0.0f;
		D3DXVec3Normalize( &KeyVector, &KeyVector );
		m_MoveDir = KeyVector;
		fDot = D3DXVec3Dot( &KeyVector, &m_Cross.m_ZVector );
		if( fDot > 0.707106f )	// 정면 90도
		{
			return 2;
		}
		else if( fDot < -0.707106f )	// 후면 90도
		{
			return 5;
		}
		fDot = D3DXVec3Dot( &KeyVector, &m_Cross.m_XVector );
		if( fDot > 0.707106f )	// 오른쪽 90도
		{
			return 4;
		}
		else	// 왼쪽 90도
		{
			return 3;
		}
	}
	else
	{
		return 2;
	}

	return 0;
}

void CTyurru::ProcessKeyEvent( int nKeyCode, KEY_EVENT_PARAM *pKeyParam )
{
	if( nKeyCode == PAD_INPUT_EVENT_RESTORE_KEY ) {
		ResetKeyProcess();
		if( IsMove() ) CmdStop();
		return;
	}
	if( g_InterfaceManager.IsShowLetterBox() )
	{
		return;
	}
	if( g_pFcRealMovie && g_pFcRealMovie->IsPlay() )
	{
		return;
	}


	int nKeyCodeSave;
	bool bRelease = false;

	nKeyCodeSave = nKeyCode;
	if( nKeyCode & 0x80000000 )
	{
		bRelease = true;
	}
	nKeyCode &= 0x7fffffff;

	if( nKeyCode == PAD_INPUT_LSTICK )
	{
		if( ( m_nAniIndex == 66 ) || ( m_nAniIndex == 67 ) || ( m_nAniIndex == 77 ) )
		{
			int nFindAni;

			if( bRelease )
			{
				ChangeSecondAniByIndex( -1 );
			}
			else 
			{
				int nWalk;

				if( CInputPad::GetInstance().GetKeyPressTick( g_FCGameData.nEnablePadID, PAD_INPUT_LTRIGGER ) )
				{
					nWalk = CalculateWalkAni( pKeyParam );
				}
				else
				{
					CalculateMoveDir( pKeyParam, m_nPlayerIndex );
					nWalk = 2;
				}

				nFindAni = FindExactAni( m_pUnitInfoData->GetAniIndex( ANI_TYPE_WALK, nWalk ) );
				if( nFindAni != -1 ) ChangeSecondAniByIndex( nFindAni );

				m_fMoveSpeed = ( ( float )m_pUnitInfoData->GetMoveSpeed( m_nSecondAni ) ); 
				MoveZ( m_fMoveSpeed );
			}
			return;
		}
		else if( m_nSecondAni != -1 )
		{
//			ChangeAnimation( ANI_TYPE_RUN, 0 );
			ChangeSecondAniByIndex( -1 );
		}
	}
	else if( nKeyCode == PAD_INPUT_Y )
	{
		if( GetAniIndex() == 17 && GetLevel() >= 5 ) {
			static float s_fPastFrame = 0.0f;
			if(s_fPastFrame > 44.0f ) s_fPastFrame = 0.0f;
			if( m_fFrame >= s_fPastFrame + 5.0f ) 
			{
				// 물폭탄 만든다.
				PROJECTILE_PARAM Param;

				ProjetileHandle Handle;

				CCrossVector Cross;

				Cross = m_Cross;

				Cross.m_PosVector += m_Cross.m_ZVector * 90.0f;

				Param.hParent = m_Handle;

				g_pSoundManager->Play3DSound(NULL,SB_COMMON,"AT_D_SWING_M",&m_Cross.m_PosVector);

				Handle = CFcProjectile::CreateObject( CFcProjectile::WATERBOMB, &Cross );

				Handle->Initialize( m_nWaterSphereSkin, &Param );
				struct sWaterBomb
				{
					int _nDestroyTick;
					int _nTotalTick;
					int _nClassify;
					float _fAddScale;
				};
				sWaterBomb* tmp = new sWaterBomb;
				tmp->_nDestroyTick = 100;
				tmp->_nTotalTick = 5;
				tmp->_fAddScale = 10.0f;
				tmp->_nClassify = 2;
				Handle->SetCustomValue((void*)tmp);
				delete tmp;

				s_fPastFrame = m_fFrame;
			}
			
		}
		else if( !m_bOrbAttack )
		{
			std::vector<CFcProjectile*> vecProjectile;
			ProjetileHandle Handle; 
			Handle->GetProjectileList2(1000, 1001, vecProjectile);
			int nSize = vecProjectile.size();
			float	fDist= -1.0f;
			float	fDot = -1.0f;
			bool	bAniChange = false;
			if( nSize > 0 )
			{
				CCrossVector ParCross = *m_pParent->GetCrossVector();

				for( int i = 0 ; i < nSize ; ++i )
				{
					CFcProjectile *tmp = vecProjectile[i];
					CCrossVector cross = *tmp->GetCrossVector();
					D3DXVECTOR3 vDist = cross.m_PosVector - ParCross.m_PosVector;
					fDist = D3DXVec3Length( &(vDist) );
					D3DXVECTOR3 vDistPerLength = vDist / fDist;
					if( fDist >= 0 && fDist <= 260.0f )
					{
						fDot = D3DXVec3Dot( &ParCross.m_ZVector, &vDistPerLength );
						if( fDot <= 1.0f && fDot >= 0.7f )
						{
							bAniChange = true;
							break;
						}
					}
				}
			}

			if( bAniChange && GetLevel() >= 3 )
			{
				// 물덩어리 치기
				ChangeAnimationByIndex(71);
				ChangeSecondAniByIndex( -1 );
			}
		}
		
	}

	CFcHeroObject::ProcessKeyEvent( nKeyCodeSave, pKeyParam );
}

int CTyurru::FindExactAni( int nAniIndex )
{
	int nAniType = m_pUnitInfoData->GetAniType( nAniIndex );
	switch( m_nAniIndex )
	{
	case 85:
		if( ( nAniType == ANI_TYPE_WALK ) || ( nAniType == ANI_TYPE_RUN ) || ( nAniType == ANI_TYPE_STAND ) )
		{
			return 85;
		}
		break;
	case 86:
		if( ( nAniType == ANI_TYPE_WALK ) || ( nAniType == ANI_TYPE_RUN ) || ( nAniType == ANI_TYPE_STAND ) )
		{
			return 86;
		}
		break;
	case 17:
		if( ( nAniType == ANI_TYPE_WALK ) || ( nAniType == ANI_TYPE_RUN ) || ( nAniType == ANI_TYPE_STAND ) )
		{
			return 17;
		}
		break;
	}
		
	return nAniIndex;
}

void CTyurru::CmdHit( HIT_PARAM *pHitParam, D3DXVECTOR3 *pHitDir /*= NULL*/ )
{
	m_nSecondAni = -1;
	CFcHeroObject::CmdHit( pHitParam );
}

void CTyurru::Enable( bool bEnable, bool bFullHP /*= false*/ )
{
	if( bEnable == false )
	{
		StopOrbSpark();
		m_Levia.EffectAndPointLightRemove();
	}

	CFcUnitObject::Enable( bEnable, bFullHP );
}







//-----------------------------------------------------------------------------

void CLeviaElement::Initialize( D3DXVECTOR3& vPos, float fScaleSpd)
{
	m_Cross.m_PosVector = vPos;
	m_Cross.m_ZVector.x = RandomNumberInRange( -1.f, 1.f);
	m_Cross.m_ZVector.y = RandomNumberInRange( -1.f, 1.f);
	m_Cross.m_ZVector.z = RandomNumberInRange( -1.f, 1.f);
	m_Cross.UpdateVectors();
	m_bIdle = false;
	m_fSize = RandomNumberInRange( 5.f, 10.f);
	m_fScaleSpd = fScaleSpd;
	m_nRoateSpd = 10;
	m_fScaleSin = 0.f;
}

void CLeviaElement::Process()
{
	if(m_bIdle)
	{
		m_fScale =0.f;
		return;
	}


	m_Cross.RotatePitch(m_nRoateSpd);
	m_fScale = sinf(m_fScaleSin);

	m_fScaleSin += m_fScaleSpd;
	if(m_fScaleSin > 6.28f)
		m_fScaleSin -= 6.28f;


	if( m_fScaleSpd != 0.f && m_fScale < 0.f)
		m_bIdle = true;		
}	

void CLeviaElement::Update(int nEngineIdx)
{
	float fScale = GetCurSize();
	static D3DXMATRIX mtx;
	D3DXMatrixScaling(&mtx, fScale, fScale, fScale);
	D3DXMatrixMultiply( &mtx, &mtx, m_Cross );
	g_BsKernel.UpdateObject(nEngineIdx, &mtx);
}



CLevia::CLevia()
{
	m_fScale = 0.f;
	m_fRotateRadius = 0.f;
	m_fSinFX = 0.f;
	m_fSinFY = 0.f;
	m_fSinFZ = 0.f;

	m_fRotX = 0.f;
	m_fRotY = 0.f;
	m_fRotZ = 0.f;

	m_fAccel = 0.f;
	m_nEngineIdx = -1;

	m_nLoadFxIdx =89;
	m_nFxIdx = -1;
	m_nPointLight = -1;
}

void CLevia::EffectAndPointLightRemove()
{
	if(m_nFxIdx != -1)
	{
		g_BsKernel.DeleteParticleObject( m_nFxIdx);
		m_nFxIdx = -1;
	}

	if( m_nPointLight != -1 ) {
		g_BsKernel.DeleteObject(m_nPointLight);
		m_nPointLight = -1;
	}
}
void CLevia::Initialize(GameObjHandle hMaster)
{
	m_hMaster = hMaster;
	m_fScale  = 25.f;
	m_fRotateRadius = 100.f;
	m_fSinFX = 0.8f*3.14f/180.f;
	m_fSinFY = 2.5f*3.14f/180.f;
	m_fSinFZ = 1.5f*3.14f/180.f;

	m_fRotX = 0.f;
	m_fRotY = 0.f;
	m_fRotZ = 0.f;
	m_nRyviusSkinIndex = -1;


	if( m_nRyviusSkinIndex == -1 )
	{
		g_BsKernel.chdir("fx");
		m_nRyviusSkinIndex = CBsKernel::GetInstance().LoadSkin( -1, "Ryvius.skin" );
		g_BsKernel.chdir("..");
	}
	

	m_nEngineIdx = CBsKernel::GetInstance().CreateStaticObjectFromSkin(m_nRyviusSkinIndex);
	for(int ii = 0; ii < _LEVIA_ELEMENTCOUNT; ++ii)
	{
		m_nElementEngineIdx[ii] = CBsKernel::GetInstance().CreateStaticObjectFromSkin(m_nRyviusSkinIndex);
	}

	D3DXVECTOR3 vMasterPos = m_hMaster->GetPos();
	vMasterPos.y += m_hMaster->GetUnitHeight()*0.5f;
	m_Cross.m_PosVector = vMasterPos;	

	if( m_nPointLight == -1 ) {
		m_nPointLight = g_BsKernel.CreateLightObject( POINT_LIGHT );
		float fRange = 300.0f;;
		D3DCOLORVALUE Color;
		Color.r = 0.5f;
		Color.g = 0.5f;
		Color.b = 0.5f;
		Color.a = 1.0f;
		g_BsKernel.SendMessage( m_nPointLight, BS_SET_POINTLIGHT_COLOR, (DWORD)&Color ); 
		g_BsKernel.SendMessage( m_nPointLight, BS_SET_POINTLIGHT_RANGE, (DWORD)&fRange );
	}
	SetLive(true);
	SetMasterLive(true);
	m_bRelease = false;
}

void CLevia::Release()
{
	SetLive(false);
	m_bRelease = true;
	if(m_nEngineIdx != -1)
	{
		CBsKernel::GetInstance().DeleteObject( m_nEngineIdx);
		m_nEngineIdx = -1;
	}

	for(int ii = 0; ii < _LEVIA_ELEMENTCOUNT; ++ii)
	{
		if(m_nElementEngineIdx[ii] == -1)
			continue;

		CBsKernel::GetInstance().DeleteObject(m_nElementEngineIdx[ii]);
		m_nElementEngineIdx[ii] = -1;
	}

	if(m_nFxIdx != -1)
	{
		g_BsKernel.DeleteParticleObject( m_nFxIdx);
		m_nFxIdx = -1;
	}
	if( m_nRyviusSkinIndex != -1 )
	{
		g_BsKernel.ReleaseSkin(m_nRyviusSkinIndex);
		m_nRyviusSkinIndex = -1;
	}
	if( m_nPointLight != -1 ) {
		g_BsKernel.DeleteObject(m_nPointLight);
		m_nPointLight = -1;
	}
}

void CLevia::Process()
{
	if( m_bRelease ) return;

	D3DXVECTOR3 vMasterPos = m_hMaster->GetPos();
	vMasterPos.y += m_hMaster->GetUnitHeight()*0.5f;
	D3DXVECTOR3 vDist = vMasterPos - m_Cross.m_PosVector;
	float fDist = D3DXVec3Length(&vDist);
	float fRate =( fDist-100.f )/500.f;
	fRate = BsMax(fRate, 0.f);
	fRate = BsMin(fRate, 1.f);

	// 기본속도 7
	if(fDist < 7.f)
	{
		m_Cross.m_PosVector = vMasterPos;
		m_fAccel = 1.f;
	}
	else
	{	


		if(fDist > 300.f)
		{
			m_fAccel *= 1.02f;
			m_fAccel = BsMin( m_fAccel, 4.f );
		}
		else// if(fDist > 100.f)
		{
			m_fAccel *= 0.98f;
			m_fAccel = BsMax( m_fAccel, 1.f );
		}
		D3DXVECTOR3 vDir;
		D3DXVec3Normalize(&vDir,  &vDist);
		m_Cross.m_PosVector +=  vDir*7.f*m_fAccel;
	}

	m_CrossR = m_Cross;
	float fRotateRadius = m_fRotateRadius*0.8f*(1.f-fRate)+0.2f;


	D3DXVECTOR3 vPosL;
	vPosL.x = fRotateRadius*sinf(m_fRotX);
	vPosL.y = fRotateRadius*sinf(m_fRotY)*0.5f+fRotateRadius*0.5f;
	vPosL.z = fRotateRadius*sinf(m_fRotZ);

	float fminz = 30.f- fabs(vPosL.x)-fDist;
	
	m_CrossR.m_PosVector += vPosL;

	float fRand = RandomNumberInRange( 0.f, 1.f);
	if( GetMasterLive() ) {
        if(fRand > 0.8f*(1.f-fRate) )
			Generate(1.f-fRate, m_CrossR.m_PosVector );
	}


	m_fRotX += m_fSinFX;
	m_fRotY += m_fSinFY;
	m_fRotZ += m_fSinFZ;

	if(m_fRotX > 6.28f)
		m_fRotX -= 6.28f;

	if(m_fRotY > 6.28f)
		m_fRotY -= 6.28f;

	if(m_fRotZ > 6.28f)
		m_fRotZ -= 6.28f;


	for(int ii = 0; ii < _LEVIA_ELEMENTCOUNT; ++ii)
	{
		m_Element[ii].Process();
	}
	


	if(m_nFxIdx == -1)
	{
		m_nFxIdx = g_BsKernel.CreateParticleObject( 89, true, false, m_CrossR,1.0f,NULL,false );
	}

	if( m_nPointLight != -1 )
        g_BsKernel.UpdateObject( m_nPointLight, m_CrossR);
	else
	{
		m_nPointLight = g_BsKernel.CreateLightObject( POINT_LIGHT );
		float fRange = 300.0f;;
		D3DCOLORVALUE Color;
		Color.r = 0.5f;
		Color.g = 0.5f;
		Color.b = 0.5f;
		Color.a = 1.0f;
		g_BsKernel.SendMessage( m_nPointLight, BS_SET_POINTLIGHT_COLOR, (DWORD)&Color ); 
		g_BsKernel.SendMessage( m_nPointLight, BS_SET_POINTLIGHT_RANGE, (DWORD)&fRange );
	}

	if( m_hMaster->GetHP() <= 0 )
	{
		SetMasterLive(false);
	}

	if( !GetMasterLive() )
	{
		if( m_fScale <= 0.2f )
			Release();
		else
            m_fScale -= 0.1f;
	}
}

void CLevia::Update()
{
	if( m_bRelease ) return;
	static D3DXMATRIX mtx;
	D3DXMatrixScaling(&mtx, m_fScale, m_fScale, m_fScale);
	D3DXMatrixMultiply( &mtx, &mtx, m_CrossR );
	g_BsKernel.UpdateObject(m_nEngineIdx, &mtx);

	CBsObject *pObj = g_BsKernel.GetEngineObjectPtr(m_nEngineIdx);
	pObj->SetEntireObjectMatrix(&mtx);

	if(m_nFxIdx != -1)
		g_BsKernel.UpdateParticle( m_nFxIdx, m_CrossR );



	for(int ii = 0; ii < _LEVIA_ELEMENTCOUNT; ++ii)
	{
		if( !m_Element[ii].IsIdle() )
		{

			m_Element[ii].Update( m_nElementEngineIdx[ii] );
			//fScale = m_Element[ii].GetCurSize();
			//D3DXMatrixScaling(&mtx, fScale, fScale, fScale);
			//D3DXMatrixMultiply( &mtx, &mtx, m_Cross );
			//g_BsKernel.UpdateObject(m_nElementEngineIdx[ii], &mtx);
		}
	}
}

void CLevia::Generate(float fLife, D3DXVECTOR3& vPos)
{
	D3DXVECTOR3 vRand;
	vRand.x = RandomNumberInRange( -m_fScale, m_fScale);
	vRand.y = RandomNumberInRange( -m_fScale, m_fScale);
	vRand.z = RandomNumberInRange( -m_fScale, m_fScale);
	vRand += vPos;
	for( int ii = 0; ii < _LEVIA_ELEMENTCOUNT ; ++ii)
	{
		if( m_Element[ii].IsIdle() )
		{
			m_Element[ii].Initialize( vRand, 3.145f/(40.f+fLife*40.f) );
			break;
		}
	}
}