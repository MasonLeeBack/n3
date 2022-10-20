#include "StdAfx.h"
#include "FcUtil.h"
#include "DebugUtil.h"
#include "BsKernel.h"
#include "Box3.h"
#include "Ray3.h"
#include "IntLin3Box3.h"
#include "Input.h"
#include "FcItem.h"
#include "SpiritObject_temp.h"		// 임시다..
#include "FcGameObject.h"
#include "FcProjectile.h"
#include "FcDropWeapon.h"
#include "FcTroopObject.h"
#include "FcAIObject.h"
#include "FcAISearchSlot.h"
#include "Data/AIDef.h"

#include "FcProp.h"
#include "FcPropManager.h"
#include "FcFxManager.h"
#include "FcHitMarkMgr.h"
#include "FcEffectLight.h"
#include "FcWorld.h"
#include "FcSOXLoader.h"
#include "AsData.h"
#include ".\\data\\AnimType.h"
#include ".\\data\\ATParamType.h"
#include ".\\data\\SignalType.h"
#include "FcState.h"
#include "FcProp.h"
#include "FcCorrelation.h"
#include "FcUnitObject.h"
#include "FcPhysicsLoader.h"
#include "FcCameraObject.h"
#include "FcGlobal.h"
#include "FcWorld.h"
#include "FcEventSequencer.h"
#include "FcStatusObject.h"

#include "FcInterfaceManager.h"
#include "FcSoundManager.h"
#include "PerfCheck.h"
#include "FcVelocityCtrl.h"

#include ".\\Data\\FcTerrainAttrDefinitions.h"
#include ".\\Data\\\Sound\\FcWaveSoundBankDef.h"
#include ".\\Data\\\Sound\\FcSoundPlayDefinitions.h"

// Include Unit Class
#include "FcUnitObject.h"
#include "FcFlyUnitObject.h"
#include "FcHeroObject.h"
#include "FcHorseObject.h"
#include "FcArcherUnitObject.h"
#include "FcCatapultObject.h"
#include "FcMoveTowerObject.h"
#include "FcAdjutantObject.h"
#include "FcUnit9SObject.h"
// Hero
#include "Aspharr.h"
#include "Inphy.h"
#include "Klarrann.h"
#include "Vigkvagk.h"
#include "Myifee.h"
#include "Dwingvatt.h"
#include "Tyurru.h"
#include "InputPad.h"
#include ".\\Data\\\FxList.h"
#include "FcRealtimeMovie.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG

int CFcGameObject::s_nEffectLightFrame = 0;
int CFcGameObject::s_nIsOrbSparkOn = 0;
int CFcGameObject::s_nUnitStateTable[ ANI_TYPE_NUM ] =
{
	UNIT_RIDABLE | UNIT_DEFENSIBLE,		// ANI_TYPE_DEFAULT
		UNIT_RIDABLE | UNIT_DEFENSIBLE | UNIT_FLOCKABLE | UNIT_MOVABLE | UNIT_ATTACKABLE | UNIT_JUMPABLE | UNIT_STATE_STAY | UNIT_STATE_TRAMPLE,		// ANI_TYPE_STAND
		UNIT_RIDABLE | UNIT_DEFENSIBLE | UNIT_FLOCKABLE | UNIT_MOVABLE | UNIT_ATTACKABLE | UNIT_JUMPABLE | UNIT_STATE_MOVE | UNIT_STATE_TRAMPLE,		// ANI_TYPE_WALK
		UNIT_RIDABLE | UNIT_FLOCKABLE | UNIT_MOVABLE | UNIT_ATTACKABLE | UNIT_JUMPABLE | UNIT_STATE_MOVE | UNIT_STATE_TRAMPLE,		// ANI_TYPE_RUN
		UNIT_RIDABLE | UNIT_STATE_TRAMPLE,		// ANI_TYPE_STOP
		UNIT_STATE_TRAMPLE,		// ANI_TYPE_RIDE_HORSE
		UNIT_FLOCKABLE | UNIT_STATE_DAMAGE | UNIT_STATE_TRAMPLE,		// ANI_TYPE_HIT
		UNIT_FLOCKABLE | UNIT_STATE_DAMAGE | UNIT_STATE_TRAMPLE,		// ANI_TYPE_RUN_HIT
		UNIT_STATE_JUMP | UNIT_STATE_DAMAGE,		// ANI_TYPE_JUMP_HIT
		UNIT_STATE_DOWN | UNIT_STATE_DAMAGE,		// ANI_TYPE_DOWN_HIT
		UNIT_STATE_DOWN | UNIT_STATE_DAMAGE,		// ANI_TYPE_DOWN
		UNIT_STATE_DIE,		// ANI_TYPE_DIE
		UNIT_FLOCKABLE | UNIT_STATE_ATTACK | UNIT_STATE_TRAMPLE,		// ANI_TYPE_ATTACK
		UNIT_STATE_JUMP | UNIT_STATE_ATTACK,		// ANI_TYPE_JUMP_ATTACK
		UNIT_FLOCKABLE | UNIT_STATE_ATTACK | UNIT_STATE_TRAMPLE,		// ANI_TYPE_RUN_ATTACK
		UNIT_STATE_ATTACK | UNIT_STATE_TRAMPLE,		// ANI_TYPE_SPECIAL_ATTACK
		UNIT_STATE_JUMP | UNIT_ATTACKABLE, // ANI_TYPE_JUMP_UP
		UNIT_STATE_JUMP,		// ANI_TYPE_JUMP_DOWN
		UNIT_FLOCKABLE | UNIT_ATTACKABLE | UNIT_JUMPABLE | UNIT_STATE_STAY | UNIT_STATE_TRAMPLE,		// ANI_TYPE_VICTORY
		UNIT_DEFENSIBLE | UNIT_FLOCKABLE | UNIT_MOVABLE | UNIT_ATTACKABLE | UNIT_JUMPABLE | UNIT_STATE_MOVE | UNIT_STATE_TRAMPLE,		// ANI_TYPE_MOVE
		UNIT_DEFENSIBLE | UNIT_FLOCKABLE | UNIT_STATE_STAY | UNIT_STATE_TRAMPLE,		// ANI_TYPE_DEFENSE
		0,		// ANI_TYPE_CUSTOM_0
		0,		// ANI_TYPE_CUSTOM_1
		UNIT_FLOCKABLE,		// ANI_TYPE_RISE
		UNIT_STATE_DOWN | UNIT_STATE_DAMAGE,		// ANI_TYPE_GROUND_HIT
		UNIT_FLOCKABLE,		// ANI_TYPE_DASH
		UNIT_FLOCKABLE | UNIT_STATE_DOWN | UNIT_STATE_DAMAGE | UNIT_STATE_TRAMPLE,		// ANI_TYPE_STUN
};

CFcGameObject::CFcGameObject(CCrossVector *pCross)
: CFcAniObject( pCross, false )
{
	SetClassID( Class_ID_Unknown );

	m_nUnitSOXIndex = -1;
	m_pUnitSOX = NULL;
	m_pASData = NULL;
	m_pUnitInfoData = NULL;
	m_nCurrentWeaponIndex = -1;
	m_HitParam.pHitSignal = NULL;

	m_nCurAniType = 0;
	m_nCurAniTypeIndex = 0;
	m_nAniLoopCount = 1;

	m_MoveVector = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	m_MoveDir = D3DXVECTOR3( 0.f, 0.f, 0.f );
	m_bEnableRotation = true;

	m_nTeam = -1;
	m_nForce = -1;		// Group이 틀리면 적, Group이 똑같으면 Team이 달라도 같은 편
	m_nLevel = 0;

	m_bGodMode = false;
	m_bInvulnerable = false;
	m_bFreezePos = false;
	m_nOnPropIndex = -1;
	m_fGravity = DEFAULT_GRAVITY;
	m_fGroundResist = DEFAULT_GROUND_RESIST;
	SetVelocity( &D3DXVECTOR3( 0, 0, 0 ) );
	m_nGravityChangeFrame = 0;

	m_AniPosToLocalPosFlag = BS_CALC_POSITION_Y;

	m_pCorrelationMng = NULL;

	m_nCross = false;

	m_nTrailHandle[ 0 ] = -1;
	m_nTrailHandle[ 1 ] = -1;
	m_bTrailOn[ 0 ] = false;
	m_bTrailOn[ 1 ] = false;

	m_bApplyFlocking = false;
	m_fMaxFlockingSize = 0.0f;

	m_nMoveLockTick1 = -1;				// 낑겼을 때 필요
	m_nMoveLockTick2 = -1;
	m_nMoveLockPos = D3DXVECTOR2( 0.f, 0.f );

	/*
	// Temp Init
	static int nStaticIntervalCount = 0;
	if( nStaticIntervalCount >= FLOCKING_INTERVAL ) nStaticIntervalCount = 0;
	m_nFlockingInterval = nStaticIntervalCount;
	nStaticIntervalCount++;
	*/

	m_PrevPos = m_Cross.m_PosVector;

	m_LocalAniMove = D3DXVECTOR3( 0.0f, 0.0f, 0.0f );

	m_nDieFrame = 0;
	m_fLastHeight = pCross->m_PosVector.y;
	m_nSoundHandle = -1;

	m_nTroopMoveQueueCount = -1;
	//	m_nDefenseProb = 0;

	m_MoveTargetPos = D3DXVECTOR3( 0.f, 0.f, 0.f );

	m_bDead = false;
	m_bInvincible = false;
	m_pSaveMaterialAmbient = NULL;

	m_bSoxType = false;

	m_pEventSeq = new CFcEventSequencerMng( (void*)this );

	m_fHP = 0.f;
	m_nMaxHPAddPer = 0;
	m_nMaxHPAdd = 0;
	m_nAttackPowerAdd = 0;
	m_nDefenseAdd = 0;
	m_bDropHorse = true;
	m_nCriticalAdd = 0;
	m_nCriticalRatioAdd = 0;
	m_bSpecialCriticalRatio = false;
	m_nExpAdd = 0;
	m_nGuardBreak = 0;
	m_nBossGuardBreak = 0;
	m_nAttackDistAdd = 0;
	m_nTrailLengAdd = 0;
	m_nLevelAdd = 0;
	m_nDefenseProbAdd = 0;
	m_nDamageRatioAdd = 0;
	m_bForceMove = false;
	m_bIsCatchable = true;
	m_bIgnoreCritical = false;
	m_bRevival = false;
	m_nRevivalHPCondition = 0;
	m_bNowSpecialAttack = false;
	m_nSpecialAttackCriticalAdd = 0;
	m_nSpecialAttackDistAdd = 0;
	m_bBoss = false;
	m_bAutoGuard = false;
	m_bForceGenerateOrb = false;
	m_nAvoidCriticalRate = 0;

	m_bOnOrbsparkGodMode = false;

	m_fMoveSpeed = 0.f;
	m_fMoveSpeedRate = 1.f;
	m_fMoveSpeedRateForTroop = 1.f;

	m_bBoost = false;
	m_bIsInScanner = false;
	m_bKeepOrbSpark = false;
	m_bRagdollUnlink = false;
	m_nStunDelay = 0;

	m_nAIControlIndex = -1;
	m_nAIPreloadIndex = -1;
	m_nDropWeaponIndex = -1;
	m_nDropWeaponSkinIndex = -1;

	m_pObjectStatus = NULL;

	m_bMoveDelay = false;

	m_nCurAniLength = 1;

	m_bItemDrop = false;
	m_bExpDrop = false;
	m_bOrbDrop = false;
	m_bDropWeapon = false;

	m_pRotateCtrl = new CFcVelocityCtrl( 3.f, 15.f, 3.f, 1.2f, 0.1f );
	m_nPrevRotationSide = -1;

	m_fRageDisposition = 1.f;

	m_nWayPointStartIndex = 0;
	m_nWayPointNum = 0;
	for( int i=0; i<5; i++ )
		m_WayPoint[i].x = m_WayPoint[i].y = 0.f;

	m_bBeatenByPlayer = false;

	m_nMovePathCnt = 0;
}

CFcGameObject::~CFcGameObject()
{
	int i, nSize;

	if ( m_nDropWeaponSkinIndex != -1 )
	{
		g_BsKernel.ReleaseSkin( m_nDropWeaponSkinIndex );
		m_nDropWeaponSkinIndex = -1;
	}

	nSize = ( int )m_ArmorList.size();
	for( i = 0; i < nSize; i++ )
	{
		if( m_ArmorList[ i ].nObjectIndex != -1 )
		{
			g_BsKernel.SendMessage( m_nEngineIndex, BS_UNLINKOBJECT, m_ArmorList[ i ].nObjectIndex );
			g_BsKernel.DeleteObject( m_ArmorList[ i ].nObjectIndex );
		}
	}
	nSize = ( int )m_WeaponList.size();
	for( i = 0; i < nSize; i++ )
	{
		if( m_WeaponList[ i ].nObjectIndex != -1 )
		{
			g_BsKernel.SendMessage( m_nEngineIndex, BS_UNLINKOBJECT, m_WeaponList[ i ].nObjectIndex );
			g_BsKernel.DeleteObject( m_WeaponList[ i ].nObjectIndex );
		}
	}
	nSize = ( int )m_PhysicsInfoList.size();
	for( i = 0; i < nSize; i++ )
	{
		CFcPhysicsLoader::ClearPhysicsData( m_PhysicsInfoList[i] );
	}

	m_pASData = NULL;
	m_pUnitInfoData = NULL;

	SAFE_DELETE( m_pCorrelationMng );
	SAFE_DELETE( m_pEventSeq );

	// World 에서 빼준다.
	for( i = 0; i < 2; i++ )
	{
		if( m_nTrailHandle[ i ] != -1 )
		{
			// 임시로 빼놓는다...
			g_pFcFXManager->SendMessage( m_nTrailHandle[ i ], FX_DELETE_OBJECT );
			m_nTrailHandle[ i ] = -1;
		}
	}

	// Renew, Disable 같은 경우 그냥 지우기 때문에 AI가 남아있는 경우가 있다.
	if( m_hUnitAIHandle ) delete m_hUnitAIHandle;
	SAFE_DELETE( m_pObjectStatus );
	SAFE_DELETE( m_pRotateCtrl );
}

GameObjHandle CFcGameObject::CreateGameObject( GameObj_ClassID ClassID, CCrossVector *pCross, int nParam )
{
	GameObjHandle Handle;
	switch( ClassID ) {
		// Hero
		case Class_ID_Hero:			Handle = CFcBaseObject::CreateObject< CFcHeroObject >( pCross );	break;
		case Class_ID_Hero_Aspharr:	Handle = CFcBaseObject::CreateObject< CAspharr >( pCross );			break;
		case Class_ID_Hero_Inphy:	Handle = CFcBaseObject::CreateObject< CInphy >( pCross );			break;
		case Class_ID_Hero_Klarrann:Handle = CFcBaseObject::CreateObject< CKlarrann >( pCross );		break;
		case Class_ID_Hero_VigkVagk:Handle = CFcBaseObject::CreateObject< CVigkvagk >( pCross );		break;
		case Class_ID_Hero_Myifee:	Handle = CFcBaseObject::CreateObject< CMyifee >( pCross );			break;
		case Class_ID_Hero_Tyurru:	Handle = CFcBaseObject::CreateObject< CTyurru >( pCross );			break;
		case Class_ID_Hero_Dwingvatt: Handle = CFcBaseObject::CreateObject< CDwingvatt >( pCross );		break;

			// Unit
		case Class_ID_Unit:			
			switch( nParam ) {
		case 65:	Handle = CFcBaseObject::CreateObject< CFcUnit9SObject >( pCross );		break;
		default:	Handle = CFcBaseObject::CreateObject< CFcUnitObject >( pCross );		break;
			}
			break;
		case Class_ID_Fly:			Handle = CFcBaseObject::CreateObject< CFcFlyUnitObject >( pCross );		break;
		case Class_ID_Horse:		Handle = CFcBaseObject::CreateObject< CFcHorseObject >( pCross );		break;
		case Class_ID_Archer:		Handle = CFcBaseObject::CreateObject< CFcArcherUnitObject >( pCross );	break;
		case Class_ID_Catapult:		Handle = CFcBaseObject::CreateObject< CFcCatapultObject >( pCross );	break;
		case Class_ID_MoveTower:	Handle = CFcBaseObject::CreateObject< CFcMoveTowerObject >( pCross );	break;
		case Class_ID_Adjutant:		Handle = CFcAdjutantObject::CreateAdjutantObject( pCross, nParam );		break;

		case Class_ID_Unknown:		Handle = CFcBaseObject::CreateObject< CFcGameObject >( pCross );	break;
	}
	return Handle;
}

void CFcGameObject::Initialize( INITGAMEOBJECTDATA* pData, int nForce, int nTeam, TroopObjHandle hTroop, float fScaleMin, float fScaleMax )
{
	PROFILE_FUNC( "CFcGameObject::Initialize_1" );
	m_nUnitSOXIndex=pData->nUnitIndex;
	m_pUnitSOX=pData->pUnitSOX;

	int nRand = rand() % m_pUnitSOX->cSkinVariationNum;	// 그래픽적인 부분이라서 일반 랜덤쓴다.. by mapping
	m_nEngineIndex=g_BsKernel.CreateAniObjectFromSkin( pData->nSkinIndex[nRand], pData->nAniIndex );
	g_BsKernel.SendMessage(m_nEngineIndex, BS_SET_BILLBOARD_INDEX, pData->nBillboardIndex);
	BsAssert( m_nEngineIndex != -1 ); 

	g_BsKernel.SetCurrentAni( m_nEngineIndex, 0, 0.0f );

	m_pASData = ASData::GetData( pData->nASDataIndex );
	m_pUnitInfoData = CAniInfoData::GetData( pData->nAniInfoDataIndex );

	m_nForce = nForce;
	m_nTeam = nTeam;
	m_hTroop = hTroop;

	if( m_hTroop )
		m_TroopOffsetPos = GetPosV2() - m_hTroop->GetPosV2();
	else
		m_TroopOffsetPos.x = m_TroopOffsetPos.y  = 0.f;

	if( fScaleMin == fScaleMax )
		m_fScale = fScaleMin;
	else
		m_fScale = RandomNumberInRange( fScaleMin, fScaleMax );

	D3DXMatrixScaling( &m_ScaleMat, m_fScale, m_fScale, m_fScale );

	CreateParts( pData ); // 물리용 Actor들을 Scale 시켜줘야 하므로 Scale이 렌더할때뿐 아니라 생성시에도 필요해서 CreateParts 밑으로 내렷습니다.by realgaia

	ChangeAnimation( ANI_TYPE_STAND, 0, 0, true );
	// 에니매이션 틀려뵈게 하려고..
	m_fFrame = (float)Random( m_nCurAniLength );
	m_DummyPos = m_Cross.m_PosVector;

	m_pCorrelationMng = new CFcCorrelationMng( m_Handle, CFcCorrelation::CT_TARGETING_COUNTER | CFcCorrelation::CT_HIT );

	// Second Job Initialize
	if( m_pUnitSOX->nSubJobSOXIndex != -1 ) {
		((CFcUnitJobChange*)this)->Initialize( this, m_pUnitSOX->nSubJobSOXIndex );
	}

	AABB *pBoundingBox;

	pBoundingBox = ( AABB * )g_BsKernel.SendMessage( m_nEngineIndex, BS_GET_BOUNDING_BOX );
	m_UnitSize = *(D3DXVECTOR3*)&(pBoundingBox->Vmax - pBoundingBox->Vmin);

#ifdef _BS_PERFORMANCE_CHECK
	// TODO : 렌더 쓰레드쪽 부하를 확인하기 위해서는 TRUE로 Setting!!!!
	g_BsKernel.SendMessage( m_nEngineIndex, BS_USE_ANI_CACHE, 1 );
#else
	g_BsKernel.SendMessage( m_nEngineIndex, BS_USE_ANI_CACHE, 1 );
#endif

	if( m_pUnitSOX->cBillboardName[0] != NULL )
	{
		g_BsKernel.SendMessage( m_nEngineIndex, BS_ENABLE_BILLBOARD, 1 );		// 빌보드 있는 녀석들
	}

	SetSoxTableType();
}

void CFcGameObject::SetSoxTableType()
{
	if( g_FcWorld.GetHeroHandle() ) {
		if( g_FcWorld.GetHeroHandle()->GetTeam() == m_nTeam ) m_bSoxType = true;
		else m_bSoxType = false;
	}
	else {
		if( m_nTeam == 0 ) m_bSoxType = true;
		else m_bSoxType = false;
	}
}
/*
void CFcGameObject::Initialize(int nUnitIndex, int nForce, int nTeam, TroopObjHandle hTroop, float fScaleMin, float fScaleMax)
{
PROFILE_FUNC( "CFcGameObject::Initialize_2" );
int nRand, nSkinIndex, nAniIndex;
char szFileName[_MAX_PATH];

BsAssert( nUnitIndex != -1 );

m_nUnitSOXIndex=nUnitIndex;
m_pUnitSOX=CUnitSOXLoader::GetInstance().GetUnitData(nUnitIndex);

BsAssert( m_pUnitSOX->cSkinVariationNum > 0 && "Invalid unit type" );

int nSkinLevel = GetSkinLevel( nType, pTroopInfo->m_troopAttr.nLeaderLevel );

nRand = Random( m_pUnitSOX->cSkinVariationNum ) + 1;
sprintf(szFileName, "%s%s%d.skin", m_pUnitSOX->cSkinDir, m_pUnitSOX->cSkinFileName, nRand);

nSkinIndex=g_BsKernel.LoadSkin(-1, szFileName);
nAniIndex=g_BsKernel.LoadAni(-1, m_pUnitSOX->cAniName);
m_nEngineIndex=g_BsKernel.CreateAniObjectFromSkin(nSkinIndex, nAniIndex);
BsAssert( m_nEngineIndex != -1 );

g_BsKernel.ReleaseSkin( nSkinIndex );

// 애니메이션이 초기화가 안되서 다운되길래 임시로 넣었음 yooty
g_BsKernel.SetCurrentAni( m_nEngineIndex, 0, 0.0f );

sprintf(szFileName, "as\\%s", m_pUnitSOX->cASFileName );
m_pASData = ASData::LoadASData( szFileName );

sprintf(szFileName, "as\\%s", m_pUnitSOX->cUnitInfoFileName );
m_pUnitInfoData = CAniInfoData::LoadAniInfoData( szFileName );
//CreateParts();

m_nTeam = nTeam;
m_nForce = nForce;
m_hTroop = hTroop;

if( m_hTroop )
{
m_TroopOffsetPos = GetPosV2() - m_hTroop->GetPosV2();
}
else
{
m_TroopOffsetPos.x = m_TroopOffsetPos.y  = 0.f;
}

if( fScaleMin == fScaleMax )
{
m_fScale = fScaleMin;
}
else{
m_fScale = RandomNumberInRange( fScaleMin, fScaleMax );
}
D3DXMatrixScaling( &m_ScaleMat, m_fScale, m_fScale, m_fScale );
CreateParts(); // 물리용 Actor들을 Scale 시켜줘야 하므로 Scale이 렌더할때뿐 아니라 생성시에도 필요해서 CreateParts 밑으로 내렷습니다.by realgaia
ChangeAnimation( ANI_TYPE_STAND, 0 );
m_DummyPos = m_Cross.m_PosVector;
m_pCorrelationMng = new CFcCorrelationMng( m_Handle, CFcCorrelation::CT_TARGETING_COUNTER | CFcCorrelation::CT_HIT );
// Second Job Initialize
if( m_pUnitSOX->nSubJobSOXIndex != -1 ) {
((CFcUnitJobChange*)this)->Initialize( this, m_pUnitSOX->nSubJobSOXIndex );
}
AABB *pBoundingBox;

pBoundingBox = ( AABB * )g_BsKernel.SendMessage( m_nEngineIndex, BS_GET_BOUNDING_BOX );
m_UnitSize = *(D3DXVECTOR3*)&(pBoundingBox->Vmax - pBoundingBox->Vmin);
SetSoxTableType();

g_BsKernel.SendMessage( m_nEngineIndex, BS_USE_ANI_CACHE, 1 );
}
*/
/*
void CFcGameObject::CreateParts()
{
int i, nPartsGroupCount, nPartsSkinIndex, nSelectIndex, nPartsObjectIndex;
ASPartsData *pPartsData;
WEAPON_OBJECT Weapon;
ARMOR_OBJECT Armor;
int nArmorRandom[ 20 ]; // Armor 의 그룹 맥스를 20정도로 셋팅한다..

for( i = 0; i < 20; i++ )
{
nArmorRandom[ i ] = -1;
}
nPartsGroupCount = m_pUnitInfoData->GetPartsGroupCount();
for( i = 0; i < nPartsGroupCount; i++ )
{
pPartsData = m_pUnitInfoData->GetPartsGroupInfo( i );

switch( pPartsData->m_nType - 1 ) {
case PARTS_TYPE_WEAPON:
case PARTS_TYPE_PROJECTILE:
nSelectIndex = Random( pPartsData->GetPartsCount() );
break;
case PARTS_TYPE_ARMOR:
if( nArmorRandom[ pPartsData->m_nGroup - 1 ] == -1 )
{
nArmorRandom[ pPartsData->m_nGroup - 1 ] = Random( pPartsData->GetPartsCount() );
}
nSelectIndex = nArmorRandom[ pPartsData->m_nGroup - 1 ];
break;
default:
continue;
}		

D3DXMATRIX matObject = *m_Cross;
D3DXMatrixMultiply(&matObject, &m_ScaleMat, &matObject);

if( pPartsData->m_nSimulation == PHYSICS_CLOTH ) {            

int nSimulParts = pPartsData->GetPartsCount();

g_BsKernel.SetPreCalcAni( m_nEngineIndex, true);

for( int i = 0; i < nSimulParts; i++ ) {

char szPhysicsFileName[255];
strcpy( szPhysicsFileName, pPartsData->GetPartsSkinName( i ) );
RemoveEXT( szPhysicsFileName );
strcat( szPhysicsFileName, ".txt" );

PHYSICS_DATA_CONTAINER *pContainer = CFcPhysicsLoader::LoadPhysicsData(szPhysicsFileName);		
m_PhysicsInfoList.push_back( pContainer );
nPartsSkinIndex = g_BsKernel.LoadSkin( -1, pPartsData->GetPartsSkinName( i ) );
nPartsObjectIndex = g_BsKernel.CreateClothObjectFromSkin( nPartsSkinIndex , pContainer, &matObject);

g_BsKernel.ReleaseSkin( nPartsSkinIndex );

g_BsKernel.SendMessage( nPartsObjectIndex, BS_PHYSICS_LINK_CHARACTER, m_nEngineIndex);			
//g_BsKernel.SendMessage( m_nEngineIndex, BS_LINKOBJECT, 0, nPartsObjectIndex);
ARMOR_OBJECT Armor;
Armor.nObjectIndex = nPartsObjectIndex;
Armor.nLinkBoneIndex = -1;
m_ArmorList.push_back( Armor );	
}
}
else if( pPartsData->m_nSimulation == PHYSICS_COLLISION) {
CreateCollisionMesh( pPartsData->GetPartsSkinName( 0 ) );
}
else if( pPartsData->m_nSimulation == PHYSICS_RAGDOLL) {
CreateRagdoll( pPartsData->GetPartsSkinName( 0 ) );
}
else {
int nBoneIndex;

g_BsKernel.SetInstancingBufferSize( 100 );
nPartsSkinIndex = g_BsKernel.LoadSkin( -1, pPartsData->GetPartsSkinName( nSelectIndex ) );
g_BsKernel.SetInstancingBufferSize( 5 );
nPartsObjectIndex = g_BsKernel.CreateStaticObjectFromSkin( nPartsSkinIndex );

g_BsKernel.ReleaseSkin( nPartsSkinIndex );

BsAssert( nPartsObjectIndex >= 0 );
nBoneIndex = g_BsKernel.SendMessage( m_nEngineIndex, BS_GET_BONE_INDEX, ( DWORD )pPartsData->GetPartsBoneName() );
g_BsKernel.SendMessage( m_nEngineIndex, BS_LINKOBJECT, ( DWORD )nBoneIndex, nPartsObjectIndex );

if( pPartsData->m_nType - 1 == PARTS_TYPE_ARMOR )
{
Armor.nObjectIndex = nPartsObjectIndex;
Armor.nLinkBoneIndex = nBoneIndex;
m_ArmorList.push_back( Armor );
}
else
{
Weapon.nGroup = pPartsData->m_nGroup - 1;
Weapon.nObjectIndex = nPartsObjectIndex;
if( pPartsData->m_nGroup - 1 == 0 )
{
Weapon.bShow = true;
}
else
{
Weapon.bShow = false;
}
Weapon.nLinkBoneIndex = nBoneIndex;
Weapon.nPartsIndex = i;
m_WeaponList.push_back( Weapon );
}
m_nCurrentWeaponIndex = 0;
}
}

int nSize;

nSize = ( int )m_ArmorList.size();
for( i = 0; i < nSize; i++ )
{
g_BsKernel.ShowObject( m_ArmorList[ i ].nObjectIndex, true );
}
nSize = ( int )m_WeaponList.size();
for( i = 0; i < nSize; i++ )
{
#pragma warning(disable:4800)
g_BsKernel.ShowObject( m_WeaponList[ i ].nObjectIndex, ( bool )( m_WeaponList[ i ].bShow ) );
#pragma warning(default:4800)
}
}
*/

