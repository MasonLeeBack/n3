#pragma once
#include "BsObject.h"

class CBsWaterObject : public CBsObject
{
public:
	CBsWaterObject();
	virtual ~CBsWaterObject();

protected:
	virtual int AttachMesh(CBsMesh* pMesh);
	virtual void Render(C3DDevice *pDevice);
	virtual void RenderAlpha(C3DDevice *pDevice);

private:
	BYTE		*m_pWaterCheckBuffer;
public:
	static BOOL	s_bCaptureRefractMap;

public:
	BYTE		*GetWaterCheckBufferPtr() { return m_pWaterCheckBuffer;}

};
