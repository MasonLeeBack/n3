#include "stdafx.h"
#include "FcFXMeteo.h"
#include "BsKernel.h"
#include "CrossVector.h"
#include "FcUtil.h"
#include "FcWorld.h"
#include "BsFXObject.h"

#include "BsHFWorld.h"
#include "FcHeroObject.h"
#include "FcFXManager.h"
#include "..\\Data\\FXList.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG

class CFcGameObject;
typedef CSmartPtr<CFcGameObject> GameObjHandle;



/*---------------------------------------------------------------------------------
-
-			CFcFXMeteor::CFcFXMeteor()
-					; 생성자.
-					  
-
---------------------------------------------------------------------------------*/
CFcFXMeteor::CFcFXMeteor()
{
	ZeroMemory( m_nPGId, sizeof(m_nPGId) );
	ZeroMemory( m_pFXObject, sizeof(CBsFXObject*)*4 );

	m_fScale[0] = 1.0f;
	m_fScale[1] = 1.0f;
	m_fVel = 0.f;
	m_fFXScale = 6.0f;

	m_nPiceOfMeteor = -1;
	m_nSavedDelTimer = 100;
	m_fIntensity = 0.0f;
#ifndef _LTCG
	SetFxRtti(FX_TYPE_METEOR);
#endif //_LTCG
}




/*---------------------------------------------------------------------------------
-
-			CFcFXMeteor::~CFcFXMeteor()
-					; 생성자.
-					  
-
---------------------------------------------------------------------------------*/
CFcFXMeteor::~CFcFXMeteor()
{
	ReleaseDeviceData();
}




/*---------------------------------------------------------------------------------
-
-			CFcFXMeteor::Initialize()
-					; 생성자.
-					  
-
---------------------------------------------------------------------------------*/
void	CFcFXMeteor::Initialize(float fMaxScale)
{
	CFcFXBase::Initialize();
	
	m_dwBlendValue[0] 			= D3DBLENDOP_ADD;
	m_dwBlendValue[1] 			= D3DBLEND_SRCALPHA;
	m_dwBlendValue[2] 			= D3DBLEND_INVSRCALPHA;
	m_fTrailValue[0]			= 60.0f;
	m_fTrailValue[1]			= 10.0f;
	m_fTrailValue[2]			= 130.0f;
	m_fPiceOfMeteorValue[0]		= 50.0f;
	m_fPiceOfMeteorValue[1]		= 200.0f;
	m_fPiceOfMeteorValue[2]		= 3.0f;
	m_fPiceOfMeteorValue[3]		= 30.0f;
	m_fPiceOfMeteorValue[4]		= 97.0f;
	m_fIntensity = 0.0f;

	m_nPGId[0] = FX_METEOR_EXP_004;
	m_nPGId[1] = FX_METEOR_EXP_001;
	m_nPGId[2] = -1;//FX_METEOR_EXP_002;
	m_nPGId[3] = -1;//FX_METEOR_EXP_003;

	//Tail
	m_pFXObject[0] = new CBsFXObject();
	m_pFXObject[0]->Initialize( g_BsKernel.GetFXTemplatePtr( m_nPGId[0] ) );
	m_pFXObject[0]->SetState( CBsFXObject::PLAY ,0 );
	m_pFXObject[0]->Show(true);

	// TODO	:	아래 코드 InitDeviceData()에서 여기로 이동 by jeremy
	m_nPiceOfMeteor = g_pFcFXManager->Create(FX_TYPE_PIECEOFMETEOR);
}


/*---------------------------------------------------------------------------------
-
-			CFcFXMeteor::InitDeviceData()
-					; 생성자.
-					  
-
---------------------------------------------------------------------------------*/
void	CFcFXMeteor::InitDeviceData() 
{
	

}




