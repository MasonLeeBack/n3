#include "stdafx.h"
#include "BsUiFont.h"
#include "LocalLanguage.h"
#include "BsKernel.h"
#include "BsUIBase.h"
#include "Parser.h"
#include "bstreamext.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG


BsUiFont* g_pFont = NULL;


#define _ITALIC_MOVE			10
#define _ASCII_BASE_NUM			'!'		//33
#define _SPACE_CODE				-1
#define _FONTATTR_PARAM_CODE	'@'

#define _TAB_WIDTH				46
#define _CHAR_HEIGHT			38
#define	_DEFAULT_SPACE			5.0f
#define	_DEFAULT_TAB			45.0f

#define	_ALPHABET_MARGIN		-2.0f
#define	_Y_MARGIN				14

#define _HANGUL_FONT_SIZE		36
#define	_HANGUL_FONT_SIZE_MIN	0.8f
#define	_HANGUL_MARGIN			-1.0f
#define _HANGUL_X_NUM			56
#define _HANGUL_Y_NUM			42
#define	_HANGUL_FONT_Y_GAP		0//4
#define _HANGUL_NUM				2350

#define _JAPAN_FONT_SIZE		36//22
#define	_JAPAN_FONT_SIZE_MIN	0.7f
#define	_JAPAN_MARGIN			0.0f
#define _JAPANESE_X_NUM			16
#define _JAPANESE_Y_NUM			93
#define _JAPANESE_NUM			3936//7392

enum eFontAttrParam
{
	_FAP_ID_TYPE		= 0,
	_FAP_ID_COLOR,
	_FAP_ID_SCALE,
	_FAP_ID_STYLE,
	_FAP_ID_ALIGN,
	_FAP_ID_SPACE,
	_FAP_ID_LINE_SPACE,
	_FAP_ID_WRAP,
	_FAP_ID_TRUNCATE,
	_FAP_ID_IGNORE,
	_FAP_ID_RESET,

	_FAP_ID_MAX,
};


int		BsUiFont :: m_nFontAttrKeyword = _FAP_ID_MAX;
char	BsUiFont :: m_FontAttrKeyword[_FAP_KEYWORD_MAX][_FAP_KEYWORD_LENGTH]= {
	"(type=",			// default/decoration
	"(color=",			// r,g,b,a
	"(scale=",			// x,y
	"(style=",			// 0/1 : regular/italic
	"(align=",			// 0/1/2 : left/center/right
	"(space=",			// 글자 간격
	"(linespace=",		// 줄 간격
	"(wrap=",			// 자동 new line
	"(truncate=",		// 축약 '...'
	"(ignore)",			// 이하 font 속성은 무시한다.
	"(reset)",			// font 속성을 초기화 한다.
};

static inline void TransfloatToChar(char* szValue, const size_t szValue_len, float fValue)
{	
#ifdef _XBOX
	sprintf_s(szValue, szValue_len, "%f", fValue);//aleksger - safe string
#else
	sprintf(szValue, "%f", fValue);//aleksger - safe string
#endif
	for(int i=(int)strlen(szValue)-1; i>=0; i--)
	{
		char token = szValue[i];
		if(token != '0')
		{
			if(token == '.')
			{
				szValue[i+1] = '0';
				szValue[i+2] = '\0';
			}
			else
			{
				szValue[i+1] = '\0';
			}
			break;
		}
	}
}

BsUiFont::BsUiFont() 
{
	m_FontAttr.Set();
	m_nFontLocal = LANGUAGE_ENG;

	for(int i=0; i<_FONT_TYPE_COUNT; i++)
	{
		m_pFont[i] = new FONT();
		m_pLocalFont[i] = NULL;
	}
	LoadFontTable(LANGUAGE_ENG, 0);

	LoadLocalFontTable();

	m_pSpecialFont = new SPECIAL_FONT();
	LoadSpecialFontTable();
}

BsUiFont::~BsUiFont()
{
	Release();
}

void BsUiFont::Release()
{ 
	ReleaseFont();
	ReleaseLocalFont();
	ReleaseSpecialFont();

	m_F_TokenInfoList.clear();
	m_L_TokenInfoList.clear();
	m_S_TokenInfoList.clear();

	m_LineWidthList.clear();
}

void BsUiFont::ReleaseFont()
{
	for(int i=0; i<_FONT_TYPE_COUNT; i++)
	{
		if(m_pFont[i] != NULL)
		{
			SAFE_RELEASE_TEXTURE(m_pFont[i]->nTexID);

			if (m_pFont[i]->rectUV)
			{
				delete [] m_pFont[i]->rectUV;
				m_pFont[i]->rectUV = NULL;
			}

			delete m_pFont[i];
			m_pFont[i] = NULL;
		}
	}
}

void BsUiFont::ReleaseLocalFont()
{
	for(int i=0; i<_FONT_TYPE_COUNT; i++)
	{
		if(m_pLocalFont[i] != NULL)
		{
			SAFE_RELEASE_TEXTURE(m_pLocalFont[i]->nTexID);

			if (m_pLocalFont[i]->rectUV)
			{
				delete [] m_pLocalFont[i]->rectUV;
				m_pLocalFont[i]->rectUV = NULL;
			}

			delete m_pLocalFont[i];
			m_pLocalFont[i] = NULL;
		}
	}
}


void BsUiFont::ReleaseSpecialFont()
{
	if(m_pSpecialFont == NULL)
		return;
	
	FONT* pFont = &m_pSpecialFont->Font;
	SAFE_RELEASE_TEXTURE(pFont->nTexID);

	if (pFont->rectUV)
	{
		delete [] pFont->rectUV;
		pFont->rectUV = NULL;
	}

	scKEYLIST* pKeyList = &(m_pSpecialFont->KeyWord);
	for(DWORD i=0; i<pKeyList->size(); i++)
	{
		SPECIAL_CHAR_KEY* pKey = (*pKeyList)[i];
		if(pKey->szKeyword != NULL)
		{	
			delete [] pKey->szKeyword;
			pKey->szKeyword = NULL;
		}

		delete pKey;
	}

	delete m_pSpecialFont;
	m_pSpecialFont = NULL;
} 


//------------------------------------------------------------------------------------------------------------------------
void BsUiFont::LoadLocalFontTable()
{
	int nCurLocal = (int)(g_LocalLanguage.GetLanguage());
	if(nCurLocal == m_nFontLocal || nCurLocal == LANGUAGE_ENG)
	{
		return;
	}
	m_nFontLocal = nCurLocal;

	ReleaseLocalFont();

	for(int i=0; i<_FONT_TYPE_COUNT; i++)
	{	
		m_pLocalFont[i] = new FONT();
	}
	
	LoadFontTable(m_nFontLocal, 0);
}

