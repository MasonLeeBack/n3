#pragma once

#include "BsMath.h"

// FXCommand 공용커맨드는 같은커맨드라도 대상 타입에 따라 파라메터의 형식이 달라질수 있음.
#define FX_INIT_OBJECT				5000 // dwParam1 : char* TexName = NULL;
#define FX_PLAY_OBJECT				5001 // FX_TYPE_WATERDROP -> dwParam1* : int nOften(생성빈도: 0~100),  dwParam2 : float* fMaxDropSize (기준 : 화면전체 -> 2.f, DEF(0) : 0.001)
										 //..
#define FX_STOP_OBJECT				5002 // dwParam1 : char* immediately
#define FX_DELETE_OBJECT			5003
#define FX_SETCOLOR					5004 // dwParam1 :  D3DXVEVTOR3* 
#define FX_SETLIFE					5005 // dwParam1 :  float*		: FRAME단위
#define FX_UPDATE_OBJECT			5006 // 
#define FX_GET_STATE				5007 // dwParam1 (OUT : State)
#define FX_SET_TEXTURE				5008 // dwParam1 (char*)
#define FX_FORCED_DELETE			5009
#ifndef _LTCG
	#define FX_GET_RTTI					5500
#endif //_LTCG


#define FX_LINKED_PLAY_OBJECT		5990 // dwParam1 : nFxIndex, dwParam2 : ( nParentObjectIndex<<16 | nBoneIndex ), dwParam3 : (DWORD)pCross
#define FX_PLAY_OBJECT_PARENT		5995 // dwParam1 : FX_BY_PARENT_PLAY_FOR
#define	FX_LINKED_STOP_OBJECT		5996 // dwParam1 : ( nParentObjectIndex<<16 | nBoneIndex )

//Trail 칼트레일
#define FX_ADD_TRAIL				5010// dwParma1 : D3DXVEVTOR3[2]의 address. 0- 하단점, 1- 상단점
#define FX_SET_OFFSETTEXTURE		5011
#define FX_SET_DIFFTEXTURE			5012
#define FX_START_TRAIL				5013
#define FX_SET_ALPHA				5014
//#define FX_STOP_TRAIL				5012

// Lens Flare
#define FX_SET_LIGHTDIRECTION		5020 // dwParam1 = Light Dir
#define FX_GET_SUNVISIBILITY		5021

//Tidal wave

//Simple Play
#define FX_FIRE_OBJECT 5022


// DropOfWater ( 튜르르 물방울 오브어텍 )
#define FX_PLAY_SIMPLE_PARENT_PARTICLE 5023

//CFcFxThrowGenerater 에서 사용
#define FX_PLAY_BECOMING			5024
#define FX_PLAY_BLAST				5025
#define FX_PLAY_EXPLOSION			5026
#define FX_STOP_BECOMING			5027
#define FX_STOP_BLAST				5028
#define FX_STOP_EXPLOSION			5029

#define FX_RESERVE_OBJECT			5030

//Screen MotionBlur
#define FX_SETBLUR_INTENCITY		5040 // dwParam1 :  float*	: (0~1.f)  Defalut 1.0f

// True orb spark Water
#define FX_WATER_GET_VERTEX_COUNT	5060
#define FX_WATER_GET_VERTEX			5061
#define FX_WATER_GET_VELOCITY		5062
#define FX_WATER_IS_FINISHED		5063


// True orb spark Fire
#define FX_FIRE_EXPLODE				5070
#define FX_ADD_VALUE				FX_FIRE_EXPLODE+1		// dwParam1, dwParam2, dwParam3 => 각각의 포인터 주소.
#define FX_ADD_VALUE2				FX_ADD_VALUE+1			// dwParam1, dwParam2, dwParam3. => 필요에 의한 값 추가.
#define FX_SET_PARENT_STATE			FX_ADD_VALUE2+1			// dwParam1 : int 
#define	FX_SET_VOLUME_TEXTURE		FX_SET_PARENT_STATE+1	// dwParam1 : char*
#define FX_GET_INTENSITY			FX_SET_VOLUME_TEXTURE+1	// dwParam1을 통해 가져옴 ( 카메라와의 거리를 통한 빛무리 세기를 가져온다.)
#define FX_SET_PRELOADTEX			FX_GET_INTENSITY+1		// dwParam1 이 미리 로드된 텍스쳐 인덱스

