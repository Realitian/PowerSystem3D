#pragma once
#include "resource.h"
#include "afxcmn.h"
#include "afxwin.h"
#include "vfw.h"

// CDlgRenderConfig dialog

class CDlgRenderConfig : public CDialog
{
	DECLARE_DYNAMIC(CDlgRenderConfig)

public:
	CDlgRenderConfig(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgRenderConfig();

// Dialog Data
	enum { IDD = IDD_RENDERCONFIG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual void OnOK();

	DECLARE_MESSAGE_MAP()
public:
	short m_nFps;
	CString m_aviPath;
	AVICOMPRESSOPTIONS m_aviOpts;
	
	CEdit m_cAviPath;
	
	afx_msg void OnDeltaposSpinFps(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedFile();
	afx_msg void OnEnChangeFilepath();
};
