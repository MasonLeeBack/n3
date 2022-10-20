#include "StdAfx.h"
#include ".\fcfxshotskin.h"
#include "BsKernel.h"
#include "FcGameObject.h"
#include "FcWorld.h"
#include "FcUtil.h"
#include "CrossVector.h"
#include "Box3.h"
#include "Ray3.h"
#include "IntLin3Box3.h"
#include "FcFXManager.h"

//튜르르 레벨에따란 물덩이 날리는것 크기.
const float g_fvalue[9] = { 50.0f, 50.0f, 50.0f, 70.0f, 70.0f, 80.0f, 80.0f, 100.0f, 100.0f };
//튜르르 레벨에따란 일반 물덩이 공격 파워.
const float g_fvalue4[9] = { 200.0f, 210.0f, 220.0f, 220.0f, 220.0f, 220.0f, 230.0f, 230.0f, 230.0f };
//튜르르 레벨에따란 일반 물덩이 데미지 반경.
const float g_fvalue6[9] = { 210.0f, 220.0f, 230.0f, 240.0f, 250.0f, 260.0f, 270.0f, 280.0f, 290.0f };

//튜르르 레벨에따란 물덩이 홀딩 최대 크기.
const float g_fvalue2[9] = { 70.0f, 80.0f, 90.0f, 100.0f, 110.0f, 120.0f, 130.0f, 140.0f, 150.0f };
//튜르르 레벨에따란 홀딩 물덩이 데미지 반경.
const float g_fvalue3[9] = { 250.0f, 250.0f, 250.0f, 280.0f, 290.0f, 300.0f, 310.0f, 320.0f, 330.0f };
//튜르르 레벨에따란 홀딩 물덩이 공격 파워.
const float g_fvalue5[9] = { 250.0f, 250.0f, 250.0f, 270.0f, 270.0f, 290.0f, 290.0f, 330.0f, 330.0f };

// 물덩어리 날아가는 속도 (가속도)
const float g_fVel = 24.0f;

// 일반 적 전방 밀기 값
const int g_nPushZsmall[9] = { -8, -8, -8, -8, -8, -8, -8, -8, -8 };
// 일반 적 상방 밀기 값
const int g_nPushYsmall[9] = { 28, 28, 28, 28, 28, 28, 28, 28, 28 };

// 홀드공격 적 전방 밀기 값
const int g_nPushZbig[9] = { -12, -12, -12, -12, -12, -12, -12, -12, -12 };
// 홀드공격 적 상방 밀기 값
const int g_nPushYbig[9] = { 24, 24, 28, 34, 34, 34, 34, 34, 34 };




/*---------------------------------------------------------------------------------
--
-			CFcFxShotSkin::CFcFxShotSkin()
-					; 생성자.
-					  
-
---------------------------------------------------------------------------------*/
CFcFxShotSkin::CFcFxShotSkin(void)
{
	SetScale(0);
	SetFxTem(-1);
	SetListGenerater(-1);
	SetSkin(-1);
	m_bFire = false;
	m_nDelay = -1;
	m_bChangeTarget = false;
#ifndef _LTCG
	SetFxRtti(FX_TYPE_SHOTSKIN);
#endif //_LTCG
}





/*---------------------------------------------------------------------------------
--
-			CFcFxShotSkin::~CFcFxShotSkin()
-					; 소멸자.
-					  
-
---------------------------------------------------------------------------------*/
CFcFxShotSkin::~CFcFxShotSkin(void)
{
}





/*---------------------------------------------------------------------------------
--
-			CFcFxShotSkin::CFcFxShotSkin()
-					; 생성자.
-					  
-
---------------------------------------------------------------------------------*/
void CFcFxShotSkin::Initialize()
{
	m_bFire = false;
	m_bChangeTarget = false;
	InitDeviceData();
	SetListGenerater( g_pFcFXManager->Create(FX_TYPE_LISTGENERATER) );
	g_pFcFXManager->SendMessage(GetListGenerater(), FX_INIT_OBJECT, (DWORD)GetFxTem(), 100);
	g_pFcFXManager->SendMessage(GetListGenerater() , FX_PLAY_OBJECT );
}