void BsUiFont::PreProcessAniSpecialFont(int nTick)
{
#ifndef _BSUI_TOOL
	scKEYLIST* pKeyList = &(m_pSpecialFont->KeyWord);
	for(DWORD dwKey=0; dwKey<pKeyList->size(); dwKey++)
	{
		SPECIAL_CHAR_KEY* pKey = (*pKeyList)[dwKey];
		if (pKey->nTotalFrame > 0 && (nTick % pKey->nSpeed) == 0)
		{
			pKey->nCurFrame = (pKey->nCurFrame + 1) % pKey->nTotalFrame;
		}
	}
#endif
}

void BsUiFont::LoadFontTable(int nLocal, int nType, int nSpaceWidth)
{
	BsAssert(nType == 0);
	BsAssert(nLocal > -1 && nLocal < MAX_LANGAUGES);
	BsAssert(nType > -1 && nType < _FONT_TYPE_COUNT);

	FONT* pFont = NULL;
	char szTexName[MAX_PATH], szUVName[MAX_PATH];
	if(nLocal == LANGUAGE_ENG)
	{
		pFont = m_pFont[nType];
		sprintf( szTexName, "Font_%d_%s.dds", nType, g_LocalLanguage.GetLanguageStr(LANGUAGE_ENG));
		sprintf( szUVName, "Font_%d_%s.txt", nType, g_LocalLanguage.GetLanguageStr(LANGUAGE_ENG));
	}
	else
	{
		pFont = m_pLocalFont[nType];
		switch(nLocal)
		{
		case LANGUAGE_GER:
		case LANGUAGE_FRA:
		case LANGUAGE_SPA:
		case LANGUAGE_ITA:
			{
				sprintf( szTexName, "Font_%d_EURO.dds", nType);
				sprintf( szUVName, "Font%d_EURO.txt", nType);
				break;
			}
		default:
			{
				sprintf( szTexName, "Font_%d_%s.dds", nType, g_LocalLanguage.GetLanguageStr());
				sprintf( szUVName, "Font_%d_%s.txt", nType, g_LocalLanguage.GetLanguageStr());
				break;
			}
		}
	}

	BsAssert(pFont != NULL);
	
	g_BsKernel.chdir("Interface");
	g_BsKernel.chdir("Font");
	char szTexfileName[MAX_PATH], szUVfileName[MAX_PATH];
	sprintf(szTexfileName, "%s%s", g_BsKernel.GetCurrentDirectory(), szTexName);
	sprintf(szUVfileName, "%s%s", g_BsKernel.GetCurrentDirectory(), szUVName);
	g_BsKernel.chdir("..");
	g_BsKernel.chdir("..");


	pFont->nTexID = g_BsKernel.LoadTexture(szTexfileName);
	BsAssert(pFont->nTexID != -1 && "텍스쳐 오류");

	switch(nLocal)
	{	
	case LANGUAGE_KOR:
		{
			pFont->nCount = _HANGUL_NUM;
			pFont->nSpace = nSpaceWidth;
			break;
		}
	case LANGUAGE_JPN:
		{
			pFont->nCount = _JAPANESE_NUM;
			pFont->nSpace = nSpaceWidth;
			break;
		}
	case LANGUAGE_CHN: BsAssert(0); break;
	default:
		{
			LoadUV(pFont, szUVfileName, nSpaceWidth);
			break;
		}
	}
}

void BsUiFont::LoadUV(FONT* pFont, char *szUVfileName, int nSpaceWidth)
{	
	BFileStream bfStream(szUVfileName);

	if (!bfStream.Valid())
	{
		BsAssert(0 && "파일 열기 오류");
		return;
	}

	int nSize=bfStream.Length();

	char *mem = new char [nSize];
	if (mem == NULL)
	{
		BsAssert(mem && "falied to allocate memory");
		return;
	}

	bfStream.Read(mem,nSize);

	RECT rect[256];

	int mode=0, ri=0, y1,y2;
	char buf[128];

	for (int i=0; i<nSize; i++) {
		if (mem[i]==';') {
			for (++i; i<nSize; i++) {
				if (mem[i]=='\n') break;
			}
		}
		else if (mem[i]=='v' && mem[i+1]==':') {
			i++;
			mode=2;
		}
		else if (mem[i]>='0' && mem[i]<='9') {
			int idx=0;
			for (; i<nSize; i++) {
				if (mem[i]>='0' && mem[i]<='9') buf[idx++]=mem[i];
				else break;
			}
			buf[idx]=0;
			
			if (mode==2) {
				y1=atoi(buf);
				mode--;
			}
			else if (mode==1) {
				y2=atoi(buf);
				mode--;
			}
			else if (mode==0) {
				rect[ri].left=atoi(buf);
				mode--;
			}
			else {
				rect[ri].right=atoi(buf);	
				rect[ri].top=y1;
				rect[ri].bottom=y2;
				ri++;
				mode=0;
			}
		}
	}

	delete [] mem;

    pFont->rectUV = new RECT [ri];
	pFont->nCount = ri;
	pFont->nSpace = nSpaceWidth;
	memcpy(pFont->rectUV, rect, ri*sizeof(RECT));	// [PREFIX:nomodify] 2006/2/16 junyash PS#5222 reports using uninitialized memory '(rect->left) & 0xff'. but if szUVfileName is correct then all member initialized.
}