/*---------------------------------------------------------------------------------
-
-			CFcFXMeteor::ReleaseDeviceData()
-					; 생성자.
-					  
-
---------------------------------------------------------------------------------*/
void	CFcFXMeteor::ReleaseDeviceData() 
{
	for(int ii = 0; ii < 4; ++ii)
	{
		if(m_pFXObject[ii] )
		{
			m_pFXObject[ii]->SetState(CBsFXObject::STOP);
			delete m_pFXObject[ii];
			m_pFXObject[ii] = NULL;
		}
	}

}




/*---------------------------------------------------------------------------------
-
-			CFcFXMeteor::Process()
-					; 생성자.
-					  
-
---------------------------------------------------------------------------------*/
void	CFcFXMeteor::Process()
{ 
	if( m_state == ENDING )
	{
		m_nDelTimer--;
		if( !m_nDelTimer)
		{
			m_state = END;
			m_DeviceDataState=FXDS_RELEASEREADY;
			return;
		}
	}
	if( m_state == END ) return;

	m_Cross.RotatePitch(20); // 회전속도
	D3DXMatrixScaling(&m_ScaleMat, m_fScale[1], m_fScale[1], m_fScale[1]);
	
	float fheight;
	float	fMapXSize, fMapYSize;
	g_FcWorld.GetMapSize( fMapXSize, fMapYSize );
	if (m_Cross.m_PosVector.x >= 0.0f && m_Cross.m_PosVector.x < fMapXSize && 
		m_Cross.m_PosVector.z >= 0.0f && m_Cross.m_PosVector.z < fMapYSize)
		fheight = g_FcWorld.GetLandHeight( m_Cross.m_PosVector.x, m_Cross.m_PosVector.z );
	else
		fheight = 0.0f;

	// 파편 생성 빈도 
	if( m_Cross.m_PosVector.y > fheight+100.0f && RandomNumberInRange( 0, 100) > (int)m_fPiceOfMeteorValue[4] )
		GenBaby();

	static D3DXMATRIX ObjectMat;
	static D3DXMATRIX scaleMat;
	static CCrossVector cross;
	cross.Reset();
	cross.m_YVector = D3DXVECTOR3( 0.0f, 1.0f, 0.0f );
	D3DXVec3Normalize(&m_vDir, &m_vDir);
	cross.m_ZVector = m_vDir;
	cross.m_PosVector = m_Cross.m_PosVector;
	cross.UpdateVectors();
	static float fFXScale;
	fFXScale = m_fScale[0]; //Tail FX scale
	D3DXMatrixScaling(&scaleMat, fFXScale, fFXScale, fFXScale);
	D3DXMatrixMultiply(&scaleMat, &scaleMat, cross);

	//Tail Update

	if(m_state == PLAY || m_state == ENDING) 
	{
		if(m_pFXObject[0])
		{
			m_pFXObject[0]->SetObjectMatrix(&scaleMat);
			m_pFXObject[0]->Show(true);
			m_pFXObject[0]->InitRender(0.f);
		}
	}

	for(int ii = 1; ii < 4 ; ++ii)
	{
		if(m_pFXObject[ii])
		{
			//m_pFXObject[ii]->GetShow();
			m_pFXObject[ii]->Show(true);
			m_pFXObject[ii]->InitRender(0.f);
		}
	}

}




/*---------------------------------------------------------------------------------
-
-			CFcFXMeteor::PreRender()
-					; 생성자.
-					  
-
---------------------------------------------------------------------------------*/
void	CFcFXMeteor::PreRender()
{
	CFcFXBase::PreRender();
}




