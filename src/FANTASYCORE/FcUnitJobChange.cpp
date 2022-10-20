#include "StdAfx.h"
#include "FcUnitJobChange.h"
#include "ASData.h"
#include "./Data/SignalType.h"
#include "FcSOXLoader.h"
#include "BsKernel.h"
#include "FcGameObject.h"
#include "FcWorld.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG


CFcUnitJobChange::CFcUnitJobChange()
{
	m_bSubJob = false;
	m_pParent = NULL;
	m_pSubUnitSOX = NULL;
	m_pSubASData = NULL;
	m_pSubUnitInfoData = NULL;
	m_nSubUnitSOXIndex = -1;
	m_nSubEngineIndex = -1;
	m_nPriEngineIndex = -1;
}

CFcUnitJobChange::~CFcUnitJobChange()
{
	if( m_pParent == NULL ) return;

	ASData *pDeleteASData;
	CAniInfoData *pDeleteAniInfoData;
	int nDeleteEngineIndex;

	if( m_bSubJob == true ) {
		pDeleteASData = m_pPriASData;
		pDeleteAniInfoData = m_pPriUnitInfoData;
		nDeleteEngineIndex = m_nPriEngineIndex;
	}
	else {
		pDeleteASData = m_pSubASData;
		pDeleteAniInfoData = m_pSubUnitInfoData;
		nDeleteEngineIndex = m_nSubEngineIndex;

		std::vector<ARMOR_OBJECT> *pVecArmorList = m_pParent->GetArmorList();
		for( DWORD i=0; i<pVecArmorList->size(); i++ ) {
			g_BsKernel.SendMessage( m_nSubEngineIndex, BS_UNLINKOBJECT, (*pVecArmorList)[i].nObjectIndex );
		}

		std::vector<WEAPON_OBJECT> *pVecWeaponList = m_pParent->GetWeaponList();
		for( DWORD i=0; i<pVecWeaponList->size(); i++ ) {
			g_BsKernel.SendMessage( m_nSubEngineIndex, BS_UNLINKOBJECT, (*pVecWeaponList)[i].nObjectIndex );
		}

	}

	if( nDeleteEngineIndex != -1 ) {
		g_BsKernel.DeleteObject( nDeleteEngineIndex );
	}
	if( pDeleteASData ) {
//		ASData::DeleteASData( pDeleteASData );
		pDeleteASData = NULL;
	}

	if( pDeleteAniInfoData )
	{
//		CAniInfoData::DeleteAniInfoData( pDeleteAniInfoData );
		pDeleteAniInfoData = NULL;
	}
	
}

void CFcUnitJobChange::Initialize( CFcGameObject *pParent, int nSubJobSOXIndex )
{
	m_pParent= pParent;

	m_nSubUnitSOXIndex = nSubJobSOXIndex;
	m_pSubUnitSOX = CUnitSOXLoader::GetInstance().GetUnitData( nSubJobSOXIndex );

	// Subjob의 에니파일 읽어노쿠
	int nSubAniIndex = g_FcWorld.LoadAniData( -1, m_pSubUnitSOX->cAniName );
	// 이미 읽혀진 스킨 인덱스 얻어노쿠
	int nPriSkinIndex = g_BsKernel.SendMessage( m_pParent->GetEngineIndex(), BS_GET_SKIN_INDEX );
	// 이미 읽혀진 에니 인덱스 얻어노쿠
//	int nPriAniIndex = g_BsKernel.SendMessage( m_pParent->GetEngineIndex(), BS_ANI_FILE_INDEX );

	m_nSubEngineIndex = g_BsKernel.CreateAniObjectFromSkin( nPriSkinIndex, nSubAniIndex );
	g_BsKernel.SendMessage( m_nSubEngineIndex, BS_USE_ANI_CACHE, 1 );

	char szFileName[_MAX_PATH];
	sprintf(szFileName, "as\\%s", m_pSubUnitSOX->cASFileName );
	m_pSubASData = ASData::LoadASData( szFileName );

	sprintf(szFileName, "as\\%s", m_pSubUnitSOX->cUnitInfoFileName );
	m_pSubUnitInfoData = CAniInfoData::LoadAniInfoData( szFileName );

	// Parts Link
	std::vector<ARMOR_OBJECT> *pVecArmorList = m_pParent->GetArmorList();
	for( DWORD i=0; i<pVecArmorList->size(); i++ ) {
		g_BsKernel.SendMessage( m_nSubEngineIndex, BS_LINKOBJECT, ( DWORD )(*pVecArmorList)[i].nLinkBoneIndex, (*pVecArmorList)[i].nObjectIndex );
	}

	std::vector<WEAPON_OBJECT> *pVecWeaponList = m_pParent->GetWeaponList();
	for( DWORD i=0; i<pVecWeaponList->size(); i++ ) {
		g_BsKernel.SendMessage( m_nSubEngineIndex, BS_LINKOBJECT, ( DWORD )(*pVecWeaponList)[i].nLinkBoneIndex, (*pVecWeaponList)[i].nObjectIndex );
	}
	
	// 원본 빽옵~
	m_nPriEngineIndex = m_pParent->GetEngineIndex();
	m_pPriASData = m_pParent->GetASData();
	m_pPriUnitInfoData = m_pParent->GetUnitInfoData();
}

void CFcUnitJobChange::ChangeJob( bool bSubJob )
{
	if( m_bSubJob != bSubJob ) {
		if( m_bSubJob == true ) {	// SubJob 에서 원래 Job 으루 변경시
			m_pParent->SetEngineIndex( m_nPriEngineIndex );
			m_pParent->SetASData( m_pPriASData );
			m_pParent->SetUnitInfoData( m_pPriUnitInfoData );
		}
		else { // 원래 Job 에서 SubJob 으로 변경시
			m_pParent->SetEngineIndex( m_nSubEngineIndex );
			m_pParent->SetASData( m_pSubASData );
			m_pParent->SetUnitInfoData( m_pSubUnitInfoData );
		}
	}
	m_bSubJob = bSubJob;
}

bool CFcUnitJobChange::IsSubJob()
{
	return m_bSubJob;
}
