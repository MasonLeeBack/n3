#pragma once

class CBsFXTable;
class BStream;
class CBsFXElement {
public:
	CBsFXElement( CBsFXElement *pParent = NULL );
	virtual ~CBsFXElement();
	enum ELEMENT_TYPE {
		UNKNOWN = -1,
		MESH,
		PARTICLE,
		POINT_LIGHT,

		ELEMENT_TYPE_FORCE_DWORD	= 0xFFFFFFFF,
	};

	enum TABLE_TYPE {
		COLOR_TABLE = 0,
		ALPHA_TABLE,
		SCALE_TABLE,
		POSITION_TABLE,
		ROTATE_TABLE,

		TABLE_TYPE_AMOUNT,
	};


protected:
	bool m_bClone;
	ELEMENT_TYPE m_Type;
	DWORD m_dwStartFrame;
	DWORD m_dwEndFrame;
	BOOL m_bShow;

	int m_nObjectIndex;
	int m_nLoadIndex;

	D3DXMATRIX m_matObject;
	D3DXMATRIX *m_pmatParent;
	CBsFXTable *m_pTable[TABLE_TYPE_AMOUNT];

	int m_nPositionInterpolationType;

	D3DCOLORVALUE m_Color;
	std::string m_szFileName;

public:
	ELEMENT_TYPE GetType();
	CBsFXTable *GetTable( TABLE_TYPE Type );
	void SetParentMatrix( D3DXMATRIX *pmat );

	virtual BOOL Activate();
	virtual void Process( DWORD dwTime, float fDistanceFromCamera, int count = 0 );

	DWORD GetStartFrame();
	DWORD GetEndFrame();

	int GetObjectIndex();

	int GetPositionInterpolationType();

	virtual BOOL Load( BStream *pStream );
	virtual void Show();
	virtual void Hide();
	virtual void SetCurFrame( DWORD dwValue ) {}
	virtual DWORD GetCurFrame() { return 0; }
	virtual void DeleteEngineObject() {}

	BOOL IsShow();

	D3DXMATRIX *GetMatrix();
};

void ReadString( std::string *szStr, BStream *pStream );