/*---------------------------------------------------------------------------------
--
-			CFcFxShotSkin::CFcFxShotSkin()
-					; 생성자.
-					  
-
---------------------------------------------------------------------------------*/
void CFcFxShotSkin::Finalize()
{
	ReleaseDeviceData();

	if( !m_listObj.empty() )
	{
		std::list<sSkinObj*>::iterator iter;
		for( iter = m_listObj.begin() ; iter != m_listObj.end() ; iter++ )
		{
			sSkinObj* tmp = *iter;
			iter = m_listObj.erase(iter);
			g_BsKernel.DeleteObject( tmp->_nObjIndex );
			delete tmp;
		}
		m_listObj.clear();
	}

	if( GetListGenerater() != -1 )
	{
		g_pFcFXManager->SendMessage( GetListGenerater(), FX_DELETE_OBJECT );
		SetListGenerater(-1);
	}
}






/*---------------------------------------------------------------------------------
--
-			CFcFxShotSkin::CFcFxShotSkin()
-					; 생성자.
-					  
-
---------------------------------------------------------------------------------*/
void CFcFxShotSkin::InitDeviceData()
{
	m_DeviceDataState = FXDS_INITREADY;
}





/*---------------------------------------------------------------------------------
--
-			CFcFxShotSkin::CFcFxShotSkin()
-					; 생성자.
-					  
-
---------------------------------------------------------------------------------*/
void CFcFxShotSkin::ReleaseDeviceData()
{
	m_DeviceDataState=FXDS_RELEASEREADY;
}





/*---------------------------------------------------------------------------------
--
-			CFcFxShotSkin::CFcFxShotSkin()
-					; 생성자.
-					  
-
---------------------------------------------------------------------------------*/
void CFcFxShotSkin::CreateObj(D3DXVECTOR3 p_vPos, D3DXVECTOR3 p_vTPos, int p_n/*=1*/ )
{
	sSkinObj* tmp = NULL;
	tmp = new sSkinObj;
	if( p_n == 0 )
	{
		tmp->_fScale = g_fvalue[m_hParent->GetLevel()];
		tmp->_state = Create;
		tmp->_fVel = g_fVel;
	}
	else
	{
		if( p_n > 50 )
		{
			tmp->_fVel = 7.0f;
		}
		else
		{
			tmp->_fVel = 5.5f;
		}
		m_nDelay = p_n;
		tmp->_state = BeforCreate;
	}
	if( GetSkin() != -1 )
        tmp->_nObjIndex = CBsKernel::GetInstance().CreateStaticObjectFromSkin( GetSkin() );
	else
		BsAssert(0);

	tmp->_vPos = p_vPos;
	tmp->_vFirstPos = p_vPos;
	tmp->_vTPos = p_vTPos;
	tmp->_vVel = D3DXVECTOR3(0,0,0);

	m_listObj.push_back(tmp);

}






/*---------------------------------------------------------------------------------
--
-			CFcFxShotSkin::CFcFxShotSkin()
-					; 생성자.
-					  
-
---------------------------------------------------------------------------------*/
void CFcFxShotSkin::Process()
{
	if( m_state == ENDING )
	{
		Finalize();
		m_state = END;
		return;
	}
	else if( m_state == END ) return;

	UpdateObj();
}





