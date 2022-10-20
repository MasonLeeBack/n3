#include "stdafx.h"
#include "FcMenu3DObject.h"
#include "BsKernel.h"
#include "BsClothObject.h"
#include "BsMaterial.h"
#include "FcSoxLoader.h"
#include "ASData.h"
#include "FcUtil.h"
#include "FcPhysicsLoader.h"
#include "BsFXObject.h"
#include "BsFXElement.h"
#include "BsPhysicsMgr.h"

#include "FcGlobal.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG


#define _CAMERA_POS				D3DXVECTOR3(0.f, 100.f, -300.f)
#define _LIGHT_POS				D3DXVECTOR3(0.f, 0.f, -400.f)
#define _CAMERA_TARGET_POS		D3DXVECTOR3(0.f, 100.f, 300.f)
#define _LIGHT_TARGET_POS		D3DXVECTOR3(0.f, 100.f, 400.f)

#define _ANGLE_MAX				360

//sox index순으로 배열되었다.
CharStandInfo g_CharStandInfo[_MENU_MAIN_CHAR_COUNT] = 
{
	{ 0.f,  85.f, -320.f, -50.f,  85.f, 0.f, 512,   5, 1.f },	// 아스파
	{ 0.f,  80.f, -300.f, -40.f,  80.f, 0.f, 512,   5, 1.f },	// 인피
	{ 0.f,  88.f, -320.f, -30.f,  88.f, 0.f, 512,   6, 1.f },	// 튜루르
	{ 0.f,  95.f, -360.f, -40.f,  95.f, 0.f, 512, 142, 1.f },	// 크라랑
	{ 0.f,  82.f, -340.f, -30.f,  82.f, 0.f, 512, 100, 1.f },	// 미피
	{ 0.f, 150.f, -600.f, -30.f, 150.f, 0.f, 512,   2, 1.f },	// 비그바그
	{ 0.f,  70.f, -280.f, -30.f,  70.f, 0.f, 512,   8, 1.f },	// 딩그밧드
};


CFcMenuObject::CFcMenuObject()
{
	m_nType = _TYPE_MENU_NONE_OBJ;
	m_hHandle = NULL;
	m_nObjIndex = -1;
	m_nEngineIndex = -1;
	m_nSkinIndex = -1;
	m_nLoadAniIndex = -1;
	
	m_bShowOn = true;	
}

CFcMenuObject::~CFcMenuObject()
{
	if(m_nEngineIndex!=-1){
		g_BsKernel.GetPhysicsMgr()->SetActiveScene( 1 );
		g_BsKernel.DeleteObject(m_nEngineIndex);
		g_BsKernel.GetPhysicsMgr()->SetActiveScene( 0 );
		m_nEngineIndex=-1;
	}
	
	SAFE_RELEASE_SKIN(m_nSkinIndex);

	SAFE_RELEASE_ANI(m_nLoadAniIndex);
	
	Release();
}

//-----------------------------------------------------------------
//-------------------------------------------------------------
CFcMenu3DObject::CFcMenu3DObject()
: CFcMenuObject()
{
	m_nType = _TYPE_MENU_3DOBJ;

	m_fFrame = 0.f;
	m_fPrevFrame = 0.f;
	m_nCurAniLength = 0;
	m_nCurrentWeaponIndex = -1;
	m_nNextAniIndex = -1;
	m_nWeaponCount = 1;

	m_Pos = D3DXVECTOR3(0, 0, 0);

	m_pUnitInfoData = NULL;
	m_pCollisionMesh = NULL;
}

CFcMenu3DObject::~CFcMenu3DObject()
{	
	int nSize = ( int )m_ArmorList.size();
	for(int i = 0; i < nSize; i++ )
	{
		if( m_ArmorList[ i ].nObjectIndex != -1 )
		{
			g_BsKernel.SendMessage( m_nEngineIndex, BS_UNLINKOBJECT, m_ArmorList[ i ].nObjectIndex );
			
			if( m_ArmorList[ i ].bUsePhysics ) g_BsKernel.GetPhysicsMgr()->SetActiveScene( 1 );			
			g_BsKernel.DeleteObject( m_ArmorList[ i ].nObjectIndex );
			if( m_ArmorList[ i ].bUsePhysics ) g_BsKernel.GetPhysicsMgr()->SetActiveScene( 0 );
			
		}
	}

	nSize = ( int )m_WeaponList.size();
	for( int i = 0; i < nSize; i++ )
	{
		if( m_WeaponList[ i ].nObjectIndex != -1 )
		{
			g_BsKernel.SendMessage( m_nEngineIndex, BS_UNLINKOBJECT, m_WeaponList[ i ].nObjectIndex );
			g_BsKernel.DeleteObject( m_WeaponList[ i ].nObjectIndex );
		}
	}
    
	nSize = ( int )m_PhysicsInfoList.size();
	for(int i = 0; i < nSize; i++ ){
		CFcPhysicsLoader::ClearPhysicsData( m_PhysicsInfoList[i] );
	}

	if( m_pCollisionMesh ) {
		CFcPhysicsLoader::ClearCollisionMesh( m_pCollisionMesh );
		m_pCollisionMesh = NULL;
	}
	//CAniInfoData::DeleteAniInfoData( m_pUnitInfoData );	
	m_pUnitInfoData = NULL;

	// m_nEngineIndex는 ~CFcMenuObject에서 처리
	DWORD dwCount = m_PartsSkinIndexList.size();
	for(DWORD i=0; i<dwCount; i++){
		SAFE_RELEASE_SKIN(m_PartsSkinIndexList[i]);
	}
	m_PartsSkinIndexList.clear();
}


