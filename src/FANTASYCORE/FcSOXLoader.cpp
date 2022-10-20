#include "StdAfx.h"
#include "FcSoxLoader.h"
#include "FcSOXFile.h"
#include "BsKernel.h"
#include "LocalLanguage.h"
#include "bstreamExt.h"
#include "BsFileIO.h"
#include "FcWorld.h"

//#include <string.h>


#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG


CUnitSOXLoader::CUnitSOXLoader(void)
{
	m_nUnitDataCount = 0;
	m_pUnitData = NULL;
}

CUnitSOXLoader::~CUnitSOXLoader(void)
{
	Clear();
}

void CUnitSOXLoader::Clear()
{
	if(m_pUnitData){
		delete [] m_pUnitData;
		m_pUnitData=NULL;
	}
}

int CUnitSOXLoader::Load()
{
	DWORD dwFileSize;
	VOID *pData;

	g_BsKernel.chdir( "sox" );
	char* pFullName = g_BsKernel.GetFullName( "chardata.sox" );
	if(FAILED(CBsFileIO::LoadFile(pFullName, &pData, &dwFileSize)))
		BsAssert(0 && "Failed to open sox file");

#ifdef ENABLE_RES_CHECKER
	g_BsResChecker.AddResList( pFullName );
#endif

	BMemoryStream Stream(pData, dwFileSize);
	g_BsKernel.chdir("..");

	CFcSOXFile Sox( &Stream );

	m_nUnitDataCount = Sox.GetItemCount();
	m_pUnitData=new UnitDataInfo[m_nUnitDataCount];
	memset(m_pUnitData, 0, sizeof(UnitDataInfo)*m_nUnitDataCount);

	CFcSOXFile::SOX_Field *pField;
	for( int i=0; i<m_nUnitDataCount; i++ ) {

		if( ( pField = Sox.GetFieldFromLablePtr( i+1, "name" ) ) )
			strcpy( m_pUnitData[i].cUnitName, pField->uData.pValue );
		if( ( pField = Sox.GetFieldFromLablePtr( i+1, "unit folder" ) ) )
			strcpy( m_pUnitData[i].cSkinDir, pField->uData.pValue );
		if( ( pField = Sox.GetFieldFromLablePtr( i+1, "skin name" ) ) )
			strcpy( m_pUnitData[i].cSkinFileName, pField->uData.pValue );

		if( ( pField = Sox.GetFieldFromLablePtr( i+1, "skin variation num" ) ) ) 
			m_pUnitData[i].cSkinVariationNum = pField->uData.nValue;

		if( ( pField = Sox.GetFieldFromLablePtr( i+1, "shadow skin name" ) ) )
			strcpy( m_pUnitData[i].cShadowSkinName, pField->uData.pValue );
		if( ( pField = Sox.GetFieldFromLablePtr( i+1, "ani name" ) ) )
			strcpy( m_pUnitData[i].cAniName, pField->uData.pValue );

		if( ( pField = Sox.GetFieldFromLablePtr( i+1, "mhp" ) ) )
			m_pUnitData[i].nMaxHP=pField->uData.nValue;;
		if( ( pField = Sox.GetFieldFromLablePtr( i+1, "attack point" ) ) )
			m_pUnitData[i].nAttackPoint=pField->uData.nValue;;
		if( ( pField = Sox.GetFieldFromLablePtr( i+1, "defense point" ) ) )
			m_pUnitData[i].nDefensePoint=pField->uData.nValue;;

		if( ( pField = Sox.GetFieldFromLablePtr( i+1, "unit type" ) ) )
			m_pUnitData[ i ].nUnitType = pField->uData.nValue;
		if( ( pField = Sox.GetFieldFromLablePtr( i+1, "unit size" ) ) )
			m_pUnitData[ i ].nUnitSize = pField->uData.nValue;

		if( ( pField = Sox.GetFieldFromLablePtr( i+1, "AS file" ) ) )
			strcpy( m_pUnitData[i].cASFileName, pField->uData.pValue );

		if( ( pField = Sox.GetFieldFromLablePtr( i+1, "Second Job Index" ) ) )
			m_pUnitData[ i ].nSubJobSOXIndex = pField->uData.nValue;

		if( ( pField = Sox.GetFieldFromLablePtr( i+1, "Parts file" ) ) )
			strcpy( m_pUnitData[i].cUnitInfoFileName, pField->uData.pValue );

		if( ( pField = Sox.GetFieldFromLablePtr( i+1, "Unit Height" ) ) )
			m_pUnitData[ i ].nUnitHeight = pField->uData.nValue;

		if( ( pField = Sox.GetFieldFromLablePtr( i+1, "LevelTable" ) ) )
			strcpy( m_pUnitData[i].cLevelTableFileName, pField->uData.pValue );

		if( ( pField = Sox.GetFieldFromLablePtr( i+1, "troop type" ) ) )
			m_pUnitData[i].nTroopType = pField->uData.nValue;

		char cDummy[64];
		if( ( pField = Sox.GetFieldFromLablePtr( i+1, "billboard name" ) ) )
			strcpy( cDummy, pField->uData.pValue );

		if( ( pField = Sox.GetFieldFromLablePtr( i+1, "billboard size" ) ) )
			m_pUnitData[i].nBillboardSize = pField->uData.nValue;
		if( ( pField = Sox.GetFieldFromLablePtr( i+1, "unit min scale" ) ) )
			m_pUnitData[i].nMinScale = pField->uData.nValue;
		if( ( pField = Sox.GetFieldFromLablePtr( i+1, "unit max scale" ) ) )
			m_pUnitData[i].nMaxScale = pField->uData.nValue;
		if( ( pField = Sox.GetFieldFromLablePtr( i+1, "MaxLevel" ) ) )
			m_pUnitData[i].nMaxLevel = pField->uData.nValue;
		if( ( pField = Sox.GetFieldFromLablePtr( i+1, "A_LevelOffset" ) ) )
			m_pUnitData[i].nLevelStartOffset[0] = pField->uData.nValue;
		if( ( pField = Sox.GetFieldFromLablePtr( i+1, "B_LevelOffset" ) ) )
			m_pUnitData[i].nLevelStartOffset[1] = pField->uData.nValue;
		if( ( pField = Sox.GetFieldFromLablePtr( i+1, "C_LevelOffset" ) ) )
			m_pUnitData[i].nLevelStartOffset[2] = pField->uData.nValue;
		if( ( pField = Sox.GetFieldFromLablePtr( i+1, "D_LevelOffset" ) ) )
			m_pUnitData[i].nLevelStartOffset[3] = pField->uData.nValue;
		if( ( pField = Sox.GetFieldFromLablePtr( i+1, "Use Level" ) ) )
			m_pUnitData[i].bUseLevel = (pField->uData.nValue != 0);
		if( ( pField = Sox.GetFieldFromLablePtr( i+1, "SoundAmor" ) ) )
			m_pUnitData[i].nSoundAmor = pField->uData.nValue;
		if( ( pField = Sox.GetFieldFromLablePtr( i+1, "SoundWeapon" ) ) )
			m_pUnitData[i].nSoundWeapon = pField->uData.nValue;
		if( ( pField = Sox.GetFieldFromLablePtr( i+1, "Weight" ) ) )
			m_pUnitData[i].fWeight = pField->uData.nValue / 100.f;
		if( ( pField = Sox.GetFieldFromLablePtr( i+1, "Kakegoe" ) ) )
			strcpy( m_pUnitData[i].cWBKakegoe, pField->uData.pValue );
		if( ( pField = Sox.GetFieldFromLablePtr( i+1, "crowd" ) ) )
			strcpy( m_pUnitData[i].cWBCrowd, pField->uData.pValue );
		if( ( pField = Sox.GetFieldFromLablePtr( i+1, "CrowdUnitType" ) ) )
			m_pUnitData[i].nCrowdUnitType = pField->uData.nValue;

		if( ( pField = Sox.GetFieldFromLablePtr( i+1, "billboard_A_Type" ) ) )
			strcpy( m_pUnitData[i].cBillboardName[0], pField->uData.pValue );
		if( ( pField = Sox.GetFieldFromLablePtr( i+1, "billboard_B_Type" ) ) )
			strcpy( m_pUnitData[i].cBillboardName[1], pField->uData.pValue );
		if( ( pField = Sox.GetFieldFromLablePtr( i+1, "billboard_C_Type" ) ) )
			strcpy( m_pUnitData[i].cBillboardName[2], pField->uData.pValue );
		if( ( pField = Sox.GetFieldFromLablePtr( i+1, "billboard_D_Type" ) ) )
			strcpy( m_pUnitData[i].cBillboardName[3], pField->uData.pValue );

	}

	CBsFileIO::FreeBuffer(pData);

	return 1;
}


