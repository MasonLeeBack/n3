#include "stdafx.h"
#include "FcTroopManager.h"
#include "FcTroopObject.h"
#include "FcGameObject.h"
#include "FcGlobal.h"




void CFcTroopManager::CaculateFakeMode( TroopObjHandle hTroop )
{
	if( (GetProcessTick() % FRAME_PER_SEC) != ( abs(hTroop->GetTroopID()) % 40 ) )
		return;

	if( hTroop->IsEliminated() )
		return;

	if( !hTroop->IsEnable() )
		return;

	if( hTroop->IsMeleeEngaged() )
	{
		int nCnt = hTroop->GetMeleeEngageTroopCount();
		for( int i=0; i<nCnt; i++ )
		{
			TroopObjHandle hTarget = hTroop->GetMeleeEngageTroop( i );
			if( hTarget->IsEnable() == false )
				continue;

			if( hTarget->IsEliminated() )
				continue;

			CaculateMeleeFakeMode( hTroop, hTarget );
		}
	}
	else if( hTroop->IsRangeEngaged() )
	{

	}
}


// ���� ���� �츮�� HP�� ��´�.
void CFcTroopManager::CaculateMeleeFakeMode( TroopObjHandle hTroop, TroopObjHandle hEnemy )
{
	if( hEnemy->GetType() == TROOPTYPE_NOBATTLE )		// �ù��� �������� ���ش�.
		return;

	if( hTroop->IsUntouchable() )
		return;

	int nCnt = hTroop->GetUnitCount();
	if( nCnt == 0 )
		return;

	int nTotalDamage = 0;
	int nUnitCnt = hEnemy->GetUnitCount();
	for( int i=0; i<nUnitCnt; i++ )
	{
		GameObjHandle hUnit = hEnemy->GetUnit( i );
		if( hUnit == NULL )								continue;
		if( CFcBaseObject::IsValid( hUnit ) == false )	continue;
		if( hUnit->IsDie() )							continue;
		if( !hUnit->IsEnable() )						continue;

		int nDamage =  hUnit->GetAttackPower() - hUnit->GetDefense();
		if( nDamage < 0 ) { nDamage = 5; }
		nTotalDamage += nDamage;
	}

	// Damage!
	int nDamage = nTotalDamage / nCnt;
	for( int i=0; i<nCnt; i++ )
	{
		GameObjHandle hUnit = hTroop->GetUnit( i );
		if( Random( 10 ) != 3 )
			continue;

		if( hUnit == NULL )								continue;
		if( CFcBaseObject::IsValid( hUnit ) == false )	continue;
		if( hUnit->IsDie() )							continue;
		if( !hUnit->IsEnable() )						continue;

		// ����� ���� ������ ����
		if( hUnit->GetClassID() >= CFcGameObject::Class_ID_Hero ||
			hUnit->GetClassID() == CFcGameObject::Class_ID_Adjutant )
			continue;

		GameObjHandle hDummy;
		if( !hUnit->IsHittable( hDummy ) )
			continue;

		hUnit->AddHP( -nDamage );

		if( hUnit->GetHP() <= 0 )
		{
			hUnit->SetAniFrame(ANI_TYPE_DIE, 0, 10000);
			//					hUnit->SetDie();
			// ������ ����� ó���� ��� �Ѵ�.
			hUnit->SetDie();		// �׾������� ���μ��� ���߰� �Ѵ�..
			if( hUnit->IsDie() ) {
				/*
				if( !m_bItemDrop )
				{
				g_FcItemManager.CreateItemToWorld( GetUnitSoxIndex(), GetLevel() , &m_Cross );
				m_bItemDrop = true;
				}
				*/
			}
		}
	}
}

