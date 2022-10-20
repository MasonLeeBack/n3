#include "StdAfx.h"
#include "BsKernel.h"

#include "FcInterfaceManager.h"
#include "spiritobject_temp.h"
#include "FcGameObject.h"
#include "FcHeroObject.h"

#include "FcFXManager.h"
#include "FcFXCommon.h"

#include "FcUtil.h"
#include "FcSoundManager.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG

/*---------------------------------------------------------------------------------
-
-			CSpritObject::CSpritObject()
-					; 생성자. 초기화
-					  
-
---------------------------------------------------------------------------------*/
CSpritObject::CSpritObject( CCrossVector *pCross )
	: CFcBaseObject( pCross )
{
	m_nEngineIndex = -1;
	m_nTrailIndex = -1;
	m_nHaleEffIndex = -1;
	m_bIsBig = false;
	m_nFxIndex = -1;
	m_DirVector = D3DXVECTOR3( 0.0f, 1.0f, 0.0f );
}




/*---------------------------------------------------------------------------------
-
-			CSpritObject::~CSpritObject()
-					; 소멸자.
-					  
-
---------------------------------------------------------------------------------*/
CSpritObject::~CSpritObject(void)
{
	if(m_nFxIndex != -1){
		g_BsKernel.DeleteParticleObject( m_nFxIndex );
		m_nFxIndex =- 1;
	}
	m_bIsBig = false;
}




/*---------------------------------------------------------------------------------
-
-			CSpritObject::Render()
-					; 머리와 꼬리를 그린다.
-					  
-
---------------------------------------------------------------------------------*/
bool CSpritObject::Render()
{
	if( m_nFxIndex != -1)
	{
		CCrossVector cross;
		cross.Reset();
		cross.m_ZVector = m_DirVector;
		D3DXVec3Normalize(&cross.m_ZVector, &cross.m_ZVector);;
		cross.m_PosVector = m_Position;
		cross.UpdateVectors();
		/*float fScale = (float)m_nAddOrb * 0.3f;
		if( fScale >= 3.5f ) fScale = 3.5f;
		D3DXMATRIX mat;
		mat = *(D3DXMATRIX*)cross;
		mat._11 = fScale, mat._22 = fScale, mat._33 = fScale;*/
		g_BsKernel.UpdateParticle( m_nFxIndex, cross );

		SAFE_FX_OPERATE( g_pFcFXManager->SendMessage(  m_nTrailIndex , FX_UPDATE_OBJECT,(DWORD)&m_Position ), m_nTrailIndex, FX_TYPE_LINETRAIL);
		
		return true;
	}
	return false;
}