ASPartsData *g_pDebugPartsData = NULL;

void CFcGameObject::CreateParts( INITGAMEOBJECTDATA* pData )
{
	int nPartsGroupCount, nSelectIndex, nPartsObjectIndex;
	ASPartsData *pPartsData;
	WEAPON_OBJECT Weapon;
	ARMOR_OBJECT Armor;
	int nArmorRandom[ 20 ]; // Armor 의 그룹 맥스를 20정도로 셋팅한다..

	int i;
	for( i = 0; i < 20; i++ ) {
		nArmorRandom[ i ] = -1;
	}

	nPartsGroupCount = m_pUnitInfoData->GetPartsGroupCount();
	for(int nOuter = 0; nOuter < nPartsGroupCount; nOuter++ ) {
		pPartsData = m_pUnitInfoData->GetPartsGroupInfo( nOuter );

		switch( pPartsData->m_nType - 1 ) {
			case PARTS_TYPE_WEAPON:
			case PARTS_TYPE_PROJECTILE:
				nSelectIndex = rand() % pPartsData->GetPartsCount();	// 그래픽적인 부분이라서 일반 랜덤쓴다.. by mapping
				break;
			case PARTS_TYPE_ARMOR:
				if( nArmorRandom[ pPartsData->m_nGroup - 1 ] == -1 )
				{
					nArmorRandom[ pPartsData->m_nGroup - 1 ] = rand() % pPartsData->GetPartsCount();	// 그래픽적인 부분이라서 일반 랜덤쓴다.. by mapping
				}
				nSelectIndex = nArmorRandom[ pPartsData->m_nGroup - 1 ];
				break;
			default:
				continue;
		}

		D3DXMATRIX matObject = *m_Cross;
		D3DXMatrixMultiply(&matObject, &m_ScaleMat, &matObject);

		if( pPartsData->m_nSimulation == PHYSICS_CLOTH ) {

			int nSimulParts = pPartsData->GetPartsCount();

			g_BsKernel.SetPreCalcAni( m_nEngineIndex, true);

			int nStartIndex = 0;
			if( pData->nSkinLevel >= 0 ) {
				BsAssert( pData->nSkinLevel == 0 || pData->nSkinLevel == 3);
				nStartIndex = nSimulParts * pData->nSkinLevel;
			}
			for( int i = nStartIndex; i < nSimulParts + nStartIndex; i++ ) {
				PHYSICS_DATA_CONTAINER *pContainer = NULL;
				int nCnt = pData->vecPhysicsInfo.size();

				for( int j=0; j<nCnt; j++ ) {
					PHYSICS_INIT_INFO* pInfo = &(pData->vecPhysicsInfo[j]);
					if( pInfo->nSkinIndex == pPartsData->m_pSkinIndex[i] ) {
						pContainer = pInfo->pData;
						break;
					}
				}
#ifdef _XBOX
				BsAssert( pContainer );
#endif //_XBOX
				if( pContainer == NULL ) continue;

				nPartsObjectIndex = g_BsKernel.CreateClothObjectFromSkin( pPartsData->m_pSkinIndex[i], pContainer, &matObject);
				g_BsKernel.SendMessage( nPartsObjectIndex, BS_PHYSICS_LINK_CHARACTER, m_nEngineIndex);
				//g_BsKernel.SendMessage( m_nEngineIndex, BS_LINKOBJECT, 0, nPartsObjectIndex);

				ARMOR_OBJECT Armor;
				Armor.nObjectIndex = nPartsObjectIndex;
				Armor.nLinkBoneIndex = -1;
				Armor.bUsePhysics = true;
				m_ArmorList.push_back( Armor );	
			}
		}
		else if( pPartsData->m_nSimulation == PHYSICS_COLLISION) {
#ifdef _XBOX			
			SetCollisionMesh( pData->pCollisionMesh );			
#endif //_XBOX
		}
		else if( pPartsData->m_nSimulation == PHYSICS_RAGDOLL) {
			SetRagdoll( pData->pRagdollSetting );
		}
		else if (pPartsData->m_nSimulation == 0)
		{
			int nBoneIndex;

			g_pDebugPartsData = pPartsData;
			BsAssert( pPartsData->m_pSkinIndex[nSelectIndex] >= 0 );

			nPartsObjectIndex = g_BsKernel.CreateStaticObjectFromSkin( pPartsData->m_pSkinIndex[nSelectIndex] );
			BsAssert( nPartsObjectIndex >= 0 );
			if( nPartsObjectIndex == -1 )
			{
				continue;
			}
			nBoneIndex = g_BsKernel.SendMessage( m_nEngineIndex, BS_GET_BONE_INDEX, ( DWORD )pPartsData->GetPartsBoneName() );

			g_BsKernel.SendMessage( m_nEngineIndex, BS_LINKOBJECT, ( DWORD )nBoneIndex, nPartsObjectIndex );

			if( pPartsData->m_nType - 1 == PARTS_TYPE_ARMOR )
			{
				Armor.nObjectIndex = nPartsObjectIndex;
				Armor.nLinkBoneIndex = nBoneIndex;
				Armor.bUsePhysics = false;
				m_ArmorList.push_back( Armor );
			}
			else
			{
				Weapon.nGroup = pPartsData->m_nGroup - 1;
				Weapon.nObjectIndex = nPartsObjectIndex;
				if( pPartsData->m_nGroup - 1 == 0 )
				{
					Weapon.bShow = true;
				}
				else
				{
					Weapon.bShow = false;
				}
				Weapon.nEngineIndex = pPartsData->m_pSkinIndex[nSelectIndex];
				Weapon.nLinkBoneIndex = nBoneIndex;
				Weapon.nPartsIndex = nOuter;
				m_WeaponList.push_back( Weapon );
			}
			m_nCurrentWeaponIndex = 0;
		}
	}

	int nSize = ( int )m_ArmorList.size();
	for( i = 0; i < nSize; i++ )
	{
		g_BsKernel.ShowObject( m_ArmorList[ i ].nObjectIndex, true );
	}
	nSize = ( int )m_WeaponList.size();
	for( i = 0; i < nSize; i++ )
	{
#pragma warning(disable:4800)
		g_BsKernel.ShowObject( m_WeaponList[ i ].nObjectIndex, ( bool )( m_WeaponList[ i ].bShow ) );
#pragma warning(default:4800)
	}
}



float CFcGameObject::GetFrameAdd()
{
	return m_fFrameAdd;
}

void CFcGameObject::CmdPlace( float nX, float nY )
{
	if( m_bDead == true ) return;
	D3DXVECTOR3 vTemp, vCurPos;

	if( m_bForceMove == true )
		vCurPos = m_Cross.m_PosVector + m_MoveVector;
	else
		vCurPos = m_Cross.m_PosVector;

	vTemp = D3DXVECTOR3( nX, 0, nY ) - vCurPos;

	Move( &vTemp );
	m_bForceMove = true;
	ResetMoveVector();
	if( m_fHP > 0.f )
		CmdStop( ANI_TYPE_STAND );
	ResetWayPoint();
}

void CFcGameObject::SetCrossVector( CCrossVector *pCross )
{
	// 이거 해주면 안될꺼 가튼데..일단 빼놀께요.. By Siva

	m_Cross = *pCross;
	CFcWorld::GetInstance().MoveUnitToScanner(m_Handle, &m_Cross.m_PosVector);
}

void CFcGameObject::Process()
{
	ProcessObjectStatus();
	if( m_bDead == false )
	{
		if( ( m_fHP <= 0.f ) || ( m_nDieFrame ) )
			m_bDead = true;
	}

	if( m_nDieFrame )
	{
		m_nDieFrame++;
		/*		if( m_nDieFrame > DIE_KEEP_FRAME )
		{
		return;
		}*/
	}
	if( m_nRagDollMode )
	{
		if( m_bRagdollUnlink )
		{
			D3DXVECTOR3 RagVel;

			g_BsKernel.SendMessage( m_nEngineIndex, BS_GET_RAGDOLL_POS_VELOCITY, ( DWORD )&m_Cross.m_PosVector, ( DWORD )&RagVel );
			if( D3DXVec3Length( &RagVel ) > 10.0f )
			{
				g_FcWorld.GiveRangeDamage( g_FcWorld.GetHeroHandle(), &m_Cross.m_PosVector, 100.0f, 50, 
					15, -20, g_FcWorld.GetHeroHandle()->GetTeam() );
				g_FcWorld.GivePhysicsRange( m_Handle, &m_Cross.m_PosVector, 100.0f, 10.0f, 300, PROP_BREAK_TYPE_NORMAL );
			}
		}
		return;
	}

	if( m_nStunDelay > 0 ) {
		--m_nStunDelay;
		if( m_nStunDelay == 0 ) {
			NextAnimation(); 
		}
	}

	CFcAniObject::Process();
	ProcessSignal();

	g_BsKernel.SetCalcAniFlag( m_nEngineIndex, m_AniPosToLocalPosFlag );

	if( (GetProcessTick() % FRAME_PER_SEC == m_nEngineIndex % FRAME_PER_SEC) && s_nUnitStateTable[ m_nCurAniType ] & UNIT_MOVABLE )
	{
		CheckMoveLock( m_hTroop );		// 낑겼을 때 빠져나오게 하는 것
		CheckUnitSpeed( m_hTroop );		// 부대를 못 따라 오는 유닛은 미묘하게 속도를 높여준다.
	}

	ProcessSound();
	m_pEventSeq->Process();

	/*
	if( m_bDead == true && m_nCurAniType != ANI_TYPE_DIE ) {
	char szStr[128];
	sprintf( szStr, "No Die!!! ( UnitID : %d, AniAttr : %d, AniType : %d, AniIndex : %d ), %d, %d\n", m_nUnitSOXIndex, m_nCurAniAttr, m_nCurAniType, m_nCurAniTypeIndex, m_fHP, m_nDieFrame );	
	g_BsKernel.AddConsoleString( szStr );
	DebugString( szStr );
	}
	if( m_bDead == false && GetTroop()->GetState() == TROOPSTATE_ELIMINATED ) {
	char szStr[128];
	sprintf( szStr, "Troop Eliminated ( UnitID : %d, AniAttr : %d, AniType : %d, AniIndex : %d ), %d, %d\n", m_nUnitSOXIndex, m_nCurAniAttr, m_nCurAniType, m_nCurAniTypeIndex, m_fHP, m_nDieFrame );	
	g_BsKernel.AddConsoleString( szStr );
	DebugString( szStr );
	}
	*/

	/*
	if( m_hUnitAIHandle && m_hUnitAIHandle->IsEnable() ) {
	if( m_hUnitAIHandle->IsDestroyThis() ) {
	m_hUnitAIHandle.Identity();
	delete m_hUnitAIHandle;
	return;
	}
	else {
	m_hUnitAIHandle->Process();
	}
	}
	*/
}

void CFcGameObject::PostProcess()
{
	if( m_nDieFrame <= DIE_KEEP_FRAME && !m_nRagDollMode ) {
		RotationProcess();
		PositionProcess();
		HitProcess();
	}
	if( m_nDieFrame > DIE_KEEP_FRAME + DIE_ALPHA_FRAME_INT )
	{
		if( GetClassID() != Class_ID_Hero )
		{
			Delete();
			if( m_hTroop )
				m_hTroop->RemoveUnit( m_Handle );
		}
	}
	if( m_bIsInScanner ) {
		CFcWorld::GetInstance().MoveUnitToScanner(m_Handle, &m_PrevPos);
		m_PrevPos = m_Cross.m_PosVector;
		if( m_nDieFrame > DIE_KEEP_FRAME || m_nRagDollMode ) {	
			SetInScanner( false );
			return;
		}
	}

	if( m_nDieFrame > DIE_KEEP_FRAME || m_nRagDollMode ) return;

	m_HitParam.nHitRemainFrame -= ( int )GetFrameAdd();

	// 이 값이 - 가 되서 추가 yooty
	if( m_HitParam.nHitRemainFrame < 0 )
	{
		m_HitParam.nHitRemainFrame = 0;
	}

	CFcAniObject::PostProcess();
	/*
	if( m_ClassID == Class_ID_Hero && ((CFcHeroObject*)this)->GetHeroClassID() == Class_ID_Hero_Inphy ) {
	DebugString( "3 Pos : %.2f, %.2f, %.2f, Target : %.2f, %.2f, %.2f, AniType : %d\n", m_Cross.m_PosVector.x, m_Cross.m_PosVector.y, m_Cross.m_PosVector.z, m_MoveTargetPos.x, m_MoveTargetPos.y, m_MoveTargetPos.z, m_nCurAniType );
	}
	*/
}

void CFcGameObject::CheckGroundVelocity()
{
	D3DXVECTOR3 Velocity;
	float fLength;

	Velocity = *GetVelocity();
	Velocity.y = 0.0f;
	fLength = D3DXVec3Length( &Velocity );
	if( fLength <= 0.0f )
	{
		return;
	}
	D3DXVec3Normalize( &Velocity, &Velocity );
	fLength -= m_fGroundResist;
	if( fLength < 0.0f )
	{
		fLength = 0.0f;
	}
	Velocity *= fLength;
	SetVelocityX( Velocity.x );
	SetVelocityZ( Velocity.z );
}

void CFcGameObject::SetFlocking( bool bFlag, float fMaxSize )
{
	m_bApplyFlocking = bFlag;
	if( m_bApplyFlocking == false ) {
		m_fMaxFlockingSize = 0.0f;
		return;
	}
	m_fMaxFlockingSize = fMaxSize;
	//	if( m_nMaxFlockingSize < nMaxSize ) m_nMaxFlockingSize = nMaxSize;

}

void CFcGameObject::ProcessFlocking()
{
	/*
	if( m_bDead ) return;

	m_nFlockingInterval++;
	if( m_nFlockingInterval % FLOCKING_INTERVAL == 0 ) {
	float fScanRange = GetUnitRadius() * 3;
	m_VecFlocking.erase( m_VecFlocking.begin(), m_VecFlocking.end() );
	CFcWorld::GetInstance().GetObjectListInRange( (D3DXVECTOR3*)&GetPos(), fScanRange, m_VecFlocking );
	}
	D3DXVECTOR3 Direction;

	float fSumRadius;
	for( int i = 0; i <(int)m_VecFlocking.size(); i++ ) {
	if( !IsValid( m_VecFlocking[i] ) ) continue;
	if( !m_VecFlocking[i]->IsFlockable( m_Handle ) ) continue;

	Direction = m_VecFlocking[i]->GetPos() - GetPos();
	Direction.y = 0.f;
	fSumRadius = GetUnitRadius() + m_VecFlocking[ i ]->GetUnitRadius();
	if( D3DXVec3LengthSq( &Direction ) > fSumRadius * fSumRadius ) continue;

	D3DXVec3Normalize( &Direction, &Direction );
	m_VecFlocking[i]->Move( &( Direction * 3.f ) );
	m_VecFlocking[i]->SetFlocking( true, fSumRadius );
	}
	*/
}


float fDebugX=0, fDebugY=0;
float fDebugMX=0, fDebugMY=0;
float fDebugMX2=0, fDebugMY2=0;
float fDebugMX3=0, fDebugMY3=0;


void CFcGameObject::PositionProcess()
{
	int nTick;

	nTick = GetProcessTick();

	if( m_bFreezePos ) return;
	ApplyAniPositionToLocalPosition();

	// 음..이거 위치가 애매..
	PostProcessObjectStatus();

	fDebugMX2 = m_MoveVector.x;
	fDebugMY2 = m_MoveVector.z;

	m_MoveVector.x += GetVelocityX();
	m_MoveVector.z += GetVelocityZ();

	if( m_MoveVector.x != 0.f || m_MoveVector.z != 0.f )
	{
		fDebugMX = m_MoveVector.x;
		fDebugMY = m_MoveVector.z;
		fDebugX = m_Cross.m_PosVector.x;
		fDebugY = m_Cross.m_PosVector.z;


		m_fLastHeight = AddPos( m_MoveVector.x, m_MoveVector.y, m_MoveVector.z );
		fDebugMX3 = m_Cross.m_PosVector.x;
		fDebugMY3 = m_Cross.m_PosVector.z;
 		m_MoveVector=D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	}

	SetVelocityY( GetVelocityY() + m_fGravity );

	m_Cross.m_PosVector.y += GetVelocityY();
	if( m_nGravityChangeFrame )
	{
		m_nGravityChangeFrame--;
		if( m_nGravityChangeFrame <= 0 )
		{
			m_nGravityChangeFrame = 0;
			m_fGravity = DEFAULT_GRAVITY;
		}
	}

	if( m_Cross.m_PosVector.y < m_fLastHeight ){
		if( ( m_fGravity != 0.0f ) || ( GetVelocityY() <= 0.0f ) )
		{
			m_Cross.m_PosVector.y = m_fLastHeight;
			SetVelocityY( 0.0f );
			m_fGravity = DEFAULT_GRAVITY;
			/*			if( ( m_nCurAniType == ANI_TYPE_JUMP_UP ) || ( m_nCurAniType == ANI_TYPE_JUMP_DOWN ) 
			//				|| ( m_nCurAniType == ANI_TYPE_DOWN ) || ( m_nCurAniType == ANI_TYPE_DOWN_HIT )
			|| ( m_nCurAniType == ANI_TYPE_JUMP_ATTACK ) )
			{
			NextAnimation();
			}*/
		}
	}
	//	if( nTick % GetTickInterval() == 0 )
	//	{
	//		ProcessFlocking();
	PROFILE_TIME_TEST( ProcessFlocking() );
	//	}
	if( ( float )fabs( m_fLastHeight - m_Cross.m_PosVector.y ) < 10.0f )
	{
		CheckGroundVelocity();
	}
	/*
	if( m_ParentLinkObjHandle )
	{
	// 말타고 있을때는 현재 위치가.. 말에 의해서 변하기 땜에.. Cross벡터 갱신해준다..
	D3DXMATRIX matTemp = *( D3DXMATRIX * )g_BsKernel.SendMessage( m_nEngineIndex, BS_GET_OBJECT_MAT );
	if( matTemp._41 != 0.f && matTemp._42 != 0.f && matTemp._43 != 0.f ) {
	m_Cross = matTemp;
	D3DXVec3Normalize( &m_Cross.m_ZVector, &m_Cross.m_ZVector );
	m_Cross.m_YVector = D3DXVECTOR3( 0.f, 1.f, 0.f );
	m_Cross.UpdateVectorsY();
	}
	}
	*/
}

void CFcGameObject::RotationProcess()
{
	float fDot, fLength;
	D3DXVECTOR3 CrossVec;

	if( !m_bEnableRotation )
	{
		m_bEnableRotation = true;
		return;
	}
	fLength=D3DXVec3LengthSq(&m_MoveDir);
	if(fLength<=0.0f){
		m_pRotateCtrl->Reset();
		m_nPrevRotationSide = -1;
		return;
	}

	fDot = D3DXVec3Dot( &m_MoveDir, &(m_Cross.m_ZVector) );
	float fAngle = acos( fDot ) * 180.f / 3.1415926f;
	if( fAngle > m_pRotateCtrl->GetMinSpeed() ) {
		D3DXVec3Cross(&CrossVec, &m_MoveDir, &(m_Cross.m_ZVector));
		float fSpeed;
		if( m_nPrevRotationSide != -1 ) {
			if( CrossVec.y > 0.f && m_nPrevRotationSide == 1 ) m_pRotateCtrl->Reset();
			else if( CrossVec.y <= 0.f && m_nPrevRotationSide == 0 ) m_pRotateCtrl->Reset();
		}
		fSpeed = m_pRotateCtrl->GetSpeed( CFcVelocityCtrl::ACCELERATION );
		if( CrossVec.y > 0 ) {
			m_Cross.RotateYaw( (int)( fSpeed * 2.84f ) );
			m_nPrevRotationSide = 0;
		}
		else {
			m_Cross.RotateYaw( -(int)( fSpeed * 2.84f ) );
			m_nPrevRotationSide = 1;
		}

		fDot = D3DXVec3Dot( &m_MoveDir, &(m_Cross.m_ZVector) );
		fAngle = acos( fDot ) * 180.f / 3.1415926f;
		if( fAngle < fSpeed ) {
			m_Cross.m_ZVector=m_MoveDir;
			m_Cross.UpdateVectors();

			m_pRotateCtrl->Reset();
			m_nPrevRotationSide = -1;
		}
		return;
	}
	else{
		m_Cross.m_ZVector=m_MoveDir;
		m_Cross.UpdateVectors();
		m_MoveDir=D3DXVECTOR3(0.0f, 0.0f, 0.0f);
		m_pRotateCtrl->Reset();
		m_nPrevRotationSide = -1;
	}
}