/*---------------------------------------------------------------------------------
-
-			CFcFXMeteor::Render()
-					; 생성자.
-					  
-
---------------------------------------------------------------------------------*/
void	CFcFXMeteor::Render(C3DDevice *pDevice)
{
	CBsCamera* pCamera = g_BsKernel.GetActiveCamera();
	D3DVIEWPORT9 vp;
	pDevice->GetViewport(&vp);
	D3DXVECTOR3 vScreen;
	D3DXVECTOR3 vMeteorPos = m_Cross.m_PosVector;
	pCamera->GetScreenPos(&vMeteorPos, &vScreen);
	D3DXVECTOR3 vCamPos = D3DXVECTOR3(pCamera->GetObjectMatrix()->_41, pCamera->GetObjectMatrix()->_42, pCamera->GetObjectMatrix()->_43);			
	D3DXVECTOR3 vDist = vMeteorPos - vCamPos;

	float fDist = D3DXVec3Length(&vDist);
	
	if( vScreen.x >= 0 && vScreen.x <= vp.Width 
		&& vScreen.y >= 0 && vScreen.y <= vp.Height 
		&& vScreen.z <= 1 && vScreen.z >= 0 && fDist <= m_fSceneValue[0])
		m_fIntensity = (1.0f - fDist/m_fSceneValue[0]) * m_fSceneValue[1];
	else
		m_fIntensity = 0.0f;
}




/*---------------------------------------------------------------------------------
-
-			CFcFXMeteor::ProcessMessage()
-					; 생성자.
-					  
-
---------------------------------------------------------------------------------*/
int	CFcFXMeteor::ProcessMessage(int nCode,DWORD dwParam1/*=0*/, DWORD dwParam2/*=0*/,DWORD dwParam3/*=0*/)
{
	switch(nCode)
	{
	case FX_INIT_OBJECT:
		memcpy( m_fScale, (DWORD*)dwParam1, sizeof(m_fScale));
		Initialize( m_fScale[1] );
		if(dwParam3) 
			m_nSavedDelTimer = *(int*)dwParam3;
		if(dwParam2) 
			m_fFXScale = *(float*)dwParam2;

		return 1;
	case FX_ADD_VALUE:	
		if( dwParam1 != NULL && dwParam2 != NULL )
		{
			memcpy( m_fTrailValue, (DWORD*)dwParam1, sizeof(m_fTrailValue));
			memcpy( m_fPiceOfMeteorValue, (DWORD*)dwParam2, sizeof(m_fPiceOfMeteorValue));
		}
		break;
	case FX_ADD_VALUE2:
		{
			memcpy( m_fPiceOfMeteorDamage, (DWORD*)dwParam1, sizeof(m_fPiceOfMeteorDamage));
			memcpy( m_fPiceOfMeteorRadius, (DWORD*)dwParam2, sizeof(m_fPiceOfMeteorRadius));
			memcpy( m_fSceneValue, (DWORD*)dwParam3, sizeof(m_fSceneValue));
		}
		break;
	case FX_UPDATE_OBJECT:
		SetPos( (D3DXVECTOR3*)dwParam1, (D3DXVECTOR3*)dwParam2, (float*)dwParam3 );
		return 1;

	case FX_PLAY_OBJECT:
		m_state = PLAY;
		return 1;

	case FX_DELETE_OBJECT:
		
		if(dwParam1)
		{
			m_state = ENDING;
#ifndef _LTCG
			if( dwParam1 )
			{
				BsAssert( GetFxRtti() == (int)dwParam1 );
			}
#endif //_LTCG
			m_nDelTimer = m_nSavedDelTimer;
			g_pFcFXManager->SendMessage(m_nPiceOfMeteor , FX_SET_PARENT_STATE, m_state);
		}
		else
		{
#ifndef _LTCG
			if( dwParam1 )
			{
				BsAssert( GetFxRtti() == (int)dwParam1 );
			}
#endif //_LTCG
			m_nDelTimer = m_nSavedDelTimer;
			m_state = ENDING;
			//m_DeviceDataState=FXDS_RELEASEREADY;
			g_pFcFXManager->SendMessage(m_nPiceOfMeteor , FX_SET_PARENT_STATE, m_state);
		}
		
		return 1;

	case FX_GET_INTENSITY:
		{
			*((float*)dwParam1) = m_fIntensity;
		}
		break;
	case FX_FIRE_EXPLODE:
		Explode();
		return 1;
	case FX_FORCED_DELETE:
		{
			g_pFcFXManager->SendMessage(m_nPiceOfMeteor , FX_FORCED_DELETE);
			m_state = END;
			m_DeviceDataState=FXDS_RELEASEREADY;
		}
		return 1;
	}
	
	if ( CFcFXBase::ProcessMessage( nCode, dwParam1, dwParam2, dwParam3) )
		return 1;

	return 0;
}




