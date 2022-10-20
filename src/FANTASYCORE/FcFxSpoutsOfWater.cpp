#include "StdAfx.h"
#include ".\fcfxspoutsofwater.h"
#include "FCWorld.h"
#include "FCGameObject.h"
#include "FcHeroObject.h"
#include "FCGlobal.h"
#include "FcFxManager.h"
#include "FcUtil.h"
#include "data\\FxList.h"
#include "Box3.h"
#include "Ray3.h"
#include "IntLin3Box3.h"
#include "FcSoundManager.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG

//싸그리 다 문자로 취급된다.
#define continuousFileName( name, num, ext )		#name#num"."#ext


const int g_nTick1 = 5;						// 물방울 생성되고 물줄기가 나가는 타이밍
const int g_nTick2 = 80;					// 바닦 이펙트 생성되고 끝나기까지의 타이밍
const int g_nTick3 = 90;					// 바닦 이펙트 생성되고 끝나기까지의 타이밍

// 튜르르 데미지 ( 예 > 1.0 ==> 원 데미지 그대로 적용됨, 0.5 ==> 원 데미지 에서 50% 만 적용됨 )
const float g_fDamagePercent = 0.5f;

// 일반공격 적 전방 밀기 값
const int g_nPushZ[9] = { -6, -6, -6, -6, -6, -6, -6, -6, -6 };

// 일반공격 적 상방 밀기 값
const int g_nPushY[9] = { 3, 3, 3, 3, 3, 3, 3, 3, 3 };



/*---------------------------------------------------------------------------------
-
-			CFcFxSpoutsOfWater::CFcFxSpoutsOfWater()
-					; 생성자.
-					  
-
---------------------------------------------------------------------------------*/
CFcFxSpoutsOfWater::CFcFxSpoutsOfWater()
{
	m_nFxIndex1 = -1;
	m_nFxIndex2 = -1;
	m_nFxIndex3	= -1;
	m_nLevel = 0;
	m_nSkinindex = -1;
	memset(m_nEnemySkinindex, -1, sizeof(m_nEnemySkinindex));
	m_nFxId		= -1;
#ifndef _LTCG
	SetFxRtti(FX_TYPE_SPOUTSOFWATER);
#endif //_LTCG
}






/*---------------------------------------------------------------------------------
-
-			CFcFxSpoutsOfWater::~CFcFxSpoutsOfWater()
-					; 소멸자.
-					  
-
---------------------------------------------------------------------------------*/
CFcFxSpoutsOfWater::~CFcFxSpoutsOfWater()
{
	Reset();
}






/*---------------------------------------------------------------------------------
-
-			CFcFxSpoutsOfWater::Reset()
-					; 소멸자에서 호출.
-					  
-
---------------------------------------------------------------------------------*/
void CFcFxSpoutsOfWater::Reset()
{
	DeleteAllObj();
	//if( m_nFxId != -1 ) g_pFcFXManager->SendMessage(m_nFxId, FX_DELETE_OBJECT );
	SAFE_DELETE_FX( m_nFxId,FX_TYPE_LISTGENERATER);

	if( m_nSkinindex != -1 )
			g_BsKernel.ReleaseSkin( m_nSkinindex );
	m_nSkinindex = -1;
	

	for( int i = 0 ; i < 35 ; i++ )						// 적 몸에 나타날 스킨 로딩
	{
		if( m_nEnemySkinindex[i] != -1 )
            g_BsKernel.ReleaseSkin( m_nEnemySkinindex[i] );
		m_nEnemySkinindex[i] = -1;
	}
}







/*---------------------------------------------------------------------------------
-
-			CFcFxSpoutsOfWater::Init()
-					; 초기값 및 이펙트 로딩.
-					  
-
---------------------------------------------------------------------------------*/
void CFcFxSpoutsOfWater::Init()
{
	g_BsKernel.chdir("Fx");

	m_nFxIndex1 = FX_TYURURUWATERSPOT01;				// 머리위 생성이펙트
	m_nFxIndex2 = FX_TYURURUWATERSPOT02;				// 바닦과 충돌생성 이펙트
	m_nFxIndex3 = FX_TYURURUWATERSPOT04;				// 바닦과 충돌생성 이펙트 큰거

	m_nSkinindex = g_BsKernel.LoadSkin( -1, "WSHOT_T001.SKIN" );

	for( int i = 0 ; i < 35 ; i++ )						// 적 몸에 나타날 스킨 로딩
	{
		BsAssert( m_nEnemySkinindex[i] == -1 );

		char buff[100];
		if( i+1 < 10 )
            sprintf( buff, "%s%d.skin" , "WATER_COLIDE_HU00",i+1);
		else
			sprintf( buff, "%s%d.skin" , "WATER_COLIDE_HU0",i+1);
        m_nEnemySkinindex[i] = g_BsKernel.LoadSkin( -1, buff );
	}

	g_BsKernel.chdir("..");
	m_nFxId = g_pFcFXManager->Create(FX_TYPE_LISTGENERATER);
	g_pFcFXManager->SendMessage(m_nFxId,FX_INIT_OBJECT, FX_TYRURUENEMYFX, 100, -1);
	g_pFcFXManager->SendMessage(m_nFxId,FX_PLAY_OBJECT);
}