bool CFcGameObject::Render()
{
	//	if( !m_bUpdateObject )
	//{
	//	return;
	//}

	if( !m_bDropWeapon )
	{
		if( ( m_nCurrentWeaponIndex != -1 ) && m_bDead )
		{
			if( Random( 100 ) < 50 )
			{
				DropWeapon();
			}
			if( Random( 100 ) < 50 )
			{
				DropArmor();
			}
			if( ( m_nCurAniType != ANI_TYPE_DOWN ) && ( m_nCurAniType != ANI_TYPE_DOWN_HIT ) && ( m_nCurAniType != ANI_TYPE_DIE ) )
			{
				ChangeAnimation( ANI_TYPE_DIE, 0 );
			}
			m_bDropWeapon = true;
		}
	}

	if( m_hTroop && m_nRagDollMode == RAGDOLL_MODE_NONE )
	{
		if( m_hTroop->IsRenderSkip() )
			return false;
	}

	/*	int nLoop;
	float fLength[2] = { -1, -1 };
	float fDot;
	D3DXVECTOR3 Distance[ 2 ];
	CameraObjHandle CamHandle[2];

	for( nLoop = 0; nLoop < g_FCGameData.nPlayerCount; nLoop++ )
	{
	CamHandle[nLoop] = CFcBaseObject::GetCameraObjectHandle( nLoop );
	if( CamHandle[nLoop] )
	{
	Distance[ nLoop ] = m_Cross.m_PosVector - CamHandle[nLoop]->GetPos();
	fLength[nLoop] = D3DXVec3LengthSq( Distance + nLoop );
	}
	}

	for( nLoop = 0; nLoop < g_FCGameData.nPlayerCount; nLoop++ )
	{
	if( fLength[nLoop] < 400000000.0f )	// D3DXVec3LengthSq 를 써서 거리가 이렇게 크다.. 루트하면 20000 이다.. 200미터
	{
	if(CamHandle[nLoop].GetPointer() == NULL)
	{
	continue;
	}

	fDot = D3DXVec3Dot( Distance + nLoop, &( CamHandle[nLoop]->GetCrossVector()->m_ZVector ) );
	if( ( fDot >= 0 ) || ( fLength[nLoop] < 90000.0f ) )	// 카메라 뒤에 있고.. 거리가 3미터 이상 떨어져 있으면 스킵한다.
	{
	break;
	}
	}
	}

	if( nLoop >= 2 )
	{
	return false;
	}*/

	if( m_nDieFrame > DIE_KEEP_FRAME )
	{
		if( m_Handle != CFcWorld::GetInstance().GetHeroHandle() )
		{
			float fAlphaWeight;
			fAlphaWeight = 1.0f - ( ( m_nDieFrame - DIE_KEEP_FRAME ) / DIE_ALPHA_FRAME );
			g_BsKernel.SendMessage( m_nEngineIndex, BS_ENABLE_OBJECT_ALPHABLEND, 1 );
			if( fAlphaWeight < 0.5f )
			{
				g_BsKernel.SendMessage( m_nEngineIndex, BS_SHADOW_CAST, 0 );
			}
			g_BsKernel.SendMessage( m_nEngineIndex, BS_SET_OBJECT_ALPHA, ( DWORD )&fAlphaWeight );
		}
	}
	if( m_ParentLinkObjHandle ) {
		g_BsKernel.SetCurrentAni( m_nEngineIndex, m_nAniIndex, ( float )( int )( m_fFrame ) );

		if( m_nChangeAni != -1 )
		{
			g_BsKernel.BlendAni( m_nEngineIndex, m_nChangeAni, ( float )( int )( m_fChangeFrame ), m_fBlendWeight );
		}

		return false;
	}

	return CFcAniObject::Render();
}

bool CFcGameObject::IsHittable( GameObjHandle Handle )
{
	if( m_bInvulnerable == true )		// 트리거에서 세팅하는 무적
		return false;

	if( m_nDieFrame > DIE_KEEP_FRAME )
	{
		return false;
	}
	/*	if( m_fHP <= 0) 
	return false;*/

	if( m_HitParam.pHitSignal )
	{
		return false;
	}
	/*
	if( Handle )
	{
	if( ( m_HitParam.nHitRemainFrame > 0 ) && ( m_HitParam.ObjectHandle.m_nHandle == Handle.m_nHandle ) )
	return false;
	}
	else
	{
	if( m_HitParam.nHitRemainFrame > 0 )
	{
	return false;
	}
	}
	*/
	if( m_HitParam.nHitRemainFrame > 0 )
	{
		return false;
	}

	if( m_bGodMode == true ) 
		return false;

	if( m_bOnOrbsparkGodMode == true)
		return false;
	return true;
}

void CFcGameObject::CreateOrbObject( GameObjHandle HeroHandle, int nGenerateOrbType, int nOrbPercent )
{
	OrbObjHandle Handle;
	HeroObjHandle Hero;
	int nPercent, nCharge;

	if( GetDischargeOrbSpark() == 0 ) return;

	if( ( nGenerateOrbType == GENERATE_ORB_DISABLE ) && ( !m_bForceGenerateOrb ) )
	{
		return;
	}

	nPercent = 100;
	Hero = CFcWorld::GetInstance().GetHeroHandle();
	if( Hero )
	{
		nPercent += Hero->GetOrbPercent();
	}
	nCharge = ( int )( GetDischargeOrbSpark() * ( nOrbPercent / 100.0f ) * ( nPercent / 100.0f ) );
	Handle = CFcBaseObject::CreateObject< CSpritObject >( &m_Cross );
	Handle->Initialize( 43, &m_Cross.m_PosVector, nCharge, HeroHandle, nGenerateOrbType );
}

void CFcGameObject::CreateDropWeapon( int nDropIndex, float fForce )
{
	D3DXMATRIX *pRetMat;
	CCrossVector Cross;
	float fWeight;
	DropWeaponHandle Handle;
	D3DXVECTOR3 ForceVector;

	pRetMat = ( D3DXMATRIX * )g_BsKernel.SendMessage( m_nEngineIndex, BS_UNLINKOBJECT, nDropIndex );
	if( !pRetMat )
	{
		return ;
	}

	if( m_bSubJob )
	{
		if( m_nPriEngineIndex > 0 ) {
			( D3DXMATRIX * )g_BsKernel.SendMessage( m_nPriEngineIndex, BS_UNLINKOBJECT, nDropIndex );			
		}
	}
	else
	{
		if( m_nSubEngineIndex > 0 ) {
			( D3DXMATRIX * )g_BsKernel.SendMessage( m_nSubEngineIndex, BS_UNLINKOBJECT, nDropIndex );			
		}
	}

	const float fLimitDistance = 500.f;

	D3DXVECTOR3 vDist = m_Cross.GetPosition() - (*(D3DXVECTOR3*)&pRetMat->_41);	
	if( D3DXVec3LengthSq( &vDist ) > fLimitDistance * fLimitDistance ) {
		Cross.SetPosition( D3DXVECTOR3(500.f, 10000.f, 500.f));
	}
	else {
		memcpy( &Cross.m_XVector, &pRetMat->_11, sizeof( D3DXVECTOR3 ) );
		memcpy( &Cross.m_YVector, &pRetMat->_21, sizeof( D3DXVECTOR3 ) );
		memcpy( &Cross.m_ZVector, &pRetMat->_31, sizeof( D3DXVECTOR3 ) );
		memcpy( &Cross.m_PosVector, &pRetMat->_41, sizeof( D3DXVECTOR3 ) );
	}

	fWeight = RandomNumberInRange( 0.1f, 0.65f );
	D3DXVec3Lerp( &ForceVector, &m_Cross.m_YVector, &(-m_Cross.m_ZVector), fWeight );
	ForceVector *= fForce;

	Handle = CFcBaseObject::CreateObject< CFcDropWeapon >( &Cross );
	Handle->Initialize( nDropIndex, &ForceVector );

}

void CFcGameObject::DropWeapon()
{
	if( (int)m_WeaponList.size() <= m_nCurrentWeaponIndex )
	{
		DebugString( "DropWeapon Index:%d, Size:%d\n", m_nCurrentWeaponIndex, (int)m_WeaponList.size() );
		DebugString( "Invalid CureentWeaponIndex\n" );
		BsAssert( 0 && "Invalid CureentWeaponIndex" );
		return;
	}

	if( m_WeaponList[ m_nCurrentWeaponIndex ].nObjectIndex == -1 ) {
		return;
	}

	BsAssert( m_nDropWeaponSkinIndex == -1 );

	// Create a new object that contains the weapon object.  This hands over ownership of the weapon object to the
	// new object.

	int nSkinIndex = g_BsKernel.SendMessage( m_WeaponList[ m_nCurrentWeaponIndex ].nObjectIndex, BS_GET_SKIN_INDEX );

	CreateDropWeapon( m_WeaponList[ m_nCurrentWeaponIndex ].nObjectIndex, RandomNumberInRange( 13.f, 18.f ) );

	g_BsKernel.AddSkinRef( nSkinIndex );		// DropWeapon Create 성공시에만 RefCount 를 올려줍니다.	

	// Save the weapon skin and weapon index so that they can be re-created if needed.  Note that we add a reference to
	// the skin until this object either re-creates the weapon or is destroyed.  This is necessary because otherwise
	// the skin could be unloaded.

	m_nDropWeaponSkinIndex = nSkinIndex;
	m_WeaponList[ m_nCurrentWeaponIndex ].nObjectIndex = -1;

	m_nDropWeaponIndex = m_nCurrentWeaponIndex;
	m_nCurrentWeaponIndex = -1;
}

void CFcGameObject::DropArmor()
{
	if( m_ArmorList.size() != 1 ) 
	{
		return;
	}
	if( m_ArmorList[ 0 ].nObjectIndex == -1 )
	{
		return;
	}
	if( m_ArmorList[ 0 ].bUsePhysics )
	{
		return;
	}

	CreateDropWeapon( m_ArmorList[ 0 ].nObjectIndex, RandomNumberInRange( 9.f, 12.f ) );

	m_ArmorList[ 0 ].nObjectIndex = -1;
}

void CFcGameObject::ChangeHitGravity()
{
	if( m_HitParam.pHitSignal->m_nID == SIGNAL_TYPE_HIT )
	{
		if( m_HitParam.pHitSignal->m_HitGravityFrame )
		{
			m_nGravityChangeFrame = m_HitParam.pHitSignal->m_HitGravityFrame;
			m_fGravity = m_HitParam.pHitSignal->m_HitGravity / 100.0f;
			if( m_fGravity == 0.0f )
			{
				SetVelocityY( 0.0f );
			}
		}
	}
	else
	{
		if( m_HitParam.pHitSignal->m_Hit2GravityFrame )
		{
			m_nGravityChangeFrame = m_HitParam.pHitSignal->m_Hit2GravityFrame;
			m_fGravity = m_HitParam.pHitSignal->m_Hit2Gravity / 100.0f;
			if( m_fGravity == 0.0f )
			{
				SetVelocityY( 0.0f );
			}
		}
	}
}

void CFcGameObject::LookAtHitUnit( D3DXVECTOR3 *pDirection )
{
	if( m_HitParam.nNotLookAtHitUnit != 0 ) return;
	float fDot;
	D3DXVECTOR3 TargetPos;

	fDot = D3DXVec3Dot( pDirection, &m_Cross.m_ZVector );
	if( fDot > 0 )
	{
		TargetPos = m_Cross.m_PosVector + *pDirection * 1000.0f;
	}
	else
	{
		TargetPos = m_Cross.m_PosVector - *pDirection * 1000.0f;
	}
	m_Cross.LookAt( &TargetPos );
}

void CFcGameObject::HitProcess()
{
	if( !CheckHit() ) return;
	if( m_HitParam.pHitSignal )
	{
		D3DXVECTOR3 LookPos, Direction;
		int nAniIndex, nAniType;
		float fHeight;

		if( m_HitParam.ObjectHandle && ( CFcBaseObject::IsValid( m_HitParam.ObjectHandle ) == false ) )
		{
			m_HitParam.pHitSignal = NULL;
			return;
		}
		if( m_bGodMode == true ) {
			m_HitParam.pHitSignal = NULL;
			return;
		}

		bool bDefense = false;
		if( IsDefensible() && CheckDefense() == true ) {
			if( m_hUnitAIHandle )
			{
				m_hUnitAIHandle->OnCallback( AI_CALLBACK_DEFENSE );
			}

			bDefense = true;
		}//Sound 처리
		if( m_HitParam.nHitType == HT_NORMAL_ATTACK && (SOUND_ATTACK_TYPE)m_HitParam.pHitSignal->m_Hit2SoundAction != ATT_SKIP)
		{
			SOUND_PLAY_INFO SoundInfo;

			SoundInfo.nEnemyWeapon = (SOUND_WEAPON_TYPE)m_HitParam.pHitSignal->m_Hit2WeaponMaterial; //적이 가진 무기타입

			if( bDefense == true ) {
				SoundInfo.nCtgy = SC_DEFENSE;
				SoundInfo.nWeapon = CUnitSOXLoader::GetInstance().GetUnitData( GetUnitSoxIndex() )->nSoundWeapon; //내가 가진 무기타입
				switch(SoundInfo.nEnemyWeapon)
				{
				case WT_ASPHARR:
				case WT_TYURRU:
				case WT_KLARRANN:
					SoundInfo.nEnemyWeapon = WT_SPEAR;
					break;

				case WT_INPHYY:				
				case WT_MYIFEE:				
				case WT_DWINGVATT:
					SoundInfo.nEnemyWeapon = WT_SWORD;
					break;

				case WT_VIGKVAGK:
					SoundInfo.nEnemyWeapon = WT_CLUB;
					break;
				}


			}
			else {
				SoundInfo.nCtgy = SC_HIT;
				SoundInfo.nAmor = CUnitSOXLoader::GetInstance().GetUnitData( GetUnitSoxIndex() )->nSoundAmor; // 내가 가진 방어타입
				if( SoundInfo.nAmor == AMST_CATAPULT ) {
					SoundInfo.nEnemyWeapon = -1;
					SoundInfo.nAmor = AMST_CATAPULT;
				}
				else{
					SoundInfo.nAmor = -1;
					SoundInfo.nAttType = (SOUND_ATTACK_TYPE)m_HitParam.pHitSignal->m_Hit2SoundAction; //적 공격 동작
				}
			}
			PlaySound(&SoundInfo,true);
		}

		//////////////////////////////////////////////////////////////////////////////////////////
		if( bDefense == true ) {
			// 진오브 막았을경우 대미지 준다!
			if( m_HitParam.nHitType >= HT_ABIL_EARTH && m_HitParam.nHitType <= HT_ABIL_WIND ) {
				int nDamage = (int)( GetMaxHP() * RandomNumberInRange( 0.05f, 0.2f ) );
				m_fHP -= (float)nDamage;
				if( m_fHP <= 0.f ) m_fHP = 1.f;
				// Interface Manager 에 Hit Message 보낸다.
				if( m_ClassID == Class_ID_Adjutant || m_ClassID == Class_ID_Hero )
					g_InterfaceManager.OnHitMsg( (GameObjHandle)m_Handle );
			}
			////////////////////////////////

			m_HitParam.pHitSignal = NULL;
			return;
		}

		m_bBeatenByPlayer = false;
		if( CFcBaseObject::IsValid( m_HitParam.ObjectHandle ) && m_HitParam.ObjectHandle->GetClassID() == Class_ID_Hero ) {		
			if(( (CFcHeroObject*)m_HitParam.ObjectHandle.GetPointer() )->GetPlayerIndex() != -1 ) {			
				m_bBeatenByPlayer = IsHero( m_HitParam.ObjectHandle->GetUnitSoxIndex() );
			}
		}

		CheckTrueOrbDefense();


		if( m_HitParam.nUseHitDir )
		{
			Direction = m_HitParam.HitDir;
		}
		else
		{
			Direction = m_HitParam.Position - m_Cross.m_PosVector;
			Direction.y = 0.0f;
		}
		D3DXVec3Normalize( &Direction, &Direction );

		LookAtHitUnit( &Direction );
		ChangeHitGravity();

		// 크리티컬
		bool bCritical = false;
		float fCriticalRatio = 1.0f;
		int nDamage = ( int )( ( m_HitParam.pHitSignal->m_HitAttackPoint / 100.0f ) * m_HitParam.nAttackPower );

		if( CFcBaseObject::IsValid( m_HitParam.ObjectHandle ) ) {
			if( !m_bIgnoreCritical )
			{
				if( Random( 100 ) < ( m_HitParam.ObjectHandle->GetCritical() * ( 1 - m_nAvoidCriticalRate / 100.0f ) * ( m_HitParam.nAddCritical / 100.0f ) ) ) 
				{
					bCritical = true;
				}
			}
			if( bCritical ) {
				fCriticalRatio = m_HitParam.ObjectHandle->GetCriticalRatio();
				if( m_bSpecialCriticalRatio )
				{
					if( Random( 100 ) < 30 )
					{
						if( IsBoss() || ( m_ClassID == Class_ID_Adjutant ) )
						{
							fCriticalRatio *= 1.3f;
						}
						else
						{
							fCriticalRatio *= 2.0f;
						}
					}
				}
				nDamage = (int)( nDamage * fCriticalRatio );
				if( Random( 100 ) < 10 )
				{
					DropArmor();
				}
			}

			// 플레이어가 때렸고 오브어택 상태고 내가 부관급일때는 대미지 줄여준다.
			if( m_HitParam.ObjectHandle->GetClassID() == Class_ID_Hero && ((CFcHeroObject*)m_HitParam.ObjectHandle.GetPointer())->GetPlayerIndex() != -1 
				&& ((CFcHeroObject*)m_HitParam.ObjectHandle.GetPointer())->IsOrbSparkOn() ) {
					switch( m_ClassID ) {
					case Class_ID_Adjutant:
					case Class_ID_Hero:
						nDamage = (int)( nDamage * 0.3f );
						break;
					default:
						break;
					}
				}
		}

		nDamage += ( int )( nDamage * ( m_nDamageRatioAdd / 100.0f ) );
		if( CalculateDamage( nDamage ) == true )
		{
			if( m_HitParam.ObjectHandle && !CFcBaseObject::IsValid( m_HitParam.ObjectHandle ) )
			{
				DebugString( "HitProcess Invalid GameObjHandle!\n" );
			}
			if( CFcBaseObject::IsValid( m_HitParam.ObjectHandle ) )
			{
				TroopObjHandle hTroop = m_HitParam.ObjectHandle->GetTroop();
				if( hTroop )
					hTroop->OnAttack( TROOP_ATTACK_TYPE_MELEE, m_Handle );
			}



			// 상관관계 체크
			CFcCorrelationHit::ParamStruct Struct;

			Struct.nDamage = m_HitParam.pHitSignal->m_HitAttackPoint;
			Struct.pHandle = (GameObjHandle*)&m_HitParam.ObjectHandle;
			m_pCorrelationMng->Influence( CFcCorrelation::CT_HIT, &Struct );

			// Interface Manager 에 Hit Message 보낸다.
			if( m_ClassID == Class_ID_Adjutant || m_ClassID == Class_ID_Hero ) {
				g_InterfaceManager.OnHitMsg( (GameObjHandle)m_Handle );
			}
		}
		else {
			m_pCorrelationMng->Reset( CFcCorrelation::CT_NONE );
		}
		// TODO : 활병한테 공격받은 것 처리해야 한다. yooty

		fHeight = CFcWorld::GetInstance().GetLandHeight( m_Cross.m_PosVector.x, m_Cross.m_PosVector.z );
		if( ( m_fHP <= 0.f ) && ( m_HitParam.pHitSignal->m_HitAniType != ANI_TYPE_DOWN ) && 
			( m_HitParam.pHitSignal->m_HitAniType != ANI_TYPE_DOWN_HIT ) )
		{
			if( m_Cross.m_PosVector.y > fHeight + AIR_HEIGHT_GAP )
			{
				nAniType = ANI_TYPE_DOWN_HIT;
				nAniIndex = 0;
			}
			else
			{
				nAniType=ANI_TYPE_DIE;
				nAniIndex = -1;
			}
		}
		else{
			// 공중에 떠 있을때 맞으면 무조건 다운히트 0번으로 간다..
			if( !m_ParentLinkObjHandle && m_Cross.m_PosVector.y > fHeight + AIR_HEIGHT_GAP )
			{
				nAniType = ANI_TYPE_DOWN_HIT;
				nAniIndex = 0;
			}
			else
			{
				nAniIndex=m_HitParam.pHitSignal->m_HitAniIndex;
				nAniType=m_HitParam.pHitSignal->m_HitAniType;
			}
		}
		// 프랍위에 있는 놈이 다운 공격 맞으면 죽여버린다~
		if( nAniType == ANI_TYPE_DOWN || nAniType == ANI_TYPE_DOWN_HIT ) {
			if( IsOnProp() == true && IsFreezePos() == true ) {
				nAniType=ANI_TYPE_DIE;
				nAniIndex=0;
				SetFreezePos( false );
				// 임시
				m_fHP = 0.f;
			}
		}

		if( m_fHP <= 0.f ) 
		{
			if( m_HitParam.ObjectHandle && 
				( m_HitParam.ObjectHandle == CFcWorld::GetInstance().GetHeroHandle() ) )
			{
				if( !m_bOrbDrop )
				{
					CreateOrbObject( m_HitParam.ObjectHandle, m_HitParam.nNotGenerateOrb, m_HitParam.nGenerateOrbPercent );
				}
			}
			if( ( !m_bExpDrop ) && ( m_HitParam.ObjectHandle ) )
			{
				m_HitParam.ObjectHandle->AddExp( GetDischargeOrbSpark(), m_Handle, 1 );
			}
			if( m_HitParam.ObjectHandle != CFcWorld::GetInstance().GetHeroHandle() )
			{
				m_bItemDrop = true;
			}
			m_bExpDrop = true;
			m_bOrbDrop = true;
		}
		if( (nAniType & 0xFF00) == ANI_ATTR_NONE && (nAniType & 0x00FF) == ANI_TYPE_HIT && Random(100) < m_HitParam.nStunProb ) {
			nAniType = ANI_TYPE_STUN;
			nAniIndex = -1;
			m_nStunDelay = m_HitParam.nStunDelay;
		}
		else m_nStunDelay = 0;



		// 말탔을때 체크
		if( m_ParentLinkObjHandle ) {
			int nAttr;
			m_ParentLinkObjHandle->CheckHitLinkObject( m_Handle, nAttr, nAniType, nAniIndex );

			ChangeAnimation( nAniType, nAniIndex, nAttr, true );

		}
		else {
			if( GetExactHitAni( nAniType, nAniIndex ) )
			{
				ChangeAnimation( nAniType & 0x00FF, nAniIndex, nAniType & 0xFF00, true );
			}
		}

		if( GetClassID() == Class_ID_Unit )	// 유닛일때만 경직시간 넣는다..
		{
			if( ( nAniType == ANI_TYPE_DOWN ) || ( nAniType == ANI_TYPE_DOWN_HIT ) )
			{
				m_nStiffFrame = Random( 120 );
			}
		}
		if( m_bFreezePos == false ) {
			float fHitVelocityZ, fHitVelocityY;

			fHitVelocityZ = m_HitParam.fVelocityZ / m_pUnitSOX->fWeight;
			fHitVelocityY = m_HitParam.fVelocityY / m_pUnitSOX->fWeight;
			if( bCritical )	// 크리티컬 일때는 뒤로 30퍼센트 정도 더 밀린다..
			{
				fHitVelocityZ += fHitVelocityZ * 0.3f;
			}
			GetExactHitVelocity( fHitVelocityZ, fHitVelocityY );

			BsAssert( Direction.x == Direction.x );
			BsAssert( Direction.z == Direction.z );
			BsAssert( fHitVelocityZ == fHitVelocityZ );
			BsAssert( fHitVelocityY == fHitVelocityY );

			SetVelocityX( Direction.x * fHitVelocityZ );
			SetVelocityZ( Direction.z * fHitVelocityZ );
			SetVelocityY( fHitVelocityY );
		}
		if( m_HitParam.nGroundResist )
		{
			m_fGroundResist = m_HitParam.nGroundResist / 100.0f;
		}

		if( m_HitParam.pHitSignal->m_HitParticle > 0 )
		{
			/*
			CCrossVector Cross;
			Cross = m_Cross;
			Cross.MoveRightLeft( ( float )m_HitParam.pHitSignal->m_HitParticleX );
			Cross.m_PosVector.y = m_HitParam.Position.y + m_HitParam.pHitSignal->m_HitParticleY;
			Cross.MoveFrontBack( ( float )m_HitParam.pHitSignal->m_HitParticleZ );
			*/
			CCrossVector Cross;
			int nPaticleID;
			CalculateParticlePosition( Cross );

			nPaticleID = m_HitParam.pHitSignal->m_HitParticle;
			if( bCritical )
			{
				nPaticleID++;
				if( fCriticalRatio > 2.0f )
				{
					nPaticleID++;
				}
			}
			g_FcHitMarkMgr.AddHitMark( Cross, GetProcessTick(), nPaticleID );
			//			g_BsKernel.CreateParticleObject( nPaticleID, false, false, Cross );
		}

		// vibration
		if( !bDefense )
		{
			if( m_Handle == CFcWorld::GetInstance().GetHeroHandle() )
			{
				// [beginmodify] 2006/1/16 junyash PS#1259,3249,3251, TCR#033PER multi controller
				if( g_FCGameData.nEnablePadID == -1 )
				{
					// 2006/2/14 junyash comment out, vib off when contoroller is non assigned.
					//CInputPad::GetInstance().AddVibration( 0, 0, 65535, 32767, 10, 10 );
				}
				else
				{
					CInputPad::GetInstance().AddVibration( g_FCGameData.nEnablePadID, 0, 65535, 32767, 10, 10 );
				}
				// [endmodify] junyash
			}
		}

		if( m_HitParam.ObjectHandle )
			m_HitParam.ObjectHandle->HitSuccess();

		m_HitParam.pHitSignal=NULL;
		if( m_hUnitAIHandle )
		{
			m_hUnitAIHandle->OnCallback( AI_CALLBACK_HIT );
		}
	}
}

bool CFcGameObject::IsHero( int nUnitSoxID)
{
	if( nUnitSoxID >= C_LP_KM && nUnitSoxID <= C_DP_GB ) {	
		return true;
	}
	return false;
}

void CFcGameObject::CalculateParticlePosition( CCrossVector &Cross )
{
	if( m_HitParam.ObjectHandle ) {
		Cross.m_PosVector = m_Cross.m_PosVector;
		D3DXVec3Normalize( &Cross.m_ZVector, &(m_HitParam.ObjectHandle->GetPos() - m_Cross.m_PosVector) );
		Cross.m_YVector = D3DXVECTOR3( 0.f, 1.f, 0.f );
		D3DXVec3Cross( &Cross.m_XVector, &Cross.m_YVector, &Cross.m_ZVector );
	}
	else Cross = m_Cross;

	Cross.MoveRightLeft( ( float )m_HitParam.pHitSignal->m_HitParticleX );
	Cross.m_PosVector.y = m_HitParam.Position.y + m_HitParam.pHitSignal->m_HitParticleY;
	Cross.MoveFrontBack( ( float )m_HitParam.pHitSignal->m_HitParticleZ );
}

