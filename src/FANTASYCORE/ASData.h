#pragma once

#include "data/AnimType.h"

#define m_HitLength				m_pParam[0]
#define m_HitAttackPoint		m_pParam[1]
#define m_HitAniType			m_pParam[2]
#define m_HitAniIndex			m_pParam[3]
#define m_HitPushY				m_pParam[4]
#define m_HitPushZ				m_pParam[5]
#define m_HitParticle			m_pParam[6]
#define m_HitParticleX			m_pParam[7]
#define m_HitParticleY			m_pParam[8]
#define m_HitParticleZ			m_pParam[9]
#define m_HitMatrixIndex		m_pParam[10]
#define m_HitAddLength			m_pParam[11]
#define m_HitCatch				m_pParam[12]
#define m_HitGravity			m_pParam[13]
#define m_HitGravityFrame		m_pParam[14]
#define m_HitGroundResist		m_pParam[15]
#define m_HitGuardBreakProb		m_pParam[16]
#define m_HitStunProb			m_pParam[17]
#define m_HitStunDelay			m_pParam[18]


#define m_Hit2Length			m_pParam[0]
#define m_Hit2AttackPoint		m_pParam[1]
#define m_Hit2AniType			m_pParam[2]
#define m_Hit2AniIndex			m_pParam[3]
#define m_Hit2PushY				m_pParam[4]
#define m_Hit2PushZ				m_pParam[5]
#define m_Hit2Particle			m_pParam[6]
#define m_Hit2ParticleX			m_pParam[7]
#define m_Hit2ParticleY			m_pParam[8]
#define m_Hit2ParticleZ			m_pParam[9]
#define m_Hit2MinDist			m_pParam[10]
#define m_Hit2MaxDist			m_pParam[11]
#define m_Hit2MinHeight			m_pParam[12]
#define m_Hit2MaxHeight			m_pParam[13]
#define m_Hit2CenterAngle		m_pParam[14]
#define m_Hit2Angle				m_pParam[15]
#define m_Hit2Catch				m_pParam[16]
#define m_Hit2Gravity			m_pParam[17]
#define m_Hit2GravityFrame		m_pParam[18]
#define m_Hit2GroundResist		m_pParam[19]
#define m_Hit2SoundAction		m_pParam[20]
#define m_Hit2WeaponMaterial	m_pParam[21]
#define m_Hit2SoundDelay		m_pParam[22]
#define m_Hit2GuardBreakProb	m_pParam[23]
#define m_Hit2StunProb			m_pParam[24]
#define m_Hit2StunDelay			m_pParam[25]
#define m_Hit2PropBreakType		m_pParam[26]
#define m_Hit2AddCritiacl		m_pParam[27]
#define m_Hit2Fx				m_pParam[28]
#define HIT2_SIGNAL_LENGTH		29

#define m_PushLength			m_pParam[0]
#define m_PushAniType			m_pParam[1]
#define m_PushAniIndex			m_pParam[2]
#define m_PushPushY				m_pParam[3]
#define m_PushPushZ				m_pParam[4]
#define m_PushParticle			m_pParam[5]
#define m_PushParticleX			m_pParam[6]
#define m_PushParticleY			m_pParam[7]
#define m_PushParticleZ			m_pParam[8]
#define m_PushMinDist			m_pParam[9]
#define m_PushMaxDist			m_pParam[10]
#define m_PushMinHeight			m_pParam[11]
#define m_PushMaxHeight			m_pParam[12]
#define m_PushCenterAngle		m_pParam[13]
#define m_PushAngle				m_pParam[14]
#define m_PushCatch				m_pParam[15]
#define m_PushGravity			m_pParam[16]
#define m_PushGravityFrame		m_pParam[17]
#define m_PushGroundResist		m_pParam[18]
#define m_PushGuardBreakProb	m_pParam[19]
#define m_PushStunProb			m_pParam[20]
#define m_PushStunDelay			m_pParam[21]


#define m_ParticleID			m_pParam[0]
#define m_ParticleX				m_pParam[1]
#define m_ParticleY				m_pParam[2]
#define m_ParticleZ				m_pParam[3]
#define m_ParticleIterate		m_pParam[4]
#define m_ParticleScale			m_pParam[5]
#define m_ParticleCount			m_pParam[6]
#define m_ParticleLoop			m_pParam[7]

