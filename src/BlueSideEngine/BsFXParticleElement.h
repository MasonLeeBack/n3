#pragma once

#include "BsFXElement.h"
class CBsFXParticleElement : public CBsFXElement
{
public:
	CBsFXParticleElement( CBsFXElement *pParent = NULL );
	virtual ~CBsFXParticleElement();

protected:
	BOOL m_bIterator;
	BOOL m_bLoop;
	BOOL m_bStopFlag;

public:
	virtual BOOL Activate();

	virtual BOOL Load( BStream *pStream );
	virtual void Process( DWORD dwTime, float fDistanceFromCamera, int count = 0 );

	virtual void SetCurFrame( DWORD dwValue );
	virtual DWORD GetCurFrame();

	void	Deactivate();
};