void CFcGameObject::CheckTrueOrbDefense()
{
	if( m_HitParam.nHitType < HT_ABIL_EARTH || m_HitParam.nHitType > HT_ABIL_WIND ) return;

	float fRatio = CLevelTableLoader::GetInstance().GetTrueOrbDamageAttenuationRatio( m_nUnitSOXIndex, GetLevel() );
	int nDamage = (int)( GetMaxHP() * ( 1.f - fRatio ) );

	if( nDamage >= GetHP() ) return;
	m_HitParam.nAttackPower = nDamage;
	if( m_HitParam.pHitSignal ) {
		m_HitParam.pHitSignal->m_Hit2AttackPoint = 100;
	}
}

bool CFcGameObject::CheckDefense()
{
	if( m_bDead ) return false;
	if( !m_HitParam.ObjectHandle ) return false;
	if( CFcBaseObject::IsValid( m_ParentLinkObjHandle ) ) return false;
	if( m_HitParam.ObjectHandle->IsGuardBreak( m_Handle ) || Random(100) < m_HitParam.nGuardBreak ) return false;


	if( Random( 100 ) < GetDefenseProb() ) {
		float fDot = D3DXVec2Dot( &D3DXVECTOR2( -m_HitParam.ObjectHandle->GetCrossVector()->m_ZVector.x, -m_HitParam.ObjectHandle->GetCrossVector()->m_ZVector.z ), &D3DXVECTOR2( m_Cross.m_ZVector.x, m_Cross.m_ZVector.z ) );
		float fAngle = acos( fDot ) * 180.f / 3.1415926f;
		if( fAngle > 90.f ) return false;

		if( m_nCurAniType == ANI_TYPE_DEFENSE )
			ChangeAnimation( ANI_TYPE_DEFENSE, m_nCurAniTypeIndex, ANI_ATTR_NONE, true );
		else ChangeAnimation( ANI_TYPE_DEFENSE, -1, ANI_ATTR_NONE, false );


		D3DXVECTOR3 vDir = m_HitParam.ObjectHandle->GetPos() - GetPos();
		vDir.y = 0.f;
		D3DXVec3Normalize( &vDir, &vDir );
		m_MoveDir = vDir;
		BsAssert( (m_MoveDir.x == m_MoveDir.x) && (m_MoveDir.y == m_MoveDir.y) && (m_MoveDir.z == m_MoveDir.z) );

		float fHitVelocityZ = ( float )m_HitParam.fVelocityZ * 0.6f;
		float fHitVelocityY = ( float )m_HitParam.fVelocityY * 0.6f;
		D3DXVECTOR3 Direction = m_HitParam.ObjectHandle->GetCrossVector()->m_PosVector - m_Cross.m_PosVector;

		Direction.y = 0.0f;
		D3DXVec3Normalize( &Direction, &Direction );

		GetExactHitVelocity( fHitVelocityZ, fHitVelocityY );

		BsAssert( Direction.x == Direction.x );
		BsAssert( Direction.z == Direction.z );
		BsAssert( fHitVelocityZ == fHitVelocityZ );
		BsAssert( fHitVelocityY == fHitVelocityY );

		SetVelocityX( Direction.x * fHitVelocityZ );
		SetVelocityZ( Direction.z * fHitVelocityZ );
		SetVelocityY( fHitVelocityY );
		return true;
	}
	return false;
}

#include "FcTroopManager.h"

bool CFcGameObject::IsPlayerDamage()
{
	if( m_HitParam.ObjectHandle.GetHandle() == -1 )	// 이런일이 없어야 하는데.. 나오네..
	{
		return false;
	}
	if( !CFcBaseObject::IsValid( m_HitParam.ObjectHandle ) ) return false;
	if( m_HitParam.ObjectHandle->GetClassID() != Class_ID_Hero ) return false;
	if( ((CFcHeroObject*)m_HitParam.ObjectHandle.GetPointer())->GetPlayerIndex() == -1 ) return false;
	if( m_HitParam.ObjectHandle->GetHandle() != g_FcWorld.GetHeroHandle() ) return false;

	return true;
}
bool CFcGameObject::CalculateDamage(int nAttackPower)
{
	int nPower;

	if( g_FCGameData.bOneShotMode && GetClassID() != Class_ID_Hero ) {
		GameObjHandle Hero = (GameObjHandle)g_FcWorld.GetTroopmanager()->GetHero();
		if( Hero && Hero->GetTeam() != GetTeam() ) {
			nAttackPower = -1;
		}
	}

	nPower = nAttackPower - GetDefense();
	if( nPower <= 0 )
	{
		nPower = 5;
	}

	if( g_FCGameData.bNoDamage == true ) {
		return 1;
	}

	if(m_bInvincible == false)
	{
		int nSaveHP = (int)m_fHP;
		if(nAttackPower != -1)
			AddHP( -nPower );
		else
			AddHP( -GetHP() );

		if( m_nRevivalHPCondition && m_nRevivalHPCondition <= nSaveHP )
		{
			if( m_fHP <= 0.f ) {			
				m_fHP = 1.f;				
			}
		}
	}

	if( m_fHP <= 0.f )
	{
		if( m_bRevival )
		{
			m_fHP = 1.f;
			m_bRevival = false;			
		}
		g_FcItemManager.NotifyRevival();
	}

	return ( nAttackPower > 0 );
}

/*void CFcGameObject::ProcessHit()
{
int nAniType=m_pUnitInfoData->GetAniType(m_nAniIndex);
if( nAniType!=ANI_TYPE_HIT )
{
int nAniIndex=m_pUnitInfoData->GetAniIndex(ANI_TYPE_HIT, 0);
ChangeAnimation(nAniIndex);
}
}*/

int CFcGameObject::GetAniTypeCount( int nAniAttr, int nAniType )
{
	return m_pUnitInfoData->GetAniTypeCount( nAniAttr, nAniType );
}

void CFcGameObject::ChangeAnimation(int nAniType, int nIndex, int nAttr /*= 0*/, bool bOverlap/*=false*/, int nLoopCount/*1*/, bool bEvent /*= false*/ )
{
	int nAniIndex;

	if( bEvent )
	{
		if( ( m_nCurAniType == ANI_TYPE_JUMP_UP ) || ( m_nCurAniType == ANI_TYPE_JUMP_ATTACK ) )
			return;
	}
	if( nIndex == -1 )
	{
		if( nAniType == m_nCurAniType && nAttr == m_nCurAniAttr )
		{
			return;
		}
	}

	nAniIndex = m_pUnitInfoData->GetAniIndex( nAniType, nIndex, nAttr);
	if( nAniIndex == -1 ) {
		char szStr[128];
		sprintf( szStr, "Can't Find Animation ( UnitID : %d, AniAttr : %d, AniType : %d, AniIndex : %d )\n", m_nUnitSOXIndex, nAttr, nAniType, nIndex );	
		g_BsKernel.AddConsoleString( szStr );
		DebugString( szStr );
		return;
	}

	ChangeAnimationByIndex( nAniIndex, bOverlap, nLoopCount );

}


void CFcGameObject::ChangeAnimationByIndex( int nAniIndex, bool bOverlap/*=false*/, int nLoopCount/*1*/, bool bProcesSignal/*true*/ )
{
	int nBlendFrame;
/*	
	if( m_bDead == true ) {
	if( m_nCurAniType != ANI_TYPE_DOWN && m_nCurAniType != ANI_TYPE_DOWN_HIT && m_nCurAniType != ANI_TYPE_DIE ) {
	assert(0);
	}
	}
*/	
	if( ( m_nAniIndex == nAniIndex ) && ( !bOverlap ) )
	{
		return;
	}

	int nTemp1 = m_nAniIndex;
	int nTemp2 = m_nCurAniType;

	m_bGodMode = false;
	nBlendFrame = m_pUnitInfoData->GetBlendFrame( m_nAniIndex );
	m_nCurAniType = m_pUnitInfoData->GetAniType( nAniIndex );
	m_nCurAniAttr = m_pUnitInfoData->GetAniAttr( nAniIndex );
	m_nCurAniTypeIndex = m_pUnitInfoData->GetAniTypeIndex( nAniIndex );
	m_nCurAniLength = g_BsKernel.GetAniLength( m_nEngineIndex, nAniIndex );
	m_LocalAniMove = D3DXVECTOR3( 0.0f, 0.0f, 0.0f );

	CFcAniObject::SetAnimation( nAniIndex, nBlendFrame );
	m_nAniLoopCount = nLoopCount;

	if( m_nGravityChangeFrame == 0 )
	{
		m_fGravity = DEFAULT_GRAVITY;
	}
	m_fGroundResist = DEFAULT_GROUND_RESIST;
	m_bKeepOrbSpark = false;
	//	m_AniPosToLocalPosFlag = BS_CALC_POSITION_Y;

	if( bProcesSignal ) ProcessSignal();

	switch( m_nCurAniType ) {
		case ANI_TYPE_ATTACK:
			if( m_hUnitAIHandle ) m_hUnitAIHandle->OnCallback( AI_CALLBACK_ATTACK );
			break;
		default:
			break;
	}
}

void CFcGameObject::ApplyAniPositionToLocalPosition()
{
	GET_ANIDISTANCE_INFO Info;
	D3DXVECTOR3 vPos = D3DXVECTOR3( 0.f, 0.f, 0.f );

	Info.fFrame1 = m_fFrame;
	Info.fFrame2 = m_fPrevFrame;
	Info.nAni1 = m_nAniIndex;
	Info.nAni2 = m_nAniIndex;
	Info.pVector = &vPos;
	g_BsKernel.GetAniDistance( m_nEngineIndex, &Info );

	D3DXVECTOR3 Pos( 0.f, 0.f, 0.f );

	if( !( m_AniPosToLocalPosFlag & BS_CALC_POSITION_X ) )
	{
		Pos += m_Cross.m_XVector * vPos.x;
	}
	if( !( m_AniPosToLocalPosFlag & BS_CALC_POSITION_Y ) )
	{
		Pos += m_Cross.m_YVector * vPos.y;
	}
	if( !( m_AniPosToLocalPosFlag & BS_CALC_POSITION_Z ) )
	{
		float fLength;
		fLength = D3DXVec3LengthSq( &m_MoveDir );
		if( fLength <= 0.0f )
		{
			Pos += m_Cross.m_ZVector * vPos.z;
		}
		else
		{
			Pos += m_MoveDir * vPos.z;
		}
	}

	m_LocalAniMove += Pos;
	m_MoveVector += Pos;
}

void CFcGameObject::ProcessSignal()
{
	int i, nCount, nBackupIndex;
	CAniSignal *pSignalList;
	ASSignalData *pSignal;

	if( m_nAniIndex == -1 ) return;
	pSignalList = m_pASData->GetSignalList( m_nAniIndex );
	nCount = pSignalList->GetSignalCount();
	nBackupIndex = m_nAniIndex;

	for( i = 0; i < nCount; i++ )
	{
		pSignal = pSignalList->GetSignal( i );
		SignalCheck( pSignal );
		if( nBackupIndex != m_nAniIndex )	// 시그널에서 ani가 바뀌면 현재 체크하던거 중단한다..
		{
			break;
		}
	}
	/*	float fSaveFrame;
	m_fFrame += 1.0f;	// 30프레임 바뀐거 땜에 중간 시그널 실행 안될수도 있어서.. 이렇게 수정..
	fSaveFrame = m_fFrame;
	for( i = 0; i < nCount; i++ )
	{
	pSignal = pSignalList->GetSignal( i );
	SignalCheck( pSignal );
	}
	if( fSaveFrame == m_fFrame )
	{
	m_fFrame -= 1.0f;
	}*/
}

void CFcGameObject::SignalCheck( ASSignalData *pSignal )
{
	switch( pSignal->m_nID )
	{
	case SIGNAL_TYPE_HIT:		// Hit Check
		DebugString( "SIGNAL_TYPE_HIT is not use anymore!! %d\n", m_nUnitSOXIndex );
		BsAssert( 0 );
		HitCheck( pSignal );
		break;
	case SIGNAL_TYPE_HIT_TEMP:		// 2ND 빌드용 임시 HitCheck다..
		HitCheckTemp( pSignal );
		break;
	case SIGNAL_TYPE_PARTICLE:
		ParticleCheck(pSignal);
		break;	// particle
	case SIGNAL_TYPE_TERRAIN_PARTICLE:
		TerrainParticleCheck( pSignal );
		break;
	case SIGNAL_TYPE_LINKED_PARTICLE:
		LinkedParticleCheck(pSignal);
		break;
	case SIGNAL_TYPE_LINKED_TO_CROSS_PARTICLE:
		LinkedToCrossParticleCheck(pSignal);
		break;
	case SIGNAL_TYPE_FX:
		FXCheck(pSignal);
		break;
	case SIGNAL_TYPE_SOUND_YEAH:
		SoundYeahCheck(pSignal);
		break;
	case SIGNAL_TYPE_SOUND:
		SoundCheck(pSignal);
		break;	// sound
	case SIGNAL_TYPE_JUMP:
		JumpCheck(pSignal);
		break;	// Jump
	case SIGNAL_TYPE_GOTO_PRESS_CHECK:
		GotoCheck( pSignal );
		break;
	case SIGNAL_TYPE_CALC_YAXIS_INGAME:
		if( ( pSignal->m_nFrame >= m_fFrame ) && ( pSignal->m_nFrame < m_fFrame + GetFrameAdd() ) )
		{
			if( pSignal->m_pParam[0] == 1 )
			{
				m_AniPosToLocalPosFlag &= !BS_CALC_POSITION_Y;
				m_fGravity = 0.0f;
			}
			else
			{
				m_AniPosToLocalPosFlag |= BS_CALC_POSITION_Y;
			}
		}
		break;
	case SIGNAL_TYPE_SHOW_WEAPON:
		if( ( pSignal->m_nFrame >= m_fFrame ) && ( pSignal->m_nFrame < m_fFrame + GetFrameAdd() ) )
		{
			//				DebugString( "ShowWeapon %x\n", pSignal );
			// 땜빵! 미피 오브스파크 공격하고 무기 날라가는 도중 피니쉬 동작 들어가면 m_WeaponList없는 경우 있어서 막음 yooty
			// 필요 있나?? by mapping
			if( pSignal->m_pParam[0] >= (int)m_WeaponList.size() )
			{
				break;
			}
			m_WeaponList[ pSignal->m_pParam[ 0 ] ].bShow = pSignal->m_pParam[ 1 ];
#pragma warning(disable:4800)
			if( m_WeaponList[ pSignal->m_pParam[ 0 ] ].nObjectIndex != -1 ) {
				g_BsKernel.ShowObject( m_WeaponList[ pSignal->m_pParam[ 0 ] ].nObjectIndex, 
					m_WeaponList[ pSignal->m_pParam[ 0 ] ].bShow );
			}
#pragma warning(default:4800)
			if( m_WeaponList[ pSignal->m_pParam[ 0 ] ].nGroup != 2 ) {
				if( pSignal->m_pParam[ 1 ] )
				{
					m_nCurrentWeaponIndex = pSignal->m_pParam[ 0 ];
				}
				else if( m_nCurrentWeaponIndex == pSignal->m_pParam[ 0 ] )
				{
					m_nCurrentWeaponIndex = -1;
				}
			}
		}
		break;
	case SIGNAL_TYPE_SHOW_TRAIL:
		TrailCheck( pSignal );
		break;
	case SIGNAL_TYPE_CAMERA_SHOCK:
		CameraShockCheck( pSignal );
		break;
	case SIGNAL_TYPE_JOB_CHANGE:
		JobChangeCheck( pSignal );
		break;
	case SIGNAL_TYPE_GOD:
		if( ( pSignal->m_nFrame >= m_fFrame ) && ( pSignal->m_nFrame < m_fFrame + GetFrameAdd() ) )
		{
			m_bGodMode = ( pSignal->m_pParam[0] == 1 ) ? true : false;
		}
		break;
	case SIGNAL_TYPE_MORPH_WEAPON:
		MorphWeaponCheck( pSignal );
		break;
	case SIGNAL_TYPE_HIT_PUSH:
		PushCheck( pSignal );
		break;

	case SIGNAL_TYPE_LINKED_FX:
		LinkedFXCheck(pSignal);
		break;

	case SIGNAL_TYPE_AI_CONTROL_CHECK:
		AIControlCheck( pSignal );
		break;
	case SIGNAL_TYPE_AI_CONTROL_RELEASE:
		AIControlRelease( pSignal );
		break;
	case SIGNAL_TYPE_AI_CONTROL_GOTO:
		AIControlGoto( pSignal );
		break;
	case SIGNAL_TYPE_AI_CONTROL_CALLBACK:
		AIControlCallback( pSignal );
		break;
	case SIGNAL_TYPE_SUMMON:
		SummonCheck( pSignal );
		break;
	}
}

void CFcGameObject::PreProcessSignal()
{
	int i, nCount;
	CAniSignal *pSignalList;
	ASSignalData *pSignal;

	if( m_nAniIndex == -1 ) return;
	pSignalList = m_pASData->GetSignalList( m_nAniIndex );
	nCount = pSignalList->GetSignalCount();

	for( i = 0; i < nCount; i++ )
	{
		pSignal = pSignalList->GetSignal( i );
		PreSignalCheck( pSignal );
	}
}

void CFcGameObject::PreSignalCheck( ASSignalData *pSignal )
{
	switch( pSignal->m_nID )
	{
	case SIGNAL_TYPE_ROTATE:
		if( ( pSignal->m_nFrame >= m_fFrame ) && ( pSignal->m_nFrame < m_fFrame + GetFrameAdd() ) )
		{
			m_Cross.RotateYaw( pSignal->m_pParam[ 0 ] * 1024 / 360 );
		}
		break;
	}
}

void CFcGameObject::GetBox3( Box3 & B, AABB *pBoundingBox )
{
	D3DXMATRIX RootBoneRot;

	D3DXVECTOR3	tmpVec = *(D3DXVECTOR3*)&pBoundingBox->GetExtent();
	B.E[0] = tmpVec.x;
	B.E[1] = tmpVec.y;
	B.E[2] = tmpVec.z;

	g_BsKernel.SendMessage( m_nEngineIndex, BS_GET_ROOT_BONE_MATRIX, ( DWORD )m_nAniIndex, 
		( DWORD )&m_fFrame, ( DWORD )&RootBoneRot );
	D3DXMatrixMultiply( &RootBoneRot, &RootBoneRot, m_Cross );

	B.C = *(BSVECTOR*)&(RootBoneRot._41);

	memcpy( &( B.A[ 0 ] ), &RootBoneRot._11, sizeof( BSVECTOR ) );
	memcpy( &( B.A[ 1 ] ), &RootBoneRot._21, sizeof( BSVECTOR ) );
	memcpy( &( B.A[ 2 ] ), &RootBoneRot._31, sizeof( BSVECTOR ) );

	// 이거 여기 추가해도 괜찮아요? yooty
	B.compute_vertices();

	/*	D3DXVECTOR3	tmpVec = (Vmax - Vmin)*0.5f;
	B.E[0] = tmpVec.x;
	B.E[1] = tmpVec.y;
	B.E[2] = tmpVec.z;

	B.C = (Vmax + Vmin)*0.5f;
	D3DXVec3TransformCoord( &B.C, &B.C, m_Cross );

	B.A[0] = m_Cross.m_XVector;
	B.A[1] = m_Cross.m_YVector;
	B.A[2] = m_Cross.m_ZVector;*/
}

bool CFcGameObject::CollisionCheck( GameObjHandle Handle, D3DXVECTOR3 *pStart, D3DXVECTOR3 *pEnd, float fAddLength )
{
	Segment3 Line;
	AABB *pBoundingBox;
	Box3 BoundingBox;
	D3DXVECTOR3 BoxMin, BoxMax, AddPos;

	AddPos = *pEnd - *pStart;
	D3DXVec3Normalize( &AddPos, &AddPos );

	//	Line.P = *(BSVECTOR*)pStart;
	memcpy( &(Line.P), pStart, sizeof(D3DXVECTOR3) );
	D3DXVECTOR3 Dir = ( *pEnd - *pStart ) + AddPos * fAddLength;
	memcpy( &(Line.D), &Dir, sizeof(D3DXVECTOR3) );

	pBoundingBox=Handle->GetBoundingBox();
	BsAssert( NULL != pBoundingBox );	// mruete: prefix bug 573: added assert

	Handle->GetBox3(BoundingBox, pBoundingBox );

	return TestIntersection ( Line, BoundingBox );
}

void CFcGameObject::SendHitSignal( HIT_PARAM *pHitParam, GameObjHandle Handle )
{
	//	BsAssert( CFcBaseObject::IsValid( pHitParam->ObjectHandle ) && "mapping 한테 바로 연락.." );

	Handle->CmdHit( pHitParam );
	/*	if( pSignal->m_HitCatch )
	{
	Catch( Result[ i ] );
	}*/
}

void CFcGameObject::HitCheck(ASSignalData *pSignal)
{
	if( ( m_fFrame >= pSignal->m_nFrame ) && ( m_fFrame < pSignal->m_nFrame + pSignal->m_HitLength ) )
	{
		if( m_nCurrentWeaponIndex == -1 )
		{
			return;
		}

		int i, j, nLoopCount, nSize, nCollisionCount, nCurHitIndex;
		D3DXVECTOR3 *pStart, *pEnd;
		D3DXVECTOR3 Start, End, OffsetStart, OffsetEnd;
		D3DXVECTOR3 CheckStart, CheckEnd;
		D3DXMATRIX HitMatrix;
		std::vector<GameObjHandle> Result;
		float fLength;

		nCollisionCount = g_BsKernel.SendMessage( m_WeaponList[ m_nCurrentWeaponIndex ].nObjectIndex, BS_GET_COLLISION_COUNT );
		nCurHitIndex = ( int )( m_fFrame - pSignal->m_nFrame );
		BsAssert(nCollisionCount>=2);
		if(nCollisionCount){
			pStart = ( D3DXVECTOR3 * )g_BsKernel.SendMessage( m_WeaponList[ m_nCurrentWeaponIndex ].nObjectIndex, BS_GET_COLLISION_POSITION, 0);
			pEnd = ( D3DXVECTOR3 * )g_BsKernel.SendMessage( m_WeaponList[ m_nCurrentWeaponIndex ].nObjectIndex, BS_GET_COLLISION_POSITION, 1 );

			memcpy( &HitMatrix, m_pASData->GetHitMatrix( pSignal->m_HitMatrixIndex + nCurHitIndex ), 
				sizeof( D3DXMATRIX ) );
			D3DXMatrixMultiply( &HitMatrix, &HitMatrix, m_Cross );
			D3DXVec3TransformCoord(&Start, pStart, &HitMatrix);
			D3DXVec3TransformCoord(&End, pEnd, &HitMatrix);

			if( ( m_fFrame >= pSignal->m_nFrame ) && ( m_fFrame < pSignal->m_nFrame + GetFrameAdd() ) )
			{
				nLoopCount = 1;
				m_PrevCollision1 = Start;
				m_PrevCollision2 = End;
				OffsetStart = D3DXVECTOR3( 0.0f, 0.0f, 0.0f );
				OffsetEnd = D3DXVECTOR3( 0.0f, 0.0f, 0.0f );
			}
			else
			{
				fLength = D3DXVec3Length( &( End - m_PrevCollision2 ) );
				nLoopCount = ( int )( fLength / 200.0f ) + 1;
				OffsetStart = ( Start - m_PrevCollision1 ) / ( float )nLoopCount;
				OffsetEnd = ( End - m_PrevCollision2 ) / ( float )nLoopCount;
			}

			CFcWorld::GetInstance().GetEnemyObjectListInRange( m_nTeam, &m_Cross.m_PosVector, 1000.0f, Result );
			nSize = Result.size();
			for( i = 0; i < nSize; i++)
			{
				if( !Result[ i ]->IsHittable( m_Handle ) )
				{
					continue;
				}
				for( j = 0; j < nLoopCount; j++ )
				{
					CheckStart = m_PrevCollision1 + OffsetStart;
					CheckEnd = m_PrevCollision2 + OffsetEnd;
					if( CollisionCheck( Result[ i ], &CheckStart, &CheckEnd, ( float )pSignal->m_HitAddLength ) )
					{
						HIT_PARAM HitParam;
						HitParam.ObjectHandle = m_Handle;
						HitParam.pHitSignal = pSignal;
						HitParam.nHitRemainFrame = ( int )( pSignal->m_HitLength - ( m_fFrame - pSignal->m_nFrame ) );
						HitParam.nAttackPower = GetAttackPower();
						HitParam.Position = m_Cross.m_PosVector;
						HitParam.nGroundResist = pSignal->m_HitGroundResist;
						HitParam.nGuardBreak = pSignal->m_HitGuardBreakProb;
						HitParam.nStunProb = pSignal->m_HitStunProb;
						HitParam.nStunDelay = pSignal->m_HitStunProb;
						HitParam.fVelocityY = ( float )pSignal->m_HitPushY;
						HitParam.fVelocityZ = ( float )pSignal->m_HitPushZ;
						SendHitSignal( &HitParam, Result[ i ] );
						break;
					}
				}
			}
			m_PrevCollision1 = Start;
			m_PrevCollision2 = End;
		}
	}
}

