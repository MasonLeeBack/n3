#include "stdafx.h"

#include "FcAbilityManager.h"
#include "FcAbilityBase.h"
#include "FcGameObject.h"
#include "FcAbilityWater.h"
#include "FcAbilityFire.h"
#include "FcAbilityEarth.h"
#include "FcAbilityWind.h"
#include "FcAbilitySoul.h"
#include "FcAbilityLight.h"
#include "FcAbilityRedLight.h"
#include "FcAbilityColumnOfWater.h"
#include "FcAbility1kArrow.h"
#include "FcGlobal.h"

#include "FcWorld.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG




CFcAbilityManager::CFcAbilityManager()
{
	m_nNotAbilityCount = 0;
	m_nOrbGenType = GENERATE_ORB_DISABLE;
	m_nOrbGenPer = 0;
	m_RealAbilityType = (FC_ABILITY)-1;
}

CFcAbilityManager::~CFcAbilityManager()
{
	StopAll();
}


bool CFcAbilityManager::Initialize()
{
	return true;
}

void CFcAbilityManager::Process()
{
	std::vector<CFcAbilityBase*>::iterator it;
/*	for( it = m_vecObjs.begin(); it!= m_vecObjs.end(); )
	{
		CFcAbilityBase* pObj = *it;
		if( pObj->IsFinished() )		//  아군이면 삭제
		{
			delete (*it);
			it = m_vecObjs.erase( it );
		}
		else
			it++;
	}
*/
	if( IsPlayRealMovie() || IsPlayEvent() ) StopAll();

	int n = 0;
	int nCnt = m_vecObjs.size();
	for( int i=0; i<nCnt; i++ )
	{
		CFcAbilityBase* pObj = m_vecObjs[i];
		pObj->Process();
		if( pObj->GetRtti() == 100 ) n++;
	}
	m_nNotAbilityCount = n;
}

void CFcAbilityManager::Update()
{
	std::vector<CFcAbilityBase*>::iterator it;
	for( it = m_vecObjs.begin(); it!= m_vecObjs.end(); )
	{
		CFcAbilityBase* pObj = *it;
		pObj->Update();
		if( pObj->IsFinished() )		//  아군이면 삭제
		{
			delete (*it);
			it = m_vecObjs.erase( it );
		}
		else
			it++;
	}
}

int CFcAbilityManager::GetRealAbilityCount()
{
	int result = m_vecObjs.size();
	result -= m_nNotAbilityCount;
	return result;
}

void CFcAbilityManager::StopAll()
{
	std::vector<CFcAbilityBase*>::iterator it;
	for( it = m_vecObjs.begin(); it!= m_vecObjs.end(); ++it)
	{
		delete (*it);
	}
	m_vecObjs.clear();

	g_FcWorld.SetTrueOrbTimer( 0 );
}


int CFcAbilityManager::Start( FC_ABILITY Ability, GameObjHandle hParent, DWORD Param1, DWORD Param2, DWORD Param3, DWORD Param4 )
{
	CFcAbilityBase* pObj;
	switch( Ability )
	{
	case FC_ABILITY_WATER:
		{
			pObj = new CFcAbilityWater( hParent );
			if( Param1==0 ) 
			{
				pObj->Initialize();
			}
			else
			{
				pObj->Initialize(true);
			}
		}
		break;
	case FC_ABILITY_FIRE:
		{
			pObj = new CFcAbilityFire( hParent );
			pObj->Initialize();
		}
		break;
	case FC_ABILITY_EARTH:
		{
			pObj = new CFcAbilityEarth( hParent );
			pObj->Initialize();
		}
		break;

	case FC_ABILITY_SOUL:
		{
			pObj = new CFcAbilitySoul( hParent );
			pObj->Initialize();
		}
		break;
	case FC_ABILITY_WIND:
		{
			pObj = new CFcAbilityWind( hParent );
			pObj->Initialize();
		}
		break;
	case FC_ABILITY_LIGHT_1:
		{
			pObj = new CFcAbilityLight( hParent );
			pObj->Initialize();
		}
		break;
	case FC_ABILITY_LIGHT_2:
		{
			pObj = new CFcAbilityRedLight( hParent );
			pObj->Initialize();
		}
		break;

	case FC_ABILITY_1KARROW:
		{
			pObj = new CFcAbility1KArrow( hParent );
			pObj->Initialize( Param1, Param2);
		}
		break;


	default:
		BsAssert( 0 );
		break;
	}
	if( !(Ability == FC_ABILITY_WATER && Param1 != 0) )
	{
		pObj->SetOrbGen( m_nOrbGenType, m_nOrbGenPer );
	}
	m_vecObjs.push_back( pObj );
	m_RealAbilityType = Ability;
	return true;
}


void CFcAbilityManager::Clear()
{

}


D3DXVECTOR3* CFcAbilityManager::GetAbilityPosition(GameObjHandle hOwner)
{
	int nSize = m_vecObjs.size();

	int ii = 0;
	for( ; ii < nSize; ++ii)
	{
		m_vecObjs[ii]->GetParent() == hOwner;
		break;
	}

	if(ii >= nSize)
		return NULL;

	return m_vecObjs[ii]->GetPosition();
}

void CFcAbilityManager::SetOrbGen( int nOrbGenType, int nOrbGenPer )
{
	m_nOrbGenType = nOrbGenType;
	m_nOrbGenPer= nOrbGenPer;
}