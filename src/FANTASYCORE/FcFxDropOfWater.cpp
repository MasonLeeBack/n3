#include "StdAfx.h"
#include ".\fcfxdropofwater.h"
#include "BsKernel.h"
#include "FcGameObject.h"
#include "FcWorld.h"
#include "FcUtil.h"
#include "CrossVector.h"
#include "FcFXManager.h"
#include "Data\\FxList.h"
#include "Box3.h"
#include "Ray3.h"
#include "IntLin3Box3.h"

const int g_nTick = 30;
const float g_nTick2 = 80;
const float g_nTick3 = 120;
const float g_fHeight = 280.0f;
const float g_fRadius = 230.0f;
const float g_fScale = 25.0f;

void D3DXVec3RotationAxis( OUT D3DXVECTOR3* p_vOut, IN D3DXVECTOR3 p_vIn, IN D3DXVECTOR3 p_vInCenter, IN float p_fAngle, IN D3DXVECTOR3 p_vAxis )
{
	D3DXMATRIX tmpMat;
	D3DXMatrixRotationAxis(&tmpMat, &p_vAxis, p_fAngle);
	tmpMat._41 = p_vInCenter.x, tmpMat._42 = p_vInCenter.y, tmpMat._43 = p_vInCenter.z;
	D3DXVECTOR3 Dir = p_vIn - p_vInCenter;
	D3DXVec3TransformCoord(p_vOut, &Dir, &tmpMat);
}

/*---------------------------------------------------------------------------------
-
-			CFcFxDropOfWater::CFcFxDropOfWater()
-					; 생성자.
-					  
-
---------------------------------------------------------------------------------*/
CFcFxDropOfWater::CFcFxDropOfWater(void)
{
	m_nTick = 0;
	m_nFxTypeIndex1 = -1;
	m_nFxTypeIndex2 = -1;
	m_nFxid = -1;
	m_nParticle	= -1;
	m_nSkinindex = -1;

	for( int i = 0 ; i < 35 ; i++ )
		m_nEnemySkinindex[i] = -1;

#ifndef _LTCG
	SetFxRtti(FX_TYPE_DROPOFWATER);
#endif //_LTCG
}





/*---------------------------------------------------------------------------------
-
-			CFcFxDropOfWater::~CFcFxDropOfWater()
-					; 소멸자.
-					  
-
---------------------------------------------------------------------------------*/
CFcFxDropOfWater::~CFcFxDropOfWater(void)
{
	Finalize();
}






/*---------------------------------------------------------------------------------
-
-			CFcFxDropOfWater::Initialize()
-					; 소멸자.
-					  
-
---------------------------------------------------------------------------------*/
void CFcFxDropOfWater::Initialize()
{
	ClearTyruruObj();
	BsAssert( m_nSkinindex == -1 );
	BsAssert( m_nFxTypeIndex1 == -1 );
	BsAssert( m_nFxTypeIndex2 == -1 );
	g_BsKernel.chdir("fx");
	char szFullFileName[_MAX_PATH];
	/*sprintf(szFullFileName, "%s%s", g_BsKernel.GetCurrentDirectory(), "water_start.bfx" );
	m_nFxTypeIndex1 = g_BsKernel.LoadFXTemplate(FX_TYURRU_DROPWATER01, szFullFileName);
	BsAssert( m_nFxTypeIndex1 == FX_TYURRU_DROPWATER01 );*/
	sprintf(szFullFileName, "%s%s", g_BsKernel.GetCurrentDirectory(), "tyurr_waterShot.bfx" );
	m_nFxTypeIndex2 = g_BsKernel.LoadFXTemplate(FX_TYURURUSHOT, szFullFileName);
	BsAssert( m_nFxTypeIndex2 == FX_TYURURUSHOT );
	
	m_nParticle = 88;
	m_nSkinindex = g_BsKernel.LoadSkin( -1, "WATERBOMBSPHERE.SKIN" );
	
	CreateObject( -5, 40);
	m_nFxid = g_pFcFXManager->Create(FX_TYPE_LISTGENERATER);
	g_pFcFXManager->SendMessage(m_nFxid,FX_INIT_OBJECT, FX_TYURURUWATER01, 180, 12);
	g_pFcFXManager->SendMessage(m_nFxid,FX_PLAY_OBJECT);

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
}





