#include "StdAfx.h"
#include ".\fcfxgenerater.h"
#include "FcGameObject.h"
#include "FcFXManager.h"
#include "FcUtil.h"
#include "CrossVector.h"
#include "FcWorld.h"
#include "FcHeroObject.h"

/*---------------------------------------------------------------------------------
--
-			CFcFxGenerater::CFcFxGenerater()
-					; 持失切.
-					  
-
---------------------------------------------------------------------------------*/
CFcFxGenerater::CFcFxGenerater(void)
{
	SetGameObj(NULL);
	SetFxObjIndex(-1);
	SetFxIndex(-1);
	SetBTop(false);
	SetBMiddle(false);
	SetLoop(false);

#ifndef _LTCG
	SetFxRtti(FX_TYPE_GENERATER);
#endif //_LTCG
}





/*---------------------------------------------------------------------------------
--
-			CFcFxGenerater::~CFcFxGenerater()
-					; 持失切.
-					  
-
---------------------------------------------------------------------------------*/
CFcFxGenerater::~CFcFxGenerater(void)
{
	if( GetFxObjIndex() != -1 ) g_BsKernel.DeleteObject( GetFxObjIndex() );
}





/*---------------------------------------------------------------------------------
--
-			CFcFxGenerater::Process()
-					; 持失切.
-					  
-
---------------------------------------------------------------------------------*/
void CFcFxGenerater::Process()
{
	if( m_state == END || m_state == STOP ) return;

	if( GetGameObj() )
	{
		CCrossVector Cross = *GetGameObj()->GetCrossVector();
		Cross.UpdateVectors();
		if( GetFxObjIndex() == -1 )
		{
			SetFxObjIndex( g_BsKernel.CreateFXObject(GetFxIndex()) );
			if( GetLoop() )
                g_BsKernel.SetFXObjectState( GetFxObjIndex(), CBsFXObject::PLAY , -99);
			else
				g_BsKernel.SetFXObjectState( GetFxObjIndex(), CBsFXObject::PLAY );

		}


	}
}

void CFcFxGenerater::Update()
{
	if( GetGameObj() )
	{
		CCrossVector Cross = *GetGameObj()->GetCrossVector();
		Cross.UpdateVectors();
		if( GetFxObjIndex() != -1 )
		{
			if( GetBTop() )
			{
				float fHe = GetGameObj()->GetUnitHeight();
				Cross.m_PosVector.y += fHe + 50.0f;
				g_BsKernel.UpdateObject( GetFxObjIndex(), Cross );
			}
			else if( GetBMiddle() )
			{
				float fHe = GetGameObj()->GetUnitHeight();
				Cross.m_PosVector.y += fHe * 0.5f;
				g_BsKernel.UpdateObject( GetFxObjIndex(), Cross );
			}
			else
				g_BsKernel.UpdateObject( GetFxObjIndex(), Cross );
			
		}
	}
}




/*---------------------------------------------------------------------------------
-
-			CFcFxGenerater::PreRender()
-					; 持失切.
-					  
-
---------------------------------------------------------------------------------*/
void CFcFxGenerater::PreRender()
{
	CFcFXBase::PreRender();
}






/*---------------------------------------------------------------------------------
-
-			CFcFxGenerater::Render()
-					; 持失切.
-					  
-
---------------------------------------------------------------------------------*/
void CFcFxGenerater::Render(C3DDevice *pDevice)
{

}



/*---------------------------------------------------------------------------------
-
-			CFcFxGenerater::ProcessMessage()
-					; 持失切.
-					  
-
---------------------------------------------------------------------------------*/
int CFcFxGenerater::ProcessMessage(int nCode,DWORD dwParam1/*=0*/, DWORD dwParam2/*=0*/,DWORD dwParam3/*=0*/)
{
	if( CFcFXBase::ProcessMessage( nCode, dwParam1, dwParam2, dwParam3) )
		return 1;

	switch(nCode)
	{
	case FX_INIT_OBJECT:
		{
			SetGameObj((CFcGameObject*)dwParam1);
			SetFxIndex((int)dwParam2);
		}
		return 1;
	case FX_PLAY_OBJECT:
		{
			m_state = PLAY;
			if( dwParam1 == 1 )
				SetBTop(true);
			else if( dwParam1 == 2)
				SetBMiddle(true);

			if( dwParam2 )
				SetLoop(true);
			else
				SetLoop(false);
			
		}
		return 1;
	case FX_STOP_OBJECT:
		{
			m_state = STOP;
		}
		return 1;
	}

	return 0;
}
//**************************************************************************************
//**************************************************************************************

