/*---------------------------------------------------------------------------------
-
-			CFcFXMeteor::SetPos()
-					; 생성자.
-					  
-
---------------------------------------------------------------------------------*/
void CFcFXMeteor::SetPos(D3DXVECTOR3* pvDir, D3DXVECTOR3* pvPos, float* fVel)
{
	m_vDir = *pvDir;
	m_Cross.m_PosVector  = *pvPos;
	if(fVel)
		m_fVel = *fVel;
}




/*---------------------------------------------------------------------------------
-
-			CFcFXMeteor::Explode()
-					; 생성자.
-					  
-
---------------------------------------------------------------------------------*/
void CFcFXMeteor::Explode()
{

	static D3DXMATRIX scaleMat;
	static CCrossVector cross;
	cross.Reset();
	cross.m_ZVector = m_vDir;
	cross.m_ZVector.y = 0.f;
	D3DXVec3Normalize(&cross.m_ZVector, &cross.m_ZVector);
	cross.m_PosVector = m_Cross.m_PosVector;
	cross.UpdateVectors();
	D3DXMatrixScaling(&scaleMat, m_fFXScale, m_fFXScale, m_fFXScale);
	D3DXMatrixMultiply(&scaleMat, &scaleMat, cross);


	//Explode
	for(int ii = 1; ii < 4 ; ++ii)
	{
		if(m_nPGId[ii] != -1)
		{
			m_pFXObject[ii] = new CBsFXObject();
			m_pFXObject[ii]->Initialize( g_BsKernel.GetFXTemplatePtr( m_nPGId[ii] ) );
			m_pFXObject[ii]->SetState( CBsFXObject::PLAY ,0 );
			m_pFXObject[ii]->Show(true);
			m_pFXObject[ii]->SetObjectMatrix(&scaleMat);
			m_pFXObject[ii]->SetObjectMatrixByRender(&scaleMat);
		}
	}
}




/*---------------------------------------------------------------------------------
-
-			CFcFXMeteor::GenBaby()
-					; 생성자.
-					  
-
---------------------------------------------------------------------------------*/
void  CFcFXMeteor::GenBaby()
{
	g_pFcFXManager->SendMessage(m_nPiceOfMeteor, FX_INIT_OBJECT, (DWORD)&m_Cross.m_PosVector, (DWORD)&m_vDir, (DWORD)&m_fVel );  // life, length, epsilon
	g_pFcFXManager->SendMessage(m_nPiceOfMeteor, FX_ADD_VALUE , (DWORD)m_dwBlendValue, (DWORD)m_fTrailValue, (DWORD)m_fPiceOfMeteorValue );
	g_pFcFXManager->SendMessage(m_nPiceOfMeteor, FX_ADD_VALUE2 , (DWORD)m_fPiceOfMeteorDamage, (DWORD)m_fPiceOfMeteorRadius );
}

//***********************************************************************************************
//***********************************************************************************************










int CPieceOfMeteor::m_nCount = 0;


/*---------------------------------------------------------------------------------
-
-			CPieceOfMeteor::CPieceOfMeteor()
-					; 생성자.
-					  
-
---------------------------------------------------------------------------------*/
CPieceOfMeteor::CPieceOfMeteor()
{
	m_nParentState = -1;
	m_ptmpTrail = NULL;
	//m_nCount = 0;
}