#define m_LinkedParticleBoneIndex	m_pParam[0]
#define m_LinkedParticleID			m_pParam[1]
#define m_LinkedParticleX			m_pParam[2]
#define m_LinkedParticleY			m_pParam[3]
#define m_LinkedParticleZ			m_pParam[4]
#define m_LinkedParticleScale		m_pParam[5]
#define m_LinkedParticleIterate		m_pParam[6]
#define m_LinkedParticleLoop		m_pParam[7]

#define m_LinkedToCrossParticleID			m_pParam[0]
#define m_LinkedToCrossParticleX			m_pParam[1]
#define m_LinkedToCrossParticleY			m_pParam[2]
#define m_LinkedToCrossParticleZ			m_pParam[3]
#define m_LinkedToCrossParticleScale		m_pParam[4]
#define m_LinkedToCrossParticleIterate		m_pParam[5]
#define m_LinkedToCrossParticleLoop			m_pParam[6]

#define	m_FXID						m_pParam[0]
#define m_FXX						m_pParam[1]
#define m_FXY						m_pParam[2]
#define m_FXZ						m_pParam[3]
#define m_FXIterate					m_pParam[4]
#define m_FXOnlyTargetHeroShow		m_pParam[5]
#define m_FXRX						m_pParam[6]
#define m_FXRY						m_pParam[7]
#define m_FXRZ						m_pParam[8]
#define m_FXRandomRX				m_pParam[9]
#define m_FXRandomRY				m_pParam[10]
#define m_FXRandomRZ				m_pParam[11]



#define m_JumpGravity			m_pParam[0]
#define m_JumpSpeed				m_pParam[1]
#define m_JumpEnableGravity		m_pParam[2]
#define m_JumpEnableSpeed		m_pParam[3]

#define m_InputLength			m_pParam[ 0 ]
#define m_InputKey				m_pParam[ 1 ]
#define m_InputAniType			m_pParam[ 2 ]
#define m_InputAniIndex			m_pParam[ 3 ]
#define m_InputDirectIndex		m_pParam[ 4 ]
#define m_InputPressFrame		m_pParam[ 5 ]
#define m_InputClear			m_pParam[ 6 ]
#define m_InputLevel			m_pParam[ 7 ]
#define m_NeedOrb				m_pParam[ 8 ]

#define m_GotoFrame				m_pParam[ 0 ]
#define m_GotoKey				m_pParam[ 1 ]
#define m_GotoKeyCount			m_pParam[ 2 ]
#define m_GotoAddKeyCount		m_pParam[ 3 ]
#define m_GotoDirection			m_pParam[ 4 ]
#define m_GotoMaxLoop			m_pParam[ 5 ]
#define m_GotoLoopLevel			m_pParam[ 6 ]

#define m_TrailLength			m_pParam[ 0 ]
#define m_TrailR				m_pParam[ 1 ]
#define m_TrailG				m_pParam[ 2 ]
#define m_TrailB				m_pParam[ 3 ]
#define m_TrailLifeTime			m_pParam[ 4 ]
#define m_TrailMatrixIndex		m_pParam[ 5 ]
#define m_TrailAddLength		m_pParam[ 6 ]
#define m_TrailKind				m_pParam[ 7 ]
#define m_TrailWeaponBone		m_pParam[ 8 ]
#define m_TrailWeaponGroup		m_pParam[ 9 ]

#define m_GotoPressCheckLength				m_pParam[ 0 ]
#define m_GotoPressCheckFrame				m_pParam[ 1 ]
#define m_GotoPressCheckKey					m_pParam[ 2 ]
#define m_GotoPressCheck					m_pParam[ 3 ]
#define m_GotoLandCheck						m_pParam[ 4 ]
#define m_GotoMoveZ							m_pParam[ 5 ]
#define m_GotoRequireLevel					m_pParam[ 6 ]

#define m_PhysicsDistance					m_pParam[ 0 ]
#define m_PhysicsAngle						m_pParam[ 1 ]
#define m_PhysicsPower						m_pParam[ 2 ]
#define m_PhysicsType						m_pParam[ 3 ]
#define m_PhysicsParticle					m_pParam[ 4 ]
#define m_PhysicsParticleY					m_pParam[ 5 ]