/*---------------------------------------------------------------------------------
--
-			CFcFxThrowGenerater::CFcFxThrowGenerater()
-					; 持失切.
-					  
-
---------------------------------------------------------------------------------*/
CFcFxThrowGenerater::CFcFxThrowGenerater(void)
{
	memset( m_nState,		 0, sizeof(int)*3);
	memset( m_nFxIndex,		-1, sizeof(int)*3);
	memset( m_nFxObjIndex,	-1, sizeof(int)*3);
	m_nTrail = -1;
	m_fSpd = 10.0f;
#ifndef _LTCG
	SetFxRtti(FX_TYPE_THROWGENERATER);
#endif //_LTCG
}





/*---------------------------------------------------------------------------------
--
-			CFcFxThrowGenerater::~CFcFxThrowGenerater()
-					; 持失切.
-					  
-
---------------------------------------------------------------------------------*/
CFcFxThrowGenerater::~CFcFxThrowGenerater(void)
{
	for(int i = 0 ; i < 3 ; i++ )
        if( m_nFxObjIndex[i] != -1 ) g_BsKernel.DeleteObject( m_nFxObjIndex[i] );
	
	//if( m_nTrail != -1 ) g_pFcFXManager->SendMessage(  m_nTrail , FX_DELETE_OBJECT );
	SAFE_DELETE_FX(m_nTrail, FX_TYPE_LINETRAIL);
}





/*---------------------------------------------------------------------------------
--
-			CFcFxThrowGenerater::Process()
-					; 持失切.
-					  
-
---------------------------------------------------------------------------------*/
void CFcFxThrowGenerater::Process()
{
	if( m_state == END || m_state == STOP ) return;

	for(int i = 0 ; i < 3 ; i++ )
	{
		if( m_nState[i] )
		{
			//Blast 戚薙闘
			CCrossVector Cross;
			Cross.m_PosVector	= m_vPos[i];
			Cross.m_ZVector		= m_vDir[i];
			Cross.UpdateVectors();

			if( m_nFxObjIndex[i] == -1 )
			{
				m_nFxObjIndex[i] = g_BsKernel.CreateFXObject(m_nFxIndex[i]);
				g_BsKernel.SetFXObjectState( m_nFxObjIndex[i], CBsFXObject::PLAY );
			}
//			g_BsKernel.UpdateObject( m_nFxObjIndex[i], Cross );

			if( i == 1 ) 
			{
				m_vPos[i] += m_vDir[i] * m_fSpd;
				if( m_bTrail ) 
					g_pFcFXManager->SendMessage(  m_nTrail , FX_UPDATE_OBJECT,(DWORD)&Cross.m_PosVector );
			}//if( i == 1 ) 

		}//if( m_nState[i] )
	}//for(int i = 0 ; i < 3 ; i++ )
	
}

void CFcFxThrowGenerater::Update()
{
	if( m_state == END || m_state == STOP ) return;
	for(int i = 0 ; i < 3 ; i++ )
	{
		if( m_nState[i] )
		{
			//Blast 戚薙闘
			CCrossVector Cross;
			Cross.m_PosVector	= m_vPos[i];
			Cross.m_ZVector		= m_vDir[i];
			Cross.UpdateVectors();

			if( m_nFxObjIndex[i] != -1 )
			{
				g_BsKernel.UpdateObject( m_nFxObjIndex[i], Cross );
			}
		}//if( m_nState[i] )
	}//for(int i = 0 ; i < 3 ; i++ )
}




/*---------------------------------------------------------------------------------
-
-			CFcFxThrowGenerater::PreRender()
-					; 持失切.
-					  
-
---------------------------------------------------------------------------------*/
void CFcFxThrowGenerater::PreRender()
{
	CFcFXBase::PreRender();
}






/*---------------------------------------------------------------------------------
-
-			CFcFxThrowGenerater::Render()
-					; 持失切.
-					  
-
---------------------------------------------------------------------------------*/
void CFcFxThrowGenerater::Render(C3DDevice *pDevice)
{

}



