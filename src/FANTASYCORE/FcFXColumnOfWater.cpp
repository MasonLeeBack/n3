#include "StdAfx.h"
#include ".\fcfxcolumnofwater.h"
#include "BsKernel.h"
#include "FcUtil.h"
#include "FcWorld.h"


//물기둥 작은것 데미지 파워.
const int g_nvalue[9] = { 220, 220, 220, 220, 220, 220, 220, 220, 220 };
//물기둥 작은것 데미지 반경.
const float g_fvalue1[9] = { 250.0f, 250.0f, 280.0f, 280.0f, 300.0f, 300.0f, 300.0f, 300.0f, 300.0f };
//물기둥 큰것 데미지 파워.
const int g_nvalue2[9] = { 500, 500, 500, 500, 500, 500, 500, 500, 500 };
//물기둥 큰것 데미지 반경.
const float g_fvalue3[9] = { 350.0f, 370.0f, 390.0f, 410.0f, 430.0f, 450.0f, 470.0f, 490.0f, 510.0f };


// 작은것 적 전방 밀기 값
const int g_nPushZsmall[9] = { 6, 6, 6, 6, 6, 6, 6, 6, 6 };
// 작은것 적 상방 밀기 값
const int g_nPushYsmall[9] = { 46, 46, 46, 46, 46, 46, 46, 46, 46 };

// 큰것 적 전방 밀기 값
const int g_nPushZbig[9] = { 9, 9, 9, 9, 9, 9, 9, 9, 9 };
// 큰것 적 상방 밀기 값
const int g_nPushYbig[9] = { 62, 62, 62, 62, 62, 62, 62, 62, 62 };


/*---------------------------------------------------------------------------------
-
-			CFcFXColumnOfWater::CFcFXColumnOfWater()
-					; 생성자.
-					  
-
---------------------------------------------------------------------------------*/
CFcFXColumnOfWater::CFcFXColumnOfWater(void)
{
	Finalize();
#ifndef _LTCG
	SetFxRtti(FX_TYPE_COLUMNWATER);
#endif //_LTCG
}





/*---------------------------------------------------------------------------------
-
-			CFcFXColumnOfWater::CFcFXColumnOfWater()
-					; 소멸자.
-					  
-
---------------------------------------------------------------------------------*/
CFcFXColumnOfWater::~CFcFXColumnOfWater(void)
{
	Finalize();
}





/*---------------------------------------------------------------------------------
-
-			CFcFXColumnOfWater::Finalize()
-					; 모든 값들을 초기화 및 소멸한다.
-					  
-
---------------------------------------------------------------------------------*/
void CFcFXColumnOfWater::Finalize()
{
	SetLife(0);
	DeleteAllObj();
	m_DeviceDataState=FXDS_RELEASEREADY;
}





/*---------------------------------------------------------------------------------
-
-			CFcFXColumnOfWater::Initialize()
-					; 초기화( 라이프 tick를 세팅).
-					  
-
---------------------------------------------------------------------------------*/
void CFcFXColumnOfWater::Initialize( int p_nLifeTick )
{
	SetParentLevel(0);
	SetLife(p_nLifeTick);
}





/*---------------------------------------------------------------------------------
-
-			CFcFXColumnOfWater::CreateNormalWaterObj()
-					; 물기둥 생성.
-					  
-
---------------------------------------------------------------------------------*/
void CFcFXColumnOfWater::CreateNormalWaterObj( D3DXVECTOR3 p_vPos, int p_nFxId, bool p_bSmall, int p_nDelayTick )
{
	ColumnWaterObj* tmp = new ColumnWaterObj;
	if( !tmp ) return;
	int objId = g_BsKernel.CreateFXObject( p_nFxId );
	tmp->_index = objId;
	tmp->_vPos = p_vPos;
	tmp->_bSmall = p_bSmall;
	tmp->_nTick = -p_nDelayTick - 1;
	PushNormalWaterObj(tmp);
	
}




/*---------------------------------------------------------------------------------
-
-			CFcFXColumnOfWater::PushNormalWaterObj()
-					; vector에 추가.
-					  
-
---------------------------------------------------------------------------------*/
void CFcFXColumnOfWater::PushNormalWaterObj( ColumnWaterObj *p_pObj )
{
	if(p_pObj)
		m_listFxObj.push_back(p_pObj);
}




/*---------------------------------------------------------------------------------
-
-			CFcFXColumnOfWater::DeleteAllObj()
-					; vector 제거.
-					  
-
---------------------------------------------------------------------------------*/
void CFcFXColumnOfWater::DeleteAllObj()
{
	std::list<ColumnWaterObj*>::iterator iter;

	for( iter = m_listFxObj.begin() ; iter != m_listFxObj.end() ; iter++ )
	{
		ColumnWaterObj* tmp = *iter;
		if( tmp )
		{
			g_BsKernel.DeleteObject(tmp->_index);
			delete tmp;
		}
	}
	m_listFxObj.clear();	
}




