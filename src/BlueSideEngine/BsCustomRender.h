#pragma once

class C3DDevice;

class CBsCustomRender
{
public:
	CBsCustomRender();
	virtual ~CBsCustomRender();
public:
	virtual void Render(C3DDevice* pDevice) = 0;
};