/*---------------------------------------------------------------------------------
-
-			CFcFxThrowGenerater::ProcessMessage()
-					; 持失切.
-					  
-
---------------------------------------------------------------------------------*/
int CFcFxThrowGenerater::ProcessMessage(int nCode,DWORD dwParam1/*=0*/, DWORD dwParam2/*=0*/,DWORD dwParam3/*=0*/)
{
	if( CFcFXBase::ProcessMessage( nCode, dwParam1, dwParam2, dwParam3) )
		return 1;

	switch(nCode)
	{
	case FX_INIT_OBJECT:
		{
			if( dwParam1 != 0 ) m_nFxIndex[0] = (int)dwParam1;
			if( dwParam2 != 0 ) m_nFxIndex[1] = (int)dwParam2;
			if( dwParam3 != 0 ) m_nFxIndex[2] = (int)dwParam3;
		}
		return 1;
	case FX_PLAY_OBJECT:
		{
			m_state = PLAY;
			if(dwParam1) m_fSpd = (float)dwParam1;
		}
		return 1;
	case FX_STOP_OBJECT:
		{
			m_state = STOP;
		}
		return 1;
	case FX_PLAY_BECOMING:
		{
			if( m_nFxIndex[0] != -1 ) 
			{
				m_nState[0] = 1;
				m_vPos[0] = *(D3DXVECTOR3*)dwParam1;
				m_vDir[0] = *(D3DXVECTOR3*)dwParam2;
			}
		}
		return 1;
	case FX_PLAY_BLAST:
		{
			if( m_nFxIndex[1] != -1 ) 
			{
				m_nState[1] = 1;
				m_vPos[1] = *(D3DXVECTOR3*)dwParam1;
				m_vDir[1] = *(D3DXVECTOR3*)dwParam2;
				if( dwParam3 ) 
				{
					char szFile[128];
					sprintf(szFile, "%s","twist_N2.dds" );
					m_bTrail = true;
					m_nTrail = g_pFcFXManager->Create(FX_TYPE_LINETRAIL);
					float fValue[2] = { 1, 5 };
					g_pFcFXManager->SendMessage(m_nTrail, FX_INIT_OBJECT, 40, (DWORD)&fValue[0], (DWORD)&fValue[1] );  // life, length, epsilon
					g_pFcFXManager->SendMessage(m_nTrail, FX_SET_BLENDOP, D3DBLENDOP_ADD);
					g_pFcFXManager->SendMessage(m_nTrail, FX_SET_SRCBLEND, D3DBLEND_SRCALPHA );
					g_pFcFXManager->SendMessage(m_nTrail, FX_SET_DSTBLEND, D3DBLEND_ONE );
					g_pFcFXManager->SendMessage(m_nTrail, FX_SET_TEXTURE, (DWORD)szFile, 60, true);
					g_pFcFXManager->SendMessage(m_nTrail, FX_PLAY_OBJECT);

				}//if
			}//if
		}
		return 1;
	case FX_PLAY_EXPLOSION:
		{
			if( m_nFxIndex[2] != -1 ) 
			{
				m_nState[2] = 1;
				m_vPos[2] = *(D3DXVECTOR3*)dwParam1;
				m_vDir[2] = *(D3DXVECTOR3*)dwParam2;
			}
		}
		return 1;
	case FX_STOP_BECOMING:
		{
			m_nState[0] = 0;
		}
		return 1;
	case FX_STOP_BLAST:
		{
			m_nState[1] = 0;
		}
		return 1;
	case FX_STOP_EXPLOSION:
		{
			m_nState[2] = 0;
		}
		return 1;
	case FX_GET_POS:
		{
			//(D3DXVECTOR3*)dwParam1;
			*(D3DXVECTOR3*)dwParam1 = m_vPos[1];
		}
		return 1;
	}

	return 0;
}
//*******************************************************************************************



















/*---------------------------------------------------------------------------------
--
-			CFcFxListGenerater::CFcFxListGenerater()
-					; 持失切.
-					  
-
---------------------------------------------------------------------------------*/
CFcFxListGenerater::CFcFxListGenerater(void)
{
	SetFxIndex(-1);
	SetLastTick(-1);
	SetLoop(false);
	SetScale(1.0f);
	SetCreatePercent(0);
#ifndef _LTCG
	SetFxRtti(FX_TYPE_LISTGENERATER);
#endif //_LTCG
}