CLevelTableLoader::CLevelTableLoader()
{
}

CLevelTableLoader::~CLevelTableLoader()
{
	Clear();
}

void CLevelTableLoader::Clear()
{
	for( int i=0; i<2; i++ ) {
		SAFE_DELETE_VEC( m_AttackData[i] );
		SAFE_DELETE_VEC( m_DefenseData[i] );
		SAFE_DELETE_VEC( m_HPData[i] );
		SAFE_DELETE_VEC( m_CriticalData[i] );
		SAFE_DELETE_VEC( m_CriticalRatioData[i] );
	}
	SAFE_DELETE_VEC( m_OrbSparkData );
	SAFE_DELETE_VEC( m_GiveExpData );
}

void CLevelTableLoader::Load()
{
	BMemoryStream *pStream;
	DWORD dwFileSize;
	VOID *pData;

	g_BsKernel.chdir( "sox" );

	char* pFullName = g_BsKernel.GetFullName( "AttackTable.sox" );
	if(FAILED(CBsFileIO::LoadFile(pFullName, &pData, &dwFileSize)))
		BsAssert(0 && "Failed to open sox file");
	pStream = new BMemoryStream( pData, dwFileSize );
	LoadTable( pStream, &m_AttackData[0] );
	pStream->Seek( 0, BStream::fromBegin );
	LoadTable( pStream, &m_AttackData[1], 11 );
	CBsFileIO::FreeBuffer(pData);
	delete pStream;

#ifdef ENABLE_RES_CHECKER
	g_BsResChecker.AddResList( pFullName );
#endif

	pFullName = g_BsKernel.GetFullName( "DefenseTable.sox" );
	if(FAILED(CBsFileIO::LoadFile(pFullName, &pData, &dwFileSize)))
		BsAssert(0 && "Failed to open sox file");
	pStream = new BMemoryStream( pData, dwFileSize );
	LoadTable( pStream, &m_DefenseData[0] );
	pStream->Seek( 0, BStream::fromBegin );
	LoadTable( pStream, &m_DefenseData[1], 11 );
	pStream->Seek( 0, BStream::fromBegin );
	LoadTable( pStream, &m_DefenseData[2], 22 );
	CBsFileIO::FreeBuffer(pData);
	delete pStream;

#ifdef ENABLE_RES_CHECKER
	g_BsResChecker.AddResList( pFullName );
#endif

	pFullName = g_BsKernel.GetFullName( "HPTable.sox" );
	if(FAILED(CBsFileIO::LoadFile(pFullName, &pData, &dwFileSize)))
		BsAssert(0 && "Failed to open sox file");
	pStream = new BMemoryStream( pData, dwFileSize );
	LoadTable( pStream, &m_HPData[0] );
	pStream->Seek( 0, BStream::fromBegin );
	LoadTable( pStream, &m_HPData[1], 11 );
	CBsFileIO::FreeBuffer(pData);
	delete pStream;

#ifdef ENABLE_RES_CHECKER
	g_BsResChecker.AddResList( pFullName );
#endif

	pFullName = g_BsKernel.GetFullName( "CriticalTable.sox" );
	if(FAILED(CBsFileIO::LoadFile(pFullName, &pData, &dwFileSize)))
		BsAssert(0 && "Failed to open sox file");
	pStream = new BMemoryStream( pData, dwFileSize );
	LoadTable( pStream, &m_CriticalData[0] );
	pStream->Seek( 0, BStream::fromBegin );
	LoadTable( pStream, &m_CriticalRatioData[0], 11 );
	pStream->Seek( 0, BStream::fromBegin );
	LoadTable( pStream, &m_CriticalData[1], 22 );
	pStream->Seek( 0, BStream::fromBegin );
	LoadTable( pStream, &m_CriticalRatioData[1], 33 );
	CBsFileIO::FreeBuffer(pData);
	delete pStream;

#ifdef ENABLE_RES_CHECKER
	g_BsResChecker.AddResList( pFullName );
#endif

	pFullName = g_BsKernel.GetFullName( "OrbTable.sox" );
	if(FAILED(CBsFileIO::LoadFile(pFullName, &pData, &dwFileSize)))
		BsAssert(0 && "Failed to open sox file");
	pStream = new BMemoryStream( pData, dwFileSize );
	LoadTable( pStream, &m_OrbSparkData );
	pStream->Seek( 0, BStream::fromBegin );
	LoadTable( pStream, &m_GiveExpData, 11 );
	CBsFileIO::FreeBuffer(pData);
	delete pStream;

#ifdef ENABLE_RES_CHECKER
	g_BsResChecker.AddResList( pFullName );
#endif

	pFullName = g_BsKernel.GetFullName( "DefenseProbTable.sox" );
	if(FAILED(CBsFileIO::LoadFile(pFullName, &pData, &dwFileSize)))
		BsAssert(0 && "Failed to open sox file");
	pStream = new BMemoryStream( pData, dwFileSize );
	LoadTable( pStream, &m_DefenseProbData[0] );
	pStream->Seek( 0, BStream::fromBegin );
	LoadTable( pStream, &m_DefenseProbData[1], 11 );
	pStream->Seek( 0, BStream::fromBegin );
	LoadTable( pStream, &m_DefenseProbData[2], 22 );
	CBsFileIO::FreeBuffer(pData);
	delete pStream;

#ifdef ENABLE_RES_CHECKER
	g_BsResChecker.AddResList( pFullName );
#endif

	pFullName = g_BsKernel.GetFullName( "LevelMatchingTable.sox" );
	if(FAILED(CBsFileIO::LoadFile(pFullName, &pData, &dwFileSize)))
		BsAssert(0 && "Failed to open sox file");
	pStream = new BMemoryStream( pData, dwFileSize );
	LoadTable( pStream, &m_LevelMatchingData, 0, 12 ); // Stage Level 별로 12개이기 때문에.. 썅!!
	CBsFileIO::FreeBuffer(pData);
	delete pStream;

#ifdef ENABLE_RES_CHECKER
	g_BsResChecker.AddResList( pFullName );
#endif

	g_BsKernel.chdir("..");
}


void CLevelTableLoader::LoadTable( BStream *pStream, std::vector<IntegerValue> *pVecList, int nOffset, int nTableCount )
{
	int i, j, nCount;
	IntegerValue Struct;
	CFcSOXFile Sox( pStream );
	CFcSOXFile::SOX_Field *pField;

	nCount = Sox.GetItemCount();
	for( i = 0; i < nCount; i++ )
	{
		for( j = 0; j < nTableCount; j++ )
		{
			pField = Sox.GetFieldPtr( i + 1, nOffset+j+1 );
			if( pField )
				Struct.nValue[j] = pField->uData.nValue;
		}
		pVecList->push_back( Struct );
	}
}

int CLevelTableLoader::GetAttackPoint( int nUnitIndex, int nLevel, bool bFriend )
{
	return m_AttackData[ bFriend ][ nUnitIndex ].nValue[ nLevel ];
}

int CLevelTableLoader::GetDefensePoint( int nUnitIndex, int nLevel, bool bFriend )
{
	return m_DefenseData[ bFriend ][ nUnitIndex ].nValue[ nLevel ];
}

int CLevelTableLoader::GetMaxHP( int nUnitIndex, int nLevel, bool bFriend )
{
	return m_HPData[ bFriend ][ nUnitIndex ].nValue[ nLevel ];
}

int CLevelTableLoader::GetCritical( int nUnitIndex, int nLevel, bool bFriend )
{
	return m_CriticalData[ bFriend ][ nUnitIndex ].nValue[ nLevel ];
}

float CLevelTableLoader::GetCriticalRatio( int nUnitIndex, int nLevel, bool bFriend )
{
	return m_CriticalRatioData[ bFriend ][ nUnitIndex ].nValue[ nLevel ] * 0.01f;
}

int CLevelTableLoader::GetOrbSparkPoint( int nUnitIndex, int nLevel )
{
	return m_OrbSparkData[ nUnitIndex ].nValue[ nLevel ];
}

int CLevelTableLoader::GetGiveExpPoint( int nUnitIndex, int nLevel )
{
	return m_GiveExpData[ nUnitIndex ].nValue[ nLevel ];
}