// True orb spark Earth
#define FX_GET_MESHFRAME			5080


#define FX_GET_POS					5090
#define FX_SET_BLENDOP				5091
#define FX_SET_SRCBLEND				5092
#define FX_SET_DSTBLEND				5093


//LomoFilter
#define FX_SET_LOMOINTENSITY		5100


//2D Full Screen Effect
#define FX_SET_FSALPHA				5110 // dwParam1 : float* : (0~1.f)  Defalut 1.0f
#define FX_SET_FSINTERVAL			5111 // dwParam1 : int
#define FX_SET_FSDEVIDE				5112 // dwParam1 : int : 한텍스쳐를 몇등분으로 나눠서 사용할지(2^n)추천


const	int c_MAX_PRIORITY			= 2;
enum FX_STATE { READY, PLAY, STOP, END, ENDING };

enum FX_DEVICEDATASTATE { FXDS_READY, FXDS_INITREADY, FXDS_INITED, FXDS_RELEASEREADY, FXDS_RELEASED };


enum FX_TYPE
{
	FX_TYPE_WEAPONTRAIL=0,
	FX_TYPE_WATERDROP  =1,
	FX_TYPE_LENSFLARE  =2,
	FX_TYPE_TIDALWAVE  =3,	
	FX_TYPE_MOTIONBLUR =4,
	FX_TYPE_METEOR	   =5,
	FX_TYPE_METEOR_BABY=6,
    FX_TYPE_SIMPLE_PLAY=7,
	FX_TYPE_EARTHEXPLODE=8,
	FX_TYPE_CASTEARTH=9,
	FX_TYPE_WINDEXPLODE=10,
	FX_TYPE_CASTWIND=11,

	FX_TYPE_XM=12,
	FX_TYPE_LINECURVE=13,
	FX_TYPE_LINEPARABOLA=14,
	FX_TYPE_LINETRAIL=15,
	FX_TYPE_DYNAMICFOG = 16,
	FX_TYPE_BBOARD = 17,
	FX_TYPE_LOMOFILTER = 18,
	FX_TYPE_PIECEOFMETEOR = 19,
	FX_TYPE_INPHYMOTIONBLUR = 20,
	FX_TYPE_COLUMNWATER = 21,
	FX_TYPE_2DFSEFFECT = 22,
	FX_TYPE_DROPOFWATER = 23,
	FX_TYPE_SPOUTSOFWATER = 24,
	FX_TYPE_BURNING = 25,
	FX_TYPE_GENERATER = 26,
	FX_TYPE_STEM = 27,
	FX_TYPE_THROWGENERATER = 28,
	FX_TYPE_PARTICLEGENERATER = 29,
	FX_TYPE_LIGHTSCATTERING = 30,
	FX_TYPE_LISTGENERATER = 31,
	FX_TYPE_RAINS = 32,
	FX_TYPE_RANDOMGENERATER = 33,
	FX_TYPE_SHOTSKIN = 34,
	FX_TYPE_COUNT,
};


// FX_TYPE_LINECURVE MSG
// FX_INIT_OBJECT : 1:(int)MAX Sample Point Count, 2:(float*) Minimum Interpolate Distant, 3:(float*) Line Width
// FX_SET_TEXTURE
// FX_PLAY_OBJECT 
// FX_STOP_OBJECT 
// FX_DELETE_OBJECT 
// FX_UPDATE_OBJECT : 1:(int)Sample Point Count, 2:(D3DXMATRIX*) SAMPLE MATRIX Array

