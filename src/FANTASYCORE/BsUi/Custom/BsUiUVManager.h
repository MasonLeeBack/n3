#pragma once

//#include "BsKernel.h"

#define _MENU_INTERFACE_UVLIST_FILE		_T("BsUiUVList.txt")
#define _MENU_INTERFACE_UVID_FILE		_T("BsUiUVID.h")

typedef int		BsUiUVId;
#define _UNIT_OF_TEXTURE				1000		// 0 - 999

struct UVImage{
	UVImage()
	{
		nId = -1;
		u1 = v1 = u2 = v2 = -1;
		nTexId = -1;
	};

	BsUiUVId nId;
	char*	szName;
	int u1,v1;
	int u2,v2;
	int nTexId;
};

typedef std::vector<UVImage>		BsUiUVLIST;

enum EditUV_STATE{
	_EditUV_SEL,
	_EditUV_AUTO_ADD,
	_EditUV_AUTO_HAND,
};

class BsUiEditUVManager
{
public:
	BsUiEditUVManager();
	~BsUiEditUVManager();

	struct TextureUVList
	{
		TextureUVList()
		{
			nTextureId = -1;
			szTexFileName = NULL;
			szTexName = NULL;
			nTexUnit = -1;
			bLocal = false;
		};

		int			nTextureId;
		char*		szTexFileName;		//filename
		char*		szTexName;			//define
		int			nTexUnit;
		bool		bLocal;

		BsUiUVLIST	UVList;
	};

	bool		Load();
	bool		Save();
	bool		New(const char* szImgFileName);

	void		Release();

	int			AddTexture(char* szTextureFileName, bool bLocal = false);
	bool		RemoveTexture(int nTextureId);

	BsUiUVId	AddInfo(int u1, int v1, int u2, int v2);
	bool		RemoveInfo(BsUiUVId nUVId);
	bool		SetInfo(BsUiUVId nUVId, int u1, int v1, int u2, int v2);
	bool		SetName(BsUiUVId nUVId, char* szName);

	int			GetTextureCount()					{ return (int)m_UiTexUVList.size(); }
	int			GetTextureIdforIndex(int nIndex);

	UVImage*	GetUVImage(BsUiUVId nUVId);
	BsUiUVLIST*	GetUVList(int nTextureId);

	int			GenerateNewTextureUnit();
	bool		IsSameTextureUnit(int nUnit);

	BsUiUVId	GenerateNewUVId();
	bool		IsSameId(BsUiUVId nUVId);

	void		GenerateNewName(char* szName, const size_t szName_len, BsUiUVId nId); //aleksger - safe string
	bool		IsSameName(char* szName);

	int			GetCurTextureId()						{ return m_nCurTextureId; }
	bool		SetCurTextureId(int nTextureId);
	SIZE		GetCurTextureSize();

	BsUiUVId	GetCurUVId()								{ return m_nCurUVId; }
	BsUiUVId	GetUVId(POINT point);
	BsUiUVId	GetUVId(char* szName);
	bool		SetCurUVId(BsUiUVId nUVId);

	bool		IsSelectedBox()								{ return m_bSelectedBox; }
	RECT		GetSelectedBox()							{ return m_rectSelected; }
	void		SetSelectedBox(bool bFlag, POINT* point);
	void		MoveSelectedBox(POINT* point);


	EditUV_STATE	GetEditUVState()						{ return m_EditUVState; }
	void			SetEditUVState(EditUV_STATE state)		{ m_EditUVState = state; }
	BsUiUVId		SetAddUV();

	bool		IsShowBlock()				{ return m_bShowBlock; }
	void		SetShowBlock(bool bSet)		{ m_bShowBlock = bSet; }
	bool		IsShowID()					{ return m_bShowID; }
	void		SetShowID(bool bSet)		{ m_bShowID = bSet; }

	void		GenerateNewTexUnitName(char* szName, const size_t szName_len, int nTexUnitID); //aleksger - safe string
	bool		IsSameTexUnitName(char* szName);

	void		SetTextureUnitName(int nTextureID, char* szName);
	char*		GetTextureUnitName(int nTextureID);
	int			GetTextureUnitId(int nTextureID);
	bool		GetTextureLocal(int nTextureID);


protected:
	bool	Load(const char* szFileName);
	bool	SaveforUVList(const char* szFileName);
	bool	SaveforUVID(const char* szFileName);
	bool	SetAddUVforAuto();
	void	ChangeTexFileNameforLocal(char* szFileName);
	void	GetFileNameforLocal(char* pSrcFileName);
	void	GetImageFileName(char* pSrcFileName, char* pDscFileName, size_t size, bool bLocal); //aleksger - safe string

protected:
	std::vector<TextureUVList*>	m_UiTexUVList;

	int							m_nCount;
	BsUiUVId					m_nCurUVId;
	int							m_nCurTextureId;

	bool					m_bSelectedBox;
	RECT					m_rectSelected;

	EditUV_STATE			m_EditUVState;
	RECT					m_rectAddUV;

	bool					m_bShowBlock;
	bool					m_bShowID;
};


class BsUiUVManager
{
public:
	BsUiUVManager();
	~BsUiUVManager();

	bool	Load(int nTexUnitID = -1);
	bool	Load(const char* szName, int nTexUnitID);
	bool	Remove(int nTexUnitID);
	void	Release();

	void	UpdateUVArray();		//load후에 반드시 호출 해야 한다.
	void	ReleaseUVArray();

	UVImage*	GetUVImage(BsUiUVId nUVId);

protected:
	void	RemoveUVList(int nTextureID);
	void	ChangeTexFileNameforLocal(char* szFileName);
	void	GetFileNameforLocal(char* pSrcFileName);
	void	GetImageFileName(char* pSrcFileName, char* pDscFileName, size_t size,  bool bLocal); //aleksger - safe string

protected:
	struct TextureUVList
	{
		int			nTextureId;
		int			nTexUnit;
	};

	BsUiUVLIST					m_UiUVList;
	std::vector<TextureUVList>	m_TextureList;

	int							(*m_UVArray)[_UNIT_OF_TEXTURE];
};

extern BsUiUVManager		g_UVMgr;
extern BsUiEditUVManager	g_EditUVMgr;
