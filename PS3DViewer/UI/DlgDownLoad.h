#pragma once
#include "afxwin.h"


// CDlgDownLoad dialog

class CDlgDownLoad : public CDialog
{
	DECLARE_DYNAMIC(CDlgDownLoad)

public:
	CDlgDownLoad(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgDownLoad();

// Dialog Data
	enum { IDD = IDD_DOWNLOAD };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
private:

public:
	CStringArray m_strNames;
	CComboBox m_ComboBox;
	CString m_strName;
public:
	afx_msg void OnCbnSetfocusNamelist();
public:
	afx_msg void OnCbnSelchangeNamelist();
};