/*---------------------------------------------------------------------------------
-
-			CFcFxDropOfWater::~Finalize()
-					; 소멸자.
-					  
-
---------------------------------------------------------------------------------*/
void CFcFxDropOfWater::Finalize()
{
	/*if( m_nFxid != -1 ) {
		g_pFcFXManager->SendMessage(m_nFxid,FX_DELETE_OBJECT);
		m_nFxid = -1;
	}*/
	SAFE_DELETE_FX(m_nFxid, FX_TYPE_LISTGENERATER);
	ClearTyruruObj();

	SAFE_RELEASE_FX(m_nFxTypeIndex1);
	SAFE_RELEASE_FX(m_nFxTypeIndex2);

	SAFE_RELEASE_SKIN(m_nSkinindex);
	
	for( int i = 0 ; i < 35 ; i++ )
	{
		SAFE_RELEASE_SKIN(m_nEnemySkinindex[i]);
	}
	m_DeviceDataState=FXDS_RELEASEREADY;
}





/*---------------------------------------------------------------------------------
-~
-			CFcFxDropOfWater::Process()
-					; 매 tick 갱신.
-					  
-
---------------------------------------------------------------------------------*/
void CFcFxDropOfWater::Process()
{
	if( m_state == END ) return;

	//UpdateObj1();
	UpdateObj2();
	
	TickCount();
}





/*---------------------------------------------------------------------------------
-
-			CFcFxDropOfWater::TickCount()
-					; .
-					  
-
---------------------------------------------------------------------------------*/
void CFcFxDropOfWater::TickCount()
{
	int size = m_vecFxObject.size();
	if( size > 0 )
	{
		for( int i = 0 ; i < size ; ++i)
		{
			FxTyruruObject* tmp;
			tmp = m_vecFxObject[i];

			switch(tmp->_nState)
			{
			case CREATE_DROP	: tmp->_nTick++; break;
			case UP_DROP		: tmp->_nTick2++; break;
            }
		}
	}
	m_nTick++;
}