bool CFcMenu3DObject::CreateObject(int nObjIndex,
								   int nAniIndex,
								   D3DXVECTOR3 vPos,
								   float fScale,
								   bool bFxLoop,
								   int nWeaponID,
								   int nWeaponCount)
{	
	BsAssert( m_nSkinIndex == -1 );
	BsAssert( m_nLoadAniIndex == -1 );
	BsAssert( m_nEngineIndex == -1 );

	m_nObjIndex = nObjIndex;

	m_pUnitSOX=CUnitSOXLoader::GetInstance().GetUnitData(nObjIndex);
	BsAssert( m_pUnitSOX->cSkinVariationNum > 0 && "Invalid unit type" );

	char szFileName[_MAX_PATH];
	sprintf(szFileName, "%s%s%d.skin", m_pUnitSOX->cSkinDir, m_pUnitSOX->cSkinFileName, 1);
	m_nSkinIndex = g_BsKernel.LoadSkin(-1, szFileName);
	if(m_nSkinIndex == -1){
		BsAssert(0);
		return false;
	}
	
	m_nLoadAniIndex = g_BsKernel.LoadAni(-1, m_pUnitSOX->cAniName);
	if(m_nLoadAniIndex == -1){
		m_nEngineIndex = g_BsKernel.CreateStaticObjectFromSkin(m_nSkinIndex);
	}
	else{
		m_nEngineIndex = g_BsKernel.CreateAniObjectFromSkin(m_nSkinIndex, m_nLoadAniIndex);
	}
    
	if(m_nEngineIndex == -1 ){
		BsAssert(0);
		return false;
	}

	// 애니메이션이 초기화가 안되서 다운되길래 임시로 넣었음 yooty
	g_BsKernel.SetCurrentAni( m_nEngineIndex, 0, 0.0f );

	sprintf(szFileName, "as\\%s", m_pUnitSOX->cUnitInfoFileName );
	m_pUnitInfoData = CAniInfoData::LoadAniInfoData( szFileName );

	m_nAniIndex = nAniIndex;
	m_nCurAniLength = g_BsKernel.GetAniLength( m_nEngineIndex, m_nAniIndex );

	m_Cross.SetPosition(vPos);
	D3DXMatrixScaling( &m_ScaleMat, fScale, fScale, fScale);

	if(g_bIsStartTitleMenu){
		CreateParts(nWeaponID, nWeaponCount);
	}

	return true;
}

bool CFcMenu3DObject::CreateObject(char* szSkinName,
								   char* szAniName,
								   char* szUnitInfoName,
								   int nAniIndex,
								   D3DXVECTOR3 vPos,
								   float fScale,
								   int nWeaponID,
								   int nWeaponCount)
{	
	BsAssert( m_nSkinIndex == -1 );
	BsAssert( m_nLoadAniIndex == -1 );
	BsAssert( m_nEngineIndex == -1 );

	m_nSkinIndex = g_BsKernel.LoadSkin(-1, szSkinName);
	if(m_nSkinIndex == -1){
		BsAssert(0);
		return false;
	}

	if(szAniName == NULL){
		m_nEngineIndex = g_BsKernel.CreateStaticObjectFromSkin(m_nSkinIndex);
	}
	else{
		m_nLoadAniIndex = g_BsKernel.LoadAni(-1, szAniName);
		if(m_nLoadAniIndex == -1){
			BsAssert(0);
			return false;
		}
		
		m_nEngineIndex = g_BsKernel.CreateAniObjectFromSkin(m_nSkinIndex, m_nLoadAniIndex);
	}

	if(m_nEngineIndex == -1 ){
		BsAssert(0);
		return false;
	}

	// 애니메이션이 초기화가 안되서 다운되길래 임시로 넣었음 yooty
	g_BsKernel.SetCurrentAni( m_nEngineIndex, 0, 0.0f );
	
	if(szUnitInfoName != NULL){
		m_pUnitInfoData = CAniInfoData::LoadAniInfoData( szUnitInfoName );
	}

	m_nAniIndex = nAniIndex;
	m_nCurAniLength = g_BsKernel.GetAniLength( m_nEngineIndex, m_nAniIndex );

	m_Cross.SetPosition(vPos);
	D3DXMatrixScaling( &m_ScaleMat, fScale, fScale, fScale);

	if(g_bIsStartTitleMenu) {
		if(szUnitInfoName != NULL){
			CreateParts(nWeaponID, nWeaponCount);
		}
	}

	return true;
}


