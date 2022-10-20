#pragma once

class C3DDevice;

class CBsWorld
{
public:
	CBsWorld();
	virtual ~CBsWorld();

protected:
public:
	virtual void Render(C3DDevice *pDevice)=0;
};