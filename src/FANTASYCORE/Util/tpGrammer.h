#ifndef _TP_GRAMMER_H_
#define _TP_GRAMMER_H_

// tpGrammer.h: lex를 활용한 간단한 문법 체크기, 아아 걍 yacc를 좀 배운 뒤에 그거 쓸까 ㅜㅜ 만들기 구찮다.

// jazzcake@hotmail.com
// 2002.2.18

#include <map>
#include <string>
#include <vector>

#include "Parser.h"				// 원래 다른 파서용이였는데 GPG Parser용으로 교체한다.
#include "Token.h"

using namespace std;

/////////////////////////////////////////////////////////////////////////////////////////////
//

// 단일 문법의 최대 토큰 길이
#define tpMAX_TOKENINGRAMMER				30

typedef struct {
	int		type;
	string	str;
	int		line;
} tpLEXTOKEN;

typedef struct {
	int		type[tpMAX_TOKENINGRAMMER];
	string	str[tpMAX_TOKENINGRAMMER];
	int		size;
} tpGRAMMERITEM;

typedef	map <int, tpGRAMMERITEM*>		tpGRAMMERMAP;
typedef tpGRAMMERMAP::iterator			tpGRAMMERMAPITOR;

// macro
typedef map <string, string>			tpGRAMMERMACROMAP;
typedef tpGRAMMERMACROMAP::iterator		tpGRAMMERMACROMAPITOR;

/////////////////////////////////////////////////////////////////////////////////////////////
//

class tpGrammer {
protected:
	vector <tpLEXTOKEN* >	m_tokens;
	tpGRAMMERMAP			m_grammer;
	tpGRAMMERMACROMAP		m_macro;

protected:
	vector <string>			m_params;			// 문법의 인자
	string					m_unknown;			// 판단할 수 없는 어휘

private:
	int						m_index;			// 내부변수
	int						m_line;

public:
	tpGrammer() { m_params.reserve(tpMAX_TOKENINGRAMMER); }		// 인자도 토큰 길이를 넘을 수 없다.
	virtual ~tpGrammer() { Release(); }

	bool			Create(const char* pFilename, const char* pHeaderPath);
	void			Release();
	void			Clear();
	void			ClearTokens();

	int				Get();						// 다 읽으면 0, 에러는 -1
	const char*		GetParam(int index);
	const char*		GetParamNotUseMacro(int index);
	const char*		GetUnknown() { return m_unknown.data(); }

	// 문법을 추가한다.
	bool			Add(int code, const char* pGrammer);
	// Macro를 추가한다.
	void			AddMacro(char* pMacro, char* pReal);		// 중복되면 udpate한다.

	// debug용
	void			PrintGrammer();
	int				GetCurLine() { return m_line; }				// 에러가 발생했을 때 찾을 수 있게 한다.

protected:
	bool			IsSame(int t1, string& s1, int t2, string& s2);
};

#endif