bool BsUiFont::LoadSpecialFontTable()
{
	if(m_pSpecialFont == NULL)
		return false;
	
	g_BsKernel.chdir("Interface");
	g_BsKernel.chdir("Font");
	char Dir[MAX_PATH], szUVfileName[MAX_PATH], szTexfileName[MAX_PATH];
	strcpy(Dir, g_BsKernel.GetCurrentDirectory());
	strcpy(szUVfileName, "Font_Special.txt");
	sprintf(szTexfileName, "%sFont_Special.dds", g_BsKernel.GetCurrentDirectory());
	g_BsKernel.chdir("..");
	g_BsKernel.chdir("..");

	Parser parser;
	TokenList toklist;

	parser.Create();
	parser.EnableComments( true );
	parser.ReserveKeyword( "SET_ANI_SPECIALCHAR" );
	parser.ReserveKeyword( "SET_ANI_SPEED" );
	parser.ReserveKeyword( "SET_ANI_FRAME_COUNT" );
	parser.ReserveKeyword( "SET_ANI_FRAME" );

	bool result = parser.ProcessSource( Dir, szUVfileName, &toklist );
	if(result==false) {
		BsAssert( 0 && "Unable to read UV List File" );
		return false;
	}
	parser.ProcessMacros(&toklist);


	scKEYLIST* pKeyList = &(m_pSpecialFont->KeyWord);

	RECT uvRect[256];
	int nCount = 0;
	TokenList::iterator itr = toklist.begin();
	Token token;
	for ( ; itr != toklist.end(); ++itr) 
	{	
		token = *itr;

		if (token.IsKeyword() == false)
		{
			SPECIAL_CHAR_KEY* pKeyWord = new SPECIAL_CHAR_KEY();
			pKeyWord->nCode = nCount;

			char szKey[64];
			strcpy(szKey, (*itr).GetString());
			int nlen = (int)strlen(szKey);
			for(int i=0; i<nlen; i++)
			{
				szKey[i] = tolower(szKey[i]);
			}
			pKeyWord->szKeyword = new char[nlen+1];
#ifdef _XBOX
			strcpy_s(pKeyWord->szKeyword, nlen+1, szKey);
#else
			strcpy(pKeyWord->szKeyword, szKey);
#endif

			pKeyList->push_back(pKeyWord);

			itr++;		uvRect[nCount].left = (*itr).GetInteger();
			itr++;		uvRect[nCount].top = (*itr).GetInteger();
			itr++;		uvRect[nCount].right = (*itr).GetInteger();
			itr++;		uvRect[nCount].bottom = (*itr).GetInteger();
		}
		else if (strcmp(token.GetKeyword(),"SET_ANI_SPECIALCHAR")==0)
		{
			itr++;
			SPECIAL_CHAR_KEY* pKeyWord = new SPECIAL_CHAR_KEY();
			pKeyWord->nCode = nCount;

			char szKey[64];
			strcpy_s(szKey, _countof(szKey), (*itr).GetString());
			int nlen = (int)strlen(szKey);
			for(int i=0; i<nlen; i++)
			{
				szKey[i] = tolower(szKey[i]);
			}
			pKeyWord->szKeyword = new char[nlen+1];
#ifdef _XBOX
			strcpy_s(pKeyWord->szKeyword, nlen+1, szKey); //aleksger - safe string
#else
			strcpy(pKeyWord->szKeyword, szKey); //aleksger - safe string
#endif

			pKeyList->push_back(pKeyWord);
			continue;
		}
		else if (strcmp(token.GetKeyword(),"SET_ANI_SPEED")==0)
		{
			itr++;
			scKEYLIST::iterator itrKey = pKeyList->end() - 1;
			SPECIAL_CHAR_KEY* pKeyWord = (*itrKey);
			pKeyWord->nSpeed = (*itr).GetInteger();
			continue;
		}
		else if (strcmp(token.GetKeyword(),"SET_ANI_FRAME_COUNT")==0)
		{
			itr++;
			scKEYLIST::iterator itrKey = pKeyList->end() - 1;
			SPECIAL_CHAR_KEY* pKeyWord = (*itrKey);
			pKeyWord->nTotalFrame = (*itr).GetInteger();
			continue;
		}
		else if (strcmp(token.GetKeyword(),"SET_ANI_FRAME")==0)
		{
			itr++;		uvRect[nCount].left = (*itr).GetInteger();
			itr++;		uvRect[nCount].top = (*itr).GetInteger();
			itr++;		uvRect[nCount].right = (*itr).GetInteger();
			itr++;		uvRect[nCount].bottom = (*itr).GetInteger();
		}

		nCount++;
	}

	FONT* pFont = &(m_pSpecialFont->Font);
	pFont->nTexID = g_BsKernel.LoadTexture(szTexfileName);
	BsAssert(pFont->nTexID != -1 && "텍스쳐 오류");
	pFont->rectUV = new RECT[nCount];
	pFont->nCount = nCount;
	memcpy(pFont->rectUV, uvRect, nCount*sizeof(RECT));

	return true;
}

//------------------------------------------------------------------------------------------------------------------------

bool BsUiFont::GetTextInt(char *szText, int &nOffset, int nLength, int nNum) 
{
	char buf[256];
	int md(0), buf_idx(0), minar(1); // 0

	for (; nOffset<nLength; nOffset++)  {
		if (szText[nOffset]=='(') { 
			if (md!=0)
				break;
			md=1; // 1
		}
		else if (szText[nOffset]=='-') {
			minar=-1;
		}
		else if (szText[nOffset]>='0' && szText[nOffset]<='9') {
			buf[buf_idx++]=szText[nOffset];
			if (buf_idx>=255)
				break;
		}
		else if (szText[nOffset]==',') {
			if (md<=0 || md>=nNum)
				break; // (1,2,3) == nNum-1

			buf[buf_idx]=0;
			m_nParsingBuf[md-1]=atoi(buf)*minar;
			minar = 1;

			buf_idx=0;
			md++; // 2,3,4 == nNum
		}
		else if (szText[nOffset]==')') {
			if (md!=nNum)
				break; // (4) == nNum

			buf[buf_idx]=0;
			m_nParsingBuf[md-1]=atoi(buf)*minar;

			return true;
		}
	}

	return false;
}

//------------------------------------------------------------------------------------------------------------------------

bool BsUiFont::GetTextFloat(char *szText, int &nOffset, int nLength, int nNum) 
{
	char buf[256];
	int md(0), buf_idx(0), minar(1); // 0
	for (; nOffset<nLength; nOffset++)  {
		if (szText[nOffset]=='(') { 
			if (md!=0)
				break;
			md=1; // 1
		}
		else if (szText[nOffset]=='-') {
			minar=-1;
		}
		else if ((szText[nOffset]>='0' && szText[nOffset]<='9') || szText[nOffset]=='.') {
			buf[buf_idx++]=szText[nOffset];
			if (buf_idx>=255)
				break;
		}
		else if (szText[nOffset]==',') {
			if (md<=0 || md>=nNum)
				break; // (1,2,3) == nNum-1

			buf[buf_idx]=0;
			m_fParsingBuf[md-1]=(float)atof(buf);

			buf_idx=0;
			md++; // 2,3,4 == nNum
		}
		else if (szText[nOffset]==')') {
			if (md!=nNum)
				break; // (4) == nNum

			buf[buf_idx]=0;
			m_fParsingBuf[md-1]=(float)atof(buf);

			return true;
		}
	}

	return false;
}


void BsUiFont::GetFontAttrStruct(SFontAttribute *pAttribute, char *szText)
{
	SFontAttribute *pAttr = (SFontAttribute*)pAttribute;
	pAttr->Set();

	while(1)
	{	
		if(IsFontAttr(&szText, pAttr))
			continue;
		else
			break;
	}
}

