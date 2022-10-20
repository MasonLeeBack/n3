#ifndef __BS_TRIGGER_ACTION_H__
#define __BS_TRIGGER_ACTION_H__
#include "BSTriggerStrDefinition.h"
#include "BSTriggerCondition.h"
#include "Token.h"

class CBSTriggerParam;
class BStream;


class CBSActionForTool : public CBSConditionActionForToolBase
{
public:
	int FillToBuffer(char *pBuf,int iStartPos = 0); //리턴 값 : 버퍼로 채워지고 난 후의 최종 위치
	void FillToAction(char *pBuf);
	void LoadAction(BStream *fp,int nVer);
	void LoadActionToText(TokenList::iterator &itr);
	void SaveAction(BFileStream *fp,int nVer);	
	void SaveActionToText(FILE *fp);
};

void memcpyAction(CBSActionForTool *pDest,CBSActionForTool *pSrc);
#endif