int CLevelTableLoader::GetDefenseProb( int nUnitIndex, int nLevel, int nType )
{
	return m_DefenseProbData[ nType ][ nUnitIndex ].nValue[ nLevel ];
}

int CLevelTableLoader::GetMatchingLevel( int nUnitIndex, int nLevel )
{
	nLevel += g_FcWorld.GetCurMapLevel();
	if( nLevel < 0 ) nLevel = 0;
	if( nLevel > 11 ) nLevel = 11;
	return m_LevelMatchingData[ nUnitIndex ].nValue[ nLevel ] - 1;
}

float CLevelTableLoader::GetTrueOrbDamageAttenuationRatio( int nUnitIndex, int nLevel )
{
	return m_DefenseData[2][ nUnitIndex ].nValue[ nLevel ] * 0.01f;
}

/*
void CLevelTableLoader::SetAttackPoint( int nUnitIndex, int nLevel, int nAttackPoint )
{
	int nCnt = m_AttackData.size();

	BsAssert( nUnitIndex >= 0 && nUnitIndex < nCnt );
	BsAssert( nLevel >= 0 && nLevel < MAX_NPC_LEVEL );

	m_AttackData[ nUnitIndex ].nValue[ nLevel ] = nAttackPoint;
}

void CLevelTableLoader::SetDefensePoint( int nUnitIndex, int nLevel, int nDefencePoint )
{
	int nCnt = m_DefenseData.size();

	BsAssert( nUnitIndex >= 0 && nUnitIndex < nCnt );
	BsAssert( nLevel >= 0 && nLevel < MAX_NPC_LEVEL );

	m_DefenseData[ nUnitIndex ].nValue[ nLevel ] = nDefencePoint;

}
void CLevelTableLoader::SetMaxHP( int nUnitIndex, int nLevel, int nMaxHP)
{
	int nCnt = m_HPData.size();

	BsAssert( nUnitIndex >= 0 && nUnitIndex < nCnt );
	BsAssert( nLevel >= 0 && nLevel < MAX_NPC_LEVEL );

	m_HPData[ nUnitIndex ].nValue[ nLevel ] = nMaxHP;

}
*/
CHeroLevelTableLoader::CHeroLevelTableLoader()
{
}

CHeroLevelTableLoader::~CHeroLevelTableLoader()
{
	Clear();
}

void CHeroLevelTableLoader::Clear()
{
	SAFE_DELETE_VEC( m_MoveSpeedData );
	SAFE_DELETE_VEC( m_SlotCountData );
	SAFE_DELETE_VEC( m_OrbDecreaseData );
	SAFE_DELETE_VEC( m_GuardianDefenseData );
	SAFE_DELETE_VEC( m_GuardianHPData );
	SAFE_DELETE_VEC( m_GuardianControlTroopCountData );
	SAFE_DELETE_VEC( m_GuardianControlTroopUnitCountData );
	SAFE_DELETE_VEC( m_AttackRangeData );
}

float CHeroLevelTableLoader::GetMoveSpeed( int nUnitIndex, int nLevel )
{
	return m_MoveSpeedData[nUnitIndex].nValue[nLevel] / 10.f;
}

int CHeroLevelTableLoader::GetSlotCount( int nUnitIndex, int nLevel )
{
	return m_SlotCountData[nUnitIndex].nValue[nLevel];
}

int CHeroLevelTableLoader::GetOrbDecrease( int nUnitIndex, int nLevel )
{
	return m_OrbDecreaseData[nUnitIndex].nValue[nLevel];
}

int CHeroLevelTableLoader::GetGuardianDefensePoint( int nUnitIndex, int nLevel )
{
	return m_GuardianDefenseData[nUnitIndex].nValue[nLevel];
}

int CHeroLevelTableLoader::GetGuardianMaxHP( int nUnitIndex, int nLevel )
{
	return m_GuardianHPData[nUnitIndex].nValue[nLevel];
}

int CHeroLevelTableLoader::GetGuardianControlTroopCount( int nUnitIndex, int nLevel )
{
	return m_GuardianControlTroopCountData[nUnitIndex].nValue[nLevel];
}

int CHeroLevelTableLoader::GetGuardianControlTroopUnitCount( int nUnitIndex, int nLevel )
{
	return m_GuardianControlTroopUnitCountData[nUnitIndex].nValue[nLevel];
}

int CHeroLevelTableLoader::GetAttackRange( int nUnitIndex, int nLevel )
{
	return m_AttackRangeData[nUnitIndex].nValue[nLevel];
}

int CHeroLevelTableLoader::Load()
{
	DWORD dwFileSize;
	VOID *pData;

	g_BsKernel.chdir( "sox" );
	char* pFullName = g_BsKernel.GetFullName( "HeroTable.sox" );
	if(FAILED(CBsFileIO::LoadFile(pFullName, &pData, &dwFileSize)))
		BsAssert(0 && "Failed to open sox file");
	BMemoryStream* pStream = new BMemoryStream( pData, dwFileSize );
	g_BsKernel.chdir("..");

#ifdef ENABLE_RES_CHECKER
	g_BsResChecker.AddResList( pFullName );
#endif

	CFcSOXFile Sox( pStream );

	char szLabel[256];
	char szTemp[256];
	IntegerValue ValueStruct;
	std::vector<IntegerValue> *pVecList;

	int nCount = Sox.GetItemCount();
	for( int i=0; i<nCount; i++ ) {
		for( int k=0; k<8; k++ ) {
			switch( k ) {
				case 0:	sprintf( szTemp, "MoveSpeed" );				pVecList = &m_MoveSpeedData;	break;
				case 1:	sprintf( szTemp, "SlotCount" );				pVecList = &m_SlotCountData;	break;
				case 2:	sprintf( szTemp, "OrbDecrease" );			pVecList = &m_OrbDecreaseData;	break;
				case 3:	sprintf( szTemp, "GuardianDefense" );		pVecList = &m_GuardianDefenseData;	break;
				case 4:	sprintf( szTemp, "GuardianHP" );			pVecList = &m_GuardianHPData;	break;
				case 5:	sprintf( szTemp, "GuardianControlTroopCount" );		pVecList = &m_GuardianControlTroopCountData;	break;
				case 6:	sprintf( szTemp, "GuardianControlTroopUnitCount" );	pVecList = &m_GuardianControlTroopUnitCountData;	break;
				case 7:	sprintf( szTemp, "AttackRange" );			pVecList = &m_AttackRangeData;	break;
			}

			for( int j=0; j<MAX_HERO_LEVEL; j++ ) {
				sprintf( szLabel, "%s%d", szTemp, j + 1 );
				
				// mruete: prefix bug 606: added intermediate variable and assert
				CFcSOXFile::SOX_Field * pField = Sox.GetFieldFromLablePtr( i + 1, szLabel );
				BsAssert( NULL != pField );
				ValueStruct.nValue[j] = pField->uData.nValue;
			}
			pVecList->push_back( ValueStruct );
		}
	}

	CBsFileIO::FreeBuffer(pData);
	delete pStream;
	return 1;
}



CItemDescTable::CItemDescTable()
{
}



CItemDescTable::~CItemDescTable()
{
	Clear();
}

void CItemDescTable::Clear()
{
	unsigned int i;
	for(i = 0;i < m_vtDescList.size();i++)
	{
		SAFE_DELETEA( m_vtDescList[i]->pDesc );
		SAFE_DELETEA( m_vtDescList[i]->pName );

		SAFE_DELETE( m_vtDescList[i] );
	}

	m_vtDescList.clear();
#ifdef _SHOW_KOREAN_ITEM_NAME
	for(i = 0;i < m_vtKoreanNames.size();i++ ){	
		SAFE_DELETEA(m_vtKoreanNames[i]);
	}
	m_vtKoreanNames.clear();
#endif
	
}
char *CItemDescTable::GetKoreanName(int nIndex)
{
#ifdef _SHOW_KOREAN_ITEM_NAME
	return m_vtKoreanNames[nIndex];
#endif
	return NULL;

}