/*---------------------------------------------------------------------------------
-
-			CSpritObject::Initialize()
-					; 초기화루틴 ( 머리 꼬리 및 흡수될때 나타날 이펙트 생성 )
-					  (m_nFxIndex : 오브 머리, m_nHaleEffIndex : 흡수될때 나타날 이펙트, m_nTrailIndex : 오브 꼬리).  
-	
---------------------------------------------------------------------------------*/
int CSpritObject::Initialize( int nParticleGroupIndex, D3DXVECTOR3 *pPosition, int nAddOrb,
	HeroObjHandle AbsorbTarget, int nGenerateOrbType )
{
	D3DCOLORVALUE	Color;
	int				nPreLoadTex = -1;

	if( nAddOrb > 10 )	m_bIsBig = true;
	m_Position = *pPosition;

	CCrossVector cross;
	cross.Reset();
	cross.m_ZVector = m_DirVector;
	D3DXVec3Normalize(&cross.m_ZVector, &cross.m_ZVector);;
	cross.m_PosVector = m_Position;
	cross.UpdateVectors();

	Color.r = 1.0f;
	Color.g = 1.0f;
	Color.b = 1.0f;
	Color.a = 1.0f;

	m_nAddOrb = nAddOrb;

	if( nAddOrb >= 400 )
	{
		m_nAddOrb = 100;
	}
	else if( nAddOrb >= 200 )
	{
		m_nAddOrb = 60;
	}
	else if ( nAddOrb >= 100 )
	{
		m_nAddOrb = 40;
	}
	else if ( nAddOrb >= 50 )
	{
		m_nAddOrb = 20;
	}
	else
	{
		m_nAddOrb = 6;
	}

	char szFile[128];
	m_AbsorbTarget = AbsorbTarget;

	BsAssert( m_nFxIndex == -1 );
	
	
	if( ( !m_AbsorbTarget->IsOrbSparkOn() ) || ( nGenerateOrbType == GENERATE_ORB_RED ) )	// 현재 영웅이 일반공격 상태이면 붉은 오브가 나타난다.
	{
		int n = -1;
		
		if( m_nAddOrb < 40 )
		{
			n = 91;
		}
		else if( m_nAddOrb < 100 )
		{
			n = 43;
		}
		else
		{
			n = 93;
		}
	
		m_nFxIndex = g_BsKernel.CreateParticleObject( n, true, true, (D3DXMATRIX*)cross, 1.0f, &Color, false ); 
		m_nOrbType = ORB_TYPE_NORMAL;
		Color.r = 1.0f;
		sprintf(szFile, "%s","twist_N.dds" );
	}
	else					// 현재 영웅이 특수기술 시전상태이면 파란 오브가 나타난다.
	{
		int n = -1;
		if( m_nAddOrb < 40 )
		{
			n = 92;
		}
		else if( m_nAddOrb < 100 )
		{
			n = 69;
		}
		else
		{
			n = 94;
		}

		m_nFxIndex = g_BsKernel.CreateParticleObject( n, true, true, (D3DXMATRIX*)cross, 1.0f , &Color, false ); 
		m_nOrbType = ORB_TYPE_TRUE;
		Color.r = 0.0f;
		sprintf(szFile, "%s","twist_N2.dds" );
	}
	
	m_nMode = SPIRIT_MODE_UP;
	m_fSpeed = RandomNumberInRange( 2, 3) * 10.0f + RandomNumberInRange( 5, 9);
	if( m_fSpeed >= 35.0f ) m_fSpeed = 35.0f;
	m_DirVector = D3DXVECTOR3( 0.0f, 1.0f, 0.0f );


	BsAssert( m_nTrailIndex == -1 );
	// 오브 꼬리 생성 및 초기화
	m_nTrailIndex = g_pFcFXManager->Create(FX_TYPE_LINETRAIL);

	float fValue[2] = { 1, 5 };
	if( m_bIsBig ) fValue[0] = 2;
	g_pFcFXManager->SendMessage(m_nTrailIndex, FX_INIT_OBJECT, 40, (DWORD)&fValue[0], (DWORD)&fValue[1] );  // life, length, epsilon
	g_pFcFXManager->SendMessage(m_nTrailIndex, FX_SET_BLENDOP, D3DBLENDOP_ADD);
	g_pFcFXManager->SendMessage(m_nTrailIndex, FX_SET_SRCBLEND, D3DBLEND_SRCALPHA );
	g_pFcFXManager->SendMessage(m_nTrailIndex, FX_SET_DSTBLEND, D3DBLEND_ONE );
	g_pFcFXManager->SendMessage(m_nTrailIndex, FX_SET_VOLUME_TEXTURE, (DWORD)szFile, 60, true);
	g_pFcFXManager->SendMessage(m_nTrailIndex, FX_PLAY_OBJECT);

	
	return 1;
}





