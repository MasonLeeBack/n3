#pragma once

#define _FONT_TYPE_COUNT	2
#define _INVALID_CODE		-2

enum	{
	_FAP_KEYWORD_MAX		= 20,
	_FAP_KEYWORD_LENGTH	= 30,
	_FAP_PARAM_LENGTH		= 20
};

enum FONT_TYPE {
	FT_Default = 0,
	FT_Addition,

	FT_MAX,
};

enum FONT_STYLE {
	FS_Regular,
	FS_Italic,

	FS_MAX,
};

enum eAllign {
	eAlignLeft,
	eAlignCenter,
	eAlignRight,

	eAlign_Max,
};

struct SFontAttribute {
	void Set() {
		nType = FT_Default;
		Color = D3DXCOLOR(1,1,1,1);
		fScaleX = fScaleY = 1.f;			// 작동
		nStyle = FS_Regular;
		nAlign = eAlignLeft;
		nSpaceX = nSpaceY = 0;
		bWrap = true;
		bTruncate = bIgnore = bReset = false;

		nClippingWidth = -1;      
		nClippingHeight = -1;
	};

	FONT_TYPE	nType;				// local에 기본 font 2벌
	D3DXCOLOR	Color;
	float		fScaleX, fScaleY;
	FONT_STYLE	nStyle;				// regular / italic
	eAllign		nAlign;
	int			nSpaceX, nSpaceY;	// char와 char의 차이 값 / line과 line의 차이 값
	bool		bWrap;				// auto new line
	bool		bTruncate;			// 끝점 이상 나가면 '...'
	bool		bIgnore;			// 이후 속성 무시
	bool		bReset;				// 이전 속성 무시

	int       nClippingWidth; // wrap 되는 클리핑 영역 입니다 (-1 일때는 적용 되지 않습니다)
	int       nClippingHeight;
};

typedef struct _FONT
{
	_FONT()
	{
		nTexID = -1;
		rectUV = NULL;
		nCount = 0;
		nSpace = 0;
	};

	int			nTexID;
	RECT*		rectUV; // font uv 를 저장 합니다 (오른쪽과 하단이 1 dot 씩 크게 설정 합니다, 메모리 관리 됩니다)
	int			nCount; // 갯수 입니다
	int			nSpace; // 스페이스 캐릭터인 경우에 건너 띄는 폭 길이 입니다
} FONT;

typedef	struct _SPECIAL_CHAR_KEY
{
	_SPECIAL_CHAR_KEY()
	{
		szKeyword = NULL;

		nCode = -1;
		nSpeed = 0;
		nTotalFrame = -1;

		nCurFrame = 0;
	};

	char*	szKeyword;

	int		nCode;
	int		nSpeed;			// tick;
	int		nTotalFrame;	// animation frame count

	int		nCurFrame;
} SPECIAL_CHAR_KEY;

typedef std::vector<SPECIAL_CHAR_KEY*>	scKEYLIST;
typedef struct _SPECIAL_FONT
{
	FONT			Font;
	scKEYLIST		KeyWord;
} SPECIAL_FONT;

class BsUiFont
{
public:
	BsUiFont();
	~BsUiFont();

	void	Release();
	void	LoadLocalFontTable();

	void	CreateFontText(int sx, int sy, int ex, int ey, char *szText, float fAlpha = 1.f);
	void	DrawUIText(int sx, int sy, int ex, int ey, char *szText, float fAlpha = 1.f)
	{
		CreateFontText(sx, sy, ex, ey, szText, fAlpha);
	}
	void	GetTextLengthInfo(int &nWidth, int &nHeight, int &nLine, int &nWordCount, int sx, int sy, int ex, int ey, char *szText);
	void	GetFontAttrStruct(SFontAttribute *pAttribute, char *szText);
	void	GetFontAttrText(char *szText, const size_t szText_len, SFontAttribute *pAttribute); //aleksger - safe string

