#pragma once
#include "BsObject.h"

class CBsSkyBoxObject : public CBsObject
{
public:
	CBsSkyBoxObject();
	virtual ~CBsSkyBoxObject();
protected:
	float m_fSaveNearZ;
	float m_fSaveFarZ;

public:
	static int	s_nKernelIndex;

	virtual void PreRender(C3DDevice *pDevice);
	virtual void PostRender(C3DDevice *pDevice);
};