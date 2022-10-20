#pragma once

enum
{
	RMTYPE_OBJ = 0,
	RMTYPE_ACT,
	RMTYPE_LIP,
	RMTYPE_FACE,
	RMTYPE_BONE,
	RMTYPE_WAVE,
	RMTYPE_CAM,
	RMTYPE_EFFECT,
	RMTYPE_PACKAGE,

	RMTYPE_MAX,

	RMTYPE_EFFECT_FADEIN,
	RMTYPE_EFFECT_FADEOUT,
	RMTYPE_EFFECT_BLACK,
	RMTYPE_EFFECT_CMD_FX,	// Trigger 호출을 통해 발동되는 FX
	RMTYPE_EFFECT_FX,
	RMTYPE_EFFECT_USECAM,
	RMTYPE_EFFECT_SHOWOBJECT,
	RMTYPE_EFFECT_SETTROOP,
	RMTYPE_EFFECT_MOVETROOP,
	RMTYPE_EFFECT_USELIGHT,
	RMTYPE_EFFECT_USEDOF,
	RMTYPE_EFFECT_USESOUND,

	RMTYPE_EFFECT_NONE,

};

// Tool Tree UI 관련 되어서만 쓰이는 메시지
// rmCommand 에서 사용.
enum
{
	RMCMD_GET_OBJ_CNT = 100,
	RMCMD_GET_ACT_CNT,
	RMCMD_GET_LIP_CNT,
	RMCMD_GET_FACE_CNT,
	RMCMD_GET_WAVE_CNT,
	RMCMD_GET_CAMERA_CNT,
	RMCMD_GET_EFFECT_CNT,

	RMCMD_GET_OBJ_PTR_LIST,
	RMCMD_GET_ACT_PTR_LIST,
	RMCMD_GET_LIP_PTR_LIST,
	RMCMD_GET_FACE_PTR_LIST,
	RMCMD_GET_BONE_PTR_LIST,
	RMCMD_GET_WAVE_PTR_LIST,
	RMCMD_GET_CAMERA_PTR_LIST,
	RMCMD_GET_EFFECT_PTR_LIST,

	RMCMD_SET_MAPFILE,			// 맵파일 설정.
	RMCMD_SET_FOV,				// FOV
	RMCMD_SET_CAMYAW,
	RMCMD_SET_CAMROLL,
	RMCMD_SET_CAMPITCH,
};


struct rmMsgItem
{
	int		nType;
	char*	szObjName;
	char*	szSubName;
	int		nReturn;
	DWORD	dwRetParam;

	rmMsgItem() 
		: nType(-1)
		, szObjName( NULL )
		, szSubName( NULL )
		, nReturn(0)
		, dwRetParam(0) {};

	rmMsgItem( int type, char* objname, char* subname)
	{
        nType = type;
		szObjName = objname;
		szSubName = subname;
		nReturn = 0;
		dwRetParam = 0;
	}
};

struct rmMsgChangeNameItem
{
	char*	szObjName;
	char*	szSubName;
	char*	szChangeName;

	rmMsgChangeNameItem() : szObjName(NULL),szSubName(NULL),szChangeName(NULL){};
};

struct rmMsgTimeItemByNodePtr
{
	int		nType;
	DWORD	dwNodePtr;	// 노드별 포인터.
	float	fTime;
	DWORD	dwTime;

	rmMsgTimeItemByNodePtr() : nType(0),dwNodePtr(0),fTime(-1.f),dwTime(-1){};
	rmMsgTimeItemByNodePtr(int type,DWORD node,float time,DWORD nTime) 
		: nType(type),dwNodePtr(node),fTime(time),dwTime(nTime) {};
};

struct rmCommand
{
	int	nCmdType;
	char* szObjName;
	char* szSubName;

	rmCommand() : nCmdType(-1), szObjName(NULL), szSubName(NULL){};
	rmCommand(int nCType,char *objname,char* subname)
	{
		nCmdType = nCType;
		szObjName = objname;
		szSubName = subname;
	}
};

struct rmEnvCmd 
{
	int	nCmdType;
	DWORD dwParam1;
	DWORD dwParam2;

	rmEnvCmd() : nCmdType(-1), dwParam1(0), dwParam2(0) {};
	rmEnvCmd(int type,DWORD param1,DWORD param2) 
		: nCmdType( type )
		, dwParam1( param1 )
		, dwParam2( param2 ) {};
};

struct rmEffWavSet
{
	DWORD	dwEffectPtr;
	DWORD	dwWavePtr;
};

typedef std::vector< VOID*	>		vecPtrList;
typedef std::vector<rmEffWavSet>	vecEffWavList;

enum
{
	RM_MSG_NONE = 0,
	RM_MSG_START = 0,

