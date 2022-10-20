#include "stdafx.h"

#include "FcWorld.h"
#include "FcTroopManager.h"
#include "FcProjectile.h"
#include "FcUtil.h"
#include "FcGlobal.h"
#include "DebugUtil.h"


#include "BsCamera.h"
#include "BsKernel.h"
#include "GenericCamera.h"

#include "FcWorld.h"

#include "GenericCameraAttach.h"
#include "Data/Camera/GenericCamDef.h"

#include "FcHeroObject.h"
#include "FcTroopObject.h"

#define	ACT_CAM_ROTATE_DELAY		20





bool CGenericCamera::GetCrdNDir(int nCamType, int nParm, D3DXVECTOR3 *pPos, D3DXVECTOR3 *pDir)
{
	TroopObjHandle	Troop;
	GameObjHandle	Unit;
	//	K2ArcherTroop	*pArcher;

	D3DXVECTOR2	v2;
	D3DXVECTOR3	v3;
	*pPos = D3DXVECTOR3(0.f, 0.f, 0.f);
	*pDir = D3DXVECTOR3(0.f, 0.f, 1.f);
	switch (nCamType)
	{
	case	CAM_ATTACH_PLAYER_TROOP:
	case	CAM_ATTACH_AXIS_PLAYERTROOP_TO_TARGET:
		Troop = g_FcWorld.GetHeroHandle()->GetTroop();
		BsAssert( Troop != NULL );
		GetTroopCrdNDir( Troop,pPos,pDir );
		break;

	case	CAM_ATTACH_PLAYER_TROOP_ACTUAL:
		Troop = g_FcWorld.GetHeroHandle()->GetTroop();
		BsAssert( Troop != NULL );
		GetTroopActualCrdNDir( Troop,pPos,pDir );
		break;

	case	CAM_ATTACH_PLAYER:
	case	CAM_ATTACH_AXIS_PLAYER_TO_TARGET:
		Unit = g_FcWorld.GetHeroHandle();
		GetUnitCrdNDir(Unit,pPos,pDir);
		break;

	case	CAM_ATTACH_TARGET_TROOP:
	case	CAM_ATTACH_AXIS_TARGETTROOP_TO_TARGET:
		Troop = m_Target;
		GetTroopCrdNDir( Troop,pPos,pDir );
		break;

	case	CAM_ATTACH_TARGET_LEADER:
		Troop = m_Target;
		if( Troop )
		{
			Unit = Troop->GetLeader();
			if( Unit == NULL )
			{
				if( Troop->GetHP() == 0 )
				{
					GetTroopCrdNDir( Troop,pPos,pDir);
				}
				else
				{
					for( int nCount = 0; nCount < Troop->GetUnitCount(); nCount++ )
					{
						//						Unit = Troop->GetUnit( 1 );
						Unit = Troop->GetUnit( nCount );	// 이거 맞나?
						if( Unit == NULL )
							continue;

						if( Unit->IsDie() == true )
						{
							Unit.Identity();
							continue;
						}
						else
							break;
					}
					GetUnitCrdNDir( Unit,pPos,pDir );
				}
			}
			else
				GetUnitCrdNDir( Unit,pPos,pDir );
		}
		break;

	case	CAM_ATTACH_TROOP_UNIT:
		{
			int	nAliveUnit;
			Troop = g_FcWorld.GetHeroHandle()->GetTroop(); // 현재 attach된 녀석을 바꾸면 그 녀석으로 바꿔줘야 한다. 
			nAliveUnit = Troop->GetAliveUnitCount();
			Unit.Identity();
			if (nAliveUnit > 0)
			{
				if (m_nRandom1 == -1 || GetTick() % 70 == 0 && Random(4) == 0)
				{
					while(1)
					{
						m_nRandom1 = (m_nRandom1 + 1) % Troop->GetUnitCount();

						Unit = Troop->GetUnit(m_nRandom1);
						if( Unit == NULL )
							continue;

						if (Unit->IsDie()==false)
							break;
					}
				}
				else
					Unit = Troop->GetUnit(m_nRandom1);
			}
			else
				Unit = Troop->GetLeader();

			if( Unit == NULL )
				Unit = m_Target->GetLeader();

			GetUnitCrdNDir( Unit,pPos,pDir );
		}
		break;

	case	CAM_ATTACH_TROOP_UNIT_ALIVE:
		Troop = g_FcWorld.GetHeroHandle()->GetTroop(); // 현재 attach된 녀석을 바꾸면 그 녀석으로 바꿔줘야 한다. 
		Unit.Identity();
		if (m_nRandom1 == -1 || GetTick() % 70 == 0 && Random(4) == 0)
			m_nRandom1 = (m_nRandom1 + 1) % Troop->GetUnitCount();

		Unit = Troop->GetUnit(m_nRandom1);

		if( Unit )
			GetUnitCrdNDir( Unit,pPos,pDir );

		break;

	case	CAM_ATTACH_TROOP_UNIT_ALIVE_3D:
		Troop = m_Target;
		if( Troop != NULL && Troop->GetUnitCount() > 0 )
		{
			int	nScan;
			Unit.Identity();
			for(nScan = 0;nScan < Troop->GetUnitCount(); nScan++)
			{
				Unit = Troop->GetUnit(nScan);
				if( Unit != NULL )
				{
					if( Unit->GetHP() > 0 )
						break;
				}
			}
			if( Unit == NULL )
				Unit = Troop->GetLeader();

			GetUnitCrdNDir( Unit,pPos,pDir );
		}
		break;

	case	CAM_ATTACH_TROOP_PROJECTILE:
		Unit = g_FcWorld.GetHeroHandle();
		GetUnitCrdNDir(Unit,pPos,pDir);
		break;


	case	CAM_ATTACH_ACTION_EYE:
		AttachActionCamera(nParm, pPos, pDir);
		break;

	case	CAM_ATTACH_LEADER_FORWARD:
		{
			Troop = m_Target;
			if( Troop )
			{
				Unit = Troop->GetLeader();
				if( Unit )
					GetUnitCrdNDir( Unit,pPos,pDir );
/*
				if (nCamType == CAM_ATTACH_LEADER_FORWARD && Unit->GetCurAniType() != ANI_TYPE_HIT)
				{
					float	fSpeed;
					fSpeed = 0;
					float fUnitHeight = Unit->GetUnitHeight();
					pPos->x += pDir->x * fUnitHeight * m_fActionPlayerFowardScale * (1.0f + fSpeed * m_fActionPlayerFowardSpeedScale);
					pPos->z += pDir->z * fUnitHeight * m_fActionPlayerFowardScale * (1.0f + fSpeed * m_fActionPlayerFowardSpeedScale);
				}
*/
			}

		}
		break;

	case	CAM_ATTACH_ORBSPARK_EYE:
	case	CAM_ATTACH_ORBSPARK_TARGET:
		{
			D3DXVECTOR3	PlayerPos=D3DXVECTOR3(0.f,0.f,0.f), AbilityPos; //aleksger: prefix bug 662: Variable unitialized if Unit == NULL
			D3DXVECTOR3	PlayerDir=D3DXVECTOR3(1.f,0.f,0.f);//aleksger: prefix bug 662: Variable unitialized if Unit == NULL
			D3DXVECTOR3	Dir;

			Unit = g_FcWorld.GetHeroHandle();
			BsAssert(Unit); //aleksger: prefix bug 662: Must have unit.
			GetUnitCrdNDir(Unit,&PlayerPos,&PlayerDir);

			AbilityPos = D3DXVECTOR3(0.f,0.f,0.f);

			D3DXVECTOR3* pAPos = g_FcWorld.GetAbilityMng()->GetAbilityPosition(g_FcWorld.GetHeroHandle());
			
			if (pAPos)
				AbilityPos = *pAPos;

			AbilityPos.y *= .5f;
			
			Dir = AbilityPos - PlayerPos;

			float	fLen = D3DXVec3Length(&Dir);

			if (fLen > 0.0f)
				*pDir = Dir / fLen;
			else
				*pDir = PlayerDir;

			if (nCamType == CAM_ATTACH_ORBSPARK_EYE)
				*pPos = PlayerPos;
			else
				*pPos = AbilityPos;
		}
		break;

	case CAM_ATTACH_MARKED_AREA:
	case CAM_ATTACH_AXIS_MARKEDAREA_TO_TARGET:
	case CAM_ATTACH_MARKED_POINT_FROM_CURRENT:
		{
			pPos->x = m_MarkedPoint.x;
			pPos->z = m_MarkedPoint.y;

			bool bProp;
			pPos->y = g_FcWorld.GetLandHeight(m_MarkedPoint.x, m_MarkedPoint.y , &bProp);
			*pDir = D3DXVECTOR3( 0.f, 0.f, 1.f );
		}
		break;

	default:
		return false;
	}
	return true;
}