void CItemDescTable::Load()
{
	DWORD dwFileSize;
	VOID *pData;

	Clear();

	g_BsKernel.chdir( "Sox" );
	
	char cSoxFileName[256];
	sprintf(cSoxFileName, "%s\\ItemDesc_%s.sox",
		g_LocalLanguage.GetLanguageDir(),
		g_LocalLanguage.GetLanguageStr());

	char *pName = g_BsKernel.GetFullName(cSoxFileName);
	if(FAILED(CBsFileIO::LoadFile(pName, &pData, &dwFileSize)))
		BsAssert(0 && "Failed to open sox file");
	BMemoryStream* pStream = new BMemoryStream( pData, dwFileSize );

#ifdef ENABLE_RES_CHECKER
	g_BsResChecker.AddResList( pName );
#endif
	
	g_BsKernel.chdir("..");

	CFcSOXFile Sox( pStream );
	CFcSOXFile::SOX_Field Field;

	int nCount = Sox.GetItemCount();

	for(int i = 0;i < nCount;i++)
	{
		Sox.GetField(i+1,0,Field);
		if( strcmp( Field.uData.pValue,"noname" ) == 0 ){
			continue;
		}

		ItemDesc *pItemDesc = new ItemDesc;
		size_t pValue_len = strlen( Field.uData.pValue ) + 1; //aleksger - safe string
		pItemDesc->pName = new char[pValue_len];
		strcpy_s(pItemDesc->pName, pValue_len, Field.uData.pValue);
		Sox.GetField(i+1, 1, Field);
		pValue_len = strlen( Field.uData.pValue ) + 1; //aleksger - safe string
		pItemDesc->pDesc = new char[pValue_len];
		strcpy_s( pItemDesc->pDesc, pValue_len, Field.uData.pValue );

		m_vtDescList.push_back( pItemDesc );
	}

	CBsFileIO::FreeBuffer(pData);
	delete pStream;

#ifdef _SHOW_KOREAN_ITEM_NAME
	g_BsKernel.chdir( "Sox" );

	sprintf(cSoxFileName, "%s\\ItemDesc_%s.sox",
		"KOR",
		"KOR");

	pName = g_BsKernel.GetFullName(cSoxFileName);
	if(FAILED(CBsFileIO::LoadFile(pName, &pData, &dwFileSize)))
		BsAssert(0 && "Failed to open sox file");
	pStream = new BMemoryStream( pData, dwFileSize );
	g_BsKernel.chdir("..");

	CFcSOXFile SoxNew( pStream );

	nCount = SoxNew.GetItemCount();

	for(int i = 0;i < nCount;i++)
	{
		SoxNew.GetField(i+1,0,Field);
		if( strcmp( Field.uData.pValue,"noname" ) == 0 ){
			continue;
		}

		size_t pValue_len = strlen( Field.uData.pValue ) + 1; //aleksger - safe string
		char *pBuf = new char[pValue_len];
		strcpy_s(pBuf, pValue_len, Field.uData.pValue);
		m_vtKoreanNames.push_back( pBuf );
	}

	CBsFileIO::FreeBuffer(pData);
	delete pStream;
#endif

}

CItemDropTable::CItemDropTable()
{
}

CItemDropTable::~CItemDropTable()
{
	for(unsigned int i = 0;i < m_vtDropInfoList.size();i++){	
		SAFE_DELETE( m_vtDropInfoList[i] );
	}
}

void CItemDropTable::Load()
{
	DWORD dwFileSize;
	VOID *pData;

	g_BsKernel.chdir( "sox" );
	char *pName = g_BsKernel.GetFullName( "ItemDropTable.sox" );
	if(FAILED(CBsFileIO::LoadFile(pName, &pData, &dwFileSize)))
		BsAssert(0 && "Failed to open sox file");
	BMemoryStream* pStream = new BMemoryStream( pData, dwFileSize );
	g_BsKernel.chdir( ".." );

#ifdef ENABLE_RES_CHECKER
	g_BsResChecker.AddResList( pName );
#endif

	CFcSOXFile Sox( pStream );
	CFcSOXFile::SOX_Field Field1;
	CFcSOXFile::SOX_Field Field2;

	int nCount = Sox.GetItemCount();

	for(int i = 0;i < nCount;i++)
	{
		ItemDropTableInfo *pInfo = new ItemDropTableInfo;

		int nCnt = 0;
		for(int s = 0;s < (ITEM_DROP_TABLE_MAX*2);s++)
		{
			Sox.GetField(i+1,s,Field1);
			Sox.GetField(i+1,s+1,Field2);

			pInfo->nProbt[nCnt]     = Field1.uData.nValue;
			pInfo->nItemIndex[nCnt] = Field2.uData.nValue;	
			s++;
			nCnt++;
		}
		m_vtDropInfoList.push_back( pInfo );		
	}
	CBsFileIO::FreeBuffer(pData);
	delete pStream;
}


CItemLevelTable::CItemLevelTable()
{
}

CItemLevelTable::~CItemLevelTable()
{
	for(unsigned int i = 0;i < m_LevelTable.size();i++)
	{
		delete m_LevelTable[i];
	}
}

void CItemLevelTable::Load()
{
	DWORD dwFileSize;
	VOID *pData;

	g_BsKernel.chdir( "sox" );
	char *pName = g_BsKernel.GetFullName( "ItemLevelTable.sox" );
	if(FAILED(CBsFileIO::LoadFile(pName, &pData, &dwFileSize)))
		BsAssert(0 && "Failed to open sox file");
	BMemoryStream* pStream = new BMemoryStream( pData, dwFileSize );
	g_BsKernel.chdir( ".." );

#ifdef ENABLE_RES_CHECKER
	g_BsResChecker.AddResList( pName );
#endif

	CFcSOXFile Sox( pStream );
	CFcSOXFile::SOX_Field Field;

	int nCount = Sox.GetItemCount();

	for(int i = 0;i < nCount;i++)
	{
		Sox.GetField(i+1,0,Field);
		ITEM_LEVEL_TABLE *pData = new ITEM_LEVEL_TABLE;
		for(int s = 0;s < ITEM_LEVEL_TABLE_MAX;s++)
		{
			Sox.GetField( i+1, s + 1, Field );
			pData->nTableIndex[s] = Field.uData.nValue;			
		}
		m_LevelTable.push_back( pData );
	}
	CBsFileIO::FreeBuffer(pData);
	delete pStream;
}







CItemSetTable::CItemSetTable()
{
}
CItemSetTable::~CItemSetTable()
{
	for(unsigned int i = 0;i < m_SetItemList.size();i++) {
		delete m_SetItemList[i];
	}
}

void CItemSetTable::Load()
{
	DWORD dwFileSize;
	VOID *pData;

	g_BsKernel.chdir( "sox" );
	char* pName = g_BsKernel.GetFullName( "ItemSetTable.sox" );
	if(FAILED(CBsFileIO::LoadFile(pName, &pData, &dwFileSize)))
				BsAssert(0 && "Failed to open sox file");
	BMemoryStream* pStream = new BMemoryStream( pData, dwFileSize );

	g_BsKernel.chdir( ".." );

#ifdef ENABLE_RES_CHECKER
	g_BsResChecker.AddResList( pName );
#endif

	CFcSOXFile Sox( pStream );
	CFcSOXFile::SOX_Field Field;

	int nCount = Sox.GetItemCount(); 

	for( int i = 0;i < nCount;i++ )
	{
		ITEM_SET_INFO *pInfo = new ITEM_SET_INFO;		

		for(int s = 0;s < MAX_SET_TABLE-1;s++)
		{
			Sox.GetField( i+1, s, Field );
			pInfo->nItem[s] = Field.uData.nValue;
		}
		Sox.GetField( i+1, MAX_SET_REQUIRE_ITEM , Field );
		pInfo->nSetItem = Field.uData.nValue;
		Sox.GetField( i+1, MAX_SET_TABLE-1 , Field );
		pInfo->nType = Field.uData.nValue;
		m_SetItemList.push_back( pInfo );
	}
	CBsFileIO::FreeBuffer(pData);
	delete pStream;
}


CSoundPlayCrowdTable::CSoundPlayCrowdTable()
{
}

CSoundPlayCrowdTable::~CSoundPlayCrowdTable()
{
	for(unsigned int i = 0;i < m_SoundCrowdTable.size();i++)
	{
		if(m_SoundCrowdTable[i]->pPlayCue[0]){delete []m_SoundCrowdTable[i]->pPlayCue[0];}
		if(m_SoundCrowdTable[i]->pPlayCue[1]){delete []m_SoundCrowdTable[i]->pPlayCue[1];}
		delete m_SoundCrowdTable[i];
	}
	m_SoundCrowdTable.clear();
}