/*---------------------------------------------------------------------------------
-
-			CFcFxDropOfWater::UpdateObj1()
-					; 물방울을 위 아래로 계속해서 회전시킨다.
-					  
-
---------------------------------------------------------------------------------*/
void CFcFxDropOfWater::UpdateObj1()  
{
	int size = m_vecFxObject.size();
	if( size > 0 )
	{
		for( int i = 0 ; i < size ; ++i)
		{
			FxTyruruObject* tmp;
			tmp = m_vecFxObject[i];

			if( tmp->_nTick == 0 )
			{
				CCrossVector Ptc;
				Ptc.m_PosVector = tmp->_vPos;
				Ptc.UpdateVectors();

				if( tmp->_nFxObjIndex1 == -1 )
				{
                    tmp->_nFxObjIndex1 = g_BsKernel.CreateFXObject(m_nFxTypeIndex1);
					g_BsKernel.SetFXObjectState( tmp->_nFxObjIndex1, CBsFXObject::PLAY );
				}

				g_BsKernel.UpdateObject( tmp->_nFxObjIndex1, Ptc );
				tmp->_nState = CREATE_DROP;
			}
			else if( tmp->_nTick > 0 && tmp->_nTick < g_nTick )
			{
				CCrossVector Ptc;
				Ptc.m_PosVector = tmp->_vPos;
				Ptc.UpdateVectors();
				g_BsKernel.UpdateObject( tmp->_nFxObjIndex1, Ptc );
			}
			else if( tmp->_nTick >= g_nTick && tmp->_nState == CREATE_DROP )
			{
				CCrossVector Ptc;
				Ptc.m_PosVector = tmp->_vPos;
				Ptc.UpdateVectors();
				
				if( tmp->_nFxObjIndex1 != -1 )
				{
					g_BsKernel.DeleteObject(tmp->_nFxObjIndex1);
					tmp->_nFxObjIndex1 = -1;
				}
				
				if( tmp->_nFxObjIndex2 == -1 )
				{
					tmp->_nFxObjIndex2 = CBsKernel::GetInstance().CreateStaticObjectFromSkin(m_nSkinindex);
				}
				D3DXMATRIX mat;
				D3DXMatrixIdentity(&mat);
				mat._41 = tmp->_vPos.x;
				mat._42 = tmp->_vPos.y;
				mat._43 = tmp->_vPos.z;
				mat._11 *= tmp->_fSize;
				mat._22 *= tmp->_fSize;
				mat._33 *= tmp->_fSize;
                g_BsKernel.UpdateObject( tmp->_nFxObjIndex2, &mat );
				tmp->_nState = UP_DROP;
				
			}//else if( tmp->_nTick >= 50 )
			else if( tmp->_nState == UP_DROP )
			{
				D3DXVECTOR3 Pos = m_vCenter;
				
				Pos.y = tmp->_vPos.y;

				float fValue;
				if( tmp->_nTick2 >= 23 ) fValue = 23.0f;
				else
					fValue = (float)tmp->_nTick2;
				D3DXVec3RotationAxis(&Pos, tmp->_vPos, Pos, D3DX_PI / (float)(40.0f - fValue) , D3DXVECTOR3(0,1,0) );

				tmp->_vPos = Pos;
				D3DXVECTOR3 vTop = D3DXVECTOR3(m_vCenter.x, m_vCenter.y+tmp->_fHeight, m_vCenter.z);
				D3DXVECTOR3 dir = tmp->_vPos - vTop;
				dir.y = 0;
				D3DXVec3Normalize(&dir, &dir);
				dir = vTop + dir*tmp->_fDistFromCenter;
				dir = dir - tmp->_vPos;
				D3DXVec3Normalize(&dir, &dir);

				float fdist = fValue * 0.2f;
				tmp->_vPos += tmp->_vDir * fdist;

				D3DXMATRIX mat;
				D3DXMatrixIdentity(&mat);
				mat._41 = tmp->_vPos.x;
				mat._42 = tmp->_vPos.y;
				mat._43 = tmp->_vPos.z;
				mat._11 *= tmp->_fSize;
				mat._22 *= tmp->_fSize;
				mat._33 *= tmp->_fSize;
				g_BsKernel.UpdateObject( tmp->_nFxObjIndex2, &mat );

				if( tmp->_vPos.y >= (m_vCenter.y + tmp->_fHeight) )
				{
					tmp->_vDir.y *= -1.0f;
				}
				else if( tmp->_vPos.y <= m_vCenter.y )
				{
					tmp->_vDir.y *= -1.0f;
				}
			}

		}// for
	}//if( size > 0 )	
}





