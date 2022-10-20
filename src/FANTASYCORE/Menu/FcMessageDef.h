#pragma once

enum FcMSG_ASK_TYPE
{
	MSG_ABORTRETRYIGNORE,
	MSG_OK,
	MSG_OKCANCEL,
	MSG_RETRYCANCEL,
	MSG_YESNO,
	MSG_YESNOCANCEL,
	MSG_MAX,
};

enum FcMSG_RESPONSE_TYPE
{
	ID_ABORT,
	ID_CANCEL, 
	ID_IGNORE,
	ID_NO,
	ID_OK,
	ID_RETRY,
	ID_YES,
	ID_ASYNC,
	ID_TIMEOUT,
};

enum FcMSG_TYPE
{	
	fcMT_MissionStart,					//UiGuardian : 게임을 시작하겠는가?
	fcMT_ExitMissionToTitle,			//uipause, uiUsual, uiFailed : title로 돌아 가겠는가?
	fcMT_RestartMission,				//uiFailed : 다시 시작하겠는가?
	fcMT_GotoSelectStage,				//uiFailed : world map으로 가겠는가?
	fcMT_LossOfController,
	fcMT_SaveGameData,					//uisavedata : data를 저장하겠는가?	
	fcMT_OverWriteGameData,				//uisavedata : data를 넢어 씌우겠는가?
	fcMT_LoadGameData,					//uiloaddata : data를 불러올것인가?
	fcMT_LoadGameDataWarning,			//uiloaddata : 현재 게임내용이 소실 될 수 있습니다. data를 불러올 것인가?
	fcMT_LibMovieStart,
	fcMT_SpecialSelStage,				//uiSpecialSelStage : 선택한 스테이지로 분기하겠는가?
	fcMT_AskClearStageSave,				//uistageresult : 한 char에 대해 clear한 정보를 저장하겠는가?
	fcMT_ExitClearStageSave,			//uisavedata : stage clear후 save menu에서 나갈때 메세지
	fcMT_ExitClearAllSave,				//uisavedata : 한 char에 대해 all stage clear후 save menu에서 나갈때 메세지

	
	fcMT_FailLevelItem,					//uistatus : Level이 맞지 안아 장착이 안된다.
	fcMT_NotStateChangeItem,			//uistatus : 던지거나 적을 찔러 들고 있는 경우, 바꿀 수 없다.
	fcMT_NoneItemInSlot,				//uistatus : 삭제될 slot에 아템이 없는 경우.
	fcMT_SucessChangeItem,				//uistatus : 아이템 변경이 성공한 경우.
	fcMT_NotAccountWeapon,				//uistatus : 무기를 해제하고 착용을 안할 경우.
	fcMT_OneEquipThisMission,			//uistatus : 한미션당 하나만 사용하는 item인데, 이미 사용한 경우

	fcMT_NoSignInWarning,				// 사인인 하지 않고 게임을 하면 세이브 게임을 할수 없음을 경고
	fcMT_SavingNow,						//uisavedata : 저장중...

	fcMT_LibOpenItem,					//Point가 n만큼드는데, 아이템을 열어 보겠는가?
	fcMT_LibOpenItemInWarning,			//Point가 모자르다..

	fcMT_NoDeviceWarning,				//장치를 선택하지 않아서 세이브가 불가능해짐을 경고
	fcMT_CancelDeviceSelector,
	fcMT_RemoveStorage,					//사용중인 장치가 제거되었을때 경고

	fcMT_FailEnumerateContents,			//컨첸츠 Enumeration 실패
	fcMT_FailLoadContent,				//컨텐트 열기 실패

	fcMT_DiskFree,						//선택된 스토리지가 용량이 부족하지만 그냥 진행할건지 아닌지 
	fcMT_DiskFull,						//현재 선택된 스토리지가 용량이 꽉찼음
	fcMT_ContentsLimit,					//세이브데이타갯수를 50로 제한

	fcMT_ProfileDelete,					//디바이스 장치를 바꿀때 기존프로필이 덮어씌워질 수 있음을 경고
	
	fcMT_MAX,
};

struct FcMessageDefine
{
	FcMSG_TYPE			nfcMsgId;
	FcMSG_ASK_TYPE		nAskId;
	int					nTextId;
};


struct FctempHeroInfo
{
	int		nPortraitID;
	char*	szName;
};

extern FcMessageDefine	g_FcMsgDef[];