void CSoundPlayCrowdTable::Load()
{
	DWORD dwFileSize;
	VOID *pData;

	g_BsKernel.chdir( "sox" );
	char* pFullName = g_BsKernel.GetFullName( "SoundPlayCrowdTable.sox" );

	if(FAILED(CBsFileIO::LoadFile(pFullName, &pData, &dwFileSize)))
		BsAssert(0 && "Failed to open sox file");
	BMemoryStream* pStream = new BMemoryStream( pData, dwFileSize );
	g_BsKernel.chdir( ".." );

#ifdef ENABLE_RES_CHECKER
	g_BsResChecker.AddResList( pFullName );
#endif

	CFcSOXFile Sox( pStream );
	CFcSOXFile::SOX_Field Field;

	int nCount = Sox.GetItemCount(); 
	for( int i = 0;i < nCount;i++ )
	{
		Sox.GetField(i+1,0,Field);
		if(Field.uData.nValue == -1)
			continue;

		SOUND_CROWD_INFO *pData = new SOUND_CROWD_INFO;

		pData->nUnitCount = Field.uData.nValue;
		Sox.GetField(i+1,1,Field);
		size_t pValue_len = strlen( Field.uData.pValue ) + 1; //aleksger - safe string
		pData->pPlayCue[0] = new char[pValue_len];
		strcpy_s( pData->pPlayCue[0], pValue_len, Field.uData.pValue );

		Sox.GetField(i+1,2,Field);
		pValue_len = strlen( Field.uData.pValue ) + 1; //aleksger - safe string
		pData->pPlayCue[1] = new char[pValue_len];
		strcpy_s( pData->pPlayCue[1], pValue_len, Field.uData.pValue );

		Sox.GetField(i+1,3,Field);
		pData->UnitType = (CROWD_UNIT_TYPE)Field.uData.nValue;

		m_SoundCrowdTable.push_back( pData );
	}
	CBsFileIO::FreeBuffer(pData);
	delete pStream;
}
//===================================================================================

CSoundPlayTable::CSoundPlayTable()
{
}

void CSoundPlayTable::Load()
{
	DWORD dwFileSize;
	VOID *pData;

	g_BsKernel.chdir( "sox" );
	char* pFullName = g_BsKernel.GetFullName( "SoundPlayTable.sox" );
	if(FAILED(CBsFileIO::LoadFile(pFullName, &pData, &dwFileSize)))
		BsAssert(0 && "Failed to open sox file");
	BMemoryStream* pStream = new BMemoryStream( pData, dwFileSize );
	g_BsKernel.chdir( ".." );

#ifdef ENABLE_RES_CHECKER
	g_BsResChecker.AddResList( pFullName );
#endif

	CFcSOXFile Sox( pStream );
	CFcSOXFile::SOX_Field Field;

	int nCount = Sox.GetItemCount();

	for(int i = 0;i < nCount;i++)
	{
		Sox.GetField(i+1,0,Field);
		if(Field.uData.nValue == -1)
			continue;

		SOUND_PLAY_INFO *pData = new SOUND_PLAY_INFO;
		pData->nCtgy = Field.uData.nValue;
		Sox.GetField( i+1, 1, Field );
		pData->nAmor = Field.uData.nValue;
		Sox.GetField( i+1, 2, Field );
		pData->nFloor = Field.uData.nValue;
		Sox.GetField( i+1, 3, Field );
		pData->nWeapon = Field.uData.nValue;
		Sox.GetField( i+1, 4, Field );
		pData->nEnemyWeapon = Field.uData.nValue;
		Sox.GetField( i+1, 5, Field );
		pData->nWeaponUseType = Field.uData.nValue;
		Sox.GetField( i+1, 6, Field );
		pData->nWeight = Field.uData.nValue;
		Sox.GetField( i+1, 7, Field );
		pData->nAttType = Field.uData.nValue;
		Sox.GetField( i+1, 8, Field );
		pData->nCueID = Field.uData.nValue;
		m_pSoundList.push_back( pData );
	}
	CBsFileIO::FreeBuffer(pData);
	delete pStream;
}



CITemTable::CITemTable()
{
}

CITemTable::~CITemTable()
{
	for(unsigned int i = 0;i < m_ItemDataList.size();i++)
	{
		delete m_ItemDataList[i];
	}
	m_ItemDataList.clear();
}

