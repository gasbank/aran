#pragma once

// CMaterialListProperty command target

class CMaterialListProperty : public CMFCPropertyGridProperty
{
public:
	CMaterialListProperty(const CString& strName, const COleVariant& varValue, LPCTSTR lpszDescr = NULL, DWORD_PTR dwData = 0,
		LPCTSTR lpszEditMask = NULL, LPCTSTR lpszEditTemplate = NULL, LPCTSTR lpszValidChars = NULL);
	virtual ~CMaterialListProperty();

	void OnClickButton(CPoint point);
};