void BsUiFont::GetFontAttrText(char *szText, const size_t szText_len, SFontAttribute *pFontAttr)
{
	SFontAttribute tempFontAttr;
	tempFontAttr.Set();

	strcpy_s(szText, szText_len, "");
	// aleksger - replaced all strcat with strcat_s for safe strings
	if(pFontAttr->bReset != false)
	{
		char szReset[64];
		sprintf(szReset, "%c%s",
			_FONTATTR_PARAM_CODE,
			m_FontAttrKeyword[_FAP_ID_RESET]);
		strcat_s(szText, szText_len, szReset);
	}

	if(pFontAttr->nType != tempFontAttr.nType)
	{
		char szType[64];
		sprintf(szType, "%c%s%d)",
			_FONTATTR_PARAM_CODE,
			m_FontAttrKeyword[_FAP_ID_TYPE],
			pFontAttr->nType);
		strcat_s(szText, szText_len, szType);
	}

	if(pFontAttr->Color != tempFontAttr.Color)
	{
		char szColor[64];
		sprintf(szColor, "%c%s%d,%d,%d,%d)",
			_FONTATTR_PARAM_CODE,
			m_FontAttrKeyword[_FAP_ID_COLOR],
			(int)(pFontAttr->Color.r * 255),
			(int)(pFontAttr->Color.g * 255),
			(int)(pFontAttr->Color.b * 255),
			(int)(pFontAttr->Color.a * 255));

		strcat_s(szText, szText_len, szColor);
	}

	if(pFontAttr->fScaleX != tempFontAttr.fScaleX || pFontAttr->fScaleY != tempFontAttr.fScaleY)
	{
		char szScaleX[64], szScaleY[64];
		TransfloatToChar(szScaleX, _countof(szScaleX), pFontAttr->fScaleX); //aleksger - safe string
		TransfloatToChar(szScaleY, _countof(szScaleY), pFontAttr->fScaleY);//aleksger - safe string
		char szSize[64];
		sprintf(szSize, "%c%s%s,%s)",
			_FONTATTR_PARAM_CODE,
			m_FontAttrKeyword[_FAP_ID_SCALE],
			szScaleX, szScaleY);
		strcat_s(szText, szText_len, szSize);
	}

	if(pFontAttr->nStyle != tempFontAttr.nStyle)
	{
		char szStype[64];
		sprintf(szStype, "%c%s%d)",
			_FONTATTR_PARAM_CODE,
			m_FontAttrKeyword[_FAP_ID_STYLE],
			pFontAttr->nStyle);
		strcat_s(szText, szText_len, szStype);
	}

	if(pFontAttr->nAlign != tempFontAttr.nAlign)
	{
		char szAlign[64];
		sprintf(szAlign, "%c%s%d)",
			_FONTATTR_PARAM_CODE,
			m_FontAttrKeyword[_FAP_ID_ALIGN],
			pFontAttr->nAlign);
		strcat_s(szText, szText_len, szAlign);
	}

	if(pFontAttr->nSpaceX != tempFontAttr.nSpaceX)
	{
		char szSpaceX[64];
		sprintf(szSpaceX, "%c%s%d)",
			_FONTATTR_PARAM_CODE,
			m_FontAttrKeyword[_FAP_ID_SPACE],
			pFontAttr->nSpaceX);
		strcat_s(szText, szText_len, szSpaceX);
	}

	if(pFontAttr->nSpaceY != tempFontAttr.nSpaceY)
	{
		char szSpaceY[64];
		sprintf(szSpaceY, "%c%s%d)",
			_FONTATTR_PARAM_CODE,
			m_FontAttrKeyword[_FAP_ID_LINE_SPACE],
			pFontAttr->nSpaceY);
		strcat_s(szText, szText_len, szSpaceY);
	}

	if(pFontAttr->bWrap != true)
	{
		char szWrap[64];
		sprintf(szWrap, "%c%s%d)",
			_FONTATTR_PARAM_CODE,
			m_FontAttrKeyword[_FAP_ID_WRAP],
			pFontAttr->bWrap);
		strcat_s(szText, szText_len, szWrap);
	}

	if(pFontAttr->bTruncate != false)
	{
		char szTruncate[64];
		sprintf(szTruncate, "%c%s%d)",
			_FONTATTR_PARAM_CODE,
			m_FontAttrKeyword[_FAP_ID_TRUNCATE],
			pFontAttr->bTruncate);
		strcat_s(szText, szText_len, szTruncate);
	}
	
	if(pFontAttr->bIgnore != false)
	{
		char szIgnore[64];
		sprintf(szIgnore, "%c%s",
			_FONTATTR_PARAM_CODE,
			m_FontAttrKeyword[_FAP_ID_IGNORE]);
		strcat_s(szText, szText_len, szIgnore);
	}
}