void CITemTable::SetItemParamValue( FC_ITEM_PARAM_TYPE Type, ItemDataTable *pItemData, int nValue )
{
	switch(Type)
	{
	case TYPE_NONE: break;
	case TYPE_DURATIONTIME:	pItemData->nDurTime = nValue; break;
	case TYPE_EFFECTINTERVAL:	pItemData->nEffectInterval = nValue; break;
	case TYPE_EFFECTAREA:	pItemData->fEfftectArea = (float)nValue; break;
	case TYPE_HP:	pItemData->nHP = nValue; break;
	case TYPE_ORB:	pItemData->nOrb = nValue; break;
	case TYPE_HPMAXADD:	pItemData->nHPMaxAdd = nValue; break;
	case TYPE_ORBMAXADD:	pItemData->nOrbMaxAdd = nValue; break;
	case TYPE_ATTACKPOINT:	pItemData->nAttackPoint = nValue; break;
	case TYPE_DEFENSEPOINT:	pItemData->nDefensePoint = nValue; break;
	case TYPE_CRITICAL:	pItemData->nCriticalProbbt = nValue; break;
	case TYPE_GUARDBREAK:	pItemData->nGuardBreak = nValue; break;
	case TYPE_ITEMGAINPROBBT:	pItemData->nItemGainProbbt = nValue; break;
	case TYPE_EXPGAINSPEED:	pItemData->nExpGainSpeed = nValue; break;
	case TYPE_ORBGAINSPEED:	pItemData->nOrbAttackGainSpeed = nValue; break;
	case TYPE_MOVESPEED:	pItemData->nMoveSpeed = nValue; break;
	case TYPE_TRAPDAMAGE:	pItemData->nGuadianTrapDamage = nValue; break;
	case TYPE_SIEGEDAMAGE:	pItemData->nSiegeDamage = nValue; break;
	case TYPE_PIXISDAMAGE:	pItemData->nPIXISDamage = nValue; break;
	case TYPE_WEAPONREACH:	pItemData->nWeaponReach = nValue; break;
	case TYPE_DEATHPROBBT:	pItemData->nDeathProbbt = nValue; break;
	case TYPE_GUARDIAN_ATTACKPOINT:	pItemData->nGuardianAttackPoint = nValue; break;
	case TYPE_POTIONPROBBT:	pItemData->nPotionProbbt = nValue; break;
	case TYPE_LVUP:	pItemData->LvUp = nValue; break;
	case TYPE_ORBSPARKFREE:	pItemData->bOrbSparkFree = (nValue != 0); break;
	case TYPE_GODMODE:	pItemData->bGodMode = (nValue != 0); break;
	case TYPE_ITEMBAG:	pItemData->nItemBag = nValue; break;
	case TYPE_GUARDIAN_DEFENSE:	pItemData->nGuardianDefensePoint = nValue; break;
	case TYPE_ARROWDEFENSE:	pItemData->nArrowDefense = nValue; break;
	case TYPE_DASHDIST:	pItemData->nDashDist = nValue; break;
	case TYPE_ORBSPLIT:	pItemData->nOrbSplit = nValue; break;
	case TYPE_ORBATTACKKEEPTIME:	pItemData->fOrbAttackKeepTime = (float)nValue; break;
	case TYPE_INPHYYTRAMPLEDAMAGE:	pItemData->bTrampleDamage = (nValue != 0); break;
	case TYPE_REBORN:	pItemData->nReborn = nValue; break;
	case TYPE_EQUIPDUP:	pItemData->bEquipDup = (nValue != 0); break;
	case TYPE_GUARDIAN_MAXHP:	pItemData->nGuadianMaxHPAdd = nValue; break;
	case TYPE_GUARDIAN_SPEEDADD:	pItemData->nGuadianSpeedAdd = nValue; break;
	case TYPE_GUARDIAN_HPADD:	pItemData->nGuadianHPAdd = nValue; break;
	case TYPE_GUARDBREAK_BOSS:	pItemData->nGuardBreakBoss = nValue; break;
	case TYPE_IGNORECRITICAL:	pItemData->bIgnoreCritical = (nValue != 0); break;
	case TYPE_ENABLEDASH:	pItemData->bEnableDash = (nValue != 0); break;
	case TYPE_DAMAGERATIOADD:	pItemData->nDamageRatioAdd = nValue; break;
	case TYPE_ENABLEJUMP:	pItemData->bEnableJump = (nValue != 0); break;
	case TYPE_CRITICALDAMAGERATIO:	pItemData->nCriticalDamageRatio = nValue; break;
	case TYPE_SPECIALATTACKCRITICALPROBBT:	pItemData->nSpecialAttackCriticalProbbt = nValue; break;
	case TYPE_REVIVAL:	pItemData->bRevival = (nValue != 0); break;
	case TYPE_ENABLEDEFENSE:	pItemData->bEnableDefense = (nValue != 0); break;
	case TYPE_NOTUSEORBSPECIALATTACK:	pItemData->bNotUseOrbSpecialAttack = (nValue != 0); break;
	case TYPE_SPECIALATTACKDISTADD:	pItemData->nSpecialAttackDistAdd = nValue; break;
	case TYPE_ALWAYSSMALLDAMAGE:	pItemData->bAlwaysSmallDamage = (nValue != 0); break;
	case TYPE_TRUEORBGAINSPEED:	pItemData->nTrueOrbGainSpeed = nValue; break;
	case TYPE_EXPADD: pItemData->nExpAdd = nValue; break;
	case TYPE_HPMAXADD_INTEGER: pItemData->nHPMaxAddInteger = nValue; break;
	case TYPE_HP_CONDITION_LOWER_PER: pItemData->nHPConditionLowerPer = nValue; break;
	case TYPE_UNITKILL_BLUEPOTION_PROBBT: pItemData->nUnitKillBluePotionProbbt = nValue; break;
	case TYPE_ORBTOHP_FILLANDGONE: pItemData->bOrbToHpFillAndGone = (nValue != 0); break;
	case TYPE_DURATIONTIME_ADD: pItemData->nDurationTimeAdd = nValue; break;
	case TYPE_HP_WITH_GUARDIAN:	pItemData->nHPWithGuardian = nValue; break;
	case TYPE_GODMODE_IN_CHARGE: pItemData->bGodModeInCharge = (nValue != 0); break;
	case TYPE_AUTO_GUARD: pItemData->bAutoGuard = (nValue != 0); break;
	case TYPE_START_FULL_ORB: pItemData->bStartFullOrb = (nValue != 0); break;
	case TYPE_SMALL_BLUEPOTION_FILL_HP: pItemData->nSmallBluePortionFillHP = nValue; break;
	case TYPE_BIG_BLUEPOTION_FILL_HP: pItemData->nBigBluePortionFillHP = nValue; break;
	case TYPE_ORB_FILL_WITH_TRUEORB: pItemData->nOrbFillWithTrueOrb = nValue; break;
	case TYPE_ONLY_DROP_BIG_RED_PORTION: pItemData->bOnlyDropBigRedPortion = (nValue != 0); break;
	case TYPE_SPECIAL_ATTACK_USE_ORB: pItemData->nSpecialAttackUseOrb = nValue; break;
	case TYPE_ORB_ATTACK_POWER_ADD: pItemData->nOrbAttackPowerAdd = nValue; break;
	case TYPE_TRAIL_LENGTH_ADD: pItemData->nTrailLengthAdd = nValue; break;
	case TYPE_AUTO_ORB_ATTACK: pItemData->bAutoOrbAttack = (nValue != 0); break;
	case TYPE_SPECIAL_CRITICAL_RATIO: pItemData->bSpecialCriticalRatio = (nValue != 0); break;
	case TYPE_ONLY_DROP_SMALL_RED_PORTION: pItemData->bOnlyDropSmallRedPortion = (nValue != 0); break;
	case TYPE_DROP_CHANGE_RED_TO_BLUE: pItemData->bDropChangeRedToBlue = (nValue != 0); break;
	case TYPE_CONSUME_ITEM_FILLHP: pItemData->nConsumeItemFillHP = nValue; break;
	case TYPE_ITEM_EQUIP_LEVEL_DOWN: pItemData->nItemEquipLevelDown = nValue; break;
	case TYPE_HP_LOWER_ADD_ORB_GAIN_SPEED: pItemData->nHPLowerAddOrbGainSpeed = nValue; break;
	case TYPE_ITEM_LIFE_TIME_ADD: pItemData->nItemLifeTimeAdd = nValue; break;
	case TYPE_ITEM_SLOT_COUNT_ADD: pItemData->nItemSlotCountAdd = nValue; break;
	case TYPE_AVOID_CRITICAL_RATE: pItemData->nAvoidCriticalRate = nValue; break;
	case TYPE_REVIVAL_HP_CONDITION: pItemData->nRevivalHPCondition = nValue; break;
	case TYPE_ENEMY_RANGE_PROBBT_ADD: pItemData->nEnemyRangeProbbtAdd = nValue; break;
	case TYPE_GUDIAN_RANGE_PROBBT_ADD: pItemData->nGuadianRangeProbbtAdd = nValue; break;
	case TYPE_ORB_SPARK_MAKE_ORB: pItemData->bOrbSparkMakeOrb = (nValue != 0); break;
	case TYPE_INTERVAL_HP_ADD: pItemData->nIntervalHPAdd = nValue; break;

	default:
		BsAssert(0 && "Not defined Item parameter type");
	}

}

void CITemTable::Load()
{
	DWORD dwFileSize;
	VOID *pData;

	g_BsKernel.chdir( "sox" );
	char* pFullName = g_BsKernel.GetFullName( "ItemTable.sox" );
	if(FAILED(CBsFileIO::LoadFile(pFullName, &pData, &dwFileSize)))
		BsAssert(0 && "Failed to open sox file");
	BMemoryStream* pStream = new BMemoryStream( pData, dwFileSize );

	CFcSOXFile Sox( pStream );
	CFcSOXFile::SOX_Field Field;
	g_BsKernel.chdir( ".." );

#ifdef ENABLE_RES_CHECKER
	g_BsResChecker.AddResList( pFullName );
#endif

	int nCount = Sox.GetItemCount();

	FC_ITEM_PARAM_TYPE nParamType = TYPE_NONE;
	int nParamVal = 0;

	for(int i = 0;i < nCount;i++)
	{
		Sox.GetFieldFromLable( i+1, "ItemName", Field );
		BsAssert( NULL != Field.uData.pValue );	// mruete: prefix bug 609: added assert
		if( strcmp( Field.uData.pValue,"noname" ) == 0 ){
			continue;
		}

		ItemDataTable *pData = new ItemDataTable;
		Sox.GetFieldFromLable( i+1, "ItemSkin", Field ); strcpy( pData->szSkinName, Field.uData.pValue );
		Sox.GetFieldFromLable( i+1, "Hero", Field ); pData->nCharSoxID = Field.uData.nValue;
		Sox.GetFieldFromLable( i+1, "ItemType", Field ); pData->nItemType = Field.uData.nValue;
		Sox.GetFieldFromLable( i+1, "ImageFile", Field ); strcpy( pData->szImageFile, Field.uData.pValue );
		Sox.GetFieldFromLable( i+1, "LevelMinimum", Field ); pData->nLevelMinimum = Field.uData.nValue;

		Sox.GetFieldFromLable( i+1, "ParamType1", Field  ); nParamType = (FC_ITEM_PARAM_TYPE)Field.uData.nValue;
		Sox.GetFieldFromLable( i+1, "ParamVal1", Field  ); nParamVal = Field.uData.nValue;
		SetItemParamValue( nParamType, pData ,nParamVal );

		Sox.GetFieldFromLable( i+1, "ParamType2", Field  ); nParamType = (FC_ITEM_PARAM_TYPE)Field.uData.nValue;
		Sox.GetFieldFromLable( i+1, "ParamVal2", Field  ); nParamVal = Field.uData.nValue;
		SetItemParamValue( nParamType, pData ,nParamVal );

		Sox.GetFieldFromLable( i+1, "ParamType3", Field  ); nParamType = (FC_ITEM_PARAM_TYPE)Field.uData.nValue;
		Sox.GetFieldFromLable( i+1, "ParamVal3", Field  ); nParamVal = Field.uData.nValue;
		SetItemParamValue( nParamType, pData ,nParamVal );

		Sox.GetFieldFromLable( i+1, "ParamType4", Field  ); nParamType = (FC_ITEM_PARAM_TYPE)Field.uData.nValue;
		Sox.GetFieldFromLable( i+1, "ParamVal4", Field  ); nParamVal = Field.uData.nValue;
		SetItemParamValue( nParamType, pData ,nParamVal );

		Sox.GetFieldFromLable( i+1, "ParamType5", Field  ); nParamType = (FC_ITEM_PARAM_TYPE)Field.uData.nValue;
		Sox.GetFieldFromLable( i+1, "ParamVal5", Field  ); nParamVal = Field.uData.nValue;
		SetItemParamValue( nParamType, pData ,nParamVal );

		Sox.GetFieldFromLable( i+1, "ParamType6", Field  ); nParamType = (FC_ITEM_PARAM_TYPE)Field.uData.nValue;
		Sox.GetFieldFromLable( i+1, "ParamVal6", Field  ); nParamVal = Field.uData.nValue;
		SetItemParamValue( nParamType, pData ,nParamVal );

		Sox.GetFieldFromLable( i+1, "ParamType7", Field  ); nParamType = (FC_ITEM_PARAM_TYPE)Field.uData.nValue;
		Sox.GetFieldFromLable( i+1, "ParamVal7", Field  ); nParamVal = Field.uData.nValue;
		SetItemParamValue( nParamType, pData ,nParamVal );

		Sox.GetFieldFromLable( i+1, "ItemCondition1", Field  ); pData->nItemCondition[0] = Field.uData.nValue;
		Sox.GetFieldFromLable( i+1, "ItemCondition2", Field  ); pData->nItemCondition[1] = Field.uData.nValue;
		Sox.GetFieldFromLable( i+1, "ItemCondition3", Field  ); pData->nItemCondition[2] = Field.uData.nValue;

		Sox.GetFieldFromLable( i+1, "HeroCondition1", Field  ); pData->nHeroCondition[0] = Field.uData.nValue;
		Sox.GetFieldFromLable( i+1, "HeroCondition2", Field  ); pData->nHeroCondition[1] = Field.uData.nValue;
		Sox.GetFieldFromLable( i+1, "HeroCondition3", Field  ); pData->nHeroCondition[2] = Field.uData.nValue;
		Sox.GetFieldFromLable( i+1, "HeroCondition4", Field  ); pData->nHeroCondition[3] = Field.uData.nValue;
		Sox.GetFieldFromLable( i+1, "HeroCondition5", Field  ); pData->nHeroCondition[4] = Field.uData.nValue;
		Sox.GetFieldFromLable( i+1, "HeroCondition6", Field  ); pData->nHeroCondition[5] = Field.uData.nValue;
		Sox.GetFieldFromLable( i+1, "HeroCondition7", Field  ); pData->nHeroCondition[6] = Field.uData.nValue;

		m_ItemDataList.push_back( pData );
	}
	CBsFileIO::FreeBuffer(pData);
	delete pStream;
}







