#pragma once
class IBsClipTest
{
public:
	virtual HRESULT	Initialize() = 0;
	virtual void	Destroy()			= 0;
	virtual void	Update()			= 0;
	virtual void	Render()			= 0;
	virtual bool	IsReady()			= 0;
	virtual void	AddBoundingBox( const FLOAT		fCx,
									const FLOAT		fCy,
									const FLOAT		fCz,
									const FLOAT		fRotateY,
									const FLOAT		fX,
									const FLOAT		fY,
									const FLOAT		fZ )	= 0;
	virtual	void	SetClipUpdate( bool bUpdate )	= 0;
	virtual	bool	GetClipUpdate()		= 0;

public:
	static IBsClipTest& GetInstance();
};

#define g_BsClipTest IBsClipTest::GetInstance()
