#pragma once

#include "BsObject.h"


class CBsFXElement;
class CBsFXTemplate;
class BStream;
class CBsFXObject : public CBsObject
{
public:
	CBsFXObject();
	virtual ~CBsFXObject();

	// BSObject Methods
//	virtual void PreRender(C3DDevice *pDevice);
	virtual bool InitRender( float fDistanceFromCamera );
//	virtual void Render( C3DDevice *pDevice );
//	virtual void RenderAlpha( C3DDevice *pDevice );
	virtual void Process();

	virtual int ProcessMessage(DWORD dwCode, DWORD dwParam1=0, DWORD dwParam2=0, DWORD dwParam3=0);

	virtual void Show( bool bIsShow );
	virtual bool GetBox3( Box3 & B );
	bool GetShow() { return m_bShow; }
	enum STATE {
		PLAY,	// p1 : Loop Count,
		STOP,
		PAUSE,
		RESUME,
		STOPLOOP,
	};

	static DWORD m_dwProcessTick;

#ifndef _LTCG
	CBsFXTemplate * GetFXTemplate();
#endif

protected:
	struct ElementStruct {
		int nObjectIndex;
		D3DXMATRIX matObject;
	};

#ifndef _LTCG
	CBsFXTemplate * m_pFXTemplate; // this is here for debugging purposes only
#endif

	std::vector<CBsFXElement *> m_pVecElement;

	STATE m_State;
	DWORD m_dwTotalFrame;
	DWORD m_dwCurFrame;
	DWORD m_dwParticleCurFrame;
	DWORD m_dwPauseFrame;

	DWORD m_dwOldTick;
	int m_nLoopCount;

	int m_nTotalLoopCount;

//	float m_fDistanceFromCamera; // mruete: prefix bug 320: renamed to m_fDistanceFromCamera

	DWORD GetProcessTick();	// 나중에 어떤 프레임을 쓸지 몰라서 함수로 빼논다.
	
public:
	// Create
	void Initialize( CBsFXTemplate *pLoadObj );

	// Base Control Func
	void SetState( STATE State, int nParam1 = 0, int nParam2 = 0 );
	STATE GetCurState();

	// Etc
	void SetCurFrame( DWORD dwFrame );
	DWORD GetCurFrame();
	DWORD GetTotalFrame();

	DWORD GetElementCount();
	CBsFXElement *GetElementFromIndex( DWORD dwIndex );

	std::vector<CBsFXElement *> *GetElementList();	

	virtual void Release();
};