#define m_ThrowWeaponLength					m_pParam[ 0 ]
#define m_ThrowWeaponSpeed					m_pParam[ 1 ]
#define m_ThrowWeaponStartBone				m_pParam[ 2 ]
#define m_ThrowWeaponEndBone				m_pParam[ 3 ]
#define m_ThrowWeaponMatrixIndex			m_pParam[ 4 ]
#define m_ThrowWeaponThrowLength			m_pParam[ 5 ]
#define m_ThrowWeaponBending				m_pParam[ 6 ]
#define m_ThrowWeaponBendPos				m_pParam[ 7 ]

#define m_LinkedFXBoneIndex					m_pParam[ 0 ]
#define m_LinkedFXID						m_pParam[ 1 ]
#define m_LinkedFXX							m_pParam[ 2 ]
#define m_LinkedFXY							m_pParam[ 3 ]
#define m_LinkedFXZ							m_pParam[ 4 ]
#define m_LinkedFXRX						m_pParam[ 5 ]
#define m_LinkedFXRY						m_pParam[ 6 ]
#define m_LinkedFXRZ						m_pParam[ 7 ]
#define m_LinkedFXScale						m_pParam[ 8 ]
#define m_LinkedFXIterate					m_pParam[ 9 ]
#define m_LinkedFXLoop						m_pParam[ 10 ]
#define m_LinkedFXOnlyTargetHeroShow		m_pParam[ 11 ]
#define m_LinkedFXRandomRX					m_pParam[ 12 ]
#define m_LinkedFXRandomRY					m_pParam[ 13 ]
#define m_LinkedFXRandomRZ					m_pParam[ 14 ]

#define m_AIControlCheckIndex				m_pParam[0]
#define m_AIControlCheckAniAttr				m_pParam[1]
#define m_AIControlCheckAniType				m_pParam[2]
#define m_AIControlCheckAniTypeIndex		m_pParam[3]
#define m_AIControlCheckAniFrame			m_pParam[4]
#define m_AIControlCheckBlendFrame			m_pParam[5]
#define m_AIControlCheckReleaseIndex		m_pParam[6]
#define m_AIControlCheckLockonTarget		m_pParam[7]
#define m_AIControlCheckLockonAngle			m_pParam[8]

#define m_AIControlGotoLength				m_pParam[0]
#define m_AIControlGotoIndex				m_pParam[1]
#define m_AIControlGotoFrame				m_pParam[2]

#define m_AIControlCallbackIndex			m_pParam[0]

#define m_SummonStartIndex					m_pParam[0]
#define m_SummonEndIndex					m_pParam[1]


#define m_DashTrailLength					m_pParam[ 0 ]
#define m_DashTrailLifeTime					m_pParam[ 1 ]


#define m_KakeGoePlayType							m_pParam[ 5 ]
#define MAX_KAKEGOE_CUE_NUM							5


enum PARTS_TYPE
{
	PARTS_TYPE_ARMOR,
	PARTS_TYPE_WEAPON,
	PARTS_TYPE_PROJECTILE
};

enum PHYSICS_TYPE
{ 
	PHYSICS_CLOTH = 1, 
	PHYSICS_COLLISION = 2, 
	PHYSICS_RAGDOLL = 3, 
	PHYSICS_BREAKABLE = 4,
};

class BStream;

class ASData;
class CAniInfoData;

struct ASDATA_CONTAINER
{
	char *pFileName;
	ASData *pASData;
};

struct ANI_INFO_CONTAINER
{
	char *pFileName;
	CAniInfoData *pAniInfoData;
};

////////////////////////////
#define AS_SIGNAL_ROW_MAX		16

#define AS_SIGNAL_DATA_RESERVED	16
class ASSignalData
{
public:
	ASSignalData();
	~ASSignalData();

	int m_nFrame;
	int m_nID;
	int m_nRow;

	int m_NumParam;
	int* m_pParam;
//	int m_pParam[NUM_UNIT_SIGNAL_PARAM];
};


#define AS_PARTS_DATA_RESERVED	116
class ASPartsData
{
public:
	ASPartsData();
	virtual ~ASPartsData();

