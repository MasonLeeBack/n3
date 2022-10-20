#pragma once

#include "SmartPtr.h"

class ASSignalData;
class CFcGameObject;
class CFcGameObject;

typedef CSmartPtr<CFcGameObject> GameObjHandle;

#define DEFAULT_FRAME_RATE				40
#define MAX_PLAYER_COUNT				4
#define MAX_TEAM_COUNT					16

#define DEFAULT_GRAVITY		-2.4F
#define AIR_HEIGHT_GAP			10.0f
#define DEFAULT_GROUND_RESIST	0.5f

#define HIT_TYPE_UNIT					0
#define HIT_TYPE_PROJECTILE				1

#define TERRAIN_PARTICLE_START_INDEX		200

enum HIT_TYPE //HT
{
	HT_NORMAL_ATTACK,
	HT_ABIL_EARTH,
	HT_ABIL_FIRE,
	HT_ABIL_LIGHT,
	HT_ABIL_RED_LIGHT,
	HT_ABIL_SOUL,
	HT_ABIL_WATER,
	HT_ABIL_WIND,
	HT_BY_TRAP,
};


#define GENERATE_ORB_NORMAL		0
#define GENERATE_ORB_DISABLE	1
#define GENERATE_ORB_RED		2
#define GENERATE_ORB_BLUE		3
struct HIT_PARAM
{
	HIT_TYPE nHitType;
	CSmartPtr<CFcGameObject> ObjectHandle;
	ASSignalData *pHitSignal;
	int	nHitRemainFrame;
	int nAdjutantHitRemainRatio;
	int nAttackPower;
	D3DXVECTOR3 Position;
	int nGroundResist;
	float fVelocityY;
	float fVelocityZ;
	int nNotGenerateOrb;
	int nNotLookAtHitUnit;
	int nGuardBreak;
	int nStunProb;
	int nStunDelay;
	int nUseHitDir;
	int nAddCritical;
	D3DXVECTOR3 HitDir;
	int nGenerateOrbPercent;

	HIT_PARAM() {
		nHitType = HT_NORMAL_ATTACK;
		nGroundResist = 0;
		fVelocityY = 0.0f;
		fVelocityZ = 0.0f;
		nNotGenerateOrb = GENERATE_ORB_NORMAL;
		nNotLookAtHitUnit = 0;
		nGuardBreak = 0;
		nStunProb = 0;
		nStunDelay = 0;
		nUseHitDir = 0;
// [PREFIX:beginmodify] 2006/2/16 junyash PS#5208 reports nAddCritical uninitialized using.
		nAddCritical = 0;
// [PREFIX:endmodify] junyash
		nGenerateOrbPercent = 100;
		nAdjutantHitRemainRatio = 1;
	}
};

class ASSignalData;
struct PROJECTILE_PARAM
{
//	int nCollisionCheckType;
//	int nOrientType;
	GameObjHandle hParent;							// 쏜 녀석
	GameObjHandle hTarget;							// 목표 녀석
	D3DXVECTOR3 vTarget;
	float		fAngle;
	ASSignalData *pSignal;
	int nHitProb;
	float fSize;
	/*
	bool bPierce;
	int  nFxIdx;
	*/

	PROJECTILE_PARAM() {
		pSignal = NULL;
		nHitProb = 100;
		fSize = 1.f;
		/*
		bPierce = false;
		nFxIdx = -1;
		*/
	}
};

#define PRJ_COLLISION_CHECK_LINE				0		// 라인으로 충돌체크..
#define PRJ_COLLISION_CHECK_SPHERE				1		// 구로 충돌체크..
#define PRJ_COLLISION_CHECK_BOUNDING_BOX		2		// 박스로 충돌체크..

#define PRJ_ORIENT_TYPE_ARROW					0		// 화살같이.. 날아가는 방향으로 향하는 타입..
#define PRJ_ORIENT_TYPE_ROCK					1		// 바위돌처럼 돌면서 날아가는 타입..


#ifndef SAFE_DELETE
#define SAFE_DELETE(p) if(p) { delete p; p = NULL; }
#endif // SAFE_DELETE

#ifndef SAFE_DELETEA
#define SAFE_DELETEA(p) if(p) { delete []p; p = NULL; }
#endif //SAFE_DELETEA



typedef CSmartPtr<CFcGameObject> GameObjHandle;