/*---------------------------------------------------------------------------------
-
-			CFcFXColumnOfWater::UpdateObj2()
-					; 공격물줄기가 날아가면서 바닦과 충돌시 혹은 적과 충돌시 적절한
-					  이펙트를 생성한다. ( 데미지 처리 포함 )
-
---------------------------------------------------------------------------------*/
void CFcFxDropOfWater::UpdateObj2() 
{
	
	if( !m_listSpoutObject.empty() )
	{
		std::list<FxSpoutWater*>::iterator iter;
		for( iter = m_listSpoutObject.begin() ; iter != m_listSpoutObject.end() ; )
		{
			
			FxSpoutWater* tmp = *iter;
			if( tmp->_nStartTick < 0 ) 
			{
				tmp->_nStartTick++;
				++iter;
				continue;
			}
			tmp->_vPos += tmp->_vVel;
			float fHeight = g_FcWorld.GetLandHeight(tmp->_vPos.x, tmp->_vPos.z);
			if( tmp->_vPos.y < fHeight )
			{
				if( tmp->_nFxIndex != -1 ) g_BsKernel.DeleteObject(tmp->_nFxIndex );
				D3DXVECTOR3 vDir = D3DXVECTOR3(tmp->_vVel.x, 0, tmp->_vVel.z);
				D3DXVec3Normalize(&vDir, &vDir);
				D3DXVECTOR3 vDist = m_vCenter - tmp->_vPos;
				if( RandomNumberInRange(0,4) == 2 )
                    g_pFcFXManager->SendMessage(m_nFxid,FX_UPDATE_OBJECT, (DWORD)&tmp->_vPos, (DWORD)&vDir );

				_FC_RANGE_DAMAGE rd;
				rd.Attacker = m_hParent;
				rd.pPosition = &tmp->_vPos;
				rd.fRadius = 500.0f;
				rd.nPower = 250; // 튜루루 오브어택 데미지파워
				rd.nPushY = 8; 
				rd.nPushZ = 10;
				rd.nTeam = m_hParent->GetTeam();

				g_FcWorld.GiveRangeDamage( rd,true,false );	// 데미지 처리하기.
				delete tmp;
				iter = m_listSpoutObject.erase(iter);
			}
			else
			{
                CCrossVector Ptc;
				Ptc.m_PosVector = tmp->_vPos;
				Ptc.m_ZVector = tmp->_vVel;
				D3DXVec3Normalize( &Ptc.m_ZVector, &Ptc.m_ZVector );
				Ptc.UpdateVectors();
				g_BsKernel.UpdateObject(tmp->_nFxIndex, Ptc);

				Box3 Box;
				Segment3 Seg3;
				std::vector<GameObjHandle> objList;
				g_FcWorld.GetEnemyObjectListInRange( m_hParent->GetTeam(), &tmp->_vPos, 150.0f, objList);

				int nSize = objList.size();
				if( nSize > 0 )
				{
					for( int i = 0 ; i < nSize ; i++ )
					{
						objList[i]->GetBox3( Box, objList[i]->GetBoundingBox() );
						Seg3.P.x = tmp->_vPos.x;
						Seg3.P.y = tmp->_vPos.y;
						Seg3.P.z = tmp->_vPos.z;

						Seg3.D.x = tmp->_vPos.x - tmp->_vVel.x;
						Seg3.D.y = tmp->_vPos.y - tmp->_vVel.y;
						Seg3.D.z = tmp->_vPos.z - tmp->_vVel.z;
						if( TestIntersection( Seg3, Box ) == true ) 
						{
							//데미지 처리및 물줄기 없애기 그리고 물 거시기 나오기
							_FC_RANGE_DAMAGE rd;
							rd.Attacker = m_hParent;
							rd.pPosition = &tmp->_vPos;
							rd.fRadius = 100.0f;
							rd.nPower = 500;
							rd.nPushY = 5; 
							rd.nPushZ = 10;
							rd.nTeam = m_hParent->GetTeam();
							rd.nGuardBreak = 15;
							g_FcWorld.GiveRangeDamage( rd,true,false );
							
							if( m_listWater.size() > 8 ) continue;

							FxWater* tmp2 = new FxWater;
							tmp2->_vPos = tmp->_vPos;
							tmp2->_vDir = tmp->_vVel;
							D3DXVec3Normalize( &tmp2->_vDir, &tmp2->_vDir );
							for( int i = 0 ; i < 35 ; i++ )
								tmp2->_nFxIndex[i] = CBsKernel::GetInstance().CreateStaticObjectFromSkin(m_nEnemySkinindex[i]);

							m_listWater.push_back(tmp2);
						}
					}
				}
				++iter;
			}
		}
	}//if

	if( !m_listWater.empty())
	{
		std::list<FxWater*>::iterator iter;
		for( iter = m_listWater.begin() ; iter != m_listWater.end() ; )
		{
			FxWater* tmp = *iter;
			CCrossVector cross;
			cross.m_PosVector = tmp->_vPos;
			cross.m_ZVector = tmp->_vDir;
			cross.m_ZVector.y = 0;
			cross.UpdateVectors();
			D3DXMATRIX mat = *(D3DXMATRIX*)cross;
			D3DXMATRIX tmp2;
			D3DXMatrixIdentity(&tmp2);
			tmp2._11 *= 0.4f, tmp2._22 *= 0.6f, tmp2._33 *= 0.4f;
			D3DXMatrixMultiply(&mat, &tmp2, &mat);
			g_BsKernel.UpdateObject(tmp->_nFxIndex[tmp->_nCurrentId++], &mat);
			if( tmp->_nCurrentId > 34 )
			{
				for( int i = 0 ; i < 35 ; i++ )
				{
					if( tmp->_nFxIndex[i] != -1 ) g_BsKernel.DeleteObject(tmp->_nFxIndex[i]);
					tmp->_nFxIndex[i] = -1; 
				}
				delete tmp;
				iter = m_listWater.erase(iter);
				continue;
			}
			++iter;
		}
	}
}