/*---------------------------------------------------------------------------------
-
-			CPieceOfMeteor::Initialize()
-					; 초기화루틴. 메테오조각의 속성값을 임시버퍼에 저장하고,
-					  후에 stl vector에 집어 넣는다. 바닦 폭파물 생성한다.
-
---------------------------------------------------------------------------------*/
void CPieceOfMeteor::Initialize( D3DXVECTOR3 p_vMeteorPos, D3DXVECTOR3 p_vMeteorDir, float p_fMeteorSpd )
{
	m_ptmpTrail = new STrail;
	if( !m_ptmpTrail ) return;
	m_ptmpTrail->_vPos		= p_vMeteorPos;
	m_ptmpTrail->_vDir		= p_vMeteorDir;
	m_ptmpTrail->_Speed		= p_fMeteorSpd;	
}





/*---------------------------------------------------------------------------------
-
-			CPieceOfMeteor::Render()
-					; 생성자.
-					  
-
---------------------------------------------------------------------------------*/
void CPieceOfMeteor::Render(C3DDevice *pDevice)
{
	
}




/*---------------------------------------------------------------------------------
-
-			CPieceOfMeteor::ProcessMessage()
-					; 생성자.
-					  
-
---------------------------------------------------------------------------------*/
int CPieceOfMeteor::ProcessMessage(int nCode,DWORD dwParam1/*=0*/, DWORD dwParam2/*=0*/,DWORD dwParam3/*=0*/)
{
	if ( CFcFXBase::ProcessMessage( nCode, dwParam1, dwParam2, dwParam3) )
		return 1;

	switch(nCode)
	{
	case FX_INIT_OBJECT:
		Initialize( *(D3DXVECTOR3*)dwParam1, *(D3DXVECTOR3*)dwParam2, *(float*)dwParam3);
		return 1;

	case FX_ADD_VALUE:
		{
		
			m_state = PLAY;
			DWORD	dwBlendValue[3];
			memset( dwBlendValue, 0, sizeof(dwBlendValue));
			memcpy( dwBlendValue, (DWORD*)dwParam1, sizeof(dwBlendValue));
			float fTrailValue[3];
			memset( fTrailValue, 0, sizeof(fTrailValue));
			memcpy( fTrailValue, (DWORD*)dwParam2, sizeof(fTrailValue));
			float fPiceOfMetorValue[5];
			memset( fPiceOfMetorValue, 0, sizeof(fPiceOfMetorValue));
			memcpy( fPiceOfMetorValue, (DWORD*)dwParam3, sizeof(fPiceOfMetorValue));
			m_ptmpTrail->_fAniSpd = fPiceOfMetorValue[3];

			// Trail Create
			char szFile[128];
			sprintf(szFile, "%s","smoke_vol_256_b.dds" );
			
			m_ptmpTrail->_nTrail = g_pFcFXManager->Create(FX_TYPE_LINETRAIL);
			g_pFcFXManager->SendMessage(m_ptmpTrail->_nTrail, FX_INIT_OBJECT, (DWORD)fTrailValue[0], (DWORD)&fTrailValue[1], (DWORD)&fTrailValue[2] );  // life, length, epsilon
			g_pFcFXManager->SendMessage(m_ptmpTrail->_nTrail, FX_SET_BLENDOP, dwBlendValue[0] );
			g_pFcFXManager->SendMessage(m_ptmpTrail->_nTrail, FX_SET_SRCBLEND, dwBlendValue[1] );
			g_pFcFXManager->SendMessage(m_ptmpTrail->_nTrail, FX_SET_DSTBLEND, dwBlendValue[2] );
			g_pFcFXManager->SendMessage(m_ptmpTrail->_nTrail, FX_SET_VOLUME_TEXTURE,(DWORD)szFile, (DWORD)m_ptmpTrail->_fAniSpd);
			g_pFcFXManager->SendMessage(m_ptmpTrail->_nTrail, FX_PLAY_OBJECT);
			

			D3DXVECTOR3 tmpDirVec;
			tmpDirVec = D3DXVECTOR3(RandomNumberInRange(-1.0f, 1.0f), RandomNumberInRange(-1.0f, 1.0f), RandomNumberInRange(-1.0f, 1.0f) );
			D3DXVec3Normalize(&tmpDirVec, &tmpDirVec);
			m_tmpDirVec = tmpDirVec;
			tmpDirVec *= RandomNumberInRange(500.0f, 1500.0f);
			m_ptmpTrail->_vPos		+= tmpDirVec;
			m_ptmpTrail->_vDir += (m_tmpDirVec*fPiceOfMetorValue[0]);
			D3DXVec3Normalize(&m_ptmpTrail->_vDir, &m_ptmpTrail->_vDir);
			m_ptmpTrail->_Speed		*= fPiceOfMetorValue[1];	

			m_fGravity = fPiceOfMetorValue[2];
			m_ptmpTrail->_vDir *= m_ptmpTrail->_Speed;

			
		}
		return 1;
	
	case FX_ADD_VALUE2:
		{
			memcpy( m_fPiceOfMeteorDamage, (DWORD*)dwParam1, sizeof(m_fPiceOfMeteorDamage));
			memcpy( m_fPiceOfMeteorRadius, (DWORD*)dwParam2, sizeof(m_fPiceOfMeteorRadius));

			m_tmpDirVec *= RandomNumberInRange(m_fPiceOfMeteorRadius[0], m_fPiceOfMeteorRadius[1]);
			m_ptmpTrail->_vPos		+= m_tmpDirVec;

			AddTrail(m_ptmpTrail);
			m_ptmpTrail = NULL;
		}
		return 1;

	case FX_SET_PARENT_STATE:
		m_nParentState = (int)dwParam1;
		m_state = PLAY;
		break;
	case FX_FORCED_DELETE:
		{
			Clear();
			m_state = ENDING;
		}
		return 1;
	}
	return 0;
}




