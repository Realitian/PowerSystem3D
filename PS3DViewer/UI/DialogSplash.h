#pragma once
#include "afxwin.h"
#include "resource.h"

// CDialogSplash dialog

class CDialogSplash : public CDialog
{
	DECLARE_DYNAMIC(CDialogSplash)

public:
	CDialogSplash(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDialogSplash();

// Dialog Data
	enum { IDD = IDD_SPLASH };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnTimer(UINT_PTR nIDEvent);
public:
	virtual BOOL OnInitDialog();
public:
	afx_msg void OnPaint();

private:
	unsigned char* m_pBmpBuffer;
	unsigned char* m_pTitleBuffer;
	unsigned char* m_pTxtBuffer;
	BITMAPINFO m_BMI;
	BITMAPINFO m_BMITitle;
	BITMAPINFO m_BMITxt;
protected:
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
};