/*---------------------------------------------------------------------------------
-
-			CFcFXColumnOfWater::PreRender()
-					; .
-					  
-
---------------------------------------------------------------------------------*/
void CFcFxDropOfWater::PreRender()
{
}





/*---------------------------------------------------------------------------------
-
-			CFcFXColumnOfWater::Render()
-					; .
-					  
-
---------------------------------------------------------------------------------*/
void CFcFxDropOfWater::Render(C3DDevice *pDevice)
{
}





/*---------------------------------------------------------------------------------
-
-			CFcFxDropOfWater::CreateObject()
-					; 물방울 생성해서 vector에 추가.
-					  
-
---------------------------------------------------------------------------------*/
void CFcFxDropOfWater::CreateObject( int p_nFirstTick, int p_nNum )
{
	D3DXVECTOR3 vPos, vMin, vMax;
	vMin = m_vCenter - D3DXVECTOR3(1,0,0) * g_fRadius - D3DXVECTOR3(0,0,1) * g_fRadius;
	vMax = m_vCenter + D3DXVECTOR3(1,0,0) * g_fRadius + D3DXVECTOR3(0,0,1) * g_fRadius;
	vMin.y += 50.0f;
	vMax.y += 150.0f;

	for( int i = 0 ; i < p_nNum ; ++i )
	{
		vPos.x = RandomNumberInRange(vMin.x, vMax.x);
		vPos.y = RandomNumberInRange(vMin.y, vMax.y);
		vPos.z = RandomNumberInRange(vMin.z, vMax.z);
		FxTyruruObject* tmp = new FxTyruruObject;
		tmp->_vPos = vPos;

		CCrossVector Ptc;
		Ptc.m_PosVector = tmp->_vPos;
		Ptc.UpdateVectors();

		tmp->_nFxObjIndex1 = -1;
		tmp->_nFxObjIndex2 = -1;
		tmp->_fDistFromCenter = 60.0f * RandomNumberInRange(0.7f, 1.5f);
		tmp->_fHeight = g_fHeight*RandomNumberInRange(0.9f, 1.2f);

		tmp->_fSize = g_fScale * RandomNumberInRange(0.5f, 1.3f);

		tmp->_nTick = p_nFirstTick - i * RandomNumberInRange( 1, 4 );
		tmp->_vDir = D3DXVECTOR3(0,1,0);
		AddTyruruObj(tmp);
	}


}





/*---------------------------------------------------------------------------------
-
-			CFcFXColumnOfWater::AddTyruruObj()
-					; vector 추가 함수.
-					  
-
---------------------------------------------------------------------------------*/
void CFcFxDropOfWater::AddTyruruObj(FxTyruruObject* p_Obj)
{
	m_vecFxObject.push_back(p_Obj);
}