/*---------------------------------------------------------------------------------
-
-			CPieceOfMeteor::AddTrail()
-					; 생성자.
-					  
-
---------------------------------------------------------------------------------*/
void CPieceOfMeteor::AddTrail( STrail* p_pTrail )
{
	m_listTrail.push_back(p_pTrail);
}




/*---------------------------------------------------------------------------------
-
-			CPieceOfMeteor::AddShock()
-					; 생성자.
-					  
-
---------------------------------------------------------------------------------*/
void CPieceOfMeteor::AddShock( SShock* p_pShock )
{
	m_listShock.push_back(p_pShock);
}




/*---------------------------------------------------------------------------------
-
-			CPieceOfMeteor::DeleteTrail()
-					; 생성자.
-					  
-
---------------------------------------------------------------------------------*/
void CPieceOfMeteor::DeleteTrail( STrail* p_pTrail )
{
	SAFE_DELETE_FX( p_pTrail->_nTrail,FX_TYPE_LINETRAIL);
	delete p_pTrail;
}




/*---------------------------------------------------------------------------------
-
-			CPieceOfMeteor::DeleteShock()
-					; 생성자.
-					  
-
---------------------------------------------------------------------------------*/
void CPieceOfMeteor::DeleteShock( SShock* p_pShock )
{
	/*char buffer[256];
	sprintf(buffer, " %d Deleted!!!! count : %d \n", p_pShock->_nHandle, --m_nCount);
	OutputDebugString(buffer);*/

	g_BsKernel.SetFXObjectState( p_pShock->_nHandle,CBsFXObject::STOP);
	g_BsKernel.DeleteObject( p_pShock->_nHandle );
	delete p_pShock;

	
}