/*---------------------------------------------------------------------------------
-
-			CFcFxSpoutsOfWater::SpoutSimulation()
-					; 물줄기 시뮬레이션 및 적의 몸 혹은 바닦에 나타날 첫번째 이펙트 로딩
-					  후에 첫번째 이펙트 로딩된것을 이용해서 계속해서 update한다.
-
---------------------------------------------------------------------------------*/
void CFcFxSpoutsOfWater::SpoutSimulation(SpoutOfWater* p_pObj)
{
	int K = 8;
	int n;
	
	if( p_pObj->_nNextIndex == -1 ) p_pObj->_nNextIndex = 0;
	else p_pObj->_nNextIndex++;

	if( p_pObj->_nCurveTrail == -1 )
	{
		p_pObj->_nCurveTrail = CBsKernel::GetInstance().CreateStaticObjectFromSkin(m_nSkinindex);
	}

	n = p_pObj->_nNextIndex;

	if( n > K ) 
	{
		if( p_pObj->_nCurveTrail != -1 ) { 
			g_BsKernel.DeleteObject(p_pObj->_nCurveTrail);
            p_pObj->_nCurveTrail = -1; 
		}
		return;
	}

	if( n >= 0 )
	{
		D3DXVECTOR3 vDir = p_pObj->_vTarget - p_pObj->_vFirstPos;
		CCrossVector cross;
		cross.m_PosVector = p_pObj->_vFirstPos + vDir * (1.0f/(float)K) * (float)(n);
		cross.m_ZVector = vDir*0.01f*(float)(K-n)* (1.0f/(float)K);
		cross.UpdateVectors();
		g_BsKernel.UpdateObject(p_pObj->_nCurveTrail, cross);
		

		if( n == 5 )
		{
			if( p_pObj->_bGround )
			{
				if( p_pObj->_nFxGround == -1 ) 
				{
					
					p_pObj->_nFxGround = g_BsKernel.CreateFXObject(m_nFxIndex2);
					_FC_RANGE_DAMAGE rd;
					rd.Attacker = m_hParent;
					rd.pPosition = &p_pObj->_vTarget;
					rd.fRadius = 100.0f;
					rd.nPower = (int)(m_hParent->GetAttackPower() * g_fDamagePercent );
					rd.nPushY = g_nPushY[m_hParent->GetLevel()]; 
					rd.nPushZ = g_nPushZ[m_hParent->GetLevel()];
					rd.nAdjutantHitRemainRatio = 4;
					rd.nTeam = m_hParent->GetTeam();
					rd.WeaponType = WT_TYURRU; 
					rd.SoundAttackType = ATT_LITTLE;
					g_FcWorld.GiveRangeDamage( rd, false,false );
					g_pSoundManager->Play3DSound(NULL,SB_COMMON,"HIT_D_SWING_S",&cross.m_PosVector);
						
					g_BsKernel.SetFXObjectState( p_pObj->_nFxGround, CBsFXObject::PLAY );
				}
			}
			else
			{
				_FC_RANGE_DAMAGE rd;
				rd.Attacker = m_hParent;
				rd.pPosition = &p_pObj->_vTarget;
				rd.fRadius = 100.0f;
				rd.nPower = (int)(m_hParent->GetAttackPower()*0.5f);
				rd.nPushY = 2; 
				rd.nPushZ = 8;
				rd.nAdjutantHitRemainRatio = 4;
				rd.nTeam = m_hParent->GetTeam();
				rd.WeaponType = WT_TYURRU; 
				rd.SoundAttackType = ATT_LITTLE;
				rd.nGuardBreak = 10;
				g_FcWorld.GiveRangeDamage( rd, false,false );
				p_pObj->_nFxEnemy[0] = CBsKernel::GetInstance().CreateStaticObjectFromSkin(m_nEnemySkinindex[0]);
				return;
			}
		}//if( n == 1 )

	}//if( p_pObj->_nCurveTrail != -1 )
}
// P:[튜르르/all] 활강도중 Y공격