/*---------------------------------------------------------------------------------
--
-			CFcFxShotSkin::CFcFxShotSkin()
-					; 생성자.
-					  
-
---------------------------------------------------------------------------------*/
void CFcFxShotSkin::UpdateObj()
{
	static int ntick = 0;

	if( !m_listObj.empty() ) {
		std::list<sSkinObj*>::iterator iter;
		for( iter = m_listObj.begin() ; iter != m_listObj.end() ; ) {
			sSkinObj* tmp = *iter;
			if( tmp->_state == Destory ) 
			{
				iter = m_listObj.erase(iter);
				if( tmp->_nObjIndex != -1 ) {
					g_BsKernel.DeleteObject(tmp->_nObjIndex);
                    tmp->_nObjIndex = -1;
				}
				delete tmp;
				continue;
			}
			else if( tmp->_state == BeforCreate ) {
				D3DXMATRIX mat;
				D3DXMatrixIdentity(&mat);
				mat._41 = tmp->_vPos.x;
				mat._42 = tmp->_vPos.y;
				mat._43 = tmp->_vPos.z;
				if( GetScale() > 0.0f ){
					tmp->_fScale = GetScale();
					mat._11 *= tmp->_fScale;
					mat._22 *= tmp->_fScale;
					mat._33 *= tmp->_fScale;
				}
				else
					tmp->_fScale = 1.0f;

				g_BsKernel.UpdateObject( tmp->_nObjIndex, &mat );

				if( m_bFire )
				{	
					tmp->_state = Create;
					m_bFire = false;
					ntick = 0;
				}
				ntick++;
				if( m_nDelay > 0 && ntick >= m_nDelay ) {
					ntick = 0;
					tmp->_state = Create;
					m_bFire = false;
				}
			}
			else {
				D3DXVECTOR3 vDir = tmp->_vTPos - tmp->_vFirstPos;
				if( m_bChangeTarget )
				{
					tmp->_vTPos = m_vTarget;
					vDir = tmp->_vTPos - tmp->_vFirstPos;
					m_bChangeTarget = false;
				}
				D3DXVec3Normalize(&vDir, &vDir);
				tmp->_vVel += vDir * tmp->_fVel;
				tmp->_vPos += tmp->_vVel;
				D3DXMATRIX mat;
				D3DXMatrixIdentity(&mat);
				CCrossVector cross;
				cross.m_PosVector = tmp->_vPos;
				cross.m_ZVector = vDir * 1.2f;
				cross.UpdateVectors();
				if( tmp->_fScale > 0.0f ){
					mat._11 *= tmp->_fScale;
					mat._22 *= tmp->_fScale;
					mat._33 *= tmp->_fScale;
				}
				D3DXMATRIX mat2;
				D3DXMatrixMultiply( &mat2, &mat, cross);
				// 충돌관련.. 젤로 빡세네..
				bool bCol = false;
				
				D3DXVECTOR3 vPrePos = tmp->_vPos - vDir * tmp->_fVel;
				Box3 Box;
				Segment3 Seg3;
				std::vector<GameObjHandle> obj;
				g_FcWorld.GetEnemyObjectListInRange(m_hParent->GetTeam(), &tmp->_vPos, tmp->_fScale*1.5f, obj);
				int nSize = obj.size();
				if( nSize>0 )
				{
					for(int i = 0 ; i < nSize ; i++ )
					{
						obj[i]->GetBox3( Box, obj[i]->GetBoundingBox() );
						Seg3.P.x = tmp->_vPos.x;
						Seg3.P.y = tmp->_vPos.y;
						Seg3.P.z = tmp->_vPos.z;

						Seg3.D.x = tmp->_vPos.x - vPrePos.x;
						Seg3.D.y = tmp->_vPos.y - vPrePos.y;
						Seg3.D.z = tmp->_vPos.z - vPrePos.z;
						if( TestIntersection( Seg3, Box ) == true ) 
						{
							bCol = true;
						}
					}
				}
				if( tmp->_vPos.y < g_FcWorld.GetLandHeight(tmp->_vPos.x, tmp->_vPos.z) || bCol ) {
					g_pFcFXManager->SendMessage( GetListGenerater(), FX_UPDATE_OBJECT, (DWORD)&tmp->_vPos, (DWORD)&D3DXVECTOR3(1,0,0));

					_FC_RANGE_DAMAGE rd;
					rd.Attacker = m_hParent;
					rd.pPosition = &tmp->_vPos;
					
					if( GetScale() > 100.0f ) {
						rd.nPower = (int)g_fvalue5[m_hParent->GetLevel()];
						rd.fRadius = g_fvalue3[m_hParent->GetLevel()];
						rd.nPushY = g_nPushYbig[m_hParent->GetLevel()]; 
						rd.nPushZ = g_nPushZbig[m_hParent->GetLevel()];
					}
					else
					{
						rd.nPower = (int)g_fvalue4[m_hParent->GetLevel()];
						rd.fRadius = g_fvalue6[m_hParent->GetLevel()];
						rd.nPushY = g_nPushYsmall[m_hParent->GetLevel()]; 
						rd.nPushZ = g_nPushZsmall[m_hParent->GetLevel()];
					}
					
					
					rd.nTeam = m_hParent->GetTeam();
					rd.nNotGenOrb = 0;
					rd.nNotLookAtHitUnit = 0;
					rd.p_bTeamDamage = false;
					rd.pHitDirection = NULL; 
					rd.nType = HT_NORMAL_ATTACK; 
					rd.WeaponType = WT_TYURRU; 
					rd.SoundAttackType = ATT_LITTLE;
					rd.nGuardBreak = 30;
					g_FcWorld.GiveRangeDamage( rd,true, false );
					tmp->_state = Destory;
				}
				
				g_BsKernel.UpdateObject( tmp->_nObjIndex, &mat2 );
			}
			iter++;

		}//for
	}//if
	

}