void CFcMenu3DObject::CreateParts(int nWeaponID, int nWeaponCount)
{
	int i, nPartsGroupCount, nPartsSkinIndex, nSelectIndex, nPartsObjectIndex;
	ASPartsData *pPartsData;
	WEAPON_OBJECT Weapon;
	ARMOR_OBJECT Armor;
	int nArmorRandom[ 20 ]; // Armor 의 그룹 맥스를 20정도로 셋팅한다..
	m_nWeaponCount = nWeaponCount;

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
				if(nWeaponID == -1){
					continue;
				}

				nSelectIndex = nWeaponID;
				break;
			case PARTS_TYPE_PROJECTILE:
				nSelectIndex = 0;//RandomforMenu( pPartsData->GetPartsCount() );
				break;
			case PARTS_TYPE_ARMOR:
				if( nArmorRandom[ pPartsData->m_nGroup - 1 ] == -1 )
				{
					nArmorRandom[ pPartsData->m_nGroup - 1 ] = 0;//RandomforMenu( pPartsData->GetPartsCount() );
				}
				nSelectIndex = nArmorRandom[ pPartsData->m_nGroup - 1 ];
				break;
			default:
				continue;
		}


		enum { PHYSICS_CLOTH = 1, PHYSICS_COLLISION = 2, PHYSICS_RAGDOLL = 3 };

		D3DXMATRIX matObject = *m_Cross;
		D3DXMatrixMultiply(&matObject, &m_ScaleMat, &matObject);
		Update();

		if( pPartsData->m_nSimulation == PHYSICS_CLOTH ) {
			
			int nSimulParts = pPartsData->GetPartsCount();

			g_BsKernel.SetPreCalcAni( m_nEngineIndex, true);

			for( int j = 0; j < nSimulParts; j++ ) { //aleksger: prefix bug 777: local variable hiding scope

				char szPhysicsFileName[255];
				strcpy( szPhysicsFileName, pPartsData->GetPartsSkinName( j ) );
				RemoveEXT( szPhysicsFileName );
				strcat( szPhysicsFileName, ".txt" );

				// 떨리는 파츠 해결을 위한 임시방편 ( 미피 체인, 카라랑 펜던트)
				char szFileName[255];
				const char *ps = strrchr( szPhysicsFileName, '\\');
				if( ps ) strcpy( szFileName, ps+1 );
				else strcpy( szFileName, szPhysicsFileName );
				
				bool bChain = false;
				if( (_stricmp(szFileName, "C_LP_WM1_CHAIN.txt") == 0 ) ) {
					bChain = true;
				}

				PHYSICS_DATA_CONTAINER *pContainer = CFcPhysicsLoader::LoadPhysicsData(szPhysicsFileName);		
				m_PhysicsInfoList.push_back( pContainer );
				nPartsSkinIndex = g_BsKernel.LoadSkin( -1, pPartsData->GetPartsSkinName( j ) );
				g_BsKernel.GetPhysicsMgr()->SetActiveScene( 1 );
				if( bChain ) g_BsKernel.GetPhysicsMgr()->SetCustomValue( CBsPhysicsMgr::CUSTOM_MYIFEE_CHAIN );
				nPartsObjectIndex = g_BsKernel.CreateClothObjectFromSkin( nPartsSkinIndex , pContainer, &matObject);
				if( bChain ) g_BsKernel.GetPhysicsMgr()->SetCustomValue( CBsPhysicsMgr::CUSTOM_NONE );
				g_BsKernel.GetPhysicsMgr()->SetActiveScene( 0 );
				g_BsKernel.SendMessage( nPartsObjectIndex, BS_PHYSICS_LINK_CHARACTER, m_nEngineIndex);				
				ARMOR_OBJECT Armor;
				Armor.nObjectIndex = nPartsObjectIndex;
				Armor.nLinkBoneIndex = -1;
				Armor.bUsePhysics = true;
				m_ArmorList.push_back( Armor );
			
				if( (_stricmp(szFileName, "C_LP_PM1_PENDANT.txt") == 0 ) ) {
					m_NoSimulateList.push_back( nPartsObjectIndex );
				}
				m_PartsSkinIndexList.push_back(nPartsSkinIndex);
			}
		}
		else if( pPartsData->m_nSimulation == PHYSICS_COLLISION) {
			g_BsKernel.GetPhysicsMgr()->SetActiveScene( 1 );
			CreateCollisionMesh( pPartsData->GetPartsSkinName( 0 ) );
			g_BsKernel.GetPhysicsMgr()->SetActiveScene( 0 );
		}
		else if( pPartsData->m_nSimulation == PHYSICS_RAGDOLL) {
			//CreateRagdoll( pPartsData->GetPartsSkinName( 0 ) );
		}
		else {
			int nBoneIndex;

			nPartsSkinIndex = g_BsKernel.LoadSkin( -1, pPartsData->GetPartsSkinName( nSelectIndex ) );
			BsAssert( nPartsSkinIndex >=0 && "Load weapon fail" );

			nPartsObjectIndex = g_BsKernel.CreateStaticObjectFromSkin( nPartsSkinIndex );
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
				if( pPartsData->m_nGroup <= nWeaponCount ){
					Weapon.bShow = true;
				}else{
					Weapon.bShow = false;
				}
				Weapon.nEngineIndex = nPartsSkinIndex;
				Weapon.nLinkBoneIndex = nBoneIndex;
				Weapon.nPartsIndex = i;
				m_WeaponList.push_back( Weapon );
			}
			m_nCurrentWeaponIndex = 0;

			m_PartsSkinIndexList.push_back(nPartsSkinIndex);
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

void CFcMenu3DObject::CreateCollisionMesh(const char *pFileName)
{
	BsAssert( m_pCollisionMesh == NULL);
	m_pCollisionMesh = CFcPhysicsLoader::LoadCollisionMesh( pFileName );
	g_BsKernel.SendMessage( m_nEngineIndex, BS_PHYSICS_COLLISIONMESH, (DWORD) m_pCollisionMesh);
}