//---------------------------------------------------------------
//---------------------------------------------------------------
CLocalTextTableLoader::CLocalTextTableLoader()
{

}


CLocalTextTableLoader::~CLocalTextTableLoader()
{
	Release();
}


void CLocalTextTableLoader::Load(BStream *pStream)
{
	Release();

	CFcSOXFile Sox( pStream );
	CFcSOXFile::SOX_Field Field;

	int nCount = Sox.GetItemCount();

	for(int i=0; i<nCount; i++)
	{
		Sox.GetField( i+1, 0, Field );
		int nTextId = Field.uData.nValue;

		if(nTextId == -1){
			continue;
		}

		LocalTextTable *pData = new LocalTextTable;
		pData->nTextId = nTextId;

		Sox.GetField(i+1, 1, Field);
		strcpy( pData->szText, Field.uData.pValue );

		m_ItemDataList.push_back( pData );
	}

}

void CLocalTextTableLoader::Release()
{
	for(DWORD i = 0;i < m_ItemDataList.size();i++)
	{
		delete m_ItemDataList[i];
	}
	m_ItemDataList.clear();
}

char* CLocalTextTableLoader::GetItemData(int nTextId)
{
	for(DWORD i=0; i<m_ItemDataList.size(); i++)
	{	
		LocalTextTable* pInfo = m_ItemDataList[i];
		if(pInfo->nTextId != nTextId){
			continue;
		}

		return pInfo->szText;
	}

	return NULL;
}


//---------------------------------------------------------------
//---------------------------------------------------------------
CAbilityTableLoader::CAbilityTableLoader()
{

}


CAbilityTableLoader::~CAbilityTableLoader()
{
	Release();
}

void CAbilityTableLoader::Release()
{
	for(DWORD i = 0;i < m_ItemDataList.size();i++)
	{
		SAFE_DELETEA( m_ItemDataList[i]->pCaption );
		delete m_ItemDataList[i];
	}
	m_ItemDataList.clear();
}

void CAbilityTableLoader::Load(int nHeroType)
{
	Release();

	char cTemp[256];
	switch(nHeroType)
	{
	case 0: strcpy(cTemp, "As"); break;		//CHAR_ID_ASPHARR:	
	case 1: strcpy(cTemp, "In"); break;		//CHAR_ID_INPHYY:	
	case 2: strcpy(cTemp, "Ty"); break;		//CHAR_ID_TYURRU:	
	case 3: strcpy(cTemp, "Kl"); break;		//CHAR_ID_KLARRANN:	
	case 4: strcpy(cTemp, "My"); break;		//CHAR_ID_MYIFEE:	
	case 5: strcpy(cTemp, "Vi"); break;		//CHAR_ID_VIGKVAGK:	
	case 6:	strcpy(cTemp, "Dw"); break;		//CHAR_ID_DWINGVATT:
	default:
		{
			BsAssert(0);
			return;
		}
	}

	g_BsKernel.chdir( "sox" );
	
	char cSoxFileName[256];
	sprintf(cSoxFileName, "%s\\AbilityLocalTable_%s_%s.sox",
		g_LocalLanguage.GetLanguageDir(),
		cTemp,
		g_LocalLanguage.GetLanguageStr());

	char* pFullName = g_BsKernel.GetFullName(cSoxFileName);

	DWORD dwFileSize;
	VOID *pData;
	if(FAILED(CBsFileIO::LoadFile(pFullName, &pData, &dwFileSize)))
		BsAssert(0 && "Failed to open sox file");
	BMemoryStream* pStream = new BMemoryStream( pData, dwFileSize );
	LoadTable(pStream);
	CBsFileIO::FreeBuffer(pData);
	delete pStream;

#ifdef ENABLE_RES_CHECKER
	g_BsResChecker.AddResList( pFullName );
#endif

	g_BsKernel.chdir("..");
}

void CAbilityTableLoader::LoadTable(BStream *pStream)
{
	CFcSOXFile Sox( pStream );
	CFcSOXFile::SOX_Field Field;

	int nCount = Sox.GetItemCount();

	for(int i=0; i<nCount; i++)
	{
		AbilityLocalTable *pData = new AbilityLocalTable;

		Sox.GetField( i+1, 0, Field );
		pData->nLevel = Field.uData.nValue;

		Sox.GetField( i+1, 1, Field );
		if(Field.uData.pValue != NULL){
			strcpy(pData->cName, Field.uData.pValue);
		}

		Sox.GetField( i+1, 2, Field );
		if(Field.uData.pValue != NULL){
			strcpy(pData->cCommand, Field.uData.pValue);
		}

		Sox.GetField( i+1, 3, Field );
		if(Field.uData.pValue != NULL)
		{
			size_t pValue_len = strlen( Field.uData.pValue ) + 1;
			pData->pCaption = new char[pValue_len];
			strcpy_s(pData->pCaption, pValue_len, Field.uData.pValue);
		}

		m_ItemDataList.push_back( pData );
	}	
}


AbilityLocalTable* CAbilityTableLoader::GetItemData(int nIndex)
{
	if(nIndex >= (int)m_ItemDataList.size()){
		return NULL;
	}

	return m_ItemDataList[nIndex];
}

//---------------------------------------------------------------
//---------------------------------------------------------------
// CTroopSOXLoader class
CTroopSOXLoader::CTroopSOXLoader()
{

}

CTroopSOXLoader::~CTroopSOXLoader()
{
	Clear();
}

bool CTroopSOXLoader::Load()
{
	DWORD dwFileSize;
	VOID *pData;

	g_BsKernel.chdir( "sox" );

	char* pFullName = g_BsKernel.GetFullName("TroopTable.sox");
	if(FAILED(CBsFileIO::LoadFile(pFullName, &pData, &dwFileSize)))
		BsAssert(0 && "Failed to open sox file");
	BMemoryStream* pStream = new BMemoryStream( pData, dwFileSize );

	if( pStream->Valid() == false )
	{
		DebugString( "Cannot load TroopTable SOX\n" );
		BsAssert( 0 );
		return false;
	}

#ifdef ENABLE_RES_CHECKER
	g_BsResChecker.AddResList( pFullName );
#endif
	
	CFcSOXFile Sox( pStream );
	CFcSOXFile::SOX_Field Field;

	m_nDataCount = Sox.GetItemCount();
	BsAssert( m_pData == NULL );
	m_pData = new TroopDataInfo[ m_nDataCount ];
	for( int i = 0; i < m_nDataCount; i++ )
	{
		int j = 0;
		for(j = 0; j < 12; j++ )
		{
			Sox.GetField( i+1, j+1+1, Field);
			m_pData[i].nRangeAttackRange[j] = Field.uData.nValue;
		}
		Sox.GetField( i+1, j+1+1, Field);
		m_pData[i].fMoveSpeed = (float)Field.uData.nValue;
	}

	CBsFileIO::FreeBuffer(pData);
	delete pStream;

	g_BsKernel.chdir("..");
	return true;
}