/*---------------------------------------------------------------------------------
-
-			CFcFxSpoutsOfWater::EnemyEffect()
-					; 물줄기 시뮬레이션.
-					  
-
---------------------------------------------------------------------------------*/
void CFcFxSpoutsOfWater::EnemyEffect(SpoutOfWater* p_pObj)
{
	if( !p_pObj->_bDraw ) return;
	int n;

	if( true )
	{
		if( p_pObj->_nNextIndex2 == -1 ) p_pObj->_nNextIndex2 = 0;
		else p_pObj->_nNextIndex2++;

		if( p_pObj->_nNextIndex2 < 35 && p_pObj->_nFxEnemy[p_pObj->_nNextIndex2] == -1 )
		{
			if( m_nEnemySkinindex[p_pObj->_nNextIndex2] != -1 )
			{
                p_pObj->_nFxEnemy[p_pObj->_nNextIndex2] = CBsKernel::GetInstance().CreateStaticObjectFromSkin(m_nEnemySkinindex[p_pObj->_nNextIndex2]);
				g_BsKernel.SendMessage( p_pObj->_nFxEnemy[p_pObj->_nNextIndex2], BS_ENABLE_OBJECT_ALPHABLEND, 1 );
			}
		}
	}
	
	n = p_pObj->_nNextIndex2;

	if( n > 34 ) 
	{
		for( int i = 0 ; i < 35 ; i++ )
		{
			if( p_pObj->_nFxEnemy[i] != -1 ) g_BsKernel.DeleteObject(p_pObj->_nFxEnemy[i]);
			p_pObj->_nFxEnemy[i] = -1; 
		}
		return;
	}

	if( p_pObj->_nFxEnemy[n] != -1 )
	{
		CCrossVector cross;
		cross.m_PosVector = p_pObj->_vTarget;
		cross.m_ZVector = p_pObj->_vTarget - p_pObj->_vFirstPos;
		cross.m_ZVector.y = 0;
		cross.m_YVector = D3DXVECTOR3(0,1,0);
		D3DXVec3Normalize(&cross.m_ZVector, &cross.m_ZVector);
		cross.UpdateVectors();

		if( n==0 )
		{
			D3DXVECTOR3 vPos = D3DXVECTOR3(cross.m_PosVector.x, cross.m_PosVector.y - 50.0f, cross.m_PosVector.z);
			g_pFcFXManager->SendMessage(m_nFxId, FX_UPDATE_OBJECT, (DWORD)&vPos ,(DWORD)&cross.m_ZVector );
		}

		if( n >= 20 )
		{
			float fAlphaWeight = (float)(34-n) / float(14);
			g_BsKernel.SendMessage( p_pObj->_nFxEnemy[n], BS_SET_OBJECT_ALPHA, ( DWORD )&fAlphaWeight );
		}
		else
		{
			float fAlphaWeight = 1.0f;
			g_BsKernel.SendMessage( p_pObj->_nFxEnemy[n], BS_SET_OBJECT_ALPHA, ( DWORD )&fAlphaWeight );
		}

		D3DXMATRIX tmp = *(D3DXMATRIX*)cross;
		D3DXMATRIX tmp2;
		D3DXMatrixIdentity(&tmp2);
				
		
		if( p_pObj->_bBig )
		{
			tmp2._11 *= 0.8f, tmp2._22 *= 1.0f, tmp2._33 *= 0.8f;
		}
		else
		{
			if( p_pObj->_fRandom < 0 ) p_pObj->_fRandom = RandomNumberInRange(0.0f, 0.3f);
			tmp2._11 *= (0.5f - p_pObj->_fRandom), tmp2._22 *= (0.7f - p_pObj->_fRandom), tmp2._33 *= (0.5f - p_pObj->_fRandom);
		}
		D3DXMatrixMultiply(&tmp, &tmp2, &tmp);
		g_BsKernel.UpdateObject(p_pObj->_nFxEnemy[n], &tmp);
	}
}