	RM_INSERT_OBJ = 1,
	RM_INSERT_ACT,		// dwParam1=[in]rmMsgItem* , dwParam2=[out]char* ObjName
	RM_INSERT_LIP,		// dwParam1=[in]rmMsgItem* , dwParam2=[out]char* LipName
	RM_INSERT_FACE,		// dwParam1=[in]rmMsgItem* , dwParam2=[out]char* FaceName
	RM_INSERT_BONE,
	RM_INSERT_WAVE,
	RM_INSERT_CAMERA,
	RM_INSERT_EFFECT,
	RM_INSERT_PARTICLE,


	RM_DELETE_OBJ,
	RM_DELETE_ACT,
	RM_DELETE_LIP,
	RM_DELETE_FACE,
	RM_DELETE_BONE,
	RM_DELETE_WAVE,
	RM_DELETE_CAMERA,
	RM_DELETE_CAMERA_PATH,
	RM_DELETE_EFFECT,

	RM_GET_OBJECT_MEMBER,
	RM_GET_ACT_MEMBER,
	RM_GET_LIP_MEMBER,
	RM_GET_FACE_MEMBER,
	RM_GET_BONE_MEMBER,

	RM_GET_WAVE_MEMBER,
	RM_GET_CAMERA_MEMBER,

	RM_GET_EFFECT_MEMBER,

	RM_GET_OBJ_PTR,
	RM_GET_ACT_PTR,			// [1]=char* szObjName, [2]=char* szActName Act 자체 포인터.
	RM_GET_LIP_PTR,
	RM_GET_FACE_PTR,
	RM_GET_BONE_PTR,
	RM_GET_WAVE_PTR,
	RM_GET_CAMERA_PTR,
	RM_GET_EFFECT_PTR,

	// 이름 변경은 중요. map 의 key이므로 주의를 요함.
	RM_CHANGE_OBJ_MEMBER,		
	RM_CHANGE_ACT_MEMBER,
	RM_CHANGE_LIP_MEMBER,
	RM_CHANGE_FACE_MEMBER,
	RM_CHANGE_BONE_MEMBER,
	RM_CHANGE_WAVE_MEMBER,
	RM_CHANGE_CAMERA_MEMBER,
	RM_CHANGE_EFFECT_MEMBER,

	RM_APPLY_EFFECTFILE,	// dwParam1=Index (Effect file name 적용)

	RM_RESET_TIME_ACT,      // dwParam1=Index
	RM_RESET_TIME_LIP,
	RM_RESET_TIME_FACE,
	RM_RESET_TIME_WAVE,
	RM_RESET_TIME_CAMERA,
	RM_RESET_TIME_EFF,

	RM_SET_DISABLE_WAVE,	// dwParam1=Index,	dwParam2=0,1
	RM_SET_DISABLE_EFF,     // dwParam1=Index,	dwParam2=0,1

	RM_DEF_MSG_END, // 디파인 -------------------------------


	RM_GET_NODE_PTR,
	RM_SET_TIME_BY_NODEPTR,
	RM_GET_NODE_NAME_BY_NODEPTR,			// NodePtr 을 필요로 함.

	RM_PRE_PLAY,				// 이벤트 맵을 만들기 위한 과정. 애니메이션 시작 전에 필요. (임시가 될까?)

	RM_COMMAND,
	RM_ENVIRONMENT_CMD,			// 작업 환경을 위한 메시지 rmEnvCmd, 	enumEnvCmdType 사용

	RM_DELETE_MSG_END, // 삭제 ------------------------------
};

enum	enumMemberType
{
	RMT_OBJNAME,		// Act,Lip,Face의 경우 연결된 오브젝트 명을 얻어올 수 있다.
	RMT_SKINFILENAME,
	RMT_BAFILENAME,
	RMT_ANICOUNT,
	RMT_OBJ_POS_X,		// Cam 도 쓰임. Effect 도 쓰임.
	RMT_OBJ_POS_Y,
	RMT_OBJ_POS_Z,
	RMT_OBJ_ROTATE,
	RMT_RELOADTYPE,		// Reload를 위한 type. 내용 변경시 사용.

	// RMT_HAIRSKINNAME,	// 머리 스킨
	// RMT_MANTLESKINNAME,	// 망토 스킨.

	RMT_PHYSICSSKINNAME,	// Physics 먹은 스킨.	Index 필요.
	RMT_PHYSICSAPPLY,		// Physics 적용.
	RMT_LINKBONEAPPLY,		// Bone 적용
    
	RMT_LINKBONE,			// Link Bone
	RMT_UNLINKBONE,
	

	RMT_ACTNAME,
	RMT_ANIINDEX,		// Lip,Face 에서 쓰임.
	RMT_STARTTIME,		// Lip, Cam,Wave 에서도 쓰임.
	RMT_STARTTICK,		// int형 시작 Tick
	RMT_TEXT_ID,		// Lip, Wave 에서 사용.
	RMT_USE_VOICE_CUE,	// Lip없이 Voice Cue 사용할것인가.
	RMT_ACTREPEAT,