void CFcGameObject::HitCheckTemp( ASSignalData *pSignal )
{
	if( ( pSignal->m_nFrame >= m_fFrame ) && ( pSignal->m_nFrame < m_fFrame + GetFrameAdd() ) )
	{
		int i, nSize;
		float fLength, fDot, fCos, fSin, fAttackDist;
		std::vector<GameObjHandle> Result;
		D3DXVECTOR3 Direction, Orient;
		//		D3DXVECTOR3 LightPos( 0.0f, 0.0f, 0.0f );
		//		int nLightCount = 0;

		fAttackDist = 1 + CalcAttackDistAdd();
		if( m_bNowSpecialAttack )
		{
			fAttackDist += m_nSpecialAttackDistAdd / 100.0f;
		}

		CFcWorld::GetInstance().GetEnemyObjectListInRange( m_nTeam, &m_Cross.m_PosVector, 
			pSignal->m_Hit2MaxDist * fAttackDist, Result );
		nSize = Result.size();
		for( i = 0; i < nSize; i++)
		{
			if( !Result[ i ]->IsHittable( m_Handle ) )
			{
				continue;
			}
			bool bCheck = true;
			for( DWORD j=0; j<m_VecLinkObjHandle.size(); j++ ) {
				if( Result[i] == m_VecLinkObjHandle[j] ) {
					bCheck = false;
					break;
				}
			}
			if( bCheck == false ) continue;

			Direction = Result[ i ]->GetPos() - m_Cross.m_PosVector;
			Direction.y = 0.0f;
			fLength = D3DXVec3Length( &Direction );
			if( fLength < pSignal->m_Hit2MinDist )
			{
				continue;
			}
			if( pSignal->m_Hit2Angle != 0 )
			{
				fCos = cos( D3DXToRadian( pSignal->m_Hit2CenterAngle ) );
				fSin = sin( D3DXToRadian( pSignal->m_Hit2CenterAngle ) );
				Orient.x = m_Cross.m_ZVector.x * fCos - m_Cross.m_ZVector.z * fSin;
				Orient.y = 0.0f;
				Orient.z = m_Cross.m_ZVector.x * fSin + m_Cross.m_ZVector.z * fCos;
				D3DXVec3Normalize( &Direction, &Direction );
				fDot = D3DXVec3Dot( &Direction, &Orient );
			}
			if( ( pSignal->m_Hit2Angle == 0 ) || ( fDot > cos( D3DXToRadian( pSignal->m_Hit2Angle ) ) ) )
			{
				D3DXVECTOR3 CheckStart, CheckEnd;

				CheckStart = Result[ i ]->GetPos();
				CheckEnd = CheckStart;
				CheckStart.y = m_Cross.m_PosVector.y + pSignal->m_Hit2MinHeight;
				CheckEnd.y = m_Cross.m_PosVector.y + pSignal->m_Hit2MaxHeight;
				if( CollisionCheck( Result[ i ], &CheckStart, &CheckEnd, 0.0f ) )
				{
					HIT_PARAM HitParam;
					HitParam.ObjectHandle = m_Handle;
					HitParam.pHitSignal = pSignal;
					HitParam.nHitRemainFrame = pSignal->m_Hit2Length;
					HitParam.nAttackPower = GetAttackPower();
					HitParam.nGuardBreak = pSignal->m_Hit2GuardBreakProb;
					HitParam.nStunProb = pSignal->m_Hit2StunProb;
					HitParam.nStunDelay = pSignal->m_Hit2StunDelay;
					HitParam.Position = m_Cross.m_PosVector;
					HitParam.nGroundResist = pSignal->m_Hit2GroundResist;
					HitParam.fVelocityY = ( float )pSignal->m_Hit2PushY;
					HitParam.fVelocityZ = ( float )pSignal->m_Hit2PushZ;
					if( pSignal->m_NumParam >= 28 )
					{
						HitParam.nAddCritical = pSignal->m_Hit2AddCritiacl;
					}
					SendHitSignal( &HitParam, Result[ i ] );

					/*					if( m_Handle == CFcWorld::GetInstance().GetHeroHandle( 0 ) )
					{
					LightPos += ( m_Cross.m_PosVector + Result[ i ]->GetPos() ) / 2.0f;
					LightPos.y += 100.0f;
					nLightCount++;
					}*/
				}
			}
		}
		/*		if( ( nLightCount ) && ( s_nEffectLightFrame <= 0 ) )
		{
		CCrossVector LightCross;
		EffectLightHandle Effect;
		D3DCOLORVALUE Color;

		Color.r = 1.0f;
		Color.g = 1.0f;
		Color.b = 0.0f;
		Color.a = 1.0f;
		LightCross.m_PosVector = LightPos / ( float )nLightCount;
		Effect = CFcBaseObject::CreateObject< CFcEffectLight >( &LightCross );
		Effect->Initialize( Color, 300.0f, EFFECT_LIGHT_FRAME );
		s_nEffectLightFrame = EFFECT_LIGHT_FRAME + 1;
		}*/
	}
}

void CFcGameObject::PushCheck( ASSignalData *pSignal )
{
	if( ( pSignal->m_nFrame >= m_fFrame ) && ( pSignal->m_nFrame < m_fFrame + GetFrameAdd() ) )
	{
		int i, nSize;
		float fLength, fDot, fCos, fSin;
		std::vector<GameObjHandle> Result;
		D3DXVECTOR3 Direction, Orient;

		CFcWorld::GetInstance().GetEnemyObjectListInRange( m_nTeam, &m_Cross.m_PosVector, ( float )pSignal->m_PushMaxDist, Result );
		nSize = Result.size();
		for( i = 0; i < nSize; i++)
		{
			if( Result[ i ]->GetHP() <= 0 )
			{
				continue;
			}
			if( !Result[ i ]->IsHittable( m_Handle ) )
			{
				continue;
			}
			//			if( Result[ i ]->GetClassID() == Class_ID_Hero && ((CFcHeroObject*)Result[ i ].GetPointer())->GetHeroClassID() == Class_ID_Hero_VigkVagk ) continue;

			// 조금 마니 가라다 ㅡㅡ;
			Result[i]->m_HitParam.ObjectHandle = m_Handle;
			Result[i]->m_HitParam.fVelocityY = ( float )pSignal->m_PushPushY;
			Result[i]->m_HitParam.fVelocityZ = ( float )pSignal->m_PushPushZ;
			Result[i]->m_HitParam.nGuardBreak = pSignal->m_PushGuardBreakProb;
			//			Result[i]->m_HitParam.nStunProb = pSignal->m_PushStunProb;
			if( Result[i]->IsDefensible() && Result[i]->CheckDefense() == true ) continue;
			////////////////////////////

			bool bCheck = true;
			for( DWORD j=0; j<m_VecLinkObjHandle.size(); j++ ) {
				if( Result[i] == m_VecLinkObjHandle[j] ) {
					bCheck = false;
					break;
				}
			}
			if( bCheck == false ) continue;

			Direction = Result[ i ]->GetPos() - m_Cross.m_PosVector;
			Direction.y = 0.0f;
			fLength = D3DXVec3Length( &Direction );
			D3DXVec3Normalize( &Direction, &Direction );
			if( fLength < pSignal->m_PushMinDist )
			{
				continue;
			}
			if( pSignal->m_PushAngle != 0 )
			{
				fCos = cos( D3DXToRadian( pSignal->m_PushCenterAngle ) );
				fSin = sin( D3DXToRadian( pSignal->m_PushCenterAngle ) );
				Orient.x = m_Cross.m_ZVector.x * fCos - m_Cross.m_ZVector.z * fSin;
				Orient.y = 0.0f;
				Orient.z = m_Cross.m_ZVector.x * fSin + m_Cross.m_ZVector.z * fCos;
				fDot = D3DXVec3Dot( &Direction, &Orient );
			}
			if( ( pSignal->m_PushAngle == 0 ) || ( fDot > cos( D3DXToRadian( pSignal->m_PushAngle ) ) ) )
			{
				D3DXVECTOR3 CheckStart, CheckEnd;

				CheckStart = Result[ i ]->GetPos();
				CheckEnd = CheckStart;
				CheckStart.y = m_Cross.m_PosVector.y + pSignal->m_PushMinHeight;
				CheckEnd.y = m_Cross.m_PosVector.y + pSignal->m_PushMaxHeight;
				if( CollisionCheck( Result[ i ], &CheckStart, &CheckEnd, 0.0f ) )
				{
					if( Result[i]->IsFreezePos() == false ) {
						float fHitVelocityZ, fHitVelocityY;

						fHitVelocityZ = ( float )pSignal->m_PushPushZ;
						fHitVelocityY = ( float )pSignal->m_PushPushY;
						Result[i]->GetExactHitVelocity( fHitVelocityZ, fHitVelocityY );

						BsAssert( Direction.x == Direction.x );
						BsAssert( Direction.z == Direction.z );
						BsAssert( fHitVelocityZ == fHitVelocityZ );
						BsAssert( fHitVelocityY == fHitVelocityY );


						Result[i]->SetVelocityX( -Direction.x * fHitVelocityZ );
						Result[i]->SetVelocityZ( -Direction.z * fHitVelocityZ );
						Result[i]->SetVelocityY( fHitVelocityY );

						int nAniType, nAniIndex;

						if( pSignal->m_PushAniType == ANI_TYPE_HIT && Random(100) < pSignal->m_PushStunProb ) {
							nAniType = ANI_TYPE_STUN;
							nAniIndex = -1;

							if( Result[i]->GetExactHitAni( nAniType, nAniIndex ) )
								Result[i]->ChangeAnimation( nAniType, nAniIndex );

							m_nStunDelay = pSignal->m_PushStunDelay;
						}
						else {
							nAniType = pSignal->m_PushAniType;
							nAniIndex = pSignal->m_PushAniIndex;

							if( Result[i]->GetExactHitAni( nAniType, nAniIndex ) )
								Result[i]->ChangeAnimation( nAniType, nAniIndex );

							m_nStunDelay = 0;
						}
					}
					if( pSignal->m_PushGroundResist )
					{
						Result[i]->SetGroundResist( pSignal->m_PushGroundResist / 100.f );
					}

					if( pSignal->m_PushParticle > 0 )
					{
						CCrossVector Cross;
						Cross = m_Cross;
						Cross.MoveRightLeft( ( float )pSignal->m_PushParticleX );
						Cross.m_PosVector.y = Result[i]->GetPos().y + pSignal->m_PushParticleY;
						Cross.MoveFrontBack( ( float )pSignal->m_PushParticleZ );

						g_FcHitMarkMgr.AddHitMark( Cross, GetProcessTick(), pSignal->m_PushParticle );
						//						g_BsKernel.CreateParticleObject( pSignal->m_PushParticle + Random( 3 ), false, false, Cross );

					}
					Result[ i ]->CmdPush();
				}
			}
		}
	}
}

void CFcGameObject::LinkedFXCheck( ASSignalData *pSignal )
{
	if( ( pSignal->m_nFrame >= m_fFrame ) && ( pSignal->m_nFrame < m_fFrame + GetFrameAdd() ) )
	{
		if( m_hTroop && m_hTroop->IsRenderSkip() ) return;
		if( pSignal->m_LinkedFXOnlyTargetHeroShow == 1 ) {
			if( !m_hUnitAIHandle ) return;
			CFcAISearchSlot *pSearchSlot = m_hUnitAIHandle->GetSearchSlot();
			if( !pSearchSlot ) return;
			bool bCheck = false;
			for( DWORD i=0; i<pSearchSlot->GetSlotCount(); i++ ) {
				if( !CFcBaseObject::IsValid( pSearchSlot->GetSlotFromIndex(i)->Handle ) ) continue;
				if( pSearchSlot->GetSlotFromIndex(i)->Handle->GetClassID() == Class_ID_Hero ) {
					bCheck = true;
					break;
				}
			}
			if( bCheck == false ) return;
		}

		if( pSignal->m_LinkedFXID == 0 )
		{
			g_FcWorld.StopSimpleLinkedFx( m_Handle, pSignal->m_LinkedFXBoneIndex );
		}
		else
		{
			CCrossVector Cross;
			Cross = m_Cross;
			Cross.MoveRightLeft( ( float )pSignal->m_LinkedFXX );
			Cross.MoveUpDown( ( float )pSignal->m_LinkedFXY );
			Cross.MoveFrontBack( ( float )pSignal->m_LinkedFXZ );

			int nRot = (int)( pSignal->m_LinkedFXRX / 360.f * 1024.f );
			int nDelta = 0;
			if( pSignal->m_LinkedFXRandomRX > 0 )
			{
				nDelta = Random( pSignal->m_LinkedFXRandomRX );
				nDelta = nDelta - ( pSignal->m_LinkedFXRandomRX / 2 );
			}
			Cross.RotatePitch( nRot + nDelta );

			nRot = (int)( pSignal->m_LinkedFXRY / 360.f * 1024.f );
			nDelta = 0;
			if( pSignal->m_LinkedFXRandomRY > 0 )
			{
				nDelta = Random( pSignal->m_LinkedFXRandomRY );
				nDelta = nDelta - ( pSignal->m_LinkedFXRandomRY / 2 );
			}
			Cross.RotateRoll( nRot + nDelta );

			nRot = (int)( pSignal->m_LinkedFXRZ / 360.f * 1024.f );
			nDelta = 0;
			if( pSignal->m_LinkedFXRandomRZ > 0 )
			{
				nDelta = Random( pSignal->m_LinkedFXRandomRZ );
				nDelta = nDelta - ( pSignal->m_LinkedFXRandomRZ / 2 );
			}
			Cross.RotateYaw( nRot + nDelta );

			if( pSignal->m_LinkedFXBoneIndex == 0 ) {
				g_FcWorld.PlaySimpleFxByParent( pSignal->m_LinkedFXID, m_Handle, &Cross );
			}
			else g_FcWorld.PlaySimpleLinkedFx( pSignal->m_LinkedFXID, m_Handle, pSignal->m_LinkedFXBoneIndex);
		}
	}
}

void CFcGameObject::ParticleCheck(ASSignalData *pSignal)
{
	if( ( pSignal->m_nFrame >= m_fFrame ) && ( pSignal->m_nFrame < m_fFrame + GetFrameAdd() ) )
	{
		CCrossVector Cross;
		int nParticleID, nParticleCount;

		Cross = m_Cross;
		Cross.MoveRightLeft( ( float )pSignal->m_ParticleX );
		Cross.MoveUpDown( ( float )pSignal->m_ParticleY );
		Cross.MoveFrontBack( ( float )pSignal->m_ParticleZ );
		nParticleCount = pSignal->m_ParticleCount;

		if( nParticleCount <= 0 )
		{
			nParticleCount = 1;
		}
		nParticleID = Random( nParticleCount ) + pSignal->m_ParticleID;

		float fScale = pSignal->m_ParticleScale / 100.f;
		g_BsKernel.CreateParticleObject( nParticleID, 
			pSignal->m_ParticleLoop != 0, 
			pSignal->m_ParticleIterate != 0, 
			Cross, 
			fScale );
	}
}

void CFcGameObject::TerrainParticleCheck( ASSignalData *pSignal )
{
	if( ( pSignal->m_nFrame >= m_fFrame ) && ( pSignal->m_nFrame < m_fFrame + GetFrameAdd() ) )
	{
		const int LAND_NONE = -1;
		const int WATER_DEEP_INSIDE = 0;
		const int WATER_SHALLOW = 1;
		const int LAND_WALKING = 2;			

		int		nParticleID, nAttr;
		int		nLandInfo = -1;
		bool	bPlayParticle = true;
		float	fLandHeight = 0.f;
		float	fWaterHeight = 0.f;
		CCrossVector FxCross;

		FxCross = m_Cross;
		FxCross.MoveRightLeft( ( float )pSignal->m_ParticleX );
		FxCross.MoveUpDown( ( float )pSignal->m_ParticleY );
		FxCross.MoveFrontBack( ( float )pSignal->m_ParticleZ );

		float fUnitHeight = GetUnitHeight();
		if( g_BsKernel.IsWaterInside( FxCross.GetPosition() ) )
		{		
			fLandHeight = g_BsKernel.GetLandHeight( m_Cross.GetPosition().x,m_Cross.GetPosition().z );
			fWaterHeight = g_BsKernel.GetWaterHeight() - fLandHeight;
			if(fWaterHeight < 0.f){fWaterHeight = 0.f;}

			
			if(WATER_RIPPLE_HEIGHT_MIN < fWaterHeight ) {
				nLandInfo = WATER_DEEP_INSIDE;
				if( (fUnitHeight * 0.7f) < fWaterHeight ) {
					nLandInfo = LAND_NONE;
				}
				if( (fUnitHeight * 0.5f) < (float)pSignal->m_ParticleY ) { //입김같은 것//입이 중간에 달린 캐릭터는 어쩐담.
					nLandInfo = LAND_NONE;
				}
			}
			else{
				nLandInfo = WATER_SHALLOW;
			}
		}
		else{
			nLandInfo = LAND_WALKING;
		}

		switch(nLandInfo)
		{
		case WATER_DEEP_INSIDE:
			bPlayParticle = false;
			FxCross.m_PosVector.y += fWaterHeight + 5.f;
			g_FcWorld.PlaySimpleFx( FX_WATERRIPPLE02 , &FxCross );
			break;
			
		case LAND_WALKING:
			nAttr = CFcWorld::GetInstance().GetLandAttrIndex( FxCross.m_PosVector.x, FxCross.m_PosVector.z );
			break;
		case WATER_SHALLOW:
			nAttr = FCTT_WATER;
			break;
		}

		if(bPlayParticle)
		{	
			BsAssert( nAttr < FC_TILE_TYPE_LAST );
			nAttr = g_nTerrainParticleTable[ nAttr ];
			nParticleID = pSignal->m_ParticleID * 10 + nAttr + TERRAIN_PARTICLE_START_INDEX;		

			float fScale = pSignal->m_ParticleScale / 100.f;
			g_BsKernel.CreateParticleObject( nParticleID, 
				pSignal->m_ParticleLoop != 0, 
				pSignal->m_ParticleIterate != 0, 
				FxCross,
				fScale );
		}
	}
}

void CFcGameObject::LinkedParticleCheck(ASSignalData* pSignal)
{
	// Rotate관련 내용이 없네.. 담에 추가해주세요...
	if( ( pSignal->m_nFrame >= m_fFrame ) && ( pSignal->m_nFrame < m_fFrame + GetFrameAdd() ) )
	{
		int nBoneIndex, nParticleID;
		nBoneIndex	= pSignal->m_LinkedParticleBoneIndex;
		nParticleID = pSignal->m_LinkedParticleID;

		if( nParticleID == 0 )
		{
			g_BsKernel.SendMessage( m_nEngineIndex, BS_UNLINKPARTICLE_BONE, nBoneIndex );
		}
		else
		{
			D3DXVECTOR3 vecOffset;
			vecOffset.x	= float(pSignal->m_LinkedParticleX);
			vecOffset.y	= float(pSignal->m_LinkedParticleY);
			vecOffset.z	= float(pSignal->m_LinkedParticleZ);

			float fScale= float(pSignal->m_LinkedParticleScale) / 100.f;

			D3DXMATRIX matTrans;
			D3DXMatrixTranslation(&matTrans, vecOffset.x, vecOffset.y, vecOffset.z);

			int nParticleHandle = g_BsKernel.CreateParticleObject( nParticleID, pSignal->m_LinkedParticleLoop != 0, pSignal->m_LinkedParticleIterate != 0, &matTrans, fScale );
			if( nParticleHandle != -1 )
			{
				g_BsKernel.SendMessage( m_nEngineIndex, BS_LINKPARTICLE, nBoneIndex, nParticleHandle, (DWORD)&matTrans );
			}
		}
	}
}

void CFcGameObject::LinkedToCrossParticleCheck( ASSignalData* pSignal )
{
	if( ( pSignal->m_nFrame >= m_fFrame ) && ( pSignal->m_nFrame < m_fFrame + GetFrameAdd() ) )
	{
		int nParticleID = pSignal->m_LinkedToCrossParticleID;

		D3DXVECTOR3 vecOffset;
		vecOffset.x	= float(pSignal->m_LinkedToCrossParticleX);
		vecOffset.y	= float(pSignal->m_LinkedToCrossParticleY);
		vecOffset.z	= float(pSignal->m_LinkedToCrossParticleZ);

		float fScale= float(pSignal->m_LinkedToCrossParticleScale) / 100.f;

		D3DXMATRIX matParticle, matTrans;
		D3DXMatrixScaling(&matParticle, fScale, fScale, fScale);
		D3DXMatrixTranslation(&matTrans, vecOffset.x, vecOffset.y, vecOffset.z);
		D3DXMatrixMultiply(&matParticle, &matParticle, &matTrans);

		g_FcWorld.PlaySimpleParticleByParent(nParticleID, 
			m_Handle, 
			pSignal->m_LinkedToCrossParticleLoop != 0,
			pSignal->m_LinkedToCrossParticleIterate != 0,
			fScale , &vecOffset);
	}
}

void CFcGameObject::FXCheck(ASSignalData *pSignal)
{
	if( ( pSignal->m_nFrame >= m_fFrame ) && ( pSignal->m_nFrame < m_fFrame + GetFrameAdd() ) )
	{
		if( m_hTroop && m_hTroop->IsRenderSkip() ) return;
		if( pSignal->m_FXOnlyTargetHeroShow == 1 ) {
			if( !m_hUnitAIHandle ) return;
			CFcAISearchSlot *pSearchSlot = m_hUnitAIHandle->GetSearchSlot();
			if( !pSearchSlot ) return;
			bool bCheck = false;
			for( DWORD i=0; i<pSearchSlot->GetSlotCount(); i++ ) {
				if( !CFcBaseObject::IsValid( pSearchSlot->GetSlotFromIndex(i)->Handle ) ) continue;
				if( pSearchSlot->GetSlotFromIndex(i)->Handle->GetClassID() == Class_ID_Hero ) {
					bCheck = true;
					break;
				}
			}
			if( bCheck == false ) return;
		}

		CCrossVector Cross;
		Cross = m_Cross;
		Cross.MoveRightLeft( ( float )pSignal->m_FXX );
		Cross.MoveUpDown( ( float )pSignal->m_FXY );
		Cross.MoveFrontBack( ( float )pSignal->m_FXZ );

		int nRot = (int)( pSignal->m_FXRX / 360.f * 1024.f );
		int nDelta = 0;
		if( pSignal->m_FXRandomRX > 0 )
		{
			DebugString( "FXRotX %d %d\n", m_nUnitSOXIndex, pSignal->m_NumParam );
			nDelta = Random( pSignal->m_FXRandomRX );
			nDelta = nDelta - ( pSignal->m_FXRandomRX / 2 );
		}
		Cross.RotatePitch( nRot + nDelta );

		nRot = (int)( pSignal->m_FXRY / 360.f * 1024.f );
		nDelta = 0;
		if( pSignal->m_FXRandomRY > 0 )
		{
			DebugString( "FXRotY %d %d\n", m_nUnitSOXIndex, pSignal->m_NumParam );
			nDelta = Random( pSignal->m_FXRandomRY );
			nDelta = nDelta - ( pSignal->m_FXRandomRY / 2 );
		}
		Cross.RotateRoll( nRot + nDelta );

		nRot = (int)( pSignal->m_FXRZ / 360.f * 1024.f );
		nDelta = 0;
		if( pSignal->m_FXRandomRZ > 0 )
		{
			DebugString( "FXRotZ %d %d\n", m_nUnitSOXIndex, pSignal->m_NumParam );
			nDelta = Random( pSignal->m_FXRandomRZ );
			nDelta = nDelta - ( pSignal->m_FXRandomRZ / 2 );
		}
		Cross.RotateYaw( nRot + nDelta );

		g_FcWorld.PlaySimpleFx( pSignal->m_FXID, &Cross );
	}
}

void CFcGameObject::SummonCheck( ASSignalData *pSignal )
{
	if( ( pSignal->m_nFrame >= m_fFrame ) && ( pSignal->m_nFrame < m_fFrame + GetFrameAdd() ) )
	{						
		std::vector<TroopObjHandle> m_vecTargetList;
		TroopObjHandle hTroop;
		char szStr[16];

		// 먼저 타겟팅 트룹 리스트를 얻어노쿠..
		for( int i=0; i<m_hTroop->GetMeleeEngageTroopCount(); i++ ) {
			hTroop = m_hTroop->GetMeleeEngageTroop(i);
			if( hTroop ) m_vecTargetList.push_back( hTroop );
		}

		// Summon 트룹들 리스트를 얻어온다.
		for( int i=pSignal->m_SummonStartIndex; i<=pSignal->m_SummonEndIndex; i++ ) {
			sprintf( szStr, "Summon%02d", i );
			hTroop = CFcWorld::GetInstance().GetTroopmanager()->GetTroopByName( szStr );
			if( hTroop && ( !hTroop->IsEnable() || ( hTroop->IsEliminated() && hTroop->IsStopProcess() ) ) ) {
				hTroop->CmdRenew( 0.f, 0.f, -1 );
				// 적이 있으면 공격 이동으로
				if( m_vecTargetList.empty() ) {
					hTroop->CmdMove( m_hTroop->GetPos().x, m_hTroop->GetPos().z, true );
				}
				else {
					hTroop->CmdAttack( m_vecTargetList[ Random( (int)m_vecTargetList.size() ) ], true );
				}
			}
		}
	}
}


bool CFcGameObject::IsNeedFloorInfoForSound(SOUND_PLAY_INFO *pType)
{
	if(pType->nCtgy == SC_WALK)return true;
	if(pType->nCtgy == SC_RUN)return true;
	if(pType->nCtgy == SC_JUMP)return true;
	if(pType->nCtgy == SC_DASH)return true;
	if(pType->nCtgy == SC_DOWN)return true;
	if(pType->nCtgy == SC_DOWN_ROLL)return true;
	if(pType->nCtgy == SC_HORSE_WALK)return true;
	if(pType->nCtgy == SC_HORSE_RUN)return true;
	if(pType->nCtgy == SC_HORSE_JUMP)return true;
	if(pType->nCtgy == SC_HORSE_STOP)return true;
	return false;
}

