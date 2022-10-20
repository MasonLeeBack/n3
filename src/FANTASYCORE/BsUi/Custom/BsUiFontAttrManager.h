#pragma once

#include "BsUiFont.h"

#define _MENU_FONT_ATTR_FILE		"%s\\BsUiFontAttr_%s.txt"
#define _MENU_FONT_ATTR_ID_FILE		"BsUiFontAttrID.h"

#define _TEXT_NEW_FONT_ATTR_NANE	"FA_New"
#define _TEXT_DEFAULT_ATTR_NAME		"FA_DEFAULT"
#define _TEXT_DEFAULT_ATTR			"@(reset)"


typedef int				BsUiAttrId;			// font attribute index
struct BsUiFontAttr
{
	BsUiFontAttr()
	{
		nAttrId = -1;
		szName = "";
		szAttr = "";
		bEnableEdit = true;
		psAttr = NULL;
	}
	BsUiAttrId	nAttrId;
	char*		szName;
	char*		szAttr;
	bool		bEnableEdit;
	SFontAttribute*	psAttr;
};

typedef std::vector<BsUiFontAttr>	BsUiFontAttrLIST;

class BsUiFontAttrManager
{
public:
	BsUiFontAttrManager();
	~BsUiFontAttrManager();

	bool	Save();
	bool	Load();
	void	Release();
    
	BsUiAttrId	AddInfo(const char* szName, char* szAttr);
	bool		RemoveInfo(BsUiAttrId nAttrId);
	
	void		SetName(BsUiAttrId nAttrId, char* szName);
	void		SetAttr(BsUiAttrId nAttrId, char* szAttr);
	void		SetAttr(BsUiAttrId nAttrId, SFontAttribute* psAttr);

	int				GetInfoCount();
	BsUiAttrId		GetAttrId(char* szName);
	char*			GetName(BsUiAttrId nAttrId);
	char*			GetAttrText(BsUiAttrId nAttrId);
	BsUiFontAttr*	GetAttrStruct(BsUiAttrId nAttrId);
	
	BsUiAttrId		GetAttrIdforIndex(int nIndex);
	char*			GetNameforIndex(int nIndex);
	char*			GetAttrforIndex(int nIndex);
	bool			GetEnableEdit(int nIndex);


	bool		IsSameName( char* szName );
	bool		IsSameAttr( char* szAttr );
	bool		IsSameAttr( BsUiAttrId nAttrId, char* szAttr );

	//추가 삭제는 eng일때만 가능하고 그외 local은 수정만 가능하다.
	void		AddInfo(BsUiAttrId nAttrId, char* szName, char* szAttr);
	bool		IsSameIndex( BsUiAttrId nAttrId );

	int 		GetCurAttrId()					{ return m_nCurIndex; }
	void		SetCurAttrId(int nIndex)		{ m_nCurIndex = nIndex; }

protected:
	bool		Load(char* szFullName, BsUiFontAttrLIST* pFontAttrInfo);
	bool		SaveforFontAttr(char* szFullName, BsUiFontAttrLIST* pFontAttrList);
	bool		SaveforFontDef(BsUiFontAttrLIST* pFontAttrList);
	void		GenerateNewName( char* szName, const size_t szName_len); //aleksger - safe string
	BsUiAttrId	GenerateNewIndex();
	
	//local마다 속성치는 다를수 있으므로 분류가 되는 index와 이름으로 bool값을 얻는다.
	bool		IsSameAttrforStr( BsUiFontAttrLIST* pFontAttrInfo, BsUiFontAttr* pstrFontAttr);
	
protected:
	BsUiFontAttrLIST*	m_FontAttrInfoList;			//BsUITool용
	int					m_nCurIndex;

public:	

};

extern BsUiFontAttrManager	g_FontAttrMgr;