	bool Save( BStream *pStream);
	bool Load( BStream *pStream);

	void SetData(std::vector<std::string>* pPartsVec);
	void Clear();

	char m_cBoneName[64];
	short m_nPartsNum;
	char** m_ppSkinName;

	int m_nType;
	int m_nGroup;
	int m_nSimulation;

	// 게임에서 사용
	int* m_pSkinIndex;		

public:
	int GetPartsCount() { return m_nPartsNum; }
	char *GetPartsBoneName() { return m_cBoneName; }
	char *GetPartsSkinName(int nIndex) { return m_ppSkinName[nIndex]; }
};


#define AS_ANI_DATA_RESERVED	58
class ASAniData
{
public:
	ASAniData();
	virtual ~ASAniData();

	bool Save( BStream *pStream );
	bool Load( BStream *pStream );

protected:

	int m_nAniAttr;
	int m_nAniType;
	int m_nAniIndex;				// 애니메이션 몇번째 것인가

	int m_nNextAni;
	int m_nBlendFrame;
	int m_nMoveSpeed;
	int m_nAniLength;

public:
	int GetAniLength()			{ return m_nAniLength; }
	int GetAniType()			{ return m_nAniType; }
	int GetAniIndex()			{ return m_nAniIndex; }
	int GetAniAttr()			{ return m_nAniAttr; }
	int GetNextAni()			{ return m_nNextAni; }
	int GetBlendFrame()			{ return m_nBlendFrame; }
	float GetMoveSpeed()		{ return m_nMoveSpeed / 10.f; }


	void SetAniLength( int nLength )			{ m_nAniLength = nLength; }
	void SetAniType( int nType )				{ m_nAniType = nType; }
	void SetAniAttr( int nAttr )				{ m_nAniAttr = nAttr; }
	void SetNextAni( int nAni )					{ m_nNextAni = nAni; }
	void SetAniIndex( int nAniIndex )			{ m_nAniIndex = nAniIndex; }
	void SetBlendFrame( int nFrame )			{ m_nBlendFrame = nFrame; }
	void SetMoveSpeed( int nMoveSpeed )			{ m_nMoveSpeed = nMoveSpeed; }

};

class CAniSignal
{
public:
	CAniSignal();
	virtual ~CAniSignal();

protected:
	int m_nSignalNum;
	ASSignalData *m_pSignal;

public:
	bool LoadSignal( BStream *pStream );
	bool SaveSignal( BStream *pStream );
	int GetSignalCount() { return m_nSignalNum; }
	ASSignalData *GetSignal( int nIndex ) { return m_pSignal + nIndex; }
	void AdjustSignalSize( int nSignalID, int nSize );
	void CheckUseParticle( int *pUseCheck );
	void CheckUseFX( int *pUseCheck );

	void SetSignalCount( int nCount ) {
		m_nSignalNum = nCount;
		assert( m_pSignal == NULL );

		if( nCount > 0 )
		{
			m_pSignal = new ASSignalData[ nCount ];
//			memset( m_pSignal, 0, sizeof( ASSignalData ) * nCount );
		}
	}
};

#define AS_DATA_RESERVED	512
class ASData
{
public:
	ASData();
	virtual ~ASData();

	void Clear();
	bool LoadSignal( const char *pFileName );
	bool SaveSignal( const char *pFileName );
	void AdjustSignalSize( int nSignalID, int nSize );

	int m_nAniNum;
	CAniSignal *m_pAniSignal;
	std::vector<D3DXMATRIX> m_HitMatrixList;

	int m_nUseParticleCount;
	int *m_pUseParticleList;
	int m_nUseFXCount;
	int *m_pUseFXList;

public:
	CAniSignal *GetSignalList( int nIndex ) { return m_pAniSignal + nIndex; }
	D3DXMATRIX *GetHitMatrix(int nIndex) { return &(m_HitMatrixList[nIndex]); }
	void CalculateHitMatrix();
	int GetAniNum() { return m_nAniNum; }

	int GetUseParticleCount() { return m_nUseParticleCount; }
	int *GetUseParticleList() { return m_pUseParticleList; }
	int GetUseFXCount() { return m_nUseFXCount; }
	int *GetUseFXList() { return m_pUseFXList; }