/*---------------------------------------------------------------------------------
-
-			CSpritObject::Process()
-					; 혼 이동 및 흡수될때 이펙트를 그리고 오브흡수 처리 루틴도 함께 한다.
-					  
-
---------------------------------------------------------------------------------*/
void CSpritObject::Process()
{
	CCrossVector*	pCross;								
	float			fLength;							// 캐릭터와 혼과의 거리
	D3DXVECTOR3		TargetPos, Direction, CrossVec;		
	

	pCross = m_AbsorbTarget->GetCrossVector();
	TargetPos = pCross->m_PosVector;
	TargetPos.y += m_AbsorbTarget->GetUnitHeight() * 0.7f;
	
	/*
		상태에 따라 움직임 패턴을 다르게 한다. 
				SPIRIT_MODE_UP : 단지 혼이 위로 올라간다. ( 올라갈때 캐릭터의 방향으로 약간 꺽여서 올라간다. )

				SPIRIT_MODE_WAIT : 공중에 잠시 멈춰서서 캐릭터 방향으로 약간씩 이동한다.

				SPIRIT_MODE_ABSORB : 흡수 상태. 캐릭터 방향으로 빠른속도로 이동한다.
	*/
	if( g_InterfaceManager.IsShowLetterBox() || ( m_nMode == SPIRIT_MODE_ABSORB ) )
	{
		Direction = TargetPos - m_Position;
		fLength = D3DXVec3Length( &Direction );
		m_fSpeed = fLength * fLength * 0.1f;
		if( m_fSpeed > 40.0f )
		{
			m_fSpeed = 40.0f;
		}
		else if( m_fSpeed <= 10.0f )
		{
			m_fSpeed = 10.0f;
		}

		D3DXVec3Normalize( &Direction, &Direction );
		m_DirVector = Direction;

		m_Position += m_DirVector * m_fSpeed;
		if( fLength < 20.0f )						// 혼이 캐릭터와 가까워지면 ( 20 보다 거리가 작으면 )
		{
			CCrossVector Ptc;
			Ptc.m_YVector *= 10.0f;
			Ptc.m_PosVector = m_Position;
			Ptc.UpdateVectors();
			D3DCOLORVALUE pColor;
			pColor.a = 1.0f;
			pColor.r = 1.0f;
			pColor.g = 1.0f;
			pColor.b = 1.0f;
			SAFE_DELETE_FX(m_nTrailIndex,FX_TYPE_LINETRAIL);
			//g_pFcFXManager->SendMessage(  m_nTrailIndex , FX_DELETE_OBJECT );
			//m_nTrailIndex = -1;
			
			if( m_nOrbType == ORB_TYPE_NORMAL )
			{
				g_pSoundManager->PlaySystemSound( SB_COMMON, "SYS_GET_ORB", SC_ADD_ORB );
				m_AbsorbTarget->AddOrbSpark( m_nAddOrb );
			}
			else
			{
				g_pSoundManager->PlaySystemSound( SB_COMMON, "SYS_GET_ORB_BLUE", SC_ADD_ORB );
				m_AbsorbTarget->AddTrueOrbSpark( m_nAddOrb );
			}

			//float fScale = m_bIsBig ? 3.f : 1.f;
			int nParticleGroupIndex = (m_nOrbType == ORB_TYPE_NORMAL) ? 72 : 73;

			g_BsKernel.CreateParticleObject( nParticleGroupIndex,false,false,Ptc);//,1.0f,&pColor,true );
			
			g_BsKernel.DeleteParticleObject( m_nFxIndex );
			m_nFxIndex = -1;

			Delete();
			return;
		}// if( fLength < 20.0f )
	}
	else if( m_nMode == SPIRIT_MODE_UP )
	{
		D3DXVECTOR3 vDiff = TargetPos - m_Position;
		D3DXVec3Normalize(&vDiff, &vDiff);
		vDiff = vDiff * 0.3f;
		vDiff = (m_DirVector + vDiff) * 0.125f;
		m_Position += vDiff * m_fSpeed;
		float fRandom = 0.1f * RandomNumberInRange( 1, 5);
		m_fSpeed -= fRandom;
		if( m_fSpeed <= 0.0f )
		{
			m_nMode = SPIRIT_MODE_WAIT;
			m_fSpeed = 0.0f;
		}
	}// if
	else if( m_nMode == SPIRIT_MODE_WAIT )
	{
		m_fSpeed += 2.0f;
		if( m_fSpeed >= 10.0f )
		{
			m_nMode = SPIRIT_MODE_ABSORB;
			m_fSpeed = 10.0f;
		}
	}// else if
}