void	CGenericCamera::AttachActionCamera(int nParm, D3DXVECTOR3	*pPos, D3DXVECTOR3	*pDir)
{
	GameObjHandle Unit;
	D3DXVECTOR3	v3UnitCrd=D3DXVECTOR3(0.f,0.f,0.f); //aleksger: prefix bug 663: Unitialized variable v3UnitCrd if Unit == NULL
	D3DXVECTOR3	v3Dir;
	D3DXVECTOR2	v2Dir;
	float	fCosVal;
	float	fSinVal;

	Unit = g_FcWorld.GetHeroHandle();
	BsAssert(Unit); //aleksger: prefix bug 663: Unitialized variable v3UnitCrd if Unit == NULL
	if( Unit )
	{
		GetUnitCrdNDir( Unit, &v3UnitCrd, &v3Dir);
		v2Dir.x = v3Dir.x;
		v2Dir.y = v3Dir.z;

		// vector 깨지는 버그!!!
		if( ( v2Dir.x == v2Dir.x ) == false )
		{
			_DEBUGBREAK;
		}
		if( ( v2Dir.y == v2Dir.y ) == false )
		{
			_DEBUGBREAK;
		}

		if (IsEditMode() == false)
		{
			//// 카메라 회전 계산.

			D3DXVec2Normalize( &v2Dir, &v2Dir );

			if (m_vActionEye.x == 0.0f && m_vActionEye.y == 0.0f)	// 초기화
			{
				m_vActionEye = v2Dir;
			}

			if (m_nFreeCamMoveCount > 0)
			{
				fCosVal = cosf(-m_fFreeCamDA);
				fSinVal = sinf(-m_fFreeCamDA);

				// vector 깨지는 버그!!!
				if( ( m_fFreeCamDA == m_fFreeCamDA ) == false )
				{
					_DEBUGBREAK;
				}
				if( ( fCosVal == fCosVal ) == false )
				{
					_DEBUGBREAK;
				}
				if( ( fSinVal == fSinVal ) == false )
				{
					_DEBUGBREAK;
				}

				v2Dir.x = m_vActionEye.x * fCosVal - m_vActionEye.y * fSinVal;
				v2Dir.y = m_vActionEye.x * fSinVal + m_vActionEye.y * fCosVal;

				// vector 깨지는 버그!!!
				if( ( v2Dir.x == v2Dir.x ) == false )
				{
					_DEBUGBREAK;
				}
				if( ( v2Dir.y == v2Dir.y ) == false )
				{
					_DEBUGBREAK;
				}

				m_vActionEye.x = v2Dir.x;
				m_vActionEye.y = v2Dir.y;
				m_fFreeCamDA = 0.0f;
			}
		}
		else
		{
			// vector 깨지는 버그!!!
			if( ( v2Dir.x == v2Dir.x ) == false )
			{
				_DEBUGBREAK;
			}
			if( ( v2Dir.y == v2Dir.y ) == false )
			{
				_DEBUGBREAK;
			}

			m_vActionEye = v2Dir;
			m_fFreeCamB = 0.0f;
		}
	}

	D3DXVec2Normalize( &m_vActionEye, &m_vActionEye );

	pDir->x = m_vActionEye.x;
	pDir->z = m_vActionEye.y;
	pDir->y = sin(m_fFreeCamB);

	D3DXVec3Normalize(pDir, pDir);

	*pPos = v3UnitCrd - *pDir;
	m_bModifyCamHeightByLand = true;
}

bool	IsLocalAttach(int nAttach)
{
	if (nAttach == CAM_ATTACH_LOCAL_CRD || 
		nAttach == CAM_ATTACH_LOCAL_AXIS ||
		nAttach == CAM_ATTACH_ACTION_EYE ||
		nAttach == CAM_ATTACH_MARKED_POINT_FROM_CURRENT ||
		nAttach >= CAM_ATTACH_AXIS_PLAYER_TO_TARGET &&
		nAttach <= CAM_ATTACH_AXIS_MARKEDAREA_TO_TARGET)
		return true;
	else
		return false;
}