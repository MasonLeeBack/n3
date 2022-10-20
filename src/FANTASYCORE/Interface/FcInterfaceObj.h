#ifndef __FC_INTERFACE_OBJ_H__
#define __FC_INTERFACE_OBJ_H__
#include "FcInterfaceDataDefine.h"

class CFcInterfaceObj
{
public:
	CFcInterfaceObj();
	virtual ~CFcInterfaceObj(){};
	
	virtual void Process(){};
	virtual void Update(){};
	virtual void ReleaseData() {};
	virtual void Clear() {};

	void SetHandle(int* Handle)			{ m_Handle = Handle; }
	int* GetHandle()					{ return m_Handle; }
	virtual void KeyInput(int nPlayerID, int KeyType,int PressTick){};		// PlayerID = 0, 1
	
	void Show(bool bShow)				{ m_bShow = bShow; }
	bool IsShow()						{ return m_bShow; }

	

protected:
	int*			m_Handle;
	bool			m_bShow;

};
#endif