void CTroopSOXLoader::Clear()
{
	if( m_pData )
	{
		delete[] m_pData;
	}
}

//===================================================================================
CLibraryTableLoader::CLibraryTableLoader()
{

}

CLibraryTableLoader::~CLibraryTableLoader()
{
	Release();
}

void CLibraryTableLoader::Release()
{
	for(DWORD i = 0;i < m_ItemDataList.size();i++)
	{
		delete m_ItemDataList[i];
	}
	m_ItemDataList.clear();
}

void CLibraryTableLoader::Load()
{
	Release();

	DWORD dwFileSize;
	VOID *pData;

	g_BsKernel.chdir( "sox" );

	char* pFullName = g_BsKernel.GetFullName("Library_Char.sox");
	if(FAILED(CBsFileIO::LoadFile(pFullName, &pData, &dwFileSize)))
		BsAssert(0 && "Failed to open sox file");
	BMemoryStream* pStream = new BMemoryStream( pData, dwFileSize );
	LoadTable(pStream);
	CBsFileIO::FreeBuffer(pData);
	delete pStream;

#ifdef ENABLE_RES_CHECKER
	g_BsResChecker.AddResList( pFullName );
#endif

	pFullName = g_BsKernel.GetFullName("Library_Movie.sox");
	if(FAILED(CBsFileIO::LoadFile(pFullName, &pData, &dwFileSize)))
		BsAssert(0 && "Failed to open sox file");
	pStream = new BMemoryStream( pData, dwFileSize );
	LoadTable(pStream);
	CBsFileIO::FreeBuffer(pData);
	delete pStream;

#ifdef ENABLE_RES_CHECKER
	g_BsResChecker.AddResList( pFullName );
#endif

	pFullName = g_BsKernel.GetFullName("Library_Mission.sox");
	if(FAILED(CBsFileIO::LoadFile(pFullName, &pData, &dwFileSize)))
		BsAssert(0 && "Failed to open sox file");
	pStream = new BMemoryStream( pData, dwFileSize );
	LoadTable(pStream);
	CBsFileIO::FreeBuffer(pData);
	delete pStream;

#ifdef ENABLE_RES_CHECKER
	g_BsResChecker.AddResList( pFullName );
#endif

	pFullName = g_BsKernel.GetFullName("Library_Art.sox");
	if(FAILED(CBsFileIO::LoadFile(pFullName, &pData, &dwFileSize)))
		BsAssert(0 && "Failed to open sox file");
	pStream = new BMemoryStream( pData, dwFileSize );
	LoadTable(pStream);
	CBsFileIO::FreeBuffer(pData);
	delete pStream;

#ifdef ENABLE_RES_CHECKER
	g_BsResChecker.AddResList( pFullName );
#endif

	pFullName = g_BsKernel.GetFullName("Library_Music.sox");
	if(FAILED(CBsFileIO::LoadFile(pFullName, &pData, &dwFileSize)))
		BsAssert(0 && "Failed to open sox file");
	pStream = new BMemoryStream( pData, dwFileSize );
	LoadTable(pStream);
	CBsFileIO::FreeBuffer(pData);
	delete pStream;

#ifdef ENABLE_RES_CHECKER
	g_BsResChecker.AddResList( pFullName );
#endif

	g_BsKernel.chdir("..");
}

void CLibraryTableLoader::LoadTable(BStream *pStream)
{
	CFcSOXFile Sox( pStream );
	CFcSOXFile::SOX_Field Field;

	int nCount = Sox.GetItemCount();

	for(int i=0; i<nCount; i++)
	{
		LibraryTable *pData = new LibraryTable;

		Sox.GetField( i+1, 0, Field );
		pData->nID = Field.uData.nValue;

		Sox.GetField( i+1, 1, Field );
		pData->nPoint = Field.uData.nValue;

		Sox.GetField( i+1, 2, Field );
		pData->nCharCondition = Field.uData.nValue;

		Sox.GetField( i+1, 3, Field );
		pData->nStageCondition = Field.uData.nValue;

		Sox.GetField( i+1, 4, Field );
		if(Field.uData.pValue != NULL){
			strcpy(pData->cImageFileName, Field.uData.pValue);
		}

		Sox.GetField( i+1, 5, Field );
		if(Field.uData.pValue != NULL){
			strcpy(pData->cIconFileName, Field.uData.pValue);
		}

		m_ItemDataList.push_back( pData );
	}	
}


LibraryTable* CLibraryTableLoader::GetItemData(int nIndex)
{
	if(nIndex >= (int)m_ItemDataList.size()){
		return NULL;
	}

	return m_ItemDataList[nIndex];
}

//---------------------------------------------------------------
//---------------------------------------------------------------
CLibCharLocalLoader::CLibCharLocalLoader()
{

}


CLibCharLocalLoader::~CLibCharLocalLoader()
{
	Release();
}

void CLibCharLocalLoader::Release()
{
	for(DWORD i = 0;i < m_ItemDataList.size();i++)
	{
		SAFE_DELETEA( m_ItemDataList[i]->pCaption );
		delete m_ItemDataList[i];
	}
	m_ItemDataList.clear();
}

void CLibCharLocalLoader::Load()
{
	Release();

	DWORD dwFileSize;
	VOID *pData;

	g_BsKernel.chdir( "sox" );

	char cSoxFileName[256];
	sprintf(cSoxFileName, "%s\\MenuLibCharInfo_%s.sox",	g_LocalLanguage.GetLanguageDir(),g_LocalLanguage.GetLanguageStr());

	char* pFullName = g_BsKernel.GetFullName(cSoxFileName);
	if(FAILED(CBsFileIO::LoadFile(pFullName, &pData, &dwFileSize)))
		BsAssert(0 && "Failed to open sox file");
	BMemoryStream* pStream = new BMemoryStream( pData, dwFileSize );
	LoadTable(pStream);
	CBsFileIO::FreeBuffer(pData);
	delete pStream;

#ifdef ENABLE_RES_CHECKER
	g_BsResChecker.AddResList( pFullName );
#endif

	g_BsKernel.chdir("..");
}


void CLibCharLocalLoader::LoadTable(BStream *pStream)
{
	CFcSOXFile Sox( pStream );
	CFcSOXFile::SOX_Field Field;

	int nCount = Sox.GetItemCount();

	for(int i=0; i<nCount; i++)
	{
		CharLocalTable *pData = new CharLocalTable;

		Sox.GetField( i+1, 0, Field );
		pData->nUnitID = Field.uData.nValue;

		Sox.GetField( i+1, 1, Field );
		if(Field.uData.pValue != NULL){
			strcpy(pData->cName, Field.uData.pValue);
		}

		Sox.GetField( i+1, 2, Field );
		if(Field.uData.pValue != NULL){
			strcpy(pData->cAge, Field.uData.pValue);
		}
				
		Sox.GetField( i+1, 3, Field );
		if(Field.uData.pValue != NULL){
			strcpy(pData->cPosition, Field.uData.pValue);
		}

		Sox.GetField( i+1, 4, Field );
		if(Field.uData.pValue != NULL){
			strcpy(pData->cWeapon, Field.uData.pValue);
		}

		Sox.GetField( i+1, 5, Field );
		if(Field.uData.pValue != NULL){
			strcpy(pData->cOrbSpark, Field.uData.pValue);
		}

		Sox.GetField( i+1, 6, Field );
		if(Field.uData.pValue != NULL)
		{
			size_t pValue_len = strlen( Field.uData.pValue ) + 1;
			pData->pCaption = new char[pValue_len];
			strcpy_s(pData->pCaption, pValue_len, Field.uData.pValue);
		}

		m_ItemDataList.push_back( pData );
	}	
}

CharLocalTable* CLibCharLocalLoader::GetItemData(int nIndex)
{
	if(nIndex >= (int)m_ItemDataList.size()){
		return NULL;
	}

	return m_ItemDataList[nIndex];
}