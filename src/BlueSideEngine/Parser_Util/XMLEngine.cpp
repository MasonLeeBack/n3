#include "stdafx.h"
#include ".\XMLEngine.h"
#include <comdef.h>

CXMLEngine::CXMLEngine(void)
{
	m_pXMLDoc = NULL;
	CoInitialize(NULL);
	CoCreateInstance(CLSID_DOMDocument, NULL, CLSCTX_ALL, IID_IXMLDOMDocument, (void**)&m_pXMLDoc);
}

CXMLEngine::~CXMLEngine(void)
{
	CoUninitialize();
}

int CXMLEngine::Open( LPCTSTR pFileName )
{
	HRESULT	hr;
	BOOL bRet = TRUE;

	try
	{
		FILE* fp;
		fp = fopen( pFileName, "rt" );
		if( !fp ) return FALSE;
		fclose( fp );

		short sResult = FALSE;

		m_pXMLDoc->put_async(FALSE);

		_bstr_t varString = (LPCTSTR)pFileName;
		VARIANT path;
		path.vt = VT_BSTR;
		path.bstrVal = varString;

		hr = m_pXMLDoc->load(path, &sResult);
		if(FAILED(hr)) return FALSE;
	}
	catch (...)
	{
		SAFE_RELEASE(m_pXMLDoc);
		bRet = FALSE;
	}

	return bRet;
}

int CXMLEngine::Close()
{
	SAFE_RELEASE(m_pXMLDoc);
	return 1;
}

IXMLDOMNodeList* CXMLEngine::FindElement( LPCTSTR strElement )
{
	IXMLDOMNodeList* pNodeList = NULL;

	if(m_pXMLDoc == NULL)
		return pNodeList;

	try 
	{
		_bstr_t bstrPath = strElement;
		m_pXMLDoc->selectNodes(bstrPath, &pNodeList);
	}
	catch(...)
	{
	}

	return pNodeList;
}

IXMLDOMNodeList* CXMLEngine::FindElement( IXMLDOMNode* pNode, LPCTSTR strElement )
{
	IXMLDOMNodeList* pNodeList = NULL;

	if(pNode == NULL)
		return pNodeList;

	try 
	{
		_bstr_t bstrPath = strElement;
		pNode->selectNodes(bstrPath, &pNodeList);
	}
	catch(...)
	{
	}

	return pNodeList;
}

int	CXMLEngine::GetElementText( IXMLDOMNode* pNode, LPSTR strRet )
{
	BSTR			bstr = NULL;

	pNode->get_text( &bstr );
	strcpy( strRet,(const char*)_bstr_t(bstr,FALSE) );

	return 1;
}

int CXMLEngine::GetAttributeText( IXMLDOMNode* pNode, LPSTR strAttrName, LPSTR strRet )
{
	wchar_t					wstrAttr[128];
	IXMLDOMNode*			pAttrNode = NULL;
	IXMLDOMNamedNodeMap*	pMap = NULL;
	VARIANT					varValue;

	try
	{ 
		size_t n = mbstowcs( wstrAttr, strAttrName, 128 );
		pNode->get_attributes( &pMap );
		pMap->getNamedItem( wstrAttr, &pAttrNode );
		pAttrNode->get_nodeValue( &varValue );
		strcpy( strRet,(const char*)_bstr_t(varValue.bstrVal,FALSE) );

		SAFE_RELEASE( pAttrNode );
		SAFE_RELEASE( pMap );

		return TRUE;
	}
	catch(...)
	{
		SAFE_RELEASE( pAttrNode );
		SAFE_RELEASE( pMap );
		return FALSE;
	}
}