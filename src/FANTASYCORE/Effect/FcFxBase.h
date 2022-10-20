#ifndef _FCFXBASE_H_
#define _FCFXBASE_H_

#include "FcFXCommon.h"
#include "CDefinition.h"
class C3DDevice;


struct TimeValue
{
public:
	DWORD nFrame;
	float fFactor;

	TimeValue()
	{
		nFrame = 0;
		fFactor = 0.f;
	}
	TimeValue(DWORD t_nFrame, float t_fFactor)
	{
		nFrame = t_nFrame;
		fFactor = t_fFactor;
	}
};


class TimeValueTable
{
protected:
	int			m_nTable;
	TimeValue*	m_pTable;

public:
	float GetVaule(DWORD nTime);

	TimeValueTable()
	{
		m_nTable = 0;
		m_pTable = NULL;
	}


	virtual ~TimeValueTable()
	{
		if(m_pTable)
			delete[] m_pTable, m_pTable;
		m_nTable = 0;
	}
};


class CFcFXBase
{
public:

	virtual int			ProcessMessage(int nCode,DWORD dwParam1=0, DWORD dwParam2=0,DWORD dwParam3=0);
	virtual void		InitDeviceData() {};
	virtual void		ReleaseDeviceData() {};
	virtual void		Process()= 0;
	virtual void		Render(C3DDevice *pDevice) {};
	virtual void		PreRender();
	virtual void		Update() {};
	virtual void		RenderZero(C3DDevice *pDevice) {}; // ......
	static	void		Tick() {CFcFXBase::s_dwFxTimer++;};

	bool				IsFinished();
	void				Initialize()	{m_DeviceDataState = FXDS_INITREADY; };
	int					GetPriority()	{ return m_nPriority;}; 
	DWORD				GetTick()		{ return  CFcFXBase::s_dwFxTimer;};
	FX_STATE			GetState()		{return m_state;};
	FX_DEVICEDATASTATE  GetDState()		{return m_DeviceDataState;};

#ifndef _LTCG
	PUBLIC_GETSET_PROP(int, FxRtti, m_nRtti);
#endif //_LTCG




protected:
	
	int					m_nPriority; 
	static DWORD		s_dwFxTimer;
	FX_STATE			m_state;
	FX_DEVICEDATASTATE	m_DeviceDataState;

	


public:

	CFcFXBase();
	virtual~CFcFXBase();
};




#endif