/*---------------------------------------------------------------------------------
--
-			CFcFxListGenerater::~CFcFxListGenerater()
-					; 社瑚切.
-					  
-
---------------------------------------------------------------------------------*/
CFcFxListGenerater::~CFcFxListGenerater(void)
{
	SetFxIndex(-1);
	SetLastTick(-1);
	if( !m_listObj.empty() )
	{
		std::list<ListObject*>::iterator iter;
		for( iter = m_listObj.begin() ; iter != m_listObj.end() ; )
		{
			ListObject* tmp = *iter;

			g_BsKernel.DeleteObject( tmp->_nFxIndex );
            iter = m_listObj.erase(iter);
			delete tmp;
		}
		m_listObj.clear();
	}
	SetLoop(false);
}





/*---------------------------------------------------------------------------------
--
-			CFcFxListGenerater::Process()
-					; .
-					  
-
---------------------------------------------------------------------------------*/
void CFcFxListGenerater::Process()
{
	if( !GetLoop() && m_state == ENDING)
	{
		if( m_listObj.empty() )
		{
			m_state = END;
			m_DeviceDataState=FXDS_RELEASEREADY;
			return;
		}
	}
	else if( GetLoop() && m_state == ENDING )
	{
		if( !m_listObj.empty() )
		{
			std::list<ListObject*>::iterator iter;
			for( iter = m_listObj.begin() ; iter != m_listObj.end() ; )
			{
				ListObject* tmp = *iter;

				g_BsKernel.DeleteObject( tmp->_nFxIndex );
				iter = m_listObj.erase(iter);
				delete tmp;
			}
			m_listObj.clear();
		}
		if( m_listObj.empty() )
		{
			m_state = END;
			m_DeviceDataState=FXDS_RELEASEREADY;
			return;
		}
	}
	if( m_state == END ) return;

	if( !m_listObj.empty() )
	{
		std::list<ListObject*>::iterator iter;
		for( iter = m_listObj.begin() ; iter != m_listObj.end() ; )
		{
			ListObject* tmp = *iter;

			if( GetLoop() )
			{
				iter++;
			}
			else
			{
				if( tmp->_nTick < GetLastTick() )
				{

					tmp->_nTick++;
					iter++;
				}
				else
				{
					iter = m_listObj.erase(iter);
					g_BsKernel.DeleteObject( tmp->_nFxIndex );
					delete tmp;
				}
			}

		}
	}

	//Update();
}





/*---------------------------------------------------------------------------------
--
-			CFcFxListGenerater::Process()
-					; .
-					  
-
---------------------------------------------------------------------------------*/
void CFcFxListGenerater::Update()
{
	D3DXVECTOR3 vHeroPos = g_FcWorld.GetHeroHandle()->GetPos();
	vHeroPos.y = 0.0f;
	if( !m_listObj.empty() )
	{
		std::list<ListObject*>::iterator iter;
		for( iter = m_listObj.begin() ; iter != m_listObj.end() ; )
		{
			ListObject* tmp = *iter;
			
			if( GetLoop() )
			{
				CCrossVector cross;
				cross.m_PosVector = tmp->_vPos;
				cross.m_ZVector = tmp->_vDir;
				D3DXMATRIX mat;
				D3DXMatrixIdentity(&mat);
				mat._11 *= GetScale(),mat._22 *= GetScale(),mat._33 *= GetScale();
				cross.UpdateVectors();
				D3DXMatrixMultiply(&mat,&mat,cross);
				g_BsKernel.UpdateObject(tmp->_nFxIndex, &mat);
				iter++;
			}
			else
			{
				if( tmp->_nTick < GetLastTick() )
				{
					CCrossVector cross;
					cross.m_PosVector = tmp->_vPos;
					cross.m_ZVector = tmp->_vDir;
					cross.UpdateVectors();
					D3DXMATRIX mat;
					D3DXMatrixIdentity(&mat);
					mat._11 *= GetScale(),mat._22 *= GetScale(),mat._33 *= GetScale();
					D3DXMatrixMultiply(&mat,&mat,cross);
					g_BsKernel.UpdateObject(tmp->_nFxIndex, &mat);
					
				}
				iter++;
			}
			
		}
	}
}




