#pragma once

class BsUiImageManager
{
public:
	BsUiImageManager();
	~BsUiImageManager();

	bool	Load(const char* szName) {;}
	void	Release() {;}
};

extern BsUiImageManager		g_ImageMgr;