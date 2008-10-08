#include "PSExportFileDlg.h"

CPSExportFileDlg::CPSExportFileDlg(BOOL bOpenFileDialog, LPCTSTR lpszDefExt, LPCTSTR lpszFileName,
								   DWORD dwFlags, LPCTSTR lpszFilter, CWnd* pParentWnd) :
CFileDialog(bOpenFileDialog, lpszDefExt, lpszFileName, dwFlags/* & ~OFN_OVERWRITEPROMPT*/, lpszFilter, pParentWnd)
{
	ZeroMemory(&m_aviOpts, sizeof m_aviOpts);
	m_aviOpts.fccType = streamtypeVIDEO;
	m_aviOpts.dwKeyFrameEvery = (DWORD) -1; // Default
	m_aviOpts.dwQuality = (DWORD) ICQUALITY_DEFAULT;
	m_aviOpts.dwFlags = AVICOMPRESSF_VALID | AVICOMPRESSF_KEYFRAMES;
	m_aviOpts.dwInterleaveEvery = 1;
}

BOOL CPSExportFileDlg::OnInitDialog()
{
	CFileDialog::OnInitDialog();
	SetWindowText("Export To AVI");
	return TRUE;
}
BOOL CPSExportFileDlg::OnFileNameOK()
{
	CVideoCompression dlg(this);
	dlg.m_sCompressOpt = m_aviOpts;

	if(dlg.DoModal() == IDOK)
	{
		m_aviOpts = dlg.m_sCompressOpt;
	}

	return CFileDialog::OnFileNameOK();
}