void CFcMenu3DObject::ChangeWeapon(int nSkinIndex)
{
	if(nSkinIndex == -1){
		return;
	}

	BsAssert( nSkinIndex >= 0 );
	
	int nPartsGroupCount = m_pUnitInfoData->GetPartsGroupCount();
	for(int i = 0; i < nPartsGroupCount; i++ )
	{
		ASPartsData *pPartsData = m_pUnitInfoData->GetPartsGroupInfo( i );

		if(pPartsData->m_nType - 1  == PARTS_TYPE_WEAPON)
		{
			int nBoneIndex = g_BsKernel.SendMessage( m_nEngineIndex, BS_GET_BONE_INDEX, (DWORD)pPartsData->GetPartsBoneName() );
			g_BsKernel.SendMessage( m_nEngineIndex, BS_UNLINKOBJECT_BONE, (DWORD)nBoneIndex );

			int nObjectIndex = g_BsKernel.CreateStaticObjectFromSkin( nSkinIndex );
			CCrossVector cv;
			cv.MoveFrontBack(1000000000.f);
			g_BsKernel.UpdateObject(nObjectIndex,cv);
			g_BsKernel.SendMessage( m_nEngineIndex, BS_LINKOBJECT, (DWORD)nBoneIndex, nObjectIndex );
			g_BsKernel.AddSkinRef(nSkinIndex);
			m_PartsSkinIndexList.push_back(nSkinIndex);

			WEAPON_OBJECT Weapon;
			Weapon.nGroup = pPartsData->m_nGroup - 1;
			Weapon.nObjectIndex = nObjectIndex;
			if( pPartsData->m_nGroup <= m_nWeaponCount ){
				Weapon.bShow = true;
			}else{
				Weapon.bShow = false;
			}
			
			Weapon.nEngineIndex = nSkinIndex;
			Weapon.nLinkBoneIndex = nBoneIndex;
			Weapon.nPartsIndex = PARTS_TYPE_WEAPON;
			m_WeaponList.push_back(Weapon);
		}
	}

	int nSize = ( int )m_WeaponList.size();
	for(int i = 0; i < nSize; i++ )
	{
#pragma warning(disable:4800)
		g_BsKernel.ShowObject( m_WeaponList[ i ].nObjectIndex, ( bool )( m_WeaponList[ i ].bShow ) );
#pragma warning(default:4800)
	}
}

void CFcMenu3DObject::Process()
{
	if(m_nEngineIndex == -1){
		return;
	}
    
	//char select menu에서 next ani가 없는 관계로 부득이 하게 아래와 같이 처리.
	if(m_nNextAniIndex != -2)
	{
		if(m_nCurAniLength -1 == (int)m_fFrame) {
			if(m_nNextAniIndex != -1)
			{
				/*
				m_nAniIndex = m_nNextAniIndex;
				m_nCurAniLength = g_BsKernel.GetAniLength( m_nEngineIndex, m_nAniIndex );
				m_fPrevFrame = m_fFrame = 0.f;
				m_nNextAniIndex = -1;
				*/
				m_nNextAniIndex = -2;
			}
			else {
				m_fPrevFrame = m_fFrame;
				m_fFrame = 0.f;
			}
		}
		else {
			m_fPrevFrame = m_fFrame;
			m_fFrame += 1.f;
		}
	}
	else {
		m_fPrevFrame = m_fFrame;
	}


/*
	//위의 문제로 인하여 정상코드는 주석처리한다.
	if(m_nCurAniLength -1 == (int)m_fFrame)
	{
		if(m_nNextAniIndex != -1)
		{
			m_nAniIndex = m_nNextAniIndex;
			m_nCurAniLength = g_BsKernel.GetAniLength( m_nEngineIndex, m_nAniIndex );
			m_fPrevFrame = m_fFrame = 0.f;
			m_nNextAniIndex = -1;
		}
		else
		{
			m_fPrevFrame = m_fFrame;
			m_fFrame = 0.f;
		}
	}
	else{
		m_fPrevFrame = m_fFrame;
		m_fFrame += 1.f;
	}
*/

	GET_ANIDISTANCE_INFO Info;
	Info.fFrame1 = m_fFrame;
	Info.fFrame2 = m_fPrevFrame;
	Info.nAni1 = m_nAniIndex;
	Info.nAni2 = m_nAniIndex;
	Info.pVector = &m_Pos;
	g_BsKernel.SendMessage( m_nEngineIndex, BS_GETANIDISTANCE, ( DWORD )&Info );

	D3DXVECTOR3 prevPos = m_Cross.GetPosition();
	m_Pos.y = 0.f;
	m_Cross.MoveFrontBack(m_Pos.z);
	m_Cross.MoveRightLeft(m_Pos.x);

}

void CFcMenu3DObject::Update()
{	
	if(m_bShowOn == false){	
		return;
	}

	if(m_nEngineIndex == -1){
		return;
	}

	g_BsKernel.SetCurrentAni(m_nEngineIndex, m_nAniIndex, m_fFrame );
	
	D3DXMATRIX matObject;
	D3DXMatrixMultiply(&matObject, &m_ScaleMat, m_Cross);

	g_BsKernel.UpdateObject( m_nEngineIndex, &matObject );	

	int i, nSize;

	nSize = ( int )m_ArmorList.size();

	int nTick = GetProcessTick();
	for( i = 0; i < nSize; i++ )
	{
		int nObjIndex = m_ArmorList[ i ].nObjectIndex;
		if( nObjIndex != -1 )
		{			
			if( m_ArmorList[ i ].bUsePhysics ) {
				float fForce = fmodf( nTick / (float)m_nCurAniLength, 1.0f);
				fForce = sinf( fForce * D3DX_PI );				
				D3DXVECTOR3 Force( -fForce * 0.2f, fForce * 0.2f, -fForce * 0.3f );
				D3DXVec3TransformNormal( &Force, &Force, m_Cross);			

				if( std::find( m_NoSimulateList.begin(), m_NoSimulateList.end(), nObjIndex ) != m_NoSimulateList.end() ) {
					((CBsClothObject*)g_BsKernel.GetEngineObjectPtr( nObjIndex ))->SetMoveCorrectionCount( 5 );
				}
				else {
					g_BsKernel.SendMessage( nObjIndex, BS_PHYSICS_ADDFORCE, (DWORD)&Force) ;
				}
			}
		}
	}
}