/*---------------------------------------------------------------------------------
--
-			CFcFxShotSkin::CFcFxShotSkin()
-					; 생성자.
-					  
-
---------------------------------------------------------------------------------*/
void CFcFxShotSkin::PreRender()
{
}






/*---------------------------------------------------------------------------------
--
-			CFcFxShotSkin::CFcFxShotSkin()
-					; 생성자.
-					  
-
---------------------------------------------------------------------------------*/
void CFcFxShotSkin::Render(C3DDevice *pDevice)
{
}



void CFcFxShotSkin::SetChangeTarget( IN D3DXVECTOR3 p_vTarget )
{
	m_vTarget = p_vTarget;
	m_bChangeTarget = true;
}



/*---------------------------------------------------------------------------------
--
-			CFcFxShotSkin::CFcFxShotSkin()
-					; 생성자.
-					  
-
---------------------------------------------------------------------------------*/
int CFcFxShotSkin::ProcessMessage(int nCode,DWORD dwParam1/*=0*/, DWORD dwParam2/*=0*/,DWORD dwParam3/*=0*/)
{
	switch(nCode)
	{
	case FX_INIT_OBJECT: 
		{
			if( dwParam1 > 0 ) SetSkin((int)dwParam1);
			if( dwParam2 > 0 ) SetFxTem((int)dwParam2);
			if( dwParam3 > 0 ) m_hParent = *(GameObjHandle*)dwParam3;
			Initialize();
		}
		return 1;
	case FX_ADD_VALUE: 
		{
			if( dwParam1 > 0 ) {	// 점점 커지는 공격
				if( ( GetScale()+ (*(float*)dwParam1) ) >= g_fvalue2[m_hParent->GetLevel()] )
					SetScale( g_fvalue2[m_hParent->GetLevel()] );
				else
					SetScale( (GetScale() + (*(float*)dwParam1)) );
			}

			if( dwParam2 > 0 ) {	// 일반공격
				if( ( GetScale()+ (*(float*)dwParam2) ) >= g_fvalue[m_hParent->GetLevel()] )
					SetScale( g_fvalue[m_hParent->GetLevel()] );
				else
					SetScale( (GetScale() + (*(float*)dwParam2)) );
			}

			if( dwParam3 > 0 ) {	// 딜레이 타임 증가
				m_nDelay += (int)dwParam3;
			}
		}
		return 1;
	case FX_ADD_VALUE2:
		{
			m_bFire = true;
			if( dwParam1>0 ) { // 점점 커지기 발사.
				SetChangeTarget(*(D3DXVECTOR3*)dwParam1);
			}
		}
		return 1;
	case FX_PLAY_OBJECT: 
		{
			m_state = PLAY;
			
		}
		return 1;
	case FX_UPDATE_OBJECT: 
		{
			//안전코드 넣지말자. 넣었다가 뻑안나고 고생하면 더 짱남. -  _-;
			if( dwParam3>0 ) { // 점점 커지기 발사.
				CreateObj( *(D3DXVECTOR3*)dwParam1, *(D3DXVECTOR3*)dwParam2, (int)dwParam3 );
			}
			else
			{				// 그냥 일반발사.
				CreateObj( *(D3DXVECTOR3*)dwParam1, *(D3DXVECTOR3*)dwParam2, 0 );
			}
			SetScale( 0.0f );
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

	}

	if ( CFcFXBase::ProcessMessage( nCode, dwParam1, dwParam2, dwParam3) )
		return 1;

	return 0;
}
