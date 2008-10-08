#pragma once


// CDlgUpLoad dialog

class CDlgUpLoad : public CDialog
{
	DECLARE_DYNAMIC(CDlgUpLoad)

public:
	CDlgUpLoad(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgUpLoad();

// Dialog Data
	enum { IDD = IDD_UPLOAD };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CString m_strScenario;
};