void CFcGameObject::GetSoundInfo(SOUND_PLAY_INFO *pType,ASSignalData *pSignal)
{
	pType->nCtgy			= pSignal->m_pParam[0];
	pType->nAmor			= pSignal->m_pParam[1];
	pType->nWeapon			= pSignal->m_pParam[2];
	pType->nWeaponUseType	= pSignal->m_pParam[3];
	pType->nWeight			= pSignal->m_pParam[4];
	pType->nAttType			= pSignal->m_pParam[5];
	pType->nCueID			= pSignal->m_pParam[6];

	//아래는 원래 동적으로 구해야 하는 얘들이지만 현재는 고정해놨음
	if( IsNeedFloorInfoForSound(pType) ) {
		D3DXVECTOR3 Pos = GetPos();
		if( g_BsKernel.IsWaterInside( Pos ) ) {
			pType->nFloor = FT_WATER;
		}
		else{
			float fWidth,fHeight;
			BYTE  nAttr;
			g_FcWorld.GetMapSize( fWidth, fHeight );
			nAttr = g_FcWorld.GetMapInfo()->GetLayerAttr( (int)((Pos.x / fWidth) * 512.f), (int)((Pos.z / fHeight) * 512.f) );

			//SOIL,ROCK,GRASS,SNOW,WATER
			switch(nAttr)
			{				
			case FCTT_AB_00://(마계기본땅)//흙
			case FCTT_AB_03://(마계마른용암)//흙
			case FCTT_AB_04://(마계용암) //흙
			case FCTT_CS_00://(성쇄도시 기본흙) //흙
			case FCTT_FM_00://(농촌 기본흑) //흙
			case FCTT_FR_03://(숲 기본흙)   //흙
			case FCTT_GV_00://(고블린 마른땅)    //흙
			case FCTT_GV_01://(고블린 풀&마른땅) //흙
			case FCTT_GV_03://(고블린 거친마른땅)//흙
			case FCTT_MP_00://(산길 흙) //흙
			case FCTT_MP_01://(산길 흙&돌) //흙
			case FCTT_MP_03://(산길 거친흙) //흙
			case FCTT_PL_02://(평원 흙)//흙
			case FCTT_PL_04://(평원 흙&돌)//흙
				pType->nFloor = FT_SOIL; break;

			case FCTT_AB_01://(마계바닥1))//돌
			case FCTT_AB_02://(마계돌)//돌			
			case FCTT_CS_01://(성쇄도시 인공돌바닥 //돌
			case FCTT_CS_02://(성쇄도시 인공돌바닥 //돌
			case FCTT_CS_03://(성쇄도시 절벽) //돌
			case FCTT_FM_01://(농촌 인공돌바닥3) //돌
			case FCTT_FM_02://(농촌 돌) //돌
			case FCTT_FR_04://(숲 페허용 돌바닥) //돌
			case FCTT_GV_02://(고블린 돌)        //돌
			case FCTT_FR_02://(숲 돌)       //돌
			case FCTT_MP_02://(산길 돌) //돌
			case FCTT_PL_03://(평원 돌)//돌			
			case FCTT_VF_02://(농촌 돌2)//돌
				pType->nFloor = FT_ROCK; break;

			case FCTT_CS_04://(성쇄도시 풀) //풀
			case FCTT_FM_03://(농촌 기본풀) //풀
			case FCTT_FM_04://(농촌 거친풀) //풀
			case FCTT_FR_00://(숲 기본풀)   //풀
			case FCTT_FR_01://(숲 이끼풀)   //풀
			case FCTT_GV_04://(고블린 풀&갈라진땅) //풀
			case FCTT_MP_04://(산길 풀) //풀
			case FCTT_PL_00://(평원 기본풀)//풀
			case FCTT_PL_01://(평원 거친풀)//풀
				pType->nFloor = FT_GRASS; break;

			case FCTT_OC_00://(설원 눈) //눈
			case FCTT_OC_01://(설원 눈2)//눈
			case FCTT_OC_02://(설원 돌) //눈
			case FCTT_OC_03://(설원 절벽) //눈
			case FCTT_OC_04://(설원 인공 돌바닥)//눈
				pType->nFloor = FT_SNOW; break;
			default:
				pType->nFloor = FT_SOIL;
			}
		}		
	}
	if( pType->nCtgy ==  SC_HIT) //사실 이 코드도 필요없음
	{
		if( pType->nAttType == WT_CLUB || pType->nAttType == WT_BOW ){ //이런 예외 들
			pType->nWeaponUseType = ATT_NONE;
		}
	}
}


void CFcGameObject::SoundYeahCheck(ASSignalData *pSignal)
{
	if( ( pSignal->m_nFrame >= m_fFrame ) && ( pSignal->m_nFrame < m_fFrame + GetFrameAdd() ) )
	{
		if( g_pFcRealMovie && g_pFcRealMovie->IsPlay() ) {		
			return;
		}
		switch( m_ClassID ) {
			case Class_ID_Adjutant:
				if( ((CFcAdjutantObject*)this)->GetDieType() != 0 ) return;
				break;
			case Class_ID_Hero:
				if( ((CFcHeroObject*)this)->GetDieType() != 0 ) return;
				break;
		}
		//if( g_pSoundManager->IsMissionWaveBankSuccess() == false) 
		//	return;
		//if( GetCurAniType() == ANI_TYPE_DIE && g_FcWorld.GetHeroHandle() != this ){		
		//	return;
		//}

		/*Type -1 : 노말
		Type -2 : 50%의 확률로 사운드 재생
		사운드 queid가 복수일 경우, 먼저, 사운드 on/off를 판단, 그후 랜덤으로 재생
		Type -3 : -2의 경우와 같고, 사운드 재생 확률이 id1:id2 = 7:3*/

		int nPlayType = pSignal->m_KakeGoePlayType;
		D3DXVECTOR3 Pos = GetPos();
		Pos.y += GetUnitHeight();

		int nCount = 0;
		int nCueList[MAX_KAKEGOE_CUE_NUM];
		int nSelIndex = 0;
		for( int i = 0;i < MAX_KAKEGOE_CUE_NUM;i++ )
		{
			if( pSignal->m_pParam[i] != -1)
			{
				nCueList[nCount] = pSignal->m_pParam[i];
				nCount++;
			}
		}

		switch( nPlayType )
		{
		case -1:
			if( nCount ) 
			{
				nSelIndex = Random( nCount );
				g_pSoundManager->PlayKakegoe( nCueList[nSelIndex], &Pos);						
			}
			break;
		case 0:
			if( nCount ) {
				if(Random(100) < 50)
				{
					nSelIndex = Random( nCount );								
					g_pSoundManager->PlayKakegoe( nCueList[nSelIndex], &Pos);
				}
			}
			break;
		case 1:
			{
				int nSelPrbbt = Random(10);
				if(nSelPrbbt < 7)
				{
					if( nCueList[0] != -1) {
						g_pSoundManager->PlayKakegoe( nCueList[0], &Pos);
					}
				}
				else
				{
					if( nCueList[1] != -1) {
						g_pSoundManager->PlayKakegoe( nCueList[1], &Pos);
					}
				}

			}
			break;
		case 2:
			{
				if( nCount ) {
					if( Random(100) < 20 )
					{
						nSelIndex = Random( nCount );								
						g_pSoundManager->PlayKakegoe( nCueList[nSelIndex], &Pos);
					}
				}
			}
			break;
		case 3:
			{
				if( nCount ) {
					if( Random(100) < 30 )
					{
						nSelIndex = Random( nCount );								
						g_pSoundManager->PlayKakegoe( nCueList[nSelIndex], &Pos);
					}
				}
			}
			break;
		}
	}
}


void CFcGameObject::SoundCheck(ASSignalData *pSignal)
{
	if( ( pSignal->m_nFrame >= m_fFrame ) && ( pSignal->m_nFrame < m_fFrame + GetFrameAdd() ) )
	{
		SOUND_PLAY_INFO Info;
		GetSoundInfo( &Info, pSignal );
		PlaySound( &Info , true);
	}
}

bool CFcGameObject::IsSoundNoSkipUnit()
{
	if( GetUnitSoxIndex() == SNSU_TROLL ){	
		return true;
	}
	return false;
}

bool CFcGameObject::IsVigkVagk()
{
	if( GetUnitSoxIndex() == 5 )
		return true;
	return false;
}

void CFcGameObject::PlaySound( SOUND_PLAY_INFO *pInfo,bool b3D )
{
	if( b3D )
	{
		if( pInfo->nCtgy == SC_RANDOM_30 ){
			if( Random( 1000 ) < 300 ){			
				pInfo->nCtgy = SC_COMMON;
			}
			else{
				return;
			}
		}
		

		D3DXVECTOR3 Pos = GetPos();
		Pos.y += GetUnitHeight();
		if( g_pSoundManager->IsInDistanceLimit(pInfo->nCtgy,Pos) )
		{
			if( pInfo->nAmor == AMST_TROLL && IsVigkVagk() )  {
				pInfo->nAmor = AMST_VIGKVAGK;
			}
			bool bSkipIgnore = IsSoundNoSkipUnit() || ( GetClassID() == Class_ID_Hero);
			m_nSoundHandle = g_pSoundManager->Play3DSound( this, pInfo, &Pos, NULL,1 ,bSkipIgnore );
		}
	}
	else{
		BsAssert( 0 && "cannot play 2d sound");
	}
}

void CFcGameObject::ProcessSound()
{
	if( m_nSoundHandle != -1 )
	{
		if( g_pSoundManager->IsOwner( this,m_nSoundHandle ) == true )
		{
			D3DXVECTOR3 Pos = GetPos();
			Pos.y += GetUnitHeight();
			g_pSoundManager->SetEmitterPos(m_nSoundHandle,&Pos);
		}
		else{
			m_nSoundHandle = -1;
		}
	}
}

void CFcGameObject::GotoCheck( ASSignalData *pSignal )
{
	if( ( m_fFrame >= pSignal->m_nFrame ) && ( m_fFrame < pSignal->m_nFrame + pSignal->m_GotoPressCheckLength ) )
	{
		if( pSignal->m_GotoLandCheck )
		{
			if( m_fLastHeight >= m_Cross.m_PosVector.y )
			{
				return;
			}
		}
		m_fFrame = ( float )pSignal->m_GotoPressCheckFrame;
		if( m_fFrame > m_nCurAniLength - 1 )
		{
			m_fFrame = ( float )( m_nCurAniLength - 1 );
		}
		MoveZ( ( float )pSignal->m_GotoMoveZ );
	}
}

void CFcGameObject::JumpCheck(ASSignalData *pSignal)
{
	if( ( pSignal->m_nFrame >= m_fFrame ) && ( pSignal->m_nFrame < m_fFrame + GetFrameAdd() ) )
	{
		if( pSignal->m_JumpEnableSpeed )
		{
			BsAssert( (float)pSignal->m_JumpSpeed == (float)pSignal->m_JumpSpeed );
			SetVelocityY( ( float )pSignal->m_JumpSpeed );
		}
		if( pSignal->m_JumpEnableGravity )
		{
			m_fGravity = pSignal->m_JumpGravity / 100.0f;
		}
	}
}

void CFcGameObject::AddTrail( int nTrailFrame, ASSignalData *pSignal, int nBookingIndex/* = 0*/, int nTrailHandle/* = 0*/ )
{
	if( m_nCurrentWeaponIndex == -1 )
	{
		return;
	}
	if( !m_bTrailOn[ nTrailHandle ] )
	{
		float fLifeTime;
		D3DXVECTOR3 Color;

		fLifeTime = ( float )( pSignal->m_TrailLifeTime );
		Color.x = pSignal->m_TrailR / 255.0f;
		Color.y = pSignal->m_TrailG / 255.0f;
		Color.z = pSignal->m_TrailB / 255.0f;
		g_pFcFXManager->SendMessage( m_nTrailHandle[ nTrailHandle ], FX_SETLIFE, ( DWORD )&fLifeTime );
		g_pFcFXManager->SendMessage( m_nTrailHandle[ nTrailHandle ], FX_SETCOLOR, ( DWORD )&Color );
	}

	int nBookingCount;
	D3DXVECTOR3 *pStart, *pEnd;
	D3DXMATRIX HitMatrix, HitMatrix2;
	D3DXVECTOR3 TrailPos[ 4 ], AddPos;

	nBookingCount = g_BsKernel.SendMessage( m_WeaponList[ m_nCurrentWeaponIndex ].nObjectIndex, BS_GET_BOOKING_COUNT );
//	BsAssert( nBookingCount >= ( nBookingIndex + 1 ) * 2 );
	if( nBookingCount < ( nBookingIndex + 1 ) * 2 )
		DebugString( "Trail을 사용하는 녀석이 Booking데이타가 부정확합니다. %s, AniType:%d, AniIndex:%d\n", m_pUnitSOX->cUnitName, m_nCurAniType, m_nCurAniTypeIndex );

	if( nBookingCount )
	{
		pStart = ( D3DXVECTOR3 * )g_BsKernel.SendMessage( m_WeaponList[ m_nCurrentWeaponIndex ].nObjectIndex, BS_GET_BOOKING_POSITION, 0 + nBookingIndex * 2 );
		pEnd = ( D3DXVECTOR3 * )g_BsKernel.SendMessage( m_WeaponList[ m_nCurrentWeaponIndex ].nObjectIndex, BS_GET_BOOKING_POSITION, 1 + nBookingIndex * 2 );

		memcpy( &HitMatrix, m_pASData->GetHitMatrix( pSignal->m_TrailMatrixIndex + nTrailFrame ), 
			sizeof( D3DXMATRIX ) );
		if( nTrailFrame < pSignal->m_HitLength -1 )
		{
			memcpy( &HitMatrix2, m_pASData->GetHitMatrix( pSignal->m_TrailMatrixIndex + nTrailFrame + 1 ), sizeof( D3DXMATRIX ));
		}
		else
		{
			memcpy( &HitMatrix2, m_pASData->GetHitMatrix( pSignal->m_TrailMatrixIndex + nTrailFrame ), sizeof( D3DXMATRIX ));
		}
		D3DXMatrixMultiply( &HitMatrix, &HitMatrix, m_Cross );
		D3DXVec3TransformCoord( TrailPos, pStart, &HitMatrix );
		D3DXVec3TransformCoord( TrailPos + 1, pEnd, &HitMatrix );
		AddPos = TrailPos[ 1 ] - TrailPos[ 0 ];
		D3DXVec3Normalize( &AddPos, &AddPos );
		TrailPos[ 1 ] += AddPos * ( float )( pSignal->m_TrailAddLength + m_nTrailLengAdd );

		D3DXMatrixMultiply( &HitMatrix2, &HitMatrix2, m_Cross );
		D3DXVec3TransformCoord( TrailPos +2, pStart, &HitMatrix2 );
		D3DXVec3TransformCoord( TrailPos + 3, pEnd, &HitMatrix2 );
		AddPos = TrailPos[ 3 ] - TrailPos[ 2 ];
		D3DXVec3Normalize( &AddPos, &AddPos );
		TrailPos[ 3 ] += AddPos * ( float )pSignal->m_TrailAddLength;

		int nTexIdx = (int)(pSignal->m_TrailKind);
		g_pFcFXManager->SendMessage( m_nTrailHandle[ nTrailHandle ], FX_SET_DIFFTEXTURE, ( DWORD )GetWeaponTrailTextureID( nTexIdx ) );
		g_pFcFXManager->SendMessage( m_nTrailHandle[ nTrailHandle ], FX_SET_OFFSETTEXTURE, ( DWORD )GetWeaponTrailOffsetTextureID() );
		g_pFcFXManager->SendMessage( m_nTrailHandle[ nTrailHandle ], FX_ADD_TRAIL, ( DWORD )TrailPos );
	}
	m_bTrailOn[ nTrailHandle ] = true;
}

void CFcGameObject::TrailCheck( ASSignalData *pSignal )
{
	if(m_nTrailHandle[ 0 ] == -1)
		return;

	if( ( m_fFrame >= pSignal->m_nFrame ) && ( m_fFrame < pSignal->m_nFrame + pSignal->m_HitLength ) )
	{
		AddTrail( ( int )( m_fFrame - pSignal->m_nFrame ), pSignal );
		return;
	}
	if( m_bTrailOn[ 0 ] )
	{
		g_pFcFXManager->SendMessage( m_nTrailHandle[ 0 ], FX_STOP_OBJECT );
	}
	m_bTrailOn[ 0 ] = false;
	m_bTrailOn[ 1 ] = false;
}




void CFcGameObject::CameraShockCheck(  ASSignalData *pSignal )
{
	if( ( pSignal->m_nFrame >= m_fFrame ) && ( pSignal->m_nFrame < m_fFrame + GetFrameAdd() ) )
	{
		// 카메라 선택하고 거리 체크하는 코드 필요
		CameraObjHandle CamHandle=CFcBaseObject::GetCameraObjectHandle(0);

		if (this == g_FcWorld.GetHeroHandle())
			CamHandle->Shock(&m_Cross.m_PosVector, 0.0f, pSignal->m_pParam[0] * .001f, pSignal->m_pParam[1] * .001f, pSignal->m_pParam[2]);
		else
			CamHandle->Shock(&m_Cross.m_PosVector, 3500.0f, pSignal->m_pParam[0] * .001f, pSignal->m_pParam[1] * .001f, pSignal->m_pParam[2]);
	}
}

void CFcGameObject::JobChangeCheck( ASSignalData *pSignal )
{
	if( m_pParent == NULL )
		return;

	if( ( pSignal->m_nFrame >= m_fFrame ) && ( pSignal->m_nFrame < m_fFrame + GetFrameAdd() ) )
	{

		int nNextAniIndex = m_pUnitInfoData->GetNextAni( m_nAniIndex );

		if( ((CFcUnitJobChange*)this)->IsSubJob() == ( pSignal->m_pParam[0] == 1 ) ) return;
		((CFcUnitJobChange*)this)->ChangeJob( pSignal->m_pParam[0] == 1 );

		m_nAniIndex = 0;
		ChangeAnimationByIndex( nNextAniIndex, true );
		m_nBlendFrame = 0;
	}
}

void CFcGameObject::MorphWeaponCheck( ASSignalData *pSignal )
{
	if( ( m_fFrame >= pSignal->m_nFrame ) && ( m_fFrame < pSignal->m_nFrame + pSignal->m_pParam[1] + GetFrameAdd() ) )
	{
		float fRange = 0.f;
		if( pSignal->m_pParam[1] <= 1 ) {
			fRange = pSignal->m_pParam[3] * 0.01f;
		}
		else {
			float fCufFrame = ( 1.f / (float)( pSignal->m_pParam[1] ) ) * ( m_fFrame - (float)pSignal->m_nFrame );

			if( fCufFrame <= 0.f ) fCufFrame = 0.f;
			else if( fCufFrame >= 1.f ) fCufFrame = 1.f;

			fRange = (pSignal->m_pParam[2] + ( (pSignal->m_pParam[3] - pSignal->m_pParam[2]) * fCufFrame )) * 0.01f;
		}

		if( pSignal->m_pParam[0] < 0 || pSignal->m_pParam[0] >= (int)m_WeaponList.size() ) return;
		int nObjectIndex = m_WeaponList[ pSignal->m_pParam[ 0 ] ].nObjectIndex;	// 요걸루 처리해주3
		if( nObjectIndex == -1 ) return;

		/*
		char szStr[64];
		sprintf( szStr, "Ratio : %.2f, %.2f\n", fCufFrame, fRange );
		DebugString( szStr );
		*/
		// 이 Parameter Index는 한번만 얻어오는 구조로 바꿔야 합니다. 바꿔종.. by jeremy
		int nParamIndex = g_BsKernel.SendMessage(nObjectIndex, BS_ADD_EDITABLE_PARAMETER, 0, PARAM_MORPH_WEIGHT );
		g_BsKernel.SendMessage(nObjectIndex, BS_SET_EDITABLE_PARAMETER, 0, nParamIndex, DWORD(&fRange));
	}
}


void CFcGameObject::CmdHit( HIT_PARAM *pHitParam, D3DXVECTOR3 *pHitDir /*= NULL*/ )
{
	assert( pHitParam && pHitParam->pHitSignal );

	m_HitParam = *pHitParam;
	if( pHitDir )
	{
		m_HitParam.nUseHitDir = 1;
		m_HitParam.HitDir = *pHitDir;
	}

	if( GetClassID() >= Class_ID_Adjutant ) {
		if( CFcBaseObject::IsValid( m_HitParam.ObjectHandle ) && m_HitParam.ObjectHandle->GetClassID() == Class_ID_Hero ) {
			if( ((CFcHeroObject*)m_HitParam.ObjectHandle.GetPointer())->GetPlayerIndex() != -1 ) {
				m_HitParam.nHitRemainFrame *= m_HitParam.nAdjutantHitRemainRatio;
			}
		}
	}
}

void CFcGameObject::NextAnimation()
{
	int nAniIndex;//, nAniType, nAniAttr;
	--m_nAniLoopCount;
	if( m_nAniLoopCount > 0 || m_nStunDelay > 0 ) {
		ChangeAnimationByIndex( GetAniIndex(), false, m_nAniLoopCount );

		m_fFrame = 0.0f;
		m_fPrevFrame = 0.0f;

		m_HitParam.nHitRemainFrame = 0;
		m_HitParam.ObjectHandle.Identity();
		m_HitParam.pHitSignal = NULL;

		return;
	}

	nAniIndex = m_pUnitInfoData->GetNextAni( m_nAniIndex ) - 1;
	if( ( ( m_nCurAniType == ANI_TYPE_DIE ) || ( m_nCurAniType == ANI_TYPE_DOWN ) 
		|| ( m_nCurAniType == ANI_TYPE_DOWN_HIT ) ) && ( m_fHP <= 0.f ) )	// 에휴 복잡해.. ani 타입 정리 해야겠다..
	{
		m_fFrame -= GetFrameAdd();
		if( m_fFrame < 0.0f )
		{
			m_fFrame = 0.0f;
		}
		SetDie();		// 죽었을때는 프로세스 멈추게 한다..
		if( !m_bItemDrop )
		{
			if(m_bBeatenByPlayer) {			
				g_FcItemManager.CreateItemToWorld( GetUnitSoxIndex(), GetLevel() , &m_Cross );
			}
			m_bItemDrop = true;
		}
		return;
	}
	/*	else if( m_nCurAniType == ANI_TYPE_DOWN )
	{
	nAniIndex = m_pUnitInfoData->GetAniIndex( ANI_TYPE_RISE, -1, m_nCurAniAttr );
	if( nAniIndex == -1 )	// 애들 애니가 Rise가 아직 없는 애들이 있다.. 그거 정리되면 빼자.. by mapping
	{
	nAniIndex = m_pUnitInfoData->GetNextAni( m_nAniIndex ) - 1;
	}
	}*/
	else if( nAniIndex == -1 )
	{
		nAniIndex = m_pUnitInfoData->GetAniIndex( ANI_TYPE_STAND, 0 );
	}

	ChangeAnimationByIndex( FindExactAni(nAniIndex) );

	m_fFrame = 0.0f;
	m_fPrevFrame = 0.0f;

	m_HitParam.nHitRemainFrame = 0;
	m_HitParam.ObjectHandle.Identity();
	m_HitParam.pHitSignal = NULL;
}

void CFcGameObject::GetGamePlayData()
{
	m_fHP = (float)GetMaxHP();
	m_bItemDrop = false;
}

void CFcGameObject::AddHPPercent( int nPercent )
{
	int nAddHP;

	nAddHP = ( int )( GetMaxHP() * ( nPercent / 100.0f ) );
	AddHP( nAddHP );
}

void CFcGameObject::AddHP(int nAddHP)
{
	int nMaxHP;

	nMaxHP = GetMaxHP();
	m_fHP += (float)nAddHP;
	if( m_fHP < 0.f )
	{
		m_fHP = 0.f;
	}
	if( m_fHP > (float)nMaxHP )
	{
		m_fHP = (float)nMaxHP;
	}
}

void CFcGameObject::AddExp( int nExp, GameObjHandle Handle, int nKillCount/* = 0*/ )
{
	int nAdd;

	nAdd = ( int )( nExp * ( 1.0f + m_nExpAdd / 100.0f ) );
	m_nExp += nAdd;
	if( m_nExp > _LIMIT_EXP )
	{
		m_nExp = _LIMIT_EXP;
	}
	if( m_hTroop )
	{
		if( m_hTroop->IsGuardian() )
		{
//			CFcWorld::GetInstance().GetHeroHandle( 0 )->AddExp( nAdd, Handle, 0 );
			CFcWorld::GetInstance().AddGuardianOrb( nAdd );
			CFcWorld::GetInstance().AddGuardianKillCount( 1 );
		}
	}
	else
	{
		BsAssert( 0 && "Troop Handle Is Not Assigned" );
	}
}

// 오브젝트 로컬 중심의 이동이 아니고.. 월드 기준 이동이다..
void CFcGameObject::Move( D3DXVECTOR3 *pMove )
{
	if( m_ParentLinkObjHandle ) return;

	m_MoveVector += *pMove;
}

void CFcGameObject::MoveX(float fDist) 
{ 
	m_MoveVector+=m_Cross.m_XVector*fDist; 
}

void CFcGameObject::MoveY(float fDist)
{ 
	m_MoveVector+=m_Cross.m_YVector*fDist; 
}

void CFcGameObject::MoveZ(float fDist)
{
	float fLength;

	fLength=D3DXVec3LengthSq(&m_MoveDir);
	if(fLength<=0.0f){
		m_MoveVector+=m_Cross.m_ZVector*fDist; 
	}
	else{
		m_MoveVector+=m_MoveDir*(fDist);
	}
}

float CFcGameObject::GetHeightFromLand()
{
	return m_Cross.m_PosVector.y - CFcWorld::GetInstance().GetLandHeight( m_Cross.m_PosVector.x, m_Cross.m_PosVector.z );
}