/*---------------------------------------------------------------------------------
-
-			CFcFxGenerater::PreRender()
-					; 持失切.
-					  
-
---------------------------------------------------------------------------------*/
void CFcFxListGenerater::PreRender()
{
	CFcFXBase::PreRender();
}






/*---------------------------------------------------------------------------------
-
-			CFcFxListGenerater::Render()
-					; 持失切.
-					  
-
---------------------------------------------------------------------------------*/
void CFcFxListGenerater::Render(C3DDevice *pDevice)
{

}




/*---------------------------------------------------------------------------------
--
-			CFcFxListGenerater::CreateFxObj()
-					; .
-					  
-
---------------------------------------------------------------------------------*/
void CFcFxListGenerater::CreateFxObj( D3DXVECTOR3 p_vPos, D3DXVECTOR3 p_vDir )
{
	if( GetFxIndex() == -1 ) return;
	if( GetCreatePercent() != 0 )
	{
		static int nCreatePercent = 0;
		if( nCreatePercent >=  GetCreatePercent() )
		{
			nCreatePercent = 0;
		}
		else
		{
			nCreatePercent++;
			return;
		}
	}
	
	int num = m_listObj.size();
	if( GetLimitNum() > 0 && num > GetLimitNum() ) return;
	ListObject* tmp = new ListObject;
	tmp->_vPos = p_vPos;
	tmp->_vDir = p_vDir;
	tmp->_nTick = 0;
	tmp->_nFxIndex = g_BsKernel.CreateFXObject(GetFxIndex());
	if( GetLoop() )
		g_BsKernel.SetFXObjectState( tmp->_nFxIndex, CBsFXObject::PLAY, -99 );
	else
        g_BsKernel.SetFXObjectState( tmp->_nFxIndex, CBsFXObject::PLAY );
	m_listObj.push_back(tmp);
}





/*---------------------------------------------------------------------------------
-
-			CFcFxListGenerater::ProcessMessage()
-					; 持失切.
-					  
-
---------------------------------------------------------------------------------*/
int CFcFxListGenerater::ProcessMessage(int nCode,DWORD dwParam1/*=0*/, DWORD dwParam2/*=0*/,DWORD dwParam3/*=0*/)
{

	switch(nCode)
	{
	case FX_INIT_OBJECT:
		{
			SetFxIndex((int)dwParam1);
			SetLastTick((int)dwParam2);
			SetLimitNum((int)dwParam3);
		}
		return 1;
	case FX_PLAY_OBJECT:
		{
			m_state = PLAY;
			if( dwParam1 > 0 )
				SetLoop(true);
			if( dwParam2 > 0 )
				SetScale(float(dwParam2));
			if( dwParam3 > 0 )
				SetCreatePercent((int)dwParam3);
			else SetCreatePercent(0);
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
			D3DXVECTOR3 vPos = *(D3DXVECTOR3*)dwParam1;
			D3DXVECTOR3 vDir = *(D3DXVECTOR3*)dwParam2;
			CreateFxObj(vPos, vDir);
		}
		return 1;
	}

	if ( CFcFXBase::ProcessMessage( nCode, dwParam1, dwParam2, dwParam3) )
		return 1;

	return 0;
}
//**************************************************************************************
//**************************************************************************************









/*---------------------------------------------------------------------------------
--
-			CFcFxRandomGenerater::CFcFxRandomGenerater()
-					; 持失切.
-					  
-
---------------------------------------------------------------------------------*/
CFcFxRandomGenerater::CFcFxRandomGenerater(void)
{
	SetPercent(-1);
	SetFxIndex(-1);
	SetFxObjIndex(-1);
	SetLife(-1);
	SetLoop(false);
	SetRandomScale(false);
	SetRandomValue(1.0f);

#ifndef _LTCG
	SetFxRtti(FX_TYPE_RANDOMGENERATER);
#endif //_LTCG
}





