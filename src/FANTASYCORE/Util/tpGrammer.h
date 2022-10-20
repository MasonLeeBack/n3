#ifndef _TP_GRAMMER_H_
#define _TP_GRAMMER_H_

// tpGrammer.h: lex�� Ȱ���� ������ ���� üũ��, �ƾ� �� yacc�� �� ��� �ڿ� �װ� ���� �̤� ����� ������.

// jazzcake@hotmail.com
// 2002.2.18

#include <map>
#include <string>
#include <vector>

#include "Parser.h"				// ���� �ٸ� �ļ����̿��µ� GPG Parser������ ��ü�Ѵ�.
#include "Token.h"

using namespace std;

/////////////////////////////////////////////////////////////////////////////////////////////
//

// ���� ������ �ִ� ��ū ����
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
	vector <string>			m_params;			// ������ ����
	string					m_unknown;			// �Ǵ��� �� ���� ����

private:
	int						m_index;			// ���κ���
	int						m_line;

public:
	tpGrammer() { m_params.reserve(tpMAX_TOKENINGRAMMER); }		// ���ڵ� ��ū ���̸� ���� �� ����.
	virtual ~tpGrammer() { Release(); }

	bool			Create(const char* pFilename, const char* pHeaderPath);
	void			Release();
	void			Clear();
	void			ClearTokens();

	int				Get();						// �� ������ 0, ������ -1
	const char*		GetParam(int index);
	const char*		GetParamNotUseMacro(int index);
	const char*		GetUnknown() { return m_unknown.data(); }

	// ������ �߰��Ѵ�.
	bool			Add(int code, const char* pGrammer);
	// Macro�� �߰��Ѵ�.
	void			AddMacro(char* pMacro, char* pReal);		// �ߺ��Ǹ� udpate�Ѵ�.

	// debug��
	void			PrintGrammer();
	int				GetCurLine() { return m_line; }				// ������ �߻����� �� ã�� �� �ְ� �Ѵ�.

protected:
	bool			IsSame(int t1, string& s1, int t2, string& s2);
};

#endif

