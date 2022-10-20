#pragma once

#include "BsFXElement.h"
class CBsFXPLElement : public CBsFXElement
{
public:
	CBsFXPLElement( CBsFXElement *pParent = NULL );
	virtual ~CBsFXPLElement();

protected:
	int m_nAttenuation;

public:
	virtual BOOL Activate();

	virtual BOOL Load( BStream *pStream );
	virtual void Process( DWORD dwTime, float fDistanceFromCamera, int count = 0 );

	virtual void SetCurFrame( DWORD dwValue );
	virtual DWORD GetCurFrame();

	virtual void DeleteEngineObject();

};