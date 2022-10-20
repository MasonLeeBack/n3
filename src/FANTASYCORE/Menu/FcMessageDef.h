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
	fcMT_MissionStart,					//UiGuardian : ������ �����ϰڴ°�?
	fcMT_ExitMissionToTitle,			//uipause, uiUsual, uiFailed : title�� ���� ���ڴ°�?
	fcMT_RestartMission,				//uiFailed : �ٽ� �����ϰڴ°�?
	fcMT_GotoSelectStage,				//uiFailed : world map���� ���ڴ°�?
	fcMT_LossOfController,
	fcMT_SaveGameData,					//uisavedata : data�� �����ϰڴ°�?	
	fcMT_OverWriteGameData,				//uisavedata : data�� ���� ����ڴ°�?
	fcMT_LoadGameData,					//uiloaddata : data�� �ҷ��ð��ΰ�?
	fcMT_LoadGameDataWarning,			//uiloaddata : ���� ���ӳ����� �ҽ� �� �� �ֽ��ϴ�. data�� �ҷ��� ���ΰ�?
	fcMT_LibMovieStart,
	fcMT_SpecialSelStage,				//uiSpecialSelStage : ������ ���������� �б��ϰڴ°�?
	fcMT_AskClearStageSave,				//uistageresult : �� char�� ���� clear�� ������ �����ϰڴ°�?
	fcMT_ExitClearStageSave,			//uisavedata : stage clear�� save menu���� ������ �޼���
	fcMT_ExitClearAllSave,				//uisavedata : �� char�� ���� all stage clear�� save menu���� ������ �޼���

	
	fcMT_FailLevelItem,					//uistatus : Level�� ���� �Ⱦ� ������ �ȵȴ�.
	fcMT_NotStateChangeItem,			//uistatus : �����ų� ���� �� ��� �ִ� ���, �ٲ� �� ����.
	fcMT_NoneItemInSlot,				//uistatus : ������ slot�� ������ ���� ���.
	fcMT_SucessChangeItem,				//uistatus : ������ ������ ������ ���.
	fcMT_NotAccountWeapon,				//uistatus : ���⸦ �����ϰ� ������ ���� ���.
	fcMT_OneEquipThisMission,			//uistatus : �ѹ̼Ǵ� �ϳ��� ����ϴ� item�ε�, �̹� ����� ���

	fcMT_NoSignInWarning,				// ������ ���� �ʰ� ������ �ϸ� ���̺� ������ �Ҽ� ������ ���
	fcMT_SavingNow,						//uisavedata : ������...

	fcMT_LibOpenItem,					//Point�� n��ŭ��µ�, �������� ���� ���ڴ°�?
	fcMT_LibOpenItemInWarning,			//Point�� ���ڸ���..

	fcMT_NoDeviceWarning,				//��ġ�� �������� �ʾƼ� ���̺갡 �Ұ��������� ���
	fcMT_CancelDeviceSelector,
	fcMT_RemoveStorage,					//������� ��ġ�� ���ŵǾ����� ���

	fcMT_FailEnumerateContents,			//��þ�� Enumeration ����
	fcMT_FailLoadContent,				//����Ʈ ���� ����

	fcMT_DiskFree,						//���õ� ���丮���� �뷮�� ���������� �׳� �����Ұ��� �ƴ��� 
	fcMT_DiskFull,						//���� ���õ� ���丮���� �뷮�� ��á��
	fcMT_ContentsLimit,					//���̺굥��Ÿ������ 50�� ����

	fcMT_ProfileDelete,					//����̽� ��ġ�� �ٲܶ� ������������ ������� �� ������ ���
	
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