void CFcGameObject::CalculateMoveDir(KEY_EVENT_PARAM *pParam, int nCamIndex)
{
	D3DXVECTOR3 KeyVector, ZDir, XDir, UpVec;
	CCrossVector *pCross;

	KeyVector.x=(float)pParam->nPosX;
	KeyVector.y=0.0f;
	KeyVector.z=(float)pParam->nPosY;
	D3DXVec3Normalize(&KeyVector, &KeyVector);

	if(D3DXVec3LengthSq(&KeyVector)<=0.0f){
		return ;
	}

	if(s_CameraHandle[nCamIndex]){
		pCross=s_CameraHandle[nCamIndex]->GetCrossVector();
		ZDir=pCross->m_ZVector;
		ZDir.y=0;
		D3DXVec3Normalize(&ZDir, &ZDir);
		UpVec=D3DXVECTOR3(0, 1, 0);
		D3DXVec3Cross(&XDir, &UpVec, &ZDir);
		m_MoveDir=XDir*KeyVector.x;
		BsAssert( (m_MoveDir.x == m_MoveDir.x) && (m_MoveDir.y == m_MoveDir.y) && (m_MoveDir.z == m_MoveDir.z) );
		m_MoveDir+=ZDir*KeyVector.z;
		BsAssert( (m_MoveDir.x == m_MoveDir.x) && (m_MoveDir.y == m_MoveDir.y) && (m_MoveDir.z == m_MoveDir.z) );
		D3DXVec3Normalize(&m_MoveDir, &m_MoveDir);
	}
	else{
		m_MoveDir=D3DXVECTOR3(0, 0, 1);
	}

}

void CFcGameObject::SetLevel( int nLevel )
{ 
	m_nLevel = nLevel; 
	if( m_nUnitSOXIndex != -1 )
		GetGamePlayData();
}

int CFcGameObject::GetMaxHP()
{ 
	int nMaxHP;

	nMaxHP = CLevelTableLoader::GetInstance().GetMaxHP( m_nUnitSOXIndex, GetLevel(), m_bSoxType );
	return ( int )( nMaxHP * ( 1.0f + m_nMaxHPAddPer / 100.0f ) ) + m_nMaxHPAdd; 
}

int CFcGameObject::GetAttackPower()
{
	int nAttackPower;

	nAttackPower = CLevelTableLoader::GetInstance().GetAttackPoint( m_nUnitSOXIndex, GetLevel(), m_bSoxType );
	return ( int )( nAttackPower * ( 1.0f + m_nAttackPowerAdd / 100.0f ) );
}

int CFcGameObject::GetDefense()
{
	int nDefense;

	nDefense = CLevelTableLoader::GetInstance().GetDefensePoint( m_nUnitSOXIndex, GetLevel(), m_bSoxType );
	return ( int )( nDefense * ( 1.0f + m_nDefenseAdd / 100.0f ) ); 
}

int CFcGameObject::GetDefenseProb()
{
	int nDefenseProb;
	int nSoxType = (int)m_bSoxType;

	if( CFcBaseObject::IsValid( m_HitParam.ObjectHandle ) ) {
		switch( m_HitParam.ObjectHandle->GetClassID() ) {
			case Class_ID_Hero:
			case Class_ID_Adjutant:
				nSoxType = 2;
				break;
		}
	}

	nDefenseProb = CLevelTableLoader::GetInstance().GetDefenseProb( m_nUnitSOXIndex, GetLevel(), nSoxType );

	return ( int )( nDefenseProb * ( 1.f + m_nDefenseProbAdd / 100.f ) );
}

int CFcGameObject::GetCritical()
{
	int nCritical;
	float fCirticalAdd;

	nCritical = CLevelTableLoader::GetInstance().GetCritical( m_nUnitSOXIndex, GetLevel(), m_bSoxType );
	fCirticalAdd = 1.0f + m_nCriticalAdd / 100.0f;
	if( m_bNowSpecialAttack )
	{
		fCirticalAdd += m_nSpecialAttackCriticalAdd / 100.0f;
	}
	return ( int ) ( nCritical * ( 1.0f + m_nCriticalAdd / 100.0f ) );
}

float CFcGameObject::GetCriticalRatio()
{
	return CLevelTableLoader::GetInstance().GetCriticalRatio( m_nUnitSOXIndex, GetLevel(), m_bSoxType ) + ( m_nCriticalRatioAdd / 100.0f );
}

bool CFcGameObject::IsGuardBreak( GameObjHandle Hitter )
{
	int nGuardBrek;

	if( ( Hitter->IsBoss() ) && ( m_nBossGuardBreak > 0 ) )
	{
		nGuardBrek = m_nBossGuardBreak;
	}
	else
	{
		nGuardBrek = m_nGuardBreak;
	}
	if( Random( 100 ) < nGuardBrek )
	{
		return true;
	}

	return false;
}

/*
void CFcGameObject::SetMaxHP( int nMaxHP )
{
CLevelTableLoader::GetInstance().SetMaxHP( m_nUnitSOXIndex, GetLevel(), nMaxHP ); 
}

void CFcGameObject::SetAttackPower( int nAttackPower )
{
CLevelTableLoader::GetInstance().SetAttackPoint( m_nUnitSOXIndex, GetLevel(), nAttackPower ); 
}

void CFcGameObject::SetDefense( int nDefencePoint )
{
CLevelTableLoader::GetInstance().SetDefensePoint( m_nUnitSOXIndex, GetLevel(), nDefencePoint );
}
*/

int CFcGameObject::GetCurAniAttr()
{
	return m_pUnitInfoData->GetAniAttr( m_nAniIndex );
}


int CFcGameObject::GetCurAniType()
{
	return m_nCurAniType + m_pUnitInfoData->GetAniAttr( m_nAniIndex );
}

int CFcGameObject::GetCurAniIndex()
{
	return m_pUnitInfoData->GetAniTypeIndex( m_nAniIndex );
}

int CFcGameObject::GetDischargeOrbSpark()
{
	return CLevelTableLoader::GetInstance().GetOrbSparkPoint( m_nUnitSOXIndex, GetLevel() );
}

int CFcGameObject::GetGiveExp()
{
	return CLevelTableLoader::GetInstance().GetGiveExpPoint( m_nUnitSOXIndex, GetLevel() );
}

void CFcGameObject::OrbSparkOn()
{
	s_nIsOrbSparkOn++;
}

void CFcGameObject::OrbSparkOff()
{
	s_nIsOrbSparkOn--;
}

bool CFcGameObject::IsMovable()
{
	if( s_nUnitStateTable[ m_nCurAniType ] & UNIT_MOVABLE )
	{
		return true;
	}

	return false;
}

bool CFcGameObject::IsAttackable()
{
	if( s_nUnitStateTable[ m_nCurAniType ] & UNIT_ATTACKABLE )
	{
		return true;
	}

	return false;
}

bool CFcGameObject::IsJumpable()
{
	if( s_nUnitStateTable[ m_nCurAniType ] & UNIT_JUMPABLE )
	{
		return true;
	}

	return false;
}

bool CFcGameObject::IsFlockable( GameObjHandle Handle )
{
	if( Handle == m_Handle ) return false;
	if( Handle->GetParentLinkHandle() == m_Handle ) return false;
	if( s_nUnitStateTable[ m_nCurAniType ] & UNIT_FLOCKABLE )
	{
		return true;
	}
	return false;
}

bool CFcGameObject::IsRidable()
{
	if( s_nUnitStateTable[ m_nCurAniType ] & UNIT_RIDABLE )
	{
		return true;
	}
	return false;
}

bool CFcGameObject::IsAttack()
{
	if( s_nUnitStateTable[ m_nCurAniType ] & UNIT_STATE_ATTACK )
	{
		return true;
	}

	return false;
}

// Jump attack포함, Jump hit포함
bool CFcGameObject::IsJump()
{
	if( s_nUnitStateTable[ m_nCurAniType ] & UNIT_STATE_JUMP )
	{
		return true;
	}

	return false;
}

bool CFcGameObject::IsHit()
{
	if( s_nUnitStateTable[ m_nCurAniType ] & UNIT_STATE_DAMAGE )
	{
		return true;
	}

	return false;
}

bool CFcGameObject::IsMove()
{
	if( s_nUnitStateTable[ m_nCurAniType ] & UNIT_STATE_MOVE )
	{
		return true;
	}

	return false;
}

// Rise포함
bool CFcGameObject::IsDown()
{
	if( s_nUnitStateTable[ m_nCurAniType ] & UNIT_STATE_DOWN )
	{
		return true;
	}

	return false;
}

bool CFcGameObject::IsDie( bool bCheckEnable )
{
	if( bCheckEnable && !m_bEnable ) return true;
	return m_bDead;
}

bool CFcGameObject::CheckHit()
{
	if( ( m_nDieFrame > DIE_KEEP_FRAME ) || ( m_nRagDollMode ) )
	{
		return false;
	}
	if( m_nCurAniType == ANI_TYPE_DIE )
	{
		if( ( int )m_fFrame >= ( int )( m_nCurAniLength * 0.6f ) )
		{
			return false;
		}
	}
	else if( ( m_fHP <= 0.f ) && ( ( m_nCurAniType == ANI_TYPE_HIT ) || ( m_nCurAniType == ANI_TYPE_DOWN_HIT ) ) )
	{
		if( ( int )m_fFrame >= m_nCurAniLength - 1 )
		{
			return false;
		}
	}

	return true;
}

bool CFcGameObject::IsStand()
{
	if( s_nUnitStateTable[ m_nCurAniType ] & UNIT_STATE_STAY )
	{
		return true;
	}

	return false;
}

bool CFcGameObject::IsTramplable()
{
	if( s_nUnitStateTable[ m_nCurAniType ] & UNIT_STATE_TRAMPLE )
	{
		return true;
	}

	return false;
}

bool CFcGameObject::IsDefensible()
{
	if( GetCurAniAttr() == ANI_ATTR_HORSE ) return false;
	if( s_nUnitStateTable[ m_nCurAniType ] & UNIT_DEFENSIBLE )
	{
		return true;
	}
	return false;
}

void CFcGameObject::SetDie( int nWeaponDropPercent /*= 100*/ )
{
	if( nWeaponDropPercent < Random( 100 ) + 1 )
	{
		m_nCurrentWeaponIndex = -1;
	}
	if( m_nDieFrame == 0 )
	{
		m_nDieFrame = 1; 
		if( m_hTroop )
		{
			// 죽을때는 CmdDie만 날려서 부대의 킬수만 올려준다..
			// 실제 부대에서 빼는건 객체 삭제할때이다..
			// 그래서 RemoveUnit() 은 객체 삭제하는곳으로 이동..
			m_hTroop->CmdDie( m_Handle );
		}
	}
	m_bDead = true;

	//	DebugString( "SetDie %d %d %d %x\n", m_Handle.GetID(), m_Handle.GetIndex(), GetProcessTick(), m_Handle.GetPointer() );
}

float CFcGameObject::GetUnitRadius() 
{ 
	return ( float )m_pUnitSOX->nUnitSize; 
}

float CFcGameObject::GetUnitHeight() 
{ 
	return m_pUnitSOX->nUnitHeight * m_fScale; 
}

void CFcGameObject::ProcessCollsionProp()
{
}

D3DXVECTOR2 g_DirVector[4] =
{
	D3DXVECTOR2( -0.7101f, 0.7101f ),
		D3DXVECTOR2( 0.7101f, 0.7101f ),
		D3DXVECTOR2( 0.7101f, -0.7101f ),
		D3DXVECTOR2( -0.7101f, -0.7101f )
};

bool CFcGameObject::CheckDiagonalBlock( float fX, float fY )
{
	BYTE cAttrEx = CFcWorld::GetInstance().GetAttrEx( fX, fY );
	BYTE cDiagonal = cAttrEx & 0x07;

	if( m_ClassID == Class_ID_Hero && g_FcWorld.GetHeroHandle() == m_Handle ) {
		if( ( ( cAttrEx & 0x10 ) >> 4 ) == 1 ) return true;
	}
	int nX = int(fX) / 100;
	int nY = int(fY) / 100;
	float fTempX = fX - ( nX * 100.f );
	float fTempY = fY - ( nY * 100.f );

	switch( cDiagonal ) {
		case 1:
			if( fTempX + fTempY > 100.f ) return false;
			break;
		case 2:
			if( fTempX > fTempY ) return false;
			break;
		case 3:
			if( fTempX + fTempY < 100.f ) return false;
			break;
		case 4:
			if( fTempX < fTempY ) return false;
			break;
	}
	return true;
}

bool CFcGameObject::IsCanMoveBlock( BYTE cAttr, BYTE cAttrEx )
{
	// 추가 속성 0000 1000 트리거 이벤트 - 아군만 못가는 속성
	if( GetTeam() == 0 && ( cAttrEx & 0x08 ) >> 3 == 1 ) return false;

	// 기본 속성 0000 0100 다리 이벤트
	// 다리 이벤트에 못가는 속성 겹쳐 있으면 못간다.
	if( cAttr == 0 || ( cAttr & 0x04 ) >> 2 == 1 && ((cAttr & 0x02 ) == 0)  ) return true;

	// 기본 속성 0001 0000 히어로만 갈 수 있는 속성
	if( ( cAttr & 0x10 ) >> 4 == 1 ) {
		if( m_Handle != g_FcWorld.GetHeroHandle() ) return false;
		else return true;
	}

	if( GetClassID() == Class_ID_Unit || GetClassID() == Class_ID_Archer )
	{
		if( ( m_nCurAniType == ANI_TYPE_DOWN ) && ( cAttr & 0x8 ) )
		{
			SetHP( 0 );
			return true;
		}
	}

	return false;
}

void CFcGameObject::CheckAndAddPos( D3DXVECTOR2 &vResult, float fWidth, float fHeight )
{
	float fTemp;
	BYTE cAttr[2];

	fTemp = m_Cross.m_PosVector.x + vResult.x;
	if( fTemp >= 0.f && fTemp < fWidth ) {
		cAttr[0] = CFcWorld::GetInstance().GetAttr( fTemp, m_Cross.m_PosVector.z );
		cAttr[1] = CFcWorld::GetInstance().GetAttrEx( fTemp, m_Cross.m_PosVector.z );
		if( IsCanMoveBlock( cAttr[0], cAttr[1] ) )
			m_Cross.m_PosVector.x += vResult.x;
	}

	fTemp = m_Cross.m_PosVector.z + vResult.y;
	if( fTemp >= 0.f && fTemp < fHeight ) {
		cAttr[0] = CFcWorld::GetInstance().GetAttr( m_Cross.m_PosVector.x, fTemp );
		cAttr[1] = CFcWorld::GetInstance().GetAttrEx( m_Cross.m_PosVector.x, fTemp );
		if( IsCanMoveBlock( cAttr[0], cAttr[1] ) )
			m_Cross.m_PosVector.z += vResult.y;
	}
}

int g_DebugCAUnitSOXIndex;
CCrossVector g_DebugCACross;
D3DXVECTOR3 g_DebugCAResultPos;

void CFcGameObject::AddPosCheckAttribute( float x, float z )
{
	float fWidth, fHeight, fResultX, fResultZ;
	fResultX = m_Cross.m_PosVector.x + x;
	fResultZ = m_Cross.m_PosVector.z + z;
	g_FcWorld.GetMapSize( fWidth, fHeight );

	if( fResultX < 0 || fResultX >= fWidth || fResultZ < 0 || fResultZ >= fHeight ) {
		return;
	}

	BYTE cPrevAttr = CFcWorld::GetInstance().GetAttr( m_Cross.m_PosVector.x, m_Cross.m_PosVector.z );
	BYTE cPrevAttrEx = CFcWorld::GetInstance().GetAttrEx( m_Cross.m_PosVector.x, m_Cross.m_PosVector.z );
	if( IsCanMoveBlock( cPrevAttr, cPrevAttrEx ) == false /*&& ( cPrevAttrEx & 0x07) == 0*/ ) {
		m_Cross.m_PosVector.x += x;
		m_Cross.m_PosVector.z += z;
		return;
	}

	g_DebugCAUnitSOXIndex = m_nUnitSOXIndex;
	g_DebugCACross = m_Cross;
	g_DebugCAResultPos = D3DXVECTOR3( fResultX, 0.f, fResultZ );

	if( GetClassID() != Class_ID_Fly && m_bForceMove == false ) {
		BYTE cAttr = CFcWorld::GetInstance().GetAttrEx( m_Cross.m_PosVector.x + x, m_Cross.m_PosVector.z + z );
		BYTE cDiagonal = cAttr & 0x07; // 0x03
		bool bEventAttr = ( ( cAttr & 0x08/*0x04*/ ) >> 3/*2*/ == 0 ) ? false : true;

		if( m_Handle == g_FcWorld.GetHeroHandle() )
			g_FcWorld.SetHeroEventBlockArea( bEventAttr );

		if( cDiagonal > 0 && !bEventAttr ) {
			bool bCheck = CheckDiagonalBlock( m_Cross.m_PosVector.x + x, m_Cross.m_PosVector.z + z );
			if( bCheck == true  ) {
				m_Cross.m_PosVector.x += x;
				m_Cross.m_PosVector.z += z;
			}
			else {
				float fLength, fDot;
				D3DXVECTOR2 MoveDir( x, z );

				fLength = D3DXVec2Length( &MoveDir );
				D3DXVec2Normalize( &MoveDir, &MoveDir );
				fDot = D3DXVec2Dot( &MoveDir, &( g_DirVector[ cDiagonal - 1 ] ) );

				D3DXVECTOR2 vResult = ( fDot * fLength ) * g_DirVector[ cDiagonal - 1 ];

				CheckAndAddPos( vResult, fWidth, fHeight );
			}
		}
		else {
			cAttr = CFcWorld::GetInstance().GetAttrEx( m_Cross.m_PosVector.x, m_Cross.m_PosVector.z );
			cDiagonal = cAttr & 0x07;
			bEventAttr = ( ( cAttr & 0x08 ) >> 3 == 0 ) ? false : true;
			if( cDiagonal > 0 && !bEventAttr ) {
				bool bCheck = CheckDiagonalBlock( m_Cross.m_PosVector.x, m_Cross.m_PosVector.z );
				if( bCheck == true && CheckDiagonalBlock( m_Cross.m_PosVector.x + x, m_Cross.m_PosVector.z + z ) ) {
					CheckAndAddPos( D3DXVECTOR2( x, z ), fWidth, fHeight );
				}
				else {
					int nX = ( int )( m_Cross.m_PosVector.x ) / 100;
					int nZ = ( int )( m_Cross.m_PosVector.z ) / 100;
					float fX = ( m_Cross.m_PosVector.x ) - ( float )( nX * 100.f );
					float fZ = ( m_Cross.m_PosVector.z ) - ( float )( nZ * 100.f );

					float fLength, fDot;
					D3DXVECTOR2 MoveDir( x, z );

					fLength = D3DXVec2Length( &MoveDir );
					D3DXVec2Normalize( &MoveDir, &MoveDir );
					fDot = D3DXVec2Dot( &MoveDir, &( g_DirVector[ cDiagonal - 1 ] ) );

					D3DXVECTOR2 vResult = ( fDot * fLength ) * g_DirVector[ cDiagonal - 1 ];
					CheckAndAddPos( vResult, fWidth, fHeight );
				}
			}
			else {
				bool bFlag = true;
				if( ( CFcWorld::GetInstance().GetAttr( m_Cross.m_PosVector.x, m_Cross.m_PosVector.z ) == 0 &&
					CFcWorld::GetInstance().GetAttrEx( m_Cross.m_PosVector.x, m_Cross.m_PosVector.z ) == 0 ) &&
					( CFcWorld::GetInstance().GetAttr( m_Cross.m_PosVector.x + x, m_Cross.m_PosVector.z + z ) != 0 ||
					CFcWorld::GetInstance().GetAttrEx( m_Cross.m_PosVector.x + x, m_Cross.m_PosVector.z + z ) != 0 ) ) {
						D3DXVECTOR2 vVec = D3DXVECTOR2( m_Cross.m_PosVector.x + x, m_Cross.m_PosVector.z + z ) - D3DXVECTOR2( m_Cross.m_PosVector.x, m_Cross.m_PosVector.z );
						D3DXVECTOR2 vTemp = D3DXVECTOR2( m_Cross.m_PosVector.x + x, m_Cross.m_PosVector.z + z );
						D3DXVec2Normalize( &vVec, &vVec );

						D3DXVECTOR2 vCross[2];
						vCross[0] = vTemp + ( D3DXVECTOR2( -vVec.y, vVec.x ) * (float)100.f );
						vCross[1] = vTemp + ( D3DXVECTOR2( vVec.y, -vVec.x ) * (float)100.f );

						int cTemp[2] = { CFcWorld::GetInstance().GetAttrEx( vCross[0].x, vCross[0].y ) & 0x07,
							CFcWorld::GetInstance().GetAttrEx( vCross[1].x, vCross[1].y ) & 0x07 };
						if( cTemp[0] > 0 && cTemp[1] > 0 && cTemp[0] == cTemp[1] ) {
							float fLength, fDot;
							D3DXVECTOR2 MoveDir( x, z );

							fLength = D3DXVec2Length( &MoveDir );
							D3DXVec2Normalize( &MoveDir, &MoveDir );
							fDot = D3DXVec2Dot( &MoveDir, &( g_DirVector[ cTemp[0] - 1 ] ) );

							D3DXVECTOR2 vResult = ( fDot * fLength ) * g_DirVector[ cTemp[0] - 1 ];

							CheckAndAddPos( vResult, fWidth, fHeight );
							bFlag = false;
						}
					}
					if( bFlag == true ) {
						CheckAndAddPos( D3DXVECTOR2( x, z ), fWidth, fHeight );
					}
			}
		}
	}
	else {
		m_Cross.m_PosVector.x += x;
		m_Cross.m_PosVector.z += z;
		m_bForceMove = false;
	}

}

void CFcGameObject::AddPosCheckProp( float &x, float &z )
{	
	if( GetClassID() == Class_ID_Hero ) { 		
		g_FcWorld.AddPosCheckDynamicProp( m_Cross.m_PosVector, x, z );
	}

	g_FcWorld.AddPosCheckCollisionProp( m_Cross.m_PosVector, 80.f, x, z);
}

float CFcGameObject::AddPos( float x, float y, float z )
{
	AddPosCheckProp	( x, z );
	if( x != 0.f || z != 0.f ) {
		D3DXVECTOR2 vVec = D3DXVECTOR2( x, z );
		float fLength = D3DXVec2Length( &vVec );
		if( fLength >= 100.f && m_bForceMove == false ) {
			int nCount = (int)(fLength / 100);
			D3DXVECTOR2 vVecRemain;
			D3DXVec2Normalize( &vVec, &vVec );
			vVecRemain = D3DXVECTOR2( x, z ) - ( vVec * ( nCount * 100.f ) );

			for( int i=0; i<nCount; i++ ) {
				AddPosCheckAttribute( vVec.x * 100.f, vVec.y * 100.f );
			}
			AddPosCheckAttribute( vVecRemain.x, vVecRemain.y );
		}
		else AddPosCheckAttribute( x, z );
	}
	m_Cross.m_PosVector.y += y;

	m_DummyPos = m_Cross.m_PosVector - m_LocalAniMove;

	float fPropHeight;
	bool bPropHeight = false;

	float fLandHeight = g_FcWorld.GetLandHeight( m_Cross.m_PosVector.x, m_Cross.m_PosVector.z, &bPropHeight);
	if( bPropHeight == true )
	{
		fPropHeight = fLandHeight;
		fLandHeight = g_BsKernel.GetLandHeight( m_Cross.m_PosVector.x, m_Cross.m_PosVector.z );

		// 프랍높이가 더 높다면
		if( fPropHeight > fLandHeight )
		{
			// 프랍높이가 80cm이하면 올라간다
			if( fPropHeight - m_Cross.m_PosVector.y < 80.f )				
			{
				return fPropHeight;
			}
			// 이동거리가 3m 이상인경우에도 무조건 올라간다. 
			if( x * x + z * z > 300.f * 300.f )
			{
				return fPropHeight;
			}
		}
	}

	return fLandHeight;
}

float CFcGameObject::SetPos( float x, float z)
{
	m_Cross.m_PosVector.x = x;
	m_Cross.m_PosVector.z = z;

	bool bPropOut;
	float fHeight = g_FcWorld.GetLandHeight( x, z, &bPropOut );

	if( !bPropOut ) {
		m_fLastHeight = fHeight;
	}
	return fHeight;
}

// Test
GameObjHandle CFcGameObject::GetProjecTileTarget( TroopObjHandle hTroop )
{
	int nTargetLiveCnt = hTroop->GetNumLiveSoldier();
	if( nTargetLiveCnt == 0 ) 
	{
		GameObjHandle hUnit;
		return hUnit;
	}

	int nIndex = Random( nTargetLiveCnt );

	int j=0;
	for( int i=0; i<hTroop->GetUnitCount(); i++ )
	{
		UnitObjHandle hUnit = hTroop->GetUnit( i );
		if( hUnit->m_bDead ) continue;
		if( j == nIndex )
		{
			return hUnit;
		}
		++j;
	}
	GameObjHandle hUnit;
	return hUnit;
}


#define MOVE_LOCK_POS_THRESHHOLD	100.f
#define MOVE_LOCK_DELAY_TICK		3
#define MOVE_LOCK_DELAY_TICK_2		5
#define ADD_MOVE_LOCK_STAY_DELAY_TICK		5