void CFcMenu3DObject::SetAniIndex(int nIndex)
{
	m_nAniIndex = nIndex;
	m_nCurAniLength = g_BsKernel.GetAniLength( m_nEngineIndex, m_nAniIndex );
	m_fPrevFrame = m_fFrame = 0.f;
}

void CFcMenu3DObject::SetNextAniIndex(int nIndex)
{
	m_nNextAniIndex = nIndex;
}

//-----------------------------------------------------------------
//-----------------------------------------------------------------
CFcMenuFxObject::CFcMenuFxObject()
{
	m_nType = _TYPE_MENU_FXOBJ;

	m_nEngineIndex = -1;
	m_nObjIndex = -1;

	m_hHandle = NULL;

	m_bShowOn = true;
}

bool CFcMenuFxObject::CreateObject(int nObjIndex, int nAniIndex, D3DXVECTOR3 vPos, float fScale, bool bFxLoop)
{
	BsAssert( m_nEngineIndex == -1 );

	m_nObjIndex = nObjIndex;

	m_nEngineIndex = g_BsKernel.CreateFXObject(nObjIndex);
	if(m_nEngineIndex == -1){
		BsAssert(0);
		return false;
	}

	m_Cross.SetPosition(vPos);

	int nFlag = 0;
	if(bFxLoop == true){
		nFlag = -1;
	}

	g_BsKernel.SetFXObjectState(m_nEngineIndex, CBsFXObject::PLAY, nFlag);

	return true;
}

void CFcMenuFxObject::Update()
{
	if(m_bShowOn == false){
		return;
	}

	if(m_nEngineIndex == -1){
		return;
	}

	g_BsKernel.UpdateObject(m_nEngineIndex, m_Cross);
}


//-----------------------------------------------------------------
//-----------------------------------------------------------------
CFcMenu3DObjManager::CFcMenu3DObjManager()
	: CBsGenerateTexture()
{	
	m_nLightIndex = -1;
	SetMenuCameraFov(0.74f);
}

CFcMenu3DObjManager::~CFcMenu3DObjManager()
{
	Release();
}

void CFcMenu3DObjManager::Create(int nTextureWidth, int nTextureHeight,
								 float fStartX, float fStartY,
								 float fWidth, float fHeight, bool bAlpha)
{	
	CBsGenerateTexture::Create(nTextureWidth, nTextureHeight, fStartX, fStartY, fWidth, fHeight, bAlpha);

	CreateCamera();
}

void CFcMenu3DObjManager::CreateCamera()
{
	g_BsKernel.SetProjectionMatrix( m_nCameraIndex, 100.f, 5000.0f );

	m_CameraCross.SetPosition(_CAMERA_POS);
	m_vCamTargetPos = _CAMERA_TARGET_POS;
	m_CameraCross.LookAt(&m_vCamTargetPos);

	D3DXVECTOR4 vecFogColor = D3DXVECTOR4(1.f,1.f,1.f,1.f);
	g_BsKernel.SetFogColor( m_nCameraIndex, &vecFogColor );
	g_BsKernel.SetFogFactor(m_nCameraIndex, 5000.f, 10000.f);

	D3DXMatrixScaling( &m_ScaleMat, 1.f, 1.f, 1.f);

}

void CFcMenu3DObjManager::CreateLight()
{
	m_LightCross.SetPosition(_LIGHT_POS);
	m_vLightTargetPos = _LIGHT_TARGET_POS;
	m_CameraCross.LookAt(&m_vLightTargetPos);

	D3DCOLORVALUE clrLightAmbient = {1.f,1.f,1.f,1.f};
	D3DCOLORVALUE clrLightDiffuse = {0.7f, 0.7f, 0.7f,1.f};
	D3DCOLORVALUE clrLightSpecular = {1.f,1.f,1.f,1.f};
	float fBlur = 0.f;
	float fGlow = 0.f;	

	m_nLightIndex = g_BsKernel.CreateLightObject( DIRECTIONAL_LIGHT );
	g_BsKernel.UpdateObject( m_nLightIndex, m_LightCross );

	g_BsKernel.SendMessage( m_nLightIndex, BS_SET_LIGHT_AMBIENT, (DWORD)&clrLightAmbient );
	g_BsKernel.SendMessage( m_nLightIndex, BS_SET_LIGHT_DIFFUSE, (DWORD)&clrLightDiffuse );
	g_BsKernel.SendMessage( m_nLightIndex, BS_SET_LIGHT_SPECULAR, (DWORD)&clrLightSpecular );

	g_BsKernel.SetBlurIntensity( fBlur );
	g_BsKernel.SetSceneIntensity( 1.0f - fBlur );
	g_BsKernel.SetGlowIntensity( fGlow );
}