/*---------------------------------------------------------------------------------
-
-			CPieceOfMeteor::Clear()
-					; 생성자.
-					  
-
---------------------------------------------------------------------------------*/
void CPieceOfMeteor::Clear()
{
	std::list<STrail*>::iterator iter;
	if( !m_listTrail.empty() )
	{
		for( iter = m_listTrail.begin() ; iter != m_listTrail.end() ;  )
		{
			STrail *tmp = *iter;
			iter = m_listTrail.erase(iter);
			DeleteTrail(tmp);
		}
		m_listTrail.clear();
	}

	std::list<SShock*>::iterator iter2;
	if( !m_listShock.empty() )
	{
		for( iter2 = m_listShock.begin() ; iter2 != m_listShock.end() ;  )
		{
			SShock *tmp = *iter2;
			iter2 = m_listShock.erase(iter2);
			DeleteShock(tmp);
		}
		m_listShock.clear();
	}
}




/*---------------------------------------------------------------------------------
-
-			CPieceOfMeteor::~CPieceOfMeteor()
-					; 생성자.
-					  
-
---------------------------------------------------------------------------------*/
CPieceOfMeteor::~CPieceOfMeteor()
{
	Clear();
}




/*---------------------------------------------------------------------------------
-
-			CPieceOfMeteor::InitDeviceData()
-					; 생성자.
-					  
-
---------------------------------------------------------------------------------*/
void CPieceOfMeteor::InitDeviceData()
{
}




/*---------------------------------------------------------------------------------
-
-			CPieceOfMeteor::PreRender()
-					; 생성자.
-					  
-
---------------------------------------------------------------------------------*/
void CPieceOfMeteor::PreRender()
{
}






/*---------------------------------------------------------------------------------
-
-			CPieceOfMeteor::ProcessShock()
-					; 생성자.
-					  
-
---------------------------------------------------------------------------------*/
void CPieceOfMeteor::ProcessShock()
{
	std::list<SShock*>::iterator iter;
	if( !m_listShock.empty() )
	{
		D3DXMATRIX	ObjectMat;
		for( iter = m_listShock.begin() ; iter != m_listShock.end() ; )
		{
			SShock*	tmp = NULL;
			tmp = (SShock*)*iter;
			CBsFXObject::STATE State = g_BsKernel.GetFXObjectState( tmp->_nHandle );
			if( State == CBsFXObject::STOP )
			{
				iter = m_listShock.erase(iter);
				DeleteShock(tmp);
			}
			else
			{
				D3DXMatrixIdentity(&ObjectMat);
				D3DXMATRIX tmpMat;
				D3DXMatrixScaling(&tmpMat, tmp->_ScaleRot.x, tmp->_ScaleRot.y, tmp->_ScaleRot.z);
				D3DXMatrixMultiply(&ObjectMat, &ObjectMat, &tmpMat);
				ObjectMat._41 = tmp->_vPos.x;
				ObjectMat._42 = tmp->_vPos.y;
				ObjectMat._43 = tmp->_vPos.z;
				//ObjectMat._11 *= tmp->_ScaleRot.x; ObjectMat._22 *= tmp->_ScaleRot.y; ObjectMat._33 *= tmp->_ScaleRot.z;
				g_BsKernel.UpdateObject(tmp->_nHandle, &ObjectMat);
				++iter;
			}
		}// for
	}// if
}