/*---------------------------------------------------------------------------------
-
-			CFcFXColumnOfWater::Update()
-					; 매 tick 호출되는 Process 내부 함수.
-					  
-
---------------------------------------------------------------------------------*/
void CFcFXColumnOfWater::Update()
{
	std::list<ColumnWaterObj*>::iterator iter;

	for( iter = m_listFxObj.begin() ; iter != m_listFxObj.end() ; )
	{
		ColumnWaterObj* tmp = *iter;
		int nLastTick;
		if( tmp->_bSmall ) nLastTick = 100;
		else nLastTick = 200;
		if( tmp->_nTick == nLastTick )
		{
			g_BsKernel.DeleteObject(tmp->_index);
			delete tmp;
            iter = m_listFxObj.erase(iter);
			continue;
		}
		if( tmp->_nTick == 0 )
		{
			D3DXMATRIX tmpMat;
			D3DXMatrixIdentity(&tmpMat);
			tmpMat._41 = tmp->_vPos.x; tmpMat._42 = tmp->_vPos.y; tmpMat._43 = tmp->_vPos.z;
			g_BsKernel.UpdateObject( tmp->_index , &tmpMat );

			g_BsKernel.SetFXObjectState( tmp->_index, CBsFXObject::PLAY );

			

		}
		else if( tmp->_nTick >= 0 )
		{
			D3DXMATRIX tmpMat;
			D3DXMatrixIdentity(&tmpMat);
			tmpMat._41 = tmp->_vPos.x; tmpMat._42 = tmp->_vPos.y; tmpMat._43 = tmp->_vPos.z;
			if(tmp->_nTick==6 ){
				GameObjHandle hParent = m_hParent;
				_FC_RANGE_DAMAGE rd;
				rd.Attacker = m_hParent;
				rd.pPosition = &tmp->_vPos;
				if( tmp->_bSmall ){
					rd.fRadius = g_fvalue1[m_hParent->GetLevel()];//100.0f;
					rd.nPower = g_nvalue[m_hParent->GetLevel()];//130;
					rd.nPushY = g_nPushYsmall[m_hParent->GetLevel()]; 
					rd.nPushZ = g_nPushZsmall[m_hParent->GetLevel()];
				}
				else
				{
					rd.fRadius = g_fvalue3[m_hParent->GetLevel()];//250.0f;
					rd.nPower = g_nvalue2[m_hParent->GetLevel()];//200;
					rd.nPushY = g_nPushYbig[m_hParent->GetLevel()]; 
					rd.nPushZ = g_nPushZbig[m_hParent->GetLevel()];
				}
				rd.nTeam = m_hParent->GetTeam();
				rd.nNotGenOrb = 0;
				rd.nNotLookAtHitUnit = 1;
				rd.WeaponType = WT_TYURRU; 
				rd.SoundAttackType = ATT_BIG;
				rd.nHitRemainFrame = 80;
				//CFcWorld::GetInstance().GiveRangeDamage( rd );
				rd.nGuardBreak = 50;
				CFcWorld::GetInstance().GiveRangeDamageCustom( rd, true, 0.7f, 0.8f);
			}
			g_BsKernel.UpdateObject( tmp->_index , &tmpMat );
		}
		
		iter++;
	}	
}




/*---------------------------------------------------------------------------------
-
-			CFcFXColumnOfWater::TickCount()
-					; 틱 카운터.
-					  
-
---------------------------------------------------------------------------------*/
void CFcFXColumnOfWater::TickCount( int &p_nTick )
{
	p_nTick++;
	std::list<ColumnWaterObj*>::iterator iter;

	for( iter = m_listFxObj.begin() ; iter != m_listFxObj.end() ; )
	{
		ColumnWaterObj* tmp = *iter;
		tmp->_nTick++;
		iter++;
	}
}





/*---------------------------------------------------------------------------------
-
-			CFcFXColumnOfWater::Process()
-					; 매 프레임 값을 변경한다.
-					  
-
---------------------------------------------------------------------------------*/
void CFcFXColumnOfWater::Process()
{
	static int tick = 0;

	if( m_state == END ) return;

	if( GetLife() == -99 )
	{
		tick = 0;
	}
	else
	{
		
		if( tick == GetLife() )
		{
			m_state = ENDING;
			tick = 0;
		}

	}

	TickCount( tick );
	
}




/*---------------------------------------------------------------------------------
-
-			CFcFXColumnOfWater::PreRender()
-					; .
-					  
-
---------------------------------------------------------------------------------*/
void CFcFXColumnOfWater::PreRender()
{

}




/*---------------------------------------------------------------------------------
-
-			CFcFXColumnOfWater::Render()
-					; .
-					  
-
---------------------------------------------------------------------------------*/
void CFcFXColumnOfWater::Render(C3DDevice *pDevice)
{
}





struct sInfo
{
	bool _bSmall;
	int _nLevel;
	int _nDelayTick;
};
/*---------------------------------------------------------------------------------
-
-			CFcFXColumnOfWater::ProcessMessage()
-					; 메세지 처리.
-					  
-
---------------------------------------------------------------------------------*/
int CFcFXColumnOfWater::ProcessMessage(int nCode,DWORD dwParam1/*=0*/, DWORD dwParam2/*=0*/,DWORD dwParam3/*=0*/)
{
	switch(nCode)
	{
	case FX_INIT_OBJECT:
		{
			m_hParent = *(GameObjHandle*)dwParam2;
			Initialize((int)dwParam1);
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
			Finalize();
			m_state = END;
		}
		return 1;

	case FX_UPDATE_OBJECT:
		{
			m_state = PLAY;
			D3DXVECTOR3 tmp = *(D3DXVECTOR3*)dwParam1;

			bool bSmall = false;
			int DelayTick = 0;
			if( dwParam3 )
			{
				sInfo tmp = *(sInfo*)dwParam3;
				bSmall = tmp._bSmall;
				SetParentLevel(tmp._nLevel);
				DelayTick = tmp._nDelayTick;
			}
			CreateNormalWaterObj( tmp, (int)dwParam2, bSmall, DelayTick );

		}
		return 1;

	}

	if( CFcFXBase::ProcessMessage( nCode, dwParam1, dwParam2, dwParam3) )
		return 1;

	return 0;
}