void CFcMenu3DObjManager::Release()
{
	if( m_nLightIndex != -1 )
	{
		g_BsKernel.DeleteObject( m_nLightIndex );
		m_nLightIndex = -1;
	}

	int nCnt = m_FxTemplateList.size();
	for( int i=0; i<nCnt; i++ )
	{
		g_BsKernel.ReleaseFXTemplate( m_FxTemplateList[i] );
	}

	m_FxTemplateList.clear();

	RemoveAllObject();
}

void CFcMenu3DObjManager::Process()
{
	if(m_nCameraIndex != -1){
		m_CameraCross.LookAt(&m_vCamTargetPos);

		g_BsKernel.SetCameraFov(m_nCameraIndex, GetMenuCameraFov());
        
		D3DXMATRIX RenderMat;
		D3DXMatrixMultiply( &RenderMat, &m_ScaleMat, m_CameraCross );
		g_BsKernel.UpdateObject( m_nCameraIndex, &RenderMat );
	}

	if(m_nLightIndex != -1){
		m_LightCross.LookAt(&m_vLightTargetPos);
		g_BsKernel.UpdateObject( m_nLightIndex, m_LightCross );
	}
	
	FcMenuObjList::iterator itr = m_ObjList.begin();
	while(1)
	{	
		if(itr == m_ObjList.end() )
			break;

		if((*itr)->GetType() == _TYPE_MENU_FXOBJ)
		{
			int nEngineIndex = (*itr)->GetEngineIndex();
			if(g_BsKernel.GetFXObjectState(nEngineIndex) == CBsFXObject::STOP)
			{
				CBsRTTCamera* pRTTCamera = (CBsRTTCamera*)g_BsKernel.GetEngineObjectPtr(m_nCameraIndex);
				pRTTCamera->EliminateRegisteredObject(nEngineIndex);				

				CBsFXObject *pObject = (CBsFXObject *)g_BsKernel.GetEngineObjectPtr( nEngineIndex );
				for( DWORD i=0; i<pObject->GetElementCount(); i++ ) {
					if( pObject->GetElementFromIndex(i)->GetType() == CBsFXElement::MESH ) {
						pRTTCamera->EliminateRegisteredObject( pObject->GetElementFromIndex(i)->GetObjectIndex() );
					}
				}

				delete (*itr);
				itr = m_ObjList.erase(itr);				
				continue;
			}
		}

		(*itr)->Process();
		(*itr)->Update();

		itr++;
	}

	int a = 1;
}

void CFcMenu3DObjManager::InitRenderRTT()
{
	CBsRTTCamera* pRTTCamera = (CBsRTTCamera*)g_BsKernel.GetEngineObjectPtr(m_nCameraIndex);
	g_BsKernel.InitRenderRTTFrame(pRTTCamera);
}

void CFcMenu3DObjManager::Render(C3DDevice *pDevice)
{
	int nLightIndex = -1;
	D3DCOLORVALUE saveAmbient, saveDiffuse, saveSpecular;
	if(m_nLightIndex == -1)
	{
		//-------------------------------------------------------------------------------
		//save
		nLightIndex = g_BsKernel.GetDirectionLightIndex();
		saveAmbient = *(D3DCOLORVALUE*)g_BsKernel.SendMessage( nLightIndex, BS_GET_LIGHT_AMBIENT);
		saveDiffuse = *(D3DCOLORVALUE*)g_BsKernel.SendMessage( nLightIndex, BS_GET_LIGHT_DIFFUSE);
		saveSpecular= *(D3DCOLORVALUE*)g_BsKernel.SendMessage( nLightIndex, BS_GET_LIGHT_SPECULAR);

		//-------------------------------------------------------------------------------
		//set
		D3DCOLORVALUE clrLightAmbient = {1.f,1.f,1.f,1.f};
		D3DCOLORVALUE clrLightDiffuse = {1.f,1.f,1.f,1.f};
		D3DCOLORVALUE clrLightSpecular = {1.f,1.f,1.f,1.f};
		g_BsKernel.SendMessage( nLightIndex, BS_SET_LIGHT_AMBIENT, (DWORD)&clrLightAmbient );
		g_BsKernel.SendMessage( nLightIndex, BS_SET_LIGHT_DIFFUSE, (DWORD)&clrLightDiffuse );
		g_BsKernel.SendMessage( nLightIndex, BS_SET_LIGHT_SPECULAR, (DWORD)&clrLightSpecular );
	}
	CBsObject* pLightObjet = g_BsKernel.GetEngineObjectPtr(g_BsKernel.GetDirectionLightIndex());
	CBsObject* pCameraObject = g_BsKernel.GetEngineObjectPtr(GetCameraIndex());
	BsAssert( pLightObjet && "Invalid Light Object!!" );
	D3DXMATRIX matSaved = *(pLightObjet->GetObjectMatrix());

	// Light Set!
	D3DXVECTOR3 vecLightDir;
	D3DXVec3Normalize(&vecLightDir, &D3DXVECTOR3(0.3f, -0.4f, 0.3f));
	D3DXVec3TransformNormal(&vecLightDir, &vecLightDir, pCameraObject->GetObjectMatrix());
	CCrossVector cvTemp;
	cvTemp.m_ZVector = vecLightDir;

	pLightObjet->SetObjectMatrixByRender(cvTemp);	

	g_BsKernel.LockActiveCamera(GetCameraIndex());

	/////////////////////////////////////////////////////////////
	// Camera Lock하는 곳으로 옮겨야합니다. by jeremy
	pDevice->SetViewport(g_BsKernel.GetActiveCamera()->GetCameraViewport());
	/////////////////////////////////////////////////////////////

	pDevice->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, TRUE);
	pDevice->SetRenderState(D3DRS_SRCBLENDALPHA, D3DBLEND_ONE);
	pDevice->SetRenderState(D3DRS_DESTBLENDALPHA, D3DBLEND_ONE);
	pDevice->SetRenderState(D3DRS_BLENDOPALPHA, D3DBLENDOP_MAX);

	pDevice->ClearBuffer(0x00000000);
    
	CBsRTTCamera* pRTTCamera = (CBsRTTCamera*)g_BsKernel.GetEngineObjectPtr(m_nCameraIndex);
	g_BsKernel.RenderRTTFrame(pRTTCamera, pDevice);

	pDevice->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, FALSE);
	pDevice->SetRenderState(D3DRS_SRCBLENDALPHA, D3DBLEND_ONE);
	pDevice->SetRenderState(D3DRS_DESTBLENDALPHA, D3DBLEND_ZERO);
	pDevice->SetRenderState(D3DRS_BLENDOPALPHA, D3DBLENDOP_ADD);

	g_BsKernel.UnlockActiveCamera();

	// Light Restore!
	pLightObjet->SetObjectMatrixByRender(&matSaved);
	if(m_nLightIndex == -1)
	{
		// Light Restore!
		//------------------------------------------------------------------------------
		//restore
		g_BsKernel.SendMessage( nLightIndex, BS_SET_LIGHT_AMBIENT, (DWORD)&saveAmbient );
		g_BsKernel.SendMessage( nLightIndex, BS_SET_LIGHT_DIFFUSE, (DWORD)&saveDiffuse );
		g_BsKernel.SendMessage( nLightIndex, BS_SET_LIGHT_SPECULAR, (DWORD)&saveSpecular );
	}
}

