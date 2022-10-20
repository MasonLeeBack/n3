#include "StdAfx.h"
#include "FcMessageDef.h"
#include "data/Text/DefaultTextTableDefine.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG

FcMessageDefine g_FcMsgDef[] = 
{	
	{ fcMT_MissionStart,		MSG_YESNO,	_TEX_MSG_START_GAME },
	{ fcMT_ExitMissionToTitle,	MSG_YESNO,	_TEX_MSG_EXIT_STAGE },
	{ fcMT_RestartMission,		MSG_YESNO,	_TEX_MSG_RESTART_STAGE },
	{ fcMT_GotoSelectStage,		MSG_YESNO,	_TEX_MSG_GO_TO_WORLD },
	{ fcMT_LossOfController,	MSG_OK,		_TEX_MSG_LOSS_CONTROLLER },
	{ fcMT_SaveGameData,		MSG_YESNO,	_TEX_MSG_SAVE_DATA },
	{ fcMT_OverWriteGameData,	MSG_YESNO,	_TEX_MSG_OVERWRITE_DATA },
	{ fcMT_LoadGameData,		MSG_YESNO,	_TEX_MSG_LOAD_DATA },
	{ fcMT_LoadGameDataWarning,	MSG_YESNO,	_TEX_MSG_LOAD_DATA_WARNING },
	{ fcMT_LibMovieStart,		MSG_YESNO,	_TEX_MSG_LOAD_DATA },
	{ fcMT_SpecialSelStage,		MSG_YESNO,	_TEX_MSG_SEL_STAGE },
	{ fcMT_AskClearStageSave,	MSG_YESNO,	_TEX_MSG_ASK_STAGE_SAVE },
	{ fcMT_ExitClearStageSave,	MSG_YESNO,	_TEX_MSG_EXIT_STAGE_SAVE },
	{ fcMT_ExitClearAllSave,	MSG_YESNO,	_TEX_MSG_EXIT_CLEAR_ALL_SAVE },
	{ fcMT_FailLevelItem,		MSG_OK,		_TEX_MSG_FAIL_LEVEL_ITEM },
	{ fcMT_NotStateChangeItem,	MSG_OK,		_TEX_MSG_FAIL_STATE_ITEM },
	{ fcMT_NoneItemInSlot,		MSG_OK,		_TEX_MSG_NONE_ITEM_IN_SLOT },
	{ fcMT_SucessChangeItem,	MSG_OK,		_TEX_MSG_SUCCESS_CHANGE_ITEM },
	{ fcMT_NotAccountWeapon,	MSG_OK,		_TEX_MSG_NOT_ACCOUNT_WEAPON },
	{ fcMT_OneEquipThisMission, MSG_OK,		_TEX_MSG_ONE_EQUIP_THIS_MISSION },

	{ fcMT_NoSignInWarning,		MSG_YESNO,	_TEX_MSG_NO_SIGNIN_WARNING },
	{ fcMT_SavingNow,			MSG_MAX,	_TEX_MSG_SAVING_NOW },			//MSG_MAX인 경우는 버튼이 없는 경우이다.

	{ fcMT_LibOpenItem,			MSG_YESNO,	_TEX_MSG_LIB_OPEN_ITEM },
	{ fcMT_LibOpenItemInWarning,MSG_OK,		_TEX_MSG_WARNING_LIB_OPEN_ITEM },

	{ fcMT_NoDeviceWarning,		MSG_YESNO,	_TEX_MSG_WARNING_NO_DEVICE },
	{ fcMT_CancelDeviceSelector,MSG_YESNO,	_TEX_MSG_CANCEL_DEVICE_SELECTOR },
	{ fcMT_RemoveStorage,		MSG_YESNO,	_TEX_MSG_REMOVE_STORAGE },
	{ fcMT_FailEnumerateContents,MSG_OK,		_TEX_MSG_FAIL_ENUMERATE_CONTENTS },
	{ fcMT_FailLoadContent,		MSG_OK,		_TEX_MSG_FAIL_LOAD_CONTENT },
	{ fcMT_DiskFree,			MSG_YESNO,	_TEX_MSG_DISK_FREE },
	{ fcMT_DiskFull,			MSG_YESNO,	_TEX_MSG_DISK_FULL },
	{ fcMT_ContentsLimit,		MSG_YESNO,	_TEX_MSG_CONTENTS_LIMIT },
	{ fcMT_ProfileDelete,		MSG_YESNO,	_TEX_MSG_PROFILE_DELETE },


};
