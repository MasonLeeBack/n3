#pragma once

#include "BsUiLoader.h"
#include ".\\data\\Menu\\BsUiControlID.h"


#define _MENU_CLASSID_FILE			"BsUiClassID.txt"
#define _MENU_CONTROLID_FILE		"BsUiControlID.h"

class BsUiClassIDManager
{
public:
	BsUiClassIDManager();
	~BsUiClassIDManager();

	struct _CLASSID_INFO
	{
		_CLASSID_INFO()
		{
			hClass = -1;
			nType = BsUiTYPE_NONE;
			nNameSize = 0;
			szName = "";
		}
		BsUiCLASS	hClass;
		BsUiTYPE	nType;
		int			nNameSize;
		char*		szName;
	};

	bool	Save();
	bool	Load();
	void	Release();

	BsUiCLASS	AddInfo(BsUiTYPE nType, const char* szName, BsUiCLASS hParentClass);
	bool		AddInfo(BsUiCLASS hClass, BsUiTYPE nType, const char* szName);
	bool		RemoveInfo(BsUiCLASS hClass);
	
	void		SetName(BsUiCLASS hClass, char* szName);

	int			GetInfoCount()	{ return (int)m_pClassInfo.size(); }
	BsUiCLASS	GetClass(int nIndex);
	BsUiTYPE	GetType(BsUiCLASS hClass);
	char*		GetName(BsUiCLASS hClass);

	bool	SetCtrlId(short nId, char* szName);
	
	void	GenerateNewName( char* szName , const size_t szName_len); //aleksger - safe string
	bool	IsSameName( char* szName );

	bool	IsSameClassID( const BsUiCLASS hClass );

protected:
	void GenerateNewClassID( BsUiCLASS& hClass, BsUiTYPE nType );
	
	std::vector<_CLASSID_INFO>	m_pClassInfo;

public:	

};



extern BsUiClassIDManager	g_ClassIdMgr;