//------------------------------------------------------------------------------------------------------------------------
bool BsUiFont::IsAlphabet(char ch)
{
	if ((ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z'))
		return true;
	else
		return false;
}

bool BsUiFont::IsUniCode(char ch)
{
	if(ch & 0x80) {
		return true;
	}
	return false;
}

bool BsUiFont::IsChar(char ch)
{
	// 39 : '
	if(IsAlphabet(ch) || IsUniCode(ch) || ch == '.' || ch == ',' || ch == 39) 
		return true;
	else
		return false;
}


int BsUiFont::GetLetterWidth(int nCode, int nLocal, int nType)
{
	switch(nLocal)
	{
	case LANGUAGE_ENG:
		{	
			int nWidth = m_pFont[nType]->rectUV[nCode].right - m_pFont[nType]->rectUV[nCode].left;
			return (int)(m_FontAttr.fScaleX * (float)(nWidth + _ALPHABET_MARGIN) + m_FontAttr.nSpaceX);
		}
	case LANGUAGE_KOR:
		{
			if (m_FontAttr.fScaleX < _HANGUL_FONT_SIZE_MIN)
				return (int)(_HANGUL_FONT_SIZE_MIN * (_HANGUL_FONT_SIZE + _HANGUL_MARGIN) + m_FontAttr.nSpaceX);
			else
				return (int)(m_FontAttr.fScaleX * (_HANGUL_FONT_SIZE + _HANGUL_MARGIN) + m_FontAttr.nSpaceX);
		}
	case LANGUAGE_GER:
	case LANGUAGE_FRA:
	case LANGUAGE_SPA:
	case LANGUAGE_ITA:
		{
			int nWidth = m_pFont[nType]->rectUV[nCode].right - m_pFont[nType]->rectUV[nCode].left;
			return (int)(m_FontAttr.fScaleX * (float)(nWidth + _ALPHABET_MARGIN) + m_FontAttr.nSpaceX);
		}
	case LANGUAGE_JPN:
		{
			if (m_FontAttr.fScaleX < _JAPAN_FONT_SIZE_MIN)
				return (int)(_JAPAN_FONT_SIZE_MIN * (_JAPAN_FONT_SIZE + _JAPAN_MARGIN) + m_FontAttr.nSpaceX);
			else
				return (int)(m_FontAttr.fScaleX * (_JAPAN_FONT_SIZE + _JAPAN_MARGIN) + m_FontAttr.nSpaceX);
		}
	case LANGUAGE_CHN: break;
	default:
		{
			BsAssert(0);
		}
	}

	return 24;
}

int BsUiFont::GetWordWidth(char *pStr, int nType)
{
	float	fRValue;
	int		nCode;
	char	*pScan;

	fRValue = 0;

	pScan = pStr;

	while((*pScan) != '\x0' )
	{
		if (IsChar(*pScan) == false)
			break;

		//unicode가 아니면 여기서 처리
		if(IsUniCode(*pScan) == false)
		{
			nCode = (*pScan) - _ASCII_BASE_NUM;
			if (nCode < 0 || nCode >= m_pFont[nType]->nCount)
				fRValue += _DEFAULT_SPACE;
			else
				fRValue += GetLetterWidth(nCode, LANGUAGE_ENG, nType);

			pScan++;
			continue;
		}
		
		//unicode면 여기서 처리
		BsAssert(*pScan !='\x0');
		switch(m_nFontLocal)
		{
		case LANGUAGE_KOR:
			{
				pScan++;
				fRValue += GetLetterWidth(-1, m_nFontLocal, nType);
				break;
			}
		case LANGUAGE_GER:
		case LANGUAGE_FRA:
		case LANGUAGE_SPA:
		case LANGUAGE_ITA:
			{
				nCode = *(unsigned char*)pScan - _ASCII_BASE_NUM;
				if (nCode < 0 || nCode >= m_pFont[nType]->nCount)
					fRValue += _DEFAULT_SPACE;
				else
					fRValue += GetLetterWidth(nCode, m_nFontLocal, nType);
				break;
			}
		case LANGUAGE_JPN:
			{
				pScan++;
				fRValue += GetLetterWidth(-1, m_nFontLocal, nType);
				break;
			}
		default:
			{
				//BsAssert("Unicode letters can not be used in English");
				FONT* pFont = pFont = &m_pSpecialFont->Font;
				RECT uvRect;
				uvRect = pFont->rectUV[0];
				fRValue += uvRect.right - uvRect.left;
				pScan++;
			}	
		}
		
		pScan++;
	}

	return (int)fRValue;
}


int	BsUiFont::GetCode(char* pszTemp, int &nLocal)
{
	if(IsUniCode(*pszTemp) == false)
	{
		nLocal = LANGUAGE_ENG;
		return ((*pszTemp) - _ASCII_BASE_NUM);
	}

	int nCode = _INVALID_CODE;
	switch(nLocal)
	{
	case LANGUAGE_KOR:
	case LANGUAGE_JPN:
	case LANGUAGE_CHN:
		{
			nCode = *(unsigned short*)pszTemp;
			break;
		}
	case LANGUAGE_GER:
	case LANGUAGE_FRA:
	case LANGUAGE_SPA:
	case LANGUAGE_ITA:
		{
			nCode = *(unsigned char*)pszTemp - _ASCII_BASE_NUM;
			break;
		}
	case LANGUAGE_ENG:
		{	
			BsAssert("Unicode letters can not be used in English");
			break;
		}
	default:
		{
			BsAssert(0);
			break;
		}
	}

	return nCode;
	
}

bool BsUiFont::IsTab(char **szText)
{
	if((**szText) == '\t')
	{
		(*szText)++;
		return true;
	}
	else if((**szText) == '\\')
	{ 
		(*szText)++;
		if((**szText)=='t')
		{
			szText++;
			return true;
		}
		(*szText)--;
	}

	return false;
}

bool BsUiFont::IsNewLine(char** szText)
{
	if ((**szText) == '\n')
	{
		(*szText)++;
		return true;
	}
	else if((**szText) == '\\')
	{ 
		(*szText)++;
		if((**szText) == 'n')
		{
			(*szText)++;
			return true;
		}
		(*szText)--;
	}

	return false;
}

bool BsUiFont::IsFontAttr(char **szText, SFontAttribute* pFontAttr)
{
	if(strlen(*szText) == 0)
		return false;

	(*szText)++;

	char Buf[_FAP_KEYWORD_LENGTH];
#ifdef _XBOX
	strncpy_s(Buf, _FAP_KEYWORD_LENGTH, (*szText), _TRUNCATE); //aleksger - safe string conversion with truncation.
#else
	strncpy(Buf, (*szText), _FAP_KEYWORD_LENGTH);
//	strncpy(Buf, (*szText), _TRUNCATE); //aleksger - safe string conversion with truncation.
#endif

	int nKey = 0;

	for(nKey = 0; nKey < _FAP_KEYWORD_LENGTH; nKey++)
	{
		if (Buf[nKey] == ')')
		{
			if (nKey < _FAP_KEYWORD_LENGTH - 1)
				Buf[nKey+1] = '\x0';

			break;
		}

		if (Buf[nKey] >= 0)
			Buf[nKey] = tolower(Buf[nKey]);
	}

	for(int nKey=0; nKey<m_nFontAttrKeyword; nKey++)
	{
		if (!strncmp(Buf,m_FontAttrKeyword[nKey],strlen(m_FontAttrKeyword[nKey])))
			break;
	}

	if(nKey >= m_nFontAttrKeyword)
	{
		(*szText)--;
		return false;
	}

	int len = (int)strlen(Buf);
	(*szText) += len;

	if(IsIgnoreFontAttr())
	{
		return false;
	}

	char* param = Buf;
	param += strlen(m_FontAttrKeyword[nKey])-1;
	param[0] = '(';
	int nOffset = 0;

	bool bFlag = false;
	switch(nKey)
	{
	case _FAP_ID_TYPE:
		{
			if(bFlag = GetTextInt(param,nOffset,(int)strlen(param),1))
			{
				pFontAttr->nType = (m_nParsingBuf[0] == 0) ? FT_Default : FT_Addition;
			}
			break;
		}
	case _FAP_ID_COLOR:
		{	
			if(bFlag = GetTextFloat(param,nOffset,(int)strlen(param),4))
			{
				pFontAttr->Color.r = m_fParsingBuf[0]/255.f;
				pFontAttr->Color.g = m_fParsingBuf[1]/255.f;
				pFontAttr->Color.b = m_fParsingBuf[2]/255.f;
				pFontAttr->Color.a = m_fParsingBuf[3]/255.f;
			}
			break;
		}
	case _FAP_ID_SCALE:
		{	
			if(bFlag = GetTextFloat(param,nOffset,(int)strlen(param),2))
			{
				pFontAttr->fScaleX = m_fParsingBuf[0];
				pFontAttr->fScaleY = m_fParsingBuf[1];
			}
			break;
		}
	case _FAP_ID_STYLE:
		{
			if(bFlag = GetTextInt(param,nOffset,(int)strlen(param),1))
			{
				pFontAttr->nStyle = (m_nParsingBuf[0] == 0) ? FS_Regular : FS_Italic;
			}
			break;
		}
	case _FAP_ID_ALIGN:
		{
			if(bFlag = GetTextInt(param,nOffset,(int)strlen(param),1))
			{
				pFontAttr->nAlign = (eAllign)m_nParsingBuf[0];
			}
			break;
		}
	case _FAP_ID_SPACE:
		{
			if(bFlag = GetTextInt(param,nOffset,(int)strlen(param),1))
			{
				pFontAttr->nSpaceX = m_nParsingBuf[0];
			}
			break;
		}
	case _FAP_ID_LINE_SPACE:
		{
			if(bFlag = GetTextInt(param,nOffset,(int)strlen(param),1))
			{
				pFontAttr->nSpaceY = m_nParsingBuf[0];
			}
			break;
		}
	case _FAP_ID_WRAP:
		{
			if(bFlag = GetTextInt(param,nOffset,(int)strlen(param),1))
			{
				pFontAttr->bWrap = m_nParsingBuf[0] ? true:false;
			}
			break;
		}
	case _FAP_ID_TRUNCATE:
		{
			if(bFlag = GetTextInt(param,nOffset,(int)strlen(param),1))
			{
				pFontAttr->bTruncate = m_nParsingBuf[0] ? true:false;
			}
			break;
		}
	case _FAP_ID_IGNORE:
		{	
			pFontAttr->bIgnore = true;
			bFlag = true;
			break;
		}
	case _FAP_ID_RESET:
		{
			pFontAttr->Set();
			pFontAttr->bReset = true;
			bFlag = true;
			break;
		}
	default:
		BsAssert(0);
	}

	return bFlag;
}

bool BsUiFont::IsSpecialFont(char **szText, int &nCode)
{
	if(m_pSpecialFont == NULL)
	{
		return false;
	}

	(*szText)++;

	char Buf[_FAP_KEYWORD_LENGTH];
#ifdef _XBOX
	strncpy_s(Buf, _FAP_KEYWORD_LENGTH, (*szText), _TRUNCATE );
#else
	strncpy_s(Buf, _FAP_KEYWORD_LENGTH, (*szText), _TRUNCATE);
	//strncpy(Buf, (*szText), _FAP_KEYWORD_LENGTH );
//	strncpy(Buf, (*szText), ((size_t)-1) );
#endif

	for(int nKey = 0; nKey < _FAP_KEYWORD_LENGTH; nKey++)
	{
		if (Buf[nKey] == ')')
		{
			if (nKey < _FAP_KEYWORD_LENGTH - 1)
				Buf[nKey+1] = '\x0';

			break;
		}

		if (Buf[nKey] >= 0)
			Buf[nKey] = tolower(Buf[nKey]);
	}
	
	scKEYLIST* pKeyList = &(m_pSpecialFont->KeyWord);
	for(DWORD dwKey=0; dwKey<pKeyList->size(); dwKey++)
	{
		SPECIAL_CHAR_KEY* pKey = (*pKeyList)[dwKey];
		if (!strncmp(Buf,pKey->szKeyword ,strlen(pKey->szKeyword)))
		{
			int len = (int)strlen(Buf);
			(*szText) += len;

			nCode = pKey->nCode + pKey->nCurFrame;
			return true;
		}
	}

	
	(*szText)--;

	return false;
}

void BsUiFont::ProcessFontText(int sx, int sy, int ex, int ey, char *szText, int &nWidth, int &nHeight, int &nWordCount, std::vector<int>* pLineWidthList)
{
	int len=(int)strlen(szText);
	if(len == 0)
		return;

	char* pszDest = new char[len+1];
	strcpy_s(pszDest, len+1, szText); //aleksger - safe string

	char* pszTemp = pszDest;
	char prevToken = '\0';
	int nX(sx), nY(sy);
	bool bFixWordWrap = m_FontAttr.bWrap;
	bool bFirstWordWrap = true;		//첫 단어가 길 경우 wrap을 막는다.

	int nTempWordCount(0);

	if(sx >= ex || ex == -1 || m_nFontLocal == LANGUAGE_JPN)
	{
		bFixWordWrap = false;
	}

	SetIgnoreFontAttr(false);

	while(1)
	{
		if (pszTemp - pszDest >= (int)strlen(pszDest))
			break;
		
		int nCode = _INVALID_CODE;
		bool bSpecial = false;
		if (*pszTemp==_FONTATTR_PARAM_CODE)
		{
			if(IsFontAttr(&pszTemp, &m_FontAttr))
			{
				continue;
			}
			else if(IsSpecialFont(&pszTemp, nCode))
			{
				bSpecial = true;	
			}
		}

		if(IsTab(&pszTemp))
		{
			int nTabWidth = (int)((_TAB_WIDTH + m_FontAttr.nSpaceX) * m_FontAttr.fScaleX);
			if(ex != -1 && nX + nTabWidth >= ex)
			{
				pLineWidthList->push_back(nX);

				nX += nTabWidth - ex;
				nY += (int)((_CHAR_HEIGHT + m_FontAttr.nSpaceY) * m_FontAttr.fScaleY);
			}
			else
			{
				nX += nTabWidth;
			}

			continue;
		}

		if(IsNewLine(&pszTemp))
		{
			pLineWidthList->push_back(nX);

			nX = sx;
			nY += (int)((_CHAR_HEIGHT + m_FontAttr.nSpaceY) * m_FontAttr.fScaleY);

			continue;
		}

		if (bFirstWordWrap == false &&
			m_FontAttr.bWrap == true && 
			bFixWordWrap == true &&
			IsChar(prevToken) == false &&
			IsChar(*pszTemp) == true)
		{
			int nWordWidth = (int)GetWordWidth(pszTemp, m_FontAttr.nType);

			if (nX + nWordWidth > ex)// && nWordWidth < ex - sx)
			{
				pLineWidthList->push_back(nX);

				nX = sx;
				nY += (int)((_CHAR_HEIGHT + m_FontAttr.nSpaceY) * m_FontAttr.fScaleY);
			}
		}

		bFirstWordWrap = false;

		int nTempLocal = m_nFontLocal;
		if(!bSpecial){
			nCode = GetCode(pszTemp, nTempLocal);
		}

		FONT* pFont = NULL;
		RECT uvRect = {-1, -1, -1, -1};
		if( nCode != _INVALID_CODE)
		{
			//BsAssert(nCode > _INVALID_CODE);

			if(nCode == _SPACE_CODE){
				nX += (int)(m_pFont[m_FontAttr.nType]->nSpace * m_FontAttr.fScaleX);
			}
			else
			{	
				if (bSpecial){
					pFont = &m_pSpecialFont->Font;
					if(pFont->nCount > nCode){
						uvRect = pFont->rectUV[nCode];
					}
				}
				else if(nTempLocal == LANGUAGE_ENG)	{
					pFont = m_pFont[m_FontAttr.nType];
					if(pFont->nCount > nCode){
						uvRect = pFont->rectUV[nCode];
					}
				}
				else if(nTempLocal == LANGUAGE_GER ||
					nTempLocal == LANGUAGE_FRA ||
					nTempLocal == LANGUAGE_SPA ||
					nTempLocal == LANGUAGE_ITA)	{
					pFont = m_pLocalFont[m_FontAttr.nType];
					if(pFont->nCount > nCode){
						uvRect = pFont->rectUV[nCode];
					}
				}
				else {
					pFont = m_pLocalFont[m_FontAttr.nType];
					GetFontUV(nCode, nTempLocal, pFont, uvRect);
				}
			}
		}

		if(nCode != _SPACE_CODE)
		{
			if(nCode == _INVALID_CODE || pFont == NULL || uvRect.left == -1)
			{
				if(nCode == _INVALID_CODE ||
					nTempLocal == LANGUAGE_KOR ||
					nTempLocal == LANGUAGE_JPN ||
					nTempLocal==LANGUAGE_CHN){
					pszTemp += 2;
				}else{
					pszTemp++;
				}

				bSpecial = true;
				pFont = &m_pSpecialFont->Font;
				uvRect = pFont->rectUV[0];
			}
		}
		
		//-------------------------------------------------------
		// Set Token Info
		int nTempWidth = uvRect.right  - uvRect.left;
		int nTempHeight = uvRect.bottom - uvRect.top;
		int nHalf = (int)(nTempHeight * m_FontAttr.fScaleY / 2.f);

		RECT posRect;
		posRect.left = nX;
		posRect.top = nY - nHalf;
		posRect.right = nX + (int)(nTempWidth * m_FontAttr.fScaleX);
		posRect.bottom = nY + nHalf;

		TOKEN_INFO tokeninfo;
		tokeninfo.nCode = nCode;
		tokeninfo.bSpecial = bSpecial;
		tokeninfo.nLine = (int)pLineWidthList->size();
		tokeninfo.nType = bSpecial ? -1 : m_FontAttr.nType;
		tokeninfo.nLocal = bSpecial ? -1 : nTempLocal;
		tokeninfo.nStype = m_FontAttr.nStyle;
		tokeninfo.Color = m_FontAttr.Color;
		tokeninfo.Color.a *= m_fAlpha;
		tokeninfo.rect = posRect;
		tokeninfo.uvRect = uvRect;

		if(bSpecial)
		{
			m_S_TokenInfoList.push_back(tokeninfo);
		}
		else if(nTempLocal == LANGUAGE_ENG)
		{
			m_F_TokenInfoList.push_back(tokeninfo);
		}
		else
		{
			m_L_TokenInfoList.push_back(tokeninfo);
		}
		//-------------------------------------------------------

		nX += (int)((nTempWidth + m_FontAttr.nSpaceX) * m_FontAttr.fScaleX);
		nTempWordCount++;

		if(bSpecial){
			continue;
		}

		prevToken = *pszTemp;
		if(nTempLocal == LANGUAGE_KOR || nTempLocal == LANGUAGE_JPN || nTempLocal==LANGUAGE_CHN)
		{
			pszTemp += 2;
		}
		else {
			pszTemp++;
		}
	}

	delete[] pszDest;
	
	pLineWidthList->push_back(nX);

	int nTempWidth = 0;
	DWORD dwCount = pLineWidthList->size();
	for(DWORD i=0; i<dwCount; i++)
	{
		if(nTempWidth < (*pLineWidthList)[i]){
			nTempWidth = (*pLineWidthList)[i];
		}
	}
	nWidth = nTempWidth - sx;
	nHeight = nY - sy;
	nWordCount = nTempWordCount;
}


void BsUiFont::CreateFontText(int sx, int sy, int ex, int ey, char *szText, float fAlpha)
{	
	if( (int)m_F_TokenInfoList.size() > 0 )
		m_F_TokenInfoList.clear();

	if( (int)m_L_TokenInfoList.size() > 0 )
		m_L_TokenInfoList.clear();

	if( (int)m_S_TokenInfoList.size() > 0 )
		m_S_TokenInfoList.clear();

	if( (int)m_LineWidthList.size() > 0 )
		m_LineWidthList.clear();
	
	m_fAlpha = fAlpha;
	sy += _Y_MARGIN;
	int nWidth(0), nHeight(0), nWordCount(0);
	ProcessFontText(sx, sy, ex, ey, szText, nWidth, nHeight, nWordCount, &m_LineWidthList);

	//eng font
	CreateFontElement(sx, ex, &m_F_TokenInfoList, (DWORD)m_pFont);

	//local font
	CreateFontElement(sx, ex, &m_L_TokenInfoList, (DWORD)m_pLocalFont);

	//special font
	CreateFontElement(sx, ex, &m_S_TokenInfoList, (DWORD)m_pSpecialFont);
}

void BsUiFont::CreateFontElement(int sx, int ex, std::vector<TOKEN_INFO>* pTokenInfoList, DWORD dwFont)
{
	for(DWORD i=0; i<pTokenInfoList->size(); i++)
	{
		TOKEN_INFO tokeninfo = (*pTokenInfoList)[i];
		
		FONT* pFont = NULL;
		if(tokeninfo.bSpecial == true)
		{
			BsAssert(tokeninfo.nType == -1);
			BsAssert(tokeninfo.nLocal == -1);
			pFont = (FONT*)dwFont;
		}
		else
		{
			BsAssert(tokeninfo.nType != -1);
			pFont = ((FONT**)dwFont)[tokeninfo.nType];
		}

		BsAssert(pFont != NULL);

		RECT pos = tokeninfo.rect;
		if (ex != -1 && m_FontAttr.nAlign != eAlignLeft)
		{
			int nWidth = ex - sx;
			int nMove = ex - m_LineWidthList[tokeninfo.nLine];

			if (m_FontAttr.nAlign == eAlignCenter)
				nMove = (int)(nMove * .5f);

			pos.left += nMove;
			pos.right += nMove;
		}

		POINT xy1 = {pos.left, pos.top};
		POINT xy2 = {pos.right, pos.top};
		POINT xy3 = {pos.left, pos.bottom};
		POINT xy4 = {pos.right, pos.bottom};
		if(tokeninfo.nStype == FS_Italic && tokeninfo.bSpecial == false)
		{
			int nMove = (int)(_ITALIC_MOVE * m_FontAttr.fScaleX);
			xy1.x += nMove;
			xy2.x += nMove;
		}

		RECT uv = tokeninfo.uvRect;
		bool bRet = g_BsKernel.DrawUIBox_Pos4(_Ui_Mode_Text,
			xy1.x, xy1.y,
			xy2.x, xy2.y,
			xy3.x, xy3.y,
			xy4.x, xy4.y,
			0.f, tokeninfo.Color,
			pFont->nTexID,
			uv.left, uv.top,
			uv.right, uv.bottom);
	}
}


void BsUiFont::GetTextLengthInfo(int &nWidth, int &nHeight, int &nLine, int &nWordCount, int sx, int sy, int ex, int ey, char *szText)
{	
	std::vector<int> LineWidthList;
	
	ProcessFontText(sx, sy, ex, ey, szText, nWidth, nHeight, nWordCount, &LineWidthList);
	nLine = (int)LineWidthList.size();
	LineWidthList.clear();
}

bool BsUiFont::GetFontUV(int nCode, int nLocal, FONT* pFont, RECT& uvRect)
{
	BsAssert(pFont != NULL);

	switch(nLocal)
	{
	case LANGUAGE_KOR:
		{
#ifdef _XBOX
			int high=nCode&0xff;
			int low=nCode>>8;
#else
			int high=nCode>>8;
			int low=nCode&0xff;
#endif

			int index=(low-176)*94+high-161;
			if (index<0 || index >= _HANGUL_NUM)
				return false;
			
			int sx = index%_HANGUL_X_NUM;
			int sy= index/_HANGUL_X_NUM;

			uvRect.left = sx*_HANGUL_FONT_SIZE;
			uvRect.top = sy*(_HANGUL_FONT_SIZE+_HANGUL_FONT_Y_GAP);
			uvRect.right = uvRect.left + _HANGUL_FONT_SIZE;
			uvRect.bottom = uvRect.top + _HANGUL_FONT_SIZE;

			return true;
		}
	case LANGUAGE_JPN:
		{
			int nIndex = GetJapanFontIndex(nCode);

			if (nIndex<0 || nIndex>=_JAPANESE_NUM)
				return false;

			int xmagin = nIndex/(_JAPANESE_X_NUM*_JAPANESE_Y_NUM);
			int sx = (nIndex%_JAPANESE_X_NUM)+(xmagin*_JAPANESE_X_NUM);
			int sy = (nIndex-(_JAPANESE_X_NUM*_JAPANESE_Y_NUM*xmagin))/_JAPANESE_X_NUM;

			uvRect.left = sx*_JAPAN_FONT_SIZE;
			uvRect.top = sy*_JAPAN_FONT_SIZE;
			uvRect.right = uvRect.left + _JAPAN_FONT_SIZE;
			uvRect.bottom = uvRect.top + _JAPAN_FONT_SIZE;

			return true;
		}
	case LANGUAGE_CHN:
		{
			;
		}
	case LANGUAGE_ENG:
	case LANGUAGE_GER:
	case LANGUAGE_FRA:
	case LANGUAGE_SPA:
	case LANGUAGE_ITA:
		{
			BsAssert(0);
			return false;
		}
	}

	return false;
	
}

int BsUiFont::GetJapanFontIndex(int nUniCode)
{
	//Shift-JIS code사용 :
	//	일본에서 온 문서의 폰트 code가 위가 같은 이유로 해서 아래와 같이 인덱스를 얻게 coding했써여..

	//81-84(4), 88-9f(24), e0-ea (11)

	//93row = (12row * 7group) + 9row
	//81-8a(7group), 8b(9row)
	//8b(3row), 8c-92(7group), 93(6row)
	//93(6row), 94-9a(7group), 9b(3row)
	//9b(9row), 9c-9f(4group), e0-e2(3group)
	//e3-e9(7group), ea(7row)

#ifdef _XBOX
	int nHigh = nUniCode >> 8;
	int nLow = nUniCode & 0xff;
#else
	int nHigh = nUniCode & 0xff;
	int nLow = nUniCode >> 8;
#endif

	//0x81-0x84(4 * 12 * 16)
	//0x88-0x9f(24 * 12 * 16)
	//0xe0-0xea(11 * 12 * 16)
	int nHighstep(0), nHighMagin(0);
	if(nHigh >= 0x81 && nHigh <= 0x84)
	{
		nHighstep = 0x81;
		nHighMagin = 0;
	}
	else if(nHigh >= 0x88 && nHigh <= 0x9f)
	{
		nHighstep = 0x88;
		//이전 index 합
		nHighMagin = 4 * 12 * 16;
	}
	else if(nHigh >= 0xe0 && nHigh <= 0xea)
	{
		nHighstep = 0xe0;
		//이전 index 합
		nHighMagin = (4 + 24) * 12 * 16;
	}


	//0x3f-0x7e(4 * 16) 
	//0x80-0x9e(2 * 16)
	//0x9f-0xfc(6 * 16)
	int nLowstep(0), nLowMagin(0);
	if( nLow >= 0x3f && nLow <= 0x7e)
	{
		nLowstep = 0x3f;
		nLowMagin = 0;
	}
	else if(nLow >= 0x80 && nLow <= 0x9e)
	{
		nLowstep = 0x80;
		//이전 index 합
		nLowMagin = 4* 16;
	}
	else if(nLow >= 0x9f && nLow <= 0xfc)
	{
		nLowstep = 0x9e;
		//이전 index 합
		nLowMagin = 6 * 16;
	}

	int nhighIndex = nHighMagin + ((nHigh-nHighstep)*16*12);
	int nlowIndex = nLowMagin + nLow - nLowstep;

	return nhighIndex + nlowIndex;
}
