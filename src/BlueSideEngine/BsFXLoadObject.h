#pragma once

class CBsFXObject;
class CBsFXElement;
class BStream;

class CBsFXTemplate {

public:
	CBsFXTemplate();
	virtual ~CBsFXTemplate();

	// Load Func
	BOOL Load( const char *szFileName );
	BOOL Load( BStream *pStream );

	void AddRef();
	int  Release();
	int  GetRefCount();

	const char *GetFileName();

	struct HEADER {
		char szStr[24];
		float fVersion;
	};

	static float m_fCurLoadVersion;

	int m_nLoadSize;

protected:
	friend CBsFXObject;

	std::string m_szFileName;
	DWORD m_dwTotalFrame;
	std::vector<CBsFXElement *> m_pVecElement;

	int m_nRefCount;

};