/*---------------------------------------------------------------------------------
-
-			CPieceOfMeteor::Process()
-					; 생성자.
-					  
-
---------------------------------------------------------------------------------*/
void CPieceOfMeteor::Process()
{
	if( m_nParentState == ENDING && m_listTrail.empty() && m_listShock.empty() ) 
		m_state = ENDING;

	if( m_state == ENDING )
	{
		Clear();
		m_state = END;
		m_DeviceDataState=FXDS_RELEASEREADY;
		return;
	}
	else if( m_state == END ) return;

	std::list<STrail*>::iterator iter;
	if( !m_listTrail.empty() )
	{
		for( iter = m_listTrail.begin() ; iter != m_listTrail.end() ;  )
		{
			STrail*	tmp = NULL;
			tmp = (STrail*)*iter;
			float fheight2;
			float	fMapXSize, fMapYSize;
			g_FcWorld.GetMapSize( fMapXSize, fMapYSize );
			if (tmp->_vPos.x >= 0.0f && tmp->_vPos.x < fMapXSize && 
				tmp->_vPos.z >= 0.0f && tmp->_vPos.z < fMapYSize)
				fheight2 = g_FcWorld.GetLandHeight( tmp->_vPos.x, tmp->_vPos.z );
			else 
				fheight2 = 0.0f;

			if( (fheight2 > tmp->_vPos.y+600.0f) || ( tmp->_nLife > 150 ) )
			{
				iter = m_listTrail.erase(iter);
				/*g_pFcFXManager->SendMessage(  tmp->_nTrail , FX_DELETE_OBJECT );
				tmp->_nTrail = -1;*/
				SAFE_DELETE_FX( tmp->_nTrail,FX_TYPE_LINETRAIL);
				//Damage
				GameObjHandle m_hParent;
				m_hParent = CFcWorld::GetInstance().GetHeroHandle();
				_FC_RANGE_DAMAGE rd;
				rd.nType = HT_ABIL_FIRE;
				rd.Attacker = m_hParent;
				rd.pPosition = &tmp->_vPos;
				rd.fRadius = m_fPiceOfMeteorDamage[0];
				rd.nPower = (int)m_fPiceOfMeteorDamage[1];
				rd.nPushY = 30; 
				rd.nPushZ = 70;
				rd.nTeam  = -1;
				rd.nNotGenOrb = 1;
				rd.nNotLookAtHitUnit = 1;
				CFcWorld::GetInstance().GiveRangeDamage( rd,true,false );

				CFcWorld::GetInstance().GivePhysicsRange( m_hParent, &tmp->_vPos, m_fPiceOfMeteorDamage[0] , m_fPiceOfMeteorDamage[1], 1000000, PROP_BREAK_TYPE_SPECIAL_ATTACK );

				

				//Shock 생성
				if( fheight2 > tmp->_vPos.y+600.0f && m_listShock.size() <= 50 )
				{
					int random = Random(100);
					int nShock;
					if( random >= 30 )
					{
                        nShock = g_BsKernel.CreateFXObject(FX_PICEOFMETEOR_SHOCK2);//FX_PICEOFMETEOR_SHOCK2
						m_nCount++;
					}
					else if( random >= 15 )
					{
						nShock = g_BsKernel.CreateFXObject(FX_PICEOFMETEOR_SHOCK1);//FX_PICEOFMETEOR_SHOCK1
						m_nCount++;
					}
					else
					{
						nShock = g_BsKernel.CreateFXObject(FX_PICEOFMETEOR_SHOCK3);//FX_PICEOFMETEOR_SHOCK3
						m_nCount++;
					}
					g_BsKernel.SetFXObjectState( nShock,CBsFXObject::PLAY);
					SShock* tmpShock = NULL;
					tmpShock = new SShock;
					tmpShock->_nHandle = nShock;
					tmpShock->_vPos = tmp->_vPos;
					tmpShock->_vPos.y += 500.0f;
					tmpShock->_ScaleRot.x = 1.0f;
					tmpShock->_ScaleRot.y = RandomNumberInRange(1.0f, 2.0f);
					tmpShock->_ScaleRot.z = 1.0f;
					tmpShock->_ScaleRot.w = 1.0f;
					AddShock(tmpShock);


				}
				

				delete tmp;
				tmp = NULL;

			}
			else
			{
				++iter;
				if( NULL == tmp ) break;
				tmp->_vPos += tmp->_vDir;
				tmp->_vDir.y -= m_fGravity;
				tmp->_vPos += tmp->_vDir ;
				g_pFcFXManager->SendMessage(  tmp->_nTrail , FX_UPDATE_OBJECT,(DWORD)&tmp->_vPos );//,
				tmp->_nLife++;
			}
			
		}// for
	}// if
	ProcessShock();
}
//***********************************************************************************************
//***********************************************************************************************