int CFcMenu3DObjManager::LoadFXTemplate(char* szFxFileName)
{
	g_BsKernel.chdir("Fx");
	char szFullFileName[_MAX_PATH];
	sprintf(szFullFileName, "%s%s", g_BsKernel.GetCurrentDirectory(), szFxFileName );
	int nFxTemplateId = g_BsKernel.LoadFXTemplate(-1, szFullFileName);
	g_BsKernel.chdir("..");

	if(nFxTemplateId != -1){
		m_FxTemplateList.push_back(nFxTemplateId);
	}

	return nFxTemplateId;
}

DWORD CFcMenu3DObjManager::AddObject(int nObjIndex,
									 int nAniIndex,
									 D3DXVECTOR3 vPos,
									 float fScale,
									 int nWeaponID,
									 int nWeaponCount)
{
	CFcMenu3DObject* pMenuObj = new CFcMenu3DObject();
	bool bCreate = pMenuObj->CreateObject(nObjIndex, nAniIndex, vPos, fScale, false, nWeaponID, nWeaponCount);
	if( bCreate == false)
	{
		delete pMenuObj;
		return NULL;
	}

	CBsRTTCamera* pRTTCamera = (CBsRTTCamera*)(g_BsKernel.GetEngineObjectPtr(m_nCameraIndex));
	pRTTCamera->RegisterObject(pMenuObj->GetEngineIndex());
	
	m_ObjList.push_back(pMenuObj);
	DWORD hHandle = (DWORD)m_ObjList.back();
	pMenuObj->SetHandle(hHandle);

	return hHandle;

	
}

DWORD CFcMenu3DObjManager::AddFXObject(int nObjIndex, D3DXVECTOR3 vPos, float fScale, bool bLoop)
{
	CFcMenuFxObject* pMenuObj = new CFcMenuFxObject();
	if(pMenuObj->CreateObject(nObjIndex, -1, vPos, fScale, bLoop) == false)
	{
		delete pMenuObj;
		return NULL;
	}

	int nEngineObjectIndex = pMenuObj->GetEngineIndex();

	CBsRTTCamera* pRTTCamera = (CBsRTTCamera*)(g_BsKernel.GetEngineObjectPtr(m_nCameraIndex));
	pRTTCamera->RegisterObject(nEngineObjectIndex);

	CBsFXObject *pObject = (CBsFXObject *)g_BsKernel.GetEngineObjectPtr( nEngineObjectIndex );
	for( DWORD i=0; i<pObject->GetElementCount(); i++ )
	{
		if( pObject->GetElementFromIndex(i)->GetType() == CBsFXElement::MESH ) {
			pRTTCamera->RegisterObject( pObject->GetElementFromIndex(i)->GetObjectIndex() );
		}
	}
	
	m_ObjList.push_back(pMenuObj);
	DWORD hHandle = (DWORD)m_ObjList.back();
	pMenuObj->SetHandle(hHandle);

	return hHandle;
}


DWORD CFcMenu3DObjManager::AddObject(char* szSkinName,
									 char* szAniName,
									 char* szUnitInfoName,
									 int nAniIndex,
									 D3DXVECTOR3 vPos,
									 float fScale,
									 int nWeaponID,
									 int nWeaponCount)
{
	CFcMenu3DObject* pMenu3DObj = new CFcMenu3DObject();
	bool bCreate = pMenu3DObj->CreateObject(szSkinName, szAniName, szUnitInfoName, nAniIndex, vPos, fScale, nWeaponID, nWeaponCount);
	if(bCreate == false)
	{
		delete pMenu3DObj;
		return NULL;
	}

	CBsRTTCamera* pRTTCamera = (CBsRTTCamera*)(g_BsKernel.GetEngineObjectPtr(m_nCameraIndex));
	pRTTCamera->RegisterObject(pMenu3DObj->GetEngineIndex());
	
	m_ObjList.push_back((CFcMenuObject*)pMenu3DObj);
	DWORD hHandle = (DWORD)m_ObjList.back();
	pMenu3DObj->SetHandle(hHandle);

	return hHandle;
}