	void	PreProcessAniSpecialFont(int nTick);

protected:
	static	int		m_nFontAttrKeyword;
	static	char	m_FontAttrKeyword[_FAP_KEYWORD_MAX][_FAP_KEYWORD_LENGTH];

	FONT*			m_pFont[_FONT_TYPE_COUNT];
	FONT*			m_pLocalFont[_FONT_TYPE_COUNT];
	SPECIAL_FONT*	m_pSpecialFont;							// 특수 문자.

	SFontAttribute	m_FontAttr;
	int				m_nSpaceWidth; // 스페이스 캐릭터인 경우에 건너 띄는 폭 길이 입니다
	int				m_nFontLocal;

	typedef struct _TOKEN_INFO
	{
		_TOKEN_INFO()
		{
			nCode = _INVALID_CODE;
			bSpecial = false;
			nLine = 0;
			nType = 0;
			nLocal = -1;
			nStype = FS_MAX;
			Color = 0xffffffff;
		};

		int			nCode;
		bool		bSpecial;
		int			nLine;
		int 		nType;
		int 		nLocal;
		int			nStype;
		D3DXCOLOR	Color;
		RECT		rect;
		RECT		uvRect;
	} TOKEN_INFO;

	std::vector<TOKEN_INFO>	m_F_TokenInfoList;		//font token info list
	std::vector<TOKEN_INFO>	m_L_TokenInfoList;		//local font token info list
	std::vector<TOKEN_INFO>	m_S_TokenInfoList;		//special font token info list
	std::vector<int> m_LineWidthList;

	int		m_nAddParsing;

	float	m_fParsingBuf[4];	// float
	int		m_nParsingBuf[4];	// value

	float	m_fAlpha;			// 속성과 상관없이 외부에서 적용되는 alpha - ctrl의 alpha

protected:
	void	LoadSpecialFontTable(char *szTexName, char *szUVName);

	void	ReleaseFont();
	void	ReleaseLocalFont();
	void	ReleaseSpecialFont();

	void	LoadFontTable(int nLocal, int nType, int nSpaceWidth=8);
	bool	LoadSpecialFontTable();

	// 폰트 uv 좌표를 읽어 옵니다
	void	LoadUV(FONT* pFont, char *szFontUVName, int nSpaceWidth=8);
	void	ProcessFontText(int sx, int sy, int ex, int ey, char *szText, int &nWidth, int &nHeight, int &nWordCount, std::vector<int>* pLineWidthList);
	void	CreateFontElement(int sx, int ex, std::vector<TOKEN_INFO>* pTokenInfoList, DWORD dwFont);

	bool	GetTextInt  (char *szText, int &nOffset, int nLength, int nNum); // nNum 만큼 값을 읽습니다 - integer
	bool	GetTextFloat(char *szText, int &nOffset, int nLength, int nNum); // nNum 만큼 값을 읽습니다 - float

	bool	IsAlphabet(char ch);
	bool	IsUniCode(char ch);
	bool	IsChar(char ch);
	bool	IsFontAttr(char **szText, SFontAttribute* pFontAttr);
	bool	IsSpecialFont(char **szText, int &nCode);
	bool	IsTab(char **szText);
	bool	IsNewLine(char **szText);
	bool	IsIgnoreFontAttr()							{ return m_FontAttr.bIgnore; }

	int		GetCode(char* pszTemp, int &nLocal);
	int		GetWordWidth(char *pStr, int nType);
	inline int GetLetterWidth(int nCode, int nLocal, int nType);
	int		GetFontLocal()								{ return m_nFontLocal; }

	void	SetIgnoreFontAttr(bool bFlag)				{ m_FontAttr.bIgnore = bFlag; }
	void	SetFontLocal(int nLocal)					{ m_nFontLocal = nLocal; }

	bool	GetFontUV(int nCode, int nLocal, FONT* pFont, RECT& uvRect);
	int		GetJapanFontIndex(int nUniCode);
};

extern BsUiFont* g_pFont;