	RMT_ACT_STARTFRAME,
	RMT_ACT_ENDFRAME,

	RMT_CROSSVECTOR,	// 위치 값.
	RMT_PATH_CROSS,		// 패스의 CrossVector
	RMT_PATHCOUNT,		// 패스 갯수.

	RMT_LIPNAME,
	RMT_LTFNAME,
	RMT_WAVNAME,		// 다른 목록에서도 쓰임.

	RMT_FACENAME,
	RMT_FACE_USETIME,
	RMT_FACECOMPLEMENT,	// 보간.
	RMT_BONENAME,
	RMT_BONE_INDEX,

	RMT_BONE_NAME0,
	RMT_BONE_NAME1,
	RMT_BONE_NAME2,
	RMT_BONE_NAME3,
	RMT_BONE_USE0,
	RMT_BONE_USE1,
	RMT_BONE_USE2,
	RMT_BONE_USE3,

	RMT_BONE_ROT_X,
	RMT_BONE_ROT_Y,
	RMT_BONE_ROT_Z,
	RMT_BONE_USETIME,

	
	// RMT_WAVNAME,		// 다른 목록에서도 쓰임.
	RMT_EFFECTNAME,
	RMT_CAMERANAME,

	RMT_WAVFILENAME,
	RMT_WAVVOLUME,		// 볼륨
	RMT_WAVLOOP,
	RMT_WAVELINK_OBJ,

	RMT_CAMFILENAME,
	RMT_CAM_USEOFFSET,
	RMT_CAM_FOV,			// Fov

	RMT_CAM_PATH_X,
	RMT_CAM_PATH_Y,
	RMT_CAM_PATH_Z,
	RMT_CAM_PATH_FOV,
	RMT_CAM_PATH_DURATION,
	RMT_CAM_PATH_SWAYTYPE,
	RMT_CAM_PATH_ROLL,
	RMT_CAM_PATH_MOVETYPE,


	RMT_EFFECTTYPE,		// Effect Type
	RMT_EFFECTFILENAME,
	RMT_EFFECTLINKEDOBJNAME,
	RMT_EFFECTLIFETIME,	// use time

	RMT_EFFECTLINKTYPE,
	RMT_EFFECTSTRPARAM,
	RMT_EFFECTPARAM1,
	RMT_EFFECTPARAM2,
	RMT_EFFECTPARAM3,
	RMT_EFFECT_YAW,
	RMT_EFFECT_ROLL,
	RMT_EFFECT_PITCH,

	RMT_EFFECT_FLOAT_PARAM1,
	RMT_EFFECT_FLOAT_PARAM2,
	RMT_EFFECT_FLOAT_PARAM3,
	RMT_EFFECT_FLOAT_PARAM4,

	RMT_EFFECT_LINK_FX,		 // 

	RMT_OBJ_PTR,
	RMT_ACT_PTR,
	RMT_LIP_PTR,
	RMT_FACE_PTR,
};

enum enumEnvCmdType
{
	ENVCMD_NONE = 0,
	ENVCMD_CAM_FLAG = 1,		// Cam 사용 유무 설정.
};

enum enumEffectLinkType
{
	eLT_None = 0,
	eLT_File,
	eLT_Object,
	eLT_Wave,
	eLT_Camera,
	eLT_Act,

	eLT_End,
};

enum enumEffectType
{ 
	eNot = 0, 
	eFadeIn, 
	eFadeOut, 
	eBlack, // 화면 관련 입니다
	eSpecialFX,		// Trigger 명령을 이용한 FX
	eTerminate, // 끝났을때 통지 합니다 
	eCustomType,	// 사용자가 정의하는 타입.
	eUseCam,		// 카메라 변경.
	eFX,			// FX
	eShowObject,	// 오브젝트 Show On/Off
	eUseBone,
	eSetFog,		// Fog 조절.

	eSetTroop,		// 군세 설정.
	eMoveTroop,		// 군세 이동.
	eUseLight,		// Light 사용.
	eUseDOF,		// DOF 사용.

	eUseWave,		// Wave 사용.
	eWaveVolume,	// Wave Volume Control.

	eDropWeapon,	// 무기 떨어트리기.
	eObjAlphaBlend,	// 오브젝트 알파블랜딩.

	eEffectMax,
	eReuseAct,		// Act 다시 사용.

}; // 해당 이펙트의 타입 입니다

enum enumRMObjType
{
	eObjType_None = 0,
	eObjType_Player1 = 1,
	eObjType_Player2,
	eObjType_P1_Weapon,
	eObjType_P2_Weapon,
	eObjType_Max
};

extern char* g_lpszEffectStr[];		// BsRealMovieObject.cpp에 데이터 정의됨.
extern char* g_lpszPackageStr[];	// BsRealMoviePackage.cpp에 데이터 정의됨.