bool CFcMenu3DObjManager::ChangeWeapon(DWORD hHandle, int nSkinID)
{
	DWORD dwCount = m_ObjList.size();
	for(DWORD i=0; i<dwCount; i++)
	{
		CFcMenuObject* pMenuObj = m_ObjList[i];
		if(pMenuObj->GetHandle() == hHandle)
		{
			((CFcMenu3DObject*)pMenuObj)->ChangeWeapon(nSkinID);
			return true;
		}
	}

	return false;
}


bool CFcMenu3DObjManager::RemoveObject(DWORD hHandle)
{
	DWORD dwCount = m_ObjList.size();
	for(DWORD i=0; i<dwCount; i++)
	{
		CFcMenuObject* pMenuObj = m_ObjList[i];
		if(pMenuObj->GetHandle() == hHandle)
		{
			delete pMenuObj;
			m_ObjList.erase(m_ObjList.begin()+i);
			return true;
		}
	}

	return false;
}

void CFcMenu3DObjManager::RemoveAllObject()
{
	CBsRTTCamera* pRTTCamera = (CBsRTTCamera*)(g_BsKernel.GetEngineObjectPtr(m_nCameraIndex));
	
	DWORD dwCount = m_ObjList.size();
	for(DWORD i=0; i<dwCount; i++)
	{
		CFcMenuObject* pMenuObj = m_ObjList[i];
		pRTTCamera->EliminateRegisteredObject(pMenuObj->GetEngineIndex());

		delete m_ObjList[i];
	}
	m_ObjList.clear();
}

void CFcMenu3DObjManager::SetShowonObject(DWORD hHandle, bool bShow)
{
	CFcMenuObject* pMenuObj = GetObject(hHandle);
	if(pMenuObj == NULL){
		return;
	}

	pMenuObj->SetShowOn(bShow);
}

bool CFcMenu3DObjManager::GetShowonObject(DWORD hHandle)
{
	CFcMenuObject* pMenuObj = GetObject(hHandle);
	if(pMenuObj == NULL){
		return false;
	}

	return pMenuObj->GetShowOn();
}

void CFcMenu3DObjManager::SetObjectScale(DWORD hHandle, float fScale)
{
	DWORD dwCount = m_ObjList.size();
	for(DWORD i=0; i<dwCount; i++)
	{
		CFcMenuObject* pMenuObj = m_ObjList[i];
		if(pMenuObj->GetHandle() == hHandle){
			((CFcMenu3DObject*)pMenuObj)->SetScale(fScale);
		}
	}
}

CCrossVector* CFcMenu3DObjManager::GetObjCrossVector(DWORD hHandle)
{
	CFcMenuObject* pMenuObj = GetObject(hHandle);
	if(pMenuObj == NULL){
		return NULL;
	}

	return pMenuObj->GetCrossVector();
}

CFcMenuObject* CFcMenu3DObjManager::GetObject(DWORD hHandle)
{
	DWORD dwCount = m_ObjList.size();
	for(DWORD i=0; i<dwCount; i++)
	{
		CFcMenuObject* pMenuObj = m_ObjList[i];
		if(pMenuObj->GetHandle() == hHandle){
			return pMenuObj;
		}
	}

	return NULL;
}

void CFcMenu3DObjManager::SetAniIndex(int nObjIndex, int nAniIndex)
{
	int nCount = m_ObjList.size();
	BsAssert( nObjIndex >=0 && nObjIndex < nCount );
/*
	for(DWORD i=0; i<dwCount; i++)
	{
		CFcMenuObject* pMenuObj = m_ObjList[i];
		if(pMenuObj->GetType() == _TYPE_MENU_3DOBJ &&
			pMenuObj->GetHandle() == hHandle)
		{
			((CFcMenu3DObject*)pMenuObj)->SetAniIndex(nAniIndex);
			break;
		}
	}
*/
	CFcMenuObject* pMenuObj = m_ObjList[nObjIndex];
	if( pMenuObj->GetType() == _TYPE_MENU_3DOBJ )
		((CFcMenu3DObject*)pMenuObj)->SetAniIndex(nAniIndex);
}

void CFcMenu3DObjManager::SetNextAniIndex(int nObjIndex, int nAniIndex)
{
/*
	DWORD dwCount = m_ObjList.size();
	for(DWORD i=0; i<dwCount; i++)
	{
		CFcMenuObject* pMenuObj = m_ObjList[i];
		if(pMenuObj->GetType() == _TYPE_MENU_3DOBJ &&
			pMenuObj->GetHandle() == hHandle)
		{
			((CFcMenu3DObject*)pMenuObj)->SetNextAniIndex(nAniIndex);
			break;
		}
	}
*/
 	int nCount = m_ObjList.size();
	BsAssert( nObjIndex >=0 && nObjIndex < nCount );

	CFcMenuObject* pMenuObj = m_ObjList[nObjIndex];
	if( pMenuObj->GetType() == _TYPE_MENU_3DOBJ )
		((CFcMenu3DObject*)pMenuObj)->SetNextAniIndex(nAniIndex);
}