/*---------------------------------------------------------------------------------
-
-			CFcFxSpoutsOfWater::UpdateObj()
-					; 모든 이펙트 갱신.
-					  
-
---------------------------------------------------------------------------------*/
void CFcFxSpoutsOfWater::UpdateObj()
{
	std::list<SpoutOfWater*>::iterator iter;
	if( !m_listSpoutsOfWater.empty() )
	{
		for( iter = m_listSpoutsOfWater.begin() ; iter != m_listSpoutsOfWater.end() ; )
		{
			SpoutOfWater* tmp = NULL;
			tmp = (SpoutOfWater*)*iter;
			if( tmp->_bDelete )
			{
				if( tmp->_nFxDropWater != -1 )	g_BsKernel.DeleteObject(tmp->_nFxDropWater);
				if( tmp->_nFxGround != -1 )		g_BsKernel.DeleteObject(tmp->_nFxGround);
				for( int i = 0 ; i < 35 ; i++ )
				{
					if( tmp->_nFxEnemy[i] != -1 ) g_BsKernel.DeleteObject(tmp->_nFxEnemy[i]);
					tmp->_nFxEnemy[i] = -1; 
				}
				if( tmp->_nCurveTrail != -1 ) {
					g_BsKernel.DeleteObject(tmp->_nCurveTrail);
					tmp->_nCurveTrail = -1; 
				}
				delete tmp;

				iter = m_listSpoutsOfWater.erase(iter);
			}
			else
			{
				if( tmp->_nFxDropWater == -1 )
				{
					tmp->_nFxDropWater = g_BsKernel.CreateFXObject(m_nFxIndex1);
					g_BsKernel.SetFXObjectState( tmp->_nFxDropWater, CBsFXObject::PLAY );
				}

				
				CCrossVector Ptc;
				Ptc.m_PosVector = tmp->_vFirstPos;
				Ptc.UpdateVectors();
				g_BsKernel.UpdateObject( tmp->_nFxDropWater, Ptc );

		
				if( tmp->_nTick1 >= g_nTick1 )
					SpoutSimulation(tmp);
				
				if( tmp->_nFxGround != -1 )
				{
					CCrossVector Ptc;
					Ptc.m_PosVector = tmp->_vTarget;
					D3DXVECTOR3 vDir = tmp->_vTarget - tmp->_vFirstPos;
					vDir.y = 0.0f;
					D3DXVec3Normalize(&vDir,&vDir);
					Ptc.m_ZVector = vDir;
					Ptc.UpdateVectors();
					g_BsKernel.UpdateObject( tmp->_nFxGround, Ptc );

				}
				else if( tmp->_nFxEnemy[0] != -1 )
				{
					EnemyEffect(tmp);
				}

				tmp->_nTick1++;

				if( tmp->_bDraw )
				{
					if( tmp->_nTick1 >= g_nTick3 ) 
						tmp->_bDelete = true;
				}
				else
				{
					if( tmp->_nFxGround != -1 )
					{
						if( tmp->_nTick1 >= 50 )
							tmp->_bDelete = true;
					}
					else
					{
						if( tmp->_nTick1 >= 12 )
							tmp->_bDelete = true;
					}
					
				}
				
				
				++iter;
			}
		}// for
	}//if( !m_listSpoutsOfWater.empty() )

}






/*---------------------------------------------------------------------------------
-
-			CFcFxSpoutsOfWater::Process()
-					; 갱신.
-					  
-
---------------------------------------------------------------------------------*/
void CFcFxSpoutsOfWater::Process()
{
	if( m_state == ENDING )
	{
		Reset();
		m_state = END;
		m_DeviceDataState=FXDS_RELEASEREADY;
		return;
	}

	if( m_state == END ) return;

	UpdateObj();
}




/*---------------------------------------------------------------------------------
-
-			CFcFxSpoutsOfWater::PreRender()
-					; .
-					  
-
---------------------------------------------------------------------------------*/
void CFcFxSpoutsOfWater::PreRender()
{
}




/*---------------------------------------------------------------------------------
-
-			CFcFxSpoutsOfWater::Render()
-					; 자체 렌더링 코드는 없다. 기존 렌더링 되는 클래스를 이용.
-					  
-
---------------------------------------------------------------------------------*/
void CFcFxSpoutsOfWater::Render(C3DDevice *pDevice)
{
}




