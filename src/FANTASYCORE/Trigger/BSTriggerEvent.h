#ifndef __BS_TRIGGER_EVENT_H__
#define __BS_TRIGGER_EVENT_H__

#include "BSTriggerCondition.h"
#include "BSTriggerAction.h"
#include "BSTrigger.h"
#include "Token.h"

#define MAX_EVENT_CODE_NAME	128
#define MAX_EVENT_CAPTION	512

class CBSEventForTool : public CBSTriggerEventBase
{
public:
	CBSEventForTool();
	void SetHandleIndex(int iIndex){m_iEventHandle = iIndex;}
	char				*GetCaption();
	void SaveEvent(BFileStream *fp,int nVer);
	void SaveEventToText(FILE *fp);
	void LoadEvent(BStream *fp,int nVer);
	void LoadEventToText(TokenList::iterator &itr);

protected:
	int m_iEventHandle;
	//char m_Reserve[MAX_EVENT_CAPTION];
	int m_iReservedData1;
};
#endif