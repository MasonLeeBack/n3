#ifndef __FC_INTERFACEDATA_DEFINE_H__
#define __FC_INTERFACEDATA_DEFINE_H__

#include "BsUiUVManager.h"
#include ".\\data\\interface\\Portrait\\PortraitUVDef.h"

class CFcUVList
{
public:
	CFcUVList();
	virtual ~CFcUVList();
	void LoadUVTable(char *szFileName);
	
	UVImage *GetUVInfo(int nId);
	UVImage *GetUVInfoforTex(int nId);
	int		GetUVCount()				{ return (int)m_UVList.size(); }

protected:	
	std::vector<UVImage *>m_UVList;
};


class CFcPortrait : public CFcUVList
{
public:

	CFcPortrait();
	~CFcPortrait();

	void Load();
	void ReleaseTexture();
	
	//아래 좌표를 기준으로(Left,Bottom좌표)
	void DrawPortrait(int nId, int iSX, int iSY, int iEX, int iEY, float fAlpha);
	void DrawPortrait(int nId, int PosX, int PosY, float fAlpha);
};

#endif