#pragma once

enum 
{ 
	EFFECTPARAM_MAX		= 3,
	eCaptionLength		=32,
	eFileNameLength		=64,
	eKeyStringLength	=130,

	eFps40				= 40,
	eFps60				= 60,
	ePublicFps			= 60,	// ���߿� 40���� �ٲܰ�.

	eOldRtmFileVersion	= 100,
	eNewRtmFileVersion	= 101,
};

#define RM_FPS				40.f