void CFcGameObject::CheckMoveLock( TroopObjHandle hTroop )
{
	if( m_bDead ) return;

	bool bEnable1 = false;
	bool bEnable2 = false;
	D3DXVECTOR2 Dir = hTroop->GetPosV2() + m_TroopOffsetPos - GetPosV2();

	// 낑겨있을 때 처리
//	if(( hTroop->GetState() != TROOPSTATE_MELEE_ATTACK && 
//		hTroop->GetState() != TROOPSTATE_RANGE_ATTACK ) )
	if( m_nCurAniType == ANI_TYPE_WALK || m_nCurAniType == ANI_TYPE_RUN || m_nCurAniType == ANI_TYPE_RUN_ATTACK )
	{
		if( fabs( m_nMoveLockPos.x - m_Cross.m_PosVector.x ) < MOVE_LOCK_POS_THRESHHOLD &&
			fabs( m_nMoveLockPos.y - m_Cross.m_PosVector.z ) < MOVE_LOCK_POS_THRESHHOLD )
		{
			if( D3DXVec2LengthSq( &Dir ) > 500.f * 500.f )
			{
				if( m_nMoveLockTick1 == -1 )
				{
					m_nMoveLockTick1 = MOVE_LOCK_DELAY_TICK;		// 3초 동안 여유를 준다.
					if(IsEnabledMoveDelay()==false)				// 이동중이 아닐 때는 8초의 여유를 준다.
						m_nMoveLockTick1 += ADD_MOVE_LOCK_STAY_DELAY_TICK;
				}
				else if( m_nMoveLockTick1 > 0 )
				{
					--m_nMoveLockTick1;
					if( m_nMoveLockTick1 == 0 )
					{
						m_nMoveLockTick1 = -1;
						D3DXVECTOR2 vResult = hTroop->GetPosV2() + m_TroopOffsetPos;
						CmdPlace( vResult.x, vResult.y );
						if( m_hUnitAIHandle ) m_hUnitAIHandle->SetForceProcessTick( 1 );//m_hUnitAIHandle->ProcessLOD();
					}
				}
				bEnable1 = true;
			}
		}
	}
/*
	// 계속 벽에 비빌 때 처리
	if( m_nMovePathCnt > 80 )		// 2초동안 매틱 벽에 비비면 워프 시킨다.
	{
		m_nMoveLockTick = -1;
		D3DXVECTOR2 vResult = hTroop->GetPosV2() + m_TroopOffsetPos;
		CmdPlace( vResult.x, vResult.y );
		if( m_hUnitAIHandle ) m_hUnitAIHandle->SetForceProcessTick( 1 );
		bEnable = true;
	}
*/
	// 거리로 체크
	float fRadius = hTroop->GetRadius() + 4000.f;
	if( D3DXVec2LengthSq( &Dir ) > (fRadius * fRadius) )
	{
		if( m_nMoveLockTick2 == -1 )
		{
			m_nMoveLockTick2 = MOVE_LOCK_DELAY_TICK_2;
		}
		else if( m_nMoveLockTick2 > 0 )
		{
			--m_nMoveLockTick2;
			if( m_nMoveLockTick2 == 0 )
			{
				m_nMoveLockTick2 = -1;
				D3DXVECTOR2 vResult = hTroop->GetPosV2() + m_TroopOffsetPos;
				CmdPlace( vResult.x, vResult.y );
				if( m_hUnitAIHandle ) m_hUnitAIHandle->SetForceProcessTick( 1 );//m_hUnitAIHandle->ProcessLOD();
			}
		}
		bEnable2 = true;
	}
	if( bEnable1 == false && bEnable2 == false )
	{
		m_nMoveLockTick1 = -1;
		m_nMoveLockTick2 = -1;
		m_nMoveLockPos = D3DXVECTOR2( m_Cross.m_PosVector.x, m_Cross.m_PosVector.z );
	}
}

void CFcGameObject::CheckUnitSpeed( TroopObjHandle hTroop )
{
	D3DXVECTOR2 Dir = GetPosV2() - (m_hTroop->GetPosV2() + m_TroopOffsetPos);
	float fDist = D3DXVec2LengthSq( &Dir );
	if( fDist >= (2000.f * 2000.f) )
	{
		SetMoveSpeedRateForTroop( 1.6f );
	}
	else if( fDist < (2000.f * 2000.f) && fDist >= (800.f * 800.f) )
	{
		SetMoveSpeedRateForTroop( 1.3f );
	}
	else
	{
		SetMoveSpeedRateForTroop( 1.f );
	}
}

void CFcGameObject::SetParentLinkHandle( GameObjHandle Handle )
{
	m_ParentLinkObjHandle = Handle;
}

bool CFcGameObject::RideOn( GameObjHandle RiderHandle )
{
	if( RiderHandle->LinkMe( m_Handle ) == false ) return false;

	m_ParentLinkObjHandle = RiderHandle;

	switch( RiderHandle->GetClassID() ) {
		case Class_ID_Horse:
			// AI Change
			if( m_hUnitAIHandle )
				m_hUnitAIHandle->SetEnable( false );
			if( m_ParentLinkObjHandle->GetAIHandle() )
				m_ParentLinkObjHandle->GetAIHandle()->SetEnable( true );

			ChangeAnimation( ANI_TYPE_STAND, 0, ANI_ATTR_HORSE );
			g_BsKernel.SetPreCalcAni( m_nEngineIndex, true);

			break;
		case Class_ID_Catapult:
			break;
	}

	return true;

}


bool CFcGameObject::RideOut()
{
	if( !m_ParentLinkObjHandle ) return false;

	if( m_ParentLinkObjHandle->UnLinkMe( m_Handle ) == false ) return false;

	switch( m_ParentLinkObjHandle->GetClassID() ) {
		case Class_ID_Horse:
			// AI Change
			if( m_hUnitAIHandle )
				m_hUnitAIHandle->SetEnable( true );
			if( m_ParentLinkObjHandle->GetAIHandle() ) 
				m_ParentLinkObjHandle->GetAIHandle()->SetEnable( false );

			g_BsKernel.SetPreCalcAni( m_nEngineIndex, true);

			break;
		case Class_ID_Catapult:
			break;
	}
	m_ParentLinkObjHandle.Identity();

	return true;

}

void CFcGameObject::SetAIHandle( AIObjHandle &Handle )
{
	m_hUnitAIHandle = Handle;
	if( m_hUnitAIHandle ) {
		m_nAIPreloadIndex = CFcAIObject::GetPreloadIndex( Handle );
	}
}

AIObjHandle CFcGameObject::GetAIHandle()
{
	return m_hUnitAIHandle;
} 

void CFcGameObject::DebugRender()
{
	CFcBaseObject::DebugRender();
}

ASData *CFcGameObject::GetASData()
{
	return m_pASData;
}

CAniInfoData *CFcGameObject::GetUnitInfoData()
{
	return m_pUnitInfoData;
}

void CFcGameObject::SetASData( ASData *pData )
{
	m_pASData = pData;
}

void CFcGameObject::SetUnitInfoData( CAniInfoData *pData )
{
	m_pUnitInfoData = pData;
}

void CFcGameObject::Enable( bool bEnable, bool bFullHP )
{
	if( m_bEnable == bEnable ) return;
	CFcBaseObject::Enable( bEnable );

	/*
	if( bEnable == true )
	ChangeAnimation( ANI_TYPE_STAND, 0 );

	if( bFullHP )
	{
	SetHP( GetMaxHP() );
	}
	*/

	DebugString( "Enable : %d, [%s/%s] ( %.2f, %.2f, %.2f )\n", m_Handle.m_nHandle, bEnable == true ? "True" : "False", m_bIsInScanner == true ? "True" : "False", m_Cross.m_PosVector.x, m_Cross.m_PosVector.y, m_Cross.m_PosVector.z );

	SetInScanner( bEnable );
	if( bEnable )
	{
		ResetMoveVector();
	}
}

void CFcGameObject::SetInScanner( bool bFlag )
{
	if( m_bIsInScanner == bFlag ) return;
	m_bIsInScanner = bFlag;
	if( m_bIsInScanner == true ) {
		m_PrevPos = m_Cross.m_PosVector;
		CFcWorld::GetInstance().AddUnitToScanner( m_Handle );
		DebugString( "In Scanner! %d ( %.2f, %.2f, %.2f)\n", m_Handle.m_nHandle, m_Cross.m_PosVector.x, m_Cross.m_PosVector.y, m_Cross.m_PosVector.z );
	}
	else {
		CFcWorld::GetInstance().MoveUnitToScanner(m_Handle, &m_PrevPos);
		CFcWorld::GetInstance().RemoveUnitToScanner( m_Handle );
		DebugString( "Out Scanner! %d ( %.2f, %.2f, %.2f)\n", m_Handle.m_nHandle, m_Cross.m_PosVector.x, m_Cross.m_PosVector.y, m_Cross.m_PosVector.z );
	}
}

bool CFcGameObject::GetTroopMovePos( D3DXVECTOR3 &vResult )
{
	if( !m_hTroop ) return false;
	std::vector<D3DXVECTOR3> *pVecList = m_hTroop->GetMoveQueue();
	if( pVecList->empty() ) return false;
	if( m_nTroopMoveQueueCount == pVecList->size() - 1 ) return false;

	vResult = (*pVecList)[m_nTroopMoveQueueCount];
	vResult.x += GetTroopOffset().x;
	vResult.z += GetTroopOffset().y;

	return true;
}

void CFcGameObject::CheckInTroopMovePos()
{
	if( !m_hTroop ) return;
	std::vector<D3DXVECTOR3> *pVecList = m_hTroop->GetMoveQueue();
	if( pVecList->empty() ) return;

	m_nTroopMoveQueueCount++;
}

bool CFcGameObject::IsUseTroopMoveQueue()
{
	if( m_nTroopMoveQueueCount == -1 || m_nTroopMoveQueueCount == m_hTroop->GetMoveQueue()->size() - 1 ) return false; 
	else return true;
}

bool CFcGameObject::GetExactHitAni( int &nAniType, int &nAniIndex )
{
	if( nAniType == ANI_TYPE_DOWN /*|| m_pUnitInfoData->GetAniTypeCount( ANI_ATTR_NONE, ANI_TYPE_HIT ) == 0*/ )
	{
//		nAniType = ANI_TYPE_DOWN;
		switch( nAniIndex )
		{
		case 0:
			nAniIndex = Random( 3 );
			break;
		case 1:
			nAniIndex = Random( 3 ) + 3;
			break;
		case 2:
			nAniIndex = Random( 2 ) + 6;
			break;
		case 3:
			nAniIndex = 8;
			break;
		}
	}
	else if( nAniType == ANI_TYPE_HIT )
	{
		if( Random( 100 ) < 5 )
		{
			nAniIndex = 6;
			return true;
		}
		switch( nAniIndex )
		{
		case 0:
			nAniIndex = Random( 2 );	// 1, 2, 7
			break;
		case 1:
			nAniIndex = Random( 3 ) * 2;	// 1, 3, 5, 7
			break;
		case 2:
			nAniIndex = Random( 3 ) * 2 + 1;	// 2, 4, 6, 7
			break;
		}
	}

	return true;
}

void CFcGameObject::GetExactHitVelocity( float &fVelocityZ, float &fVelocityY )
{
	if( m_ParentLinkObjHandle ) {
		fVelocityY = fVelocityZ = 0.f;
		return;
	}
	fVelocityZ += RandomNumberInRange( 0.0f, fVelocityZ * 0.1f );
	if( ( m_nCurAniType == ANI_TYPE_DOWN ) || ( m_nCurAniType == ANI_TYPE_DOWN_HIT ) )
	{
		fVelocityY += RandomNumberInRange( 0.0f, fVelocityY * 0.1f );
	}
	else
	{
		fVelocityY = 0.0f;
	}

	float fWeightRatio;

	fWeightRatio = ( m_pUnitSOX->fWeight - 1.0f ) / 20.0f;
	if( fWeightRatio > 0.5f )
	{
		fWeightRatio = 0.5f;
	}
	fVelocityZ -= fVelocityZ * fWeightRatio;
	fVelocityY -= fVelocityY * fWeightRatio;
}

void CFcGameObject::ShowParts( bool bShow )
{
	int i, nSize;

	nSize = m_ArmorList.size();
	for( i = 0; i < nSize; i++ )
	{
		g_BsKernel.ShowObject( m_ArmorList[ i ].nObjectIndex, bShow ); 
	}
}

D3DXVECTOR3 CFcGameObject::GetBone0Pos()
{
	char* pBoneName = (char*)g_BsKernel.SendMessage( m_nEngineIndex, BS_GET_BONENAME, 0 );
	D3DXMATRIX boneMatrix = *((D3DMATRIX*)g_BsKernel.SendMessage( m_nEngineIndex, BS_GET_BONE_MATRIX, (DWORD)pBoneName ));
	return D3DXVECTOR3( boneMatrix._41, boneMatrix._42, boneMatrix._43 );
}

void CFcGameObject::SetAniFrame( int nAniType, int nAniIndex, int nFrame )
{
	ChangeAnimation(nAniType , nAniIndex);
	int nIndex = m_pUnitInfoData->GetAniIndex( nAniType, nAniIndex, 0);
	if( nIndex == -1 ) return;
	int nTotalFrame = g_BsKernel.GetAniLength( m_nEngineIndex, nIndex );

	if( nFrame > nTotalFrame )	{ nFrame = nTotalFrame - 1; }
	if( nFrame < 0 )			{ nFrame = 0; }

	g_BsKernel.SetCurrentAni( m_nEngineIndex, nIndex, (float)nFrame );
	m_nBlendFrame = 0;
	m_fFrame = (float)nFrame;
}


bool CFcGameObject::SetMaterialAmbient(D3DXVECTOR4 vColor)
{
	if(m_pSaveMaterialAmbient == NULL){
		return false;
	}

	int nSubMeshCount = g_BsKernel.SendMessage(m_nEngineIndex, BS_GET_SUBMESH_COUNT);
	for(int i=0; i<nSubMeshCount; i++)
	{
		int nParamIndex = g_BsKernel.SendMessage(m_nEngineIndex, BS_ADD_EDITABLE_PARAMETER, i, PARAM_MATERIALAMBIENT);
		g_BsKernel.SendMessage(m_nEngineIndex, BS_SET_EDITABLE_PARAMETER, i, nParamIndex, DWORD(&vColor));
	}

	return true;
}

bool CFcGameObject::SaveMaterialAmbient()
{
	if(m_pSaveMaterialAmbient != NULL){
		return false;
	}

	int nSubMeshCount = g_BsKernel.SendMessage(m_nEngineIndex, BS_GET_SUBMESH_COUNT);
	m_pSaveMaterialAmbient = new D3DXVECTOR4[nSubMeshCount];

	D3DXCOLOR color;
	for(int i=0; i<nSubMeshCount; i++)
	{
		int nParamIndex = g_BsKernel.SendMessage(m_nEngineIndex, BS_ADD_EDITABLE_PARAMETER, i, PARAM_MATERIALAMBIENT);
		g_BsKernel.SendMessage(m_nEngineIndex, BS_GET_EDITABLE_PARAMETER, i, nParamIndex, (DWORD)&m_pSaveMaterialAmbient[i]);
	}

	return true;
}

bool CFcGameObject::RestoreMaterialAmbient()
{
	if(m_pSaveMaterialAmbient == NULL){
		return false;
	}

	int nSubMeshCount = g_BsKernel.SendMessage(m_nEngineIndex, BS_GET_SUBMESH_COUNT);

	for(int i=0; i<nSubMeshCount; i++)
	{
		int nParamIndex = g_BsKernel.SendMessage(m_nEngineIndex, BS_ADD_EDITABLE_PARAMETER, i, PARAM_MATERIALAMBIENT);
		g_BsKernel.SendMessage(m_nEngineIndex, BS_SET_EDITABLE_PARAMETER, i, nParamIndex, DWORD(&m_pSaveMaterialAmbient[i]));
	}

	delete [] m_pSaveMaterialAmbient;
	m_pSaveMaterialAmbient = NULL;

	return true;
}

float CFcGameObject::GetDistSqWayPoint()
{
	D3DXVECTOR2 Pos;
	bool bWayPoint = GetCurWayPoint( Pos );
	if( bWayPoint )
	{
		D3DXVECTOR2 Dir = GetPosV2() - Pos;
		return D3DXVec2LengthSq( &Dir );
	}
	return 0.f;
}

void CFcGameObject::PushWayPoint( D3DXVECTOR2* Pos )
{
	// 내 위치에서 더 가까운 waypoint가 있으면 무시한다.

	D3DXVECTOR2 Dir = *Pos + GetPosV2();
	float fNewDistSq = D3DXVec2LengthSq( &Dir );

	bool bValid = true;
	for( int i=0; i<m_nWayPointNum; i++ )
	{
		int nIndex = (m_nWayPointStartIndex+i) % 5;
		D3DXVECTOR2 Dir = m_WayPoint[nIndex] + GetPosV2();
		float fDistSq = D3DXVec2LengthSq( &Dir );
		if( fNewDistSq > fDistSq )
		{
			bValid = false;
			break;
		}
	}

	if( bValid == false )
		return;

	// 갯수 넘으면 제일 오래된 슬롯을 갱신
	int nIndex = (m_nWayPointStartIndex+m_nWayPointNum) % 5;
	m_WayPoint[nIndex] = *Pos;

	if( m_nWayPointNum < 5 )
		++m_nWayPointNum;
}

void CFcGameObject::PopWayPoint()
{
	if( m_nWayPointNum == 0 )
		return;

	++m_nWayPointStartIndex;
	if( m_nWayPointStartIndex >= 5 )
		m_nWayPointStartIndex = 0;

	--m_nWayPointNum;
}

bool CFcGameObject::GetCurWayPoint( D3DXVECTOR2& Pos )
{
	if( m_nWayPointNum == 0 )
		return false;

	Pos = m_WayPoint[m_nWayPointStartIndex];
	return true;
}


int CFcGameObject::FindExactAni( int nAniIndex )
{
	return nAniIndex;
}


void CFcGameObject::AIControlCheck( ASSignalData *pSignal )
{
	if( m_fFrame >= pSignal->m_nFrame ) {
		if( m_nAIControlIndex != pSignal->m_AIControlCheckIndex ) return;

		if( pSignal->m_AIControlCheckLockonTarget == TRUE ) {

			GameObjHandle hUnit;
			if( GetAIHandle() ) {
				GameObjHandle *pSearchHandle = NULL;
				for( int i=0; i<2; i++ ) {
					pSearchHandle = GetAIHandle()->GetSearchSlot()->GetHandle(i);
					if( pSearchHandle ) {
						hUnit = *pSearchHandle;
						break;
					}
				}
			}

			if( !CFcBaseObject::IsValid( hUnit ) ) return;

			bool bReleaseControl = false;
			D3DXVECTOR2 vVec = hUnit->GetPosV2() - GetPosV2();
			if( D3DXVec2LengthSq( &vVec ) <= 0.f ) bReleaseControl = true;
			else {
				D3DXVec2Normalize( &vVec, &vVec );

				float fDot = D3DXVec2Dot( &vVec, &D3DXVECTOR2( m_Cross.m_ZVector.x, m_Cross.m_ZVector.z ) );
				float fAngle = acos( fDot ) * 180.f / 3.1415926f;
				if( fAngle > (float)pSignal->m_AIControlCheckLockonAngle ) bReleaseControl = true;
			}
			CmdLookUp( hUnit );
			if( bReleaseControl == true ) {
				m_nAIControlIndex = -1;
				return;
			}
		}
		//		if( pSignal->m_AIControlCheckIndex == -1 ) return;

		int nAniAttr = 0;
		switch( pSignal->m_AIControlCheckAniAttr ) {
			case 0: nAniAttr = 0;	break;
			case 1:	nAniAttr = ANI_ATTR_BATTLE;	break;
			case 2:	nAniAttr = ANI_ATTR_BACK;	break;
			case 3:	nAniAttr = ANI_ATTR_HORSE;	break;
		}
		int nAniIndex = m_pUnitInfoData->GetAniIndex( pSignal->m_AIControlCheckAniType, pSignal->m_AIControlCheckAniTypeIndex, nAniAttr );
		ChangeAnimationByIndex( nAniIndex, true, 1, false );
		m_fFrame = (float)pSignal->m_AIControlCheckAniFrame;
		m_fPrevFrame = m_fFrame;

		if( pSignal->m_AIControlCheckReleaseIndex == TRUE ) {
			m_nAIControlIndex = -1;
		}
		ProcessSignal();
	}
}

void CFcGameObject::AIControlRelease( ASSignalData *pSignal )
{
	if( ( m_fFrame >= pSignal->m_nFrame ) && ( m_fFrame < pSignal->m_nFrame + GetFrameAdd() ) ) {
		m_nAIControlIndex = -1;
	}
}

void CFcGameObject::AIControlGoto( ASSignalData *pSignal )
{
	if( ( m_fFrame >= pSignal->m_nFrame ) && ( m_fFrame < pSignal->m_nFrame + pSignal->m_AIControlGotoLength ) )
	{
		if( m_nAIControlIndex == pSignal->m_AIControlGotoIndex ) {
			m_fFrame = ( float )pSignal->m_AIControlGotoFrame;
			m_fPrevFrame = m_fFrame;
		}
	}
}

void CFcGameObject::AIControlCallback( ASSignalData *pSignal )
{
	if( ( m_fFrame >= pSignal->m_nFrame ) && ( m_fFrame < pSignal->m_nFrame + GetFrameAdd() ) ) {
		m_nAIControlIndex = pSignal->m_AIControlCallbackIndex;
		if( m_hUnitAIHandle ) m_hUnitAIHandle->OnCallback( AI_CALLBACK_SIGNAL );
	}
}

void CFcGameObject::Reborn( float fHPPer )
{
	if( m_bDead == false ) return;
	if( m_bIsInScanner == false ) {
		BsAssert( 0 && "이럴수가!! Siva 한테!!" );
	}
	if( m_fFrame != m_pUnitInfoData->GetAniData( m_nAniIndex )->GetAniLength() - 1 ) return;
	if( m_hTroop->GetState() == TROOPSTATE_ELIMINATED ) {
		m_hTroop->SetDefaultState();
	}

	SetInScanner( true );

	m_bDead = false;
	m_nDieFrame = 0;
	ResetMoveVector();
	m_fHP = float( GetMaxHP() * fHPPer );

	// Link Weapon
	if( m_nDropWeaponIndex != -1 && m_nDropWeaponSkinIndex != -1 ) {
		m_WeaponList[ m_nDropWeaponIndex ].bShow = true;
		m_WeaponList[ m_nDropWeaponIndex ].nObjectIndex = g_BsKernel.CreateStaticObjectFromSkin( m_nDropWeaponSkinIndex );

		g_BsKernel.ReleaseSkin( m_nDropWeaponSkinIndex );
		m_nDropWeaponSkinIndex = -1;

		g_BsKernel.ShowObject( m_WeaponList[ m_nDropWeaponIndex ].nObjectIndex, true );
		m_nCurrentWeaponIndex = m_nDropWeaponIndex;

		g_BsKernel.SendMessage( m_nEngineIndex, BS_LINKOBJECT, ( DWORD )m_WeaponList[ m_nDropWeaponIndex ].nLinkBoneIndex, m_WeaponList[ m_nDropWeaponIndex ].nObjectIndex );
	}

	// Reinit AI Object
	if( m_nAIPreloadIndex == -1 ) return;
	if( m_hUnitAIHandle ) {
		BsAssert(0&&"이런일이!!");
		delete m_hUnitAIHandle;
	}

	AIObjHandle AIHandle;
	AIHandle = CFcAIObject::CreateObject<CFcAIObject>();
	if( AIHandle->Initialize( CFcAIObject::GetPreloadName( m_nAIPreloadIndex ), m_Handle ) == false ) {
		BsAssert(0&&"Can't reborn AI Object");
		return;
	}
	SetAIHandle( AIHandle );

	// Change Animation
	if( m_nCurAniType == ANI_TYPE_DIE ) {
		int nAniAttr = 0;
		if( m_nCurAniTypeIndex == 1 ) nAniAttr = ANI_ATTR_BACK;
		ChangeAnimation( ANI_TYPE_RISE, -1, nAniAttr );
	}
	else {
		NextAnimation();
	}

}



void CFcGameObject::ProcessObjectStatus()
{
	if( m_pObjectStatus ) {
		if( m_pObjectStatus->IsFinish() || m_bDead == true ) {
			SAFE_DELETE( m_pObjectStatus );
		}
		else m_pObjectStatus->Process();
	}
}

void CFcGameObject::PostProcessObjectStatus()
{
	if( m_pObjectStatus ) {
		m_pObjectStatus->PostProcess();
	}
}


void CFcGameObject::BeginStatus( CFcStatusObject::STATUS_TYPE Type, int nParam1, int nParam2, int nParam3, bool bOverlap )
{
	if( m_bDead ) return;
	if( m_bGodMode || m_bOnOrbsparkGodMode || m_bInvulnerable ) return;

	if( m_pObjectStatus ) {
		if( bOverlap == false ) return;
		else {
			SAFE_DELETE( m_pObjectStatus );
		}
	}

	m_pObjectStatus = CFcStatusObject::BeginStatus( m_Handle, Type, nParam1, nParam2, nParam3 );
}

void CFcGameObject::SetMaxHPAdd( int nAdd ,bool bPercent /*= true*/ ) 
{
	if(bPercent){	
		m_nMaxHPAddPer = nAdd; 
	}
	else
		m_nMaxHPAdd = nAdd;

	int nMaxHP;

	nMaxHP = GetMaxHP();
	if( m_fHP > (float)nMaxHP )
	{
		m_fHP = (float)nMaxHP;
	}
}

int CFcGameObject::GetMaxHPAdd( bool bPercent /*= true*/ )
{
	if(bPercent)
		return m_nMaxHPAddPer; 
	else
		return m_nMaxHPAdd;	
}

void CFcGameObject::SetRagDollMode( int nMode )
{
	if( m_nRagDollMode == RAGDOLL_MODE_NONE )
	{
		if( !m_bDropWeapon && m_nCurrentWeaponIndex != -1 )
		{
			DropWeapon();
		}
		m_bDropWeapon = true;
		if( m_hTroop )
		{
			m_hTroop->CmdDie( m_Handle );
			m_hTroop->RemoveUnit( m_Handle );
		}
		AddProcessList();	// Ragdoll Mode 되면 부대 컨트롤에서 완전히 빠진다..
	}
	CFcAniObject::SetRagDollMode( nMode );
}

void CFcGameObject::ResetMoveVector()
{
	m_MoveTargetPos = D3DXVECTOR3( 0.f, 0.f, 0.f );
	m_MoveDir = D3DXVECTOR3( 0.f, 0.f, 0.f );
}