/*---------------------------------------------------------------------------------
--
-			CFcFxRandomGenerater::~CFcFxRandomGenerater()
-					; 持失切.
-					  
-
---------------------------------------------------------------------------------*/
CFcFxRandomGenerater::~CFcFxRandomGenerater(void)
{
	if( GetFxObjIndex() != -1 ) g_BsKernel.DeleteObject( GetFxObjIndex() );
	SetPercent(-1);
	SetFxIndex(-1);
	SetFxObjIndex(-1);
	SetLife(-1);
	SetLoop(false);
}





/*---------------------------------------------------------------------------------
--
-			CFcFxRandomGenerater::Process()
-					; 持失切.
-					  
-
---------------------------------------------------------------------------------*/
void CFcFxRandomGenerater::Process()
{
	static int s_nTick = 0;

	if( m_state == ENDING )
	{
		s_nTick = 0;
		m_state = END;
		m_DeviceDataState=FXDS_RELEASEREADY;
	}
	if( m_state == END || m_state == STOP ) return;

	
	if( GetFxIndex() != -1 && GetFxObjIndex() == -1 )
	{
		SetFxObjIndex( g_BsKernel.CreateFXObject(GetFxIndex()) );
		g_BsKernel.SetFXObjectState( GetFxObjIndex(), CBsFXObject::PLAY );
	}
	else
	{
		if( Random( GetPercent() ) ==  1 )
			SetStart(true);
				
		if( GetStart() )
		{
			
			if( s_nTick == GetLife() )
			{
				s_nTick = 0;
				g_BsKernel.DeleteObject(GetFxObjIndex());
				SetFxObjIndex(-1);
				if( !GetLoop() )
					m_state = ENDING;
			}
			else
			{
				CCrossVector Cross;
				Cross.m_PosVector = GetPos();
				Cross.m_ZVector = GetDir();
				Cross.UpdateVectors();
				if( GetRandomScale() )
				{
					D3DXMATRIX tmp = *(D3DXMATRIX*)Cross;
					D3DXMATRIX tmp2;
					D3DXMatrixIdentity(&tmp2);
					tmp2._11 *= GetRandomValue(), tmp2._22 *= GetRandomValue(), tmp2._33 *= GetRandomValue();
					D3DXMatrixMultiply(&tmp, &tmp2, &tmp);
					g_BsKernel.UpdateObject(GetFxObjIndex(), &tmp);
				}
				else
				{
					g_BsKernel.UpdateObject(GetFxObjIndex(), Cross);
				}//if( GetRandomScale() )
				
			}
			
			s_nTick++;
		}
	}

}





/*---------------------------------------------------------------------------------
-
-			CFcFxRandomGenerater::PreRender()
-					; 持失切.
-					  
-
---------------------------------------------------------------------------------*/
void CFcFxRandomGenerater::PreRender()
{
	CFcFXBase::PreRender();
}






/*---------------------------------------------------------------------------------
-
-			CFcFxRandomGenerater::Render()
-					; 持失切.
-					  
-
---------------------------------------------------------------------------------*/
void CFcFxRandomGenerater::Render(C3DDevice *pDevice)
{

}



/*---------------------------------------------------------------------------------
-
-			CFcFxRandomGenerater::ProcessMessage()
-					; .
-					  
-
---------------------------------------------------------------------------------*/
int CFcFxRandomGenerater::ProcessMessage(int nCode,DWORD dwParam1/*=0*/, DWORD dwParam2/*=0*/,DWORD dwParam3/*=0*/)
{
	switch(nCode)
	{
	case FX_INIT_OBJECT: // fx, life, loop
		{
			SetFxIndex((int)dwParam1);
			SetLife((int)dwParam2);
			if(dwParam3) SetLoop(true);
			else SetLoop(false);
		}
		return 1;
	case FX_ADD_VALUE: // pos, dir, randomPercent
		{
			SetPos( *(D3DXVECTOR3*)dwParam1 );
			SetDir( *(D3DXVECTOR3*)dwParam2 );
			SetPercent((int)dwParam3);
			m_DeviceDataState = FXDS_INITREADY;
		}
		return 1;
	case FX_PLAY_OBJECT: 
		{
			m_state = PLAY;
			if( dwParam1 )
			{
				SetRandomScale(true);
				SetRandomValue( (float)RandomNumberInRange( (int)dwParam2, (int)dwParam3) );
			}
		}
		return 1;
	case FX_STOP_OBJECT:
		{
			m_state = STOP;
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
//**************************************************************************************
//**************************************************************************************