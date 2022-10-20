#pragma once
//----------------------------------------------------------------
//	현재 PC만 지원!!
//----------------------------------------------------------------

class CXMLEngine
{
	IXMLDOMDocument*	m_pXMLDoc;
public:
	CXMLEngine(void);
	~CXMLEngine(void);

	int						Open( LPCTSTR pFileName );
	int						Close();
	IXMLDOMNodeList*		FindElement( LPCTSTR strElement );
	IXMLDOMNodeList*		FindElement( IXMLDOMNode* pNode, LPCTSTR strElement );
	int						GetElementText( IXMLDOMNode* pNode, LPSTR strRet );
	int						GetAttributeText( IXMLDOMNode* pNode, LPSTR strAttrName, LPSTR strRet );
};