/*---------------------------------------------------------------------------------
-
-			CFcFXColumnOfWater::ClearTyruruObj()
-					; 모든 컨테이너 값 제거.
-					  
-
---------------------------------------------------------------------------------*/
void CFcFxDropOfWater::ClearTyruruObj()
{
	int size = m_vecFxObject.size();
	if( size > 0 )
	{
		for( int i = 0 ; i < size ; ++i)
		{
			FxTyruruObject* tmp;
			tmp = m_vecFxObject[i];
			if( tmp )
			{
				if( tmp->_nFxObjIndex1 != -1 ) g_BsKernel.DeleteObject(tmp->_nFxObjIndex1);
				if( tmp->_nFxObjIndex2 != -1 ) g_BsKernel.DeleteObject(tmp->_nFxObjIndex2);
				delete tmp;
			}
		}// for
		m_vecFxObject.clear();
	}//if( size > 0 )	

	if( !m_listSpoutObject.empty() )
	{
		std::list<FxSpoutWater*>::iterator iter;
		for( iter = m_listSpoutObject.begin() ; iter != m_listSpoutObject.end() ; iter++ )
		{
			FxSpoutWater* tmp = *iter;
			if( tmp->_nFxIndex != -1 ) g_BsKernel.DeleteObject(tmp->_nFxIndex );
			delete tmp;
		}
		m_listSpoutObject.clear();
	}

	if( !m_listWater.empty())
	{
		std::list<FxWater*>::iterator iter;
		for( iter = m_listWater.begin() ; iter != m_listWater.end() ; iter++ )
		{
			FxWater* tmp = *iter;
			for( int i = 0 ; i < 35 ; i++ )
			{
				if( tmp->_nFxIndex[i] != -1 ) g_BsKernel.DeleteObject(tmp->_nFxIndex[i]);
				tmp->_nFxIndex[i] = -1; 
			}
			delete tmp;
		}
		m_listWater.clear();
	}
}





/*---------------------------------------------------------------------------------
-
-			CFcFXColumnOfWater::ProcessMessage()
-					; 메세지 처리.
-					  
-
---------------------------------------------------------------------------------*/
int	CFcFxDropOfWater::ProcessMessage(int nCode,DWORD dwParam1/*=0*/, DWORD dwParam2/*=0*/,DWORD dwParam3/*=0*/)
{
	switch(nCode)
	{
	case FX_INIT_OBJECT:
		{
			m_vCenter = *(D3DXVECTOR3*)dwParam1;
			m_hParent = *(GameObjHandle*)dwParam2;
			Initialize();
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
			m_state = END;
			Finalize();
		}
		return 1;
	case FX_UPDATE_OBJECT:
		{
			D3DXVECTOR3 vPos, vVel;
			vPos = *(D3DXVECTOR3*)dwParam1;
			vVel = *(D3DXVECTOR3*)dwParam2;
			FxSpoutWater* tmp = new FxSpoutWater;
			tmp->_nFxIndex = g_BsKernel.CreateFXObject(m_nFxTypeIndex2);
			g_BsKernel.SetFXObjectState( tmp->_nFxIndex, CBsFXObject::PLAY, -99 );
			tmp->_vPos = vPos;
			tmp->_vVel = vVel;

			m_vCenter = *(D3DXVECTOR3*)dwParam3;
			tmp->_nStartTick = 0;//-(int)dwParam3 * RandomNumberInRange(3,6);
			CCrossVector cross;
			cross.m_PosVector = tmp->_vPos;
			cross.m_ZVector = tmp->_vVel;
			D3DXVec3Normalize(&cross.m_ZVector, &cross.m_ZVector);
			cross.UpdateVectors();
			m_listSpoutObject.push_back(tmp);

			g_BsKernel.CreateParticleObject(m_nParticle, false, false, cross);
		}
		return 1;
	}

	if( CFcFXBase::ProcessMessage( nCode, dwParam1, dwParam2, dwParam3) )
		return 1;

	return 0;
}