bool CFcFxSpoutsOfWater::IsAddObj( D3DXVECTOR3 p_vPos )
{
	if( !m_listSpoutsOfWater.empty() )
	{
		std::list<SpoutOfWater*>::iterator iter;
		for( iter = m_listSpoutsOfWater.begin() ; iter != m_listSpoutsOfWater.end() ; iter++ )
		{
			SpoutOfWater* tmp = *iter;
			if( tmp ) 
			{
				if( !tmp->_bDraw ) continue;
				D3DXVECTOR3 vPos1 = tmp->_vTarget;
				float fDist = D3DXVec3Length( &(vPos1-p_vPos) );
				if( fDist <= 80.0f ) return false;
			}
		}
	}
	return true;
}
/*---------------------------------------------------------------------------------
-
-			CFcFxSpoutsOfWater::CreateSpoutsOfWater()
-					; 물줄기 이펙트 생성.
-					  
-
---------------------------------------------------------------------------------*/
void CFcFxSpoutsOfWater::CreateSpoutsOfWater( D3DXVECTOR3 p_vPos, D3DXVECTOR3 p_vTarget, int p_nStart, bool p_bGround, bool p_bBig )
{
	SpoutOfWater* tmp = new SpoutOfWater;
	tmp->_nFxDropWater = -1;
    tmp->_nFxGround = -1;
	tmp->_vFirstPos = p_vPos;
	tmp->_vTarget = p_vTarget;

	tmp->_nTick1 = -p_nStart;
	tmp->_bGround = p_bGround;
	tmp->_bDelete = FALSE;
	tmp->_bBig = p_bBig;
	tmp->_nFxEnemy[0] = -1;
	tmp->_fRandom = -1;
	tmp->_bDraw = IsAddObj(p_vTarget);

	PushObj(tmp);

}





/*---------------------------------------------------------------------------------
-
-			CFcFxSpoutsOfWater::PushObj()
-					; vector에 삽입.
-					  
-
---------------------------------------------------------------------------------*/
void CFcFxSpoutsOfWater::PushObj( SpoutOfWater* p_pObj )
{
	m_listSpoutsOfWater.push_back(p_pObj);
}





/*---------------------------------------------------------------------------------
-
-			CFcFxSpoutsOfWater::DeleteAllObj()
-					; vector 에 삭제.
-					  
-
---------------------------------------------------------------------------------*/
void CFcFxSpoutsOfWater::DeleteAllObj()
{
	std::list<SpoutOfWater*>::iterator iter;
	if( !m_listSpoutsOfWater.empty() )
	{
		for( iter = m_listSpoutsOfWater.begin() ; iter != m_listSpoutsOfWater.end() ; iter++ )
		{
			SpoutOfWater* tmp = NULL;
			tmp = (SpoutOfWater*)*iter;
			if( tmp )
			{
				if( tmp->_nFxDropWater != -1 ) g_BsKernel.DeleteObject(tmp->_nFxDropWater);
				if( tmp->_nFxGround != -1 ) g_BsKernel.DeleteObject(tmp->_nFxGround);
				for( int i = 0 ; i < 35 ; i++ )
				{
					if( tmp->_nFxEnemy[i] != -1 ) g_BsKernel.DeleteObject(tmp->_nFxEnemy[i]);
					tmp->_nFxEnemy[i] = -1; 
				}
				
				if( tmp->_nCurveTrail != -1 ) {
					g_BsKernel.DeleteObject(tmp->_nCurveTrail);
					tmp->_nCurveTrail = -1; 
				}
				delete tmp;
			}
		}// for
		m_listSpoutsOfWater.clear();
	}//if( size > 0 )	
}





/*---------------------------------------------------------------------------------
-
-			CFcFxSpoutsOfWater::ProcessMessage()
-					; 메세지 처리.
-					  
-
---------------------------------------------------------------------------------*/
int  CFcFxSpoutsOfWater::ProcessMessage(int nCode,DWORD dwParam1/*=0*/, DWORD dwParam2/*=0*/,DWORD dwParam3/*=0*/)
{
	switch(nCode)
	{
	case FX_INIT_OBJECT:
		{
			m_hParent = *(GameObjHandle*)dwParam1;
			Init();
		}
		return 1;
	case FX_PLAY_OBJECT:
		{
			m_state = PLAY;
		}
		return 1;
	case FX_DELETE_OBJECT:
		{
#ifndef _LTCG
			if( dwParam1 )
			{
				BsAssert( GetFxRtti() == (int)dwParam1 );
			}
#endif //_LTCG
			m_state = ENDING;
		}
		return 1;
	case FX_UPDATE_OBJECT:
		{
			D3DXVECTOR3 a = *(D3DXVECTOR3*)dwParam1;
			D3DXVECTOR3 b = *(D3DXVECTOR3*)dwParam2;
			int c[4];
			memcpy( c, (int*)dwParam3, sizeof(c));
			bool bGround = false;
			bool bBig = false;
			if( c[1] > 0 ) bGround = true;
			if( c[3] > 0 ) bBig = true;
			CreateSpoutsOfWater( a, b, c[0], bGround, bBig );
			m_nLevel = c[2];
		}
		return 1;
	}

	if( CFcFXBase::ProcessMessage( nCode, dwParam1, dwParam2, dwParam3) )
		return 1;

	return 0;
}