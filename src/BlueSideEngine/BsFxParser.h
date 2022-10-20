#pragma once

//------------------------------------------------------------------------------------------------
				 
class BsFxParser // FX 파일을 파싱하여 해당 정보를 관리합니다 (툴에서 사용)
{
protected:
	enum { TEXTURE_TYPE_NAME_MAX=5, };

	char m_szTextureTypeName[256][TEXTURE_TYPE_NAME_MAX];
	int  m_nTextureTypeName;

public: 
	void Parsing(char *szFxFileName);
};

//------------------------------------------------------------------------------------------------




