#pragma once

//------------------------------------------------------------------------------------------------
				 
class BsFxParser // FX ������ �Ľ��Ͽ� �ش� ������ �����մϴ� (������ ���)
{
protected:
	enum { TEXTURE_TYPE_NAME_MAX=5, };

	char m_szTextureTypeName[256][TEXTURE_TYPE_NAME_MAX];
	int  m_nTextureTypeName;

public: 
	void Parsing(char *szFxFileName);
};

//------------------------------------------------------------------------------------------------