	void SetAniNum( int nAniNum ) { 
		m_nAniNum = nAniNum; 
		assert( m_pAniSignal == NULL );
		assert( nAniNum > 0 );
		if( nAniNum > 0 )
		{
			m_pAniSignal = new CAniSignal[nAniNum];
//			memset( m_pAniSignal, 0, sizeof( CAniSignal ) * nAniNum );
		}
	}

protected:
	static void AdjustSignal( ASData* pASData );		// 시그널 겹친 녀석들 안 겹치게
	static void DeleteASData(int nIndex);

public:
	static std::vector<ASDATA_CONTAINER *> s_ASDataContainer;
	static ASData* LoadASData( const char* pFileName, int* pIndex = NULL );
	static bool SaveASData( const char* pFileName );
	static void DeleteASData(const char *pFileName);
	static void DeleteASData(ASData *pData);
	static void DeleteAllASData();
	static ASData* GetData( int nIndex );
};

#define UNIT_INFO_DATA_RESERVED	512
class CAniInfoData
{
public:
	CAniInfoData();
	virtual ~CAniInfoData();

protected:
	int m_nUnitID;
	int m_nAniTypeCount[ ANI_ATTR_NUM ][ ANI_TYPE_NUM ];
	int m_nAniNum;
	ASAniData *m_pAni;

	int m_nPartsGroupNum;
	ASPartsData *m_pPartsData;

public:

	bool LoadData( const char *pFileName );
	bool SaveData( const char *pFileName );

	int GetAniNum()	{ return m_nAniNum; }

	int GetAniAttr( int nAniIndex );
	int GetAniType(int nAniIndex);
	int GetAniTypeIndex(int nAniIndex);
	int FindSimilarAniIndex( int nType, int nAttr );
	int GetAniIndex( int nType, int nIndex, int nAttr = 0 );
	int GetBlendFrame(int nAniIndex);
	int GetNextAni(int nAniIndex);
	float GetMoveSpeed( int nAniIndex ) { return m_pAni[ nAniIndex ].GetMoveSpeed(); }
	ASAniData *GetAniData(int nAniIndex) { return m_pAni+nAniIndex; }
	int GetAniTypeCount( int nAniAttr, int nAniType );

	int GetPartsGroupCount() { return m_nPartsGroupNum; }
	ASPartsData *GetPartsGroupInfo(int nIndex) { return m_pPartsData + nIndex; }

	void SetAniNum( int nNum );
	void SetAniAttr( int nAniIndex, int nAttr )					{ m_pAni[ nAniIndex ].SetAniAttr( nAttr ); }
	void SetAniType( int nAniIndex, int nType )					{ m_pAni[ nAniIndex ].SetAniType( nType ); }
	void SetAniTypeIndex( int nAniIndex, int nTypeIndex )		{ m_pAni[ nAniIndex ].SetAniIndex( nTypeIndex ); }
	void SetBlendFrame( int nAniIndex, int nBlendFrame )		{ m_pAni[ nAniIndex ].SetBlendFrame( nBlendFrame ); }
	void SetNextAni( int nAniIndex, int nNextAni )				{ m_pAni[ nAniIndex ].SetNextAni( nNextAni ); }
	void SetMoveSpeed( int nAniIndex, int nMoveSpeed )			{ m_pAni[ nAniIndex ].SetMoveSpeed( nMoveSpeed ); }

	void SetPartsGroupCount( int nCnt ) {
		m_nPartsGroupNum = nCnt;
		assert( m_pPartsData == NULL );
		if( nCnt > 0 )
		{
			m_pPartsData = new ASPartsData[ nCnt ];
//			memset( m_pPartsData, 0, sizeof( ASPartsData ) * nCnt );
		}
	}


protected:
	static void DeleteAniInfoData(int nIndex);
public:
	static std::vector< ANI_INFO_CONTAINER *> s_AniInfoDataContainer;
	static CAniInfoData *LoadAniInfoData( const char* pFileName, int* pIndex = NULL );
	static bool			SaveAniInfoData( const char* pFileName );
	static void DeleteAniInfoData(const char *pFileName);
	static void DeleteAniInfoData( CAniInfoData *pData );
	static void DeleteAllAniInfoData();
	static CAniInfoData *GetData( int nIndex );
};
